// TreeBlock.cpp
// 木のオブジェクト - Boxコライダー版（動的可能、倒れる）

#include "TreeBlock.h"
#include "manager.h"
#include "camera.h"
#include "input.h"
#include "explosion.h"
#include "PhysicsManager.h"

void TreeBlock::Init()
{
    // モデルのロード
    LoadModel("asset\\model\\BullutObject\\tree_pineTallA.obj");

    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitColorVS.cso");

    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitColorPS.cso");

    m_Started = false;

    SetName("TreeBlock");



    // オプション1: Boxコライダー（動的可能、倒れる）
    m_UseTriangleMesh = false; 
    SetMass(50.0f);            



    // スケール設定
    SetScale(Vector3(1.0f, 1.0f, 1.0f));

    // 破壊設定
    SetDestructible(true);
    SetDestructionThreshold(15.0f);
    SetGroupSize(5);
    SetExplosionForce(15.0f);

    SetTag(GameObjectTag::Ground);
}

void TreeBlock::Start()
{
    // 親クラスのStart()を呼ぶだけ
    // コライダーの作成は自動的に行われる
    FragmentObject::Start();
}

void TreeBlock::Uninit()
{
    if (m_RigidBody)
    {
        PhysicsObject::Uninit();
    }

    if (m_VertexLayout) m_VertexLayout->Release();
    if (m_VertexShader) m_VertexShader->Release();
    if (m_PixelShader) m_PixelShader->Release();
}

void TreeBlock::Update()
{
    CheckAndCallStart();
    if (m_Started)
    {
        // テスト用: Gキーで破壊
        if (Input::GetKeyTrigger(KK_G))
        {
            DestroyObject(m_Position);
        }
    }
}

void TreeBlock::Draw()
{
    if (!m_ModelRenderer) return;

    // レイアウト・シェーダをセット
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

    // UnlitColor用（テクスチャスロットをクリア）
    ID3D11ShaderResourceView* nullSRV = nullptr;
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

    // ワールド行列を設定
    Renderer::SetWorldMatrix(
        UpdatePhysicsWithModel(m_ModelScale)
    );

    // モデルの描画
    m_ModelRenderer->Draw();
}