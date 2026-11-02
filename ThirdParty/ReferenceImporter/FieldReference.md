# BIN / JSON 필드 빠른 참고 — v2

필드당 한두 문장으로 의미를 설명한다. JSON과 BIN은 같은 논리 데이터를 담는다. 정확한 직렬화 순서·범위는 [FileFormat](FileFormat.md)을 기준으로 한다.

## 파일 전체

| 필드 | 의미 |
|---|---|
| `magic` | 이 포맷인지 식별하는 `UNITY_CUSTOM_EXPORT`. BIN은 길이 접두사 없는 고정 ASCII다. |
| `version` | 포맷 버전. 현재 출력은 2, C++ Reader는 2만 읽는다. v1은 재Export한다. |
| `coordinateSystem` | 출력 좌표계. JSON은 이름, BIN은 0 UnityOriginal / 1 DirectXLeftHandedYUp / 2 RightHandedYUp이다. |
| `nodes` | 부모·자식 관계와 메시/머테리얼 참조를 가진 오브젝트 목록. |
| `meshes` | 고유 메시 데이터 목록. 여러 노드가 하나의 메시를 참조할 수 있다. |
| `materials` | 표면 값과 사용 중인 텍스처 참조 목록. 실제 텍스처 파일은 포함하지 않는다. |

DirectXLeftHandedYUp은 현재 Unity와 같은 축 방향을 사용한다. RightHandedYUp은 Z 반사와 삼각형 winding 변환을 적용한다. 파일의 좌표 변환을 Import 단계에서 반복하지 않는다.

행렬은 row-major 저장·column-vector 계산(`M × p`)으로 고정한다. Unit Scale은 좌표·이동·Bounds에 이미 적용하며 파일에는 기록하지 않는다. Submesh 번호는 배열 순서(0부터)다.

## Node — `nodes[]`

| 필드 | 의미 |
|---|---|
| `id` | 파일 안에서 노드를 구분하는 정수 키. 이름이나 배열 위치로 대체하지 않는다. |
| `parentId` | 부모 노드 ID. `-1`이면 최상위 노드다. |
| `name` | 원본 오브젝트 이름. 중복 이름을 허용한다. |
| `meshId` | 사용할 메시 ID. `-1`이면 메시 없는 계층 노드다. |
| `localMatrix` | 부모 기준 4×4 변환. 현재 규약에서는 `world = parentWorld × local`이다. |
| `materialIds` | Submesh 순서에 대응하는 머테리얼 ID 배열. `-1`은 엔진 기본 머테리얼이다. |

ID는 파일 내부 참조이며 편집 전후에 유지되는 영구 에셋 ID가 아니다. `Output Name Override`는 파일명만 바꾸고 이 이름/ID를 바꾸지 않는다.

## Mesh — `meshes[]`

| 필드 | 의미 |
|---|---|
| `id` / `name` | 메시의 파일 내부 키와 원본 이름. 노드는 `meshId`로 연결한다. |
| `bounds.center` | 메시 로컬 공간 AABB의 중심. 좌표·단위 변환이 적용된 값이다. |
| `bounds.extents` | AABB 중심에서 각 축 끝까지의 거리. 전체 크기의 절반이다. |
| `positions` | 정점의 로컬 위치 `[x,y,z]` 배열. |
| `normals` | 정점 법선 `[x,y,z]` 배열. 없으면 빈 배열이며 있는 경우 정점 수와 일치한다. |
| `uv0` | 첫 번째 UV `[u,v]` 배열. 표면 텍스처를 사용하면 필요하다. |
| `tangents` | 정점 접선 `[x,y,z,w]`. xyz는 방향, w는 bitangent 방향을 결정하는 ±1 부호다. |
| `submeshes` | 그리기 단위별 인덱스 목록. 머테리얼 슬롯과 순서대로 대응한다. |
| `submeshes[].indices` | 삼각형 정점 인덱스. 세 값씩 한 삼각형이며 positions를 참조한다. |

Normal/Height 맵은 유효한 법선·탄젠트가 필요하다. 탄젠트가 없으면 Export용 복사본에서 생성할 수 있으며 원본 메시를 수정하지 않는다.

## Material — `materials[]`

| 필드 | 의미 |
|---|---|
| `id` / `name` | 파일 내부 머테리얼 키와 원본 이름. 노드의 materialIds에서 참조한다. |
| `shadingModel` | `0` Metallic PBR, `1` Unlit. Unity 셰이더 이름을 저장하지 않는다. |
| `colorSpace` | 색상 **상수 RGB**의 해석: `0` Linear(glTFast), `1` gamma/sRGB(URP Lit/Unlit 원본 GetColor). alpha는 선형이다. |
| `baseColor` | 기본 색상 RGBA. BaseColor 맵이 있으면 색상과 곱한다. |
| `metallic` | Metallic 맵이 없을 때의 금속성 `[0,1]`. 맵이 있으면 선택 채널이 이 값을 대체한다. |
| `gloss` | Roughness/Smoothness 값 `[0,1]`. Gloss 맵이 있으면 샘플에 곱하는 배율이다. |
| `glossMode` | `0` Roughness(GLB/glTF), `1` Smoothness(URP Lit). Smoothness는 엔진이 필요하면 `1 - smoothness`로 변환한다. |
| `normalScale` | Normal 맵 강도. 맵이 없으면 사용하지 않는다. |
| `occlusionStrength` | AO 강도 `[0,1]`. 결과는 `lerp(1, AO 샘플, 강도)`이며 맵이 없으면 1이다. |
| `heightScale` | Height 맵의 Parallax 높이 배율. 실제 메시 변형이나 Displacement를 뜻하지 않는다. |
| `emission` | 발광 RGB 상수. 맵이 있으면 곱하며, 발광 비활성 시 0이다. 주변을 비추는 기능은 렌더러가 구현한다. |
| `surface` | `0` 불투명, `1` 일반 straight-alpha 반투명. 정렬·깊이 처리 등은 엔진에서 결정한다. |
| `alphaClip` | `0` 비활성, `1`이면 최종 alpha가 cutoff 미만인 픽셀을 버린다. |
| `cull` | `0` 제거 없음(양면), `1` 앞면 제거, `2` 뒷면 제거. |
| `alphaSource` | `0` baseColor.a × BaseColor 맵 alpha, `1` baseColor.a만 사용. 맵이 없으면 샘플 alpha는 1이다. |
| `alphaCutoff` | Alpha Clip 기준값 `[0,1]`. alphaClip이 0이면 사용하지 않는다. |
| `uvScale` | 모든 텍스처에 공통 적용할 UV 타일링. 기본 `[1,1]`. |
| `uvOffset` | 모든 텍스처에 공통 적용할 UV 이동. 기본 `[0,0]`; 최종 UV는 `uv0 × uvScale + uvOffset`. |
| `textures` | 연결된 활성 텍스처만 포함. 비어 있으면 색상·수치만으로 표현한다. |

## Texture — `materials[].textures[]`

| 필드 | 의미 |
|---|---|
| `usage` | 연결할 표면 용도. 아래 표의 값으로 구분하며 같은 용도는 중복될 수 없다. |
| `wrapU` / `wrapV` | 축별 주소 모드: `0` Repeat, `1` ClampToEdge, `2` MirroredRepeat. |
| `textureRef` | 원본 텍스처 파일 stem. GLB/glTF 내장 이미지는 `<모델 stem>_<Texture 이름>`(예: `DamagedHelmet_image_0`). 실제 경로·확장자는 엔진이 결정한다. |
| `channel` | `0` R / `1` G / `2` B / `3` A / `4` RGB. 패킹된 맵에서 사용할 값을 고른다. |

| usage | 용도 | 채널 |
|---:|---|---|
| 0 | BaseColor | 4 RGB; 투명도는 alphaSource 규칙 적용 |
| 1 | Normal | 4 RGB; 최종 이미지의 Normal 인코딩은 엔진 정책 |
| 2 | Metallic | 단일 채널; URP Lit는 R, glTFast는 B |
| 3 | Gloss | 단일 채널; URP Smoothness는 A, glTFast Roughness는 G |
| 4 | Occlusion(AO) | 단일 채널; URP Lit는 G, glTFast는 R |
| 5 | Emission | 4 RGB |
| 6 | Height | 단일 채널; 현재 URP는 G |

BaseColor/Emission 이미지 RGB는 sRGB, Normal·수치 맵은 Linear data로 준비한다. Wrap U/V는 텍스처 바인딩 필드다. 파일 확장자·Filter·Normal 디코딩·GPU 핸들은 엔진 리소스 설정이다.

## BIN을 직접 읽을 때

실수는 float32, 수치는 little-endian이며 구조체 padding이 없다. 문자열은 uint32 바이트 길이 + UTF-8, 배열은 uint32 개수 + 원소다. `magic`만 길이 접두사가 없다. JSON의 필드명을 BIN에 저장하지 않으므로 [정확한 필드 순서](FileFormat.md#binary-전체-순서)를 따라야 한다.

C++에서는 직접 파싱하기보다 `ReadModel()` 사용을 권장한다. v2 머테리얼은 `material.surface`에서 읽으며 머테리얼의 id/name/surface 외의 구버전 필드는 없다.
