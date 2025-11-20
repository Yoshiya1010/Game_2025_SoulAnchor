struct VS_IN
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct PS_IN
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

PS_IN main(VS_IN input)
{
    PS_IN output;
    
    // フルスクリーンクアッド用なのでそのまま出力
    output.Position = float4(input.Position, 1.0);
    output.TexCoord = input.TexCoord;
    
    return output;
}