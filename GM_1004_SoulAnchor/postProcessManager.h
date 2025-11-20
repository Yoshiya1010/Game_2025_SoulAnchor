#pragma once
#include <d3d11.h>
#include <vector>
#include <DirectXMath.h>

using namespace DirectX;

enum class PostEffectType
{
    VIGNETTE,
    BLOOM,
    BLUR
};


struct VignetteParams
{
    float intensity;  // ビネット強度
    float smoothness; // ぼかし範囲
    float radius;     // 効果半径
    float padding;    // 16バイトアライメント用
};

class PostProcessManager
{
private:
    // ピンポンバッファ (2つのテクスチャを交互に使用)
    static ID3D11Texture2D* m_RenderTexture[2];
    static ID3D11RenderTargetView* m_RenderTargetView[2];
    static ID3D11ShaderResourceView* m_ShaderResourceView[2];

    // フルスクリーンクアッド用
    static ID3D11Buffer* m_VertexBuffer;
    static ID3D11VertexShader* m_FullscreenVS;
    static ID3D11InputLayout* m_VertexLayout;

    // 各エフェクトのピクセルシェーダー
    static ID3D11PixelShader* m_VignettePS;
    static ID3D11PixelShader* m_BloomPS;
    static ID3D11PixelShader* m_BlurPS;

    static ID3D11SamplerState* m_SamplerState;

    static std::vector<PostEffectType> m_Effects;
    static int m_CurrentBuffer;

    static void ApplyEffect(PostEffectType type);
    static void DrawFullscreenQuad();
    static void SwapBuffers();


    static ID3D11Buffer* m_VignetteParamBuffer;
    static VignetteParams m_VignetteParams;
public:
    static void Init();
    static void Uninit();

    static void BeginCapture();
    static void EndCapture();
    static void ApplyEffects();

    static void AddEffect(PostEffectType type);
    static void ClearEffects();

    static VignetteParams& GetVignetteParams() { return m_VignetteParams; }
    static void UpdateVignetteParams();
};