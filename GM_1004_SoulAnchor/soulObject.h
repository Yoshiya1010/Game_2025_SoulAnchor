#pragma once


#include"main.h"
#include "renderer.h"
#include "gameObject.h"
#include<memory>
#include"physicsObject.h"



class SoulObject :public PhysicsObject {

private:
	ID3D11Buffer* m_VertexBuffer;

	ID3D11VertexShader* m_VertexShader;	// 頂点シェーダーオブジェクト
	ID3D11PixelShader* m_PixelShader;		// ピクセルシェーダーオブジェクト
	ID3D11InputLayout* m_VertexLayout;	// 頂点レイアウトオブジェクト

	ID3D11ShaderResourceView* m_Texture; // テクスチャ

	int   m_Cols = 1;
	int   m_Rows = 1;
	float m_Frame = 0.0f;
	float m_FrameSpeed = 1.0f;

	int m_Soul_Level = 1;
	


public:
	void Init() override;
	void Start()override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void OnTriggerEnter(GameObject* other) override;


	void SetSoulLevel(int level)
	{
		m_Soul_Level = level;
	}

	int GetSoulLevel(void) { return m_Soul_Level; };


	void GetSoulGauge();

};