// フラットシェーディング+リムライト用頂点シェーダー
#include "flatShadedCommon.hlsl"

void main(in VS_IN input, out PS_IN_FLAT output)
{
    // ワールド変換
    float4 worldPos = mul(input.Position, World);
    output.WorldPosition = worldPos;
    
    // ビュー,プロジェクション変換
    matrix vp = mul(View, Projection);
    output.Position = mul(worldPos, vp);
    
    // 法線をワールド空間に変換
    output.Normal = normalize(mul(input.Normal.xyz, (float3x3) World));
    
    // モデルの色を取得（マテリアルの色も反映）
    output.Diffuse = input.Diffuse * Material.Diffuse;
    
    // テクスチャ座標
    output.TexCoord = input.TexCoord;
    
    // ライト空間の位置を計算（シャドウ処理に使う）
    output.ShadowPos = mul(worldPos, LightViewProjection);
}