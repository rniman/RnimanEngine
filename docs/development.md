# 개발 설정과 검증

[프로젝트 소개와 실행](../README.md) · [Architecture](design.md) · [프로젝트 구조](project-structure.md) · [마일스톤 기록](milestones/README.md)

현재 컨벤션 적용 상태와 개발 설정·검증 방법을 관리한다. 기능 계획은 [로드맵](roadmap.md), 달성 결과와 당시 제약은 마일스톤 문서에서 관리한다.

## 컨벤션

공통 기준은 위키의 현재 [채택 규칙](https://rniman.github.io/convention/baseline/)과 [적용·근거 검토 절차](https://rniman.github.io/convention/baseline/#위키의-적용과-프로젝트-근거-검토)다. 작업 전 필수·권장·선택과 채택·초안을 구분해 확인한다. 확인 커밋은 고정 기준이 아니다.

변경 범위의 실제 적용·선택 이유·예외·검증과 미적용 대상의 이유·후속 조건을 이 문서에 기록한다. 확인만 한 내용과 실제 적용을 구분하며, 위키 문서 갱신과 반영 상태 관리는 위키 정비 담당이 맡는다.

### 확인 범위

- 2026-10-20: 공개 baseline·채택 문서 14개·관련 초안 6개·프로젝트 참조 문서를 확인하고 코드·프로젝트 설정과 대조했다. 당시 위키 원본은 `4beaa82`이며, 후속 코드 적용의 근거는 아래에 요약한다.
- 2026-10-21: 공개 위키의 웹 조회와 HTTP 조회가 실패해 최신 공개본 일치는 확인하지 못했다. 임시로 로컬 `D:/Docs/Convention/docs/`의 baseline·채택된 프로젝트 구조·세부 규칙 미작성 상태인 문서화 목차를 참고했다. 공개본은 접근 복구 후 재확인한다.
- 2026-10-22: 공개 baseline의 웹·HTTP 조회가 다시 실패했다(DNS/접근 오류). 최신 공개본과의 일치는 미확인이다. 임시로 로컬 `D:/Docs/Convention/docs/`의 baseline·명명·소유권·프로젝트 구조를 재확인하고 기존 프로젝트 적용 상태를 유지했다. 아래 ImGui 적용은 이 범위에서의 시험 적용이며 위키는 수정하지 않았다.
- 2026-10-23: 공개 baseline·포맷 페이지의 웹 조회는 접근 오류로 실패했다. 최신 공개본과의 일치는 미확인이며, 임시로 로컬 `D:/Docs/Convention/docs/baseline.md`와 `coding/cpp/formatting.md`의 현재 채택 범위를 확인했다. 여러 줄 인자와 작업별 실패 처리의 기존 필수 규칙을 이번 ImGui 자체 코드에 재적용했다.

### 적용 상태

2026-10-23 ImGui 학습 메모 작성에서는 공개 baseline을 다시 조회했으나 접근 오류로 최신 공개본을 확인하지 못했다.
임시로 로컬 `D:/Docs/Convention/docs/baseline.md`와 채택된 포맷·명명 규칙을 참고해 메모의 자체 C++ 예제에 적용했다.
일반 문서화 세부 규칙은 미작성 범위이므로 추측해 추가하지 않았다. 공개본은 접근 복구 후 재확인한다.
새 규칙·런타임 코드 변경 없이 `docs/memo/ImGui.md`를 추가하고 README·솔루션 탐색 경로를 연결했다.
메모의 API를 포함된 v1.93 헤더·실제 연동 코드와 대조하고 로컬 링크·솔루션 XML·UTF-8/LF·코드 블록·공백을 확인했다.
공식 Getting Started·FAQ·폰트 문서와 DX12 예제 원문을 조회했다. 문서 변경만 수행했으므로 예제 컴파일·빌드·실행·시각 검증은 재수행하지 않았다.

2026-10-25 사용자 지정에 따라 메모를 `docs/memo/`로 옮기고 상대 링크·README·솔루션 항목을 맞췄다.
공개 baseline 재조회는 접근 오류로 실패해 로컬 `D:/Docs/Convention/docs/baseline.md`의 채택 범위를 참고했다.
새 규칙은 추가하지 않았으며 문서 링크·솔루션 XML·공백을 확인했다. 경로 변경만 수행해 빌드·실행은 생략했다.

2026-10-26 메모를 Dear ImGui 자체의 개념·함수·사용 패턴 중심으로 줄이고 프로젝트 내용은 말미의 참고로 모았다.
1절에는 ImGui와 앱의 역할·백엔드와 D3D12의 관계·프레임 간 상태 유지·패널 예제의 데이터 흐름을 보강했다.
공개 baseline은 접근 오류로 확인하지 못해 로컬 `D:/Docs/Convention/docs/baseline.md`·채택된 포맷 규칙을 참고했다.
새 규칙·런타임 변경은 없으며 v1.93 API·문서 링크·코드 블록·공백을 확인했다. 문서 편집으로 예제 컴파일·빌드·실행은 수행하지 않았다.

같은 날 사용자 지정에 따라 솔루션의 `Docs/memo` 폴더·파일 항목을 제외했다. 실제 `docs/memo/` 파일과 README 링크는 유지한다.
공개 baseline 재조회는 접근 오류로 실패해 로컬의 채택된 프로젝트 구조·솔루션 표시 기준을 참고했다.
솔루션 XML·등록 경로·메모 원본 유지·공백을 확인했으며 표시 항목 변경이므로 빌드·실행은 생략했다.

| 항목 | 실제 적용·예외와 선택 이유 |
|---|---|
| 명명·포맷·헤더 | 기존 명명·초기화·const·직접 include·한국어 API 주석·반환 계약·RAII를 유지한다. Engine·Sandbox·Tests의 직접 작성 C++는 제품 소속과 이름 충돌 방지를 위해 `rengine` 단일 루트를 사용한다. main·wWinMain은 전역, 외부 API·HLSL은 원래 이름을 유지한다. |
| 타입 명시 | 기존 적용 범위의 실패 결과·DirectX 데이터·경로·창 상태·D3D12 descriptor에 타입을 명시한다. WindowSmoke의 상태 참조·입력 복사본·optional 종료 코드, LightingChecks의 FrameLighting·std::size_t 픽셀 index도 포함한다. 범위 반복·기존 요소 참조·캐스트로 타입이 드러나는 포인터·chrono 시각·람다의 기존 auto 사용은 유지한다. |
| 반환값 진단 | [반환값 규칙](https://rniman.github.io/convention/coding/cpp/functions/#반환값-무시와-nodiscard)에 따라 실패 후 검증을 중단해야 하는 RendererReadback::Capture에 `[[nodiscard]]`를 적용했다. 상태·자원을 변경하지 않는 행렬 계산인 Transform::GetWorld·SandboxCamera::GetView에는 적용하지 않는다. |
| 소스 소속·의존성 | 공통 원본은 Engine, 데모 실행·조작은 Sandbox에 둔다. 네 실행 프로젝트가 필요한 원본을 직접 빌드하며, 별도 라이브러리·공개 헤더 계층은 실제 재사용 수요가 생길 때 검토한다. [include 기준과 허용 의존성](project-structure.md#현재-include-기준과-허용-의존성)을 따른다. |
| 프로젝트 한정 채택 | 창·렌더러 소유 분리, 하드웨어 우선·WARP 자동 대체 없음, 프레임 경계 리사이즈·슬롯 재사용 전 Fence 대기를 유지한다. 공용 PNG는 복사 없이 원본에서 읽고 경로 계산은 Build/SourcePaths.props가 맡는다. [실행 계약](design.md#sandbox-실행-구조)과 [에셋 경로](project-structure.md#개발-실행의-공용-에셋-경로)를 따른다. |
| 시험 적용 | WIC·STA·RGBA8, sRGB·단일 밉·선형 Clamp, ComPtr·Event 수명·bool/HRESULT 진단, Debug 레이어·두 백 버퍼·VSync, FXC·SM5.0·D32·Transform·카메라·Phong·테스트 readback. 유지·변경 조건은 [구현 선택과 후속 조건](design.md#구현-선택과-후속-조건)을 따른다. |
| 예외·미정 | Sandbox 전용 구현은 필요 시 pch.h를 먼저 포함하고 Engine은 PCH를 사용하지 않는다. Windows→DirectX include 순서·긴 오류 문자열 예외와 Sandbox·Tests의 평면 배치를 유지한다. 로컬 정면·거리 단위·외부 에셋 변환은 도입 시 결정하고, 비정방 행렬·행렬 배열은 별도 패킹 검증이 필요하다. |
| ImGui 시험 적용 | SandboxGui 단일 문맥·전용 64칸 SRV 힙·GPU 대기 후 종료, Win32 메시지 콜백·선택 OverlayPass를 실제 Sandbox·ImGuiSmoke에 적용했다. 장면/Engine의 ImGui 의존성과 범용 UI/descriptor 관리자는 추가하지 않았다. 외부 소스는 원본·공급 구조·MIT를 유지하고 해당 두 타깃에서만 PCH 없이 `/W3`·`/WX` 해제로 컴파일한다. 자체 C++는 기존 `rengine`·직접 include·RAII를 적용한다. 사용자 텍스처/여러 도구의 공유 수요가 생기면 descriptor 할당을 공통화할 후보로 재검토한다. |
| 공통 초안 | 자료형·변환·attribute 전체, 클래스 미결정 정책, HLSL·검증·원본/생성물 규칙은 공통 채택으로 확대하지 않는다. 포맷터·경고 오류화·캐시는 필요와 검증 조건이 생길 때 검토한다. |

### 마우스 캡처 충돌 보정 (2026-10-26)

- 실제 ImGui Win32 백엔드에서 오른쪽 누름 → 왼쪽 누름 → 오른쪽만 해제하면, 백엔드가 유지한 HWND 캡처를 창의 카메라 처리가 해제하는 문제를 확인했다. 기존 Smoke는 단순 메시지 관찰 콜백을 사용해 이 충돌을 검출하지 못했다.
- Win32Window에 캡처 획득 주체를 구분하는 상태를 추가했다. 창이 직접 획득한 캡처만 오른쪽 버튼 해제 시 반환하고, 백엔드가 먼저 획득한 캡처는 빌려 쓴다. UI로 입력을 넘기거나 캡처를 잃으면 앱 소유 표시를 비운다. UI 없는 창의 오른쪽 드래그·포커스/모달/최소화 정리 계약은 유지한다. 범용 입력 관리자를 추가하지 않고 기존 창 내부에서 책임을 구분한 선택이다.
- ImGuiSmoke에 실제 SandboxGui 연결 상태의 오른쪽 단독·오른쪽과 왼쪽/가운데/X1/X2 조합을 추가했다. 오른쪽 해제 후 캡처 유지·카메라 회전 종료·마지막 버튼 해제 후 캡처 반환을 확인한다. WindowSmoke는 이미 획득된 캡처 대여와 UI 없는 창의 직접 획득·반환을 구분해 검사한다. 수정 전 새 회귀 검사는 `kept=0`으로 실패했다.
- 공개 baseline은 접근 오류로 최신 공개본을 확인하지 못해 로컬 `D:/Docs/Convention/docs/`의 채택된 소유권·포맷 규칙을 참고했다. 새 공통 규칙이나 공급자 원본 변경은 없으며 위키는 수정하지 않았다.
- 수정 후 x64 Debug·Release 솔루션 빌드·세 Smoke가 통과했다. 실제 백엔드의 다섯 캡처 조합과 창의 대여/직접 획득 검사가 통과했고 D3D12 오류/경고는 0이다. 현재 Sandbox 오브젝트로 임시 하네스를 다시 연결해 두 구성의 TEMP 디렉터리 숨김 창 실행·정상 종료(코드 0)를 확인했다. 변경 헤더의 PCH 없는 `/Zs /W4 /WX` 독립 컴파일·자체 코드 UTF-8/LF·문서 경로·공백도 확인했다. 물리 장치로 창 밖까지 드래그하는 수동 화면 검증은 수행하지 않았으며 자동 메시지·캡처 상태 검사와 구분한다.
- 후속 컨벤션 검토는 Win32Window.cpp/.h·WindowSmoke.cpp·ImGuiSmoke.cpp의 캡처 수정·회귀 검사와 관련 선언을 대상으로 했다. 공개 위키 접근 실패로 로컬 채택본의 명명·포맷·초기화·auto/const·헤더·주석·함수·클래스·소유권을 대조했다. 필수 규칙 누락은 발견하지 못했고, 같은 파일의 기존 WindowSmoke 호출 한 줄만 120열 권장에 맞춰 인자별 줄바꿈했다. 네 파일의 UTF-8/BOM 없음·LF·탭·공백·120열, 창 헤더 독립 컴파일과 두 구성의 WindowSmoke 빌드·실행을 통과했다. 제품 동작 변경은 없어 다른 Smoke·Sandbox 실행·시각 검증은 반복하지 않았다. 공급자 원본과 미결정 규칙은 수정 대상에서 제외했다.

### ImGui 프로젝트 항목 경고 수정 (2026-10-25)

- Visual Studio 솔루션 로드 시 Build/ImGui.props의 ClCompile·ClInclude·None에 와일드카드 경고가 발생했다. 실제 `*.h`뿐 아니라 `;`로 묶은 파일 목록도 IDE에서 지원되지 않는다. 앞선 MSBuild 평가·필터 대응 검사만으로 IDE 호환성을 확인할 수 없었던 부분을 정정한다. [Microsoft의 VC 프로젝트 항목 제약](https://learn.microsoft.com/en-us/cpp/build/reference/vcxproj-files-and-wildcards?view=msvc-170)을 근거로 공급 파일 18개를 각각 등록했다.
- 소스 7개의 PCH 비사용·`/W3`·`/WX` 해제와 두 소비자 한정 import를 유지한다. 공급자 원본·필터의 파일 소속은 바꾸지 않았으며 향후 공급 파일 변경 시 props·양쪽 필터를 함께 관리한다. 공통 위키 규칙의 개정이 아닌 기존 소스 배치·필터 기준의 적용 보정이다.
- 공개 baseline 조회는 접근 오류로 실패해 최신 공개본 일치는 미확인이다. 임시로 로컬 `D:/Docs/Convention/docs/baseline.md`와 채택된 `coding/cpp/project-structure.md`의 써드파티·IDE·프로젝트 입력 기준을 확인했으며 위키는 수정하지 않았다.
- 별도의 숨김 Visual Studio 인스턴스에서 솔루션 로드·DTE 프로젝트 항목·출력 창을 확인했다. 두 소비자 모두 해당 경고가 없고 ImGui 루트 항목 15개(backend 필터 포함)·backend 파일 4개가 등록되어 있다. 이는 IDE API를 통한 확인이며 화면의 수동 시각 검증과 구분한다. MSBuild로 두 구성의 파일 18개·입력 중복 없음·필터 대응·외부 컴파일 설정도 확인했다.
- x64 Debug·Release 솔루션 빌드·ImGuiSmoke 실행 통과. UI readback은 다섯 크기에서 각각 48,000픽셀, D3D12 오류/경고 0이다. 현재 Sandbox 오브젝트로 임시 하네스를 다시 연결해 두 구성의 TEMP 디렉터리 숨김 창 실행·정상 종료(코드 0)를 확인했다. XML·문서 경로·UTF-8/LF·원본 16개 해시·공백 검사도 통과했다. 런타임 구현을 바꾸지 않아 WindowSmoke·RendererSmoke 실행과 수동 화면 조작은 반복하지 않았다.

### ImGui 전체 컨벤션 재검토 (2026-10-23)

대상은 ImGui 도입의 자체 C++ 13개 파일(기존 파일은 변경 구간과 관련 호출·수명 경계), 빌드·프로젝트·필터 6개 파일과 연결 문서다. 공급자 원본 16개는 스타일 적용에서 제외하고 해시·라이선스·공급 구조만 확인했다. 작업 전 변경을 보존했으며 무관한 기존 코드를 일괄 수정하지 않았다.

공개 baseline 조회 실패로 최신 공개본과의 일치는 미확인이다. 임시로 `D:/Docs/Convention/docs/`의 baseline·채택된 C++ 11개 문서·좌표축/행렬 문서를 참고했다. 추가 정리에서는 포맷·클래스 책임·주석을 재확인했다. 초안·미결정 항목을 필수 규칙으로 확대하지 않았고 브랜치·커밋 생성이나 위키 수정은 수행하지 않았다.

| 구분 | 실제 수정·확인과 이유 |
|---|---|
| 헤더·의존성 (수정) | SandboxGui의 외부 타입 직접 전방 선언을 공급자 정의 헤더로 교체하고 자체 코드의 직접 include·그룹 정렬을 보완했다. Windows 선행 예외는 주석으로 남겼다. |
| API·수명 (수정) | 초기화·Begin/End/Record 순서, 콜백의 비소유 수명·null 조건, 빈 개체 목록의 UI 그리기와 조명 단위·범위를 선언에 명시했다. 문맥 해제·GPU 대기 실패 후 정리와 콜백 주소 유지를 위한 복사·이동 금지를 보완했다. |
| 포맷·작업 경계 (수정) | 여러 줄 인자를 첫 인자부터 새 줄·탭 한 단계·인자별 한 줄로 배치하고 초기화·실패 검사를 작업별로 분리했다. 컨테이너 초기화·멤버 역할 구분·120열도 보정했다. 앞선 120열 검사만으로 포맷을 충분히 확인했다고 판단한 부분을 정정한다. |
| 테스트·빌드 (수정·확인) | ImGuiSmoke는 프레임당 Record를 한 번만 수행하고 진단 API 실패에 위치·HRESULT를 표시한다. 양 소비자의 ThirdParty 필터·메타데이터 표시를 import와 맞췄다. MSBuild 평가로 입력 중복 없음·필터 대응·외부 cpp 7개의 Sandbox/ImGuiSmoke 한정 컴파일을 확인했다. |
| 중복·가독성 (추가 정리) | 조명 키 입력·UI의 보정을 ApplyParameters로 모으고 PITCH_LIMIT·MAX_INTENSITY를 패널과 공유했다. DrawTools의 개체·조명 편집은 별도 함수로 나누고 벡터 입력 변환은 파일 내부 EditVector3로 옮겼다. 불필요한 changed 초기 대입·오류 문자열의 용량 중복을 없애고 배열 복사·편집 검증·Begin/End의 이유를 주석에 남겼다. 같은 날짜의 포맷·전체 점검 기록은 이 절로 통합했다. |
| 기존 규칙 (확인) | 명명·auto·const·네임스페이스·좌표/행렬·소유권을 대조했다. 자체 Transform/HLSL·파싱/GPU 업로드 경계는 유지하며 공급자 셰이더에 자체 row_major 규칙을 강제하지 않는다. |

전체 점검에서는 헤더 7개를 PCH·강제 include 없이 `/Zs /W4 /WX` 독립 컴파일했다. C++ 13개 파일의 UTF-8/BOM 없음·LF·끝 개행·공백, 새 자체 코드의 120열·프로젝트 XML/평가 입력·문서 경로·원본 해시·`git diff --check`도 확인했다.

추가 정리 후 x64 Debug·Release 솔루션 빌드·세 Smoke를 재실행해 통과했다. RendererSmoke의 D3D12 오류/경고·잘못된 가림 픽셀 0, ImGuiSmoke는 다섯 크기에서 각 48,000 UI 픽셀·D3D12 오류/경고 0이다. 변경한 App·Lighting 헤더의 독립 컴파일과 자체 코드 포맷·문서 링크·원본 해시도 재확인했다. 새 테스트 타깃은 추가하지 않았다.

실제 Sandbox의 두 구성은 임시 하네스(`obj/ConventionAudit/2026-10-23/`, Git 제외)로 TEMP 디렉터리에서 `Run(SW_HIDE)` → WM_CLOSE → 반환·소멸까지 종료 코드 0을 확인했다. 숨김 창 실행과 MSBuild 필터 대응 검사는 수동 화면·장치 입력·Visual Studio GUI 확인을 대신하지 않는다.

남은 예외: 공급자 지침에 따른 Win32 메시지 함수 선언은 유지한다. descriptor 콜백의 `std::terminate`는 스택 해제를 보장하지 않아 정상·조기 반환의 RAII와 구분한다. [실패 전달 한계](dependencies.md#dear-imgui)는 메모리 부족·장치 제거 주입으로 검증하지 않았다. 복구를 지원하려면 공급자 백엔드의 실패 전파·부분 자원 정리를 함께 보완해야 한다. 사용자 텍스처·다중 도구 수요가 생기면 descriptor 공유와 이 정책을 함께 재검토한다.

### ImGui 최소 도입 (2026-10-22)

- x64 Debug·Release 솔루션 빌드, WindowSmoke·RendererSmoke·ImGuiSmoke 실행 통과. RendererSmoke의 D3D12 오류/경고·잘못된 가림 픽셀 0, ImGuiSmoke의 D3D12 오류/경고 0이다.
- 변경한 창·렌더러·SandboxGui·카메라·조명·앱 헤더 여섯 개를 PCH·강제 include 없이 `/Zs /W4 /WX` 독립 컴파일했다. 프로젝트/필터/솔루션 XML, 로컬 문서 경로·원본 16개 해시·`git diff --check`도 통과했다. Visual Studio GUI에서의 필터 탐색은 별도로 확인하지 않았다.
- WindowSmoke에 UI 캡처 시작 시 누적 키/드래그 제거, 새 앱 입력 차단, 같은 HWND의 UI 캡처 유지, 수명 메시지 전달·콜백 연결 해제 검증을 추가했다. 실제 장치 입력의 모든 경계 조합을 검증한 것은 아니다.
- ImGuiSmoke는 60회 반복 프레임, 다섯 크기 전환의 UI GPU readback, 폰트 크기 변경, 포커스/최소화 메시지·마지막 제출 후 UI 소멸을 검사한다. 카메라·조명 편집의 비유한 값 거부·부분 변경 없음·범위 제한도 확인한다. 각 readback에서 배경과 다른 UI 픽셀 48,000개를 확인했다. GPU 시간·성능 향상을 측정한 결과가 아니다.
- 실제 Sandbox의 Debug Demo Window와 Release 편집 패널을 화면에서 확인했다. TEMP 작업 디렉터리에서 시작했다. 데스크톱 입력 변경으로 수동 회전 편집·재개·텍스트 편집 중 카메라 차단·창 최소화/복원·정상 종료의 전체 조작 검증은 확정하지 못했다. 자동 테스트의 자원 정리·입력 계약 결과와 구분한다.
- 최초 Developer PowerShell 미초기화 상태의 링크는 MSPDB140.DLL 오류로 실패했다. Visual Studio DevShell 초기화 후 정상 빌드했다. 검증 프로세스가 EXE를 점유한 링크 실패는 해당 검증 프로세스 종료 후 해소했다.
- 단일 GPU/Win32 창·내장 영문 폰트·고정 용량 힙·동일 UI 스레드로 시험 적용했다. 한글 폰트/IME·도킹·다중 뷰포트·레이아웃 저장·메모리 부족/장치 손실 복구는 미적용이며 해당 기능 수요와 실패 전달 검토 후 확장한다. 공급자 소스에 공통 스타일·PCH를 강제하지 않는 예외와 실패 전달 한계는 [의존성](dependencies.md#dear-imgui)에 기록했다.

### 기존 적용의 검증 근거

아래는 기존 작업에서 확인한 결과다. 문서 정리 과정에서 빌드·실행을 다시 수행한 결과는 아니다.

| 적용 | 검증 결과와 한계 |
|---|---|
| 공용 원본 경로 (2026-10-16) | x64 Debug·Release 빌드·두 Smoke·Sandbox 실행 성공. TEMP 작업 디렉터리에서 로딩을 확인하고 Release는 공백·한글 경로와 변경한 출력 깊이에서도 검증했다. 원본 누락 시 빌드·런타임이 실패하고 EXE 옆의 과거 PNG로 대체하지 않음을 확인했다. |
| 컨벤션·소스 소속 정리 (2026-10-20) | 두 구성의 솔루션 빌드·두 Smoke 통과. RendererSmoke의 D3D12 오류/경고·잘못된 가림 픽셀 0. Sandbox는 실제 BaseColor·TEMP 작업 디렉터리·숨김 창으로 실행하고 정상 종료했다. 화면의 시각적 확인과는 구분한다. |

2026-10-20에는 변경한 헤더를 PCH·강제 include 없이 독립 컴파일했다. Capture 결과 무시는 C4834와 `/WX` 실패로 진단됐고 GetWorld·GetView 결과 무시는 통과했다. 소속 분리 후 Engine 원본의 독립 컴파일·소비자 include·솔루션 로드도 확인했다. 행렬·셰이더 패킹 대조는 [2026-10-13 마일스톤 검증](milestones/v0.1.0-sandbox.md)을 유지한다.

## 프로젝트 설정

개발 환경·빌드·실행·에셋 준비는 [README](../README.md#빌드와-실행)를 따른다. 네 프로젝트는 x64 Debug·Release를 지원한다.

| 항목 | 설정 |
|---|---|
| C++ | C++20, Unicode, `/utf-8`, `/W4`, `/permissive-`, `/EHsc`; Debug `/MDd`, Release `/MD` |
| 경고 | Sandbox·RendererSmoke·ImGuiSmoke는 `/WX` 끔, WindowSmoke는 `/WX` |
| 출력 | [Directory.Build.props](../Directory.Build.props): `bin/<프로젝트>/<구성>/`, `obj/<프로젝트>/<구성>/`. Sandbox 디버거 작업 디렉터리는 `$(OutDir)` |
| 연결 | Sandbox·RendererSmoke·ImGuiSmoke: d3d12·dxgi·d3dcompiler·windowscodecs·ole32. WindowSmoke는 D3D12를 연결하지 않음 |
| 헤더·PCH | `WIN32_LEAN_AND_MEAN`, `NOMINMAX`. Sandbox PCH는 `$(IntDir)$(ProjectName).pch`; Engine·SandboxGui·Smoke·ImGui 원본은 PCH 비사용. 일반 헤더는 필요한 의존성을 직접 포함 |
| 셰이더 | [CubeShaders.props](../Sandbox/CubeShaders.props): FXC·SM5.0·진입점 Main, EXE 옆 `Shaders/`에 CSO 생성·로딩 |
| 파일·Git | [.editorconfig](../.editorconfig): UTF-8 BOM 없음·LF·끝 개행·C++ 탭 너비 4. [.gitattributes](../.gitattributes): 텍스트 LF·Windows 스크립트 CRLF. [.gitignore](../.gitignore): 출력·개인 캐시·루트 Assets 제외, Tests/Assets 추적 |

## 검증 방법

공통 코드·API·경로를 바꾸면 Sandbox와 영향받는 Smoke를 x64 Debug·Release로 빌드·실행한다. 헤더는 PCH·강제 include 없이 독립 컴파일하고, 파일 재배치 시 프로젝트 입력·include·필터·솔루션 표시를 확인한다. 행렬·셰이더 경계 변경 시 패킹과 GPU 픽셀 검증을 다시 수행한다.

빌드·실행·시각 확인을 구분해 결과와 미수행 이유를 기록한다. 성능은 측정한 범위만 보고하며, 문서만 수정하면 내용·링크를 확인한다.

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
MSBuild.exe Tests/ImGuiSmoke.vcxproj -nologo -t:Build -p:Configuration=Debug -p:Platform=x64
./bin/ImGuiSmoke/Debug/ImGuiSmoke.exe
MSBuild.exe Tests/ImGuiSmoke.vcxproj -nologo -t:Build -p:Configuration=Release -p:Platform=x64
./bin/ImGuiSmoke/Release/ImGuiSmoke.exe
```

| 대상 | 검사 범위 | 실행 조건 |
|---|---|---|
| [RendererSmoke](../Tests/RendererSmoke.cpp) | PNG·UV/sRGB/alpha·Phong·법선·Transform/카메라 수학, 가림·리사이즈·동기화·정리 | 두 구성 모두 D3D12 FL11_0 하드웨어·Graphics Tools·Cube CSO 필요. 저장소의 Tests/Assets/TextureTest.png 사용, 벽돌 에셋 불필요 |
| [WindowSmoke](../Tests/WindowSmoke.cpp) | 키·마우스 소비, 캡처/포커스/모달 초기화, 최소화·리사이즈·종료·창 수명 | Win32 환경. D3D12·Graphics Tools·CSO 불필요 |
| [ImGuiSmoke](../Tests/ImGuiSmoke.cpp) | ImGui 초기화·UI 픽셀·폰트 크기 변경·리사이즈·소멸·D3D12 진단 | 두 구성 모두 FL11_0 하드웨어·Graphics Tools·Cube CSO 필요. 외부 PNG 불필요 |
| Sandbox | 실제 화면·장치 입력·창 조작의 통합 흐름 | BaseColor·CSO·D3D12 하드웨어 필요. Graphics Tools는 Debug에 필수. WARP 대체 없음 |

### GPU 픽셀 검증

[RendererReadback](../Tests/RendererReadback.h)은 Present 없이 그린 뒤 GPU 대기·복사·Map을 거쳐 RowPitch 패딩을 제외한 RGBA8을 읽는다. flip-discard 이후 내용에 의존하지 않으며 Capture 실패 시 검증을 중단한다. 어댑터는 GPU 완료 후 렌더러보다 먼저 파괴한다.

- `CheckOcclusion`: 세 카메라/해상도에서 앞·뒤 단독 및 그리기 순서를 비교한다. 겹침·색 차이·뒤 가시 영역이 각각 100픽셀 이상이고, 결과가 깊이와 일치해야 한다.
- [CheckPhongLighting](../Tests/LightingChecks.cpp): 독립적인 sRGB 기대값과 비균일 Scale·회전·전단으로 조명과 역전치 법선을 검증한다. 텍스처·가림 검사는 Ambient=1·직접광=0을 사용한다.
- [3×3 기준 PNG](../Tests/Assets/TextureTest.png): 여섯 면의 UV·채널·행 패딩·sRGB·출력 alpha를 비교한다. RGB 허용 오차는 텍스처 1, 조명 2다.
