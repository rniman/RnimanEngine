#pragma once

#include "Platform/Win32Window.h"
#include "Rendering/D3D12Renderer.h"

// DX12 백엔드가 D3D12 헤더를 포함하므로 Windows 선언을 먼저 준비한다.
#include <Windows.h>
#include <backends/imgui_impl_dx12.h>
#include <d3d12.h>
#include <imgui.h>
#include <wrl/client.h>

#include <array>

namespace rengine
{
	/// @brief Sandbox의 단일 ImGui 문맥·Win32/DX12 백엔드·전용 SRV 힙을 소유한다.
	/// @note window와 renderer는 이 객체보다 오래 유지한다. 콜백이 this를 보관하므로 복사·이동하지 않는다.
	class SandboxGui
	{
	public:
		SandboxGui(Win32Window& window, D3D12Renderer& renderer);
		~SandboxGui();
		SandboxGui(const SandboxGui&) = delete;
		SandboxGui& operator=(const SandboxGui&) = delete;
		SandboxGui(SandboxGui&&) = delete;
		SandboxGui& operator=(SandboxGui&&) = delete;

		/// @brief 창·렌더러 초기화 성공 후 UI 스레드에서 한 번 호출한다.
		/// @return 실패 위치를 보고하고 false. 부분 초기화도 소멸자가 정리한다.
		/// @note 실패 후 재시도하거나 프레임을 시작하지 않고 이 객체를 파괴한다.
		[[nodiscard]] bool Initialize();

		/// @brief 메시지 처리·리사이즈 후 호출하고 앱 입력 소비 전에 캡처 상태를 반영한다.
		/// @note Initialize 성공 후 제출할 프레임마다 한 번 호출한다. 이전 프레임의 Record가 끝나야 한다.
		void BeginFrame();

		/// @brief BeginFrame 뒤 패널 작성을 마치고 렌더러에 제출하기 전에 한 번 호출한다.
		void EndFrame();

		/// @brief EndFrame 이후 다음 BeginFrame 전 한 번 기록할 비소유 그리기 연결을 반환한다.
		/// @note 연결은 이 객체보다 오래 유지하지 않는다. Render는 연결과 userData를 보관하지 않는다.
		D3D12Renderer::OverlayPass GetOverlayPass();

	private:
		static LRESULT HandleMessage(void* userData, HWND window, UINT message, WPARAM wParam, LPARAM lParam);
		static bool Record(void* userData, ID3D12GraphicsCommandList* commands);
		static void AllocateDescriptor(
			ImGui_ImplDX12_InitInfo* info,
			D3D12_CPU_DESCRIPTOR_HANDLE* cpu,
			D3D12_GPU_DESCRIPTOR_HANDLE* gpu);
		static void FreeDescriptor(
			ImGui_ImplDX12_InitInfo* info,
			D3D12_CPU_DESCRIPTOR_HANDLE cpu,
			D3D12_GPU_DESCRIPTOR_HANDLE gpu);
		[[nodiscard]] bool CheckResult(HRESULT result, const wchar_t* operation) const;

		static constexpr UINT DESCRIPTOR_COUNT = 64;

		Win32Window& mWindow;
		D3D12Renderer& mRenderer;

		// 외부 API가 반환한 소유 문맥. 소멸자가 ImGui::DestroyContext로 해제한다.
		ImGuiContext* mContext = nullptr;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSrvHeap;
		std::array<bool, DESCRIPTOR_COUNT> mDescriptors{};
		UINT mDescriptorStride = 0;

		bool mPlatformInitialized = false;
		bool mRendererInitialized = false;
	};
}
