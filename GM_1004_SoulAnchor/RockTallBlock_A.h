#pragma once


#include "main.h"
#include "renderer.h"
#include "gameObject.h"
#include"modelRenderer.h"
#include<memory>
#include"PhysicsObject.h"



class RockTallBlock_A :public PhysicsObject {
private:
	ID3D11VertexShader* m_VertexShader;	// 頂点シェーダーオブジェクト
	ID3D11PixelShader* m_PixelShader;		// ピクセルシェーダーオブジェクト
	ID3D11InputLayout* m_VertexLayout;	// 頂点レイアウトオブジェクト


	//モデル
	class ModelRenderer* m_ModelRenderer; // 前方宣言
	const float m_modelScale = 2.0f;

public:
	void Init() override;
	void Start()override;
	void Uninit() override;
	void Update() override;
	void Draw() override;


};