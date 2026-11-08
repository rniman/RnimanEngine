# 개발 설정과 검증

[프로젝트 소개와 실행](../README.md) · [Architecture](design.md) · [프로젝트 구조](project-structure.md) · [로드맵](roadmap.md)

개발 설정, 컨벤션 적용 범위와 테스트 실행 방법을 다룬다. 기능과 개발 순서는 [로드맵](roadmap.md)을 따른다.

## 컨벤션

작업 전 [컨벤션 위키 홈](https://rniman.github.io/convention/)에 접근해 현재 목차와 관련 문서를 확인한다. 공통 기준은 위키의 현재 [채택 규칙](https://rniman.github.io/convention/baseline/)과 [적용·근거 검토 절차](https://rniman.github.io/convention/baseline/#위키의-적용과-프로젝트-근거-검토)다. 필수·권장·선택과 채택·초안을 구분한다. 확인 커밋은 고정 기준이 아니다.

변경 범위의 실제 적용·선택 이유·예외·검증과 미적용 대상의 이유·후속 조건을 이 문서에 기록한다. 확인만 한 내용과 실제 적용을 구분하며, 위키 문서 갱신과 반영 상태 관리는 위키 정비 담당이 맡는다.

### 접근 절차

1. 위키 홈과 작업에 관련된 공개 문서를 조회한다. 과거 접근 실패는 현재 접속 상태의 근거로 사용하지 않는다.
2. 웹 조회 도구가 접근 오류를 반환하면 PowerShell의 `curl.exe -L --fail --max-time 30 https://rniman.github.io/convention/`으로 확인한다. 관련 문서도 같은 방식으로 조회하고 HTTP 상태뿐 아니라 본문을 읽는다.
3. 샌드박스에서 `Could not resolve host`가 발생하면 허용된 권한 상승 절차로 샌드박스 밖에서 같은 공개 URL을 재시도한다. 도구·샌드박스의 실패와 사이트 자체의 장애를 구분하며 실행 환경의 승인 정책을 따른다.
4. 재시도 후에도 접근하지 못한 경우에만 실패한 URL·방법·오류와 미확인 범위를 이번 작업 기록에 남긴다. 로컬 사본을 사용했다면 출처·시점을 명시하고 최신 공개본으로 취급하지 않는다. 공개본 조회에 성공하면 해당 문서의 로컬 사본을 기본 참고 자료로 사용하지 않는다.

### 공개본 적용 범위

2026-11-03 공개 홈·baseline·관련 C++ 문서를 조회했다. 10월 21일~11월 2일 변경은 공개본 접근 실패로 당시 로컬 보관본을 참고했으며, 전체 코드의 최신 규칙 대조를 뜻하지 않는다.

### 적용 상태

| 항목 | 실제 적용·예외와 선택 이유 |
|---|---|
| 명명·포맷·헤더 | 기존 명명·초기화·const·직접 include·한국어 API 주석·반환 계약·RAII를 유지한다. Engine·Sandbox·Tests의 직접 작성 C++는 제품 소속과 이름 충돌 방지를 위해 `rengine` 단일 루트를 사용한다. main·wWinMain은 전역, 외부 API·HLSL은 원래 이름을 유지한다. |
| 타입 명시 | 기존 적용 범위의 실패 결과·DirectX 데이터·경로·창 상태·D3D12 descriptor에 타입을 명시한다. WindowSmoke의 상태 참조·입력 복사본·optional 종료 코드, LightingChecks의 FrameLighting·std::size_t 픽셀 index도 포함한다. 범위 반복·기존 요소 참조·캐스트로 타입이 드러나는 포인터·chrono 시각·람다의 기존 auto 사용은 유지한다. |
| 반환값 진단 | [반환값 규칙](https://rniman.github.io/convention/coding/cpp/functions/#반환값-무시와-nodiscard)에 따라 실패 후 검증을 중단해야 하는 RendererReadback::Capture에 `[[nodiscard]]`를 적용했다. 상태·자원을 변경하지 않는 행렬 계산인 Transform::GetWorld·SandboxCamera::GetView에는 적용하지 않는다. |
| 소스 소속·의존성 | 공통 원본은 Engine, 데모 실행·조작은 Sandbox에 둔다. 네 실행 프로젝트가 필요한 원본을 직접 빌드하며, 별도 라이브러리·공개 헤더 계층은 실제 재사용 수요가 생길 때 검토한다. [include 기준과 허용 의존성](project-structure.md#현재-include-기준과-허용-의존성)을 따른다. |
| 프로젝트 한정 채택 | 창·렌더러 소유 분리, 하드웨어 우선·WARP 자동 대체 없음, 프레임 경계 리사이즈·슬롯 재사용 전 Fence 대기를 유지한다. 공용 PNG는 복사 없이 원본에서 읽고 경로 계산은 Build/SourcePaths.props가 맡는다. [실행 계약](design.md#sandbox-실행-구조)과 [에셋 경로](project-structure.md#개발-실행의-공용-에셋-경로)를 따른다. |
| 시험 적용 | WIC·STA·RGBA8, sRGB·단일 밉·선형 필터·모델별 축별 Repeat/Clamp/Mirror, ComPtr·Event 수명·bool/HRESULT 진단, Debug 레이어·두 백 버퍼·VSync, FXC·SM5.0·D32·Transform·카메라·Phong·테스트 readback. 유지·변경 조건은 [구현 선택과 후속 조건](design.md#구현-선택과-후속-조건)을 따른다. |
| 예외·미정 | Sandbox 전용 구현은 필요 시 pch.h를 먼저 포함하고 Engine은 PCH를 사용하지 않는다. Windows→DirectX include 순서·긴 오류 문자열 예외와 Sandbox·Tests의 평면 배치를 유지한다. 모델별 의미상 정면·거리 단위는 에셋 계약에 두고, 비정방 행렬·행렬 배열은 별도 패킹 검증이 필요하다. |
| ImGui 시험 적용 | SandboxGui 단일 문맥·전용 64칸 SRV 힙·GPU 대기 후 종료, Win32 메시지 콜백·선택 OverlayPass를 실제 Sandbox·ImGuiSmoke에 적용했다. 장면/Engine의 ImGui 의존성과 범용 UI/descriptor 관리자는 추가하지 않았다. 외부 소스는 원본·공급 구조·MIT를 유지하고 해당 두 타깃에서만 PCH 없이 `/W3`·`/WX` 해제로 컴파일한다. 자체 C++는 기존 `rengine`·직접 include·RAII를 적용한다. 사용자 텍스처/여러 도구의 공유 수요가 생기면 descriptor 할당을 공통화할 후보로 재검토한다. |
| Math | 헤더 전용·직접 include·출력 값 초기화·실패 계약 주석을 적용한다. 실패 가능한 연산은 Try 접두사 없이 optional로 결과를 전달하는 프로젝트 규칙을 사용한다. 공용 연산의 선행 추가는 사용처 우선 원칙의 예외이며 Sandbox 연산을 일괄 교체하지 않는다. SandboxRotation은 Quaternion과 Euler 편집값의 일관성을 위해 비공개 상태를 사용한다. |
| M03 식별·저장 | App의 vector 값 소유와 optional 선택 ID를 적용했다. 작은 목록은 선형 조회하며 반환 포인터의 추가·삭제 전 수명을 명시한다. 별도 관리자·검색 맵은 도입하지 않고 빈번한 조회·재사용 수요가 생길 때 검토한다. |
| 공통 초안 | 자료형·변환·attribute 전체, 클래스 미결정 정책, HLSL·검증·원본/생성물 규칙은 공통 채택으로 확대하지 않는다. 포맷터·경고 오류화·캐시는 필요와 검증 조건이 생길 때 검토한다. |

### 모델 로딩 적용

ReferenceImporter v2는 공급자 네임스페이스·예외 API를 유지하고 PCH 없이 빌드한다. 사본의 API 수명 주석·초기화·ID 검증 보완과 해시는 [공급 기록](../ThirdParty/ReferenceImporter/README.md)에 둔다. ModelLoader는 CPU 값 소유·오류 결과·직접 include를 적용하고 GPU 업로드는 렌더러에 남겼다. 기존 Transform 검증을 재사용하며 외부 파서 타입을 엔진 공개 헤더에 노출하지 않는다.

TextureWrapMode를 CPU 이미지 계약에 두고 BaseColor 바인딩의 U/V를 값으로 전달한다. SM5.0의 동적 Sampler 인덱싱 대신 9개 Static Sampler를 선택한다. 작은 정적 메시의 Upload Heap 유지·모델별 텍스처 복사는 현재 데모 한정 선택이다. 큰 메시·스트리밍·실제 공유 수요가 생기면 Default Heap 업로드와 캐시를 검토한다. MirrorOnce·Filter 전달·다중 재질·PBR은 미적용이다.

## 프로젝트 설정

개발 환경·빌드·실행·에셋 준비는 [README](../README.md#빌드와-실행)를 따른다. 네 실행 프로젝트와 Math Utility는 x64 Debug·Release를 지원한다. 아래 컴파일·링크 설정은 실행 프로젝트 기준이며, Math Utility에는 헤더 분석용 C++20·Engine include 경로·`/utf-8`을 지정한다.

| 항목 | 설정 |
|---|---|
| C++ | C++20, Unicode, `/utf-8`, `/W4`, `/permissive-`, `/EHsc`; Debug `/MDd`, Release `/MD` |
| 경고 | Sandbox·RendererSmoke·ImGuiSmoke는 `/WX` 끔, WindowSmoke는 `/WX` |
| 출력 | [Directory.Build.props](../Directory.Build.props): `bin/<프로젝트>/<구성>/`, `obj/<프로젝트>/<구성>/`. Sandbox 디버거 작업 디렉터리는 `$(OutDir)` |
| 연결 | Sandbox·RendererSmoke·ImGuiSmoke: d3d12·dxgi·d3dcompiler·windowscodecs·ole32. WindowSmoke는 D3D12를 연결하지 않음 |
| 헤더·PCH | `WIN32_LEAN_AND_MEAN`, `NOMINMAX`. Sandbox PCH는 `$(IntDir)$(ProjectName).pch`; Engine·SandboxGui·Smoke·ImGui 원본은 PCH 비사용. 일반 헤더는 필요한 의존성을 직접 포함 |
| 셰이더 | [CubeShaders.props](../Sandbox/CubeShaders.props): FXC·SM5.0·진입점 Main, EXE 옆 `Shaders/`에 CSO 생성·로딩 |
| 파일·Git | [.editorconfig](../.editorconfig): UTF-8 BOM 없음·LF·끝 개행·C++ 탭 너비 4. [.gitattributes](../.gitattributes): 텍스트 LF·Windows 스크립트 CRLF. [.gitignore](../.gitignore): 출력·개인 캐시·Assets 전체 제외, Tests/Assets 추적 |

## 검증 방법

변경한 기능과 영향받는 대상만 검증한다. 평소에는 작업 중인 구성(기본 Debug)으로 빌드·실행하고, 공통 렌더링 변경·구성별 동작 변경·마일스톤에서는 영향받는 대상의 Debug·Release를 검증한다. 공통 코드 변경 시 대표 데모 Sandbox도 실행한다. 문서만 바꾸면 내용·링크를 확인하며, 실제 테스트 파일을 재배치하면 프로젝트 입력·include·필터와 영향받는 타깃 빌드를 확인한다. 솔루션 표시만 바꾸면 프로젝트 등록과 경로를 확인한다.

헤더 의존성 변경은 PCH·강제 include 없는 독립 컴파일, 행렬·셰이더 경계 변경은 GPU 픽셀 검증을 포함한다. 빌드·실행·수동 화면·성능 확인을 구분하고 수행하지 못한 범위와 이유를 남긴다.

### 자동 검증

솔루션 빌드는 테스트를 실행하지 않는다. 새 Smoke 프로젝트 추가는 보류하고 기존 프로젝트에 필요한 회귀 검사만 추가한다. Tests의 코드·프로젝트·고정 fixture는 Git에 유지한다. UI 배치·조작감은 수동 확인을 기본으로 하며, 실제 결함에 대한 회귀 필요가 있을 때 자동 검사를 보강한다.

저장소 루트의 Developer PowerShell에서 영향받는 프로젝트를 빌드하고 실행 종료 코드를 확인한다.

```powershell
MSBuild.exe Tests/RendererSmoke.vcxproj -nologo -t:Build -p:Configuration=Debug -p:Platform=x64
./bin/RendererSmoke/Debug/RendererSmoke.exe
# Math·CPU 로더 변경: GPU 초기화 없이 회귀 검사
./bin/RendererSmoke/Debug/RendererSmoke.exe --cpu
# 렌더링·셰이더·모델 통합 변경 또는 마일스톤: 기존 전체 검사
./bin/RendererSmoke/Debug/RendererSmoke.exe --full
```

Release 검증이 필요하면 빌드의 Configuration과 실행 경로를 모두 Release로 바꾼다. `--help`는 실행 범위를 표시한다. 옵션은 하나만 허용하며 잘못된 옵션은 종료 코드 2, 검사 실패는 1이다. 선택하지 않은 범위는 통과한 것으로 보고하지 않는다. `--full`의 에셋 누락은 실패이며 자동으로 건너뛰지 않는다.

| 대상 | 실행 시점·검사 범위 | 실행 조건 |
|---|---|---|
| [RendererSmoke](../Tests/RendererSmoke.cpp) 기본 | CPU 회귀 + 초기화·10프레임·2회 리사이즈 요청·대기·재개·소멸·D3D12 진단 | 저장소 PNG·JPEG·고정 모델 fixture, FL11_0 하드웨어·Graphics Tools·Cube CSO. 실제 모델 3개 불필요 |
| RendererSmoke `--cpu` | Math·Transform·카메라·조명 데이터·이미지 디코딩·고정 모델 TRS/손상·이전 포맷 거부 회귀 | 저장소 fixture·Win32/WIC. GPU·Graphics Tools·CSO 실행 의존성 없음. 기존 GPU 소스의 빌드·링크는 유지 |
| RendererSmoke `--full` | 기본 CPU 검사 + UV/sRGB/alpha·Phong·법선·가림·모델·9개 Wrap·136프레임·8회 리사이즈 요청 | 기본 실행 조건 + 별도 준비한 세 기준 모델 bin. 외부 BaseColor 텍스처 불필요 |
| [WindowSmoke](../Tests/WindowSmoke.cpp) | Win32 창·입력 변경 시 키·마우스·포커스·최소화·종료 회귀 | Win32. D3D12·Graphics Tools·CSO 불필요 |
| [ImGuiSmoke](../Tests/ImGuiSmoke.cpp) | ImGui 연결·입력 캡처·descriptor·렌더링 연동 변경 시 기존 UI 픽셀·폰트·리사이즈·소멸 검사 | FL11_0 하드웨어·Graphics Tools·Cube CSO, 메모리 텍스처 |
| Sandbox | 공통 코드 변경의 대표 흐름과 UI 변경의 실제 조작·화면 확인 | 세 모델 bin·BaseColor 이미지·CSO·D3D12 하드웨어. Graphics Tools는 Debug 필수 |

WindowSmoke·ImGuiSmoke는 각각 `Tests/WindowSmoke.vcxproj`·`Tests/ImGuiSmoke.vcxproj`를 같은 구성으로 빌드하고 `bin/<프로젝트>/<구성>/<프로젝트>.exe`를 실행한다. 두 테스트의 기존 검사 범위는 유지한다.

### GPU 픽셀 검증

[RendererReadback](../Tests/RendererReadback.h)은 Present 없이 그린 뒤 GPU 대기·복사·Map을 거쳐 RowPitch 패딩을 제외한 RGBA8을 읽는다. flip-discard 이후 내용에 의존하지 않으며 Capture 실패 시 검증을 중단한다. 어댑터는 GPU 완료 후 렌더러보다 먼저 파괴한다.

- `CheckOcclusion`: 세 카메라/해상도에서 앞·뒤 단독 및 그리기 순서를 비교한다. 겹침·색 차이·뒤 가시 영역이 각각 100픽셀 이상이고, 결과가 깊이와 일치해야 한다.
- [CheckPhongLighting](../Tests/LightingChecks.cpp): 독립적인 sRGB 기대값과 비균일 Scale·회전·전단으로 조명과 역전치 법선을 검증한다. 텍스처·가림 검사는 Ambient=1·직접광=0을 사용한다.
- [3×3 기준 PNG](../Tests/Assets/TextureTest.png): 여섯 면의 UV·채널·행 패딩·sRGB·출력 alpha를 비교한다. RGB 허용 오차는 텍스처 1, 조명 2다.

## 검증 기록

| 대상·시점 | 결과·범위 |
|---|---|
| Math·Quaternion (2026-11-02~03) | Debug·Release 솔루션, RendererSmoke·ImGuiSmoke 통과. Math 헤더 독립 컴파일·TRS 극점 회귀 포함. Sandbox는 숨김 실행·합성 입력·리사이즈·정상 종료, WindowSmoke는 빌드만 수행 |
| RendererSmoke 실행 범위 (2026-11-03) | Debug 기본·`--cpu`·`--full` 종료 코드 0, GPU 오류·경고 0. Release는 실행 범위 변경 후 재실행하지 않음 |
| M02 회전 UI (2026-11-03) | 기존 Debug·Release 실행 파일의 Model 0에서 370° 유지, ±90° 경계 편집, 자동 회전 정지·재개. UI 자동화·화면 관찰이며 재빌드·D3D12 진단 수집 없음 |
| M02 수동 조작 (2026-11-03) | 세 모델 표시·선택·Transform·카메라·조명·입력 차단·포커스·리사이즈·종료 이상 없음. 실행 구성 미기록 |
| M03 (2026-11-04~05) | Debug·Release 빌드·RendererSmoke 전체·ImGuiSmoke 통과. 두 프레임 슬롯의 0개·증가·감소 픽셀 비교 포함. Debug UI에서 생성·삭제·ID 보존·독립 편집·리사이즈·종료 관찰. Release UI·실패 주입은 미수행 |

메모리 부족·장치 제거 주입, 전체 live-object 검사와 성능 측정은 미수행이다. 이전 마일스톤은 [v0.1.0 기록](milestones/v0.1.0-sandbox.md)을 참고한다.
