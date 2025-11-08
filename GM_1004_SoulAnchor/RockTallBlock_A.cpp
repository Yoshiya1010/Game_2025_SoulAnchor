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
    m_ModelRenderer->Load("asset\\model\\rock_tallA.obj");


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

    delete m_ModelRenderer;
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
            DestroyRock(Vector3());
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
        UpdatePhysicsWithModel(m_modelScale));
    // モデルの描画
    m_ModelRenderer->Draw();

}


void RockTallBlock_A::OnCollisionEnter(GameObject* other, const Vector3& hitPoint)
{
    // 既に破壊されている、または破壊不可能な場合は何もしない
    if (m_IsDestroyed || !m_Destructible) return;



    if (other->GetTag() == GameObjectTag::Anchor) {
        // アンカーとの衝突時に破壊チェック
        DestroyRock(hitPoint);
      
    }
}

void RockTallBlock_A::DestroyRock(const Vector3& impactPoint)
{
    // 既に破壊済みならスキップ
    if (m_IsDestroyed) return;
    m_IsDestroyed = true;

    // シーンを取得（実際のプロジェクトの取得方法に合わせて修正してください）
    Scene* scene = Manager::GetScene();

    if (!scene || !m_ModelRenderer) {
        SetDestroy();
        return;
    }

    // モデルを取得
    MODEL* model = m_ModelRenderer->GetModel();

    if (!model) {
        SetDestroy();
        return;
    }

    // ワールド行列を計算
    XMMATRIX worldMatrix =
        XMMatrixScaling(m_Scale.x * m_modelScale, m_Scale.y * m_modelScale, m_Scale.z * m_modelScale) *
        XMMatrixRotationRollPitchYaw(
            m_Rotation.x * DEG2RAD,
            m_Rotation.y * DEG2RAD,
            m_Rotation.z * DEG2RAD
        ) *
        XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

    // 爆発の中心は衝突点
    Vector3 explosionCenter = impactPoint;

    // 爆発の力
    float explosionForce = 15.0f;

    // グループ化して破壊（パフォーマンス考慮）
    // 岩は複雑なメッシュなのでグループサイズを大きめに設定
    MeshDestroyer::DestroyModelGrouped(
        model,
        worldMatrix,
        explosionCenter,
        explosionForce,
        scene,
        5  // 5個の三角形を1グループに（調整可能）
    );

    // 自分自身を削除
    SetDestroy();
}