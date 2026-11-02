#pragma once

#include <cstdint>
#include <vector>

namespace rengine
{
	/// @brief 이미지와 별개로 텍스처 바인딩마다 지정하는 2D 주소 모드. 현재 파일 v2와 값이 같다.
	enum class TextureWrapMode : std::uint32_t
	{
		Repeat = 0,
		ClampToEdge = 1,
		MirroredRepeat = 2
	};

	/// @brief 위에서 아래로, 행 패딩 없이 저장한 RGBA8 픽셀. GPU 자원을 소유하지 않는다.
	struct ImageData
	{
		std::uint32_t width = 0;
		std::uint32_t height = 0;
		std::vector<std::uint8_t> pixels;
	};
}
