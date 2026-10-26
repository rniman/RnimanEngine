#pragma once

#include "SandboxCamera.h"
#include "SandboxGui.h"
#include "SandboxLighting.h"

#include "Math/Transform.h"
#include "Platform/Win32Window.h"
#include "Rendering/D3D12Renderer.h"

#include <Windows.h>

#include <array>

namespace rengine
{
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
		void DrawTools();
		void DrawObjectTools();
		void DrawLightingTools();
		[[nodiscard]] bool Render();

		// 선언 역순으로 UI → 렌더러 → 창을 정리한다. UI 콜백·GPU 자원은 앞선 소유자에 의존한다.
		Win32Window mWindow;
		D3D12Renderer mRenderer;
		SandboxGui mGui;

		SandboxCamera mCamera;
		SandboxLighting mLighting;
		std::array<bool, 3> mAutoRotate = { true, true, true };
		double mPreviousAnimationTime = 0.0;
		int mSelectedObject = 0;
		bool mShowDemo = false;
		bool mInvalidEdit = false;
		std::array<Transform, 3> mObjects = {
			Transform{ { -2.5f, 0.0f, 1.0f }, {}, { 0.65f, 0.65f, 0.65f } },
			Transform{ { 0.0f, 1.0f, 4.0f }, {}, { 0.8f, 1.1f, 0.6f } },
			Transform{ { 3.5f, -3.0f, 3.0f }, {}, { 0.75f, 0.5f, 0.75f } }
		};
	};
}
