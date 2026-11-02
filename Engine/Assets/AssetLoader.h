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

	/// @brief 단일 프레임 PNG 또는 JPEG를 행 패딩 없는 RGBA8로 디코딩한다. JPEG alpha는 255다.
	/// @note 확장자가 아닌 WIC 컨테이너로 형식을 판별한다. COM STA·소유권·실패 계약은 LoadPngImage와 같다.
	/// @note ICC·EXIF 변환 없이 읽으며 GPU 업로드와 sRGB 해석은 호출자가 담당한다.
	[[nodiscard]] ImageLoadResult LoadImage(const std::filesystem::path& path);
}
