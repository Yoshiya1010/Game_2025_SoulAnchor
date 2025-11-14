#include "ChainSystem.h"
#include "renderer.h"
#include "PhysicsManager.h"
#include "manager.h"
#include "scene.h"
#include <cmath>


void ChainSystem::Init()
{
    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");
    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");

    m_StartObject = nullptr;
    m_EndObject = nullptr;
    m_PhysicsLinkCount = 6;
    m_VisualLinkCount = 20;
    m_LinkRadius = 0.05f;
    m_LinkLength = 0.3f;
    m_LinkMass = 0.1f;

    SetName("ChainSystem");

    m_ModelRenderer = new ModelRenderer();
    m_ModelRenderer->Load("asset\\model\\bullet.obj");
}

void ChainSystem::CreateChain(GameObject* startObj, GameObject* endObj,
    int physicsLinkCount,
    float linkRadius,
    float linkLength,
    float linkMass)
{
    if (!startObj || !endObj) return;

    m_StartObject = startObj;
    m_EndObject = endObj;
    m_PhysicsLinkCount = physicsLinkCount;
    m_LinkRadius = linkRadius;
    m_LinkLength = linkLength;
    m_LinkMass = linkMass;

    // 物理リンクを作成
    CreatePhysicsLinks();

    // ジョイントで接続
    CreateConstraints();

    // 視覚的な補間ポイントを計算
    CalculateVisualPoints();
}

void ChainSystem::CreatePhysicsLinks()
{
    if (!m_StartObject || !m_EndObject) return;

    Vector3 start = m_StartObject->GetPosition();
    Vector3 end = m_EndObject->GetPosition();

    // リンク間の距離を計算
    Vector3 direction = end - start;
    float totalDistance = direction.Length();
    direction.Normalize();

    // 各リンクの配置
    for (int i = 0; i < m_PhysicsLinkCount; i++)
    {
        float t = (float)(i + 1) / (float)(m_PhysicsLinkCount + 1);
        Vector3 position = start + (direction * (totalDistance * t));

        // チェーンリンクを作成
        ChainLink* link = Manager::GetScene()->AddGameObject<ChainLink>(OBJECT);
        link->Init();
        link->InitializeLink(position, m_LinkRadius, m_LinkLength, m_LinkMass);

        m_PhysicsLinks.push_back(link);
    }
}

void ChainSystem::CreateConstraints()
{
    if (!m_StartObject || !m_EndObject || m_PhysicsLinks.empty()) return;

    auto* world = PhysicsManager::GetWorld();
    if (!world) return;

    // 開始オブジェクトと最初のリンクを接続
    PhysicsObject* startPhysics = dynamic_cast<PhysicsObject*>(m_StartObject);
    if (startPhysics && startPhysics->GetRigidBody())
    {
        btRigidBody* bodyA = startPhysics->GetRigidBody();
        btRigidBody* bodyB = m_PhysicsLinks[0]->GetRigidBody();

        if (bodyA && bodyB)
        {
            btVector3 pivotA(0, -1.0f, 0);  // プレイヤーの下部
            btVector3 pivotB(0, m_LinkLength * 0.5f, 0);  // リンクの上部

            btPoint2PointConstraint* constraint = new btPoint2PointConstraint(*bodyA, *bodyB, pivotA, pivotB);
            world->addConstraint(constraint, true);
            m_Constraints.push_back(constraint);
        }
    }

    // リンク同士を接続
    for (size_t i = 0; i < m_PhysicsLinks.size() - 1; i++)
    {
        btRigidBody* bodyA = m_PhysicsLinks[i]->GetRigidBody();
        btRigidBody* bodyB = m_PhysicsLinks[i + 1]->GetRigidBody();

        if (bodyA && bodyB)
        {
            btVector3 pivotA(0, -m_LinkLength * 0.5f, 0);  // 下部
            btVector3 pivotB(0, m_LinkLength * 0.5f, 0);   // 上部

            btPoint2PointConstraint* constraint = new btPoint2PointConstraint(*bodyA, *bodyB, pivotA, pivotB);
            world->addConstraint(constraint, true);
            m_Constraints.push_back(constraint);
        }
    }

    // 最後のリンクと終了オブジェクトを接続
    PhysicsObject* endPhysics = dynamic_cast<PhysicsObject*>(m_EndObject);
    if (endPhysics && endPhysics->GetRigidBody())
    {
        btRigidBody* bodyA = m_PhysicsLinks.back()->GetRigidBody();
        btRigidBody* bodyB = endPhysics->GetRigidBody();

        if (bodyA && bodyB)
        {
            btVector3 pivotA(0, -m_LinkLength * 0.5f, 0);  // リンクの下部
            btVector3 pivotB(0, 0, 0);  // アンカーの中心

            btPoint2PointConstraint* constraint = new btPoint2PointConstraint(*bodyA, *bodyB, pivotA, pivotB);
            world->addConstraint(constraint, true);
            m_Constraints.push_back(constraint);
        }
    }

 
}

void ChainSystem::CalculateVisualPoints()
{
    m_VisualPoints.clear();

    if (!m_StartObject || !m_EndObject) return;

    // 開始点
    m_VisualPoints.push_back(m_StartObject->GetPosition());

    // 物理リンクの位置を追加
    for (ChainLink* link : m_PhysicsLinks)
    {
        m_VisualPoints.push_back(link->GetPosition());
    }

    // 終了点
    m_VisualPoints.push_back(m_EndObject->GetPosition());

    // 物理リンク間をカテナリー曲線で補間
    std::vector<Vector3> interpolatedPoints;
    interpolatedPoints.push_back(m_VisualPoints[0]);

    for (size_t i = 0; i < m_VisualPoints.size() - 1; i++)
    {
        Vector3 start = m_VisualPoints[i];
        Vector3 end = m_VisualPoints[i + 1];

        // 各セグメント間に複数の補間ポイントを追加
        int subdivisions = m_VisualLinkCount / m_PhysicsLinkCount;
        for (int j = 1; j <= subdivisions; j++)
        {
            float t = (float)j / (float)(subdivisions + 1);

            // シンプルな重力カーブ（カテナリー曲線の簡易版）
            Vector3 midpoint = start + (end - start) * t;
            float sag = 0.2f * std::sin(t * 3.14159f);  // 重力による垂れ下がり
            midpoint.y -= sag;

            interpolatedPoints.push_back(midpoint);
        }

        interpolatedPoints.push_back(end);
    }

    m_VisualPoints = interpolatedPoints;
}

void ChainSystem::UpdateChain()
{
    // 視覚的な補間ポイントを再計算
    CalculateVisualPoints();
}

void ChainSystem::Uninit()
{
    DestroyChain();

    if (m_VertexBuffer)  m_VertexBuffer->Release();
    if (m_VertexLayout)  m_VertexLayout->Release();
    if (m_VertexShader)  m_VertexShader->Release();
    if (m_PixelShader)   m_PixelShader->Release();
}

void ChainSystem::DestroyChain()
{
    
    for (ChainLink* link : m_PhysicsLinks)
    {
        if (link)
        {
            link->SetDestroy();
        }
    }
    m_PhysicsLinks.clear();

    // Constraintのポインタをクリアするだけ（deleteしない）
    m_Constraints.clear();

    m_StartObject = nullptr;
    m_EndObject = nullptr;
}

void ChainSystem::Update()
{
    // 開始または終了オブジェクトが存在しない場合は削除
    if (!m_StartObject || !m_EndObject ||
        m_StartObject->Destroy() || m_EndObject->Destroy())
    {
        SetDestroy();
        return;
    }

    // 視覚的な補間ポイントを更新
    CalculateVisualPoints();
}

void ChainSystem::Draw()
{
    if (m_VisualPoints.size() < 2) return;

    // ライン描画でチェーンを表現
    // 各セグメント間に線を描画
    for (size_t i = 0; i < m_VisualPoints.size() - 1; i++)
    {
        Vector3 start = m_VisualPoints[i];
        Vector3 end = m_VisualPoints[i + 1];

        // 線分を円柱で描画（実装に応じて変更）
        // ここでは既存の描画システムを利用

        // 中点を計算
        Vector3 midpoint = (start + end) * 0.5f;
        Vector3 direction = end - start;
        float length = direction.Length();
        direction.Normalize();

        // 回転を計算
        Vector3 up(0, 1, 0);
        Vector3 right = Vector3::Cross(up, direction);
        right.Normalize();
        Vector3 forward = Vector3::Cross(right,direction);

        // ワールド行列を構築
        XMMATRIX scale = XMMatrixScaling(m_LinkRadius, length * 0.5f, m_LinkRadius);

        XMMATRIX rotation = XMMatrixIdentity();
        rotation.r[0] = XMVectorSet(right.x, right.y, right.z, 0);
        rotation.r[1] = XMVectorSet(direction.x, direction.y, direction.z, 0);
        rotation.r[2] = XMVectorSet(forward.x, forward.y, forward.z, 0);

        XMMATRIX translation = XMMatrixTranslation(midpoint.x, midpoint.y, midpoint.z);

        XMMATRIX world = scale * rotation * translation;

        Renderer::SetWorldMatrix(world);

        // シェーダー設定
        Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
        Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
        Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

        ID3D11ShaderResourceView* nullSRV = nullptr;
        Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);
    
        // モデルの描画
        m_ModelRenderer->Draw();
        
    }


}

Vector3 ChainSystem::InterpolateCatenary(const Vector3& start, const Vector3& end, float t, float sag)
{
    // カテナリー曲線の簡易実装
    Vector3 result = start + (end - start) * t;

    // 放物線的な垂れ下がりを追加
    float sagAmount = sag * std::sin(t * 3.14159f);
    result.y -= sagAmount;

    return result;
}