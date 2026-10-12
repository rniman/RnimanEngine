#pragma once

#include <cstdint>
#include <vector>

/// @brief 위에서 아래로, 행 패딩 없이 저장한 RGBA8 픽셀. GPU 자원을 소유하지 않는다.
struct ImageData
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::vector<std::uint8_t> pixels;
};
