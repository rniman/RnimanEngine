# 개발 설정과 검증

[프로젝트 소개와 실행](../README.md) · 문서 기준일: 2026-10-12

현재 설정, 구현 계약과 검증 범위를 관리한다. 기능 목표는 [로드맵](roadmap.md), 설계 방향은 [엔진 설계](design.md), 파일별 책임과 출력 배치는 [프로젝트 구조](project-structure.md), 작업 지침은 [AGENTS.md](../AGENTS.md)를 따른다.

## 컨벤션

공통 기준은 위키의 현재 [채택 규칙](https://rniman.github.io/convention/baseline/)과 [적용·근거 검토 절차](https://rniman.github.io/convention/baseline/#위키의-적용과-프로젝트-근거-검토)다. 확인 커밋은 상태 기록이며 고정 기준이 아니다. 필수·권장·선택 강도를 유지하고 초안·시험 적용을 공통 채택으로 취급하지 않는다.

프로젝트 담당은 작업 전에 관련 규칙·적용 범위를 확인하며 접근하지 못한 범위와 임시 참고 자료를 기록한다. 새 선택·예외·시험 적용·실제 적용 범위와 검증 결과는 프로젝트에 기록한다. 사용자 선택이나 채택 근거가 없는 결정은 시험 적용으로 남긴다. 확인한 위키 개정의 영향은 관련 코드·설정부터 맞추고, 프로젝트에 적용하지 못한 기준은 대상·이유·후속 적용 조건을 기록한다.

위키 정비 담당은 프로젝트의 코드·문서·적용 결과를 근거로 공통 규칙과 프로젝트 요약·전용 규칙·근거를 선별하고 위키를 갱신한다. 프로젝트에 남긴 공통화 후보·예외·후속 검토 사항은 그 근거로 사용한다. 프로젝트 작업에는 위키 문서 갱신이나 반영 완료 확인을 포함하지 않으며, 위키의 커밋·푸시·배포 상태도 위키 쪽에서 관리한다. 공통 규칙 전문과 정비 시 선별한 엔진 전용 기준은 위키의 주제별 문서·[RnimanEngine 문서](https://rniman.github.io/convention/projects/rniman-engine/)에서 관리한다.

### 확인·적용 상태

| 구분 | 현재 상태 |
|---|---|
| 확인 | 2026-10-06 공개 C++ 위키와 로컬 `D:/Docs/Convention` HEAD `33e96f6`의 채택 규칙·초안을 대조했다. 공개 조회는 웹 열기 도구의 접근 실패 후 승인된 PowerShell HTTP 조회로 확인했다. |
| 실제 적용 | Sandbox·RendererSmoke의 지역 상수 명명, 빈 함수 본문, 직접 include와 API 주석을 채택 규칙에 맞췄다. Win32Window·SandboxApp 추출에도 기존 명명·포맷·헤더·함수·소유권·초기화 규칙을 적용했다. |
| 프로젝트 한정 채택 | 창·렌더러 소유 분리, 고성능 하드웨어 우선·WARP 자동 대체 없음, 프레임 경계 리사이즈, 프레임 자원 재사용 전 Fence 대기. |
| 시험 적용 | ComPtr·Event 수명, bool/API/HRESULT 실패 보고, Debug 레이어 필수, VSync·두 백 버퍼, FXC·SM5.0, 행렬·D32·오브젝트별 상수, 카메라·Transform, 테스트 전용 readback과 아래 자료형 선택. |
| 최근 확인 | 2026-10-12 텍스처 변경 전체를 최종 점검했다. 공개 위키 조회 실패로 현재 로컬 원본을 임시 참고했으며 공개 배포본과의 일치 여부는 미확인이다. 직접 include·정렬·순서 예외 주석, 출력 인자와 업로드 수명 설명의 실제 수정 범위·검증 결과는 아래 최종 점검 기록을 따른다. |

2026-10-12 텍스처 작업에서는 공개 위키의 웹 조회가 실패하여 `D:/Docs/Convention/docs/`의 현재 로컬 원본을 임시 참고했다. 공개 배포본과 로컬 원본의 일치 여부·로컬 Git 메타데이터는 확인하지 못했다. baseline과 C++ 9개 채택 문서, HLSL 초안에서 작업 관련 범위를 확인했다. 2026-10-09 함수 규칙의 이름 있는 구조체 반환·의도적 결과 무시와 클래스 규칙의 특수 멤버 검토를 새 코드에 적용했다. `ImageLoadResult`·`AssetDirectoryResult`는 필드 이름으로 데이터·오류를 구분하고, `ComApartment`는 복사·이동을 제공하지 않아 COM 초기화 횟수를 중복 정리하지 않는다. 기존 `(void)WaitForGpu()` 종료 정리 경로는 확인만 했으며 변경하지 않았다. 나머지 신규 C++·HLSL에는 현재 프로젝트의 명명·포맷·헤더·API 주석·수명 기준을 적용했다. 독립 헤더·파일 형식·Debug/Release 검증 범위는 아래 결과에 기록한다. 위키 파일은 수정하지 않았다.

사용자가 정한 이번 범위는 BaseColor PNG 텍스처 큐브와 루트 `Assets/` 전체의 Git 제외다. 테스트 기준 이미지는 `Tests/Assets/`에서 추적한다. WIC 직접 사용·STA COM 수명·RGBA8 결과 반환·sRGB 출력·단일 밉·선형 Clamp·초기 업로드 전체 대기는 Sandbox 한정 시험 적용이다. WICTextureLoader12와 외부 헬퍼 없이 Windows SDK만으로 필요한 디코딩·복사를 구현하며, 렌더러는 CPU 이미지의 참조를 초기화 이후 저장하지 않는다. GPU 실패 보고·장치 손실 정리는 기존 시험 적용 계약을 유지한다. 범용 로더·비동기 업로드·디스크립터 할당기로 확대하지 않는다. 후속 메시 로더에서 같은 RGBA 업로드 소비자가 생기면 `CreateTexture` 분리를 검토하고, 두 실제 소비자가 생긴 EXE 기준 경로 계산은 `GetExecutableDirectory`로 공통화했다.

자료형·변환과 HLSL은 공통 초안이다. 자체 별칭·범용 변환 도우미는 미도입이며, 다른 실제 소비자가 생기면 Transform·렌더 데이터 수집·입력 초기화·버퍼 업로드의 공통화를 검토한다. 현재 Sandbox의 선택을 범용 엔진 정책으로 확대하지 않는다.

### 자료형·변환 초안의 적용 범위

2026-10-06 [자료형 선택과 별칭](https://rniman.github.io/convention/coding/cpp/type-selection/)·[타입 변환과 범위 검사](https://rniman.github.io/convention/coding/cpp/conversions/)를 확인하고 다음 범위를 시험 적용했다.

- `RenderInternal`의 상수 간격·CPU 복사 크기·span 인덱스는 `std::size_t`를 사용한다. 곱셈 전 `std::numeric_limits<std::size_t>` 범위 검사를 유지한다.
- 크기·인덱스·디스크립터 간격의 `UINT`, Fence·자원 크기의 `UINT64`, InfoQueue 출력의 `SIZE_T`와 디스크립터 주소 계산은 API 계약을 유지한다. 출력 포인터를 다른 정수 타입으로 캐스팅하지 않는다.
- 전체 범위가 보존되는 전달에는 캐스트를 추가하지 않는다. 고정 배열 길이·크기의 `UINT` 변환, Win32·COM 포인터 변환은 기존 범위·객체 계약을 따른다. readback 픽셀 출력은 실패 시에도 바뀔 수 있다.

## 프로젝트 설정

빌드·실행과 카메라 조작은 [README](../README.md)를 따른다. Visual Studio 프로젝트를 직접 관리하며 세 프로젝트 모두 x64 Debug·Release만 사용한다.

| 항목 | 설정 |
|---|---|
| 확인한 환경 | Visual Studio Community 2026 18.10.3, MSVC `v145` / `14.51.36231`, Windows SDK `10.0.26100.0` |
| C++ | C++20, Unicode, `/utf-8`, `/W4`, `/permissive-`, `/EHsc` |
| 경고·런타임 | Sandbox·RendererSmoke는 경고의 오류 처리 끔, WindowSmoke는 `/WX`; Debug `/MDd`, Release `/MD` |
| 출력 | `bin/<프로젝트>/<구성>/`, `obj/<프로젝트>/<구성>/`; Sandbox 디버거 작업 디렉터리 `$(OutDir)` |
| 연결 | Sandbox·RendererSmoke는 `d3d12.lib`·`dxgi.lib`·`d3dcompiler.lib`·`windowscodecs.lib`·`ole32.lib`; WindowSmoke는 D3D12 라이브러리를 연결하지 않음 |
| 헤더 | `WIN32_LEAN_AND_MEAN`, `NOMINMAX`; Sandbox는 PCH 사용, 두 Smoke는 PCH 없이 실제 구현 파일을 컴파일 |
| 파일 형식 | [.editorconfig](../.editorconfig)의 UTF-8 BOM 없음·LF·마지막 개행, C++ 탭 너비 4칸 |

Sandbox 구현 파일은 `pch.h`를 먼저 include하며 일반 헤더는 필요한 의존성을 직접 포함한다. PCH는 `$(IntDir)$(ProjectName).pch`에 생성한다. `AssetLoader.cpp`도 Sandbox·RendererSmoke에서 직접 컴파일한다. 소스 등록·필터 목록과 테스트의 실제 구현 직접 컴파일 관계는 [현재 프로젝트와 코드](project-structure.md#현재-프로젝트와-코드)를 따른다.

[CubeShaders.props](../Sandbox/CubeShaders.props)가 Sandbox·RendererSmoke에 `d3dcompiler.lib`와 FXC·SM5.0 빌드를 연결한다. CSO는 EXE 옆 `Shaders/`에 생성하며 EXE 위치를 기준으로 로딩한다. 실행 파일을 옮길 때 셰이더도 함께 복사한다. [.gitignore](../.gitignore)는 빌드 출력·개인 캐시·루트 `/Assets/`를 제외한다. Unity `.meta`·모델 `.obj`·`.bin`·DDS·외부 라이브러리는 확장자로 전역 제외하지 않으며, `Tests/Assets/`의 테스트 PNG는 Git에 포함한다. [.gitattributes](../.gitattributes)는 텍스트를 LF로 정규화하고 Windows 명령 스크립트는 CRLF로 체크아웃한다.

## D3D12 화면 출력과 검증

### 진입점과 수명

[Main.cpp](../Sandbox/Main.cpp)의 `wWinMain` → `SandboxApp::Run` → 창 생성·PNG 디코딩·렌더러 초기화와 업로드 → 메시지 처리 → 리사이즈 → 입력·Transform 갱신 → 렌더링 → 종료 시 GPU 대기 → RAII 정리 순서다. 앱은 창 → 렌더러 순으로 멤버를 선언하여 역순 파괴에서 GPU 자원을 HWND보다 먼저 정리한다.

Win32 실패는 API·오류 코드를, D3D12 실패는 API·HRESULT와 가능한 Device removed reason을 보고한다. 부분 자원은 RAII로 정리하며 장치 손실 자동 복구는 없다. 초기 클라이언트 크기는 1280×720이다.

### 창·입력 추출 (2026-10-06)

[Win32Window](../Sandbox/Win32Window.h)는 창 하나·UI 스레드의 창 수명, 메시지 처리·대기와 입력 수집을 담당한다. GPU·카메라를 참조하지 않는다.

- `State`는 크기·최소화·포커스·리사이즈 요청을 보유한다. 앱은 `GetState`로 읽고 GPU 리사이즈 성공 후에만 `ClearResizeRequest`를 호출한다. 최소화·0 크기에서는 입력을 소비하지 않고 메시지를 기다린다.
- `ConsumeInput`은 입력 복사본을 반환한다. 키 눌림은 유지하고 누적 마우스 이동량·시간 초기화 요청만 비운다. 포커스 해제·최소화·모달 조작은 입력을 초기화하며 캡처 상실 시 드래그 회전을 중단한다.
- `WM_NCCREATE`에서 `GWLP_USERDATA`에 비소유 객체 주소를 저장한다. 콜백 상태는 창이 소유하며 복사·이동 금지로 주소를 유지한다. `GetHandle`의 HWND는 비소유다.
- `PumpMessages`는 `WM_QUIT` 코드를 `std::optional<int>`로 반환하며 0도 종료 요청이다. `WM_CLOSE`는 종료만 요청한다. GPU 대기·렌더러 정리 후 창 소멸자가 HWND를 파괴하므로 그때까지 콜백 상태가 유효하다.

### 앱 실행부 추출 (2026-10-06)

[SandboxApp](../Sandbox/SandboxApp.h)은 창·렌더러·카메라·큐브 3개를 소유하며 복사·이동하지 않는다. Main은 앱 생성·`Run` 호출·반환 코드 전달을 담당한다.

- `Run`은 한 번만 호출한다. 메시지 처리 → 최소화 대기 → 프레임 경계 리사이즈 → 입력 소비·시간 계산 → `Update` → `Render`를 반복한다. 정상 종료에서는 GPU 완료 확인 후 WM_QUIT 코드를 반환하고 초기화·실행·GPU 대기 실패는 1을 반환한다. 반환 후 앱을 파괴한다.
- `Update`는 포커스가 있을 때 입력을 카메라에 반영한다. 입력 초기화 후 이동 시간은 0, 일반 프레임은 최대 0.1초다. 저프레임에서는 이동이 실제 시간보다 느릴 수 있다. 큐브 회전은 입력 중단과 무관한 절대 경과 시간을 사용한다.
- `Render`는 현재 카메라·창 종횡비·Transform으로 WVP 목록을 만든다. 시간·입력 해석·데모 갱신은 앱에, GPU 명령·동기화는 렌더러에 둔다. 범용 Application·Scene·Input·Engine 계층은 미도입이다.

### 프레임별 Fence 동기화

[D3D12Renderer](../Sandbox/D3D12Renderer.cpp)는 Device·Queue·Swap Chain·RTV/DSV·깊이 버퍼·파이프라인·메시와 프레임 자원을 소유한다. 두 `FrameResources`가 각각 Allocator·Constant Buffer·마지막 제출 Fence를 보유하고 단일 Command List·Direct Queue·Fence/Event를 공유한다.

프레임은 슬롯 Fence 대기 → 상수 복사 → Allocator/List Reset → 색상·깊이 clear → draw → PRESENT 복귀 → 제출·Present·Signal 순서다. Fence는 단조 증가하며 `UINT64_MAX` 완료 값은 장치 제거로 처리한다. `WaitForGpu`는 리사이즈·종료·미완료 작업이 있는 소멸자에서 전체 제출을 기다린다. Signal·Present 실패 후에도 정리를 시도한다. Resize는 동일 크기·0 크기를 무시하고 전체 GPU 대기 후 백 버퍼·깊이 버퍼를 재생성한다.

## 큐브 출력과 검증

### 메시와 행렬

면별 정점 24개·`R16_UINT` 인덱스 36개의 고정 Upload Heap 메시를 공유한다. CPU는 행 벡터의 `World * View * Projection`을 저장하고 [CubeVS.hlsl](../Shaders/Samples/Sandbox/CubeVS.hlsl)은 `row_major`·`mul(position, matrix)`로 전치 없이 읽는다. 왼손 View·Projection, 시계 방향 앞면과 전체 창 Viewport를 사용한다. D32 버퍼는 `DEPTH_WRITE`를 유지하며 매 프레임 1로 지우고 PSO는 LESS·깊이 쓰기·뒷면 컬링을 사용한다.

### 텍스처 큐브 (2026-10-12)

`SandboxApp::Initialize` → `GetExecutableDirectory` → `LoadPngImage` → `D3D12Renderer::Initialize(..., const ImageData&)` → `CreateTexture` → 매 프레임 SRV·Sampler 샘플링 순서다. [ImageData](../Sandbox/ImageData.h)는 CPU의 위→아래 RGBA8 행을 패딩 없이 보유한다. [AssetLoader](../Sandbox/AssetLoader.cpp)는 WIC로 PNG를 읽고 크기·UINT 곱셈 범위·단일 프레임을 검사한다. 오류는 빈 이미지·HRESULT·API 이름의 구조체로 반환하고 앱이 전체 경로와 함께 표시한다. WIC 변환은 채널 형식만 RGBA8로 맞추며 ICC 프로파일 변환·EXIF 회전은 적용하지 않는다. 입력 BaseColor는 sRGB로 간주한다.

- 로더의 로컬 `ComApartment`는 STA의 `CoInitializeEx`가 S_OK/S_FALSE인 횟수만 `CoUninitialize`한다. COM 객체를 먼저 해제한 뒤 아파트먼트를 정리한다. 기존 MTA 호출은 RPC_E_CHANGED_MODE로 실패하며 로딩 실패를 숨기지 않는다.
- 큐브는 각 면에 독립적인 네 정점·UV를 사용한다. 옆면의 위는 +Y, 윗면은 +Z, 아랫면은 −Z이며 외부에서 좌상 (0, 0)·우하 (1, 1)이다. 각 면에 이미지 전체를 한 번 적용하므로 비균일 Scale에서는 무늬도 늘어난다.
- 렌더러는 양수 크기·D3D12 최대 Texture2D 크기·정확한 RGBA8 바이트 수를 확인한다. Default Heap의 단일 밉 RGBA8 sRGB 텍스처를 COPY_DEST로 생성하고, `GetCopyableFootprints`의 RowPitch로 Upload Heap에 행별 복사한다. 명령 제출 후 PIXEL_SHADER_RESOURCE로 전환하며 첫 사용 전에 전체 GPU 완료를 기다린다. 완료 후 Upload Heap을 해제한다. 대기 실패 시에는 멤버에 유지해 소멸자의 완료 시도까지 수명을 보장한다. 실패 후 재사용은 지원하지 않는다.
- Shader-visible SRV 한 개와 정적 선형 Clamp Sampler를 렌더러가 소유한다. Root CBV b0는 VS, SRV t0·Sampler s0는 PS에 연결한다. 모든 큐브가 같은 텍스처를 공유하고 리사이즈에서도 텍스처·SRV는 유지한다.
- Swap Chain은 RGBA8 UNORM, RTV와 PSO 출력은 RGBA8 sRGB다. SRV에서 선형으로 읽고 RTV에서 sRGB로 기록한다. clearColor도 선형 값으로 해석하므로 기존 배경보다 밝게 보인다. 텍스처 alpha와 무관하게 PS가 alpha 1을 출력하며 깊이 쓰기·불투명 가림을 유지한다.

현재 조명·Normal/Roughness/AO/Height·밉맵 생성·DDS·압축·변환 캐시·실행 중 교체는 없다. 먼 거리의 축소 샘플링에서 무늬가 떨릴 수 있으며 밉맵은 후속 단계다. 에셋 배치·복사·Git 제외는 [README](../README.md#빌드와-실행)를 따른다.

### 카메라 이동과 입력 (2026-10-01)

[SandboxCamera](../Sandbox/SandboxCamera.h)의 초기 위치는 `(0, 2, -5)`, 시선은 원점이다. 전후는 시선 방향, 좌우는 수평, 상하는 월드 Y축으로 초당 3 단위 이동하며 대각선 속도를 정규화한다. 회전 감도는 픽셀당 0.003 rad, pitch 제한은 ±(π/2 − 0.01), yaw는 한 바퀴 범위다. 앱이 입력을 `Move`·`Rotate`에 반영하고 `GetView`를 WVP 계산에 사용한다.

### 여러 오브젝트의 Transform (2026-10-05)

[Transform](../Sandbox/Transform.h)은 위치·Euler 회전(radian)·크기를 보유하고 `Scale * Rotation * Translation`을 계산한다. 큐브 3개는 서로 다른 Transform과 같은 카메라를 사용한다. 부모 계층·ECS는 없다.

`Render(std::span<const XMFLOAT4X4>)`는 WVP를 호출 중 복사하고 목록을 보관하지 않는다. 슬롯 Fence 완료 후 오브젝트마다 256바이트 상수 영역을 확보하고 draw마다 root CBV를 바꾼다. 버퍼는 필요 시 확장하며 감소 시 축소하지 않는다. 빈 목록은 clear·Present만 수행하고 크기 오버플로·생성 실패는 보고한다.

## 검증 방법

### 자동 검증

솔루션 빌드는 테스트를 실행하지 않는다. 다음 명령은 저장소 루트의 Developer PowerShell에서 각각 빌드·실행하며 종료 코드와 출력을 확인한다.

```powershell
MSBuild.exe Tests/RendererSmoke.vcxproj -nologo -t:Build -p:Configuration=Debug -p:Platform=x64
./bin/RendererSmoke/Debug/RendererSmoke.exe
MSBuild.exe Tests/RendererSmoke.vcxproj -nologo -t:Build -p:Configuration=Release -p:Platform=x64
./bin/RendererSmoke/Release/RendererSmoke.exe
MSBuild.exe Tests/WindowSmoke.vcxproj -nologo -t:Build -p:Configuration=Debug -p:Platform=x64
./bin/WindowSmoke/Debug/WindowSmoke.exe
MSBuild.exe Tests/WindowSmoke.vcxproj -nologo -t:Build -p:Configuration=Release -p:Platform=x64
./bin/WindowSmoke/Release/WindowSmoke.exe
```

| 대상 | 검사 범위 | 실행 조건 |
|---|---|---|
| [RendererSmoke](../Tests/RendererSmoke.cpp) | PNG RGBA8·누락/손상, 여섯 면 UV·행 패딩·sRGB·불투명 alpha, Transform·카메라 수학, 아래 픽셀 비교, 136프레임·8회 리사이즈 요청. 0·1·3·9개 오브젝트, 동일·0·1×1·확대·축소 크기, 전체 대기·재개·소멸자 정리 | 두 구성 모두 D3D12 Feature Level 11_0 하드웨어·Graphics Tools, EXE 옆 Cube CSO·`Assets/TextureTest.png` (빌드 시 복사, 벽돌 원본 불필요) |
| [WindowSmoke](../Tests/WindowSmoke.cpp) | 숨김 창·합성 메시지로 키 유지/해제, 음수 마우스 좌표·누적량 소비, 캡처/포커스/모달 초기화, 최소화·복원·리사이즈 요청, 메시지 처리·대기, 종료 코드·반복 창 수명 | Windows 창·메시지 환경; D3D12·Graphics Tools·CSO 불필요 |
| Sandbox | 실제 화면·장치 입력·창 조작과 통합 실행 | 하드웨어·Cube CSO·BaseColor PNG 필요. Graphics Tools는 Debug에 필수, Release는 디버그 레이어를 요구하지 않음. WARP 자동 대체 없음 |

공통 코드 변경 시 영향받는 유지 샘플과 대표 데모를 빌드·실행한다. 빌드·실행·시각 확인을 구분하고 성능은 측정 조건·값과 함께 보고한다. 문서만 변경하면 내용과 링크를 확인한다. 기존 `obj` 접근 제한은 승인된 샌드박스 밖 빌드로 검증했으며 과거 `Path`·`PATH` 중복도 실행 환경에서 정리했다.

### 겹치는 오브젝트의 픽셀 검증 (2026-10-05)

`CheckOcclusion`은 교차하지 않는 앞/뒤 큐브의 빈 화면·앞 단독·뒤 단독·정순·역순 이미지를 세 카메라 위치·해상도에서 비교한다. 앞 큐브 영역은 앞 단독 이미지와, 나머지는 뒤 단독 이미지와 같아야 한다. 겹침·앞뒤 색 차이·뒤쪽 가시 영역이 각각 100픽셀 이상이어야 하므로 빈 화면으로 통과하지 않는다.

[RendererReadback.h](../Tests/RendererReadback.h)의 테스트 전용 friend 어댑터는 동일 내부 draw를 Present 없이 실행하여 flip-discard 이후 내용에 의존하지 않는다. GPU 대기 → COPY_SOURCE·복사·PRESENT 복귀 → GPU 대기 → Map 순서로 RowPitch 패딩을 제외한 RGBA8을 비교한다. 앱 Render는 Present를 유지한다. 어댑터는 GPU 완료 후 렌더러보다 먼저 파괴하며 Capture 실패 시 테스트를 종료한다. 실패한 어댑터의 재사용은 지원하지 않는다.

## 최신 검증 결과

### 텍스처 코드 컨벤션 최종 점검 (2026-10-12)

텍스처 변경 전체의 C++ 명명·포맷·직접 의존성·초기화·const·API 주석·반환 계약·특수 멤버·소유권을 확인했다. `D:/Docs/Convention/docs/`의 현재 채택 규칙을 임시 참고했으며 공개 위키 접근 실패와 배포본·로컬 Git 메타데이터 미확인 조건은 위 적용 상태와 같다. HLSL 명명·진입점·빌드 연결도 확인했으며 리소스 명명·레지스터·자료형 변환은 기존 Sandbox 시험 적용 범위다.

실제로 수정한 범위는 `AssetLoader.cpp`의 `<filesystem>`와 RendererSmoke의 `<ios>` 직접 include, 테스트 프로젝트 헤더 정렬, Windows→DirectX 순서와 예외 이유 주석이다. SandboxApp·RendererSmoke의 HRESULT 진단은 `%lX`에 맞는 `unsigned long`으로 명시적으로 전달한다. 테스트 이미지 출력 인자의 성공 시 교체·실패 시 유지와 업로드 메모리의 GPU 완료 확인 실패 시 수명도 주석에 맞췄다. 나머지 명명·RAII·복사/이동 제한·CPU 디코딩과 GPU 업로드 경계·반환값 검사는 확인만 했으며 변경하지 않았다. 새 공통 정책이나 범용 도우미를 도입하지 않았고 위키 문서는 수정하지 않았다.

| 구분 | 수정 후 확인한 결과 |
|---|---|
| 빌드·자동 검사 | x64 Debug·Release 세 프로젝트·셰이더 빌드, RendererSmoke·WindowSmoke 종료 코드 0. PNG·6개 면 UV·행 패딩·sRGB·alpha·세 해상도 가림 검사 통과, 잘못된 픽셀·D3D12 오류·경고 0 |
| 실제 앱 | 두 구성에서 실제 BaseColor를 사용한 숨김 창 실행·크기 변경 2회·WM_CLOSE 종료 코드 0·창 파괴 확인. 격리 Debug 복사본의 PNG 누락·손상은 전체 경로·API·HRESULT 표시 후 종료 코드 1 |
| 헤더·형식 | ImageData.h·AssetLoader.h·D3D12Renderer.h의 PCH 없는 `/W4 /WX` 독립 컴파일 성공. 수정 텍스트 형식·프로젝트 등록·필터·문서 로컬 링크·앵커·diff 검사 성공 |
| 실행·시각 확인 구분 | 사용자는 실행에 큰 문제가 없다고 보고했다. 이번 점검에서는 화면 시각 확인·실제 장치 입력·F5를 새로 수행하지 않았으며 GPU readback 시각 결과는 아래 텍스처 구현 검증 기록을 따른다. |

### 텍스처 큐브 검증 (2026-10-12)

| 구분 | 확인한 결과 |
|---|---|
| 빌드 | x64 Debug·Release Sandbox·RendererSmoke·WindowSmoke·셰이더 성공. 최초 샌드박스 빌드의 MSPDB140.DLL/PDB 오류 이후 승인된 샌드박스 밖 빌드로 확인 |
| CPU 이미지 | 직접 만든 3×3 RGBA8 기준 PNG의 전체 바이트·alpha, 누락·손상 파일의 실패와 빈 결과 확인. 벽돌 원본 없이 테스트 실행 가능 |
| GPU 텍스처 | 6개 면 × 9개 텍셀의 RGBA 픽셀 확인. RGB 오차 ≤1, alpha 255. UV 방향·채널 순서·12바이트 행→256바이트 RowPitch·sRGB 왕복·초기화 후 CPU 픽셀 해제 확인 |
| 가림·회귀 | 세 해상도 15프레임 가림 비교·정역순 일치, 잘못된 픽셀 0. 이후 136프레임·8회 리사이즈 요청·전체 대기·재개·소멸자 정리 성공. 두 구성 D3D12 오류·경고 0 |
| 창 | 두 구성 WindowSmoke 성공 |
| 실제 앱 | 실제 1024×1024 BaseColor로 Debug·Release 숨김 창 실행, 실제 클라이언트 784×561→1084×661 변경, WM_CLOSE 종료 코드 0·창 파괴 확인. 작업 디렉터리는 EXE 폴더와 다른 저장소 루트 |
| 오류 경로 | 원본 경로를 없는 임시 루트로 지정한 CopySandboxTexture 타깃이 안내와 함께 실패. 원본을 건드리지 않은 격리 실행 복사본에서 Debug의 PNG 누락·손상 각각 전체 경로·WIC API·HRESULT 표시, 오류 창 확인 후 종료 코드 1 |
| 시각 확인 | 임시 GPU readback 도구에 실제 BaseColor와 Sandbox와 같은 카메라·Transform·회전비를 전달해 1280×720 결과를 이미지로 확인. 세 오브젝트의 벽돌 무늬·면별 배치를 확인했으며 실제 창 캡처는 아님. 기존 배치의 위쪽 오브젝트 일부가 화면 위에서 잘리고 단일 밉 축소에 무늬 떨림이 남음 |
| 에셋 | BaseColor 원본 SHA-256 `EF794AE92568ABA7D77C8FE40F5D7830E0BFAF312CF692BB625ED8E696338EF1`. Sandbox 출력 복사본 일치, 해당 원본 폴더의 Git 제외 확인 |
| 형식·등록 | 수정 텍스트 17개의 UTF-8 BOM 없음·LF·마지막 개행·후행 공백, C++/HLSL 탭·추가 코드 120열, 프로젝트 소스 등록·Sandbox 필터 일치, 수정 문서의 로컬 링크·앵커 57개와 diff 검사 성공. 변경하지 않은 기존 오류 문자열 한 줄은 120열 예외 유지. 새 ImageData.h·AssetLoader.h 및 변경 D3D12Renderer.h의 PCH 없는 독립 /W4 /WX 컴파일 성공 |
| 한계 | 실제 창 화면·키보드/마우스·F5는 이번에 미검증. PNG 프로파일·애니메이션·광범위한 입력 형식, GPU 강제 실패·장치 손실·전체 live-object 검사는 미검증 |

기준 PNG는 [TextureTest.png](../Tests/Assets/TextureTest.png)이며 9개 색상과 중앙 alpha 128을 직접 작성했다. 여섯 방향의 정면 직교 투영으로 위/아래·좌/우·면별 배치와 RGB 중간값을 검사한다. 선형 필터의 텍셀 중심을 비교하고 GPU의 sRGB 변환 반올림을 위해 RGB 오차 1을 허용한다. 앱은 항상 불투명하게 출력한다.

두 구성의 가림 픽셀 결과:

| 크기 | 겹침 픽셀 | 앞뒤 색이 다른 겹침 | 뒤 큐브만 보임 | 잘못된 픽셀 | 정순/역순 |
|---|---:|---:|---:|---:|---|
| 321×241 | 3614 | 3570 | 2648 | 0 | 완전 일치 |
| 640×360 | 6405 | 6405 | 7838 | 0 | 완전 일치 |
| 257×193 | 1568 | 1567 | 2553 | 0 | 완전 일치 |


### 컨벤션 관리 책임 정리 (2026-10-07)

사용자 결정에 따라 AGENTS·README·개발 문서에 프로젝트의 컨벤션 확인·적용·기록과 위키 정비 담당의 문서 갱신 책임을 명시했다. 프로젝트에서 위키의 미커밋·푸시·배포 상태를 추적하던 항목을 제거했다. 문서 4개의 형식과 로컬 링크·앵커 51개, 로컬 위키 원본 기준 연결 5개·역방향 연결 7개 및 diff 검사를 통과했다. 문서만 수정하여 빌드·실행·시각 검증은 반복하지 않았다.

### 커밋 전 코드 점검 (2026-10-07)

창·입력·앱 실행부 분리의 변경 전체를 컨벤션·가독성·호출 계약·문서 일치·불필요한 블록 관점에서 점검했다. SandboxApp 초기화의 중복 반환 분기와 WindowSmoke 구성별 매크로 중복을 제거하고, 플랫폼 include 순서·초기 크기 대체 이유·Win32 초기화 실패 계약을 정리했다. 입력 소비와 GPU·HWND 수명 보장을 위한 코드 및 직접 include는 유지했다.

| 구분 | 확인한 결과 |
|---|---|
| 빌드·자동 검사 | x64 Debug·Release 세 프로젝트 빌드와 RendererSmoke·WindowSmoke 성공. GPU 오류·경고 0, 픽셀 결과는 아래 2026-10-06 표와 동일 |
| 앱 실행 | Debug·Release 숨김 창 시작·실제 크기 변경 2회·WM_CLOSE 종료 코드 0·창 파괴. Debug 별도 실행은 WM_QUIT 코드 7 전달·창 파괴 확인 |
| 형식·등록 | C++ 14개 파일의 UTF-8 BOM 없음·LF·마지막 개행·탭 들여쓰기·후행 공백 검사, 솔루션 GUID·파일 경로·중복 항목·Sandbox 필터 일치, 추적 문서의 로컬 링크·앵커 56개·위키 연결 4개와 diff 검사. WindowSmoke 구성별 공통·조건부 매크로는 각각 한 번씩 평가됨 |
| 확인 범위 | 관련 C++ 채택 규칙은 로컬 HEAD `56c63fe`에서 이전 확인 이후 변경 없음. 운영 절차 개정은 이 문서에 반영. 이번 점검에서 위키 수정·커밋·시각 확인·실제 장치 입력 검증은 수행하지 않음 |

### 프로젝트·문서 갱신 (2026-10-07)

x64 Debug·Release 솔루션 빌드 성공. 세 프로젝트의 GUID·등록 파일 경로·중복 항목, Sandbox 프로젝트·필터 일치, 문서 로컬 링크 50개·제목 앵커, UTF-8 BOM 없음·LF·마지막 개행과 `git diff --check`를 확인했다.

이후 개발 문서 정리에서는 저장소 로컬 링크·앵커 56개와 로컬 위키에서 들어오는 링크·앵커 4개, 파일 형식과 diff를 확인했다. 문서만 변경하여 빌드·실행·시각 검증을 반복하지 않았다.

### 앱 실행부 추출 검증 (2026-10-06)

아래 결과는 앱 실행부 추출 후 기록이다. 자동 검사와 Sandbox는 x64 Debug·Release를 사용했다.

| 구분 | 확인한 결과 |
|---|---|
| 빌드 | Sandbox·RendererSmoke·WindowSmoke·셰이더 빌드 성공 |
| CPU | Transform 기본값·SRT 순서, 카메라 초기 위치·대각선 속도·시간 분할·0초/무입력·큰 회전 입력의 유한 행렬 통과 |
| GPU | 픽셀 비교 15프레임·3회 리사이즈 + smoke 136프레임·8회 요청. 종료 코드 0, InfoQueue 오류·경고 0 |
| 창·입력 | WindowSmoke의 입력 소비·캡처/포커스/모달 초기화·리사이즈·메시지·종료·반복 창 수명 통과, 종료 코드 0 |
| 앱 실행 | 숨김 창 실제 크기 변경·합성 입력/최소화 메시지 처리·WM_CLOSE 종료, 코드 0. Debug 별도 실행은 WM_QUIT 코드 7 전달·창 파괴 확인 |
| 코드 형식·헤더 | 컨벤션 적용 때 C++ 파일 9개의 형식과 공개 헤더 4개의 PCH 없는 `/W4 /WX` 독립 컴파일을 확인했다. 앱 추출 후 추가 SandboxApp.h의 독립 컴파일도 성공 |
| 이전 시각 확인 | Debug의 배치·독립 회전·가림, Release의 현재 큐브 3개·최대화·최소화/복원·Alt+F4 종료 확인. UI 종료 코드는 미수집. 추출 후 화면·실제 입력은 미검증 |

두 구성의 픽셀 결과:

| 크기 | 겹침 픽셀 | 앞뒤 색이 다른 겹침 | 뒤 큐브만 보임 | 잘못된 픽셀 | 정순/역순 |
|---|---:|---:|---:|---:|---|
| 321×241 | 3614 | 3614 | 2648 | 0 | 완전 일치 |
| 640×360 | 6405 | 6405 | 7838 | 0 | 완전 일치 |
| 257×193 | 1568 | 1568 | 2553 | 0 | 완전 일치 |

### 남은 검증과 제약

- 실제 WASD·오른쪽 드래그 연속 이동, Alt+Tab 중 키 해제, 창 밖 버튼 해제, 모달 리사이즈 후 입력, 연속 드래그 리사이즈와 Visual Studio F5.
- 최신 Cube CSO 누락·손상, PSO·Signal·Present 강제 실패와 장치 손실, 전체 live-object 누수 검사, 성능 측정.
- 공면·교차 메시·투명도는 미검증이다. 테스트 장면은 현재 앱의 사용자 배치와 다르며 불투명 큐브의 세 조건만 검사한다. D3D12 오류·경고 0건도 자동 테스트 범위다. 실제 창의 진단 큐는 별도 수집하지 않았고 테스트가 보유한 InfoQueue Device 참조 때문에 전체 참조 해제 후 live-object 검사를 대신하지 않는다.
- DPI 정책·최소 창 크기는 미결정이다. 모달 리사이즈 동안 렌더링이 멈출 수 있고 좁은 창의 물체 잘림을 카메라 거리로 자동 보정하지 않는다. Raw Input·커서 고정·충돌은 없으며 VSync·Present 대기는 남아 있다. 성능 향상은 측정하지 않았다. 범용 메시 로딩 도입 시 Default Heap 업로드를 검토한다.

## 주요 이력

| 시점 | 변경 |
|---|---|
| 2026-09-15 | 창·D3D12 초기화, 저장소·출력 구조 |
| 2026-09-17 | 매 프레임 전체 대기를 프레임별 Allocator·Fence 재사용 대기로 교체 |
| 2026-09-24 | 삼각형을 인덱스 큐브·WVP·깊이 버퍼로 교체 |
| 2026-09-30 | 큐브 코드 정리, 사용자 화면 확인·두 구성 자동 검증 |
| 2026-10-05 | 카메라 입력·여러 Transform·겹침 픽셀 비교 연결 |
| 2026-10-06 | 채택 컨벤션·자료형 초안 적용, Win32Window·WindowSmoke 추가, SandboxApp으로 앱 실행부 분리 |
| 2026-10-07 | 프로젝트 등록·필터·문서 정리, 커밋 전 코드·설정 점검과 회귀 검증, 컨벤션 적용·기록과 위키 정비의 관리 책임 구분 |
| 2026-10-12 | PNG CPU 로딩·면별 UV·Default Heap 업로드·sRGB 텍스처 큐브, 기준 PNG 회귀 검사, 루트 Assets 전체 Git 제외·EXE 기준 에셋 복사 |

과거 위키 작업에서는 코드 적용 결과와 창·입력·앱 실행부의 책임·수명을 로컬 위키의 프로젝트 문서·변경 기록에 반영하고 strict 사이트 빌드를 통과했다. 이 기록은 당시 작업 이력이며 현재 위키의 반영 상태는 위키 쪽에서 관리한다.

### 삼각형 출력과 검증

2026-09-18 고정 삼각형·중앙 정사각형 Viewport에서 두 구성의 빌드·smoke·화면, EXE 기준 CSO 경로·누락 오류를 검증했다. 2026-09-24 Cube 셰이더로 교체했으므로 당시 결과를 최신 큐브의 오류 경로 검증으로 간주하지 않는다.

## API 참고

기존 구현 확인에 사용한 Microsoft 문서다.

- [Adapter 우선순위](https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_6/nf-dxgi1_6-idxgifactory6-enumadapterbygpupreference) · [Fence 완료 값](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12fence-getcompletedvalue)
- [Allocator Reset](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12commandallocator-reset) · [Command List Reset](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-reset)
- [WIC 픽셀 형식](https://learn.microsoft.com/en-us/windows/win32/wic/-wic-codec-native-pixel-formats) · [텍스처 업로드](https://learn.microsoft.com/en-us/windows/win32/direct3d12/upload-and-readback-of-texture-data)
- [Readback](https://learn.microsoft.com/en-us/windows/win32/direct3d12/readback-data-using-heaps) · [GetCopyableFootprints](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-getcopyablefootprints)
