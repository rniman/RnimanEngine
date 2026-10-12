#pragma once

#include "D3D12Renderer.h"
#include "RenderData.h"

// Windows 선언은 DirectX 헤더보다 먼저 필요하다.
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <span>
#include <vector>

// 테스트 전용 동기식 readback. flip-discard가 내용을 폐기하기 전, Present 없이 읽는다.
class RendererReadback
{
public:
	/// @brief renderer의 Present 전 픽셀을 읽는 테스트 어댑터를 생성한다.
	/// @param renderer 이 어댑터보다 오래 살아 있어야 하는 초기화된 렌더러.
	explicit RendererReadback(D3D12Renderer& renderer)
		: mRenderer{ renderer }
	{
	}

	~RendererReadback()
	{
		// 실패 경로에서도 로컬 복사 자원을 해제하기 전에 제출 완료를 기다린다.
		if (!mRenderer.WaitForGpu())
		{
			std::fputs("FAIL: readback cleanup GPU wait\n", stderr);
		}
	}

	RendererReadback(const RendererReadback&) = delete;
	RendererReadback& operator=(const RendererReadback&) = delete;

	/// @brief Present 전 RGBA8 픽셀을 행 패딩 없이 복사한다.
	/// @param objects CreateRenderObject로 만든 큐브 데이터. 호출 중에만 읽는다.
	/// @param lighting D3D12Renderer::Render와 같은 조건의 프레임 조명. 호출 중에만 읽는다.
	/// @param pixels 성공 시 픽셀로 교체한다. 실패 시에도 내용이나 크기가 바뀔 수 있다.
	/// @return 성공 시 true. 실패 시 false를 반환하며 테스트를 종료해야 한다.
	bool Capture(
		std::span<const RenderObject> objects,
		const FrameLighting& lighting,
		std::vector<std::uint32_t>& pixels)
	{
		const UINT index = mRenderer.mSwapChain->GetCurrentBackBufferIndex();
		if (!mRenderer.RenderInternal(objects, lighting, false))
		{
			return false;
		}

		if (!mRenderer.WaitForGpu())
		{
			return false;
		}

		auto* source = mRenderer.mBackBuffers[index].Get();
		const auto description = source->GetDesc();
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
		UINT64 bytes = 0;
		mRenderer.mDevice->GetCopyableFootprints(&description, 0, 1, 0, &footprint, nullptr, nullptr, &bytes);

		D3D12_HEAP_PROPERTIES heap{};
		heap.Type = D3D12_HEAP_TYPE_READBACK;
		D3D12_RESOURCE_DESC buffer{};
		buffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		buffer.Width = bytes;
		buffer.Height = 1;
		buffer.DepthOrArraySize = 1;
		buffer.MipLevels = 1;
		buffer.SampleDesc.Count = 1;
		buffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		if (!Check(
			mRenderer.mDevice->CreateCommittedResource(
				&heap,
				D3D12_HEAP_FLAG_NONE,
				&buffer,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&mReadback)),
			"Create readback"))
		{
			return false;
		}

		if (!mAllocator)
		{
			if (!Check(
				mRenderer.mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mAllocator)),
				"Create readback allocator"))
			{
				return false;
			}

			if (!Check(
				mRenderer.mDevice->CreateCommandList(
					0,
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					mAllocator.Get(),
					nullptr,
					IID_PPV_ARGS(&mCommands)),
				"Create readback list"))
			{
				return false;
			}
		}
		else
		{
			if (!Check(mAllocator->Reset(), "Reset readback allocator"))
			{
				return false;
			}

			if (!Check(mCommands->Reset(mAllocator.Get(), nullptr), "Reset readback list"))
			{
				return false;
			}
		}

		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = source;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
		mCommands->ResourceBarrier(1, &barrier);

		D3D12_TEXTURE_COPY_LOCATION destination{};
		destination.pResource = mReadback.Get();
		destination.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		destination.PlacedFootprint = footprint;
		D3D12_TEXTURE_COPY_LOCATION origin{};
		origin.pResource = source;
		origin.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		mCommands->CopyTextureRegion(&destination, 0, 0, 0, &origin, nullptr);

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		mCommands->ResourceBarrier(1, &barrier);

		if (!Check(mCommands->Close(), "Close readback list"))
		{
			return false;
		}

		ID3D12CommandList* lists[] = { mCommands.Get() };
		mRenderer.mCommandQueue->ExecuteCommandLists(1, lists);
		mRenderer.mGpuWorkPending = true;
		if (!mRenderer.WaitForGpu())
		{
			return false;
		}

		pixels.resize(static_cast<std::size_t>(description.Width) * description.Height);
		void* mapped = nullptr;
		const D3D12_RANGE range{ 0, static_cast<SIZE_T>(bytes) };
		if (!Check(mReadback->Map(0, &range, &mapped), "Map readback"))
		{
			return false;
		}

		for (UINT row = 0; row < description.Height; ++row)
		{
			std::memcpy(
				pixels.data() + row * description.Width,
				static_cast<const std::byte*>(mapped) + footprint.Offset + row * footprint.Footprint.RowPitch,
				static_cast<std::size_t>(description.Width) * sizeof(std::uint32_t));
		}

		const D3D12_RANGE written{ 0, 0 };
		mReadback->Unmap(0, &written);
		return true;
	}

private:
	static bool Check(HRESULT result, const char* operation)
	{
		if (FAILED(result))
		{
			std::printf("FAIL: %s HRESULT=0x%08lX\n", operation, result);
			return false;
		}

		return true;
	}

	D3D12Renderer& mRenderer;
	Microsoft::WRL::ComPtr<ID3D12Resource> mReadback;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommands;
};
