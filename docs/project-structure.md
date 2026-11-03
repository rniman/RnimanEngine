# 프로젝트와 공유 자원 구조

[README](../README.md) · [설계](design.md) · [개발·검증](development.md)

## 현재 프로젝트와 코드

네 실행 프로젝트가 필요한 공통 원본을 직접 컴파일하며, Math Utility 프로젝트가 수학 헤더를 별도로 관리한다. 별도 Engine 라이브러리는 없으며 Smoke는 Sandbox의 배포 의존성이 아니다.

솔루션 탐색기의 `Smoke` 폴더에는 RendererSmoke·WindowSmoke·ImGuiSmoke를 묶는다. 실제 파일은 `Tests/`에 유지하며 이 폴더는 표시용으로만 사용한다.

| 프로젝트 | 주요 입력 |
|---|---|
| [Math](../Engine/Math/Math.vcxproj) | 헤더 전용 Utility 프로젝트. 수치·벡터·행렬·Quaternion·Transform 헤더 표시·관리, 컴파일·링크 산출물 없음 |
| [Sandbox](../Sandbox/Sandbox.vcxproj) | 앱·카메라·조명·UI, Engine 창·이미지/모델 로더·렌더러, ReferenceImporter·ImGui·셰이더 |
| [RendererSmoke](../Tests/RendererSmoke.vcxproj) | 렌더러·이미지/모델 로더·ReferenceImporter·셰이더, 카메라/조명·MathChecks·CPU/GPU 검사 |
| [WindowSmoke](../Tests/WindowSmoke.vcxproj) | Win32Window·입력 검사. D3D12 의존성 없음 |
| [ImGuiSmoke](../Tests/ImGuiSmoke.vcxproj) | SandboxGui·Engine 창/이미지 로더/렌더러·ImGui·셰이더 |

| 경로 | 역할 |
|---|---|
| Engine/Platform, Assets, Rendering, Math | 창·CPU 에셋·GPU 렌더링·수학/Transform |
| Sandbox/ | 실행·조작·UI·PCH·CubeShaders.props |
| Tests/ | Smoke·readback·수학/조명/모델 검사와 기준 Assets |
| Build/ | SourcePaths.props·ImGui.props·ModelImporter.props |
| Shaders/Samples/Sandbox/ | CubeVS.hlsl·CubePS.hlsl. 모델에도 같은 Phong 파이프라인 사용 |
| ThirdParty/ | imgui·ReferenceImporter 소스·공급 기록 |
| Assets/ | 로컬 모델·이미지. 전체 Git 제외 |
| bin/, obj/ | 프로젝트·구성별 생성물. Git 제외 |

### 정적 모델 연결로 추가된 파일

정적 모델 연결로 추가된 파일은 다음 역할을 맡는다. 기존 소속을 바꾸는 범용 자원 관리자나 별도 라이브러리는 추가하지 않았다.

| 추가 파일·경로 | 역할 |
|---|---|
| [MeshData.h](../Engine/Assets/MeshData.h)·[ModelLoader](../Engine/Assets/ModelLoader.h) | CPU 메시·모델 값과 Binary v2 변환 진입점 |
| [ModelImporter.props](../Build/ModelImporter.props)·[ReferenceImporter](../ThirdParty/ReferenceImporter/README.md) | Sandbox·RendererSmoke의 공통 모델 소스 등록과 공급자 파서·규격 |
| [ModelChecks](../Tests/ModelChecks.cpp)·[MultiModelChecks](../Tests/MultiModelChecks.cpp) | 고정 입력 손상·변환, 실제 모델·JPEG·다중 모델·Wrap 검사 |
| Tests/Assets/ModelFixture·LegacyModels·WrapModes·ColorTest.jpg | 고정 v2·이전 형식 거부·축별 Wrap·JPEG 회귀 입력 |

### 헤더 전용 Math 모듈

[Math Utility](../Engine/Math/Math.vcxproj)는 수치·벡터·행렬·Quaternion·Transform의 여섯 헤더를 관리한다. 소비자는 Engine include 경로로 포함하며 별도 cpp·lib·ProjectReference는 필요 없다. 헤더는 소비 프로젝트에서 컴파일한다. Utility의 C++20·Engine include 경로·`/utf-8`은 자체 편집·분석 문맥을 위한 설정이다.

실제 사용 흐름은 `LoadStaticModel → DecomposeTransform`, `BuildObjectTransformConstants → BuildNormalTransform`이다. 파일 오류 전달은 Assets, WVP·셰이더 상수 구성은 Rendering에 남는다. TRS 분해는 Quaternion을 직접 저장하고 재구성 오차로 전단을 거부한다. 표시용 Euler와 편집 캐시는 [SandboxRotation](../Sandbox/SandboxRotation.h)이 관리하며, Math에는 UI 상태를 두지 않는다. [계층 지원 범위](static-model.md#계층-지원-범위와-알려진-문제)를 따른다.

연산 계약·사용 예·[MathChecks의 실행 흐름](math.md#mathchecks-검증)은 Math 문서에서 관리한다. MathChecks는 RendererSmoke에 포함하며 별도 MathTests 프로젝트는 없다.

### 현재 include 기준과 허용 의존성

Engine은 `Engine/`을 include 루트로 쓰고 Sandbox·Tests·PCH에 의존하지 않는다. Sandbox·UI/렌더러 검사는 필요한 `Sandbox/` 루트를 추가한다. ImGui·ReferenceImporter include 루트는 각각 사용하는 프로젝트에만 props로 추가한다.

앱은 창·로더·렌더러·Transform을 조합하고, 이미지 로더는 WIC, 모델 로더는 ReferenceImporter를 사용한다. ModelLoader는 헤더 전용 Math/TransformMath.h의 TRS 복원을 사용하며 Rendering에 의존하지 않는다. RenderData도 같은 Math 모듈의 아핀 검증·역전치 계산을 사용한다. Math는 DirectXMath·표준 라이브러리와 기존 Transform만 사용하고 Assets·Rendering·ImGui에 의존하지 않는다. Engine 공개 데이터에 공급자 파서 타입·ImGui 타입을 노출하지 않는다. RendererReadback의 friend 접근은 검증에 한정한다.

Main·SandboxApp·pch.cpp만 PCH를 사용하며 Engine·SandboxGui·Smoke·공급자 원본은 사용하지 않는다. C++ include는 코드 루트 기준, 프로젝트 입력·import는 상대 경로를 사용한다. 공통 원본을 복사하거나 같은 EXE에 중복 등록하지 않는다.

props의 IDE 항목은 파일별로 등록하고 추가·삭제 시 소비자의 필터도 맞춘다. Sandbox는 Engine 기능별·ThirdParty·Assets 필터를, 소규모 테스트 입력은 기존 평면 표시를 유지한다. 자체 네임스페이스는 [rengine](design.md#네임스페이스)다.

## 공유 자원과 실행 배치

### 공통 경로 설정

[Directory.Build.props](../Directory.Build.props)는 저장소 루트를 EngineRoot로 두고 `bin/<프로젝트>/<구성>/`과 `obj/<프로젝트>/<구성>/`을 지정한다. Sandbox 디버거 작업 디렉터리는 OutDir이다. 프로젝트 파일명은 출력 충돌을 막도록 고유해야 한다.

### 개발 실행의 공용 에셋 경로

[SourcePaths.props](../Build/SourcePaths.props)는 Sandbox·RendererSmoke가 OutDir 설정 뒤 import한다. EXE 출력에서 저장소 루트까지의 경로를 `RNIMAN_SOURCE_ROOT`로 전달한다. 기본값은 `../../../`이며 다른 드라이브 출력에서는 절대 경로가 될 수 있다.

앱·테스트는 GetExecutableDirectory와 이 값을 합쳐 원본 Assets·Tests/Assets를 읽는다. 작업 디렉터리나 EXE 옆 에셋 복사본에 의존하지 않는다. 기본 구조를 유지한 저장소 이동은 가능하나 EXE·Shaders만 옮기는 독립 배포는 미지원이다. 출력 위치를 바꾸면 다시 빌드한다.

원본은 읽기 전용이다. 손상 이미지 검사는 구성별 EXE 옆 임시 파일, 모델 변조 검사는 시스템 임시 파일을 생성·정리한다.

### 셰이더

[CubeShaders.props](../Sandbox/CubeShaders.props)를 Sandbox·RendererSmoke·ImGuiSmoke가 공유한다. FXC·SM5.0·Main 진입점으로 컴파일하고 EXE 옆 `Shaders/CubeVS.cso`·`CubePS.cso`를 읽는다. Debug는 `/Zi /Od`, Release는 기본 최적화다. HLSLI·변형·캐시는 아직 없다.

### Git 관리

소스·HLSL·Tests/Assets·공급 기록은 추적한다. `Assets/` 전체와 `bin/`·`obj/`는 제외한다. 기준 모델 BIN과 대조용 JSON이 프로젝트에 표시되어도 Git에 포함된 것은 아니다. [모델 준비](static-model.md#기준-에셋-준비)를 따른다.

Engine 라이브러리·Samples·Tools·Cache 디렉터리는 실제 사용 수요가 생길 때 도입한다. [캐시 조건](dependencies.md#텍스처-변환과-캐시)과 [변경 검증](development.md#검증-방법)을 함께 적용한다.
