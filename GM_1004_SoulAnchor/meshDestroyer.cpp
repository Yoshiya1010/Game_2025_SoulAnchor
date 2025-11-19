#include "MeshDestroyer.h"
#include <algorithm>
#include "modelRenderer.h"

void MeshDestroyer::DestroyModel(
    MODEL* model,//読み込んだモデル
    const XMMATRIX& worldMatrix,//モデルからワールド座標にしたもの
    const Vector3& explosionCenter,//爆発が発生する場所　爆心地
    float explosionForce,//破壊された時にもらう飛び散る方向
    Scene* scene//シーンのインスタンス
)
{
    if (!model || !scene) return;

    // ワールド行列から回転成分を抽出


    //モデルの各サブセットを処理　（マテリアルの境界ごとね　色が混ざるからね）
    for (unsigned int subsetIdx = 0; subsetIdx < model->SubsetNum; subsetIdx++) {
        SUBSET& subset = model->SubsetArray[subsetIdx];

        //このサブセットの三角形を処理
        for (unsigned int i = subset.StartIndex; i < subset.StartIndex + subset.IndexNum; i += 3) {
            // 三角形の3頂点を取得
            if (i + 2 >= model->CollisionIndices.size()) break;

            //三角形の頂点のインデックス番号を貰う
            unsigned int idx0 = model->CollisionIndices[i + 0];
            unsigned int idx1 = model->CollisionIndices[i + 1];
            unsigned int idx2 = model->CollisionIndices[i + 2];

            //頂点番号の境界をチェックする（データが壊れてた場合の対策）
            if (idx0 >= model->CollisionVertices.size() ||
                idx1 >= model->CollisionVertices.size() ||
                idx2 >= model->CollisionVertices.size()) {
                continue;
            }

            // 頂点データを作成（法線とテクスチャ座標は簡易的に設定）
            //三角メッシュだから頂点３つでとる
            VERTEX_3D vertices[3];

            // 三角形の法線をローカル空間で計算
            XMVECTOR p0 = XMLoadFloat3(&model->CollisionVertices[idx0]);
            XMVECTOR p1 = XMLoadFloat3(&model->CollisionVertices[idx1]);
            XMVECTOR p2 = XMLoadFloat3(&model->CollisionVertices[idx2]);
            XMVECTOR edge1 = XMVectorSubtract(p1, p0);
            XMVECTOR edge2 = XMVectorSubtract(p2, p0);
            XMVECTOR localNormal = XMVector3Normalize(XMVector3Cross(edge1, edge2));
            XMFLOAT3 normalFloat;
            XMStoreFloat3(&normalFloat, localNormal);

            //頂点0
            vertices[0].Position = model->CollisionVertices[idx0];
            vertices[0].Normal = normalFloat;
            vertices[0].Diffuse = XMFLOAT4(1, 1, 1, 1);
            vertices[0].TexCoord = XMFLOAT2(0, 0);

            //頂点1
            vertices[1].Position = model->CollisionVertices[idx1];
            vertices[1].Normal = normalFloat;
            vertices[1].Diffuse = XMFLOAT4(1, 1, 1, 1);
            vertices[1].TexCoord = XMFLOAT2(1, 0);

            //頂点2
            vertices[2].Position = model->CollisionVertices[idx2];
            vertices[2].Normal = normalFloat;
            vertices[2].Diffuse = XMFLOAT4(1, 1, 1, 1);
            vertices[2].TexCoord = XMFLOAT2(0, 1);

            //ワールド空間での位置を計算　ワールド座標系へ変換　(位置は行列変換、法線は回転のみ）
            VERTEX_3D worldVertices[3];
            for (int v = 0; v < 3; v++) {
                worldVertices[v] = TransformVertex(vertices[v], worldMatrix);
            }

            //三角形の中心をワールド空間で計算
            //破片オブジェクトの原点に使う
            Vector3 triangleCenter = CalculateTriangleCenter(
                worldVertices[0].Position,
                worldVertices[1].Position,
                worldVertices[2].Position
            );

            //頂点を中心基準のローカル座標に変換
            for (int v = 0; v < 3; v++) {
                vertices[v].Position.x = worldVertices[v].Position.x - triangleCenter.x;
                vertices[v].Position.y = worldVertices[v].Position.y - triangleCenter.y;
                vertices[v].Position.z = worldVertices[v].Position.z - triangleCenter.z;

                // 法線はワールド空間のものを使う
                vertices[v].Normal = worldVertices[v].Normal;
            }

            // 破片を生成 ゲームオブジェクト扱いにしてる
            TriangleMeshFragment* fragment = scene->AddGameObject<TriangleMeshFragment>(OBJECT);
            fragment->Init();
            fragment->SetPosition(triangleCenter);
         
            fragment->SetScale(Vector3(1, 1, 1));

            //3D立体破片の設定
            fragment->SetUseExtrusion(true);             // 3D立体化ON
            fragment->SetExtrusionDepth(0.15f);          // 厚さ15で基本的には問題ないけど　ここ動的な方がいいかも　一旦仮にするわ　TODO
        
            //メッシュを三角形（3頂点）としてセット
            fragment->SetTriangleMesh(vertices, 3);
           

            //マテリアル設定
            fragment->SetMaterial(subset.Material.Material);
            if (subset.Material.Texture) {
                fragment->SetTexture(subset.Material.Texture);
            }

            //爆発の力を計算して適用
            Vector3 direction = triangleCenter - explosionCenter;
            float distance = direction.Length();
            if (distance > 0.001f) {
                direction.Normalize();

                // 距離に応じて力を減衰
                float forceMagnitude = explosionForce / (1.0f + distance * 0.1f);
                Vector3 force = direction * forceMagnitude;

                //指向性だけ渡して、ある程度ランダムにする
                force.x += (rand() % 200 - 100) / 100.0f * explosionForce * 0.3f;
                force.y += (rand() % 200 - 100) / 100.0f * explosionForce * 0.3f;
                force.z += (rand() % 200 - 100) / 100.0f * explosionForce * 0.3f;

                //Start後に力を適用するため、少し遅延　Bodyの生成自体は　Startでやってるからね
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

    // ワールド行列からスケール成分を抽出
    XMVECTOR scaleVec, rotQuat, transVec;
    XMMatrixDecompose(&scaleVec, &rotQuat, &transVec, worldMatrix);
    float avgScale = (XMVectorGetX(scaleVec) + XMVectorGetY(scaleVec) + XMVectorGetZ(scaleVec)) / 3.0f;

    // モデルのバウンディングボックスサイズを元に厚さを決定（モデル全体の5%程度）
    float modelSize = 0.0f;
    if (!model->CollisionVertices.empty()) {
        XMFLOAT3 minPos = model->CollisionVertices[0];
        XMFLOAT3 maxPos = model->CollisionVertices[0];
        for (const auto& v : model->CollisionVertices) {
            minPos.x = std::min(minPos.x, v.x); minPos.y = std::min(minPos.y, v.y); minPos.z = std::min(minPos.z, v.z);
            maxPos.x = std:: max(maxPos.x, v.x); maxPos.y = std::max(maxPos.y, v.y); maxPos.z = std::max(maxPos.z, v.z);
        }
        float sizeX = (maxPos.x - minPos.x) * avgScale;
        float sizeY = (maxPos.y - minPos.y) * avgScale;
        float sizeZ = (maxPos.z - minPos.z) * avgScale;
        modelSize = (sizeX + sizeY + sizeZ) / 3.0f;
    }

    // モデルサイズの5-10%を厚さにする（調整可能）
    float dynamicDepth = modelSize * 0.08f;

    // 三角形データを格納する構造体
    struct TriangleData {
        VERTEX_3D worldVertices[3];  // ワールド座標の頂点
        Vector3 center;               // 三角形の中心
        MATERIAL material;
        ID3D11ShaderResourceView* texture;
        bool processed;               // 処理済みフラグ
    };

    // モデルの各サブセットを処理
    for (unsigned int subsetIdx = 0; subsetIdx < model->SubsetNum; subsetIdx++) {
        SUBSET& subset = model->SubsetArray[subsetIdx];

        // このサブセットの全三角形を収集
        std::vector<TriangleData> triangles;

        for (unsigned int i = subset.StartIndex; i < subset.StartIndex + subset.IndexNum; i += 3) {
            if (i + 2 >= model->CollisionIndices.size()) break;

            unsigned int idx0 = model->CollisionIndices[i + 0];
            unsigned int idx1 = model->CollisionIndices[i + 1];
            unsigned int idx2 = model->CollisionIndices[i + 2];

            if (idx0 >= model->CollisionVertices.size() ||
                idx1 >= model->CollisionVertices.size() ||
                idx2 >= model->CollisionVertices.size()) {
                continue;
            }

            // 三角形の法線をローカル空間で計算
            XMVECTOR p0 = XMLoadFloat3(&model->CollisionVertices[idx0]);
            XMVECTOR p1 = XMLoadFloat3(&model->CollisionVertices[idx1]);
            XMVECTOR p2 = XMLoadFloat3(&model->CollisionVertices[idx2]);
            XMVECTOR edge1 = XMVectorSubtract(p1, p0);
            XMVECTOR edge2 = XMVectorSubtract(p2, p0);
            XMVECTOR localNormal = XMVector3Normalize(XMVector3Cross(edge1, edge2));
            XMFLOAT3 normalFloat;
            XMStoreFloat3(&normalFloat, localNormal);

            // 頂点データを作成
            VERTEX_3D vertices[3];
            vertices[0].Position = model->CollisionVertices[idx0];
            vertices[0].Normal = normalFloat;
            vertices[0].Diffuse = XMFLOAT4(1, 1, 1, 1);
            vertices[0].TexCoord = XMFLOAT2(0, 0);

            vertices[1].Position = model->CollisionVertices[idx1];
            vertices[1].Normal = normalFloat;
            vertices[1].Diffuse = XMFLOAT4(1, 1, 1, 1);
            vertices[1].TexCoord = XMFLOAT2(1, 0);

            vertices[2].Position = model->CollisionVertices[idx2];
            vertices[2].Normal = normalFloat;
            vertices[2].Diffuse = XMFLOAT4(1, 1, 1, 1);
            vertices[2].TexCoord = XMFLOAT2(0, 1);

            // ワールド空間での頂点を計算
            TriangleData triData;
            for (int v = 0; v < 3; v++) {
                triData.worldVertices[v] = TransformVertex(vertices[v], worldMatrix);
            }

            // 三角形の中心を計算
            triData.center = CalculateTriangleCenter(
                triData.worldVertices[0].Position,
                triData.worldVertices[1].Position,
                triData.worldVertices[2].Position
            );

            triData.material = subset.Material.Material;
            triData.texture = subset.Material.Texture;
            triData.processed = false;

            triangles.push_back(triData);
        }

        // 距離ベースでグループ化して破片を生成
        while (true) {
            // 未処理の三角形を探す
            int seedIndex = -1;
            for (size_t i = 0; i < triangles.size(); i++) {
                if (!triangles[i].processed) {
                    seedIndex = (int)i;
                    break;
                }
            }

            // 全て処理済みなら終了
            if (seedIndex == -1) break;

            // シード三角形を中心にグループを構築
            std::vector<int> group;
            group.push_back(seedIndex);
            triangles[seedIndex].processed = true;

            Vector3 seedCenter = triangles[seedIndex].center;

            // 最も近い三角形を順次追加（groupSize個まで）
            while (group.size() < (size_t)groupSize) {
                int nearestIndex = -1;
                float nearestDist = FLT_MAX;

                // 未処理の三角形の中から最も近いものを探す
                for (size_t i = 0; i < triangles.size(); i++) {
                    if (triangles[i].processed) continue;

                    // グループの重心との距離を計算
                    Vector3 groupCenter(0, 0, 0);
                    for (int gIdx : group) {
                        groupCenter = groupCenter + triangles[gIdx].center;
                    }
                    groupCenter = groupCenter * (1.0f / group.size());

                    Vector3 diff = triangles[i].center - groupCenter;
                    float dist = diff.Length();

                    if (dist < nearestDist) {
                        nearestDist = dist;
                        nearestIndex = (int)i;
                    }
                }

                // 追加できる三角形がなければ終了
                if (nearestIndex == -1) break;

                group.push_back(nearestIndex);
                triangles[nearestIndex].processed = true;
            }

            // グループから破片を生成
            std::vector<VERTEX_3D> groupVertices;
            Vector3 groupCenter(0, 0, 0);

            // グループの中心を計算
            for (int idx : group) {
                groupCenter = groupCenter + triangles[idx].center;
            }
            groupCenter = groupCenter * (1.0f / group.size());

            // 頂点をローカル座標に変換してまとめる
            for (int idx : group) {
                for (int v = 0; v < 3; v++) {
                    VERTEX_3D localVertex = triangles[idx].worldVertices[v];
                    localVertex.Position.x -= groupCenter.x;
                    localVertex.Position.y -= groupCenter.y;
                    localVertex.Position.z -= groupCenter.z;
                    groupVertices.push_back(localVertex);
                }
            }

            // 破片を生成
            TriangleMeshFragment* fragment = scene->AddGameObject<TriangleMeshFragment>(OBJECT);
            fragment->Init();
            fragment->SetPosition(groupCenter);
            fragment->SetScale(Vector3(1, 1, 1));

            // 3D立体破片の設定
            fragment->SetUseExtrusion(true);
            fragment->SetExtrusionDepth(dynamicDepth);

            fragment->SetTriangleMesh(groupVertices.data(), (unsigned int)groupVertices.size());

            // マテリアル設定（グループの最初の三角形のものを使用）
            fragment->SetMaterial(triangles[group[0]].material);
            if (triangles[group[0]].texture) {
                fragment->SetTexture(triangles[group[0]].texture);
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