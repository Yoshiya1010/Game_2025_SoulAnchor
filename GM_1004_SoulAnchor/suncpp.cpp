#include "Sun.h"
#include "manager.h"
#include "input.h"
#include "imgui.h"

void Sun::Init()
{
    SetName("Sun");

    // 太陽のモデルを読み込み（球体）
    m_ModelRenderer = std::make_unique<ModelRenderer>();
    m_ModelRenderer->Load("asset\\model\\bullet.obj");  // 球体モデルのパス
    m_ModelScale = 3.0f;  // 表示サイズ

    // シェーダーの読み込み（UnlitColorを使用）
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitColorVS.cso");
    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitColorPS.cso");

    // 初期パラメータ設定
    m_Position = Vector3(50.0f, 100.0f, 50.0f);      // 太陽の位置
    m_TargetPosition = Vector3(0.0f, 0.0f, 0.0f);    // 見ている場所
    m_LightDirection = Vector3(1.0f, -1.0f, 0.5f);   // ライトの方向
    m_Intensity = 1.5f;                               // 光の強さ
    m_AmbientStrength = 0.3f;                         // 環境光
    m_OrthoSize = 100.0f;                             // シャドウマップの範囲

    // 初期設定を反映
    ApplyLight();
}

void Sun::Uninit()
{
    if (m_VertexLayout) m_VertexLayout->Release();
    if (m_VertexShader) m_VertexShader->Release();
    if (m_PixelShader) m_PixelShader->Release();
}

void Sun::Update()
{
    //// ImGuiウィンドウの表示
    //if (m_ShowImGui)
    //{
    //    ImGui::Begin("Sun Control", &m_ShowImGui);

    //    // 位置の調整
    //    ImGui::Text("Position");
    //    ImGui::DragFloat3("Sun Pos", &m_Position.x, 1.0f, -200.0f, 200.0f);

    //    ImGui::Separator();

    //    // ターゲット位置の調整
    //    ImGui::Text("Target");
    //    ImGui::DragFloat3("Look At", &m_TargetPosition.x, 1.0f, -100.0f, 100.0f);

    //    ImGui::Separator();

    //    // ライトの方向（手動調整用）
    //    ImGui::Text("Light Direction (Manual)");
    //    ImGui::DragFloat3("Direction", &m_LightDirection.x, 0.01f, -1.0f, 1.0f);

    //    ImGui::Separator();

    //    // 光の強さ
    //    ImGui::Text("Intensity");
    //    ImGui::SliderFloat("Light", &m_Intensity, 0.0f, 3.0f);
    //    ImGui::SliderFloat("Ambient", &m_AmbientStrength, 0.0f, 1.0f);

    //    ImGui::Separator();

    //    // シャドウマップの範囲
    //    ImGui::Text("Shadow Settings");
    //    ImGui::SliderFloat("Ortho Size", &m_OrthoSize, 50.0f, 300.0f);

    //    ImGui::Separator();

    //    // プリセットボタン
    //    if (ImGui::Button("Morning Sun"))
    //    {
    //        m_Position = Vector3(80.0f, 50.0f, 0.0f);
    //        m_LightDirection = Vector3(1.0f, -0.5f, 0.0f);
    //        m_Intensity = 1.2f;
    //        m_AmbientStrength = 0.4f;
    //    }
    //    ImGui::SameLine();
    //    if (ImGui::Button("Noon Sun"))
    //    {
    //        m_Position = Vector3(0.0f, 100.0f, 0.0f);
    //        m_LightDirection = Vector3(0.0f, -1.0f, 0.0f);
    //        m_Intensity = 1.8f;
    //        m_AmbientStrength = 0.3f;
    //    }
    //    ImGui::SameLine();
    //    if (ImGui::Button("Evening Sun"))
    //    {
    //        m_Position = Vector3(-80.0f, 40.0f, 0.0f);
    //        m_LightDirection = Vector3(-1.0f, -0.4f, 0.0f);
    //        m_Intensity = 1.0f;
    //        m_AmbientStrength = 0.5f;
    //    }

    //    ImGui::End();
    //}

    //// Tキーでウィンドウの表示/非表示
    //if (Input::GetKeyTrigger('T'))
    //{
    //    m_ShowImGui = !m_ShowImGui;
    //}

    // ライト設定を更新
    ApplyLight();
}

void Sun::Draw()
{
    if (!m_ModelRenderer) return;

    // シェーダーをセット
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

    // テクスチャスロットをクリア
    ID3D11ShaderResourceView* nullSRV = nullptr;
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

    // ワールド行列を設定（太陽の位置にモデルを配置）
    XMMATRIX scale = XMMatrixScaling(m_ModelScale, m_ModelScale, m_ModelScale);
    XMMATRIX trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
    Renderer::SetWorldMatrix(scale * trans);

    // マテリアルを明るい黄色に設定
    MATERIAL material;
    material.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.3f, 1.0f);  // 黄色
    material.Ambient = XMFLOAT4(1.0f, 1.0f, 0.3f, 1.0f);
    material.Emission = XMFLOAT4(0.5f, 0.5f, 0.0f, 1.0f); // 発光
    Renderer::SetMaterial(material);

    // モデルの描画
    m_ModelRenderer->Draw();
}

void Sun::ApplyLight()
{
    // ライトの設定を作成
    LIGHT light;
    light.Enable = true;

    // 方向を正規化
    XMVECTOR dirVec = XMVectorSet(m_LightDirection.x, m_LightDirection.y, m_LightDirection.z, 0.0f);
    dirVec = XMVector3Normalize(dirVec);
    XMStoreFloat4(&light.Direction, dirVec);

    // 光の強さを適用
    light.Diffuse = XMFLOAT4(m_Intensity, m_Intensity, m_Intensity, 1.0f);
    light.Ambient = XMFLOAT4(m_AmbientStrength, m_AmbientStrength, m_AmbientStrength, 1.0f);

    // Rendererに設定
    Renderer::SetLight(light);
}

XMMATRIX Sun::GetLightViewMatrix()
{
    // 太陽の位置からターゲットを見るビュー行列
    return XMMatrixLookAtLH(
        XMVectorSet(m_Position.x, m_Position.y, m_Position.z, 1.0f),
        XMVectorSet(m_TargetPosition.x, m_TargetPosition.y, m_TargetPosition.z, 1.0f),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    );
}

XMMATRIX Sun::GetLightProjectionMatrix()
{
    // 正射影行列（シャドウマップ用）
    return XMMatrixOrthographicLH(m_OrthoSize, m_OrthoSize, 1.0f, 200.0f);
}

XMMATRIX Sun::GetLightViewProjectionMatrix()
{
    return XMMatrixMultiply(GetLightViewMatrix(), GetLightProjectionMatrix());
}