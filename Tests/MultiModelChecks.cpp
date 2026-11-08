#include "MultiModelChecks.h"

#include "RendererReadback.h"

#include "Assets/AssetLoader.h"
#include "Assets/ImageData.h"
#include "Assets/MeshData.h"
#include "Assets/ModelLoader.h"
#include "Rendering/D3D12Renderer.h"
#include "Rendering/RenderData.h"

// Windows 선언은 DirectX 헤더보다 먼저 필요하다.
#include <Windows.h>
#include <DirectXMath.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <initializer_list>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace rengine
{
	bool CheckJpegLoading()
	{
		const ExecutableDirectoryResult executable = GetExecutableDirectory();
		if (FAILED(executable.result))
		{
			return false;
		}

		const std::filesystem::path directory = executable.directory / RNIMAN_SOURCE_ROOT / L"Tests/Assets";
		const ImageLoadResult jpeg = LoadImage(directory / L"ColorTest.jpg");
		if (FAILED(jpeg.result) || jpeg.image.width != 3 || jpeg.image.height != 2 || jpeg.image.pixels.size() != 24)
		{
			std::puts("FAIL: JPEG dimensions or decode");
			return false;
		}

		// JPEG의 양자화 오차만 허용하며 누락된 alpha는 모든 픽셀에서 255여야 한다.
		constexpr std::array<int, 3> color = { 64, 128, 192 };
		for (std::size_t pixel = 0; pixel < 6; ++pixel)
		{
			for (std::size_t channel = 0; channel < color.size(); ++channel)
			{
				if (std::abs(static_cast<int>(jpeg.image.pixels[pixel * 4 + channel]) - color[channel]) > 2)
				{
					std::puts("FAIL: JPEG RGB conversion");
					return false;
				}
			}
			if (jpeg.image.pixels[pixel * 4 + 3] != 255)
			{
				std::puts("FAIL: JPEG alpha");
				return false;
			}
		}

		const ImageLoadResult strict = LoadPngImage(directory / L"ColorTest.jpg");
		if (SUCCEEDED(strict.result) || !strict.image.pixels.empty())
		{
			std::puts("FAIL: PNG-only API accepted JPEG");
			return false;
		}

		const ImageLoadResult missing = LoadImage(directory / L"Missing.jpg");
		if (SUCCEEDED(missing.result) || !missing.image.pixels.empty())
		{
			return false;
		}

		const ImageLoadResult png = LoadImage(directory / L"TextureTest.png");
		const ImageLoadResult reference = LoadPngImage(directory / L"TextureTest.png");
		if (FAILED(png.result) || FAILED(reference.result) || png.image.pixels != reference.image.pixels ||
			png.image.width != reference.image.width || png.image.height != reference.image.height)
		{
			std::puts("FAIL: shared PNG decoder changed output");
			return false;
		}

		std::puts("JPEG: RGBA8, opaque alpha, PNG-only rejection, missing input and PNG byte equality passed");
		return true;
	}

	bool CheckMultipleModels(HWND window)
	{
		const ExecutableDirectoryResult executable = GetExecutableDirectory();
		if (FAILED(executable.result))
		{
			return false;
		}

		constexpr std::array files = { L"BrickWallCube.bin", L"BrickWallSphere.bin", L"DamagedHelmet.bin" };
		constexpr std::array<std::size_t, 3> vertexCounts = { 24, 515, 14556 };
		constexpr std::array<std::size_t, 3> indexCounts = { 36, 2304, 46356 };
		std::array<StaticModel, 3> models;
		for (std::size_t index = 0; index < models.size(); ++index)
		{
			ModelLoadResult loaded = LoadStaticModel(
				executable.directory / RNIMAN_SOURCE_ROOT / L"Assets/Models" / files[index]);
			if (!loaded.model || loaded.model->mesh.vertices.size() != vertexCounts[index] ||
				loaded.model->mesh.indices.size() != indexCounts[index])
			{
				std::printf("FAIL: multi-model input %zu: %s\n", index, loaded.error.c_str());
				return false;
			}

			models[index] = std::move(*loaded.model);
		}

		// 외부 텍스처 없이도 모델마다 다른 SRV와 BaseColor 선택을 구분할 수 있는 입력이다.
		std::array<ImageData, 3> images = {
			ImageData{ 1, 1, { 255, 128, 64, 255 } },
			ImageData{ 1, 1, { 32, 255, 96, 255 } },
			ImageData{ 1, 1, { 64, 96, 255, 255 } }
		};
		const std::array sources = {
			D3D12Renderer::ModelSource{ images[0], &models[0].mesh, { 1, 0.5f, 0.5f, 1 } },
			D3D12Renderer::ModelSource{ images[1], &models[1].mesh, { 0.75f, 1, 0.5f, 1 } },
			D3D12Renderer::ModelSource{ images[2], &models[2].mesh, { 0.5f, 0.75f, 1, 1 } }
		};
		const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(
			DirectX::XMVectorSet(0, 0, -6, 1),
			DirectX::XMVectorZero(),
			DirectX::XMVectorSet(0, 1, 0, 0));
		const DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicLH(7, 4, 0.1f, 20);
		std::array<ObjectTransformConstants, 3> objects;
		for (std::size_t index = 0; index < objects.size(); ++index)
		{
			const DirectX::XMMATRIX world = models[index].transform.GetWorld() *
				DirectX::XMMatrixTranslation((static_cast<float>(index) - 1.0f) * 2.0f, 0, 0);
			objects[index] = BuildObjectTransformConstants(world, view * projection).value();
		}
		FrameLighting lighting;
		lighting.direction = { 0, 0, 1 };
		lighting.cameraPosition = { 0, 0, -6 };

		std::array<std::vector<std::uint32_t>, 3> references;
		for (std::size_t index = 0; index < sources.size(); ++index)
		{
			D3D12Renderer renderer;
			if (!renderer.Initialize(window, 320, 240, std::span<const D3D12Renderer::ModelSource>(&sources[index], 1)))
			{
				return false;
			}

			RendererReadback readback{ renderer };
			if (!readback.Capture(
				std::span<const ObjectTransformConstants>(&objects[index], 1),
				lighting,
				references[index]))
			{
				return false;
			}
		}

		D3D12Renderer renderer;
		if (!renderer.Initialize(window, 320, 240, sources))
		{
			return false;
		}

		// 초기화 후 CPU 입력을 해제해도 GPU 모델은 그릴 수 있어야 한다.
		models = {};
		images = {};
		RendererReadback readback{ renderer };
		constexpr std::array<std::size_t, 3> order = { 2, 0, 1 };
		const std::array reordered = { objects[2], objects[0], objects[1] };
		std::vector<std::uint32_t> pixels;
		if (!readback.Capture(reordered, lighting, pixels, nullptr, order))
		{
			return false;
		}

		const std::uint32_t background = references[0][0];
		std::array<std::size_t, 3> visible{};
		std::size_t different = 0;
		for (std::size_t pixel = 0; pixel < pixels.size(); ++pixel)
		{
			std::uint32_t expected = background;
			for (std::size_t model = 0; model < references.size(); ++model)
			{
				if (references[model][pixel] != background)
				{
					++visible[model];
					expected = references[model][pixel];
				}
			}
			different += pixels[pixel] != expected;
		}
		std::printf(
			"Multi-model GPU: visible=%zu/%zu/%zu different=%zu\n",
			visible[0], visible[1], visible[2], different);
		if (different != 0 || *std::min_element(visible.begin(), visible.end()) < 100)
		{
			return false;
		}

		if (!renderer.Resize(640, 480))
		{
			return false;
		}
		if (!renderer.Resize(320, 240))
		{
			return false;
		}

		std::vector<std::uint32_t> restored;
		if (!readback.Capture(reordered, lighting, restored, nullptr, order))
		{
			return false;
		}

		if (restored != pixels)
		{
			std::puts("FAIL: multi-model resources changed after resize");
			return false;
		}

		// 서로 떨어진 Cube 6개의 단독 결과를 합쳐 같은 모델의 반복 인스턴스 기준을 만든다.
		std::vector<ObjectTransformConstants> repeated;
		std::vector<std::size_t> repeatedModels(6, 0);
		std::vector<std::uint32_t> expected(pixels.size(), background);
		std::vector<std::uint32_t> single;
		std::vector<std::uint32_t> firstReference;
		for (std::size_t index = 0; index < repeatedModels.size(); ++index)
		{
			const DirectX::XMMATRIX world = DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f) *
				DirectX::XMMatrixTranslation(
					(static_cast<float>(index % 3) - 1.0f) * 2.0f,
					index < 3 ? -1.0f : 1.0f, 0);
			repeated.push_back(BuildObjectTransformConstants(world, view * projection).value());
			if (!readback.Capture(std::span<const ObjectTransformConstants>(&repeated.back(), 1), lighting, single))
			{
				return false;
			}
			std::size_t covered = 0;
			if (index == 0)
			{
				firstReference = single;
			}
			for (std::size_t pixel = 0; pixel < single.size(); ++pixel)
			{
				if (single[pixel] != background)
				{
					expected[pixel] = single[pixel];
					++covered;
				}
			}
			if (covered < 100)
			{
				std::printf("FAIL: repeated model reference %zu is not visible\n", index);
				return false;
			}
		}

		// 각 슬롯의 readback을 비교한 뒤 Present로 다음 슬롯으로 이동한다.
		for (const std::size_t count : { 0u, 6u, 1u, 0u, 6u })
		{
			const std::span<const ObjectTransformConstants> frameObjects(repeated.data(), count);
			const std::span<const std::size_t> frameModels(repeatedModels.data(), count);
			for (std::size_t slot = 0; slot < 2; ++slot)
			{
				if (!readback.Capture(frameObjects, lighting, restored, nullptr, frameModels))
				{
					return false;
				}
				for (std::size_t pixel = 0; pixel < restored.size(); ++pixel)
				{
					const std::uint32_t expectedPixel = count == 0 ? background :
						(count == 1 ? firstReference[pixel] : expected[pixel]);
					if (restored[pixel] != expectedPixel)
					{
						std::printf("FAIL: instance count %zu slot pass %zu pixel %zu\n", count, slot, pixel);
						return false;
					}
				}
				if (!renderer.Render(frameObjects, lighting, nullptr, frameModels))
				{
					return false;
				}
			}
		}

		std::puts("Multi-model: resource selection, lifetime, resize, empty/grow/shrink and repeated model passed");
		return true;
	}

	bool CheckTextureAddressing(HWND window)
	{
		const ExecutableDirectoryResult executable = GetExecutableDirectory();
		if (FAILED(executable.result))
		{
			return false;
		}
		const std::filesystem::path fixtures = executable.directory / RNIMAN_SOURCE_ROOT /
			L"Tests/Assets/WrapModes";
		const ImageData colors{ 2, 2, {
			255, 0, 0, 255, 0, 255, 0, 255,
			0, 0, 255, 255, 255, 255, 0, 255
		} };
		std::array<StaticModel, 9> imported;
		std::vector<D3D12Renderer::ModelSource> sources;
		sources.reserve(9);
		for (std::uint32_t u = 0; u < 3; ++u)
		{
			for (std::uint32_t v = 0; v < 3; ++v)
			{
				const std::filesystem::path file = fixtures /
					("surface_wrap_" + std::to_string(u) + "_" + std::to_string(v) + ".bin");
				ModelLoadResult loaded = LoadStaticModel(file);
				if (!loaded.model || loaded.model->wrapU != static_cast<TextureWrapMode>(u) ||
					loaded.model->wrapV != static_cast<TextureWrapMode>(v))
				{
					std::printf("FAIL: exported wrap fixture %u/%u: %s\n", u, v, loaded.error.c_str());
					return false;
				}
				StaticModel& model = imported[u * 3 + v];
				model = std::move(*loaded.model);
				sources.push_back({ colors, &model.mesh, model.baseColor, model.wrapU, model.wrapV });
			}
		}
		D3D12Renderer renderer;
		if (!renderer.Initialize(window, 128, 128, sources))
		{
			return false;
		}

		RendererReadback readback{ renderer };
		const std::array objects = { BuildObjectTransformConstants(
			DirectX::XMMatrixIdentity(), DirectX::XMMatrixOrthographicLH(2, 2, 0.1f, 10)).value() };
		FrameLighting lighting;
		lighting.intensity = 0;
		lighting.ambientStrength = 1;
		lighting.specularStrength = 0;
		constexpr std::array<std::uint32_t, 4> texels = { 0xff0000ff, 0xff00ff00, 0xffff0000, 0xff00ffff };
		for (const std::size_t model : { 0u, 4u, 8u, 1u, 2u, 3u, 5u, 6u, 7u, 0u, 4u })
		{
			const std::array selected = { model };
			std::vector<std::uint32_t> pixels;
			if (!readback.Capture(objects, lighting, pixels, nullptr, selected))
			{
				return false;
			}

			for (std::size_t index = 0; index < texels.size(); ++index)
			{
				const std::size_t u = model / 3;
				const std::size_t v = model % 3;
				const std::size_t column = u == 0 ? index % 2 : (u == 1 ? 0 : 1 - index % 2);
				const std::size_t row = v == 0 ? index / 2 : (v == 1 ? 1 : 1 - index / 2);
				const std::uint32_t expected = texels[row * 2 + column];
				const std::uint32_t actual = pixels[(32 + (index / 2) * 64) * 128 + 32 + (index % 2) * 64];
				if (actual != expected)
				{
					std::printf(
						"FAIL: address mode %zu texel %zu: %08x expected %08x\n",
						model, index, actual, expected);
					return false;
				}
			}
		}

		std::puts("Texture addressing: 9 Unity BIN -> loader -> independent U/V samplers; negative U/V and GPU texels passed");
		return true;
	}
}
