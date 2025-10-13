#include "score.h"
#include "manager.h"
#include "camera.h"
#include "textureManager.h"
#include "scene.h"

void Score::Init()
{
    VERTEX_3D vertex[4];

    vertex[0].Position = XMFLOAT3(-1.0f, 1.0f, 0.0f);
    vertex[0].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

    vertex[1].Position = XMFLOAT3(1.0f, 1.0f, 0.0f);
    vertex[1].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

    vertex[2].Position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
    vertex[2].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

    vertex[3].Position = XMFLOAT3(1.0f,-1.0f, 0.0f);
    vertex[3].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

    // 頂点バッファ作成
    D3D11_BUFFER_DESC bd{};
    bd.Usage = D3D11_USAGE_DYNAMIC; // DEFAULTだとmap,Unmapが使えない（基本的にアニメーションやるならDYNAMIC）
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = vertex;

    Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);

    // テクスチャ読み込み
    m_Texture = TextureManager::Load("asset\\texture\\number.png");

    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");

    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");

    m_Value = 0;
}

void Score::Uninit()
{
    if (m_VertexBuffer)     m_VertexBuffer->Release();
    if (m_VertexLayout)     m_VertexLayout->Release();
    if (m_VertexShader)     m_VertexShader->Release();
    if (m_PixelShader)      m_PixelShader->Release();
}

void Score::Update()
{

}

void Score::Draw()
{
    // 入力レイアウト設定
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

    // シェーダー設定
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

    // マトリクス設定
    Renderer::SetWorldViewProjection2D();

    // マトリクス設定
    XMMATRIX world, scale, rot, trans;
    scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
    rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
    trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
    world = scale * rot * trans;

    Renderer::SetWorldMatrix(world);

    // マテリアル設定
    MATERIAL material{};
    material.Diffuse = { 1.0f,1.0f,1.0f,1.0f };
    material.TextureEnable = true;
    Renderer::SetMaterial(material);

    // 頂点バッファ設定
    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);

    // テクスチャ設定
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);

    // プリミティブタイプの設定
    Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);



    int value = m_Value;

    for (int i = 0; i < 4; i++) {
        // 頂点データ書き換え
        D3D11_MAPPED_SUBRESOURCE msr;
        Renderer::GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

        VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

        int num = value % 10;
        value /= 10;

        float x = 50.0f * (4 - 1) - 50.0f * i;
        float y = 0.0f;
        float w = 50.0f;
        float h = 50.0f;

        float tw = 1.0f / 5;
        float th = 1.0f / 5;
        float tx = (num % 5) * tw;
        float ty = (num / 5) * th;

        vertex[0].Position = XMFLOAT3(x, y, 0.0f);
        vertex[0].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
        vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        vertex[0].TexCoord = XMFLOAT2(tx, ty);

        vertex[1].Position = XMFLOAT3(x + w, y, 0.0f);
        vertex[1].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
        vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        vertex[1].TexCoord = XMFLOAT2(tx + tw, ty);

        vertex[2].Position = XMFLOAT3(x, y + h, 0.0f);
        vertex[2].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
        vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        vertex[2].TexCoord = XMFLOAT2(tx, ty + th);

        vertex[3].Position = XMFLOAT3(x + w, y + h, 0.0f);
        vertex[3].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
        vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        vertex[3].TexCoord = XMFLOAT2(tx + tw, ty + th);

        Renderer::GetDeviceContext()->Unmap(m_VertexBuffer, 0);

        // ポリゴン描画
        Renderer::GetDeviceContext()->Draw(4, 0);
    }
}