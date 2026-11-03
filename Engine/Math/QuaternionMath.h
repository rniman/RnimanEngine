#pragma once

#include "Math/VectorMath.h"

#include <DirectXMath.h>

#include <cmath>
#include <optional>

namespace rengine
{
	/// @brief Quaternion은 XMFLOAT4의 (x,y,z,w)로 전달한다. 0·비유한 입력은 nullopt다.
	[[nodiscard]] inline std::optional<DirectX::XMFLOAT4> NormalizeQuaternion(const DirectX::XMFLOAT4& value)
	{
		if (!IsFinite(value))
		{
			return std::nullopt;
		}

		const double length = std::sqrt(
			static_cast<double>(value.x) * value.x + static_cast<double>(value.y) * value.y +
			static_cast<double>(value.z) * value.z + static_cast<double>(value.w) * value.w);
		if (length == 0)
		{
			return std::nullopt;
		}

		return DirectX::XMFLOAT4{ static_cast<float>(value.x / length), static_cast<float>(value.y / length),
			static_cast<float>(value.z / length), static_cast<float>(value.w / length) };
	}

	/// @brief radian의 pitch(X), yaw(Y), roll(Z)에서 단위 Quaternion 생성. 적용 순서는 Roll→Pitch→Yaw다.
	/// @return 입력이나 계산 결과가 비유한 값이거나 결과를 정규화할 수 없으면 nullopt.
	[[nodiscard]] inline std::optional<DirectX::XMFLOAT4> QuaternionFromEuler(const DirectX::XMFLOAT3& angles)
	{
		if (!IsFinite(angles))
		{
			return std::nullopt;
		}

		DirectX::XMFLOAT4 result{};
		DirectX::XMStoreFloat4(&result, DirectX::XMQuaternionRotationRollPitchYaw(angles.x, angles.y, angles.z));
		return NormalizeQuaternion(result);
	}

	/// @brief Quaternion을 표시용 pitch/yaw/roll(radian)로 변환한다. 입력은 내부 정규화한다.
	/// @return 0·비유한 입력이면 nullopt. pitch는 [-pi/2,pi/2]이며 극점에서는 roll=0을 선택한다.
	/// @note Euler는 유일하지 않다. 저장·회전 누적에는 Quaternion을 사용하고 UI는 편집값을 별도 유지한다.
	[[nodiscard]] inline std::optional<DirectX::XMFLOAT3> ConvertQuaternionToEuler(const DirectX::XMFLOAT4& value)
	{
		if (!IsFinite(value))
		{
			return std::nullopt;
		}

		const double length = std::sqrt(
			static_cast<double>(value.x) * value.x + static_cast<double>(value.y) * value.y +
			static_cast<double>(value.z) * value.z + static_cast<double>(value.w) * value.w);
		if (length == 0)
		{
			return std::nullopt;
		}

		// float 행렬의 asin 입력이 극점 근처에서 ±1로 반올림되는 것을 피한다.
		const double x = value.x / length;
		const double y = value.y / length;
		const double z = value.z / length;
		const double w = value.w / length;
		const double m12 = 2 * (x * y + w * z);
		const double m22 = 1 - 2 * (x * x + z * z);
		const double cosine = std::hypot(m12, m22);
		const double pitch = std::atan2(2 * (w * x - y * z), cosine);
		const double yaw = cosine > 1e-6 ?
			std::atan2(2 * (x * z + w * y), 1 - 2 * (x * x + y * y)) :
			std::atan2(2 * (w * y - x * z), 1 - 2 * (y * y + z * z));
		const double roll = cosine > 1e-6 ? std::atan2(m12, m22) : 0;
		return DirectX::XMFLOAT3{
			static_cast<float>(pitch), static_cast<float>(yaw), static_cast<float>(roll) };
	}

	/// @brief 축은 내부에서 정규화하고 각도는 radian으로 받는다. 0축·비유한 입력은 nullopt다.
	[[nodiscard]] inline std::optional<DirectX::XMFLOAT4> QuaternionFromAxisAngle(
		const DirectX::XMFLOAT3& axis,
		float angle)
	{
		const std::optional<DirectX::XMFLOAT3> unit = Normalize(axis);
		if (!unit || !std::isfinite(angle))
		{
			return std::nullopt;
		}

		DirectX::XMFLOAT4 result{};
		DirectX::XMStoreFloat4(&result, DirectX::XMQuaternionRotationNormal(DirectX::XMLoadFloat3(&*unit), angle));
		return NormalizeQuaternion(result);
	}

	/// @brief first 회전 후 second 회전을 적용한다. 행렬의 M(first)*M(second)와 같은 순서다.
	/// @note 입력은 내부 정규화한다. DirectX의 XMQuaternionMultiply(first,second)는 second*first를 반환한다.
	/// @return 입력 Quaternion이 0·비유한 값이거나 결과를 정규화할 수 없으면 nullopt.
	[[nodiscard]] inline std::optional<DirectX::XMFLOAT4> ComposeRotations(
		const DirectX::XMFLOAT4& first,
		const DirectX::XMFLOAT4& second)
	{
		const std::optional<DirectX::XMFLOAT4> a = NormalizeQuaternion(first);
		const std::optional<DirectX::XMFLOAT4> b = NormalizeQuaternion(second);
		if (!a || !b)
		{
			return std::nullopt;
		}

		DirectX::XMFLOAT4 result{};
		DirectX::XMStoreFloat4(
			&result,
			DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&*a), DirectX::XMLoadFloat4(&*b)));
		return NormalizeQuaternion(result);
	}

	/// @brief 최단 회전 경로로 보간한다. q와 -q는 같은 회전이다. t는 유한한 [0,1]만 허용한다.
	/// @return t의 범위 위반, 0·비유한 Quaternion 또는 정규화할 수 없는 결과이면 nullopt.
	[[nodiscard]] inline std::optional<DirectX::XMFLOAT4> Slerp(
		const DirectX::XMFLOAT4& first,
		const DirectX::XMFLOAT4& second,
		float t)
	{
		if (!std::isfinite(t) || t < 0 || t > 1)
		{
			return std::nullopt;
		}

		const std::optional<DirectX::XMFLOAT4> a = NormalizeQuaternion(first);
		const std::optional<DirectX::XMFLOAT4> b = NormalizeQuaternion(second);
		if (!a || !b)
		{
			return std::nullopt;
		}

		DirectX::XMFLOAT4 result{};
		DirectX::XMStoreFloat4(
			&result,
			DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&*a), DirectX::XMLoadFloat4(&*b), t));
		return NormalizeQuaternion(result);
	}

	/// @brief Quaternion을 정규화한 뒤 벡터를 회전한다.
	/// @return 0 Quaternion 또는 비유한 입력·계산 결과이면 nullopt.
	[[nodiscard]] inline std::optional<DirectX::XMFLOAT3> RotateVector(
		const DirectX::XMFLOAT3& value,
		const DirectX::XMFLOAT4& rotation)
	{
		const std::optional<DirectX::XMFLOAT4> unit = NormalizeQuaternion(rotation);
		if (!unit || !IsFinite(value))
		{
			return std::nullopt;
		}

		DirectX::XMFLOAT3 result{};
		DirectX::XMStoreFloat3(
			&result,
			DirectX::XMVector3Rotate(DirectX::XMLoadFloat3(&value), DirectX::XMLoadFloat4(&*unit)));
		if (!IsFinite(result))
		{
			return std::nullopt;
		}

		return result;
	}
}
