// fragmentObject.cpp
// 破壊機能を持つPhysicsObjectの実装
// コライダータイプ: トライアングルメッシュ（静的） or Box（動的可能）

#include "fragmentObject.h"
#include "manager.h"
#include "TriangleMeshBuilder.h"

void FragmentObject::Start()
{
    // 既にRigidBodyがあれば何もしない
    if (m_RigidBody) return;

    // モデルがない場合は何もしない
    if (!m_ModelRenderer) return;

    // 衝突レイヤー設定
    SetupCollisionLayer();

    // コライダータイプに応じて作成
    if (m_UseTriangleMesh) {
        // トライアングルメッシュコライダー（静的のみ）
        MODEL* model = m_ModelRenderer->GetModel();
        if (!model) return;

        btTriangleMesh* triMesh = new btTriangleMesh();

        // スケールを反映した頂点座標でメッシュを作成
        for (unsigned int i = 0; i < model->CollisionIndices.size(); i += 3) {
            auto& p0 = model->CollisionVertices[model->CollisionIndices[i + 0]];
            auto& p1 = model->CollisionVertices[model->CollisionIndices[i + 1]];
            auto& p2 = model->CollisionVertices[model->CollisionIndices[i + 2]];

            triMesh->addTriangle(
                btVector3(p0.x * m_Scale.x, p0.y * m_Scale.y, p0.z * m_Scale.z),
                btVector3(p1.x * m_Scale.x, p1.y * m_Scale.y, p1.z * m_Scale.z),
                btVector3(p2.x * m_Scale.x, p2.y * m_Scale.y, p2.z * m_Scale.z)
            );
        }

        m_CollisionShape = std::unique_ptr<btCollisionShape>(
            new btBvhTriangleMeshShape(triMesh, true)
        );

        // トライアングルメッシュは必ず静的（mass=0）
        CreateRigidBody(0.0f);
        m_mass = 0.0f;
    }
    else {
        // Boxコライダー（動的可能）
        // モデルからバウンディングボックスを自動計算
        m_AutoBoxHalfSize = CalculateModelBounds();

        // スケールを反映したサイズでBoxを作成
        Vector3 scaledHalfSize = {
            m_AutoBoxHalfSize.x * m_Scale.x,
            m_AutoBoxHalfSize.y * m_Scale.y,
            m_AutoBoxHalfSize.z * m_Scale.z
        };

        // コライダーオフセット（Boxの中心をモデルの中心に合わせる）
        m_ColliderOffset = Vector3(0, m_AutoBoxHalfSize.y * m_Scale.y, 0);

        // Boxコライダー作成
        CreateBoxCollider(m_AutoBoxHalfSize, m_mass);
    }

    RecreateCollider();
}

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
           /* DestroyObject(hitPoint);*/
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

Vector3 FragmentObject::CalculateModelBounds()
{
    // モデルがない場合はデフォルト値
    if (!m_ModelRenderer) return Vector3(1.0f, 1.0f, 1.0f);

    MODEL* model = m_ModelRenderer->GetModel();
    if (!model || model->CollisionVertices.empty()) {
        return Vector3(1.0f, 1.0f, 1.0f);
    }

    // 最小・最大座標を初期化
    XMFLOAT3 min = model->CollisionVertices[0];
    XMFLOAT3 max = model->CollisionVertices[0];

    // 全頂点をチェックして最小・最大を求める
    for (const auto& vertex : model->CollisionVertices) {
        // 最小値
        if (vertex.x < min.x) min.x = vertex.x;
        if (vertex.y < min.y) min.y = vertex.y;
        if (vertex.z < min.z) min.z = vertex.z;

        // 最大値
        if (vertex.x > max.x) max.x = vertex.x;
        if (vertex.y > max.y) max.y = vertex.y;
        if (vertex.z > max.z) max.z = vertex.z;
    }

    // バウンディングボックスのサイズを計算（半分のサイズ）
    Vector3 halfSize;
    halfSize.x = (max.x - min.x) * 0.5f;
    halfSize.y = (max.y - min.y) * 0.5f;
    halfSize.z = (max.z - min.z) * 0.5f;

    return halfSize;
}

void FragmentObject::RecreateCollider()
{
    auto* world = PhysicsManager::GetWorld();
    if (!world) return;

    if (m_RigidBody) {
        world->removeRigidBody(m_RigidBody.get());
        m_RigidBody->setUserPointer(nullptr);
        m_RigidBody.reset();
    }

    if (m_UseTriangleMesh && m_ModelRenderer) {

        m_CollisionShape.reset();        //Shapeを破棄
        m_TriMesh = std::make_unique<btTriangleMesh>();  

        MODEL* model = m_ModelRenderer->GetModel();
        if (!model) return;

        m_TriMesh = std::make_unique<btTriangleMesh>();

        for (unsigned int i = 0; i < model->CollisionIndices.size(); i += 3) {
            auto& p0 = model->CollisionVertices[model->CollisionIndices[i]];
            auto& p1 = model->CollisionVertices[model->CollisionIndices[i + 1]];
            auto& p2 = model->CollisionVertices[model->CollisionIndices[i + 2]];

            m_TriMesh->addTriangle(
                btVector3(p0.x * m_Scale.x, p0.y * m_Scale.y, p0.z * m_Scale.z),
                btVector3(p1.x * m_Scale.x, p1.y * m_Scale.y, p1.z * m_Scale.z),
                btVector3(p2.x * m_Scale.x, p2.y * m_Scale.y, p2.z * m_Scale.z)
            );
        }

        m_CollisionShape = std::make_unique<btBvhTriangleMeshShape>(m_TriMesh.get(), true);

        CreateRigidBody(0.0f);
        m_mass = 0.0f;
        return;
    }

    PhysicsObject::RecreateCollider();
}

PhysicsObject* FragmentObject::SetMass(float mass)
{
    // トライアングルメッシュの場合
    if (m_UseTriangleMesh) {
        // トライアングルメッシュは動的オブジェクトにできない（Bulletの制限）
        if (mass > 0.0f) {
            // 質量を0に固定
            mass = 0.0f;
        }

        // 質量を記録
        m_mass = mass;

        // 親クラスのSetMass()は呼ばない（トライアングルメッシュは常に静的）
        return this;
    }

    // Boxコライダーの場合は親クラスの処理を呼ぶ（動的可能）
    return PhysicsObject::SetMass(mass);
}