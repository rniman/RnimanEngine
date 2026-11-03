#pragma once

#include "SandboxCamera.h"
#include "SandboxGui.h"
#include "SandboxLighting.h"
#include "SandboxRotation.h"

#include "Math/Transform.h"
#include "Platform/Win32Window.h"
#include "Rendering/D3D12Renderer.h"

// Windows 선언은 DirectX 헤더보다 먼저 필요하다.
#include <Windows.h>
#include <DirectXMath.h>

#include <array>
#include <cstddef>
#include <string>

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
		struct ModelInstance
		{
			Transform transform;
			// 월드 공간의 고정 회전축. 편집한 Quaternion에 증분 회전을 합성한다.
			DirectX::XMFLOAT3 rotationAxis;
			double rotationSpeed; // 회전 속도(radian/second)
			bool autoRotate;
			SandboxRotation rotationEditor;
		};

		// 초기 목록에서 개수를 유도한다. 선택 UI·회전·렌더링은 이 목록과 같은 크기를 사용한다.
		static constexpr std::array INITIAL_OBJECTS = {
			ModelInstance{
				Transform{}, // 로딩 성공 시 파일의 Transform으로 교체한다.
				{ 0, 1, 0 }, 0.7, false, {}
			},
			ModelInstance{
				Transform{ { -2.0f, 0.0f, 0.0f }, { 0, 0, 0, 1 }, { 1.25f, 1.25f, 1.25f } },
				{ 1, 0, 0 }, 0.35, true, {}
			},
			ModelInstance{
				Transform{ { 2.0f, 0.0f, 0.0f }, { 0, 0, 0, 1 }, { 0.8f, 0.8f, 0.8f } },
				{ 0, 0, 1 }, -0.7, true, {}
			}
		};

		struct ModelInfo
		{
			std::size_t vertexCount = 0;
			std::size_t triangleCount = 0;
			std::string materialWarning;
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
		std::array<ModelInfo, INITIAL_OBJECTS.size()> mModelInfo;
		bool mShowDemo = false;
		bool mInvalidEdit = false;
		std::array<ModelInstance, INITIAL_OBJECTS.size()> mObjects = INITIAL_OBJECTS;
	};
}
