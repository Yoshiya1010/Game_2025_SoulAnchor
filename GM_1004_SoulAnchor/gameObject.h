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


enum CollisionGroup {
	COL_NOTHING = 0,
	COL_PLAYER = 1 << 0,   // 1
	COL_ENEMY = 1 << 1,    // 2  
	COL_SHIP = 1 << 2,     // 4
	COL_BULLET = 1 << 3,   // 8
	COL_WALL = 1 << 4,     // 16
	COL_GROUND = 1 << 5,   // 32
	COL_DEFAULT = 1 << 6   // 64 - デフォルト（全てと衝突）
};
class GameObject {
protected:

	

	// 基本トランスフォーム
	Vector3 m_Position{ 0.0f, 0.0f, 0.0f };
	Vector3 m_Rotation{ 0.0f, 0.0f, 0.0f }; 
	Vector3 m_Scale{ 1.0f, 1.0f, 1.0f };


	Vector3 m_FirstPosition{ 0.0f,0.0f,0.0f };
	Vector3 m_FirstRotation{ 0.0f,0.0f,0.0f };

	// ライフサイクル管理フラグ
	bool m_Started = false; // Start() が呼ばれたかどうか
	bool m_Destroy = false; // 破棄予約（Destroy() 時に delete this）


	// 物理（Bullet）関連
	unique_ptr<btRigidBody> m_RigidBody = nullptr; // 所有（Uninit で破棄）
	unique_ptr<btCollisionShape> m_CollisionShape = nullptr; // 所有（Uninit で破棄）
	Vector3           m_ColliderOffset{ 0.0f, 0.0f, 0.0f }; // 見た目中心に対するコライダーのオフセット

	//あたり判定に必要
	unique_ptr <btDefaultMotionState> m_MotionState = nullptr;
	// 識別・分類
	GameObjectTag m_Tag = GameObjectTag::None;
	std::string   m_Name = "";

	//レイヤー分け
	CollisionGroup m_CollisionGroup = COL_DEFAULT;
	int m_CollisionMask = -1; // デフォルトは全てと衝突
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

	// 終了処理（リソース解放）
	// ・Bullet のワールドから剛体を外す
	// ・MotionState / RigidBody / CollisionShape を delete
	virtual void Uninit() {
		auto* world = PhysicsManager::GetWorld();
		if (m_RigidBody && world) {
			world->removeRigidBody(m_RigidBody.get());
		}
		// 所有物をまとめて解放
		m_RigidBody.reset();
		m_MotionState.reset();
		m_CollisionShape.reset();
	}

	// 毎フレーム更新
	virtual void Update() {};
	

	// 描画（派生で実装）
	virtual void Draw() {}

	// 破棄予約
	void SetDestroy() {
		if (!m_Destroy) {
			m_Destroy = true;

			// ★ RigidBodyがある場合のみ物理無効化
			if (m_RigidBody) {
				DisablePhysicsBody();
			}
		}
	}

	// 破棄実行（予約時に Uninit → delete this）
	// 戻り値: 破棄したら true（呼び元の配列/リストからの除去に利用）
	bool Destroy() {
		if (m_Destroy) {
			Uninit();
			delete this;
			return true;
		}
		else {
			return false;
		}
	}

	// ----- コライダー作成（位置指定）---------------------------------------
	// 位置 position / サイズ size / 質量 mass / 見た目からの offset で Box コライダー生成
	void CreateBoxColliderAt(Vector3 position, Vector3 size, float mass = 0.0f, Vector3 offset = Vector3(0.0f, 0.0f, 0.0f)) {
		m_Position = position;
		m_ColliderOffset = offset;
		m_CollisionShape = make_unique<btBoxShape>(btVector3(size.x, size.y, size.z));
		CreateRigidBody(mass);
	}

	// Sphere コライダー生成
	void CreateSphereColliderAt(Vector3 position, float radius, float mass = 0.0f, Vector3 offset = Vector3(0.0f, 0.0f, 0.0f)) {
		m_Position = position;
		m_ColliderOffset = offset;
		m_CollisionShape = make_unique<btSphereShape>(radius);
		CreateRigidBody(mass);
	}

	// Capsule コライダー生成（height はシリンダー部の長さ）
	void CreateCapsuleColliderAt(Vector3 position, float radius, float height, float mass = 0.0f, Vector3 offset = Vector3(0.0f, 0.0f, 0.0f)) {
		m_Position = position;
		m_ColliderOffset = offset;
		m_CollisionShape = make_unique<btCapsuleShape>(radius, height);
		CreateRigidBody(mass);
	}

	// ----- 見た目 → 物理 同期 --------------------------------------------
	// ゲーム側の m_Position / m_Rotation / m_ColliderOffset を Bullet の Transform に反映
	// ・回転済みオフセットを考慮して剛体の原点へセット
	// ・回転は Euler（RollPitchYaw）→ Bullet へ setEulerZYX
	void SyncToPhysics() {
		if (m_RigidBody) {
			btTransform transform;
			transform.setIdentity();

			// 回転行列（DirectXMath）
			XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);

			// オフセットを回転させてから適用
			XMFLOAT3 offsetFloat3(m_ColliderOffset.x, m_ColliderOffset.y, m_ColliderOffset.z);
			XMVECTOR offsetVec = XMLoadFloat3(&offsetFloat3);
			XMVECTOR rotatedOffset = XMVector3Transform(offsetVec, rotMatrix);

			XMFLOAT3 rotatedOffsetFloat;
			XMStoreFloat3(&rotatedOffsetFloat, rotatedOffset);

			Vector3 colliderPos = m_Position + Vector3(rotatedOffsetFloat.x, rotatedOffsetFloat.y, rotatedOffsetFloat.z);
			transform.setOrigin(btVector3(colliderPos.x, colliderPos.y, colliderPos.z));

			// 回転（Bullet は ZYX の順で角を受け取る）
			btQuaternion rotation;
			rotation.setEulerZYX(m_Rotation.z, m_Rotation.y, m_Rotation.x);
			transform.setRotation(rotation);

			m_RigidBody->setWorldTransform(transform);
		}
	}

	// ----- 物理 → 見た目 同期 --------------------------------------------
	// Bullet の Transform から m_Position へ反映
	// ※ここでは「オフセットを引く」だけで回転は拾わない（意図そのまま）
	void SyncFromPhysics() {
		if (m_RigidBody && m_RigidBody->getMotionState()) {
			btTransform transform;
			m_RigidBody->getMotionState()->getWorldTransform(transform);

			btVector3 origin = transform.getOrigin();

			// 物理原点 ＝ 見た目原点 + オフセット（回転なし前提）
			m_Position.x = origin.getX() - m_ColliderOffset.x;
			m_Position.y = origin.getY() - m_ColliderOffset.y;
			m_Position.z = origin.getZ() - m_ColliderOffset.z;
		}
	}

	// 回転のみを物理へ反映（強制適用）
	// ・オブジェクトの見た目 m_Rotation を Bullet 側へ書き戻す
	// ・姿勢を物理に合わせたい/合わせさせたいときに使用
	void SyncRotationOnly() {
		if (m_RigidBody) {
			// 物理をアクティブ化（スリープ中でも更新されるように）
			m_RigidBody->activate();

			btTransform transform = m_RigidBody->getWorldTransform();

			// 回転（ZYX 順でセット）
			btQuaternion rotation;
			rotation.setEulerZYX(m_Rotation.z, m_Rotation.y, m_Rotation.x);
			transform.setRotation(rotation);

			// ※位置のオフセット再計算は既存コメント通り省略（元コードの意図を保持）
			m_RigidBody->setWorldTransform(transform);
			m_RigidBody->getMotionState()->setWorldTransform(transform);
		}
	}
	//剛体の取得
	btRigidBody* GetRigidBody() const { return m_RigidBody.get(); }
	btCollisionShape* GetCollisionShape() const { return m_CollisionShape.get(); }

	// ----- アクセサ（チェーン用に一部は this を返す） ---------------------
	Vector3 GetPosition() { return m_Position; }
	virtual GameObject* SetPosition(Vector3 Position) { m_Position = Position; return this; }

	Vector3 GetRotation() { return m_Rotation; }
	GameObject* SetRotation(Vector3 Rotation) { m_Rotation = Rotation; return this; }

	Vector3 GetScale() { return m_Scale; }
	GameObject* SetScale(Vector3 Scale) { m_Scale = Scale; return this; }

	// ローカル軸ベクトル取得（DirectXMath の回転行列 r[0]/r[1]/r[2] をそのまま使用）
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

	// ----- 衝突コールバック（必要に応じて派生でオーバーライド） ----------
	virtual void OnCollisionEnter(GameObject* other, const Vector3& hitPoint) {}
	virtual void OnCollisionStay(GameObject* other, const Vector3& hitPoint) {}
	virtual void OnCollisionExit(GameObject* other) {}

	// ----- ダメージ処理（必要なら派生で実装） ------------------------------
	virtual void TakeDamage(float damage, GameObject* attacker = nullptr) {
		// 基底では処理しない
	}

	// ----- タグ/名前 -------------------------------------------------------
	void SetTag(GameObjectTag tag) { m_Tag = tag; }
	GameObjectTag GetTag() const { return m_Tag; }
	void SetName(const std::string& name) { m_Name = name; }
	const std::string& GetName() const { return m_Name; }


	virtual void SetupCollisionLayer() {
		switch (m_Tag) {
		case GameObjectTag::Player:
			m_CollisionGroup = COL_PLAYER;
			m_CollisionMask = COL_ENEMY | COL_SHIP | COL_WALL | COL_GROUND | COL_DEFAULT;
			break;

		case GameObjectTag::Ship:
			m_CollisionGroup = COL_SHIP;
			m_CollisionMask = COL_PLAYER | COL_BULLET | COL_WALL | COL_GROUND | COL_DEFAULT;
			break;

		case GameObjectTag::PlayerBullet:
			m_CollisionGroup = COL_BULLET;
			m_CollisionMask = COL_ENEMY | COL_SHIP | COL_WALL | COL_DEFAULT;
			break;

		case GameObjectTag::EnemyBullet:
			m_CollisionGroup = COL_BULLET;
			m_CollisionMask = COL_PLAYER | COL_WALL | COL_DEFAULT;
			break;

		case GameObjectTag::Wall:
		case GameObjectTag::Ground:
			m_CollisionGroup = COL_WALL;
			m_CollisionMask = -1; // 全てと衝突
			break;

		default:
			// ★ デフォルト：全てと衝突（UI、エフェクト等）
			m_CollisionGroup = COL_DEFAULT;
			m_CollisionMask = -1;
			break;
		}
	}

	//  カスタムレイヤー設定（必要な時だけ呼ぶ）
	void SetCollisionLayer(CollisionGroup group, int mask) {
		m_CollisionGroup = group;
		m_CollisionMask = mask;
	}

	//  衝突無効化（UI要素等で使用）
	void DisableCollision() {
		m_CollisionGroup = COL_NOTHING;
		m_CollisionMask = 0;
	}
private:
	// 剛体生成の内部ヘルパ
	// ・m_CollisionShape / PhysicsManager::GetWorld が前提
	// ・回転済みオフセットを適用した位置に原点を取る
	// ・質量が 0 の場合は静的剛体
	void CreateRigidBody(float mass) {
		auto* world = PhysicsManager::GetWorld();
		if (!m_CollisionShape || !world) return;

		btTransform transform;
		transform.setIdentity();

		// 回転行列（見た目 Euler）
		XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);

		// 回転済みオフセット
		XMFLOAT3 offsetFloat3(m_ColliderOffset.x, m_ColliderOffset.y, m_ColliderOffset.z);
		XMVECTOR offsetVec = XMLoadFloat3(&offsetFloat3);
		XMVECTOR rotatedOffset = XMVector3Transform(offsetVec, rotMatrix);

		XMFLOAT3 rotatedOffsetFloat{};
		XMStoreFloat3(&rotatedOffsetFloat, rotatedOffset);

		Vector3 colliderPos = m_Position + Vector3(rotatedOffsetFloat.x, rotatedOffsetFloat.y, rotatedOffsetFloat.z);
		transform.setOrigin(btVector3(colliderPos.x, colliderPos.y, colliderPos.z));

		// 慣性（動的のみ）
		btVector3 inertia(0, 0, 0);
		if (mass != 0.0f) {
			m_CollisionShape->calculateLocalInertia(mass, inertia);
		}

		// MotionState / RigidBody 生成（所有）
		m_MotionState = std::make_unique<btDefaultMotionState>(transform);

		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, m_MotionState.get(), m_CollisionShape.get(), inertia);
		m_RigidBody = std::make_unique<btRigidBody>(rbInfo);
		m_RigidBody->setUserPointer(this);

		// タグに応じて自動設定
		SetupCollisionLayer();

		// ワールドに登録
		world->addRigidBody(m_RigidBody.get(), m_CollisionGroup, m_CollisionMask);


	}

	// 物理ボディを無効化（Nullチェック付き）
	void DisablePhysicsBody() {
		auto* world = PhysicsManager::GetWorld();
		// 1. ワールドが有効で、リジッドボディが存在する場合
		if (world && m_RigidBody) {
			// ワールドからRigidBodyを削除
			world->removeRigidBody(m_RigidBody.get());

			// UserPointerを解除（Bulletの衝突コールバック対策）
			m_RigidBody->setUserPointer(nullptr);
		}

		// 2. 物理オブジェクトをまとめて解放
		m_RigidBody.reset();
		m_MotionState.reset();
		m_CollisionShape.reset();

	}


	public:
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

};