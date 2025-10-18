#include"GroundBlock.h"
#include "manager.h"

#include "camera.h"
#include "input.h"
#include"explosion.h"
#include "PhysicsManager.h"
#include <btBulletDynamicsCommon.h>
#include "animationModel.h"

void GroundBlock::Init()
{
  

    // モデルのロード
    m_ModelRenderer = make_unique<AnimationModel>();
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
        if (!m_RigidBody || !m_RigidBody->getMotionState()) return;

        // 物理エンジンから位置を手動で取得
        btTransform trans = m_RigidBody->getCenterOfMassTransform();
        m_Position.x = trans.getOrigin().getX();
        m_Position.y = trans.getOrigin().getY();
        m_Position.z = trans.getOrigin().getZ();


    }
}

void GroundBlock::Draw()
{

    btQuaternion quaternion = m_RigidBody->getCenterOfMassTransform().getRotation();

    XMVECTOR rotationQuaternion = XMVectorSet(
        quaternion.x(),
        quaternion.y(),
        quaternion.z(),
        quaternion.w()
    );
   
    XMMATRIX S_p = XMMatrixScaling(m_Scale.x*m_modelScale, m_Scale.y * m_modelScale, m_Scale.z*m_modelScale);
    XMMATRIX R_p = XMMatrixRotationQuaternion(rotationQuaternion);
    XMMATRIX T_p = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
    XMMATRIX parentWorld = S_p * R_p * T_p;

    Renderer::SetWorldMatrix(parentWorld);
    // モデルの描画
    m_ModelRenderer->Draw();

}

json GroundBlock::ToJson() const {
    json j = GameObject::ToJson();
    j["Type"] = "GroundBlock";
    j["Model"] = "asset\\model\\GroundBlock.fbx";
    j["ModelScale"] = m_modelScale;
    return j;
}

void GroundBlock::FromJson(const json& j)
{
    GameObject::FromJson(j);

    // モデルとスケール情報
    std::string modelPath = j.value("Model", "asset\\model\\GroundBlock.fbx");
    float scale = j.value("ModelScale", 2.0f);

    // モデル読み込み
    m_ModelRenderer = std::make_unique<AnimationModel>();
    m_ModelRenderer->Load(modelPath.c_str());

    // スケール設定
    m_Scale = { 1.0f, 1.0f, 1.0f };
    // m_modelScale は const なので変更不可（読み込み時は定数でOK）

    SetName("GroundBlock");
}