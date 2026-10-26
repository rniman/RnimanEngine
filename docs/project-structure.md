# 프로젝트와 공유 자원 구조

[프로젝트 소개와 실행](../README.md) · [엔진 설계](design.md) · [개발 설정과 검증](development.md)

## 현재 프로젝트와 코드

[솔루션](../RnimanEngine.slnx)의 네 실행 프로젝트는 x64 Debug·Release를 지원한다. 별도 Engine 라이브러리는 아직 없으며 각 프로젝트가 필요한 공통 원본을 직접 컴파일한다.

| 프로젝트 | 역할과 입력 |
|---|---|
| [Sandbox](../Sandbox/Sandbox.vcxproj) | 대표 데모. Sandbox 코드·Engine의 창/이미지 로더/렌더러·큐브 셰이더 |
| [RendererSmoke](../Tests/RendererSmoke.vcxproj) | CPU·GPU 검증. D3D12Renderer.cpp·AssetLoader.cpp·큐브 셰이더와 테스트 코드 |
| [WindowSmoke](../Tests/WindowSmoke.vcxproj) | Win32 창·입력 검증. Win32Window.cpp와 테스트 코드 |
| [ImGuiSmoke](../Tests/ImGuiSmoke.vcxproj) | UI GPU·수명 검증. SandboxGui·Engine 창/로더/렌더러·ImGui·큐브 셰이더 |

Smoke는 독립 테스트 실행 파일이며 Sandbox의 실행·배포 의존성이 아니다. 검사 범위·실행 명령은 [자동 검증](development.md#자동-검증)을 따른다.

```text
RnimanEngine/
├─ RnimanEngine.slnx
├─ Directory.Build.props       # 공통 출력 경로
├─ Engine/
│  ├─ Platform/                # Win32Window
│  ├─ Assets/                  # AssetLoader, ImageData
│  ├─ Rendering/               # D3D12Renderer, RenderData
│  └─ Math/                    # Transform
├─ Build/                      # SourcePaths.props, ImGui.props
├─ Sandbox/                    # Main, SandboxApp, 카메라·조명·SandboxGui, PCH, CubeShaders.props
├─ Tests/                      # Smoke, readback·조명 검사, Assets/TextureTest.png
├─ Shaders/Samples/Sandbox/    # CubeVS.hlsl, CubePS.hlsl
├─ ThirdParty/imgui/           # 고정 버전 원본·라이선스·해시
├─ Assets/                     # 별도 준비하는 로컬 에셋, 전체 Git 제외
└─ docs/
```

### 현재 include 기준과 허용 의존성

| 소비자 | include 루트·PCH |
|---|---|
| Engine | Engine/. 구현은 자기 헤더를 먼저 포함하며 Sandbox·Tests·PCH를 참조하지 않음 |
| Sandbox | Sandbox/·Engine/·ThirdParty/imgui/. Main·SandboxApp·pch.cpp에 PCH 사용, Engine·SandboxGui·외부 구현은 PCH 비사용 |
| RendererSmoke | Engine/·Sandbox/, PCH 비사용. 데모 카메라·조명은 조작 계약 검증에만 참조 |
| WindowSmoke | Engine/, PCH 비사용 |
| ImGuiSmoke | Engine/·Sandbox/·ThirdParty/imgui/, PCH 비사용 |

앱 → SandboxGui·창·로더·렌더러·Transform, SandboxGui → Win32/DX12 백엔드, 로더 → ImageData·Windows/WIC, 렌더러 → ImageData·RenderData·EXE 경로 함수·D3D12의 의존성을 유지한다. 창은 GPU·카메라를, CPU 데이터·Transform은 앱·렌더러 구현을 참조하지 않는다. RendererReadback은 검증용 friend로 렌더러 내부에 접근한다.

SandboxGui·외부 ImGui 원본은 Sandbox·ImGuiSmoke만 빌드한다. 자체 연결 설정은 Build/ImGui.props에 두고 외부 원본은 수정하지 않는다. VC 프로젝트의 IDE 표시를 위해 props의 ClCompile·ClInclude·None은 와일드카드·세미콜론 목록 없이 파일별로 등록한다. 공급 파일을 추가·제거할 때 props와 두 소비자의 필터를 함께 맞춘다. Sandbox·ImGuiSmoke의 외부 코드는 ThirdParty 필터, Sandbox의 자체 설정은 Build 필터에 표시한다. ImGuiSmoke의 소규모 자체 테스트 입력은 평면 필터를 유지한다.

공유 원본을 복사하거나 같은 EXE에 중복 등록하지 않는다. `../`는 프로젝트 입력·import에 사용하고 C++ include는 각 코드 루트 기준으로 작성한다. 필터는 실제 Engine 기능별 배치를 반영하며 네임스페이스는 [rengine 규약](design.md#네임스페이스)을 따른다. 적용·검증 근거는 [개발 문서](development.md#기존-적용의-검증-근거)에 둔다.

## 공유 자원과 실행 배치

### 공통 경로 설정

[Directory.Build.props](../Directory.Build.props)는 자신의 위치를 EngineRoot로 두고 다음 출력을 설정한다. MSBuild가 프로젝트의 상위 폴더에서 자동 탐색하며 프로젝트 이름은 `.vcxproj` 파일명에서 가져온다.

- 실행·CSO: `$(EngineRoot)bin\$(MSBuildProjectName)\$(Configuration)\`
- 중간 산출물: `$(EngineRoot)obj\$(MSBuildProjectName)\$(Configuration)\`
- Sandbox 디버거 작업 디렉터리: `$(OutDir)`

프로젝트 파일명은 출력 충돌을 막도록 고유해야 한다. 하위에 별도 Directory.Build.props를 두면 루트 설정을 명시적으로 import해야 하며 개별 OutDir·IntDir 정의는 공통값을 덮어쓸 수 있다.

### 개발 실행의 공용 에셋 경로

[SourcePaths.props](../Build/SourcePaths.props)는 Sandbox·RendererSmoke가 OutDir 설정 뒤에 import한다. `MSBuild::MakeRelative(OutDir, EngineRoot)` 결과를 슬래시 표기의 wide 문자열 매크로 `RNIMAN_SOURCE_ROOT`로 전달한다. 기본 출력의 값은 `../../../`이며 다른 드라이브 출력에서는 절대 경로가 될 수 있다.

앱·테스트는 `GetExecutableDirectory()`와 이 매크로에 각각 `Assets/`·`Tests/Assets/`를 합쳐 정규화한다. 작업 디렉터리·고정된 상위 폴더 개수에 의존하지 않는다. PNG는 Debug·Release가 같은 원본을 직접 읽으며 출력에 복사하지 않는다. 누락·디코딩 실패 시 실제 경로·API·HRESULT로 실패하고 EXE 옆의 과거 복사본으로 대체하지 않는다.

기본 출력 구조를 유지하면 저장소 전체를 이동할 수 있다. EXE·Shaders만 옮기는 독립 실행은 미지원이며 출력 위치 변경 시 다시 빌드한다. 런타임 루트 설정·상위 폴더 검색·독립 배포 패키징은 미구현이다.

원본은 읽기 전용으로 소비한다. RendererSmoke의 손상 PNG는 구성별 EXE 옆 `CorruptTexture.png`에서 생성·삭제한다. 사용자 파일·캐시 등 쓰기 위치도 도입 시 원본과 분리한다.

### 셰이더

[CubeShaders.props](../Sandbox/CubeShaders.props)를 Sandbox·RendererSmoke·ImGuiSmoke가 공유한다. SDK FXC의 `vs_5_0`·`ps_5_0`, 진입점 Main으로 컴파일해 각 프로젝트의 `$(OutDir)Shaders/CubeVS.cso`·`CubePS.cso`에 생성한다. Debug는 `/Zi /Od`, Release는 기본 최적화다. 런타임은 EXE 옆 Shaders를 읽는다.

HLSLI·매크로 변형·공용 캐시는 아직 없다. Shader Model 6 기능 도입 시 DXC 전환을 검토한다. 캐시를 추가하면 소스·include·매크로·진입점·대상·컴파일러 버전/옵션을 재사용 조건에 포함하고 완성된 결과만 공개한다.

### Git 관리

| 대상 | 관리 방식 |
|---|---|
| 루트 Assets/ | 전체 Git 제외. 실행 에셋 별도 준비 |
| Tests/Assets/·HLSL·변환 설정 | 재현용 원본으로 추적 |
| bin/·obj/ | 앱·구성별 생성물, Git 제외. 수동 수정하지 않음 |
| 향후 .bin 모델·DDS·Cache/ | 기준 에셋은 출처·추적 위치를 정하고, 재생성 가능한 캐시는 도입과 함께 제외 규칙 추가 |

## 향후 확장 구조

아래는 미구현 배치 후보이며 실제 사용하는 기능과 함께 추가한다. 현재 M02는 기존 Sandbox에 통합한다.

| 위치 | 도입 조건·역할 |
|---|---|
| Engine 라이브러리 프로젝트 | 독립 빌드·재사용이 필요할 때 추가하고 소비자의 직접 원본 빌드를 프로젝트 참조로 전환 |
| Samples/ | 기능별 장면·입력·에셋을 가진 독립 실행 데모가 필요할 때 추가 |
| Tools/ | Unity 익스포터 등 개발 도구 |
| Assets/Shared·Assets/Samples, 공용 셰이더 | 앱 간 실제 공유 수요에 맞춰 분리 |
| Cache/ | 텍스처·셰이더 변환 비용이 생길 때 추가. [텍스처 캐시 규칙](dependencies.md#텍스처-변환과-캐시) 적용 |

배포 시 필요한 에셋·CSO·DLL만 수집하고 동명 파일 덮어쓰기를 막는 경로 규칙을 정한다. 범용 가상 파일 시스템·에셋 DB는 실제 필요가 생길 때 검토한다. 배치 변경 시 관련 설정·문서·Git 제외 규칙을 맞추고 [검증 방법](development.md#검증-방법)에 따라 확인한다.
