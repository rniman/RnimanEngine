#pragma once

#include <Windows.h>

#include <array>
#include <optional>

namespace rengine
{
	/// @brief UI 스레드에서 창과 입력 수집 상태를 소유한다. GPU·카메라는 참조하지 않는다.
	/// @note 현재 앱의 창 하나를 대상으로 한다. Initialize는 한 번만 호출하며 복사·이동하지 않는다.
	class Win32Window
	{
	public:
		struct State
		{
			UINT width = 1280;
			UINT height = 720;
			bool minimized = false;
			bool resizePending = false;
			bool focused = false;
		};

		struct InputState
		{
			std::array<bool, 256> keys{};
			float mouseDeltaX = 0.0f;
			float mouseDeltaY = 0.0f;
			bool resetFrameTime = true;
		};

		explicit Win32Window(HINSTANCE instance);
		~Win32Window();

		Win32Window(const Win32Window&) = delete;
		Win32Window& operator=(const Win32Window&) = delete;
		Win32Window(Win32Window&&) = delete;
		Win32Window& operator=(Win32Window&&) = delete;

		/// @brief 창을 등록·생성하고 ShowWindow로 표시 상태를 설정한다.
		/// @return 실패 시 false. API와 Win32 오류를 보고하며 부분 자원은 소멸자가 정리한다.
		[[nodiscard]] bool Initialize(int showCommand);

		/// @brief 비소유 HWND. 이 창을 사용하는 렌더러는 창보다 먼저 정리한다.
		HWND GetHandle() const;

		/// @brief 창이 소유하는 상태. 참조는 객체 수명 동안 유효하며 메시지 처리 시 갱신된다.
		const State& GetState() const;

		/// @brief 입력을 복사하고 마우스 이동량·시간 초기화 요청만 비운다. 키 눌림은 유지한다.
		[[nodiscard]] InputState ConsumeInput();

		/// @brief 앱이 프레임 경계의 GPU 리사이즈에 성공한 뒤 호출한다.
		void ClearResizeRequest();

		/// @brief UI 스레드의 메시지를 처리한다. WM_QUIT이면 종료 코드, 아니면 nullopt를 반환한다.
		[[nodiscard]] std::optional<int> PumpMessages();

		/// @brief 최소화·0 크기에서 다음 메시지를 기다린다. 실패 시 Win32 오류를 보고한다.
		[[nodiscard]] bool WaitForMessage() const;

	private:
		static LRESULT CALLBACK ProcessWindowMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT HandleMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
		void ClearInput(HWND window);

		HINSTANCE mInstance;
		HWND mWindow = nullptr;
		bool mClassRegistered = false;
		State mState;
		InputState mInput;
		bool mRotating = false;
		POINT mMousePosition{};
	};
}
