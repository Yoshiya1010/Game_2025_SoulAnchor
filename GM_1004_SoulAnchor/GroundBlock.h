#pragma once

#include "main.h"
#include "renderer.h"
#include "gameObject.h"
#include <btBulletDynamicsCommon.h>
#include"animationModel.h"
#include<memory>



class GroundBlock :public GameObject {
private:
	ID3D11VertexShader* m_VertexShader;	// 頂点シェーダーオブジェクト
	ID3D11PixelShader* m_PixelShader;		// ピクセルシェーダーオブジェクト
	ID3D11InputLayout* m_VertexLayout;	// 頂点レイアウトオブジェクト


	//モデル
	unique_ptr<AnimationModel> m_ModelRenderer = nullptr;
	const float m_modelScale = 2.0f;

public:
	void Init() override;
	void Start()override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	json ToJson() const override;
	void FromJson(const json& j);
};