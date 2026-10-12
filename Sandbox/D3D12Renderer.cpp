#include "pch.h"
#include "D3D12Renderer.h"

#include "AssetLoader.h"
#include "ImageData.h"

// Windows 선언은 DirectX 헤더보다 먼저 필요하다.
#include <Windows.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <filesystem>
#include <iterator>
#include <limits>
#include <span>
#include <string>
#include <utility>

using Microsoft::WRL::ComPtr;

namespace
{
	struct Vertex
	{
		float position[3];
		float uv[2];
	};

	constexpr DXGI_FORMAT BACK_BUFFER_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
	constexpr DXGI_FORMAT COLOR_VIEW_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	constexpr DXGI_FORMAT DEPTH_BUFFER_FORMAT = DXGI_FORMAT_D32_FLOAT;
	// 면별로 정점을 분리한다. UV는 좌상 (0, 0), 우하 (1, 1)이다.
	constexpr Vertex CUBE_VERTICES[] = {
		{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 1.0f } },
		{ { -1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f } },
		{ { 1.0f, 1.0f, -1.0f }, { 1.0f, 0.0f } },
		{ { 1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f } },
		{ { 1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ { -1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
		{ { -1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f } },
		{ { -1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ { -1.0f, 1.0f, -1.0f }, { 1.0f, 0.0f } },
		{ { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f } },
		{ { 1.0f, -1.0f, -1.0f }, { 0.0f, 1.0f } },
		{ { 1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f } },
		{ { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
		{ { 1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f } },
		{ { -1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f } },
		{ { -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
		{ { 1.0f, 1.0f, -1.0f }, { 1.0f, 1.0f } },
		{ { -1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f } },
		{ { 1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f } },
		{ { 1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f } }
	};

	// 외부에서 볼 때 화면에서 시계 방향인 면을 앞면으로 사용한다.
	constexpr std::uint16_t CUBE_INDICES[] = {
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23
	};
}

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

bool D3D12Renderer::Initialize(HWND window, UINT width, UINT height, const ImageData& texture)
{
	if (window == nullptr || width == 0 || height == 0 || mFactory)
	{
		return CheckResult(E_INVALIDARG, L"D3D12Renderer::Initialize");
	}

	// 제한 이후의 RGBA8 크기 계산은 x64 size_t 범위에 들어간다.
	if (texture.width == 0 || texture.height == 0 ||
		texture.width > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION ||
		texture.height > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION ||
		texture.pixels.size() != static_cast<std::size_t>(texture.width) * texture.height * 4)
	{
		return CheckResult(E_INVALIDARG, L"Initialize: invalid RGBA8 texture dimensions or pixel count");
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
		return CheckResult(DXGI_ERROR_UNSUPPORTED, L"No hardware adapter supports D3D12 feature level 11_0 (WARP disabled)");
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
	swapDescription.Format = BACK_BUFFER_FORMAT;
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

	heapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heapDescription.NumDescriptors = 1;
	if (!CheckResult(
		mDevice->CreateDescriptorHeap(&heapDescription, IID_PPV_ARGS(&mDsvHeap)),
		L"CreateDescriptorHeap(DSV)"))
	{
		return false;
	}

	for (auto& frame : mFrames)
	{
		// 루트 CBV도 GPU 주소가 256바이트 경계에 있어야 한다.
		if (!CreateUploadBuffer(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, frame.constantBuffer))
		{
			return false;
		}

		if (!CheckResult(
			mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frame.commandAllocator)),
			L"CreateCommandAllocator"))
		{
			return false;
		}
	}

	if (!CheckResult(
		mDevice->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			mFrames[0].commandAllocator.Get(),
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

	if (!CreateDepthBuffer())
	{
		return false;
	}

	if (!CreateCubePipeline())
	{
		return false;
	}

	if (!CreateCubeGeometry())
	{
		return false;
	}

	return CreateTexture(texture);
}

bool D3D12Renderer::CreateCubePipeline()
{
	// 작업 디렉터리와 무관하게 실행 파일과 함께 배치된 CSO를 읽는다.
	const auto assetDirectory = GetExecutableDirectory();
	if (!CheckResult(assetDirectory.result, L"GetModuleFileNameW"))
	{
		return false;
	}

	const auto shaderDirectory = assetDirectory.directory / L"Shaders";
	const auto vertexPath = shaderDirectory / L"CubeVS.cso";
	const auto pixelPath = shaderDirectory / L"CubePS.cso";
	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;
	const std::wstring vertexOperation = L"D3DReadFileToBlob: " + vertexPath.wstring();
	if (!CheckResult(D3DReadFileToBlob(vertexPath.c_str(), &vertexShader), vertexOperation.c_str()))
	{
		return false;
	}

	const std::wstring pixelOperation = L"D3DReadFileToBlob: " + pixelPath.wstring();
	if (!CheckResult(D3DReadFileToBlob(pixelPath.c_str(), &pixelShader), pixelOperation.c_str()))
	{
		return false;
	}

	D3D12_DESCRIPTOR_RANGE textureRange{};
	textureRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	textureRange.NumDescriptors = 1;
	textureRange.BaseShaderRegister = 0;
	textureRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	D3D12_ROOT_PARAMETER parameters[2]{};
	parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	parameters[0].Descriptor.ShaderRegister = 0;
	parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameters[1].DescriptorTable.NumDescriptorRanges = 1;
	parameters[1].DescriptorTable.pDescriptorRanges = &textureRange;
	parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	D3D12_STATIC_SAMPLER_DESC sampler{};
	// 단일 밉과 선형 필터를 사용하며 면 끝에서 반대편 픽셀을 섞지 않는다.
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.MaxAnisotropy = 1;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	D3D12_ROOT_SIGNATURE_DESC rootDescription{};
	rootDescription.NumParameters = static_cast<UINT>(std::size(parameters));
	rootDescription.pParameters = parameters;
	rootDescription.NumStaticSamplers = 1;
	rootDescription.pStaticSamplers = &sampler;
	rootDescription.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	ComPtr<ID3DBlob> serializedRoot;
	ComPtr<ID3DBlob> errors;
	HRESULT result = D3D12SerializeRootSignature(
		&rootDescription,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&serializedRoot,
		&errors);
	if (errors)
	{
		OutputDebugStringA(static_cast<const char*>(errors->GetBufferPointer()));
	}

	if (!CheckResult(result, L"D3D12SerializeRootSignature"))
	{
		return false;
	}

	if (!CheckResult(
		mDevice->CreateRootSignature(
			0,
			serializedRoot->GetBufferPointer(),
			serializedRoot->GetBufferSize(),
			IID_PPV_ARGS(&mRootSignature)),
		L"CreateRootSignature"))
	{
		return false;
	}

	constexpr D3D12_INPUT_ELEMENT_DESC inputElements[] = {
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
			0, offsetof(Vertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
			0, offsetof(Vertex, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		}
	};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline{};
	pipeline.pRootSignature = mRootSignature.Get();
	pipeline.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
	pipeline.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
	pipeline.InputLayout = { inputElements, static_cast<UINT>(std::size(inputElements)) };
	pipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipeline.SampleMask = std::numeric_limits<UINT>::max();
	pipeline.SampleDesc.Count = 1;
	pipeline.NumRenderTargets = 1;
	pipeline.RTVFormats[0] = COLOR_VIEW_FORMAT;
	pipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	pipeline.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	pipeline.RasterizerState.DepthClipEnable = TRUE;
	pipeline.DSVFormat = DEPTH_BUFFER_FORMAT;
	pipeline.DepthStencilState.DepthEnable = TRUE;
	pipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	pipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	pipeline.DepthStencilState.StencilEnable = FALSE;

	auto& blend = pipeline.BlendState.RenderTarget[0];
	blend.SrcBlend = D3D12_BLEND_ONE;
	blend.DestBlend = D3D12_BLEND_ZERO;
	blend.BlendOp = D3D12_BLEND_OP_ADD;
	blend.SrcBlendAlpha = D3D12_BLEND_ONE;
	blend.DestBlendAlpha = D3D12_BLEND_ZERO;
	blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blend.LogicOp = D3D12_LOGIC_OP_NOOP;
	blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	return CheckResult(
		mDevice->CreateGraphicsPipelineState(&pipeline, IID_PPV_ARGS(&mPipelineState)),
		L"CreateGraphicsPipelineState(cube)");
}

bool D3D12Renderer::CreateUploadBuffer(UINT64 size, ComPtr<ID3D12Resource>& resource)
{
	D3D12_HEAP_PROPERTIES heap{};
	heap.Type = D3D12_HEAP_TYPE_UPLOAD;
	D3D12_RESOURCE_DESC buffer{};
	buffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	buffer.Width = size;
	buffer.Height = 1;
	buffer.DepthOrArraySize = 1;
	buffer.MipLevels = 1;
	buffer.SampleDesc.Count = 1;
	buffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	ComPtr<ID3D12Resource> created;
	if (!CheckResult(
		mDevice->CreateCommittedResource(
			&heap,
			D3D12_HEAP_FLAG_NONE,
			&buffer,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&created)),
		L"CreateCommittedResource(upload buffer)"))
	{
		return false;
	}

	resource = std::move(created);
	return true;
}

bool D3D12Renderer::CreateTexture(const ImageData& image)
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptors{};
	descriptors.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptors.NumDescriptors = 1;
	descriptors.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	if (!CheckResult(
		mDevice->CreateDescriptorHeap(&descriptors, IID_PPV_ARGS(&mSrvHeap)),
		L"CreateDescriptorHeap(texture SRV)"))
	{
		return false;
	}

	D3D12_HEAP_PROPERTIES heap{};
	heap.Type = D3D12_HEAP_TYPE_DEFAULT;
	D3D12_RESOURCE_DESC texture{};
	texture.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texture.Width = image.width;
	texture.Height = image.height;
	texture.DepthOrArraySize = 1;
	texture.MipLevels = 1;
	texture.Format = COLOR_VIEW_FORMAT;
	texture.SampleDesc.Count = 1;
	if (!CheckResult(
		mDevice->CreateCommittedResource(
			&heap,
			D3D12_HEAP_FLAG_NONE,
			&texture,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&mTexture)),
		L"CreateCommittedResource(texture)"))
	{
		return false;
	}

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
	UINT64 uploadBytes = 0;
	mDevice->GetCopyableFootprints(&texture, 0, 1, 0, &footprint, nullptr, nullptr, &uploadBytes);
	if (!CreateUploadBuffer(uploadBytes, mTextureUpload))
	{
		return false;
	}

	void* mapped = nullptr;
	const D3D12_RANGE readRange{ 0, 0 };
	if (!CheckResult(mTextureUpload->Map(0, &readRange, &mapped), L"TextureUpload::Map"))
	{
		return false;
	}

	// WIC의 촘촘한 행을 D3D12의 256바이트 RowPitch에 맞춰 복사한다.
	const std::size_t rowBytes = static_cast<std::size_t>(image.width) * 4;
	for (UINT row = 0; row < image.height; ++row)
	{
		const auto destination = static_cast<std::byte*>(mapped) + footprint.Offset +
			static_cast<std::size_t>(row) * footprint.Footprint.RowPitch;
		std::memcpy(
			destination,
			image.pixels.data() + row * rowBytes,
			rowBytes);
	}

	const D3D12_RANGE writtenRange{ 0, static_cast<SIZE_T>(uploadBytes) };
	mTextureUpload->Unmap(0, &writtenRange);

	if (!CheckResult(mFrames[0].commandAllocator->Reset(), L"CommandAllocator::Reset(texture upload)"))
	{
		return false;
	}

	if (!CheckResult(
		mCommandList->Reset(mFrames[0].commandAllocator.Get(), nullptr),
		L"CommandList::Reset(texture upload)"))
	{
		return false;
	}

	D3D12_TEXTURE_COPY_LOCATION destination{};
	destination.pResource = mTexture.Get();
	destination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	D3D12_TEXTURE_COPY_LOCATION source{};
	source.pResource = mTextureUpload.Get();
	source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	source.PlacedFootprint = footprint;
	mCommandList->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = mTexture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	mCommandList->ResourceBarrier(1, &barrier);
	if (!CheckResult(mCommandList->Close(), L"CommandList::Close(texture upload)"))
	{
		return false;
	}

	ID3D12CommandList* lists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(1, lists);
	mGpuWorkPending = true;
	// 초기화에서만 전체 완료를 기다린다. 실패하면 업로드 자원도 소멸자까지 유지한다.
	if (!WaitForGpu())
	{
		return false;
	}

	mTextureUpload.Reset();
	D3D12_SHADER_RESOURCE_VIEW_DESC view{};
	view.Format = COLOR_VIEW_FORMAT;
	view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	view.Texture2D.MipLevels = 1;
	mDevice->CreateShaderResourceView(mTexture.Get(), &view, mSrvHeap->GetCPUDescriptorHandleForHeapStart());
	return true;
}

bool D3D12Renderer::CreateCubeGeometry()
{
	// 작은 고정 메시의 정점·인덱스만 Upload Heap에서 직접 읽고 이후 수정하지 않는다.
	if (!CreateUploadBuffer(sizeof(CUBE_VERTICES), mVertexBuffer))
	{
		return false;
	}

	if (!CreateUploadBuffer(sizeof(CUBE_INDICES), mIndexBuffer))
	{
		return false;
	}

	void* mapped = nullptr;
	const D3D12_RANGE readRange{ 0, 0 };
	if (!CheckResult(mVertexBuffer->Map(0, &readRange, &mapped), L"VertexBuffer::Map"))
	{
		return false;
	}

	std::memcpy(mapped, CUBE_VERTICES, sizeof(CUBE_VERTICES));
	const D3D12_RANGE vertexWrittenRange{ 0, sizeof(CUBE_VERTICES) };
	mVertexBuffer->Unmap(0, &vertexWrittenRange);
	mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
	mVertexBufferView.SizeInBytes = sizeof(CUBE_VERTICES);
	mVertexBufferView.StrideInBytes = sizeof(Vertex);

	if (!CheckResult(mIndexBuffer->Map(0, &readRange, &mapped), L"IndexBuffer::Map"))
	{
		return false;
	}

	std::memcpy(mapped, CUBE_INDICES, sizeof(CUBE_INDICES));
	const D3D12_RANGE indexWrittenRange{ 0, sizeof(CUBE_INDICES) };
	mIndexBuffer->Unmap(0, &indexWrittenRange);
	mIndexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
	mIndexBufferView.SizeInBytes = sizeof(CUBE_INDICES);
	mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
	return true;
}

bool D3D12Renderer::CreateDepthBuffer()
{
	D3D12_HEAP_PROPERTIES heap{};
	heap.Type = D3D12_HEAP_TYPE_DEFAULT;
	D3D12_RESOURCE_DESC depth{};
	depth.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depth.Width = mWidth;
	depth.Height = mHeight;
	depth.DepthOrArraySize = 1;
	depth.MipLevels = 1;
	depth.Format = DEPTH_BUFFER_FORMAT;
	depth.SampleDesc.Count = 1;
	depth.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	D3D12_CLEAR_VALUE clear{};
	clear.Format = DEPTH_BUFFER_FORMAT;
	clear.DepthStencil.Depth = 1.0f;
	if (!CheckResult(
		mDevice->CreateCommittedResource(
			&heap,
			D3D12_HEAP_FLAG_NONE,
			&depth,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clear,
			IID_PPV_ARGS(&mDepthBuffer)),
		L"CreateCommittedResource(depth buffer)"))
	{
		return false;
	}

	mDevice->CreateDepthStencilView(mDepthBuffer.Get(), nullptr, mDsvHeap->GetCPUDescriptorHandleForHeapStart());
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

		D3D12_RENDER_TARGET_VIEW_DESC view{};
		view.Format = COLOR_VIEW_FORMAT;
		view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		mDevice->CreateRenderTargetView(mBackBuffers[index].Get(), &view, handle);
		handle.ptr += mRtvStride;
	}

	return true;
}

bool D3D12Renderer::Render(std::span<const DirectX::XMFLOAT4X4> transforms)
{
	return RenderInternal(transforms, true);
}

bool D3D12Renderer::RenderInternal(std::span<const DirectX::XMFLOAT4X4> transforms, bool present)
{
	constexpr std::size_t constantStride = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
	static_assert(sizeof(DirectX::XMFLOAT4X4) <= constantStride);
	if (transforms.size() > std::numeric_limits<std::size_t>::max() / constantStride)
	{
		return CheckResult(E_INVALIDARG, L"Render: constant buffer size overflow");
	}

	const UINT index = mSwapChain->GetCurrentBackBufferIndex();
	auto& frame = mFrames[index];

	// 같은 백 버퍼를 다시 사용할 때만 이전 기록 메모리의 GPU 사용 완료를 확인한다.
	if (!WaitForFence(frame.fenceValue))
	{
		return false;
	}

	// 슬롯의 Fence 완료 후에만 확장·복사한다. 다른 슬롯의 실행 중인 상수는 유지한다.
	const std::size_t requiredBytes = transforms.size() * constantStride;
	if (requiredBytes > frame.constantBuffer->GetDesc().Width)
	{
		if (!CreateUploadBuffer(requiredBytes, frame.constantBuffer))
		{
			return false;
		}
	}

	if (!transforms.empty())
	{
		void* mapped = nullptr;
		const D3D12_RANGE readRange{ 0, 0 };
		if (!CheckResult(frame.constantBuffer->Map(0, &readRange, &mapped), L"ConstantBuffer::Map"))
		{
			return false;
		}

		for (std::size_t objectIndex = 0; objectIndex < transforms.size(); ++objectIndex)
		{
			std::memcpy(
				static_cast<std::byte*>(mapped) + objectIndex * constantStride,
				&transforms[objectIndex],
				sizeof(DirectX::XMFLOAT4X4));
		}

		const D3D12_RANGE writtenRange{ 0, requiredBytes };
		frame.constantBuffer->Unmap(0, &writtenRange);
	}

	if (!CheckResult(frame.commandAllocator->Reset(), L"CommandAllocator::Reset"))
	{
		return false;
	}

	// Command List는 제출 후 재기록할 수 있지만 Allocator의 메모리는 완료 전 재사용할 수 없다.
	if (!CheckResult(mCommandList->Reset(frame.commandAllocator.Get(), mPipelineState.Get()), L"CommandList::Reset"))
	{
		return false;
	}

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = mBackBuffers[index].Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	mCommandList->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
	renderTargetHandle.ptr += static_cast<SIZE_T>(index) * mRtvStride;
	constexpr float clearColor[] = { 0.08f, 0.16f, 0.28f, 1.0f };
	mCommandList->ClearRenderTargetView(renderTargetHandle, clearColor, 0, nullptr);

	const auto depthHandle = mDsvHeap->GetCPUDescriptorHandleForHeapStart();
	mCommandList->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// 전체 창을 사용하며 종횡비 보정은 앱의 Projection 행렬이 담당한다.
	const D3D12_VIEWPORT viewport{
		0.0f, 0.0f,
		static_cast<float>(mWidth), static_cast<float>(mHeight),
		0.0f, 1.0f
	};
	const D3D12_RECT scissor{ 0, 0, static_cast<LONG>(mWidth), static_cast<LONG>(mHeight) };
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
	ID3D12DescriptorHeap* heaps[] = { mSrvHeap.Get() };
	mCommandList->SetDescriptorHeaps(1, heaps);
	mCommandList->SetGraphicsRootDescriptorTable(1, mSrvHeap->GetGPUDescriptorHandleForHeapStart());
	mCommandList->RSSetViewports(1, &viewport);
	mCommandList->RSSetScissorRects(1, &scissor);
	mCommandList->OMSetRenderTargets(1, &renderTargetHandle, FALSE, &depthHandle);
	mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mCommandList->IASetVertexBuffers(0, 1, &mVertexBufferView);
	mCommandList->IASetIndexBuffer(&mIndexBufferView);
	for (std::size_t objectIndex = 0; objectIndex < transforms.size(); ++objectIndex)
	{
		mCommandList->SetGraphicsRootConstantBufferView(
			0,
			frame.constantBuffer->GetGPUVirtualAddress() + objectIndex * constantStride);
		mCommandList->DrawIndexedInstanced(static_cast<UINT>(std::size(CUBE_INDICES)), 1, 0, 0, 0);
	}

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

	// 픽셀 테스트는 flip-discard의 Present 이후 내용에 의존하지 않고 같은 draw 경로를 읽는다.
	HRESULT presentResult = present ? mSwapChain->Present(1, 0) : S_OK;

	// Present 이후에도 제출한 프레임의 인덱스에 기록한다. 실패 시 소멸자가 전체 대기를 시도한다.
	const UINT64 target = ++mFenceValue;
	if (!CheckResult(mCommandQueue->Signal(mFence.Get(), target), L"CommandQueue::Signal(frame)"))
	{
		return false;
	}

	frame.fenceValue = target;
	if (!CheckResult(presentResult, L"SwapChain::Present"))
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

	if (!WaitForFence(target))
	{
		return false;
	}

	mGpuWorkPending = false;
	return true;
}

bool D3D12Renderer::WaitForFence(UINT64 target)
{
	UINT64 completed = mFence->GetCompletedValue();
	if (completed < target)
	{
		if (!CheckResult(mFence->SetEventOnCompletion(target, mFenceEvent), L"Fence::SetEventOnCompletion"))
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

	mDepthBuffer.Reset();
	for (auto& buffer : mBackBuffers)
	{
		buffer.Reset();
	}

	if (!CheckResult(
		mSwapChain->ResizeBuffers(
			BUFFER_COUNT,
			width,
			height,
			BACK_BUFFER_FORMAT,
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

	return CreateDepthBuffer();
}
