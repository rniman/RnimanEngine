#pragma once

#include "Math/QuaternionMath.h"
#include "Math/Transform.h"

#include <DirectXMath.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>

namespace rengine
{
	/// @brief 유한한 아핀·양수 행렬식 World의 역전치 행렬을 계산한다. 전단은 허용한다.
	/// @return 입력 조건 위반 또는 역행렬 범위 초과이면 nullopt. GPU·셰이더 데이터는 사용하지 않는다.
	[[nodiscard]] inline std::optional<DirectX::XMMATRIX> BuildNormalTransform(const DirectX::XMMATRIX& world)
	{
		if (DirectX::XMMatrixIsNaN(world) || DirectX::XMMatrixIsInfinite(world) ||
			DirectX::XMVectorGetW(world.r[0]) != 0.0f || DirectX::XMVectorGetW(world.r[1]) != 0.0f ||
			DirectX::XMVectorGetW(world.r[2]) != 0.0f || DirectX::XMVectorGetW(world.r[3]) != 1.0f)
		{
			return std::nullopt;
		}

		const float determinant = DirectX::XMVectorGetX(DirectX::XMMatrixDeterminant(world));
		if (!std::isfinite(determinant) || determinant <= 0.0f)
		{
			return std::nullopt;
		}

		const DirectX::XMMATRIX normal = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, world));
		if (DirectX::XMMatrixIsNaN(normal) || DirectX::XMMatrixIsInfinite(normal))
		{
			return std::nullopt;
		}

		return normal;
	}

	struct TransformDecompositionResult
	{
		// 성공 시에만 값이 있다. 실패 시 error는 정적 문자열이며 해제할 필요가 없다.
		std::optional<Transform> transform;
		const char* error = nullptr;
	};

	/// @brief World를 양수 Scale·Quaternion의 Transform으로 분해하고 재구성 오차를 검사한다.
	/// @return 실패 시 빈 transform과 원인. 로그·예외 변환·파일 경로 표시는 호출자가 담당한다.
	[[nodiscard]] inline TransformDecompositionResult DecomposeTransform(const DirectX::XMMATRIX& world)
	{
		if (!BuildNormalTransform(world))
		{
			return { std::nullopt, "Unsupported non-affine, singular or reflected model transform" };
		}

		DirectX::XMVECTOR scale{};
		DirectX::XMVECTOR rotation{};
		DirectX::XMVECTOR translation{};
		if (!DirectX::XMMatrixDecompose(&scale, &rotation, &translation, world))
		{
			return { std::nullopt, "Model transform cannot be decomposed as TRS" };
		}

		Transform transform;
		DirectX::XMStoreFloat3(&transform.position, translation);
		DirectX::XMStoreFloat3(&transform.scale, scale);
		if (transform.scale.x <= 0 || transform.scale.y <= 0 || transform.scale.z <= 0)
		{
			return { std::nullopt, "Model requires positive TRS scale" };
		}

		DirectX::XMFLOAT4 quaternion{};
		DirectX::XMStoreFloat4(&quaternion, rotation);
		const std::optional<DirectX::XMFLOAT4> normalized = NormalizeQuaternion(quaternion);
		if (!normalized)
		{
			return { std::nullopt, "Model rotation cannot be normalized" };
		}

		transform.rotation = *normalized;

		// 편집 가능한 TRS로 재구성되지 않는 전단을 조용히 근사하지 않는다.
		DirectX::XMFLOAT4X4 original{};
		DirectX::XMFLOAT4X4 rebuilt{};
		DirectX::XMStoreFloat4x4(&original, world);
		DirectX::XMStoreFloat4x4(&rebuilt, transform.GetWorld());
		for (std::size_t row = 0; row < 4; ++row)
		{
			for (std::size_t column = 0; column < 4; ++column)
			{
				if (std::abs(original.m[row][column] - rebuilt.m[row][column]) >
					1e-4f * std::max(1.0f, std::abs(original.m[row][column])))
				{
					return { std::nullopt, "Model transform contains unsupported shear or unstable TRS" };
				}
			}
		}

		return { transform, nullptr };
	}
}
