#pragma once

// Windows 선언은 DirectX 헤더보다 먼저 필요하다.
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <array>

/// @brief UI 스레드에서 사용하는 삼각형 렌더러. 창은 앱이 소유한다.
/// @note Initialize는 한 번만 호출한다. 실패 후에는 재사용하지 않고 파괴한다.
class D3D12Renderer
{
public:
	D3D12Renderer() = default;
	~D3D12Renderer();

	D3D12Renderer(const D3D12Renderer&) = delete;
	D3D12Renderer& operator=(const D3D12Renderer&) = delete;

	/// @brief 렌더러보다 오래 유지되는 window와 양수 클라이언트 크기로 초기화한다.
	/// @return 실패 시 false. API와 HRESULT를 보고하며 부분 자원은 소멸 시 정리한다.
	[[nodiscard]] bool Initialize(HWND window, UINT width, UINT height);

	/// @brief 초기화 성공 후 호출한다. 사용할 프레임 자원이 실행 중일 때만 재사용을 기다린다.
	[[nodiscard]] bool Render();

	/// @brief 프레임 경계에서 호출한다. 0 크기는 무시하며 실패 후 렌더링을 중단해야 한다.
	[[nodiscard]] bool Resize(UINT width, UINT height);

	/// @brief 종료 전에 호출하여 GPU 완료를 확인한다. 소멸자도 미완료 작업을 정리한다.
	[[nodiscard]] bool WaitForGpu();

private:
	struct FrameResources
	{
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
		UINT64 fenceValue = 0;
	};

	[[nodiscard]] bool CreateBackBuffers();
	[[nodiscard]] bool CreateTrianglePipeline();
	[[nodiscard]] bool CreateTriangleVertices();
	[[nodiscard]] bool WaitForFence(UINT64 target);
	[[nodiscard]] bool CheckResult(HRESULT result, const wchar_t* operation) const;

	static constexpr UINT BUFFER_COUNT = 2;

	Microsoft::WRL::ComPtr<IDXGIFactory6> mFactory;
	Microsoft::WRL::ComPtr<ID3D12Device> mDevice;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;

	Microsoft::WRL::ComPtr<IDXGISwapChain3> mSwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, BUFFER_COUNT> mBackBuffers;

	// 백 버퍼 인덱스로 선택하며, 기록 메모리는 해당 제출의 Fence 완료까지 유지한다.
	std::array<FrameResources, BUFFER_COUNT> mFrames;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;
	Microsoft::WRL::ComPtr<ID3D12Resource> mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};

	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
	HANDLE mFenceEvent = nullptr;
	UINT64 mFenceValue = 0;

	UINT mRtvStride = 0;
	UINT mWidth = 0;
	UINT mHeight = 0;
	bool mGpuWorkPending = false;
};
