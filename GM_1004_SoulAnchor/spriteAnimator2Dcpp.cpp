#include"spriteAnimator2D.h"

void SpriteAnimator2D::Init(float x, float y, float w, float h, const char* FileName, int cols, int rows)
{
    m_Position.x = x; m_Position.y = y; m_Scale.x = w; m_Scale.y = h;
    m_Cols = cols; m_Rows = rows;

    VERTEX_3D v[4] =
    {
        {{x,     y,     0}, {}, {1,1,1,1}, {0,0}},
        {{x + w,   y,     0}, {}, {1,1,1,1}, {1,0}},
        {{x,     y + h,   0}, {}, {1,1,1,1}, {0,1}},
        {{x + w,   y + h,   0}, {}, {1,1,1,1}, {1,1}},
    };

    D3D11_BUFFER_DESC bd{};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = v;

    Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);

    m_Texture = TextureManager::Load(FileName);

    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
    Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
}

void SpriteAnimator2D::Update()
{

    if (!m_Playing || m_FrameSpeed == 0.0f) return;

    m_Frame += m_FrameSpeed; // 1フレームあたり増加・減少

    int maxFrame = m_Cols * m_Rows;

    if (m_Loop)
    {
        // ループする場合
        if (m_Frame >= maxFrame) m_Frame -= maxFrame;
        if (m_Frame < 0)        m_Frame += maxFrame;
    }
    else
    {
        // ループしない場合（端で停止）
        if (m_Frame >= maxFrame)
        {
            m_Frame = maxFrame - 1;
            m_Playing = false;
        }
        if (m_Frame < 0)
        {
            m_Frame = 0;
            m_Playing = false;
        }
    }

}

void SpriteAnimator2D::Draw()
{

    if (!m_DrawFlag)return;
    Renderer::SetWorldViewProjection2D();
    Renderer::SetDepthEnable(false);

    //頂点を求める
    float cx = m_Position.x;
    float cy = m_Position.y;
    float w = m_Scale.x;
    float h = m_Scale.y;

    float x0 = cx - w / 2.0f;
    float y0 = cy - h / 2.0f;

    VERTEX_3D v[4] =
    {
        {{x0,     y0,     0}, {}, {1,1,1,1}, {}},
        {{x0 + w, y0,     0}, {}, {1,1,1,1}, {}},
        {{x0,     y0 + h, 0}, {}, {1,1,1,1}, {}},
        {{x0 + w, y0 + h, 0}, {}, {1,1,1,1}, {}},
    };

    //UVを求める
    int col = (int)m_Frame % m_Cols;
    int row = m_Frame / m_Cols;
    float u0 = (float)col / m_Cols;
    float v0 = (float)row / m_Rows;
    float u1 = (float)(col + 1) / m_Cols;
    float v1 = (float)(row + 1) / m_Rows;

    v[0].TexCoord = { u0, v0 };
    v[1].TexCoord = { u1, v0 };
    v[2].TexCoord = { u0, v1 };
    v[3].TexCoord = { u1, v1 };

    //頂点更新
    D3D11_MAPPED_SUBRESOURCE mp;
    Renderer::GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mp);
    memcpy(mp.pData, v, sizeof(v));
    Renderer::GetDeviceContext()->Unmap(m_VertexBuffer, 0);


    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

    MATERIAL mat{};
    mat.Diffuse = { 1,1,1,1 };
    mat.TextureEnable = true;
    Renderer::SetMaterial(mat);

    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);
    Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    Renderer::GetDeviceContext()->Draw(4, 0);

    Renderer::SetDepthEnable(true);
}


void SpriteAnimator2D::Uninit()
{
    if (m_VertexBuffer) m_VertexBuffer->Release();
    if (m_VertexLayout) m_VertexLayout->Release();
    if (m_VertexShader) m_VertexShader->Release();
    if (m_PixelShader)  m_PixelShader->Release();
}
