#pragma once
#include "main.h"
#include "modelRenderer.h"
#include "TriangleMeshFragment.h"
#include "scene.h"
#include <vector>
#include"modelRenderer.h"

// メッシュを破壊して破片を生成するヘルパークラス
class MeshDestroyer {
public:
    // モデル全体を三角形破片に分解
    // model: 破壊するモデル
    // worldMatrix: モデルのワールド変換行列
    // explosionCenter: 爆発の中心座標
    // explosionForce: 爆発の力の大きさ
    // scene: 破片を追加するシーン
    static void DestroyModel(
        MODEL* model,
        const XMMATRIX& worldMatrix,
        const Vector3& explosionCenter,
        float explosionForce,
        Scene* scene
    );

    // モデルを複数のグループに分けて破壊（パフォーマンス考慮版）
    // groupSize: 何個の三角形を1つの破片にまとめるか
    static void DestroyModelGrouped(
        MODEL* model,
        const XMMATRIX& worldMatrix,
        const Vector3& explosionCenter,
        float explosionForce,
        Scene* scene,
        int groupSize = 3
    );

private:
    // 三角形の中心座標を計算
    static Vector3 CalculateTriangleCenter(const XMFLOAT3& v0, const XMFLOAT3& v1, const XMFLOAT3& v2);

    // 頂点をワールド空間に変換
    static VERTEX_3D TransformVertex(const VERTEX_3D& vertex, const XMMATRIX& worldMatrix);
};