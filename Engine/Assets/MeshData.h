#pragma once

#include <cstdint>
#include <vector>

namespace rengine
{
	struct MeshVertex
	{
		float position[3];
		float uv[2];
		float normal[3];
	};

	// CPU 소유 삼각형 메시. UV 원점은 이미지의 왼쪽 위이며 인덱스는 시계 방향 앞면이다.
	// 렌더러에 전달할 때 위치·UV는 유한하고 법선은 단위 벡터여야 한다.
	struct MeshData
	{
		std::vector<MeshVertex> vertices;
		std::vector<std::uint32_t> indices;
	};
}
