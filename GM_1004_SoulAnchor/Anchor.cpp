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

    // PendingVelocityがあれば反映
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
        // 引き寄せ処理
        if (m_IsPulling && m_Attached && m_AttachedTarget && m_Owner)
        {
            Vector3 anchorPos = m_AttachedTarget->GetPosition();
            Vector3 ownerPos = m_Owner->GetPosition();

            Vector3 direction = anchorPos - ownerPos;
            float distance = direction.Length();

            // まだ距離がある場合
            if (distance > m_PullDistance)
            {
                direction.Normalize();

                // アンカーが刺さっている物体を引き寄せる力を与える
                btVector3 pullForce(
                    direction.x * m_PullForce,
                    direction.y * m_PullForce,
                    direction.z * m_PullForce
                );

                // 接続先の物体に力を加える
                PhysicsObject* targetPhysics = dynamic_cast<PhysicsObject*>(m_AttachedTarget);
                if (targetPhysics && targetPhysics->GetRigidBody())
                {
                    targetPhysics->GetRigidBody()->applyCentralForce(pullForce);
                    targetPhysics->GetRigidBody()->activate(true);
                }
            }
            else
            {
                // 引き寄せ完了 - ジョイント解除
                StopPulling();
                Detach();

                // アンカー自体を削除
                SetDestroy();
            }
        }
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
    // まだ接続していなくて、プレイヤー以外に当たったら接続
    if (!m_Attached && other->GetTag() != GameObjectTag::Player) {
        AttachTo(other, hitPoint);
    }
}

void Anchor::AttachTo(GameObject* target, const Vector3& hitPoint)
{
    auto* world = PhysicsManager::GetWorld();
    if (!world || !target) return;

    // ターゲットがPhysicsObjectでない場合は接続できない
    PhysicsObject* targetPhysics = dynamic_cast<PhysicsObject*>(target);
    if (!targetPhysics || !targetPhysics->GetRigidBody()) return;

    btRigidBody* bodyA = m_RigidBody.get();
    btRigidBody* bodyB = targetPhysics->GetRigidBody();

    // アンカーのローカル座標での接続点
    btVector3 pivotInA(0, 0, 0);

    // ターゲットのローカル座標での接続点
    btVector3 pivotInB = btVector3(
        hitPoint.x - target->GetPosition().x,
        hitPoint.y - target->GetPosition().y,
        hitPoint.z - target->GetPosition().z
    );

    // Point2Pointジョイント作成
    m_Joint = new btPoint2PointConstraint(*bodyA, *bodyB, pivotInA, pivotInB);
    world->addConstraint(m_Joint, true);

    m_Attached = true;
    m_AttachedTarget = target;

    // 接続したら速度を0にする（刺さった状態）
    if (m_RigidBody) {
        m_RigidBody->setLinearVelocity(btVector3(0, 0, 0));
        m_RigidBody->setAngularVelocity(btVector3(0, 0, 0));
    }
}

void Anchor::Detach()
{
    if (m_Joint) {
        auto* world = PhysicsManager::GetWorld();
        if (world) {
            world->removeConstraint(m_Joint);
        }
        delete m_Joint;
        m_Joint = nullptr;
    }
    m_Attached = false;
    m_AttachedTarget = nullptr;
}

void Anchor::StartPulling()
{
    if (m_Attached) {
        m_IsPulling = true;
    }
}

void Anchor::StopPulling()
{
    m_IsPulling = false;
}