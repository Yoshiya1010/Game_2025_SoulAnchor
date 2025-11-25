
Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

cbuffer BrightParams : register(b0)
{
    float threshold; // 輝度の閾値
    float intensity; // ブルームの強度
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
    
    // 閾値以下は黒に
    if (luminance < threshold)
    {
        return float4(0, 0, 0, 1);
    }
    
    // 閾値以上は強度を適用
    return float4(color.rgb * intensity, 1.0);
}