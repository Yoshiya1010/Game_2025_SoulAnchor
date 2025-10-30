#include"FPSPlayer.h"
#include "manager.h"
#include "modelRenderer.h"
#include "audio.h"
#include"FPSCamera.h"
#include "input.h"
#include "bullet.h"
#include "scene.h"
#include "animationModel.h"
#include"Anchor.h"
#include "meshField.h"

void FPSPlayer::Init()
{
   

    // モデルのロード
    m_AnimationModel = make_unique<AnimationModel>();
    m_AnimationModel->Load("asset\\model\\Akai.fbx");

    // モデルのアニメーションをロード
    m_AnimationModel->LoadAnimation("asset\\model\\Akai_Run.fbx", "Run");
    m_AnimationModel->LoadAnimation("asset\\model\\Akai_Idle.fbx", "Idle");

    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");

    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");

    // Audioの初期化
    m_SE = make_unique<Audio>();
    m_SE->Load("asset\\audio\\wan.wav");

    m_Scale = Vector3(1.0f, 1.0f, 1.0f);

    m_Frame = 0;

    m_AnimationName = "Idle";
    m_AnimationNameNext = "Run";
    m_AnimationBlend = 0.0f;

    SetTag(GameObjectTag::Player);


    SetName("FPSPlayer");
}


void FPSPlayer::Start()
{
    if (m_RigidBody) return;
    // 物理コライダーの設定
    if (PhysicsManager::GetWorld()) {
        // 衝突レイヤー設定
        SetupCollisionLayer();


        m_ColliderOffset = Vector3(0, 1.f, 0);
        CreateBoxCollider(Vector3(1.0f, 2.0f, 1.0f), 70.0f);


        //こけないように　回転を一部固定
        m_RigidBody->setAngularFactor(btVector3(0, 1, 0));
    }
}

void FPSPlayer::Uninit()
{
    //Bodyは削除
    if (m_RigidBody)
    {
        PhysicsObject::Uninit();
    }

    m_SE->Uninit();
    m_SE.reset();

    m_AnimationModel->Uninit();
    m_AnimationModel.reset();

    if (m_VertexLayout)     m_VertexLayout->Release();
    if (m_VertexShader)     m_VertexShader->Release();
    if (m_PixelShader)      m_PixelShader->Release();
}

void FPSPlayer::Update()
{
    CheckAndCallStart();

    if (m_Started)
    {
        FPSCamera* camera = Manager::GetScene()->GetGameObject<FPSCamera>();
        if (!camera) return;

        m_RigidBody->activate(true);

        Vector3 moveDir = { 0,0,0 };
        bool isMoving = false;

        // カメラの向き基準
        Vector3 camForward = camera->GetForward();
        Vector3 camRight = camera->GetRight();
        camForward.y = 0;
        camRight.y = 0;

        if (Input::GetKeyPress(KK_W)) { moveDir += camForward; isMoving = true; }
        if (Input::GetKeyPress(KK_S)) { moveDir -= camForward; isMoving = true; }
        if (Input::GetKeyPress(KK_A)) { moveDir -= camRight; isMoving = true; }
        if (Input::GetKeyPress(KK_D)) { moveDir += camRight; isMoving = true; }

        btVector3 velocity(0, 0, 0);
        if (isMoving)
        {
            moveDir.Normalize();
            velocity = btVector3(moveDir.x, moveDir.y, moveDir.z) * 5.0f;
        }

        btVector3 currentVel = m_RigidBody->getLinearVelocity();
        btVector3 newVel = btVector3(velocity.x(), currentVel.getY(), velocity.z());
        m_RigidBody->setLinearVelocity(newVel);

        // カメラ方向をプレイヤー向きに反映（水平のみ）
        m_Rotation.y = camera->GetRotation().y;

      
        // アンカー発射
        if (Input::GetKeyTrigger(KK_T)) {
            FPSCamera* camera = Manager::GetScene()->GetGameObject<FPSCamera>();
            ThrowAnchor(camera);
        }


        //ジャンプの処理
        if (Input::GetKeyTrigger(KK_SPACE) && m_IsOnGround)
        {
             m_RigidBody->applyCentralImpulse(btVector3(0, 10.0f, 0)); // 上方向に力を加える
            m_RigidBody->activate(true);
            m_IsOnGround = false; // 空中に出たのでリセット
        }
    }


}

void FPSPlayer::Draw()
{

    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

    Renderer::SetWorldMatrix(
        //モデルと物理の座標を同期させる
        UpdatePhysicsWithModel(m_modelScale));
    m_AnimationModel->Draw();


}

void FPSPlayer::ThrowAnchor(FPSCamera* camera)
{
    if (m_CurrentAnchor) return; // すでに存在する場合は無視

    // Anchor生成
    Anchor* anchor = Manager::GetScene()->AddGameObject<Anchor>(OBJECT);

    // プレイヤー位置＋少し前方に配置
    Vector3 camForward = camera->GetForward();
    
    Vector3 spawnPos = Vector3((m_Position.x + (camForward.x * 2.0f)), (m_Position.y + 2.0f+ (camForward.y * 2.0f)), (m_Position.z + (camForward.z * 2.0f)));
    anchor->SetPosition(spawnPos);


    // 飛ばす方向の速度設定（Start後に反映される仕組み）
    anchor->SetVelocity(camForward * 50.0f);

    // 所持中アンカー登録
    m_CurrentAnchor = anchor;
}
void FPSPlayer::PullAnchor() {
    if (!m_CurrentAnchor) return;

    // Anchorが何かにジョイント済みなら、引っ張る
    if (m_CurrentAnchor->IsAttached()) {
        Vector3 dir = (m_CurrentAnchor->GetPosition() - m_Position);
        dir.Normalize();
        m_RigidBody->applyCentralImpulse(btVector3(dir.x, dir.y, dir.z) * 500.0f);
    }
}

void FPSPlayer::ReleaseAnchor() {
    if (!m_CurrentAnchor) return;
    m_CurrentAnchor->Detach();
    m_CurrentAnchor = nullptr;
}