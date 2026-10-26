# Rniman Engine

D3D12 기반 미니 게임 엔진. 장면·입력·게임 로직·렌더링·리소스 로딩을 작은 게임과 그래픽 실험에 연결한다.

## 현재 상태

첫 마일스톤은 텍스처 큐브 3개·카메라·기본 Phong 조명을 사용하는 Sandbox 데모다. 공통 원본은 Engine, 실행·조작은 Sandbox에 있으며 네 실행 프로젝트가 필요한 원본을 직접 빌드한다.

**ImGui 최소 도입을 구현했다.** Sandbox에서 큐브·카메라·조명을 편집하고 Demo Window를 열 수 있다. 다음 순서는 **Unity 정적 모델 로딩·렌더링 → 일반 오브젝트와 ECS 비교**다. Unity 익스포터/로더·ECS·PBR은 미구현이다. [로드맵](docs/roadmap.md) · [M02 계획](docs/plans/m02-static-model.md) · [첫 마일스톤](docs/milestones/v0.1.0-sandbox.md)

## 빌드와 실행

확인한 환경은 Visual Studio Community 2026 18.11.0, C++ 데스크톱 개발 도구, MSVC `v145` / `14.51.36231`, Windows SDK `10.0.26100.0`, C++20·x64다.

1. BaseColor PNG를 아래 경로에 준비한다.
2. [솔루션](RnimanEngine.slnx)을 열고 Sandbox를 시작 프로젝트로 선택한다.
3. x64 Debug 또는 Release를 빌드·실행한다. 종료는 창 닫기 또는 Alt+F4다.

```text
Assets/BrickWall17_MR_1K/BrickWall17_1K_BaseColor.png
```

기준 원본은 1024×1024 PNG다. 루트 Assets는 전체 Git 제외이므로 새 체크아웃에서도 별도 준비해야 한다. 누락 시 Sandbox 빌드가 경로·준비 안내와 함께 실패한다.

D3D12 Feature Level 11_0 하드웨어가 필요하며 WARP로 자동 대체하지 않는다. Debug 실행에는 Windows 선택적 기능 Graphics Tools가 필요하고, Release Sandbox는 디버그 레이어를 요구하지 않는다.

저장소 루트의 Developer PowerShell에서 빌드할 수도 있다.

```powershell
MSBuild.exe RnimanEngine.slnx -nologo -t:Build -p:Configuration=Debug -p:Platform=x64
MSBuild.exe RnimanEngine.slnx -nologo -t:Build -p:Configuration=Release -p:Platform=x64
```

실행 파일은 `bin/Sandbox/<구성>/Sandbox.exe`, 중간 산출물은 `obj/Sandbox/<구성>/`, 컴파일한 셰이더는 EXE 옆 `Shaders/`에 생성된다. 솔루션 빌드는 세 Smoke도 빌드하며 테스트 실행은 [자동 검증](docs/development.md#자동-검증)을 따른다. RendererSmoke는 기준 PNG, ImGuiSmoke는 메모리의 1×1 텍스처를 사용해 벽돌 에셋 없이 실행할 수 있으며 두 테스트는 두 구성 모두 Graphics Tools가 필요하다.

PNG는 [SourcePaths.props](Build/SourcePaths.props)가 지정한 저장소 원본에서, CSO는 EXE 옆에서 읽는다. 누락·디코딩 실패 시 경로·API·HRESULT를 보고하고 종료한다. 출력 폴더의 과거 PNG로 대체하지 않는다. 기본 출력 구조를 유지한 저장소 이동은 가능하지만 EXE·Shaders만 옮기는 독립 실행은 미지원이다. 출력 위치 변경 시 다시 빌드한다. [경로 규칙](docs/project-structure.md#개발-실행의-공용-에셋-경로)

## 개발 UI

`Sandbox Tools` 패널에서 큐브 선택·위치·회전(radian)·양수 Scale, 카메라 위치, 방향광 yaw/pitch·강도를 편집한다. 숫자는 드래그하거나 Ctrl+클릭으로 입력한다. 편집 값은 실행 중에만 유지한다.

- 회전 편집은 선택한 큐브의 자동 회전을 정지한다. `Auto rotate`로 재개하면 편집 각도부터 이어간다. 위치·Scale 편집은 자동 회전을 중단하지 않는다.
- Scale은 0.05~10, 위치는 −100~100으로 제한한다. 비유한·특이·반전 변환은 거부하고 패널에 표시한다.
- `Show Dear ImGui Demo`로 공식 Demo Window를 열고, `Reset light`로 기존 R 키와 같은 초기값을 적용한다.
- UI가 마우스·키보드를 캡처하는 동안 해당 카메라·조명 조작을 차단한다. 장면의 빈 곳을 클릭하면 앱 키 조작으로 돌아간다.
- ms/frame·FPS는 ImGui의 평균 프레임 간격이다. CPU/GPU 실행 시간의 분리 측정이 아니며 VSync·Present 대기를 포함할 수 있다.

Dear ImGui v1.93.0 소스·MIT 라이선스는 저장소에 포함되어 별도 다운로드가 필요 없다. 기본 내장 폰트·영문 패널을 사용한다. 한글 폰트/IME·도킹·다중 뷰포트·레이아웃 저장은 후속 범위다. [의존성](docs/dependencies.md#dear-imgui)

## 카메라 조작

- WASD: 시선 방향 전후·수평 좌우, Q/E: 월드 Y축 하강·상승. 속도는 초당 3 단위다.
- 마우스 오른쪽 드래그: 시점 회전. 커서 숨김·고정은 없으므로 화면 가장자리에서 다시 드래그한다.
- 포커스 해제·최소화·크기 조절 시 입력을 초기화한다. 복원 후 키를 다시 누른다. 자동 회전이 켜진 큐브는 입력 중단 시간도 경과 시간에 반영한다. 수동 정지 중 시간은 적립하지 않는다.

## 조명 조작

- 방향키 좌/우: yaw, 위/아래: pitch를 초당 1 rad 조절한다. 방향은 빛이 진행하는 월드 방향이다.
- Page Up/Down: 직접광 강도를 초당 1씩 조절하며 범위는 0~2다. 강도 0에서도 Ambient 0.12는 남는다.
- R: yaw 0.6 rad·pitch −0.8 rad·강도 0.8로 초기화하며 다른 조명 키보다 우선한다. 광색은 흰색, 반사 강도 0.25·광택 지수 32다.

카메라·조명 조작은 창에 포커스가 있을 때만 적용하며 입력 중단 동안의 이동 시간을 누적하지 않는다.

## 코드와 문서 안내

진입점은 [Main.cpp](Sandbox/Main.cpp)의 `wWinMain` → [rengine::SandboxApp::Run](Sandbox/SandboxApp.cpp)이다. [실행·소유권 계약](docs/design.md#sandbox-실행-구조)과 [프로젝트별 소스 구성](docs/project-structure.md#현재-프로젝트와-코드)을 참고한다.

Visual Studio 솔루션 탐색기의 Docs에서 README·AGENTS·프로젝트 문서를 열 수 있다. 마일스톤·계획은 Docs 하위의 milestones·plans에 표시하며 새 문서는 솔루션 항목에 수동 등록한다. 학습 메모(`docs/memo/`)는 솔루션 표시에서 제외하고 아래 링크로 연다.

| 문서 | 내용 |
|---|---|
| [개발 목표와 순서](docs/roadmap.md) | 개발 범위·순서와 다음 마일스톤 |
| [엔진 설계](docs/design.md) | 좌표계·실행·렌더링 계약과 확장 방향 |
| [프로젝트 구조](docs/project-structure.md) | 코드 소속·의존성·공유 자원·출력 경로 |
| [외부 라이브러리와 에셋](docs/dependencies.md) | 사용 중인 구성·도입 후보·텍스처 캐시 계획 |
| [개발 설정과 검증](docs/development.md) | 컨벤션 적용 상태·설정·검증 명령 |
| [ImGui 사용 메모](docs/memo/ImGui.md) | Dear ImGui 기본 개념·주요 함수·예제·ID·입력 캡처 |
| [마일스톤 기록](docs/milestones/README.md) | 달성 범위·검증 결과·당시 제약 |

작업 전 [컨벤션 적용 상태](docs/development.md#컨벤션)와 [AGENTS.md](AGENTS.md)를 확인한다.
