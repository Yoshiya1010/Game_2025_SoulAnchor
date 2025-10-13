#include"GroundBlock.h"
#include "manager.h"
#include "modelRenderer.h"
#include "camera.h"
#include "input.h"
#include"explosion.h"
#include "PhysicsManager.h"
#include <btBulletDynamicsCommon.h>

void GroundBlock::Init()
{
    m_ModelRenderer = new ModelRenderer();
    m_ModelRenderer->Load("asset\\model\\player.obj");


    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");

    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");



    m_Started = false;


}
void GroundBlock::Start()
{

    // 物理コライダーの設定
    if (PhysicsManager::GetWorld()) {
        // ボックス形状のコライダー
        m_CollisionShape = new btBoxShape(btVector3(m_Scale.x, m_Scale.y,m_Scale.z));

        // 初期トランスフォーム
        btTransform startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(btVector3(m_Position.x, m_Position.y, m_Position.z));

        // MotionStateを作成（これで位置同期が可能に）
        m_MotionState = new btDefaultMotionState(startTransform);


        // 質量と慣性
        btScalar mass = 1.0f;
        btVector3 localInertia(0, 0, 0);
        if (mass != 0.0f) {
            m_CollisionShape->calculateLocalInertia(mass, localInertia);
        }

        // リジッドボディ作成
        btRigidBody::btRigidBodyConstructionInfo rbInfo(
            mass, m_MotionState, m_CollisionShape, localInertia);
        m_RigidBody = new btRigidBody(rbInfo);

        // 物理世界に追加
        PhysicsManager::GetWorld()->addRigidBody(m_RigidBody);

        printf("Enemy physics body added with MotionState at: %.2f, %.2f, %.2f\n",
            m_Position.x, m_Position.y, m_Position.z);
    }

    SetName("GroundBlock");
}


void GroundBlock::Uninit()
{
    // 物理オブジェクトの削除
    if (m_RigidBody && PhysicsManager::GetWorld()) {
        PhysicsManager::GetWorld()->removeRigidBody(m_RigidBody);
        delete m_RigidBody;
    }


    delete m_ModelRenderer;
    if (m_VertexLayout)     m_VertexLayout->Release();
    if (m_VertexShader)     m_VertexShader->Release();
    if (m_PixelShader)      m_PixelShader->Release();
}

void GroundBlock::Update()
{
    CheckAndCallStart();
    if (m_Started)
    {
        // 物理エンジンから位置を手動で取得
        btTransform trans = m_RigidBody->getCenterOfMassTransform();
        m_Position.x = trans.getOrigin().getX();
        m_Position.y = trans.getOrigin().getY();
        m_Position.z = trans.getOrigin().getZ();
    }
}

void GroundBlock::Draw()
{
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

    // シェーダー設定
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

    // マトリクス設定
    XMMATRIX world, scale, rot, trans;
    scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
    rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y + XM_PI, m_Rotation.z);
    trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
    world = scale * rot * trans;
    Renderer::SetWorldMatrix(world);

    // モデルの描画
    m_ModelRenderer->Draw();

}