#include "pch.h"
#include "SandboxApp.h"

#include "SandboxCamera.h"
#include "SandboxLighting.h"

#include "Assets/AssetLoader.h"
#include "Math/Transform.h"
#include "Platform/Win32Window.h"
#include "Rendering/D3D12Renderer.h"
#include "Rendering/RenderData.h"

// Windows 선언은 DirectX 헤더보다 먼저 필요하다.
#include <Windows.h>
#include <DirectXMath.h>

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
	SandboxApp::SandboxApp(HINSTANCE instance)
		: mWindow{ instance }
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
		return mRenderer.Initialize(
			mWindow.GetHandle(),
			state.width == 0 ? 1280 : state.width,
			state.height == 0 ? 720 : state.height,
			loaded.image);
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

			const Win32Window::InputState input = mWindow.ConsumeInput();
			const auto now = std::chrono::steady_clock::now();
			// 모달 창 조작·최소화 이후 경과 시간은 이동에 누적하지 않는다.
			const float deltaSeconds = input.resetFrameTime ? 0.0f :
				std::min(std::chrono::duration<float>(now - previousTime).count(), 0.1f);
			previousTime = now;

			const double elapsedSeconds = std::chrono::duration<double>(now - startTime).count();
			Update(input, deltaSeconds, elapsedSeconds);
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

		// 큐브 회전은 입력 중단과 관계없이 시작 이후 경과 시간을 사용한다.
		const float angle = static_cast<float>(std::fmod(elapsedSeconds * 0.7, DirectX::XM_2PI));
		mObjects[0].rotation.y = angle;
		mObjects[1].rotation.x = static_cast<float>(std::fmod(elapsedSeconds * 0.35, DirectX::XM_2PI));
		mObjects[2].rotation.z = -angle;
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
		return mRenderer.Render(objectConstants, lighting);
	}
}
