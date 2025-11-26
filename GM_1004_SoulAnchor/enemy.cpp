#include "enemy.h"
#include "manager.h"
#include "modelRenderer.h"
#include "camera.h"
#include "input.h"


void Enemy::Init()
{
    m_AnimationModel = make_unique<AnimationModel>();
    m_AnimationModel->Load("asset\\model\\Enemy\\character-skeleton.fbx");
    m_AnimationModel->LoadAllAnimations("asset\\model\\Enemy\\character-skeleton.fbx");

    SetShaderType(ShaderType::UNLIT_TEXTURE);
    SetScale(Vector3(0.01f, 0.01f, 0.01f));
    SetPosition(Vector3(0.0f, 1.0f, 0.0f));

    std::vector<std::string> animNames = m_AnimationModel->GetAnimationNames();
    if (!animNames.empty())
    {
        m_AnimationModel->Play(animNames[0].c_str(), true);
        m_AnimationModel->Update();  // 初回更新で頂点を変換
        OutputDebugStringA("初回Update実行\n");
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
    //カリングを一時的に無効化（テスト用）
    ID3D11RasterizerState* oldState;
    Renderer::GetDeviceContext()->RSGetState(&oldState);

    D3D11_RASTERIZER_DESC rasterizerDesc;
    ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;  // カリングなし
    rasterizerDesc.FrontCounterClockwise = FALSE;

    ID3D11RasterizerState* rasterizerState;
    Renderer::GetDevice()->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
    Renderer::GetDeviceContext()->RSSetState(rasterizerState);

    XMMATRIX S_p = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
    XMMATRIX R_p = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
    XMMATRIX T_p = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
    XMMATRIX parentWorld = S_p * R_p * T_p;
    Renderer::SetWorldMatrix(parentWorld);
    m_AnimationModel->Draw();

    //元に戻す
    Renderer::GetDeviceContext()->RSSetState(oldState);
    if (oldState) oldState->Release();
    if (rasterizerState) rasterizerState->Release();

}

AnimationModel* Enemy::GetAnimationModel()
{
    return m_AnimationModel.get();
}