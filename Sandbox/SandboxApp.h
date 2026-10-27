#pragma once

#include "SandboxCamera.h"
#include "SandboxGui.h"
#include "SandboxLighting.h"

#include "Math/Transform.h"
#include "Platform/Win32Window.h"
#include "Rendering/D3D12Renderer.h"

// Windows 선언은 DirectX 헤더보다 먼저 필요하다.
#include <Windows.h>
#include <DirectXMath.h>

#include <array>
#include <cstddef>

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
		struct Cube
		{
			Transform transform;
			// 개체 주소 대신 회전 성분을 지정하므로 Cube를 복사해도 다른 개체를 참조하지 않는다.
			float DirectX::XMFLOAT3::* rotationAxis;
			double rotationSpeed; // 회전 속도(radian/second)
			bool autoRotate;
		};

		// 초기 목록에서 개수를 유도한다. 선택 UI·회전·렌더링은 이 목록과 같은 크기를 사용한다.
		static constexpr std::array INITIAL_OBJECTS = {
			Cube{
				Transform{ { -2.5f, 0.0f, 1.0f }, {}, { 0.65f, 0.65f, 0.65f } },
				&DirectX::XMFLOAT3::y, 0.7, true
			},
			Cube{
				Transform{ { 0.0f, 1.0f, 4.0f }, {}, { 0.8f, 1.1f, 0.6f } },
				&DirectX::XMFLOAT3::x, 0.35, true
			},
			Cube{
				Transform{ { 3.5f, -3.0f, 3.0f }, {}, { 0.75f, 0.5f, 0.75f } },
				&DirectX::XMFLOAT3::z, -0.7, true
			}
		};

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
		double mPreviousAnimationTime = 0.0;
		std::size_t mSelectedObject = 0;
		bool mShowDemo = false;
		bool mInvalidEdit = false;
		std::array<Cube, INITIAL_OBJECTS.size()> mObjects = INITIAL_OBJECTS;
	};
}
