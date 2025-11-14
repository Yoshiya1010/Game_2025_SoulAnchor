#include "ChainLink.h"
#include "renderer.h"
#include "PhysicsManager.h"

void ChainLink::Init()
{
    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");
    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");

    m_Radius = 0.1f;
    m_Length = 0.3f;

    SetName("ChainLink");
    SetTag(GameObjectTag::None);
}

void ChainLink::InitializeLink(Vector3 position, float radius, float length, float mass)
{
    m_Position = position;
    m_Radius = radius;
    m_Length = length;

    // カプセルコライダーを作成（チェーンリンク形状）
    if (PhysicsManager::GetWorld())
    {
        m_ColliderOffset = Vector3(0, 0, 0);
        CreateCapsuleCollider(radius, length, mass);

        if (m_RigidBody)
        {
            // チェーン専用のコリジョンレイヤー設定
            m_CollisionGroup = COL_CHAIN;
            // プレイヤー、アンカー、他のチェーンとは衝突しない
            // 壁と地面とのみ衝突
            m_CollisionMask = COL_WALL | COL_GROUND | COL_DEFAULT;

            // 物理ワールドから一旦削除して再登録
            auto* world = PhysicsManager::GetWorld();
            if (world)
            {
                world->removeRigidBody(m_RigidBody.get());
                world->addRigidBody(m_RigidBody.get(), m_CollisionGroup, m_CollisionMask);
            }

            // 物理パラメータの調整
            m_RigidBody->setDamping(0.5f, 0.7f);  // 減衰を高めに設定（揺れを抑える）
            m_RigidBody->setFriction(0.3f);
            m_RigidBody->setRestitution(0.1f);    // 反発を低く

            // 回転の制限（チェーンらしい動きに）
            m_RigidBody->setAngularFactor(btVector3(1, 0.5f, 1));
        }
    }
}

void ChainLink::Uninit()
{
    if (m_RigidBody)
    {
        PhysicsObject::Uninit();
    }

    if (m_VertexBuffer)  m_VertexBuffer->Release();
    if (m_VertexLayout)  m_VertexLayout->Release();
    if (m_VertexShader)  m_VertexShader->Release();
    if (m_PixelShader)   m_PixelShader->Release();
}

void ChainLink::Update()
{
    // 物理演算による位置更新のみ
    if (m_RigidBody)
    {
        UpdatePhysicsWithModel();
    }
}

void ChainLink::Draw()
{
    
}

Vector3 ChainLink::GetConnectionPoint() 
{
    // リンクの下端の位置を返す
    Vector3 down =GetUp() * -1.0f;
    return m_Position + (down * (m_Length * 0.5f));
}