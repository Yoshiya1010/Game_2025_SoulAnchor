#pragma once

#include "main.h"
#include "renderer.h"
#include"animationModel.h"
#include "gameObject.h"

class Enemy :public GameObject {
private:
	ID3D11VertexShader* m_VertexShader;	// 頂点シェーダーオブジェクト
	ID3D11PixelShader* m_PixelShader;		// ピクセルシェーダーオブジェクト
	ID3D11InputLayout* m_VertexLayout;	// 頂点レイアウトオブジェクト

	unique_ptr<AnimationModel> m_AnimationModel;


	unsigned int m_Frame;

	std::string m_AnimationName;
	std::string m_AnimationNameNext;

	float m_AnimationBlend;
public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;
};