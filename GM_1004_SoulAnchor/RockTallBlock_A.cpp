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


    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");

    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");



    m_Started = false;

    SetName("RockTallBlock_A");

    SetTag(GameObjectTag::Ground);


}
void RockTallBlock_A::Start()
{
    if (m_RigidBody) return;
    // 物理コライダーの設定
    if (PhysicsManager::GetWorld()) {
        // 衝突レイヤー設定
        SetupCollisionLayer();

        // トライアングルメッシュコライダーを作成
        btBvhTriangleMeshShape* shape = CreateTriangleMeshShape(m_ModelRenderer->GetModel());

        // PhysicsObjectのm_CollisionShapeに設定
        m_CollisionShape = std::unique_ptr<btCollisionShape>(shape);

        // RigidBodyを作成（質量0で静的オブジェクト）
        CreateRigidBody(0.0f);


        // 破壊設定（オプション）
        SetDestructionThreshold(15.0f);
        SetGroupSize(5);
        SetExplosionForce(15.0f);
    }


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

   
    if (m_VertexLayout)     m_VertexLayout->Release();
    if (m_VertexShader)     m_VertexShader->Release();
    if (m_PixelShader)      m_PixelShader->Release();
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

    // まずこのオブジェクト用のレイアウト＆シェーダを必ずセット
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

    // UnlitColor はテクスチャ使わないのでスロットをクリア（保険）
    ID3D11ShaderResourceView* nullSRV = nullptr;
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

    Renderer::SetWorldMatrix(
        //モデルと物理の座標を同期させる
        UpdatePhysicsWithModel(1.0f));
    // モデルの描画
    m_ModelRenderer->Draw();

}




