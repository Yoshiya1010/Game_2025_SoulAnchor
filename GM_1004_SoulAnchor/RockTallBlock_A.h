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


	// 破壊機能関連
	bool m_Destructible = true;                       // 破壊可能かどうか
	float m_DestructionThreshold = 15.0f;             // 破壊に必要な衝撃速度
	bool m_IsDestroyed = false;                       // 既に破壊されたか

public:
	void Init() override;
	void Start()override;
	void Uninit() override;
	void Update() override;
	void Draw() override;


	// 衝突時のコールバック
	void OnCollisionEnter(GameObject* other, const Vector3& hitPoint) override;

	// 破壊処理
	void DestroyRock(const Vector3& impactPoint);

	// 破壊設定
	void SetDestructible(bool destructible) { m_Destructible = destructible; }
	void SetDestructionThreshold(float threshold) { m_DestructionThreshold = threshold; }

};