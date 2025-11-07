#pragma once
#include "btBulletDynamicsCommon.h"
#include "modelRenderer.h"
#include "ConvexDecomposition.h"

// 三角メッシュ形状を作成（重い！非推奨）
inline btBvhTriangleMeshShape* CreateTriangleMeshShape(MODEL* model)
{
    btTriangleMesh* triMesh = new btTriangleMesh();
    for (unsigned int i = 0; i < model->CollisionIndices.size(); i += 3)
    {
        auto& p0 = model->CollisionVertices[model->CollisionIndices[i + 0]];
        auto& p1 = model->CollisionVertices[model->CollisionIndices[i + 1]];
        auto& p2 = model->CollisionVertices[model->CollisionIndices[i + 2]];
        triMesh->addTriangle(
            btVector3(p0.x, p0.y, p0.z),
            btVector3(p1.x, p1.y, p1.z),
            btVector3(p2.x, p2.y, p2.z)
        );
    }
    return new btBvhTriangleMeshShape(triMesh, true);
}

// ★★★ 推奨：凸分解形状を作成（キャッシュ機能付き） ★★★
inline btCompoundShape* CreateConvexDecompositionShape(
    MODEL* model,
    const std::string& cacheKey,
    unsigned int maxConvexHulls = 16,
    unsigned int resolution = 20000)
{
    return ConvexDecomposition::CreateConvexDecomposition(
        model,
        cacheKey,
        maxConvexHulls,
        resolution,
        0.001
    );
}

// ★★★ 超高速版：ゲーム用（推奨） ★★★
inline btCompoundShape* CreateConvexDecompositionShapeFast(MODEL* model, const std::string& cacheKey)
{
    // 超高速・低精度（1-3秒）
    return ConvexDecomposition::CreateConvexDecomposition(
        model,
        cacheKey,
        8,       // 凸包8個（超軽い）
        10000,   // 超低解像度
        0.01
    );
}

// 簡易版：デフォルトパラメータ（5-10秒）
inline btCompoundShape* CreateConvexDecompositionShapeSimple(MODEL* model, const std::string& cacheKey)
{
    // 高速・低精度
    return ConvexDecomposition::CreateConvexDecomposition(
        model,
        cacheKey,
        16,      // 凸包16個
        20000,   // 低解像度
        0.01
    );
}

// 高精度版：エディタ用（30-60秒）
inline btCompoundShape* CreateConvexDecompositionShapeHighQuality(MODEL* model, const std::string& cacheKey)
{
    // 低速・高精度
    return ConvexDecomposition::CreateConvexDecomposition(
        model,
        cacheKey,
        64,       // 凸包64個
        100000,   // 高解像度
        0.0005
    );
}