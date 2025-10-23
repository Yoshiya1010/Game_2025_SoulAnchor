#include"TreeBlock.h"
#include "manager.h"
#include "camera.h"
#include "input.h"
#include"explosion.h"
#include "PhysicsManager.h"
#include "animationModel.h"
#include"ModelFBX.h"

void TreeBlock::Init()
{
    // モデルのロード
    m_ModelRenderer = std::make_unique<StaticFBXModel>();
    m_ModelRenderer->Load("asset\\model\\tree_pineTallA.fbx");


    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitColorVS.cso");

    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitColorPS.cso");



    m_Started = false;

    SetName("TreeBlock");


}
void TreeBlock::Start()
{
    if (m_RigidBody) return;
    // 物理コライダーの設定
    if (PhysicsManager::GetWorld()) {
        // 衝突レイヤー設定（任意）
        SetupCollisionLayer();


        m_ColliderOffset = Vector3(0, 7.f, 0);
        CreateBoxCollider(Vector3(1.0f, 7.0f, 1.0f), 0.0f);

    }


}


void TreeBlock::Uninit()
{
    if (m_RigidBody) 
    {
        PhysicsObject::Uninit();
    }

    m_ModelRenderer.reset();
    if (m_VertexLayout)     m_VertexLayout->Release();
    if (m_VertexShader)     m_VertexShader->Release();
    if (m_PixelShader)      m_PixelShader->Release();
}

void TreeBlock::Update()
{
    CheckAndCallStart();
    if (m_Started)
    {


    }
}

void TreeBlock::Draw()
{

    // まずこのオブジェクト用のレイアウト＆シェーダを必ずセット
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

    // UnlitColor はテクスチャ使わないのでスロットをクリア（保険）
    ID3D11ShaderResourceView* nullSRV = nullptr;
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

    Renderer::SetWorldMatrix(
        //モデルと物理の座標を同期させる
        UpdatePhysicsWithModel(m_modelScale));
    // モデルの描画
    m_ModelRenderer->Draw();

}
