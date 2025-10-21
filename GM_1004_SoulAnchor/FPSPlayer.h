#pragma once


#include "main.h"
#include "renderer.h"
#include "gameObject.h"
#include"audio.h"
#include"animationModel.h"
#include"modelRenderer.h"

class FPSPlayer :public GameObject {
private:
	ID3D11VertexShader* m_VertexShader;	// 頂点シェーダーオブジェクト
	ID3D11PixelShader* m_PixelShader;		// ピクセルシェーダーオブジェクト
	ID3D11InputLayout* m_VertexLayout;	// 頂点レイアウトオブジェクト

	unique_ptr <ModelRenderer> m_ModelRenderer; // 前方宣言
	unique_ptr<AnimationModel> m_AnimationModel;

	unique_ptr<Audio> m_SE;

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