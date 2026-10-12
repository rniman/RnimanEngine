#pragma once

#include <DirectXMath.h>

#include <cmath>
#include <optional>

/// @brief CPU/HLSL이 같은 순서로 읽는 행 벡터 행렬. GPU 자원을 소유하지 않는다.
struct RenderObject
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

/// @brief World·WVP·역전치 법선 행렬을 만든다. 앱과 검증 코드가 같은 변환 계약을 사용한다.
/// @return 비유한 값, 비아핀 World, 0 이하 행렬식 또는 역행렬 범위 초과이면 nullopt.
/// @note 양수 Scale의 회전·이동을 대상으로 하며 뒤집힌 면의 컬링 보정은 제공하지 않는다.
[[nodiscard]] inline std::optional<RenderObject> CreateRenderObject(
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

	const auto normal = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, world));
	const auto wvp = world * viewProjection;
	if (DirectX::XMMatrixIsNaN(normal) || DirectX::XMMatrixIsInfinite(normal) ||
		DirectX::XMMatrixIsNaN(wvp) || DirectX::XMMatrixIsInfinite(wvp))
	{
		return std::nullopt;
	}

	RenderObject object;
	DirectX::XMStoreFloat4x4(&object.worldViewProjection, wvp);
	DirectX::XMStoreFloat4x4(&object.world, world);
	DirectX::XMStoreFloat4x4(&object.normalTransform, normal);
	return object;
}
