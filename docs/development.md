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

- **마지막 확인:** 2026-09-15 커밋 전 검토에서 로컬 Convention HEAD `7b31c7bb593c872eaeb7a56cf3e70817b6ea51c5`의 채택 기준·상호 반영 절차·포맷·프로젝트 문서·커밋 메시지 규칙을 확인했다. 미커밋 변경은 없었다. 배포 사이트와 원격 최신 상태는 이번에 확인하지 않았다.
- **기존 적용 기록:** `9e2765c194991072378328d35d7f054fb361fa08`은 기존 코드·설정에서 사용한 컨벤션 버전이다. 이번 문서 변경이 이후 규칙 전체의 코드 반영이나 재검증을 의미하지 않는다.
- **이번 반영:** 초기 구현 당시 로컬 Convention `75bd213835f2b22164e26e430c7e3bed5e507e41`의 채택 기준과 관련 C++ 문서를 확인하여 D3D12Renderer·창 루프·테스트에 적용했다. 확정한 네 가지 초기화 선택과 구현 단계의 시험 적용 사항을 [위키 프로젝트 문서](https://rniman.github.io/convention/projects/rniman-engine/#d3d12-초기-구현-선택)에 로컬 반영했다. 후속 가독성 정리에서는 위키 포맷 문서에 작업별 실패 처리·인자 배치 규칙을 반영했다.
- **프로젝트 전용 예외·시험 적용:** 네 가지 선택은 사용자 확정 범위에 한해 채택했다. ComPtr·Event 소유, bool 반환과 API/HRESULT 보고, Debug 레이어 필수, VSync·두 백 버퍼는 이번 Sandbox의 시험 적용이다. 위키 프로젝트 문서 전체를 채택한 것으로 취급하지 않는다.
- **후속 반영:** Convention의 초기 구현 선택·가독성 규칙은 위 로컬 HEAD에 커밋된 상태이며, 엔진 변경도 목적별 로컬 커밋으로 정리한다. 원격 최신 상태·위키 배포는 확인하지 않았으므로 위키 URL에는 아직 새 절이 없을 수 있다. 원격 반영 후 양쪽 적용 상태를 갱신한다. 화면 검증 후 Fence 개선 전환 조건을 다시 평가한다.

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
- `d3d12.lib`, `dxgi.lib`를 연결한다. 셰이더 컴파일러는 아직 선택·연결하지 않았다. 디버거 작업 디렉터리는 `$(OutDir)`로 지정한다.
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

초기화 순서는 Factory·Adapter·Device → Queue → Swap Chain → RTV·백 버퍼 → Allocator·Command List → Fence·Event다. Render는 PRESENT → RENDER_TARGET 전이, 어두운 파란색 지우기, PRESENT 복귀, 명령 제출, Present(1, 0), Fence 대기를 수행한다. 셰이더·Root Signature·PSO는 아직 필요하지 않다.

Resize는 GPU 완료 확인 → 백 버퍼 참조 해제 → ResizeBuffers → 백 버퍼·RTV 재생성 순서다. 동일 크기와 0 크기 요청은 무시한다. COM 자원은 ComPtr로, Fence Event는 렌더러 소멸자에서 정리한다. 실패 시 API·HRESULT·Device removed reason을 표시하고 앱을 종료한다. 장치 손실 자동 복구는 구현하지 않았다.

### Fence 후속 개선 — 필수 추적

현재 매 프레임 GPU 완료를 기다려 하나의 Allocator를 안전하게 재사용한다. 초기 동기화와 수명 검증을 단순화하기 위한 사용자 선택이며 CPU·GPU 병렬 실행을 제한한다.

**전환 조건:** 기본 화면 출력·실제 창 리사이즈·종료 검증을 마친 뒤 프레임별 Command Allocator와 Fence 값을 도입한다. 해당 프레임 자원을 다시 사용할 때만 기다리도록 변경하고, resize·종료에는 필요한 전체 GPU 완료 대기를 유지한다. 화면 검증이 남아 있으므로 이번에는 전환하지 않았다. Render의 대기 위치에도 이유·제약·완료 조건을 TODO로 기록했다. 성능 향상 수치는 측정 전 주장하지 않는다.

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
