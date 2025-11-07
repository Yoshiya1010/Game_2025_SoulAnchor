#pragma once
#include "gameObject.h"
#include "scene.h"
#include "vector3.h"

class EditorObjectCreator {
public:
  
  

    //オブジェクトの生成
    static GameObject* CreateGroundBlock(Vector3 position, Vector3 scale);
    static GameObject* CreateTree(Vector3 position , Vector3 scale);
    static GameObject* CreateRockTall_A(Vector3 position, Vector3 scale);

    // ユーティリティメソッド
    static Vector3 GetSafeSpawnPosition();
 
private:
    static int s_CreatedObjectCount; // 連番用
};
