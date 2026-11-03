# 헤더 전용 Math

[README](../README.md) · [프로젝트 구조](project-structure.md#헤더-전용-math-모듈)

`Engine/Math/Math.vcxproj`는 헤더를 관리하는 Utility 프로젝트다. 별도 lib·cpp·MathTests 프로젝트는 없다. 소비자는 Engine include 경로에서 필요한 헤더만 포함한다. Math 프로젝트 자체의 헤더 편집·분석에도 C++20·Engine include 경로·`/utf-8`을 지정한다. 일반 함수는 inline이며 PCH·Assets·Rendering·ImGui에 의존하지 않는다.

## 타입과 규약

기존 DirectXMath의 XMFLOAT3·XMFLOAT4·XMMATRIX를 사용한다. 자체 Vector/Matrix 래퍼와 MathTypes 별칭 계층은 도입하지 않았다. Quaternion은 XMFLOAT4 `(x,y,z,w)`이며 단위 회전은 `(0,0,0,1)`이다. 일반 XMFLOAT4와 타입으로 구분되지 않으므로 Quaternion 함수 이름으로 의도를 표시한다.

행 벡터·World = Scale × Rotation × Translation을 유지한다. Euler 입력은 radian의 pitch(X)·yaw(Y)·roll(Z)이며 실제 회전 적용 순서는 Roll → Pitch → Yaw다. 회전 API는 radian을 받는다. ConvertQuaternionToEuler는 UI 표시용으로만 사용한다. Transform은 정규화된 Quaternion을 저장한다.

## 연산 목록

| 헤더 | 제공 기능 |
|---|---|
| [MathUtils.h](../Engine/Math/MathUtils.h) | NearlyEqual, WrapRadians·DeltaAngle, LerpClamped·InverseLerpClamped·RemapClamped·SmoothStep·MoveTowards |
| [VectorMath.h](../Engine/Math/VectorMath.h) | XMFLOAT3/4의 IsFinite, XMFLOAT3의 NearlyEqual·Normalize·ProjectOnVector·ProjectOnPlane·Reflect·AngleBetween |
| [MatrixMath.h](../Engine/Math/MatrixMath.h) | IsFinite·IsAffine·NearlyEqual, Inverse·TransformPoint·TransformDirection |
| [QuaternionMath.h](../Engine/Math/QuaternionMath.h) | NormalizeQuaternion·QuaternionFromEuler·ConvertQuaternionToEuler·QuaternionFromAxisAngle·ComposeRotations·Slerp·RotateVector |
| [Transform.h](../Engine/Math/Transform.h) | 위치·Quaternion 회전·Scale 데이터와 GetWorld |
| [TransformMath.h](../Engine/Math/TransformMath.h) | 기존 BuildNormalTransform·DecomposeTransform과 실패 원인 |

기본 덧셈·내적·외적·행렬 곱·뷰/투영 생성은 DirectXMath, 일반 Clamp·Lerp는 표준 라이브러리를 사용할 수 있다. 위 함수는 실패 검증·단위·보간 범위 등 프로젝트 계약을 더한다. 보간/이동·투영·회전 기능은 사용자의 요청에 따라 후속 구현을 위해 미리 준비했으며 현재 Sandbox의 연산을 일괄 교체하지 않았다.

## 실패와 수치 범위

- 실패 가능한 연산은 `std::optional`로 성공 값 또는 nullopt를 반환한다. 함수 이름에는 Try 접두사를 붙이지 않으며 반환형·`[[nodiscard]]`·API 주석으로 실패 계약을 표시한다. 실패 시 0·Identity로 대체하지 않는다. 기존 DecomposeTransform은 TransformDecompositionResult로 원인도 반환한다. 로그·오류 UI는 호출자가 맡는다.
- NearlyEqual은 유한한 값에 `|a-b| <= max(절대 오차, 상대 오차 × max(|a|, |b|))`를 적용한다. 허용 오차는 호출자가 명시한다. 음수·비유한 오차와 NaN·Infinity 비교는 false다. 벡터·행렬 비교는 성분별 비교다.
- Normalize는 0벡터를 거부하며 길이를 double로 계산한다. Quaternion도 같은 방식으로 0을 거부한다. 투영·반사·Quaternion 함수는 축·법선·회전을 내부 정규화한다. 나머지 DirectXMath 계산은 float 범위이며 중간 계산/출력 overflow는 실패할 수 있다.
- Inverse는 일반 4×4에 사용할 수 있어 반전·투영 행렬도 허용한다. float 행렬식이 0·비유한 값이거나 역행렬이 비유한 값이면 실패한다. 조건수·잔차를 보증하지 않으므로 수치적으로 불안정한 입력의 정책은 사용처에서 정한다. 기존 BuildNormalTransform의 양수 행렬식 제한과 다르다.
- 각도 단위 변환은 DirectXMath의 `XMConvertToRadians`·`XMConvertToDegrees`를 사용한다. 입력·결과의 유한성은 호출자가 확인한다. WrapRadians는 float pi를 기준으로 `[-pi,pi)`에 접고 DeltaAngle은 최단 부호 각도를 반환한다. 정확히 반 바퀴면 `-pi`를 선택한다. 큰 입력에서 이미 사라진 각도 정밀도는 복구하지 않는다.
- Clamped 보간·Remap·SmoothStep은 구간 밖을 끝점으로 제한하며 역방향 구간도 허용한다. 역보간의 같은 끝점은 실패다. MoveTowards의 maxDelta는 음수일 수 없다.
- Slerp는 정규화한 회전 사이의 최단 경로를 보간하며 유한한 t `[0,1]`만 받는다. q와 -q는 같은 회전이다. ComposeRotations(first, second)는 first 후 second이며 `M(first) * M(second)`와 일치한다.
- TransformPoint는 `(x,y,z,1) * M` 후 w로 나눈다. w=0이면 실패한다. TransformDirection은 아핀 행렬에서 이동을 제외하고 크기 변화를 유지한다. 표면 법선은 방향 변환과 달리 역전치가 필요하므로 BuildNormalTransform을 사용한다.

## 사용 예

```cpp
#include "Math/VectorMath.h"

#include <DirectXMath.h>
#include <optional>

const std::optional<DirectX::XMFLOAT3> direction =
    rengine::Normalize(DirectX::XMFLOAT3{ 3.0f, 0.0f, 4.0f });
if (!direction)
{
    // 호출 목적에 맞게 실패를 전달한다.
    return false;
}
```

## MathChecks 검증

MathChecks는 **기존 RendererSmoke에 포함된 CPU 수학 검사 코드**다. [MathChecks.h](../Tests/MathChecks.h)에 진입점을 선언하고 [MathChecks.cpp](../Tests/MathChecks.cpp)의 `CheckMathUtilities()`에서 검사한다. Math Utility는 헤더 관리용이며, 별도 MathTests 프로젝트나 독립 Math 테스트 실행 파일은 구성하지 않았다.

[RendererSmoke의 main](../Tests/RendererSmoke.cpp)은 GPU 초기화와 에셋 검사 전에 `CheckMathUtilities()`를 호출한다. 실패하면 `FAIL: Math ...` 진단을 출력하고 false를 반환하며, main은 종료 코드 1로 검증을 중단한다. 성공하면 기존 렌더러 검사를 계속한다.

| 범위 | 주요 검사 |
|---|---|
| 수치·각도·보간 | 절대/상대 오차, 비유한 입력, 각도 경계·최단 방향, 보간 끝점·역방향·큰 구간 |
| 벡터 | 0·큰·작은 벡터 정규화, 투영·반사, 평행·반평행 각도 |
| 행렬 | 역행렬·반전·특이 입력, 점/방향 구분, 투영의 near/far·w=0 |
| Quaternion | 잘못된 입력, 회전 순서·합성, Slerp의 양 끝점·중간점, q와 -q의 동등성 |

`RendererSmoke --cpu`로 GPU 없이 실행할 수 있다. 명령과 준비 조건은 [테스트 실행 방법](development.md#자동-검증)을 따른다.

모델 fixture의 TRS 검사는 기존 [ModelChecks.cpp](../Tests/ModelChecks.cpp)의 `CheckModelTransforms`가 담당한다. 양쪽 ±90도와 인접 각도의 실제 파일 로딩·행렬 보존을 검사한다. MathChecks는 표시용 Euler 왕복, 전단·특이·반전 거부와 SandboxRotation의 370° 유지·외부 회전 동기화도 검사한다.

## Quaternion 저장과 Euler 편집

Transform::rotation은 정규화된 XMFLOAT4이며 기본값은 `(0,0,0,1)`이다. 기존 XMFLOAT3 회전 대입은 QuaternionFromEuler 등의 생성 함수로 바꿔야 한다. GetWorld는 단위 Quaternion을 전제로 하며 임의의 네 성분을 회전각처럼 수정하지 않는다.

DecomposeTransform은 Scale·Quaternion·Translation을 저장한 뒤 원본 행렬과 재구성 결과를 비교한다. ConvertQuaternionToEuler는 double 계산과 atan2로 표시 각도를 구하며 극점에서는 roll=0을 선택한다. 회전의 유일한 Euler 표현이나 극점에서의 연속 표시를 보장하지 않는다.

[SandboxRotation](../Sandbox/SandboxRotation.h)은 모델별 Euler degree와 마지막 Quaternion을 비공개로 보관한다. GetDegrees로 편집 복사본을 얻고 SetDegrees·Synchronize를 통해 갱신한다. UI 입력은 degree→radian→Quaternion으로 반영하고, 같은 회전을 다시 읽어 편집값을 덮어쓰지 않는다. 외부 Quaternion 변경 시 동등한 두 Euler 분기와 360° 주기 중 이전 값에 가까운 표현을 선택한다. UI 표시·동기화는 Transform이나 로더의 책임이 아니다.
