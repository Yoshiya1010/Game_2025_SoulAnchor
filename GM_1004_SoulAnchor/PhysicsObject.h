#pragma once
#include"gameObject.h"
#include"PhysicsManager.h"
#include <btBulletDynamicsCommon.h>

enum CollisionGroup {
    COL_NOTHING = 0,
    COL_PLAYER = 1 << 0,
    COL_ENEMY = 1 << 1,
    COL_SHIP = 1 << 2,
    COL_BULLET = 1 << 3,
    COL_WALL = 1 << 4,
    COL_GROUND = 1 << 5,
    COL_DEFAULT = 1 << 6
};
class PhysicsObject : public GameObject {
protected:
    std::unique_ptr<btRigidBody> m_RigidBody;
    std::unique_ptr<btCollisionShape> m_CollisionShape;
    std::unique_ptr<btDefaultMotionState> m_MotionState;
    Vector3 m_ColliderOffset{ 0, 0, 0 };

    CollisionGroup m_CollisionGroup = COL_DEFAULT;
    int m_CollisionMask = -1;

    float m_mass=0;
public:
    virtual ~PhysicsObject() = default;

    // 剛体生成 ここでボディを作ってる
    void CreateRigidBody(float mass) {

        //質量一旦保存
        this->m_mass = mass;

        auto* world = PhysicsManager::GetWorld();
        if (!m_CollisionShape || !world) return;

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(m_Position.x, m_Position.y, m_Position.z));

        btVector3 inertia(0, 0, 0);
        if (mass != 0.0f)
            m_CollisionShape->calculateLocalInertia(mass, inertia);

        m_MotionState = std::make_unique<btDefaultMotionState>(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, m_MotionState.get(), m_CollisionShape.get(), inertia);
        m_RigidBody = std::make_unique<btRigidBody>(rbInfo);
        m_RigidBody->setUserPointer(this);

        world->addRigidBody(m_RigidBody.get(), m_CollisionGroup, m_CollisionMask);
    }

    // 形状生成
    void CreateBoxCollider(Vector3 halfSize, float mass = 0.0f) {
        m_CollisionShape = std::make_unique<btBoxShape>(btVector3(halfSize.x, halfSize.y, halfSize.z));
        CreateRigidBody(mass);
    }

    void CreateSphereCollider(float radius, float mass = 0.0f) {
        m_CollisionShape = std::make_unique<btSphereShape>(radius);
        CreateRigidBody(mass);
    }

    void CreateCapsuleCollider(float radius, float height, float mass = 0.0f)
    {
        m_CollisionShape = std::make_unique<btCapsuleShape>(radius, height);
        CreateRigidBody(mass);
    }

    // 同期関数
    void SyncFromPhysics() {
        if (m_RigidBody && m_RigidBody->getMotionState()) {
            btTransform transform;
            m_RigidBody->getMotionState()->getWorldTransform(transform);
            btVector3 origin = transform.getOrigin();
            m_Position = { origin.getX(), origin.getY(), origin.getZ() };
        }
    }

    void SyncToPhysics() {
        if (m_RigidBody) {
            btTransform transform;
            transform.setIdentity();
            transform.setOrigin(btVector3(m_Position.x, m_Position.y, m_Position.z));
            m_RigidBody->setWorldTransform(transform);
        }
    }

    // 終了処理（物理のみ）
    virtual void Uninit() override {
        auto* world = PhysicsManager::GetWorld();
        if (m_RigidBody && world) {
            world->removeRigidBody(m_RigidBody.get());
            m_RigidBody->setUserPointer(nullptr);
        }
        m_RigidBody.reset();
        m_MotionState.reset();
        m_CollisionShape.reset();
    }


    XMMATRIX UpdatePhysicsWithModel(float ModelScale)
    {
        if (!m_RigidBody || !m_RigidBody->getMotionState()) return XMMatrixIdentity();

        // 物理エンジンから位置を手動で取得
        btTransform trans = m_RigidBody->getCenterOfMassTransform();
        m_Position.x = trans.getOrigin().getX();
        m_Position.y = trans.getOrigin().getY();
        m_Position.z = trans.getOrigin().getZ();


        btQuaternion quaternion = m_RigidBody->getCenterOfMassTransform().getRotation();

        XMVECTOR rotationQuaternion = XMVectorSet(
            quaternion.x(),
            quaternion.y(),
            quaternion.z(),
            quaternion.w()
        );

        XMMATRIX S_p = XMMatrixScaling(m_Scale.x*ModelScale, m_Scale.y* ModelScale, m_Scale.z * ModelScale);
        XMMATRIX R_p = XMMatrixRotationQuaternion(rotationQuaternion);
        XMMATRIX T_p = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

        XMMATRIX parentWorld = S_p * R_p * T_p;

        return parentWorld;
    }

    // Rigtbody　のゲッター
    btRigidBody* GetRigidBody() const { return m_RigidBody.get(); }

    //衝突のレイヤーを設定してる
    //これで衝突しないとかのレイヤー分けができる
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



    
    // コライダー生成関数群

    //Box
    void CreateBoxColliderAt(Vector3 position, Vector3 size, float mass = 0.0f, Vector3 offset = { 0, 0, 0 }) {
        m_Position = position;
        m_ColliderOffset = offset;
        m_CollisionShape = std::make_unique<btBoxShape>(btVector3(size.x, size.y, size.z));
        CreateRigidBody(mass);
    }

    //スフィア
    void CreateSphereColliderAt(Vector3 position, float radius, float mass = 0.0f, Vector3 offset = { 0, 0, 0 }) {
        m_Position = position;
        m_ColliderOffset = offset;
        m_CollisionShape = std::make_unique<btSphereShape>(radius);
        CreateRigidBody(mass);
    }

    //カプセル
    void CreateCapsuleColliderAt(Vector3 position, float radius, float height, float mass = 0.0f, Vector3 offset = { 0, 0, 0 }) {
        m_Position = position;
        m_ColliderOffset = offset;
        m_CollisionShape = std::make_unique<btCapsuleShape>(radius, height);
        CreateRigidBody(mass);
    }

private:


	//物理ボディを無効化（Nullチェックしてる）
	void DisablePhysicsBody() {
		auto* world = PhysicsManager::GetWorld();
		// 1. ワールドが有効で、リジッドボディが存在する場合
		if (world && m_RigidBody) {
			// ワールドからRigidBodyを削除
			world->removeRigidBody(m_RigidBody.get());

			// UserPointerを解除（Bulletの衝突コールバック対策）
			m_RigidBody->setUserPointer(nullptr);
		}

		//物理オブジェクトをまとめて解放
		m_RigidBody.reset();
		m_MotionState.reset();
		m_CollisionShape.reset();

	}

    //-------------------------------------
    //JSONのGameObject.hからの　物理持ってる時だけに追加してるやつ

    json ToJson() const {
        json j = GameObject::ToJson(); // 継承元の基本情報を継承

        if (m_RigidBody) {
            btScalar invMass = m_RigidBody->getInvMass();
            float mass = (invMass == 0.0f) ? 0.0f : 1.0f / invMass;
            std::string colliderType = GetShapeTypeName(m_CollisionShape.get());

            j["RigidBody"] = {
                { "Mass", mass },
                { "ColliderType", colliderType },
                { "ColliderOffset", { m_ColliderOffset.x, m_ColliderOffset.y, m_ColliderOffset.z } },
                { "CollisionGroup", (int)m_CollisionGroup },
                { "CollisionMask", m_CollisionMask }
            };
        }

        return j;
    }



    void FromJson(const json& j)
    {
        GameObject::FromJson(j);

        if (!j.contains("RigidBody")) return;
        const auto& rb = j["RigidBody"];

        float mass = rb.value("Mass", 0.0f);
        std::string colliderType = rb.value("ColliderType", "Box");

        Vector3 size{ 1.0f, 1.0f, 1.0f };
        if (j.contains("Scale"))
            size = { j["Scale"][0], j["Scale"][1], j["Scale"][2] };

        if (colliderType == "Box")
            CreateBoxCollider(size, mass);
        else if (colliderType == "Sphere")
            CreateSphereCollider(size.x, mass);
        else if (colliderType == "Capsule")
            CreateCapsuleCollider(size.x, size.y, mass);
        
    }


    static std::string GetShapeTypeName(const btCollisionShape* shape)
    {
        if (!shape) return "None";

        switch (shape->getShapeType())
        {
        case BOX_SHAPE_PROXYTYPE:
            return "Box";
        case SPHERE_SHAPE_PROXYTYPE:
            return "Sphere";
        case CAPSULE_SHAPE_PROXYTYPE:
            return "Capsule";
        case CYLINDER_SHAPE_PROXYTYPE:
            return "Cylinder";
        case CONE_SHAPE_PROXYTYPE:
            return "Cone";
        default:
            return "Unknown";
        }
    }


};