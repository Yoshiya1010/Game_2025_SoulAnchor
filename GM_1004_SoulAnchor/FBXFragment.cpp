#include"FBXFragment.h"
#include"manager.h"

void FBXFragment::Init()
{
    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");
    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");

    // モデル読み込み
    if (!m_ModelPath.empty()) {
        m_ModelRenderer = std::make_unique<StaticFBXModel>();
        m_ModelRenderer->Load(m_ModelPath.c_str());
    }

    SetName("FBXFragment");
    SetTag(GameObjectTag::Ground);  // 破片同士衝突しない

    // Init()で即座にRigidBody生成（これ重要！）
    if (PhysicsManager::GetWorld()) {
        SetupCollisionLayer();

        // スケールに基づいたBoxCollider
        // m_Scaleは既に小さい値になっている
        CreateBoxCollider(m_Scale, 1.0f);  // 質量1kg

        if (m_RigidBody) {
            // 空気抵抗（すぐ止まる）
            m_RigidBody->setDamping(0.6f, 0.8f);
        }

        printf("[FBXFragment] Created: pos=(%.2f,%.2f,%.2f) scale=(%.2f,%.2f,%.2f) modelScale=%.2f\n",
            m_Position.x, m_Position.y, m_Position.z,
            m_Scale.x, m_Scale.y, m_Scale.z,
            m_ModelScale);
    }
}

void FBXFragment::Start()
{
    if (!m_ModelPath.empty() && !m_ModelLoaded) {
        m_ModelRenderer = std::make_unique<StaticFBXModel>();
        m_ModelRenderer->Load(m_ModelPath.c_str());
        m_ModelLoaded = true;

        printf("[FBXFragment] Model loaded: %s at pos=(%.2f,%.2f,%.2f) scale=(%.2f,%.2f,%.2f)\n",
            m_ModelPath.c_str(),
            m_Position.x, m_Position.y, m_Position.z,
            m_Scale.x, m_Scale.y, m_Scale.z);
    }
    else if (m_ModelPath.empty()) {
        printf("[FBXFragment ERROR] Model path not set!\n");
    }
}

void FBXFragment::Uninit()
{
    if (m_RigidBody) {
        PhysicsObject::Uninit();
    }

    m_ModelRenderer.reset();
    if (m_VertexLayout) m_VertexLayout->Release();
    if (m_VertexShader) m_VertexShader->Release();
    if (m_PixelShader) m_PixelShader->Release();
}

void FBXFragment::Update()
{
    CheckAndCallStart();

    if (m_Started) {
        m_Timer += 0.016f;  // デルタタイム

        //// 寿命切れで削除
        //if (m_Timer > m_Lifetime) {
        //    SetDestroy();
        //    return;
        //}

        //// 静止したら削除（パフォーマンス向上）
        //if (m_RigidBody) {
        //    Vector3 vel = GetVelocity();
        //    if (vel.Length() < 0.1f && m_Timer > 1.0f) {
        //        SetDestroy();
        //    }
        //}
    }
}

void FBXFragment::Draw()
{
    if (!m_ModelRenderer) return;

    // シェーダー設定
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

    // ワールド行列設定（物理と同期、モデルスケールも考慮）
    Renderer::SetWorldMatrix(UpdatePhysicsWithModel(m_ModelScale));

    // FBXモデルの描画
    m_ModelRenderer->Draw();
}