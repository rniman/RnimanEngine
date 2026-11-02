#pragma once

#include <Windows.h>

namespace rengine
{
	/// @brief 유효한 window를 빌려 Binary v2 고정 입력의 CPU 변환·손상 검사와 실제 모델의 GPU Phong·UV·색상을 검증한다.
	/// @return 실패 시 false이며 호출자는 검증을 중단한다. window 소유권은 이전하지 않는다.
	[[nodiscard]] bool CheckImportedModel(HWND window);
}
