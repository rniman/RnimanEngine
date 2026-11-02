#pragma once

#include "Rendering/RenderData.h"

#include "Assets/ImageData.h"
#include "Assets/MeshData.h"

// Windows 선언은 DirectX 헤더보다 먼저 필요하다.
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace rengine
{
	/// @brief UI 스레드에서 단일 재질 정적 모델들을 그리는 Phong 렌더러. 창은 앱이 소유한다.
	/// @note Initialize는 한 번만 호출한다. 실패 후에는 재사용하지 않고 파괴한다.
	class D3D12Renderer
	{
	public:
		/// @brief 초기화 호출 동안만 빌리는 CPU 모델 입력. texture와 mesh의 참조를 저장하지 않는다.
		struct ModelSource
		{
			const ImageData& texture;
			// nullptr는 회귀 기준 큐브를 뜻한다. BaseColor는 유한한 선형 RGBA이며 출력 alpha는 1이다.
			const MeshData* mesh = nullptr;
			std::array<float, 4> baseColor{ 1, 1, 1, 1 };
			TextureWrapMode wrapU = TextureWrapMode::ClampToEdge;
			TextureWrapMode wrapV = TextureWrapMode::ClampToEdge;
		};

		/// @brief 개발 UI 어댑터만 사용하는 비소유 GPU 문맥. 장면 데이터에는 전달하지 않는다.
		/// @note GetOverlayContext는 Initialize 성공 후 사용한다. 포인터의 소유자는 렌더러다.
		struct OverlayContext
		{
			ID3D12Device* device;
			ID3D12CommandQueue* commandQueue;
			DXGI_FORMAT renderTargetFormat;
			UINT framesInFlight;
		};

		/// @brief 현재 Render 호출 중에만 대여하는 마지막 그리기 패스.
		struct OverlayPass
		{
			/// @brief record에 전달할 비소유 문맥. Render가 끝날 때까지 유효해야 한다.
			/// @note nullptr 허용 여부는 record가 정한다.
			void* userData;
			/// @brief null이 아닌 기록 함수. 호출 중에만 Command List를 사용하고 보관하지 않는다.
			/// @note 기존 RTV·DSV에 기록하며 자체 자원의 Fence·해제 조건은 어댑터가 관리한다.
			/// @return 실패 원인을 보고하고 false를 반환하면 제출을 중단한다. 실패 후 재사용하지 않는다.
			bool (*record)(void* userData, ID3D12GraphicsCommandList* commands);
		};

		D3D12Renderer() = default;
		~D3D12Renderer();

		D3D12Renderer(const D3D12Renderer&) = delete;
		D3D12Renderer& operator=(const D3D12Renderer&) = delete;

		/// @brief 렌더러보다 오래 유지되는 window와 양수 클라이언트 크기로 초기화한다.
		/// @return 실패 시 false. API와 HRESULT를 보고하며 부분 자원은 소멸 시 정리한다.
		/// @param texture sRGB BaseColor RGBA8. 호출 중 복사하며 참조를 저장하지 않는다.
		/// @note 이미지 크기는 각 축 1~16384, pixels는 width * height * 4바이트여야 한다.
		/// @param mesh nullptr이면 회귀 기준 큐브를 사용한다. 지정하면 호출 중 복사하며 참조를 저장하지 않는다.
		/// @param baseColor 유한한 선형 RGBA 배율. 호출 중 복사하며 alpha는 불투명 출력에서 사용하지 않는다.
		/// @note 메시·재질·텍스처는 초기화 후 고정이며 모든 인스턴스가 공유한다.
		[[nodiscard]] bool Initialize(
			HWND window,
			UINT width,
			UINT height,
			const ImageData& texture,
			const MeshData* mesh = nullptr,
			const std::array<float, 4>& baseColor = { 1, 1, 1, 1 });

		/// @brief 하나 이상의 모델을 복사해 초기화한다. 배열 순서가 렌더러 내 모델 번호가 된다.
		/// @note 메시·텍스처·재질은 종료까지 고정한다. window·크기·오류 계약은 단일 모델 초기화와 같다.
		[[nodiscard]] bool Initialize(HWND window, UINT width, UINT height, std::span<const ModelSource> models);

		/// @brief 초기화 성공 후 호출한다. 사용할 프레임 자원이 실행 중일 때만 재사용을 기다린다.
		/// @param objectConstants BuildObjectTransformConstants로 계산한 메시 인스턴스별 변환 상수. 호출 중 복사하며 참조를 저장하지 않는다.
		/// @param lighting 유한한 프레임 값. direction은 단위 벡터, intensity는 0~2다.
		/// @note color·ambientStrength·specularStrength는 0~1, shininess는 1~256이다.
		/// @note 빈 목록은 메시를 생략하고 clear·선택 overlay·Present한다. 같은 모델 번호는 메시·재질을 공유하며 모든 모델이 파이프라인을 공유한다.
		/// @param overlay nullptr이면 UI 없이 그린다. 슬롯 Fence 대기 후 마지막 패스로 호출하며 보관하지 않는다.
		/// @param modelIndices 비어 있으면 모든 인스턴스가 모델 0을 사용한다. 지정 시 변환 상수와 같은 개수이며
		/// 각 값은 Initialize의 모델 배열 범위 안이어야 한다. 호출 중에만 읽고 보관하지 않는다.
		[[nodiscard]] bool Render(
			std::span<const ObjectTransformConstants> objectConstants,
			const FrameLighting& lighting,
			const OverlayPass* overlay = nullptr,
			std::span<const std::size_t> modelIndices = {});

		/// @brief Initialize 성공 후 UI 어댑터에만 대여한다. GPU 문맥은 렌더러보다 먼저 사용을 끝낸다.
		OverlayContext GetOverlayContext() const;

		/// @brief 프레임 경계에서 호출한다. 0 크기는 무시하며 실패 후 렌더링을 중단해야 한다.
		[[nodiscard]] bool Resize(UINT width, UINT height);

		/// @brief 종료 전에 호출하여 GPU 완료를 확인한다. 소멸자도 미완료 작업을 정리한다.
		[[nodiscard]] bool WaitForGpu();

	private:
		// GPU 픽셀 검증은 테스트 어댑터만 내부 큐·백 버퍼에 접근한다.
		friend class RendererReadback;
		[[nodiscard]] bool RenderInternal(
			std::span<const ObjectTransformConstants> objectConstants,
			const FrameLighting& lighting,
			bool present,
			const OverlayPass* overlay = nullptr,
			std::span<const std::size_t> modelIndices = {});

		struct GpuModel
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
			D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
			Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
			D3D12_INDEX_BUFFER_VIEW indexBufferView{};
			UINT indexCount = 0;
			std::array<float, 4> baseColor{ 1, 1, 1, 1 };
			TextureWrapMode wrapU = TextureWrapMode::ClampToEdge;
			TextureWrapMode wrapV = TextureWrapMode::ClampToEdge;
			Microsoft::WRL::ComPtr<ID3D12Resource> texture;
			// 업로드 완료 확인 실패 시에도 소멸자의 GPU 완료 시도까지 유지한다.
			Microsoft::WRL::ComPtr<ID3D12Resource> textureUpload;
		};

		struct FrameResources
		{
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
			// 첫 렌더링 전에는 비어 있다. 이후 슬롯의 Fence 완료 후에만 생성·확장한다.
			Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer;
			UINT64 fenceValue = 0;
		};

		// Debug 레이어·DXGI Factory와 하드웨어 장치를 생성한다. WARP로 대체하지 않는다.
		[[nodiscard]] bool CreateDevice();
		[[nodiscard]] bool CreateBackBuffers();
		[[nodiscard]] bool CreateMeshPipeline();
		// 입력을 검증한 뒤 렌더러 소유 버퍼로 복사한다. 실패 시 Initialize를 중단하고 파괴한다.
		[[nodiscard]] bool CreateGeometry(
			std::span<const MeshVertex> vertices,
			std::span<const std::uint32_t> indices,
			GpuModel& model);
		[[nodiscard]] bool CreateTexture(const ImageData& image, GpuModel& model, std::size_t modelIndex);
		[[nodiscard]] bool CreateDepthBuffer();

		// 호출자는 조명·크기 검증과 슬롯 Fence 대기를 끝낸다. 실패 시 해당 프레임을 제출하지 않는다.
		// objectConstants·lighting을 읽어 frame.constantBuffer를 생성·갱신한다.
		// 생성·확장 후 Map이 실패하면 false를 반환해도 버퍼는 이미 교체됐을 수 있다.
		[[nodiscard]] bool UploadFrameConstants(
			FrameResources& frame,
			std::span<const ObjectTransformConstants> objectConstants,
			const FrameLighting& lighting);

		// 성공 시 resource를 교체하고 실패 시 유지한다. 기존 자원의 GPU 사용 완료는 호출자가 보장한다.
		[[nodiscard]] bool CreateUploadBuffer(
			UINT64 size,
			Microsoft::WRL::ComPtr<ID3D12Resource>& resource);

		[[nodiscard]] bool WaitForFence(UINT64 target);
		[[nodiscard]] bool CheckResult(HRESULT result, const wchar_t* operation) const;

		static constexpr UINT BUFFER_COUNT = 2;

		Microsoft::WRL::ComPtr<IDXGIFactory6> mFactory;
		Microsoft::WRL::ComPtr<ID3D12Device> mDevice;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;

		Microsoft::WRL::ComPtr<IDXGISwapChain3> mSwapChain;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;
		Microsoft::WRL::ComPtr<ID3D12Resource> mDepthBuffer;
		std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, BUFFER_COUNT> mBackBuffers;

		// 백 버퍼 인덱스로 선택하며, 기록 메모리는 해당 제출의 Fence 완료까지 유지한다.
		std::array<FrameResources, BUFFER_COUNT> mFrames;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;
		// 초기화한 모델과 SRV를 같은 번호로 참조하고 GPU 완료 후 함께 정리한다.
		std::vector<GpuModel> mModels;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSrvHeap;
		UINT mSrvStride = 0;

		Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
		HANDLE mFenceEvent = nullptr;
		UINT64 mFenceValue = 0;

		UINT mRtvStride = 0;
		UINT mWidth = 0;
		UINT mHeight = 0;
		bool mGpuWorkPending = false;
	};
}
