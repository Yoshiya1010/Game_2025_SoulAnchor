#include "common.hlsl"

// シャドウマップ用のバッファ (b7)
cbuffer ShadowBuffer : register(b7)
{
    matrix LightViewProjection;
    float4 LightPosition;
}

// フラットシェーディング+リムライト用の構造体
struct PS_IN_FLAT
{
    float4 Position : SV_POSITION; // スクリーン空間の位置
    float4 WorldPosition : TEXCOORD0; // ワールド空間の位置（リムライト計算に必要）
    float3 Normal : TEXCOORD1; // 法線（念のため）
    float4 Diffuse : COLOR0; // 頂点カラー
    float2 TexCoord : TEXCOORD2; // テクスチャ座標
    float4 ShadowPos : TEXCOORD3; // シャドウマップ用座標
};

// シャドウマップのテクスチャとサンプラー（Toonと同じくt0を使用）
Texture2D g_ShadowMap : register(t0);
SamplerState g_SamplerState : register(s0);