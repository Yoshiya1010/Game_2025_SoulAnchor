#pragma once

#include"main.h"
#include "renderer.h"
#include "gameObject.h"
#include"ModelFBX.h"
#include<memory>
#include"PhysicsObject.h"
class Anchor :public PhysicsObject {
private:
	ID3D11VertexShader* m_VertexShader;	// 頂点シェーダーオブジェクト
	ID3D11PixelShader* m_PixelShader;		// ピクセルシェーダーオブジェクト
	ID3D11InputLayout* m_VertexLayout;	// 頂点レイアウトオブジェクト


	//モデル
	unique_ptr<StaticFBXModel> m_ModelRenderer = nullptr;
	const float m_modelScale = 1.0f;

	Vector3 m_PendingVelocity = { 0,0,0 }; // まだRigidBodyがない場合の一時保存

private:
	bool m_Attached = false;
	GameObject* m_AttachedTarget = nullptr;
	btPoint2PointConstraint* m_Joint = nullptr;

public:
	void Init() override;
	void Start()override;
	void Uninit() override;
	void Update() override;
	void Draw() override;


	void OnCollisionEnter(GameObject* other, const Vector3& hitPoint)override;

	//物体同士をジョイント
	void AttachTo(GameObject* target, const Vector3& hitPoint);

	//ジョイントを削除
	void Detach();

	bool IsAttached() const { return m_Attached; };


	void SetVelocity(const Vector3& vel)
	{
		if (m_RigidBody)
		{
			// Rigidbodyが存在すれば即反映
			m_RigidBody->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
			m_RigidBody->activate(true);
		}
		else
		{
			// Start()後に適用するために保持しておく
			m_PendingVelocity = vel;
		}
	}
};