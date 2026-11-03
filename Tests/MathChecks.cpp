#include "MathChecks.h"

#include "Math/MathUtils.h"
#include "Math/MatrixMath.h"
#include "Math/QuaternionMath.h"
#include "Math/Transform.h"
#include "Math/TransformMath.h"
#include "Math/VectorMath.h"
#include "SandboxRotation.h"

#include <DirectXMath.h>

#include <array>
#include <cmath>
#include <cstdio>
#include <limits>
#include <optional>

namespace rengine
{
	namespace
	{
		bool Check(bool condition, const char* label)
		{
			if (!condition)
			{
				std::printf("FAIL: Math %s\n", label);
			}

			return condition;
		}

		bool Near(const std::optional<float>& value, float expected)
		{
			return value && NearlyEqual(*value, expected, 1e-5f, 1e-5f);
		}

		bool Near(const std::optional<DirectX::XMFLOAT3>& value, const DirectX::XMFLOAT3& expected)
		{
			return value && NearlyEqual(*value, expected, 1e-5f, 1e-5f);
		}
	}

	bool CheckMathUtilities()
	{
		using DirectX::XMFLOAT3;
		using DirectX::XMFLOAT4;
		const float infinity = std::numeric_limits<float>::infinity();
		const float nan = std::numeric_limits<float>::quiet_NaN();
		const float large = (std::numeric_limits<float>::max)();
		const float tiny = (std::numeric_limits<float>::min)();
		if (!Check(
			NearlyEqual(0, 1e-6f, 1e-5f, 0) && NearlyEqual(1000000, 1000001, 0, 2e-6f) &&
			!NearlyEqual(0, 1, 0, 0) && !NearlyEqual(infinity, infinity, 1, 1) &&
			!NearlyEqual(nan, nan, 1, 1) && !NearlyEqual(1, 1, -1, 0) &&
			!NearlyEqual(1, 1, 0, infinity) && !NearlyEqual(large, -large, 0, 1),
			"finite absolute/relative comparison"))
		{
			return false;
		}

		if (!Check(
			NearlyEqual(DirectX::XMConvertToRadians(180), DirectX::XM_PI, 1e-6f, 0) &&
			NearlyEqual(DirectX::XMConvertToDegrees(DirectX::XM_PIDIV2), 90, 1e-5f, 0) &&
			Near(WrapRadians(DirectX::XM_PI), -DirectX::XM_PI) &&
			Near(WrapRadians(-DirectX::XM_PI), -DirectX::XM_PI) &&
			Near(WrapRadians(2 * DirectX::XM_PI), 0) &&
			Near(
				DeltaAngle(DirectX::XMConvertToRadians(170), DirectX::XMConvertToRadians(-170)),
				DirectX::XMConvertToRadians(20)) &&
			Near(
				DeltaAngle(DirectX::XMConvertToRadians(-170), DirectX::XMConvertToRadians(170)),
				DirectX::XMConvertToRadians(-20)) &&
			!WrapRadians(infinity) && !DeltaAngle(nan, 0),
			"angle boundaries and shortest direction"))
		{
			return false;
		}

		if (!Check(
			Near(LerpClamped(-large, large, 0.5f), 0) && Near(LerpClamped(2, 4, -1), 2) &&
			Near(LerpClamped(2, 4, 2), 4) && Near(InverseLerpClamped(10, 0, 7.5f), 0.25f) &&
			Near(InverseLerpClamped(-large, large, 0), 0.5f) &&
			Near(RemapClamped(25, 0, 100, -1, 1), -0.5f) &&
			Near(RemapClamped(200, 0, 100, 1, -1), -1) &&
			Near(SmoothStep(0, 1, 0.5f), 0.5f) && Near(SmoothStep(0, 1, -1), 0) &&
			Near(MoveTowards(0, 1, 2), 1) && Near(MoveTowards(5, 0, 2), 3) &&
			Near(MoveTowards(-large, large, large), 0) &&
			!InverseLerpClamped(1, 1, 1) && !RemapClamped(1, 0, 1, nan, 1) &&
			!LerpClamped(0, 1, nan) && !SmoothStep(0, infinity, 1) &&
			!MoveTowards(0, 1, -1),
			"interpolation bounds and invalid intervals"))
		{
			return false;
		}

		if (!Check(
			Near(Normalize(XMFLOAT3{ 3, 4, 0 }), { 0.6f, 0.8f, 0 }) &&
			Near(Normalize(XMFLOAT3{ large, large, 0 }), { 0.70710678f, 0.70710678f, 0 }) &&
			Near(Normalize(XMFLOAT3{ tiny, 0, 0 }), { 1, 0, 0 }) &&
			!Normalize(XMFLOAT3{}) && !Normalize(XMFLOAT3{ infinity, 0, 0 }) &&
			!Normalize(XMFLOAT3{ nan, 0, 0 }),
			"normalization range and failure"))
		{
			return false;
		}

		if (!Check(
			Near(ProjectOnVector({ 2, 3, 4 }, { 0, 2, 0 }), { 0, 3, 0 }) &&
			Near(ProjectOnPlane({ 2, 3, 4 }, { 0, 2, 0 }), { 2, 0, 4 }) &&
			Near(Reflect({ 1, -2, 0 }, { 0, 7, 0 }), { 1, 2, 0 }) &&
			Near(AngleBetween({ 2, 0, 0 }, { 0, 3, 0 }), DirectX::XM_PIDIV2) &&
			Near(AngleBetween({ 1, 0, 0 }, { -1, 0, 0 }), DirectX::XM_PI) &&
			Near(AngleBetween({ 1, 2, 3 }, { 2, 4, 6 }), 0) &&
			!ProjectOnVector({ 1, 2, 3 }, {}) && !ProjectOnPlane({ 1, 2, 3 }, {}) &&
			!Reflect({ 1, 2, 3 }, {}) && !AngleBetween({}, { 1, 0, 0 }),
			"projection, reflection and angle"))
		{
			return false;
		}

		const DirectX::XMMATRIX world = DirectX::XMMatrixScaling(2, 3, 4) * DirectX::XMMatrixTranslation(10, 20, 30);
		const std::optional<DirectX::XMMATRIX> inverse = Inverse(world);
		if (!Check(
			inverse && NearlyEqual(world * *inverse, DirectX::XMMatrixIdentity(), 1e-5f, 1e-5f) &&
			Near(TransformPoint({ 1, 2, 3 }, world), { 12, 26, 42 }) &&
			Near(TransformDirection({ 1, 2, 3 }, world), { 2, 6, 12 }) &&
			Inverse(DirectX::XMMatrixScaling(-1, 1, 1)).has_value() &&
			!Inverse(DirectX::XMMatrixScaling(0, 1, 1)),
			"matrix inverse and point/direction distinction"))
		{
			return false;
		}

		const DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, 1, 1, 10);
		DirectX::XMMATRIX invalid = DirectX::XMMatrixIdentity();
		invalid.r[0] = DirectX::XMVectorSetX(invalid.r[0], nan);
		if (!Check(
			!IsAffine(projection) && Inverse(projection).has_value() &&
			Near(TransformPoint({ 0, 0, 1 }, projection), { 0, 0, 0 }) &&
			Near(TransformPoint({ 0, 0, 10 }, projection), { 0, 0, 1 }) &&
			!TransformPoint({}, projection) && !TransformDirection({ 1, 0, 0 }, projection) &&
			!Inverse(invalid) && !NearlyEqual(invalid, invalid, 1, 1) &&
			!TransformPoint({ 1, 2, 3 }, invalid),
			"projection divide and non-finite rejection"))
		{
			return false;
		}

		const std::optional<XMFLOAT4> yaw = QuaternionFromAxisAngle({ 0, 5, 0 }, DirectX::XM_PIDIV2);
		const std::optional<XMFLOAT4> pitch = QuaternionFromEuler({ DirectX::XM_PIDIV2, 0, 0 });
		if (!Check(
			yaw && pitch && !NormalizeQuaternion({ 0, 0, 0, 0 }) &&
			!NormalizeQuaternion({ nan, 0, 0, 1 }) &&
			NormalizeQuaternion({ large, large, large, large }).has_value() &&
			!QuaternionFromAxisAngle({}, 1) && !QuaternionFromEuler({ 0, infinity, 0 }),
			"Quaternion input contracts"))
		{
			return false;
		}

		const std::optional<XMFLOAT4> composed = ComposeRotations(*pitch, *yaw);
		if (!composed)
		{
			return Check(false, "Quaternion composition result");
		}

		const std::optional<XMFLOAT3> sequentialFirst = RotateVector({ 0, 1, 0 }, *pitch);
		if (!sequentialFirst)
		{
			return Check(false, "Quaternion first rotation");
		}

		const std::optional<XMFLOAT3> sequential = RotateVector(*sequentialFirst, *yaw);
		const std::optional<XMFLOAT3> combined = RotateVector({ 0, 1, 0 }, *composed);
		const DirectX::XMMATRIX expectedRotation = DirectX::XMMatrixRotationX(DirectX::XM_PIDIV2) *
			DirectX::XMMatrixRotationY(DirectX::XM_PIDIV2);
		if (!Check(
			sequential && combined && NearlyEqual(*sequential, *combined, 1e-5f, 1e-5f) &&
			NearlyEqual(
				DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&*composed)),
				expectedRotation,
				1e-5f,
				1e-5f) &&
			Near(RotateVector({ 0, 0, 1 }, *yaw), { 1, 0, 0 }),
			"Quaternion order and handedness"))
		{
			return false;
		}

		const XMFLOAT4 identity{ 0, 0, 0, 1 };
		const XMFLOAT4 negative{ -yaw->x, -yaw->y, -yaw->z, -yaw->w };
		for (const float t : std::array{ 0.0f, 0.5f, 1.0f })
		{
			const std::optional<XMFLOAT4> same = Slerp(*yaw, negative, t);
			const std::optional<XMFLOAT4> interpolated = Slerp(identity, *yaw, t);
			if (!same || !interpolated)
			{
				return Check(false, "Slerp result");
			}

			if (!Check(
				Near(RotateVector({ 0, 0, 1 }, *same), { 1, 0, 0 }) &&
				NearlyEqual(
					DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&*interpolated)),
					DirectX::XMMatrixRotationY(DirectX::XM_PIDIV2 * t),
					1e-5f,
					1e-5f),
				"Slerp endpoints, midpoint and q/-q"))
			{
				return false;
			}
		}

		if (!Check(
			!Slerp(identity, *yaw, -1) && !Slerp(identity, *yaw, nan) &&
			!ComposeRotations(identity, { 0, 0, 0, 0 }) && !RotateVector({ infinity, 0, 0 }, identity),
			"Quaternion failure propagation"))
		{
			return false;
		}

		// 행렬에서 Quaternion으로 복원하는 로딩 경로와 표시용 Euler 경로를 각각 검사한다.
		for (const float sign : std::array{ -1.0f, 1.0f })
		{
			for (const float offset : std::array{ -0.001f, -0.000001f, 0.0f, 0.000001f, 0.001f })
			{
				const XMFLOAT3 angles{ sign * DirectX::XM_PIDIV2 + offset, -0.6f, 0.4f };
				const std::optional<XMFLOAT4> rotation = QuaternionFromEuler(angles);
				if (!rotation)
				{
					return Check(false, "boundary Quaternion creation");
				}

				Transform source;
				source.position = { -2, 3, 4 };
				source.scale = { 2, 0.7f, 1.3f };
				source.rotation = *rotation;
				const TransformDecompositionResult restored = DecomposeTransform(source.GetWorld());
				const std::optional<XMFLOAT3> euler = ConvertQuaternionToEuler(*rotation);
				const std::optional<XMFLOAT4> roundTrip = euler ? QuaternionFromEuler(*euler) : std::nullopt;
				if (!Check(
					restored.transform && NearlyEqual(
						source.GetWorld(),
						restored.transform->GetWorld(),
						1e-4f,
						1e-4f) &&
					roundTrip && NearlyEqual(
						DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&*rotation)),
						DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&*roundTrip)),
						1e-5f,
						1e-5f),
					"Quaternion TRS and display Euler near both poles"))
				{
					return false;
				}
			}
		}

		DirectX::XMMATRIX shear = DirectX::XMMatrixIdentity();
		shear.r[0] = DirectX::XMVectorSet(1, 0.5f, 0, 0);
		if (!Check(
			!DecomposeTransform(shear).transform &&
			!DecomposeTransform(DirectX::XMMatrixScaling(0, 1, 1)).transform &&
			!DecomposeTransform(DirectX::XMMatrixScaling(-1, 1, 1)).transform &&
			!ConvertQuaternionToEuler({ 0, 0, 0, 0 }) && !ConvertQuaternionToEuler({ nan, 0, 0, 1 }),
			"TRS unsupported transforms and invalid Euler input"))
		{
			return false;
		}

		SandboxRotation editor;
		const std::optional<XMFLOAT4> entered = editor.SetDegrees({ 0, 370, 0 });
		if (!Check(entered.has_value(), "Euler edit conversion"))
		{
			return false;
		}

		if (!editor.Synchronize(*entered))
		{
			return Check(false, "Euler edit synchronization");
		}

		if (!Check(editor.GetDegrees().y == 370, "Euler edit turns retained"))
		{
			return false;
		}

		const std::optional<XMFLOAT4> invalidEdit = editor.SetDegrees({ nan, 0, 0 });
		if (!Check(!invalidEdit && editor.GetDegrees().y == 370, "invalid Euler edit preserves state"))
		{
			return false;
		}

		const std::optional<XMFLOAT4> external = QuaternionFromEuler({ 0, DirectX::XMConvertToRadians(20), 0 });
		if (!external)
		{
			return Check(false, "external rotation creation");
		}

		if (!editor.Synchronize(*external))
		{
			return Check(false, "external rotation synchronization");
		}

		if (!Check(
			NearlyEqual(editor.GetDegrees().y, 380, 1e-4f, 0),
			"external Quaternion synchronizes nearest Euler turn"))
		{
			return false;
		}

		const std::optional<XMFLOAT4> beforePole = editor.SetDegrees({ 89, 20, 30 });
		const std::optional<XMFLOAT4> afterPole = QuaternionFromEuler({
			DirectX::XMConvertToRadians(91),
			DirectX::XMConvertToRadians(20),
			DirectX::XMConvertToRadians(30) });
		if (!beforePole || !afterPole)
		{
			return Check(false, "pole edit and external rotation creation");
		}

		if (!editor.Synchronize(*afterPole))
		{
			return Check(false, "pole rotation synchronization");
		}

		if (!Check(
			NearlyEqual(editor.GetDegrees(), { 91, 20, 30 }, 0.001f, 0),
			"Euler external update selects continuous branch across pole"))
		{
			return false;
		}

		std::puts("Math CPU: comparisons, angles, interpolation, vectors, matrices and Quaternion contracts passed");
		return true;
	}
}
