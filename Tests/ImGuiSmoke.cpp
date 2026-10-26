#include "RendererReadback.h"

#include "Assets/ImageData.h"
#include "Platform/Win32Window.h"
#include "Rendering/D3D12Renderer.h"
#include "Rendering/RenderData.h"
#include "SandboxCamera.h"
#include "SandboxGui.h"
#include "SandboxLighting.h"

// Windows 선언은 DirectX 헤더보다 먼저 필요하다.
#include <Windows.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <imgui.h>
#include <wrl/client.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <vector>

namespace rengine
{
	namespace
	{
		bool CheckHresult(HRESULT result, const char* operation)
		{
			if (FAILED(result))
			{
				std::printf("FAIL: %s HRESULT=0x%08lX\n", operation, static_cast<unsigned long>(result));
				return false;
			}

			return true;
		}

		bool CheckMouseCapture(Win32Window& window)
		{
			struct MouseButton
			{
				UINT downMessage;
				UINT upMessage;
				WORD flag;
				WORD xButton;
				const char* name;
			};
			constexpr MouseButton otherButtons[] = {
				{ WM_LBUTTONDOWN, WM_LBUTTONUP, MK_LBUTTON, 0, "left" },
				{ WM_MBUTTONDOWN, WM_MBUTTONUP, MK_MBUTTON, 0, "middle" },
				{ WM_XBUTTONDOWN, WM_XBUTTONUP, MK_XBUTTON1, XBUTTON1, "X1" },
				{ WM_XBUTTONDOWN, WM_XBUTTONUP, MK_XBUTTON2, XBUTTON2, "X2" }
			};
			const HWND handle = window.GetHandle();
			window.SetInputCapture(false, false);
			SendMessageW(handle, WM_SETFOCUS, 0, 0);
			for (const auto& button : otherButtons)
			{
				// 실제 백엔드가 획득한 캡처는 다른 버튼이 눌려 있으면 유지해야 한다.
				SendMessageW(handle, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(600, 500));
				SendMessageW(
					handle,
					button.downMessage,
					MAKEWPARAM(MK_RBUTTON | button.flag, button.xButton),
					MAKELPARAM(600, 500));
				const bool acquired = GetCapture() == handle;
				SendMessageW(handle, WM_RBUTTONUP, button.flag, MAKELPARAM(600, 500));
				const bool kept = GetCapture() == handle;
				SendMessageW(handle, WM_MOUSEMOVE, button.flag, MAKELPARAM(620, 520));
				const Win32Window::InputState afterRightUp = window.ConsumeInput();
				SendMessageW(
					handle,
					button.upMessage,
					MAKEWPARAM(0, button.xButton),
					MAKELPARAM(620, 520));
				const bool released = GetCapture() != handle;
				// 합성 입력을 뒤의 GPU 픽셀 검사에 넘기지 않는다.
				ImGui::GetIO().ClearEventsQueue();
				ImGui::GetIO().ClearInputMouse();
				if (!acquired || !kept || !released ||
					afterRightUp.mouseDeltaX != 0.0f || afterRightUp.mouseDeltaY != 0.0f)
				{
					std::printf(
						"FAIL: right + %s capture acquired=%d kept=%d released=%d\n",
						button.name,
						acquired,
						kept,
						released);
					return false;
				}
			}

			SendMessageW(handle, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(600, 500));
			const bool acquired = GetCapture() == handle;
			SendMessageW(handle, WM_RBUTTONUP, 0, MAKELPARAM(600, 500));
			const bool released = GetCapture() != handle;
			ImGui::GetIO().ClearEventsQueue();
			ImGui::GetIO().ClearInputMouse();
			if (!acquired || !released)
			{
				std::puts("FAIL: right-only ImGui mouse capture");
				return false;
			}

			std::puts("ImGui mouse: right-only and right + left/middle/X1/X2 capture ownership passed");
			return true;
		}
	}
}

int main()
{
	rengine::SandboxCamera camera;
	rengine::SandboxLighting light;
	const float invalid = std::numeric_limits<float>::quiet_NaN();
	const bool validPositionAccepted = camera.SetPosition({ 1.0f, 2.0f, 3.0f });
	const bool invalidPositionAccepted = camera.SetPosition({ invalid, 0.0f, 0.0f });
	const bool validLightAccepted = light.SetParameters(0.4f, 10.0f, 10.0f);
	const bool invalidLightAccepted = light.SetParameters(invalid, 0.0f, 0.0f);
	if (!validPositionAccepted || invalidPositionAccepted || camera.GetPosition().x != 1.0f ||
		!validLightAccepted || invalidLightAccepted ||
		light.GetIntensity() != 2.0f || light.GetPitch() >= DirectX::XM_PIDIV2)
	{
		std::puts("FAIL: UI camera/light edit bounds or non-finite rejection");
		return 1;
	}

	Microsoft::WRL::ComPtr<ID3D12Debug> debug;
	if (!rengine::CheckHresult(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)), "D3D12GetDebugInterface"))
	{
		std::puts("FAIL: ImGuiSmoke requires Graphics Tools");
		return 1;
	}

	debug->EnableDebugLayer();
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> messages;
	{
		rengine::Win32Window window{ GetModuleHandleW(nullptr) };
		rengine::D3D12Renderer renderer;
		rengine::ImageData texture;
		texture.width = texture.height = 1;
		texture.pixels = { 255, 255, 255, 255 };
		if (!window.Initialize(SW_HIDE))
		{
			return 1;
		}

		if (!renderer.Initialize(window.GetHandle(), 1280, 720, texture))
		{
			return 1;
		}

		if (!rengine::CheckHresult(
			renderer.GetOverlayContext().device->QueryInterface(IID_PPV_ARGS(&messages)),
			"QueryInterface(ID3D12InfoQueue)"))
		{
			return 1;
		}

		rengine::RendererReadback readback{ renderer };
		rengine::SandboxGui gui{ window, renderer };
		if (!gui.Initialize())
		{
			return 1;
		}

		if (!rengine::CheckMouseCapture(window))
		{
			return 1;
		}

		std::vector<std::uint32_t> pixels;
		rengine::FrameLighting lighting;
		if (!readback.Capture({}, lighting, pixels))
		{
			return 1;
		}

		const std::uint32_t clearPixel = pixels.front();
		const UINT widths[] = { 1280, 640, 960, 640, 1280 };
		for (UINT width : widths)
		{
			RECT bounds{ 0, 0, static_cast<LONG>(width), 720 };
			if (!AdjustWindowRect(&bounds, WS_OVERLAPPEDWINDOW, FALSE))
			{
				std::printf("FAIL: AdjustWindowRect error=%lu\n", GetLastError());
				return 1;
			}

			if (!SetWindowPos(
				window.GetHandle(),
				nullptr,
				0,
				0,
				bounds.right - bounds.left,
				bounds.bottom - bounds.top,
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE))
			{
				std::printf("FAIL: SetWindowPos error=%lu\n", GetLastError());
				return 1;
			}

			if (!renderer.Resize(width, 720))
			{
				return 1;
			}

			SendMessageW(window.GetHandle(), WM_SIZE, SIZE_RESTORED, MAKELPARAM(width, 720));
			for (int frame = 0; frame < 12; ++frame)
			{
				gui.BeginFrame();
				ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);
				ImGui::SetNextWindowSize(ImVec2(320.0f, 150.0f), ImGuiCond_Always);
				ImGui::Begin("GPU UI check");
				ImGui::Text("Frame %d, width %u", frame, width);
				ImGui::Button("Font atlas + widgets");
				ImGui::End();
				// 폰트 아틀라스 갱신·크기 변경도 공급자 백엔드의 텍스처 경로를 통과시킨다.
				if (frame == 6)
				{
					ImGui::GetStyle().FontScaleMain = 1.4f;
				}

				gui.EndFrame();
				const rengine::D3D12Renderer::OverlayPass overlay = gui.GetOverlayPass();
				if (frame == 11)
				{
					if (!readback.Capture({}, lighting, pixels, &overlay))
					{
						return 1;
					}

					const std::ptrdiff_t changed = std::count_if(
						pixels.begin(),
						pixels.end(),
						[clearPixel](std::uint32_t pixel) { return pixel != clearPixel; });
					if (changed < 1000)
					{
						std::printf("FAIL: UI pixels missing: %td\n", changed);
						return 1;
					}

					std::printf("UI readback width=%u changedPixels=%td\n", width, changed);
				}
				else if (!renderer.Render({}, lighting, &overlay))
				{
					return 1;
				}
			}

			SendMessageW(window.GetHandle(), WM_SIZE, SIZE_MINIMIZED, 0);
			SendMessageW(window.GetHandle(), WM_KILLFOCUS, 0, 0);
			SendMessageW(window.GetHandle(), WM_SIZE, SIZE_RESTORED, MAKELPARAM(width, 720));
		}

		gui.BeginFrame();
		ImGui::ShowDemoWindow();
		gui.EndFrame();
		const rengine::D3D12Renderer::OverlayPass finalOverlay = gui.GetOverlayPass();
		if (!renderer.Render({}, lighting, &finalOverlay))
		{
			return 1;
		}

		// 마지막 제출 후 flush 없이 gui를 먼저 파괴해 UI 자원 해제 전 대기를 검사한다.
	}

	UINT failures = 0;
	for (UINT64 index = 0; index < messages->GetNumStoredMessages(); ++index)
	{
		SIZE_T bytes = 0;
		if (!rengine::CheckHresult(messages->GetMessage(index, nullptr, &bytes), "InfoQueue::GetMessage(size)"))
		{
			return 1;
		}

		std::vector<unsigned char> storage(bytes);
		auto* message = reinterpret_cast<D3D12_MESSAGE*>(storage.data());
		if (!rengine::CheckHresult(messages->GetMessage(index, message, &bytes), "InfoQueue::GetMessage(data)"))
		{
			return 1;
		}

		if (message->Severity <= D3D12_MESSAGE_SEVERITY_WARNING)
		{
			std::printf("D3D12: %s\n", message->pDescription);
			++failures;
		}
	}

	std::printf(
		"ImGui: 60 frames, 5 resizes/readbacks, focus/minimize messages, shutdown: warnings/errors=%u\n",
		failures);
	return failures == 0 && messages->GetNumMessagesDiscardedByMessageCountLimit() == 0 ? 0 : 1;
}
