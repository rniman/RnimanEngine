#include "ExportFormatReader.h"

#include "ExportData.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace custom_asset_export
{
	ReadError::ReadError(ErrorCode code, std::size_t offset, const std::string& message)
		: std::runtime_error(message + " (byte " + std::to_string(offset) + ")"), mCode{ code }, mOffset{ offset }
	{
	}

	namespace
	{
		static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559, "IEEE 754 float32 required");

		class Reader
		{
		public:
			Reader(const std::uint8_t* data, std::size_t size, const ReadLimits& limits)
				: mData{ data }, mSize{ size }, mLimits{ limits }
			{
				if (size > limits.maxFileBytes)
				{
					ThrowError(ErrorCode::LimitExceeded, "File size limit");
				}

				if (!data && size)
				{
					ThrowError(ErrorCode::InvalidFormat, "Null input");
				}
			}

			Model Read()
			{
				RequireBytes(19);
				Validate(std::memcmp(mData, "UNITY_CUSTOM_EXPORT", 19) == 0, "Invalid magic");
				mPosition += 19;
				Model model;
				model.version = ReadUInt32();
				if (model.version != 2)
				{
					ThrowError(ErrorCode::UnsupportedVersion, "Unsupported format version");
				}

				const std::uint32_t coordinates = ReadUInt32();
				Validate(coordinates <= 2, "Unknown coordinate system");
				model.coordinateSystem = static_cast<CoordinateSystem>(coordinates);

				const std::size_t nodes = ReadCount(84, mLimits.maxNodes, sizeof(Node));
				Validate(nodes != 0, "Root node required");
				// ID 조회 Node와 Bucket의 비용을 보수적으로 예약한다.
				ReserveBudget(nodes, 64);
				std::unordered_set<std::int32_t> nodeIds;
				nodeIds.reserve(nodes);
				model.nodes.reserve(nodes);
				for (std::size_t i = 0; i < nodes; ++i)
				{
					Node node;
					node.id = ReadInt32();
					node.parentId = ReadInt32();
					node.name = ReadString();
					node.meshId = ReadInt32();
					node.localMatrix = ReadVector<16>();

					const std::size_t slots = ReadCount(4, mLimits.maxArrayElements, sizeof(std::int32_t));
					node.materialIds.resize(slots);
					for (auto& id : node.materialIds)
					{
						id = ReadInt32();
						Validate(id >= -1, "Invalid material reference sentinel");
					}

					Validate(node.id >= 0 && nodeIds.count(node.id) == 0, "Invalid or duplicate node ID");
					Validate(
						i == 0 ? node.id == 0 && node.parentId == -1 : nodeIds.count(node.parentId) != 0,
						"Root or parent-before-child rule violated");
					Validate(node.meshId >= -1, "Invalid mesh reference sentinel");
					if (i == 0)
					{
						for (std::size_t j = 0; j < 16; ++j)
						{
							Validate(
								std::fabs(node.localMatrix[j] - (j % 5 == 0 ? 1.f : 0.f)) <= 1e-5f,
								"Root matrix must be identity");
						}
					}

					nodeIds.insert(node.id);
					model.nodes.push_back(std::move(node));
				}

				const std::size_t meshes = ReadCount(48, mLimits.maxMeshes, sizeof(Mesh));
				ReserveBudget(meshes, 64);
				std::unordered_map<std::int32_t, std::size_t> meshIds;
				meshIds.reserve(meshes);
				model.meshes.reserve(meshes);
				for (std::size_t i = 0; i < meshes; ++i)
				{
					Mesh mesh;
					mesh.id = ReadInt32();
					Validate(mesh.id >= 0, "Invalid or duplicate mesh ID");
					const bool inserted = meshIds.emplace(mesh.id, model.meshes.size()).second;
					Validate(inserted, "Invalid or duplicate mesh ID");
					mesh.name = ReadString();
					mesh.bounds.center = ReadVector<3>();
					mesh.bounds.extents = ReadVector<3>();
					for (auto value : mesh.bounds.extents)
					{
						Validate(value >= 0, "Negative bounds extent");
					}

					mesh.positions = ReadVectorArray<3>();
					mesh.normals = ReadVectorArray<3>();
					mesh.uv0 = ReadVectorArray<2>();

					mesh.tangents = ReadVectorArray<4>();

					Validate(
						mesh.tangents.empty() || mesh.tangents.size() == mesh.positions.size(),
						"Tangent count mismatch");
					for (const auto& t : mesh.tangents)
					{
						Validate(
							(t[3] == -1 || t[3] == 1) && std::fabs(t[0] * t[0] + t[1] * t[1] + t[2] * t[2] - 1) <= .01f,
							"Invalid tangent");
					}
					Validate(
						mesh.normals.empty() || mesh.normals.size() == mesh.positions.size(),
						"Normal count mismatch");
					Validate(mesh.uv0.empty() || mesh.uv0.size() == mesh.positions.size(), "UV0 count mismatch");
					const std::size_t submeshes = ReadCount(4, mLimits.maxArrayElements, sizeof(Submesh));
					mesh.submeshes.reserve(submeshes);
					for (std::size_t j = 0; j < submeshes; ++j)
					{
						Submesh submesh;
						submesh.index = static_cast<std::uint32_t>(j);
						const std::size_t indices = ReadCount(4, mLimits.maxArrayElements, sizeof(std::uint32_t));
						Validate(indices % 3 == 0, "Index count must be divisible by three");
						submesh.indices.resize(indices);
						for (auto& index : submesh.indices)
						{
							index = ReadUInt32();
							Validate(index < mesh.positions.size(), "Vertex index out of range");
						}

						mesh.submeshes.push_back(std::move(submesh));
					}

					model.meshes.push_back(std::move(mesh));
				}

				std::unordered_map<std::int32_t, std::size_t> materialIndices;

				const std::size_t materials = ReadCount(108, mLimits.maxMaterials, sizeof(Material));
				ReserveBudget(materials, 64);
				materialIndices.reserve(materials);
				model.materials.reserve(materials);
				for (std::size_t i = 0; i < materials; ++i)
				{
					Material material;
					material.id = ReadInt32();
					Validate(material.id >= 0, "Invalid or duplicate material ID");
					const bool inserted = materialIndices.emplace(material.id, model.materials.size()).second;
					Validate(inserted, "Invalid or duplicate material ID");
					material.name = ReadString();
					material.surface = ReadSurfaceMaterial();
					model.materials.push_back(std::move(material));
				}

				for (const auto& node : model.nodes)
				{
					Validate(node.meshId == -1 || meshIds.count(node.meshId) != 0, "Missing mesh reference");

					Validate(
						node.materialIds.size() ==
							(node.meshId == -1 ? 0 : model.meshes[meshIds.at(node.meshId)].submeshes.size()),
						"Material slot count mismatch");
					for (auto id : node.materialIds)
					{
						if (id == -1)
						{
							continue;
						}
						const auto materialIndex = materialIndices.find(id);
						Validate(materialIndex != materialIndices.end(), "Missing material reference");
						const auto& material = model.materials[materialIndex->second];
						const auto& mesh = model.meshes[meshIds.at(node.meshId)];
						if (!material.surface.textures.empty())
						{
							Validate(!mesh.uv0.empty(), "Textured material requires UV0");
						}
						const bool needsBasis = std::any_of(
							material.surface.textures.begin(),
							material.surface.textures.end(),
							[](const SurfaceTexture& t)
							{
								return t.usage == 1 || t.usage == 6;
							});
						if (needsBasis)
						{
							Validate(
								!mesh.tangents.empty() && mesh.normals.size() == mesh.positions.size(),
								"Normal/height requires tangent basis");
							for (std::size_t i = 0; i < mesh.tangents.size(); ++i)
							{
								const auto& n = mesh.normals[i];
								const auto& t = mesh.tangents[i];
								Validate(
									std::fabs(n[0] * n[0] + n[1] * n[1] + n[2] * n[2] - 1) <= .01f &&
										std::fabs(n[0] * t[0] + n[1] * t[1] + n[2] * t[2]) <= .01f,
									"Invalid tangent basis");
							}
						}
					}
				}

				Validate(mPosition == mSize, "Trailing bytes after model");
				return model;
			}

		private:
			SurfaceMaterial ReadSurfaceMaterial()
			{
				SurfaceMaterial s;
				s.shadingModel = ReadUInt32();
				s.colorSpace = ReadUInt32();
				s.baseColor = ReadVector<4>();
				s.metallic = ReadFloat();
				s.gloss = ReadFloat();
				s.glossMode = ReadUInt32();
				s.normalScale = ReadFloat();
				s.occlusionStrength = ReadFloat();
				s.heightScale = ReadFloat();
				s.emission = ReadVector<3>();
				s.surface = ReadUInt32();
				s.alphaClip = ReadUInt32();
				s.cull = ReadUInt32();
				s.alphaSource = ReadUInt32();
				s.alphaCutoff = ReadFloat();
				Validate(
					s.shadingModel <= 1 && s.colorSpace <= 1 && s.glossMode <= 1 && s.surface <= 1 &&
						s.alphaClip <= 1 && s.cull <= 2 && s.alphaSource <= 1,
					"Invalid surface enum");
				Validate(
					s.metallic >= 0 && s.metallic <= 1 && s.gloss >= 0 && s.gloss <= 1 && s.occlusionStrength >= 0 &&
						s.occlusionStrength <= 1 && s.alphaCutoff >= 0 && s.alphaCutoff <= 1,
					"Invalid surface factor");

				s.uvScale = ReadVector<2>();
				s.uvOffset = ReadVector<2>();

				const auto count =
					ReadCount(20, (std::min)(std::size_t(7), mLimits.maxArrayElements), sizeof(SurfaceTexture));
				s.textures.reserve(count);
				std::array<bool, 7> used{};
				for (std::size_t i = 0; i < count; ++i)
				{
					SurfaceTexture t;
					t.usage = ReadUInt32();
					Validate(t.usage <= 6 && !used[t.usage], "Invalid/duplicate texture usage");
					used[t.usage] = true;
					t.textureRef = ReadString();
					Validate(!t.textureRef.empty(), "Empty source texture reference");

					const auto& name = t.textureRef;
					Validate(
						name != "." && name != ".." && name.find_first_not_of(" ") != std::string::npos,
						"Invalid texture name");
					for (unsigned char c : name)
					{
						Validate(c >= 32 && c != 127 && c != '/' && c != '\\' && c != ':', "Invalid texture name");
					}

					t.channel = ReadUInt32();
					const bool rgb = t.usage == 0 || t.usage == 1 || t.usage == 5;
					Validate(rgb ? t.channel == 4 : t.channel < 4, "Invalid texture channel");
					t.wrapU = ReadUInt32();
					t.wrapV = ReadUInt32();
					Validate(t.wrapU <= 2 && t.wrapV <= 2, "Invalid texture wrap mode");

					s.textures.push_back(std::move(t));
				}
				return s;
			}

			[[noreturn]] void ThrowError(ErrorCode code, const std::string& message) const
			{
				throw ReadError(code, mPosition, message);
			}

			void Validate(bool valid, const char* message) const
			{
				if (!valid)
				{
					ThrowError(ErrorCode::InvalidFormat, message);
				}
			}

			void RequireBytes(std::size_t bytes) const
			{
				if (bytes > mSize - mPosition)
				{
					ThrowError(ErrorCode::Truncated, "Unexpected end of input");
				}
			}

			void ReserveBudget(std::size_t count, std::size_t width)
			{
				if (count > (mLimits.maxDecodedBytes - mAllocatedBytes) / width)
				{
					ThrowError(ErrorCode::LimitExceeded, "Decoded allocation budget");
				}

				mAllocatedBytes += count * width;
			}

			std::uint32_t ReadUInt32()
			{
				RequireBytes(4);
				std::uint32_t value = 0;
				for (unsigned i = 0; i < 4; ++i)
				{
					value |= std::uint32_t(mData[mPosition++]) << (8 * i);
				}

				return value;
			}

			std::int32_t ReadInt32()
			{
				const std::uint32_t value = ReadUInt32();
				return value <= 0x7fffffffu
					? static_cast<std::int32_t>(value)
					: static_cast<std::int32_t>(static_cast<std::int64_t>(value) - 0x100000000LL);
			}

			float ReadFloat()
			{
				const std::uint32_t bits = ReadUInt32();
				float value = 0.0f;
				std::memcpy(&value, &bits, 4);
				Validate(std::isfinite(value), "Non-finite float");
				return value;
			}

			std::size_t ReadCount(std::size_t minimumBytes, std::size_t limit, std::size_t elementBytes)
			{
				const std::uint32_t elementCount = ReadUInt32();
				if (elementCount > limit)
				{
					ThrowError(ErrorCode::LimitExceeded, "Element count limit");
				}

				if (elementCount > (mSize - mPosition) / minimumBytes)
				{
					ThrowError(ErrorCode::Truncated, "Count exceeds remaining input");
				}

				ReserveBudget(elementCount, elementBytes);
				return elementCount;
			}

			std::string ReadString()
			{
				const std::size_t elementCount = ReadCount(1, mLimits.maxStringBytes, 1);
				std::string result(reinterpret_cast<const char*>(mData + mPosition), elementCount);
				mPosition += elementCount;
				// Overlong 인코딩, Surrogate, 잘못된 연속 Byte와 U+10FFFF 초과를 거부한다.
				for (std::size_t i = 0; i < elementCount;)
				{
					unsigned char byte = static_cast<unsigned char>(result[i++]);
					if (byte < 0x80)
					{
						continue;
					}

					unsigned remainingBytes = 0;
					unsigned codePoint = 0;
					unsigned minimumCodePoint = 0;
					if (byte >= 0xc2 && byte <= 0xdf)
					{
						remainingBytes = 1;
						codePoint = byte & 31;
						minimumCodePoint = 0x80;
					}
					else if (byte >= 0xe0 && byte <= 0xef)
					{
						remainingBytes = 2;
						codePoint = byte & 15;
						minimumCodePoint = 0x800;
					}
					else if (byte >= 0xf0 && byte <= 0xf4)
					{
						remainingBytes = 3;
						codePoint = byte & 7;
						minimumCodePoint = 0x10000;
					}
					else
					{
						ThrowError(ErrorCode::InvalidFormat, "Invalid UTF-8 lead byte");
					}

					Validate(remainingBytes <= elementCount - i, "Truncated UTF-8");
					while (remainingBytes--)
					{
						byte = static_cast<unsigned char>(result[i++]);
						Validate((byte & 0xc0) == 0x80, "Invalid UTF-8 continuation");
						codePoint = (codePoint << 6) | (byte & 63);
					}

					Validate(
						codePoint >= minimumCodePoint && codePoint <= 0x10ffff &&
							!(codePoint >= 0xd800 && codePoint <= 0xdfff),
						"Invalid UTF-8 code point");
				}

				return result;
			}

			template <std::size_t N> std::array<float, N> ReadVector()
			{
				std::array<float, N> result{};
				for (auto& value : result)
				{
					value = ReadFloat();
				}

				return result;
			}

			template <std::size_t N> std::vector<std::array<float, N>> ReadVectorArray()
			{
				const std::size_t elementCount =
					ReadCount(N * 4, mLimits.maxArrayElements, sizeof(std::array<float, N>));
				std::vector<std::array<float, N>> result(elementCount);
				for (auto& value : result)
				{
					value = ReadVector<N>();
				}

				return result;
			}

			// 입력 버퍼와 limits는 ReadModel 호출 동안만 대여한다.
			const std::uint8_t* mData;
			std::size_t mSize;
			std::size_t mPosition = 0;
			std::size_t mAllocatedBytes = 0;
			const ReadLimits& mLimits;
		};
	} // namespace

	Model ReadModel(const std::uint8_t* data, std::size_t size, const ReadLimits& limits)
	{
		return Reader(data, size, limits).Read();
	}

	Model ReadModel(const std::filesystem::path& path, const ReadLimits& limits)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file)
		{
			throw ReadError(ErrorCode::Io, 0, "Cannot open model file");
		}

		const std::ifstream::pos_type end = file.tellg();
		if (end < 0)
		{
			throw ReadError(ErrorCode::Io, 0, "Cannot determine file length");
		}

		const std::uintmax_t length = static_cast<std::uintmax_t>(end);
		if (length > limits.maxFileBytes ||
			length > static_cast<std::uintmax_t>((std::numeric_limits<std::streamsize>::max)()))
		{
			throw ReadError(ErrorCode::LimitExceeded, 0, "File size limit");
		}

		std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));
		file.seekg(0);
		if (!bytes.empty() &&
			!file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())))
		{
			throw ReadError(ErrorCode::Io, 0, "Cannot read complete file");
		}

		if (file.peek() != std::char_traits<char>::eof())
		{
			throw ReadError(ErrorCode::Io, bytes.size(), "File grew while reading");
		}

		return ReadModel(bytes.data(), bytes.size(), limits);
	}
} // namespace custom_asset_export
