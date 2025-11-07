#pragma once

#include <vector>
#include "main.h"
#include "renderer.h"

// 分割されたメッシュデータ
struct SlicedMesh {
    std::vector<VERTEX_3D> vertices;
    std::vector<unsigned int> indices;
    Vector3 center;  // メッシュの中心
    float volume;    // 体積（質量計算用）
};

// FBXメッシュスライサー
class FBXMeshSlicer {
public:
    // FBXを空間分割してメッシュの配列を返す
    static std::vector<SlicedMesh> SliceFBX(
        const aiScene* scene,
        Vector3 objectScale,
        int gridX, int gridY, int gridZ
    );

private:
    // 頂点がグリッドセル内にあるかチェック
    static bool IsVertexInCell(
        const Vector3& vertex,
        const Vector3& cellMin,
        const Vector3& cellMax
    );

    // 三角形がグリッドセルと交差しているかチェック
    static bool IsTriangleIntersectingCell(
        const Vector3& v0,
        const Vector3& v1,
        const Vector3& v2,
        const Vector3& cellMin,
        const Vector3& cellMax
    );

    // メッシュの中心と体積を計算
    static void CalculateCenterAndVolume(SlicedMesh& mesh);
};