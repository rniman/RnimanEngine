# 정적 모델 계약

[README](../README.md) · [로드맵](roadmap.md) · [공급 파일 규격](../ThirdParty/ReferenceImporter/FileFormat.md)

## 기준 에셋 준비

`Assets/` 전체는 Git 제외다. 새 체크아웃에서는 아래 BIN과 [README의 이미지 경로](../README.md#빌드와-실행)를 별도로 준비한다. JSON은 필드 대조용이며 런타임에서 읽지 않는다.

| Assets/Models/의 BIN | 정점·삼각형 | BaseColor textureRef | Wrap U/V |
|---|---|---|---|
| BrickWallCube.bin | 24·12 | BrickWall17_1K_BaseColor | Repeat / Repeat |
| BrickWallSphere.bin | 515·768 | BrickWall17_1K_BaseColor | Repeat / Repeat |
| DamagedHelmet.bin | 14,556·15,452 | DamagedHelmet_image_0 | Repeat / Repeat |

외부 Unity Exporter 프로젝트는 현재 로컬 `D:/DevelopTool/ModelExporter/Exporter`에 있으며 이 저장소에는 포함하지 않는다. 재생성은 원본 Scene·Transform·재질 설정을 유지해 BIN v2와 대조용 JSON을 함께 내보낸다. 이 저장소만으로 원본 Unity 장면을 재생성할 수는 없다.

v2는 텍스처 바인딩별 `wrapU`·`wrapV`를 저장한다(0 Repeat, 1 ClampToEdge, 2 MirroredRepeat). 이전 v1과 과거 실험 포맷은 **버전 숫자만 바꾸지 말고 원본에서 재익스포트**한다. 음수·1 초과 UV를 보존하며 Clamp·frac를 미리 적용하지 않는다.

## 실행과 데이터 흐름

1. `SandboxApp::Initialize` → `LoadStaticModel` → `custom_asset_export::ReadModel`: 파일 검증 후 CPU MeshData·Transform·BaseColor·textureRef·Wrap 반환.
2. Sandbox의 `MODEL_ASSETS`가 textureRef에 폴더·확장자를 붙이고 `LoadImage`로 PNG/JPEG를 디코딩한다.
3. `D3D12Renderer::Initialize(ModelSource 목록)`가 모델별 GPU 자원으로 복사한다. CPU 입력은 이후 해제할 수 있다.
4. 프레임마다 Transform 상수·모델 번호를 전달해 Phong으로 그리고 ImGui를 마지막에 기록한다.

누락·파싱·변환·이미지 로딩 실패는 경로와 원인을 보고하고 초기화를 중단한다. 다른 파일이나 고정 큐브로 대체하지 않는다. 텍스처 참조 자체가 없는 재질만 흰색 1×1 이미지를 사용한다.

## 지원 범위와 변환 책임

| 항목 | 계약 |
|---|---|
| 형식 | UNITY_CUSTOM_EXPORT Binary v2만 지원. JSON 로더 없음 |
| 메시 | 파일별 단일 메시·메시 노드·Submesh. 비어 있지 않은 삼각형·Position·단위 Normal·UV0 필수 |
| 계층·행렬 | 부모 선행 ID 참조. 파일의 열 벡터 행렬을 전치하고 행 벡터 World = Local × Parent로 합성. 양수 Scale TRS로 분해·재구성하며 전단·특이·반전·비유한 결과 거부 |
| 좌표·단위 | DirectXLeftHandedYUp만 허용. unitScale은 익스포트에 이미 적용되어 다시 곱하지 않음. 기준 큐브 한 변은 엔진 1단위 |
| UV·앞면 | uv0 × uvScale + uvOffset 이후 이미지의 위쪽 행 원점에 맞춰 V를 1−V로 한 번 변환. LH 출력 인덱스 유지·시계 방향 앞면 |
| 재질 | BaseColor·colorSpace·UV 변환·textureRef·Wrap 사용. colorSpace=1의 RGB는 선형 변환. alpha는 출력에 미사용. 재질 ID -1은 흰색 |
| 제한 | 불투명 Lit·뒤쪽 컬링만 허용. Unlit·alpha clip·반투명·다른 컬링은 오류. PBR 값·기타 맵은 미적용 진단 |
| 이미지 | 확장자 없는 UTF-8 textureRef. 큐브·구체는 벽돌 폴더의 PNG, Helmet은 별도 폴더의 JPEG. TSV·GUID 매핑·재귀 검색 없음 |

`Model 0`은 내보낸 Transform·회전 정지 상태로 시작한다. Model 1은 구체를 `(-2,0,0)`·Scale 1.25, Model 2는 Helmet을 `(2,0,0)`·Scale 0.8로 배치해 X/Z축으로 회전한다. UI 편집은 메모리에만 유지한다.

## 계층 지원 범위와 알려진 문제

Reader가 읽는 노드 계층과 엔진이 보존하는 장면 계층은 다르다. 부모 노드가 여러 개여도 메시 노드가 하나이고 합성 결과가 지원하는 TRS이면 로딩할 수 있다. 반환값에는 최종 Transform 하나만 남으며 부모 관계·개별 노드 편집은 보존하지 않는다. 여러 메시 노드는 거부한다. 부모의 비균일 Scale과 자식 회전으로 생기는 전단도 현재 TRS 계약 밖이다.

이전 Euler 기반 Transform에서는 pitch ±90도 근처에서 정상 TRS가 거부됐다. 재현 입력은 rotation `(π/2 − 0.001, −0.6, 0.4)`, scale `(2, 0.7, 1.3)`이다. 현재는 분해 결과의 Quaternion을 정규화해 직접 저장하므로 로딩에서 Euler 복원을 거치지 않는다. 양쪽 부호의 정확한 극점·인접 각도에서 행렬 보존을 회귀 검사하며, TRS로 표현할 수 없는 전단은 재구성 검사로 계속 거부한다. Binary v2 규격은 변경하지 않아 재익스포트는 필요 없다.

## 여러 모델의 GPU 수명과 선택

ModelSource는 호출 중에만 빌리는 이미지 참조·메시 포인터와 BaseColor·Wrap 값이다. Initialize 목록 순서가 모델 번호이며, 렌더러가 모델별 메시·텍스처·재질과 동일 번호의 SRV를 소유한다. 같은 번호의 인스턴스는 자원을 공유한다. 실행 중 모델 추가·삭제·교체는 미지원이다.

Render의 modelIndices는 비어 있으면 모델 0을 사용한다. 지정하면 Transform 수와 같고 모든 번호가 범위 안이어야 한다. Sandbox는 `{0,1,2}`를 전달한다. draw마다 메시·SRV·재질을 선택하며 PSO·조명·깊이 버퍼·Fence는 공유한다.

메시는 Upload Heap, 텍스처는 Default Heap에 보관한다. 큰 메시·스트리밍이 필요하면 메시 Default Heap 업로드를 검토한다. 같은 벽돌 이미지도 현재 두 모델에 각각 복사하며 캐시는 없다. GPU 완료 후 종료·부분 초기화 자원을 정리한다.

## Wrap과 UV 원점

`SurfaceTexture.wrapU/V → StaticModel → ModelSource → GpuModel → b2 samplerIndex(U*3+V) → s0~s8`로 전달한다. SM5.0에서 각 Sample의 Sampler 인덱스를 상수로 유지하려고 9개 조합을 등록한다. Filter는 단일 밉·선형이며 MirrorOnce는 미지원이다. UV 경계 처리는 GPU Sampler가 맡는다.

b2는 선형 RGB 3개와 Wrap 조합 uint 1개다. Object·Frame 상수와 동기화는 [렌더링 설계](design.md)를 따른다. nullptr 메시의 기본 큐브는 Smoke의 독립 회귀 기준으로 유지한다.

## 검증

- ModelChecks: 실제 BrickWallCube의 개수·bounds·재질과 GPU Phong·UV·색상을 비교한다. TRS·재질 변환·잘못된 Wrap·이전 v1·손상 입력·전단/반전/0 법선 거부는 별도의 [고정 CubeV2 fixture](../Tests/Assets/ModelFixture/README.md)를 사용한다. 바이트 offset은 이 fixture에만 적용하므로 데모 재익스포트 시 손상 검사 입력은 변하지 않는다.
- MultiModelChecks: JPEG RGBA·alpha·형식 제한, 세 모델의 개별/동시 렌더링·순서 변경·CPU 입력 해제·리사이즈 비교.
- [Wrap fixture 9개](../Tests/Assets/WrapModes/README.md): 축별 모드가 로더부터 GPU texel까지 유지되는지 확인. LegacyModels의 BIN 2개는 과거 포맷 거부용이다.

테스트 이미지는 메모리 또는 Tests/Assets에서 읽지만 세 기준 모델 BIN은 별도 준비해야 한다.
