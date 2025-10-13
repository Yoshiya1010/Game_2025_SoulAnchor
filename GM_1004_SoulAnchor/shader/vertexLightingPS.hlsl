
#include "common.hlsl"

Texture2D g_Texture : register(t0); // テクスチャ画像を表す変数
SamplerState g_SamplerState : register(s0); // テクスチャの取得の仕方の設定

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
    // テクスチャのデータを取得する
    outDiffuse = g_Texture.Sample(g_SamplerState, In.TexCoord);

    // ポリゴンの色と合成して出力
    outDiffuse *= In.Diffuse;
}