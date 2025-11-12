#include "enemy.h"
#include "manager.h"
#include "modelRenderer.h"
#include "camera.h"
#include "input.h"


void Enemy::Init()
{
    m_AnimationModel = make_unique<AnimationModel>();
    m_AnimationModel->Load("asset\\model\\GOLEM.fbx");

    // モデルのアニメーションをロード
    m_AnimationModel->LoadAnimation("asset\\model\\GOLEM.fbx", "Punch");
    m_AnimationModel->LoadAnimation("asset\\model\\GOLEM.fbx", "LeanBack");
    m_AnimationModel->LoadAnimation("asset\\model\\GOLEM.fbx", "FallDown");
    m_AnimationModel->LoadAnimation("asset\\model\\GOLEM.fbx", "Break");
    m_AnimationModel->LoadAnimation("asset\\model\\GOLEM.fbx", "Idle");
    m_AnimationModel->LoadAnimation("asset\\model\\GOLEM.fbx", "Walk");
    m_AnimationModel->LoadAnimation("asset\\model\\GOLEM.fbx", "Run");



    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");

    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");


    m_AnimationName = "Run";
    m_AnimationNameNext = "Run";
    m_AnimationBlend = 0.0f;
}

void Enemy::Uninit()
{
    m_AnimationModel->Uninit();
    if (m_VertexLayout)     m_VertexLayout->Release();
    if (m_VertexShader)     m_VertexShader->Release();
    if (m_PixelShader)      m_PixelShader->Release();
}

void Enemy::Update()
{
    m_AnimationModel->Update(m_AnimationName.c_str(), m_Frame, m_AnimationNameNext.c_str(), m_Frame, m_AnimationBlend);
    m_Frame++;

    m_AnimationBlend += 0.1f;
    if (m_AnimationBlend > 1.0f) {
        m_AnimationBlend = 1.0f;
    }
}

void Enemy::Draw()
{
    // 入力レイアウト設定
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);
  

    XMMATRIX S_p = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
    XMMATRIX R_p = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
    XMMATRIX T_p = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
    XMMATRIX parentWorld = S_p * R_p * T_p;

    Renderer::SetWorldMatrix(parentWorld);
    m_AnimationModel->Draw();

}