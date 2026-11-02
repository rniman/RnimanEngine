#pragma once

#include "Assets/ImageData.h"
#include "Assets/MeshData.h"

#include "Math/Transform.h"

#include <array>
#include <filesystem>
#include <optional>
#include <string>

namespace rengine
{
	struct StaticModel
	{
		MeshData mesh;
		// 선형 RGB와 원본 alpha. 현재 파이프라인은 불투명으로 출력한다.
		std::array<float, 4> baseColor{ 1, 1, 1, 1 };
		// 확장자 없는 UTF-8 파일명. 폴더·확장자 선택과 이미지 로딩은 호출자가 담당한다.
		std::string baseColorTextureRef;
		// 원본 BaseColor 바인딩의 축별 Wrap. 이미지 경로/확장자 선택과는 독립이다.
		TextureWrapMode wrapU = TextureWrapMode::Repeat;
		TextureWrapMode wrapV = TextureWrapMode::Repeat;
		Transform transform;
	};

	struct ModelLoadResult
	{
		// 성공 시에만 model이 존재한다. warning은 성공한 Phong 근사의 미적용 항목을 알린다.
		std::optional<StaticModel> model;
		std::string error;
		std::string warning;
	};

	/// @brief 현재 Binary v2의 단일 메시·단일 메시 노드를 CPU 데이터로 읽는다. GPU·창은 사용하지 않는다.
	/// @note LH Y-up, 법선·UV0, 단일 Submesh, 양수 Scale의 TRS만 지원한다. 계층은 World로 합친다.
	/// @note 재질은 불투명·뒤쪽 컬링 표면의 BaseColor를 Phong으로 근사한다. 미적용 항목은 warning에 담는다.
	/// @return 성공 시 모든 데이터를 소유한 model, 실패 시 빈 model과 원인. 경로 표시는 호출자가 담당한다.
	[[nodiscard]] ModelLoadResult LoadStaticModel(const std::filesystem::path& path);
}
