// simpleBlurPS.hlsl - 修正版

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

struct PS_IN
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float4 main(PS_IN input) : SV_TARGET
{
    // テクスチャサイズを動的に取得
    uint width, height;
    g_Texture.GetDimensions(width, height);
    float2 texelSize = float2(1.0 / float(width), 1.0 / float(height));
    
    float4 color = float4(0, 0, 0, 0);
    
    // 3x3カーネルでブラー（5x5より軽量）
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float2 offset = float2(x, y) * texelSize * 2.0;
            color += g_Texture.Sample(g_SamplerState, input.texcoord + offset);
        }
    }
    
    return color / 9.0;
}