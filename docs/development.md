# 개발 설정과 검증

[프로젝트 소개와 실행](../README.md) · [Architecture](design.md) · [마일스톤 기록](milestones/README.md)

현재 개발에 필요한 컨벤션 적용 상태·설정·검증 방법을 관리한다. 날짜별 결과와 당시 제약은 마일스톤 문서에 기록한다.

## 컨벤션

공통 기준은 위키의 현재 [채택 규칙](https://rniman.github.io/convention/baseline/)과 [적용·근거 검토 절차](https://rniman.github.io/convention/baseline/#위키의-적용과-프로젝트-근거-검토)다. 작업 전 관련 규칙의 필수·권장·선택과 채택·초안·시험 적용을 구분한다. 확인 커밋은 고정 기준이 아니다.

프로젝트에는 실제 적용 범위·이유·예외·검증·후속 조건을 기록한다. 접근하지 못한 기준과 임시 자료도 명시한다. 공통 규칙 전문은 복제하지 않으며 위키 문서 갱신·반영 상태 관리는 위키 정비 담당의 책임이다.

### 확인·적용 상태

| 구분 | 현재 상태 |
|---|---|
| 확인 범위 | 2026-10-13 공개 baseline·좌표축/행렬 규칙 확인. C++ 9개 채택 문서는 2026-10-12 로컬 위키로 확인했으며 그 당시 공개본 일치는 미확인. 상세 근거는 [마일스톤 기록](milestones/v0.1.0-sandbox.md#컨벤션-확인-기록-2026-10-13) 참고 |
| 실제 적용 | 명명·포맷·초기화·const·직접 include·한국어 API 주석·반환 계약·소유권·특수 멤버. 행 우선·행 벡터·row_major·추가 전달 전치 없음은 기존 코드 확인 및 CPU/GPU·셰이더 패킹 검증 완료 |
| 프로젝트 한정 채택 | 창·렌더러 소유 분리, 고성능 하드웨어 우선·WARP 자동 대체 없음, 프레임 경계 리사이즈, 슬롯 재사용 전 Fence 대기 |
| 시험 적용 | WIC·STA·RGBA8, sRGB·단일 밉·선형 Clamp, ComPtr·Event 수명·bool/HRESULT 진단, Debug 레이어·두 백 버퍼·VSync, FXC·SM5.0·D32·카메라·Transform·Phong·테스트 readback |
| 예외·미정 | PCH는 자기 헤더보다 먼저 포함. Windows→DirectX include 순서·긴 오류 문자열 예외 유지. 로컬 정면·거리 단위·외부 에셋 변환은 실제 사용 시 결정. 비정방 행렬·행렬 배열 도입 시 별도 패킹 검증 |
| 초안 | C++ 자료형·변환·attribute와 HLSL·검증 문서의 초안을 공통 채택으로 확대하지 않음 |

프로젝트의 [구현 선택과 후속 조건](design.md#구현-선택과-후속-조건)을 따른다. 규칙 변경 시 해당 범위의 코드·설정을 확인하고, 확인만 한 항목과 실제 수정·시험 적용한 항목을 구분한다. 적용하지 못하면 대상·이유·후속 조건을 남긴다. 날짜별 근거는 해당 마일스톤에 기록한다.

## 프로젝트 설정

빌드·실행·에셋 준비와 조작은 [README](../README.md#빌드와-실행)를 따른다. 세 프로젝트는 x64 Debug·Release를 지원한다.

| 항목 | 설정 |
|---|---|
| 확인한 환경 | Visual Studio Community 2026 18.10.3, MSVC `v145` / `14.51.36231`, Windows SDK `10.0.26100.0` |
| C++ | C++20, Unicode, `/utf-8`, `/W4`, `/permissive-`, `/EHsc`; Debug `/MDd`, Release `/MD` |
| 경고 | Sandbox·RendererSmoke는 `/WX` 끔, WindowSmoke는 `/WX` |
| 출력 | `bin/<프로젝트>/<구성>/`, `obj/<프로젝트>/<구성>/`; Sandbox 디버거 작업 디렉터리 `$(OutDir)` |
| 연결 | Sandbox·RendererSmoke: `d3d12.lib`·`dxgi.lib`·`d3dcompiler.lib`·`windowscodecs.lib`·`ole32.lib`. WindowSmoke는 D3D12를 연결하지 않음 |
| 헤더 | `WIN32_LEAN_AND_MEAN`, `NOMINMAX`; Sandbox는 PCH, Smoke는 PCH 없이 실제 구현을 컴파일 |
| 셰이더 | [CubeShaders.props](../Sandbox/CubeShaders.props)의 FXC·SM5.0·진입점 Main. 구성별 EXE 옆 `Shaders/`에 CSO 생성·EXE 기준 로딩 |
| 파일·Git | [.editorconfig](../.editorconfig): UTF-8 BOM 없음·LF·끝 개행·C++ 탭 너비 4. [.gitattributes](../.gitattributes): 텍스트 LF, Windows 명령 스크립트 CRLF. [.gitignore](../.gitignore): 빌드 출력·개인 캐시·루트 `/Assets/` 제외, `Tests/Assets/` 추적 |

일반 헤더는 PCH 없이 필요한 의존성을 직접 포함한다. Sandbox PCH는 `$(IntDir)$(ProjectName).pch`에 생성한다. 자원은 EXE 위치로 찾으며 실행 파일 이동 시 `Shaders`·`Assets`도 함께 복사한다. 상세 소스 등록·필터·복사 경로는 [프로젝트 구조](project-structure.md#현재-프로젝트와-코드)와 README에서 관리한다.

## 검증 방법

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
```

| 대상 | 검사 범위 | 실행 조건 |
|---|---|---|
| [RendererSmoke](../Tests/RendererSmoke.cpp) | PNG·면 UV/sRGB/alpha·Phong·역전치 법선·Transform/카메라 수학、가림·리사이즈·프레임 재사용·정리 | 두 구성 모두 D3D12 FL11_0 하드웨어·Graphics Tools·Cube CSO·테스트 PNG 필요. 빌드가 EXE 옆에 복사하며 벽돌 원본은 불필요 |
| [WindowSmoke](../Tests/WindowSmoke.cpp) | 키·마우스 소비, 캡처/포커스/모달 초기화, 최소화·리사이즈·메시지·종료·창 수명 | Win32 환경. D3D12·Graphics Tools·CSO 불필요 |
| Sandbox | 실제 화면·장치 입력·창 조작의 통합 흐름 | BaseColor·CSO·하드웨어 필요. Graphics Tools는 Debug에 필수, Release는 요구하지 않음. WARP 대체 없음 |

공통 코드 변경 시 영향받는 테스트와 대표 앱을 빌드·실행한다. 빌드·실행·시각 확인을 구분하고, 미수행 항목·이유와 성능 측정 조건을 기록한다. 문서만 수정하면 내용·링크를 확인한다.

<a name="겹치는-오브젝트의-픽셀-검증-2026-10-05"></a>

### GPU 픽셀 검증

[RendererReadback](../Tests/RendererReadback.h)은 같은 draw를 Present 없이 실행해 flip-discard 이후 내용에 의존하지 않는다. GPU 대기 → COPY_SOURCE 복사·PRESENT 복귀 → GPU 대기 → Map 후 RowPitch를 제외한 RGBA8을 읽는다. 앱은 Present를 유지하며 어댑터는 GPU 완료 후 렌더러보다 먼저 파괴한다. Capture 실패 시 검증을 종료한다.

- `CheckOcclusion`: 빈 화면·앞 단독·뒤 단독·정순·역순을 세 카메라/해상도에서 비교한다. 겹침·색 차이·뒤 가시 영역이 각각 100픽셀 이상이어야 하며 앞 영역은 앞 단독, 나머지는 뒤 단독과 같아야 한다.
- [LightingChecks](../Tests/LightingChecks.cpp)의 `CheckPhongLighting`: Ambient 기준 픽셀에 독립적인 sRGB 기대값을 적용한다. 잘못된 World 법선 변환을 구별하려고 비균일 Scale·회전에 전단을 더한다. 기존 텍스처·가림은 Ambient=1·직접광=0인 동일 파이프라인에서 검사한다.
- [3×3 기준 PNG](../Tests/Assets/TextureTest.png)는 9색·중앙 alpha 128을 직접 작성했다. 여섯 면의 텍셀 중심에서 UV·채널·업로드 행 패딩·sRGB·alpha 255를 비교한다. RGB 허용 오차는 텍스처 1, 조명 2다.

## 이전 문서 링크

<a name="d3d12-화면-출력과-검증"></a>
<a name="진입점과-수명"></a>
<a name="창입력-추출-2026-10-06"></a>
<a name="앱-실행부-추출-2026-10-06"></a>
<a name="프레임별-fence-동기화"></a>
<a name="좌표축과-행렬"></a>
<a name="메시행렬렌더-데이터"></a>

실행 구조·수명·좌표계·렌더 데이터 계약은 [Architecture](design.md#sandbox-실행-구조)로 옮겼다.

<a name="최신-검증-결과"></a>
<a name="2026-09-17-프레임-동기화-검증"></a>

검증 결과는 [마일스톤 목록](milestones/README.md)에서 확인한다. 기존 링크의 2026-09-17 개별 기록은 현재 문서에 남아 있지 않으며, v0.1.0의 후속 검증을 해당 날짜의 결과로 간주하지 않는다.
