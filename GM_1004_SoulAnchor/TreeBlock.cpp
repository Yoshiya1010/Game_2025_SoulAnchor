#include"TreeBlock.h"
#include "manager.h"
#include "camera.h"
#include "input.h"
#include"explosion.h"
#include "PhysicsManager.h"
#include "TriangleMeshBuilder.h"
#include "MeshDestroyer.h"

void TreeBlock::Init()
{
    // モデルのロード
    m_ModelRenderer = new ModelRenderer();
    m_ModelRenderer->Load("asset\\model\\BullutObject\\tree_pineTallA.obj");


    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitColorVS.cso");

    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitColorPS.cso");



    m_Started = false;

    SetName("TreeBlock");

    // トライアングルメッシュを使用することを設定
    m_UseTriangleMesh = true;

    SetTag(GameObjectTag::Ground);



}
void TreeBlock::Start()
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


void TreeBlock::Uninit()
{
    if (m_RigidBody) 
    {
        PhysicsObject::Uninit();//Rigtbodyを消す
    }

    if (m_VertexLayout)     m_VertexLayout->Release();
    if (m_VertexShader)     m_VertexShader->Release();
    if (m_PixelShader)      m_PixelShader->Release();
}

void TreeBlock::Update()
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

void TreeBlock::Draw()
{

    // まずこのオブジェクト用のレイアウト＆シェーダを必ずセット
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

    // UnlitColor はテクスチャ使わないのでスロットをクリア
    ID3D11ShaderResourceView* nullSRV = nullptr;
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

    Renderer::SetWorldMatrix(
        //モデルと物理の座標を同期させる
        UpdatePhysicsWithModel(1.0f));
    // モデルの描画
    m_ModelRenderer->Draw();

}


