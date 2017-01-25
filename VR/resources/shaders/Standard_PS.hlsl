struct Input
{
    float4 position : SV_POSITION;
    float3 worldPosition : WORLDPOSITION;
    float2 uv : UV;
    float3x3 tbn : TBN;
};

Texture2D txDiffuse : register(t0);
Texture2D txNormal : register(t1);

struct Meta
{
    float3 cameraPosition;
    float pad;
};
StructuredBuffer<Meta> g_Meta : register(t2);

TextureCube txSkybox : register(t8);

SamplerState samp : register(s0);

float4 main(Input input) : SV_TARGET0
{
    Meta meta = g_Meta[0];
	float3 cameraPosition = meta.cameraPosition;

    float3 worldPosition = input.worldPosition;
    float2 uv = input.uv;

    float3 diffuse = txDiffuse.Sample(samp, uv).rgb;
    float3 normal = mul(normalize(2.f * txNormal.Sample(samp, uv).rgb - 1.f), input.tbn);

	float3 cameraVec = normalize(cameraPosition - worldPosition);
	float3 reflectVec = reflect(-cameraVec, normal);
	
	float3 reflectionDiffuse;
	{
		float3 skybox = txSkybox.Sample(samp, reflectVec).rgb;
		float r = saturate(dot(cameraVec, normal));
		float f = 0.32f;
		float fresnel = f + (1.f - f) * pow(1.f - r, 5);
		reflectionDiffuse = skybox * fresnel;
	}

	float3 lightDiffuse;
	{
		float3 power = 5.f;
		float3 lPosition = 0.f;
		float3 lDiffuse = 1.f;
		float3 lightVec = lPosition - worldPosition;
		float len = length(lightVec);
		lightVec = normalize(lightVec);
		float diffuseFactor = saturate(dot(lightVec, normal));
		float k = len + 1;
		float distanceFactor = power / (k * k);
		lightDiffuse = diffuseFactor * distanceFactor * diffuse;
	}

	float3 ambiantDiffuse = 0.2f * diffuse;

	float3 finalColor = lightDiffuse + reflectionDiffuse + ambiantDiffuse;

	// Tone mapping
	finalColor = finalColor / (finalColor + 1);

    return float4(finalColor, 1.f);
}
