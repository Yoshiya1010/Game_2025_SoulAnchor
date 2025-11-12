#include "toonShadowCommon.hlsl"
void main(in VS_IN In, out PS_IN_TOON Out)
{
    // ワールド変換
    float4 worldPos = mul(In.Position, World);
    Out.WorldPos = worldPos;
    
    //ビュー,プロジェクション変換
    matrix vp = mul(View, Projection);
    Out.Position = mul(worldPos, vp);
    
    //法線をワールド空間に変換
    Out.Normal = normalize(mul(In.Normal.xyz, (float3x3) World));
    
    //モデルの色を取得  もともとのモデルの色を持っておく
    Out.Diffuse = In.Diffuse * Material.Diffuse;
    
    //ライト空間の位置を計算(シャドウ処理に使う)
    Out.LightSpacePos = mul(worldPos, LightViewProjection);
}