#include"GroundBlock.h"
#include "manager.h"

#include "camera.h"
#include "input.h"
#include"explosion.h"
#include "PhysicsManager.h"
#include <btBulletDynamicsCommon.h>
#include "animationModel.h"
#include"ModelFBX.h"

void GroundBlock::Init()
{
  

    // モデルのロード
    m_ModelRenderer = std::make_unique<StaticFBXModel>();
    m_ModelRenderer->Load("asset\\model\\GroundBlock.fbx");


    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");

    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");



    m_Started = false;

    SetName("GroundBlock");


}
void GroundBlock::Start()
{
    if (m_RigidBody) return;
    // 物理コライダーの設定
    if (PhysicsManager::GetWorld()) {
        m_CollisionShape = std::make_unique<btBoxShape>(
            btVector3(m_Scale.x, m_Scale.y, m_Scale.z)
        );

        btTransform startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(btVector3(m_Position.x, m_Position.y, m_Position.z));

        m_MotionState = std::make_unique<btDefaultMotionState>(startTransform);

        btScalar mass = 1.0f;
        btVector3 localInertia(0, 0, 0);
        if (mass != 0.0f)
            m_CollisionShape->calculateLocalInertia(mass, localInertia);

        auto rbInfo = btRigidBody::btRigidBodyConstructionInfo(
            mass, m_MotionState.get(), m_CollisionShape.get(), localInertia
        );

        m_RigidBody = std::make_unique<btRigidBody>(rbInfo);
        m_RigidBody->setUserPointer(this);

        PhysicsManager::GetWorld()->addRigidBody(m_RigidBody.get());
    }

 
}


void GroundBlock::Uninit()
{
    // 物理オブジェクトの削除
    if (m_RigidBody && PhysicsManager::GetWorld()) {
        PhysicsManager::GetWorld()->removeRigidBody(m_RigidBody.get());
        m_RigidBody->setUserPointer(nullptr);
    }

    m_RigidBody.reset();
    m_MotionState.reset();
    m_CollisionShape.reset();

    m_ModelRenderer.reset();
    if (m_VertexLayout)     m_VertexLayout->Release();
    if (m_VertexShader)     m_VertexShader->Release();
    if (m_PixelShader)      m_PixelShader->Release();
}

void GroundBlock::Update()
{
    CheckAndCallStart();
    if (m_Started)
    {
       


    }
}

void GroundBlock::Draw()
{
    Renderer::SetWorldMatrix( 
        //モデルと物理の座標を同期させる
        UpdatePhysicsWithModel(m_modelScale));
    // モデルの描画
    m_ModelRenderer->Draw();

}
