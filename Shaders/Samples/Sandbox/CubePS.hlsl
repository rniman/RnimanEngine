Texture2D<float4> baseColor : register(t0);
SamplerState textureSamplers[9] : register(s0);

cbuffer MaterialConstants : register(b2)
{
	float3 baseColorFactor;
	uint samplerIndex;
};

cbuffer FrameConstants : register(b1)
{
	float3 lightDirection;
	float lightIntensity;
	float3 lightColor;
	float ambientStrength;
	float3 cameraPositionWS;
	float specularStrength;
	float shininess;
	float3 padding;
};

// CPU가 검증한 U * 3 + V 조합을 선택한다. 각 Sample의 Sampler 인덱스는 컴파일 시 상수다.
float3 SampleBaseColor(float2 uv)
{
	switch (samplerIndex)
	{
	case 0: return baseColor.Sample(textureSamplers[0], uv).rgb;
	case 1: return baseColor.Sample(textureSamplers[1], uv).rgb;
	case 2: return baseColor.Sample(textureSamplers[2], uv).rgb;
	case 3: return baseColor.Sample(textureSamplers[3], uv).rgb;
	case 4: return baseColor.Sample(textureSamplers[4], uv).rgb;
	case 5: return baseColor.Sample(textureSamplers[5], uv).rgb;
	case 6: return baseColor.Sample(textureSamplers[6], uv).rgb;
	case 7: return baseColor.Sample(textureSamplers[7], uv).rgb;
	case 8: return baseColor.Sample(textureSamplers[8], uv).rgb;
	default: return float3(1, 0, 1);
	}
}

float4 Main(
	float4 position : SV_POSITION,
	float2 uv : TEXCOORD,
	float3 positionWS : TEXCOORD1,
	float3 normalWS : TEXCOORD2) : SV_TARGET
{
	const float3 normal = normalize(normalWS);
	// direction은 빛의 진행 방향이므로 표면에서 광원을 향하는 벡터는 반대다.
	const float3 toLight = -lightDirection;
	const float diffuse = max(dot(normal, toLight), 0.0f);
	const float3 viewOffset = cameraPositionWS - positionWS;
	// 카메라가 표면과 같아도 0 벡터 정규화로 NaN을 만들지 않는다.
	const float3 toCamera = viewOffset * rsqrt(max(dot(viewOffset, viewOffset), 0.00000001f));
	const float3 reflected = reflect(-toLight, normal);
	const float specular = diffuse > 0.0f ? pow(max(dot(reflected, toCamera), 0.0f), shininess) : 0.0f;
	// sRGB SRV에서 읽은 선형 색상으로 계산하고 sRGB RTV가 출력 변환한다.
	const float3 sampledColor = SampleBaseColor(uv);
	const float3 albedo = sampledColor * baseColorFactor;
	const float3 directLight = lightColor * lightIntensity;
	const float3 color = albedo * (ambientStrength + directLight * diffuse) +
		directLight * specularStrength * specular;
	return float4(color, 1.0f);
}
