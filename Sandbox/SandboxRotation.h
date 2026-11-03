#pragma once

#include "Math/QuaternionMath.h"

#include <DirectXMath.h>

#include <cmath>
#include <optional>

namespace rengine
{
	/// @brief 모델별 degree 편집값. Quaternion이 바뀐 경우만 동기화해 사용자의 회전 횟수 표현을 유지한다.
	class SandboxRotation
	{
	public:
		DirectX::XMFLOAT3 GetDegrees() const
		{
			return mDegrees;
		}

		/// @return 비유한·0 Quaternion이면 false이며 편집 상태를 유지한다.
		[[nodiscard]] bool Synchronize(const DirectX::XMFLOAT4& rotation)
		{
			if (rotation.x == mLastRotation.x && rotation.y == mLastRotation.y &&
				rotation.z == mLastRotation.z && rotation.w == mLastRotation.w)
			{
				return true;
			}

			const std::optional<DirectX::XMFLOAT3> angles = ConvertQuaternionToEuler(rotation);
			if (!angles)
			{
				return false;
			}

			const auto nearest = [](float angle, float previous)
			{
				return previous + static_cast<float>(std::remainder(static_cast<double>(angle) - previous, 360.0));
			};
			const float x = DirectX::XMConvertToDegrees(angles->x);
			const float y = DirectX::XMConvertToDegrees(angles->y);
			const float z = DirectX::XMConvertToDegrees(angles->z);
			const DirectX::XMFLOAT3 primary{
				nearest(x, mDegrees.x), nearest(y, mDegrees.y), nearest(z, mDegrees.z) };
			const DirectX::XMFLOAT3 alternate{
				nearest(180 - x, mDegrees.x), nearest(y + 180, mDegrees.y), nearest(z + 180, mDegrees.z) };
			const auto distance = [this](const DirectX::XMFLOAT3& value)
			{
				return std::hypot(value.x - mDegrees.x, value.y - mDegrees.y, value.z - mDegrees.z);
			};
			mDegrees = distance(primary) <= distance(alternate) ? primary : alternate;
			mLastRotation = rotation;
			return true;
		}

		/// @brief degree 입력에서 단위 Quaternion을 만든다. 실패 시 상태를 유지한다.
		/// @return 비유한 입력이면 nullopt. 성공 시 입력 각도를 그대로 유지한다.
		[[nodiscard]] std::optional<DirectX::XMFLOAT4> SetDegrees(const DirectX::XMFLOAT3& value)
		{
			const std::optional<DirectX::XMFLOAT4> rotation = QuaternionFromEuler({
				DirectX::XMConvertToRadians(value.x),
				DirectX::XMConvertToRadians(value.y),
				DirectX::XMConvertToRadians(value.z) });
			if (!rotation)
			{
				return std::nullopt;
			}

			mDegrees = value;
			mLastRotation = *rotation;
			return rotation;
		}
	private:
		DirectX::XMFLOAT3 mDegrees{};
		DirectX::XMFLOAT4 mLastRotation{ 0, 0, 0, 1 };
	};
}
