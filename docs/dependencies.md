# 외부 라이브러리와 에셋 처리

[프로젝트 소개와 실행](../README.md)

## 외부 라이브러리와 도구

필요한 기능부터 도입하고 입출력, 소유권, 실패 동작을 이해하며 연결한다. 현재 사용 중인 구성과 후속 도입 후보를 구분해 기록한다. 새로 도입할 때 버전·출처·라이선스·사용 범위와 준비 방법을 기록한다.

### 현재 사용하는 이미지 로딩

2026-10-12 Sandbox는 Windows SDK의 WIC·COM을 직접 사용한다. [AssetLoader](../Sandbox/AssetLoader.cpp)가 단일 프레임 PNG를 CPU RGBA8로 반환하고 D3D12Renderer가 GPU 업로드를 담당한다. 외부 WICTextureLoader12·DirectXTex 코드는 아직 도입하지 않았다. 링크 의존성은 `windowscodecs.lib`·`ole32.lib`이며 별도 외부 패키지 설치는 없다.

현재 원본은 별도로 준비한 `Assets/BrickWall17_MR_1K/BrickWall17_1K_BaseColor.png`다. 저장소 루트의 `Assets/` 전체를 Git에서 제외하며, Sandbox의 Debug·Release는 같은 원본을 직접 읽는다. PNG를 빌드 출력으로 복사하지 않는다. 원본 준비와 개발 실행 경로의 제약은 [README](../README.md#빌드와-실행)를 따른다. 실제 원본의 배포 출처·라이선스는 확인하지 않았다. 테스트 기준 PNG는 직접 작성해 `Tests/Assets/`에 포함하며 RendererSmoke도 원본을 직접 읽는다. DDS·압축·밉맵·변환 캐시·독립 배포 패키징은 아래 후속 계획이며 현재 실행에는 적용하지 않았다.

### 현재 사용하는 수학 연산

수학 연산 기반으로 [DirectXMath](https://github.com/microsoft/DirectXMath)를 사용한다. 좌표계와 CPU/HLSL 전달 계약은 [좌표축과 행렬](design.md#좌표축과-행렬)을 따른다.

### 우선 도입 후보

| 구성 | 역할과 사용 범위 |
|---|---|
| [d3dx12.h](https://github.com/microsoft/DirectX-Headers) | D3D12 구조체와 반복 작업을 돕는 헬퍼. 초기화·명령 제출·동기화 흐름은 직접 구현한다. |
| [WICTextureLoader12](https://github.com/microsoft/DirectXTex/tree/main/WICTextureLoader) | 초기 데모에서 일반 이미지를 직접 로딩할 때 활용한다. DDS 변환·캐시 생성 기능과는 구분한다. |
| [DDSTextureLoader12](https://github.com/microsoft/DirectXTex/tree/main/DDSTextureLoader) | 준비된 DDS와 변환 캐시를 읽는 실행 경로에 활용한다. |
| [DirectXTex / texconv](https://github.com/microsoft/DirectXTex) | 원본 이미지의 형식 변환·압축·밉맵 생성과 DDS 저장. 변환 API를 연결할지 도구를 호출할지는 구현 시 선택한다. |
| [Dear ImGui](https://github.com/ocornut/imgui) | 개발용 상태 표시·값 편집 UI. Inspector 동작과 성능 계측은 엔진에서 구현하며, 게임 로직은 ImGui에 의존하지 않는다. |

### 필요할 때 검토할 후보

| 구성 | 검토 조건 |
|---|---|
| [DXC](https://github.com/microsoft/DirectXShaderCompiler) | 셰이더 빌드 경로와 대상 Shader Model을 정할 때 검토한다. |
| [MikkTSpace](https://github.com/mmikk/MikkTSpace) | 탄젠트 생성이 필요할 때 검토한다. Unity에서 내보낸 탄젠트를 사용할 수 있으면 재생성하지 않고, 노멀맵과 좌표 변환의 일치 여부를 확인한다. |
| [spdlog](https://github.com/gabime/spdlog) | 콘솔·파일 로그 관리가 필요할 때 검토한다. |
| [Tracy](https://github.com/wolfpld/tracy) | 실제 시스템의 실행 시간을 분석할 때 검토한다. |
| [D3D12 Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator) | 기본 GPU 자원 관리를 이해한 뒤 메모리 할당 관리가 필요할 때 검토한다. 동기화와 자원 사용 수명은 엔진이 관리한다. |
| [RmlUi](https://github.com/mikke89/RmlUi) | HUD·인벤토리·메뉴 등 플레이어용 UI가 필요할 때 검토한다. ImGui 개발 도구와 병행할 수 있다. |

모델·씬·애니메이션 입력은 Unity 익스포터와 직접 작성하는 bin 로더가 담당하며 **Assimp는 도입 대상에서 제외한다.** ECS는 직접 구현을 주력으로 하고, [EnTT](https://github.com/skypjack/entt)는 설계 참고나 비교 대상으로 둔다.

### 텍스처 변환과 캐시

초기 데모는 원본 이미지 직접 로딩으로 시작할 수 있다. 에셋 처리 단계에서는 개발 실행 중 시작·장면 로딩 시 필요한 텍스처를 변환하고, 이후 실행에서는 DDS 캐시를 재사용하는 흐름을 만든다. 플레이 도중의 동기 변환은 피한다.

```text
텍스처 요청 → 캐시 유효성 확인
               ├─ 유효 → DDS 로드 → GPU 업로드
               └─ 없음·변경 → 원본 변환·압축·밉맵 생성 → DDS 캐시 저장 → 로드
```

- DDS 자체를 캐시 파일로 사용한다. 원본 내용과 해상도·압축 형식·밉맵·색 공간 설정, 변환 도구·처리 규칙 버전을 캐시 키 또는 메타데이터에 반영한다.
- 불완전하거나 손상된 캐시는 재생성하고, 변환 실패 원인을 표시한다. 변환과 저장이 끝난 결과만 유효한 캐시로 취급한다.
- 첫 변환 비용과 이후 캐시 로딩 시간을 구분해 측정한다. 디스크 파일 크기와 GPU 메모리 사용량은 별도로 확인한다. DDS 확장자만으로 압축 여부를 판단하지 않는다.
- 최종 배포에는 미리 변환한 DDS를 준비한다. 개발용 변환 기능과 원본 파일을 배포 실행에 필수로 요구하지 않는다.
- 캐시는 재생성 가능한 산출물로 관리하고, 경로·보관·정리 정책은 구현 시 정한다. 원본과 변환 설정은 재현 가능하게 보관한다.
