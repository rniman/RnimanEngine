# Rniman Engine

D3D12 기반의 미니 게임 엔진. 장면 구성, 입력, 게임 로직, 렌더링과 리소스 로딩을 연결하여 작은 게임과 그래픽 실험에 사용한다. 각 시스템을 직접 이해하고 설명할 수 있는 크기로 개발한다.

## 현재 상태

2026-09-18 기준, Sandbox에서 D3D12로 RGB 정점 색상을 보간한 삼각형을 출력한다. 빌드 시 컴파일한 Vertex·Pixel Shader, Root Signature, PSO, Vertex Buffer를 연결했다. 어두운 파란색 배경 중앙에 그리며 정사각형 Viewport로 리사이즈 후에도 삼각형 비율을 유지한다. 고성능 하드웨어 GPU 선택, 프레임별 Command Allocator·Fence와 리사이즈·종료의 전체 GPU 대기는 유지한다.

x64 Debug·Release 빌드와 각 136프레임·8회 크기 변경 요청의 smoke test가 성공했으며 D3D12 오류·경고는 0건이었다. 실제 두 구성의 삼각형 출력·Alt+F4 종료, Debug의 최대화·복원·최소화 후 복원을 확인했다. 저장소 밖 작업 디렉터리에서의 셰이더 로딩과 CSO 누락 오류도 확인했다. 연속 드래그 리사이즈·Visual Studio F5·성능 측정은 미검증이다. 상세 범위는 [삼각형 검증 기록](docs/development.md#삼각형-출력과-검증)을 따른다.

ECS, Unity 익스포터·로더, PBR은 아직 구현하지 않았다. 다음 단계는 직접 정의한 메시와 Transform·카메라를 연결하고 남은 창 동작을 검증하는 것이다. 첫 마일스톤은 메시·텍스처·카메라·기본 조명을 사용하는 데모를 문서대로 다시 실행할 수 있는 상태다.

## 빌드와 실행

확인한 개발 환경:

- Visual Studio Community 2026 18.10.0, C++ 데스크톱 개발 도구
- MSVC `v145` / `14.51.36231`, Windows SDK `10.0.26100.0`
- C++20, x64, Debug·Release

[솔루션](RnimanEngine.slnx)을 열고 `Sandbox`를 시작 프로젝트로 선택한다. x64 Debug 또는 Release를 빌드한 뒤 실행한다. 현재 외부 에셋은 필요하지 않으며, 창 닫기 또는 Alt+F4로 종료한다. D3D12 Feature Level 11_0을 지원하는 하드웨어가 필요하며 WARP로 자동 대체하지 않는다. Debug 실행에는 Windows 선택적 기능 Graphics Tools가 필요하다. 없으면 초기화 오류를 보고하며, Release 앱은 디버그 레이어를 요구하지 않는다.

저장소 루트의 Developer PowerShell에서:

```powershell
MSBuild.exe RnimanEngine.slnx -nologo -t:Build -p:Configuration=Debug -p:Platform=x64
MSBuild.exe RnimanEngine.slnx -nologo -t:Build -p:Configuration=Release -p:Platform=x64
```

실행 파일은 `bin/Sandbox/<구성>/Sandbox.exe`, 중간 산출물은 `obj/Sandbox/<구성>/`에 생성된다. SDK의 FXC가 `Shaders/Samples/Sandbox/`의 HLSL을 Shader Model 5.0으로 컴파일하여 EXE 옆 `Shaders/TriangleVS.cso`와 `Shaders/TrianglePS.cso`를 만든다. **실행 파일을 옮길 때는 이 `Shaders` 폴더도 함께 복사한다.** 로더는 작업 디렉터리와 무관하게 EXE 위치를 기준으로 CSO를 찾으며, 누락되면 경로와 HRESULT를 표시하고 종료한다. 디버거 작업 디렉터리는 `$(OutDir)`다. `bin/`과 `obj/`는 Git에서 제외한다.

## 코드와 문서 안내

대표 실행 프로젝트는 [Sandbox](Sandbox/Sandbox.vcxproj) 하나다. 진입점은 [Main.cpp](Sandbox/Main.cpp)의 `wWinMain`이며 창 생성 → [D3D12Renderer](Sandbox/D3D12Renderer.cpp) 초기화 → 메시지 처리·리사이즈·렌더링 → GPU 완료 확인 → RAII 정리 순서로 실행한다. 엔진 라이브러리와 추가 샘플은 필요할 때 분리한다. 별도 [RendererSmoke 테스트 프로젝트](Tests/RendererSmoke.vcxproj)는 현재 솔루션에 포함되어 있으며 별도 [검증 명령](docs/development.md#d3d12-화면-출력과-검증)으로 실행한다.

| 문서 | 내용 |
|---|---|
| [개발 목표와 순서](docs/roadmap.md) | 기능 범위, 단계별 완료 기준과 첫 마일스톤 |
| [엔진 설계](docs/design.md) | 장면·ECS·렌더러 책임, 소유권·수명, Unity 데이터 규격 |
| [프로젝트와 공유 자원 구조](docs/project-structure.md) | Sandbox·Samples 역할, 공용 에셋·셰이더·CSO·캐시·출력 배치 |
| [외부 라이브러리와 에셋 처리](docs/dependencies.md) | 도입 후보·제외 대상, 텍스처 변환과 DDS 캐시 |
| [개발 설정과 검증](docs/development.md) | 적용 컨벤션, 컴파일·PCH·Git 설정, 검증 기준·기록과 제약 |

작업 전 [컨벤션 적용 기준과 상호 반영 절차](docs/development.md#컨벤션)를 확인한다. 공통 기준은 위키의 현재 채택 규칙이며, 마지막 확인·반영 시점과 프로젝트 적용 상태는 해당 문서에서 관리한다. 저장소의 에이전트 지침 파일은 [AGENTS.md](AGENTS.md)다.
