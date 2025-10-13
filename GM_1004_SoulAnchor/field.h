#pragma once

#include "main.h"
#include "renderer.h"
#include "gameObject.h"

class Field :public GameObject {

private:
	ID3D11Buffer* m_VertexBuffer;

	ID3D11VertexShader* m_VertexShader;	// 頂点シェーダーオブジェクト
	ID3D11PixelShader* m_PixelShader;		// ピクセルシェーダーオブジェクト
	ID3D11InputLayout* m_VertexLayout;	// 頂点レイアウトオブジェクト

	ID3D11ShaderResourceView* m_Texture; // テクスチャ

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;
};