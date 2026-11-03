#pragma once

#include <Windows.h>

namespace rengine
{
	/// @brief 저장소의 Binary v2 고정 입력으로 CPU 변환·손상 회귀와 이전 포맷 거부를 검사한다.
	/// @return 입력 누락 또는 검사 실패 시 false이며 호출자는 검증을 중단한다.
	[[nodiscard]] bool CheckModelRegression();

	/// @brief 유효한 window를 빌려 실제 모델의 로딩·GPU Phong·UV·색상을 검증한다.
	/// @return 실패 시 false이며 호출자는 검증을 중단한다. window 소유권은 이전하지 않는다.
	[[nodiscard]] bool CheckImportedModel(HWND window);
}
