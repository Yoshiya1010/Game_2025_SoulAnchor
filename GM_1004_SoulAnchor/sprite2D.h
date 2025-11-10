#pragma once
#include "main.h"
#include "renderer.h"
#include "textureManager.h"
#include"gameObject.h"

class Sprite2D :public GameObject
{
private:
    ID3D11Buffer* m_VertexBuffer;

    ID3D11VertexShader* m_VertexShader;	// 頂点シェーダーオブジェクト
    ID3D11PixelShader* m_PixelShader;	// ピクセルシェーダーオブジェクト
    ID3D11InputLayout* m_VertexLayout;	// 頂点レイアウトオブジェクト

    ID3D11ShaderResourceView* m_Texture; // テクスチャ

    

    

public:
    Sprite2D() {}
    ~Sprite2D() { Uninit(); }

    void Init(float x, float y, float width, float height, const char* fileName);
    void Init() {};
    void Uninit() override;
    void Update() override;
    void Draw() override;

  
   


   

private:
    
};
