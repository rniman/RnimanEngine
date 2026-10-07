#include "pch.h"
#include "Win32Window.h"

#include <Windows.h>
#include <windowsx.h>

#include <cwchar>
#include <optional>

namespace
{
	constexpr wchar_t WINDOW_CLASS_NAME[] = L"RnimanSandboxWindow";
	constexpr wchar_t WINDOW_TITLE[] = L"Rniman Engine - Sandbox";

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
}

Win32Window::Win32Window(HINSTANCE instance)
	: mInstance{ instance }
{
}

Win32Window::~Win32Window()
{
	// DestroyWindow 중에도 콜백이 상태를 참조하므로 소멸자 본문이 끝날 때까지 유지한다.
	if (mWindow != nullptr && !DestroyWindow(mWindow))
	{
		ReportWin32Error(L"DestroyWindow", GetLastError());
	}

	if (mClassRegistered && !UnregisterClassW(WINDOW_CLASS_NAME, mInstance))
	{
		ReportWin32Error(L"UnregisterClassW", GetLastError());
	}
}

bool Win32Window::Initialize(int showCommand)
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
		this);
	if (mWindow == nullptr)
	{
		ReportWin32Error(L"CreateWindowExW", GetLastError());
		return false;
	}

	ShowWindow(mWindow, showCommand);
	return true;
}

HWND Win32Window::GetHandle() const
{
	return mWindow;
}

const Win32Window::State& Win32Window::GetState() const
{
	return mState;
}

Win32Window::InputState Win32Window::ConsumeInput()
{
	const InputState input = mInput;
	mInput.mouseDeltaX = 0.0f;
	mInput.mouseDeltaY = 0.0f;
	mInput.resetFrameTime = false;
	return input;
}

void Win32Window::ClearResizeRequest()
{
	mState.resizePending = false;
}

std::optional<int> Win32Window::PumpMessages()
{
	MSG message{};
	while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
	{
		if (message.message == WM_QUIT)
		{
			return static_cast<int>(message.wParam);
		}

		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	return std::nullopt;
}

bool Win32Window::WaitForMessage() const
{
	if (!WaitMessage())
	{
		ReportWin32Error(L"WaitMessage", GetLastError());
		return false;
	}

	return true;
}

void Win32Window::ClearInput(HWND window)
{
	mInput.keys.fill(false);
	mRotating = false;
	mInput.mouseDeltaX = 0.0f;
	mInput.mouseDeltaY = 0.0f;
	mInput.resetFrameTime = true;

	if (GetCapture() == window && !ReleaseCapture())
	{
		ReportWin32Error(L"ReleaseCapture", GetLastError());
	}
}

LRESULT CALLBACK Win32Window::ProcessWindowMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
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

	auto* owner = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(window, GWLP_USERDATA));
	if (owner == nullptr)
	{
		return DefWindowProcW(window, message, wParam, lParam);
	}

	return owner->HandleMessage(window, message, wParam, lParam);
}

LRESULT Win32Window::HandleMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SETFOCUS:
		mState.focused = true;
		mInput.resetFrameTime = true;
		return 0;

	case WM_KILLFOCUS:
		mState.focused = false;
		ClearInput(window);
		return 0;

	case WM_ENTERSIZEMOVE:
	case WM_EXITSIZEMOVE:
	case WM_ENTERMENULOOP:
	case WM_EXITMENULOOP:
	case WM_CANCELMODE:
		ClearInput(window);
		return 0;

	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		if (mState.focused && wParam < mInput.keys.size())
		{
			mInput.keys[wParam] = message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
		}

		if (message == WM_SYSKEYDOWN || message == WM_SYSKEYUP)
		{
			return DefWindowProcW(window, message, wParam, lParam);
		}

		return 0;

	case WM_RBUTTONDOWN:
		if (mState.focused)
		{
			SetCapture(window);
			mRotating = GetCapture() == window;
			if (!mRotating)
			{
				OutputDebugStringW(L"SetCapture: camera mouse capture was not acquired.\n");
			}

			mMousePosition = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		}

		return 0;

	case WM_RBUTTONUP:
		mRotating = false;
		if (GetCapture() == window && !ReleaseCapture())
		{
			ReportWin32Error(L"ReleaseCapture", GetLastError());
		}

		return 0;

	case WM_CAPTURECHANGED:
		mRotating = false;
		mInput.mouseDeltaX = 0.0f;
		mInput.mouseDeltaY = 0.0f;
		return 0;

	case WM_MOUSEMOVE:
		if (mRotating)
		{
			const POINT position{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			mInput.mouseDeltaX += static_cast<float>(position.x - mMousePosition.x);
			mInput.mouseDeltaY += static_cast<float>(position.y - mMousePosition.y);
			mMousePosition = position;
		}

		return 0;

	case WM_SIZE:
		mState.width = LOWORD(lParam);
		mState.height = HIWORD(lParam);
		mState.minimized = wParam == SIZE_MINIMIZED;
		mState.resizePending = true;
		mInput.resetFrameTime = true;
		if (mState.minimized)
		{
			ClearInput(window);
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
		return DefWindowProcW(window, message, wParam, lParam);
	}
}
