#include "toonShadowCommon.hlsl"

// シンプルな出力構造体
struct VS_OUT_SHADOW
{
    float4 Position : SV_POSITION;
    float Depth : TEXCOORD0;
};

void main(in VS_IN In, out VS_OUT_SHADOW Out)
{
    // ワールド変換
    float4 worldPos = mul(In.Position, World);
    
    // ライト空間への変換
    Out.Position = mul(worldPos, LightViewProjection);
    
    // 深度値を保存
    Out.Depth = Out.Position.z / Out.Position.w;
}