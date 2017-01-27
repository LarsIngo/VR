struct Input
{
    float4 position : SV_POSITION;
    float3 worldPosition : WORLDPOSITION;
    float2 uv : UV;
    float3x3 tbn : TBN;
};

Texture2D txDiffuse : register(t0);
Texture2D txNormal : register(t1);
Texture2D txGloss : register(t2);

struct Meta
{
    float3 cameraPosition;
	uint skyboxMipLevels;
};
StructuredBuffer<Meta> g_Meta : register(t7);

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

	float gloss = txGloss.Sample(samp, uv).x;
	
	// TMP POINTLIGHT
	float3 power = 5.f;
	float3 lPosition = float3(5, 5, -5);
	float3 lDiffuse = 1.f;
	float3 lightVec = lPosition - worldPosition;
	float len = length(lightVec);
	lightVec = normalize(lightVec);

	float3 lightDiffuse;
	float3 specular;
	float specularFactor;
	{
		float diffuseFactor = saturate(dot(lightVec, normal));
		float k = len + 1;
		float distanceFactor = 1;//power / (k * k);
		lightDiffuse = diffuseFactor * distanceFactor * diffuse;
		float r = saturate(dot(reflectVec, lightVec));
		r = pow(r, 5.f + 5.f * gloss);
		specular = (1.f - diffuseFactor) * lDiffuse * r;
		specularFactor = r;
	}

	float3 reflectionDiffuse;
	{
		float3 skybox = txSkybox.SampleLevel(samp, reflectVec, meta.skyboxMipLevels * (1.f - gloss)).rgb;
		float r = saturate(dot(cameraVec, normal));
		float f = 0.42f;
		float fresnel = f + (1.f - f) * pow(1.f - r, 5);
		reflectionDiffuse = skybox * fresnel * (1.f - specularFactor);
	}

	float3 ambiantDiffuse = 0.2f * diffuse;

	float3 finalColor = specular + lightDiffuse + reflectionDiffuse + ambiantDiffuse;

	// Tone mapping
	finalColor = finalColor / (finalColor + 1);

    return float4(finalColor, 1.f);
}
