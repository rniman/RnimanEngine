#include "LightingChecks.h"

#include "D3D12Renderer.h"
#include "RenderData.h"
#include "RendererReadback.h"
#include "SandboxLighting.h"

// Windows 선언은 DirectX 헤더보다 먼저 필요하다.
#include <Windows.h>
#include <DirectXMath.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <optional>
#include <vector>

namespace
{
	float DecodeSrgb(int value)
	{
		const float encoded = static_cast<float>(value) / 255.0f;
		return encoded <= 0.04045f ? encoded / 12.92f : std::pow((encoded + 0.055f) / 1.055f, 2.4f);
	}

	int EncodeSrgb(float value)
	{
		const float linear = std::clamp(value, 0.0f, 1.0f);
		const float encoded = linear <= 0.0031308f ? linear * 12.92f :
			1.055f * std::pow(linear, 1.0f / 2.4f) - 0.055f;
		return static_cast<int>(std::lround(encoded * 255.0f));
	}

	bool CheckPixel(
		std::uint32_t actual,
		std::uint32_t baseline,
		const DirectX::XMFLOAT3& scale,
		const DirectX::XMFLOAT3& bias,
		const char* operation)
	{
		const float factors[] = { scale.x, scale.y, scale.z };
		const float offsets[] = { bias.x, bias.y, bias.z };
		for (UINT channel = 0; channel < 4; ++channel)
		{
			const int observed = (actual >> (channel * 8)) & 0xff;
			const int source = (baseline >> (channel * 8)) & 0xff;
			const int expected = channel == 3 ? 255 :
				EncodeSrgb(DecodeSrgb(source) * factors[channel] + offsets[channel]);
			if (channel == 3 ? observed != 255 : std::abs(observed - expected) > 2)
			{
				std::printf("FAIL: %s channel=%u actual=%d expected=%d\n", operation, channel, observed, expected);
				return false;
			}
		}

		return true;
	}

	FrameLighting CreateAmbientLighting()
	{
		FrameLighting lighting;
		lighting.ambientStrength = 1.0f;
		lighting.intensity = 0.0f;
		lighting.specularStrength = 0.0f;
		return lighting;
	}
}

bool CheckLightingData()
{
	SandboxLighting lighting;
	const DirectX::XMFLOAT3 initial = lighting.GetDirection();
	SandboxLighting stepped;
	for (int index = 0; index < 10; ++index)
	{
		lighting.Adjust(1.0f, 0.5f, 0.25f, 0.1f);
		stepped.Adjust(1.0f, 0.5f, 0.25f, 0.05f);
		stepped.Adjust(1.0f, 0.5f, 0.25f, 0.05f);
	}

	const DirectX::XMFLOAT3 direction = lighting.GetDirection();
	const DirectX::XMFLOAT3 splitDirection = stepped.GetDirection();
	if (!DirectX::XMVector3NearEqual(
		DirectX::XMLoadFloat3(&direction),
		DirectX::XMLoadFloat3(&splitDirection),
		DirectX::XMVectorReplicate(0.0001f)) ||
		std::abs(lighting.GetIntensity() - stepped.GetIntensity()) > 0.0001f ||
		std::abs(lighting.GetIntensity() - 1.05f) > 0.0001f)
	{
		std::puts("FAIL: lighting time subdivision");
		return false;
	}

	lighting.Adjust(-1.0f, -1.0f, -1.0f, 0.0f);
	const DirectX::XMFLOAT3 unchanged = lighting.GetDirection();
	if (!DirectX::XMVector3Equal(DirectX::XMLoadFloat3(&direction), DirectX::XMLoadFloat3(&unchanged)))
	{
		return false;
	}

	for (int index = 0; index < 200; ++index)
	{
		lighting.Adjust(1.0f, 1.0f, 1.0f, 0.1f);
	}

	const DirectX::XMFLOAT3 upper = lighting.GetDirection();
	if (lighting.GetIntensity() != 2.0f || upper.y >= 1.0f || upper.y < 0.99f ||
		std::abs(DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DirectX::XMLoadFloat3(&upper))) - 1.0f) > 0.0001f)
	{
		std::puts("FAIL: lighting upper limits or unit direction");
		return false;
	}

	for (int index = 0; index < 200; ++index)
	{
		lighting.Adjust(-1.0f, -1.0f, -1.0f, 0.1f);
	}

	const DirectX::XMFLOAT3 lower = lighting.GetDirection();
	if (lighting.GetIntensity() != 0.0f || lower.y <= -1.0f || lower.y > -0.99f)
	{
		return false;
	}

	lighting.Reset();
	const DirectX::XMFLOAT3 reset = lighting.GetDirection();
	if (lighting.GetIntensity() != 0.8f ||
		!DirectX::XMVector3Equal(DirectX::XMLoadFloat3(&initial), DirectX::XMLoadFloat3(&reset)))
	{
		return false;
	}

	DirectX::XMMATRIX shear = DirectX::XMMatrixIdentity();
	shear.r[2] = DirectX::XMVectorSet(0.5f, 0.0f, 1.0f, 0.0f);
	const DirectX::XMMATRIX world = DirectX::XMMatrixScaling(1.4f, 0.7f, 0.9f) * shear *
		DirectX::XMMatrixRotationY(0.4f) * DirectX::XMMatrixTranslation(3.0f, 2.0f, 1.0f);
	const std::optional<ObjectTransformConstants> constantsResult = BuildObjectTransformConstants(
		world, DirectX::XMMatrixIdentity());
	if (!constantsResult)
	{
		return false;
	}

	const DirectX::XMVECTOR normal = DirectX::XMVector3TransformNormal(
		DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), DirectX::XMLoadFloat4x4(&constantsResult->normalTransform));
	const DirectX::XMVECTOR tangent = DirectX::XMVector3TransformNormal(
		DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), world);
	if (std::abs(DirectX::XMVectorGetX(DirectX::XMVector3Dot(normal, tangent))) > 0.0001f)
	{
		std::puts("FAIL: inverse-transpose normal orthogonality");
		return false;
	}

	const DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX invalid = identity;
	invalid.r[0] = DirectX::XMVectorSet(std::numeric_limits<float>::quiet_NaN(), 0.0f, 0.0f, 0.0f);
	if (BuildObjectTransformConstants(DirectX::XMMatrixScaling(0.0f, 1.0f, 1.0f), identity) ||
		BuildObjectTransformConstants(DirectX::XMMatrixScaling(-1.0f, 1.0f, 1.0f), identity) ||
		BuildObjectTransformConstants(invalid, identity))
	{
		std::puts("FAIL: invalid world matrices accepted");
		return false;
	}

	invalid = identity;
	invalid.r[0] = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.5f);
	if (BuildObjectTransformConstants(invalid, identity) || BuildObjectTransformConstants(identity,
		DirectX::XMMatrixScaling(std::numeric_limits<float>::infinity(), 1.0f, 1.0f)))
	{
		return false;
	}

	std::puts("Lighting CPU: time subdivision, bounds, reset, normal transform and invalid matrices passed");
	return true;
}

bool CheckPhongLighting(D3D12Renderer& renderer)
{
	if (!renderer.Resize(192, 192))
	{
		return false;
	}

	RendererReadback readback{ renderer };
	const auto ambient = CreateAmbientLighting();
	const DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicLH(4.0f, 4.0f, 0.1f, 10.0f) *
		DirectX::XMMatrixTranslation(1.0f / 192, -1.0f / 192, 0.0f);
	constexpr std::array<DirectX::XMFLOAT3, 6> eyes = {
		DirectX::XMFLOAT3{ 0.0f, 0.0f, -5.0f }, { 0.0f, 0.0f, 5.0f }, { -5.0f, 0.0f, 0.0f },
		{ 5.0f, 0.0f, 0.0f }, { 0.0f, 5.0f, 0.0f }, { 0.0f, -5.0f, 0.0f }
	};
	constexpr std::array<DirectX::XMFLOAT3, 6> upVectors = {
		DirectX::XMFLOAT3{ 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }
	};
	for (std::size_t face = 0; face < eyes.size(); ++face)
	{
		const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(
			DirectX::XMLoadFloat3(&eyes[face]),
			DirectX::XMVectorZero(),
			DirectX::XMLoadFloat3(&upVectors[face]));
		const std::array<ObjectTransformConstants, 1> objects = {
			BuildObjectTransformConstants(DirectX::XMMatrixIdentity(), view * projection).value()
		};
		std::vector<std::uint32_t> baseline;
		std::vector<std::uint32_t> lit;
		FrameLighting lighting;
		lighting.direction = { -eyes[face].x / 5.0f, -eyes[face].y / 5.0f, -eyes[face].z / 5.0f };
		lighting.color = { 0.7f, 0.5f, 0.3f };
		lighting.intensity = 0.6f;
		lighting.ambientStrength = 0.0f;
		lighting.specularStrength = 0.0f;
		if (!readback.Capture(objects, ambient, baseline))
		{
			return false;
		}

		if (!readback.Capture(objects, lighting, lit))
		{
			return false;
		}

		for (UINT row = 0; row < 3; ++row)
		{
			for (UINT column = 0; column < 3; ++column)
			{
				const auto index = (64 + row * 32) * 192 + 64 + column * 32;
				if (!CheckPixel(lit[index], baseline[index], { 0.42f, 0.3f, 0.18f }, {}, "six-face diffuse"))
				{
					return false;
				}
			}
		}
	}

	// 이동한 표면을 사용해 local 위치를 specular 계산에 잘못 전달하는 경우도 구별한다.
	const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(0.75f, 0.5f, -3.0f, 1.0f),
		DirectX::XMVectorSet(0.75f, 0.5f, 2.0f, 1.0f),
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	const std::array<ObjectTransformConstants, 1> objects = {
		BuildObjectTransformConstants(DirectX::XMMatrixTranslation(0.75f, 0.5f, 2.0f), view * projection).value()
	};
	std::vector<std::uint32_t> baseline;
	if (!readback.Capture(objects, ambient, baseline))
	{
		return false;
	}

	FrameLighting lighting;
	lighting.direction = { 0.0f, 0.0f, 1.0f };
	lighting.cameraPosition = { 0.75f, 0.5f, -3.0f };
	lighting.intensity = 0.5f;
	lighting.ambientStrength = 0.1f;
	lighting.specularStrength = 0.4f;
	constexpr std::size_t center = 96 * 192 + 96;
	auto captureCenter = [&](const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT3& bias, const char* name)
	{
		std::vector<std::uint32_t> pixels;
		if (!readback.Capture(objects, lighting, pixels))
		{
			return false;
		}

		return CheckPixel(pixels[center], baseline[center], scale, bias, name);
	};
	if (!captureCenter({ 0.6f, 0.6f, 0.6f }, { 0.2f, 0.2f, 0.2f }, "aligned Phong reflection"))
	{
		return false;
	}

	lighting.cameraPosition = { 4.75f, 0.5f, 1.0f };
	if (!captureCenter({ 0.6f, 0.6f, 0.6f }, {}, "camera-dependent reflection"))
	{
		return false;
	}

	lighting.cameraPosition = { 0.75f, 0.5f, 1.0f };
	if (!captureCenter({ 0.6f, 0.6f, 0.6f }, {}, "camera on surface"))
	{
		return false;
	}

	lighting.cameraPosition = { 2.0f, 0.5f, -3.0f };
	constexpr float exponents[] = { 4.0f, 64.0f };
	for (float exponent : exponents)
	{
		lighting.shininess = exponent;
		const float highlight = 0.2f * std::pow(4.0f / std::sqrt(16.0f + 1.25f * 1.25f), exponent);
		if (!captureCenter({ 0.6f, 0.6f, 0.6f }, { highlight, highlight, highlight }, "shininess"))
		{
			return false;
		}
	}

	lighting.direction = { 0.0f, 0.0f, -1.0f };
	lighting.cameraPosition = { 0.75f, 0.5f, 5.0f };
	if (!captureCenter({ 0.1f, 0.1f, 0.1f }, {}, "unlit back-facing surface"))
	{
		return false;
	}

	lighting.intensity = 0.0f;
	lighting.ambientStrength = 0.25f;
	if (!captureCenter({ 0.25f, 0.25f, 0.25f }, {}, "ambient with zero direct intensity"))
	{
		return false;
	}

	lighting.ambientStrength = 0.0f;
	if (!captureCenter({}, {}, "all light contributions disabled"))
	{
		return false;
	}

	// 큐브의 축 법선은 단순 S*R만으로는 잘못된 World 변환과 구별되지 않아 전단도 넣는다.
	DirectX::XMMATRIX shear = DirectX::XMMatrixIdentity();
	shear.r[2] = DirectX::XMVectorSet(0.5f, 0.0f, 1.0f, 0.0f);
	const DirectX::XMMATRIX world = DirectX::XMMatrixScaling(1.4f, 0.7f, 0.9f) * shear *
		DirectX::XMMatrixRotationY(0.4f);
	const DirectX::XMVECTOR faceCenter = DirectX::XMVector3TransformCoord(
		DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f), world);
	const DirectX::XMVECTOR normal = DirectX::XMVectorSet(-std::sin(0.4f), 0.0f, -std::cos(0.4f), 0.0f);
	const DirectX::XMMATRIX shearedView = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorAdd(faceCenter, DirectX::XMVectorScale(normal, 5.0f)),
		faceCenter,
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	const std::array<ObjectTransformConstants, 1> shearedObjects = {
		BuildObjectTransformConstants(world, shearedView * projection).value()
	};
	DirectX::XMStoreFloat3(&lighting.direction, DirectX::XMVectorNegate(normal));
	lighting.intensity = 0.5f;
	lighting.specularStrength = 0.0f;
	std::vector<std::uint32_t> pixels;
	if (!readback.Capture(shearedObjects, ambient, baseline))
	{
		return false;
	}

	if (!readback.Capture(shearedObjects, lighting, pixels))
	{
		return false;
	}

	if (!CheckPixel(pixels[center], baseline[center], { 0.5f, 0.5f, 0.5f }, {}, "scaled and sheared normal"))
	{
		return false;
	}

	std::puts(
		"Phong GPU: 6 face normals, linear colors, reflection, shininess, zero intensity and sheared scale passed");
	return true;
}
