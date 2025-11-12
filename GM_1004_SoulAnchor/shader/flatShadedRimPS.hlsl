// フラットシェーディング+リムライト用ピクセルシェーダー
#include "flatShadedCommon.hlsl"

float4 main(in PS_IN_FLAT input) : SV_TARGET
{
    // フラットシェーディング用の法線を動的に計算
    float3 flatNormal = -normalize(cross(
        ddx(input.WorldPosition.xyz),
        ddy(input.WorldPosition.xyz)
    ));
    
    // フラットシェーディングの法線が正しく計算できない場合は元の法線を使用
    // (ddx/ddyが0になる場合などのフォールバック)
    if (length(flatNormal) < 0.1)
    {
        flatNormal = normalize(input.Normal);
    }
    
    // ライトの方向ベクトルを正規化
    float3 lightDir = normalize(-Light.Direction.xyz);
    
    // ディフューズライティング（拡散反射）を計算
    float NdotL = dot(flatNormal, lightDir);
    
    // シャドウマッピング
    float shadow = 1.0;
    
    // ライト空間の座標を正規化
    float3 projCoords = input.ShadowPos.xyz / input.ShadowPos.w;
    
    // テクスチャ座標に変換 ([-1,1] -> [0,1])
    projCoords.x = projCoords.x * 0.5 + 0.5;
    projCoords.y = projCoords.y * -0.5 + 0.5;
    
    // 範囲内の場合のみシャドウチェック
    if (projCoords.x >= 0.0 && projCoords.x <= 1.0 &&
        projCoords.y >= 0.0 && projCoords.y <= 1.0 &&
        projCoords.z >= 0.0 && projCoords.z <= 1.0)
    {
        // シャドウマップから深度値を取得
        float shadowDepth = g_ShadowMap.Sample(g_SamplerState, projCoords.xy).r;
        
        // 現在のピクセルの深度
        float currentDepth = projCoords.z;
        
        // バイアスを追加して影のアーティファクトを防ぐ
        float bias = 0.005;
        
        // 深度比較
        if (currentDepth - bias > shadowDepth)
        {
            shadow = 0.5; // 影の中
        }
    }
    
    // ディフューズライティングの強さを計算
    float diffuse = saturate(NdotL);
    
    // ライティングとシャドウを合成
    float lighting = diffuse * shadow;
    
    // 環境光
    float3 ambient = Light.Ambient.rgb * 0.5;
    
    // リムライトの計算
    float3 viewDir = normalize(CameraPosition.xyz - input.WorldPosition.xyz);
    float rim = 1.0 - saturate(dot(viewDir, flatNormal));
    rim = pow(rim, 3.0);
    
    // リムライトの色
    float3 rimColor = float3(0.1, 0.1, 0.1) * rim;
    
    // 最終カラー
    float4 outColor;
    outColor.rgb = input.Diffuse.rgb * (lighting + ambient);
    outColor.rgb *= (1.0 + rimColor);
    outColor.a = input.Diffuse.a;
    
    return outColor;
}