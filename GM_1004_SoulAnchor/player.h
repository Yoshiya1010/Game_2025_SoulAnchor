#pragma once

#include "main.h"
#include "renderer.h"
#include "gameObject.h"

class Player :public GameObject {
private:
	ID3D11VertexShader* m_VertexShader;	// 頂点シェーダーオブジェクト
	ID3D11PixelShader* m_PixelShader;		// ピクセルシェーダーオブジェクト
	ID3D11InputLayout* m_VertexLayout;	// 頂点レイアウトオブジェクト

	class ModelRenderer* m_ModelRenderer; // 前方宣言
	class AnimationModel* m_AnimationModel;

	class Audio* m_SE;

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