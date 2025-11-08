#pragma once
#include "gameObject.h"
#include "renderer.h"
#include "textureManager.h"

class SpriteAnimator2D : public GameObject
{
private:
    ID3D11Buffer* m_VertexBuffer = nullptr;
    ID3D11ShaderResourceView* m_Texture = nullptr;
    ID3D11VertexShader* m_VertexShader = nullptr;
    ID3D11PixelShader* m_PixelShader = nullptr;
    ID3D11InputLayout* m_VertexLayout = nullptr;

    float m_X, m_Y, m_W, m_H;

    int m_Cols = 1;
    int m_Rows = 1;
    int m_Frame = 0;
    float m_FrameTime{ 0.1f };
    float m_Timer{ 0.0f };
    bool m_Playing = true;
public:
    void Init() {}
    void Init(float x, float y, float w, float h, const char* FileName, int cols, int rows);
    void SetFrame(int frame) { m_Frame = frame; }
    int  GetFrame() const { return m_Frame; }

    void SetFrameSpeed(float secPerFrame) { m_FrameTime = secPerFrame; }

    void Update() override;
    void Draw() override;
    void Uninit() override;
};