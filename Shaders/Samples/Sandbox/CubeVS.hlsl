// CPU의 DirectXMath 행 벡터 행렬을 전치 없이 전달한다.
cbuffer ObjectConstants : register(b0)
{
	row_major float4x4 worldViewProjection;
	row_major float4x4 world;
	row_major float4x4 normalTransform;
};

struct VertexInput
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};

struct PixelInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 positionWS : TEXCOORD1;
	float3 normalWS : TEXCOORD2;
};

PixelInput Main(VertexInput input)
{
	PixelInput output;
	output.position = mul(float4(input.position, 1.0f), worldViewProjection);
	output.uv = input.uv;
	output.positionWS = mul(float4(input.position, 1.0f), world).xyz;
	// 비균일 Scale에서도 법선과 접선의 직교 관계를 유지하는 역전치 행렬이다.
	output.normalWS = mul(input.normal, (float3x3)normalTransform);
	return output;
}
