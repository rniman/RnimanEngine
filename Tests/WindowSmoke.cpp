#include "Win32Window.h"

#include <Windows.h>

#include <algorithm>
#include <cstdio>
#include <optional>

namespace rengine
{
	namespace
	{
		bool Check(bool condition, const char* operation)
		{
			if (!condition)
			{
				std::printf("FAIL: %s\n", operation);
			}

			return condition;
		}

		bool IsCleared(const Win32Window::InputState& input)
		{
			return std::none_of(input.keys.begin(), input.keys.end(), [](bool pressed) { return pressed; }) &&
				input.mouseDeltaX == 0.0f && input.mouseDeltaY == 0.0f && input.resetFrameTime;
		}

		bool CheckWindow(Win32Window& window)
		{
			const HWND handle = window.GetHandle();
			const Win32Window::State& state = window.GetState();
			if (!Check(state.width == 1280 && state.height == 720, "initial client size"))
			{
				return false;
			}

			// 키는 유지하면서 프레임마다 누적 입력이 다시 적용되지 않는 경계를 검사한다.
			SendMessageW(handle, WM_SETFOCUS, 0, 0);
			SendMessageW(handle, WM_KEYDOWN, 'W', 0);
			const Win32Window::InputState first = window.ConsumeInput();
			const Win32Window::InputState next = window.ConsumeInput();
			if (!Check(state.focused && first.keys['W'] && first.resetFrameTime &&
				next.keys['W'] && !next.resetFrameTime, "key hold and one-shot time reset"))
			{
				return false;
			}

			constexpr UINT lightingKeys[] = { VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN, VK_PRIOR, VK_NEXT, 'R' };
			for (UINT key : lightingKeys)
			{
				SendMessageW(handle, WM_KEYDOWN, key, 0);
			}

			const Win32Window::InputState lightingInput = window.ConsumeInput();
			for (UINT key : lightingKeys)
			{
				if (!Check(lightingInput.keys[key], "lighting key hold"))
				{
					return false;
				}

				SendMessageW(handle, WM_KEYUP, key, 0);
			}

			const Win32Window::InputState lightingReleased = window.ConsumeInput();
			for (UINT key : lightingKeys)
			{
				if (!Check(!lightingReleased.keys[key], "lighting key release"))
				{
					return false;
				}
			}

			SendMessageW(handle, WM_RBUTTONDOWN, 0, MAKELPARAM(10, 20));
			if (!Check(GetCapture() == handle, "mouse capture"))
			{
				return false;
			}

			SendMessageW(handle, WM_MOUSEMOVE, 0, MAKELPARAM(-5, 30));
			SendMessageW(handle, WM_MOUSEMOVE, 0, MAKELPARAM(-8, 24));
			const Win32Window::InputState drag = window.ConsumeInput();
			const Win32Window::InputState consumed = window.ConsumeInput();
			if (!Check(drag.mouseDeltaX == -18.0f && drag.mouseDeltaY == 4.0f &&
				consumed.mouseDeltaX == 0.0f && consumed.mouseDeltaY == 0.0f && consumed.keys['W'],
				"signed mouse coordinates and one-shot accumulated drag"))
			{
				return false;
			}

			SendMessageW(handle, WM_MOUSEMOVE, 0, MAKELPARAM(40, 40));
			if (!Check(ReleaseCapture() != FALSE, "release capture"))
			{
				return false;
			}

			SendMessageW(handle, WM_MOUSEMOVE, 0, MAKELPARAM(60, 60));
			const Win32Window::InputState lostCapture = window.ConsumeInput();
			if (!Check(lostCapture.keys['W'] && lostCapture.mouseDeltaX == 0.0f && lostCapture.mouseDeltaY == 0.0f,
				"capture loss cancels pending and subsequent drag"))
			{
				return false;
			}

			SendMessageW(handle, WM_RBUTTONDOWN, 0, MAKELPARAM(10, 20));
			SendMessageW(handle, WM_MOUSEMOVE, 0, MAKELPARAM(20, 30));
			SendMessageW(handle, WM_KILLFOCUS, 0, 0);
			SendMessageW(handle, WM_KEYDOWN, 'D', 0);
			if (!Check(!state.focused && GetCapture() != handle && IsCleared(window.ConsumeInput()),
				"focus loss releases capture, clears input and ignores unfocused keys"))
			{
				return false;
			}

			SendMessageW(handle, WM_SETFOCUS, 0, 0);
			constexpr UINT resetMessages[] = {
				WM_ENTERSIZEMOVE, WM_EXITSIZEMOVE, WM_ENTERMENULOOP, WM_EXITMENULOOP, WM_CANCELMODE
			};
			for (UINT message : resetMessages)
			{
				SendMessageW(handle, WM_KEYDOWN, 'W', 0);
				SendMessageW(handle, WM_RBUTTONDOWN, 0, MAKELPARAM(10, 20));
				SendMessageW(handle, WM_MOUSEMOVE, 0, MAKELPARAM(20, 30));
				SendMessageW(handle, message, 0, 0);
				if (!Check(GetCapture() != handle && IsCleared(window.ConsumeInput()), "modal input reset"))
				{
					return false;
				}
			}

			SendMessageW(handle, WM_KEYDOWN, 'W', 0);
			SendMessageW(handle, WM_SIZE, SIZE_MINIMIZED, 0);
			if (!Check(state.minimized && state.width == 0 && state.height == 0 &&
				state.resizePending && IsCleared(window.ConsumeInput()), "minimize clears input and requests resize"))
			{
				return false;
			}

			SendMessageW(handle, WM_SIZE, SIZE_RESTORED, MAKELPARAM(640, 480));
			const Win32Window::InputState restored = window.ConsumeInput();
			if (!Check(!state.minimized && state.width == 640 && state.height == 480 &&
				state.resizePending && restored.resetFrameTime && !restored.keys['W'], "restore keeps keys released"))
			{
				return false;
			}

			window.ClearResizeRequest();
			if (!Check(!state.resizePending, "resize acknowledgement"))
			{
				return false;
			}

			if (!Check(PostMessageW(handle, WM_KEYDOWN, 'E', 0) != FALSE, "post keyboard message"))
			{
				return false;
			}

			if (!window.WaitForMessage())
			{
				return false;
			}

			const std::optional<int> queuedExitCode = window.PumpMessages();
			const Win32Window::InputState queuedInput = window.ConsumeInput();
			if (!Check(!queuedExitCode.has_value() && queuedInput.keys['E'], "pump queued input"))
			{
				return false;
			}

			SendMessageW(handle, WM_KEYUP, 'E', 0);
			if (!Check(!window.ConsumeInput().keys['E'], "key release"))
			{
				return false;
			}

			PostQuitMessage(7);
			if (!Check(window.PumpMessages() == 7, "nonzero quit code"))
			{
				return false;
			}

			SendMessageW(handle, WM_CLOSE, 0, 0);
			return Check(window.PumpMessages() == 0 && IsWindow(handle), "close requests exit before window destruction");
		}
	}
}

int main()
{
	HWND previousHandle = nullptr;
	{
		rengine::Win32Window window{ GetModuleHandleW(nullptr) };
		if (!window.Initialize(SW_HIDE))
		{
			return 1;
		}

		previousHandle = window.GetHandle();
		if (!rengine::CheckWindow(window))
		{
			return 1;
		}
	}

	if (!rengine::Check(
		!IsWindow(previousHandle) && GetCapture() != previousHandle,
		"destructor destroys window and capture"))
	{
		return 1;
	}

	// WM_DESTROY의 종료 알림을 소비하고 창 클래스 등록·해제를 반복할 수 있는지 확인한다.
	MSG message{};
	while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	{
		rengine::Win32Window window{ GetModuleHandleW(nullptr) };
		if (!window.Initialize(SW_HIDE))
		{
			return 1;
		}
	}

	std::puts("Window: input consumption, capture/focus/modal reset, resize, message pump and lifetime checks passed");
	return 0;
}
