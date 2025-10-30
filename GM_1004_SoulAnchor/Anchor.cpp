#include"Anchor.h"
#include "manager.h"
#include "camera.h"
#include "input.h"
#include"explosion.h"
#include "PhysicsManager.h"
#include "animationModel.h"
#include"ModelFBX.h"

void Anchor::Init()
{
    // モデルのロード
    m_ModelRenderer = std::make_unique<StaticFBXModel>();
    m_ModelRenderer->Load("asset\\model\\Anchor.fbx");


    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");

    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");



    m_Started = false;

    SetName("Anchor");

    SetTag(GameObjectTag::Anchor);


}
void Anchor::Start()
{
    // Rigidbody未生成なら作る
    if (!m_RigidBody && PhysicsManager::GetWorld()) {
        SetupCollisionLayer();
        m_ColliderOffset = Vector3(0, 0, 0);
        CreateBoxCollider(Vector3(1.0f, 1.0f, 1.0f), 1.0f);
    }

    // PendingVelocityがあれば反映（0でもOK）
    SetVelocity(m_PendingVelocity);
    m_PendingVelocity = Vector3(0, 0, 0);


}


void Anchor::Uninit()
{
    if (m_RigidBody)
    {
        PhysicsObject::Uninit();//Rigtbodyを消す
    }

    m_ModelRenderer.reset();
    if (m_VertexLayout)     m_VertexLayout->Release();
    if (m_VertexShader)     m_VertexShader->Release();
    if (m_PixelShader)      m_PixelShader->Release();
}

void Anchor::Update()
{
    CheckAndCallStart();
    if (m_Started)
    {


    }
}

void Anchor::Draw()
{

    // まずこのオブジェクト用のレイアウト＆シェーダを必ずセット
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

    // UnlitColor はテクスチャ使わないのでスロットをクリア
    ID3D11ShaderResourceView* nullSRV = nullptr;
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

    Renderer::SetWorldMatrix(
        //モデルと物理の座標を同期させる
        UpdatePhysicsWithModel(m_modelScale));
    // モデルの描画
    m_ModelRenderer->Draw();

}

void Anchor::OnCollisionEnter(GameObject* other, const Vector3& hitPoint)
{
    if (!m_Attached && other->GetTag() != GameObjectTag::Player) {
        AttachTo(other, hitPoint);
    }
}

void Anchor::AttachTo(GameObject* target, const Vector3& hitPoint)
{
    auto* world = PhysicsManager::GetWorld();
    if (!world || !target) return;

    btRigidBody* bodyA = m_RigidBody.get();
    btRigidBody* bodyB = ((PhysicsObject*)target)->GetRigidBody();

    btVector3 pivotInA(0, 0, 0);
    btVector3 pivotInB = btVector3(
        hitPoint.x - target->GetPosition().x,
        hitPoint.y - target->GetPosition().y,
        hitPoint.z - target->GetPosition().z);

    m_Joint = new btPoint2PointConstraint(*bodyA, *bodyB, pivotInA, pivotInB);
    world->addConstraint(m_Joint, true);

    m_Attached = true;
    m_AttachedTarget = target;
}

void Anchor::Detach()
{
    if (m_Joint) {
        auto* world = PhysicsManager::GetWorld();
        world->removeConstraint(m_Joint);
        delete m_Joint;
        m_Joint = nullptr;
    }
    m_Attached = false;
    m_AttachedTarget = nullptr;
}
