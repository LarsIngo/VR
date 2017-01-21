struct Input
{
    float4 position : SV_POSITION;
    float3 worldPosition : WORLDPOSITION;
    float2 uv : UV;
    float3x3 tbn : TBN;
};

Texture2D txDiffuse : register(t0);
Texture2D txNormal : register(t1);

SamplerState samp : register(s0);

float4 main(Input input) : SV_TARGET0
{
    float3 finalColor = float3(0.f, 0.f, 0.f);

    float3 worldPosition = input.worldPosition;
    float2 uv = input.uv;
    float3 diffuse = txDiffuse.Sample(samp, uv).rgb;
    float3 normal = mul(normalize(2.f * txNormal.Sample(samp, uv).rgb - float3(1.f, 1.f, 1.f)), input.tbn);

    finalColor = diffuse * dot(normal, normalize(input.worldPosition)) + diffuse * 0.1f;

    return float4(saturate(finalColor), 1.f);
}
