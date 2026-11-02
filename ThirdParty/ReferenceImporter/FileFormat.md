# Unity Custom Export 파일 포맷 — v2

v2는 텍스처 바인딩마다 Wrap U/V를 추가한다. Writer와 ReferenceImporter는 **v2만 지원**한다. Wrap을 저장하지 않은 이전 v1과 과거 실험 포맷(v1~v8)은 지원하지 않으며 원본 모델에서 재Export해야 한다. 기존 파일에 기본 Wrap을 추측해 채워 넣지 않는다.

## 기본 인코딩과 규약

- 모든 정수는 32비트 little-endian, 실수는 IEEE 754 float32다. 패딩은 없다.
- 문자열은 uint32 UTF-8 바이트 길이 + 해당 바이트이며 null 종단자는 없다. 잘못된 UTF-8은 거부한다.
- 배열은 uint32 원소 수 + 원소들이다. 아래 float32[N]은 길이 접두사 없는 고정 벡터다.
- JSON은 같은 논리 데이터를 담는다. 배열 Count는 별도 키로 쓰지 않고 배열 길이를 사용한다. bool 성격의 필드도 0/1 정수다.
- 행렬은 row-major 저장·column-vector 계산이다. Translation은 3, 7, 11이며 `world = parentWorld × local`, `p' = M × p`다.
- Unit Scale은 Export 설정으로만 존재하며 위치·Translation·Bounds에 이미 적용한다. Import에서 다시 곱하지 않는다.

## Binary 전체 순서

Header → nodeCount/Node 배열 → meshCount/Mesh 배열 → materialCount/Material 배열. 마지막 Material 뒤의 추가 바이트는 오류다.

### Header

| 순서 | 필드 | 인코딩 |
|---|---|---|
| 1 | magic | 고정 ASCII `UNITY_CUSTOM_EXPORT` 19바이트 |
| 2 | version | uint32, 2 |
| 3 | coordinateSystem | uint32: 0 UnityOriginal, 1 DirectXLeftHandedYUp, 2 RightHandedYUp |

헤더는 27바이트다. unitScale/matrixLayout/vectorConvention은 기록하지 않는다.

### Node

| 순서 | 필드 | 인코딩 |
|---|---|---|
| 1 | id | int32 |
| 2 | parentId | int32, 루트는 -1 |
| 3 | name | 문자열 |
| 4 | meshId | int32, 메시 없으면 -1 |
| 5 | localMatrix | float32[16], JSON에서는 4×4 배열 |
| 6 | materialIds | uint32 count + int32 배열 |

루트는 id=0, parentId=-1, Identity Matrix다. 부모는 자식보다 먼저 기록한다. ID는 배열 위치가 아니며 이름 중복과 비연속 ID를 허용한다. materialIds는 Mesh의 Submesh 순서와 대응하고 -1은 엔진 기본 머테리얼이다. 메시 없는 노드는 슬롯이 비어야 한다.

### Mesh

| 순서 | 필드 | 인코딩 |
|---|---|---|
| 1 | id | int32 |
| 2 | name | 문자열 |
| 3 | bounds.center | float32[3] |
| 4 | bounds.extents | float32[3] |
| 5 | positions | count + float32[3] 배열 |
| 6 | normals | count + float32[3] 배열 |
| 7 | uv0 | count + float32[2] 배열 |
| 8 | tangents | count + float32[4] 배열 |
| 9 | submeshes | count + Submesh 배열 |

각 Submesh는 uint32 indexCount + uint32[indexCount] indices다. 별도 index 필드는 없으며 배열 순서가 번호다. Index는 Mesh 전체 positions를 참조한다. Triangle List만 지원하고 인덱스 수는 3의 배수다. 별도 baseVertex는 없다.

여러 노드가 meshId로 동일 메시를 공유할 수 있다. Normal/UV0/Tangent는 없으면 빈 배열, 있으면 정점 수와 같아야 한다. Tangent xyz는 단위 방향, w는 ±1이다. Bounds extents는 음수가 될 수 없다.

### Material

먼저 int32 id와 문자열 name을 기록한 뒤 아래 순서로 기록한다.

| 필드 | 타입 | 의미 |
|---|---|---|
| `shadingModel` | uint32 | 0 Metallic PBR, 1 Unlit |
| `colorSpace` | uint32 | 색상 상수 RGB: 0 Linear, 1 sRGB/gamma; alpha는 선형. URP Lit/Unlit은 GetColor 값을 1로, glTFast는 Linear 상수를 0으로 출력 |
| `baseColor` | float32[4] | RGBA 원본 색상 상수, HDR 허용 |
| `metallic` | float32 | 맵이 없을 때 금속성 [0,1] |
| `gloss` | float32 | 거칠기/매끈함 값 [0,1]; 맵이 있으면 샘플에 곱하는 배율 |
| `glossMode` | uint32 | 0 Roughness, 1 Smoothness. 현재 URP 추출은 1 |
| `normalScale` | float32 | Normal 강도 |
| `occlusionStrength` | float32 | AO 강도 [0,1] |
| `heightScale` | float32 | Parallax 높이 배율; 실제 displacement 아님 |
| `emission` | float32[3] | 발광 RGB 상수, HDR 허용; 비활성 발광은 0 |
| `surface` | uint32 | 0 Opaque, 1 Straight-alpha blend |
| `alphaClip` | uint32 | 0/1; 최종 alpha가 cutoff 미만이면 버림 |
| `cull` | uint32 | 0 None, 1 Front, 2 Back |
| `alphaSource` | uint32 | 0 BaseColor.a × BaseMap.a, 1 BaseColor.a만 사용 |
| `alphaCutoff` | float32 | [0,1] |
| `uvScale` | float32[2] | 모든 맵의 공통 UV 타일링 |
| `uvOffset` | float32[2] | 모든 맵의 공통 UV 이동 |
| `textures` | uint32 count + Texture 배열 | 활성 맵만 포함, 0~7개 |

### Texture

uint32 usage → 문자열 textureRef → uint32 channel → uint32 wrapU → uint32 wrapV 순서다. v1 Texture 레코드보다 8바이트 증가한다.

- usage: 0 BaseColor, 1 Normal, 2 Metallic, 3 Gloss, 4 Occlusion, 5 Emission, 6 Height. 중복 용도는 오류다.
- channel: 0 R, 1 G, 2 B, 3 A, 4 RGB. BaseColor/Normal/Emission은 RGB, 나머지는 단일 채널이다.
- textureRef: 원본 텍스처 파일의 마지막 확장자를 제외한 이름. `Body.v2.png` → `Body.v2`. GLB/glTF 내장 Texture 서브에셋은 별도 파일명이 없으므로 `<모델 파일 stem>_<Unity Texture 이름>`을 사용한다(예: `DamagedHelmet_image_0`). 폴더·GUID·확장자는 전달하지 않는다. 대소문자와 Unicode를 보존한다.
- 빈 값·ASCII 공백(U+0020)만인 이름·`.`/`..`·`/`·`\`·`:`·ASCII 제어문자(0~31,127)는 거부한다. 저장되지 않은 임시 텍스처는 Export할 수 없다. 동명 에셋은 경고 후 같은 이름을 전달한다.
- wrapU / wrapV: 각각 0 Repeat, 1 ClampToEdge, 2 MirroredRepeat. 모든 바인딩에 명시하며 다른 값은 거부한다. Unity MirrorOnce는 지원하지 않고 Export 오류를 보고한다. 같은 textureRef도 바인딩마다 다른 Wrap을 가질 수 있다.
- 실제 경로·확장자·동명 충돌 해결·Filter·Normal 인코딩·파일 탐색은 엔진 책임이다. 매핑 테이블은 선택 사항이며 Exporter/Reader는 요구하지 않는다.

모든 맵은 `uv0 × uvScale + uvOffset`을 사용한다. 텍스처별 UV는 없다.

### UV 원점과 Wrap 적용

uv0는 Unity Mesh의 아래쪽 기준 UV를 그대로 기록한다. 음수·1 초과 값을 허용하며 Exporter/Reader에서 Clamp·frac·원점 변환을 적용하지 않는다. 좌표계 선택은 UV에 영향을 주지 않는다.

소비 엔진은 먼저 `uv = uv0 * uvScale + uvOffset`을 계산한다. 원본 PNG/JPEG를 위에서 아래로 저장한 행 순서로 GPU에 올리는 엔진에서는 이후 `v = 1 - uv.y`를 정확히 한 번 적용하고 바인딩의 Wrap U/V로 샘플링한다. 이미지 행 자체를 뒤집는 구현은 UV까지 중복 반전하지 않는다.

Repeat는 `x - floor(x)`, MirroredRepeat는 `1 - abs((x - 2 * floor(x / 2)) - 1)`, ClampToEdge는 가장자리 texel을 사용한다. 이 수식은 의미 설명이며 UV 데이터에 미리 굽지 않는다. GPU Sampler가 경계 처리와 필터링을 함께 수행한다.

### 샘플 적용 규칙

- BaseColor: colorSpace에 따라 Linear로 해석한 상수 RGB × sRGB 디코딩한 BaseColor 맵 RGB. 맵이 없으면 상수만 사용한다.
- Normal: 선형 데이터로 읽고 normalScale을 적용한다. 원본 이미지의 RGB/RG 인코딩·Y 부호·Filter는 엔진 카탈로그가 지정하며 Wrap은 해당 바인딩 값을 따른다.
- Metallic: 맵이 있으면 해당 채널이 상수를 대체한다.
- Gloss: 맵이 있으면 `sample[channel] * gloss`, 없으면 `gloss`. glossMode=1이면 엔진이 `roughness = 1 - smoothness`로 변환한다.
- AO: `lerp(1, sample[channel], occlusionStrength)`. 맵이 없으면 1.
- Emission: colorSpace에 따라 Linear로 해석한 상수 RGB × sRGB 디코딩한 맵 RGB. 맵이 없으면 상수만 사용한다.
- Height: 단일 채널과 heightScale을 Parallax 입력으로 사용한다. UV 기준은 `uv0 * uvScale + uvOffset`이다.
- 최종 리소스는 BaseColor/Emission RGB=sRGB, 나머지=Linear data 계약에 맞춰 준비한다. 이미지 변환·복사는 Exporter가 수행하지 않는다.

### 추출·검증·소비 경계

Unity 기본 지원 입력은 URP Lit Metallic workflow와 URP Unlit이며, glTFast 입력은 아래 규칙을 따른다. Exporter가 지원 셰이더의 프로퍼티/키워드를 해석하여 공통 의미로 추출한다. URP 맵의 UV는 실제 셰이더에서 쓰는 BaseMap UV를 전달한다. 색상과 Smoothness 수치는 변환하지 않는다. 비활성 텍스처, 호환용 중복 프로퍼티, Material에 연결되지 않은 독립 Roughness는 포함하지 않는다.

glTFast 6.x URP의 `Shader Graphs/glTF-pbrMetallicRoughness`와 `Shader Graphs/glTF-unlit`도 지원한다. GLB/glTF는 v2의 공통 표면·텍스처 바인딩 배치를 사용한다. BaseColor는 glTFast의 gamma 저장값을 Linear로 변환하고 Emission은 Linear로 유지해 `colorSpace=0`으로 기록한다. Metallic=B, Roughness=G(`glossMode=0`), AO=R이며 Roughness factor는 gloss에 기록한다. 텍스처별 UV 변환은 모든 활성 맵이 동일한 UV0 scale/offset을 쓸 때만 공통 UV에 기록한다. 회전·UV1·서로 다른 변환은 오류다. Metallic 맵과 1이 아닌 metallicFactor의 곱은 v2에서 표현할 수 없어 거부한다. Straight alpha만 지원하며 Transmission/ClearCoat/Specular는 지원하지 않는다.

알 수 없는 셰이더, Specular workflow, Additive/Premultiply/Multiply, 활성 Detail/ClearCoat는 오류다. MaterialPropertyBlock은 경고하고 shared Material만 읽는다. 이 계약은 일반 표면 표현용이며 Unity의 렌더패스·GI·조명 토글·베이크·전체 셰이더 효과를 복제하지 않는다.

Reader의 Material은 id/name/surface로 구성된다. JSON에서는 표면 필드를 Material 객체에 직접 기록한다.

엔진의 `PrepareMaterial`은 surface를 엔진 데이터로 변환하고 이름→경로 매핑으로 파일을 연결한다. 색 공간·Smoothness 변환도 엔진에서 수행한다. JSON과 BIN은 동일한 논리 데이터를 담는다.

## 좌표·검증·소비

UnityOriginal과 DirectXLeftHandedYUp은 현재 같은 축을 사용한다. RightHandedYUp은 Z를 반사하고 삼각형 winding을 뒤집는다. 배율 s, Z 반사 C를 사용하면 Position은 sCp, Normal은 Cn, Bounds center는 sCc, extents는 s·extents다. Local Matrix는 CMC로 축 변환하고 Translation에 s를 적용한다. Node의 음수·비균일 Scale은 Matrix에 보존한다. 소비 엔진은 자기 행렬 규약과 Normal/Cull 처리를 적용한다.

모든 수치는 유한해야 한다. ID는 각 테이블에서 유일한 음이 아닌 정수, 참조는 존재하는 ID 또는 지정 sentinel이어야 한다. 빈 컨테이너·빈 Mesh·빈 Submesh는 허용하지만 Root가 없는 파일은 오류다. 활성 텍스처는 UV0, Normal/Height는 유효한 Normal/Tangent basis가 필요하다.

Reader는 할당 전 Count와 남은 바이트를 검사하고 ReadLimits를 적용한다. 기본 한도: 파일 256 MiB, 문자열 1 MiB, Node/Mesh/Material 각각 100,000개, 배열 16,000,000개, 누적 디코딩 할당 512 MiB. 누적 예산은 실제 프로세스 메모리의 엄밀한 상한이 아니다. 오류는 코드·바이트 위치를 가진 ReadError이며 부분 모델을 반환하지 않는다. 메모리 부족은 표준 할당 예외로 보고될 수 있다.

ReadModel은 값과 참조만 복원한다. 엔진 예제 PrepareMaterial의 EngineMaterial은 별도 CPU 구조이며 실제 이미지 디코딩·GPU 업로드를 수행하지 않는다.

## JSON 표현

최상위 키는 magic/version/coordinateSystem/nodes/meshes/materials다. coordinateSystem은 열거값 이름 문자열, localMatrix는 네 행의 배열, bounds는 center/extents 객체다. Submesh 객체는 indices만 포함한다. Material의 모든 표면 필드는 같은 객체에 놓이며 별도 surface 중첩 객체가 아니다. 필드 의미는 [빠른 참고](FieldReference.md)를 따른다.

## 파일 출력과 일관성

- `Model Relative Path`는 프로젝트 `Exports/` 기준 출력 폴더다. 빈 값은 `Exports/` 바로 아래에 출력하며 하위 폴더는 출력 시 생성한다. 모델 출력 폴더 자체는 파일에 기록하지 않는다.
- 상대 폴더 경로는 해석 시 구분자를 `/`로 통일하고 중복 구분자와 `.`를 정리한다. 절대·드라이브 경로, `..`, 제어 문자와 `< > : " | ? *`, 끝이 공백·마침표인 폴더, Windows 예약 장치명은 거부한다. 원본 Inspector 문자열 자체를 다시 쓰지는 않는다.

- Inspector 다중 Export는 파일명 정리 후 절대 경로를 대소문자 구분 없이 비교하여 중복을 사전 거부한다. 충돌이 있으면 어떤 선택 대상도 Export하지 않는다. 단일 대상의 기존 출력 갱신은 허용한다. 이 검사는 여러 대상을 하나의 저장 트랜잭션으로 묶거나 서로 다른 실행 간 쓰기를 잠그는 기능은 아니다.

- JSON과 Binary를 함께 선택하면 하나의 출력 묶음으로 처리한다.
- 두 파일을 모두 임시 경로에 완성하기 전에는 기존 최종 파일을 변경하지 않는다.
- 최종 파일 교체 중 실패하면 같은 폴더에 만든 백업을 사용해 이미 교체한 파일을 이전 상태로 복구한다.
- 개별 임시 파일은 최종 파일과 같은 폴더에 만들어 같은 파일 시스템 안에서 이동한다.
- 정상 완료 후 임시 파일과 백업 파일의 제거를 시도한다. 정리 중 I/O·접근 권한 오류가 나면 경고로 보고하며, 완료된 Export를 실패로 바꾸지는 않는다.
- 복구 중 한 파일에서 실패해도 나머지 복구는 계속 시도한다. 복구 실패 경로와 백업 위치를 보고하고 실패한 백업은 보존한다.
- 프로세스 강제 종료나 전원 차단 중 서로 다른 두 최종 경로를 동시에 교체하는 것까지는 보장하지 않는다.

이 출력 처리 정책은 직렬화되는 데이터 Layout을 바꾸지 않으므로 Format Version에 영향을 주지 않는다.

## 버전 정책

v2는 v1에 바인딩별 Wrap U/V를 추가한 현재 지원 기준이다. v1을 포함한 이전 파일은 원본에서 재Export한다. 이후 호환되지 않는 배치·의미 변경은 새 버전을 사용하고 Writer·Reader·검증기·명세를 함께 변경한다. 과거 실험 버전 지원은 재도입하지 않는다.
