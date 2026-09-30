// CPU의 DirectXMath 행 벡터 행렬을 전치 없이 전달한다.
cbuffer ObjectConstants : register(b0)
{
	row_major float4x4 worldViewProjection;
};

struct VertexInput
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct PixelInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PixelInput Main(VertexInput input)
{
	PixelInput output;
	output.position = mul(float4(input.position, 1.0f), worldViewProjection);
	output.color = input.color;
	return output;
}
