#include "SlicedMeshFragment.h"
#include "manager.h"

void SlicedMeshFragment::Init()
{
    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");
    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");

    SetName("SlicedMeshFragment");
    SetTag(GameObjectTag::Ground);
}

void SlicedMeshFragment::Start()
{
    // RigidBody生成
    if (PhysicsManager::GetWorld() && !m_RigidBody) {
        SetupCollisionLayer();

        // メッシュのサイズに基づいてコライダーを作成
        CreateBoxCollider(m_Scale, 1.0f);  // 質量1kg

        if (m_RigidBody) {
            m_RigidBody->setDamping(0.6f, 0.8f);
        }
    }
}

void SlicedMeshFragment::Uninit()
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

void SlicedMeshFragment::Update()
{
    CheckAndCallStart();

    if (m_Started) {
        m_Timer += 0.016f;

    /*    if (m_Timer > m_Lifetime) {
            SetDestroy();
            return;
        }

        if (m_RigidBody) {
            Vector3 vel = GetVelocity();
            if (vel.Length() < 0.1f && m_Timer > 1.0f) {
                SetDestroy();
            }
        }*/
    }
}

void SlicedMeshFragment::Draw()
{
    if (!m_VertexBuffer || !m_IndexBuffer || m_IndexCount == 0) return;

    // シェーダー設定
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

    // マテリアル設定
    MATERIAL material;
    ZeroMemory(&material, sizeof(material));
    material.Diffuse = { 0.8f, 0.6f, 0.4f, 1.0f };
    material.Ambient = { 0.8f, 0.6f, 0.4f, 1.0f };
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
    Renderer::GetDeviceContext()->DrawIndexed(m_IndexCount, 0, 0);
}

void SlicedMeshFragment::SetMeshData(const SlicedMesh& meshData)
{
    if (meshData.vertices.empty() || meshData.indices.empty()) {
        printf("[SlicedMeshFragment] Empty mesh data!\n");
        return;
    }

    // 頂点バッファ作成
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(VERTEX_3D) * meshData.vertices.size();
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = meshData.vertices.data();

    HRESULT hr = Renderer::GetDevice()->CreateBuffer(&vbDesc, &vbData, &m_VertexBuffer);
    if (FAILED(hr)) {
        printf("[SlicedMeshFragment] Failed to create vertex buffer!\n");
        return;
    }

    // インデックスバッファ作成
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.ByteWidth = sizeof(unsigned int) * meshData.indices.size();
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = meshData.indices.data();

    hr = Renderer::GetDevice()->CreateBuffer(&ibDesc, &ibData, &m_IndexBuffer);
    if (FAILED(hr)) {
        printf("[SlicedMeshFragment] Failed to create index buffer!\n");
        return;
    }

    m_IndexCount = meshData.indices.size();

    // メッシュの中心を位置として設定
    m_Position = meshData.center;

    // スケールを設定（バウンディングボックスのサイズ）
    // これはコライダーのサイズになる
    Vector3 bMin(FLT_MAX, FLT_MAX, FLT_MAX);
    Vector3 bMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (const auto& vertex : meshData.vertices) {
        bMin.x = std::min(bMin.x, vertex.Position.x);
        bMin.y = std::min(bMin.y, vertex.Position.y);
        bMin.z = std::min(bMin.z, vertex.Position.z);
        bMax.x = std::max(bMax.x, vertex.Position.x);
        bMax.y = std::max(bMax.y, vertex.Position.y);
        bMax.z = std::max(bMax.z, vertex.Position.z);
    }

    m_Scale = {
        (bMax.x - bMin.x) * 0.5f,
        (bMax.y - bMin.y) * 0.5f,
        (bMax.z - bMin.z) * 0.5f
    };

    printf("[SlicedMeshFragment] Mesh set: %zu vertices, %zu indices, center=(%.2f,%.2f,%.2f)\n",
        meshData.vertices.size(), meshData.indices.size(),
        m_Position.x, m_Position.y, m_Position.z);
}