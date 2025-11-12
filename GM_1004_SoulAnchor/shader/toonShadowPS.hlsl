// Toonシェーダー用のピクセルシェーダー
// 段階的な陰影とシャドウマッピングを実装

#include "toonShadowCommon.hlsl"

// シャドウマップのテクスチャ
Texture2D g_ShadowMap : register(t0);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN_TOON In, out float4 outColor : SV_Target)
{
    // 法線を正規化
    float3 normal = normalize(In.Normal);
    
    // ライトの方向を正規化
    float3 lightDir = normalize(-Light.Direction.xyz);
    
    // 拡散反射の計算
    float NdotL = dot(normal, lightDir);
    
    // Toon効果: 段階的な明るさに変換
    float toonLevel;
    if (NdotL > 0.8)
        toonLevel = 1.0; // 最も明るい
    else if (NdotL > 0.5)
        toonLevel = 0.7; // 中間の明るさ
    else if (NdotL > 0.2)
        toonLevel = 0.4; // 暗い
    else
        toonLevel = 0.2; // 最も暗い
    
    // シャドウマッピング
    float shadow = 1.0;
    
    // ライト空間の座標を正規化
    float3 projCoords = In.LightSpacePos.xyz / In.LightSpacePos.w;
    
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
    
    // 環境光
    float3 ambient = Light.Ambient.rgb * 0.3;
    outColor.rgb = In.Diffuse.rgb * (toonLevel * shadow + ambient);
    outColor.a = In.Diffuse.a;
}