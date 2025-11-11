// Toon + Shadow共通定義

#include "common.hlsl"

// シャドウマップ用のバッファ (b7)
cbuffer ShadowBuffer : register(b7)
{
    matrix LightViewProjection; // ライトから見たビュー×プロジェクション行列
    float4 LightPosition; // ライトの位置
}

// Toon用の追加構造体
struct PS_IN_TOON
{
    float4 Position : SV_POSITION; // スクリーン空間の位置
    float4 WorldPos : TEXCOORD0; // ワールド空間の位置
    float3 Normal : TEXCOORD1; // 法線
    float4 Diffuse : COLOR0; // モデルの色
    float4 LightSpacePos : TEXCOORD2; // ライト空間の位置(シャドウ用)
};