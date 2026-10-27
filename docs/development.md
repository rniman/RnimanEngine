# 개발 설정과 검증

[프로젝트 소개와 실행](../README.md) · [Architecture](design.md) · [프로젝트 구조](project-structure.md) · [마일스톤 기록](milestones/README.md)

현재 컨벤션 적용 상태와 개발 설정·검증 방법을 관리한다. 기능 계획은 [로드맵](roadmap.md), 달성 결과와 당시 제약은 마일스톤 문서에서 관리한다.

## 컨벤션

공통 기준은 위키의 현재 [채택 규칙](https://rniman.github.io/convention/baseline/)과 [적용·근거 검토 절차](https://rniman.github.io/convention/baseline/#위키의-적용과-프로젝트-근거-검토)다. 작업 전 필수·권장·선택과 채택·초안을 구분해 확인한다. 확인 커밋은 고정 기준이 아니다.

변경 범위의 실제 적용·선택 이유·예외·검증과 미적용 대상의 이유·후속 조건을 이 문서에 기록한다. 확인만 한 내용과 실제 적용을 구분하며, 위키 문서 갱신과 반영 상태 관리는 위키 정비 담당이 맡는다.

### 확인 범위

2026-10-20에 공개 baseline·채택 문서 14개·관련 초안 6개·프로젝트 참조 문서를 코드·설정과 대조했다(당시 위키 원본 `4beaa82`). 10월 21~27일에는 공개본 접근 오류로 최신본 일치를 확인하지 못해 로컬 `D:/Docs/Convention/docs/`의 baseline·관련 채택 규칙을 임시 참고했다. 아래 적용은 이 확인 범위에 한정하며 공개본은 접근 복구 후 재확인한다. 초안·미결정 사항은 공통 채택으로 확대하지 않는다.

### 적용 상태

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

### 코드 검토 후 개선 (2026-10-27)

대상은 D3D12Renderer·Win32Window·SandboxApp·SandboxCamera다. 기존 채택 규칙을 적용하고 중복과 작업 경계를 정리했다.

| 대상 | 실제 변경·선택 이유와 유지한 계약 |
|---|---|
| Constant Buffer | 초기 슬롯별 256바이트 선행 할당을 제거하고 첫 사용 시 필요한 크기로 생성한다. 큐브 3개는 1,024바이트, 빈 목록은 조명용 256바이트다. Fence 완료 후 생성·확장·복사하며 축소하지 않는다. 최초 할당 실패는 Render에서 보고한다. |
| 렌더러 함수 분리 | Initialize의 Debug 레이어·Factory·장치 선택을 CreateDevice로, RenderInternal의 버퍼 생성·확장·복사를 UploadFrameConstants로 분리했다. 입력 검증·Fence 대기는 호출부에 유지하며 업로드와 draw는 CONSTANT_STRIDE를 공유한다. 실패 전달·소유권·제출 순서를 유지한다. |
| 큐브 설정 | Cube에 Transform·회전 축·속도·자동 회전 상태를 묶고 INITIAL_OBJECTS에서 실제 큐브·렌더링 배열 크기를 유도한다. 선택 UI도 목록을 순회한다. 축은 XMFLOAT3 멤버 포인터로 지정해 복사 후 다른 개체를 참조하지 않는다. 고정·비어 있지 않은 데모 목록에 한정하며 런타임 추가/삭제는 미지원이다. |
| 마우스 초기화 | SetInputCapture·ClearInput·WM_CAPTURECHANGED의 회전·캡처 소유 표시·이동량 초기화를 ClearMouseInput으로 모았다. OS 캡처 해제·키·시간 초기화는 호출부에 유지한다. WM_RBUTTONUP은 이동량을 직접 지우지 않는 기존 동작을 보존한다. |
| 주석·읽기 전용 값 | 렌더러 소멸자에 초기화 중 업로드 제출 후 실패도 대기 대상이며 대기 실패 시 정리를 계속함을 명시했다. 카메라 API에 유한한 입력·호출자 검증 책임을 기록하고 Move의 0 이상 시간 전제와 앱의 0~0.1초 제한을 구분했다. Signal 이후 검사할 Present 결과는 const HRESULT로 보존한다. 런타임 입력 검증이나 지역 변수 전체의 const 적용은 추가하지 않았다. |

기존 세 큐브의 배치·Scale·Y/X/Z 회전 축·0.7/0.35/−0.7 rad/s 속도·편집 시 정지/재개 계산과 카메라의 1초 이동 테스트를 코드 대조했다. 헬퍼의 사전 조건·실패 후 제출 금지를 선언에 기록했다. 후속 검토에서는 UploadFrameConstants가 읽는 입력·변경하는 버퍼와 생성·확장 후 Map 실패 시 버퍼가 이미 교체됐을 수 있다는 계약을 보완했다. 범용 관리자·ECS·애니메이션 시스템은 추가하지 않았다. 실행 흐름은 [설계 문서](design.md#프레임별-fence-동기화)를 따른다.

빌드·자동 검증은 각 변경 직후 x64 Debug·Release에서 수행했다. 아래는 그 결과와 사용자가 보고한 수동 검증을 통합한 기록이며 모든 테스트를 매 단계 반복한 것은 아니다.

| 검증 | 수행 범위·결과 |
|---|---|
| 빌드·헤더 | 솔루션 빌드 및 네 변경 헤더의 PCH 없는 `/Zs /W4 /WX` 독립 컴파일 통과. |
| RendererSmoke | 렌더러 변경·최종 const 보완 후 통과. 136프레임·8회 리사이즈, 빈 목록·0→1→3→9개 확장/재사용·GPU 픽셀·종료를 검사했으며 D3D12 오류/경고는 0이다. |
| WindowSmoke | 마우스 초기화 변경 후 통과. 입력 소비·캡처/포커스/모달 초기화·창 수명을 검사했다. 이후 카메라 주석·const 변경에서는 반복하지 않았다. |
| ImGuiSmoke | 각 기능 변경 후 통과. 60프레임·5회 UI readback과 실제 백엔드의 오른쪽 단독/다른 버튼 조합 캡처·종료를 검사했으며 D3D12 오류/경고는 0이다. |
| Sandbox | 실제 에셋과 별도 작업 디렉터리에서 숨김 창 실행·WM_CLOSE·정상 종료(코드 0)를 확인했다. 저장소에 포함되지 않은 임시 검증 프로그램을 사용했으며 수동 화면 검증과 구분한다. |
| 정적 확인 | 변경 범위의 인코딩·공백과 `git diff --check`를 확인했다. 기존 회귀 검사를 활용해 새 테스트는 추가하지 않았다. 주석만 보완한 단계에서는 빌드·실행을 반복하지 않았다. |
| 사용자 수동 검증 — 큐브 | 사용자가 Sandbox에서 선택한 큐브만 변경되고, 회전 편집 시 해당 큐브만 정지하며, Auto rotate 재개 시 수정된 각도에서 회전함을 확인했다. |
| 사용자 수동 검증 — 마우스 | 사용자가 물리 마우스로 창 밖에서도 드래그가 유지되고, 버튼 해제 후 카메라 회전이 중단되며, Alt+Tab 후 입력이 초기화됨을 확인했다. |

사용자 수동 검증의 실행 구성은 별도로 보고되지 않았으며 에이전트의 자동 검증과 구분한다. ImGuiSmoke와 숨김 창 실행이 수동 검증을 대신한 것은 아니다. 장치 제거·메모리 부족 주입과 성능 측정은 미수행이다.

### 마우스 캡처 충돌 보정 (2026-10-26)

실제 ImGui 백엔드에서 오른쪽·왼쪽 버튼을 누른 뒤 오른쪽만 해제하면 창이 UI 소유 캡처까지 해제하는 문제를 수정했다. Win32Window는 직접 획득한 캡처만 일반 버튼 해제 시 반환하고, UI가 먼저 획득한 캡처는 빌려 쓴다. 포커스 해제·모달·최소화의 입력 정리 계약은 유지한다.

ImGuiSmoke에 오른쪽 단독 및 왼쪽/가운데/X1/X2 조합 검사를, WindowSmoke에 캡처 대여·직접 획득/반환 검사를 적용했다. 새 검사는 수정 전 `kept=0`으로 실패했고 수정 후 두 구성에서 통과했다. x64 Debug·Release 솔루션 빌드·세 Smoke·Sandbox 숨김 실행/정상 종료와 변경 헤더 독립 컴파일도 통과했으며 D3D12 오류/경고는 0이다. 당시 물리 마우스 검증은 미수행이었고 후속 수동 결과는 위 10월 27일 기록에 구분한다.

### ImGui 프로젝트 항목 경고 수정 (2026-10-25)

Visual Studio가 Build/ImGui.props의 와일드카드·세미콜론 파일 목록에 경고를 표시해 공급 파일 18개를 각각 등록했다. [VC 프로젝트 항목 제약](https://learn.microsoft.com/en-us/cpp/build/reference/vcxproj-files-and-wildcards?view=msvc-170)에 따른 수정이며 외부 소스 7개의 PCH 비사용·`/W3`·`/WX` 해제와 두 소비자 한정 빌드는 유지한다. 파일 변경 시 props·양쪽 필터를 함께 관리한다.

IDE API로 솔루션 로드·프로젝트 항목·경고 없음을 확인하고 MSBuild로 입력 중복·필터 대응·외부 컴파일 설정을 확인했다. IDE 화면을 직접 확인한 결과와는 구분한다. 두 구성의 솔루션 빌드·ImGuiSmoke·Sandbox 숨김 실행/정상 종료가 통과했으며 UI readback은 다섯 크기에서 각 48,000픽셀, D3D12 오류/경고는 0이다.

### ImGui 전체 컨벤션 재검토 (2026-10-23)

자체 C++ 13개 파일과 빌드·프로젝트·필터 6개 파일에 다음을 적용했다. 공급자 원본 16개는 스타일 변경에서 제외하고 해시·라이선스·공급 구조만 확인했다.

| 구분 | 실제 수정·확인과 이유 |
|---|---|
| 헤더·의존성 | 외부 타입의 직접 전방 선언을 공급자 헤더로 교체하고 직접 include·그룹 정렬·Windows 선행 예외를 보완했다. |
| API·수명 | Begin/End/Record 순서, 콜백의 비소유 수명·null 조건, 빈 목록 그리기, 조명 단위·범위를 명시했다. 부분 초기화·GPU 대기 실패 후 정리와 콜백 주소 유지를 위한 복사·이동 금지를 보완했다. |
| 포맷·작업 경계 | 여러 줄 인자 배치와 작업별 실패 검사를 분리하고 초기화·멤버 역할 구분·120열을 보정했다. |
| 테스트·빌드 | ImGuiSmoke의 프레임당 Record 1회와 진단 API 실패 보고를 적용했다. 외부 소스의 두 소비자 한정 컴파일·입력 중복 없음·필터 대응을 확인했다. |
| 중복·가독성 | 조명 보정을 ApplyParameters로 모으고 제한 상수를 UI와 공유했다. 개체·조명 편집과 벡터 입력 변환을 분리하고 불필요한 초기 대입·오류 문자열 용량 중복을 제거했다. |
| 기존 규칙 확인 | 명명·auto·const·네임스페이스·좌표/행렬·소유권을 대조했다. CPU 파싱/GPU 업로드 경계를 유지하며 공급자 셰이더에 자체 row_major 규칙을 강제하지 않는다. |

헤더 7개의 PCH 없는 `/Zs /W4 /WX` 독립 컴파일, 인코딩·공백·프로젝트 입력·문서 링크·공급자 원본 해시 검사를 통과했다. 두 구성의 솔루션 빌드·세 Smoke와 Sandbox 숨김 실행/정상 종료도 통과했다. RendererSmoke의 D3D12 오류/경고·잘못된 가림 픽셀은 0, ImGuiSmoke는 다섯 크기에서 각 48,000 UI 픽셀·D3D12 오류/경고 0이다.

예외와 후속 조건: 공급자 지침의 Win32 메시지 함수 선언을 유지한다. descriptor 콜백의 `std::terminate`는 스택 해제를 보장하지 않는다. [실패 전달 한계](dependencies.md#dear-imgui)는 메모리 부족·장치 제거 주입으로 검증하지 않았으며, 복구 지원 시 공급자 백엔드의 실패 전파·부분 자원 정리를 함께 보완해야 한다. 사용자 텍스처·다중 도구 수요가 생기면 descriptor 공유와 이 정책을 재검토한다.

### ImGui 최소 도입 (2026-10-22)

단일 GPU/Win32 창·내장 영문 폰트·고정 용량 힙·동일 UI 스레드로 시험 적용했다. 한글 폰트/IME·도킹·다중 뷰포트·레이아웃 저장·장치 손실 복구는 미지원이며 기능 수요와 실패 전달 검토 후 확장한다.

- x64 Debug·Release 솔루션 빌드·세 Smoke 통과. RendererSmoke의 D3D12 오류/경고·잘못된 가림 픽셀과 ImGuiSmoke의 D3D12 오류/경고는 0이다.
- WindowSmoke는 UI 캡처 시작 시 누적 입력 제거·새 앱 입력 차단·UI 캡처 유지·수명 메시지·콜백 해제를 검사했다. 실제 백엔드의 버튼 조합 검사는 10월 26일 보강했다.
- ImGuiSmoke는 60프레임·다섯 크기의 UI readback(각 48,000픽셀)·폰트 크기 변경·포커스/최소화·마지막 제출 후 소멸을 검사했다. 카메라·조명 편집의 비유한 값 거부·부분 변경 없음·범위 제한도 확인했다.
- 변경 헤더 6개의 독립 컴파일과 프로젝트 XML·문서 링크·공급자 원본 해시·공백 검사를 통과했다.
- Debug Demo Window와 Release 편집 패널을 실제 화면에서 확인했다. 당시 회전 편집/재개·텍스트 편집 중 카메라 차단·최소화/복원·종료의 전체 수동 조작은 확정하지 못했다. 후속 수동 결과는 10월 27일 기록과 구분한다.

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
