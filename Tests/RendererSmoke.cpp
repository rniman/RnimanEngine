// 별도 콘솔 테스트이므로 PCH 없이 컴파일한다.
#include "D3D12Renderer.h"
#include "SandboxCamera.h"
#include "Transform.h"

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
#include <span>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace
{
	struct Size
	{
		UINT width;
		UINT height;
	};

	bool CheckTransform()
	{
		Transform transform;
		const auto point = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
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
			return DirectX::XMMatrixInverse(nullptr, camera.GetView()).r[3];
		};
		auto positionsMatch = [](DirectX::FXMVECTOR left, DirectX::FXMVECTOR right)
		{
			return DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(left, right))) < 0.0001f;
		};
		SandboxCamera camera;
		const auto origin = position(camera);
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

		const auto displacement = DirectX::XMVectorSubtract(position(camera), origin);
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
		if (!renderer.Initialize(window.GetHandle(), 1280, 720))
		{
			return 1;
		}

		// 아직 제출하지 않은 상태의 전체 대기는 자원을 건드리지 않아야 한다.
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

		constexpr Size SIZES[] = {
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

			const auto view = DirectX::XMMatrixLookAtLH(
				DirectX::XMVectorSet(0.0f, 2.0f, -5.0f, 1.0f),
				DirectX::XMVectorZero(),
				DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			const auto projection = DirectX::XMMatrixPerspectiveFovLH(
				DirectX::XM_PIDIV4,
				static_cast<float>(width) / static_cast<float>(height),
				0.1f,
				100.0f);

			// 두 슬롯에서 0→1→3→9개를 사용해 확장·재사용·빈 목록을 검증한다.
			constexpr std::size_t COUNTS[] = { 0, 1, 3, 9 };
			const std::size_t count = COUNTS[(renderedFrames / 2) % 4];
			std::array<DirectX::XMFLOAT4X4, 9> transforms;
			for (std::size_t index = 0; index < count; ++index)
			{
				Transform object;
				object.position = { static_cast<float>(index) * 0.3f, 0.0f, static_cast<float>(index) * 0.5f };
				object.rotation = { angle * 0.5f, angle, 0.0f };
				object.scale = { 0.5f, 0.8f, 0.6f };
				DirectX::XMStoreFloat4x4(&transforms[index], object.GetWorld() * view * projection);
			}

			if ((renderedFrames % 2) != 0)
			{
				std::reverse(transforms.begin(), transforms.begin() + count);
			}

			++renderedFrames;
			return renderer.Render(std::span<const DirectX::XMFLOAT4X4>(transforms.data(), count));
		};

		for (auto size : SIZES)
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
