#include "pch.h"
#include "SandboxApp.h"

#include "SandboxCamera.h"
#include "SandboxGui.h"
#include "SandboxLighting.h"

#include "Assets/AssetLoader.h"
#include "Math/Transform.h"
#include "Platform/Win32Window.h"
#include "Rendering/D3D12Renderer.h"
#include "Rendering/RenderData.h"

// Windows 선언은 DirectX 헤더보다 먼저 필요하다.
#include <Windows.h>
#include <DirectXMath.h>
#include <imgui.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cwchar>
#include <filesystem>
#include <optional>
#include <string>

namespace rengine
{
	namespace
	{
		bool EditVector3(
			const char* label,
			DirectX::XMFLOAT3& vector,
			float step,
			float minimum,
			float maximum)
		{
			// XMFLOAT3 멤버를 포인터 산술로 순회하지 않고 ImGui의 배열 입력으로 복사한다.
			float values[] = { vector.x, vector.y, vector.z };
			const bool changed = ImGui::DragFloat3(
				label,
				values,
				step,
				minimum,
				maximum,
				"%.3f",
				ImGuiSliderFlags_AlwaysClamp);
			if (changed)
			{
				vector = { values[0], values[1], values[2] };
			}

			return changed;
		}
	}

	SandboxApp::SandboxApp(HINSTANCE instance)
		: mWindow{ instance }, mGui{ mWindow, mRenderer }
	{
	}

	bool SandboxApp::Initialize(int showCommand)
	{
		if (!mWindow.Initialize(showCommand))
		{
			return false;
		}

		const ExecutableDirectoryResult executableDirectoryResult = GetExecutableDirectory();
		if (FAILED(executableDirectoryResult.result))
		{
			wchar_t message[256]{};
			swprintf_s(
				message,
				L"GetModuleFileNameW failed. HRESULT: 0x%08lX",
				static_cast<unsigned long>(executableDirectoryResult.result));
			OutputDebugStringW(message);
			MessageBoxW(mWindow.GetHandle(), message, L"Sandbox asset error", MB_OK | MB_ICONERROR);
			return false;
		}

		// 빌드가 계산한 EXE→저장소 루트 상대 경로를 사용하며 실행용 복사본은 만들지 않는다.
		const std::filesystem::path assetDirectory =
			(executableDirectoryResult.directory / RNIMAN_SOURCE_ROOT / L"Assets").lexically_normal();
		const std::filesystem::path path = assetDirectory / L"BrickWall17_MR_1K" / L"BrickWall17_1K_BaseColor.png";
		const ImageLoadResult loaded = LoadPngImage(path);
		if (FAILED(loaded.result))
		{
			wchar_t errorCode[32]{};
			swprintf_s(errorCode, L"0x%08lX", static_cast<unsigned long>(loaded.result));
			const std::wstring message = std::wstring(loaded.operation) + L" failed. HRESULT: " + errorCode +
				L"\nPath: " + path.wstring();
			OutputDebugStringW(message.c_str());
			MessageBoxW(mWindow.GetHandle(), message.c_str(), L"Sandbox asset error", MB_OK | MB_ICONERROR);
			return false;
		}

		const Win32Window::State& state = mWindow.GetState();
		// 최소화 상태로 시작해도 GPU 자원은 양수 크기로 초기화한다.
		if (!mRenderer.Initialize(
			mWindow.GetHandle(),
			state.width == 0 ? 1280 : state.width,
			state.height == 0 ? 720 : state.height,
			loaded.image))
		{
			return false;
		}

		return mGui.Initialize();
	}

	int SandboxApp::Run(int showCommand)
	{
		if (!Initialize(showCommand))
		{
			return 1;
		}

		const Win32Window::State& state = mWindow.GetState();
		const auto startTime = std::chrono::steady_clock::now();
		auto previousTime = startTime;

		while (true)
		{
			if (const std::optional<int> exitCode = mWindow.PumpMessages())
			{
				return mRenderer.WaitForGpu() ? *exitCode : 1;
			}

			if (state.minimized || state.width == 0 || state.height == 0)
			{
				if (!mWindow.WaitForMessage())
				{
					return 1;
				}

				continue;
			}

			if (state.resizePending)
			{
				if (!mRenderer.Resize(state.width, state.height))
				{
					return 1;
				}

				mWindow.ClearResizeRequest();
			}

			mGui.BeginFrame();
			const Win32Window::InputState input = mWindow.ConsumeInput();
			const auto now = std::chrono::steady_clock::now();
			// 모달 창 조작·최소화 이후 경과 시간은 이동에 누적하지 않는다.
			const float deltaSeconds = input.resetFrameTime ? 0.0f :
				std::min(std::chrono::duration<float>(now - previousTime).count(), 0.1f);
			previousTime = now;

			const double elapsedSeconds = std::chrono::duration<double>(now - startTime).count();
			Update(input, deltaSeconds, elapsedSeconds);
			DrawTools();
			mGui.EndFrame();
			if (!Render())
			{
				return 1;
			}
		}
	}

	void SandboxApp::Update(const Win32Window::InputState& input, float deltaSeconds, double elapsedSeconds)
	{
		if (mWindow.GetState().focused)
		{
			if (input.keys['R'])
			{
				mLighting.Reset();
			}
			else
			{
				mLighting.Adjust(
					static_cast<float>(input.keys[VK_RIGHT]) - static_cast<float>(input.keys[VK_LEFT]),
					static_cast<float>(input.keys[VK_UP]) - static_cast<float>(input.keys[VK_DOWN]),
					static_cast<float>(input.keys[VK_PRIOR]) - static_cast<float>(input.keys[VK_NEXT]),
					deltaSeconds);
			}

			mCamera.Rotate(input.mouseDeltaX, input.mouseDeltaY);
			mCamera.Move(
				static_cast<float>(input.keys['D']) - static_cast<float>(input.keys['A']),
				static_cast<float>(input.keys['E']) - static_cast<float>(input.keys['Q']),
				static_cast<float>(input.keys['W']) - static_cast<float>(input.keys['S']),
				deltaSeconds);
		}

		// 시간 기준은 유지하되 편집한 회전을 덮어쓰지 않는다. 정지 중 시간은 회전에 적립하지 않는다.
		const double animationSeconds = elapsedSeconds - mPreviousAnimationTime;
		mPreviousAnimationTime = elapsedSeconds;
		float* angles[] = { &mObjects[0].rotation.y, &mObjects[1].rotation.x, &mObjects[2].rotation.z };
		constexpr double speeds[] = { 0.7, 0.35, -0.7 };
		for (std::size_t index = 0; index < mObjects.size(); ++index)
		{
			if (mAutoRotate[index])
			{
				*angles[index] = static_cast<float>(std::remainder(
					*angles[index] + animationSeconds * speeds[index],
					static_cast<double>(DirectX::XM_2PI)));
			}
		}
	}

	void SandboxApp::DrawTools()
	{
		ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(400.0f, 505.0f), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Sandbox Tools"))
		{
			const ImGuiIO& io = ImGui::GetIO();
			ImGui::Text(
				"Objects: %zu | %.2f ms/frame | %.1f FPS",
				mObjects.size(),
				1000.0f / io.Framerate,
				io.Framerate);
			ImGui::TextDisabled("Frame interval; not GPU execution time");
			ImGui::Checkbox("Show Dear ImGui Demo", &mShowDemo);
			DrawObjectTools();

			ImGui::SeparatorText("Camera");
			DirectX::XMFLOAT3 cameraPosition = mCamera.GetPosition();
			if (EditVector3("Camera position", cameraPosition, 0.02f, -100.0f, 100.0f))
			{
				mInvalidEdit = !mCamera.SetPosition(cameraPosition);
			}

			DrawLightingTools();
			if (mInvalidEdit)
			{
				ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.3f, 1.0f), "Invalid edit rejected; previous value kept.");
			}
		}

		// Begin이 false인 접힌 창도 End로 닫아야 한다.
		ImGui::End();
		if (mShowDemo)
		{
			ImGui::ShowDemoWindow(&mShowDemo);
		}
	}

	void SandboxApp::DrawObjectTools()
	{
		ImGui::SeparatorText("Cube");
		ImGui::Combo("Selected", &mSelectedObject, "Cube 0\0Cube 1\0Cube 2\0");
		const std::size_t index = static_cast<std::size_t>(mSelectedObject);
		// 편집 복사본 전체를 검증한 뒤 공개해 부분 적용을 피한다.
		Transform edited = mObjects[index];
		bool changed = EditVector3("Position", edited.position, 0.02f, -100.0f, 100.0f);
		const bool rotationChanged = EditVector3(
			"Rotation (rad)",
			edited.rotation,
			0.01f,
			-DirectX::XM_PI,
			DirectX::XM_PI);
		changed |= rotationChanged;
		changed |= EditVector3("Scale", edited.scale, 0.01f, 0.05f, 10.0f);
		if (changed)
		{
			// 렌더러와 같은 유한·비특이·양수 행렬식 조건을 사용한다.
			mInvalidEdit = !BuildObjectTransformConstants(edited.GetWorld(), DirectX::XMMatrixIdentity()).has_value();
			if (!mInvalidEdit)
			{
				mObjects[index] = edited;
				if (rotationChanged)
				{
					mAutoRotate[index] = false;
				}
			}
		}

		ImGui::Checkbox("Auto rotate", &mAutoRotate[index]);
		ImGui::TextWrapped("Editing rotation pauses this cube; resume from edited angle.");
	}

	void SandboxApp::DrawLightingTools()
	{
		ImGui::SeparatorText("Directional light");
		float yaw = mLighting.GetYaw();
		float pitch = mLighting.GetPitch();
		float intensity = mLighting.GetIntensity();
		bool changed = ImGui::SliderFloat(
			"Yaw (rad)",
			&yaw,
			-DirectX::XM_PI,
			DirectX::XM_PI,
			"%.3f",
			ImGuiSliderFlags_AlwaysClamp);
		changed |= ImGui::SliderFloat(
			"Pitch (rad)",
			&pitch,
			-SandboxLighting::PITCH_LIMIT,
			SandboxLighting::PITCH_LIMIT,
			"%.3f",
			ImGuiSliderFlags_AlwaysClamp);
		changed |= ImGui::SliderFloat(
			"Intensity",
			&intensity,
			0.0f,
			SandboxLighting::MAX_INTENSITY,
			"%.3f",
			ImGuiSliderFlags_AlwaysClamp);
		if (changed)
		{
			mInvalidEdit = !mLighting.SetParameters(yaw, pitch, intensity);
		}

		if (ImGui::Button("Reset light"))
		{
			mLighting.Reset();
		}
	}

	bool SandboxApp::Render()
	{
		const Win32Window::State& state = mWindow.GetState();
		const DirectX::XMMATRIX view = mCamera.GetView();
		const DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(
			DirectX::XM_PIDIV4,
			static_cast<float>(state.width) / static_cast<float>(state.height),
			0.1f,
			100.0f);

		std::array<ObjectTransformConstants, 3> objectConstants;
		for (std::size_t index = 0; index < mObjects.size(); ++index)
		{
			const std::optional<ObjectTransformConstants> constantsResult = BuildObjectTransformConstants(
				mObjects[index].GetWorld(), view * projection);
			if (!constantsResult)
			{
				const wchar_t* message = L"Sandbox cube transform is non-finite, singular or reflected.";
				OutputDebugStringW(message);
				MessageBoxW(mWindow.GetHandle(), message, L"Sandbox transform error", MB_OK | MB_ICONERROR);
				return false;
			}

			objectConstants[index] = *constantsResult;
		}

		FrameLighting lighting;
		lighting.direction = mLighting.GetDirection();
		lighting.intensity = mLighting.GetIntensity();
		lighting.cameraPosition = mCamera.GetPosition();
		const D3D12Renderer::OverlayPass overlay = mGui.GetOverlayPass();
		return mRenderer.Render(objectConstants, lighting, &overlay);
	}
}
