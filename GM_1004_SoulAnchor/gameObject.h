#pragma once

#include"vector3.h"
#include"main.h"
#include"PhysicsManager.h"
#include <btBulletDynamicsCommon.h>
#include <DirectXMath.h>
#include<memory>
#include"json.hpp"
#include <iostream>
using std::unique_ptr;
using std::make_unique;

using json = nlohmann::json;

// オブジェクトタグの定義（フィルタリング/識別に使用）
enum class GameObjectTag {
	None,
	Player,
	Enemy,
	Ship,
	Bullet,
	PlayerBullet,
	EnemyBullet,
	Wall,
	Ground,
	Item
};



class GameObject {
protected:

	

	// 基本トランスフォーム
	Vector3 m_Position{ 0.0f, 0.0f, 0.0f };
	Vector3 m_Rotation{ 0.0f, 0.0f, 0.0f }; 
	Vector3 m_Scale{ 1.0f, 1.0f, 1.0f };

	//初期に持たせる　セーブとかにつかう
	Vector3 m_FirstPosition{ 0.0f,0.0f,0.0f };
	Vector3 m_FirstRotation{ 0.0f,0.0f,0.0f };

	// スタートを読んだか
	bool m_Started = false; 

	//　破壊予約フラグ
	bool m_Destroy = false; 

	std::string m_Name = "";
	GameObjectTag m_Tag = GameObjectTag::None;
public:
	// 初期化（生成直後に1回想定）
	virtual void Init() {}

	// 遅延初期化（最初の Update/Draw 前に1回だけ呼び出し）
	virtual void Start() {}

	// Start() を未実行の場合に呼び出すヘルパ
	void CheckAndCallStart() {
		if (!m_Started) {
			Start();
			m_Started = true;
		}
	}

	// 破棄関連
	void SetDestroy() { m_Destroy = true; }
	bool Destroy() const { return m_Destroy; }


	// 終了処理(派生クラス）でやってる
	virtual void Uninit() {}

	// 毎フレーム更新
	virtual void Update() {};
	

	// 描画（派生で実装）
	virtual void Draw() {}


	



	// ----- 衝突コールバック（必要に応じて派生でオーバーライド） ----------
	virtual void OnCollisionEnter(GameObject* other, const Vector3& hitPoint) {}
	virtual void OnCollisionStay(GameObject* other, const Vector3& hitPoint) {}
	virtual void OnCollisionExit(GameObject* other) {}


	// ----- タグ/名前 -------------------------------------------------------
	void SetTag(GameObjectTag tag) { m_Tag = tag; }
	GameObjectTag GetTag() const { return m_Tag; }
	void SetName(const std::string& name) { m_Name = name; }
	const std::string& GetName() const { return m_Name; }


	

	public:
		//Json周り
		virtual json ToJson() const {
			json j;
			j["Type"] = "GameObject";
			j["Name"] = m_Name;
			j["Position"] = { m_Position.x, m_Position.y, m_Position.z };
			j["Rotation"] = { m_Rotation.x, m_Rotation.y, m_Rotation.z };
			j["Scale"] = { m_Scale.x, m_Scale.y, m_Scale.z };
			return j;
		}


		virtual void FromJson(const json& j) {
			m_Name = j.value("Name", "");
			auto p = j["Position"];
			m_Position = { p[0], p[1], p[2] };

			auto r = j["Rotation"];
			m_Rotation = { r[0], r[1], r[2] };

			auto s = j["Scale"];
			m_Scale = { s[0], s[1], s[2] };
		}


		//---------------------------------------------------------------------------------------------

		Vector3 GetPosition() { return m_Position; }
		virtual GameObject* SetPosition(Vector3 Position) { m_Position = Position; return this; }

		Vector3 GetRotation() { return m_Rotation; }
		GameObject* SetRotation(Vector3 Rotation) { m_Rotation = Rotation; return this; }

		Vector3 GetScale() { return m_Scale; }
		GameObject* SetScale(Vector3 Scale) { m_Scale = Scale; return this; }

		//----------------------------------------------------------------------------------------------



	    // ローカル軸ベクトル取得 Bulletに依存しないやつら
		Vector3 GetRight() {
			XMMATRIX matrix = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
			Vector3 right;
			XMStoreFloat3((XMFLOAT3*)&right, matrix.r[0]);
			return right;
		}

		Vector3 GetUp() {
			XMMATRIX matrix = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
			Vector3 up;
			XMStoreFloat3((XMFLOAT3*)&up, matrix.r[1]);
			return up;
		}

		Vector3 GetForward() {
			XMMATRIX matrix = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
			Vector3 forward;
			XMStoreFloat3((XMFLOAT3*)&forward, matrix.r[2]);
			return forward;
		}

		// 任意位置との距離（ワールド空間）
		float GetDistance(Vector3 Position) {
			Vector3 direction = m_Position - Position;
			return direction.Length();
		}

};