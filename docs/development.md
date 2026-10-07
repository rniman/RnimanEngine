# 개발 설정과 검증

[프로젝트 소개와 실행](../README.md) · 문서 기준일: 2026-10-07

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
| 최근 확인 | 2026-10-07 커밋 전 점검에서 로컬 HEAD `56c63fe`와 미커밋 문서를 확인했으며 관련 C++ 채택 규칙은 이전 확인 이후 변경이 없었다. 이후 문서 정리에서 로컬 `docs/baseline.md`의 운영 기준을 다시 확인하고 위키 정비 담당과 프로젝트 담당의 책임을 이 문서·작업 지침에 명시했다. 공개 위키는 재조회하지 않았다. |

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
| 연결 | Sandbox·RendererSmoke는 `d3d12.lib`·`dxgi.lib`·`d3dcompiler.lib`; WindowSmoke는 D3D12 라이브러리를 연결하지 않음 |
| 헤더 | `WIN32_LEAN_AND_MEAN`, `NOMINMAX`; Sandbox는 PCH 사용, 두 Smoke는 PCH 없이 실제 구현 파일을 컴파일 |
| 파일 형식 | [.editorconfig](../.editorconfig)의 UTF-8 BOM 없음·LF·마지막 개행, C++ 탭 너비 4칸 |

Sandbox 구현 파일은 `pch.h`를 먼저 include하며 일반 헤더는 필요한 의존성을 직접 포함한다. PCH는 `$(IntDir)$(ProjectName).pch`에 생성한다. 소스 등록·필터 목록과 테스트의 실제 구현 직접 컴파일 관계는 [현재 프로젝트와 코드](project-structure.md#현재-프로젝트와-코드)를 따른다.

[CubeShaders.props](../Sandbox/CubeShaders.props)가 Sandbox·RendererSmoke에 `d3dcompiler.lib`와 FXC·SM5.0 빌드를 연결한다. CSO는 EXE 옆 `Shaders/`에 생성하며 EXE 위치를 기준으로 로딩한다. 실행 파일을 옮길 때 셰이더도 함께 복사한다. [.gitignore](../.gitignore)는 빌드 출력·개인 캐시를 제외하고 Unity `.meta`·모델 `.obj`·`.bin`·DDS·외부 라이브러리는 전역 제외하지 않는다. [.gitattributes](../.gitattributes)는 텍스트를 LF로 정규화하고 Windows 명령 스크립트는 CRLF로 체크아웃한다.

## D3D12 화면 출력과 검증

### 진입점과 수명

[Main.cpp](../Sandbox/Main.cpp)의 `wWinMain` → `SandboxApp::Run` → 창·렌더러 초기화 → 메시지 처리 → 리사이즈 → 입력·Transform 갱신 → 렌더링 → 종료 시 GPU 대기 → RAII 정리 순서다. 앱은 창 → 렌더러 순으로 멤버를 선언하여 역순 파괴에서 GPU 자원을 HWND보다 먼저 정리한다.

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

정점 8개·`R16_UINT` 인덱스 36개의 고정 Upload Heap 메시를 공유한다. CPU는 행 벡터의 `World * View * Projection`을 저장하고 [CubeVS.hlsl](../Shaders/Samples/Sandbox/CubeVS.hlsl)은 `row_major`·`mul(position, matrix)`로 전치 없이 읽는다. 왼손 View·Projection, 시계 방향 앞면과 전체 창 Viewport를 사용한다. D32 버퍼는 `DEPTH_WRITE`를 유지하며 매 프레임 1로 지우고 PSO는 LESS·깊이 쓰기·뒷면 컬링을 사용한다.

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
| [RendererSmoke](../Tests/RendererSmoke.cpp) | Transform·카메라 수학, 아래 픽셀 비교, 136프레임·8회 리사이즈 요청. 0·1·3·9개 오브젝트, 동일·0·1×1·확대·축소 크기, 전체 대기·재개·소멸자 정리 | 두 구성 모두 D3D12 Feature Level 11_0 하드웨어·Graphics Tools, EXE 옆 Cube CSO |
| [WindowSmoke](../Tests/WindowSmoke.cpp) | 숨김 창·합성 메시지로 키 유지/해제, 음수 마우스 좌표·누적량 소비, 캡처/포커스/모달 초기화, 최소화·복원·리사이즈 요청, 메시지 처리·대기, 종료 코드·반복 창 수명 | Windows 창·메시지 환경; D3D12·Graphics Tools·CSO 불필요 |
| Sandbox | 실제 화면·장치 입력·창 조작과 통합 실행 | 하드웨어·Cube CSO 필요. Graphics Tools는 Debug에 필수, Release는 디버그 레이어를 요구하지 않음. WARP 자동 대체 없음 |

공통 코드 변경 시 영향받는 유지 샘플과 대표 데모를 빌드·실행한다. 빌드·실행·시각 확인을 구분하고 성능은 측정 조건·값과 함께 보고한다. 문서만 변경하면 내용과 링크를 확인한다. 기존 `obj` 접근 제한은 승인된 샌드박스 밖 빌드로 검증했으며 과거 `Path`·`PATH` 중복도 실행 환경에서 정리했다.

### 겹치는 오브젝트의 픽셀 검증 (2026-10-05)

`CheckOcclusion`은 교차하지 않는 앞/뒤 큐브의 빈 화면·앞 단독·뒤 단독·정순·역순 이미지를 세 카메라 위치·해상도에서 비교한다. 앞 큐브 영역은 앞 단독 이미지와, 나머지는 뒤 단독 이미지와 같아야 한다. 겹침·앞뒤 색 차이·뒤쪽 가시 영역이 각각 100픽셀 이상이어야 하므로 빈 화면으로 통과하지 않는다.

[RendererReadback.h](../Tests/RendererReadback.h)의 테스트 전용 friend 어댑터는 동일 내부 draw를 Present 없이 실행하여 flip-discard 이후 내용에 의존하지 않는다. GPU 대기 → COPY_SOURCE·복사·PRESENT 복귀 → GPU 대기 → Map 순서로 RowPitch 패딩을 제외한 RGBA8을 비교한다. 앱 Render는 Present를 유지한다. 어댑터는 GPU 완료 후 렌더러보다 먼저 파괴하며 Capture 실패 시 테스트를 종료한다. 실패한 어댑터의 재사용은 지원하지 않는다.

## 최신 검증 결과

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

과거 위키 작업에서는 코드 적용 결과와 창·입력·앱 실행부의 책임·수명을 로컬 위키의 프로젝트 문서·변경 기록에 반영하고 strict 사이트 빌드를 통과했다. 이 기록은 당시 작업 이력이며 현재 위키의 반영 상태는 위키 쪽에서 관리한다.

### 삼각형 출력과 검증

2026-09-18 고정 삼각형·중앙 정사각형 Viewport에서 두 구성의 빌드·smoke·화면, EXE 기준 CSO 경로·누락 오류를 검증했다. 2026-09-24 Cube 셰이더로 교체했으므로 당시 결과를 최신 큐브의 오류 경로 검증으로 간주하지 않는다.

## API 참고

기존 구현 확인에 사용한 Microsoft 문서다.

- [Adapter 우선순위](https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_6/nf-dxgi1_6-idxgifactory6-enumadapterbygpupreference) · [Fence 완료 값](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12fence-getcompletedvalue)
- [Allocator Reset](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12commandallocator-reset) · [Command List Reset](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-reset)
- [Readback](https://learn.microsoft.com/en-us/windows/win32/direct3d12/readback-data-using-heaps) · [GetCopyableFootprints](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-getcopyablefootprints)
