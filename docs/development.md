# 개발 설정과 검증

[프로젝트 소개와 실행](../README.md)

## 컨벤션

[컨벤션 사이트](https://rniman.github.io/convention/) · [GitHub 저장소](https://github.com/rniman/convention)

**적용 기준:** 위키의 현재 [채택 규칙](https://rniman.github.io/convention/baseline/)과 [위키와 프로젝트의 상호 반영](https://rniman.github.io/convention/baseline/#위키와-프로젝트의-상호-반영)을 따른다. 필수·권장·선택 강도를 유지하며 초안·보류·폐기·미결정 내용은 채택 규칙처럼 적용하지 않는다. 확인 커밋은 과거 규칙에 고정하기 위한 기준이 아니라 확인·반영 상태의 기록이다.

Rniman Engine을 Convention의 주요 적용·검증 프로젝트로 삼아 함께 발전시킨다. 규칙 전문은 위키 한 곳에서 관리하며 별도 사본이나 submodule은 두지 않는다.

### 작업 시 적용

- 작업 전에 관련 규칙의 현재 채택 상태와 적용 범위를 확인한다. 로컬 Convention을 사용하면 HEAD와 미커밋 변경을 확인한다. 접근하지 못한 범위와 임시 참고 자료는 적용 상태에 기록한다.
- 구현에 필요한 미지정 규칙은 선택 이유·적용 범위·예외·필요한 예시를 기록한다. 사용자 선택이나 기존 채택 근거가 없는 임시 결정은 시험 적용으로 구분한다.
- 규칙을 구체화하거나 개정하면 상호 반영 절차에 따라 공통 내용은 위키의 주제별 문서에, 엔진 전용 컨벤션과 예외는 [위키의 RnimanEngine 문서](https://rniman.github.io/convention/projects/rniman-engine/)에 반영하고 관련 변경 기록을 갱신한다. 제안은 초안으로 남긴다. 엔진의 설계·구현·실행·검증 상세는 이 저장소에서 관리한다.
- 작업을 마칠 때 위키 링크와 실제 적용 범위, 시험 적용·예외·미반영 사항을 아래에 기록한다. 위키 개정의 영향을 받는 코드·도구 설정은 관련 작업 범위부터 맞추며, 즉시 반영하지 못한 대상은 이유와 후속 반영 조건을 남긴다. 시험 적용 결과의 제약과 유지·수정·폐기 이유를 위키에 되돌려 반영하는 것을 권장한다.
- 위키 편집과 커밋·푸시·배포는 별개로 다룬다. 위키에 아직 반영하지 못한 결정은 이 문서에 임시 기록하고 후속 대상으로 유지한다.

### 확인·반영 상태

- **2026-09-30 커밋 전 정리:** 로컬 `D:/Docs/Convention` HEAD `a39c47b`의 채택 기준·상호 반영 절차와 관련 C++ 포맷·헤더·명명·const·소유권 규칙을 확인했다. `docs/baseline.md`와 `docs/projects/rniman-engine.md`에는 기존 미커밋 변경이 있었으며 그대로 유지했다. 기존 규칙을 이번 C++ 변경 범위에 적용했고 공통 규칙·시험 적용 정책은 개정하지 않았다. 원격·배포 상태는 확인하지 않았다.
- **이전 확인:** 2026-09-24 로컬 `D:/Docs/Convention` HEAD `5ca03a1`의 채택 기준·상호 반영 절차, 관련 C++ 규칙과 엔진·HLSL 문서를 확인했다. 작업 전 미커밋 변경은 없었다. Git 소유권 경고는 명령 단위 `safe.directory` 지정으로 읽었으며 전역 설정은 변경하지 않았다. HLSL 문서는 공통 초안이며 채택 규칙으로 간주하지 않았다. 배포 사이트·원격 최신 상태는 확인하지 않았다.
- **기존 적용 기록:** `9e2765c194991072378328d35d7f054fb361fa08`은 기존 코드·설정에서 사용한 컨벤션 버전이다. 이번 문서 변경이 이후 규칙 전체의 코드 반영이나 재검증을 의미하지 않는다.
- **이번 반영:** 초기 구현 당시 로컬 Convention `75bd213835f2b22164e26e430c7e3bed5e507e41`의 채택 기준과 관련 C++ 문서를 확인하여 D3D12Renderer·창 루프·테스트에 적용했다. 확정한 네 가지 초기화 선택과 구현 단계의 시험 적용 사항을 [위키 프로젝트 문서](https://rniman.github.io/convention/projects/rniman-engine/#d3d12-초기-구현-선택)에 로컬 반영했다. 후속 가독성 정리에서는 위키 포맷 문서에 작업별 실패 처리·인자 배치 규칙을 반영했다.
- **프로젝트 전용 예외·시험 적용:** 네 가지 선택은 사용자 확정 범위에 한해 채택했다. ComPtr·Event 소유, bool 반환과 API/HRESULT 보고, Debug 레이어 필수, VSync·두 백 버퍼는 이번 Sandbox의 시험 적용이다. 위키 프로젝트 문서 전체를 채택한 것으로 취급하지 않는다.
- **프레임 동기화 전환:** 2026-09-17 사용자 요청으로 프레임별 Allocator·Fence 값과 재사용 시 대기를 구현했다. 기존 화면 검증 선행 조건은 이번 요청 범위에서 변경했으며 미검증 항목은 완료로 간주하지 않는다. 두 슬롯의 구조체·단일 Command List·공유 Fence/Event는 현재 Sandbox에 한정한 구현 선택이다. 공통 컨벤션·도구 설정은 개정하지 않았다.
- **위키 반영·후속:** 2026-09-17 [프레임 동기화 전환](https://rniman.github.io/convention/projects/rniman-engine/#프레임-동기화-전환)과 baseline 변경 기록은 현재 위 HEAD에 포함되어 있다. 2026-09-18 FXC·Shader Model 5.0, EXE 기준 CSO 로딩, 고정 삼각형의 Upload Heap과 정사각형 Viewport를 [삼각형 출력의 시험 적용](https://rniman.github.io/convention/projects/rniman-engine/#삼각형-출력의-시험-적용)과 baseline 변경 기록에 로컬 반영했다. 공통 C++ 규칙은 개정하지 않았다. 당시 위키 변경은 미커밋 상태였으나 2026-09-24 확인한 위 HEAD에는 포함되어 있다. 푸시·배포는 수행하지 않았다. 원격 최신 상태·배포 완료는 확인하지 않았다.

## 검증과 문서

대표 데모는 개별 시스템 개발 중에도 유지한다. 작은 상호작용을 일찍 넣어 엔진을 사용하는 흐름을 확인한다.

- 유지 샘플과 특정 버전으로 보존할 샘플을 구분하고, 공통 코드 변경 시 영향받는 유지 샘플을 빌드·실행한다.
- ECS 수명·삭제·쿼리, Transform 계층과 바이너리 파싱은 자동 검증 대상으로 삼는다. 화면 결과와 GPU 수명은 실행 확인과 D3D12 디버그 레이어 등으로 보완한다.
- 성능 측정에는 빌드 구성과 조건을 남긴다. 빌드·실행·시각적 확인 결과를 구분하고, 확인하지 않은 기능을 완료로 표시하지 않는다.
- 실행 방법, 코드 진입점, 데이터 흐름과 설계 이유를 기록한다. 상세 문서는 실제 구현이 구체화될 때 추가하고 같은 내용을 중복 관리하지 않는다.

에이전트의 필수 작업 지침은 [AGENTS.md](../AGENTS.md)에 둔다.

## 프로젝트 설정

Visual Studio 프로젝트를 직접 관리하며, CMake는 필요해질 때 전환한다. [솔루션](../RnimanEngine.slnx)을 열고 `Sandbox`의 x64 / Debug 또는 Release 구성을 사용한다. 솔루션과 프로젝트는 x64의 Debug·Release 구성만 유지한다.

- 확인한 설치 환경: Visual Studio Community 2026 18.10.0 (18.10.12201.205), 플랫폼 도구 집합 `v145` (MSVC `14.51.36231`), Windows SDK `10.0.26100.0`.
- x64 공통 설정: C++20, Unicode, `/utf-8`, `/W4`, `/permissive-`, `/EHsc`, Windows 서브시스템, 미리 컴파일된 헤더 사용. 경고를 오류로 처리하는 옵션은 끈다.
- 런타임 라이브러리: Debug `/MDd`, Release `/MD`.
- 출력 위치: `bin/Sandbox/<구성>/`, 중간 산출물: `obj/Sandbox/<구성>/`.
- 루트 [.editorconfig](../.editorconfig)는 UTF-8 BOM 없음·LF·마지막 개행을 지정하고, C++ 파일에 탭 들여쓰기와 표시 너비 4칸을 적용한다. Markdown의 의미 있는 줄 끝 공백은 보존한다. 중괄호·포인터 배치 등은 컨벤션을 직접 따른다.

빠른 빌드·실행 절차는 [README](../README.md#빌드와-실행)를 따른다.

### PCH와 D3D12 기본 연결

- [pch.h](../Sandbox/pch.h)에 Windows·D3D12·DXGI·WRL과 기본 STL 헤더를 모은다. [pch.cpp](../Sandbox/pch.cpp)는 `/Yc`로 생성하고, 이후 추가하는 `.cpp`는 첫 줄에서 `pch.h`를 include하여 `/Yu`로 사용한다.
- PCH 파일은 `$(IntDir)$(ProjectName).pch`로 구성별 분리한다. 일반 프로젝트 헤더에는 PCH를 include하지 않으며, PCH에 있더라도 직접 사용하는 의존성은 각 파일에서 include한다. 이는 채택 컨벤션의 PCH 예외 범위다.
- 프로젝트 전처리 정의에 `WIN32_LEAN_AND_MEAN`, `NOMINMAX`를 지정하여 Windows 헤더 포함 범위를 줄이고 STL의 min/max와 매크로 충돌을 피한다.
- `d3d12.lib`, `dxgi.lib`와 `d3dcompiler.lib`를 연결한다. FXC는 빌드 시 셰이더를 컴파일하며 D3DCompiler API는 실행 시 CSO 읽기·Root Signature 직렬화에 사용한다. 디버거 작업 디렉터리는 `$(OutDir)`로 지정한다.
- 2026-09-14 검증: x64 Debug·Release에서 PCH 생성과 `Main.cpp`의 PCH 사용(`/Yu`), 전체 컴파일·링크가 성공했다. 두 EXE에서 제목 `Rniman Engine - Sandbox`, 창 표시·응답 상태와 클라이언트 크기 1280×720을 Win32 API로 확인했다. `WM_CLOSE` 전송 후 모두 종료 코드 0으로 종료했다. 화면 캡처를 통한 시각 검증과 오류 경로의 강제 실패 검증은 수행하지 않았다.
- 검증 도중 자동 실행 환경의 `Path`·`PATH` 중복과 샌드박스의 MSBuild FileTracker 접근 거부가 발생했다. 검증 프로세스의 환경 변수 중복을 정리하고 샌드박스 밖에서 재검증했으며 프로젝트 설정으로 오류를 우회하지 않았다.

### Git 관리

- 저장소 루트는 루트 README가 있는 폴더이며 초기 브랜치는 `main`이다.
- [.gitignore](../.gitignore)는 Visual Studio 개인 설정·캐시, C++ 중간 파일·디버그 정보·프로파일 결과와 엔진의 빌드 출력 폴더를 제외한다. 솔루션·프로젝트·소스·문서는 버전 관리 대상이다.
- [.gitattributes](../.gitattributes)는 텍스트를 LF로 정규화하며 바이너리 에셋은 줄바꿈 변환에서 제외한다. Windows 명령 스크립트(`.bat`, `.cmd`)의 체크아웃 줄바꿈은 CRLF로 지정한다. 파일 인코딩과 편집 시 포맷은 `.editorconfig`에서 관리한다.
- Unity `.meta`, OBJ 모델(`.obj`), `.bin`·DDS 에셋과 외부 라이브러리 `.lib`·`.dll`·도구 `.exe`는 전역 제외하지 않는다. `bin/`·`x64/` 같은 디렉터리도 엔진의 출력 경로에 한정해 제외한다. 재생성 가능한 에셋 캐시는 실제 저장 경로가 정해지면 해당 디렉터리를 제외한다.

### 창 진입점과 제약

[Main.cpp](../Sandbox/Main.cpp)의 `wWinMain` → `WindowResources::Initialize` → `D3D12Renderer::Initialize` → `PeekMessageW`/`DispatchMessageW`·렌더링 순서로 실행한다. 창 닫기 요청은 `WM_QUIT`로 루프를 종료하고, GPU 완료 확인 후 지역 RAII 객체가 렌더러 → 창 → 창 클래스 순서로 정리한다. 최소화·0 크기에서는 WaitMessage로 대기한다. Win32 호출 실패는 발생 API와 오류 코드를 디버그 출력·메시지 상자로 알린다.

빌드·실행 명령은 [README](../README.md#빌드와-실행)에서 관리한다. 검증에서는 설치된 MSBuild 실행 파일을 직접 호출했다.

1280×720은 초기 클라이언트 크기이며, DPI 정책·최소 창 크기는 아직 미결정이다. WM_SIZE는 최신 크기를 기록하고 다음 프레임에서 백 버퍼를 재생성한다. 창 테두리를 끄는 동안에는 Win32 모달 루프가 끝날 때까지 프레임이 멈출 수 있다.

2026-09-15에 솔루션과 Sandbox 프로젝트를 저장소 루트 구조로 옮겼다. 프로젝트 상대 경로를 유지하며 출력은 루트의 `bin/`, 중간 산출물은 `obj/`를 사용한다. 이동 후 x64 Debug·Release 전체 재빌드와 각 EXE의 창 응답·닫기 후 종료 코드 0을 확인했다. 사용자가 최소 창의 화면 표시를 확인했다.

2026-09-15 초기 커밋 전 점검: 루트 폴더 이름을 공백 없는 RnimanEngine으로 변경한 환경에서 x64 Debug·Release 전체 재빌드, 창 응답과 정상 종료(코드 0), 솔루션·소스·README 링크 및 빌드 산출물 제외 규칙을 확인했다.

2026-09-15 출력 구조 적용: bin/Sandbox/<구성>·obj/Sandbox/<구성> 및 디버거 작업 디렉터리 변경 후 두 구성 재빌드와 새 출력 폴더에서의 직접 실행·정상 종료를 확인했다. 솔루션의 Sandbox 참조는 유지하며, 이전 x64 출력 폴더는 정리했다. Visual Studio에서의 F5 실행은 이번 변경 후 별도로 확인하지 않았다.

## D3D12 화면 출력과 검증

### 진입점과 수명

[Main.cpp](../Sandbox/Main.cpp)의 WindowState는 창 콜백이 빌려 쓰는 크기·최소화 상태다. 앱은 상태 → 창 → 렌더러 순으로 생성한다. WM_SIZE에서는 GPU를 다루지 않고 최신 크기만 기록한다. 프레임 경계에서 Resize를 호출하고 최소화·0 크기에서는 렌더링을 멈춘다.

[D3D12Renderer.cpp](../Sandbox/D3D12Renderer.cpp)는 고성능 순서의 하드웨어 중 Feature Level 11_0 지원 장치를 선택한다. 선택·실패 후보는 디버그 출력으로 기록하며 WARP는 사용하지 않는다. Debug는 장치 생성 전에 디버그 레이어를 켜고 Graphics Tools가 없으면 실패한다.

초기화 순서는 Factory·Adapter·Device → Queue → Swap Chain → RTV·백 버퍼 → DSV Heap·프레임별 Constant Buffer·Allocator·단일 Command List → Fence·Event → 깊이 버퍼·큐브 Pipeline·Vertex/Index Buffer다. Render는 현재 백 버퍼의 이전 제출 완료 확인 → 해당 슬롯의 행렬 복사 → Allocator·Command List Reset → PRESENT에서 RENDER_TARGET 전이 → 색상·깊이 지우기·큐브 indexed draw → PRESENT 복귀 → 명령 제출 → Present(1, 0) → Fence Signal과 해당 프레임 값 기록 순서다.

Resize는 GPU 완료 확인 → 깊이·백 버퍼 참조 해제 → ResizeBuffers → 백 버퍼·RTV·깊이 버퍼·DSV 재생성 순서다. 동일 크기와 0 크기 요청은 무시한다. COM 자원은 ComPtr로, Fence Event는 렌더러 소멸자에서 정리한다. 실패 시 API·HRESULT·Device removed reason을 표시하고 앱을 종료한다. 장치 손실 자동 복구는 구현하지 않았다.

### 프레임별 Fence 동기화

2026-09-17 사용자 요청으로 매 프레임 전체 대기에서 프레임 자원 재사용 시 대기로 전환했다. 기존 선택은 초기 수명 검증을 단순화하기 위한 것이었다. 화면 검증을 모두 끝낸 뒤 전환한다는 이전 조건은 이번 요청에 따라 변경했고, 아래 미검증 범위는 유지한다.

- `mFrames`의 두 `FrameResources`는 각 백 버퍼에 대응하며 Allocator와 마지막 제출의 Fence 값을 소유한다. `GetCurrentBackBufferIndex()`로 고른 슬롯은 `WaitForFence(frame.fenceValue)` 성공 후에만 Reset한다. 초기 값 0은 제출 이력이 없음을 뜻한다.
- 하나의 Command List는 제출 후 다음 슬롯의 Allocator로 재기록한다. GPU가 사용하는 기록 메모리는 슬롯별 Allocator가 유지한다. Fence 값은 렌더러 전체에서 단조 증가하며 Present 이후에도 제출 당시 슬롯에 저장한다.
- `WaitForFence`는 완료 값을 먼저 확인하여 미완료일 때만 Event를 기다린다. 완료 값이 `UINT64_MAX`이면 장치 제거를 보고한다. 프레임 하나의 완료는 다른 제출까지 모두 완료했다는 뜻이 아니므로 전체 미완료 상태를 지우지 않는다.
- `WaitForGpu`는 Queue 끝에 새 Signal을 넣어 전체 제출 완료를 확인한다. 실제 크기가 바뀌는 Resize, 앱 종료, 미완료 작업이 있는 소멸자에서 사용한다. Signal·Present 실패 후에도 소멸자가 전체 대기를 시도하는 경로를 유지한다.
- Resize 이후에도 Fence 카운터와 슬롯의 값을 유지한다. 백 버퍼를 해제하기 전에 전체 대기를 끝냈으므로 기존 값은 이미 완료된 하한이며 다음 제출에서 교체된다.

현재 단일 UI 스레드·단일 Direct Queue·두 백 버퍼 범위의 구현이다. 재사용 대기와 전체 대기의 공통 부분만 렌더러 내부 `WaitForFence`로 묶었으며 별도 범용 동기화 시스템은 도입하지 않았다. VSync와 DXGI의 Present 대기는 계속 존재하며 성능 향상 수치는 측정하지 않았다.

### 자동 검증

[RendererSmoke.cpp](../Tests/RendererSmoke.cpp)는 숨김 테스트 창에서 실제 하드웨어 GPU로 공개 렌더러 API를 실행한다. 테스트 프로젝트는 PCH를 사용하지 않아 D3D12Renderer.h의 독립 컴파일도 확인한다. 각 구성에서 테스트 자체가 디버그 레이어를 활성화하므로 Release 테스트도 Graphics Tools가 필요하다.

저장소 루트의 Developer PowerShell에서:

```powershell
MSBuild.exe Tests/RendererSmoke.vcxproj -nologo -t:Build -p:Configuration=Debug -p:Platform=x64
./bin/RendererSmoke/Debug/RendererSmoke.exe
MSBuild.exe Tests/RendererSmoke.vcxproj -nologo -t:Build -p:Configuration=Release -p:Platform=x64
./bin/RendererSmoke/Release/RendererSmoke.exe
```

2026-09-15 검증:

- Sandbox x64 Debug·Release 빌드 성공.
- 두 구성의 smoke test 실행·종료 코드 0. 각각 24프레임, 8번의 크기 변경 요청(동일 크기·0 크기·1×1·확대·축소 포함), 렌더러 파괴까지 D3D12 InfoQueue 오류·경고 0건.
- 테스트는 InfoQueue를 유지하기 위해 Device 참조를 보관한다. 모든 Device 참조 해제 후의 live-object 검사나 전체 자원 누수 검증을 대신하지 않는다.
- 미검증: 실제 Sandbox의 표시 색상·최소화/복원·연속 드래그 리사이즈·닫기, Visual Studio F5, 오류 경로 강제 실패·장치 손실.
- 시각 검증 도구의 node_repl 실행이 환경 초기화 오류(helper_unknown_error)로 실패했다. 숨김 렌더러 테스트는 GPU 실행 검증이며 창 UI·시각 검증으로 간주하지 않는다.

수동 확인은 Debug로 어두운 파란색 표시 → 크기 변경 → 최소화·복원 → Alt+F4 종료 순서로 수행한다. Visual Studio 출력 창에서 선택 Adapter와 D3D12 오류를 함께 확인한다.

### API 확인 근거

Microsoft의 [Adapter 우선순위 열거](https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_6/nf-dxgi1_6-idxgifactory6-enumadapterbygpupreference)와 [Fence 완료 값](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12fence-getcompletedvalue)을 확인했다(2026-09-15). 장치 제거 시 UINT64_MAX를 정상 완료로 취급하지 않으며 Device removed reason을 보고한다.

### 코드 가독성 정리

2026-09-15 Main·D3D12Renderer·RendererSmoke에서 작업별 실패 처리를 분리하고 긴 호출은 첫 인자부터 한 줄에 하나씩 배치했다. 함수·초기화 단계·멤버 역할 사이의 빈 줄을 정리했다. 짧은 호출과 같은 의미 단위의 값은 함께 유지한다.

[위키 포맷 규칙](https://rniman.github.io/convention/coding/cpp/formatting/)과 [프로젝트 적용 상태](https://rniman.github.io/convention/projects/rniman-engine/#코드-가독성-적용)를 로컬 수정했다. 가독성 정리 당시 120열은 프로젝트 시험 적용이었다. 이후 위키에서 공통 120열 권장과 의미 구조 우선 배치를 채택했으며, 커밋 전 검토에서 현재 코드 배치를 확인했다. 기존 여러 줄 호출은 유지했고 도구 설정은 추가 변경하지 않았다. 기존 미커밋 변경과 사용자 식별자·솔루션의 테스트 등록은 유지했다. Fence 대기 방식과 TODO도 유지한다.

정리 후 검증: 솔루션 x64 Debug·Release 빌드와 각 RendererSmoke 실행 성공(종료 코드 0, 24프레임·8회 요청, 오류·경고 0). Main·헤더·테스트의 토큰 순서가 변경 전과 같음을 확인했다. 렌더러는 실패 처리 분리와 대기 오류의 지역 변수 도입을 검토했다. 실제 창 시각 확인·F5는 이번 포맷 정리에서 재수행하지 않았다.

2026-09-15 커밋 전 재검증: 솔루션 x64 Debug·Release 빌드 및 두 RendererSmoke 실행 성공(각 종료 코드 0, 24프레임·8회 요청, D3D12 오류·경고 0). 실제 창 조작·시각 확인·F5는 재수행하지 않았으며 위 미검증 범위를 유지한다. 프로젝트 파일 두 개의 누락된 마지막 개행을 보완했다.

### 2026-09-17 프레임 동기화 검증

- 솔루션 x64 Debug·Release 빌드 성공. 샌드박스의 기존 `obj` 접근 거부 이후 샌드박스 밖에서 빌드했으며 프로젝트 설정으로 우회하지 않았다.
- 두 구성의 RendererSmoke 종료 코드 0. 각각 136프레임·8회 크기 변경 요청, 제출 전 대기, 반복 전체 대기, 대기 후 렌더링 재개, 마지막 제출 후 소멸자 정리를 검사했다. D3D12 InfoQueue 오류·경고는 각각 0건이다.
- 실제 Debug·Release Sandbox를 실행하여 파란색 화면과 Alt+F4 이후 프로세스 종료를 확인했다. UI 실행의 종료 코드는 수집하지 못했으므로 smoke test의 종료 코드와 구분한다. Debug에서는 최대화·복원과 최소화 후 복원된 파란색 화면도 확인했다.
- 미검증: 연속 드래그 리사이즈, Visual Studio F5, 강제 Signal·Present 실패와 장치 손실, 전체 live-object 누수 검사, 성능 비교. 시각 확인 시 Sandbox의 InfoQueue를 별도로 수집하지 않았으며 오류·경고 0건은 위 smoke test 범위다. 가벼운 clear 작업의 테스트가 장시간 GPU 부하나 모든 대기 분기의 강제 실행을 보장하지 않는다.

동기화 API 근거(2026-09-17 확인): Microsoft의 [Command Allocator Reset](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12commandallocator-reset)과 [Command List Reset](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-reset). 전자는 해당 Allocator를 쓰는 GPU 실행 완료가 필요하며, 후자는 제출된 List의 실행 중에도 재기록할 수 있다.

## 삼각형 출력과 검증

이 절은 2026-09-18 당시의 삼각형 구현·검증 이력이다. 2026-09-24 큐브로 교체했으며 현재 구현은 아래 큐브 절을 따른다. 2026-09-18에 정점 색상을 보간하는 고정 삼각형을 Sandbox에 연결했다. 아래 선택은 초기 데모 범위의 시험 적용이며 범용 메시·재질·카메라 시스템은 아직 없다.

### 셰이더와 파이프라인

- `TriangleShaders.props`(현재 [CubeShaders.props](../Sandbox/CubeShaders.props))가 두 프로젝트의 셰이더 빌드 설정을 공유한다. 원본·출력 위치와 컴파일 옵션은 [셰이더 배치](project-structure.md#셰이더와-공용-캐시)를 따른다. 원본 HLSL과 props는 Git에서 관리하고 CSO는 기존 bin 제외 규칙으로 관리 대상에서 뺀다.
- `CreateTrianglePipeline()`은 `GetModuleFileNameW()`로 EXE 위치를 구하고 `D3DReadFileToBlob()`으로 두 CSO를 읽는다. 셰이더 로딩 실패는 대상 경로·API·HRESULT를 보고한다. 런타임 HLSL 컴파일이나 다른 경로로의 대체 검색은 없다.
- Root Signature 1.0은 입력 레이아웃만 허용하고 매개변수·Sampler는 없다. PSO는 POSITION(float3)·COLOR(float4), VS·PS, 삼각형 토폴로지, RGBA8 단일 Render Target을 연결한다. 솔리드·뒷면 컬링을 사용하고 깊이·스텐실·블렌딩·MSAA는 끈다.
- `CreateTriangleVertices()`는 위치·색상으로 구성한 세 정점을 Upload Heap에 한 번 복사한다. 총 84바이트의 고정 데모여서 별도 Copy Queue·Default Heap 업로드 단계를 먼저 만들지 않았다. CPU가 이후 내용을 수정하지 않으므로 프레임별 복제는 필요하지 않으며 렌더러가 종료 대기 후 해제한다. 일반 메시 로딩 시에는 Default Heap으로의 업로드를 검토한다.
- 빌드 시 CSO 배치는 기존 프로젝트 방향의 실제 적용이다. FXC·SM5.0은 설치된 SDK와 기존 Feature Level 11_0 요구를 유지하는 최소 선택이며, 장기 셰이더 컴파일러 정책의 확정은 아니다.

### 진입점과 데이터 흐름

[Main.cpp](../Sandbox/Main.cpp)의 `wWinMain` → `Initialize()` → `CreateTrianglePipeline()`·`CreateTriangleVertices()` → 매 프레임 `Render()` 순서다.

정점 배열 → Vertex Buffer → Input Assembler → `TriangleVS.hlsl`의 clip-space 위치·색상 → 래스터화와 색상 보간 → `TrianglePS.hlsl`의 색상 출력 → 백 버퍼 → Present로 이어진다.

`Render()`는 현재 슬롯의 완료 후 PSO를 지정하여 Command List를 Reset한다. 백 버퍼를 지운 뒤 Root Signature·Viewport·Scissor·RTV·Vertex Buffer·TRIANGLELIST를 연결하고 `DrawInstanced(3, 1, 0, 0)`을 호출한다. 기존 상태 전이와 슬롯별 Fence 흐름은 유지한다.

Viewport는 창의 짧은 변을 한 변으로 하는 중앙 정사각형이고, Scissor는 전체 백 버퍼다. 창 비율에 따른 삼각형 왜곡을 피하기 위한 데모 선택이며 향후 카메라의 투영 행렬과는 별개다. Resize에서 저장한 너비·높이를 다음 Render가 사용하므로 창 크기가 바뀌어도 Vertex Buffer를 다시 쓰지 않는다.

### 검증 결과와 남은 범위

- 2026-09-18 커밋 전 정리: 백 버퍼 포맷을 Swap Chain·PSO·Resize가 공유하고, 입력 요소 수와 draw 정점 수를 배열에서 계산하도록 정리했다. 읽기 전용 값과 초기화 배치는 기존 컨벤션을 적용했으며 공통 규칙은 개정하지 않았다. 정리 후 x64 Debug·Release 빌드와 각 RendererSmoke 실행(136프레임·8회 크기 변경 요청, 종료 코드 0, D3D12 오류·경고 0)을 재확인했다. 실제 Sandbox 화면은 아래의 기존 검증 기록이며 이번 정리 후 별도로 재확인하지 않았다.
- 솔루션 x64 Debug·Release 빌드 성공. Sandbox·RendererSmoke 각각의 출력 폴더에 VS·PS CSO가 생성되는 것을 확인했다. MSBuild 추적 로그에서 `Main`, `vs_5_0`·`ps_5_0`, Debug `/Zi /Od`와 Release 기본 옵션을 확인했다.
- 두 구성의 RendererSmoke 실행 성공(각 종료 코드 0, 136프레임·8회 크기 변경 요청, D3D12 오류·경고 0). 현재 테스트의 Render 호출은 삼각형 draw까지 실행한다. 테스트는 픽셀 내용을 판정하지 않으므로 화면 확인을 별도로 수행했다.
- Release smoke test를 저장소 밖 `C:/Users/Public/Documents/ESTsoft/CreatorTemp` 작업 디렉터리에서 실행하여 EXE 기준 셰이더 경로를 확인했다.
- 실제 Debug·Release 창에서 빨강 위 꼭짓점·초록 오른쪽 아래·파랑 왼쪽 아래와 내부 색상 보간을 확인했다. Debug 최대화·복원 및 최소화 후 복원에서도 중앙 배치와 비율을 확인했다. Release는 창을 전면 활성화한 뒤 삼각형 출력을 확인했다. 두 구성 모두 Alt+F4 후 프로세스 종료를 확인했으며 UI 실행의 종료 코드는 별도로 수집하지 않았다.
- 별도 임시 폴더에 Release EXE만 복사하여 CSO 누락을 검사했다. `TriangleVS.cso`의 경로와 `D3DReadFileToBlob`·HRESULT `0x80070003`을 표시하고 오류 확인 후 프로세스가 종료됐다. 정상 출력 폴더의 파일은 변경하지 않았다.
- 미검증: Visual Studio F5, 연속 드래그 리사이즈, HLSLI 변경 추적(현재 include 없음), 손상된 CSO·PSO 강제 실패·장치 손실, 전체 live-object 검사와 성능 측정. InfoQueue 오류·경고 0건은 smoke test 범위이며 실제 창의 진단 큐는 별도 수집하지 않았다.

API 흐름은 Microsoft의 [기본 D3D12 컴포넌트 예제](https://learn.microsoft.com/en-us/windows/win32/direct3d12/creating-a-basic-direct3d-12-component)를 참고했다(2026-09-18 확인). 예제의 런타임 셰이더 컴파일·매 프레임 전체 대기 대신 이 프로젝트의 빌드 시 컴파일·프레임별 동기화를 유지한다.


## 큐브 출력과 검증

2026-09-24 고정 삼각형을 회전하는 색상 큐브로 교체했다. 입력으로 카메라를 움직이는 기능이나 범용 장면 시스템은 아직 없으며, 이번 범위는 인덱스 메시·행렬 전달·깊이 처리의 실제 연결이다.

### 진입점과 데이터 흐름

[Main.cpp](../Sandbox/Main.cpp)의 `wWinMain`이 `steady_clock` 경과 시간으로 Y축 회전(0.7 rad/s)을 계산한다. World 회전 → `(0, 2, -5)`에서 원점을 보는 View → 수직 FOV 45도·현재 창 종횡비·near 0.1·far 100의 Projection을 곱해 `Render(worldViewProjection)`에 전달한다. 최소화 중에는 그리기를 멈추지만 시간은 계속 흐르므로 복원 시 현재 시각의 회전 각도로 이어진다.

[D3D12Renderer.cpp](../Sandbox/D3D12Renderer.cpp)의 `Initialize` → `CreateCubePipeline`·`CreateCubeGeometry`·`CreateDepthBuffer`가 GPU 자원을 준비한다. `Render`는 현재 슬롯의 Fence 완료 → 64바이트 행렬 복사 → b0 루트 CBV 연결 → Vertex/Index Buffer 연결 → `DrawIndexedInstanced(36, 1, 0, 0, 0)` → Present·Signal 순서다. 입력 행렬의 참조는 저장하지 않는다.

[CubeVS.hlsl](../Shaders/Samples/Sandbox/CubeVS.hlsl)은 로컬 정점에 `mul(float4(position, 1), worldViewProjection)`을 적용하고 색상을 전달한다. [CubePS.hlsl](../Shaders/Samples/Sandbox/CubePS.hlsl)은 보간된 색상을 출력한다. [CubeShaders.props](../Sandbox/CubeShaders.props)를 두 프로젝트가 공유하며 원본·CSO 이름을 함께 Cube로 변경했다. 과거 Triangle CSO가 출력 폴더에 남아도 읽지 않는다. 배포에는 새 EXE와 Cube CSO 둘을 함께 사용한다.

### 자원 수명과 구현 선택

- 정점 8개(224바이트)와 `R16_UINT` 인덱스 36개(72바이트)는 고정 Upload Heap이다. 범용 메시·파일 로딩 경로를 도입할 때 Default Heap 업로드를 검토한다. 세 종류의 실제 소비자(Vertex·Index·Constant Buffer)가 공유하는 생성 코드만 렌더러 내부 `CreateUploadBuffer`로 묶었다.
- 각 프레임 슬롯이 256바이트 Constant Buffer를 소유한다. 해당 슬롯의 이전 제출 완료 후 Map·복사·Unmap하므로 실행 중인 GPU 데이터와 충돌하지 않는다. 영구 매핑이나 범용 Constant Buffer allocator는 도입하지 않았다.
- CPU는 DirectXMath 행 벡터의 `World * View * Projection`을 저장하고 HLSL은 `row_major`로 읽어 전치하지 않는다. 왼손 View·Projection과 화면에서 시계 방향인 앞면을 사용한다. 이 행렬·좌표 규약은 Sandbox 한정 시험 적용이며 Unity 익스포트 규약의 확정은 아니다.
- 단일 Direct Queue가 순서대로 사용하는 D32 깊이 버퍼 하나를 공유한다. `DEPTH_WRITE` 상태를 유지하고 프레임마다 1로 지운다. PSO는 LESS 비교·깊이 쓰기·뒷면 컬링을 사용한다. Resize에서는 전체 GPU 대기 후 해제·재생성하므로 DSV를 실행 중에 덮어쓰지 않는다.
- 전체 창 Viewport를 사용하고 투영 행렬로 종횡비를 보정한다. 매우 좁은 창에서는 큐브 일부가 시야 밖으로 나갈 수 있으며 자동 카메라 거리 조절은 하지 않는다.

### 검증 결과와 남은 범위

- 솔루션 x64 Debug·Release에서 Sandbox·RendererSmoke와 VS·PS 셰이더 빌드 성공. 샌드박스의 기존 obj 접근 거부 후 승인된 샌드박스 밖 빌드로 확인했다.
- 두 RendererSmoke 실행 모두 종료 코드 0. 각 136프레임·8회 크기 변경 요청, 프레임마다 다른 행렬, 반복 전체 대기·렌더링 재개·마지막 제출 후 소멸자 정리를 실행했다. D3D12 InfoQueue 오류·경고는 각각 0건이다. 동일·0 크기·1×1·확대·축소를 포함하여 깊이 버퍼 재생성과 프레임별 CBV 사용을 함께 검사했다.
- Debug 실제 창에서 색상 큐브 출력, 시간 차이가 있는 화면의 회전 변화, 최대화 후 출력을 확인했다. 실제 창의 InfoQueue는 별도로 수집하지 않았다. UI 도구의 복원 입력은 사용자 입력 감지와 대상 창 가림으로 완료를 확인하지 못했다.
- 큐브 변경 후 Release 실제 화면, 창 복원·최소화·연속 드래그·Alt+F4 종료는 재검증하지 않았다. 이전 삼각형의 결과와 구분한다. Visual Studio F5, CSO 누락·손상 강제 실패, 장치 손실, 전체 live-object 검사, 성능 측정도 이번 검증 범위에 포함하지 않는다.
- smoke test는 GPU 진단과 자원 수명을 검사하며 픽셀 정답·깊이 가림의 독립적인 정확성 검사를 대신하지 않는다. 현재 단일 볼록 큐브는 컬링으로도 뒷면이 제거되므로 여러 물체가 겹치는 장면의 깊이 검증은 후속 범위다.

### 컨벤션 반영 상태

기존 C++ 채택 규칙을 변경 범위에 적용했으며 공통 규칙은 개정하지 않았다. 행렬 전달·프레임별 상수 수명·D32 공유는 위키의 엔진 전용 시험 적용 대상으로 정리한다. 일반화 후보는 CPU/HLSL 행렬 계약과 변경 가능한 GPU 데이터의 재사용 조건이며, 다른 실제 소비자가 생길 때 공통화 범위를 검토한다. 로컬 위키의 [큐브 출력의 시험 적용](https://rniman.github.io/convention/projects/rniman-engine/#큐브-출력의-시험-적용)과 baseline 변경 기록에 반영했다. `D:/Docs/Convention/docs/projects/rniman-engine.md`·`docs/baseline.md`만 수정했고 문서 내용·로컬 링크·diff 공백 검사를 확인했다. 사이트 구성은 변경하지 않았으며 사이트 빌드·커밋·푸시·배포는 수행하지 않았다. 위 URL의 새 절은 배포 이후 제공된다.

### 2026-09-30 커밋 전 정리와 검증

- 사용자가 현재 정상 렌더링을 확인했다. 구성·창 조작별 확인 범위는 특정하지 않았으므로 기존의 세부 미검증 항목을 일괄 완료 처리하지 않는다.
- Main·렌더러·테스트의 include 그룹, 행렬 계산 구간과 인자 배치를 정리했다. 렌더러의 RTV 핸들과 테스트의 WVP 변수 이름을 명확히 하고, 제출 슬롯 인덱스를 읽기 전용으로 표시했다. Render API 주석은 Constant Buffer 복사를 명시한다. 행렬 계산식·draw·GPU 동기화 순서는 유지했다.
- 솔루션 x64 Debug·Release 빌드 성공. 샌드박스의 기존 obj 접근 거부 이후 승인된 샌드박스 밖 빌드로 확인했다.
- 두 RendererSmoke 실행 모두 종료 코드 0. 각각 136프레임·8회 크기 변경 요청, 반복 전체 대기·렌더링 재개·소멸자 정리를 통과했고 D3D12 오류·경고는 0건이다. git diff --check도 통과했다.
- 정리 후 실제 Sandbox 창의 시각·조작 확인과 Visual Studio F5는 재수행하지 않았다. 사용자 확인과 자동 테스트 결과를 구분한다.
- 기존 컨벤션만 적용했으며 새 공통 규칙은 추가하지 않았다. 버퍼 Map·복사 코드는 향후 다른 실제 업로드 경로의 요구가 확인될 때 공통화를 검토한다. 현재의 짧은 호출별 진단 경로는 유지한다.

## 카메라 이동과 입력

ProcessWindowMessage가 입력을 모으고 SandboxCamera의 Move·Rotate가 갱신한 View를 WVP 계산에 사용한다. 이동 시간은 최대 0.1초이며 대각선 속도를 정규화한다. RendererSmoke에 초기 위치·대각선 속도·시간 분할·0초·큰 회전 입력의 CPU 검사를 추가했다.
