// 별도 콘솔 테스트이므로 PCH 없이 컴파일한다.
#include "AssetLoader.h"
#include "D3D12Renderer.h"
#include "ImageData.h"
#include "LightingChecks.h"
#include "RenderData.h"
#include "RendererReadback.h"
#include "SandboxCamera.h"
#include "Transform.h"

// Windows 선언은 DirectX 헤더보다 먼저 필요하다.
#include <Windows.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <ios>
#include <span>
#include <utility>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace
{
	constexpr std::array<std::uint8_t, 36> TEXTURE_PIXELS = {
		240, 32, 16, 255, 16, 224, 48, 255, 32, 48, 240, 255,
		224, 192, 16, 255, 128, 128, 128, 128, 16, 192, 224, 255,
		192, 16, 224, 255, 64, 96, 128, 255, 224, 224, 224, 255
	};

	// 성공 시 image를 교체하고 실패 시 유지한다.
	bool CheckImageLoading(ImageData& image)
	{
		const ExecutableDirectoryResult executableDirectoryResult = GetExecutableDirectory();
		if (FAILED(executableDirectoryResult.result))
		{
			return false;
		}

		const std::filesystem::path directory =
			(executableDirectoryResult.directory / RNIMAN_SOURCE_ROOT / L"Tests" / L"Assets").lexically_normal();
		ImageLoadResult loaded = LoadPngImage(directory / L"TextureTest.png");
		if (FAILED(loaded.result))
		{
			std::printf("FAIL: PNG loading HRESULT=0x%08lX\n", static_cast<unsigned long>(loaded.result));
			return false;
		}

		const bool pixelsMatch = std::equal(
			loaded.image.pixels.begin(), loaded.image.pixels.end(),
			TEXTURE_PIXELS.begin(), TEXTURE_PIXELS.end());
		if (loaded.image.width != 3 || loaded.image.height != 3 || !pixelsMatch)
		{
			return false;
		}

		const ImageLoadResult missing = LoadPngImage(directory / L"MissingTexture.png");
		if (SUCCEEDED(missing.result) || !missing.image.pixels.empty())
		{
			return false;
		}

		// 공유 기준 이미지 폴더에는 쓰지 않고 구성별 테스트 출력에서만 손상 파일을 만든다.
		const std::filesystem::path corruptPath = executableDirectoryResult.directory / L"CorruptTexture.png";
		{
			std::ofstream stream(corruptPath, std::ios::binary);
			stream << "invalid PNG";
			if (!stream)
			{
				return false;
			}
		}

		const ImageLoadResult corrupt = LoadPngImage(corruptPath);
		std::filesystem::remove(corruptPath);
		if (SUCCEEDED(corrupt.result) || !corrupt.image.pixels.empty())
		{
			return false;
		}

		image = std::move(loaded.image);
		std::puts("PNG: 3x3 RGBA8 bytes, missing and corrupt file checks passed");
		return true;
	}

	bool CheckTextureFaces(D3D12Renderer& renderer)
	{
		if (!renderer.Resize(192, 192))
		{
			return false;
		}

		RendererReadback readback{ renderer };
		FrameLighting lighting;
		// 텍스처·가림 기준 이미지는 ambient=1인 동일 조명 경로에서 비교한다.
		lighting.ambientStrength = 1.0f;
		lighting.intensity = 0.0f;
		lighting.specularStrength = 0.0f;
		constexpr std::array<DirectX::XMFLOAT3, 6> eyes = {
			DirectX::XMFLOAT3{ 0, 0, -5 }, { 0, 0, 5 }, { -5, 0, 0 },
			{ 5, 0, 0 }, { 0, 5, 0 }, { 0, -5, 0 }
		};
		constexpr std::array<DirectX::XMFLOAT3, 6> upVectors = {
			DirectX::XMFLOAT3{ 0, 1, 0 }, { 0, 1, 0 }, { 0, 1, 0 },
			{ 0, 1, 0 }, { 0, 0, 1 }, { 0, 0, -1 }
		};
		for (std::size_t face = 0; face < eyes.size(); ++face)
		{
			const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(
				DirectX::XMLoadFloat3(&eyes[face]),
				DirectX::XMVectorZero(),
				DirectX::XMLoadFloat3(&upVectors[face]));
			// 화면의 반 픽셀 이동으로 3x3 텍셀 중심과 readback 픽셀 중심을 일치시킨다.
			const DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicLH(4, 4, 0.1f, 10) *
				DirectX::XMMatrixTranslation(1.0f / 192, -1.0f / 192, 0);
			std::array<ObjectTransformConstants, 1> transforms;
			transforms[0] = BuildObjectTransformConstants(DirectX::XMMatrixIdentity(), view * projection).value();
			std::vector<std::uint32_t> pixels;
			if (!readback.Capture(transforms, lighting, pixels))
			{
				return false;
			}

			for (UINT row = 0; row < 3; ++row)
			{
				for (UINT column = 0; column < 3; ++column)
				{
					const auto pixel = pixels[(64 + row * 32) * 192 + 64 + column * 32];
					for (UINT channel = 0; channel < 4; ++channel)
					{
						const int actual = (pixel >> (channel * 8)) & 0xff;
						const int expected = channel == 3 ? 255 : TEXTURE_PIXELS[(row * 3 + column) * 4 + channel];
						if (std::abs(actual - expected) > 1)
						{
							std::printf(
								"FAIL: texture face=%zu row=%u col=%u channel=%u actual=%d expected=%d\n",
								face, row, column, channel, actual, expected);
							return false;
						}
					}
				}
			}
		}

		std::puts("Texture: 6 faces, UV orientation, upload row padding, sRGB and opaque alpha checks passed");
		return true;
	}

	struct Size
	{
		UINT width;
		UINT height;
	};

	bool CheckOcclusion(D3D12Renderer& renderer)
	{
		RendererReadback readback{ renderer };
		FrameLighting lighting;
		// 텍스처·가림 기준 이미지는 ambient=1인 동일 조명 경로에서 비교한다.
		lighting.ambientStrength = 1.0f;
		lighting.intensity = 0.0f;
		lighting.specularStrength = 0.0f;
		constexpr std::array<Size, 3> sizes = { Size{ 321, 241 }, Size{ 640, 360 }, Size{ 257, 193 } };
		for (std::size_t scenario = 0; scenario < sizes.size(); ++scenario)
		{
			const UINT width = sizes[scenario].width;
			const UINT height = sizes[scenario].height;
			if (!renderer.Resize(width, height))
			{
				return false;
			}

			// 위치가 바뀐 카메라에서도 두 큐브가 겹치지만 서로 교차하지 않는 장면이다.
			const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(
				DirectX::XMVectorSet(static_cast<float>(scenario) * 0.5f, 0.5f, -6.0f, 1.0f),
				DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f),
				DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			const DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(
				DirectX::XM_PIDIV4,
				static_cast<float>(width) / static_cast<float>(height),
				0.1f,
				100.0f);
			Transform front;
			front.position.x = -0.4f;
			front.scale = { 0.8f, 0.8f, 0.8f };
			Transform back;
			back.position = { 0.5f, 0.0f, 3.0f };
			back.rotation.y = 0.6f;
			std::array<ObjectTransformConstants, 2> transforms;
			transforms[0] = BuildObjectTransformConstants(front.GetWorld(), view * projection).value();
			transforms[1] = BuildObjectTransformConstants(back.GetWorld(), view * projection).value();
			std::vector<std::uint32_t> clear;
			std::vector<std::uint32_t> frontOnly;
			std::vector<std::uint32_t> backOnly;
			std::vector<std::uint32_t> forward;
			std::vector<std::uint32_t> reverse;
			if (!readback.Capture({}, lighting, clear))
			{
				return false;
			}

			if (!readback.Capture(std::span(transforms).first(1), lighting, frontOnly))
			{
				return false;
			}

			if (!readback.Capture(std::span(transforms).last(1), lighting, backOnly))
			{
				return false;
			}

			if (!readback.Capture(transforms, lighting, forward))
			{
				return false;
			}

			std::reverse(transforms.begin(), transforms.end());
			if (!readback.Capture(transforms, lighting, reverse))
			{
				return false;
			}

			std::size_t overlap = 0;
			std::size_t visibleBack = 0;
			std::size_t distinctOverlap = 0;
			std::size_t wrong = 0;
			for (std::size_t pixel = 0; pixel < clear.size(); ++pixel)
			{
				const bool hasFront = frontOnly[pixel] != clear[pixel];
				const bool hasBack = backOnly[pixel] != clear[pixel];
				overlap += hasFront && hasBack;
				visibleBack += !hasFront && hasBack;
				distinctOverlap += hasFront && hasBack && frontOnly[pixel] != backOnly[pixel];
				const auto expected = hasFront ? frontOnly[pixel] : backOnly[pixel];
				wrong += forward[pixel] != expected || reverse[pixel] != expected;
			}

			const bool sameOrder = forward == reverse;
			std::printf(
				"Occlusion %ux%u: overlap=%zu distinct=%zu visibleBack=%zu wrong=%zu orderEqual=%d\n",
				width,
				height,
				overlap,
				distinctOverlap,
				visibleBack,
				wrong,
				sameOrder);
			if (overlap < 100 || distinctOverlap < 100 || visibleBack < 100 || wrong != 0 || !sameOrder)
			{
				std::puts("FAIL: occlusion pixels or draw order");
				return false;
			}
		}

		return true;
	}

	bool CheckTransform()
	{
		Transform transform;
		const DirectX::XMVECTOR point = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
		if (!DirectX::XMVector3NearEqual(
			DirectX::XMVector3TransformCoord(point, transform.GetWorld()),
			point,
			DirectX::XMVectorReplicate(0.0001f)))
		{
			return false;
		}

		transform.position = { 3.0f, 4.0f, 5.0f };
		transform.rotation.z = DirectX::XM_PIDIV2;
		transform.scale = { 2.0f, 3.0f, 4.0f };
		return DirectX::XMVector3NearEqual(
			DirectX::XMVector3TransformCoord(point, transform.GetWorld()),
			DirectX::XMVectorSet(3.0f, 6.0f, 5.0f, 1.0f),
			DirectX::XMVectorReplicate(0.0001f));
	}

	bool CheckCamera()
	{
		auto position = [](const SandboxCamera& camera)
		{
			const DirectX::XMFLOAT3 stored = camera.GetPosition();
			return DirectX::XMLoadFloat3(&stored);
		};
		auto positionsMatch = [](DirectX::FXMVECTOR left, DirectX::FXMVECTOR right)
		{
			return DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(left, right))) < 0.0001f;
		};
		SandboxCamera camera;
		const DirectX::XMVECTOR origin = position(camera);
		if (!positionsMatch(origin, DirectX::XMVectorSet(0.0f, 2.0f, -5.0f, 1.0f)))
		{
			return false;
		}

		SandboxCamera stepped;
		camera.Move(1.0f, 0.0f, 1.0f, 1.0f);
		for (int index = 0; index < 100; ++index)
		{
			stepped.Move(1.0f, 0.0f, 1.0f, 0.01f);
		}

		const DirectX::XMVECTOR displacement = DirectX::XMVectorSubtract(position(camera), origin);
		const float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(displacement));
		if (std::abs(distance - 3.0f) > 0.0001f || !positionsMatch(position(camera), position(stepped)))
		{
			return false;
		}

		camera.Move(1.0f, 1.0f, 1.0f, 0.0f);
		camera.Move(0.0f, 0.0f, 0.0f, 1.0f);
		if (!positionsMatch(position(camera), position(stepped)))
		{
			return false;
		}

		camera.Rotate(1000000.0f, 1000000.0f);
		DirectX::XMFLOAT4X4 view;
		DirectX::XMStoreFloat4x4(&view, camera.GetView());
		for (const auto& row : view.m)
		{
			for (float value : row)
			{
				if (!std::isfinite(value))
				{
					return false;
				}
			}
		}

		return true;
	}

	class TestWindow
	{
	public:
		TestWindow()
		{
			WNDCLASSW description{};
			description.lpfnWndProc = DefWindowProcW;
			description.hInstance = GetModuleHandleW(nullptr);
			description.lpszClassName = L"RnimanRendererSmoke";
			mClass = RegisterClassW(&description);
			if (mClass != 0)
			{
				mWindow = CreateWindowExW(
					0,
					description.lpszClassName,
					L"Renderer smoke",
					WS_OVERLAPPEDWINDOW,
					0, 0,
					1280, 720,
					nullptr,
					nullptr,
					description.hInstance,
					nullptr);
			}
		}

		~TestWindow()
		{
			if (mWindow != nullptr && !DestroyWindow(mWindow))
			{
				std::fputs("FAIL: DestroyWindow\n", stderr);
				std::abort();
			}

			if (mClass != 0)
			{
				if (!UnregisterClassW(L"RnimanRendererSmoke", GetModuleHandleW(nullptr)))
				{
					std::fputs("FAIL: UnregisterClassW\n", stderr);
					std::abort();
				}
			}
		}

		TestWindow(const TestWindow&) = delete;
		TestWindow& operator=(const TestWindow&) = delete;

		HWND GetHandle() const
		{
			return mWindow;
		}

	private:
		ATOM mClass = 0;
		HWND mWindow = nullptr;
	};
}

int main()
{
	if (!CheckLightingData())
	{
		std::puts("FAIL: lighting control or transform contract");
		return 1;
	}

	ImageData texture;
	if (!CheckImageLoading(texture))
	{
		std::puts("FAIL: image decoding or error contracts");
		return 1;
	}

	if (!CheckTransform())
	{
		std::puts("FAIL: Transform identity or scale-rotation-translation order");
		return 1;
	}

	std::puts("Transform: identity and scale-rotation-translation checks passed");
	if (!CheckCamera())
	{
		std::puts("FAIL: camera movement, frame time or rotation");
		return 1;
	}

	std::puts("Camera: initial position, diagonal speed, frame time and pitch checks passed");

	ComPtr<ID3D12Debug> debug;
	HRESULT result = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
	if (FAILED(result))
	{
		std::printf("FAIL: Debug layer unavailable: 0x%08lX\n", result);
		return 1;
	}

	debug->EnableDebugLayer();

	ComPtr<ID3D12InfoQueue> messages;
	{
		TestWindow window;
		if (window.GetHandle() == nullptr)
		{
			return 1;
		}

		D3D12Renderer renderer;
		if (!renderer.Initialize(window.GetHandle(), 1280, 720, texture))
		{
			return 1;
		}

		// Initialize 이후 CPU 픽셀을 해제해도 GPU 텍스처와 디스크립터는 렌더러가 유지한다.
		texture = {};

		// 초기 업로드 완료 이후 반복 전체 대기는 자원을 건드리지 않아야 한다.
		if (!renderer.WaitForGpu())
		{
			return 1;
		}

		// 같은 Adapter의 Device는 프로세스 내 singleton이다. 테스트가 InfoQueue를 보관하여
		// 렌더러 공개 API를 진단용으로 확장하지 않고 생성·렌더링·파괴 메시지를 검사한다.
		ComPtr<IDXGIFactory6> factory;
		if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory))))
		{
			return 1;
		}

		ComPtr<ID3D12Device> device;
		for (UINT index = 0; ; ++index)
		{
			ComPtr<IDXGIAdapter1> adapter;
			result = factory->EnumAdapterByGpuPreference(
				index,
				DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
				IID_PPV_ARGS(&adapter));
			if (result == DXGI_ERROR_NOT_FOUND)
			{
				break;
			}

			if (FAILED(result))
			{
				return 1;
			}

			DXGI_ADAPTER_DESC1 description{};
			if (FAILED(adapter->GetDesc1(&description)))
			{
				return 1;
			}

			if ((description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
			{
				continue;
			}

			if (SUCCEEDED(D3D12CreateDevice(
				adapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&device))))
			{
				std::printf(
					"Hardware adapter selected: vendor=0x%04X device=0x%04X\n",
					description.VendorId,
					description.DeviceId);
				break;
			}
		}

		if (!device)
		{
			std::puts("FAIL: no supported hardware adapter");
			return 1;
		}

		if (FAILED(device.As(&messages)))
		{
			return 1;
		}

		if (!CheckTextureFaces(renderer))
		{
			return 1;
		}

		if (!CheckOcclusion(renderer))
		{
			return 1;
		}

		if (!CheckPhongLighting(renderer))
		{
			return 1;
		}

		constexpr Size sizes[] = {
			{ 1280, 720 },
			{ 640, 480 },
			{ 0, 0 },
			{ 1, 1 },
			{ 1920, 1080 },
			{ 800, 600 },
			{ 800, 600 },
			{ 1280, 720 }
		};
		UINT width = 1280;
		UINT height = 720;
		UINT renderedFrames = 0;
		auto renderFrame = [&]()
		{
			const float angle = static_cast<float>(renderedFrames) * 0.07f;

			const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(
				DirectX::XMVectorSet(0.0f, 2.0f, -5.0f, 1.0f),
				DirectX::XMVectorZero(),
				DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			const DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(
				DirectX::XM_PIDIV4,
				static_cast<float>(width) / static_cast<float>(height),
				0.1f,
				100.0f);

			// 두 슬롯에서 0→1→3→9개를 사용해 확장·재사용·빈 목록을 검증한다.
			constexpr std::size_t counts[] = { 0, 1, 3, 9 };
			const std::size_t count = counts[(renderedFrames / 2) % 4];
			std::array<ObjectTransformConstants, 9> transforms;
			for (std::size_t index = 0; index < count; ++index)
			{
				Transform object;
				object.position = { static_cast<float>(index) * 0.3f, 0.0f, static_cast<float>(index) * 0.5f };
				object.rotation = { angle * 0.5f, angle, 0.0f };
				object.scale = { 0.5f, 0.8f, 0.6f };
				transforms[index] = BuildObjectTransformConstants(object.GetWorld(), view * projection).value();
			}

			if ((renderedFrames % 2) != 0)
			{
				std::reverse(transforms.begin(), transforms.begin() + count);
			}

			++renderedFrames;
			FrameLighting lighting;
			DirectX::XMStoreFloat3(&lighting.direction, DirectX::XMVector3Normalize(
				DirectX::XMVectorSet(std::sin(angle), -1.0f, std::cos(angle), 0.0f)));
			lighting.intensity = 0.2f + static_cast<float>(renderedFrames % 10) * 0.1f;
			return renderer.Render(std::span<const ObjectTransformConstants>(transforms.data(), count), lighting);
		};

		for (auto size : sizes)
		{
			if (!renderer.Resize(size.width, size.height))
			{
				return 1;
			}

			if (size.width != 0 && size.height != 0)
			{
				width = size.width;
				height = size.height;
			}

			// 명시적 전체 대기 없이 두 프레임 자원을 반복해서 재사용한다.
			for (int frame = 0; frame < 16; ++frame)
			{
				if (!renderFrame())
				{
					return 1;
				}
			}
		}

		if (!renderer.WaitForGpu())
		{
			return 1;
		}

		if (!renderer.WaitForGpu())
		{
			return 1;
		}

		// 전체 대기 후에도 제출을 재개할 수 있으며 마지막 제출의 정리는 소멸자가 맡는다.
		for (int frame = 0; frame < 8; ++frame)
		{
			if (!renderFrame())
			{
				return 1;
			}
		}
	}

	if (messages->GetNumMessagesDiscardedByMessageCountLimit() != 0)
	{
		std::puts("FAIL: debug messages were discarded");
		return 1;
	}

	UINT errors = 0;
	UINT warnings = 0;
	for (UINT64 index = 0; index < messages->GetNumStoredMessages(); ++index)
	{
		SIZE_T size = 0;
		if (FAILED(messages->GetMessage(index, nullptr, &size)))
		{
			return 1;
		}

		std::vector<unsigned char> storage(size);
		auto* message = reinterpret_cast<D3D12_MESSAGE*>(storage.data());
		if (FAILED(messages->GetMessage(index, message, &size)))
		{
			return 1;
		}

		if (message->Severity <= D3D12_MESSAGE_SEVERITY_ERROR)
		{
			++errors;
		}

		if (message->Severity == D3D12_MESSAGE_SEVERITY_WARNING)
		{
			++warnings;
		}

		if (message->Severity <= D3D12_MESSAGE_SEVERITY_WARNING)
		{
			std::printf("D3D12: %s\n", message->pDescription);
		}
	}

	std::printf(
		"136 frames, 8 resize requests, repeated flush, resume, destruction: errors=%u warnings=%u\n",
		errors,
		warnings);
	return errors == 0 && warnings == 0 ? 0 : 1;
}
