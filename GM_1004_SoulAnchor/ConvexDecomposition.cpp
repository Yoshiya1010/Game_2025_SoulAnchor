// ★★★ このファイルの先頭に必ず書く ★★★
#define ENABLE_VHACD_IMPLEMENTATION 1

#include "ConvexDecomposition.h"
#include "modelRenderer.h"
#include <iostream>

// キャッシュの静的変数
std::unordered_map<std::string, btCompoundShape*> ConvexDecomposition::s_Cache;

btCompoundShape* ConvexDecomposition::CreateConvexDecomposition(
    MODEL* model,
    const std::string& cacheKey,
    unsigned int maxConvexHulls,
    unsigned int resolution,
    double concavity,
    double alpha,
    double beta)
{
    // ★★★ キャッシュチェック ★★★
    auto it = s_Cache.find(cacheKey);
    if (it != s_Cache.end()) {
        printf("[ConvexDecomposition] Using cached result for '%s'\n", cacheKey.c_str());
        return it->second;
    }

    printf("[ConvexDecomposition] Computing convex decomposition for '%s'...\n", cacheKey.c_str());
    printf("  This may take a while...\n");

    // 凸分解を計算
    btCompoundShape* shape = ComputeConvexDecomposition(model, maxConvexHulls, resolution);

    if (shape) {
        // キャッシュに保存
        s_Cache[cacheKey] = shape;
        printf("[ConvexDecomposition] Cached result for '%s'\n", cacheKey.c_str());
    }

    return shape;
}

btCompoundShape* ConvexDecomposition::ComputeConvexDecomposition(
    MODEL* model,
    unsigned int maxConvexHulls,
    unsigned int resolution)
{
    if (!model || model->CollisionVertices.empty() || model->CollisionIndices.empty()) {
        printf("[ConvexDecomposition] Invalid model data!\n");
        return nullptr;
    }

    printf("[ConvexDecomposition] Starting decomposition...\n");
    printf("  Vertices: %zu, Indices: %zu\n",
        model->CollisionVertices.size(),
        model->CollisionIndices.size());
    printf("  Max hulls: %u, Resolution: %u\n", maxConvexHulls, resolution);

    // 頂点データをdouble配列に変換
    std::vector<double> vertices;
    vertices.reserve(model->CollisionVertices.size() * 3);

    size_t vertexCount = model->CollisionVertices.size();
    for (size_t i = 0; i < vertexCount; i++) {
        const XMFLOAT3& v = model->CollisionVertices[i];
        vertices.push_back(v.x);
        vertices.push_back(v.y);
        vertices.push_back(v.z);
    }

    // インデックスをuint32_t配列に変換
    std::vector<uint32_t> indices;
    indices.reserve(model->CollisionIndices.size());

    size_t indexCount = model->CollisionIndices.size();
    for (size_t i = 0; i < indexCount; i++) {
        indices.push_back(static_cast<uint32_t>(model->CollisionIndices[i]));
    }

    // ★★★ V-HACD v4のパラメータ設定 ★★★
    VHACD::IVHACD::Parameters params;

    params.m_maxConvexHulls = maxConvexHulls;
    params.m_resolution = resolution;
    params.m_maxNumVerticesPerCH = 64;
    params.m_maxRecursionDepth = 10;
    params.m_minEdgeLength = 2;
    params.m_fillMode = VHACD::FillMode::FLOOD_FILL;
    params.m_shrinkWrap = true;
    params.m_asyncACD = true;
    params.m_findBestPlane = false;
    params.m_minimumVolumePercentErrorAllowed = 1.0;

    // プログレスコールバック
    ProgressCallback callback;
    params.m_callback = &callback;
    params.m_logger = nullptr;
    params.m_taskRunner = nullptr;

    // V-HACDインスタンス作成
    VHACD::IVHACD* vhacd = VHACD::CreateVHACD();

    // 凸分解を実行
    printf("[ConvexDecomposition] Computing... (this may take 5-30 seconds)\n");
    bool success = vhacd->Compute(
        vertices.data(),
        vertices.size() / 3,
        indices.data(),
        indices.size() / 3,
        params
    );

    if (!success) {
        printf("[ConvexDecomposition] Failed to compute decomposition!\n");
        vhacd->Release();
        return nullptr;
    }

    // 結果を取得
    unsigned int numHulls = vhacd->GetNConvexHulls();
    printf("[ConvexDecomposition] Generated %u convex hulls\n", numHulls);

    if (numHulls == 0) {
        printf("[ConvexDecomposition] Error: No convex hulls generated!\n");
        vhacd->Release();
        return nullptr;
    }

    // Bulletの複合形状を作成
    btCompoundShape* compound = new btCompoundShape();

    // 各凸包をBulletの凸包形状に変換
    for (unsigned int i = 0; i < numHulls; i++) {
        VHACD::IVHACD::ConvexHull hull;
        vhacd->GetConvexHull(i, hull);

        if (hull.m_points.empty()) continue;

        btConvexHullShape* convexShape = new btConvexHullShape();

        for (const auto& point : hull.m_points) {
            btVector3 btPoint(point.mX, point.mY, point.mZ);
            convexShape->addPoint(btPoint, false);
        }

        convexShape->recalcLocalAabb();
        convexShape->optimizeConvexHull();
        convexShape->setMargin(0.01f);

        btTransform localTransform;
        localTransform.setIdentity();
        compound->addChildShape(localTransform, convexShape);

        printf("[ConvexDecomposition] Hull %u: %zu vertices\n", i, hull.m_points.size());
    }

    vhacd->Release();

    printf("[ConvexDecomposition] Decomposition complete!\n");
    return compound;
}

void ConvexDecomposition::ClearCache()
{
    printf("[ConvexDecomposition] Clearing cache (%zu entries)...\n", s_Cache.size());

    for (auto& pair : s_Cache) {
        delete pair.second;
    }

    s_Cache.clear();
    printf("[ConvexDecomposition] Cache cleared\n");
}