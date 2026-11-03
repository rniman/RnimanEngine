#include "ModelChecks.h"

#include "RendererReadback.h"

#include "Assets/AssetLoader.h"
#include "Assets/ImageData.h"
#include "Assets/MeshData.h"
#include "Assets/ModelLoader.h"
#include "Math/QuaternionMath.h"
#include "Math/Transform.h"
#include "Rendering/D3D12Renderer.h"
#include "Rendering/RenderData.h"

// Windows 선언은 DirectX 헤더보다 먼저 필요하다.
#include <Windows.h>
#include <DirectXMath.h>
#include <ExportFormatReader.h>

#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <span>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace rengine
{
	namespace
	{
		// Tests/Assets/ModelFixture/CubeV2.bin 전용 배치. 실제 데모 BIN에는 이 offset을 적용하지 않는다.
		constexpr std::size_t MODEL_FILE_SIZE = 1778;

		// 이 검증 파일 안에서만 사용하는 임시 입력의 단독 소유자다.
		class TemporaryModelFile
		{
		public:
			TemporaryModelFile() = default;
			~TemporaryModelFile()
			{
				if (!mPath.empty())
				{
					std::error_code error;
					std::filesystem::remove(mPath, error);
					if (error)
					{
						std::fprintf(stderr, "FAIL: temporary model cleanup: %s\n", error.message().c_str());
					}
				}
			}

			TemporaryModelFile(const TemporaryModelFile&) = delete;
			TemporaryModelFile& operator=(const TemporaryModelFile&) = delete;

			// 한 번 생성한 파일을 사례마다 덮어쓴다. 생성 실패 시 검증을 중단한다.
			[[nodiscard]] bool Initialize()
			{
				wchar_t directory[MAX_PATH]{};
				const DWORD length = GetTempPathW(MAX_PATH, directory);
				if (length == 0 || length >= MAX_PATH)
				{
					std::printf("FAIL: GetTempPathW: length=%lu error=%lu\n", length, GetLastError());
					return false;
				}

				wchar_t filename[MAX_PATH]{};
				if (GetTempFileNameW(directory, L"mdl", 0, filename) == 0)
				{
					std::printf("FAIL: GetTempFileNameW: error=%lu\n", GetLastError());
					return false;
				}

				mPath = filename;
				return true;
			}

			[[nodiscard]] bool Write(std::span<const std::uint8_t> bytes) const
			{
				std::ofstream output(mPath, std::ios::binary | std::ios::trunc);
				output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
				output.close();
				if (!output)
				{
					std::puts("FAIL: temporary model write or close");
					return false;
				}

				return true;
			}

			const std::filesystem::path& GetPath() const
			{
				return mPath;
			}

		private:
			std::filesystem::path mPath;
		};

		bool CheckModelTransforms(const std::vector<std::uint8_t>& original)
		{
			// 이 fixture의 두 번째 Node localMatrix 시작 위치. 형식 변경 시 함께 갱신한다.
			constexpr std::size_t matrixOffset = 144;
			if (original.size() != MODEL_FILE_SIZE)
			{
				return false;
			}

			TemporaryModelFile temporary;
			if (!temporary.Initialize())
			{
				return false;
			}

			for (int test = 0; test < 11; ++test)
			{
				Transform transform;
				transform.position = { -2, 3, 4 };
				transform.scale = { 2, 0.7f, 1.3f };
				const float pitch = test >= 5 ?
					(test < 8 ? 1.0f : -1.0f) * (DirectX::XM_PIDIV2 + (test % 3 - 1) * 0.001f) :
					(test == 1 ? DirectX::XM_PIDIV2 : 0.3f);
				transform.rotation = *QuaternionFromEuler({ pitch, -0.6f, 0.4f });
				DirectX::XMMATRIX world = transform.GetWorld();
				if (test == 2)
				{
					world = DirectX::XMMatrixScaling(-1, 1, 1) * world;
				}
				if (test == 3)
				{
					DirectX::XMMATRIX shear = DirectX::XMMatrixIdentity();
					shear.r[0] = DirectX::XMVectorSet(1, 0.5f, 0, 0);
					world = shear * world;
				}

				DirectX::XMFLOAT4X4 stored{};
				DirectX::XMStoreFloat4x4(&stored, DirectX::XMMatrixTranspose(world));
				std::vector<std::uint8_t> bytes = original;
				// Identity 부모를 앞에 삽입하여 루트 제약을 유지한 채 자식 TRS를 검사한다.
				std::array<std::uint8_t, 84> parent{};
				const std::int32_t absent = -1;
				std::memcpy(parent.data() + 4, &absent, 4);
				std::memcpy(parent.data() + 12, &absent, 4);
				const float one = 1;
				for (std::size_t diagonal = 0; diagonal < 4; ++diagonal)
				{
					std::memcpy(parent.data() + 16 + diagonal * 20, &one, 4);
				}
				bytes.insert(bytes.begin() + 31, parent.begin(), parent.end());
				bytes[27] = 2;
				bytes[115] = 1;
				std::fill(bytes.begin() + 119, bytes.begin() + 123, std::uint8_t{ 0 });

				std::memcpy(bytes.data() + matrixOffset, &stored, sizeof(stored));
				if (test == 4)
				{
					// 첫 법선을 0으로 만들어 파일 파서 통과 후 엔진의 Phong 계약에서 거부한다.
					std::fill(bytes.begin() + 552, bytes.begin() + 564, std::uint8_t{ 0 });
				}

				if (!temporary.Write(bytes))
				{
					return false;
				}

				const ModelLoadResult loaded = LoadStaticModel(temporary.GetPath());
				if (test >= 2 && test <= 4)
				{
					if (loaded.model || loaded.error.empty())
					{
						return false;
					}
					continue;
				}

				if (!loaded.model)
				{
					std::printf("FAIL: TRS case %d: %s\n", test, loaded.error.c_str());
					return false;
				}

				const DirectX::XMMATRIX actual = loaded.model->transform.GetWorld();
				for (std::size_t row = 0; row < 4; ++row)
				{
					if (!DirectX::XMVector4NearEqual(actual.r[row], world.r[row], DirectX::XMVectorReplicate(1e-4f)))
					{
						return false;
					}
				}
			}

			return true;
		}

		bool CheckMaterialColor(HWND window, const StaticModel& model)
		{
			const ImageData white{ 1, 1, { 255, 255, 255, 255 } };
			const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(
				DirectX::XMVectorSet(0, 0, -3, 1), DirectX::XMVectorZero(), DirectX::XMVectorSet(0, 1, 0, 0));
			const std::array objects = { BuildObjectTransformConstants(
				model.transform.GetWorld(), view * DirectX::XMMatrixOrthographicLH(2, 2, 0.1f, 10)).value() };
			for (int test = 0; test < 2; ++test)
			{
				// 중간 선형 값은 sRGB 이중 디코딩을 구분한다. alpha는 불투명 출력을 유지한다.
				const std::array<float, 4> factor = test == 0 ? model.baseColor :
					std::array<float, 4>{ 0.25f, 0.5f, 0.75f, 0.2f };
				D3D12Renderer renderer;
				if (!renderer.Initialize(window, 256, 256, white, &model.mesh, factor))
				{
					return false;
				}

				RendererReadback readback{ renderer };
				FrameLighting lighting;
				lighting.intensity = 0;
				lighting.ambientStrength = 1;
				lighting.specularStrength = 0;
				std::vector<std::uint32_t> pixels;
				if (!readback.Capture(objects, lighting, pixels))
				{
					return false;
				}
				const std::uint32_t actual = pixels[128 * 256 + 128];
				for (unsigned channel = 0; channel < 4; ++channel)
				{
					const float linear = factor[channel];
					const float srgb = linear <= 0.0031308f ? 12.92f * linear :
						1.055f * std::pow(linear, 1.0f / 2.4f) - 0.055f;
					const int expected = channel == 3 ? 255 : static_cast<int>(std::lround(srgb * 255));
					if (std::abs(static_cast<int>((actual >> (channel * 8)) & 255) - expected) > 1)
					{
						std::printf(
							"FAIL: material test %d channel %u: %08x expected %d\n",
							test,
							channel,
							actual,
							expected);
						return false;
					}
				}
			}

			std::puts("Material: BrickWallCube current v2, linear BaseColor and opaque alpha passed");
			return true;
		}

		bool CheckSurfaceModel(const std::vector<std::uint8_t>& original)
		{
			TemporaryModelFile temporary;
			if (!temporary.Initialize())
			{
				return false;
			}

			for (int test = 0; test < 6; ++test)
			{
				std::vector<std::uint8_t> bytes = original;
				const float half = 0.5f;
				const std::array<float, 4> uv = { 2, 3, 0.25f, 0.125f };
				std::memcpy(bytes.data() + 1523, &half, sizeof(half));
				std::memcpy(bytes.data() + 1595, uv.data(), sizeof(uv));
				if (test == 1)
				{
					bytes[1519] = 0; // 선형 입력은 재변환하지 않는다.
				}
				if (test == 2)
				{
					bytes[1515] = 1; // Unlit을 Phong으로 조용히 대체하지 않는다.
				}
				if (test == 3)
				{
					bytes[1575] = 1; // 반투명 표면은 지원 범위 밖이다.
				}
				if (test >= 4)
				{
					// BaseColor 바인딩의 wrapU(1651)·wrapV(1655)는 0~2만 허용한다.
					const std::uint32_t invalidWrap = 3;
					std::memcpy(bytes.data() + 1651 + (test - 4) * 4, &invalidWrap, sizeof(invalidWrap));
				}
				if (!temporary.Write(bytes))
				{
					return false;
				}

				const ModelLoadResult changed = LoadStaticModel(temporary.GetPath());
				if (changed.model.has_value() != (test < 2))
				{
					std::printf("FAIL: surface case %d: %s\n", test, changed.error.c_str());
					return false;
				}
				if (!changed.model && changed.error.empty())
				{
					std::puts("FAIL: rejected surface has no diagnostic");
					return false;
				}
				if (test >= 4 && changed.error.find("Invalid texture wrap mode") == std::string::npos)
				{
					std::puts("FAIL: invalid U/V wrap did not reach Reader validation");
					return false;
				}
				if (changed.model)
				{
					const float red = test == 0 ? 0.21404114f : 0.5f;
					const auto& vertices = changed.model->mesh.vertices;
					if (std::abs(changed.model->baseColor[0] - red) > 1e-6f ||
						vertices[0].uv[0] != 0.25f || vertices[0].uv[1] != 0.875f ||
						vertices[2].uv[0] != 0.25f || vertices[2].uv[1] != -2.125f)
					{
						std::puts("FAIL: current v2 gamma or UV transform order");
						return false;
					}
				}
			}

			std::puts("current v2: texture filename, gamma/linear color, UV, surface limits and invalid U/V wrap passed");
			return true;
		}

		bool CheckModelFixture(const std::filesystem::path& path)
		{
			std::ifstream file(path, std::ios::binary);
			if (!file)
			{
				// 시스템 코드 페이지로 표현할 수 없는 경로도 진단 중 예외 없이 출력한다.
				const std::u8string utf8Path = path.u8string();
				std::printf("FAIL: cannot open fixed model fixture: %s\n",
					reinterpret_cast<const char*>(utf8Path.c_str()));
				return false;
			}
			const std::vector<std::uint8_t> original{
				std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()
			};
			if (file.bad() || original.size() != MODEL_FILE_SIZE)
			{
				std::puts("FAIL: fixed model fixture read or size mismatch; restore CubeV2.bin");
				return false;
			}
			const ModelLoadResult baseline = LoadStaticModel(path);
			if (!baseline.model)
			{
				std::printf("FAIL: fixed model fixture baseline: %s\n", baseline.error.c_str());
				return false;
			}
			if (!CheckSurfaceModel(original))
			{
				return false;
			}

			if (!CheckModelTransforms(original))
			{
				std::puts("FAIL: importer TRS conversion or unsupported transform/normal rejection");
				return false;
			}

			for (int corruption = 0; corruption < 5; ++corruption)
			{
				std::vector<std::uint8_t> bytes = original;
				if (corruption == 0)
				{
					bytes.pop_back();
				}
				else if (corruption == 1)
				{
					bytes[19] = 99; // version
				}
				else if (corruption == 2)
				{
					std::fill(bytes.begin() + 1348, bytes.begin() + 1352, std::uint8_t{ 255 }); // first index
				}
				else if (corruption == 3)
				{
					bytes.push_back(0);
				}
				else
				{
					bytes[19] = 1; // Wrap이 없는 이전 v1을 기본값으로 보완하지 않는다.
				}

				try
				{
					// 거부 예외가 기대 결과이므로 반환 모델은 의도적으로 사용하지 않는다.
					(void)custom_asset_export::ReadModel(bytes.data(), bytes.size());
					std::printf("FAIL: corrupt binary accepted: %d\n", corruption);
					return false;
				}
				catch (const custom_asset_export::ReadError& error)
				{
					if (corruption == 4 && error.GetCode() != custom_asset_export::ErrorCode::UnsupportedVersion)
					{
						std::puts("FAIL: v1 did not report UnsupportedVersion");
						return false;
					}
				}
			}

			std::puts("Fixed model fixture: TRS, surface and corrupt inputs passed");
			return true;
		}
	}

	bool CheckModelRegression()
	{
		const ExecutableDirectoryResult directory = GetExecutableDirectory();
		if (FAILED(directory.result))
		{
			return false;
		}

		const std::filesystem::path sourceDirectory = directory.directory / RNIMAN_SOURCE_ROOT;
		if (!CheckModelFixture(sourceDirectory / L"Tests/Assets/ModelFixture/CubeV2.bin"))
		{
			return false;
		}

		for (const wchar_t* legacyName : { L"CubeExample.bin", L"BlueCube.bin" })
		{
			const std::filesystem::path legacyPath = sourceDirectory / L"Tests/Assets/LegacyModels" / legacyName;
			// 파일 누락을 포맷 거부 성공으로 처리하지 않는다.
			std::ifstream input(legacyPath, std::ios::binary);
			if (!input || input.peek() == std::ifstream::traits_type::eof())
			{
				std::printf("FAIL: legacy fixture missing, unreadable or empty: %ls\n", legacyName);
				return false;
			}

			const ModelLoadResult legacy = LoadStaticModel(legacyPath);
			if (legacy.model || legacy.error.empty())
			{
				std::puts("FAIL: experimental format accepted or rejected without diagnostic");
				return false;
			}
		}

		std::puts("Legacy model fixtures: unsupported formats rejected");
		return true;
	}

	bool CheckImportedModel(HWND window)
	{
		const ExecutableDirectoryResult directory = GetExecutableDirectory();
		if (FAILED(directory.result))
		{
			return false;
		}

		const std::filesystem::path sourceDirectory = directory.directory / RNIMAN_SOURCE_ROOT;
		const std::filesystem::path path = sourceDirectory / L"Assets/Models/BrickWallCube.bin";
		const ModelLoadResult loaded = LoadStaticModel(path);
		if (!loaded.model)
		{
			std::printf("FAIL: model load %s\n", loaded.error.c_str());
			return false;
		}


		const StaticModel& model = *loaded.model;
		if (model.baseColor != std::array<float, 4>{ 1, 1, 1, 1 } ||
			model.baseColorTextureRef != "BrickWall17_1K_BaseColor" || loaded.warning.empty())
		{
			std::puts("FAIL: current v2 material color, texture filename or preview warning");
			return false;
		}

		if (!CheckMaterialColor(window, model))
		{
			return false;
		}

		if (model.mesh.vertices.size() != 24 || model.mesh.indices.size() != 36 ||
			model.mesh.vertices[0].uv[0] != 0 || model.mesh.vertices[0].uv[1] != 1)
		{
			std::puts("FAIL: imported counts or Unity UV conversion");
			return false;
		}

		DirectX::XMFLOAT3 minimum{ FLT_MAX, FLT_MAX, FLT_MAX };
		DirectX::XMFLOAT3 maximum{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
		for (const MeshVertex& vertex : model.mesh.vertices)
		{
			const DirectX::XMVECTOR position = DirectX::XMVector3TransformCoord(
				DirectX::XMVectorSet(vertex.position[0], vertex.position[1], vertex.position[2], 1),
				model.transform.GetWorld());
			DirectX::XMStoreFloat3(&minimum, DirectX::XMVectorMin(DirectX::XMLoadFloat3(&minimum), position));
			DirectX::XMStoreFloat3(&maximum, DirectX::XMVectorMax(DirectX::XMLoadFloat3(&maximum), position));
		}

		if (minimum.x != -0.5f || minimum.y != -0.5f || minimum.z != -0.5f ||
			maximum.x != 0.5f || maximum.y != 0.5f || maximum.z != 0.5f)
		{
			std::puts("FAIL: imported world bounds");
			return false;
		}

		if (LoadStaticModel(path / L"missing.bin").model)
		{
			return false;
		}

		// 기존 큐브와 독립적으로 비교: 파일 큐브는 한 변 1, 회귀 큐브는 한 변 2다.
		// 서로 다른 정점·삼각형 배치를 같은 위치에 그려 컬링·법선·Transform·Phong을 비교한다.
		const ImageData white{ 1, 1, { 255, 255, 255, 255 } };
		std::array<std::vector<std::uint32_t>, 3> expected;
		for (int source = 0; source < 2; ++source)
		{
			D3D12Renderer renderer;
			if (!renderer.Initialize(window, 256, 256, white, source == 0 ? nullptr : &model.mesh))
			{
				return false;
			}

			RendererReadback readback{ renderer };
			for (std::size_t test = 0; test < expected.size(); ++test)
			{
				Transform edited = model.transform;
				edited.position = { 1, 2, 3 };
				edited.scale = { 2, 1, 0.5f };
				const float step = static_cast<float>(test);
				edited.rotation = *QuaternionFromEuler({ 0.2f * step, -0.35f * step, 0.1f * step });
				const DirectX::XMMATRIX world = (source == 0 ? DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f) :
					DirectX::XMMatrixIdentity()) * edited.GetWorld();
				const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(
					DirectX::XMVectorSet(4, 4, -3, 1),
					DirectX::XMVectorSet(1, 2, 3, 1),
					DirectX::XMVectorSet(0, 1, 0, 0));
				const DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicLH(4, 4, 0.1f, 20);
				const std::array objects = {
					BuildObjectTransformConstants(world, view * projection).value(),
					BuildObjectTransformConstants(
						world * DirectX::XMMatrixTranslation(-1, -1, 2),
						view * projection).value()
				};
				FrameLighting lighting;
				lighting.direction = { 0, 0, 1 };
				lighting.cameraPosition = { 4, 4, -3 };
				std::vector<std::uint32_t> pixels;
				if (!readback.Capture(objects, lighting, pixels))
				{
					return false;
				}

				if (source == 0)
				{
					expected[test] = std::move(pixels);
					continue;
				}

				std::size_t different = 0;
				std::size_t visible = 0;
				for (std::size_t index = 0; index < pixels.size(); ++index)
				{
					visible += expected[test][index] != expected[test][0];
					for (unsigned channel = 0; channel < 4; ++channel)
					{
						const int observed = (pixels[index] >> (channel * 8)) & 255;
						const int reference = (expected[test][index] >> (channel * 8)) & 255;
						if (std::abs(observed - reference) > 2)
						{
							++different;
							break;
						}
					}
				}

				std::printf("Imported mesh Phong test %zu: visible=%zu different=%zu\n", test, visible, different);
				if (visible < 1000 || different > 20)
				{
					return false;
				}
			}
		}

		// Unity Cube의 -Z면 UV는 x/y에 대해 뒤집혀 있다. 비대칭 2x2 PNG 행 순서를 GPU에서 검사한다.
		const ImageData colors{ 2, 2, {
			255, 0, 0, 255, 0, 255, 0, 255,
			0, 0, 255, 255, 255, 255, 0, 255
		} };
		D3D12Renderer renderer;
		if (!renderer.Initialize(window, 256, 256, colors, &model.mesh))
		{
			return false;
		}

		RendererReadback readback{ renderer };
		const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(
			DirectX::XMVectorSet(0, 0, -3, 1),
			DirectX::XMVectorZero(),
			DirectX::XMVectorSet(0, 1, 0, 0));
		const DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicLH(2, 2, 0.1f, 10) *
			DirectX::XMMatrixTranslation(1.0f / 256, -1.0f / 256, 0);
		const std::array objects = {
			BuildObjectTransformConstants(DirectX::XMMatrixIdentity(), view * projection).value()
		};
		FrameLighting ambient;
		ambient.intensity = 0;
		ambient.ambientStrength = 1;
		ambient.specularStrength = 0;
		std::vector<std::uint32_t> pixels;
		if (!readback.Capture(objects, ambient, pixels))
		{
			return false;
		}

		constexpr std::array<std::uint32_t, 4> texels = { 0xff00ffff, 0xffff0000, 0xff00ff00, 0xff0000ff };
		for (std::size_t index = 0; index < texels.size(); ++index)
		{
			const std::uint32_t pixel = pixels[(96 + (index / 2) * 64) * 256 + 96 + (index % 2) * 64];
			for (unsigned channel = 0; channel < 4; ++channel)
			{
				if (std::abs(static_cast<int>((pixel >> (channel * 8)) & 255) -
					static_cast<int>((texels[index] >> (channel * 8)) & 255)) > 1)
				{
					std::printf("FAIL: imported UV texel %zu: %08x expected %08x\n", index, pixel, texels[index]);
					return false;
				}
			}
		}

		std::puts("Live model: counts, bounds, material, GPU Phong and UV passed");
		return true;
	}
}
