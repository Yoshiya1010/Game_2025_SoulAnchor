// fragmentObject.cpp
// 破壊機能を持つPhysicsObjectの実装

#include "fragmentObject.h"
#include "manager.h"
#include "TriangleMeshBuilder.h"

void FragmentObject::OnCollisionEnter(GameObject* other, const Vector3& hitPoint)
{
    // 既に破壊されている、または破壊不可能な場合は何もしない
    if (m_IsDestroyed || !m_Destructible) return;

    // 衝突相手が物理オブジェクトの場合、速度を取得
    PhysicsObject* physicsOther = dynamic_cast<PhysicsObject*>(other);
    if (physicsOther && physicsOther->GetRigidBody()) {
        Vector3 velocity = physicsOther->GetVelocity();
        float impactSpeed = velocity.Length();

        // 閾値を超えたら破壊
        if (impactSpeed > m_DestructionThreshold) {
            DestroyObject(hitPoint);
        }
    }
}

void FragmentObject::DestroyObject(const Vector3& impactPoint)
{
    // 既に破壊済みならスキップ
    if (m_IsDestroyed) return;
    m_IsDestroyed = true;

    // シーンを取得
    Scene* scene = Manager::GetScene();

    if (!scene || !m_ModelRenderer) {
        SetDestroy();
        return;
    }

    // モデルを取得
    MODEL* model = m_ModelRenderer->GetModel();

    if (!model) {
        SetDestroy();
        return;
    }

    // ワールド行列を計算
    XMMATRIX worldMatrix =
        XMMatrixScaling(m_Scale.x * m_ModelScale, m_Scale.y * m_ModelScale, m_Scale.z * m_ModelScale) *
        XMMatrixRotationRollPitchYaw(
            m_Rotation.x * DEG2RAD,
            m_Rotation.y * DEG2RAD,
            m_Rotation.z * DEG2RAD
        ) *
        XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

    // グループ化して破壊（三角形を指定個数ずつまとめて破片にする）
    MeshDestroyer::DestroyModelGrouped(
        model,
        worldMatrix,
        impactPoint,
        m_ExplosionForce,
        scene,
        m_GroupSize
    );

    // 自分自身を削除
    SetDestroy();
}

void FragmentObject::LoadModel(const char* filepath)
{
    if (!m_ModelRenderer) {
        m_ModelRenderer = new ModelRenderer();
    }
    m_ModelRenderer->Load(filepath);
}

void FragmentObject::RecreateCollider()
{
    // トライアングルメッシュの場合
    if (m_UseTriangleMesh && m_ModelRenderer) {
        if (!m_CollisionShape) return;
        auto* world = PhysicsManager::GetWorld();
        if (!world) return;

        // 既存の剛体を一旦削除
        if (m_RigidBody) {
            world->removeRigidBody(m_RigidBody.get());
            m_RigidBody->setUserPointer(nullptr);
        }

        // トライアングルメッシュを再作成（スケールを反映）
        btTriangleMesh* triMesh = new btTriangleMesh();
        MODEL* model = m_ModelRenderer->GetModel();

        if (model) {
            for (unsigned int i = 0; i < model->CollisionIndices.size(); i += 3) {
                auto& p0 = model->CollisionVertices[model->CollisionIndices[i + 0]];
                auto& p1 = model->CollisionVertices[model->CollisionIndices[i + 1]];
                auto& p2 = model->CollisionVertices[model->CollisionIndices[i + 2]];

                // スケールを適用して頂点座標を作成
                triMesh->addTriangle(
                    btVector3(p0.x * m_Scale.x, p0.y * m_Scale.y, p0.z * m_Scale.z),
                    btVector3(p1.x * m_Scale.x, p1.y * m_Scale.y, p1.z * m_Scale.z),
                    btVector3(p2.x * m_Scale.x, p2.y * m_Scale.y, p2.z * m_Scale.z)
                );
            }

            m_CollisionShape = std::unique_ptr<btCollisionShape>(
                new btBvhTriangleMeshShape(triMesh, true)
            );
        }

        // 剛体を再登録
        CreateRigidBody(m_mass);
    }
    else {
        // 通常のコライダーは親クラスの処理を呼ぶ
        PhysicsObject::RecreateCollider();
    }
}