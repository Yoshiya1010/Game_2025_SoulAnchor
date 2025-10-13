
#include "common.hlsl"

void main(in VS_IN In, out PS_IN Out)
{
    matrix wvp;
    // 頂点変換のための行列を作る（World * View * Projection）
    wvp = mul(World, View);
    wvp = mul(wvp, Projection);

    // ポリゴンの頂点を変換行列で変換して出力
    Out.Position = mul(In.Position, wvp);
    
    
    // 法線の計算
    float4 worldNormal, normal;
    normal = float4(In.Normal.xyz, 0.0f);   // 入力法線ベクトルのwを0としてコピー（平行移動しないため）
    worldNormal = mul(normal, World);       // 法線をワールド行列で回転
    worldNormal = normalize(worldNormal);   // 正規化
    //Out.n = worldNormal;
    
    float3 lightDirection = normalize(Light.Direction.xyz);
    
    // 光の計算
    float light = -dot(lightDirection, worldNormal.xyz);
    light = saturate(light); // 明るさを(0と1の間で調整する）
    
    // 明るさを頂点色として出力
    Out.Diffuse.rgb = light;
    Out.Diffuse.a = In.Diffuse.a;
    
    Out.TexCoord = In.TexCoord;
}

