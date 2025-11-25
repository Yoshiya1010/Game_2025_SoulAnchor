// brightExtractPS.hlsl - ‚‹P“x’Šo‚Ì‚İ

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

cbuffer BrightParams : register(b0)
{
    float threshold;
    float intensity;
    float2 padding;
}

struct PS_IN
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float4 main(PS_IN input) : SV_TARGET
{
    return float4(1.0, 1.0, 1.0, 1.0);
}