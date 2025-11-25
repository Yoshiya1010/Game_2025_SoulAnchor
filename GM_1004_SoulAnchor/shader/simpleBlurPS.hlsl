// simpleBlurPS.hlsl

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

struct PS_IN
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float4 main(PS_IN input) : SV_TARGET
{
    float2 texelSize = float2(1.0 / 1920.0, 1.0 / 1080.0); // SCREEN_WIDTHとHEIGHTに合わせて調整
    
    float4 color = float4(0, 0, 0, 0);
    
    // 5x5カーネルでブラー
    for (int x = -2; x <= 2; x++)
    {
        for (int y = -2; y <= 2; y++)
        {
            float2 offset = float2(x, y) * texelSize;
            color += g_Texture.Sample(g_SamplerState, input.texcoord + offset);
        }
    }
    
    return color / 25.0; // 25サンプルの平均
}