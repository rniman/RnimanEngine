# Rniman Engine

D3D12 기반의 미니 게임 엔진. 장면 구성, 입력, 게임 로직, 렌더링과 리소스 로딩을 연결하여 작은 게임과 그래픽 실험에 사용한다. 각 시스템을 직접 이해하고 설명할 수 있는 크기로 개발한다.

## 현재 상태

2026-10-07 기준, Sandbox에서 위치·크기가 다른 색상 큐브 3개가 각각 Y·X·Z축으로 회전한다.

- Sandbox 전용 카메라에 키보드 이동·마우스 회전을 연결했다.
- 창 수명·Win32 메시지·입력 수집은 `Win32Window`, 실행 루프·카메라·큐브 갱신은 `SandboxApp`으로 분리했다. Main은 앱을 생성하고 실행한다.
- 큐브는 고정 메시를 공유하며 오브젝트별 Transform과 World·View·Projection 행렬을 사용한다.
- D32 깊이 버퍼로 가림을 처리하고, 전체 창 Viewport와 현재 종횡비의 투영 행렬을 사용한다.
- 프레임별 Command Allocator·Constant Buffer·Fence를 관리하며 리사이즈·종료 시 전체 GPU 작업을 기다린다.

2026-10-07 커밋 전 점검 후 x64 Debug·Release 빌드와 RendererSmoke·WindowSmoke를 통과했다. 세 해상도의 가림 검증에서 잘못된 픽셀은 0개였고, 두 그리기 순서의 결과가 일치했다. 자동 테스트의 D3D12 오류·경고는 0건이다. Sandbox 두 구성의 숨김 창 실제 크기 변경·정상 종료와 Debug의 지정 종료 코드 전달도 확인했다. 실제 화면은 이전 구현 검증에서 확인했으며 추출 후에는 시각 확인과 실제 키보드·마우스 입력 검증을 수행하지 않았다. 상세 범위와 제약은 [최신 검증 결과](docs/development.md#최신-검증-결과)를 따른다.

텍스처 구현 전 창·입력과 앱 실행부를 분리했다. 다음 구현은 텍스처와 기본 조명이다. **첫 마일스톤은 메시·텍스처·카메라·기본 조명을 사용하는 데모를 문서대로 다시 실행할 수 있는 상태**다. 범용 장면·Transform 계층·카메라 시스템, ECS, Unity 익스포터·로더, PBR은 아직 구현하지 않았다. 단계별 목표는 [로드맵](docs/roadmap.md)에서 관리한다.

## 카메라 조작

- 창에 포커스가 있을 때 WASD로 시선 방향 전후·수평 좌우 이동, Q/E로 월드 Y축 하강·상승한다. 이동 속도는 초당 3 단위다.
- 마우스 오른쪽 버튼을 누르고 드래그하면 시점이 회전한다. 커서는 숨기거나 고정하지 않으므로 화면 가장자리에서는 드래그를 다시 시작한다.
- 포커스 해제·최소화·창 크기 조절 시 입력을 초기화한다. 복원 후 이동 키를 다시 누른다. 큐브 회전은 시작 이후 경과 시간을 따른다.

## 빌드와 실행

확인한 개발 환경:

- Visual Studio Community 2026 18.10.3, C++ 데스크톱 개발 도구
- MSVC `v145` / `14.51.36231`, Windows SDK `10.0.26100.0`
- C++20, x64, Debug·Release

[솔루션](RnimanEngine.slnx)을 열고 `Sandbox`를 시작 프로젝트로 선택한다. x64 Debug 또는 Release를 빌드한 뒤 실행한다. 현재 외부 에셋은 필요하지 않으며, 창 닫기 또는 Alt+F4로 종료한다. D3D12 Feature Level 11_0을 지원하는 하드웨어가 필요하며 WARP로 자동 대체하지 않는다. Debug 실행에는 Windows 선택적 기능 Graphics Tools가 필요하다. 없으면 초기화 오류를 보고하며, Release 앱은 디버그 레이어를 요구하지 않는다.

저장소 루트의 Developer PowerShell에서:

```powershell
MSBuild.exe RnimanEngine.slnx -nologo -t:Build -p:Configuration=Debug -p:Platform=x64
MSBuild.exe RnimanEngine.slnx -nologo -t:Build -p:Configuration=Release -p:Platform=x64
```

솔루션 빌드는 Sandbox와 검증용 RendererSmoke·WindowSmoke를 함께 빌드한다. 테스트 실행은 별도 [검증 명령](docs/development.md#자동-검증)을 따른다. RendererSmoke 실행은 Debug·Release 모두 D3D12 하드웨어와 Graphics Tools가 필요하며, WindowSmoke는 Win32 창·입력만 검사한다. Sandbox 실행·배포에는 두 Smoke 실행 파일이 필요하지 않다.

실행 파일은 `bin/Sandbox/<구성>/Sandbox.exe`, 중간 산출물은 `obj/Sandbox/<구성>/`에 생성된다. SDK의 FXC가 `Shaders/Samples/Sandbox/`의 HLSL을 Shader Model 5.0으로 컴파일하여 EXE 옆 `Shaders/CubeVS.cso`와 `Shaders/CubePS.cso`를 만든다. **실행 파일을 옮길 때는 이 `Shaders` 폴더도 함께 복사한다.** 로더는 작업 디렉터리와 무관하게 EXE 위치를 기준으로 CSO를 찾으며, 누락되면 경로와 HRESULT를 표시하고 종료한다. 디버거 작업 디렉터리는 `$(OutDir)`다. `bin/`과 `obj/`는 Git에서 제외한다.

## 코드와 문서 안내

대표 실행 프로젝트는 [Sandbox](Sandbox/Sandbox.vcxproj) 하나다. 진입점 [Main.cpp](Sandbox/Main.cpp)의 `wWinMain`은 [SandboxApp](Sandbox/SandboxApp.cpp)을 생성하고 `Run`을 호출한다. 앱은 [Win32Window](Sandbox/Win32Window.cpp)의 창 생성 → [D3D12Renderer](Sandbox/D3D12Renderer.cpp) 초기화 → 메시지 처리 → 리사이즈 → 입력·Transform 갱신 → 렌더링 → GPU 완료 확인 → RAII 정리 순서로 실행한다. 현재 파일별 책임과 두 Smoke 프로젝트의 역할은 [현재 프로젝트와 코드](docs/project-structure.md#현재-프로젝트와-코드)에서 확인한다. 엔진 라이브러리와 추가 샘플은 필요할 때 분리한다.

| 문서 | 내용 |
|---|---|
| [개발 목표와 순서](docs/roadmap.md) | 기능 범위, 단계별 완료 기준과 첫 마일스톤 |
| [엔진 설계](docs/design.md) | 장면·ECS·렌더러 책임, 소유권·수명, Unity 데이터 규격 |
| [프로젝트와 공유 자원 구조](docs/project-structure.md) | 현재 코드·검증 프로젝트의 역할, 향후 확장 구조와 공유 자원·출력 배치 |
| [외부 라이브러리와 에셋 처리](docs/dependencies.md) | 도입 후보·제외 대상, 텍스처 변환과 DDS 캐시 |
| [개발 설정과 검증](docs/development.md) | 적용 컨벤션, 컴파일·PCH·Git 설정, 검증 기준·기록과 제약 |

작업 전 [컨벤션 적용 기준과 적용 상태](docs/development.md#컨벤션)를 확인한다. 위키를 컨벤션 확인·적용의 기준으로 사용하며, 컨벤션 확인 시점과 프로젝트의 선택·예외·적용 상태는 해당 문서에서 관리한다. 위키의 프로젝트 문서 갱신과 반영 상태 관리는 위키 정비 담당이 맡는다. 저장소의 에이전트 지침 파일은 [AGENTS.md](AGENTS.md)다.
