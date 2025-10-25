#pragma once


#include "main.h"
#include "renderer.h"
#include "gameObject.h"
#include"audio.h"
#include"animationModel.h"
#include"modelRenderer.h"
#include"PhysicsObject.h"

class FPSPlayer :public PhysicsObject {
private:
	ID3D11VertexShader* m_VertexShader;	// 頂点シェーダーオブジェクト
	ID3D11PixelShader* m_PixelShader;		// ピクセルシェーダーオブジェクト
	ID3D11InputLayout* m_VertexLayout;	// 頂点レイアウトオブジェクト

	
	unique_ptr<AnimationModel> m_AnimationModel;

	unique_ptr<Audio> m_SE;

	unsigned int m_Frame;

	std::string m_AnimationName;
	std::string m_AnimationNameNext;
	float m_AnimationBlend;


	const float m_modelScale = 0.015;

	bool m_IsOnGround = false;
public:
	void SetOnGround(bool onGround) { m_IsOnGround = onGround; }
	bool IsOnGround() const { return m_IsOnGround; }

	void Init() override;
	void Start()override;
	void Uninit() override;
	void Update() override;
	void Draw() override;
};