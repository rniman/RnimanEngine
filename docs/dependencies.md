# 외부 라이브러리와 에셋 처리

[프로젝트 소개와 실행](../README.md) · [M02 계획](plans/m02-static-model.md)

필요한 기능부터 도입하고 입출력·소유권·실패 동작을 확인한다. 새 의존성은 버전·출처·라이선스·사용 범위·준비 방법을 기록한다.

## 현재 사용 중인 구성

| 구성 | 사용 범위 |
|---|---|
| Windows SDK WIC·COM | [AssetLoader](../Engine/Assets/AssetLoader.cpp)가 PNG를 CPU RGBA8로 디코딩하고 렌더러가 GPU 업로드. windowscodecs.lib·ole32.lib 사용, 별도 패키지 설치 없음 |
| [DirectXMath](https://github.com/microsoft/DirectXMath) | Transform·카메라·렌더 데이터 계산. [좌표축과 행렬 계약](design.md#좌표축과-행렬) 적용 |

Sandbox의 BaseColor PNG는 별도 준비하며 실제 원본의 배포 출처·라이선스는 미확인이다. 테스트 PNG는 직접 작성해 Tests/Assets에 포함한다. 준비·로딩 경로는 [README](../README.md#빌드와-실행)를 따른다. DDS·압축·밉맵·변환 캐시·독립 배포는 미구현이다.

## 도입 계획과 후보

| 구성 | 도입 시점·조건 |
|---|---|
| [Dear ImGui](https://github.com/ocornut/imgui) | 다음 구현. Win32·D3D12 백엔드로 개발용 편집·상태 표시를 연결. 상세 범위는 [M02 1단계](plans/m02-static-model.md#1단계--imgui-최소-도입) |
| [d3dx12.h](https://github.com/microsoft/DirectX-Headers) | D3D12 반복 작업용 헬퍼가 필요할 때. 초기화·제출·동기화는 직접 관리 |
| [WICTextureLoader12](https://github.com/microsoft/DirectXTex/tree/main/WICTextureLoader) | 일반 이미지 로딩의 대체 후보. 현재 직접 구현한 WIC 로더와 구분 |
| [DDSTextureLoader12](https://github.com/microsoft/DirectXTex/tree/main/DDSTextureLoader), [DirectXTex / texconv](https://github.com/microsoft/DirectXTex) | DDS 로딩·이미지 변환·압축·밉맵이 필요할 때. 변환 API와 도구 호출 중 구현 방식 선택 |
| [DXC](https://github.com/microsoft/DirectXShaderCompiler) | Shader Model 6 기능 도입 시 셰이더 빌드와 함께 검토 |
| [MikkTSpace](https://github.com/mmikk/MikkTSpace) | 탄젠트 생성이 필요할 때. Unity 탄젠트가 있으면 변환·노멀맵과 일치 여부를 먼저 확인 |
| [spdlog](https://github.com/gabime/spdlog), [Tracy](https://github.com/wolfpld/tracy) | 각각 콘솔/파일 로그 관리, 실행 시간 분석이 필요할 때 |
| [D3D12 Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator) | GPU 메모리 할당 관리가 필요할 때. 동기화·자원 수명은 엔진 책임 |
| [RmlUi](https://github.com/mikke89/RmlUi) | 플레이어용 HUD·인벤토리·메뉴가 필요할 때. ImGui 개발 도구와 병행 가능 |

모델·씬·애니메이션은 Unity 익스포터와 자체 bin 로더로 전달하며 **Assimp는 도입하지 않는다.** ECS도 직접 구현을 주력으로 하고 [EnTT](https://github.com/skypjack/entt)는 설계 참고·비교 대상으로 둔다.

## 텍스처 변환과 캐시

후속 계획이다. 시작·장면 로딩 시 필요한 텍스처를 변환하고 이후에는 DDS 캐시를 재사용한다. 플레이 중 동기 변환은 피한다.

```text
텍스처 요청 → 캐시 확인 → 유효하면 DDS 로드 → GPU 업로드
                       → 없거나 변경됐으면 원본 변환·압축·밉맵 생성 → 저장 → 로드
```

- 원본 내용·해상도·압축 형식·밉맵·색 공간·변환 도구/규칙 버전으로 캐시를 구분한다. 손상된 캐시는 재생성하고 변환 실패 원인을 표시하며 완성된 결과만 공개한다.
- 최초 변환과 재로딩 시간을 따로 측정하고 디스크 크기·GPU 메모리도 구분한다. DDS 확장자만으로 압축 여부를 판단하지 않는다.
- 배포에는 미리 변환한 DDS를 준비해 개발용 변환기·원본을 요구하지 않게 한다. 캐시는 재생성 가능한 결과로 관리하고 원본·변환 설정은 보존한다. [배치·Git 관리](project-structure.md#git-관리)
