#pragma once
#include "gameObject.h"
#include "scene.h"
#include "vector3.h"

class EditorObjectCreator {
public:
    // 基本図形作成
    static GameObject* CreateCube(Vector3 position = Vector3(0, 5, 0), float size = 1.0f);
    static GameObject* CreateSphere(Vector3 position = Vector3(0, 5, 0), float radius = 1.0f);
    static GameObject* CreateCapsule(Vector3 position = Vector3(0, 5, 0), float radius = 0.5f, float height = 2.0f);
    static GameObject* CreateTerrain(Vector3 position);
    // 物理設定付き作成
    static GameObject* CreateStaticCube(Vector3 position, Vector3 size);
    static GameObject* CreateDynamicCube(Vector3 position, Vector3 size, float mass = 1.0f);

    static GameObject* CreateGroundBlock(Vector3 position, Vector3 scale);

    // ユーティリティメソッド
    static Vector3 GetSafeSpawnPosition();
    static Vector3 GetTerrainSpawnPosition(Vector3 worldPosition);
    static bool IsPositionOnTerrain(Vector3 position);


    static GameObject* CreateTerrainWithMeshCollider(Vector3 position);
    static GameObject* CreateCustomTerrain(Vector3 position, float heightData[21][21] = nullptr);
private:
    static int s_CreatedObjectCount; // 連番用
};
