#include "pch.h"
#include "AssetLoader.h"

#include "ImageData.h"

#include <Windows.h>
#include <objbase.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <filesystem>
#include <limits>
#include <new>
#include <string>
#include <utility>

namespace rengine
{
	using Microsoft::WRL::ComPtr;

	namespace
	{
		class ComApartment
		{
		public:
			ComApartment()
				: mResult{ CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED) }
			{
			}

			~ComApartment()
			{
				if (SUCCEEDED(mResult))
				{
					CoUninitialize();
				}
			}

			ComApartment(const ComApartment&) = delete;
			ComApartment& operator=(const ComApartment&) = delete;

			HRESULT GetResult() const
			{
				return mResult;
			}

		private:
			HRESULT mResult;
		};
	}

	ExecutableDirectoryResult GetExecutableDirectory()
	{
		std::wstring executablePath(32768, L'\0');
		const DWORD length = GetModuleFileNameW(
			nullptr,
			executablePath.data(),
			static_cast<DWORD>(executablePath.size()));
		if (length == 0)
		{
			return { {}, HRESULT_FROM_WIN32(GetLastError()) };
		}

		if (length >= executablePath.size())
		{
			return { {}, HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) };
		}

		executablePath.resize(length);
		return { std::filesystem::path(executablePath).parent_path(), S_OK };
	}

	ImageLoadResult LoadPngImage(const std::filesystem::path& path)
	{
		ComApartment apartment;
		if (FAILED(apartment.GetResult()))
		{
			return { {}, apartment.GetResult(), L"CoInitializeEx(STA)" };
		}

		ComPtr<IWICImagingFactory> factory;
		HRESULT result = CoCreateInstance(
			CLSID_WICImagingFactory,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&factory));
		if (FAILED(result))
		{
			return { {}, result, L"CoCreateInstance(WIC)" };
		}

		ComPtr<IWICBitmapDecoder> decoder;
		result = factory->CreateDecoderFromFilename(
			path.c_str(),
			nullptr,
			GENERIC_READ,
			WICDecodeMetadataCacheOnLoad,
			&decoder);
		if (FAILED(result))
		{
			return { {}, result, L"WIC::CreateDecoderFromFilename" };
		}

		GUID container{};
		result = decoder->GetContainerFormat(&container);
		if (FAILED(result))
		{
			return { {}, result, L"WIC::GetContainerFormat" };
		}

		if (container != GUID_ContainerFormatPng)
		{
			return { {}, WINCODEC_ERR_UNKNOWNIMAGEFORMAT, L"LoadPngImage: PNG required" };
		}

		UINT frameCount = 0;
		result = decoder->GetFrameCount(&frameCount);
		if (FAILED(result))
		{
			return { {}, result, L"WIC::GetFrameCount" };
		}

		if (frameCount != 1)
		{
			return { {}, WINCODEC_ERR_UNSUPPORTEDOPERATION, L"LoadPngImage: single frame required" };
		}

		ComPtr<IWICBitmapFrameDecode> frame;
		result = decoder->GetFrame(0, &frame);
		if (FAILED(result))
		{
			return { {}, result, L"WIC::GetFrame" };
		}

		UINT width = 0;
		UINT height = 0;
		result = frame->GetSize(&width, &height);
		if (FAILED(result))
		{
			return { {}, result, L"WIC::GetSize" };
		}

		// CopyPixels의 행 간격·총 바이트는 UINT다. 곱셈 전에 범위를 검사한다.
		if (width == 0 || height == 0 || width > std::numeric_limits<UINT>::max() / 4)
		{
			return { {}, E_INVALIDARG, L"LoadPngImage: invalid dimensions" };
		}

		const UINT rowBytes = width * 4;
		if (height > std::numeric_limits<UINT>::max() / rowBytes)
		{
			return { {}, E_INVALIDARG, L"LoadPngImage: pixel size overflow" };
		}

		ComPtr<IWICFormatConverter> converter;
		result = factory->CreateFormatConverter(&converter);
		if (FAILED(result))
		{
			return { {}, result, L"WIC::CreateFormatConverter" };
		}

		result = converter->Initialize(
			frame.Get(),
			GUID_WICPixelFormat32bppRGBA,
			WICBitmapDitherTypeNone,
			nullptr,
			0.0,
			WICBitmapPaletteTypeCustom);
		if (FAILED(result))
		{
			return { {}, result, L"WIC::FormatConverter::Initialize(RGBA8)" };
		}

		ImageData image;
		image.width = width;
		image.height = height;
		const UINT bytes = rowBytes * height;
		try
		{
			image.pixels.resize(bytes);
		}
		catch (const std::bad_alloc&)
		{
			return { {}, E_OUTOFMEMORY, L"LoadPngImage: allocate pixels" };
		}

		result = converter->CopyPixels(nullptr, rowBytes, bytes, image.pixels.data());
		if (FAILED(result))
		{
			return { {}, result, L"WIC::CopyPixels" };
		}

		return { std::move(image), S_OK, L"LoadPngImage" };
	}
}
