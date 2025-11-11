#pragma once


#include"main.h"
#include "renderer.h"
#include "gameObject.h"
#include"ModelFBX.h"
#include<memory>
#include"fragmentObject.h"



class TreeBlock :public FragmentObject {
private:
	ID3D11VertexShader* m_VertexShader;	// 頂点シェーダーオブジェクト
	ID3D11PixelShader* m_PixelShader;		// ピクセルシェーダーオブジェクト
	ID3D11InputLayout* m_VertexLayout;	// 頂点レイアウトオブジェクト


	// シャドウマップ生成用シェーダー
	ID3D11VertexShader* m_ShadowMapVS;
	ID3D11PixelShader* m_ShadowMapPS;
	ID3D11InputLayout* m_ShadowMapLayout;



public:
	void Init() override;
	void Start()override;
	void Uninit() override;
	void Update() override;
	void Draw() override;


	// シャドウマップ用の描画
	void DrawShadowMap();
};