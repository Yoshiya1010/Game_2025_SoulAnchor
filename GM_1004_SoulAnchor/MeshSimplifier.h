#pragma once
#include <vector>
#include <DirectXMath.h>
#include "btBulletDynamicsCommon.h"

using namespace DirectX;

// メッシュ簡略化クラス
class MeshSimplifier {
public:
    // 頂点データを間引いて簡略化
    static std::vector<XMFLOAT3> SimplifyVertices(
        const std::vector<XMFLOAT3>& vertices,
        const std::vector<unsigned int>& indices,
        int targetVertexCount = 10
    );

    // バウンディングボックスの8頂点を取得（最も簡単）
    static std::vector<XMFLOAT3> GetBoundingBoxVertices(
        const std::vector<XMFLOAT3>& vertices
    );

    // 凸包の頂点を取得（適度な精度）
    static std::vector<XMFLOAT3> GetConvexHullVertices(
        const std::vector<XMFLOAT3>& vertices,
        int maxVertices = 20
    );

    // 簡略化された三角メッシュを作成
    static btBvhTriangleMeshShape* CreateSimplifiedTriangleMesh(
        const std::vector<XMFLOAT3>& vertices,
        int targetVertexCount = 10
    );

private:
    // 距離に基づいて頂点を間引く
    static std::vector<XMFLOAT3> DecimateVertices(
        const std::vector<XMFLOAT3>& vertices,
        int targetCount
    );
};