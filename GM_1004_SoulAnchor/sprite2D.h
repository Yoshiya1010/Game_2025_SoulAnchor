#pragma once
#include "main.h"
#include "renderer.h"
#include "textureManager.h"

class Sprite2D
{
private:
    ID3D11Buffer* m_VertexBuffer;

    ID3D11VertexShader* m_VertexShader;	// 頂点シェーダーオブジェクト
    ID3D11PixelShader* m_PixelShader;	// ピクセルシェーダーオブジェクト
    ID3D11InputLayout* m_VertexLayout;	// 頂点レイアウトオブジェクト

    ID3D11ShaderResourceView* m_Texture; // テクスチャ

    float m_X = 0, m_Y = 0;
    float m_Width = 0, m_Height = 0;

public:
    Sprite2D() {}
    ~Sprite2D() { Uninit(); }

    void Init(float x, float y, float width, float height, const char* fileName);
    void Uninit();
    void Draw();

    void SetPos(float x, float y) { m_X = x; m_Y = y; }
    void SetSize(float w, float h) { m_Width = w; m_Height = h; RebuildVertex(); }

private:
    void RebuildVertex();
};
