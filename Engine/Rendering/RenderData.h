#pragma once

#include <DirectXMath.h>

#include <cmath>
#include <optional>

namespace rengine
{
	/// @brief 오브젝트별 셰이더 변환 상수. CPU/HLSL이 같은 순서로 읽으며 GPU 자원을 소유하지 않는다.
	struct ObjectTransformConstants
	{
		DirectX::XMFLOAT4X4 worldViewProjection;
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4X4 normalTransform;
	};

	/// @brief 프레임 공통 Phong 값. 색상은 선형, direction은 빛이 진행하는 월드 방향이다.
	struct FrameLighting
	{
		DirectX::XMFLOAT3 direction{ 0.0f, -1.0f, 0.0f };
		float intensity = 0.8f;
		DirectX::XMFLOAT3 color{ 1.0f, 1.0f, 1.0f };
		float ambientStrength = 0.12f;
		DirectX::XMFLOAT3 cameraPosition{ 0.0f, 2.0f, -5.0f };
		float specularStrength = 0.25f;
		float shininess = 32.0f;
		DirectX::XMFLOAT3 padding{};
	};

	/// @brief World를 검증하고 World·WVP·역전치 법선 상수를 계산한다. 장면 오브젝트나 GPU 자원은 생성하지 않는다.
	/// @return 비유한 값, 비아핀 World, 0 이하 행렬식 또는 역행렬 범위 초과이면 nullopt.
	/// @note 양수 Scale의 회전·이동을 대상으로 하며 뒤집힌 면의 컬링 보정은 제공하지 않는다.
	[[nodiscard]] inline std::optional<ObjectTransformConstants> BuildObjectTransformConstants(
		const DirectX::XMMATRIX& world,
		const DirectX::XMMATRIX& viewProjection)
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
		const DirectX::XMMATRIX wvp = world * viewProjection;
		if (DirectX::XMMatrixIsNaN(normal) || DirectX::XMMatrixIsInfinite(normal) ||
			DirectX::XMMatrixIsNaN(wvp) || DirectX::XMMatrixIsInfinite(wvp))
		{
			return std::nullopt;
		}

		ObjectTransformConstants constants;
		DirectX::XMStoreFloat4x4(&constants.worldViewProjection, wvp);
		DirectX::XMStoreFloat4x4(&constants.world, world);
		DirectX::XMStoreFloat4x4(&constants.normalTransform, normal);
		return constants;
	}
}
