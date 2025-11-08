#include "MeshSimplifier.h"
#include <algorithm>
#include <cmath>

// バウンディングボックスの8頂点を取得（最も簡単で軽い）
std::vector<XMFLOAT3> MeshSimplifier::GetBoundingBoxVertices(
    const std::vector<XMFLOAT3>& vertices)
{
    if (vertices.empty()) return {};

    // AABB（軸並行バウンディングボックス）を計算
    XMFLOAT3 min = vertices[0];
    XMFLOAT3 max = vertices[0];

    for (const auto& v : vertices) {
        min.x = std::min(min.x, v.x);
        min.y = std::min(min.y, v.y);
        min.z = std::min(min.z, v.z);
        max.x = std::max(max.x, v.x);
        max.y = std::max(max.y, v.y);
        max.z = std::max(max.z, v.z);
    }

    // 8頂点を生成
    std::vector<XMFLOAT3> boxVertices = {
        {min.x, min.y, min.z},
        {max.x, min.y, min.z},
        {min.x, max.y, min.z},
        {max.x, max.y, min.z},
        {min.x, min.y, max.z},
        {max.x, min.y, max.z},
        {min.x, max.y, max.z},
        {max.x, max.y, max.z}
    };

    return boxVertices;
}

// 頂点を間引いて簡略化
std::vector<XMFLOAT3> MeshSimplifier::SimplifyVertices(
    const std::vector<XMFLOAT3>& vertices,
    const std::vector<unsigned int>& indices,
    int targetVertexCount)
{
    if (vertices.size() <= targetVertexCount) {
        return vertices;  // すでに少ない
    }

    // 簡易版：等間隔でサンプリング
    std::vector<XMFLOAT3> simplified;
    simplified.reserve(targetVertexCount);

    int step = vertices.size() / targetVertexCount;
    if (step < 1) step = 1;

    for (size_t i = 0; i < vertices.size(); i += step) {
        simplified.push_back(vertices[i]);
        if (simplified.size() >= targetVertexCount) break;
    }

    return simplified;
}

// 凸包の頂点を取得（Bulletを使用）
std::vector<XMFLOAT3> MeshSimplifier::GetConvexHullVertices(
    const std::vector<XMFLOAT3>& vertices,
    int maxVertices)
{
    if (vertices.empty()) return {};

    // Bulletの凸包計算を使用
    btConvexHullShape* tempHull = new btConvexHullShape();

    for (const auto& v : vertices) {
        tempHull->addPoint(btVector3(v.x, v.y, v.z), false);
    }

    tempHull->recalcLocalAabb();
    tempHull->optimizeConvexHull();

    // 凸包の頂点を取得
    std::vector<XMFLOAT3> hullVertices;

    // Bulletの内部頂点にアクセス
    const btVector3* points = tempHull->getUnscaledPoints();
    int numPoints = tempHull->getNumPoints();

    // 最大頂点数まで取得
    int count = std::min(numPoints, maxVertices);
    for (int i = 0; i < count; i++) {
        hullVertices.push_back(XMFLOAT3(
            points[i].x(),
            points[i].y(),
            points[i].z()
        ));
    }

    delete tempHull;
    return hullVertices;
}

// 簡略化された三角メッシュを作成
btBvhTriangleMeshShape* MeshSimplifier::CreateSimplifiedTriangleMesh(
    const std::vector<XMFLOAT3>& vertices,
    int targetVertexCount)
{
    if (vertices.empty()) return nullptr;

    // バウンディングボックス方式（8頂点、超軽量）
    std::vector<XMFLOAT3> simplified = GetBoundingBoxVertices(vertices);

    // Bulletの三角メッシュを作成
    btTriangleMesh* triMesh = new btTriangleMesh();

    // ボックスの三角形を追加（12個の三角形 = 6面 × 2）
    // 前面
    triMesh->addTriangle(
        btVector3(simplified[0].x, simplified[0].y, simplified[0].z),
        btVector3(simplified[1].x, simplified[1].y, simplified[1].z),
        btVector3(simplified[2].x, simplified[2].y, simplified[2].z)
    );
    triMesh->addTriangle(
        btVector3(simplified[1].x, simplified[1].y, simplified[1].z),
        btVector3(simplified[3].x, simplified[3].y, simplified[3].z),
        btVector3(simplified[2].x, simplified[2].y, simplified[2].z)
    );

    // 背面
    triMesh->addTriangle(
        btVector3(simplified[4].x, simplified[4].y, simplified[4].z),
        btVector3(simplified[6].x, simplified[6].y, simplified[6].z),
        btVector3(simplified[5].x, simplified[5].y, simplified[5].z)
    );
    triMesh->addTriangle(
        btVector3(simplified[5].x, simplified[5].y, simplified[5].z),
        btVector3(simplified[6].x, simplified[6].y, simplified[6].z),
        btVector3(simplified[7].x, simplified[7].y, simplified[7].z)
    );

    // 左面
    triMesh->addTriangle(
        btVector3(simplified[0].x, simplified[0].y, simplified[0].z),
        btVector3(simplified[2].x, simplified[2].y, simplified[2].z),
        btVector3(simplified[4].x, simplified[4].y, simplified[4].z)
    );
    triMesh->addTriangle(
        btVector3(simplified[2].x, simplified[2].y, simplified[2].z),
        btVector3(simplified[6].x, simplified[6].y, simplified[6].z),
        btVector3(simplified[4].x, simplified[4].y, simplified[4].z)
    );

    // 右面
    triMesh->addTriangle(
        btVector3(simplified[1].x, simplified[1].y, simplified[1].z),
        btVector3(simplified[5].x, simplified[5].y, simplified[5].z),
        btVector3(simplified[3].x, simplified[3].y, simplified[3].z)
    );
    triMesh->addTriangle(
        btVector3(simplified[3].x, simplified[3].y, simplified[3].z),
        btVector3(simplified[5].x, simplified[5].y, simplified[5].z),
        btVector3(simplified[7].x, simplified[7].y, simplified[7].z)
    );

    // 上面
    triMesh->addTriangle(
        btVector3(simplified[2].x, simplified[2].y, simplified[2].z),
        btVector3(simplified[3].x, simplified[3].y, simplified[3].z),
        btVector3(simplified[6].x, simplified[6].y, simplified[6].z)
    );
    triMesh->addTriangle(
        btVector3(simplified[3].x, simplified[3].y, simplified[3].z),
        btVector3(simplified[7].x, simplified[7].y, simplified[7].z),
        btVector3(simplified[6].x, simplified[6].y, simplified[6].z)
    );

    // 下面
    triMesh->addTriangle(
        btVector3(simplified[0].x, simplified[0].y, simplified[0].z),
        btVector3(simplified[4].x, simplified[4].y, simplified[4].z),
        btVector3(simplified[1].x, simplified[1].y, simplified[1].z)
    );
    triMesh->addTriangle(
        btVector3(simplified[1].x, simplified[1].y, simplified[1].z),
        btVector3(simplified[4].x, simplified[4].y, simplified[4].z),
        btVector3(simplified[5].x, simplified[5].y, simplified[5].z)
    );

    return new btBvhTriangleMeshShape(triMesh, true);
}