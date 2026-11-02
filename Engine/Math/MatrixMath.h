#pragma once

#include "Math/MathUtils.h"
#include "Math/VectorMath.h"

#include <DirectXMath.h>

#include <cmath>
#include <cstddef>
#include <optional>

namespace rengine
{
	inline bool IsFinite(const DirectX::XMMATRIX& matrix)
	{
		return !DirectX::XMMatrixIsNaN(matrix) && !DirectX::XMMatrixIsInfinite(matrix);
	}

	/// @brief 행 벡터 규약의 유한한 아핀 행렬인지 정확한 마지막 열 값으로 검사한다.
	inline bool IsAffine(const DirectX::XMMATRIX& matrix)
	{
		return IsFinite(matrix) && DirectX::XMVectorGetW(matrix.r[0]) == 0 &&
			DirectX::XMVectorGetW(matrix.r[1]) == 0 && DirectX::XMVectorGetW(matrix.r[2]) == 0 &&
			DirectX::XMVectorGetW(matrix.r[3]) == 1;
	}

	/// @brief 행렬의 각 요소를 절대/상대 오차로 비교한다. NaN·Infinity는 일치로 취급하지 않는다.
	inline bool NearlyEqual(
		const DirectX::XMMATRIX& a,
		const DirectX::XMMATRIX& b,
		float absoluteTolerance,
		float relativeTolerance)
	{
		DirectX::XMFLOAT4X4 first{};
		DirectX::XMFLOAT4X4 second{};
		DirectX::XMStoreFloat4x4(&first, a);
		DirectX::XMStoreFloat4x4(&second, b);
		for (std::size_t row = 0; row < 4; ++row)
		{
			for (std::size_t column = 0; column < 4; ++column)
			{
				if (!NearlyEqual(first.m[row][column], second.m[row][column], absoluteTolerance, relativeTolerance))
				{
					return false;
				}
			}
		}

		return true;
	}

	/// @brief 일반 4x4 역행렬. 반전·투영 행렬도 허용하며 특이·비유한 입력/계산 결과는 nullopt다.
	/// @note float 행렬식이 0 또는 범위 밖이면 거부한다. 조건수나 오차를 보증하는 수치해석용 역행렬은 아니다.
	[[nodiscard]] inline std::optional<DirectX::XMMATRIX> Inverse(const DirectX::XMMATRIX& matrix)
	{
		if (!IsFinite(matrix))
		{
			return std::nullopt;
		}

		DirectX::XMVECTOR determinant{};
		const DirectX::XMMATRIX inverse = DirectX::XMMatrixInverse(&determinant, matrix);
		const float value = DirectX::XMVectorGetX(determinant);
		if (!std::isfinite(value) || value == 0 || !IsFinite(inverse))
		{
			return std::nullopt;
		}

		return inverse;
	}

	/// @brief 행 벡터 점 (x,y,z,1)을 변환한 뒤 w로 나눈다. w=0·비유한 결과는 nullopt다.
	[[nodiscard]] inline std::optional<DirectX::XMFLOAT3> TransformPoint(
		const DirectX::XMFLOAT3& point,
		const DirectX::XMMATRIX& matrix)
	{
		if (!IsFinite(point) || !IsFinite(matrix))
		{
			return std::nullopt;
		}

		const DirectX::XMVECTOR homogeneous = DirectX::XMVector4Transform(
			DirectX::XMVectorSetW(DirectX::XMLoadFloat3(&point), 1),
			matrix);
		DirectX::XMFLOAT4 stored{};
		DirectX::XMStoreFloat4(&stored, homogeneous);
		if (!IsFinite(stored) || stored.w == 0)
		{
			return std::nullopt;
		}

		const DirectX::XMFLOAT3 result{ stored.x / stored.w, stored.y / stored.w, stored.z / stored.w };
		if (!IsFinite(result))
		{
			return std::nullopt;
		}

		return result;
	}

	/// @brief 아핀 행렬로 방향 (x,y,z,0)을 변환한다. 이동은 무시하며 정규화하지 않는다.
	/// @note 표면 법선의 역전치 변환과 다르다. 법선에는 TransformMath의 BuildNormalTransform을 사용한다.
	/// @return 비아핀 행렬 또는 비유한 입력·계산 결과이면 nullopt.
	[[nodiscard]] inline std::optional<DirectX::XMFLOAT3> TransformDirection(
		const DirectX::XMFLOAT3& direction,
		const DirectX::XMMATRIX& matrix)
	{
		if (!IsFinite(direction) || !IsAffine(matrix))
		{
			return std::nullopt;
		}

		DirectX::XMFLOAT3 result{};
		DirectX::XMStoreFloat3(&result, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&direction), matrix));
		if (!IsFinite(result))
		{
			return std::nullopt;
		}

		return result;
	}
}
