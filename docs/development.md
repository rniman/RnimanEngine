# 개발 설정과 검증

[프로젝트 소개와 실행](../README.md) · [Architecture](design.md) · [마일스톤 기록](milestones/README.md)

현재 개발에 필요한 컨벤션 적용 상태·적용별 검증 근거·설정·검증 방법을 관리한다. 마일스톤 달성 결과와 당시 제약은 마일스톤 문서에 기록한다.

다음 구현은 [M02 · ImGui로 조작하는 정적 3D 모델 데모](plans/m02-static-model.md)이며, ImGui 최소 도입 → 모델 로딩·렌더링 → ECS 비교의 순서와 단계별 완료 기준은 [로드맵](roadmap.md)에서 관리한다. 아래 검증 결과는 현재 구현의 기록이며 계획한 기능의 구현·통과를 뜻하지 않는다.

## 컨벤션

공통 기준은 위키의 현재 [채택 규칙](https://rniman.github.io/convention/baseline/)과 [적용·근거 검토 절차](https://rniman.github.io/convention/baseline/#위키의-적용과-프로젝트-근거-검토)다. 작업 전 관련 규칙의 필수·권장·선택과 채택·초안·시험 적용을 구분한다. 확인 커밋은 고정 기준이 아니다.

프로젝트에는 실제 적용 범위·이유·예외·검증·후속 조건을 기록한다. 접근하지 못한 기준과 임시 자료도 명시한다. 공통 규칙 전문은 복제하지 않으며 위키 문서 갱신·반영 상태 관리는 위키 정비 담당의 책임이다.

### 확인·적용 상태

| 구분 | 현재 상태 |
|---|---|
| 확인 범위 | 2026-10-20 공개 baseline·채택 문서 14개·관련 초안 6개·프로젝트 참조 문서 확인. 위키 원본 `4beaa82`와 엔진 `087570c` 기준의 [최초 검토](#컨벤션-재검토와-후속-적용-2026-10-20), 이후 [rengine 적용](#단일-루트-네임스페이스-적용-2026-10-20)과 과거 [마일스톤 기록](milestones/v0.1.0-sandbox.md#컨벤션-확인-기록-2026-10-13)을 구분 |
| 실제 적용 | 기존 적용 범위의 명명·포맷·초기화·const·직접 include·한국어 API 주석·반환 계약·소유권·특수 멤버 유지. RendererReadback::Capture에 nodiscard 적용. GetWorld/GetView는 단순 계산 API로 nodiscard 비적용. 타입 명시는 10월 15일 범위와 20일 WindowSmoke·LightingChecks의 지정 변수에 적용. 공용 원본 경로는 10월 16일 적용 범위 유지. rengine 적용 후 CPU/GPU 회귀 검증 재실행. 셰이더 패킹 확인은 과거 검증 유지 |
| 프로젝트 한정 채택 | 창·렌더러 소유 분리, 고성능 하드웨어 우선·WARP 자동 대체 없음, 프레임 경계 리사이즈, 슬롯 재사용 전 Fence 대기, Engine·Sandbox·테스트 C++ 코드의 rengine 단일 루트·Engine/Sandbox 소스 소속 분리 |
| 시험 적용 | WIC·STA·RGBA8, sRGB·단일 밉·선형 Clamp, ComPtr·Event 수명·bool/HRESULT 진단, Debug 레이어·두 백 버퍼·VSync, FXC·SM5.0·D32·카메라·Transform·Phong·테스트 readback |
| 예외·미정·후속 | Sandbox 전용 구현은 PCH를 자기 헤더보다 먼저 포함하며 Engine 구현은 PCH 비사용. Windows→DirectX include 순서·긴 오류 문자열 예외 유지. Sandbox·Tests의 소규모 평면 배치는 유지하고 Engine은 기능별로 배치. main·wWinMain과 외부 API·HLSL은 rengine 적용 대상에서 제외. 소스 소속 분리와 추후 라이브러리 경계는 아래 적용 기록 참고. 로컬 정면·거리 단위·외부 에셋 변환은 실제 사용 시 결정. 비정방 행렬·행렬 배열은 도입 시 별도 패킹 검증 |
| 초안 | C++ 자료형·변환·attribute·클래스 미결정 정책과 HLSL·검증·원본/생성물 규칙은 공통 초안 유지. 기존 프로젝트 한정 결정과 구분 |

프로젝트의 [구현 선택과 후속 조건](design.md#구현-선택과-후속-조건)을 따른다. 규칙 변경 시 해당 범위의 코드·설정을 확인하고, 확인만 한 항목과 실제 수정·시험 적용한 항목을 구분한다. 적용하지 못하면 대상·이유·후속 조건을 남긴다. 적용별 근거는 아래 기록에, 마일스톤 검증은 해당 마일스톤에 기록한다.

### Engine과 Sandbox 소스 소속 분리 (2026-10-20)

- Phong 데모와 일반 오브젝트·ECS 단계 사이에서 공통 코드의 소속을 먼저 정리했다. 창·이미지 로딩·렌더링·렌더 데이터·Transform의 C++ 9개 파일은 Engine의 Platform·Assets·Rendering·Math에, 실행·장면 구성·카메라/조명 조작은 Sandbox에 둔다. 세 실행 프로젝트와 `rengine` 단일 루트를 유지하며 별도 라이브러리·추상 API는 추가하지 않았다.
- 웹 조회 도구는 공개 위키에 접근하지 못했지만 읽기 전용 HTTP 조회로 baseline·프로젝트 구조·헤더 문서의 응답 200을 재확인했다. 로컬 위키 `4beaa82`는 깨끗했으며 해당 채택 규칙의 파일 동시 배치·include 기준·허용 의존성과 PCH 예외를 확인했다. 위키 문서는 수정하지 않았다.
- Engine 구현 3개는 Sandbox PCH include를 제거하고 Sandbox 프로젝트에서도 파일별 PCH 비사용으로 빌드한다. Engine은 Sandbox include 경로 없이 사용할 수 있다. Sandbox와 RendererSmoke는 Engine·Sandbox 루트를, WindowSmoke는 Engine 루트만 사용한다. 실제 경로·프로젝트 입력·Sandbox의 기능별 필터·문서 링크를 함께 갱신했다.
- 공통 원본 경로 설정은 `Build/SourcePaths.props`로 옮기고 두 소비자의 import를 갱신했다. `Sandbox/CubeShaders.props`와 `Shaders/Samples/Sandbox/`는 큐브 데모 설정으로 유지한다. RendererSmoke가 데모 카메라·조명을 검증하는 참조도 유지한다.
- 알고리즘·공개 타입·호출 순서·자원 수명·오류 처리·CPU/HLSL 레이아웃·에셋/출력 경로는 유지한다. 현재 렌더러의 고정 큐브·Phong 파이프라인과 창의 Sandbox 이름·기본값은 남은 제약이며 실제 재사용 시 API 분리를 검토한다. 이후 라이브러리 분리에서도 소비자의 프로젝트 참조·초기화 계약 변경은 필요할 수 있다.
- 검증: Visual Studio Community 2026 18.11.0·MSVC v145/14.51.36231·SDK 10.0.26100.0·C++20·Unicode의 x64 Debug·Release 솔루션 빌드 성공. 두 구성 모두 RendererSmoke·WindowSmoke를 TEMP 작업 디렉터리에서 실행해 통과했다. RendererSmoke는 각 136프레임·8회 리사이즈·D3D12 오류/경고 0·잘못된 가림 픽셀 0이며 CPU 행렬/카메라/조명·PNG·GPU 텍스처/Phong/가림을 확인했다.
- 구성별로 Engine 헤더 6개와 구현 3개를 Engine include 경로만 사용해 PCH·강제 include 없이 `/Zs /Y- /W4 /WX` 컴파일했다. 소비자 헤더 5개도 해당 include 루트에서 독립 컴파일을 통과했다. 프로젝트별 원본 입력·중복 등록·import·PCH 비사용 설정과 필터 참조를 대조했으며, 숨김 Visual Studio 자동화로 세 C++ 프로젝트 로드와 Sandbox의 Engine 기능별 필터가 실제 파일 9개를 가리킴을 확인했다.
- Sandbox는 두 구성 모두 실제 BaseColor·TEMP 작업 디렉터리·숨김 창으로 실행해 합성 조명 키·포커스 해제·리사이즈 2회·WM_CLOSE 종료 코드 0·창 파괴를 확인했다. 셰이더 소스·CPU/HLSL 계약이 그대로여서 패킹 dumpbin 대조는 과거 검증을 유지한다.
- C++ 21개 파일은 include·공백을 제외한 내용이 이동 전과 같고 SourcePaths.props 내용도 동일하다. 저장소 문서의 로컬 링크·앵커와 변경 파일의 UTF-8/BOM 없음·LF·끝 개행·git diff --check를 확인했다. 기존 DirectXMath 문서 정리는 보존했으며 원본 에셋·손상 PNG 테스트 파일을 변경 사항에 포함하지 않았다.

<a name="행렬-계산-결과-진단-적용-2026-10-20"></a>

### nodiscard 적용 범위 (2026-10-20)

- [반환값 규칙](https://rniman.github.io/convention/coding/cpp/functions/#반환값-무시와-nodiscard)에 따라 Getter라는 이유로 `[[nodiscard]]`를 일괄 적용하지 않는다. 이 프로젝트에서는 성공 여부·완료 여부 등 반환값 확인이 후속 동작의 전제인 API에 우선 적용한다.
- [Transform::GetWorld](../Engine/Math/Transform.h)·[SandboxCamera::GetView](../Sandbox/SandboxCamera.h)는 상태 변경이나 자원 처리가 없는 행렬 계산이므로 `[[nodiscard]]`를 사용하지 않는다. 반환형·const·행렬 계산·기존 호출부의 결과 사용은 유지한다.
- [RendererReadback::Capture](../Tests/RendererReadback.h)는 실패 시 픽셀 검증을 중단해야 하므로 `[[nodiscard]]`를 유지한다. 기존 초기화·GPU 대기·입력 소비 API의 표기도 유지하며 경고 설정·공통 attribute 초안의 상태는 변경하지 않는다.
- 검증: MSVC v145/14.51.36231·C++20의 x64 Debug·Release 솔루션 빌드 성공. RendererSmoke는 두 구성 모두 CPU 계약·PNG·카메라와 GPU 텍스처/Phong/가림을 통과했고 각 136프레임·8회 리사이즈·D3D12 오류/경고 0·잘못된 가림 픽셀 0이다.
- 구성별 최소 번역 단위에서 GetWorld·GetView의 결과를 버리는 호출이 PCH·강제 include 없이 `/Zs /Y- /W4 /WX` 컴파일을 통과했다. Capture 결과를 버리는 코드는 계속 C4834를 진단하고 `/WX`에서 컴파일 실패했다.
- Sandbox는 두 구성 모두 실제 BaseColor·TEMP 작업 디렉터리·숨김 창으로 각 2초 실행한 뒤 WM_CLOSE 종료 코드 0·창 파괴를 확인했다. 두 헤더가 namespace 커밋 당시 내용과 같고 Capture의 표기는 유지됨을 대조했으며 문서 링크·파일 형식·git diff --check도 확인했다.

### 테스트 상태와 픽셀 인덱스 타입 명시 (2026-10-20)

- [auto 기준](https://rniman.github.io/convention/coding/cpp/auto/)에 따라 WindowSmoke의 상태 참조 1개를 `const Win32Window::State&`, 입력 복사본 9개를 `const Win32Window::InputState`, 종료 코드 1개를 `const std::optional<int>`로 명시했다. 상태의 참조 수명·입력 소비·nullopt와 종료 코드 0의 구분을 유지하며 `<optional>`을 직접 포함한다.
- LightingChecks의 ambient는 `const FrameLighting`, 픽셀 index는 `const std::size_t`로 명시했다. row·column은 각각 0~2이므로 기존 UINT 계산식의 index는 12352~24704다. 192×192 readback의 36864픽셀 범위 안에 있고 계산 중 UINT 오버플로도 없다. 계산식은 유지하고 결과를 size_t로 받으며 불필요한 캐스트는 추가하지 않았다.
- 변경은 지정한 선언과 직접 include에 한정한다. captureCenter 람다의 auto, 반복·기존 요소 참조 등의 기존 사용 기준은 유지한다. 자료형 선택 초안 전체를 공통 채택하지 않으며 새 타입 별칭·변환 도우미는 추가하지 않았다.
- 검증: MSVC v145/14.51.36231·C++20의 x64 Debug·Release에서 WindowSmoke·RendererSmoke 빌드·실행 성공. WindowSmoke는 입력 소비·캡처/포커스/모달 초기화·리사이즈·종료·수명 검증을 통과했다. RendererSmoke는 CPU 계약·PNG·카메라와 GPU 텍스처/Phong/가림을 통과했으며 각 136프레임·8회 리사이즈·D3D12 오류/경고 0·잘못된 가림 픽셀 0이다.
- 선언 13개와 optional 직접 include를 제외한 두 테스트 파일의 내용이 기존 코드와 같음을 대조했다. 문서 링크·파일 형식·git diff --check도 확인했다. Sandbox의 코드·헤더는 이 타입 명시 작업에서 변경하지 않아 빌드·실행 대상에서 제외했다. 기존 Capture의 nodiscard는 유지했다.

### Readback 결과 확인 진단 적용 (2026-10-20)

- [RendererReadback::Capture](../Tests/RendererReadback.h)에 `[[nodiscard]]`를 적용했다. 실패 시 픽셀 검증을 중단해야 하는 기존 반환 계약을 호출자가 확인하도록 진단하기 위한 선언 보강이다. [채택된 반환값 규칙](https://rniman.github.io/convention/coding/cpp/functions/#반환값-무시와-nodiscard)을 적용했으며 attribute 문서 전체의 공통 초안 상태는 유지한다.
- RendererSmoke·LightingChecks의 Capture 호출 12개는 모두 실패 시 검증을 중단한다. 호출부·실행 로직·출력 버퍼의 부분 변경 가능성·GPU 자원 수명은 유지했다. 결과를 변수에 저장하는 것만으로 성공 여부 검사가 보장되지는 않으며, 기존 실패 처리가 계속 필요하다.
- 검증: MSVC v145/14.51.36231·C++20에서 Debug·Release RendererSmoke 빌드·실행 성공. 각 136프레임·8회 리사이즈·D3D12 오류/경고 0·잘못된 가림 픽셀 0을 확인했다. PNG·Transform·카메라·조명과 GPU 텍스처/Phong/가림 검증도 통과했다.
- 구성별 최소 번역 단위에서 Capture 결과를 버리는 코드는 C4834를 진단했고 `/WX`에서 컴파일 실패했다. 결과를 상위 호출자에게 반환하는 코드는 PCH·강제 include 없이 `/Zs /Y- /W4 /WX` 컴파일을 통과했다. 진단 검증용 파일은 obj 아래에만 생성했으며 프로젝트의 경고 설정은 변경하지 않았다.
- Sandbox·WindowSmoke는 이 테스트 어댑터를 사용하지 않아 Capture 적용 작업의 빌드·실행 대상에서 제외했다. 문서 링크·파일 형식·git diff --check를 확인했다.

### 단일 루트 네임스페이스 적용 (2026-10-20)

- 직접 작성한 C++ 코드의 공통 루트를 `rengine`으로 프로젝트 한정 채택·적용했다. 제품 소속을 드러내고 전역 이름 충돌을 줄이기 위한 변경이다. [공통 명명 규칙](https://rniman.github.io/convention/coding/cpp/naming/#네임스페이스)의 권장 루트를 이 프로젝트에 구체화했으며 하위 모듈은 추가하지 않았다.
- 적용 전 공개 baseline·명명·헤더·포맷의 응답 200과 로컬 위키 `4beaa82`의 깨끗한 상태를 재확인했다. 위키 문서 편집은 수행하지 않았다.
- Sandbox의 데이터·수학·경로/PNG 로더·창·렌더러와 SandboxApp·카메라·조명, 테스트의 RendererReadback·LightingChecks·파일 내부 보조 코드를 같은 루트에 배치했다. 익명 네임스페이스는 루트 안에 유지하고 `RendererReadback` friend와 `LightingChecks.h` 전방 선언도 같은 타입을 가리키도록 맞췄다.
- `main`·`wWinMain`은 전역에 유지하고 `rengine::SandboxApp` 등 한정된 이름으로 호출한다. include는 루트 밖에 두며 전역 호환 별칭·헤더의 using namespace는 추가하지 않았다. Win32·DirectX·표준 라이브러리·HLSL은 기존 계약을 유지한다. 진입점과 범위는 [설계](design.md#네임스페이스), include·의존성은 [프로젝트 구조](project-structure.md#현재-include-기준과-허용-의존성)를 따른다.
- 일반 C++ 헤더·구현 18개와 앱 진입점 1개를 수정했다. PCH·프로젝트·필터·공용 props·셰이더·에셋 경로는 유지하고 들여쓰기 증가로 120열을 넘는 선언·호출은 줄바꿈했다. 별도 시스템의 공통화 없이 기존 RenderData·경로/셰이더 props를 계속 공유한다. Capture·테스트 타입의 후속 적용은 각각 위 기록을 따른다.
- 기존 전역 타입·함수 이름은 `rengine::` 한정이 필요하므로 외부 C++ 소비자가 있다면 소스 수정·재빌드가 필요하다. 직렬화 형식·CPU/HLSL 레이아웃·호출 순서·수명·실패 계약 변경은 없다.
- 검증: Visual Studio Community 2026 18.11.0·MSVC v145/14.51.36231·SDK 10.0.26100.0의 x64 Developer PowerShell에서 Debug·Release 솔루션 빌드 성공. 변경한 헤더 11개를 구성별 최소 번역 단위로 PCH·강제 include 없이 `/Zs /Y- /W4 /WX` 단독 컴파일해 모두 통과했다. 기존 MSPDB 제약을 고려해 빌드는 PreferredToolArchitecture=x64·nodeReuse:false로 실행했으며 이번 실행은 샌드박스 안에서 성공했다.
- RendererSmoke는 두 구성 모두 CPU 조명/행렬·PNG·카메라와 GPU 텍스처/Phong/가림 검증을 통과했다. 각 136프레임·8회 리사이즈·D3D12 오류/경고 0·잘못된 가림 픽셀 0을 확인했고 WindowSmoke도 두 구성 종료 코드 0이다.
- 실제 BaseColor를 사용하는 Sandbox는 두 구성 모두 TEMP 작업 디렉터리에서 숨김 창으로 각 2초 실행한 뒤 WM_CLOSE로 종료 코드 0·창 파괴를 확인했다. C++ 19개 파일의 변경을 네임스페이스·이름 한정·공백 변경을 제외한 토큰과 대조해 기존 로직이 유지됨을 확인했다.
- 최종 점검: 수정한 문서 4개의 로컬 링크 91개·절 앵커 29개, 변경 파일 23개의 UTF-8/BOM 없음·LF·끝 개행과 git diff --check를 통과했다. 이름 있는 C++ 네임스페이스는 rengine 하나이며 using namespace 없이 호출하고 손상 PNG 테스트 파일도 정리됐음을 확인했다.

### 컨벤션 재검토와 후속 적용 (2026-10-20)

최초 검토는 **코드 대조와 프로젝트 문서 수정** 범위였다. 아래 표는 후속 코드 적용까지 반영한 현재 상태이며, 세부 범위와 검증은 위 적용별 기록을 따른다. 권장·선택 항목을 필수 위반으로 취급하지 않는다.

#### 확인 근거와 범위

- 웹 도구와 기본 네트워크 조회는 실패했지만, 읽기 전용 HTTP 조회로 공개 사이트 22개 문서의 응답 200·본문·상태 표시를 확인했다. 대응하는 GitHub `master`의 Markdown 원본 22개는 로컬 `D:/Docs/Convention/docs/`와 LF 정규화 후 내용이 같았고, 로컬 위키 `4beaa82`의 작업 트리는 깨끗했다. 이번 판단은 공개본을 확인한 결과이며, 10월 15·16일의 접근 실패 기록은 당시 사실로 보존한다.
- [baseline](https://rniman.github.io/convention/baseline/)의 채택 범위는 C++ 11개·좌표축/행렬 1개·Git 2개다. `auto`는 10월 15일, 프로젝트 구조·네임스페이스는 10월 16일 채택이며 필수·권장·선택 강도는 각각 유지한다. C++ 문서 간소화만으로 기존 규칙이 새 의무가 된 것으로 해석하지 않는다.
- 관련 초안은 자료형·변환·attribute·HLSL·검증·원본/생성물 6개와 클래스 문서의 미결정 절을 확인했다. [위키의 프로젝트 참조 문서](https://rniman.github.io/convention/projects/rniman-engine/)는 대조 근거로 읽었으며 수정하지 않았다. Unity·HTML·다른 프로젝트의 규칙은 이번 적용 검토에서 제외했다.
- 엔진 `087570c`의 Sandbox·두 Smoke·셰이더, 프로젝트/필터·공용 props·편집/Git 설정과 현재 문서를 정적으로 대조했다. 작업 시작 시 엔진 작업 트리는 깨끗했다. 과거 기록의 “미적용 항목 없음”은 당시 확인 범위의 판단이며 저장소 전체의 준수나 아래 후보의 적용 완료를 뜻하지 않는다.

#### 항목별 적용 상태와 후속 조건

| 항목·상태 | 근거와 실제 대상 | 적용 내용·판단 이유 | 검증·후속 조건 |
|---|---|---|---|
| 1 · 적용 완료 | [반환값 규칙](https://rniman.github.io/convention/coding/cpp/functions/#반환값-무시와-nodiscard), [RendererReadback::Capture](../Tests/RendererReadback.h) | `[[nodiscard]]`를 적용해 실패 시 검증을 중단해야 하는 기존 계약을 호출자 진단으로 드러냄. RendererSmoke·LightingChecks의 호출부는 기존 결과 검사 유지 | [적용·검증 기록](#readback-결과-확인-진단-적용-2026-10-20) 참고. attribute 문서 전체의 초안 상태와 구분 |
| 2 · 적용 완료 | [네임스페이스 권장](https://rniman.github.io/convention/coding/cpp/naming/#네임스페이스), Sandbox·테스트의 타입·함수·전방 선언·friend | `rengine` 단일 루트 적용. Sandbox 전용 앱·카메라·조명과 테스트도 포함하며 하위 모듈은 추가하지 않음 | [실제 적용 기록](#단일-루트-네임스페이스-적용-2026-10-20) 참고. 폴더 재배치와 별개이며 main·wWinMain은 전역 유지 |
| 3 · 적용 완료 | [auto 기준](https://rniman.github.io/convention/coding/cpp/auto/), [WindowSmoke](../Tests/WindowSmoke.cpp)의 State·InputState·optional 종료 코드, [LightingChecks](../Tests/LightingChecks.cpp)의 ambient·픽셀 index | State는 const 참조, InputState는 const 값, 종료 코드는 std::optional<int>로 명시. ambient는 FrameLighting, index는 std::size_t로 명시하고 기존 계산 범위를 확인 | [적용·검증 기록](#테스트-상태와-픽셀-인덱스-타입-명시-2026-10-20) 참고. 람다·chrono·범위 반복·기존 요소 참조·캐스트로 타입이 드러나는 포인터의 auto 유지 |
| 4 · 비적용 | [반환값 규칙](https://rniman.github.io/convention/coding/cpp/functions/#반환값-무시와-nodiscard), [Transform::GetWorld](../Engine/Math/Transform.h)·[SandboxCamera::GetView](../Sandbox/SandboxCamera.h) | 상태·자원을 변경하지 않는 단순 행렬 계산 API이므로 `[[nodiscard]]`를 사용하지 않음. 기존 호출부는 반환 행렬을 계속 사용 | [적용 범위와 검증](#nodiscard-적용-범위-2026-10-20) 참고. 실패 후 검증 중단이 필요한 Capture에는 유지 |
| 5 · 소스 소속 분리 적용 | [프로젝트 구조](https://rniman.github.io/convention/coding/cpp/project-structure/), Engine·Sandbox·테스트 프로젝트/필터 | 공통 소스는 Engine의 기능별 폴더에, 데모 실행·조작은 Sandbox에 배치. 실제 입력·include·필터를 함께 수정 | [적용 기록](#engine과-sandbox-소스-소속-분리-2026-10-20) 참고. 별도 라이브러리·공개 API 분리는 실제 독립 빌드·재사용 수요가 생길 때 검토 |

폴더 재배치 시 프로젝트 입력·include·필터와 두 Smoke가 직접 빌드하는 Engine 원본 경로를 함께 맞춘다. `RendererReadback`의 friend와 `LightingChecks.h`의 전방 선언은 같은 `rengine` 타입을 계속 가리켜야 한다. 실제 공유 코드는 원본 한곳을 계속 참조하며 새 복사본을 만들지 않는다. 현재 확인된 사용처만으로 기존 RenderData·경로/셰이더 props 외에 범용 매니저·기본 타입 별칭·변환 도우미를 추가할 이유는 없다.

#### 문서 반영과 검토 결과

| 대상 | 반영 내용·검토 결과 |
|---|---|
| 이 문서의 상태 표 | 공개본 확인일·범위와 auto·구조·네임스페이스의 채택 상태를 갱신. 과거 적용·검증과 이번 정적 대조를 구분하고 후속 후보를 명시 |
| [프로젝트 구조](project-structure.md#현재-include-기준과-허용-의존성) | Engine/Sandbox 소스 소속과 include 기준·허용 의존성·PCH 및 소비자별 직접 원본 빌드를 명시. 빌드 단위는 기존 세 실행 프로젝트 유지 |
| [설계의 후속 조건](design.md#구현-선택과-후속-조건) | 포괄적인 “미적용 항목 없음” 표현을 확인 범위·항목별 상태로 대체. Capture의 nodiscard·테스트 타입 명시와 GetWorld/GetView의 nodiscard 비적용 기준을 각각 적용 기록에 연결 |
| 행렬·좌표계 | RenderData의 행 우선 저장, CubeVS의 행렬별 row_major·행 벡터 mul, 법선용 역전치와 CPU 크기/offset 검사는 현행 유지. 추가 전달 전치·열 우선 전환의 근거 없음. 셰이더 패킹 대조는 10월 13일 검증을 유지하며, 후속 코드 적용에서는 위 기록대로 CPU/GPU 회귀 검증을 재실행 |
| 소유권·오류·주석 | 앱의 창/렌더러 역순 정리, COM/ComPtr·Fence 수명과 반환 실패 처리를 유지. 소멸자 결과 무시의 기존 예외는 GPU 완료 보장이 아님. 모든 Getter의 주석·nodiscard·특수 멤버 다섯 선언을 기계적으로 추가하지 않음 |
| 공통 초안·미결정 | HLSL 명명·파일 구성, 자료형·변환·attribute 전체, 오류 정책·noexcept는 공통 채택으로 확대하지 않음. 실제 사용 중인 WIC·sRGB·Phong 등의 프로젝트 상태도 유지 |
| 도구·생성물·Git | .editorconfig·.gitattributes·현재 출력/원본 배치 유지. 포맷터·경고 오류화·캐시는 도입 필요와 검증 조건이 생길 때 별도 검토. 후속 적용은 네임스페이스 커밋 `3b72de6`과 테스트 계약 커밋 `0a67d1b`에 기록. 새 브랜치 생성·이력 재작성은 수행하지 않음 |

최초 검토의 검증: C++ 소스·헤더 21개의 UTF-8 유효성·BOM 없음·LF·끝 개행·후행 공백 없음·줄 시작 공백 들여쓰기 없음을 확인했다. 프로젝트 입력·include 설정·호출부를 정적으로 대조하고, 수정한 문서 3개의 로컬 링크 61개·절 앵커 18개와 공개 위키 링크 8개·해당 앵커, UTF-8/BOM 없음·LF·끝 개행·git diff --check를 통과했다. 이 확인은 120열·모든 선언의 의미적 준수나 헤더 독립 컴파일을 보장하지 않는다. 최초 검토에서는 문서만 수정하여 빌드·Smoke·Sandbox 실행·시각 확인은 수행하지 않았다. 후속 코드 변경의 빌드·실행·헤더 독립 컴파일·진단 검증은 위 적용별 기록에 구분해 남겼다.

후속 코드 적용 시에는 변경한 헤더를 PCH·강제 include 없이 단독 컴파일하고, 영향받는 두 구성의 Smoke를 빌드·실행한다. 공통 API·네임스페이스·경로를 바꾸면 Sandbox와 영향받는 Smoke를 x64 Debug·Release로 빌드·실행하며, 재배치 시 Visual Studio 로드·파일 표시도 확인한다. 행렬·셰이더 경계가 바뀌면 패킹과 GPU 픽셀 검증을 다시 수행하고, 실제 화면 확인 결과는 실행 성공과 별도로 기록한다.

### 타입 명시와 렌더 데이터 명명 적용 (2026-10-15)

- 공개 baseline 조회가 실패하여 현재 공개본과의 일치는 확인하지 못했다. 임시 참고 자료는 로컬 `D:/Docs/Convention/docs/`의 baseline·C++ 명명·초기화·헤더·자료형 선택 문서다. 명명·초기화·헤더는 채택, 자료형 선택은 초안으로 구분했다. 위키 문서 수정은 수행하지 않았다.
- Sandbox의 실패 가능한 `std::optional`·이름 있는 결과 구조체, DirectX 행렬·벡터·저장 타입, 경로·입력/창 상태와 D3D12 descriptor의 타입을 명시했다. 영향받는 RendererSmoke·LightingChecks·RendererReadback도 같은 기준으로 맞췄다. 이 범위를 프로젝트 한정 채택으로 기록하며 자료형 초안 전체를 공통 채택으로 확대하지 않는다.
- 범위 반복, 기존 요소의 참조, 오른쪽 캐스트로 타입이 드러나는 포인터, chrono 시각과 람다는 `auto`를 유지한다. 10월 15일 적용에서는 WindowSmoke의 선언을 제외했다. 해당 선언은 [10월 20일 후속 적용](#테스트-상태와-픽셀-인덱스-타입-명시-2026-10-20)에서 명시적 타입으로 맞췄다.
- `RenderObject` → `ObjectTransformConstants`, `CreateRenderObject` → `BuildObjectTransformConstants`로 앱·렌더러·검증 코드·설계 문서를 함께 변경했다. 장면 오브젝트 생성과 변환 상수 계산의 구분이 목적이며 행렬 계산·실패 계약·192바이트 레이아웃·GPU 전달 방식은 유지한다.
- EXE 폴더를 Assets 폴더로 오해하지 않도록 `AssetDirectoryResult` → `ExecutableDirectoryResult`, 호출부의 변수 → `executableDirectoryResult`로 맞췄다. 공통 계산은 기존 RenderData와 EXE 경로 함수에서 계속 공유하며 새 범용 추상화는 추가하지 않았다.
- 검증: x64 Debug·Release 솔루션 빌드 성공. RendererSmoke는 두 구성 모두 CPU 행렬/실패 계약·PNG·카메라·조명과 GPU 텍스처/Phong/가림을 통과했고 각 136프레임·8회 리사이즈·D3D12 오류/경고 0·잘못된 가림 픽셀 0을 확인했다. WindowSmoke도 두 구성 종료 코드 0이다.
- Sandbox는 두 구성을 숨김 창으로 실행했다. 작업 디렉터리를 TEMP로 바꿔 실제 BaseColor·CSO의 EXE 기준 로딩을 확인하고 각 2초 실행 후 WM_CLOSE로 종료 코드 0을 얻었다. 변경한 문서의 로컬 링크 파일 대상 67개와 새 제안 절 앵커·diff/파일 형식을 확인했다.
- 최초 샌드박스 기본 도구 실행은 두 구성 모두 LNK1101(MSPDB140.DLL 버전 오류)로 링크 실패했다. VsDevCmd의 x64 환경·PreferredToolArchitecture=x64·node reuse 비활성화로 다시 실행했으며 Debug는 샌드박스 안, Release는 샌드박스 밖에서 빌드 성공했다. 환경과 실행 조건이 달라 오류 해소 원인을 특정하지 않는다. 에셋 경로 공유는 제안만 기록했으며 빌드 복사·런타임 로딩 동작은 변경하지 않았다.

### 공용 원본 경로 적용 (2026-10-16)

- 공개 baseline 조회가 실패하여 현재 공개본 일치는 확인하지 못했다. 로컬 `D:/Docs/Convention/docs/`의 최신 baseline·C++ 명명·초기화·auto·프로젝트 구조를 임시 참고했다. 로컬 baseline의 auto·프로젝트 구조 문서가 현재 채택 상태임을 확인했으며 이전 기록의 초안·확인 범위와 구분한다.
- Sandbox의 루트 `Assets/` 직접 읽기와 PNG 빌드 복사 제거를 프로젝트 한정 채택했다. RendererSmoke도 `Tests/Assets/`를 공유하며 손상 PNG 쓰기는 테스트 출력으로 분리했다. 원본 누락의 빌드 오류·런타임 경로/API/HRESULT 진단은 유지한다.
- 실제 두 소비자가 공유하는 빌드 설정을 `Sandbox/SourcePaths.props`로 분리했다. 이 설정은 10월 20일 소스 소속 정리에서 `Build/SourcePaths.props`로 이동했다. 확정된 `OutDir`와 `EngineRoot`로 원본 경로를 계산하여 전달하므로 개발자별 절대 경로나 고정된 상위 폴더 개수를 C++ 코드에 넣지 않는다. 런타임 경로 변경 요구가 없어 설정 파일 파서·원본 검색·범용 자원 시스템은 도입하지 않았다. 경로 계산은 앱·테스트에서, 디코딩은 AssetLoader에서, GPU 업로드는 렌더러에서 담당한다.
- 새 경로 선언은 명시적 타입을 사용했다. 기존 클래스·네임스페이스의 일괄 재배치는 하지 않았으며 당시 Sandbox의 평면 파일 배치를 유지했다. SourcePaths.props는 실제 빌드에서 두 프로젝트가 import하는 공용 설정으로 추가했고 기존 include 기준·공개 API·C++ 소스/필터 배치는 변경하지 않았다. 위키 문서 편집은 수행하지 않았다.
- 배포본의 독립 실행, 실행 중 에셋 추가·루트 선택, DDS 캐시는 후속 구현 범위다. 기본 출력 구조를 유지하여 저장소 전체를 이동하는 경우와 EXE만 옮기는 경우의 차이는 README와 프로젝트 구조 문서에 기록했다.
- 검증: x64 Debug·Release 솔루션 빌드·RendererSmoke·WindowSmoke 성공. RendererSmoke는 각 136프레임·8회 리사이즈·D3D12 오류/경고 0·잘못된 가림 픽셀 0이다. 최초 두 구성 빌드는 기존 MSPDB 제약을 피한 샌드박스 밖 VsDevCmd x64 환경에서 실행했고, 복사본 정리 후 재생성 여부 확인을 위한 빌드는 샌드박스 안에서도 성공했다.
- 원본과 해시가 같은 기존 Sandbox·RendererSmoke의 Debug/Release PNG 복사본 4개만 삭제하고 다시 빌드해 재생성되지 않음을 확인했다. 두 구성 Sandbox를 TEMP 작업 디렉터리·숨김 창으로 각 2초 실행하고 WM_CLOSE 정상 종료 코드 0을 확인했다. 원본 PNG 2개의 해시가 그대로이며 손상 PNG 테스트 파일도 정리됐다.
- 격리한 Release 실행에서 공용 원본을 제외하고 EXE 옆에는 정상 PNG를 두었다. 원본 경로·WIC API·HRESULT 진단과 종료 코드 1을 확인하여 과거 복사본으로 대체하지 않음을 검증했다. 원본이 없는 EngineRoot로 빌드 검증 타깃도 실행해 준비 안내와 실패를 확인했다. 실제 원본 에셋은 이동·수정하지 않았다. 기본 출력의 원본 상대 경로는 `../../../`, 깊이를 바꾼 OutDir의 초기 MSBuild 속성 평가는 `../../`였다.
- 커밋 전 최종 점검에서 공백·한글이 있는 별도 EngineRoot와 깊이가 다른 OutDir·IntDir로 Release RendererSmoke를 실제 빌드하고 TEMP 작업 디렉터리에서 실행했다. PNG 원본 직접 읽기·손상 파일 정리와 136프레임·8회 리사이즈·D3D12 오류/경고 0을 확인했다. 기본 출력의 PNG 복사본 부재·원본 해시 불변, 문서 로컬 링크 76개·앵커 18개도 재확인했다. 공개 baseline은 여전히 접근 불가였으며 로컬 최신 C++ 규칙과 Git 커밋 메시지 규칙을 참고했다. 이번 추가 점검에서는 코드 변경 없이 기존 검증 결과와 전체 diff를 대조했다.
- 문서 로컬 링크·이전 제안 앵커·UTF-8/LF·diff를 점검했다. 원본 경로와 PNG 복사 타깃만 바꿨으며 WIC 디코딩·GPU 업로드·행렬/조명 계약은 유지한다.

## 프로젝트 설정

빌드·실행·에셋 준비와 조작은 [README](../README.md#빌드와-실행)를 따른다. 세 프로젝트는 x64 Debug·Release를 지원한다.

| 항목 | 설정 |
|---|---|
| 확인한 환경 | Visual Studio Community 2026 18.11.0, MSVC `v145` / `14.51.36231`, Windows SDK `10.0.26100.0` |
| C++ | C++20, Unicode, `/utf-8`, `/W4`, `/permissive-`, `/EHsc`; Debug `/MDd`, Release `/MD` |
| 경고 | Sandbox·RendererSmoke는 `/WX` 끔, WindowSmoke는 `/WX` |
| 출력 | `bin/<프로젝트>/<구성>/`, `obj/<프로젝트>/<구성>/`; Sandbox 디버거 작업 디렉터리 `$(OutDir)` |
| 연결 | Sandbox·RendererSmoke: `d3d12.lib`·`dxgi.lib`·`d3dcompiler.lib`·`windowscodecs.lib`·`ole32.lib`. WindowSmoke는 D3D12를 연결하지 않음 |
| 헤더 | `WIN32_LEAN_AND_MEAN`, `NOMINMAX`; Sandbox는 PCH, Smoke는 PCH 없이 실제 구현을 컴파일 |
| 셰이더 | [CubeShaders.props](../Sandbox/CubeShaders.props)의 FXC·SM5.0·진입점 Main. 구성별 EXE 옆 `Shaders/`에 CSO 생성·EXE 기준 로딩 |
| 파일·Git | [.editorconfig](../.editorconfig): UTF-8 BOM 없음·LF·끝 개행·C++ 탭 너비 4. [.gitattributes](../.gitattributes): 텍스트 LF, Windows 명령 스크립트 CRLF. [.gitignore](../.gitignore): 빌드 출력·개인 캐시·루트 `/Assets/` 제외, `Tests/Assets/` 추적 |

일반 헤더는 PCH 없이 필요한 의존성을 직접 포함한다. Sandbox PCH는 `$(IntDir)$(ProjectName).pch`에 생성하며 Engine 구현에는 사용하지 않는다. Sandbox·RendererSmoke의 [SourcePaths.props](../Build/SourcePaths.props)가 확정된 출력 위치에서 저장소까지의 경로를 `RNIMAN_SOURCE_ROOT`로 전달한다. PNG는 원본에서 직접 읽고 CSO는 EXE 옆 `Shaders/`에서 읽는다. 출력 폴더만 옮기는 독립 실행은 지원하지 않으며 출력 경로 변경 시 다시 빌드한다. 상세 경로는 [프로젝트 구조](project-structure.md#개발-실행의-공용-에셋-경로)와 README에서 관리한다.

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
| [RendererSmoke](../Tests/RendererSmoke.cpp) | PNG·면 UV/sRGB/alpha·Phong·역전치 법선·Transform/카메라 수학、가림·리사이즈·프레임 재사용·정리 | 두 구성 모두 D3D12 FL11_0 하드웨어·Graphics Tools·Cube CSO 필요. Tests/Assets/TextureTest.png를 직접 읽으며 벽돌 원본은 불필요 |
| [WindowSmoke](../Tests/WindowSmoke.cpp) | 키·마우스 소비, 캡처/포커스/모달 초기화, 최소화·리사이즈·메시지·종료·창 수명 | Win32 환경. D3D12·Graphics Tools·CSO 불필요 |
| Sandbox | 실제 화면·장치 입력·창 조작의 통합 흐름 | BaseColor·CSO·하드웨어 필요. Graphics Tools는 Debug에 필수, Release는 요구하지 않음. WARP 대체 없음 |

공통 코드 변경 시 영향받는 테스트와 대표 앱을 빌드·실행한다. 빌드·실행·시각 확인을 구분하고, 미수행 항목·이유와 성능 측정 조건을 기록한다. 문서만 수정하면 내용·링크를 확인한다.

<a name="겹치는-오브젝트의-픽셀-검증-2026-10-05"></a>

### GPU 픽셀 검증

[RendererReadback](../Tests/RendererReadback.h)은 같은 draw를 Present 없이 실행해 flip-discard 이후 내용에 의존하지 않는다. GPU 대기 → COPY_SOURCE 복사·PRESENT 복귀 → GPU 대기 → Map 후 RowPitch를 제외한 RGBA8을 읽는다. 앱은 Present를 유지하며 어댑터는 GPU 완료 후 렌더러보다 먼저 파괴한다. Capture 실패 시 검증을 종료한다.

- `CheckOcclusion`: 빈 화면·앞 단독·뒤 단독·정순·역순을 세 카메라/해상도에서 비교한다. 겹침·색 차이·뒤 가시 영역이 각각 100픽셀 이상이어야 하며 앞 영역은 앞 단독, 나머지는 뒤 단독과 같아야 한다.
- [LightingChecks](../Tests/LightingChecks.cpp)의 `CheckPhongLighting`: Ambient 기준 픽셀에 독립적인 sRGB 기대값을 적용한다. 잘못된 World 법선 변환을 구별하려고 비균일 Scale·회전에 전단을 더한다. 기존 텍스처·가림은 Ambient=1·직접광=0인 동일 파이프라인에서 검사한다.
- [3×3 기준 PNG](../Tests/Assets/TextureTest.png)는 9색·중앙 alpha 128을 직접 작성했다. 여섯 면의 텍셀 중심에서 UV·채널·업로드 행 패딩·sRGB·alpha 255를 비교한다. RGB 허용 오차는 텍스처 1, 조명 2다.

## 이전 문서 링크

<a name="d3d12-화면-출력과-검증"></a>
<a name="진입점과-수명"></a>
<a name="창입력-추출-2026-10-06"></a>
<a name="앱-실행부-추출-2026-10-06"></a>
<a name="프레임별-fence-동기화"></a>
<a name="좌표축과-행렬"></a>
<a name="메시행렬렌더-데이터"></a>

실행 구조·수명·좌표계·렌더 데이터 계약은 [Architecture](design.md#sandbox-실행-구조)로 옮겼다.

<a name="최신-검증-결과"></a>
<a name="2026-09-17-프레임-동기화-검증"></a>

검증 결과는 [마일스톤 목록](milestones/README.md)에서 확인한다. 기존 링크의 2026-09-17 개별 기록은 현재 문서에 남아 있지 않으며, v0.1.0의 후속 검증을 해당 날짜의 결과로 간주하지 않는다.
