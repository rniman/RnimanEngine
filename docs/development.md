# 개발 설정과 검증

[프로젝트 소개와 실행](../README.md) · [Architecture](design.md) · [프로젝트 구조](project-structure.md) · [로드맵](roadmap.md)

현재 컨벤션 적용 상태와 개발 설정·검증 방법을 먼저 설명하고, 변경 당시 근거는 마지막 검증 기록에 보관한다. 기능 계획은 [로드맵](roadmap.md), 달성 결과와 당시 제약은 마일스톤 문서에서 관리한다.

## 컨벤션

작업 전 [컨벤션 위키 홈](https://rniman.github.io/convention/)에 접근해 현재 목차와 관련 문서를 확인한다. 공통 기준은 위키의 현재 [채택 규칙](https://rniman.github.io/convention/baseline/)과 [적용·근거 검토 절차](https://rniman.github.io/convention/baseline/#위키의-적용과-프로젝트-근거-검토)다. 필수·권장·선택과 채택·초안을 구분한다. 확인 커밋은 고정 기준이 아니다.

변경 범위의 실제 적용·선택 이유·예외·검증과 미적용 대상의 이유·후속 조건을 이 문서에 기록한다. 확인만 한 내용과 실제 적용을 구분하며, 위키 문서 갱신과 반영 상태 관리는 위키 정비 담당이 맡는다.

### 접근 절차

1. 위키 홈과 작업에 관련된 공개 문서를 조회한다. 과거 접근 실패는 현재 접속 상태의 근거로 사용하지 않는다.
2. 웹 조회 도구가 접근 오류를 반환하면 PowerShell의 `curl.exe -L --fail --max-time 30 https://rniman.github.io/convention/`으로 확인한다. 관련 문서도 같은 방식으로 조회하고 HTTP 상태뿐 아니라 본문을 읽는다.
3. 샌드박스에서 `Could not resolve host`가 발생하면 허용된 권한 상승 절차로 샌드박스 밖에서 같은 공개 URL을 재시도한다. 도구·샌드박스의 실패와 사이트 자체의 장애를 구분하며 실행 환경의 승인 정책을 따른다.
4. 재시도 후에도 접근하지 못한 경우에만 실패한 URL·방법·오류와 미확인 범위를 이번 작업 기록에 남긴다. 로컬 사본을 사용했다면 출처·시점을 명시하고 최신 공개본으로 취급하지 않는다. 공개본 조회에 성공하면 해당 문서의 로컬 사본을 기본 참고 자료로 사용하지 않는다.

### 현재 확인 범위 (2026-11-03)

웹 조회 도구는 홈·baseline 접근 오류를 반환했고 샌드박스 내부의 curl은 DNS 해석에 실패했다. 샌드박스 밖의 curl 재시도에서는 두 URL 모두 HTTP 200과 HTML 본문 수신을 확인했으며 baseline의 현재 채택 범위·프로젝트 적용 절차를 읽었다. 따라서 이번 확인에서 사이트 접속은 가능하며, 기존의 접근 복구 대기 상태를 해제한다. 전체 채택 문서와 엔진 코드의 재대조를 완료했다는 의미는 아니다. 이후 작업에서도 관련 공개 문서를 새로 확인한다.

이번 변경은 홈 진입점과 접근 절차를 AGENTS·README·이 문서에 적용한 문서 변경이다. 공통 규칙·엔진 코드·위키는 변경하지 않았으며 빌드·실행은 수행하지 않았다.

### 과거 확인 범위

2026-10-20에 공개 baseline·채택 문서 14개·관련 초안 6개·프로젝트 참조 문서를 코드·설정과 대조했다(당시 위키 원본 `4beaa82`). 10월 21일~11월 2일 작업에서는 공개본 접근 오류로 최신본 일치를 확인하지 못해 컨벤션 위키 로컬 사본의 baseline·관련 채택 규칙을 임시 참고했다. 이는 당시 적용의 검증 한계이며 현재 접속 실패나 로컬 사본 사용을 지시하는 내용이 아니다. 초안·미결정 사항은 공통 채택으로 확대하지 않는다.

임시 참고 자료는 `D:/DevelopTool/ModelExporter/Exporter/Logs/`의 `convention-baseline.txt`, `convention-coding-cpp-functions.txt`, `convention-coding-cpp-headers.txt`, `convention-coding-cpp-project-structure.txt`다. 아래 변경은 기존 규칙의 프로젝트 적용이며 신규 공통 규칙 채택·위키 수정은 없다.

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
| [RendererSmoke](../Tests/RendererSmoke.cpp) | [MathChecks](math.md#mathchecks-검증)의 CPU 연산·경계 검사, PNG/JPEG·다중 모델 선택·UV/sRGB/alpha·Phong·법선·Transform/카메라 수학, 가림·리사이즈·동기화·정리 | 두 구성 모두 D3D12 FL11_0 하드웨어·Graphics Tools·Cube CSO 필요. 저장소의 Tests/Assets/TextureTest.png·ColorTest.jpg와 세 기준 모델 bin 사용, 외부 텍스처 불필요 |
| [WindowSmoke](../Tests/WindowSmoke.cpp) | 키·마우스 소비, 캡처/포커스/모달 초기화, 최소화·리사이즈·종료·창 수명 | Win32 환경. D3D12·Graphics Tools·CSO 불필요 |
| [ImGuiSmoke](../Tests/ImGuiSmoke.cpp) | ImGui 초기화·UI 픽셀·폰트 크기 변경·리사이즈·소멸·D3D12 진단 | 두 구성 모두 FL11_0 하드웨어·Graphics Tools·Cube CSO 필요. 외부 PNG 불필요 |
| Sandbox | 실제 화면·장치 입력·창 조작의 통합 흐름 | 세 모델 bin·벽돌 PNG·Helmet BaseColor JPEG·CSO·D3D12 하드웨어 필요. Graphics Tools는 Debug에 필수. WARP 대체 없음 |

### GPU 픽셀 검증

[RendererReadback](../Tests/RendererReadback.h)은 Present 없이 그린 뒤 GPU 대기·복사·Map을 거쳐 RowPitch 패딩을 제외한 RGBA8을 읽는다. flip-discard 이후 내용에 의존하지 않으며 Capture 실패 시 검증을 중단한다. 어댑터는 GPU 완료 후 렌더러보다 먼저 파괴한다.

- `CheckOcclusion`: 세 카메라/해상도에서 앞·뒤 단독 및 그리기 순서를 비교한다. 겹침·색 차이·뒤 가시 영역이 각각 100픽셀 이상이고, 결과가 깊이와 일치해야 한다.
- [CheckPhongLighting](../Tests/LightingChecks.cpp): 독립적인 sRGB 기대값과 비균일 Scale·회전·전단으로 조명과 역전치 법선을 검증한다. 텍스처·가림 검사는 Ambient=1·직접광=0을 사용한다.
- [3×3 기준 PNG](../Tests/Assets/TextureTest.png): 여섯 면의 UV·채널·행 패딩·sRGB·출력 alpha를 비교한다. RGB 허용 오차는 텍스처 1, 조명 2다.

## 검증 기록

기록은 수행 당시의 코드와 환경에 대한 결과다. 빌드·자동 실행·수동 화면 확인을 구분하며, 이후 변경이나 문서 정리가 과거 결과를 재검증한 것은 아니다. `obj/`의 로그·임시 검사는 Git 제외 산출물이므로 다른 환경에 없을 수 있다.

### Quaternion 전환과 후속 검증 (2026-11-02~03)

Transform을 단위 Quaternion 저장으로 바꿔 로딩의 Euler 복원 경로를 제거했다. 원본 행렬과 TRS 재구성 비교는 유지하며 Binary v2·에셋 재익스포트는 필요 없다. Sandbox는 모델별 Euler degree 편집값을 유지하고 자동 회전은 월드 고정 축의 Quaternion 증분으로 합성한다. 현재 계약과 표시 한계는 [Quaternion 저장과 Euler 편집](math.md#quaternion-저장과-euler-편집)에 둔다.

컨벤션은 11월 2일 로컬 보관본 참고 후, 11월 3일 접근 절차에 따른 샌드박스 밖 curl로 공개 홈·baseline·관련 C++ 채택 문서를 다시 확인했다. 직접 include·그룹 내 정렬, 동사형 ConvertQuaternionToEuler, 테스트 상태 변경의 단계별 검사를 적용했다. SandboxRotation은 두 캐시의 일관성을 위해 class의 비공개 상태로 바꾸고 GetDegrees·SetDegrees·Synchronize로 접근한다. 복사·이동은 값 멤버의 기본 동작을 유지한다. 기존 Transform의 공개 값 타입과 QuaternionFromEuler 등 기존 API 이름은 이번 정리에서 제외하며 해당 API 개정 시 별도로 검토한다. 위키·공통 규칙 변경은 없다.

| 단계 | 당시 검증 결과·근거 | 한계 |
|---|---|---|
| Quaternion 전환 (11월 2일) | Debug·Release 솔루션·RendererSmoke·ImGuiSmoke 통과(오류·경고 0). fixture 로딩의 ±90도·±0.001 인접 각도, MathChecks의 ±0.000001·Euler 왕복·전단/특이/반전 거부·370° 유지·외부 20°→380° 동기화·89°→91° 분기 검사 통과. Transform·TransformMath·QuaternionMath·SandboxRotation의 PCH 없는 C++20 `/W4 /WX /Zs` 검사 통과. `obj/quaternion-*.log` | 당시 UI 위젯의 실제 입력 검사는 미수행. 이후 발견한 Clamp 문제는 다음 행에서 수정 |
| 회전 입력 Clamp 수정 (11월 2일) | `(0,0)` 범위에 AlwaysClamp의 ClampZeroRange가 적용되어 회전을 0으로 고정하던 문제 수정. 유효 범위가 있을 때만 Clamp를 적용. 실제 EditVector3를 추출한 ImGui 문맥에서 수정 전 드래그·370 입력은 0, 수정 후 드래그 50·텍스트 370 확인. Debug·Release Sandbox 빌드 통과 | `obj/rotation-widget-before.cpp`·`obj/rotation-widget-check.cpp`는 임시 검사이며 유지 테스트 프로젝트가 아님. 이 단계에서는 공통 Math 변경이 없어 RendererSmoke 반복 없음 |
| 컨벤션 보완 (11월 3일) | Debug·Release 솔루션·RendererSmoke·ImGuiSmoke 통과. QuaternionMath·TransformMath·SandboxRotation 독립 헤더 컴파일, 로컬 링크·앵커 155개·공백 검사 통과. `obj/quaternion-convention-*.log` | 기존 API의 일괄 명명 변경은 범위 밖 |

각 단계에서 두 구성의 Sandbox를 실제 에셋으로 숨김 실행해 합성 입력·2회 리사이즈·정상 종료를 확인했다. WindowSmoke는 솔루션 빌드만 수행했다. 실제 창에서의 선택 전환·회전 편집·자동 회전 재개와 물리 입력·시각 확인은 미수행이며 M02 수동 검증은 남아 있다. 아래 Math 분리 기록의 미해결 TRS 설명은 그 작업 당시의 상태다.

### Math 적용과 검증 (2026-11-02)

Math는 헤더 전용 모듈이며 x64 Debug·Release의 Utility 프로젝트에서 관리한다. 기존 DirectXMath 타입·rengine·inline·직접 include·한국어 계약 주석을 유지한다. 별도 cpp·lib·MathTypes 래퍼·MathTests·ProjectReference는 추가하지 않았다. 소비 프로젝트의 중복 헤더 표시와 Sandbox의 빈 Math 필터는 제거했다.

- **책임 분리:** TransformMath에 기존 행렬 검증·역전치·TRS 분해를 옮겼다. ModelLoader는 Math의 결과를 기존 오류 전달에 연결하고 Rendering 의존성을 제거했다. RenderData는 역전치를 받아 WVP·셰이더 상수를 구성한다. 기존 계산식·오차·진단은 유지했으므로 [90도 근처 TRS 결함](static-model.md#계층-지원-범위와-알려진-문제)은 남아 있다.
- **연산 확장:** 사용자 요청으로 MathUtils·VectorMath·MatrixMath·QuaternionMath를 사전 추가했다. 사용처 우선 원칙의 이번 요청 범위 예외이며 Sandbox 연산은 일괄 교체하지 않았다. 새 연산은 기존 RendererSmoke의 MathChecks에 연결했다. 실패 전달·수치 범위·회전 순서는 [Math 계약](math.md)을 따른다. Quaternion→Euler·조건수 검증·자체 벡터 타입은 실제 요구와 별도 검증이 생길 때 검토한다.
- **편집 문맥:** Math Utility에도 C++20·`$(EngineRoot)Engine` include 경로·`/utf-8`과 옵션 상속을 지정했다. Utility 자체는 소스를 컴파일하지 않으므로 타깃 빌드 성공과 헤더 검증은 구분한다.

아래는 각 변경 당시 수행한 결과다. 뒤 단계에서 실행하지 않은 검사를 다시 통과한 것으로 취급하지 않는다.

| 변경 단계 | 검증 결과·근거 | 미수행·한계 |
|---|---|---|
| 기존 Math 분리 | Debug·Release 솔루션 빌드, RendererSmoke·ImGuiSmoke 통과(오류·경고 0). PCH 없는 C++20 `/W4 /WX` 헤더 단독·두 번역 단위 링크 통과. 기존 TRS와 36개 입력의 성공/거부·성공 행렬 일치, 전단 역전치 허용·TRS 거부 확인. `obj/math-module-check/`, `obj/math-*.log` | WindowSmoke는 빌드만 수행. 기존 동작 보존 검사이며 TRS 수정 완료가 아님 |
| Utility 등록 | Debug·Release 솔루션 빌드, XML·등록 경로·공백 검사 통과. `obj/math-project-debug.log`, `obj/math-project-release.log` | 구성 변경으로 실행 반복 없음. Utility 성공은 헤더 컴파일 검증이 아님 |
| 공용 연산·MathChecks 추가 | Debug·Release 솔루션 빌드, RendererSmoke의 CPU·기존 GPU 검사 통과(오류·경고 0). 신규 네 헤더를 PCH 없이 C++20 `/Zs /W4 /WX`로 단독 검사하고 여러 번역 단위 링크·실행 통과. `obj/math-utils-check/`, `obj/math-utils-*.log`. 로컬 링크·앵커 146개와 프로젝트 등록 검사 통과 | ImGuiSmoke·WindowSmoke는 빌드만 수행 |
| 헤더 분석 설정 | 두 구성에 임시 ClCompile 항목을 주입한 MSBuild 평가로 stdcpp20·Engine 절대 경로·/utf-8 확인. 저장 프로젝트에는 임시 항목 없음. 주입 없는 Utility 빌드 두 구성·공백 검사 통과 | 실제 IDE 오류 밑줄·자동 완성 미확인. 런타임 변경이 없어 솔루션 전체 빌드·실행 반복 없음 |

Math 분리와 공용 연산 추가 단계에서는 두 구성의 Sandbox를 실제 에셋·TEMP 작업 디렉터리에서 숨김 실행해 합성 입력·2회 리사이즈·정상 종료를 확인했다. 수동 화면·물리 입력은 확인하지 않았다. 임시 독립 검사 파일은 검증 근거이며 유지되는 별도 테스트 프로젝트가 아니다.

### Math 명명 정리 (2026-11-02)

사용자 요청으로 Math의 실패 가능한 연산에서 Try 접두사를 제거했다. 연산 자체를 이름으로 드러내며 `std::optional`·`[[nodiscard]]`·API 주석의 실패 계약은 유지한다. Math 헤더·RenderData 호출부·MathChecks·관련 문서에 적용했고 다른 모듈의 명명 규칙으로 확대하지 않는다. 각도 변환 래퍼 두 개는 제거하고 호출부를 DirectXMath의 XMConvertToRadians·XMConvertToDegrees로 통일했다. LerpClamped 등 검증·범위 정책이 있는 함수는 유지한다. 기존 TRS 계산식은 변경하지 않았다. 공개 위키 최신본 미확인 범위는 위 확인 범위를 따르며 이번 명명은 사용자 지정 프로젝트 규칙이다.

Debug·Release 솔루션 빌드, RendererSmoke의 Math CPU·GPU 검사, ImGuiSmoke 통과(오류·경고 0). Sandbox는 두 구성의 실제 에셋 숨김 실행·합성 조명 입력·2회 리사이즈·WM_CLOSE 종료 코드 0을 확인했다. WindowSmoke는 빌드만 수행했고 수동 화면·물리 입력은 확인하지 않았다. 빌드 로그는 `obj/math-naming-Debug.log`·`obj/math-naming-Release.log`다.

### Math 컨벤션 보완 (2026-11-02)

Math 신규 헤더의 여러 줄 매개변수·호출 인자를 첫 인자부터 한 줄씩 배치하고 긴 식과 실패 처리 뒤 빈 줄을 정리했다. DirectXMath 출력 변수는 `{}`로 초기화하고 공개 API의 0 입력·비유한 값·정규화 실패에 대한 nullopt 계약을 보완했다. 계산식·반환형·TRS 허용 범위는 유지한다. 공개 위키 접근 오류·DNS 실패로 최신본은 확인하지 못했으며 기존 로컬 보관본의 formatting·initialization·comments 채택 규칙을 적용했다. 공통 규칙 개정이나 위키 수정은 없다.

Debug·Release 최종 솔루션 빌드·RendererSmoke(Math CPU 포함)·ImGuiSmoke 통과. 두 구성 Sandbox의 실제 에셋 숨김 실행·합성 입력·2회 리사이즈·정상 종료를 확인했다. WindowSmoke는 빌드만, 수동 화면·물리 입력은 미확인이다. 로그는 `obj/math-style-final-*.log`, `obj/math-style-renderer-*.log`, `obj/math-style-imgui-*.log`다. Math 헤더의 탭 4칸 기준 120열과 공백 검사도 확인했다.

후속으로 MathChecks.cpp의 여러 줄 Check·중첩 호출 인자와 실패 처리 뒤 빈 줄을 같은 기준으로 정리했다. 변경 전후 문자열을 포함한 토큰 일치, 탭 4칸 기준 120열 이내, 공백 검사를 확인했다. 공백·줄바꿈만 변경하여 빌드·실행은 반복하지 않았다.

### 모델 로더·fixture 변경 (2026-10-30)

| 변경 | 적용 이유·범위 | 당시 검증과 한계 |
|---|---|---|
| ReferenceImporter 재질 조회 통합 | ID→배열 위치 단일 테이블·UV/접선 순회 통합·조회 예산 64바이트를 사본에 반영. 기존 ID 검증·초기화·수명 주석 유지, 공급 해시 갱신. 원본의 Submesh.index 제거는 미반영. BIN v2·재익스포트 조건은 동일 | Debug·Release 솔루션·RendererSmoke 통과(오류·경고 0), 실제 에셋 Sandbox 숨김 실행·리사이즈·정상 종료. C++17 `/W4 /WX` ReaderTests의 합성 입력·거부·모델 3개 바이트 왕복 통과. 임시 사본의 Submesh.index 초기화 한 줄만 조정. `obj/importer-source-review/`. ImGuiSmoke·WindowSmoke는 빌드만, 수동 화면 미확인 |
| 고정 모델 fixture 분리 | 변조·TRS·재질 입력을 Tests/Assets/ModelFixture/CubeV2.bin으로 고정. CheckImportedModel → CheckModelFixture에서 열기·크기·정상 파싱 후 메모리 복사본만 변조. 실제 모델은 정상 로딩·GPU 비교에 사용 | Debug·Release RendererSmoke의 TRS·표면·손상·Phong·UV·다중 모델·9개 Wrap·136프레임·8회 리사이즈 통과(오류·경고 0). 크기·해시·Git·프로젝트 검사 통과. `obj/fixture-split-*.log`. 테스트 전용 변경으로 Sandbox·다른 Smoke·수동 화면 반복 없음 |
| UTF-8 진단·주석 보완 | 파일 열기 실패의 path.string()을 명시적 UTF-8 변환으로 교체해 진단 중 변환 예외 방지. CheckImportedModel 주석을 고정 입력·실제 모델 검사로 정정 | Debug·Release RendererSmoke 통과(오류·경고 0). 이모지 경로 독립 C++20 `/W4 /WX` 검사에서 UTF-8 보존·예외 없는 출력·종료 코드 0. `obj/fixture-diagnostic-*.log`. Sandbox·수동 화면 반복 없음 |

fixture의 출처·1,778바이트 스냅샷·SHA-256·offset·교체 조건은 [fixture 문서](../Tests/Assets/ModelFixture/README.md)에 둔다. 실제 모델의 메시·재질을 바꾸면 정상 입력 검사의 기대값도 갱신해야 한다. 위 변경은 기존 직접 include·RAII·오류 전달 규칙을 적용했으며 TRS 경계 오류를 수정하지 않았다.

### 이전 검증 기록

아래는 수행 당시 결과다. 문서 정리 과정에서 빌드·실행을 반복한 결과가 아니다.

| 시점·대상 | 확인 결과와 범위 |
|---|---|
| 2026-10-31 코드 검토 | Visual Studio 개발 환경 설정 후 Debug 솔루션 빌드·RendererSmoke 통과, D3D12 오류·경고 0. 별도 ReadTransform 검사에서 정상 양수 Scale TRS의 pitch 90도 인접 각도 실패 재현. Release·Sandbox 수동 화면은 미수행. 초기 링크의 MSPDB140.DLL 버전 오류는 개발 환경 설정 후 재빌드로 해소 |
| 2026-10-30 최종 코드 정리 | Debug·Release 솔루션 빌드, RendererSmoke·ImGuiSmoke 통과. 다중 모델·Phong·9개 Wrap 조합·리사이즈에서 D3D12 오류·경고 0. Sandbox는 실제 에셋·TEMP 작업 디렉터리·숨김 실행·합성 입력·리사이즈·WM_CLOSE 종료 코드 0. 로그 `obj/review-*.log`. WindowSmoke는 빌드만 수행 |
| 2026-10-30 v2 연결 | 잘못된 U/V·이전 v1 거부, 9개 Unity Wrap BIN의 로더→GPU texel 일치. ImageData·ModelLoader·D3D12Renderer 헤더를 PCH 없이 `/Zs /W4 /WX` 검사. 공급 해시·XML·fixture 원본 바이트 대조 |
| 2026-10-28~30 모델 로더 | 기준 개수·bounds·색 공간·UV, 누락·잘림·잘못된 인덱스·후행 바이트·TRS·전단/반전/0 법선 거부와 GPU Phong·UV 비교 통과. 외부 Reader의 합성 입력·잘림·UTF-8·자원 한도 검사도 당시 수행 |
| 2026-10-28 모델 UI | Debug의 초기 Transform·편집·Auto rotate 재개·회전 편집 시 정지를 화면으로 확인. Release는 모델·초기 Transform 표시 확인. 현재 세 모델 v2 화면의 재검증과 구분 |
| 2026-10-22~27 ImGui·입력 | 두 구성의 세 Smoke·Sandbox 숨김 실행, 관련 헤더 독립 컴파일 통과. UI readback 5회 각 48,000픽셀. 마우스 캡처 충돌은 수정 전 실패·수정 후 버튼 조합 검사 통과 |
| 2026-10-27 사용자 확인 | 선택 큐브 편집·회전 정지/재개, 창 밖 드래그·버튼 해제·Alt+Tab 입력 초기화 확인. 실행 구성은 별도 보고되지 않음 |
| 기존 구조·경로 | 10월 16일 출력 깊이·공백/한글 경로·원본 누락 진단, 20일 소속 분리·헤더 독립성 검증. 첫 렌더링 마일스톤은 [v0.1.0](milestones/v0.1.0-sandbox.md)에 보존 |

최종 코드 정리에서는 미사용 Legacy JSON 2개·덮어쓰는 Sampler 초기 대입을 제거하고 과거 포맷 거부 검사의 파일 누락 오판을 수정했다. PNG 전용 API·기본 큐브·Legacy BIN·Wrap fixture는 회귀 기준으로 유지한다.

수동 화면·물리 입력의 최신 v2 재확인, Unity 재익스포트, 메모리 부족·장치 제거 주입·전체 live-object·성능 측정은 최종 검증에서 수행하지 않았다. ImGui 백엔드의 실패 전파 한계는 [의존성](dependencies.md#dear-imgui)을 따른다. Debug 레이어 오류 0이 누수·성능 전체를 보장하지 않는다.

### 문서 정비 기록

| 시점 | 정리한 내용 | 당시 확인 |
|---|---|---|
| 2026-10-30 | v2·로컬 Assets 정책 최신화, M02 계획·마일스톤 목록을 로드맵에 통합 | 로컬 링크·앵커 117개, 솔루션 문서 항목·공백 검사 |
| 2026-11-02 상태 반영 | 고정 인스턴스·M02 미완료·계층/TRS 제약과 생성·삭제/ECS 후속 계획 반영 | Markdown 8개, 로컬 링크·앵커 127개, 솔루션 문서 경로·공백 검사 |
| 2026-11-02 Math 정리 | 연산 계약·MathChecks는 Math 문서, 소속·의존성은 프로젝트 구조, 검증 근거는 이 문서로 통합. 프로젝트 수 정정 | 코드·진입점 대조, 로컬 링크·앵커 153개·공백 검사 |

문서 정비에서는 코드·공통 규칙·위키를 변경하지 않았으며 빌드·실행을 반복하지 않았다. 삭제된 문서의 대체 위치는 [문서 통합 기록](roadmap.md#기존-계획과-문서-통합)을 따른다. 기존 마일스톤과 회귀 기준은 유지하고 계획을 구현 완료로 표시하지 않는다.
