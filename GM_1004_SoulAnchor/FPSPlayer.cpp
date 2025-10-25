#include"FPSPlayer.h"
#include "manager.h"
#include "modelRenderer.h"
#include "audio.h"
#include"FPSCamera.h"
#include "input.h"
#include "bullet.h"
#include "scene.h"
#include "animationModel.h"

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
        CreateBoxCollider(Vector3(1.0f, 2.0f, 1.0f), 1.0f);


        //こけないように追加
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

        // Rigidbodyに速度を適用
        m_RigidBody->setLinearVelocity(velocity);

        // カメラ方向をプレイヤー向きに反映（水平のみ）
        m_Rotation.y = camera->GetRotation().y;

      
        m_RigidBody->activate(true);

        // 弾発射（カメラの正面方向）
        if (Input::GetKeyTrigger(KK_SPACE)) {
            Bullet* bullet = Manager::GetScene()->AddGameObject<Bullet>(OBJECT);
            bullet->SetPosition(m_Position + camForward * 1.0f);
            bullet->SetVelocity(camForward * 0.5f);
            m_SE->Play();
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