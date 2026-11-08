#include "pch.h"
#include "SandboxApp.h"

#include "SandboxCamera.h"
#include "SandboxGui.h"
#include "SandboxLighting.h"
#include "SandboxRotation.h"

#include "Assets/AssetLoader.h"
#include "Assets/ImageData.h"
#include "Assets/ModelLoader.h"
#include "Math/QuaternionMath.h"
#include "Math/Transform.h"
#include "Platform/Win32Window.h"
#include "Rendering/D3D12Renderer.h"
#include "Rendering/RenderData.h"

// Windows 선언은 DirectX 헤더보다 먼저 필요하다.
#include <Windows.h>
#include <DirectXMath.h>
#include <imgui.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cwchar>
#include <filesystem>
#include <limits>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace rengine
{
	namespace
	{
		struct ModelAsset
		{
			const char* name;
			const wchar_t* file;
			const wchar_t* textureDirectory;
			const wchar_t* textureExtension;
		};

		constexpr std::array MODEL_ASSETS = {
			ModelAsset{ "BrickWallCube", L"BrickWallCube.bin", L"BrickWall17_MR_1K", L".png" },
			ModelAsset{ "BrickWallSphere", L"BrickWallSphere.bin", L"BrickWall17_MR_1K", L".png" },
			ModelAsset{ "DamagedHelmet", L"DamagedHelmet.bin", L"DamagedHelmet_textures", L".jpg" }
		};

		bool EditVector3(
			const char* label,
			DirectX::XMFLOAT3& vector,
			float step,
			float minimum,
			float maximum)
		{
			// 범위 없는 회전 입력은 ClampZeroRange를 포함한 AlwaysClamp를 사용하지 않는다.
			// XMFLOAT3 멤버를 포인터 산술로 순회하지 않고 ImGui의 배열 입력으로 복사한다.
			float values[] = { vector.x, vector.y, vector.z };
			const bool changed = ImGui::DragFloat3(
				label,
				values,
				step,
				minimum,
				maximum,
				"%.3f",
				minimum < maximum ? ImGuiSliderFlags_AlwaysClamp : ImGuiSliderFlags_None);
			if (changed)
			{
				vector = { values[0], values[1], values[2] };
			}

			return changed;
		}
	}

	SandboxApp::SandboxApp(HINSTANCE instance)
		: mWindow{ instance }, mGui{ mWindow, mRenderer }
	{
	}

	bool SandboxApp::Initialize(int showCommand)
	{
		if (!mWindow.Initialize(showCommand))
		{
			return false;
		}

		const ExecutableDirectoryResult executableDirectoryResult = GetExecutableDirectory();
		if (FAILED(executableDirectoryResult.result))
		{
			wchar_t message[256]{};
			swprintf_s(
				message,
				L"GetModuleFileNameW failed. HRESULT: 0x%08lX",
				static_cast<unsigned long>(executableDirectoryResult.result));
			OutputDebugStringW(message);
			MessageBoxW(mWindow.GetHandle(), message, L"Sandbox asset error", MB_OK | MB_ICONERROR);
			return false;
		}

		// 빌드가 계산한 EXE→저장소 루트 상대 경로를 사용하며 실행용 복사본은 만들지 않는다.
		const std::filesystem::path assetDirectory =
			(executableDirectoryResult.directory / RNIMAN_SOURCE_ROOT / L"Assets").lexically_normal();
		mModelInfo.resize(MODEL_ASSETS.size());
		std::array<StaticModel, MODEL_ASSETS.size()> models;
		std::array<ImageData, MODEL_ASSETS.size()> textures;
		std::vector<D3D12Renderer::ModelSource> sources;
		sources.reserve(MODEL_ASSETS.size());
		for (std::size_t index = 0; index < MODEL_ASSETS.size(); ++index)
		{
			const ModelAsset& asset = MODEL_ASSETS[index];
			const std::filesystem::path modelPath = assetDirectory / L"Models" / asset.file;
			ModelLoadResult loadedModel = LoadStaticModel(modelPath);
			if (!loadedModel.model)
			{
				// Reader 진단은 ASCII API/필드 이름이며 파일 경로는 Unicode로 보존한다.
				const std::wstring reason(loadedModel.error.begin(), loadedModel.error.end());
				const std::wstring message = L"LoadStaticModel failed: " + reason + L"\nPath: " + modelPath.wstring();
				OutputDebugStringW(message.c_str());
				MessageBoxW(mWindow.GetHandle(), message.c_str(), L"Sandbox model error", MB_OK | MB_ICONERROR);
				return false;
			}

			models[index] = std::move(*loadedModel.model);
			ModelInfo& info = mModelInfo[index];
			info.name = asset.name;
			info.defaultTransform = models[index].transform;
			info.vertexCount = models[index].mesh.vertices.size();
			info.triangleCount = models[index].mesh.indices.size() / 3;
			info.materialWarning = std::move(loadedModel.warning);
			if (!info.materialWarning.empty())
			{
				OutputDebugStringA((std::string(asset.name) + ": " + info.materialWarning + "\n").c_str());
			}

			const std::string& textureName = models[index].baseColorTextureRef;
			if (textureName.empty())
			{
				textures[index] = { 1, 1, { 255, 255, 255, 255 } };
			}
			else
			{
				std::filesystem::path textureFile{ std::u8string(textureName.begin(), textureName.end()) };
				// 확장자 없는 UTF-8 참조를 보존하며 폴더·형식은 이 데모에서 명시한다.
				textureFile += asset.textureExtension;
				const std::filesystem::path texturePath = assetDirectory / asset.textureDirectory / textureFile;
				ImageLoadResult loadedImage = LoadImage(texturePath);
				if (FAILED(loadedImage.result))
				{
					wchar_t errorCode[32]{};
					swprintf_s(errorCode, L"0x%08lX", static_cast<unsigned long>(loadedImage.result));
					const std::wstring message = std::wstring(loadedImage.operation) +
						L" failed. HRESULT: " + errorCode + L"\nPath: " + texturePath.wstring();
					OutputDebugStringW(message.c_str());
					MessageBoxW(mWindow.GetHandle(), message.c_str(), L"Sandbox texture error", MB_OK | MB_ICONERROR);
					return false;
				}

				textures[index] = std::move(loadedImage.image);
			}

			// 배열의 CPU 데이터는 아래 Initialize 호출이 끝날 때까지 유효하다.
			sources.push_back({ textures[index], &models[index].mesh, models[index].baseColor,
				models[index].wrapU, models[index].wrapV });
		}

		mObjects.front().transform = mModelInfo[mObjects.front().modelIndex].defaultTransform;
		for (const auto& object : INITIAL_OBJECTS)
		{
			mModelInfo[object.modelIndex].rotationAxis = object.rotationAxis;
			mModelInfo[object.modelIndex].rotationSpeed = object.rotationSpeed;
		}
		if (!mCamera.SetPosition({ 0.0f, 3.0f, -8.0f }))
		{
			return false;
		}

		const Win32Window::State& state = mWindow.GetState();
		// 최소화 상태로 시작해도 GPU 자원은 양수 크기로 초기화한다.
		if (!mRenderer.Initialize(
			mWindow.GetHandle(),
			state.width == 0 ? 1280 : state.width,
			state.height == 0 ? 720 : state.height,
			sources))
		{
			return false;
		}

		return mGui.Initialize();
	}

	int SandboxApp::Run(int showCommand)
	{
		if (!Initialize(showCommand))
		{
			return 1;
		}

		const Win32Window::State& state = mWindow.GetState();
		const auto startTime = std::chrono::steady_clock::now();
		auto previousTime = startTime;

		while (true)
		{
			if (const std::optional<int> exitCode = mWindow.PumpMessages())
			{
				return mRenderer.WaitForGpu() ? *exitCode : 1;
			}

			if (state.minimized || state.width == 0 || state.height == 0)
			{
				if (!mWindow.WaitForMessage())
				{
					return 1;
				}

				continue;
			}

			if (state.resizePending)
			{
				if (!mRenderer.Resize(state.width, state.height))
				{
					return 1;
				}

				mWindow.ClearResizeRequest();
			}

			mGui.BeginFrame();
			const Win32Window::InputState input = mWindow.ConsumeInput();
			const auto now = std::chrono::steady_clock::now();
			// 모달 창 조작·최소화 이후 경과 시간은 이동에 누적하지 않는다.
			const float deltaSeconds = input.resetFrameTime ? 0.0f :
				std::min(std::chrono::duration<float>(now - previousTime).count(), 0.1f);
			previousTime = now;

			const double elapsedSeconds = std::chrono::duration<double>(now - startTime).count();
			Update(input, deltaSeconds, elapsedSeconds);
			DrawTools();
			mGui.EndFrame();
			ApplyObjectRequests();
			if (!Render())
			{
				return 1;
			}
		}
	}

	void SandboxApp::Update(const Win32Window::InputState& input, float deltaSeconds, double elapsedSeconds)
	{
		if (mWindow.GetState().focused)
		{
			if (input.keys['R'])
			{
				mLighting.Reset();
			}
			else
			{
				mLighting.Adjust(
					static_cast<float>(input.keys[VK_RIGHT]) - static_cast<float>(input.keys[VK_LEFT]),
					static_cast<float>(input.keys[VK_UP]) - static_cast<float>(input.keys[VK_DOWN]),
					static_cast<float>(input.keys[VK_PRIOR]) - static_cast<float>(input.keys[VK_NEXT]),
					deltaSeconds);
			}

			mCamera.Rotate(input.mouseDeltaX, input.mouseDeltaY);
			mCamera.Move(
				static_cast<float>(input.keys['D']) - static_cast<float>(input.keys['A']),
				static_cast<float>(input.keys['E']) - static_cast<float>(input.keys['Q']),
				static_cast<float>(input.keys['W']) - static_cast<float>(input.keys['S']),
				deltaSeconds);
		}

		// 시간 기준은 유지하되 편집한 회전을 덮어쓰지 않는다. 정지 중 시간은 회전에 적립하지 않는다.
		const double animationSeconds = elapsedSeconds - mPreviousAnimationTime;
		mPreviousAnimationTime = elapsedSeconds;
		for (auto& object : mObjects)
		{
			if (object.autoRotate)
			{
				const float angle = static_cast<float>(std::remainder(
					animationSeconds * object.rotationSpeed,
					static_cast<double>(DirectX::XM_2PI)));
				const std::optional<DirectX::XMFLOAT4> delta = QuaternionFromAxisAngle(object.rotationAxis, angle);
				const std::optional<DirectX::XMFLOAT4> rotation = delta ?
					ComposeRotations(object.transform.rotation, *delta) : std::nullopt;
				if (!rotation)
				{
					object.autoRotate = false;
					mInvalidEdit = true;
					continue;
				}

				object.transform.rotation = *rotation;
			}
		}
	}

	void SandboxApp::DrawTools()
	{
		ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(400.0f, 505.0f), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Sandbox Tools"))
		{
			const ImGuiIO& io = ImGui::GetIO();
			ImGui::Text(
				"Objects: %zu | %.2f ms/frame | %.1f FPS",
				mObjects.size(),
				1000.0f / io.Framerate,
				io.Framerate);
			ImGui::TextDisabled("Frame interval; not GPU execution time");
			ImGui::Checkbox("Show Dear ImGui Demo", &mShowDemo);
			DrawObjectTools();

			ImGui::SeparatorText("Camera");
			DirectX::XMFLOAT3 cameraPosition = mCamera.GetPosition();
			if (EditVector3("Camera position", cameraPosition, 0.02f, -100.0f, 100.0f))
			{
				mInvalidEdit = !mCamera.SetPosition(cameraPosition);
			}

			DrawLightingTools();
			if (mInvalidEdit)
			{
				ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.3f, 1.0f), "Invalid edit rejected; previous value kept.");
			}
		}

		// Begin이 false인 접힌 창도 End로 닫아야 한다.
		ImGui::End();
		if (mShowDemo)
		{
			ImGui::ShowDemoWindow(&mShowDemo);
		}
	}

	SandboxApp::ModelInstance* SandboxApp::FindObject(std::uint64_t id)
	{
		for (auto& object : mObjects)
		{
			if (object.id == id)
			{
				return &object;
			}
		}

		return nullptr;
	}

	std::optional<std::uint64_t> SandboxApp::CreateObject(std::size_t modelIndex)
	{
		if (modelIndex >= mModelInfo.size() || !mNextObjectId)
		{
			mObjectError = modelIndex >= mModelInfo.size() ? "Invalid model index." : "Object IDs exhausted.";
			return std::nullopt;
		}

		const ModelInfo& model = mModelInfo[modelIndex];
		const std::uint64_t id = *mNextObjectId;
		try
		{
			mObjects.push_back({ id, modelIndex, model.defaultTransform,
				model.rotationAxis, model.rotationSpeed, false, {} });
		}
		catch (const std::bad_alloc&)
		{
			mObjectError = "Cannot allocate another object.";
			return std::nullopt;
		}
		catch (const std::length_error&)
		{
			mObjectError = "Object storage limit reached.";
			return std::nullopt;
		}

		if (id == std::numeric_limits<std::uint64_t>::max())
		{
			mNextObjectId.reset();
		}
		else
		{
			mNextObjectId = id + 1;
		}
		return id;
	}

	void SandboxApp::DeleteObject(std::uint64_t id)
	{
		const auto found = std::find_if(mObjects.begin(), mObjects.end(),
			[id](const ModelInstance& object) { return object.id == id; });
		if (found == mObjects.end())
		{
			mObjectError = "Object to delete was not found.";
			return;
		}
		mObjects.erase(found);
		mSelectedObject.reset();
	}

	void SandboxApp::ApplyObjectRequests()
	{
		if (mDeleteRequest || mCreateRequest)
		{
			mObjectError.clear();
			mInvalidEdit = false;
		}
		// UI에서 빌린 참조의 사용이 끝난 뒤 삭제·생성을 순서대로 적용한다.
		if (mDeleteRequest)
		{
			DeleteObject(*mDeleteRequest);
			mDeleteRequest.reset();
		}
		if (mCreateRequest)
		{
			if (const std::optional<std::uint64_t> id = CreateObject(*mCreateRequest))
			{
				mSelectedObject = *id;
			}
			mCreateRequest.reset();
		}
	}

	void SandboxApp::DrawObjectTools()
	{
		ImGui::SeparatorText("Object");
		if (ImGui::BeginCombo("Create model", mModelInfo[mCreateModel].name.c_str()))
		{
			for (std::size_t index = 0; index < mModelInfo.size(); ++index)
			{
				const bool selected = index == mCreateModel;
				if (ImGui::Selectable(mModelInfo[index].name.c_str(), selected))
				{
					mCreateModel = index;
				}
				if (selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("Create"))
		{
			mCreateRequest = mCreateModel;
		}
		if (!mObjectError.empty())
		{
			ImGui::TextWrapped("%s", mObjectError.c_str());
		}
		const ModelInstance* selectedObject = mSelectedObject ? FindObject(*mSelectedObject) : nullptr;
		if (selectedObject == nullptr)
		{
			mSelectedObject.reset();
		}

		const std::string selectedLabel = selectedObject != nullptr ?
			"Object " + std::to_string(selectedObject->id) + " - " + mModelInfo[selectedObject->modelIndex].name :
			"None";
		if (ImGui::BeginCombo("Selected", selectedLabel.c_str()))
		{
			for (const auto& object : mObjects)
			{
				const std::string label = "Object " + std::to_string(object.id) + " - " + mModelInfo[object.modelIndex].name;
				const bool selected = mSelectedObject == object.id;
				if (ImGui::Selectable(label.c_str(), selected))
				{
					mSelectedObject = object.id;
				}

				if (selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		// 선택 변경 후 다시 조회해 통계와 편집 대상을 같은 프레임에 맞춘다.
		ModelInstance* editableObject = mSelectedObject ? FindObject(*mSelectedObject) : nullptr;
		ImGui::BeginDisabled(editableObject == nullptr);
		if (ImGui::Button("Delete selected"))
		{
			mDeleteRequest = mSelectedObject;
		}
		ImGui::EndDisabled();
		if (editableObject == nullptr)
		{
			mSelectedObject.reset();
			ImGui::TextDisabled("No object selected.");
			return;
		}

		ModelInstance& object = *editableObject;
		const ModelInfo& info = mModelInfo[object.modelIndex];
		ImGui::TextUnformatted(info.name.c_str());
		if (!info.materialWarning.empty())
		{
			ImGui::TextWrapped("%s", info.materialWarning.c_str());
		}
		ImGui::TextDisabled("%zu vertices / %zu triangles", info.vertexCount, info.triangleCount);
		if (!object.rotationEditor.Synchronize(object.transform.rotation))
		{
			mInvalidEdit = true;
			return;
		}

		SandboxRotation rotationEditor = object.rotationEditor;
		DirectX::XMFLOAT3 degrees = rotationEditor.GetDegrees();
		// 편집 복사본 전체를 검증한 뒤 공개해 부분 적용을 피한다.
		Transform edited = object.transform;
		bool changed = EditVector3("Position", edited.position, 0.02f, -100.0f, 100.0f);
		const bool rotationChanged = EditVector3(
			"Rotation (deg)",
			degrees,
			0.5f,
			0.0f,
			0.0f);
		if (rotationChanged)
		{
			const std::optional<DirectX::XMFLOAT4> rotation = rotationEditor.SetDegrees(degrees);
			if (!rotation)
			{
				mInvalidEdit = true;
				return;
			}

			edited.rotation = *rotation;
		}

		changed |= rotationChanged;
		changed |= EditVector3("Scale", edited.scale, 0.01f, 0.05f, 10.0f);
		if (changed)
		{
			// 렌더러와 같은 유한·비특이·양수 행렬식 조건을 사용한다.
			mInvalidEdit = !BuildObjectTransformConstants(edited.GetWorld(), DirectX::XMMatrixIdentity()).has_value();
			if (!mInvalidEdit)
			{
				object.transform = edited;
				object.rotationEditor = rotationEditor;
				if (rotationChanged)
				{
					object.autoRotate = false;
				}
			}
		}

		ImGui::Checkbox("Auto rotate", &object.autoRotate);
		ImGui::TextWrapped("Rotation edits pause animation.");
	}

	void SandboxApp::DrawLightingTools()
	{
		ImGui::SeparatorText("Directional light");
		float yaw = mLighting.GetYaw();
		float pitch = mLighting.GetPitch();
		float intensity = mLighting.GetIntensity();
		bool changed = ImGui::SliderFloat(
			"Yaw (rad)",
			&yaw,
			-DirectX::XM_PI,
			DirectX::XM_PI,
			"%.3f",
			ImGuiSliderFlags_AlwaysClamp);
		changed |= ImGui::SliderFloat(
			"Pitch (rad)",
			&pitch,
			-SandboxLighting::PITCH_LIMIT,
			SandboxLighting::PITCH_LIMIT,
			"%.3f",
			ImGuiSliderFlags_AlwaysClamp);
		changed |= ImGui::SliderFloat(
			"Intensity",
			&intensity,
			0.0f,
			SandboxLighting::MAX_INTENSITY,
			"%.3f",
			ImGuiSliderFlags_AlwaysClamp);
		if (changed)
		{
			mInvalidEdit = !mLighting.SetParameters(yaw, pitch, intensity);
		}

		if (ImGui::Button("Reset light"))
		{
			mLighting.Reset();
		}
	}

	bool SandboxApp::Render()
	{
		const Win32Window::State& state = mWindow.GetState();
		const DirectX::XMMATRIX view = mCamera.GetView();
		const DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(
			DirectX::XM_PIDIV4,
			static_cast<float>(state.width) / static_cast<float>(state.height),
			0.1f,
			100.0f);

		mObjectConstants.clear();
		mModelIndices.clear();
		const DirectX::XMMATRIX viewProjection = view * projection;
		for (const auto& object : mObjects)
		{
			const std::optional<ObjectTransformConstants> constantsResult = BuildObjectTransformConstants(
				object.transform.GetWorld(),
				viewProjection);
			if (!constantsResult)
			{
				wchar_t message[256]{};
				swprintf_s(message,
					L"Sandbox object %llu (model %zu): transform is non-finite, singular or reflected.",
					static_cast<unsigned long long>(object.id), object.modelIndex);
				OutputDebugStringW(message);
				MessageBoxW(mWindow.GetHandle(), message, L"Sandbox transform error", MB_OK | MB_ICONERROR);
				return false;
			}

			mObjectConstants.push_back(*constantsResult);
			mModelIndices.push_back(object.modelIndex);
		}

		FrameLighting lighting;
		lighting.direction = mLighting.GetDirection();
		lighting.intensity = mLighting.GetIntensity();
		lighting.cameraPosition = mCamera.GetPosition();
		const D3D12Renderer::OverlayPass overlay = mGui.GetOverlayPass();
		return mRenderer.Render(mObjectConstants, lighting, &overlay, mModelIndices);
	}
}
