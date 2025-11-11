// シャドウマップ生成用のピクセルシェーダー
// 深度値をテクスチャに書き込む

struct PS_IN_SHADOW
{
    float4 Position : SV_POSITION;
    float Depth : TEXCOORD0;
};

void main(in PS_IN_SHADOW In, out float4 outColor : SV_Target)
{
    // 深度値をそのまま出力
    outColor = float4(In.Depth, In.Depth, In.Depth, 1.0);
}