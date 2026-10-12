Texture2D<float4> baseColor : register(t0);
SamplerState baseColorSampler : register(s0);

float4 Main(float4 position : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET
{
	// sRGB SRV에서 선형 색상으로 읽고 sRGB RTV로 출력한다. 조명과 투명도는 아직 처리하지 않는다.
	return float4(baseColor.Sample(baseColorSampler, uv).rgb, 1.0f);
}
