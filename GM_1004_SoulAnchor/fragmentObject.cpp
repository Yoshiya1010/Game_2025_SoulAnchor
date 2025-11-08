#include"fragmentObject.h"
#include "manager.h"

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

    // グループ化して破壊
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