#include "FracturedMesh.h"
#include "manager.h"
#include "MeshSimplifier.h"
#include <random>
#include"scene.h"

// ========== FracturedMeshFragment実装 ==========

void FracturedMeshFragment::Init()
{
    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");
    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");

    SetName("FracturedMeshFragment");
    SetTag(GameObjectTag::Ground);
}

void FracturedMeshFragment::Start()
{
    // RigidBody生成（すでにSetMeshDataで頂点データがある）
    if (PhysicsManager::GetWorld() && !m_RigidBody && m_IndexCount > 0) {
        SetupCollisionLayer();

        // 簡略化された三角メッシュを作成
        // TODO: ここで頂点データから簡略化
        CreateBoxCollider(m_Scale, 1.0f);  // とりあえずBox

        if (m_RigidBody) {
            m_RigidBody->setDamping(0.6f, 0.8f);
        }
    }
}

void FracturedMeshFragment::Uninit()
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

void FracturedMeshFragment::Update()
{
    CheckAndCallStart();

    if (m_Started) {
        m_Timer += 0.016f;

        if (m_Timer > m_Lifetime) {
            SetDestroy();
            return;
        }

        if (m_RigidBody) {
            Vector3 vel = GetVelocity();
            if (vel.Length() < 0.1f && m_Timer > 1.0f) {
                SetDestroy();
            }
        }
    }
}

void FracturedMeshFragment::Draw()
{
    if (!m_VertexBuffer || !m_IndexBuffer || m_IndexCount == 0) return;

    // シェーダー設定
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

    // マテリアル設定
    Renderer::SetMaterial(m_Material);

    // テクスチャ設定
    if (m_Texture) {
        Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);
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

void FracturedMeshFragment::SetMeshData(
    const std::vector<VERTEX_3D>& vertices,
    const std::vector<unsigned int>& indices,
    const MATERIAL& material,
    ID3D11ShaderResourceView* texture)
{
    if (vertices.empty() || indices.empty()) return;

    // 頂点バッファ作成
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(VERTEX_3D) * vertices.size();
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices.data();

    Renderer::GetDevice()->CreateBuffer(&vbDesc, &vbData, &m_VertexBuffer);

    // インデックスバッファ作成
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.ByteWidth = sizeof(unsigned int) * indices.size();
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices.data();

    Renderer::GetDevice()->CreateBuffer(&ibDesc, &ibData, &m_IndexBuffer);

    m_IndexCount = indices.size();
    m_Material = material;
    m_Texture = texture;
}

// ========== MeshFracture実装 ==========

std::vector<MeshFracture::MeshPart> MeshFracture::SplitMesh(
    MODEL* model,
    const Vector3& objectScale,
    int gridX, int gridY, int gridZ)
{
    std::vector<MeshPart> parts;

    if (!model || model->CollisionVertices.empty() || model->CollisionIndices.empty()) {
        return parts;
    }

    // AABBを計算
    XMFLOAT3 bbMin = model->CollisionVertices[0];
    XMFLOAT3 bbMax = model->CollisionVertices[0];

    for (const auto& v : model->CollisionVertices) {
        bbMin.x = std::min(bbMin.x, v.x);
        bbMin.y = std::min(bbMin.y, v.y);
        bbMin.z = std::min(bbMin.z, v.z);
        bbMax.x = std::max(bbMax.x, v.x);
        bbMax.y = std::max(bbMax.y, v.y);
        bbMax.z = std::max(bbMax.z, v.z);
    }

    // グリッドセルのサイズ
    float cellSizeX = (bbMax.x - bbMin.x) / gridX;
    float cellSizeY = (bbMax.y - bbMin.y) / gridY;
    float cellSizeZ = (bbMax.z - bbMin.z) / gridZ;

    printf("[MeshFracture] Splitting mesh into %dx%dx%d = %d parts\n",
        gridX, gridY, gridZ, gridX * gridY * gridZ);

    // 各グリッドセルについて処理
    for (int x = 0; x < gridX; x++) {
        for (int y = 0; y < gridY; y++) {
            for (int z = 0; z < gridZ; z++) {
                // セルの範囲
                float cellMinX = bbMin.x + x * cellSizeX;
                float cellMinY = bbMin.y + y * cellSizeY;
                float cellMinZ = bbMin.z + z * cellSizeZ;
                float cellMaxX = cellMinX + cellSizeX;
                float cellMaxY = cellMinY + cellSizeY;
                float cellMaxZ = cellMinZ + cellSizeZ;

                MeshPart part;
                std::map<unsigned int, unsigned int> vertexMap;

                // TODO: 三角形を処理してセル内のものだけ追加
                // 簡略版：全頂点を使用（あとで改善）

                // とりあえず、セルの中心位置を設定
                part.center = Vector3(
                    (cellMinX + cellMaxX) * 0.5f * objectScale.x,
                    (cellMinY + cellMaxY) * 0.5f * objectScale.y,
                    (cellMinZ + cellMaxZ) * 0.5f * objectScale.z
                );

                // ダミーの頂点データ（Box）を追加
                // TODO: 実際のメッシュを分割

                parts.push_back(part);
            }
        }
    }

    return parts;
}

void MeshFracture::FractureAndCreateFragments(
    MODEL* model,
    const Vector3& objectPosition,
    const Vector3& objectScale,
    const Vector3& impactPoint,
    const Vector3& impactForce,
    int gridX, int gridY, int gridZ)
{
    printf("[MeshFracture] Fracturing mesh...\n");

    // メッシュを分割
    auto parts = SplitMesh(model, objectScale, gridX, gridY, gridZ);

    printf("[MeshFracture] Generated %zu parts\n", parts.size());

    // ランダム生成器
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> randomRotation(-15.0f, 15.0f);

    // 衝撃方向を正規化
    Vector3 impactDir = impactForce;
    if (impactDir.Length() > 0.001f) {
        impactDir.Normalize();
    }
    else {
        impactDir = Vector3(0, 1, 0);
    }

    // 各破片を生成
    for (size_t i = 0; i < parts.size(); i++) {
        auto& part = parts[i];

        if (part.vertices.empty()) continue;  // 頂点がない場合はスキップ

        // 破片を生成
        auto* fragment = Manager::GetScene()->AddGameObject<FracturedMeshFragment>(OBJECT);

        // メッシュデータを設定
        MATERIAL mat;
        mat.Diffuse = XMFLOAT4(0.7f, 0.6f, 0.5f, 1.0f);
        mat.Ambient = mat.Diffuse;
        mat.TextureEnable = false;

        fragment->SetMeshData(part.vertices, part.indices, mat, nullptr);

        // 位置とスケールを設定
        Vector3 worldPos = objectPosition + part.center;
        fragment->SetPosition(worldPos);
        fragment->SetScale(Vector3(objectScale.x/(float)gridX, objectScale.y / (float)gridY, objectScale.z / (float)gridZ));

        // 力の計算
        Vector3 toFragment = worldPos - impactPoint;
        float distance = toFragment.Length();

        if (distance > 0.01f) {
            toFragment.Normalize();
        }
        else {
            toFragment = Vector3(0, 1, 0);
        }

        Vector3 forceDir = impactDir * 0.4f + toFragment * 0.6f;
        forceDir.y += 0.4f;
        forceDir.Normalize();

        float forceMagnitude = 600.0f / (distance + 1.0f);
        forceMagnitude = std::max(forceMagnitude, 150.0f);
        forceMagnitude = std::min(forceMagnitude, 800.0f);

        Vector3 force = forceDir * forceMagnitude;

        // RigidBodyに力を加える
        if (fragment->GetRigidBody()) {
            fragment->GetRigidBody()->applyCentralImpulse(
                btVector3(force.x, force.y, force.z)
            );

            fragment->GetRigidBody()->applyTorqueImpulse(
                btVector3(
                    randomRotation(gen),
                    randomRotation(gen),
                    randomRotation(gen)
                )
            );

            fragment->GetRigidBody()->activate(true);
        }
    }

    printf("[MeshFracture] Fracture complete!\n");
}