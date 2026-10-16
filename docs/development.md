# 개발 설정과 검증

[프로젝트 소개와 실행](../README.md) · [Architecture](design.md) · [마일스톤 기록](milestones/README.md)

현재 개발에 필요한 컨벤션 적용 상태·설정·검증 방법을 관리한다. 날짜별 결과와 당시 제약은 마일스톤 문서에 기록한다.

## 컨벤션

공통 기준은 위키의 현재 [채택 규칙](https://rniman.github.io/convention/baseline/)과 [적용·근거 검토 절차](https://rniman.github.io/convention/baseline/#위키의-적용과-프로젝트-근거-검토)다. 작업 전 관련 규칙의 필수·권장·선택과 채택·초안·시험 적용을 구분한다. 확인 커밋은 고정 기준이 아니다.

프로젝트에는 실제 적용 범위·이유·예외·검증·후속 조건을 기록한다. 접근하지 못한 기준과 임시 자료도 명시한다. 공통 규칙 전문은 복제하지 않으며 위키 문서 갱신·반영 상태 관리는 위키 정비 담당의 책임이다.

### 확인·적용 상태

| 구분 | 현재 상태 |
|---|---|
| 확인 범위 | 2026-10-13 공개 baseline·좌표축/행렬 규칙 확인. 과거 근거는 [마일스톤 기록](milestones/v0.1.0-sandbox.md#컨벤션-확인-기록-2026-10-13), 2026-10-15·16 로컬 관련 규칙 재확인과 공개본 접근 제약은 아래 적용 기록 참고 |
| 실제 적용 | 명명·포맷·초기화·const·직접 include·한국어 API 주석·반환 계약·소유권·특수 멤버. 행 우선·행 벡터·row_major·추가 전달 전치 없음은 기존 코드 확인 및 CPU/GPU·셰이더 패킹 검증 완료 |
| 프로젝트 한정 채택 | 창·렌더러 소유 분리, 고성능 하드웨어 우선·WARP 자동 대체 없음, 프레임 경계 리사이즈, 슬롯 재사용 전 Fence 대기 |
| 시험 적용 | WIC·STA·RGBA8, sRGB·단일 밉·선형 Clamp, ComPtr·Event 수명·bool/HRESULT 진단, Debug 레이어·두 백 버퍼·VSync, FXC·SM5.0·D32·카메라·Transform·Phong·테스트 readback |
| 예외·미정 | PCH는 자기 헤더보다 먼저 포함. Windows→DirectX include 순서·긴 오류 문자열 예외 유지. 로컬 정면·거리 단위·외부 에셋 변환은 실제 사용 시 결정. 비정방 행렬·행렬 배열 도입 시 별도 패킹 검증 |
| 초안 | C++ 자료형·변환·attribute와 HLSL·검증 문서의 초안을 공통 채택으로 확대하지 않음 |

프로젝트의 [구현 선택과 후속 조건](design.md#구현-선택과-후속-조건)을 따른다. 규칙 변경 시 해당 범위의 코드·설정을 확인하고, 확인만 한 항목과 실제 수정·시험 적용한 항목을 구분한다. 적용하지 못하면 대상·이유·후속 조건을 남긴다. 날짜별 근거는 해당 마일스톤에 기록한다.

### 타입 명시와 렌더 데이터 명명 적용 (2026-10-15)

- 공개 baseline 조회가 실패하여 현재 공개본과의 일치는 확인하지 못했다. 임시 참고 자료는 로컬 `D:/Docs/Convention/docs/`의 baseline·C++ 명명·초기화·헤더·자료형 선택 문서다. 명명·초기화·헤더는 채택, 자료형 선택은 초안으로 구분했다. 위키 문서 수정은 수행하지 않았다.
- 사용자 선택으로 Sandbox의 실패 가능한 `std::optional`·이름 있는 결과 구조체, DirectX 행렬·벡터·저장 타입, 경로·입력/창 상태와 D3D12 descriptor의 타입을 명시했다. 영향받는 RendererSmoke·LightingChecks·RendererReadback도 같은 기준으로 맞췄다. 이 범위를 프로젝트 한정 채택으로 기록하며 자료형 초안 전체를 공통 채택으로 확대하지 않는다.
- 범위 반복, 기존 요소의 참조, 오른쪽 캐스트로 타입이 드러나는 포인터, chrono 시각과 람다는 `auto`를 유지한다. WindowSmoke의 선언은 이번 적용 범위에서 제외했다. 이후 해당 코드를 변경할 때 실패 계약이나 값/참조 구분이 중요하면 같은 기준으로 검토한다. 현재 규칙에 따른 미적용 의무 항목은 아니다.
- `RenderObject` → `ObjectTransformConstants`, `CreateRenderObject` → `BuildObjectTransformConstants`로 앱·렌더러·검증 코드·설계 문서를 함께 변경했다. 장면 오브젝트 생성과 변환 상수 계산의 구분이 목적이며 행렬 계산·실패 계약·192바이트 레이아웃·GPU 전달 방식은 유지한다.
- EXE 폴더를 Assets 폴더로 오해하지 않도록 `AssetDirectoryResult` → `ExecutableDirectoryResult`, 호출부의 변수 → `executableDirectoryResult`로 맞췄다. 공통 계산은 기존 RenderData와 EXE 경로 함수에서 계속 공유하며 새 범용 추상화는 추가하지 않았다.
- 검증: x64 Debug·Release 솔루션 빌드 성공. RendererSmoke는 두 구성 모두 CPU 행렬/실패 계약·PNG·카메라·조명과 GPU 텍스처/Phong/가림을 통과했고 각 136프레임·8회 리사이즈·D3D12 오류/경고 0·잘못된 가림 픽셀 0을 확인했다. WindowSmoke도 두 구성 종료 코드 0이다.
- Sandbox는 두 구성을 숨김 창으로 실행했다. 작업 디렉터리를 TEMP로 바꿔 실제 BaseColor·CSO의 EXE 기준 로딩을 확인하고 각 2초 실행 후 WM_CLOSE로 종료 코드 0을 얻었다. 실제 화면·장치 입력·F5 시각 확인은 수행하지 않았다. 변경한 문서의 로컬 링크 파일 대상 67개와 새 제안 절 앵커·diff/파일 형식을 확인했다.
- 최초 샌드박스 기본 도구 실행은 두 구성 모두 LNK1101(MSPDB140.DLL 버전 오류)로 링크 실패했다. VsDevCmd의 x64 환경·PreferredToolArchitecture=x64·node reuse 비활성화로 다시 실행했으며 Debug는 샌드박스 안, Release는 승인된 샌드박스 밖에서 빌드 성공했다. 환경과 실행 조건이 달라 오류 해소 원인을 특정하지 않는다. 에셋 경로 공유는 제안만 기록했으며 빌드 복사·런타임 로딩 동작은 변경하지 않았다.

### 공용 원본 경로 적용 (2026-10-16)

- 공개 baseline 조회가 실패하여 현재 공개본 일치는 확인하지 못했다. 로컬 `D:/Docs/Convention/docs/`의 최신 baseline·C++ 명명·초기화·auto·프로젝트 구조를 임시 참고했다. 로컬 baseline의 auto·프로젝트 구조 문서가 현재 채택 상태임을 확인했으며 이전 기록의 초안·확인 범위와 구분한다.
- 사용자 선택으로 Sandbox의 루트 `Assets/` 직접 읽기와 PNG 빌드 복사 제거를 프로젝트 한정 채택했다. RendererSmoke도 `Tests/Assets/`를 공유하며 손상 PNG 쓰기는 테스트 출력으로 분리했다. 원본 누락의 빌드 오류·런타임 경로/API/HRESULT 진단은 유지한다.
- 실제 두 소비자가 공유하는 빌드 설정을 `Sandbox/SourcePaths.props`로 분리했다. 확정된 `OutDir`와 `EngineRoot`로 원본 경로를 계산하여 전달하므로 개발자별 절대 경로나 고정된 상위 폴더 개수를 C++ 코드에 넣지 않는다. 런타임 경로 변경 요구가 없어 설정 파일 파서·원본 검색·범용 자원 시스템은 도입하지 않았다. 경로 계산은 앱·테스트에서, 디코딩은 AssetLoader에서, GPU 업로드는 렌더러에서 담당한다.
- 새 경로 선언은 명시적 타입을 사용했다. 기존 클래스·네임스페이스의 일괄 재배치는 하지 않았으며 작은 Sandbox의 평면 파일 배치를 유지한다. SourcePaths.props는 실제 빌드에서 두 프로젝트가 import하는 공용 설정으로 추가했고 기존 include 기준·공개 API·C++ 소스/필터 배치는 변경하지 않았다. 위키 문서 편집은 수행하지 않았다.
- 배포본의 독립 실행, 실행 중 에셋 추가·루트 선택, DDS 캐시는 후속 구현 범위다. 기본 출력 구조를 유지하여 저장소 전체를 이동하는 경우와 EXE만 옮기는 경우의 차이는 README와 프로젝트 구조 문서에 기록했다.
- 검증: x64 Debug·Release 솔루션 빌드·RendererSmoke·WindowSmoke 성공. RendererSmoke는 각 136프레임·8회 리사이즈·D3D12 오류/경고 0·잘못된 가림 픽셀 0이다. 최초 두 구성 빌드는 기존 MSPDB 제약을 피한 샌드박스 밖 VsDevCmd x64 환경에서 실행했고, 복사본 정리 후 재생성 여부 확인을 위한 빌드는 샌드박스 안에서도 성공했다.
- 원본과 해시가 같은 기존 Sandbox·RendererSmoke의 Debug/Release PNG 복사본 4개만 삭제하고 다시 빌드해 재생성되지 않음을 확인했다. 두 구성 Sandbox를 TEMP 작업 디렉터리·숨김 창으로 각 2초 실행하고 WM_CLOSE 정상 종료 코드 0을 확인했다. 원본 PNG 2개의 해시가 그대로이며 손상 PNG 테스트 파일도 정리됐다.
- 격리한 Release 실행에서 공용 원본을 제외하고 EXE 옆에는 정상 PNG를 두었다. 원본 경로·WIC API·HRESULT 진단과 종료 코드 1을 확인하여 과거 복사본으로 대체하지 않음을 검증했다. 원본이 없는 EngineRoot로 빌드 검증 타깃도 실행해 준비 안내와 실패를 확인했다. 실제 사용자 원본은 이동·수정하지 않았다. 기본 출력의 원본 상대 경로는 `../../../`, 깊이를 바꾼 OutDir의 초기 MSBuild 속성 평가는 `../../`였다.
- 커밋 전 최종 점검에서 공백·한글이 있는 별도 EngineRoot와 깊이가 다른 OutDir·IntDir로 Release RendererSmoke를 실제 빌드하고 TEMP 작업 디렉터리에서 실행했다. PNG 원본 직접 읽기·손상 파일 정리와 136프레임·8회 리사이즈·D3D12 오류/경고 0을 확인했다. 기본 출력의 PNG 복사본 부재·원본 해시 불변, 문서 로컬 링크 76개·앵커 18개도 재확인했다. 공개 baseline은 여전히 접근 불가였으며 로컬 최신 C++ 규칙과 Git 커밋 메시지 규칙을 참고했다. 이번 추가 점검에서는 코드 변경 없이 기존 검증 결과와 전체 diff를 대조했다.
- 실제 화면·장치 입력·F5 시각 확인은 수행하지 않았다. 문서 로컬 링크·이전 제안 앵커·UTF-8/LF·diff를 점검했다. 원본 경로와 PNG 복사 타깃만 바꿨으며 WIC 디코딩·GPU 업로드·행렬/조명 계약은 유지한다.

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

일반 헤더는 PCH 없이 필요한 의존성을 직접 포함한다. Sandbox PCH는 `$(IntDir)$(ProjectName).pch`에 생성한다. Sandbox·RendererSmoke의 [SourcePaths.props](../Sandbox/SourcePaths.props)가 확정된 출력 위치에서 저장소까지의 경로를 `RNIMAN_SOURCE_ROOT`로 전달한다. PNG는 원본에서 직접 읽고 CSO는 EXE 옆 `Shaders/`에서 읽는다. 출력 폴더만 옮기는 독립 실행은 지원하지 않으며 출력 경로 변경 시 다시 빌드한다. 상세 경로는 [프로젝트 구조](project-structure.md#개발-실행의-공용-에셋-경로)와 README에서 관리한다.

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
| [RendererSmoke](../Tests/RendererSmoke.cpp) | PNG·면 UV/sRGB/alpha·Phong·역전치 법선·Transform/카메라 수학、가림·리사이즈·프레임 재사용·정리 | 두 구성 모두 D3D12 FL11_0 하드웨어·Graphics Tools·Cube CSO 필요. Tests/Assets/TextureTest.png를 직접 읽으며 벽돌 원본은 불필요 |
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
