#pragma once
#include"gameObject.h"
#include"PhysicsManager.h"
#include"main.h"

enum CollisionGroup {
    COL_NOTHING = 0,
    COL_PLAYER  = 1 << 0,
    COL_ENEMY   = 1 << 1,
    COL_WALL    = 1 << 2,
    COL_GROUND  = 1 << 3,
    COL_BULLET  = 1 << 4,
    COL_ANCHOR  = 1 << 5,
    COL_ITEM    = 1 << 6,
    COL_DEFAULT = 1 << 7,
};
class PhysicsObject : public GameObject {
protected:
    std::unique_ptr<btRigidBody> m_RigidBody;
    std::unique_ptr<btCollisionShape> m_CollisionShape;
    std::unique_ptr<btDefaultMotionState> m_MotionState;
    Vector3 m_ColliderOffset{ 0, 0, 0 };
    Vector3 m_OriginalColliderHalfSize{ 1,1,1 };

    CollisionGroup m_CollisionGroup = COL_DEFAULT;
    int m_CollisionMask = -1;

    float m_mass=0;

   
public:

    int m_RotationSyncCountdown = 0; // UI 編集後の優先同期フレーム数

    // Rigtbody　のゲッター
    btRigidBody* GetRigidBody() const { return m_RigidBody.get(); }

    float GetMass() { return m_mass; }
    virtual PhysicsObject* SetMass(float mass)
    {
        m_mass = mass;

        if (!m_RigidBody || !m_CollisionShape) return this;

        auto* world = PhysicsManager::GetWorld();
        if (!world) return this;

        // 一旦物理世界から削除
        world->removeRigidBody(m_RigidBody.get());

        btVector3 inertia(0, 0, 0);
        if (mass > 0.f)
            m_CollisionShape->calculateLocalInertia(mass, inertia);

        //剛体情報を再設定
        m_RigidBody->setMassProps(mass, inertia);
        m_RigidBody->updateInertiaTensor();

        // 速度をリセットして安定化
        m_RigidBody->setLinearVelocity(btVector3(0, 0, 0));
        m_RigidBody->setAngularVelocity(btVector3(0, 0, 0));

        //再登録
        world->addRigidBody(m_RigidBody.get(), m_CollisionGroup, m_CollisionMask);
        m_RigidBody->activate(true);

        return this;
    }

    // 終了処理（物理のみ）
    virtual void Uninit() override {

        auto* world = PhysicsManager::GetWorld();
        if (m_RigidBody && world) {
            // RigidBodyを削除する前に、このBodyに接続されている全てのConstraint（ジョイント）を削除
            int numConstraints = world->getNumConstraints();
            for (int i = numConstraints - 1; i >= 0; i--) {
                btTypedConstraint* constraint = world->getConstraint(i);

                // このRigidBodyに接続されているConstraintか確認
                if (&constraint->getRigidBodyA() == m_RigidBody.get() ||
                    &constraint->getRigidBodyB() == m_RigidBody.get()) {
                    world->removeConstraint(constraint);
                    delete constraint;
                }
            }

            // その後でRigidBodyを削除
            world->removeRigidBody(m_RigidBody.get());
            m_RigidBody->setUserPointer(nullptr);
        }
        m_RigidBody.reset();
        m_MotionState.reset();
        m_CollisionShape.reset();
    }



    virtual ~PhysicsObject() = default;

    //剛体生成ここでボディを作ってる
    void CreateRigidBody(float mass) {

        //質量一旦保存
        this->m_mass = mass;

        auto* world = PhysicsManager::GetWorld();
        if (!m_CollisionShape || !world) return;

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(m_Position.x+m_ColliderOffset.x,
                                      m_Position.y+m_ColliderOffset.y,
                                      m_Position.z+m_ColliderOffset.z));


        btQuaternion q;
        q.setEulerZYX(
            m_Rotation.y * DEG2RAD, // yaw
            m_Rotation.x * DEG2RAD, // pitch
            m_Rotation.z * DEG2RAD  // roll
        );
        transform.setRotation(q);

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

        m_OriginalColliderHalfSize = halfSize;

        // スケールを反映したサイズを算出
        Vector3 scaledHalfSize = {
            halfSize.x * m_Scale.x,
            halfSize.y * m_Scale.y,
            halfSize.z * m_Scale.z
        };

        // スケールを反映したオフセットも計算
        Vector3 scaledOffset = {
            m_ColliderOffset.x * m_Scale.x,
            m_ColliderOffset.y * m_Scale.y,
            m_ColliderOffset.z * m_Scale.z
        };

        m_CollisionShape = std::make_unique<btBoxShape>(
            btVector3(scaledHalfSize.x, scaledHalfSize.y, scaledHalfSize.z)
        );

        // Rigidbody生成（オフセットを内部で使う）
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

    void RecreateCollider()
    {
        if (!m_CollisionShape) return;
        auto* world = PhysicsManager::GetWorld();
        if (!world) return;

        // 既存の剛体を一旦削除
        if (m_RigidBody)
        {
            world->removeRigidBody(m_RigidBody.get());
            m_RigidBody->setUserPointer(nullptr);
        }

        // 形状タイプを調べて再生成
        std::string shapeType = GetShapeTypeName(m_CollisionShape.get());

        if (shapeType == "Box")
        {
           
            Vector3 scaledHalfSize = {
                m_OriginalColliderHalfSize.x * m_Scale.x,
                m_OriginalColliderHalfSize.y * m_Scale.y,
                m_OriginalColliderHalfSize.z * m_Scale.z
            };
            m_CollisionShape = std::make_unique<btBoxShape>(btVector3(
                scaledHalfSize.x,
                scaledHalfSize.y,
                scaledHalfSize.z
            ));
        }
        else if (shapeType == "Sphere")
            m_CollisionShape = std::make_unique<btSphereShape>(m_Scale.x);
        else if (shapeType == "Capsule")
            m_CollisionShape = std::make_unique<btCapsuleShape>(m_Scale.x, m_Scale.y);

        // 剛体を再登録
        CreateRigidBody(m_mass);
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
        if (!m_RigidBody) return;

        btTransform t = m_RigidBody->getWorldTransform();
        t.setOrigin(btVector3(m_Position.x, m_Position.y, m_Position.z));

        //Bulletは ZYX（yaw, pitch, roll）の順で受け取る
        btQuaternion q;
        q.setEulerZYX(
            m_Rotation.y * DEG2RAD, // yaw (Y)
            m_Rotation.x * DEG2RAD, // pitch (X)
            m_Rotation.z * DEG2RAD  // roll (Z)
        );
        t.setRotation(q);

        m_RigidBody->setWorldTransform(t);
        if (m_RigidBody->getMotionState())
            m_RigidBody->getMotionState()->setWorldTransform(t);

        m_RigidBody->activate(true);
    }

    




    XMMATRIX UpdatePhysicsWithModel(float ModelScale=1.0f)
    {
        if (!m_RigidBody || !m_RigidBody->getMotionState()) return XMMatrixIdentity();

        // 物理エンジンから位置を手動で取得
        btTransform trans = m_RigidBody->getCenterOfMassTransform();
        m_Position.x = trans.getOrigin().getX();
        m_Position.y = trans.getOrigin().getY();
        m_Position.z = trans.getOrigin().getZ();


        if (m_RotationSyncCountdown > 0)
        {
            m_RotationSyncCountdown--; // UI優先中
        }
        else
        {
            btQuaternion rot = trans.getRotation();
            btScalar yaw, pitch, roll;
            rot.getEulerZYX(yaw, pitch, roll);
            m_Rotation = Vector3((float)pitch, (float)yaw, (float)roll);
        }

        btQuaternion quaternion = m_RigidBody->getCenterOfMassTransform().getRotation();

        XMVECTOR rotationQuaternion = XMVectorSet(
            quaternion.x(),
            quaternion.y(),
            quaternion.z(),
            quaternion.w()
        );


        XMVECTOR offsetVec = {
        m_ColliderOffset.x * m_Scale.x * ModelScale,
        m_ColliderOffset.y * m_Scale.y * ModelScale,
        m_ColliderOffset.z * m_Scale.z * ModelScale,
        0.0f
        };

        offsetVec = XMVector3Rotate(offsetVec, rotationQuaternion); 

        // 回転を考慮した平行移動
        XMMATRIX S_p = XMMatrixScaling(m_Scale.x * ModelScale, m_Scale.y * ModelScale, m_Scale.z * ModelScale);
        XMMATRIX R_p = XMMatrixRotationQuaternion(rotationQuaternion);
        XMMATRIX T_p = XMMatrixTranslation(
            m_Position.x - XMVectorGetX(offsetVec),
            m_Position.y - XMVectorGetY(offsetVec),
            m_Position.z - XMVectorGetZ(offsetVec)
        );


        XMMATRIX parentWorld = S_p * R_p * T_p;

        return parentWorld;
    }


    //------------------------------------------------------------------------
    //ベクトルを触る
    void SetVelocity(const Vector3& velocity)
    {
        if (m_RigidBody)
        {
            m_RigidBody->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z));
            m_RigidBody->activate(true);
        }
    }

    Vector3 GetVelocity() const
    {
        if (m_RigidBody)
        {
            btVector3 v = m_RigidBody->getLinearVelocity();
            return Vector3(v.getX(), v.getY(), v.getZ());
        }
        return Vector3(0, 0, 0);
    }
    //--------------------------------------------------------------------------
    //衝突のレイヤーを設定してる
    //これで衝突しないとかのレイヤー分けができる
	virtual void SetupCollisionLayer() {
        switch (m_Tag) {
        case GameObjectTag::Player:
            m_CollisionGroup = COL_PLAYER;
            m_CollisionMask = COL_ENEMY | COL_WALL | COL_GROUND | COL_ITEM;
            break;

        case GameObjectTag::Enemy:
            m_CollisionGroup = COL_ENEMY;
            m_CollisionMask = COL_PLAYER | COL_WALL | COL_GROUND;
            break;

        case GameObjectTag::Anchor:
            m_CollisionGroup = COL_ANCHOR;
            m_CollisionMask = COL_WALL | COL_GROUND | COL_DEFAULT;
            break;
   

        case GameObjectTag::Item:
            m_CollisionGroup = COL_ITEM;
            m_CollisionMask = COL_PLAYER; // 取れるのはプレイヤーのみ
            break;

        case GameObjectTag::Ground:
            m_CollisionGroup = COL_GROUND;
            m_CollisionMask = COL_PLAYER | COL_ENEMY | COL_BULLET|COL_DEFAULT;
            break;

        case GameObjectTag::Wall:
            m_CollisionGroup = COL_WALL;
            m_CollisionMask = COL_PLAYER | COL_ENEMY | COL_BULLET;
            break;

        default:
            m_CollisionGroup = COL_DEFAULT;
            m_CollisionMask = -1;//すべてと衝突する
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

    
    // コライダー生成関数
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
                { "CollisionMask", m_CollisionMask },
                { "OriginalColliderHalfSize", { m_OriginalColliderHalfSize.x,m_OriginalColliderHalfSize.y,m_OriginalColliderHalfSize.z }}
            };
        }

        return j;
    }



    void FromJson(const json& j)
    {
        GameObject::FromJson(j);

        if (!j.contains("RigidBody")) return;
        const auto& rb = j["RigidBody"];

        m_mass = rb.value("Mass", 0.0f);
        std::string colliderType = rb.value("ColliderType", "Box");

     

        if (rb.contains("ColliderOffset")) {
            m_ColliderOffset = {
                rb["ColliderOffset"][0],
                rb["ColliderOffset"][1],
                rb["ColliderOffset"][2]
            };
        }
        else {
            m_ColliderOffset = { 0,0,0 };
        }

        if (rb.contains("OriginalColliderHalfSize"))
        {
            m_OriginalColliderHalfSize = {
                rb["OriginalColliderHalfSize"][0],
                rb["OriginalColliderHalfSize"][1],
                rb["OriginalColliderHalfSize"][2]
            };
        }
        else
        {
            //なかったら初期値　全部１
            m_OriginalColliderHalfSize = m_Scale;
        }

        if (colliderType == "Box") {
            CreateBoxCollider(m_OriginalColliderHalfSize, m_mass);
        }
        else if (colliderType == "Sphere") {
            CreateSphereCollider(m_OriginalColliderHalfSize.x, m_mass);
        }
        else if (colliderType == "Capsule") {
            CreateCapsuleCollider(m_OriginalColliderHalfSize.x, m_OriginalColliderHalfSize.y, m_mass);
        }
        
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