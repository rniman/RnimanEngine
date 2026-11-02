# 외부 라이브러리와 에셋 처리

[프로젝트 소개와 실행](../README.md) · [M02 계획](roadmap.md#m02-상태와-완료-기준)

필요한 기능부터 도입하고 입출력·소유권·실패 동작을 확인한다. 새 의존성은 버전·출처·라이선스·사용 범위·준비 방법을 기록한다.

## 현재 사용 중인 구성

| 구성 | 사용 범위 |
|---|---|
| Windows SDK WIC·COM | [AssetLoader](../Engine/Assets/AssetLoader.cpp)가 PNG·JPEG를 CPU RGBA8로 디코딩하고 렌더러가 GPU 업로드. windowscodecs.lib·ole32.lib 사용, 별도 패키지 설치 없음 |
| [DirectXMath](https://github.com/microsoft/DirectXMath) | Transform·카메라·렌더 데이터 계산. [좌표축과 행렬 계약](design.md#좌표축과-행렬) 적용 |

Sandbox는 BrickWallCube·BrickWallSphere의 BaseColor를 벽돌 PNG에, DamagedHelmet의 BaseColor를 별도 폴더의 JPEG에 연결한다. WIC로 PNG·JPEG를 RGBA8로 읽으며 PNG 전용 API도 유지한다. 테스트 PNG는 직접 작성해 Tests/Assets에 포함한다. 준비·로딩 경로는 [README](../README.md#빌드와-실행)를 따른다. DDS·압축·밉맵·변환 캐시·독립 배포는 미구현이다.

## Dear ImGui

- 버전: [v1.93.0](https://github.com/ocornut/imgui/releases/tag/v1.93.0), 커밋 `c1aaac65e896a45f81f47a4d68f9959e0f86c859`의 master 계열. 도킹·다중 뷰포트는 사용하지 않는다.
- 공급: [ThirdParty/imgui](../ThirdParty/imgui/README.rniman.md)에 코어·Demo·Win32/DX12 백엔드와 필요한 헤더만 원본 그대로 보관한다. [MIT 라이선스](../ThirdParty/imgui/LICENSE.txt), [SHA-256 목록](../ThirdParty/imgui/SHA256SUMS.txt)을 포함한다. 별도 설치·빌드 중 네트워크 복원은 없다.
- 빌드: [Build/ImGui.props](../Build/ImGui.props)를 Sandbox·ImGuiSmoke만 import한다. 동일 `/MDd`·`/MD` 런타임을 사용하고 공급자 소스는 PCH 없이 `/W3`·`/WX` 해제로 컴파일한다. 자체 Engine·장면 데이터에는 ImGui 의존성을 추가하지 않는다.
- 사용: [SandboxGui](../Sandbox/SandboxGui.cpp)가 단일 문맥·Win32/DX12 백엔드·전용 64칸 SRV 힙을 소유한다. 내장 폰트 사용, ini·로그 파일 쓰기는 비활성화한다. 사용자 텍스처 미리보기·폰트 파일·번역 UI는 아직 없다.
- 한계: 공급자 DX12 백엔드의 일부 GPU 할당/Map·업로드 실패는 `void` 반환·assert 기반으로 처리되므로 앱으로 상세 HRESULT를 전달하지 못한다. 자체 초기화·힙·장치 제거는 진단하며, descriptor 고갈/잘못된 해제는 오류 표시 후 `std::terminate`로 종료하므로 스택 해제·RAII 정리를 보장하지 않는다. 이 시험 적용 예외와 보완 조건은 [전체 컨벤션 재검토](development.md#적용-상태)에 구분했다. 메모리 부족·장치 손실 복구는 보장하지 않는다. 필요 시 공급자 백엔드의 실패 전파를 별도 검토한다.

## ReferenceImporter와 에셋

[ReferenceImporter](../ThirdParty/ReferenceImporter/README.md)는 Binary v2 Reader 소스 3개·규격·사본 해시를 포함한다. 공급자 네임스페이스·예외 API를 유지하며 Build/ModelImporter.props로 Sandbox·RendererSmoke에서만 빌드한다. 별도 라이선스 파일은 확인하지 못했다.

Unity Exporter·원본 장면은 외부 프로젝트에서 관리한다. `Assets/`의 모델 BIN·대조용 JSON·이미지는 Git 제외이며 [준비·재익스포트 조건](static-model.md#기준-에셋-준비)을 따른다. Tests/Assets의 PNG·JPEG·고정 ModelFixture·Legacy BIN·Wrap BIN은 회귀 검증용으로 포함한다. BaseColor 외 맵·전체 PBR 외형은 재현하지 않는다.

## 후속 후보

현재 모델 전달은 Unity 익스포터·자체 로더로 유지하며 Assimp는 도입하지 않는다. ECS는 직접 구현을 주력으로 하고 EnTT를 설계 참고·비교 대상으로 둔다. DDS·압축·밉맵이 필요하면 DirectXTex/texconv, Shader Model 6이면 DXC, 계측이 필요하면 Tracy를 검토한다. 아직 빌드 의존성이 아니다.

### 텍스처 변환과 캐시

미구현 계획이다. 도입 시 원본 내용·색 공간·해상도·압축·밉맵·변환 도구 버전으로 유효성을 판정하고 완성된 결과만 공개한다. 변환은 로딩 시점에 수행하며 플레이 중 동기 변환을 피한다. 최초 변환과 재로딩 비용을 따로 측정하고, 배포에는 미리 변환한 결과를 준비한다. 원본·변환 설정은 보존하고 재생성 가능한 캐시만 Git에서 제외한다.
