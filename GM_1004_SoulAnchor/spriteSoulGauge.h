#pragma once
#include "main.h"
#include "renderer.h"
#include "textureManager.h"
#include "gameObject.h"

class SpriteSoulGauge : public GameObject
{
private:
    ID3D11Buffer* m_VertexBuffer = nullptr;
    ID3D11VertexShader* m_VertexShader = nullptr;
    ID3D11PixelShader* m_PixelShader = nullptr;
    ID3D11InputLayout* m_VertexLayout = nullptr;
    ID3D11ShaderResourceView* m_Texture1 = nullptr;
    ID3D11ShaderResourceView* m_Texture2 = nullptr;
    ID3D11ShaderResourceView* m_Texture3 = nullptr;

    float m_Width = 0.f;
    float m_Height = 0.f;
    bool  m_DrawFlag = true;

    float m_Value = 0.f;
    float m_TargetValue = 0.f;
    float m_Speed = 15.f;
    bool  m_Smooth = true;

    // 頂点を現在値で更新（動的VB）
    void UpdateGeometry();

public:
    ~SpriteSoulGauge() { Uninit(); }

  
    void Init(float x, float y, float width, float height, const char* fileName1, const char* fileName2, const char* fileName3);
    void Init(){} // 未使用
    void Uninit() override;
    void Update() override; 
    void Draw() override;


    void SetTargetValue(float percent); // ゆっくり目標へ
    float GetTargetValue() { return m_TargetValue; }
    void SetSpeed(float percentPerSec) { m_Speed = percentPerSec; }
    


    // 0～100 にクランプして反映
    void SetValue(float percent);
    float GetValue() { return m_Value; }
    void SetDrawFlag(bool f) { m_DrawFlag = f; }
};
