# 개발 설정과 검증

[프로젝트 소개와 실행](../README.md) · 기준일: 2026-10-06

현재 개발 기준, 코드 흐름, 최신 검증 결과를 관리한다. 기능 목표는 [로드맵](roadmap.md), 설계 방향은 [엔진 설계](design.md), 작업 지침은 [AGENTS.md](../AGENTS.md)를 따른다.

## 컨벤션

공통 기준은 위키의 현재 [채택 규칙](https://rniman.github.io/convention/baseline/)과 [상호 반영 절차](https://rniman.github.io/convention/baseline/#위키와-프로젝트의-상호-반영)다. 필수·권장·선택 강도를 유지하고, 초안·시험 적용을 공통 채택으로 취급하지 않는다. 확인 커밋은 상태 기록이며 고정 기준이 아니다.

- 작업 전에 관련 규칙과 적용 범위를 확인한다. 접근하지 못한 범위와 임시 참고 자료는 기록한다.
- 새 규칙·예외를 구체화하면 공통 내용은 위키의 해당 주제에, 엔진 전용 내용은 [RnimanEngine 문서](https://rniman.github.io/convention/projects/rniman-engine/)에 반영한다. 사용자 선택이나 채택 근거가 없는 결정은 시험 적용으로 남긴다.
- 프로젝트에는 실제 적용 범위와 미반영 대상·이유·후속 조건을 기록한다. 위키 변경의 영향을 받는 코드·설정은 관련 작업 범위부터 맞춘다.
- 규칙 전문은 위키에서, 엔진 구현·실행·검증 상세는 이 저장소에서 관리한다. 위키 편집과 커밋·푸시·배포는 별개다.

### 확인·반영 상태

| 구분 | 현재 상태 |
|---|---|
| 확인 | 2026-10-06 공개 C++ 위키와 로컬 `D:/Docs/Convention` HEAD `33e96f6`의 채택 규칙·초안을 대조했다. Sandbox·RendererSmoke의 지역 상수 명명, 빈 함수 본문, 직접 include와 API 주석을 현재 채택 규칙에 맞췄다. |
| 프로젝트 한정 채택 | 앱의 창·렌더러 소유 분리, 고성능 하드웨어 우선과 WARP 대체 없음, 프레임 경계 리사이즈, 프레임 자원 재사용 전 Fence 대기. 초기의 매 프레임 전체 대기는 2026-09-17에 대체했다. |
| 시험 적용 | ComPtr·Event 수명, bool/API/HRESULT 실패 보고, Debug 레이어 필수, VSync·두 백 버퍼, FXC·SM5.0, 행렬 계약·D32 공유, 카메라·Transform·오브젝트별 상수, 테스트 전용 readback. 자료형 초안 중 CPU 복사 크기·span 순회에 `std::size_t`를 사용하고 전용 API 타입을 유지하는 선택을 적용했다. |
| 위키 반영 | 자료형·변환 초안은 공개 위키에서 확인했다. 이번 코드 적용 결과는 위키의 RnimanEngine 프로젝트 문서와 적용 기준의 변경 기록에 반영하고 strict 사이트 빌드를 통과했다. 이번 위키 편집은 로컬 미커밋 상태다. |
| 남은 범위 | 자료형·변환과 HLSL은 공통 초안이며 전체 채택을 뜻하지 않는다. 자체 별칭·범용 변환 도우미는 실제 반복 요구가 생기면 검토한다. 위키 변경의 커밋·푸시·배포는 수행하지 않았다. |

다른 실제 소비자가 생기면 Transform·렌더 데이터 수집, 입력 초기화, 버퍼 업로드의 공통화를 검토한다. 현재 Sandbox의 선택을 범용 엔진 정책으로 확대하지 않는다.

### 자료형·변환 초안의 적용 범위

2026-10-06 [자료형 선택과 별칭](https://rniman.github.io/convention/coding/cpp/type-selection/)과 [타입 변환과 범위 검사](https://rniman.github.io/convention/coding/cpp/conversions/)를 확인하고 다음 범위를 코드에 시험 적용했다. 두 문서는 공통 초안이며 규칙 전문은 위키에서 관리한다.

- `RenderInternal`의 상수 간격·CPU 복사 크기·span 순회 인덱스를 `SIZE_T`에서 `std::size_t`로 변경했다. 곱셈 전에 `std::numeric_limits<std::size_t>`로 범위를 확인하며, 기존 GPU 동기화와 자원 확장 동작은 유지한다.
- `UINT` 크기·백 버퍼 인덱스·디스크립터 간격, `UINT64` Fence·자원 크기, InfoQueue의 `SIZE_T` 출력 변수와 디스크립터 주소 계산은 전용 API 계약에 맞춰 유지한다. 출력 포인터를 다른 정수 타입으로 캐스팅하지 않는다.
- 전체 범위가 보존되는 API 전달에는 캐스트를 추가하지 않았다. 고정 배열 길이·크기의 `UINT` 변환과 Win32·COM 포인터 변환은 기존 범위·객체 계약을 유지한다. 테스트 readback의 픽셀 출력은 실패 시에도 바뀔 수 있다는 계약을 주석에 명시했다.
- 자체 기본 타입 별칭·공통 변환 도우미는 미도입이다. 실제 모듈·변환 소비자의 반복 요구가 생기면 공통 초안의 도입 조건과 실패 보고의 공통화를 검토한다.

공개 위키는 웹 열기 도구에서 접근하지 못해 승인된 PowerShell HTTP 조회로 확인했다. 초기 문서 작성 때의 C++20 예제 4개 컴파일·범위 검사와 엔진 검증은 별개다. 이번 코드 변경 후에는 아래 빌드·실행 검증과 공개 헤더 4개의 PCH 없는 `/W4 /WX` 독립 컴파일을 수행했다. C++ 파일 9개의 UTF-8 BOM 없음·LF·마지막 개행·탭 들여쓰기·후행 공백을 확인했다.

## 프로젝트 설정

빌드·실행은 [README](../README.md#빌드와-실행), 입력 방법은 [카메라 조작](../README.md#카메라-조작)을 따른다. Visual Studio 프로젝트를 직접 관리하며 x64 Debug·Release만 사용한다.

| 항목 | 설정 |
|---|---|
| 확인한 환경 | Visual Studio Community 2026 18.10.3, MSVC `v145` / `14.51.36231`, Windows SDK `10.0.26100.0` |
| C++ | C++20, Unicode, `/utf-8`, `/W4`, `/permissive-`, `/EHsc`; 경고의 오류 처리는 끔 |
| 런타임 | Debug `/MDd`, Release `/MD` |
| 출력 | `bin/<프로젝트>/<구성>/`, `obj/<프로젝트>/<구성>/`; 디버거 작업 디렉터리 `$(OutDir)` |
| 연결 | `d3d12.lib`, `dxgi.lib`, `d3dcompiler.lib` |
| 헤더 | `WIN32_LEAN_AND_MEAN`, `NOMINMAX`; Sandbox는 PCH 사용, RendererSmoke는 PCH 없이 헤더 독립성 확인 |
| 파일 형식 | [.editorconfig](../.editorconfig)의 UTF-8 BOM 없음·LF·마지막 개행, C++ 탭 너비 4칸 |

Sandbox 구현 파일은 `pch.h`를 먼저 include하고, 일반 헤더는 필요한 의존성을 직접 포함한다. PCH는 구성별 `$(IntDir)$(ProjectName).pch`에 생성한다.

[CubeShaders.props](../Sandbox/CubeShaders.props)를 Sandbox·RendererSmoke가 공유한다. FXC가 `CubeVS.hlsl`·`CubePS.hlsl`을 SM5.0 CSO로 컴파일하며 EXE 옆 `Shaders/`에 배치한다. 로더는 EXE 위치를 기준으로 읽고 실패 시 경로·HRESULT를 보고한다. 실행 파일을 옮길 때 셰이더도 함께 복사한다.

[.gitignore](../.gitignore)는 빌드 출력·개인 캐시를 제외한다. Unity `.meta`, 모델 `.obj`, `.bin`·DDS·외부 라이브러리는 전역 제외하지 않는다. [.gitattributes](../.gitattributes)는 텍스트를 LF로 정규화하고 Windows 명령 스크립트는 CRLF로 체크아웃한다.

검증 환경에서 MSBuild의 기존 `obj` 접근이 거부되면 승인된 샌드박스 밖 빌드로 확인했다. 과거 `Path`·`PATH` 중복도 실행 환경에서 정리했으며 프로젝트 설정으로 우회하지 않았다.

## D3D12 화면 출력과 검증

### 진입점과 수명

[Main.cpp](../Sandbox/Main.cpp)의 `wWinMain` → 창·렌더러 초기화 → 메시지 처리 → 리사이즈 → 입력·Transform 갱신 → 렌더링 → GPU 대기 → RAII 정리 순서다. 앱은 상태 → 창 → 렌더러 순으로 생성하며 역순으로 파괴한다.

- `WM_SIZE`는 크기만 기록하고 GPU 자원은 다음 프레임에서 재생성한다. 최소화·0 크기에서는 `WaitMessage`로 대기한다.
- 고성능 순서로 Feature Level 11_0 지원 하드웨어를 선택한다. Debug 앱과 두 구성의 테스트에는 Graphics Tools가 필요하다. Release 앱은 디버그 레이어를 요구하지 않는다.
- [D3D12Renderer.cpp](../Sandbox/D3D12Renderer.cpp)는 Device·Queue·Swap Chain·RTV/DSV·프레임 자원·Fence·깊이 버퍼·파이프라인·메시를 소유한다.
- 프레임은 슬롯 Fence 대기 → 상수 복사 → Allocator/List Reset → 색상·깊이 clear → 오브젝트별 draw → PRESENT 복귀 → 제출·Present·Signal 순서다.
- Resize는 전체 GPU 대기 후 백 버퍼·깊이 버퍼를 해제·재생성한다. 동일 크기·0 크기 요청은 무시한다.
- 실패 시 API·오류 코드와 가능한 Device removed reason을 보고하고 종료한다. 부분 자원은 RAII로 정리하며 장치 손실 자동 복구는 없다.

초기 클라이언트 크기는 1280×720이다. DPI 정책·최소 창 크기는 미결정이며, 연속 리사이즈의 Win32 모달 루프 동안 렌더링이 멈출 수 있다.

### 프레임별 Fence 동기화

두 `FrameResources`가 각각 Allocator·Constant Buffer·마지막 제출 Fence 값을 소유한다. 현재 백 버퍼 슬롯의 이전 제출이 완료된 뒤에만 기록 메모리와 상수를 재사용한다. 하나의 Command List와 단일 Direct Queue·Fence/Event를 공유한다.

Fence 값은 단조 증가하고 제출 당시 슬롯에 기록한다. `UINT64_MAX` 완료 값은 장치 제거로 처리한다. `WaitForGpu`는 리사이즈·종료·미완료 작업이 있는 소멸자에서 전체 제출을 기다린다. Signal·Present 실패 후에도 정리를 시도한다. VSync·Present 대기는 남아 있으며 성능 향상은 측정하지 않았다.

### 자동 검증

저장소 루트의 Developer PowerShell에서:

```powershell
MSBuild.exe Tests/RendererSmoke.vcxproj -nologo -t:Build -p:Configuration=Debug -p:Platform=x64
./bin/RendererSmoke/Debug/RendererSmoke.exe
MSBuild.exe Tests/RendererSmoke.vcxproj -nologo -t:Build -p:Configuration=Release -p:Platform=x64
./bin/RendererSmoke/Release/RendererSmoke.exe
```

[RendererSmoke.cpp](../Tests/RendererSmoke.cpp)는 실제 하드웨어와 숨김 창을 사용한다. CPU 수학 검사, 아래 픽셀 검증, 136프레임·8회 크기 변경의 GPU smoke 구간을 실행한다. smoke 구간은 0·1·3·9개 오브젝트, 같은 크기·0 크기·1×1·확대·축소, 반복 전체 대기·재개·소멸자 정리를 포함한다.

공통 코드 변경 시 영향받는 유지 샘플과 대표 데모를 빌드·실행한다. 빌드·실행·시각 확인을 구분하고, 성능은 조건과 측정값이 있을 때만 보고한다. InfoQueue 보관용 Device 참조 때문에 이 테스트는 모든 참조 해제 후의 live-object 검사를 대신하지 않는다.

## 큐브 출력과 검증

### 메시와 행렬

- 정점 8개·`R16_UINT` 인덱스 36개의 고정 Upload Heap 메시를 모든 큐브가 공유한다. 범용 메시 로딩 시 Default Heap 업로드를 검토한다.
- CPU는 행 벡터의 `World * View * Projection`을 저장하고 [CubeVS.hlsl](../Shaders/Samples/Sandbox/CubeVS.hlsl)은 `row_major`·`mul(position, matrix)`로 전치 없이 읽는다. 왼손 View·Projection과 화면에서 시계 방향인 앞면을 사용한다.
- 단일 D32 깊이 버퍼는 `DEPTH_WRITE`를 유지하며 매 프레임 1로 지운다. PSO는 LESS 비교·깊이 쓰기·뒷면 컬링을 사용한다.
- 전체 창 Viewport와 현재 종횡비 Projection을 사용한다. 좁은 창에서 물체가 잘릴 수 있으며 카메라 거리를 자동 조절하지 않는다.

### 카메라 이동과 입력 (2026-10-01)

`ProcessWindowMessage`가 키·드래그 상태를 모으고 [SandboxCamera.h](../Sandbox/SandboxCamera.h)의 `Move`·`Rotate`가 카메라를 갱신한다. `GetView`를 WVP 계산에 사용하며 렌더러는 입력을 참조하지 않는다.

초기 위치는 `(0, 2, -5)`, 시선은 원점이다. 전후는 시선 방향, 좌우는 수평, 상하는 월드 Y축으로 초당 3 단위 이동한다. 이동 벡터를 정규화해 대각선 속도를 맞춘다. 회전 감도는 픽셀당 0.003 rad이며 pitch를 ±(π/2 − 0.01)로 제한하고 yaw를 한 바퀴 범위로 줄인다.

포커스 해제·최소화·모달 크기 조절·메뉴 진입/종료 시 입력과 다음 이동 시간을 초기화한다. 일반 프레임 이동 시간은 최대 0.1초로 제한하므로 저프레임에서는 실제 시간보다 느리게 이동할 수 있다. 큐브 회전은 절대 경과 시간을 유지한다. 오른쪽 드래그 중에만 마우스를 캡처하며 캡처 상실 시 회전을 중단한다. Raw Input·커서 고정·충돌은 없다.

### 여러 오브젝트의 Transform (2026-10-05)

[Transform.h](../Sandbox/Transform.h)는 위치·Euler 회전(radian)·크기를 보유하고 `Scale * Rotation * Translation`을 계산한다. 부모 계층·ECS는 없다. Main의 큐브 3개가 서로 다른 Transform과 같은 카메라를 사용한다.

`Render(std::span<const XMFLOAT4X4>)`는 WVP 목록을 호출 중 복사하고 보관하지 않는다. 각 프레임 슬롯의 Constant Buffer에 오브젝트마다 256바이트 영역을 배정한다. 슬롯 Fence 완료 후 필요한 크기로 확장하며 감소 시 축소하지 않는다. 각 draw에서 root CBV 주소를 바꾸고, 빈 목록은 clear·Present만 수행한다. 크기 오버플로·생성 실패는 보고한다.

## 겹치는 오브젝트의 픽셀 검증 (2026-10-05)

`CheckOcclusion`은 서로 교차하지 않는 앞/뒤 큐브의 빈 화면·앞 단독·뒤 단독·정순·역순 이미지를 비교한다. 앞 큐브가 있는 픽셀은 앞 단독 이미지와, 나머지는 뒤 단독 이미지와 같아야 한다. 겹침·앞뒤 색 차이·뒤쪽 가시 영역이 각각 100픽셀 이상이어야 하므로 빈 화면으로 통과하지 않는다. 세 조건에서 카메라 위치와 해상도를 바꾼다.

[RendererReadback.h](../Tests/RendererReadback.h)의 테스트 전용 friend 어댑터가 동일한 내부 draw 경로를 Present 없이 실행한다. flip-discard 이후 내용에 의존하지 않기 위한 선택이며 앱의 공개 Render는 Present를 유지한다. GPU 대기 → COPY_SOURCE 전이·readback 복사·PRESENT 복귀 → GPU 대기 → Map 순서로 읽고, RowPitch 패딩을 제외한 RGBA8을 비교한다. 어댑터는 렌더러보다 먼저 파괴하며 자원 해제 전 GPU 완료를 기다린다. Capture 실패 시 테스트를 종료하며 실패한 어댑터의 재사용은 지원하지 않는다.

## 최신 검증 결과

2026-10-06 컨벤션 적용 후 Debug·Release 빌드와 RendererSmoke를 다시 실행했다. Sandbox는 두 구성 모두 숨김 창으로 실행하고 `WM_CLOSE`로 정상 종료했다. 커밋 전 diff 검토에서도 두 구성의 빌드·RendererSmoke·숨김 창 정상 종료를 재확인했다. 화면 확인은 이전 구현 검증 결과이며 이번에는 시각 확인·실제 입력 검증을 수행하지 않았다.

| 구분 | 확인한 결과 |
|---|---|
| 빌드 | x64 Debug·Release의 Sandbox·RendererSmoke·셰이더 빌드 성공 |
| CPU 검사 | Transform 기본값·SRT 순서, 카메라 초기 위치·대각선 속도·시간 분할·0초/무입력·큰 회전 입력의 유한 행렬 통과 |
| GPU 검사 | 두 구성 모두 픽셀 검증 15프레임·3회 리사이즈 + smoke 136프레임·8회 요청, 종료 코드 0, InfoQueue 오류·경고 0 |
| 앱 실행 | Debug·Release 숨김 창 초기화·메시지 루프·WM_CLOSE 종료, 두 구성 종료 코드 0. 화면·입력 검증은 제외 |
| Debug 화면 | 여러 큐브 도입 당시 배치·크기·독립 회전·가림 관찰 |
| Release 화면 | 현재 앱 배치의 큐브 3개, 최대화·최소화 후 복원·Alt+F4 후 창 종료 확인. UI 종료 코드는 미수집 |

두 구성의 픽셀 결과:

| 크기 | 겹침 픽셀 | 앞뒤 색이 다른 겹침 | 뒤 큐브만 보임 | 잘못된 픽셀 | 정순/역순 |
|---|---:|---:|---:|---:|---|
| 321×241 | 3614 | 3614 | 2648 | 0 | 완전 일치 |
| 640×360 | 6405 | 6405 | 7838 | 0 | 완전 일치 |
| 257×193 | 1568 | 1568 | 2553 | 0 | 완전 일치 |

현재 앱의 사용자 수정 배치와 픽셀 테스트 장면은 다르다. D3D12 오류·경고 0건은 자동 테스트 범위이며 실제 창의 진단 큐는 별도 수집하지 않았다.

### 남은 검증과 제약

- 실제 WASD·오른쪽 드래그를 통한 연속 카메라 이동, Alt+Tab 중 키 해제, 창 밖 버튼 해제, 모달 리사이즈 후 입력.
- 연속 드래그 리사이즈, Visual Studio F5, 최신 Cube CSO 누락·손상, PSO·Signal·Present 강제 실패와 장치 손실.
- 같은 깊이의 공면·교차 메시·투명도, 전체 live-object 누수 검사, 성능 측정.

자동 픽셀 검증은 세 카메라 위치·해상도에서 불투명 큐브의 가림을 확인한다. 실제 입력 경로나 모든 장면의 깊이 정확성을 대신하지 않는다.

## 주요 이력

| 시점 | 변경 |
|---|---|
| 2026-09-15 | 창·D3D12 초기화, 저장소와 출력 구조 정리 |
| 2026-09-17 | 매 프레임 전체 대기에서 프레임별 Allocator·Fence 재사용 대기로 전환 |
| 2026-09-24 | 삼각형을 인덱스 큐브·WVP·깊이 버퍼로 교체 |
| 2026-09-30 | 큐브 코드 정리, 사용자 정상 렌더링 확인과 두 구성 자동 검증 기록 |
| 2026-10-05 | 카메라 입력 → 여러 Transform → 겹침 픽셀 비교 연결 |
| 2026-10-06 | 지역 상수 명명·빈 본문·직접 include·API 계약 주석 정리, CPU 크기·span 인덱스에 표준 타입 시험 적용 |

### 삼각형 출력과 검증

2026-09-18 당시 고정 삼각형·중앙 정사각형 Viewport와 빌드 시 CSO를 연결했다. 두 구성의 빌드·smoke·화면 확인, EXE 기준 경로와 CSO 누락 오류를 검증했다. 2026-09-24 Cube 셰이더와 전체 창 Viewport로 교체했으며 당시 결과를 최신 큐브의 오류 경로 검증으로 간주하지 않는다. 배포에는 Cube CSO를 사용한다.

## API 참고

기존 구현 확인에 사용한 Microsoft 문서다.

- [Adapter 우선순위](https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_6/nf-dxgi1_6-idxgifactory6-enumadapterbygpupreference) · [Fence 완료 값](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12fence-getcompletedvalue)
- [Allocator Reset](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12commandallocator-reset) · [Command List Reset](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-reset)
- [Readback](https://learn.microsoft.com/en-us/windows/win32/direct3d12/readback-data-using-heaps) · [GetCopyableFootprints](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-getcopyablefootprints)
