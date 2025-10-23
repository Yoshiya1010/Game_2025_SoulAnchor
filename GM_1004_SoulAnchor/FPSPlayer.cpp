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
    m_ModelRenderer = make_unique<ModelRenderer>();
    m_ModelRenderer->Load("asset\\model\\player.obj");

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

    m_Scale = Vector3(0.015f, 0.015f, 0.015f);

    m_Frame = 0;

    m_AnimationName = "Idle";
    m_AnimationNameNext = "Run";
    m_AnimationBlend = 0.0f;




    SetName("FPSPlayer");
}

void FPSPlayer::Uninit()
{
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

    if (isMoving)
    {
        moveDir.Normalize();
        m_Position += moveDir * 0.1f;
    }

    // カメラ方向をプレイヤー向きに反映（水平のみ）
    m_Rotation.y = camera->GetRotation().y;

    // 弾発射（カメラの正面方向）
    if (Input::GetKeyTrigger(KK_SPACE)) {
        Bullet* bullet = Manager::GetScene()->AddGameObject<Bullet>(OBJECT);
        bullet->SetPosition(m_Position + camForward * 1.0f);
        bullet->SetVelocity(camForward * 0.5f);
        m_SE->Play();
    }


}

void FPSPlayer::Draw()
{

    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

    {
        XMMATRIX S_p = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
        XMMATRIX R_p = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
        XMMATRIX T_p = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
        XMMATRIX parentWorld = S_p * R_p * T_p;

        Renderer::SetWorldMatrix(parentWorld);
        m_AnimationModel->Draw();

        // 子のローカル変換（親からの相対だけを入れる）
        Vector3 childLocalPos = { 0.0f, 0.0f, 0.0f };   // 親の前方50cm
        Vector3 childLocalRot = { 0.0f, 0.0f, 0.0f };   // 回転なし
        Vector3 childLocalScale = { m_Scale.x * 1000.0f, m_Scale.y * 1000.0f, m_Scale.z * 1000.0f }; //行列に*100はしない！

        XMMATRIX S_c = XMMatrixScaling(childLocalScale.x, childLocalScale.y, childLocalScale.z);
        XMMATRIX R_c = XMMatrixRotationRollPitchYaw(childLocalRot.x, childLocalRot.y, childLocalRot.z);
        XMMATRIX T_c = XMMatrixTranslation(childLocalPos.x, childLocalPos.y, childLocalPos.z);

        // 子の最終ワールド： 子ローカル * 親ワールド
        XMMATRIX childWorld = S_c * R_c * T_c * parentWorld;

        Renderer::SetWorldMatrix(childWorld);
        m_ModelRenderer->Draw();


    }
}