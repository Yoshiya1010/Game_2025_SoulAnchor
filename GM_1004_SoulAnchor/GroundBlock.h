#pragma once

#include "main.h"
#include "renderer.h"
#include "gameObject.h"
#include <btBulletDynamicsCommon.h>
#include"modelRenderer.h"

class GroundBlock :public GameObject {
private:
	ID3D11VertexShader* m_VertexShader;	// 頂点シェーダーオブジェクト
	ID3D11PixelShader* m_PixelShader;		// ピクセルシェーダーオブジェクト
	ID3D11InputLayout* m_VertexLayout;	// 頂点レイアウトオブジェクト


	class ModelRenderer* m_ModelRenderer; // 前方宣言

public:
	void Init() override;
	void Start()override;
	void Uninit() override;
	void Update() override;
	void Draw() override;
};