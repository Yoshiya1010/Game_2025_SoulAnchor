#include"RockTallBlock_A.h"
#include "manager.h"
#include "camera.h"
#include "input.h"
#include"explosion.h"
#include "PhysicsManager.h"
#include "animationModel.h"
#include"TriangleMeshBuilder.h"


void RockTallBlock_A::Init()
{


    // モデルのロード
    m_ModelRenderer = new ModelRenderer();
    m_ModelRenderer->Load("asset\\model\\rock_tallA.obj");


    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");

    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");



    m_Started = false;

    SetName("RockTallBlock_A");

    SetTag(GameObjectTag::Ground);


}
void RockTallBlock_A::Start()
{
    if (m_RigidBody) return;
    // 物理コライダーの設定
    if (PhysicsManager::GetWorld()) {
        // 衝突レイヤー設定
        SetupCollisionLayer();

        btCompoundShape* shape = CreateConvexDecompositionShapeFast(
            m_ModelRenderer->GetModel(),
            "rock_tallA"  // ← キャッシュキー（モデル名）
        );

        if (!shape) {
            printf("[RockTallBlock_A] Failed to create collision shape!\n");
            return;
        }

        // RigidBody作成
        btTransform t;
        t.setIdentity();
        t.setOrigin(btVector3(m_Position.x, m_Position.y, m_Position.z));

        btScalar mass = 0.0f;  // 静的オブジェクト
        btVector3 inertia(0, 0, 0);

        m_MotionState = std::make_unique<btDefaultMotionState>(t);

        btRigidBody::btRigidBodyConstructionInfo info(mass, m_MotionState.get(), shape, inertia);
        m_RigidBody = std::make_unique<btRigidBody>(info);
        m_CollisionShape.reset(shape);

        m_RigidBody->setUserPointer(this);
        PhysicsManager::GetWorld()->addRigidBody(
            m_RigidBody.get(),
            m_CollisionGroup,
            m_CollisionMask
        );


    }


}


void RockTallBlock_A::Uninit()
{
    // 物理オブジェクトの削除
    if (m_RigidBody && PhysicsManager::GetWorld()) {
        PhysicsManager::GetWorld()->removeRigidBody(m_RigidBody.get());
        m_RigidBody->setUserPointer(nullptr);
    }

    m_RigidBody.reset();
    m_MotionState.reset();
    m_CollisionShape.reset();

    delete m_ModelRenderer;
    if (m_VertexLayout)     m_VertexLayout->Release();
    if (m_VertexShader)     m_VertexShader->Release();
    if (m_PixelShader)      m_PixelShader->Release();
}

void RockTallBlock_A::Update()
{
    CheckAndCallStart();
    if (m_Started)
    {



    }
}

void RockTallBlock_A::Draw()
{

    // まずこのオブジェクト用のレイアウト＆シェーダを必ずセット
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

    // UnlitColor はテクスチャ使わないのでスロットをクリア（保険）
    ID3D11ShaderResourceView* nullSRV = nullptr;
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

    Renderer::SetWorldMatrix(
        //モデルと物理の座標を同期させる
        UpdatePhysicsWithModel(m_modelScale));
    // モデルの描画
    m_ModelRenderer->Draw();

}
