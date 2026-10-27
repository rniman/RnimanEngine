# 엔진 설계 (Architecture)

[프로젝트 소개와 실행](../README.md) · [프로젝트 구조](project-structure.md) · [개발 설정과 검증](development.md)

현재 Sandbox의 실행·소유권·렌더링 계약을 관리한다. 미구현 기능의 범위와 완료 기준은 [로드맵](roadmap.md)과 [M02 계획](plans/m02-static-model.md)을 따른다.

## 설계 기준

- 앱은 실행 수명과 장면 상태, 렌더러는 GPU 자원·명령·동기화를 담당한다. 장면·ECS는 렌더 데이터를 통해 연결하고 D3D12 명령 목록·디스크립터·PSO를 직접 다루지 않는다.
- 파일 파싱은 CPU 데이터까지, 렌더링 코드는 GPU 업로드부터 담당한다. 소유자·참조 유효 기간·실패 후 정리를 명시한다.
- 추상화·라이브러리·범용 관리자는 실제 사용처와 해결할 문제가 생길 때 도입한다. 현재 렌더러는 고정 큐브·Phong 전용이다.

### 네임스페이스

Engine·Sandbox·Tests의 직접 작성 C++는 `rengine` 단일 루트를 사용한다. 예: `rengine::Transform`, `rengine::SandboxApp`. 파일 내부 보조 코드는 루트 안의 익명 네임스페이스에 두고 include는 루트 밖에 둔다. 전역 별칭·헤더의 using namespace는 사용하지 않는다.

`main`·`wWinMain`은 전역이며 외부 API·HLSL은 원래 이름을 유지한다. 폴더별 하위 네임스페이스는 만들지 않는다. 적용·검증 근거는 [개발 문서](development.md#컨벤션)를 따른다.

## 좌표축과 행렬

| 항목 | 현재 계약 |
|---|---|
| 월드·로컬 | 위쪽 `+Y`, 무회전 기준 오른쪽 `+X`·전방 `+Z`. 의미상 로컬 정면 축·거리 단위는 외부 에셋 도입 시 결정 |
| 뷰·투영 | `XMMatrixLookToLH`, `XMMatrixPerspectiveFovLH`, 뷰 앞쪽 `+Z`, near 0.1·far 100 |
| NDC·화면 | NDC X/Y는 −1~1·Z는 0~1. 화면은 왼쪽 위 원점·오른쪽 +X·아래쪽 +Y |
| 벡터·합성 | 행 벡터 `p * M`, `World = S * R * T`, `WVP = World * View * Projection` |
| 저장·전달 | `XMStoreFloat4x4` 행 우선 저장 → HLSL `row_major float4x4` → `mul(position, matrix)`. 전달용 전치 없음 |
| 법선 | World 역전치의 3×3 부분으로 변환한 뒤 PS에서 정규화 |
| 회전 | `rotation.x/y/z`는 pitch(X)/yaw(Y)/roll(Z), radian. `XMMatrixRotationRollPitchYaw` 사용 |

행 우선 저장·행 벡터 연산·좌표계의 손잡이는 각각 별도 선택이다. Direct3D가 월드 축이나 왼손 좌표계를 강제하지 않는다.

## Sandbox 실행 구조

### 진입점과 수명

[Main.cpp](../Sandbox/Main.cpp)의 `wWinMain` → [SandboxApp::Run](../Sandbox/SandboxApp.cpp) → 창 생성 → PNG 디코딩 → GPU 초기화·업로드 → ImGui 초기화 → 프레임 반복 → GPU 대기 → ImGui·GPU·창의 RAII 정리 순서다. 앱은 창을 렌더러보다 먼저 선언해 GPU 자원이 HWND보다 먼저 정리되게 한다.

초기 클라이언트 크기는 1280×720이다. Win32 오류는 API·오류 코드, D3D12 오류는 API·HRESULT·가능한 Device removed reason을 보고한다. 부분 자원은 RAII로 정리하며 실패 후 재사용·장치 손실 자동 복구는 지원하지 않는다.

### 창·입력

[Win32Window](../Engine/Platform/Win32Window.h)는 창 하나·UI 스레드에서 창 수명·메시지·입력을 관리한다.

- `GetState`는 크기·최소화·포커스·리사이즈 요청을 제공한다. 앱은 GPU 리사이즈 성공 후 `ClearResizeRequest`를 호출하고, 최소화·0 크기에서는 메시지를 기다린다.
- `ConsumeInput`은 복사본을 반환한다. 키 눌림은 유지하고 누적 마우스 이동·시간 초기화 요청은 비운다. 포커스 해제·최소화·모달 조작 시 입력을 초기화하고 캡처 상실 시 드래그를 중단한다.
- `GWLP_USERDATA`·HWND는 비소유이며 창 객체는 콜백 주소 유지를 위해 복사·이동하지 않는다. `PumpMessages`의 optional 종료 코드에서 0도 종료 요청이다. `WM_CLOSE` 이후 GPU 정리까지 HWND·콜백 상태를 유지한다.

### 앱 실행

SandboxApp은 창·렌더러·SandboxGui·카메라·조명·큐브 3개를 소유하며 복사·이동하지 않는다. `Run`은 한 번만 호출하고 메시지 → 최소화 대기 → 프레임 경계 리사이즈 → ImGui `NewFrame`·캡처 반영 → 입력 소비 → `Update` → `DrawTools`·ImGui `Render` → 렌더러 `Render`를 반복한다. 정상 종료는 GPU 완료 후 WM_QUIT 코드, 실패는 1을 반환한다.

카메라·조명은 포커스가 있을 때 조절한다. 입력 초기화 직후 이동 시간은 0, 일반 프레임은 최대 0.1초이며 큐브 회전은 경과 시간의 차분을 현재 각도에 더한다. 선택 큐브의 회전 편집은 자동 회전을 멈추고, 정지 중 시간은 적립하지 않아 재개 시 편집한 각도에서 이어간다. 카메라 초기 위치는 `(0, 2, -5)`·원점 시선이다. 키·속도는 [README](../README.md#카메라-조작)를 따른다.

### 프레임별 Fence 동기화

Sandbox의 큐브는 `SandboxApp::Cube`에 Transform·회전 축·초당 회전 속도·자동 회전 여부를 함께 보관한다. `INITIAL_OBJECTS`의 초기 항목 수로 실제 큐브와 렌더링 상수 배열 크기를 정하며 선택 UI는 실제 목록을 순회한다. 회전 축은 XMFLOAT3 멤버 포인터로 지정해 개체 주소와 분리한다. 초기 목록은 비어 있지 않은 고정 데모 목록이며 런타임 추가·삭제는 지원하지 않는다.

[D3D12Renderer](../Engine/Rendering/D3D12Renderer.cpp)는 두 프레임 슬롯의 Allocator·Constant Buffer·마지막 제출 Fence와 단일 Command List·Direct Queue·Fence/Event를 소유한다.

`Initialize`는 입력 검증 후 `CreateDevice`에 Debug 레이어·DXGI Factory·하드웨어 장치 선택을 맡기고 나머지 GPU 자원을 순서대로 생성한다. `RenderInternal`은 입력 검증과 슬롯 Fence 대기를 직접 수행한 뒤 `UploadFrameConstants`에 버퍼 생성·확장·Map·복사·Unmap을 맡긴다. 복사와 draw의 오브젝트 주소 계산은 파일 내부 `CONSTANT_STRIDE`를 공유한다. 두 헬퍼는 실패를 false로 전달하며 호출자는 이후 초기화 또는 제출을 중단한다.

슬롯 Fence 대기 → 상수 복사 → Allocator/List Reset → clear/큐브 draw/선택 UI draw → PRESENT 복귀 → 제출·Present·Signal 순서다. Fence는 단조 증가하며 완료 값 `UINT64_MAX`는 장치 제거로 처리한다. 슬롯 완료 전 자원을 재사용하지 않는다.

리사이즈·종료와 미완료 제출이 있는 소멸자는 전체 GPU 완료를 시도한다. Signal·Present 실패도 정리 대상이며 소멸자의 `(void)WaitForGpu()`는 진단 후 정리를 계속하는 예외다. 대기 실패 시 완료를 보장하지 않는다. Resize는 동일 크기·0 크기를 무시하고 GPU 대기 후 백 버퍼·깊이 버퍼를 재생성한다. 상수 버퍼는 슬롯의 첫 렌더링에서 `(오브젝트 수 + 1) × 256`바이트로 생성한다. 빈 목록도 조명용 256바이트를 사용한다. 생성·확장·복사는 슬롯 완료 후 수행하며 오브젝트 감소 시 축소하지 않는다. 최초 상수 버퍼 할당 실패는 Initialize가 아닌 Render에서 보고한다.

### ImGui 연결과 수명

`SandboxGui::Initialize`는 렌더러의 `GetOverlayContext`에서 device·direct queue·실제 sRGB RTV 형식·두 프레임 수를 대여한다. UI는 전용 shader-visible SRV 힙을 소유하며 기존 큐브 텍스처 힙은 유지한다. 장면 데이터·Engine은 ImGui 타입을 참조하지 않는다.

`SandboxGui::BeginFrame` → `SandboxApp::Update/DrawTools` → `SandboxGui::EndFrame` → `D3D12Renderer::Render`의 선택 `OverlayPass` → `SandboxGui::Record` 순서다. Record는 슬롯 Fence 대기·큐브 그리기 이후, PRESENT 전환 전에 같은 Command List에 기록하고 UI 힙으로 전환한다. 다음 프레임은 큐브 렌더 상태를 다시 설정한다. 콜백과 userData는 호출 중에만 사용하며 백 버퍼·Command List를 보관하지 않는다. 공식 DX12 백엔드에는 두 프레임 수를 전달하며 Record는 제출당 한 번 호출한다.

창의 `SetMessageHandler`는 모든 메시지를 UI에 전달한다. 반환값으로 창의 포커스·크기·종료 처리를 생략하지 않고 기본 처리에서 커서 등 처리 결과를 사용한다. `NewFrame` 후 `WantCaptureMouse/Keyboard`를 `SetInputCapture`에 반영해 누적 드래그·키를 비우고 새 앱 입력을 차단한다. 키 해제는 계속 반영하며, 캡처 해제 후 드래그는 새 오른쪽 버튼 누름으로 시작한다. UI 캡처 시작 시에는 공유 HWND의 캡처를 강제로 해제하지 않는다.

오른쪽 드래그와 HWND 캡처의 소유는 구분한다. 창은 직접 획득한 캡처만 오른쪽 버튼 해제 시 반환한다. UI 백엔드가 메시지 전달 단계에서 먼저 획득한 캡처는 빌려 쓰며, 백엔드가 마지막 마우스 버튼 해제까지 관리한다. UI로 입력을 넘길 때 앱 소유 표시는 비우고, 포커스 해제·취소·최소화는 기존처럼 캡처와 앱 입력을 정리한다.

SandboxGui는 렌더러 뒤에 선언해 먼저 파괴한다. 소멸자는 창 콜백 연결 해제 → GPU 대기 → DX12·Win32 백엔드 종료 → 문맥·힙 정리 순서로 부분 초기화·조기 반환도 처리한다. 대기 실패는 진단 후 최선의 정리를 수행하며 GPU 완료를 보장하지 않는다. 리사이즈는 기존 전체 GPU 대기·백 버퍼/깊이 재생성을 사용하고 UI 문맥·폰트·힙은 유지한다.

편집 진입점은 `DrawTools`다. 개체는 `DrawObjectTools`, 조명은 `DrawLightingTools`에서 앱 소유 상태에 적용한다. 벡터 위젯의 배열 변환은 파일 내부 `EditVector3`가 맡는다. Transform은 편집 복사본 전체의 유한·비특이·양수 행렬식 검사 후 공개하고 Scale을 0.05~10으로 제한한다. 카메라는 `SetPosition`, 조명은 `SetParameters/Reset`을 사용한다. 조명의 `Adjust`와 `SetParameters`는 `ApplyParameters`로 같은 보정을 적용하며 패널도 `PITCH_LIMIT`·`MAX_INTENSITY`를 공유한다. ms/frame·FPS는 프레임 간격이며 CPU/GPU 분리 프로파일러가 아니다. 공급자 백엔드의 실패 전달 한계는 [의존성](dependencies.md#dear-imgui)에 기록한다.

## 큐브 렌더링 계약

### 메시·행렬·렌더 데이터

면별 위치·UV·법선 정점 24개와 `R16_UINT` 인덱스 36개의 고정 Upload Heap 메시를 공유한다. 시계 방향 앞면·뒷면 컬링·전체 창 Viewport, D32 깊이 clear 1·LESS·깊이 쓰기를 사용한다.

`SandboxApp::Render` → [BuildObjectTransformConstants](../Engine/Rendering/RenderData.h) → WVP·World·World 역전치 → [CubeVS](../Shaders/Samples/Sandbox/CubeVS.hlsl) 순서다. 유한한 아핀 World·양수 행렬식·유한한 역행렬/WVP를 요구하며 위반 시 nullopt다. 반전 Transform의 컬링 보정은 없다.

`Render(std::span<const ObjectTransformConstants>, const FrameLighting&)`는 입력을 호출 중 복사하고 참조를 보관하지 않는다. CPU 크기·offset은 static_assert로 확인한다. ObjectTransformConstants는 192바이트, FrameLighting은 64바이트이며 CB의 첫 256바이트는 PS b1, 이후 오브젝트별 256바이트는 VS b0다. 빈 목록도 clear·Present하고 버퍼 크기 오버플로·생성 실패는 보고한다.

### 텍스처·sRGB

[LoadPngImage](../Engine/Assets/AssetLoader.cpp) → CPU [ImageData](../Engine/Assets/ImageData.h) → `D3D12Renderer::CreateTexture`로 연결한다. 원본 위치와 실패 시 경로 처리는 [공용 에셋 경로](project-structure.md#개발-실행의-공용-에셋-경로)를 따른다.

- WIC는 단일 프레임 PNG를 위→아래·행 패딩 없는 RGBA8로 디코딩한다. 크기·UINT 산술 범위·프레임 수를 검사하고 실패 시 빈 이미지·HRESULT·API를 반환한다. 앱은 전체 경로를 함께 표시한다. COM은 STA 초기화 성공 횟수만 정리하며 기존 MTA는 실패한다. ICC·EXIF 변환 없이 BaseColor를 sRGB로 간주한다.
- GPU 입력은 각 축 1~16384, `pixels.size() == width * height * 4`다. 단일 밉 Default Heap에 `GetCopyableFootprints`의 RowPitch로 복사하고 COPY_DEST→PIXEL_SHADER_RESOURCE로 전환한다. GPU 완료 후 Upload Heap을 해제하며 대기 실패 시 멤버에 보존해 소멸 시 다시 완료를 시도한다.
- 면마다 이미지 전체를 사용하고 비균일 Scale에서는 무늬도 늘어난다. 모든 큐브는 SRV t0·선형 Clamp Sampler s0를 공유하며 리사이즈 시 텍스처를 유지한다.
- Swap Chain은 RGBA8 UNORM, SRV·RTV·PSO 출력은 RGBA8 sRGB다. 선형 RGB로 조명 계산 후 sRGB로 기록한다. clearColor는 선형, PS alpha는 항상 1이다.

### 기본 Phong 조명

`SandboxApp::Update` → [SandboxLighting](../Sandbox/SandboxLighting.h)의 `Adjust`/`Reset` → FrameLighting·카메라 위치 → [CubePS](../Shaders/Samples/Sandbox/CubePS.hlsl) 순서다.

- `direction`은 빛이 진행하는 월드 단위 방향이며 표면→광원 L은 반대다. Diffuse는 `max(N·L, 0)`, Specular는 `reflect(-L, N)`과 표면→카메라 방향 내적의 거듭제곱이다. N·L≤0이면 Specular는 0이고 카메라=표면도 안전하게 처리한다.
- 출력은 `BaseColor * (Ambient + 광색 * 강도 * Diffuse) + 광색 * 강도 * 반사 강도 * Specular`다. Ambient는 직접광 강도와 독립적이며 1 초과 출력은 UNORM에서 잘린다.
- Render는 유한한 값, direction 길이 제곱 오차 ≤0.001, 강도 0~2, 광색·Ambient·반사 강도 0~1, 광택 지수 1~256을 요구한다. 위반은 API·HRESULT로 보고한다.

## 구현 선택과 후속 조건

ImageLoadResult·ExecutableDirectoryResult는 데이터·오류를 구분하는 결과 구조체, ObjectTransformConstants·FrameLighting은 공개 데이터다. SandboxLighting은 각도·강도 조건을 보호하는 Rule of Zero 값 클래스이며 ComApartment는 중복 COM 정리를 막기 위해 복사·이동하지 않는다. CPU 크기·span 인덱스는 std::size_t, D3D12 크기·Fence·출력은 API 타입을 유지하고 산술 범위를 검사한다. 타입·반환값 진단·PCH 예외는 [컨벤션 적용 상태](development.md#적용-상태)를 따른다.

### ECS와 도구

정적 모델 장면을 확보한 뒤 일반 오브젝트 방식과 ECS가 같은 렌더 데이터를 만들게 한다. 생성·삭제·이동/회전·컴포넌트 조합별 갱신을 같은 개체 수·작업량·빌드 구성에서 비교하고, 갱신/수집 비용과 사용성을 기록한다. 저장 방식·실행 순서·핸들 재사용·참조 무효화·순회 중 구조 변경 정책은 해당 단계에서 정한다. Transform 계층은 후속이다.

ImGui는 엔진의 상태 변경 경로와 GPU 수명 계약을 따르며 일반 장면 데이터가 ImGui 타입에 의존하지 않게 한다. 지표는 단위와 측정 대상을 표시하고 CPU·GPU 시간과 Present 대기를 구분한다. PBR 첫 후보는 metallic-roughness·직접광이며 IBL·그림자·후처리는 별도 단계다.

### Unity 데이터 규격

미구현이며 첫 전달 범위는 [M02 모델 단계](plans/m02-static-model.md#2단계--정적-모델-로딩과-렌더링)를 따른다. 파일 식별값·버전·바이트 순서·필드 타입·인코딩·개수·참조, 좌표·단위·winding·노멀·UV·텍스처 계약을 명시하고 C++ 객체 메모리를 파일 형식으로 사용하지 않는다.

파서는 파일 범위·산술 오버플로·인덱스·참조를 검사하고 원인이 드러나게 실패한다. 작은 기준 에셋으로 데이터·화면을 대조하며 규격 변경 시 익스포터·로더·에셋·문서와 호환/재익스포트 조건을 함께 갱신한다. 탄젠트·스켈레톤·애니메이션 계약은 해당 기능 도입 시 정한다.

## 현재 제약

- 고정 큐브·단일 BaseColor·방향광 하나만 지원한다. Normal/Roughness/AO/Height·DDS·압축·밉맵·캐시·텍스처 교체, 그림자·감쇠·다중 광원·PBR·HDR는 미구현이다.
- 상단 큐브 일부 잘림·단일 밉 축소 떨림이 남는다. DPI·최소 창 크기는 미정이며 모달 리사이즈 중 렌더링이 멈출 수 있다. Raw Input·커서 고정·충돌·장치 손실 복구는 없다.
- 오류 경로의 검증 범위는 [첫 마일스톤](milestones/v0.1.0-sandbox.md#당시-남은-검증과-제약)을 따른다.

## API 참고

기존 구현 확인에 사용한 Microsoft 문서: [프레임 자원 재사용](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12commandallocator-reset) · [Fence 완료 값](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12fence-getcompletedvalue) · [텍스처 업로드](https://learn.microsoft.com/en-us/windows/win32/direct3d12/upload-and-readback-of-texture-data) · [HLSL packing](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules)
