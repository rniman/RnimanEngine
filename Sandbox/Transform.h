#pragma once

#include <DirectXMath.h>

// 부모 계층이 없는 로컬 Transform. 회전은 pitch(X), yaw(Y), roll(Z), 단위는 radian이다.
struct Transform
{
	DirectX::XMFLOAT3 position{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 rotation{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 scale{ 1.0f, 1.0f, 1.0f };

	DirectX::XMMATRIX GetWorld() const
	{
		// 행 벡터 규약: 로컬 크기 → 회전 → 월드 위치 순서로 적용한다.
		return DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) *
			DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) *
			DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	}
};
