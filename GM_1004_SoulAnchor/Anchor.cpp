#include"Anchor.h"
#include "manager.h"
#include "camera.h"
#include "input.h"
#include"explosion.h"
#include "PhysicsManager.h"
#include "animationModel.h"
#include"ModelFBX.h"
#include"FPSPlayer.h"
#include"scene.h"
#include"manager.h"


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


    SetMass(30.f);
    m_Started = false;

    SetName("Anchor");

    SetTag(GameObjectTag::Anchor);


}
void Anchor::Start()
{
    if (m_PendingRotation.x != 0.0f || m_PendingRotation.y != 0.0f || m_PendingRotation.z != 0.0f)
    {
        m_Rotation = m_PendingRotation;
        m_PendingRotation = Vector3(0, 0, 0);
    }

    //Rigidbody未生成なら作る
    if (!m_RigidBody && PhysicsManager::GetWorld()) {
        SetupCollisionLayer();
        m_ColliderOffset = Vector3(0, 0, 0);
        CreateBoxCollider(Vector3(1.0f, 1.0f, 3.0f), m_mass);
    }

    // PendingVelocityがあれば反映
    SetVelocity(m_PendingVelocity);
    m_PendingVelocity = Vector3(0, 0, 0);

    // チェーンシステムを作成
    if (m_Owner && !m_ChainSystem)
    {
        m_ChainSystem = Manager::GetScene()->AddGameObject<ChainSystem>(OBJECT);
        m_ChainSystem->Init();

        // プレイヤーとアンカーの間にチェーンを作成
        // パラメータ: 物理リンク数6個、半径0.05、長さ0.3、質量0.1
        m_ChainSystem->CreateChain(m_Owner, this, 20, 0.3f, 0.3f, 0.5f);
    }
   
}


void Anchor::Uninit()
{
    // チェーンシステムを削除
    if (m_ChainSystem)
    {
        m_ChainSystem->DestroyChain();
        m_ChainSystem->SetDestroy();
        m_ChainSystem = nullptr;
    }

    m_Joint = nullptr;
    m_IsPulling = false;
    m_Attached = false;
    m_AttachedTarget = nullptr;

    // その後でRigidBodyを削除
    if (m_RigidBody)
    {
        PhysicsObject::Uninit(); // RigidBody削除（ジョイントは既に削除済み）
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
        if (m_Started)
        {
            // 引き寄せ処理
            if (m_IsPulling && m_Owner)
            {
                Vector3 ownerPos = m_Owner->GetPosition();

                // 自分自身を引き寄せる場合（静的オブジェクトに当たった）
                if (m_PullingSelf)
                {
                    Vector3 anchorPos = this->GetPosition();
                    Vector3 direction = ownerPos - anchorPos;
                    float distance = direction.Length();

                    if (distance > m_PullDistance)
                    {
                        direction.Normalize();
                        btVector3 pullForce(direction.x * m_PullForce, direction.y * m_PullForce, direction.z * m_PullForce);

                        if (m_RigidBody) {
                            m_RigidBody->applyCentralForce(pullForce);
                            m_RigidBody->activate(true);
                        }
                    }
                    else
                    {
                        NotifyOwnerAnchorRemoved();
                        StopPulling();
                        SetDestroy();
                    }
                }
                // 物体を引き寄せる場合
                else if (m_Attached && m_AttachedTarget)
                {
                    Vector3 targetPos = m_AttachedTarget->GetPosition();
                    Vector3 direction = ownerPos - targetPos;
                    float distance = direction.Length();

                    if (distance > m_PullDistance)
                    {
                        direction.Normalize();
                        btVector3 pullForce(direction.x * m_PullForce, direction.y * m_PullForce, direction.z * m_PullForce);

                        PhysicsObject* targetPhysics = dynamic_cast<PhysicsObject*>(m_AttachedTarget);
                        if (targetPhysics && targetPhysics->GetRigidBody())
                        {
                            targetPhysics->GetRigidBody()->applyCentralForce(pullForce);
                            targetPhysics->GetRigidBody()->activate(true);
                        }
                    }
                    else
                    {
                        NotifyOwnerAnchorRemoved();
                        StopPulling();
                        Detach();
                        SetDestroy();
                    }
                }
            }

            // チェーンシステムを更新（両端の位置が変わっている）
            if (m_ChainSystem)
            {
                m_ChainSystem->UpdateChain();
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

    // 質量が0（静的オブジェクト）の場合はジョイントせずに即座にpulling開始
    float targetMass = targetPhysics->GetMass();
    if (targetMass == 0.0f) {;
        // ジョイントは作らず、自分自身をプレイヤーに引き寄せる
        m_Attached = false;
        m_AttachedTarget = nullptr;
        m_IsPulling = true;
        m_PullingSelf = true;  // 自分自身を引き寄せる

        // 速度を0にして制御可能にする
        if (m_RigidBody) {
            m_RigidBody->setLinearVelocity(btVector3(0, 0, 0));
            m_RigidBody->setAngularVelocity(btVector3(0, 0, 0));
        }
        return; // ここで終了、ジョイントは作らない
    }

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
    m_PullingSelf = false;  // 物体を引き寄せる

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

void Anchor::NotifyOwnerAnchorRemoved()
{
    if (m_Owner) {
        class FPSPlayer* player = dynamic_cast<class FPSPlayer*>(m_Owner);
        if (player) {
            player->OnAnchorDestroyed(this);  
        }
    }
}