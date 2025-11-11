#pragma once


#include "main.h"
#include "renderer.h"
#include "gameObject.h"
#include <memory>
#include"modelRenderer.h"

// 太陽クラス - ディレクショナルライトとして機能
class Sun : public GameObject
{
private:
    // モデル表示用のシェーダー
    ID3D11VertexShader* m_VertexShader;
    ID3D11PixelShader* m_PixelShader;
    ID3D11InputLayout* m_VertexLayout;

    // 太陽のパラメータ
    Vector3 m_LightDirection;    // ライトの方向
    Vector3 m_TargetPosition;    // 見ている場所
    float m_Intensity;           // 光の強さ
    float m_AmbientStrength;     // 環境光の強さ
    float m_OrthoSize;           // シャドウマップの範囲

    // 表示用モデル
    std::unique_ptr<ModelRenderer> m_ModelRenderer;
    float m_ModelScale;

    // ImGui用
    bool m_ShowImGui;

public:
    Sun() : m_ShowImGui(true) {}

    void Init() override;
    void Uninit() override;
    void Update() override;
    void Draw() override;

    // ライトの設定をRendererに反映
    void ApplyLight();

    // シャドウマップ用の行列を取得
    XMMATRIX GetLightViewMatrix();
    XMMATRIX GetLightProjectionMatrix();
    XMMATRIX GetLightViewProjectionMatrix();

    // ゲッター
    Vector3 GetLightDirection() const { return m_LightDirection; }
    Vector3 GetTargetPosition() const { return m_TargetPosition; }
    float GetOrthoSize() const { return m_OrthoSize; }
};