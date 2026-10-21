# 프로젝트와 공유 자원 구조

[프로젝트 소개와 실행](../README.md)

## 현재 프로젝트와 코드

2026-10-20 기준, [솔루션](../RnimanEngine.slnx)에는 대표 앱 하나와 자동 검증용 실행 프로젝트 두 개가 있다. 세 프로젝트는 x64 Debug·Release를 지원한다.

| 프로젝트 | 역할과 빌드 대상 |
|---|---|
| [Sandbox](../Sandbox/Sandbox.vcxproj) | 창·입력·카메라·큐브·렌더러를 연결하는 대표 데모. Sandbox 전용 코드·필요한 Engine 원본과 큐브 셰이더를 빌드 |
| [RendererSmoke](../Tests/RendererSmoke.vcxproj) | PNG CPU 로딩·여섯 면 텍스처·Phong 픽셀, 조명 조작·Transform·카메라 수학, 깊이 가림 픽셀, 리사이즈·GPU 동기화 검증. 실제 `D3D12Renderer.cpp`·`AssetLoader.cpp`와 큐브 셰이더를 함께 빌드 |
| [WindowSmoke](../Tests/WindowSmoke.vcxproj) | 창 수명·Win32 메시지·입력 소비·리사이즈 요청 검증. 실제 `Win32Window.cpp`를 함께 빌드하며 렌더러·셰이더는 사용하지 않음 |

Smoke 프로젝트는 테스트 전용 진입점을 가진 실행 파일이다. Sandbox가 참조하는 라이브러리가 아니며 앱 실행·배포에 포함할 필요가 없다. 솔루션 빌드가 테스트를 실행하지는 않는다. 실행 명령과 검사 범위는 [자동 검증](development.md#자동-검증)을 따른다.

```text
RnimanEngine/
├─ RnimanEngine.slnx
├─ Directory.Build.props         # 프로젝트별 출력·중간 산출물 경로
├─ Engine/                      # 공통 원본, 아직 별도 빌드 프로젝트 없음
│  ├─ Platform/Win32Window.h / .cpp
│  ├─ Assets/AssetLoader.h / .cpp
│  ├─ Assets/ImageData.h
│  ├─ Rendering/D3D12Renderer.h / .cpp
│  ├─ Rendering/RenderData.h
│  └─ Math/Transform.h
├─ Build/SourcePaths.props       # 앱·테스트 공용 EXE→원본 경로 설정
├─ Sandbox/
│  ├─ Sandbox.vcxproj / Sandbox.vcxproj.filters
│  ├─ Main.cpp                    # wWinMain 진입점
│  ├─ SandboxApp.h / .cpp         # 장면 구성·실행·수명
│  ├─ SandboxCamera.h            # 데모 카메라 조작
│  ├─ SandboxLighting.h          # 데모 방향광 조절
│  ├─ pch.h / .cpp               # Sandbox 전용 PCH
│  └─ CubeShaders.props          # 큐브 셰이더 빌드 설정
├─ Tests/
│  ├─ RendererSmoke.vcxproj / RendererSmoke.cpp / RendererReadback.h
│  ├─ LightingChecks.h / .cpp
│  ├─ Assets/TextureTest.png
│  └─ WindowSmoke.vcxproj / WindowSmoke.cpp
├─ Shaders/Samples/Sandbox/
│  ├─ CubeVS.hlsl
│  └─ CubePS.hlsl
├─ Assets/                       # 전체 Git 제외, 별도 준비하는 로컬 에셋
│  └─ BrickWall17_MR_1K/
└─ docs/
```

SandboxApp이 창 → CPU PNG 디코딩 → 렌더러·텍스처 업로드 순으로 초기화하고 렌더러 → 창 순으로 정리한다. Win32Window는 입력을 수집하고 앱이 프레임마다 소비하며, D3D12Renderer는 초기화 중 CPU RGBA8 이미지를 복사하고 GPU 완료까지 업로드 메모리를 유지하며, 전달받은 오브젝트 행렬·프레임 조명을 렌더 호출 중 복사한다. 앱이 카메라·조명 상태와 입력을 해석하고, RenderData가 GPU 자원 없이 WVP·World·역전치 법선 행렬을 만든다. 상세 진입점·데이터 흐름과 수명 제약은 [Architecture](design.md#진입점과-수명)에서 관리한다. Engine에는 창·CPU 이미지 로딩·렌더 데이터·D3D12 렌더링·Transform 원본을 모았다. Sandbox에는 진입점·장면 구성·카메라/조명 조작을 둔다. 별도 Engine 라이브러리와 Scene·Input 계층은 아직 없다. D3D12Renderer의 고정 큐브·Phong 파이프라인과 Win32Window의 Sandbox 창 이름·기본값은 현행 제약으로 유지하며 범용 API 분리는 실제 사용 수요가 생길 때 검토한다.

### 현재 include 기준과 허용 의존성

2026-10-20 [공통 구조 규칙](https://rniman.github.io/convention/coding/cpp/project-structure/#프로젝트-내부-파일-배치)에 따라 공통 소스 소속과 데모 소속을 구분했다. 실제 경로·빌드 입력·필터·include를 함께 맞췄으며 빌드 단위는 기존 세 실행 프로젝트를 유지한다.

| 대상 | 현재 탐색 기준·사용 범위 |
|---|---|
| Engine 헤더·구현 | `Engine/`를 include 기준으로 `"Rendering/RenderData.h"`처럼 사용. Sandbox 헤더·PCH를 참조하지 않으며 구현은 자기 헤더를 먼저 포함 |
| Sandbox | `Sandbox/`와 `Engine/`를 include 기준으로 사용. Main·SandboxApp·pch.cpp에만 Sandbox PCH를 사용하고, 직접 빌드하는 Engine 구현 3개는 PCH 비사용 |
| RendererSmoke | `Engine/`와 `Sandbox/`를 include 기준으로 사용. SandboxCamera·SandboxLighting은 데모 조작 계약 검증에만 참조. Engine의 D3D12Renderer.cpp·AssetLoader.cpp를 PCH 없이 직접 컴파일 |
| WindowSmoke | `Engine/`를 include 기준으로 사용. Engine의 Win32Window.cpp를 PCH 없이 직접 컴파일하며 Sandbox include 경로는 사용하지 않음 |

SandboxApp → Engine의 창·로더·렌더러·Transform 및 Sandbox 카메라·조명, 로더 → CPU ImageData·Windows/WIC, 렌더러 → ImageData·RenderData·EXE 경로 함수·D3D12의 의존성을 유지한다. 창은 GPU·카메라·테스트를 참조하지 않고, CPU 데이터·Transform은 앱이나 렌더러 구현을 참조하지 않는다. Engine에서 Sandbox·Tests 헤더로 향하는 include는 없다. RendererReadback은 검증용 friend로 렌더러 내부 큐·버퍼에 접근하며 Sandbox는 Tests 구현이나 실행 파일을 참조하지 않는다.

공유 구현은 Engine 원본 한곳을 참조한다. 현재는 별도 EXE마다 필요한 원본을 직접 컴파일하며 같은 EXE에 중복 등록하지 않는다. `../`는 프로젝트 입력·import 경로에만 사용하고 C++ include는 각 코드 루트 기준으로 작성한다. Sandbox 필터에는 실제 Engine 기능별 배치를 표시한다.

현재 헤더는 저장소 내부의 앱·테스트가 사용하는 코드이며 별도 배포 API 집합은 정하지 않았다. C++는 Engine·Sandbox·Tests 모두 `rengine` 단일 루트를 유지하고 폴더에 따라 네임스페이스를 추가하지 않는다. RendererReadback friend와 LightingChecks 전방 선언도 같은 루트다. 범위·예외는 [네임스페이스 규약](design.md#네임스페이스)을 따른다.

추후 라이브러리 분리 시 Engine의 빌드 단위와 소비자의 프로젝트 참조를 바꾼다. API·include 기준을 유지하면 Sandbox의 장면·조작 코드를 유지할 수 있지만, 초기화·자원 계약이 바뀌면 해당 호출부도 함께 수정한다. `src/`·`include/`·빈 모듈·공개 API 계층은 선행 도입하지 않는다. 적용·검증과 남은 경계는 [소스 소속 분리 기록](development.md#engine과-sandbox-소스-소속-분리-2026-10-20)을 따른다.

## 향후 확장 구조

2026-09-15에 정리한 확장 방향이며 Engine 원본 소속은 2026-10-20 현재 배치를 반영했다. **아래 트리는 목표 구조다.** 새 폴더와 공용 시스템은 실제 사용하는 기능을 추가할 때 만든다.

```text
RnimanEngine/
├─ RnimanEngine.slnx
├─ Engine/                        # 현재 공통 원본, 필요 시 라이브러리 프로젝트 추가
├─ Sandbox/                       # 대표 통합 데모·실행 프로젝트
├─ Tests/                         # 자동 검증용 실행 프로젝트
├─ Samples/                       # 기능별 실행 프로젝트 모음
│  └─ LightingDemo/               # 예시: 조명 기능 검증
├─ Tools/                         # Unity 익스포터 등 필요할 때 추가
├─ ThirdParty/                    # 외부 코드·버전·라이선스 정보
├─ Assets/
│  ├─ Shared/                     # 여러 앱이 공유하는 원본·기준 에셋
│  └─ Samples/<앱 이름>/          # Sandbox를 포함한 앱별 에셋
├─ Shaders/
│  ├─ Shared/                     # 공통 HLSL·HLSLI
│  └─ Samples/<앱 이름>/          # 앱별 셰이더 소스
├─ Cache/                         # 필요할 때 도입할 공용 변환 캐시
│  ├─ Textures/
│  └─ Shaders/
├─ bin/<프로젝트>/<구성>/         # Debug 또는 Release
│  ├─ 실행 파일·필요한 DLL
│  └─ Shaders/                    # 해당 앱 실행에 필요한 CSO
└─ obj/<프로젝트>/<구성>/         # OBJ·PCH 등 프로젝트별 중간 산출물
```

### 프로젝트 역할

- `Engine`은 현재 공통 원본을 관리한다. 별도 라이브러리로 분리할 때 이 위치에 프로젝트를 추가하고 앱·테스트의 직접 원본 빌드를 프로젝트 참조로 전환한다.
- `Sandbox`는 개발 단계의 기능을 연결해 보는 대표 통합 데모다. `Samples` 아래의 각 프로젝트는 조명·리소스 로딩 등 특정 기능을 사용하는 장면·입력·에셋을 구성한다.
- 각 샘플은 엔진을 사용하는 실행 맥락(Context)에 해당한다. `Samples` 자체는 실행 프로젝트를 모으는 폴더이며 별도 Context 객체나 시스템을 뜻하지 않는다. 샘플끼리 엔진 구현을 복사하지 않는다.

### 공유 자원과 실행 배치

**개발 실행은 원본을 공유하고 실행 파일·CSO는 앱과 구성별로 분리한다.** 현재 Sandbox는 루트 `Assets/`를, RendererSmoke는 `Tests/Assets/`를 직접 읽는다. 위 목표 구조의 `Assets/Shared`·`Assets/Samples`와 공용 셰이더 경로는 실제 공유 수요가 생길 때 도입한다. PNG의 Debug·Release별 복사본은 만들지 않는다.

| 대상 | 관리 기준 |
|---|---|
| 로컬 원본 `Assets/` | 저장소 루트 폴더 전체를 Git 제외. 실행에 필요한 파일은 별도 준비 |
| 테스트 기준 에셋, HLSL·HLSLI, 변환 설정 | Git 관리 대상. 테스트 이미지는 `Tests/Assets/`에 보관 |
| Unity 기준 `.bin`, 직접 준비한 DDS | 재현에 필요한 기준 에셋의 추적 위치·별도 보관 방식은 도입 시 결정. 확장자만으로 제외하지 않음 |
| 자동 생성 CSO·향후 DDS | CSO는 앱·구성별 `bin`에 생성. DDS 공용 재사용 결과는 도입 시 `Cache`에 배치 |
| OBJ·PCH·빌드 추적 파일 | `obj/<프로젝트>/<구성>`에 배치 |
| `bin/`, `obj/`, `Cache/` | 생성 결과로 취급하여 Git 제외. `Cache/` 제외 규칙은 캐시 도입과 함께 추가 |

원본은 소비자가 읽기만 하며 출력 폴더는 자동 생성물로 취급한다. 실행 파일을 원본 경로와 분리하여 배포하는 패키징은 별도 후속 작업이다. 향후 필요한 파일만 배포 폴더에 모으고 공용·앱별 동명 파일의 덮어쓰기를 막는 경로 규칙을 정한다. 엔진을 DLL로 분리하면 각 앱에 필요한 DLL도 해당 실행 출력에 배치한다.

런타임 원본 경로는 EXE 위치와 빌드가 지정한 저장소 경로를 합쳐 계산한다. 셰이더는 EXE 옆 `Shaders/`를 사용하며, 디버거 작업 디렉터리는 `$(OutDir)`다. 작업 디렉터리 설정만으로 직접 실행 시의 경로 처리를 대신하지 않는다.

<a name="개발-실행의-에셋-중복-축소-제안-2026-10-15"></a>

### 개발 실행의 공용 에셋 경로

2026-10-16 루트 `Assets/`를 직접 읽도록 적용했다. Sandbox의 `CopySandboxTexture`와 RendererSmoke의 `CopyTextureFixture`를 제거하고 원본 누락을 알리는 검증 타깃만 유지한다. 빌드 후에도 PNG를 출력에 배치하지 않는다. 이전 빌드의 복사본은 런타임에서 읽거나 갱신하지 않는다.

[SourcePaths.props](../Build/SourcePaths.props)는 두 프로젝트가 `OutDir` 설정 뒤에 import한다. `MSBuild::MakeRelative(OutDir, EngineRoot)`로 계산한 경로를 슬래시 표기로 바꾸고 `RNIMAN_SOURCE_ROOT` wide 문자열 매크로로 전달한다. 기본 출력에서는 `../../../`지만 C++ 코드에는 상위 폴더 개수를 고정하지 않는다. 같은 드라이브의 다른 출력 깊이에도 빌드 설정으로 대응하며, 서로 다른 드라이브에서는 MSBuild 결과가 절대 경로일 수 있다.

앱은 `GetExecutableDirectory()` 결과와 이 매크로·`Assets/`를 합쳐 정규화한다. 테스트는 `Tests/Assets/`를 합친다. 작업 디렉터리와 무관하며 개발자별 경로를 소스에 직접 적지 않는다. 런타임 경로 설정 파일과 상위 폴더 검색은 도입하지 않았다. 원본 누락·디코딩 실패 시 실제 경로·API·HRESULT를 보고하고 실패하며 EXE 옆 복사본으로 대체하지 않는다.

| 단계 | 상태와 관리 방식 |
|---|---|
| 현재 PNG 개발 실행 | 적용 완료. Sandbox는 루트 `Assets/`, RendererSmoke는 `Tests/Assets/`를 직접 읽음 |
| DDS 변환 도입 후 | 후속 계획. 원본 내용·변환 설정·도구 버전으로 구분한 공용 캐시를 앱·구성 간 재사용 |
| 배포 준비 | 미구현. 대상 앱의 에셋 목록으로 필요한 PNG 또는 변환 결과·CSO·DLL만 독립 배포 폴더에 수집 |

현재 실행은 빌드가 지정한 원본 위치를 요구한다. 기본 출력 구조를 유지하여 저장소 전체를 이동할 수 있지만 EXE·Shaders만 옮기는 독립 실행은 지원하지 않는다. 출력 경로를 바꾸면 다시 빌드한다. 실행 중 에셋 루트를 선택하거나 독립 배포를 구현할 때 명시적인 경로 설정을 검토한다.

공유 경로는 읽기 전용 소비를 전제로 한다. RendererSmoke의 손상 PNG는 구성별 테스트 EXE 옆 `CorruptTexture.png`에서 생성·삭제하며 `Tests/Assets/`에는 쓰지 않는다. 실행 중 사용자 파일·세이브·스크린샷 또는 변환 캐시를 도입할 때도 원본과 쓰기 위치를 분리한다.

출력 폴더는 자동 생성물로 취급하고 수동 수정하지 않는다. 이후 에셋 목록을 도입하면 같은 목록으로 의존성 검사·배포 파일 선별·불필요한 배포 결과 정리를 연결한다. 캐시 삭제는 재생성 가능 범위로 제한하며 원본과 변환 설정은 보존한다. 범용 가상 파일 시스템·에셋 DB는 이 흐름에 실제 필요가 생길 때 검토한다.

### 셰이더와 공용 캐시

셰이더는 빌드 시 컴파일하여 앱 출력의 `Shaders/`에 CSO를 배치한다. Debug·Release와 매크로 조합·진입점·대상에 따른 변형을 구분하고, HLSLI를 도입하면 include 변경 시 관련 결과도 다시 생성되는지 검증한다.

2026-09-24 큐브 데모에서는 [CubeShaders.props](../Sandbox/CubeShaders.props)를 Sandbox·RendererSmoke에서 함께 import한다. `Shaders/Samples/Sandbox/CubeVS.hlsl`과 `CubePS.hlsl`을 SDK FXC의 `vs_5_0`·`ps_5_0`, 진입점 `Main`으로 컴파일한다. 각 프로젝트의 `$(OutDir)Shaders/`에 `CubeVS.cso`·`CubePS.cso`를 직접 생성하여 출력 충돌을 피한다. Debug는 `/Zi /Od`, Release는 FXC 기본 최적화를 사용한다. 현재 HLSLI·매크로 변형·공용 캐시는 없다. FXC 선택은 초기 데모의 시험 적용이며 향후 Shader Model 6 기능 도입 시 DXC 전환을 검토한다.

공용 캐시는 여러 샘플에서 실제 변환 비용이 발생할 때 도입한다. 텍스처는 [텍스처 변환·캐시 규칙](dependencies.md#텍스처-변환과-캐시)을 따르고, 셰이더는 소스·include 내용, 매크로, 진입점, 대상과 컴파일러 버전·옵션을 재사용 조건에 포함한다. 동시 빌드가 같은 결과를 손상하지 않도록 완성된 결과만 공개하며, 캐시 구현을 먼저 범용 시스템으로 확장하지 않는다.

## 현재 설정과 적용 순서

1. Sandbox·RendererSmoke·WindowSmoke의 출력은 `bin/<프로젝트>/<구성>`, 중간 산출물은 `obj/<프로젝트>/<구성>`으로 적용했다. 세 프로젝트 모두 x64로 유지한다.
2. Sandbox 디버거 작업 디렉터리는 `$(OutDir)`로 적용했다. 큐브 CSO는 EXE 기준으로 로딩하며 Sandbox·RendererSmoke 빌드가 각 출력 폴더에 직접 생성한다. PNG는 SourcePaths.props의 원본 경로로 직접 읽고 출력에는 복사하지 않는다. 벽돌 원본 폴더는 Git 제외 대상이며 새 체크아웃에서는 별도로 준비한다.
3. 추가 프로젝트·에셋·셰이더는 해당 기능을 구현할 때 연결한다. 루트의 `Directory.Build.props`를 자동으로 읽어 저장소 기준 출력 경로를 사용한다. 프로젝트별 `..` 개수에 의존하지 않는다.
4. 출력 경로·파일 배치를 바꾸면 관련 문서와 제외 규칙을 맞추고, 영향받는 앱의 빌드 및 Visual Studio·EXE 직접 실행을 확인한다.

### 공통 경로 설정

[Directory.Build.props](../Directory.Build.props)의 위치를 `EngineRoot`로 사용한다. MSBuild가 프로젝트의 상위 폴더에서 이 파일을 자동 탐색하므로, 각 프로젝트에 `../` 경로를 적어 import할 필요가 없다.

- 출력: `$(EngineRoot)bin\$(MSBuildProjectName)\$(Configuration)\`
- 중간 산출물: `$(EngineRoot)obj\$(MSBuildProjectName)\$(Configuration)\`
- 프로젝트 이름은 `.vcxproj` 파일명에서 가져온다. 출력 충돌을 막기 위해 솔루션 안에서 파일명을 고유하게 사용한다.
- Sandbox 디버거 작업 폴더는 `$(OutDir)`다. 초기 속성 평가에서 아직 비어 있던 `$(TargetDir)` 대신 이미 정의된 출력 경로를 사용한다.
- 하위에 별도 `Directory.Build.props`를 만들면 가장 가까운 파일이 선택되므로 루트 설정을 명시적으로 이어서 import해야 한다. 새 프로젝트에 `OutDir`·`IntDir`를 개별 정의하면 공통값을 덮어쓸 수 있다.
- Directory.Build.props는 빌드 경로를 공통화한다. SourcePaths.props는 확정된 OutDir·EngineRoot를 바탕으로 런타임 원본 경로를 계산한다. C++ 파일의 include·빌드 입력 경로와는 별도 계약이다.

2026-09-15 검증: Debug·Release 빌드 성공. 루트보다 세 단계 아래에 둔 임시 프로젝트의 속성 평가에서도 루트의 `bin/PathProbe/Debug`, `obj/PathProbe/Debug`를 얻는 것을 확인했다. 검증용 프로젝트 파일은 제거했다.
