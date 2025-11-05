#include "VoxelFragment.h"
#include "manager.h"

void VoxelFragment::Init()
{
    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");
    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");

    // 立方体メッシュ生成
    CreateCubeMesh();

    SetName("VoxelFragment");
    SetTag(GameObjectTag::Ground);  
}

void VoxelFragment::Start()
{
    if (m_RigidBody) return;

    if (PhysicsManager::GetWorld()) {
        SetupCollisionLayer();
        CreateBoxCollider(m_Scale, 1.0f);  // 質量1kg

        // 空気抵抗を追加（すぐ止まるように）
        m_RigidBody->setDamping(0.5f, 0.7f);
    }
}

void VoxelFragment::Uninit()
{
    if (m_RigidBody) {
        PhysicsObject::Uninit();
    }

    if (m_VertexBuffer) m_VertexBuffer->Release();
    if (m_IndexBuffer) m_IndexBuffer->Release();
    if (m_VertexLayout) m_VertexLayout->Release();
    if (m_VertexShader) m_VertexShader->Release();
    if (m_PixelShader) m_PixelShader->Release();
}

void VoxelFragment::Update()
{
    CheckAndCallStart();

    if (m_Started) {
        m_Timer += 0.016f;  // deltaTime

        // 寿命切れまたは静止したら削除
        if (m_Timer > m_Lifetime) {
            SetDestroy();
        }

        //// ほぼ静止したら削除
        //if (m_RigidBody && GetVelocity().Length() < 0.1f && m_Timer > 1.0f) {
        //    SetDestroy();
        //}
    }
}

void VoxelFragment::Draw()
{
    if (!m_VertexBuffer || !m_IndexBuffer) return;

    // シェーダー設定
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

    // マテリアル設定（単色）
    MATERIAL material;
    ZeroMemory(&material, sizeof(material));
    material.Diffuse = m_Color;
    material.Ambient = m_Color;
    material.TextureEnable = false;
    Renderer::SetMaterial(material);

    // ワールド行列設定
    Renderer::SetWorldMatrix(UpdatePhysicsWithModel(1.0f));

    // 描画
    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
    Renderer::GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Renderer::GetDeviceContext()->DrawIndexed(36, 0, 0);  // 立方体 = 12三角形 = 36頂点
}

void VoxelFragment::CreateCubeMesh()
{
    // 立方体の頂点データ（-0.5 ~ 0.5 の範囲）
    VERTEX_3D vertices[24] = {
        // Front face (Z+)
        { XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0, 0, 1), XMFLOAT4(1,1,1,1), XMFLOAT2(0,1) },
        { XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(0, 0, 1), XMFLOAT4(1,1,1,1), XMFLOAT2(1,1) },
        { XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(0, 0, 1), XMFLOAT4(1,1,1,1), XMFLOAT2(1,0) },
        { XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0, 0, 1), XMFLOAT4(1,1,1,1), XMFLOAT2(0,0) },

        // Back face (Z-)
        { XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(0, 0, -1), XMFLOAT4(1,1,1,1), XMFLOAT2(0,1) },
        { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0, 0, -1), XMFLOAT4(1,1,1,1), XMFLOAT2(1,1) },
        { XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0, 0, -1), XMFLOAT4(1,1,1,1), XMFLOAT2(1,0) },
        { XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(0, 0, -1), XMFLOAT4(1,1,1,1), XMFLOAT2(0,0) },

        // Top face (Y+)
        { XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0, 1, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(0,1) },
        { XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(0, 1, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(1,1) },
        { XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(0, 1, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(1,0) },
        { XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0, 1, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(0,0) },

        // Bottom face (Y-)
        { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0, -1, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(0,1) },
        { XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(0, -1, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(1,1) },
        { XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(0, -1, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(1,0) },
        { XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0, -1, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(0,0) },

        // Right face (X+)
        { XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1, 0, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(0,1) },
        { XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1, 0, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(1,1) },
        { XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1, 0, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(1,0) },
        { XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1, 0, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(0,0) },

        // Left face (X-)
        { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(-1, 0, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(0,1) },
        { XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(-1, 0, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(1,1) },
        { XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(-1, 0, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(1,0) },
        { XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(-1, 0, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(0,0) },
    };

    // インデックスデータ（各面2三角形 = 6面 x 6頂点 = 36インデックス）
    unsigned int indices[36] = {
        0,  1,  2,   0,  2,  3,   // Front
        4,  5,  6,   4,  6,  7,   // Back
        8,  9,  10,  8,  10, 11,  // Top
        12, 13, 14,  12, 14, 15,  // Bottom
        16, 17, 18,  16, 18, 19,  // Right
        20, 21, 22,  20, 22, 23   // Left
    };

    // 頂点バッファ作成
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices;

    Renderer::GetDevice()->CreateBuffer(&vbDesc, &vbData, &m_VertexBuffer);

    // インデックスバッファ作成
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.ByteWidth = sizeof(indices);
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices;

    Renderer::GetDevice()->CreateBuffer(&ibDesc, &ibData, &m_IndexBuffer);
}
