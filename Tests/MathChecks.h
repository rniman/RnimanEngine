#pragma once

namespace rengine
{
	/// @brief 헤더 전용 Math의 CPU 계약을 검사한다. GPU·창은 사용하지 않는다.
	[[nodiscard]] bool CheckMathUtilities();
}
