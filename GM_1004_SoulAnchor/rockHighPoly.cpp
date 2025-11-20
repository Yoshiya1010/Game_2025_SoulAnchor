// TreeBlock.cpp
// 木のオブジェクト - Boxコライダー版（動的可能、倒れる）

#include"rockHighPoly.h"
#include "manager.h"
#include "camera.h"
#include "input.h"
#include "explosion.h"
#include "PhysicsManager.h"

void RockHighPoly::Init()
{
    // モデルのロード
    LoadModel("asset\\model\\BullutObject\\boulder_01_4k.obj");

    SetShaderType(ShaderType::TOON_SHADOW);


    // オプション1: Boxコライダー（動的可能、倒れる）
    m_UseTriangleMesh = false;
    SetMass(50.0f);



    // スケール設定
    SetScale(Vector3(1.0f, 1.0f, 1.0f));



    // 破壊設定
    SetDestructible(true);
    SetDestructionThreshold(15.0f);
    SetGroupSize(5);
    SetExplosionForce(0.0f);

    m_Started = false;
    SetTag(GameObjectTag::Ground);
    SetName("TreeBlock");

}

void RockHighPoly::Start()
{
    // 親クラスのStart()を呼ぶだけ
    // コライダーの作成は自動的に行われる
    FragmentObject::Start();
}

void RockHighPoly::Uninit()
{
    if (m_RigidBody)
    {
        PhysicsObject::Uninit();
    }
}

void RockHighPoly::Update()
{
    CheckAndCallStart();

    if (Input::GetKeyTrigger(KK_L))
    {
        DestroyObject(Vector3());
    }

}

void RockHighPoly::Draw()
{
    if (!m_ModelRenderer) return;



    // UnlitColor用（テクスチャスロットをクリア）
    ID3D11ShaderResourceView* nullSRV = nullptr;
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

    // ワールド行列を設定
    Renderer::SetWorldMatrix(
        UpdatePhysicsWithModel()
    );

    // モデルの描画
    m_ModelRenderer->Draw();
}
