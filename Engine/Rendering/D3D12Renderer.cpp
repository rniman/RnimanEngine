#include "Rendering/D3D12Renderer.h"

#include "Rendering/RenderData.h"

#include "Assets/AssetLoader.h"
#include "Assets/ImageData.h"
#include "Assets/MeshData.h"

// Windows 선언은 DirectX 헤더보다 먼저 필요하다.
#include <Windows.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <filesystem>
#include <iterator>
#include <limits>
#include <span>
#include <string>
#include <type_traits>
#include <utility>

namespace rengine
{
	using Microsoft::WRL::ComPtr;

	namespace
	{
		// 정점 복사·입력 레이아웃이 같은 32바이트 position/UV/normal 배치를 사용한다.
		static_assert(std::is_standard_layout_v<MeshVertex> && std::is_trivially_copyable_v<MeshVertex>);
		static_assert(sizeof(MeshVertex) == 32 && offsetof(MeshVertex, uv) == 12 && offsetof(MeshVertex, normal) == 20);

		// CPU 구조체와 HLSL 레지스터의 실제 배치를 함께 검증한다.
		static_assert(std::is_trivially_copyable_v<ObjectTransformConstants> &&
			sizeof(ObjectTransformConstants) == 192);
		static_assert(offsetof(ObjectTransformConstants, world) == 64 &&
			offsetof(ObjectTransformConstants, normalTransform) == 128);
		static_assert(std::is_trivially_copyable_v<FrameLighting> && sizeof(FrameLighting) == 64);
		static_assert(offsetof(FrameLighting, intensity) == 12 && offsetof(FrameLighting, ambientStrength) == 28);
		static_assert(offsetof(FrameLighting, specularStrength) == 44 && offsetof(FrameLighting, shininess) == 48);

		bool IsValidLighting(const FrameLighting& lighting)
		{
			const float values[] = {
				lighting.direction.x, lighting.direction.y, lighting.direction.z, lighting.intensity,
				lighting.color.x, lighting.color.y, lighting.color.z, lighting.ambientStrength,
				lighting.cameraPosition.x, lighting.cameraPosition.y, lighting.cameraPosition.z,
				lighting.specularStrength, lighting.shininess
			};
			for (float value : values)
			{
				if (!std::isfinite(value))
				{
					return false;
				}
			}

			const float lengthSquared = DirectX::XMVectorGetX(
				DirectX::XMVector3LengthSq(DirectX::XMLoadFloat3(&lighting.direction)));
			return std::abs(lengthSquared - 1.0f) <= 0.001f &&
				lighting.intensity >= 0.0f && lighting.intensity <= 2.0f &&
				lighting.color.x >= 0.0f && lighting.color.x <= 1.0f &&
				lighting.color.y >= 0.0f && lighting.color.y <= 1.0f &&
				lighting.color.z >= 0.0f && lighting.color.z <= 1.0f &&
				lighting.ambientStrength >= 0.0f && lighting.ambientStrength <= 1.0f &&
				lighting.specularStrength >= 0.0f && lighting.specularStrength <= 1.0f &&
				lighting.shininess >= 1.0f && lighting.shininess <= 256.0f;
		}

		constexpr std::size_t CONSTANT_STRIDE = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
		constexpr DXGI_FORMAT BACK_BUFFER_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
		constexpr DXGI_FORMAT COLOR_VIEW_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		constexpr DXGI_FORMAT DEPTH_BUFFER_FORMAT = DXGI_FORMAT_D32_FLOAT;
		// 면별로 정점을 분리한다. UV는 좌상 (0, 0), 우하 (1, 1)이다.
		constexpr MeshVertex CUBE_VERTICES[] = {
			{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
			{ { -1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
			{ { 1.0f, 1.0f, -1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
			{ { 1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
			{ { 1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
			{ { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
			{ { -1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
			{ { -1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
			{ { -1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
			{ { -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
			{ { -1.0f, 1.0f, -1.0f }, { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
			{ { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
			{ { 1.0f, -1.0f, -1.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { 1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { 1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
			{ { -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ { 1.0f, 1.0f, -1.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
			{ { -1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
			{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
			{ { 1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
			{ { 1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } }
		};

		// 외부에서 볼 때 화면에서 시계 방향인 면을 앞면으로 사용한다.
		constexpr std::uint32_t CUBE_INDICES[] = {
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
		// 초기화 중 텍스처 업로드를 제출한 뒤 실패한 경로도 포함해 미완료 GPU 작업의 완료를 시도한다.
		// 대기 실패는 WaitForGpu가 보고하며, 완료를 보장하지 못해도 자원 정리를 계속한다.
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

	bool D3D12Renderer::Initialize(
		HWND window,
		UINT width,
		UINT height,
		const ImageData& texture,
		const MeshData* mesh,
		const std::array<float, 4>& baseColor)
	{
		const ModelSource source{ texture, mesh, baseColor };
		return Initialize(window, width, height, std::span<const ModelSource>(&source, 1));
	}

	bool D3D12Renderer::Initialize(HWND window, UINT width, UINT height, std::span<const ModelSource> models)
	{
		if (window == nullptr || width == 0 || height == 0 || mFactory ||
			models.empty() || models.size() > D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1)
		{
			return CheckResult(E_INVALIDARG, L"D3D12Renderer::Initialize");
		}

		for (const ModelSource& source : models)
		{
			if (static_cast<std::uint32_t>(source.wrapU) > 2 || static_cast<std::uint32_t>(source.wrapV) > 2)
			{
				return CheckResult(E_INVALIDARG, L"Initialize: unsupported texture address mode");
			}

			const ImageData& texture = source.texture;
			// 제한 이후의 RGBA8 크기 계산은 x64 size_t 범위에 들어간다.
			if (texture.width == 0 || texture.height == 0 ||
				texture.width > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION ||
				texture.height > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION ||
				texture.pixels.size() != static_cast<std::size_t>(texture.width) * texture.height * 4)
			{
				return CheckResult(E_INVALIDARG, L"Initialize: invalid RGBA8 texture dimensions or pixel count");
			}

			for (const float component : source.baseColor)
			{
				if (!std::isfinite(component))
				{
					return CheckResult(E_INVALIDARG, L"Initialize: non-finite material BaseColor");
				}
			}
		}

		if (!CreateDevice())
		{
			return false;
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

		if (!CreateMeshPipeline())
		{
			return false;
		}

		D3D12_DESCRIPTOR_HEAP_DESC descriptors{};
		descriptors.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptors.NumDescriptors = static_cast<UINT>(models.size());
		descriptors.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		if (!CheckResult(
			mDevice->CreateDescriptorHeap(&descriptors, IID_PPV_ARGS(&mSrvHeap)),
			L"CreateDescriptorHeap(model SRVs)"))
		{
			return false;
		}

		mSrvStride = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		mModels.resize(models.size());
		for (std::size_t modelIndex = 0; modelIndex < models.size(); ++modelIndex)
		{
			const ModelSource& source = models[modelIndex];
			GpuModel& model = mModels[modelIndex];
			model.baseColor = source.baseColor;
			model.wrapU = source.wrapU;
			model.wrapV = source.wrapV;
			const std::span<const MeshVertex> vertices = source.mesh ?
				std::span<const MeshVertex>(source.mesh->vertices) : CUBE_VERTICES;
			const std::span<const std::uint32_t> indices = source.mesh ?
				std::span<const std::uint32_t>(source.mesh->indices) : CUBE_INDICES;
			if (!CreateGeometry(vertices, indices, model))
			{
				return false;
			}

			if (!CreateTexture(source.texture, model, modelIndex))
			{
				return false;
			}
		}

		return true;
	}

	bool D3D12Renderer::CreateDevice()
	{
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

		return true;
	}

	bool D3D12Renderer::CreateMeshPipeline()
	{
		// 작업 디렉터리와 무관하게 실행 파일과 함께 배치된 CSO를 읽는다.
		const ExecutableDirectoryResult executableDirectoryResult = GetExecutableDirectory();
		if (!CheckResult(executableDirectoryResult.result, L"GetModuleFileNameW"))
		{
			return false;
		}

		const std::filesystem::path shaderDirectory = executableDirectoryResult.directory / L"Shaders";
		const std::filesystem::path vertexPath = shaderDirectory / L"CubeVS.cso";
		const std::filesystem::path pixelPath = shaderDirectory / L"CubePS.cso";
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
		D3D12_ROOT_PARAMETER parameters[4]{};
		parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		parameters[0].Descriptor.ShaderRegister = 0;
		parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		parameters[1].DescriptorTable.NumDescriptorRanges = 1;
		parameters[1].DescriptorTable.pDescriptorRanges = &textureRange;
		parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		parameters[2].Descriptor.ShaderRegister = 1;
		parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		parameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		// CubePS의 b2에는 선형 RGB 3개와 축별 Wrap 조합 번호 uint 1개를 전달한다. 출력 alpha는 항상 1이다.
		parameters[3].Constants.ShaderRegister = 2;
		parameters[3].Constants.Num32BitValues = 4;
		parameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		D3D12_STATIC_SAMPLER_DESC sampler{};
		// 단일 밉·선형 필터를 공유하고 U/V 주소 모드는 아래 조합별로 지정한다.
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.MaxAnisotropy = 1;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		// SM5.0에서 동적 Sampler 인덱싱에 의존하지 않도록 3x3 조합을 정적으로 등록한다.
		constexpr std::array<D3D12_TEXTURE_ADDRESS_MODE, 3> addressModes = {
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_MIRROR
		};
		std::array<D3D12_STATIC_SAMPLER_DESC, 9> samplers{};
		for (UINT u = 0; u < 3; ++u)
		{
			for (UINT v = 0; v < 3; ++v)
			{
				D3D12_STATIC_SAMPLER_DESC& item = samplers[u * 3 + v];
				item = sampler;
				item.ShaderRegister = u * 3 + v;
				item.AddressU = addressModes[u];
				item.AddressV = addressModes[v];
			}
		}
		D3D12_ROOT_SIGNATURE_DESC rootDescription{};
		rootDescription.NumParameters = static_cast<UINT>(std::size(parameters));
		rootDescription.pParameters = parameters;
		rootDescription.NumStaticSamplers = static_cast<UINT>(samplers.size());
		rootDescription.pStaticSamplers = samplers.data();
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
				0, offsetof(MeshVertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			},
			{
				"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
				0, offsetof(MeshVertex, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			},
			{
				"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,
				0, offsetof(MeshVertex, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
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

	bool D3D12Renderer::CreateTexture(const ImageData& image, GpuModel& model, std::size_t modelIndex)
	{
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
				IID_PPV_ARGS(&model.texture)),
			L"CreateCommittedResource(texture)"))
		{
			return false;
		}

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
		UINT64 uploadBytes = 0;
		mDevice->GetCopyableFootprints(&texture, 0, 1, 0, &footprint, nullptr, nullptr, &uploadBytes);
		if (!CreateUploadBuffer(uploadBytes, model.textureUpload))
		{
			return false;
		}

		void* mapped = nullptr;
		const D3D12_RANGE readRange{ 0, 0 };
		if (!CheckResult(model.textureUpload->Map(0, &readRange, &mapped), L"TextureUpload::Map"))
		{
			return false;
		}

		// WIC의 촘촘한 행을 D3D12의 256바이트 RowPitch에 맞춰 복사한다.
		const std::size_t rowBytes = static_cast<std::size_t>(image.width) * 4;
		for (UINT row = 0; row < image.height; ++row)
		{
			std::byte* const destination = static_cast<std::byte*>(mapped) + footprint.Offset +
				static_cast<std::size_t>(row) * footprint.Footprint.RowPitch;
			std::memcpy(
				destination,
				image.pixels.data() + row * rowBytes,
				rowBytes);
		}

		const D3D12_RANGE writtenRange{ 0, static_cast<SIZE_T>(uploadBytes) };
		model.textureUpload->Unmap(0, &writtenRange);

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
		destination.pResource = model.texture.Get();
		destination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		D3D12_TEXTURE_COPY_LOCATION source{};
		source.pResource = model.textureUpload.Get();
		source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		source.PlacedFootprint = footprint;
		mCommandList->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);

		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = model.texture.Get();
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

		model.textureUpload.Reset();
		D3D12_SHADER_RESOURCE_VIEW_DESC view{};
		view.Format = COLOR_VIEW_FORMAT;
		view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		view.Texture2D.MipLevels = 1;
		D3D12_CPU_DESCRIPTOR_HANDLE handle = mSrvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += modelIndex * mSrvStride;
		mDevice->CreateShaderResourceView(model.texture.Get(), &view, handle);
		return true;
	}

	bool D3D12Renderer::CreateGeometry(
		std::span<const MeshVertex> vertices,
		std::span<const std::uint32_t> indices,
		GpuModel& model)
	{
		// 작은 정적 데모 메시를 Upload Heap에서 직접 읽는다. 이후 수정 없이 렌더러가 소유한다.
		if (vertices.empty() || indices.empty() || indices.size() % 3 != 0 ||
			vertices.size() > UINT_MAX / sizeof(MeshVertex) || indices.size() > UINT_MAX / sizeof(std::uint32_t))
		{
			return CheckResult(E_INVALIDARG, L"CreateGeometry: invalid or oversized mesh");
		}

		for (const MeshVertex& vertex : vertices)
		{
			for (float value : vertex.position)
			{
				if (!std::isfinite(value))
				{
					return CheckResult(E_INVALIDARG, L"CreateGeometry: non-finite position");
				}
			}

			for (float value : vertex.uv)
			{
				if (!std::isfinite(value))
				{
					return CheckResult(E_INVALIDARG, L"CreateGeometry: non-finite UV");
				}
			}

			const double normalLength = static_cast<double>(vertex.normal[0]) * vertex.normal[0] +
				static_cast<double>(vertex.normal[1]) * vertex.normal[1] +
				static_cast<double>(vertex.normal[2]) * vertex.normal[2];
			if (!std::isfinite(normalLength) || std::abs(normalLength - 1.0) > 1e-3)
			{
				return CheckResult(E_INVALIDARG, L"CreateGeometry: normal must be a unit vector");
			}
		}

		for (std::uint32_t index : indices)
		{
			if (index >= vertices.size())
			{
				return CheckResult(E_INVALIDARG, L"CreateGeometry: index out of bounds");
			}
		}

		if (!CreateUploadBuffer(vertices.size_bytes(), model.vertexBuffer))
		{
			return false;
		}

		if (!CreateUploadBuffer(indices.size_bytes(), model.indexBuffer))
		{
			return false;
		}

		void* mapped = nullptr;
		const D3D12_RANGE readRange{ 0, 0 };
		if (!CheckResult(model.vertexBuffer->Map(0, &readRange, &mapped), L"VertexBuffer::Map"))
		{
			return false;
		}

		std::memcpy(mapped, vertices.data(), vertices.size_bytes());
		const D3D12_RANGE vertexWrittenRange{ 0, vertices.size_bytes() };
		model.vertexBuffer->Unmap(0, &vertexWrittenRange);
		model.vertexBufferView.BufferLocation = model.vertexBuffer->GetGPUVirtualAddress();
		model.vertexBufferView.SizeInBytes = static_cast<UINT>(vertices.size_bytes());
		model.vertexBufferView.StrideInBytes = sizeof(MeshVertex);

		if (!CheckResult(model.indexBuffer->Map(0, &readRange, &mapped), L"IndexBuffer::Map"))
		{
			return false;
		}

		std::memcpy(mapped, indices.data(), indices.size_bytes());
		const D3D12_RANGE indexWrittenRange{ 0, indices.size_bytes() };
		model.indexBuffer->Unmap(0, &indexWrittenRange);
		model.indexBufferView.BufferLocation = model.indexBuffer->GetGPUVirtualAddress();
		model.indexBufferView.SizeInBytes = static_cast<UINT>(indices.size_bytes());
		model.indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		model.indexCount = static_cast<UINT>(indices.size());
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

	D3D12Renderer::OverlayContext D3D12Renderer::GetOverlayContext() const
	{
		return { mDevice.Get(), mCommandQueue.Get(), COLOR_VIEW_FORMAT, BUFFER_COUNT };
	}

	bool D3D12Renderer::Render(
		std::span<const ObjectTransformConstants> objectConstants,
		const FrameLighting& lighting,
		const OverlayPass* overlay,
		std::span<const std::size_t> modelIndices)
	{
		return RenderInternal(objectConstants, lighting, true, overlay, modelIndices);
	}

	bool D3D12Renderer::UploadFrameConstants(
		FrameResources& frame,
		std::span<const ObjectTransformConstants> objectConstants,
		const FrameLighting& lighting)
	{
		// 슬롯의 첫 사용 시 필요한 크기로 생성한다. 생성·확장·복사는 슬롯의 Fence 완료 후에만 수행한다.
		const std::size_t requiredBytes = (objectConstants.size() + 1) * CONSTANT_STRIDE;
		if (!frame.constantBuffer || requiredBytes > frame.constantBuffer->GetDesc().Width)
		{
			if (!CreateUploadBuffer(requiredBytes, frame.constantBuffer))
			{
				return false;
			}
		}

		void* mapped = nullptr;
		const D3D12_RANGE readRange{ 0, 0 };
		if (!CheckResult(frame.constantBuffer->Map(0, &readRange, &mapped), L"ConstantBuffer::Map"))
		{
			return false;
		}

		// 첫 256바이트는 프레임 공통 b1, 뒤에는 오브젝트별 b0를 배치한다.
		std::memcpy(mapped, &lighting, sizeof(FrameLighting));
		for (std::size_t objectIndex = 0; objectIndex < objectConstants.size(); ++objectIndex)
		{
			std::memcpy(
				static_cast<std::byte*>(mapped) + (objectIndex + 1) * CONSTANT_STRIDE,
				&objectConstants[objectIndex],
				sizeof(ObjectTransformConstants));
		}

		const D3D12_RANGE writtenRange{ 0, requiredBytes };
		frame.constantBuffer->Unmap(0, &writtenRange);

		return true;
	}

	bool D3D12Renderer::RenderInternal(
		std::span<const ObjectTransformConstants> objectConstants,
		const FrameLighting& lighting,
		bool present,
		const OverlayPass* overlay,
		std::span<const std::size_t> modelIndices)
	{
		if (overlay != nullptr && overlay->record == nullptr)
		{
			return CheckResult(E_INVALIDARG, L"Render: missing overlay recorder");
		}

		if (!modelIndices.empty() && modelIndices.size() != objectConstants.size())
		{
			return CheckResult(E_INVALIDARG, L"Render: model index count differs from instance count");
		}

		for (const std::size_t modelIndex : modelIndices)
		{
			if (modelIndex >= mModels.size())
			{
				return CheckResult(E_INVALIDARG, L"Render: model index out of bounds");
			}
		}

		static_assert(sizeof(ObjectTransformConstants) <= CONSTANT_STRIDE && sizeof(FrameLighting) <= CONSTANT_STRIDE);
		if (!IsValidLighting(lighting))
		{
			return CheckResult(E_INVALIDARG, L"Render: invalid Phong lighting values");
		}

		if (objectConstants.size() > std::numeric_limits<std::size_t>::max() / CONSTANT_STRIDE - 1)
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

		if (!UploadFrameConstants(frame, objectConstants, lighting))
		{
			return false;
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

		const D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = mDsvHeap->GetCPUDescriptorHandleForHeapStart();
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
		mCommandList->SetGraphicsRootConstantBufferView(2, frame.constantBuffer->GetGPUVirtualAddress());
		mCommandList->RSSetViewports(1, &viewport);
		mCommandList->RSSetScissorRects(1, &scissor);
		mCommandList->OMSetRenderTargets(1, &renderTargetHandle, FALSE, &depthHandle);
		mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		for (std::size_t objectIndex = 0; objectIndex < objectConstants.size(); ++objectIndex)
		{
			const std::size_t modelIndex = modelIndices.empty() ? 0 : modelIndices[objectIndex];
			const GpuModel& model = mModels[modelIndex];
			D3D12_GPU_DESCRIPTOR_HANDLE texture = mSrvHeap->GetGPUDescriptorHandleForHeapStart();
			texture.ptr += modelIndex * mSrvStride;
			mCommandList->SetGraphicsRootDescriptorTable(1, texture);
			mCommandList->SetGraphicsRoot32BitConstants(3, 3, model.baseColor.data(), 0);
			const UINT samplerIndex = static_cast<UINT>(model.wrapU) * 3u + static_cast<UINT>(model.wrapV);
			mCommandList->SetGraphicsRoot32BitConstant(3, samplerIndex, 3);
			mCommandList->IASetVertexBuffers(0, 1, &model.vertexBufferView);
			mCommandList->IASetIndexBuffer(&model.indexBufferView);
			mCommandList->SetGraphicsRootConstantBufferView(
				0,
				frame.constantBuffer->GetGPUVirtualAddress() + (objectIndex + 1) * CONSTANT_STRIDE);
			mCommandList->DrawIndexedInstanced(model.indexCount, 1, 0, 0, 0);
		}

		// 마지막 패스는 렌더 상태를 바꿔도 된다. 포인터는 호출 중에만 대여한다.
		if (overlay != nullptr && !overlay->record(overlay->userData, mCommandList.Get()))
		{
			return false;
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
		const HRESULT presentResult = present ? mSwapChain->Present(1, 0) : S_OK;

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
}
