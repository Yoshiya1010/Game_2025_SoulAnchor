#pragma once

#include "main.h"
#include"main.h"
#include "renderer.h"
#include "gameObject.h"
#include"ModelFBX.h"
#include<memory>
#include"PhysicsObject.h"



class TreeBlock :public PhysicsObject {
private:
	ID3D11VertexShader* m_VertexShader;	// 頂点シェーダーオブジェクト
	ID3D11PixelShader* m_PixelShader;		// ピクセルシェーダーオブジェクト
	ID3D11InputLayout* m_VertexLayout;	// 頂点レイアウトオブジェクト


	//モデル
	unique_ptr<StaticFBXModel> m_ModelRenderer = nullptr;
	const float m_modelScale = 1.0f;

public:
	void Init() override;
	void Start()override;
	void Uninit() override;
	void Update() override;
	void Draw() override;


};