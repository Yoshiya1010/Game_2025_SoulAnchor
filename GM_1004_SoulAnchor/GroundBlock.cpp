#include"GroundBlock.h"
#include "manager.h"
#include "camera.h"
#include "input.h"
#include"explosion.h"
#include "PhysicsManager.h"
#include "animationModel.h"
#include"ModelFBX.h"

void GroundBlock::Init()
{
  

    // モデルのロード
  
      // モデルのロード
    LoadModel("asset\\model\\BullutObject\\cliff_block_stone.obj");

    SetShaderType(ShaderType::TOON_SHADOW);


    // オプション1: Boxコライダー（動的可能、倒れる）
    m_UseTriangleMesh = false;
    SetMass(0.0f);



    // スケール設定
    SetScale(Vector3(1.0f, 1.0f, 1.0f));

    // 破壊設定
    SetDestructible(false);
    SetDestructionThreshold(15.0f);
    SetGroupSize(1);
    SetExplosionForce(0.0f);



    m_Started = false;

    SetName("GroundBlock");

    SetTag(GameObjectTag::Ground);


}
void GroundBlock::Start()
{
    FragmentObject::Start();
}


void GroundBlock::Uninit()
{
    if (m_RigidBody)
    {
        PhysicsObject::Uninit();
    }

 
}

void GroundBlock::Update()
{
    CheckAndCallStart();
    if (m_Started)
    {
       


    }
}

void GroundBlock::Draw()
{

    if (!m_ModelRenderer) return;
    // UnlitColor はテクスチャ使わないのでスロットをクリア（保険）
  

    Renderer::SetWorldMatrix( 
        //モデルと物理の座標を同期させる
        UpdatePhysicsWithModel(1.0));
    // モデルの描画
    m_ModelRenderer->Draw();

}
