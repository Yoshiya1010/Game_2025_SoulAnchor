#pragma once


#include "main.h"
#include "renderer.h"
#include "gameObject.h"
#include"modelRenderer.h"
#include<memory>
#include"fragmentObject.h"



class RockTallBlock_A :public FragmentObject {
private:
	ID3D11VertexShader* m_VertexShader;	// 頂点シェーダーオブジェクト
	ID3D11PixelShader* m_PixelShader;		// ピクセルシェーダーオブジェクト
	ID3D11InputLayout* m_VertexLayout;	// 頂点レイアウトオブジェクト





	

public:
	void Init() override;
	void Start()override;
	void Uninit() override;
	void Update() override;
	void Draw() override;


	

};