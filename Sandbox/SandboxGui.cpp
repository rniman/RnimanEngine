#include "SandboxGui.h"

#include "Platform/Win32Window.h"
#include "Rendering/D3D12Renderer.h"

// Win32·D3D12 선언은 백엔드 헤더보다 먼저 필요하다.
#include <Windows.h>
#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_win32.h>
#include <d3d12.h>
#include <imgui.h>

#include <cwchar>
#include <exception>

// 공식 헤더는 Windows 타입의 강제 include를 피하려고 이 선언을 비활성화한다.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

namespace rengine
{
	SandboxGui::SandboxGui(Win32Window& window, D3D12Renderer& renderer)
		: mWindow{ window }, mRenderer{ renderer }
	{
	}

	SandboxGui::~SandboxGui()
	{
		mWindow.SetMessageHandler(nullptr, nullptr);
		if (mContext == nullptr)
		{
			return;
		}

		// Run의 조기 반환·부분 초기화도 포함한다. UI 자원을 먼저 해제하면 GPU가 참조할 수 있다.
		// 실패는 WaitForGpu가 보고하고 정리를 계속한다. 이 경우 GPU 완료를 보장하지 않는다.
		(void)mRenderer.WaitForGpu();
		ImGui::SetCurrentContext(mContext);
		if (mRendererInitialized)
		{
			ImGui_ImplDX12_Shutdown();
		}

		if (mPlatformInitialized)
		{
			ImGui_ImplWin32_Shutdown();
		}

		ImGui::DestroyContext(mContext);
	}

	bool SandboxGui::CheckResult(HRESULT result, const wchar_t* operation) const
	{
		if (SUCCEEDED(result))
		{
			return true;
		}

		wchar_t message[512]{};
		swprintf_s(message, L"%ls failed. HRESULT: 0x%08lX\n", operation, static_cast<unsigned long>(result));
		OutputDebugStringW(message);
		MessageBoxW(mWindow.GetHandle(), message, L"Sandbox ImGui error", MB_OK | MB_ICONERROR);
		return false;
	}

	bool SandboxGui::Initialize()
	{
		if (mContext != nullptr)
		{
			return CheckResult(E_UNEXPECTED, L"SandboxGui::Initialize called twice");
		}

		const D3D12Renderer::OverlayContext gpu = mRenderer.GetOverlayContext();
		if (gpu.device == nullptr || gpu.commandQueue == nullptr || mWindow.GetHandle() == nullptr)
		{
			return CheckResult(E_UNEXPECTED, L"SandboxGui::Initialize before window/renderer");
		}

		IMGUI_CHECKVERSION();
		mContext = ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		// 레이아웃 파일·외부 폰트가 없어도 체크아웃과 작업 디렉터리에 관계없이 시작한다.
		io.IniFilename = nullptr;
		io.LogFilename = nullptr;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		ImGui::StyleColorsDark();

		D3D12_DESCRIPTOR_HEAP_DESC heap{};
		heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heap.NumDescriptors = DESCRIPTOR_COUNT;
		heap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		if (!CheckResult(
			gpu.device->CreateDescriptorHeap(&heap, IID_PPV_ARGS(&mSrvHeap)),
			L"CreateDescriptorHeap(ImGui)"))
		{
			return false;
		}

		mDescriptorStride = gpu.device->GetDescriptorHandleIncrementSize(heap.Type);
		mPlatformInitialized = ImGui_ImplWin32_Init(mWindow.GetHandle());
		if (!mPlatformInitialized)
		{
			return CheckResult(E_FAIL, L"ImGui_ImplWin32_Init");
		}

		ImGui_ImplDX12_InitInfo info;
		info.Device = gpu.device;
		info.CommandQueue = gpu.commandQueue;
		info.NumFramesInFlight = static_cast<int>(gpu.framesInFlight);
		// Swap Chain은 UNORM이지만 실제 장면 RTV는 sRGB다. PSO 형식도 RTV와 맞춘다.
		info.RTVFormat = gpu.renderTargetFormat;
		info.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		info.SrvDescriptorHeap = mSrvHeap.Get();
		info.UserData = this;
		info.SrvDescriptorAllocFn = AllocateDescriptor;
		info.SrvDescriptorFreeFn = FreeDescriptor;
		mRendererInitialized = ImGui_ImplDX12_Init(&info);
		if (!mRendererInitialized)
		{
			return CheckResult(E_FAIL, L"ImGui_ImplDX12_Init");
		}

		if (!ImGui_ImplDX12_CreateDeviceObjects())
		{
			return CheckResult(E_FAIL, L"ImGui_ImplDX12_CreateDeviceObjects");
		}

		mWindow.SetMessageHandler(HandleMessage, this);
		return true;
	}

	void SandboxGui::BeginFrame()
	{
		ImGui::SetCurrentContext(mContext);
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		const ImGuiIO& io = ImGui::GetIO();
		mWindow.SetInputCapture(io.WantCaptureMouse, io.WantCaptureKeyboard);
	}

	void SandboxGui::EndFrame()
	{
		ImGui::Render();
	}

	D3D12Renderer::OverlayPass SandboxGui::GetOverlayPass()
	{
		return { this, Record };
	}

	LRESULT SandboxGui::HandleMessage(void* userData, HWND window, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto* gui = static_cast<SandboxGui*>(userData);
		ImGui::SetCurrentContext(gui->mContext);
		return ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam);
	}

	bool SandboxGui::Record(void* userData, ID3D12GraphicsCommandList* commands)
	{
		auto* gui = static_cast<SandboxGui*>(userData);
		ID3D12DescriptorHeap* heaps[] = { gui->mSrvHeap.Get() };
		commands->SetDescriptorHeaps(1, heaps);
		// 렌더러의 슬롯 Fence 대기 후 정확히 한 번 기록한다. UI 버퍼도 두 프레임보다 앞서 재사용하지 않는다.
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commands);
		return gui->CheckResult(
			gui->mRenderer.GetOverlayContext().device->GetDeviceRemovedReason(),
			L"ImGui draw/device");
	}

	void SandboxGui::AllocateDescriptor(
		ImGui_ImplDX12_InitInfo* info,
		D3D12_CPU_DESCRIPTOR_HANDLE* cpu,
		D3D12_GPU_DESCRIPTOR_HANDLE* gpu)
	{
		auto* gui = static_cast<SandboxGui*>(info->UserData);
		for (UINT index = 0; index < DESCRIPTOR_COUNT; ++index)
		{
			if (!gui->mDescriptors[index])
			{
				gui->mDescriptors[index] = true;
				cpu->ptr = gui->mSrvHeap->GetCPUDescriptorHandleForHeapStart().ptr +
					static_cast<SIZE_T>(index) * gui->mDescriptorStride;
				gpu->ptr = gui->mSrvHeap->GetGPUDescriptorHandleForHeapStart().ptr +
					static_cast<UINT64>(index) * gui->mDescriptorStride;
				return;
			}
		}

		// 공급자 콜백에는 실패 반환이 없다. 잘못된 핸들로 계속 실행하지 않는다.
		(void)gui->CheckResult(E_OUTOFMEMORY, L"ImGui SRV heap exhausted");
		std::terminate();
	}

	void SandboxGui::FreeDescriptor(
		ImGui_ImplDX12_InitInfo* info,
		D3D12_CPU_DESCRIPTOR_HANDLE cpu,
		D3D12_GPU_DESCRIPTOR_HANDLE gpu)
	{
		auto* gui = static_cast<SandboxGui*>(info->UserData);
		const SIZE_T cpuOffset = cpu.ptr - gui->mSrvHeap->GetCPUDescriptorHandleForHeapStart().ptr;
		const UINT64 gpuOffset = gpu.ptr - gui->mSrvHeap->GetGPUDescriptorHandleForHeapStart().ptr;
		const SIZE_T index = cpuOffset / gui->mDescriptorStride;
		if (index >= DESCRIPTOR_COUNT || cpuOffset % gui->mDescriptorStride != 0 ||
			gpuOffset != cpuOffset || !gui->mDescriptors[index])
		{
			(void)gui->CheckResult(E_INVALIDARG, L"ImGui SRV descriptor release");
			std::terminate();
		}

		gui->mDescriptors[index] = false;
	}
}
