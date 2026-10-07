#include "pch.h"
#include "SandboxApp.h"

#include "D3D12Renderer.h"
#include "SandboxCamera.h"
#include "Transform.h"
#include "Win32Window.h"

#include <DirectXMath.h>
#include <Windows.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>

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

	const auto& state = mWindow.GetState();
	// 최소화 상태로 시작해도 GPU 자원은 양수 크기로 초기화한다.
	return mRenderer.Initialize(
		mWindow.GetHandle(),
		state.width == 0 ? 1280 : state.width,
		state.height == 0 ? 720 : state.height);
}

int SandboxApp::Run(int showCommand)
{
	if (!Initialize(showCommand))
	{
		return 1;
	}

	const auto& state = mWindow.GetState();
	const auto startTime = std::chrono::steady_clock::now();
	auto previousTime = startTime;

	while (true)
	{
		if (const auto exitCode = mWindow.PumpMessages())
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

		const auto input = mWindow.ConsumeInput();
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
	const auto& state = mWindow.GetState();
	const auto view = mCamera.GetView();
	const auto projection = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XM_PIDIV4,
		static_cast<float>(state.width) / static_cast<float>(state.height),
		0.1f,
		100.0f);

	std::array<DirectX::XMFLOAT4X4, 3> transforms;
	for (std::size_t index = 0; index < mObjects.size(); ++index)
	{
		DirectX::XMStoreFloat4x4(&transforms[index], mObjects[index].GetWorld() * view * projection);
	}

	return mRenderer.Render(transforms);
}
