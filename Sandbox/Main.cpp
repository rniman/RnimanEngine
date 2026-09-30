#include "pch.h"
#include "D3D12Renderer.h"

#include <Windows.h>
#include <DirectXMath.h>

#include <chrono>
#include <cmath>
#include <cwchar>

namespace
{
	constexpr wchar_t WINDOW_CLASS_NAME[] = L"RnimanSandboxWindow";
	constexpr wchar_t WINDOW_TITLE[] = L"Rniman Engine - Sandbox";

	struct WindowState
	{
		UINT width = 1280;
		UINT height = 720;
		bool minimized = false;
		bool resizePending = false;
	};

	void ReportWin32Error(const wchar_t* operation, DWORD error)
	{
		wchar_t message[512]{};
		swprintf_s(
			message,
			L"%ls failed. Win32 error: %lu\n",
			operation,
			error);
		OutputDebugStringW(message);
		MessageBoxW(nullptr, message, L"Sandbox error", MB_OK | MB_ICONERROR);
	}

	LRESULT CALLBACK ProcessWindowMessage(
		HWND window,
		UINT message,
		WPARAM wParam,
		LPARAM lParam)
	{
		if (message == WM_NCCREATE)
		{
			auto* creation = reinterpret_cast<CREATESTRUCTW*>(lParam);
			SetLastError(ERROR_SUCCESS);
			if (SetWindowLongPtrW(
				window,
				GWLP_USERDATA,
				reinterpret_cast<LONG_PTR>(creation->lpCreateParams)) == 0 &&
				GetLastError() != ERROR_SUCCESS)
			{
				ReportWin32Error(L"SetWindowLongPtrW", GetLastError());
				return FALSE;
			}
		}

		auto* state = reinterpret_cast<WindowState*>(GetWindowLongPtrW(window, GWLP_USERDATA));
		switch (message)
		{
		case WM_SIZE:
			if (state != nullptr)
			{
				state->width = LOWORD(lParam);
				state->height = HIWORD(lParam);
				state->minimized = wParam == SIZE_MINIMIZED;
				state->resizePending = true;
			}

			return 0;
		case WM_CLOSE:
			// 앱이 GPU 작업을 정리한 뒤 창 소유자를 파괴하도록 루프만 종료한다.
			PostQuitMessage(0);
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		default:
			return DefWindowProcW(
				window,
				message,
				wParam,
				lParam);
		}
	}

	// UI 스레드가 창을 소유하며 창 파괴 후 클래스를 해제한다.
	class WindowResources
	{
	public:
		explicit WindowResources(HINSTANCE instance)
			: mInstance{ instance }
		{}

		~WindowResources()
		{
			if (mWindow != nullptr && !DestroyWindow(mWindow))
			{
				ReportWin32Error(L"DestroyWindow", GetLastError());
			}

			if (mClassRegistered && !UnregisterClassW(WINDOW_CLASS_NAME, mInstance))
			{
				ReportWin32Error(L"UnregisterClassW", GetLastError());
			}
		}

		WindowResources(const WindowResources&) = delete;
		WindowResources& operator=(const WindowResources&) = delete;

		bool Initialize(int showCommand, WindowState& state)
		{
			WNDCLASSEXW windowClass{};
			windowClass.cbSize = sizeof(windowClass);
			windowClass.lpfnWndProc = ProcessWindowMessage;
			windowClass.hInstance = mInstance;
			windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
			if (windowClass.hCursor == nullptr)
			{
				ReportWin32Error(L"LoadCursorW", GetLastError());
				return false;
			}

			windowClass.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
			windowClass.lpszClassName = WINDOW_CLASS_NAME;
			if (RegisterClassExW(&windowClass) == 0)
			{
				ReportWin32Error(L"RegisterClassExW", GetLastError());
				return false;
			}

			mClassRegistered = true;

			RECT bounds{ 0, 0, 1280, 720 };
			if (!AdjustWindowRectEx(
				&bounds,
				WS_OVERLAPPEDWINDOW,
				FALSE,
				0))
			{
				ReportWin32Error(L"AdjustWindowRectEx", GetLastError());
				return false;
			}

			mWindow = CreateWindowExW(
				0,
				WINDOW_CLASS_NAME,
				WINDOW_TITLE,
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				bounds.right - bounds.left,
				bounds.bottom - bounds.top,
				nullptr,
				nullptr,
				mInstance,
				&state);
			if (mWindow == nullptr)
			{
				ReportWin32Error(L"CreateWindowExW", GetLastError());
				return false;
			}

			ShowWindow(mWindow, showCommand);
			return true;
		}

		HWND GetHandle() const
		{
			return mWindow;
		}

	private:
		HINSTANCE mInstance;
		HWND mWindow = nullptr;
		bool mClassRegistered = false;
	};
}

int WINAPI wWinMain(
	HINSTANCE instance,
	HINSTANCE,
	PWSTR,
	int showCommand)
{
	// 콜백의 비소유 상태 포인터는 창 파괴까지 유효해야 한다.
	WindowState state;
	WindowResources window{ instance };
	if (!window.Initialize(showCommand, state))
	{
		return 1;
	}

	// 역순 파괴로 렌더러의 GPU 자원이 HWND보다 먼저 정리된다.
	D3D12Renderer renderer;
	if (!renderer.Initialize(
		window.GetHandle(),
		state.width == 0 ? 1280 : state.width,
		state.height == 0 ? 720 : state.height))
	{
		return 1;
	}

	const auto startTime = std::chrono::steady_clock::now();
	while (true)
	{
		MSG message{};
		while (PeekMessageW(
			&message,
			nullptr,
			0,
			0,
			PM_REMOVE))
		{
			if (message.message == WM_QUIT)
			{
				return renderer.WaitForGpu() ? static_cast<int>(message.wParam) : 1;
			}

			TranslateMessage(&message);
			DispatchMessageW(&message);
		}

		if (state.minimized || state.width == 0 || state.height == 0)
		{
			if (!WaitMessage())
			{
				ReportWin32Error(L"WaitMessage", GetLastError());
				return 1;
			}

			continue;
		}

		if (state.resizePending)
		{
			if (!renderer.Resize(state.width, state.height))
			{
				return 1;
			}

			state.resizePending = false;
		}

		// 시간·Transform·카메라는 앱이 결정하고 렌더러에는 최종 행렬만 전달한다.
		const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - startTime).count();
		const float angle = static_cast<float>(std::fmod(seconds * 0.7, DirectX::XM_2PI));
		const auto world = DirectX::XMMatrixRotationY(angle);

		const auto view = DirectX::XMMatrixLookAtLH(
			DirectX::XMVectorSet(0.0f, 2.0f, -5.0f, 1.0f),
			DirectX::XMVectorZero(),
			DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		const auto projection = DirectX::XMMatrixPerspectiveFovLH(
			DirectX::XM_PIDIV4,
			static_cast<float>(state.width) / static_cast<float>(state.height),
			0.1f,
			100.0f);

		DirectX::XMFLOAT4X4 worldViewProjection;
		DirectX::XMStoreFloat4x4(&worldViewProjection, world * view * projection);
		if (!renderer.Render(worldViewProjection))
		{
			return 1;
		}
	}
}
