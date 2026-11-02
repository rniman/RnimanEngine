#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace custom_asset_export
{
	enum class CoordinateSystem : std::uint32_t
	{
		UnityOriginal,
		DirectXLeftHandedYUp,
		RightHandedYUp
	};
	using Vector2 = std::array<float, 2>;
	using Vector3 = std::array<float, 3>;
	using Vector4 = std::array<float, 4>;

	// v2 texture binding. References identify source assets, not engine resources.
	struct SurfaceTexture
	{
		// 0 BaseColor, 1 Normal, 2 Metallic, 3 Gloss, 4 Occlusion, 5 Emission, 6 Height.
		std::uint32_t usage = 0;
		std::string textureRef;	   // Source filename without extension.
		std::uint32_t channel = 4; // 0 R, 1 G, 2 B, 3 A, 4 RGB.
		// 0 Repeat, 1 ClampToEdge, 2 MirroredRepeat. 텍스처 바인딩별 값이다.
		std::uint32_t wrapU = 0;
		std::uint32_t wrapV = 0;
	};

	struct SurfaceMaterial
	{
		std::uint32_t shadingModel = 0; // 0 Metallic PBR, 1 Unlit.
		std::uint32_t colorSpace = 1;
		Vector4 baseColor{1, 1, 1, 1};
		float metallic = 0;
		float gloss = .5f;
		std::uint32_t glossMode = 1; // 0 Roughness, 1 Smoothness; gloss multiplies a mapped sample.
		float normalScale = 1;
		float occlusionStrength = 1;
		float heightScale = 0;
		Vector3 emission{};
		std::uint32_t surface = 0;
		std::uint32_t alphaClip = 0;
		std::uint32_t cull = 2;
		std::uint32_t alphaSource = 0;
		float alphaCutoff = .5f;
		// All texture usages share uv0 * uvScale + uvOffset.
		Vector2 uvScale{1, 1};
		Vector2 uvOffset{};
		std::vector<SurfaceTexture> textures;
	};

	struct Material
	{
		std::int32_t id = 0;
		std::string name;
		SurfaceMaterial surface;
	};

	/// Row-major 저장, Column Vector 연산이다. Translation은 원소 3, 7, 11이다.
	using Matrix4 = std::array<float, 16>;

	struct Bounds
	{
		Vector3 center{};
		Vector3 extents{};
	};

	struct Submesh
	{
		std::uint32_t index = 0; // v2: derived array position, not stored in the file.
		std::vector<std::uint32_t> indices;
	};

	struct Mesh
	{
		std::int32_t id = 0;
		std::string name;
		Bounds bounds;
		std::vector<Vector3> positions;
		std::vector<Vector3> normals;
		std::vector<Vector2> uv0;
		std::vector<Vector4> tangents;
		std::vector<Submesh> submeshes;
	};

	struct Node
	{
		std::int32_t id = 0;
		std::int32_t parentId = -1;
		std::int32_t meshId = -1;
		std::string name;
		Matrix4 localMatrix{};
		/// Submesh별 Material ID. -1은 Consumer 기본 Material.
		std::vector<std::int32_t> materialIds;
	};

	struct Model
	{
		std::uint32_t version = 2;
		CoordinateSystem coordinateSystem = CoordinateSystem::UnityOriginal;
		/// ID는 배열 위치가 아니다. 이름 대신 ID로 참조를 해석한다.
		std::vector<Node> nodes;
		std::vector<Mesh> meshes;
		std::vector<Material> materials;
	};
} // namespace custom_asset_export
