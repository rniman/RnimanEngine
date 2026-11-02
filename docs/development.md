# 개발 설정과 검증

[프로젝트 소개와 실행](../README.md) · [Architecture](design.md) · [프로젝트 구조](project-structure.md) · [로드맵](roadmap.md)

현재 컨벤션 적용 상태와 개발 설정·검증 방법을 관리한다. 기능 계획은 [로드맵](roadmap.md), 달성 결과와 당시 제약은 마일스톤 문서에서 관리한다.

## 컨벤션

공통 기준은 위키의 현재 [채택 규칙](https://rniman.github.io/convention/baseline/)과 [적용·근거 검토 절차](https://rniman.github.io/convention/baseline/#위키의-적용과-프로젝트-근거-검토)다. 작업 전 필수·권장·선택과 채택·초안을 구분해 확인한다. 확인 커밋은 고정 기준이 아니다.

변경 범위의 실제 적용·선택 이유·예외·검증과 미적용 대상의 이유·후속 조건을 이 문서에 기록한다. 확인만 한 내용과 실제 적용을 구분하며, 위키 문서 갱신과 반영 상태 관리는 위키 정비 담당이 맡는다.

### 확인 범위

2026-10-20에 공개 baseline·채택 문서 14개·관련 초안 6개·프로젝트 참조 문서를 코드·설정과 대조했다(당시 위키 원본 `4beaa82`). 10월 21~30일에는 공개본 접근 오류로 최신본 일치를 확인하지 못해 컨벤션 위키 로컬 사본의 baseline·관련 채택 규칙을 임시 참고했다. 아래 적용은 이 확인 범위에 한정하며 공개본은 접근 복구 후 재확인한다. 초안·미결정 사항은 공통 채택으로 확대하지 않는다.

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

### ReferenceImporter 재질 조회 통합 (2026-10-30)

외부 원본의 ID→배열 위치 단일 테이블·UV/접선 검증 순회 통합·조회 예산 64바이트를 엔진 사본에 반영했다. 기존 ID 검증/삽입 분리·초기화·수명 주석을 유지하고 공급 기록·해시를 갱신했다. 이번 세션에서 확인한 로컬 컨벤션 범위를 적용했으며 새 규칙은 도입하지 않았다. 원본의 별도 Submesh.index 제거는 반영하지 않았다. BIN v2·에셋·재익스포트 조건은 변경하지 않는다.

Debug·Release 솔루션 빌드·RendererSmoke 및 실제 에셋의 Sandbox 숨김 실행·리사이즈·정상 종료를 확인했다. RendererSmoke의 D3D12 오류·경고는 0이다. C++17 `/W4 /WX`로 엔진 Reader와 원본 ReaderTests를 빌드해 합성 입력·거부 검사·모델 3개 바이트 왕복을 통과했다. 테스트는 엔진의 Submesh.index에 맞춘 초기화 한 줄만 임시 사본에서 변경했으며 외부 원본은 수정하지 않았다. 로그·임시 테스트는 `obj/importer-source-review/`에 둔다. 해시·공백 검사도 통과했다. 수동 화면 확인은 수행하지 않았으며 영향 없는 ImGuiSmoke·WindowSmoke는 빌드만 수행했다.

### 테스트 모델 입력 분리 (2026-10-30)

ModelChecks의 바이트 변조·TRS·재질 변환 입력을 `Tests/Assets/ModelFixture/CubeV2.bin`으로 고정했다. 현재 BrickWallCube의 1,778바이트 스냅샷과 출처·SHA-256·offset·교체 조건을 함께 보관하고 프로젝트에 등록했다. 실제 Assets/Models는 정상 로딩·GPU 비교에 사용한다. 진입점 CheckImportedModel은 CheckModelFixture를 먼저 실행하며, 고정 파일의 열기·크기·정상 파싱을 확인한 뒤 메모리 복사본만 변조한다. 기존 임시 파일 RAII와 확인한 로컬 컨벤션을 유지하고 새 규칙은 도입하지 않았다.

Debug·Release RendererSmoke 빌드·실행 통과: 고정 입력의 TRS·표면·손상 검사, 실제 모델 Phong·UV·다중 모델·9개 Wrap·136프레임·8회 리사이즈에서 D3D12 오류·경고 0. 로그는 `obj/fixture-split-*.log`다. fixture 크기·해시·Git 제외 여부, 프로젝트 XML·문서 링크·공백을 확인했다. 테스트 전용 변경이므로 Sandbox·다른 Smoke·수동 화면 검증은 반복하지 않았다. 실제 모델의 메시·재질을 바꾸면 정상 입력 검사의 기대값은 여전히 갱신해야 한다.

### fixture 진단·API 주석 보완 (2026-10-30)

파일 열기 실패 진단의 path.string()을 명시적 UTF-8 변환으로 바꿔 시스템 코드 페이지에 없는 문자가 포함된 경로에서도 진단 중 변환 예외가 발생하지 않게 했다. CheckImportedModel 주석은 Binary v2 고정 입력 검사·실제 모델 GPU 검사로 맞췄다. 기존 주석·직접 include·실패 전달 규칙을 적용했고 새 규칙은 도입하지 않았다.

Debug·Release RendererSmoke 빌드·실행 통과, D3D12 오류·경고 0. 이전에 예외를 재현한 이모지 경로는 동일 진단 구문의 독립 C++20 `/W4 /WX` 검사에서 UTF-8 바이트 보존·예외 없는 출력·종료 코드 0을 확인했다. 로그는 `obj/fixture-diagnostic-*.log`다. 테스트 전용 변경으로 Sandbox·수동 화면 검증은 반복하지 않았다. 90도 근처 Euler 복원의 알려진 오차는 이번 수정 대상이 아니다.

### 검증 기록

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

### 문서 정리 (2026-10-30)

코드·프로젝트·Git 설정을 대조해 v2·로컬 Assets 준비 정책을 최신화했다. M02 계획과 마일스톤 목록을 로드맵에 통합하고, 반복 작업 이력은 위 적용·검증 근거로 압축했다. 신규 규칙 채택·위키 수정은 없으며 공개 위키 접근 한계는 위 확인 범위를 유지한다. 로컬 링크·앵커 117개, 솔루션 문서 항목과 공백 검사를 통과했다. 문서·솔루션 표시 항목만 수정했으므로 빌드·실행은 반복하지 않았다.

### 문서 정리와 남은 작업 (2026-11-02)

현재 코드·변경 상태를 대조해 README에 고정 인스턴스·M02 미완료 상태, 정적 모델 계약에 계층 제한과 TRS 결함, 로드맵에 일반 오브젝트 생성·삭제 및 ECS 비교 구현안을 반영했다. 삭제된 M02 계획·마일스톤 목록의 대체 위치를 로드맵에 기록했다. 기존 v0.1.0 기록·회귀 기준은 유지하며 새 계획을 구현 완료로 표시하지 않는다.

공개 baseline은 이번에도 접근 오류로 최신 채택 규칙을 확인하지 못했다. 저장소 적용 기록과 임시 보관본 `D:/DevelopTool/ModelExporter/Exporter/Logs/convention-baseline.txt`의 적용·근거 검토 절차를 참고했다. 신규 공통 규칙 채택·위키 수정·코드 수정은 없다. 문서 내용 대조, Markdown 8개의 로컬 링크·앵커 127개, 솔루션 문서 경로와 공백 검사를 통과했다. 문서만 수정했으므로 빌드·실행은 반복하지 않았다. 10월 31일 실행 결과는 위 검증 기록과 구분한다.

## 프로젝트 설정

개발 환경·빌드·실행·에셋 준비는 [README](../README.md#빌드와-실행)를 따른다. 네 프로젝트는 x64 Debug·Release를 지원한다.

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
| [RendererSmoke](../Tests/RendererSmoke.cpp) | PNG/JPEG·다중 모델 선택·UV/sRGB/alpha·Phong·법선·Transform/카메라 수학, 가림·리사이즈·동기화·정리 | 두 구성 모두 D3D12 FL11_0 하드웨어·Graphics Tools·Cube CSO 필요. 저장소의 Tests/Assets/TextureTest.png·ColorTest.jpg와 세 기준 모델 bin 사용, 외부 텍스처 불필요 |
| [WindowSmoke](../Tests/WindowSmoke.cpp) | 키·마우스 소비, 캡처/포커스/모달 초기화, 최소화·리사이즈·종료·창 수명 | Win32 환경. D3D12·Graphics Tools·CSO 불필요 |
| [ImGuiSmoke](../Tests/ImGuiSmoke.cpp) | ImGui 초기화·UI 픽셀·폰트 크기 변경·리사이즈·소멸·D3D12 진단 | 두 구성 모두 FL11_0 하드웨어·Graphics Tools·Cube CSO 필요. 외부 PNG 불필요 |
| Sandbox | 실제 화면·장치 입력·창 조작의 통합 흐름 | 세 모델 bin·벽돌 PNG·Helmet BaseColor JPEG·CSO·D3D12 하드웨어 필요. Graphics Tools는 Debug에 필수. WARP 대체 없음 |

### GPU 픽셀 검증

[RendererReadback](../Tests/RendererReadback.h)은 Present 없이 그린 뒤 GPU 대기·복사·Map을 거쳐 RowPitch 패딩을 제외한 RGBA8을 읽는다. flip-discard 이후 내용에 의존하지 않으며 Capture 실패 시 검증을 중단한다. 어댑터는 GPU 완료 후 렌더러보다 먼저 파괴한다.

- `CheckOcclusion`: 세 카메라/해상도에서 앞·뒤 단독 및 그리기 순서를 비교한다. 겹침·색 차이·뒤 가시 영역이 각각 100픽셀 이상이고, 결과가 깊이와 일치해야 한다.
- [CheckPhongLighting](../Tests/LightingChecks.cpp): 독립적인 sRGB 기대값과 비균일 Scale·회전·전단으로 조명과 역전치 법선을 검증한다. 텍스처·가림 검사는 Ambient=1·직접광=0을 사용한다.
- [3×3 기준 PNG](../Tests/Assets/TextureTest.png): 여섯 면의 UV·채널·행 패딩·sRGB·출력 alpha를 비교한다. RGB 허용 오차는 텍스처 1, 조명 2다.
