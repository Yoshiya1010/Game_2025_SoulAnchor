Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

cbuffer VignetteParams : register(b0)
{
    float intensity;
    float smoothness;
    float radius;
    float padding;
};

struct PS_IN
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

float4 main(PS_IN input) : SV_TARGET
{
    float4 color = g_Texture.Sample(g_SamplerState, input.TexCoord);
    
    float2 center = float2(0.5, 0.5);
    float dist = distance(input.TexCoord, center);
    
    // パラメータを使用
    float vignette = smoothstep(radius + smoothness, radius - smoothness, dist);
    vignette = lerp(1.0, vignette, intensity);
    
    return color * vignette;
}