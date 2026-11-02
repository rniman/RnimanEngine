#pragma once

#include "ExportData.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace custom_asset_export
{
	enum class ErrorCode
	{
		Io,
		Truncated,
		InvalidFormat,
		UnsupportedVersion,
		LimitExceeded
	};

	class ReadError : public std::runtime_error
	{
	public:
		ReadError(ErrorCode code, std::size_t offset, const std::string& message);

		ErrorCode GetCode() const noexcept
		{
			return mCode;
		}

		/// @return 오류를 검출한 바이트 위치. 잘못된 필드의 시작 위치와 다를 수 있으며 파일 열기 오류는 0이다.
		std::size_t GetOffset() const noexcept
		{
			return mOffset;
		}

	private:
		ErrorCode mCode;
		std::size_t mOffset;
	};

	struct ReadLimits
	{
		std::size_t maxFileBytes = 256u * 1024u * 1024u;
		std::size_t maxStringBytes = 1024u * 1024u;
		std::size_t maxNodes = 100000;
		std::size_t maxMeshes = 100000;
		std::size_t maxMaterials = 100000;
		std::size_t maxArrayElements = 16000000;
		/// 컨테이너·문자열·조회 항목에 대한 누적 할당 예산이다.
		/// Allocator 부가 비용과 Capacity 반올림은 포함하지 않는다.
		std::size_t maxDecodedBytes = 512u * 1024u * 1024u;
	};

	/// @brief 현재 Binary v2 파일을 검증하고 모든 데이터를 소유한 모델을 반환한다.
	/// @note limits는 호출 동안만 읽는다. 실패하면 부분 모델 없이 ReadError를 던진다.
	/// @note 자원 부족 시 std::bad_alloc 또는 std::length_error가 발생할 수 있다.
	[[nodiscard]] Model ReadModel(const std::filesystem::path& path, const ReadLimits& limits = {});

	/// @brief 메모리의 현재 Binary v2 데이터를 검증하고 소유 모델로 반환한다.
	/// @param data 호출 동안 읽을 수 있는 size 바이트. nullptr는 size == 0일 때만 허용한다.
	/// @note 입력 버퍼와 limits는 저장하지 않는다. 빈 입력은 잘림 오류다.
	/// @note 반환 데이터의 소유권과 예외 계약은 파일 경로 오버로드와 같다.
	[[nodiscard]] Model ReadModel(const std::uint8_t* data, std::size_t size, const ReadLimits& limits = {});
} // namespace custom_asset_export
