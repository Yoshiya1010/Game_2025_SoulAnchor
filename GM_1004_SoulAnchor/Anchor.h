#pragma once

#include"main.h"
#include "renderer.h"
#include "gameObject.h"
#include<memory>
#include"PhysicsObject.h"
#include"chainSystem.h"
#include"modelRenderer.h"

class Anchor :public PhysicsObject {
private:
	ID3D11VertexShader* m_VertexShader;	// 頂点シェーダーオブジェクト
	ID3D11PixelShader* m_PixelShader;		// ピクセルシェーダーオブジェクト
	ID3D11InputLayout* m_VertexLayout;	// 頂点レイアウトオブジェクト


	//モデル
	unique_ptr<ModelRenderer> m_ModelRenderer = nullptr;


	Vector3 m_PendingVelocity = { 0,0,0 }; // まだRigidBodyがない場合の一時保存
	Vector3 m_PendingRotation = { 0,0,0 }; // まだRigidBodyがない場合の回転一時保存
private:
	// アンカーの状態
	bool m_Attached = false;
	GameObject* m_AttachedTarget = nullptr;
	btPoint2PointConstraint* m_Joint = nullptr;

	// プレイヤーへの参照（引き寄せ用）
	GameObject* m_Owner = nullptr;

	// 引き寄せパラメータ
	float m_PullForce = 1000.0f;      // 引き寄せる力
	float m_PullDistance = 10.0f;     // この距離以下になったら引き寄せ完了
	bool m_IsPulling = false;        // 引き寄せ中かどうか

	bool m_PullingSelf = false;//静的オブジェクトにぶつかった

	// チェーンシステム
	ChainSystem* m_ChainSystem = nullptr;

public:
	void Init() override;
	void Start()override;
	void Uninit() override;
	void Update() override;
	void Draw() override;


	// 衝突時の処理
	void OnCollisionEnter(GameObject* other, const Vector3& hitPoint)override;

	// 物体同士をジョイント
	void AttachTo(GameObject* target, const Vector3& hitPoint);

	// ジョイントを削除
	void Detach();

	// アンカーの所有者（プレイヤー）を設定
	void SetOwner(GameObject* owner) { m_Owner = owner; }

	// 引き寄せ開始
	void StartPulling();

	// 引き寄せ停止
	void StopPulling();

	//オーナーにアンカーの削除を伝える
	void NotifyOwnerAnchorRemoved();

	// 接続状態の取得
	bool IsAttached() const { return m_Attached; }
	bool IsPulling() const { return m_IsPulling; }

	// 速度設定（投擲時用）
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

	// 回転設定（投擲時用）- SetRotationをオーバーライド
	void SetRotation(Vector3 Rotation)
	{
		m_Rotation = Rotation;
		if (m_RigidBody)
		{
			// Rigidbodyが存在すれば物理エンジンに即反映
			SyncToPhysics();
		}
		else
		{
			// Start()後に適用するために保持しておく
			m_PendingRotation = Rotation;
		}
	}

	// 引き寄せパラメータの設定
	void SetPullForce(float force) { m_PullForce = force; }
	void SetPullDistance(float distance) { m_PullDistance = distance; }
};