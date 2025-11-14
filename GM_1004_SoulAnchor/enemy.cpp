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
    m_AnimationModel->LoadAllAnimations("asset\\model\\GOLEM.fbx");

    SetShaderType(ShaderType::UNLIT_TEXTURE);



    SetScale(Vector3(0.01f, 0.01f, 0.01f));
    std::vector<std::string> animNames = m_AnimationModel->GetAnimationNames();
    if (!animNames.empty())
    {
        m_AnimationModel->Play(animNames[0].c_str(), true);
    }
}

void Enemy::Uninit()
{
    m_AnimationModel->Uninit();

}

void Enemy::Update()
{
    m_AnimationModel->Update();
}

void Enemy::Draw()
{
    XMMATRIX S_p = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
    XMMATRIX R_p = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
    XMMATRIX T_p = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
    XMMATRIX parentWorld = S_p * R_p * T_p;

    Renderer::SetWorldMatrix(parentWorld);
    m_AnimationModel->Draw();

}

AnimationModel* Enemy::GetAnimationModel()
{
    return m_AnimationModel.get();
}