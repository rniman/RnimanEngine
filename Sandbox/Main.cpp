#include "pch.h"

#include <Windows.h>

#include <cwchar>

namespace
{
	constexpr wchar_t WINDOW_CLASS_NAME[] = L"RnimanSandboxWindow";
	constexpr wchar_t WINDOW_TITLE[] = L"Rniman Engine - Sandbox";

	void ReportWin32Error(const wchar_t* operation, DWORD error)
	{
		wchar_t message[512]{};
		swprintf_s(message, L"%ls failed. Win32 error: %lu\n", operation, error);
		OutputDebugStringW(message);
		MessageBoxW(nullptr, message, L"Sandbox error", MB_OK | MB_ICONERROR);
	}

	LRESULT CALLBACK ProcessWindowMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_CLOSE:
			// The message loop exits before the owning scope destroys the window.
			PostQuitMessage(0);
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		default:
			return DefWindowProcW(window, message, wParam, lParam);
		}
	}

	// One UI thread owns both resources; destroy the window before unregistering its class.
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

		bool Initialize(int showCommand)
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
			if (!AdjustWindowRectEx(&bounds, WS_OVERLAPPEDWINDOW, FALSE, 0))
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
				nullptr);
			if (mWindow == nullptr)
			{
				ReportWin32Error(L"CreateWindowExW", GetLastError());
				return false;
			}

			ShowWindow(mWindow, showCommand);
			return true;
		}

	private:
		HINSTANCE mInstance;
		HWND mWindow = nullptr;
		bool mClassRegistered = false;
	};
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
	WindowResources window{ instance };
	if (!window.Initialize(showCommand))
	{
		return 1;
	}

	// Wait for messages while no rendering loop exists; do not busy-spin.
	MSG message{};
	while (true)
	{
		BOOL result = GetMessageW(&message, nullptr, 0, 0);
		if (result == -1)
		{
			ReportWin32Error(L"GetMessageW", GetLastError());
			return 1;
		}
		if (result == 0)
		{
			return static_cast<int>(message.wParam);
		}
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}
}
