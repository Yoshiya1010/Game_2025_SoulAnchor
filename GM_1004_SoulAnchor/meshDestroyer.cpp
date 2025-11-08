#include "MeshDestroyer.h"
#include <algorithm>
#include"modelRenderer.h"

void MeshDestroyer::DestroyModel(
    MODEL* model,
    const XMMATRIX& worldMatrix,
    const Vector3& explosionCenter,
    float explosionForce,
    Scene* scene
)
{
    if (!model || !scene) return;

    // モデルの各サブセットを処理
    for (unsigned int subsetIdx = 0; subsetIdx < model->SubsetNum; subsetIdx++) {
        SUBSET& subset = model->SubsetArray[subsetIdx];

        // このサブセットの三角形を処理
        for (unsigned int i = subset.StartIndex; i < subset.StartIndex + subset.IndexNum; i += 3) {
            // 三角形の3頂点を取得
            if (i + 2 >= model->CollisionIndices.size()) break;

            unsigned int idx0 = model->CollisionIndices[i + 0];
            unsigned int idx1 = model->CollisionIndices[i + 1];
            unsigned int idx2 = model->CollisionIndices[i + 2];

            if (idx0 >= model->CollisionVertices.size() ||
                idx1 >= model->CollisionVertices.size() ||
                idx2 >= model->CollisionVertices.size()) {
                continue;
            }

            // 頂点データを作成（法線とテクスチャ座標は簡易的に設定）
            VERTEX_3D vertices[3];

            // 頂点0
            vertices[0].Position = model->CollisionVertices[idx0];
            vertices[0].Normal = XMFLOAT3(0, 1, 0);  // 上向き法線（仮）
            vertices[0].Diffuse = XMFLOAT4(1, 1, 1, 1);
            vertices[0].TexCoord = XMFLOAT2(0, 0);

            // 頂点1
            vertices[1].Position = model->CollisionVertices[idx1];
            vertices[1].Normal = XMFLOAT3(0, 1, 0);
            vertices[1].Diffuse = XMFLOAT4(1, 1, 1, 1);
            vertices[1].TexCoord = XMFLOAT2(1, 0);

            // 頂点2
            vertices[2].Position = model->CollisionVertices[idx2];
            vertices[2].Normal = XMFLOAT3(0, 1, 0);
            vertices[2].Diffuse = XMFLOAT4(1, 1, 1, 1);
            vertices[2].TexCoord = XMFLOAT2(0, 1);

            // まずワールド空間での位置を計算
            VERTEX_3D worldVertices[3];
            for (int v = 0; v < 3; v++) {
                worldVertices[v] = TransformVertex(vertices[v], worldMatrix);
            }

            // 三角形の中心をワールド空間で計算
            Vector3 triangleCenter = CalculateTriangleCenter(
                worldVertices[0].Position,
                worldVertices[1].Position,
                worldVertices[2].Position
            );

            // 頂点を中心基準のローカル座標に変換
            for (int v = 0; v < 3; v++) {
                vertices[v].Position.x = worldVertices[v].Position.x - triangleCenter.x;
                vertices[v].Position.y = worldVertices[v].Position.y - triangleCenter.y;
                vertices[v].Position.z = worldVertices[v].Position.z - triangleCenter.z;

                // 法線はワールド空間のものを使用
                vertices[v].Normal = worldVertices[v].Normal;
            }

            // 破片を生成
            TriangleMeshFragment* fragment = scene->AddGameObject<TriangleMeshFragment>(OBJECT);
            fragment->Init();
            fragment->SetPosition(triangleCenter);
            fragment->SetScale(Vector3(1, 1, 1));
            fragment->SetTriangleMesh(vertices, 3);

            // マテリアル設定
            fragment->SetMaterial(subset.Material.Material);
            if (subset.Material.Texture) {
                fragment->SetTexture(subset.Material.Texture);
            }

            // 爆発の力を計算して適用
            Vector3 direction = triangleCenter - explosionCenter;
            float distance = direction.Length();
            if (distance > 0.001f) {
               direction.Normalize();

                // 距離に応じて力を減衰
                float forceMagnitude = explosionForce / (1.0f + distance * 0.1f);
                Vector3 force = direction * forceMagnitude;

                // ランダム性を追加
                force.x += (rand() % 200 - 100) / 100.0f * explosionForce * 0.3f;
                force.y += (rand() % 200 - 100) / 100.0f * explosionForce * 0.3f;
                force.z += (rand() % 200 - 100) / 100.0f * explosionForce * 0.3f;

                // Start後に力を適用するため、少し遅延
                fragment->Start();
                if (fragment->GetRigidBody()) {
                    fragment->SetVelocity(force);

                    // 回転も追加
                    btVector3 angularVel(
                        (rand() % 200 - 100) / 50.0f,
                        (rand() % 200 - 100) / 50.0f,
                        (rand() % 200 - 100) / 50.0f
                    );
                    fragment->GetRigidBody()->setAngularVelocity(angularVel);
                }
            }
        }
    }
}

void MeshDestroyer::DestroyModelGrouped(
    MODEL* model,
    const XMMATRIX& worldMatrix,
    const Vector3& explosionCenter,
    float explosionForce,
    Scene* scene,
    int groupSize
)
{
    if (!model || !scene || groupSize < 1) return;

    // モデルの各サブセットを処理
    for (unsigned int subsetIdx = 0; subsetIdx < model->SubsetNum; subsetIdx++) {
        SUBSET& subset = model->SubsetArray[subsetIdx];

        std::vector<VERTEX_3D> groupVertices;
        Vector3 groupCenter(0, 0, 0);
        int triangleCount = 0;

        // このサブセットの三角形を処理
        for (unsigned int i = subset.StartIndex; i < subset.StartIndex + subset.IndexNum; i += 3) {
            // 三角形の3頂点を取得
            if (i + 2 >= model->CollisionIndices.size()) break;

            unsigned int idx0 = model->CollisionIndices[i + 0];
            unsigned int idx1 = model->CollisionIndices[i + 1];
            unsigned int idx2 = model->CollisionIndices[i + 2];

            if (idx0 >= model->CollisionVertices.size() ||
                idx1 >= model->CollisionVertices.size() ||
                idx2 >= model->CollisionVertices.size()) {
                continue;
            }

            // 頂点データを作成
            VERTEX_3D vertices[3];
            vertices[0].Position = model->CollisionVertices[idx0];
            vertices[0].Normal = XMFLOAT3(0, 1, 0);
            vertices[0].Diffuse = XMFLOAT4(1, 1, 1, 1);
            vertices[0].TexCoord = XMFLOAT2(0, 0);

            vertices[1].Position = model->CollisionVertices[idx1];
            vertices[1].Normal = XMFLOAT3(0, 1, 0);
            vertices[1].Diffuse = XMFLOAT4(1, 1, 1, 1);
            vertices[1].TexCoord = XMFLOAT2(1, 0);

            vertices[2].Position = model->CollisionVertices[idx2];
            vertices[2].Normal = XMFLOAT3(0, 1, 0);
            vertices[2].Diffuse = XMFLOAT4(1, 1, 1, 1);
            vertices[2].TexCoord = XMFLOAT2(0, 1);

            // まずワールド空間での頂点を計算
            VERTEX_3D worldVertices[3];
            for (int v = 0; v < 3; v++) {
                worldVertices[v] = TransformVertex(vertices[v], worldMatrix);
            }

            // グループの中心を累積（ワールド空間）
            Vector3 triCenter = CalculateTriangleCenter(
                worldVertices[0].Position,
                worldVertices[1].Position,
                worldVertices[2].Position
            );
            groupCenter = groupCenter + triCenter;
            triangleCount++;

            // 頂点は後でローカル座標に変換するため、一旦ワールド座標で保存
            for (int v = 0; v < 3; v++) {
                groupVertices.push_back(worldVertices[v]);
            }

            // グループサイズに達したら破片を生成
            if (triangleCount >= groupSize) {
                // グループの中心を平均化
                groupCenter = groupCenter * (1.0f / triangleCount);

                // 頂点を中心基準のローカル座標に変換
                for (auto& vertex : groupVertices) {
                    vertex.Position.x -= groupCenter.x;
                    vertex.Position.y -= groupCenter.y;
                    vertex.Position.z -= groupCenter.z;
                }

                // 破片を生成
                TriangleMeshFragment* fragment = scene->AddGameObject<TriangleMeshFragment>(OBJECT);
                fragment->Init();
                fragment->SetPosition(groupCenter);
                fragment->SetScale(Vector3(1, 1, 1));
                fragment->SetTriangleMesh(groupVertices.data(), groupVertices.size());

                // マテリアル設定
                fragment->SetMaterial(subset.Material.Material);
                if (subset.Material.Texture) {
                    fragment->SetTexture(subset.Material.Texture);
                }

                // 爆発の力を適用
                Vector3 direction = groupCenter - explosionCenter;
                float distance = direction.Length();
                if (distance > 0.001f) {
                    direction.Normalize();
                    float forceMagnitude = explosionForce / (1.0f + distance * 0.1f);
                    Vector3 force = direction * forceMagnitude;

                    // ランダム性を追加
                    force.x += (rand() % 200 - 100) / 100.0f * explosionForce * 0.3f;
                    force.y += (rand() % 200 - 100) / 100.0f * explosionForce * 0.3f;
                    force.z += (rand() % 200 - 100) / 100.0f * explosionForce * 0.3f;

                    fragment->Start();
                    if (fragment->GetRigidBody()) {
                        fragment->SetVelocity(force);

                        // 回転も追加
                        btVector3 angularVel(
                            (rand() % 200 - 100) / 50.0f,
                            (rand() % 200 - 100) / 50.0f,
                            (rand() % 200 - 100) / 50.0f
                        );
                        fragment->GetRigidBody()->setAngularVelocity(angularVel);
                    }
                }

                // 次のグループのためにリセット
                groupVertices.clear();
                groupCenter = Vector3(0, 0, 0);
                triangleCount = 0;
            }
        }

        // 残りの三角形があれば破片として生成
        if (triangleCount > 0) {
            groupCenter = groupCenter * (1.0f / triangleCount);

            // 頂点を中心基準のローカル座標に変換
            for (auto& vertex : groupVertices) {
                vertex.Position.x -= groupCenter.x;
                vertex.Position.y -= groupCenter.y;
                vertex.Position.z -= groupCenter.z;
            }

            TriangleMeshFragment* fragment = scene->AddGameObject<TriangleMeshFragment>(OBJECT);
            fragment->Init();
            fragment->SetPosition(groupCenter);
            fragment->SetScale(Vector3(1, 1, 1));
            fragment->SetTriangleMesh(groupVertices.data(), groupVertices.size());

            fragment->SetMaterial(subset.Material.Material);
            if (subset.Material.Texture) {
                fragment->SetTexture(subset.Material.Texture);
            }

            Vector3 direction = groupCenter - explosionCenter;
            float distance = direction.Length();
            if (distance > 0.001f) {
                direction.Normalize();
                float forceMagnitude = explosionForce / (1.0f + distance * 0.1f);
                Vector3 force = direction * forceMagnitude;

                force.x += (rand() % 200 - 100) / 100.0f * explosionForce * 0.3f;
                force.y += (rand() % 200 - 100) / 100.0f * explosionForce * 0.3f;
                force.z += (rand() % 200 - 100) / 100.0f * explosionForce * 0.3f;

                fragment->Start();
                if (fragment->GetRigidBody()) {
                    fragment->SetVelocity(force);

                    btVector3 angularVel(
                        (rand() % 200 - 100) / 50.0f,
                        (rand() % 200 - 100) / 50.0f,
                        (rand() % 200 - 100) / 50.0f
                    );
                    fragment->GetRigidBody()->setAngularVelocity(angularVel);
                }
            }
        }
    }
}

Vector3 MeshDestroyer::CalculateTriangleCenter(const XMFLOAT3& v0, const XMFLOAT3& v1, const XMFLOAT3& v2)
{
    // 三角形の重心を計算
    return Vector3(
        (v0.x + v1.x + v2.x) / 3.0f,
        (v0.y + v1.y + v2.y) / 3.0f,
        (v0.z + v1.z + v2.z) / 3.0f
    );
}

VERTEX_3D MeshDestroyer::TransformVertex(const VERTEX_3D& vertex, const XMMATRIX& worldMatrix)
{
    VERTEX_3D result = vertex;

    // 位置を変換
    XMVECTOR pos = XMLoadFloat3(&vertex.Position);
    pos = XMVector3Transform(pos, worldMatrix);
    XMStoreFloat3(&result.Position, pos);

    // 法線を変換（回転のみ）
    XMVECTOR normal = XMLoadFloat3(&vertex.Normal);
    XMMATRIX rotationMatrix = worldMatrix;
    rotationMatrix.r[3] = XMVectorSet(0, 0, 0, 1);  // 平行移動成分を除去
    normal = XMVector3TransformNormal(normal, rotationMatrix);
    normal = XMVector3Normalize(normal);
    XMStoreFloat3(&result.Normal, normal);

    return result;
}