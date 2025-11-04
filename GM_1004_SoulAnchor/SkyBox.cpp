#include"SkyBox.h"
#include "manager.h"
#include "modelRenderer.h"
#include"scene.h"
#include"FPSCamera.h"



void SkyBox::Init()
{
    m_ModelRenderer = new ModelRenderer();
    m_ModelRenderer->Load("asset\\model\\sky.obj");

    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");

    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");

    SetName("SkyBox");

    m_Scale = Vector3(100.f, 100.f, 100.f);

}

void SkyBox::Uninit()
{
    delete m_ModelRenderer;
    if (m_VertexLayout)     m_VertexLayout->Release();
    if (m_VertexShader)     m_VertexShader->Release();
    if (m_PixelShader)      m_PixelShader->Release();
}

void SkyBox::Update()
{
    FPSCamera* camera = Manager::GetScene()->GetGameObject<FPSCamera>();

    m_Position = camera->GetPosition();


}

void SkyBox::Draw()
{
    //SkyBoxの高さを調整
    float HorizonHeight = m_Scale.y / 4;
    // 入力レイアウト設定
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

    // シェーダー設定
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

    // マトリクス設定
    XMMATRIX world, scale, rot, trans;
    scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
    rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y + XM_PI, m_Rotation.z);
    trans = XMMatrixTranslation(m_Position.x, m_Position.y- HorizonHeight, m_Position.z);
    world = scale * rot * trans;
    Renderer::SetWorldMatrix(world);

    // モデルの描画
    m_ModelRenderer->Draw();

}