#pragma once

#include "Assets/ImageData.h"

#include <Windows.h>

#include <filesystem>

namespace rengine
{
	struct ExecutableDirectoryResult
	{
		std::filesystem::path directory;
		HRESULT result = S_OK;
	};

	struct ImageLoadResult
	{
		ImageData image;
		HRESULT result = S_OK;
		const wchar_t* operation = L"LoadPngImage";
	};

	/// @brief 작업 디렉터리와 무관하게 실행 중인 EXE의 부모 디렉터리를 반환한다.
	/// @return 실패 시 result에 HRESULT를 반환한다.
	[[nodiscard]] ExecutableDirectoryResult GetExecutableDirectory();

	/// @brief 단일 프레임 PNG를 RGBA8로 디코딩한다. GPU나 창을 사용하지 않는다.
	/// @note 호출 스레드의 COM을 STA로 초기화하고 성공한 횟수만 정리한다. 기존 MTA에서는 실패한다.
	/// @return 실패 시 빈 image와 HRESULT·발생 API를 반환한다. 진단 표시는 호출자가 담당한다.
	[[nodiscard]] ImageLoadResult LoadPngImage(const std::filesystem::path& path);
}
