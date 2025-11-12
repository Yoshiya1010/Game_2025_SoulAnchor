#include"RockTallBlock_A.h"
#include "manager.h"
#include "camera.h"
#include "input.h"
#include"explosion.h"
#include "PhysicsManager.h"
#include "animationModel.h"
#include "TriangleMeshBuilder.h"
#include "MeshDestroyer.h"



void RockTallBlock_A::Init()
{


    // モデルのロード
    m_ModelRenderer = new ModelRenderer();
    m_ModelRenderer->Load("asset\\model\\BullutObject\\rock_tallA.obj");



    SetShaderType(ShaderType::TOON_SHADOW);

    m_Started = false;

    // トライアングルメッシュを使用することを設定
    m_UseTriangleMesh = true;

    SetName("RockTallBlock_A");

    SetTag(GameObjectTag::Ground);


}
void RockTallBlock_A::Start()
{
    FragmentObject::Start();
}


void RockTallBlock_A::Uninit()
{
    // 物理オブジェクトの削除
    if (m_RigidBody && PhysicsManager::GetWorld()) {
        PhysicsManager::GetWorld()->removeRigidBody(m_RigidBody.get());
        m_RigidBody->setUserPointer(nullptr);
    }

    m_RigidBody.reset();
    m_MotionState.reset();
    m_CollisionShape.reset();

}

void RockTallBlock_A::Update()
{
    CheckAndCallStart();
    if (m_Started)
    {

        if (Input::GetKeyTrigger(KK_G))
        {
            DestroyObject(Vector3());
        }

    }
}

void RockTallBlock_A::Draw()
{

    if (!m_ModelRenderer)return;

    // UnlitColor はテクスチャ使わないのでスロットをクリア（保険）
    ID3D11ShaderResourceView* nullSRV = nullptr;
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

    Renderer::SetWorldMatrix(
        //モデルと物理の座標を同期させる
        UpdatePhysicsWithModel(1.0f));
    // モデルの描画
    m_ModelRenderer->Draw();

}




