#pragma once

#include "main.h"
#include "renderer.h"

#include "gameObject.h"

class Polygon2D : public GameObject{

private:
	ID3D11Buffer* m_VertexBuffer;

	ID3D11VertexShader* m_VertexShader;	// 頂点シェーダーオブジェクト
	ID3D11PixelShader* m_PixelShader;		// ピクセルシェーダーオブジェクト
	ID3D11InputLayout* m_VertexLayout;	// 頂点レイアウトオブジェクト

	ID3D11ShaderResourceView* m_Texture; // テクスチャ

public:
	void Init() {}
	void Init(float x, float y, float width, float height, const char* FileName);
	void Uninit() override;
	void Update() override;
	void Draw() override;
};