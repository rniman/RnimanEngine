#pragma once

#include "D3D12Renderer.h"
#include "SandboxCamera.h"
#include "Transform.h"
#include "Win32Window.h"

#include <Windows.h>

#include <array>

/// @brief UI 스레드에서 창·렌더러·카메라·데모 오브젝트의 수명과 프레임 실행을 관리한다.
class SandboxApp
{
public:
	explicit SandboxApp(HINSTANCE instance);

	SandboxApp(const SandboxApp&) = delete;
	SandboxApp& operator=(const SandboxApp&) = delete;
	SandboxApp(SandboxApp&&) = delete;
	SandboxApp& operator=(SandboxApp&&) = delete;

	/// @brief 한 번만 호출한다. 창·렌더러 초기화부터 메시지 루프와 종료 시 GPU 대기까지 실행한다.
	/// @return 정상 종료 시 WM_QUIT 코드, 초기화·실행·GPU 대기 실패 시 1. 반환 후 앱을 파괴한다.
	[[nodiscard]] int Run(int showCommand);

private:
	[[nodiscard]] bool Initialize(int showCommand);
	void Update(const Win32Window::InputState& input, float deltaSeconds, double elapsedSeconds);
	[[nodiscard]] bool Render();

	// 선언 역순으로 파괴되므로 렌더러의 GPU 자원을 HWND보다 먼저 정리한다.
	Win32Window mWindow;
	D3D12Renderer mRenderer;
	SandboxCamera mCamera;
	std::array<Transform, 3> mObjects = {
		Transform{ { -2.5f, 0.0f, 1.0f }, {}, { 0.65f, 0.65f, 0.65f } },
		Transform{ { 0.0f, 1.0f, 4.0f }, {}, { 0.8f, 1.1f, 0.6f } },
		Transform{ { 3.5f, -3.0f, 3.0f }, {}, { 0.75f, 0.5f, 0.75f } }
	};
};
