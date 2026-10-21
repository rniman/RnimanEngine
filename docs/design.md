# 엔진 설계 (Architecture)

[프로젝트 소개와 실행](../README.md)

현재 Sandbox의 좌표계·실행 구조·소유권·렌더링 계약과 향후 설계 방향을 관리한다. ImGui 최소 도입 → 정적 3D 모델 로딩·렌더링 → 일반 오브젝트와 ECS 비교 순서로 확장한다. 설계 기준의 장면·ECS·도구와 Unity 데이터 규격은 개발 방향이며 아직 구현하지 않았다. 기능 요약은 [README](../README.md#현재-상태), 다음 구현 범위는 [M02 계획](plans/m02-static-model.md), 시점별 결과는 [마일스톤](milestones/README.md)을 따른다.

## 설계 기준

하나의 솔루션에서 공통 소스는 Engine, 데모 실행·조작 코드는 Sandbox에 배치한다. 현재는 세 실행 프로젝트가 필요한 공통 원본을 직접 빌드하며, 실제 필요가 생길 때 라이브러리 분리를 고려한다. 새로운 추상화에는 해결할 문제와 사용 예시가 있어야 한다.

```text
입력 → 장면·게임 로직 갱신 → 렌더 데이터 수집 → D3D12 렌더러 → 화면 출력
Unity → bin → CPU 에셋 데이터 → 장면 구성 / GPU 리소스 생성
```

### 네임스페이스

2026-10-20 직접 작성한 C++ 코드의 단일 루트를 `rengine`으로 채택했다. 예를 들어 `rengine::Transform`, `rengine::D3D12Renderer`, `rengine::SandboxApp`, `rengine::LoadPngImage`로 사용한다. 이하 설명에서는 문맥이 분명하면 루트 이름을 생략한다.

Engine의 데이터·수학·로더·창·렌더러와 Sandbox의 앱·카메라·조명, 테스트의 RendererReadback·LightingChecks를 같은 루트에 둔다. 파일 내부 보조 타입·함수·상수는 그 안의 익명 네임스페이스에 유지한다. 헤더의 include는 네임스페이스 밖에 두고, 외부에서 사용할 때는 `rengine::`로 이름을 한정한다. 전역 별칭이나 헤더의 using namespace는 제공하지 않는다.

`main`·`wWinMain`은 전역에 유지하고 제품 코드를 한정된 이름으로 호출한다. Win32·DirectX·표준 라이브러리·HLSL은 기존 계약을 유지한다. 폴더나 프로젝트 이름에 따라 하위 네임스페이스를 만들지 않으며, 향후 독립 API·이름 충돌이 생기면 별도로 검토한다. 실제 적용·검증 기록은 [개발 문서](development.md#단일-루트-네임스페이스-적용-2026-10-20)를 따른다.

### 책임과 의존성

공통 원본은 Engine에 두고 Sandbox의 장면 구성·입력 조작·실행 수명과 구분한다. Engine은 Sandbox 헤더·PCH를 참조하지 않는다. 실제 배치·include 기준과 소비자별 원본 빌드 범위는 [프로젝트 구조](project-structure.md#현재-include-기준과-허용-의존성)를 따른다. 고정 큐브·Phong 파이프라인은 현재 렌더러의 제한으로 유지하며, 소스 이동을 범용 렌더링 API 완성으로 취급하지 않는다.

- 앱은 실행 수명주기와 엔진 구성을, 장면·ECS는 게임 상태와 갱신을 담당한다. Application·Scene과 ECS는 함께 사용할 수 있다.
- 렌더 데이터 수집 단계가 장면과 렌더러를 연결한다. 게임 로직과 ECS 저장소는 API 전용 명령 목록·디스크립터·PSO를 직접 다루지 않는다.
- 공통 렌더 데이터는 변환·카메라·조명·리소스 식별자 등으로 구성하고, 참조 대상과 유효 기간을 명확히 한다. D3D12 고유 기능은 백엔드 또는 명시적인 확장 경로에 둔다.
- 파일 파싱과 GPU 업로드를 분리하여 CPU 데이터는 렌더러 없이 검증할 수 있게 한다. 소유자와 초기화·종료 순서를 명시하고 전역 매니저로 의존성을 숨기지 않는다.
- 커스텀 할당자, 범용 매니저와 자체 분석 도구는 실제 필요에 따라 도입한다. 외부 코드는 입출력·소유권·실패 동작·버전·라이선스를 확인하고 활용한다.

### ECS

정적 모델을 로딩·렌더링하는 장면을 확보한 뒤 일반 오브젝트 방식과 ECS를 비교한다. 첫 비교 작업은 이동·회전, 개체 생성·삭제, 컴포넌트 조합별 갱신으로 정하고 Transform 계층은 후속 확장으로 둔다. 타입별 저장, sparse set, archetype 등의 선택 이유와 제약은 해당 구현 단계에서 기록한다.

일반 오브젝트 방식과 ECS가 같은 렌더 데이터를 만드는 작은 비교 샘플을 유지한다. 두 방식의 모든 기능을 동일하게 확장할 필요는 없다. 같은 개체 수·작업량·빌드 구성에서 갱신과 렌더 데이터 수집 비용을 비교하고, 사용 편의성과 수정 부담도 확인한다.

Entity 재사용과 오래된 핸들, 컴포넌트 참조 무효화, 순회 중 구조 변경 정책을 검증한다. 시스템의 실행 순서와 읽고 쓰는 데이터를 명시하며, ECS라는 이유만으로 성능이나 병렬화 이점을 가정하지 않는다.

### 렌더링과 도구

현재 큐브·Phong 데모에 ImGui 최소 편집·상태 표시를 먼저 연결한다. 이후 Unity의 정적 메시·Transform을 자체 bin으로 전달하고 같은 Phong 렌더링과 UI 편집 경로를 사용한다. 이 결과로 일반 오브젝트와 ECS 비교 장면을 구성한 뒤 재질·PBR로 확장한다. 첫 PBR 범위의 후보는 metallic-roughness 재질과 직접광이며, IBL·그림자·후처리는 별도 단계로 검증한다.

- 좌표계, 행렬 저장·곱셈·셰이더 전달 규칙과 색 공간을 명시한다.
- GPU 작업이 끝나기 전에 리소스·업로드 메모리·디스크립터를 재사용하거나 해제하지 않는다.
- 도구 UI는 엔진의 상태 변경 경로를 사용하며 Transform 갱신 규칙과 게임 입력 처리를 우회하지 않는다.
- ImGui 최소 범위는 Transform·카메라·조명 편집, 자동 회전 정지/재개와 프레임 상태 표시다. 플랫폼 입력은 UI에 전달하고 입력 캡처 상태로 앱 조작을 구분한다. UI용 GPU 자원도 기존 Fence·리사이즈·종료 계약을 따른다. 도입·편집의 상세 완료 기준은 [M02 계획](plans/m02-static-model.md#1단계--imgui-최소-도입)에서 관리한다.
- 지표에는 대상과 단위를 표시하고 CPU 시간, GPU 시간과 Present 대기를 구분한다. 깊은 분석에는 기존 도구도 활용한다.

## 좌표축과 행렬

| 항목 | 현재 Sandbox 계약 |
|---|---|
| 월드·로컬 | 월드 위쪽 `+Y`. 무회전 기준 오른쪽 `+X`·전방 `+Z`. 큐브의 의미상 로컬 정면 축과 물리적 거리 단위는 미정이며 외부 에셋 도입 시 결정 |
| 뷰 | `XMMatrixLookToLH`, 뷰 앞쪽 `+Z`. 카메라 전방은 yaw·pitch로 계산하며 월드 축은 회전하지 않음 |
| 투영·화면 | `XMMatrixPerspectiveFovLH`, near 0.1·far 100. 원근 나눗셈 후 NDC의 X/Y는 −1~1·Z는 0~1. 화면은 왼쪽 위 원점·오른쪽 +X·아래쪽 +Y |
| 벡터·합성 | 행 벡터 `p * M`. `World = S * R * T`, `WVP = World * View * Projection` |
| 저장·전달 | `XMStoreFloat4x4`의 행 우선 저장 → HLSL `row_major float4x4` → `mul(position, matrix)`. 패킹용 전치 없음 |
| 법선 | World 역전치의 3×3 부분으로 변환. 이 전치는 법선 변환에 필요하며 전달용 전치와 구분 |
| 회전 | `rotation.x/y/z`는 pitch(X)/yaw(Y)/roll(Z), 단위 radian. `XMMatrixRotationRollPitchYaw` 사용 |

행 우선은 **저장 순서**, 행 벡터는 **곱셈 방향**, 왼손은 **축의 방향 관계**다. 하나로 나머지를 추정하지 않는다. Direct3D 자체가 월드 축이나 왼손 좌표계를 강제하는 것은 아니며, 현재 카메라·투영 구현에 맞춘 프로젝트 선택이다. 앞면·깊이·GPU 레이아웃은 [렌더링 계약](#메시행렬렌더-데이터)을 따른다.

## Unity 데이터 규격

M02에서는 단일 정적 메시의 위치·법선·UV0·삼각형 인덱스와 Transform부터 전달한다. 기본 Phong 재질과 BaseColor PNG를 사용하고 한 메시를 여러 Transform으로 배치한다. 이후 ECS 비교와 실제 사용에 맞춰 재질·텍스처·씬 계층, 스켈레톤·애니메이션으로 범위를 확장한다. 지원할 데이터와 변환 규칙·파일 버전은 익스포터와 로더를 작성할 때 함께 명시한다. 첫 전달 범위와 완료 기준은 [모델 단계 계획](plans/m02-static-model.md#2단계--정적-모델-로딩과-렌더링)을 따른다.

- 파일 식별값·버전·바이트 순서·숫자 크기·문자열 인코딩·개수·리소스 참조를 명시한다. C++ 객체 메모리를 그대로 파일 형식으로 사용하지 않는다.
- 좌표계·단위·행렬·삼각형 winding·UV·노멀·탄젠트 변환과 텍스처 저장·참조 방식을 정한다. 애니메이션 추가 시 본 인덱스·bind pose·시간 단위·샘플링 규칙도 정한다.
- 파서는 파일 범위, 크기 계산의 오버플로, 인덱스와 참조를 검사한다. 잘린 파일·미지원 버전·데이터에서 원인을 알 수 있게 실패하고 부분 로딩 자원을 정리한다.
- 작은 기준 에셋으로 데이터와 화면 결과를 비교한다. 규격 변경 시 익스포터·로더·에셋·문서를 함께 맞추고 호환 또는 재익스포트 조건을 기록한다.

## Sandbox 실행 구조

### 진입점과 수명

[Main.cpp](../Sandbox/Main.cpp)의 `wWinMain` → [SandboxApp](../Sandbox/SandboxApp.cpp)의 `Run` → 창 생성·PNG 디코딩·GPU 초기화/업로드 → 메시지·리사이즈·입력/Transform 갱신 → 렌더링 → GPU 대기 → RAII 정리 순서다. 앱은 창을 렌더러보다 먼저 선언하여 역순 파괴에서 GPU 자원을 HWND보다 먼저 정리한다.

초기 클라이언트 크기는 1280×720이다. Win32 실패는 API·오류 코드, D3D12 실패는 API·HRESULT·가능한 Device removed reason을 보고한다. 부분 자원은 RAII로 정리하며 실패 후 재사용·장치 손실 자동 복구는 지원하지 않는다.

<a name="창입력-추출-2026-10-06"></a>

### 창·입력

[Win32Window](../Engine/Platform/Win32Window.h)는 창 하나·UI 스레드에서 창 수명·메시지·입력을 담당하며 GPU·카메라를 참조하지 않는다.

- `GetState`로 크기·최소화·포커스·리사이즈 요청을 읽는다. 앱은 GPU 리사이즈 성공 후 `ClearResizeRequest`를 호출하며 최소화·0 크기에서는 메시지를 기다린다.
- `ConsumeInput`은 복사본을 반환한다. 키 눌림은 유지하고 누적 마우스 이동·시간 초기화 요청을 비운다. 포커스 해제·최소화·모달 조작은 입력을 초기화하고 캡처 상실은 드래그를 중단한다.
- `GWLP_USERDATA`·HWND는 비소유다. 창 객체는 복사·이동 금지로 콜백 주소를 유지한다. `PumpMessages`는 `WM_QUIT` 코드를 optional로 반환하므로 0도 종료 요청이다. `WM_CLOSE` 후 GPU 정리를 마칠 때까지 HWND·콜백 상태를 유지한다.

<a name="앱-실행부-추출-2026-10-06"></a>

### 앱 실행

SandboxApp은 창·렌더러·카메라·조명·큐브 3개를 소유하며 복사·이동하지 않는다. `Run`은 한 번만 호출하며 메시지 → 최소화 대기 → 프레임 경계 리사이즈 → 입력 소비 → `Update` → `Render`를 반복한다. 정상 종료는 GPU 완료 후 WM_QUIT 코드, 초기화·실행·GPU 대기 실패는 1을 반환한다.

입력 해석은 앱, GPU 명령·동기화는 렌더러의 책임이다. 포커스가 있을 때 카메라·조명을 조절하며 입력 초기화 직후 이동 시간은 0, 일반 프레임은 최대 0.1초다. 큐브 회전은 입력 중단과 무관한 절대 경과 시간을 사용한다. [Transform](../Engine/Math/Transform.h)은 `Scale * Rotation * Translation`, [SandboxCamera](../Sandbox/SandboxCamera.h)는 초기 위치 `(0, 2, -5)`·원점 시선을 사용한다. 조작·속도는 README를 따른다.

### 프레임별 Fence 동기화

[D3D12Renderer](../Engine/Rendering/D3D12Renderer.cpp)는 GPU 자원과 두 프레임 슬롯의 Allocator·Constant Buffer·마지막 제출 Fence를 소유한다. 단일 Command List·Direct Queue·Fence/Event를 공유한다.

슬롯 Fence 대기 → 상수 복사 → Allocator/List Reset → 색상·깊이 clear/draw → PRESENT 복귀 → 제출·Present·Signal 순서다. Fence는 단조 증가하고 완료 값 `UINT64_MAX`는 장치 제거로 처리한다. 리사이즈·종료·미완료 제출이 있는 소멸자는 전체 GPU 완료를 시도하며 Signal·Present 실패도 정리 대상으로 남긴다. 소멸자의 `(void)WaitForGpu()`는 내부 진단 후 정리를 계속하는 예외이며 완료 보장을 뜻하지 않는다.

Resize는 동일 크기·0 크기를 무시하고 전체 GPU 대기 후 백 버퍼·깊이 버퍼를 재생성한다. 프레임 상수는 슬롯 완료 후에만 확장·복사하고 오브젝트 감소 시 축소하지 않는다.

## 큐브 렌더링 계약



좌표축·행렬 규약은 [좌표축과 행렬](#좌표축과-행렬)를 따른다.

### 메시·행렬·렌더 데이터

면별 위치·UV·법선 정점 24개와 `R16_UINT` 인덱스 36개의 고정 Upload Heap 메시를 공유한다. 왼손 View·Projection, 시계 방향 앞면·뒷면 컬링·전체 창 Viewport를 사용한다. D32 깊이는 매 프레임 1로 지우며 LESS·깊이 쓰기를 사용한다.

`SandboxApp::Render`가 [RenderData](../Engine/Rendering/RenderData.h)의 `BuildObjectTransformConstants`로 매 프레임 WVP·World·World 역전치를 계산한다. 결과인 `ObjectTransformConstants`는 오브젝트별 셰이더 변환 상수이며 장면 오브젝트나 GPU 자원을 생성하지 않는다. 유한한 아핀 World·양수 행렬식·유한한 역행렬/WVP를 요구하며 위반은 nullopt다. 반전 Transform의 컬링 보정은 없다. CPU는 행 벡터 행렬을 저장하고 [CubeVS](../Shaders/Samples/Sandbox/CubeVS.hlsl)는 `row_major`·`mul(position, matrix)`로 전치 없이 읽는다. 법선은 역전치의 3×3 부분으로 변환하고 PS에서 정규화한다.

`Render(std::span<const ObjectTransformConstants>, const FrameLighting&)`는 입력을 호출 중 복사하고 참조를 보관하지 않는다. ObjectTransformConstants 192바이트·FrameLighting 64바이트와 필드 offset을 static_assert로 확인한다. CB의 첫 256바이트는 PS b1, 이후 오브젝트마다 256바이트는 VS b0다. 빈 목록도 프레임 값을 기록하고 clear·Present하며 버퍼 크기 오버플로·생성 실패는 보고한다.

### 텍스처·sRGB

`SandboxApp::Initialize` → EXE 위치 + [SourcePaths.props](../Build/SourcePaths.props)가 빌드 시 전달한 저장소 루트 경로 → 루트 `Assets/`의 PNG → [AssetLoader](../Engine/Assets/AssetLoader.cpp)의 `LoadPngImage` → CPU [ImageData](../Engine/Assets/ImageData.h) → `D3D12Renderer::CreateTexture` 순서다. Debug·Release는 같은 원본을 읽으며 파일 복사·원본 쓰기·런타임 상위 폴더 검색은 하지 않는다. 셰이더 CSO는 계속 EXE 옆에서 읽는다. RendererSmoke는 같은 빌드 설정으로 `Tests/Assets/`를 직접 읽고 손상 파일은 테스트 EXE 옆에만 만든다.

- WIC가 단일 프레임 PNG를 위→아래, 행 패딩 없는 RGBA8로 디코딩한다. 크기·UINT 산술 범위·프레임 수를 검사하며 실패는 빈 이미지·HRESULT·API 이름으로 반환한다. 앱이 전체 경로와 함께 표시한다. STA 초기화 성공 횟수만 정리하며 기존 MTA는 실패한다. ICC 변환·EXIF 회전은 적용하지 않고 BaseColor를 sRGB로 간주한다.
- 렌더러는 각 축 1~16384와 `pixels.size() == width * height * 4`를 요구한다. 단일 밉 Default Heap 텍스처에 `GetCopyableFootprints`의 RowPitch로 행별 복사하고 COPY_DEST→PIXEL_SHADER_RESOURCE로 전환한다. 첫 사용 전 GPU 완료 후 Upload Heap을 해제한다. 대기 실패 시 업로드 자원을 멤버에 유지해 소멸자의 완료 시도까지 보존한다.
- 면마다 이미지 전체를 사용하며 비균일 Scale에서는 무늬도 늘어난다. 모든 큐브는 SRV t0·정적 선형 Clamp Sampler s0를 공유하고 리사이즈에서도 텍스처를 유지한다.
- Swap Chain은 RGBA8 UNORM, SRV·RTV와 PSO 출력은 RGBA8 sRGB다. 선형 RGB로 읽고 조명을 계산한 뒤 sRGB로 기록한다. clearColor도 선형이며 PS alpha는 항상 1로 불투명 가림을 유지한다.

### 기본 Phong 조명

`SandboxApp::Update` → [SandboxLighting](../Sandbox/SandboxLighting.h)의 `Adjust`/`Reset` → 앱 Render → 오브젝트 행렬·카메라 위치·FrameLighting → [CubePS](../Shaders/Samples/Sandbox/CubePS.hlsl) 순서다. 조명·렌더 데이터는 창·GPU 자원을 소유하지 않는다.

- `direction`은 빛이 진행하는 월드 단위 방향이며 표면→광원 L은 반대다. Diffuse는 `max(N·L, 0)`, Specular는 `reflect(-L, N)`과 표면→카메라 방향의 내적을 광택 지수로 거듭제곱한다. N·L≤0이면 Specular도 0이며 카메라=표면인 0 길이 View 벡터를 안전하게 처리한다.
- 출력은 `BaseColor * (Ambient + 광색 * 강도 * Diffuse) + 광색 * 강도 * 반사 강도 * Specular`다. Ambient는 직접광 강도와 독립적이고 1 초과 출력은 UNORM RTV에서 잘린다. HDR·톤 매핑은 없다.
- Render는 유한한 조명 값, direction 길이 제곱 오차 ≤0.001, 강도 0~2, 광색·Ambient·반사 강도 0~1, 광택 지수 1~256을 요구한다. 위반은 API·HRESULT로 보고하고 중단한다.
- 키·초기값은 [조명 조작](../README.md#조명-조작)을 따른다. R은 다른 조명 키보다 우선하며 카메라와 같은 포커스·시간 초기화 경계를 사용한다.

## 구현 선택과 후속 조건

- `ImageLoadResult`·`ExecutableDirectoryResult`는 이름 있는 구조체로 데이터·오류를 구분한다. `ObjectTransformConstants`·`FrameLighting`은 공개 데이터, `SandboxLighting`은 각도·강도 조건을 보호하는 값 클래스이며 Rule of Zero를 따른다. `ComApartment`는 복사·이동을 금지해 COM 초기화 횟수를 중복 정리하지 않는다.
- 실제 소비자인 앱·테스트가 EXE 기준 경로 계산과 `BuildObjectTransformConstants`를 공유한다. 렌더러는 호출 중 입력을 복사하고 참조를 보관하지 않는다. GPU 실패 시 수명과 정리는 위 실행·렌더링 계약을 따른다.
- CPU 복사 크기·span 인덱스는 `std::size_t`, D3D12 크기·Fence·출력 인자는 API 타입을 유지한다. 크기 산술 범위를 검사하고 출력 포인터를 다른 정수 타입으로 캐스팅하지 않는다. 고정 배열의 API 전달·Win32/COM 변환은 실제 범위와 객체 계약에 맞춘다.
- 테스트 어댑터 RendererReadback의 Capture는 실패 후 픽셀 검증을 중단해야 하므로 `[[nodiscard]]`를 사용한다. GetWorld·GetView는 상태·자원을 변경하지 않는 행렬 계산 API로 `[[nodiscard]]`를 사용하지 않는다. [적용 범위와 검증](development.md#nodiscard-적용-범위-2026-10-20)을 따른다.
- WindowSmoke의 상태 참조·입력 복사본·optional 종료 코드와 LightingChecks의 ambient·픽셀 index는 타입을 명시한다. [테스트 타입 적용](development.md#테스트-상태와-픽셀-인덱스-타입-명시-2026-10-20)에서 범위·검증을 확인한다.
- Sandbox 전용 구현의 PCH 요구 시 자기 헤더보다 `pch.h`를 먼저 포함한다. Engine 구현은 PCH 없이 자기 헤더를 먼저 포함한다. 플랫폼 include의 Windows→DirectX 순서는 이유를 주석으로 남기며, 기존 긴 오류 문자열은 포맷 규칙의 문자열 예외를 유지한다.
- 후속 메시·재질·광원에 실제 공유 수요가 생기면 텍스처 업로드·렌더 데이터·재질 상수 분리를 검토한다. 범용 로더·광원 관리자·자체 자료형 별칭·변환 도우미·ECS는 선행 구현하지 않는다. 컨벤션의 확인 범위·적용 상태와 파일 재배치 조건은 [컨벤션 재검토](development.md#컨벤션-재검토와-후속-적용-2026-10-20)에서 관리한다.

## 현재 제약

- Normal/Roughness/AO/Height·DDS·압축·밉맵·캐시·텍스처 교체, 그림자·감쇠·다중 광원·PBR·HDR는 지원하지 않는다.
- 상단 큐브 일부 잘림·단일 밉 축소 떨림이 남으며 물체 잘림을 자동 보정하지 않는다.
- DPI·최소 창 크기는 미결정이다. 모달 리사이즈 중 렌더링이 멈출 수 있다. Raw Input·커서 고정·충돌은 없으며 VSync·Present 대기는 유지한다.
- 장치 손실 자동 복구는 없다. 오류 경로의 검증 범위는 [첫 마일스톤](milestones/v0.1.0-sandbox.md#당시-남은-검증과-제약)에 기록한다.

## API 참고

기존 구현 확인에 사용한 Microsoft 문서다.

- [Adapter 선택](https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_6/nf-dxgi1_6-idxgifactory6-enumadapterbygpupreference) · [Fence 완료 값](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12fence-getcompletedvalue) · [Allocator Reset](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12commandallocator-reset) · [Command List Reset](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-reset)
- [WIC 픽셀 형식](https://learn.microsoft.com/en-us/windows/win32/wic/-wic-codec-native-pixel-formats) · [텍스처 업로드](https://learn.microsoft.com/en-us/windows/win32/direct3d12/upload-and-readback-of-texture-data) · [GetCopyableFootprints](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-getcopyablefootprints) · [Readback](https://learn.microsoft.com/en-us/windows/win32/direct3d12/readback-data-using-heaps)
- [HLSL packing](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules) · [reflect](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-reflect) · [XMMatrixInverse](https://learn.microsoft.com/en-us/windows/win32/api/directxmath/nf-directxmath-xmmatrixinverse)
