#include "BulletDebugDrawer.h"
#include "renderer.h"
#include "input.h"

BulletDebugDrawer::BulletDebugDrawer()
    : m_DebugMode(btIDebugDraw::DBG_DrawWireframe)
{
}

BulletDebugDrawer::~BulletDebugDrawer()
{
    Uninit();
}

void BulletDebugDrawer::Init()
{
    if (m_Initialized) return;

    // 線描画用シェーダー読み込み
    Renderer::CreateLineVertexShader(&m_VertexShader, &m_VertexLayout, "shader/lineVS.cso");
    Renderer::CreatePixelShader(&m_PixelShader, "shader/linePS.cso");

    // 動的頂点バッファ作成
    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(VERTEX_LINE) * 20000; // 線10000本分
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    Renderer::GetDevice()->CreateBuffer(&bufferDesc, nullptr, &m_VertexBuffer);

    m_Initialized = true;
    printf("BulletDebugDrawer initialized\n");
}

void BulletDebugDrawer::Uninit()
{
    if (m_VertexShader)  m_VertexShader->Release();
    if (m_PixelShader)   m_PixelShader->Release();
    if (m_VertexLayout)  m_VertexLayout->Release();
    if (m_VertexBuffer)  m_VertexBuffer->Release();

    m_Initialized = false;
}

void BulletDebugDrawer::Begin()
{
    m_Lines.clear();
}

void BulletDebugDrawer::End()
{
    if (!m_Initialized || m_Lines.empty()) return;

    // 頂点データを作成
    std::vector<VERTEX_LINE> vertices;
    vertices.reserve(m_Lines.size() * 2);

    for (const auto& line : m_Lines)
    {
        VERTEX_LINE v1{};
        v1.Position = line.from;
        v1.Color = line.color;
        v1.Normal = XMFLOAT3(0, 1, 0);   
        v1.TexCoord = XMFLOAT2(0, 0);    

        VERTEX_LINE v2{};
        v2.Position = line.to;
        v2.Color = line.color;
        v2.Normal = XMFLOAT3(0, 1, 0);   
        v2.TexCoord = XMFLOAT2(0, 0);    

        vertices.push_back(v1);
        vertices.push_back(v2);
    }

    // バッファ更新
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Renderer::GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, vertices.data(), sizeof(VERTEX_LINE) * vertices.size());
    Renderer::GetDeviceContext()->Unmap(m_VertexBuffer, 0);

    // 描画設定
    Renderer::SetWorldMatrix(XMMatrixIdentity());
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);
    Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    // 深度無効・カリング無効
    Renderer::SetDepthEnable(false);
    Renderer::SetCullNone(true);

    // 頂点バッファ設定・描画
    UINT stride = sizeof(VERTEX_LINE);
    UINT offset = 0;
    Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
    Renderer::GetDeviceContext()->Draw(vertices.size(), 0);

    // 設定を元に戻す
    Renderer::SetDepthEnable(true);
    Renderer::SetCullNone(false);
}

void BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    DebugLine line;
    line.from = XMFLOAT3(from.x(), from.y(), from.z());
    line.to = XMFLOAT3(to.x(), to.y(), to.z());
    line.color = XMFLOAT4(color.x(), color.y(), color.z(), 1.0f);

    m_Lines.push_back(line);
}

void BulletDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB,
    btScalar distance, int lifeTime, const btVector3& color)
{
    // 衝突点を小さな十字で表示
    btVector3 from = PointOnB;
    btVector3 to = PointOnB + normalOnB * 0.5f;
    drawLine(from, to, color);

    // 小さな十字マーク（法線方向も考慮）
    // XY平面の十字
    btVector3 cross1 = PointOnB + btVector3(0.1f, 0, 0);
    btVector3 cross2 = PointOnB - btVector3(0.1f, 0, 0);
    btVector3 cross3 = PointOnB + btVector3(0, 0.1f, 0);
    btVector3 cross4 = PointOnB - btVector3(0, 0.1f, 0);

    drawLine(cross1, cross2, color);
    drawLine(cross3, cross4, color);

    // XZ平面の十字（オプション：より見やすくするため）
    btVector3 cross5 = PointOnB + btVector3(0.1f, 0, 0);
    btVector3 cross6 = PointOnB - btVector3(0.1f, 0, 0);
    btVector3 cross7 = PointOnB + btVector3(0, 0, 0.1f);
    btVector3 cross8 = PointOnB - btVector3(0, 0, 0.1f);

    drawLine(cross5, cross6, btVector3(color.x() * 0.7f, color.y() * 0.7f, color.z() * 0.7f));
    drawLine(cross7, cross8, btVector3(color.x() * 0.7f, color.y() * 0.7f, color.z() * 0.7f));
}

void BulletDebugDrawer::reportErrorWarning(const char* warningString)
{
    printf("Bullet Warning: %s\n", warningString);
}

void BulletDebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
    // 3Dテキストは実装省略（必要に応じて）
}

void BulletDebugDrawer::setDebugMode(int debugMode)
{
    m_DebugMode = debugMode;
}

int BulletDebugDrawer::getDebugMode() const
{
    return m_DebugMode;
}

void BulletDebugDrawer::SetEnabled(bool enabled)
{
    if (enabled)
    {
        m_DebugMode = btIDebugDraw::DBG_DrawWireframe |
            btIDebugDraw::DBG_DrawContactPoints |
            btIDebugDraw::DBG_DrawConstraints;
    }
    else
    {
        m_DebugMode = btIDebugDraw::DBG_NoDebug;
    }
}
