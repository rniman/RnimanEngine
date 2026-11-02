#pragma once

#include <Windows.h>

namespace rengine
{
	/// @brief 추적한 PNG·JPEG 기준 파일의 디코딩과 형식 제한을 검사한다. 실패 시 검증을 중단한다.
	[[nodiscard]] bool CheckJpegLoading();

	/// @brief 유효한 window를 빌려 세 실제 모델의 동시 그리기·자원 선택·수명을 GPU 픽셀로 검사한다.
	[[nodiscard]] bool CheckMultipleModels(HWND window);

	/// @brief 범위 밖 UV의 Repeat·Clamp를 비대칭 이미지로 구분하고 모델 전환 후에도 검사한다.
	[[nodiscard]] bool CheckTextureAddressing(HWND window);
}
