// TriangleMeshFragment.cpp
// 3D立体破片の実装

#include "TriangleMeshFragment.h"
#include "manager.h"
#include <vector>

void TriangleMeshFragment::Init()
{
    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");
    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");

    // デフォルトマテリアル設定
    ZeroMemory(&m_Material, sizeof(m_Material));
    m_Material.Diffuse = m_FragmentColor;
    m_Material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_Material.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    m_Material.Emission = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    m_Material.Shininess = 10.0f;
    m_Material.TextureEnable = false;

    SetName("TriangleMeshFragment");//仮で名前をいれる
    SetTag(GameObjectTag::Ground);
}

void TriangleMeshFragment::Start()
{
    if (m_RigidBody) return;

    if (PhysicsManager::GetWorld()) {
        SetupCollisionLayer();

        // 計算されたバウンディングボックスサイズを使用
        CreateBoxCollider(m_ColliderHalfSize, 1.0f);  // 質量1kg

        // 空気抵抗を設定（すぐ止まるように）
        if (m_RigidBody) {
            m_RigidBody->setDamping(0.5f, 0.7f);
        }
    }
}

void TriangleMeshFragment::Uninit()
{
    // 物理オブジェクトの解放
    if (m_RigidBody) {
        PhysicsObject::Uninit();
    }

    // メッシュバッファの解放
    if (m_VertexBuffer) m_VertexBuffer->Release();
    if (m_IndexBuffer) m_IndexBuffer->Release();
    if (m_VertexLayout) m_VertexLayout->Release();
    if (m_VertexShader) m_VertexShader->Release();
    if (m_PixelShader) m_PixelShader->Release();
}

void TriangleMeshFragment::Update()
{
    CheckAndCallStart();

    if (m_Started) {
        m_Timer += 0.016f;  // deltaTime

        // 寿命切れたら削除
        if (m_Timer > m_Lifetime) {
            SetDestroy();
        }
    }
}

void TriangleMeshFragment::Draw()
{
    if (!m_VertexBuffer || !m_IndexBuffer) return;

    // シェーダー設定
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

    // マテリアル設定
    Renderer::SetMaterial(m_Material);

    // テクスチャ設定
    if (m_Texture && m_Material.TextureEnable) {
        Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);
    }
    else {
        // テクスチャなしの場合はnullをセット
        ID3D11ShaderResourceView* nullSRV = nullptr;
        Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);
    }

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

void TriangleMeshFragment::SetTriangleMesh(const VERTEX_3D* vertices, unsigned int vertexCount)
{
    // バウンディングボックスのサイズを計算
    m_ColliderHalfSize = CalculateBoundingBoxHalfSize(vertices, vertexCount);

    // 3D立体化するかどうかで処理を分岐
    if (m_UseExtrusion) {
        CreateExtrudedMesh(vertices, vertexCount);
    }
    else {
        CreateFlatMesh(vertices, vertexCount);
    }
}

void TriangleMeshFragment::CreateMeshBuffers(const VERTEX_3D* vertices, unsigned int vertexCount)
{
    // この関数は使用しない（CreateExtrudedMeshとCreateFlatMeshで直接作成）
}

void TriangleMeshFragment::CreateExtrudedMesh(const VERTEX_3D* vertices, unsigned int vertexCount)
{
    // 三角形のグループを処理
    std::vector<VERTEX_3D> extrudedVertices;
    std::vector<unsigned int> indices;

    // 3頂点ずつ処理（三角形ごと）
    for (unsigned int i = 0; i + 2 < vertexCount; i += 3) {
        const VERTEX_3D& v0 = vertices[i + 0];
        const VERTEX_3D& v1 = vertices[i + 1];
        const VERTEX_3D& v2 = vertices[i + 2];

        // 三角形の法線を計算
        XMVECTOR p0 = XMLoadFloat3(&v0.Position);
        XMVECTOR p1 = XMLoadFloat3(&v1.Position);
        XMVECTOR p2 = XMLoadFloat3(&v2.Position);

        XMVECTOR edge1 = XMVectorSubtract(p1, p0);
        XMVECTOR edge2 = XMVectorSubtract(p2, p0);
        XMVECTOR normal = XMVector3Normalize(XMVector3Cross(edge1, edge2));

        // 押し出しベクトル
        XMVECTOR extrusion = XMVectorScale(normal, -m_ExtrusionDepth);

        unsigned int baseIndex = (unsigned int)extrudedVertices.size();

        // 上面の3頂点（元の三角形）
        VERTEX_3D topVerts[3] = { v0, v1, v2 };
        for (int j = 0; j < 3; j++) {
          
            XMStoreFloat3(&topVerts[j].Normal, normal);
            extrudedVertices.push_back(topVerts[j]);
        }

        // 下面の3頂点（押し出した三角形）
        VERTEX_3D bottomVerts[3];
        for (int j = 0; j < 3; j++) {
            bottomVerts[j] = topVerts[j];
            XMVECTOR pos = XMLoadFloat3(&bottomVerts[j].Position);
            pos = XMVectorAdd(pos, extrusion);
            XMStoreFloat3(&bottomVerts[j].Position, pos);

            // 下面の法線は逆向き
            XMVECTOR invertedNormal = XMVectorNegate(normal);
            XMStoreFloat3(&bottomVerts[j].Normal, invertedNormal);

           
            extrudedVertices.push_back(bottomVerts[j]);
        }

        // 上面の三角形（0, 1, 2）
        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);

        // 下面の三角形（5, 4, 3 - 逆順で法線を逆に）
        indices.push_back(baseIndex + 5);
        indices.push_back(baseIndex + 4);
        indices.push_back(baseIndex + 3);

        // 側面の3つの四角形（各辺に1つ）
        // 側面1: 辺(0-1)
        unsigned int sideBase = baseIndex + 6;

        // 辺0-1の側面
        VERTEX_3D side01[4];
        side01[0] = topVerts[0];
        side01[1] = topVerts[1];
        side01[2] = bottomVerts[1];
        side01[3] = bottomVerts[0];

        // 側面の法線を計算
        XMVECTOR s01_edge1 = XMVectorSubtract(XMLoadFloat3(&side01[1].Position), XMLoadFloat3(&side01[0].Position));
        XMVECTOR s01_edge2 = XMVectorSubtract(XMLoadFloat3(&side01[3].Position), XMLoadFloat3(&side01[0].Position));
        XMVECTOR s01_normal = XMVector3Normalize(XMVector3Cross(s01_edge1, s01_edge2));

        for (int j = 0; j < 4; j++) {
            XMStoreFloat3(&side01[j].Normal, s01_normal);
            extrudedVertices.push_back(side01[j]);
        }

        // 側面1の2つの三角形
        indices.push_back(sideBase + 0);
        indices.push_back(sideBase + 1);
        indices.push_back(sideBase + 2);
        indices.push_back(sideBase + 0);
        indices.push_back(sideBase + 2);
        indices.push_back(sideBase + 3);

        // 辺1-2の側面
        sideBase += 4;
        VERTEX_3D side12[4];
        side12[0] = topVerts[1];
        side12[1] = topVerts[2];
        side12[2] = bottomVerts[2];
        side12[3] = bottomVerts[1];

        XMVECTOR s12_edge1 = XMVectorSubtract(XMLoadFloat3(&side12[1].Position), XMLoadFloat3(&side12[0].Position));
        XMVECTOR s12_edge2 = XMVectorSubtract(XMLoadFloat3(&side12[3].Position), XMLoadFloat3(&side12[0].Position));
        XMVECTOR s12_normal = XMVector3Normalize(XMVector3Cross(s12_edge1, s12_edge2));

        for (int j = 0; j < 4; j++) {
            side12[j].Diffuse = m_FragmentColor;
            XMStoreFloat3(&side12[j].Normal, s12_normal);
            extrudedVertices.push_back(side12[j]);
        }

        indices.push_back(sideBase + 0);
        indices.push_back(sideBase + 1);
        indices.push_back(sideBase + 2);
        indices.push_back(sideBase + 0);
        indices.push_back(sideBase + 2);
        indices.push_back(sideBase + 3);

        // 辺2-0の側面
        sideBase += 4;
        VERTEX_3D side20[4];
        side20[0] = topVerts[2];
        side20[1] = topVerts[0];
        side20[2] = bottomVerts[0];
        side20[3] = bottomVerts[2];

        XMVECTOR s20_edge1 = XMVectorSubtract(XMLoadFloat3(&side20[1].Position), XMLoadFloat3(&side20[0].Position));
        XMVECTOR s20_edge2 = XMVectorSubtract(XMLoadFloat3(&side20[3].Position), XMLoadFloat3(&side20[0].Position));
        XMVECTOR s20_normal = XMVector3Normalize(XMVector3Cross(s20_edge1, s20_edge2));

        for (int j = 0; j < 4; j++) {
            XMStoreFloat3(&side20[j].Normal, s20_normal);
            extrudedVertices.push_back(side20[j]);
        }

        indices.push_back(sideBase + 0);
        indices.push_back(sideBase + 1);
        indices.push_back(sideBase + 2);
        indices.push_back(sideBase + 0);
        indices.push_back(sideBase + 2);
        indices.push_back(sideBase + 3);
    }

    m_VertexCount = (unsigned int)extrudedVertices.size();
    m_IndexCount = (unsigned int)indices.size();

    // 頂点バッファ作成
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(VERTEX_3D) * m_VertexCount;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = extrudedVertices.data();

    Renderer::GetDevice()->CreateBuffer(&vbDesc, &vbData, &m_VertexBuffer);

    // インデックスバッファ作成
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.ByteWidth = sizeof(unsigned int) * m_IndexCount;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices.data();

    Renderer::GetDevice()->CreateBuffer(&ibDesc, &ibData, &m_IndexBuffer);
}

void TriangleMeshFragment::CreateFlatMesh(const VERTEX_3D* vertices, unsigned int vertexCount)
{
    // 従来の平面メッシュ（ペラペラ）
    m_VertexCount = vertexCount;
    m_IndexCount = vertexCount;

    // 色を適用
    std::vector<VERTEX_3D> coloredVertices(vertexCount);
    for (unsigned int i = 0; i < vertexCount; i++) {
        coloredVertices[i] = vertices[i];
    }

    // 頂点バッファ作成
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(VERTEX_3D) * vertexCount;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = coloredVertices.data();

    Renderer::GetDevice()->CreateBuffer(&vbDesc, &vbData, &m_VertexBuffer);

    // インデックスバッファ作成（頂点を順番に使用）
    std::vector<unsigned int> indices(vertexCount);
    for (unsigned int i = 0; i < vertexCount; i++) {
        indices[i] = i;
    }

    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.ByteWidth = sizeof(unsigned int) * vertexCount;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices.data();

    Renderer::GetDevice()->CreateBuffer(&ibDesc, &ibData, &m_IndexBuffer);
}

Vector3 TriangleMeshFragment::CalculateBoundingBoxHalfSize(const VERTEX_3D* vertices, unsigned int vertexCount)
{
    if (vertexCount == 0) return Vector3(0.5f, 0.5f, 0.5f);

    // 最小・最大座標を初期化
    XMFLOAT3 minPos = vertices[0].Position;
    XMFLOAT3 maxPos = vertices[0].Position;

    // 全頂点を走査して最小・最大を求める
    for (unsigned int i = 1; i < vertexCount; i++) {
        const XMFLOAT3& pos = vertices[i].Position;

        minPos.x = fmin(minPos.x, pos.x);
        minPos.y = fmin(minPos.y, pos.y);
        minPos.z = fmin(minPos.z, pos.z);

        maxPos.x = fmax(maxPos.x, pos.x);
        maxPos.y = fmax(maxPos.y, pos.y);
        maxPos.z = fmax(maxPos.z, pos.z);
    }

    // 押し出しを考慮してサイズを拡張
    float extrusionMargin = m_UseExtrusion ? m_ExtrusionDepth : 0.0f;

    // ハーフサイズを計算
    Vector3 halfSize(
        (maxPos.x - minPos.x) * 0.5f + extrusionMargin,
        (maxPos.y - minPos.y) * 0.5f + extrusionMargin,
        (maxPos.z - minPos.z) * 0.5f + extrusionMargin
    );

    // 最小サイズを保証（あまりに小さいと物理演算が不安定になる）
    halfSize.x = fmax(halfSize.x, 0.1f);
    halfSize.y = fmax(halfSize.y, 0.1f);
    halfSize.z = fmax(halfSize.z, 0.1f);

    return halfSize;
}