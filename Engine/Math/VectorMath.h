#pragma once

#include "Math/MathUtils.h"

#include <DirectXMath.h>

#include <cmath>
#include <optional>

namespace rengine
{
	inline bool IsFinite(const DirectX::XMFLOAT3& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
	}

	inline bool IsFinite(const DirectX::XMFLOAT4& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z) && std::isfinite(value.w);
	}

	/// @brief 벡터의 각 성분을 비교한다. 거리 기준 비교가 아니다.
	inline bool NearlyEqual(
		const DirectX::XMFLOAT3& a,
		const DirectX::XMFLOAT3& b,
		float absoluteTolerance,
		float relativeTolerance)
	{
		return NearlyEqual(a.x, b.x, absoluteTolerance, relativeTolerance) &&
			NearlyEqual(a.y, b.y, absoluteTolerance, relativeTolerance) &&
			NearlyEqual(a.z, b.z, absoluteTolerance, relativeTolerance);
	}

	/// @brief 0·비유한 벡터는 nullopt. 매우 크거나 작은 유한 벡터도 double 길이로 정규화한다.
	[[nodiscard]] inline std::optional<DirectX::XMFLOAT3> Normalize(const DirectX::XMFLOAT3& value)
	{
		if (!IsFinite(value))
		{
			return std::nullopt;
		}

		const double length = std::hypot(
			static_cast<double>(value.x),
			static_cast<double>(value.y),
			static_cast<double>(value.z));
		if (length == 0)
		{
			return std::nullopt;
		}

		return DirectX::XMFLOAT3{
			static_cast<float>(value.x / length),
			static_cast<float>(value.y / length),
			static_cast<float>(value.z / length) };
	}

	/// @brief 방향 벡터에 직교 투영한다. direction은 단위 벡터일 필요가 없다.
	/// @return 방향이 0이거나 입력/출력이 비유한 값이면 nullopt.
	[[nodiscard]] inline std::optional<DirectX::XMFLOAT3> ProjectOnVector(
		const DirectX::XMFLOAT3& value,
		const DirectX::XMFLOAT3& direction)
	{
		const std::optional<DirectX::XMFLOAT3> unit = Normalize(direction);
		if (!unit || !IsFinite(value))
		{
			return std::nullopt;
		}

		DirectX::XMFLOAT3 result{};
		const DirectX::XMVECTOR axis = DirectX::XMLoadFloat3(&*unit);
		DirectX::XMStoreFloat3(
			&result,
			DirectX::XMVectorMultiply(axis, DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&value), axis)));
		if (!IsFinite(result))
		{
			return std::nullopt;
		}

		return result;
	}

	/// @brief 원점을 지나는 평면에 벡터를 투영한다. normal은 단위 벡터일 필요가 없다.
	/// @return 법선이 0이거나 입력·계산 결과가 비유한 값이면 nullopt.
	[[nodiscard]] inline std::optional<DirectX::XMFLOAT3> ProjectOnPlane(
		const DirectX::XMFLOAT3& value,
		const DirectX::XMFLOAT3& normal)
	{
		const std::optional<DirectX::XMFLOAT3> projected = ProjectOnVector(value, normal);
		if (!projected)
		{
			return std::nullopt;
		}

		const DirectX::XMFLOAT3 result{ value.x - projected->x, value.y - projected->y, value.z - projected->z };
		if (!IsFinite(result))
		{
			return std::nullopt;
		}

		return result;
	}

	/// @brief 법선에 대해 입사 벡터를 반사한다. 법선은 내부에서 정규화한다.
	/// @return 법선이 0이거나 입력·계산 결과가 비유한 값이면 nullopt.
	[[nodiscard]] inline std::optional<DirectX::XMFLOAT3> Reflect(
		const DirectX::XMFLOAT3& incident,
		const DirectX::XMFLOAT3& normal)
	{
		const std::optional<DirectX::XMFLOAT3> unit = Normalize(normal);
		if (!unit || !IsFinite(incident))
		{
			return std::nullopt;
		}

		DirectX::XMFLOAT3 result{};
		DirectX::XMStoreFloat3(
			&result,
			DirectX::XMVector3Reflect(DirectX::XMLoadFloat3(&incident), DirectX::XMLoadFloat3(&*unit)));
		if (!IsFinite(result))
		{
			return std::nullopt;
		}

		return result;
	}

	/// @brief 두 방향의 무부호 각도 [0,pi]를 radian으로 반환한다. 0·비유한 방향은 nullopt다.
	[[nodiscard]] inline std::optional<float> AngleBetween(
		const DirectX::XMFLOAT3& a,
		const DirectX::XMFLOAT3& b)
	{
		const std::optional<DirectX::XMFLOAT3> first = Normalize(a);
		const std::optional<DirectX::XMFLOAT3> second = Normalize(b);
		if (!first || !second)
		{
			return std::nullopt;
		}

		const double dot = static_cast<double>(first->x) * second->x +
			static_cast<double>(first->y) * second->y + static_cast<double>(first->z) * second->z;
		// acos(dot)보다 평행·반평행 근처에 안정적이며 같은 방향의 정규화 반올림을 각도로 확대하지 않는다.
		const double crossX = static_cast<double>(first->y) * second->z - static_cast<double>(first->z) * second->y;
		const double crossY = static_cast<double>(first->z) * second->x - static_cast<double>(first->x) * second->z;
		const double crossZ = static_cast<double>(first->x) * second->y - static_cast<double>(first->y) * second->x;
		return static_cast<float>(std::atan2(std::hypot(crossX, crossY, crossZ), dot));
	}
}
