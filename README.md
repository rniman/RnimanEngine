# Rniman Engine

D3D12 기반의 미니 게임 엔진. 장면 구성, 입력, 게임 로직, 렌더링과 리소스 로딩을 연결하여 작은 게임과 그래픽 실험에 사용한다. 각 시스템을 직접 이해하고 설명할 수 있는 크기로 개발한다.

## 현재 상태

**첫 마일스톤 달성:** 직접 정의한 메시·텍스처·카메라·기본 Phong 조명을 연결한 Sandbox 데모를 문서의 절차로 빌드·실행할 수 있다. 위치·크기가 다른 큐브 3개를 렌더링하며 카메라와 방향광을 조절한다.

다음 단계는 일반 오브젝트 방식과 ECS 비교다. ECS·Unity 익스포터/로더·PBR은 아직 구현하지 않았다. [로드맵](docs/roadmap.md) · [Architecture와 좌표계 규약](docs/design.md) · [첫 마일스톤 기록](docs/milestones/v0.1.0-sandbox.md)을 참고한다.

## 카메라 조작

- 창에 포커스가 있을 때 WASD로 시선 방향 전후·수평 좌우 이동, Q/E로 월드 Y축 하강·상승한다. 이동 속도는 초당 3 단위다.
- 마우스 오른쪽 버튼을 누르고 드래그하면 시점이 회전한다. 커서는 숨기거나 고정하지 않으므로 화면 가장자리에서는 드래그를 다시 시작한다.
- 포커스 해제·최소화·창 크기 조절 시 입력을 초기화한다. 복원 후 이동 키를 다시 누른다. 큐브 회전은 시작 이후 경과 시간을 따른다.

## 조명 조작

- 방향키 좌/우는 방향광의 yaw, 위/아래는 pitch를 초당 1 rad로 조절한다. 방향은 빛이 진행하는 월드 방향이며 카메라 이동과 독립적이다.
- Page Up/Down은 직접광 강도를 초당 1씩 증감하며 0~2로 제한한다. 강도 0에서도 Ambient 0.12는 남는다.
- R은 방향과 강도를 초기값(yaw 0.6 rad, pitch −0.8 rad, 강도 0.8)으로 복원한다. R과 다른 조명 키를 함께 누르면 초기화가 우선이다.
- 조작은 창에 포커스가 있을 때만 적용한다. 이동과 동일하게 최소화·모달 창 조작 후 경과 시간을 누적하지 않는다. 광색은 흰색, 반사 강도 0.25·광택 지수 32다.

## 빌드와 실행

확인한 개발 환경:

- Visual Studio Community 2026 18.11.0, C++ 데스크톱 개발 도구
- MSVC `v145` / `14.51.36231`, Windows SDK `10.0.26100.0`
- C++20, x64, Debug·Release

[솔루션](RnimanEngine.slnx)을 열고 `Sandbox`를 시작 프로젝트로 선택한다. x64 Debug 또는 Release를 빌드한 뒤 실행한다. 빌드 전에 아래 BaseColor 에셋을 준비한다. 창 닫기 또는 Alt+F4로 종료한다. D3D12 Feature Level 11_0을 지원하는 하드웨어가 필요하며 WARP로 자동 대체하지 않는다. Debug 실행에는 Windows 선택적 기능 Graphics Tools가 필요하다. 없으면 초기화 오류를 보고하며, Release 앱은 디버그 레이어를 요구하지 않는다.

별도로 준비한 `BrickWall17_MR_1K`의 `BrickWall17_1K_BaseColor.png`를 다음 위치에 둔다.

```text
Assets/BrickWall17_MR_1K/BrickWall17_1K_BaseColor.png
```

저장소 루트의 `Assets/` 전체는 로컬 에셋 보관용으로 Git에서 제외한다. `Tests/Assets/`의 테스트 기준 이미지는 저장소에 포함한다. 새 체크아웃에서도 BaseColor를 별도로 준비해야 하며, 없으면 Sandbox 빌드가 원본 경로와 준비 안내를 표시하고 실패한다. Sandbox의 Debug·Release는 루트 `Assets/`의 같은 PNG를 직접 읽으며 출력 폴더에 복사하지 않는다. 현재 기준 이미지는 1024×1024 PNG다. 나머지 Normal·Roughness·AO·Height는 사용하지 않는다. DDS·압축·밉맵 생성·변환 캐시는 아직 없다.

공용 경로는 [SourcePaths.props](Sandbox/SourcePaths.props)가 빌드 시 계산하는 EXE→저장소 루트 경로를 사용한다. 실행 중 폴더를 검색하거나 작업 디렉터리에 의존하지 않는다. 배치와 후속 계획은 [개발 실행의 공용 에셋 경로](docs/project-structure.md#개발-실행의-공용-에셋-경로)를 참고한다. 런타임 경로 설정 파일과 독립 배포 패키징은 아직 구현하지 않았다.

저장소 루트의 Developer PowerShell에서:

```powershell
MSBuild.exe RnimanEngine.slnx -nologo -t:Build -p:Configuration=Debug -p:Platform=x64
MSBuild.exe RnimanEngine.slnx -nologo -t:Build -p:Configuration=Release -p:Platform=x64
```

솔루션 빌드는 Sandbox와 검증용 RendererSmoke·WindowSmoke를 함께 빌드한다. 테스트 실행은 별도 [검증 명령](docs/development.md#자동-검증)을 따른다. RendererSmoke는 저장소에 포함한 [3×3 기준 PNG](Tests/Assets/TextureTest.png)를 원본 위치에서 직접 읽으며 출력 폴더에 복사하지 않는다. 벽돌 에셋 없이 별도로 빌드·실행할 수 있다. 손상 PNG 검사용 임시 파일은 테스트 EXE 옆에서만 생성·삭제한다. 실행은 Debug·Release 모두 D3D12 하드웨어와 Graphics Tools가 필요하며, WindowSmoke는 Win32 창·입력만 검사한다. Sandbox 실행에는 두 Smoke 실행 파일이 필요하지 않다.

실행 파일은 `bin/Sandbox/<구성>/Sandbox.exe`, 중간 산출물은 `obj/Sandbox/<구성>/`에 생성된다. SDK의 FXC가 `Shaders/Samples/Sandbox/`의 HLSL을 Shader Model 5.0으로 컴파일하여 EXE 옆 `Shaders/CubeVS.cso`와 `Shaders/CubePS.cso`를 만든다. CSO는 EXE 옆, PNG는 빌드가 지정한 저장소 원본 위치에서 읽는다. 누락·디코딩 실패 시 경로·API·HRESULT를 표시하고 종료하며 EXE 옆의 과거 Assets 복사본으로 대체하지 않는다. 디버거 작업 디렉터리는 `$(OutDir)`다. `bin/`과 `obj/`는 Git에서 제외한다.

현재 실행 파일은 개발용 공용 원본 경로에 의존한다. 기본 출력 배치를 유지한 채 저장소 전체를 옮기면 상대 경로가 유지되지만, EXE와 `Shaders`만 다른 위치로 옮기는 독립 실행은 지원하지 않는다. 출력 위치를 변경하면 다시 빌드하여 원본 경로를 맞춘다. 기존 `bin` 아래 PNG 복사본은 더 이상 읽거나 갱신하지 않는다.

## 코드와 문서 안내

대표 실행 프로젝트는 [Sandbox](Sandbox/Sandbox.vcxproj) 하나다. 진입점 [Main.cpp](Sandbox/Main.cpp)의 `wWinMain`은 [rengine::SandboxApp](Sandbox/SandboxApp.cpp)을 생성하고 `Run`을 호출한다. 앱은 [Win32Window](Sandbox/Win32Window.cpp)의 창 생성 → [AssetLoader](Sandbox/AssetLoader.cpp)의 PNG 디코딩 → [D3D12Renderer](Sandbox/D3D12Renderer.cpp) 초기화·텍스처 업로드 → 메시지 처리 → 리사이즈 → 입력·Transform 갱신 → 렌더링 → GPU 완료 확인 → RAII 정리 순서로 실행한다. 현재 파일별 책임과 두 Smoke 프로젝트의 역할은 [현재 프로젝트와 코드](docs/project-structure.md#현재-프로젝트와-코드)에서 확인한다. 엔진 라이브러리와 추가 샘플은 필요할 때 분리한다.

직접 작성한 C++ 타입·함수는 Sandbox 전용 코드와 테스트를 포함해 `rengine` 단일 루트 네임스페이스를 사용한다. `main`·`wWinMain`은 전역 진입점이며 외부 API와 HLSL 이름은 원래 표기를 유지한다. 적용 범위와 호출 예시는 [네임스페이스 규약](docs/design.md#네임스페이스)을 따른다.

| 문서 | 내용 |
|---|---|
| [개발 목표와 순서](docs/roadmap.md) | 기능 범위, 단계별 완료 기준과 첫 마일스톤 |
| [엔진 설계](docs/design.md) | 현재 좌표계·실행·렌더링 계약과 장면·ECS·Unity 설계 방향 |
| [프로젝트와 공유 자원 구조](docs/project-structure.md) | 현재 코드·검증 프로젝트의 역할, 향후 확장 구조와 공유 자원·출력 배치 |
| [외부 라이브러리와 에셋 처리](docs/dependencies.md) | 도입 후보·제외 대상, 텍스처 변환과 DDS 캐시 |
| [개발 설정과 검증](docs/development.md) | 현재 컨벤션 적용 상태, 개발 설정과 검증 실행 방법 |

[마일스톤 목록](docs/milestones/README.md)에서 태그별 달성 범위·검증 결과·당시 제약을 확인한다.

작업 전 [컨벤션 적용 기준과 적용 상태](docs/development.md#컨벤션)를 확인한다. 위키를 컨벤션 확인·적용의 기준으로 사용하며, 컨벤션 확인 시점과 프로젝트의 선택·예외·적용 상태는 해당 문서에서 관리한다. 위키의 프로젝트 문서 갱신과 반영 상태 관리는 위키 정비 담당이 맡는다. 저장소의 에이전트 지침 파일은 [AGENTS.md](AGENTS.md)다.
