# 개발 설정과 검증

[프로젝트 소개와 실행](../README.md) · 기준일: 2026-10-12

현재 설정·구현 계약·최신 검증 결과를 관리한다. 기능 목표는 [로드맵](roadmap.md), 설계 방향은 [엔진 설계](design.md), 코드·출력 배치는 [프로젝트 구조](project-structure.md), 작업 지침은 [AGENTS.md](../AGENTS.md)를 따른다.

## 컨벤션

공통 기준은 위키의 현재 [채택 규칙](https://rniman.github.io/convention/baseline/)과 [적용·근거 검토 절차](https://rniman.github.io/convention/baseline/#위키의-적용과-프로젝트-근거-검토)다. 작업 전에 관련 규칙을 확인하고 필수·권장·선택 및 채택·초안·시험 적용을 구분한다. 확인 커밋은 과거 적용 상태이며 고정 기준이 아니다.

프로젝트는 실제 적용 범위·선택 이유·예외·검증·후속 적용 조건을 기록한다. 접근하지 못한 기준과 임시 참고 자료도 남긴다. 위키 문서의 선별·갱신과 반영 상태 관리는 위키 정비 담당의 책임이다. 규칙 전문은 위키에 두며 프로젝트 작업에 위키 수정·배포 확인을 포함하지 않는다.

### 확인·적용 상태

| 구분 | 현재 상태 |
|---|---|
| 최근 확인 | 2026-10-12 기본 조명 컨벤션 검토. 공개 위키 조회 실패로 `D:/Docs/Convention/docs/`의 현재 baseline·C++ 9개 채택 문서를 임시 참고했다. 로컬 Git 조회도 접근 거부되어 메타데이터·공개 배포본 일치는 미확인 |
| 실제 적용 | 명명·포맷·초기화·const·직접 include·한국어 API 주석·반환 계약·소유권·특수 멤버 기준. 최신 검토에서는 GPU 캡처의 호출/실패 검사 분리, 여러 줄 인자 배치, 불필요 include 제거를 실제 수정했다. 나머지는 확인만 했다 |
| 프로젝트 한정 채택 | 창·렌더러 소유 분리, 고성능 하드웨어 우선·WARP 자동 대체 없음, 프레임 경계 리사이즈, 슬롯 재사용 전 Fence 대기 |
| 시험 적용 | WIC·STA·RGBA8 반환, sRGB·단일 밉·선형 Clamp, ComPtr·Event 수명과 bool/HRESULT 진단, Debug 레이어·두 백 버퍼·VSync, FXC·SM5.0, 행렬·D32·카메라·Transform·Phong·테스트 readback |
| 초안의 적용 범위 | C++ 자료형·변환·attribute와 HLSL·검증 문서는 공통 초안이다. 현재 렌더 데이터·레지스터·포맷·조명 범위와 진단 설정을 공통 채택으로 확대하지 않는다 |

현재의 구체적인 선택과 적용 이유:

- `ImageLoadResult`·`AssetDirectoryResult`는 이름 있는 구조체로 데이터·오류를 구분한다. `RenderObject`·`FrameLighting`은 공개 데이터, `SandboxLighting`은 각도·강도 조건을 보호하는 값 클래스이며 Rule of Zero를 따른다. `ComApartment`는 복사·이동을 금지해 COM 초기화 횟수를 중복 정리하지 않는다.
- 실제 소비자인 앱·테스트가 EXE 기준 경로 계산과 `CreateRenderObject`를 공유한다. 렌더러는 호출 중 입력을 복사하고 참조를 보관하지 않는다. GPU 실패 시 수명과 정리는 아래 계약을 따른다.
- CPU 복사 크기·span 인덱스는 `std::size_t`, D3D12 크기·Fence·출력 인자는 API 타입을 유지한다. 크기 산술 범위를 검사하고 출력 포인터를 다른 정수 타입으로 캐스팅하지 않는다. 고정 배열의 API 전달·Win32/COM 변환은 실제 범위와 객체 계약에 맞춘다.
- PCH 요구 시 자기 헤더보다 `pch.h`를 먼저 포함한다. 플랫폼 include의 Windows→DirectX 순서는 이유를 주석으로 남기며, 기존 긴 오류 문자열은 포맷 규칙의 문자열 예외를 유지한다.
- 후속 메시·재질·광원에 실제 공유 수요가 생기면 텍스처 업로드·렌더 데이터·재질 상수 분리를 검토한다. 범용 로더·광원 관리자·자체 자료형 별칭·변환 도우미·ECS는 선행 구현하지 않는다. 채택 기준의 미적용 항목은 확인 범위에서 없다.

## 프로젝트 설정

빌드·실행·에셋 준비와 조작은 [README](../README.md#빌드와-실행)를 따른다. 세 프로젝트는 x64 Debug·Release를 지원한다.

| 항목 | 설정 |
|---|---|
| 확인한 환경 | Visual Studio Community 2026 18.10.3, MSVC `v145` / `14.51.36231`, Windows SDK `10.0.26100.0` |
| C++ | C++20, Unicode, `/utf-8`, `/W4`, `/permissive-`, `/EHsc`; Debug `/MDd`, Release `/MD` |
| 경고 | Sandbox·RendererSmoke는 `/WX` 끔, WindowSmoke는 `/WX` |
| 출력 | `bin/<프로젝트>/<구성>/`, `obj/<프로젝트>/<구성>/`; Sandbox 디버거 작업 디렉터리 `$(OutDir)` |
| 연결 | Sandbox·RendererSmoke: `d3d12.lib`·`dxgi.lib`·`d3dcompiler.lib`·`windowscodecs.lib`·`ole32.lib`. WindowSmoke는 D3D12를 연결하지 않음 |
| 헤더 | `WIN32_LEAN_AND_MEAN`, `NOMINMAX`; Sandbox는 PCH, Smoke는 PCH 없이 실제 구현을 컴파일 |
| 셰이더 | [CubeShaders.props](../Sandbox/CubeShaders.props)의 FXC·SM5.0·진입점 Main. 구성별 EXE 옆 `Shaders/`에 CSO 생성·EXE 기준 로딩 |
| 파일·Git | [.editorconfig](../.editorconfig): UTF-8 BOM 없음·LF·끝 개행·C++ 탭 너비 4. [.gitattributes](../.gitattributes): 텍스트 LF, Windows 명령 스크립트 CRLF. [.gitignore](../.gitignore): 빌드 출력·개인 캐시·루트 `/Assets/` 제외, `Tests/Assets/` 추적 |

일반 헤더는 PCH 없이 필요한 의존성을 직접 포함한다. Sandbox PCH는 `$(IntDir)$(ProjectName).pch`에 생성한다. 자원은 EXE 위치로 찾으며 실행 파일 이동 시 `Shaders`·`Assets`도 함께 복사한다. 상세 소스 등록·필터·복사 경로는 [프로젝트 구조](project-structure.md#현재-프로젝트와-코드)와 README에서 관리한다.

## D3D12 화면 출력과 검증

### 진입점과 수명

[Main.cpp](../Sandbox/Main.cpp)의 `wWinMain` → [SandboxApp](../Sandbox/SandboxApp.cpp)의 `Run` → 창 생성·PNG 디코딩·GPU 초기화/업로드 → 메시지·리사이즈·입력/Transform 갱신 → 렌더링 → GPU 대기 → RAII 정리 순서다. 앱은 창을 렌더러보다 먼저 선언하여 역순 파괴에서 GPU 자원을 HWND보다 먼저 정리한다.

초기 클라이언트 크기는 1280×720이다. Win32 실패는 API·오류 코드, D3D12 실패는 API·HRESULT·가능한 Device removed reason을 보고한다. 부분 자원은 RAII로 정리하며 실패 후 재사용·장치 손실 자동 복구는 지원하지 않는다.

<span id="창입력-추출-2026-10-06"></span>

### 창·입력

[Win32Window](../Sandbox/Win32Window.h)는 창 하나·UI 스레드에서 창 수명·메시지·입력을 담당하며 GPU·카메라를 참조하지 않는다.

- `GetState`로 크기·최소화·포커스·리사이즈 요청을 읽는다. 앱은 GPU 리사이즈 성공 후 `ClearResizeRequest`를 호출하며 최소화·0 크기에서는 메시지를 기다린다.
- `ConsumeInput`은 복사본을 반환한다. 키 눌림은 유지하고 누적 마우스 이동·시간 초기화 요청을 비운다. 포커스 해제·최소화·모달 조작은 입력을 초기화하고 캡처 상실은 드래그를 중단한다.
- `GWLP_USERDATA`·HWND는 비소유다. 창 객체는 복사·이동 금지로 콜백 주소를 유지한다. `PumpMessages`는 `WM_QUIT` 코드를 optional로 반환하므로 0도 종료 요청이다. `WM_CLOSE` 후 GPU 정리를 마칠 때까지 HWND·콜백 상태를 유지한다.

<span id="앱-실행부-추출-2026-10-06"></span>

### 앱 실행

SandboxApp은 창·렌더러·카메라·조명·큐브 3개를 소유하며 복사·이동하지 않는다. `Run`은 한 번만 호출하며 메시지 → 최소화 대기 → 프레임 경계 리사이즈 → 입력 소비 → `Update` → `Render`를 반복한다. 정상 종료는 GPU 완료 후 WM_QUIT 코드, 초기화·실행·GPU 대기 실패는 1을 반환한다.

입력 해석은 앱, GPU 명령·동기화는 렌더러의 책임이다. 포커스가 있을 때 카메라·조명을 조절하며 입력 초기화 직후 이동 시간은 0, 일반 프레임은 최대 0.1초다. 큐브 회전은 입력 중단과 무관한 절대 경과 시간을 사용한다. [Transform](../Sandbox/Transform.h)은 `Scale * Rotation * Translation`, [SandboxCamera](../Sandbox/SandboxCamera.h)는 초기 위치 `(0, 2, -5)`·원점 시선을 사용한다. 조작·속도는 README를 따른다.

### 프레임별 Fence 동기화

[D3D12Renderer](../Sandbox/D3D12Renderer.cpp)는 GPU 자원과 두 프레임 슬롯의 Allocator·Constant Buffer·마지막 제출 Fence를 소유한다. 단일 Command List·Direct Queue·Fence/Event를 공유한다.

슬롯 Fence 대기 → 상수 복사 → Allocator/List Reset → 색상·깊이 clear/draw → PRESENT 복귀 → 제출·Present·Signal 순서다. Fence는 단조 증가하고 완료 값 `UINT64_MAX`는 장치 제거로 처리한다. 리사이즈·종료·미완료 제출이 있는 소멸자는 전체 GPU 완료를 시도하며 Signal·Present 실패도 정리 대상으로 남긴다. 소멸자의 `(void)WaitForGpu()`는 내부 진단 후 정리를 계속하는 예외이며 완료 보장을 뜻하지 않는다.

Resize는 동일 크기·0 크기를 무시하고 전체 GPU 대기 후 백 버퍼·깊이 버퍼를 재생성한다. 프레임 상수는 슬롯 완료 후에만 확장·복사하고 오브젝트 감소 시 축소하지 않는다.

## 큐브 렌더링 계약

### 좌표축과 행렬

| 항목 | 현재 Sandbox 계약 |
|---|---|
| 월드·로컬 | 월드 위쪽 `+Y`. 무회전 기준 오른쪽 `+X`·전방 `+Z`. 큐브의 의미상 로컬 정면 축과 물리적 거리 단위는 미정이며 외부 에셋 도입 시 결정 |
| 뷰 | `XMMatrixLookToLH`, 뷰 앞쪽 `+Z`. 카메라 전방은 yaw·pitch로 계산하며 월드 축은 회전하지 않음 |
| 투영·화면 | `XMMatrixPerspectiveFovLH`, near 0.1·far 100. 원근 나눗셈 후 NDC의 X/Y는 −1~1·Z는 0~1. 화면은 왼쪽 위 원점·오른쪽 +X·아래쪽 +Y |
| 벡터·합성 | 행 벡터 `p * M`. `World = S * R * T`, `WVP = World * View * Projection` |
| 저장·전달 | `XMStoreFloat4x4`의 행 우선 저장 → HLSL `row_major float4x4` → `mul(position, matrix)`. 패킹용 전치 없음 |
| 법선 | World 역전치의 3×3 부분으로 변환. 이 전치는 법선 변환에 필요하며 전달용 전치와 구분 |
| 회전 | `rotation.x/y/z`는 pitch(X)/yaw(Y)/roll(Z), 단위 radian. `XMMatrixRotationRollPitchYaw` 사용 |

행 우선은 **저장 순서**, 행 벡터는 **곱셈 방향**, 왼손은 **축의 방향 관계**다. 하나로 나머지를 추정하지 않는다. Direct3D 자체가 월드 축이나 왼손 좌표계를 강제하는 것은 아니며, 현재 카메라·투영 구현에 맞춘 프로젝트 선택이다. 앞면·깊이·GPU 레이아웃은 아래 계약을 따른다.

2026-10-13 [위키 채택 규칙](https://rniman.github.io/convention/coding/coordinates-and-matrices/) 확인을 시도했으나 공개 baseline 조회가 실패하여 `D:/Docs/Convention/docs/baseline.md`와 `coding/coordinates-and-matrices.md`를 임시 참고했다. 공개 배포본과의 일치는 미확인이다. 기존 Transform·카메라·RenderData·CubeVS의 구현을 확인해 위 요약과 README에 반영했으며 코드 변경·새로운 시험 적용·성능 예외는 없다. 로컬 정면·거리 단위·외부 에셋 변환은 실제 에셋 사용 흐름이 생길 때 계약을 정한다. 공통화할 새 코드 후보는 없다.

이번에는 문서·소스 대응과 링크·diff만 검사했다. 기존 CPU/GPU 검증 결과는 아래 기록을 유지하며 빌드·실행·시각 검증을 반복하지 않았다. 채택 규칙이 요구하는 실제 CubeVS의 reflection/어셈블리 패킹 확인은 이번에 수행하지 않았으므로 적용 완료로 주장하지 않으며, 다음 셰이더·렌더 데이터 변경 시 크기·offset·행렬 stride를 CPU 선언과 대조한다.

### 메시·행렬·렌더 데이터

면별 위치·UV·법선 정점 24개와 `R16_UINT` 인덱스 36개의 고정 Upload Heap 메시를 공유한다. 왼손 View·Projection, 시계 방향 앞면·뒷면 컬링·전체 창 Viewport를 사용한다. D32 깊이는 매 프레임 1로 지우며 LESS·깊이 쓰기를 사용한다.

앱이 [RenderData](../Sandbox/RenderData.h)의 `CreateRenderObject`로 WVP·World·World 역전치를 만든다. 유한한 아핀 World·양수 행렬식·유한한 역행렬/WVP를 요구하며 위반은 nullopt다. 반전 Transform의 컬링 보정은 없다. CPU는 행 벡터 행렬을 저장하고 [CubeVS](../Shaders/Samples/Sandbox/CubeVS.hlsl)는 `row_major`·`mul(position, matrix)`로 전치 없이 읽는다. 법선은 역전치의 3×3 부분으로 변환하고 PS에서 정규화한다.

`Render(std::span<const RenderObject>, const FrameLighting&)`는 입력을 호출 중 복사하고 참조를 보관하지 않는다. RenderObject 192바이트·FrameLighting 64바이트와 필드 offset을 static_assert로 확인한다. CB의 첫 256바이트는 PS b1, 이후 오브젝트마다 256바이트는 VS b0다. 빈 목록도 프레임 값을 기록하고 clear·Present하며 버퍼 크기 오버플로·생성 실패는 보고한다.

### 텍스처·sRGB

`SandboxApp::Initialize` → EXE 기준 경로 → [AssetLoader](../Sandbox/AssetLoader.cpp)의 `LoadPngImage` → CPU [ImageData](../Sandbox/ImageData.h) → `D3D12Renderer::CreateTexture` 순서다.

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

## 검증 방법

### 자동 검증

솔루션 빌드는 테스트를 실행하지 않는다. 저장소 루트의 Developer PowerShell에서 각각 빌드·실행하고 종료 코드·출력을 확인한다.

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
| [RendererSmoke](../Tests/RendererSmoke.cpp) | PNG·면 UV/sRGB/alpha·Phong·역전치 법선·Transform/카메라 수학、가림·리사이즈·프레임 재사용·정리 | 두 구성 모두 D3D12 FL11_0 하드웨어·Graphics Tools·Cube CSO·테스트 PNG 필요. 빌드가 EXE 옆에 복사하며 벽돌 원본은 불필요 |
| [WindowSmoke](../Tests/WindowSmoke.cpp) | 키·마우스 소비, 캡처/포커스/모달 초기화, 최소화·리사이즈·메시지·종료·창 수명 | Win32 환경. D3D12·Graphics Tools·CSO 불필요 |
| Sandbox | 실제 화면·장치 입력·창 조작의 통합 흐름 | BaseColor·CSO·하드웨어 필요. Graphics Tools는 Debug에 필수, Release는 요구하지 않음. WARP 대체 없음 |

공통 코드 변경 시 영향받는 테스트와 대표 앱을 빌드·실행한다. 빌드·실행·시각 확인을 구분하고, 미수행 항목·이유와 성능 측정 조건을 기록한다. 문서만 수정하면 내용·링크를 확인한다.

<span id="겹치는-오브젝트의-픽셀-검증-2026-10-05"></span>

### GPU 픽셀 검증

[RendererReadback](../Tests/RendererReadback.h)은 같은 draw를 Present 없이 실행해 flip-discard 이후 내용에 의존하지 않는다. GPU 대기 → COPY_SOURCE 복사·PRESENT 복귀 → GPU 대기 → Map 후 RowPitch를 제외한 RGBA8을 읽는다. 앱은 Present를 유지하며 어댑터는 GPU 완료 후 렌더러보다 먼저 파괴한다. Capture 실패 시 검증을 종료한다.

- `CheckOcclusion`: 빈 화면·앞 단독·뒤 단독·정순·역순을 세 카메라/해상도에서 비교한다. 겹침·색 차이·뒤 가시 영역이 각각 100픽셀 이상이어야 하며 앞 영역은 앞 단독, 나머지는 뒤 단독과 같아야 한다.
- [LightingChecks](../Tests/LightingChecks.cpp)의 `CheckPhongLighting`: Ambient 기준 픽셀에 독립적인 sRGB 기대값을 적용한다. 잘못된 World 법선 변환을 구별하려고 비균일 Scale·회전에 전단을 더한다. 기존 텍스처·가림은 Ambient=1·직접광=0인 동일 파이프라인에서 검사한다.
- [3×3 기준 PNG](../Tests/Assets/TextureTest.png)는 9색·중앙 alpha 128을 직접 작성했다. 여섯 면의 텍셀 중심에서 UV·채널·업로드 행 패딩·sRGB·alpha 255를 비교한다. RGB 허용 오차는 텍스처 1, 조명 2다.

## 최신 검증 결과

**2026-10-12 기본 조명 컨벤션 정리 후 재검증** 결과다. 별도 표기한 시각·오류 경로는 같은 날 앞선 텍스처·조명 구현에서 확인했으며 최신 재실행과 구분한다.

| 구분 | 확인한 결과 |
|---|---|
| 빌드 | x64 Debug·Release Sandbox·RendererSmoke·WindowSmoke·셰이더 성공. 승인된 샌드박스 밖 MSBuild 사용 |
| CPU | PNG 바이트·누락/손상, Transform·카메라 수학, 조명 시간 분할·0초·yaw/pitch·단위 방향·강도 범위·초기화, 역전치 직교와 무효 행렬 거부 통과 |
| GPU | 여섯 면 UV·sRGB·alpha, 월드 위치·카메라 반사광·광택 지수 4/64·뒷면 Specular 억제·카메라=표면·강도 0/Ambient·전단 법선 통과 |
| 가림·프레임 | 321×241·640×360·257×193에서 잘못된 픽셀 0·정순/역순 일치. 136프레임·8회 리사이즈·0/1/3/9개 오브젝트·광원 변화·대기/재개/종료 성공. 두 구성 D3D12 오류·경고 0 |
| 창·앱 | 두 구성 WindowSmoke 성공. 실제 BaseColor의 Sandbox 숨김 창에서 합성 조명 키 7개·포커스 해제·크기 변경 2회·WM_CLOSE 종료 코드 0·창 파괴 확인 |
| 헤더·형식 | 7개 관련 헤더의 PCH 없는 `/W4 /WX` 독립 컴파일 성공. 변경 20개 파일 형식·소스 등록·필터·로컬 링크/앵커 61개·diff 검사 통과 |
| 앞선 시각 확인 | 실제 에셋과 앱의 카메라·Transform·조명 초기값으로 만든 1280×720 GPU readback 이미지에서 세 큐브·방향/강도 변화·강도 0의 Ambient를 확인. 실제 창 캡처는 아님 |
| 앞선 오류 경로 | 텍스처 빌드 복사 타깃의 원본 누락 실패, 격리 Debug 실행의 PNG 누락·손상 진단과 종료 코드 1 확인. 조명 정리 후에는 다시 강제하지 않음 |

초기 조명 구현 중 Debug 증분 링크의 LightingChecks.obj에서 LNK1163이 발생했으나 Rebuild 후 통과했고 이후 컨벤션 정리의 빌드·실행도 성공했다. 잘못된 오브젝트 생성 원인은 미확정이다.

### 남은 검증과 제약

- 실제 키보드/마우스·Alt+Tab·창 밖 버튼 해제·모달/연속 리사이즈·Visual Studio F5·실제 창 화면은 최신 변경 기준으로 미검증이다. 합성 입력의 앱 실행과 GPU 픽셀 검사는 별도 검사이며 키 조작→실제 창 밝기 변화까지 확인한 것은 아니다.
- 최신 CSO 누락/손상, 조명 입력 거부의 모달 진단, PSO·Signal·Present 강제 실패·장치 손실·전체 live-object·극단적 수치 정밀도·성능은 미검증이다. 오류·경고 0은 자동 테스트 범위이며 전체 누수·성능 보장이 아니다.
- Normal/Roughness/AO/Height·DDS·압축·밉맵·캐시·텍스처 교체, 그림자·감쇠·다중 광원·PBR·HDR는 없다. 상단 큐브 일부 잘림·단일 밉 축소 떨림이 남으며 공면·교차 메시·투명도는 검증하지 않았다.
- DPI·최소 창 크기는 미결정이다. 모달 리사이즈 중 렌더링이 멈출 수 있고 물체 잘림을 자동 보정하지 않는다. Raw Input·커서 고정·충돌은 없으며 VSync·Present 대기는 유지한다.

이번 문서 정리에서는 파일 형식·저장소 로컬 링크/앵커 68개·로컬 위키에서 연결한 앵커 6개와 diff 검사를 통과했다. 빌드·실행·시각 검증은 반복하지 않았다.

## API 참고

기존 구현 확인에 사용한 Microsoft 문서다.

- [Adapter 선택](https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_6/nf-dxgi1_6-idxgifactory6-enumadapterbygpupreference) · [Fence 완료 값](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12fence-getcompletedvalue) · [Allocator Reset](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12commandallocator-reset) · [Command List Reset](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-reset)
- [WIC 픽셀 형식](https://learn.microsoft.com/en-us/windows/win32/wic/-wic-codec-native-pixel-formats) · [텍스처 업로드](https://learn.microsoft.com/en-us/windows/win32/direct3d12/upload-and-readback-of-texture-data) · [GetCopyableFootprints](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-getcopyablefootprints) · [Readback](https://learn.microsoft.com/en-us/windows/win32/direct3d12/readback-data-using-heaps)
- [HLSL packing](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules) · [reflect](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-reflect) · [XMMatrixInverse](https://learn.microsoft.com/en-us/windows/win32/api/directxmath/nf-directxmath-xmmatrixinverse)
