#pragma once

namespace rengine
{
	class D3D12Renderer;

	/// @brief 조명 조작·렌더 행렬의 CPU 계약을 확인한다. 실패 시 false다.
	[[nodiscard]] bool CheckLightingData();

	/// @brief 초기화된 renderer에서 조명 픽셀을 읽는다. 실패 시 false이며 검증을 중단한다.
	[[nodiscard]] bool CheckPhongLighting(D3D12Renderer& renderer);
}
