// brightExtractPS.hlsl - 高輝度抽出のみ

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
    float4 color = g_Texture.Sample(g_SamplerState, input.texcoord);
    
    // 輝度計算
    float luminance = dot(color.rgb, float3(0.299, 0.587, 0.114));
    
    // 閾値以上の明るさの部分だけ抽出
    float bloom = max(0, luminance - threshold);
    
    // 明るい部分だけ返す（元画像は含めない）
    return float4(color.rgb * bloom * intensity, 1.0);
}