#include "pch.h"
#include "D3D12Renderer.h"

#include <Windows.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <cwchar>
#include <limits>

using Microsoft::WRL::ComPtr;

D3D12Renderer::~D3D12Renderer()
{
	// 부분 초기화에는 제출된 작업이 없다. 제출 이후 실패한 경로도 완료를 시도한다.
	if (mGpuWorkPending)
	{
		(void)WaitForGpu();
	}

	if (mFenceEvent != nullptr && !CloseHandle(mFenceEvent))
	{
		(void)CheckResult(HRESULT_FROM_WIN32(GetLastError()), L"CloseHandle(Fence)");
	}
}

bool D3D12Renderer::CheckResult(HRESULT result, const wchar_t* operation) const
{
	if (SUCCEEDED(result))
	{
		return true;
	}

	wchar_t message[768]{};
	HRESULT removedReason = mDevice ? mDevice->GetDeviceRemovedReason() : S_OK;
	swprintf_s(
		message,
		L"%ls failed. HRESULT: 0x%08lX\nDevice removed reason: 0x%08lX\n",
		operation,
		static_cast<unsigned long>(result),
		static_cast<unsigned long>(removedReason));
	OutputDebugStringW(message);
	MessageBoxW(nullptr, message, L"Sandbox D3D12 error", MB_OK | MB_ICONERROR);
	return false;
}

bool D3D12Renderer::Initialize(HWND window, UINT width, UINT height)
{
	if (window == nullptr || width == 0 || height == 0 || mFactory)
	{
		return CheckResult(E_INVALIDARG, L"D3D12Renderer::Initialize");
	}

	UINT factoryFlags = 0;
#if defined(_DEBUG)
	ComPtr<ID3D12Debug> debug;
	if (!CheckResult(
		D3D12GetDebugInterface(IID_PPV_ARGS(&debug)),
		L"D3D12GetDebugInterface (install Windows Graphics Tools)"))
	{
		return false;
	}

	debug->EnableDebugLayer();
	factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	if (!CheckResult(
		CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&mFactory)),
		L"CreateDXGIFactory2"))
	{
		return false;
	}

	for (UINT index = 0; ; ++index)
	{
		ComPtr<IDXGIAdapter1> adapter;
		HRESULT result = mFactory->EnumAdapterByGpuPreference(
			index,
			DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(&adapter));
		if (result == DXGI_ERROR_NOT_FOUND)
		{
			break;
		}

		if (!CheckResult(result, L"EnumAdapterByGpuPreference"))
		{
			return false;
		}

		DXGI_ADAPTER_DESC1 description{};
		if (!CheckResult(adapter->GetDesc1(&description), L"IDXGIAdapter1::GetDesc1"))
		{
			return false;
		}

		if ((description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
		{
			continue;
		}

		result = D3D12CreateDevice(
			adapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&mDevice));
		wchar_t adapterMessage[512]{};
		swprintf_s(
			adapterMessage,
			L"Adapter: %ls; D3D12CreateDevice: 0x%08lX\n",
			description.Description,
			static_cast<unsigned long>(result));
		OutputDebugStringW(adapterMessage);
		if (SUCCEEDED(result))
		{
			break;
		}

		// 지원하지 않는 후보는 건너뛰되 메모리 부족 등 실제 생성 실패는 숨기지 않는다.
		if (result != DXGI_ERROR_UNSUPPORTED && result != E_NOINTERFACE)
		{
			return CheckResult(result, L"D3D12CreateDevice");
		}
	}

	if (!mDevice)
	{
		return CheckResult(
			DXGI_ERROR_UNSUPPORTED,
			L"No hardware adapter supports D3D12 feature level 11_0 (WARP disabled)");
	}

	D3D12_COMMAND_QUEUE_DESC queueDescription{};
	queueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	if (!CheckResult(
		mDevice->CreateCommandQueue(&queueDescription, IID_PPV_ARGS(&mCommandQueue)),
		L"CreateCommandQueue"))
	{
		return false;
	}

	DXGI_SWAP_CHAIN_DESC1 swapDescription{};
	swapDescription.Width = width;
	swapDescription.Height = height;
	swapDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDescription.SampleDesc.Count = 1;
	swapDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDescription.BufferCount = BUFFER_COUNT;
	swapDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	ComPtr<IDXGISwapChain1> swapChain;
	if (!CheckResult(
		mFactory->CreateSwapChainForHwnd(
			mCommandQueue.Get(),
			window,
			&swapDescription,
			nullptr,
			nullptr,
			&swapChain),
		L"CreateSwapChainForHwnd"))
	{
		return false;
	}

	if (!CheckResult(swapChain.As(&mSwapChain), L"QueryInterface(IDXGISwapChain3)"))
	{
		return false;
	}

	if (!CheckResult(
		mFactory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER),
		L"MakeWindowAssociation"))
	{
		return false;
	}

	D3D12_DESCRIPTOR_HEAP_DESC heapDescription{};
	heapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDescription.NumDescriptors = BUFFER_COUNT;
	if (!CheckResult(
		mDevice->CreateDescriptorHeap(&heapDescription, IID_PPV_ARGS(&mRtvHeap)),
		L"CreateDescriptorHeap(RTV)"))
	{
		return false;
	}

	mRtvStride = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	if (!CreateBackBuffers())
	{
		return false;
	}

	if (!CheckResult(
		mDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&mCommandAllocators)),
		L"CreateCommandAllocator"))
	{
		return false;
	}

	if (!CheckResult(
		mDevice->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			mCommandAllocators.Get(),
			nullptr,
			IID_PPV_ARGS(&mCommandList)),
		L"CreateCommandList"))
	{
		return false;
	}

	if (!CheckResult(mCommandList->Close(), L"CommandList::Close(initial)"))
	{
		return false;
	}

	if (!CheckResult(
		mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)),
		L"CreateFence"))
	{
		return false;
	}

	mFenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
	if (mFenceEvent == nullptr)
	{
		return CheckResult(HRESULT_FROM_WIN32(GetLastError()), L"CreateEventW(Fence)");
	}

	mWidth = width;
	mHeight = height;

	return true;
}

bool D3D12Renderer::CreateBackBuffers()
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT index = 0; index < BUFFER_COUNT; ++index)
	{
		if (!CheckResult(
			mSwapChain->GetBuffer(index, IID_PPV_ARGS(&mBackBuffers[index])),
			L"SwapChain::GetBuffer"))
		{
			return false;
		}

		mDevice->CreateRenderTargetView(mBackBuffers[index].Get(), nullptr, handle);
		handle.ptr += mRtvStride;
	}

	return true;
}

bool D3D12Renderer::Render()
{
	if (!CheckResult(mCommandAllocators->Reset(), L"CommandAllocator::Reset"))
	{
		return false;
	}

	if (!CheckResult(mCommandList->Reset(mCommandAllocators.Get(), nullptr), L"CommandList::Reset"))
	{
		return false;
	}

	UINT index = mSwapChain->GetCurrentBackBufferIndex();
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = mBackBuffers[index].Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	mCommandList->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE handle = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += static_cast<SIZE_T>(index) * mRtvStride;
	constexpr float CLEAR_COLOR[] = { 0.08f, 0.16f, 0.28f, 1.0f };
	mCommandList->ClearRenderTargetView(handle, CLEAR_COLOR, 0, nullptr);

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	mCommandList->ResourceBarrier(1, &barrier);

	if (!CheckResult(mCommandList->Close(), L"CommandList::Close"))
	{
		return false;
	}

	ID3D12CommandList* lists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(1, lists);
	mGpuWorkPending = true;

	HRESULT presentResult = mSwapChain->Present(1, 0);

	// 초기 동기화 검증을 위해 매 프레임 대기한다. CPU·GPU 병렬 실행이 제한된다.
	// TODO: 화면 출력·리사이즈·종료 검증 후 프레임별 Allocator와 Fence 값을 도입하고,
	// 해당 프레임 자원을 재사용할 때만 대기하도록 전환한다. docs/development.md 참조.
	bool completed = WaitForGpu();
	if (!CheckResult(presentResult, L"SwapChain::Present"))
	{
		return false;
	}

	if (!completed)
	{
		return false;
	}

	if (presentResult == DXGI_STATUS_OCCLUDED)
	{
		// 가려진 창에서 Present가 즉시 반환하더라도 반복 실행을 제한한다.
		Sleep(50);
	}

	return true;
}

bool D3D12Renderer::WaitForGpu()
{
	if (!mGpuWorkPending)
	{
		return true;
	}

	const UINT64 target = ++mFenceValue;
	if (!CheckResult(mCommandQueue->Signal(mFence.Get(), target), L"CommandQueue::Signal"))
	{
		return false;
	}

	UINT64 completed = mFence->GetCompletedValue();
	if (completed < target)
	{
		if (!CheckResult(
			mFence->SetEventOnCompletion(target, mFenceEvent),
			L"Fence::SetEventOnCompletion"))
		{
			return false;
		}

		// 장치 제거 시에도 이벤트가 해제되므로 대기 후 Fence 값을 다시 검사한다.
		DWORD waitResult = WaitForSingleObject(mFenceEvent, INFINITE);
		if (waitResult != WAIT_OBJECT_0)
		{
			HRESULT waitError = waitResult == WAIT_FAILED ? HRESULT_FROM_WIN32(GetLastError()) : E_FAIL;
			return CheckResult(waitError, L"WaitForSingleObject(Fence)");
		}

		completed = mFence->GetCompletedValue();
	}

	if (completed == std::numeric_limits<UINT64>::max())
	{
		mGpuWorkPending = false;
		return CheckResult(DXGI_ERROR_DEVICE_REMOVED, L"Fence::GetCompletedValue");
	}

	if (completed < target)
	{
		return CheckResult(E_FAIL, L"Fence completion value");
	}

	mGpuWorkPending = false;

	return true;
}

bool D3D12Renderer::Resize(UINT width, UINT height)
{
	if (width == 0 || height == 0 || (width == mWidth && height == mHeight))
	{
		return true;
	}

	if (!WaitForGpu())
	{
		return false;
	}

	for (auto& buffer : mBackBuffers)
	{
		buffer.Reset();
	}

	if (!CheckResult(
		mSwapChain->ResizeBuffers(
			BUFFER_COUNT,
			width,
			height,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			0),
		L"SwapChain::ResizeBuffers"))
	{
		return false;
	}

	if (!CreateBackBuffers())
	{
		return false;
	}

	mWidth = width;
	mHeight = height;

	return true;
}
