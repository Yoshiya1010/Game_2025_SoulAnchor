#include"common.hlsl"


struct VS_IN_LINE
{
    float3 Position : POSITION0;
    float3 Normal : NORMAL0;
    float4 Color : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

struct PS_IN_LINE
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR0;
};

PS_IN_LINE main(VS_IN_LINE input)
{
    PS_IN_LINE output;
    
    // float3をfloat4に変換
    float4 pos = float4(input.Position, 1.0);
    
    // ワールド → ビュー → プロジェクション変換
    output.Position = mul(pos, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    output.Color = input.Color;
    
    return output;
}