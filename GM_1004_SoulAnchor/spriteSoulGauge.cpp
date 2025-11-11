#include"spriteSoulGauge.h"
#include"scene.h"

void SpriteSoulGauge::Init(float x, float y, float width, float height, const char* fileName1, const char* fileName2, const char* fileName3)
{
    m_Position = { x, y, 0.0f };
    m_Scale = { 1.0f, 1.0f, 1.0f };
    m_Rotation = { 0.0f, 0.0f, 0.0f };

    m_Width = width;
    m_Height = height;

    // 動的VB（毎回Mapで書き換える）
    D3D11_BUFFER_DESC bd{};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    // 初期は0%で作る
    VERTEX_3D v[4]{};
    {
        const float ratio = 0.0f;                // 0%
        const float halfH = m_Height * 0.5f;
        const float visH = m_Height * ratio;

        // 下端固定：bottomY は +halfH、topY は bottom - visH
        const float bottomY = +halfH;
        const float topY = bottomY - visH;

        const float leftX = -m_Width * 0.5f;
        const float rightX = +m_Width * 0.5f;

        // UV: v=1 が下、v=0 が上。可視上端は (1 - ratio)
        const float uvTop = 1.0f - ratio;
        const float uvBottom = 1.0f;

        v[0].Position = { leftX,  topY,    0 }; v[0].TexCoord = { 0, uvTop }; v[0].Diffuse = { 1,1,1,1 };
        v[1].Position = { rightX, topY,    0 }; v[1].TexCoord = { 1, uvTop }; v[1].Diffuse = { 1,1,1,1 };
        v[2].Position = { leftX,  bottomY, 0 }; v[2].TexCoord = { 0, uvBottom }; v[2].Diffuse = { 1,1,1,1 };
        v[3].Position = { rightX, bottomY, 0 }; v[3].TexCoord = { 1, uvBottom }; v[3].Diffuse = { 1,1,1,1 };
    }

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = v;

    Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);

    // テクスチャとシェーダ
    m_Texture1 = TextureManager::Load(fileName1);
    m_Texture2 = TextureManager::Load(fileName2);
    m_Texture3 = TextureManager::Load(fileName3);
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
    Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");

    // デフォルト値（例：0%）
    m_Value = 0.0f;
}

void SpriteSoulGauge::Uninit()
{
    if (m_VertexBuffer) { m_VertexBuffer->Release(); m_VertexBuffer = nullptr; }
    if (m_VertexLayout) { m_VertexLayout->Release(); m_VertexLayout = nullptr; }
    if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
    if (m_PixelShader) { m_PixelShader->Release();  m_PixelShader = nullptr; }

}

void SpriteSoulGauge::SetValue(float percent)
{
    // 0～100 にクランプ
    if (percent < 0.0f)   percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    m_Value = percent;

    UpdateGeometry();
}

void SpriteSoulGauge::UpdateGeometry()
{
    // 現在値 → 表示高さ（下端固定）
    const float ratio = m_Value / 100.0f; // 0.0～1.0
    const float halfH = m_Height * 0.5f;
    const float visH = m_Height * ratio;

    const float bottomY = +halfH;
    const float topY = bottomY - visH;

    const float leftX = -m_Width * 0.5f;
    const float rightX = +m_Width * 0.5f;

    // UV：下端=1.0、可視上端=1.0 - ratio
    const float uvTop = 1.0f - ratio;
    const float uvBottom = 1.0f;

    VERTEX_3D v[4]{};
    v[0].Position = { leftX,  topY,    0 }; v[0].TexCoord = { 0, uvTop }; v[0].Diffuse = { 1,1,1,1 };
    v[1].Position = { rightX, topY,    0 }; v[1].TexCoord = { 1, uvTop }; v[1].Diffuse = { 1,1,1,1 };
    v[2].Position = { leftX,  bottomY, 0 }; v[2].TexCoord = { 0, uvBottom }; v[2].Diffuse = { 1,1,1,1 };
    v[3].Position = { rightX, bottomY, 0 }; v[3].TexCoord = { 1, uvBottom }; v[3].Diffuse = { 1,1,1,1 };

    D3D11_MAPPED_SUBRESOURCE mp{};
    Renderer::GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mp);
    memcpy(mp.pData, v, sizeof(v));
    Renderer::GetDeviceContext()->Unmap(m_VertexBuffer, 0);
}

void SpriteSoulGauge::Draw()
{
    if (!m_DrawFlag) return;

    // パイプライン
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

    Renderer::SetWorldViewProjection2D();

    // 行列（中心原点）
    XMMATRIX world = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z)
        * XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.z, m_Rotation.y)
        * XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
    Renderer::SetWorldMatrix(world);

    MATERIAL mat{};
    mat.Diffuse = { 1,1,1,1 };
    mat.TextureEnable = true;
    Renderer::SetMaterial(mat);

    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);

    //テクスチャ選択
    ID3D11ShaderResourceView* tex = nullptr;
    if (m_Value <= 30.0f)        tex = m_Texture1;
    else if (m_Value <= 70.0f)   tex = m_Texture2;
    else                         tex = m_Texture3;

    // テクスチャ設定
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &tex);

    Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    Renderer::GetDeviceContext()->Draw(4, 0);
}

static inline float Clamp01hundred(float v) {
    if (v < 0.f) v = 0.f; else if (v > 100.f) v = 100.f;
    return v;
}

void SpriteSoulGauge::SetTargetValue(float percent)
{
    m_TargetValue = Clamp01hundred(percent);
    // ここでは UpdateGeometry() は呼ばない（Updateで滑らかに動かす）
}

void SpriteSoulGauge::SetValueImmediate(float percent)
{
    m_Value = Clamp01hundred(percent);
    m_TargetValue = m_Value;
    UpdateGeometry(); // 即座に見た目も更新
}

void SpriteSoulGauge::Update()
{
    if (!m_Smooth) return;

    const float dt = Scene::GetDeltaTime();       // 秒
    float diff = m_TargetValue - m_Value;     // 目標との差
    const float eps = 0.01f;                       // 収束しきい値（%）
    if (fabsf(diff) <= eps) return;

    const float step = m_Speed * dt;                // このフレームで動かす最大量（%）
    if (fabsf(diff) <= step) {
        m_Value = m_TargetValue;
    }
    else {
        m_Value += (diff > 0.f ? step : -step);
    }
    UpdateGeometry();
}