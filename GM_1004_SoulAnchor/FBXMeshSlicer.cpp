#include "FBXMeshSlicer.h"
#include <map>
#include <algorithm>

// FBXを空間分割してメッシュの配列を返す
std::vector<SlicedMesh> FBXMeshSlicer::SliceFBX(
    const aiScene* scene,
    Vector3 objectScale,
    int gridX, int gridY, int gridZ)
{
    std::vector<SlicedMesh> slicedMeshes;

    if (!scene || scene->mNumMeshes == 0) {
        printf("[FBXMeshSlicer] No meshes in scene!\n");
        return slicedMeshes;
    }

    // FBXの全メッシュを結合
    std::vector<VERTEX_3D> allVertices;
    std::vector<unsigned int> allIndices;

    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* mesh = scene->mMeshes[m];
        unsigned int baseVertex = allVertices.size();

        // 頂点を追加
        for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
            VERTEX_3D vertex;
            vertex.Position = {
                mesh->mVertices[v].x * objectScale.x,
                mesh->mVertices[v].y * objectScale.y,
                mesh->mVertices[v].z * objectScale.z
            };
            vertex.Normal = {
                mesh->mNormals[v].x,
                mesh->mNormals[v].y,
                mesh->mNormals[v].z
            };
            if (mesh->mTextureCoords[0]) {
                vertex.TexCoord = {
                    mesh->mTextureCoords[0][v].x,
                    mesh->mTextureCoords[0][v].y
                };
            }
            else {
                vertex.TexCoord = { 0, 0 };
            }
            vertex.Diffuse = { 1, 1, 1, 1 };
            allVertices.push_back(vertex);
        }

        // インデックスを追加
        for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
            const aiFace& face = mesh->mFaces[f];
            for (unsigned int i = 0; i < face.mNumIndices; i++) {
                allIndices.push_back(baseVertex + face.mIndices[i]);
            }
        }
    }

    printf("[FBXMeshSlicer] Total vertices: %zu, indices: %zu\n",
        allVertices.size(), allIndices.size());

    // バウンディングボックスを計算
    Vector3 bbMin(FLT_MAX, FLT_MAX, FLT_MAX);
    Vector3 bbMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (const auto& vertex : allVertices) {
        bbMin.x = std::min(bbMin.x, vertex.Position.x);
        bbMin.y = std::min(bbMin.y, vertex.Position.y);
        bbMin.z = std::min(bbMin.z, vertex.Position.z);
        bbMax.x = std::max(bbMax.x, vertex.Position.x);
        bbMax.y = std::max(bbMax.y, vertex.Position.y);
        bbMax.z = std::max(bbMax.z, vertex.Position.z);
    }

    printf("[FBXMeshSlicer] BBox: (%.2f,%.2f,%.2f) to (%.2f,%.2f,%.2f)\n",
        bbMin.x, bbMin.y, bbMin.z, bbMax.x, bbMax.y, bbMax.z);

    // グリッドセルのサイズ
    Vector3 cellSize = {
        (bbMax.x - bbMin.x) / gridX,
        (bbMax.y - bbMin.y) / gridY,
        (bbMax.z - bbMin.z) / gridZ
    };

    printf("[FBXMeshSlicer] Cell size: (%.2f,%.2f,%.2f)\n",
        cellSize.x, cellSize.y, cellSize.z);

    // 各グリッドセルについて処理
    for (int x = 0; x < gridX; x++) {
        for (int y = 0; y < gridY; y++) {
            for (int z = 0; z < gridZ; z++) {
                // セルの範囲を計算
                Vector3 cellMin = {
                    bbMin.x + x * cellSize.x,
                    bbMin.y + y * cellSize.y,
                    bbMin.z + z * cellSize.z
                };
                Vector3 cellMax = {
                    cellMin.x + cellSize.x,
                    cellMin.y + cellSize.y,
                    cellMin.z + cellSize.z
                };

                SlicedMesh slicedMesh;
                std::map<unsigned int, unsigned int> vertexMap;  // 元のindex → 新しいindex

                // 三角形を処理
                for (size_t i = 0; i < allIndices.size(); i += 3) {
                    unsigned int idx0 = allIndices[i];
                    unsigned int idx1 = allIndices[i + 1];
                    unsigned int idx2 = allIndices[i + 2];

                    Vector3 v0(allVertices[idx0].Position.x,
                        allVertices[idx0].Position.y,
                        allVertices[idx0].Position.z);
                    Vector3 v1(allVertices[idx1].Position.x,
                        allVertices[idx1].Position.y,
                        allVertices[idx1].Position.z);
                    Vector3 v2(allVertices[idx2].Position.x,
                        allVertices[idx2].Position.y,
                        allVertices[idx2].Position.z);

                    // 三角形がセルと交差しているかチェック
                    if (IsTriangleIntersectingCell(v0, v1, v2, cellMin, cellMax)) {
                        // 頂点を追加（重複チェック）
                        for (unsigned int idx : {idx0, idx1, idx2}) {
                            if (vertexMap.find(idx) == vertexMap.end()) {
                                vertexMap[idx] = slicedMesh.vertices.size();
                                slicedMesh.vertices.push_back(allVertices[idx]);
                            }
                        }

                        // インデックスを追加
                        slicedMesh.indices.push_back(vertexMap[idx0]);
                        slicedMesh.indices.push_back(vertexMap[idx1]);
                        slicedMesh.indices.push_back(vertexMap[idx2]);
                    }
                }

                // 頂点があるセルのみ追加
                if (!slicedMesh.vertices.empty()) {
                    CalculateCenterAndVolume(slicedMesh);
                    slicedMeshes.push_back(slicedMesh);

                    printf("[FBXMeshSlicer] Cell[%d,%d,%d]: %zu vertices, %zu indices\n",
                        x, y, z, slicedMesh.vertices.size(), slicedMesh.indices.size());
                }
            }
        }
    }

    printf("[FBXMeshSlicer] Created %zu mesh fragments\n", slicedMeshes.size());
    return slicedMeshes;
}

// 頂点がセル内にあるかチェック
bool FBXMeshSlicer::IsVertexInCell(
    const Vector3& vertex,
    const Vector3& cellMin,
    const Vector3& cellMax)
{
    return vertex.x >= cellMin.x && vertex.x <= cellMax.x &&
        vertex.y >= cellMin.y && vertex.y <= cellMax.y &&
        vertex.z >= cellMin.z && vertex.z <= cellMax.z;
}

// 三角形がセルと交差しているかチェック（簡易版：頂点が1つでも含まれるか、三角形の中心が含まれる）
bool FBXMeshSlicer::IsTriangleIntersectingCell(
    const Vector3& v0,
    const Vector3& v1,
    const Vector3& v2,
    const Vector3& cellMin,
    const Vector3& cellMax)
{
    // 頂点のいずれかがセル内にあるか
    if (IsVertexInCell(v0, cellMin, cellMax) ||
        IsVertexInCell(v1, cellMin, cellMax) ||
        IsVertexInCell(v2, cellMin, cellMax)) {
        return true;
    }

    // 三角形の中心がセル内にあるか
    Vector3 center = {
        (v0.x + v1.x + v2.x) / 3.0f,
        (v0.y + v1.y + v2.y) / 3.0f,
        (v0.z + v1.z + v2.z) / 3.0f
    };

    return IsVertexInCell(center, cellMin, cellMax);
}

// メッシュの中心と体積を計算
void FBXMeshSlicer::CalculateCenterAndVolume(SlicedMesh& mesh)
{
    if (mesh.vertices.empty()) {
        mesh.center = Vector3(0, 0, 0);
        mesh.volume = 0.0f;
        return;
    }

    // 中心を計算（頂点の平均）
    Vector3 sum(0, 0, 0);
    for (const auto& vertex : mesh.vertices) {
        sum.x += vertex.Position.x;
        sum.y += vertex.Position.y;
        sum.z += vertex.Position.z;
    }

    mesh.center = sum / (float)mesh.vertices.size();

    // 体積を概算（バウンディングボックスの体積）
    Vector3 bMin(FLT_MAX, FLT_MAX, FLT_MAX);
    Vector3 bMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (const auto& vertex : mesh.vertices) {
        bMin.x = std::min(bMin.x, vertex.Position.x);
        bMin.y = std::min(bMin.y, vertex.Position.y);
        bMin.z = std::min(bMin.z, vertex.Position.z);
        bMax.x = std::max(bMax.x, vertex.Position.x);
        bMax.y = std::max(bMax.y, vertex.Position.y);
        bMax.z = std::max(bMax.z, vertex.Position.z);
    }

    mesh.volume = (bMax.x - bMin.x) * (bMax.y - bMin.y) * (bMax.z - bMin.z);
}