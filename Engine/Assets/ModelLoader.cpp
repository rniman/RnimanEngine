#include "Assets/ModelLoader.h"

#include "Assets/ImageData.h"
#include "Assets/MeshData.h"

#include "Math/Transform.h"
#include "Math/TransformMath.h"

#include <DirectXMath.h>
#include <ExportFormatReader.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace rengine
{
	namespace
	{
		TextureWrapMode ReadWrap(std::uint32_t mode)
		{
			switch (mode)
			{
			case 0: return TextureWrapMode::Repeat;
			case 1: return TextureWrapMode::ClampToEdge;
			case 2: return TextureWrapMode::MirroredRepeat;
			default: throw std::runtime_error("Unsupported texture wrap mode");
			}
		}

		float DecodeColor(float value)
		{
			// 원본 Color의 gamma 계약. HDR 구간은 Exporter의 엔진 연결 예제와 일치시킨다.
			if (value <= 0.04045f)
			{
				return value / 12.92f;
			}

			return value < 1.0f ? std::pow((value + 0.055f) / 1.055f, 2.4f) : std::pow(value, 2.2f);
		}

		// source가 살아 있는 동안만 유효하다. 기본 재질(-1)은 별도 surface가 없다.
		const custom_asset_export::SurfaceMaterial* FindSurface(
			const custom_asset_export::Model& source,
			const custom_asset_export::Node& node)
		{
			if (node.materialIds.empty() || node.materialIds.front() == -1)
			{
				return nullptr;
			}

			const std::int32_t materialId = node.materialIds.front();
			const auto material = std::find_if(
				source.materials.begin(),
				source.materials.end(),
				[materialId](const custom_asset_export::Material& value) { return value.id == materialId; });
			if (material == source.materials.end())
			{
				throw std::runtime_error("Missing material reference");
			}

			const custom_asset_export::SurfaceMaterial& surface = material->surface;
			// 공급 규격의 0 = Metallic PBR/Lit·Opaque, 2 = Back culling이다.
			if (surface.shadingModel != 0 || surface.surface != 0 || surface.alphaClip != 0 || surface.cull != 2)
			{
				throw std::runtime_error("Phong preview requires opaque lit material, no alpha clip and back-face culling");
			}

			return &surface;
		}
	}

	ModelLoadResult LoadStaticModel(const std::filesystem::path& path)
	{
		try
		{
			const custom_asset_export::Model source = custom_asset_export::ReadModel(path);
			if (source.coordinateSystem != custom_asset_export::CoordinateSystem::DirectXLeftHandedYUp ||
				source.meshes.size() != 1)
			{
				throw std::runtime_error("Expected DirectXLeftHandedYUp and exactly one mesh");
			}

			const auto& mesh = source.meshes.front();
			if (mesh.positions.empty() || mesh.normals.size() != mesh.positions.size() ||
				mesh.uv0.size() != mesh.positions.size() || mesh.submeshes.size() != 1 ||
				mesh.submeshes.front().indices.empty())
			{
				throw std::runtime_error("Expected nonempty triangles, normals, UV0 and one submesh");
			}

			StaticModel model;
			std::array<float, 2> uvScale{ 1, 1 };
			std::array<float, 2> uvOffset{};
			std::string warning;
			std::unordered_map<std::int32_t, DirectX::XMFLOAT4X4> worldById;
			const custom_asset_export::Node* meshNode = nullptr;
			// Reader가 부모 선행 순서·ID 유일성·참조를 검증한다. 여기서는 World 합성만 맡는다.
			for (const auto& node : source.nodes)
			{
				DirectX::XMFLOAT4X4 stored{};
				std::memcpy(&stored, node.localMatrix.data(), sizeof(stored));
				// 열 벡터 local을 행 벡터로 바꾸므로 World = Local * Parent가 된다.
				DirectX::XMMATRIX world = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&stored));
				if (node.parentId != -1)
				{
					world *= DirectX::XMLoadFloat4x4(&worldById.at(node.parentId));
				}

				DirectX::XMFLOAT4X4 storedWorld{};
				DirectX::XMStoreFloat4x4(&storedWorld, world);
				worldById.emplace(node.id, storedWorld);
				if (node.meshId == -1)
				{
					continue;
				}

				if (meshNode != nullptr)
				{
					throw std::runtime_error("Expected exactly one mesh node; scene import is not supported");
				}

				meshNode = &node;
				const TransformDecompositionResult decomposed = DecomposeTransform(world);
				if (!decomposed.transform)
				{
					throw std::runtime_error(decomposed.error);
				}
				model.transform = *decomposed.transform;
			}

			if (meshNode == nullptr)
			{
				throw std::runtime_error("Expected exactly one mesh node; scene import is not supported");
			}

			if (const custom_asset_export::SurfaceMaterial* surface = FindSurface(source, *meshNode))
			{
				model.baseColor = surface->baseColor;
				for (std::size_t channel = 0; channel < 3; ++channel)
				{
					// colorSpace 1은 gamma 입력이다. alpha에는 RGB 변환을 적용하지 않는다.
					if (surface->colorSpace == 1)
					{
						model.baseColor[channel] = DecodeColor(model.baseColor[channel]);
					}

					if (!std::isfinite(model.baseColor[channel]))
					{
						throw std::runtime_error("Material color conversion overflow");
					}
				}

				uvScale = surface->uvScale;
				uvOffset = surface->uvOffset;
				for (const auto& texture : surface->textures)
				{
					// 공급 규격의 BaseColor 슬롯이다.
					if (texture.usage == 0)
					{
						model.baseColorTextureRef = texture.textureRef;
						model.wrapU = ReadWrap(texture.wrapU);
						model.wrapV = ReadWrap(texture.wrapV);
						break;
					}
				}

				warning = "Phong preview: BaseColor only; PBR values and other maps are not applied.";
			}

			model.mesh.vertices.reserve(mesh.positions.size());
			for (std::size_t index = 0; index < mesh.positions.size(); ++index)
			{
				const auto& position = mesh.positions[index];
				const auto& normal = mesh.normals[index];
				const double lengthSquared = static_cast<double>(normal[0]) * normal[0] +
					static_cast<double>(normal[1]) * normal[1] + static_cast<double>(normal[2]) * normal[2];
				if (std::abs(lengthSquared - 1.0) > 1e-3)
				{
					throw std::runtime_error("Model normal must be a nonzero unit vector");
				}

				// 원본 UV에 재질 변환을 먼저 적용하고 위에서 아래로 저장한 이미지 행 원점에 맞춰 V를 한 번 뒤집는다.
				const float u = mesh.uv0[index][0] * uvScale[0] + uvOffset[0];
				const float v = 1.0f - (mesh.uv0[index][1] * uvScale[1] + uvOffset[1]);
				if (!std::isfinite(u) || !std::isfinite(v))
				{
					throw std::runtime_error("Material UV conversion overflow");
				}
				model.mesh.vertices.push_back({
					{ position[0], position[1], position[2] },
					{ u, v },
					{ normal[0], normal[1], normal[2] }
				});
			}

			model.mesh.indices = mesh.submeshes.front().indices;
			return { std::move(model), {}, std::move(warning) };
		}
		catch (const std::exception& error)
		{
			return { std::nullopt, error.what(), {} };
		}
	}
}
