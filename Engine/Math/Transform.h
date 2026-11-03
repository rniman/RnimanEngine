#pragma once

#include <DirectXMath.h>

namespace rengine
{
	// 부모 계층이 없는 로컬 Transform. 회전은 정규화된 Quaternion (x,y,z,w)이며 단위 회전은 (0,0,0,1)이다.
	struct Transform
	{
		DirectX::XMFLOAT3 position{ 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT4 rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		DirectX::XMFLOAT3 scale{ 1.0f, 1.0f, 1.0f };

		DirectX::XMMATRIX GetWorld() const
		{
			// 행 벡터 규약: 로컬 크기 → 회전 → 월드 위치 순서로 적용한다.
			return DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) *
				DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation)) *
				DirectX::XMMatrixTranslation(position.x, position.y, position.z);
		}
	};
}
