// 별도 콘솔 테스트이므로 PCH 없이 컴파일한다.
#include "D3D12Renderer.h"

#include <Windows.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <cstdio>
#include <cstdlib>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace
{
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

		struct Size
		{
			UINT width;
			UINT height;
		};
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
		for (auto size : SIZES)
		{
			if (!renderer.Resize(size.width, size.height))
			{
				return 1;
			}

			for (int frame = 0; frame < 3; ++frame)
			{
				if (!renderer.Render())
				{
					return 1;
				}
			}
		}

		if (!renderer.WaitForGpu())
		{
			return 1;
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
		"24 frames, 8 resize requests, destruction: errors=%u warnings=%u\n",
		errors,
		warnings);
	return errors == 0 && warnings == 0 ? 0 : 1;
}
