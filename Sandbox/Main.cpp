#include "pch.h"
#include "D3D12Renderer.h"
#include "SandboxCamera.h"

#include <Windows.h>
#include <DirectXMath.h>
#include <windowsx.h>

#include <algorithm>
#include <array>
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

		bool focused = false;
		bool resetFrameTime = true;

		bool rotating = false;
		POINT mousePosition{};
		float mouseDeltaX = 0.0f;
		float mouseDeltaY = 0.0f;
		std::array<bool, 256> keys{};
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

	void ClearInput(HWND window, WindowState& state)
	{
		state.keys.fill(false);
		state.rotating = false;
		state.mouseDeltaX = 0.0f;
		state.mouseDeltaY = 0.0f;
		state.resetFrameTime = true;

		if (GetCapture() == window && !ReleaseCapture())
		{
			ReportWin32Error(L"ReleaseCapture", GetLastError());
		}
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

			LONG_PTR setResult = SetWindowLongPtrW(
				window,
				GWLP_USERDATA,
				reinterpret_cast<LONG_PTR>(creation->lpCreateParams));
			if (setResult == 0 && GetLastError() != ERROR_SUCCESS)
			{
				ReportWin32Error(L"SetWindowLongPtrW", GetLastError());
				return FALSE;
			}
		}

		auto* state = reinterpret_cast<WindowState*>(GetWindowLongPtrW(window, GWLP_USERDATA));
		switch (message)
		{
		case WM_SETFOCUS:
			if (state != nullptr)
			{
				state->focused = true;
				state->resetFrameTime = true;
			}

			return 0;

		case WM_KILLFOCUS:
			if (state != nullptr)
			{
				state->focused = false;
				ClearInput(window, *state);
			}

			return 0;

		case WM_ENTERSIZEMOVE:
		case WM_EXITSIZEMOVE:
		case WM_ENTERMENULOOP:
		case WM_EXITMENULOOP:
		case WM_CANCELMODE:
			if (state != nullptr)
			{
				ClearInput(window, *state);
			}

			return 0;

		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
			if (state != nullptr && state->focused && wParam < state->keys.size())
			{
				state->keys[wParam] = message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
			}

			if (message == WM_SYSKEYDOWN || message == WM_SYSKEYUP)
			{
				return DefWindowProcW(window, message, wParam, lParam);
			}

			return 0;

		case WM_RBUTTONDOWN:
			if (state != nullptr && state->focused)
			{
				SetCapture(window);
				state->rotating = GetCapture() == window;
				if (!state->rotating)
				{
					OutputDebugStringW(L"SetCapture: camera mouse capture was not acquired.\n");
				}

				state->mousePosition = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			}

			return 0;

		case WM_RBUTTONUP:
			if (state != nullptr)
			{
				state->rotating = false;
				if (GetCapture() == window && !ReleaseCapture())
				{
					ReportWin32Error(L"ReleaseCapture", GetLastError());
				}
			}

			return 0;

		case WM_CAPTURECHANGED:
			if (state != nullptr)
			{
				state->rotating = false;
				state->mouseDeltaX = 0.0f;
				state->mouseDeltaY = 0.0f;
			}

			return 0;

		case WM_MOUSEMOVE:
			if (state != nullptr && state->rotating)
			{
				const POINT position{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				state->mouseDeltaX += static_cast<float>(position.x - state->mousePosition.x);
				state->mouseDeltaY += static_cast<float>(position.y - state->mousePosition.y);
				state->mousePosition = position;
			}

			return 0;

		case WM_SIZE:
			if (state != nullptr)
			{
				state->width = LOWORD(lParam);
				state->height = HIWORD(lParam);
				state->minimized = wParam == SIZE_MINIMIZED;
				state->resizePending = true;
				state->resetFrameTime = true;
				if (state->minimized)
				{
					ClearInput(window, *state);
				}
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
	auto previousTime = startTime;
	SandboxCamera camera;

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

		const auto now = std::chrono::steady_clock::now();
		// 모달 창 조작·최소화 이후 경과 시간은 이동에 누적하지 않는다.
		const float deltaSeconds = state.resetFrameTime ? 0.0f :
			std::min(std::chrono::duration<float>(now - previousTime).count(), 0.1f);
		previousTime = now;
		state.resetFrameTime = false;

		if (state.focused)
		{
			camera.Rotate(state.mouseDeltaX, state.mouseDeltaY);
			camera.Move(
				static_cast<float>(state.keys['D']) - static_cast<float>(state.keys['A']),
				static_cast<float>(state.keys['E']) - static_cast<float>(state.keys['Q']),
				static_cast<float>(state.keys['W']) - static_cast<float>(state.keys['S']),
				deltaSeconds);
		}

		state.mouseDeltaX = 0.0f;
		state.mouseDeltaY = 0.0f;

		// 큐브 회전은 입력 중단과 관계없이 시작 이후 경과 시간을 사용한다.
		const double seconds = std::chrono::duration<double>(now - startTime).count();
		const float angle = static_cast<float>(std::fmod(seconds * 0.7, DirectX::XM_2PI));
		const auto world = DirectX::XMMatrixRotationY(angle);

		const auto view = camera.GetView();
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
