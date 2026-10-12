// CPU의 DirectXMath 행 벡터 행렬을 전치 없이 전달한다.
cbuffer ObjectConstants : register(b0)
{
	row_major float4x4 worldViewProjection;
};

struct VertexInput
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct PixelInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

PixelInput Main(VertexInput input)
{
	PixelInput output;
	output.position = mul(float4(input.position, 1.0f), worldViewProjection);
	output.uv = input.uv;
	return output;
}
