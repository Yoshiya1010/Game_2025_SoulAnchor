#include "EditorObjectCreator.h"
#include "manager.h"
#include "player.h"
#include"enemy.h"
#include"MeshField.h"
#include<iostream>
#include"json.hpp"
#include <fstream>
#include"GroundBlock.h"
#include"gameObject.h"
#include"PhysicsObject.h"


int EditorObjectCreator::s_CreatedObjectCount = 0;

GameObject* EditorObjectCreator::CreateGroundBlock(Vector3 position, float size) {
    
    
    //シーンを取得
    Scene* scene = Manager::GetScene();
    // 新しいGameObjectを作成（適切なクラスに置き換える）
    auto* cube = scene->AddGameObject<GroundBlock>(OBJECT);

    // 名前設定
    s_CreatedObjectCount++;
    char name[64];
    sprintf_s(name, "EditorGround_%03d", s_CreatedObjectCount);


   

    return cube;
}

GameObject* EditorObjectCreator::CreateCube(Vector3 position, float size) {
    Scene* scene = Manager::GetScene();




    return 0;
}

GameObject* EditorObjectCreator::CreateSphere(Vector3 position, float radius) {
    //Scene* scene = Manager::GetScene();

    //auto* sphere = scene->AddGameObject<Player>(OBJECT);

    //s_CreatedObjectCount++;
    //char name[64];
    //sprintf_s(name, "EditorSphere_%03d", s_CreatedObjectCount);
    //sphere->SetName(name);

    //sphere->SetPosition(position);
    //sphere->SetScale(Vector3(radius, radius, radius));
    //sphere->SetTag(GameObjectTag::Item);

    //// SphereCollider作成
    //sphere->CreateSphereColliderAt(position, radius, 1.0f);

    return 0;
}

GameObject* EditorObjectCreator::CreateCapsule(Vector3 position, float radius, float height) {
    //Scene* scene = Manager::GetScene();

    //auto* capsule = scene->AddGameObject<Player>(OBJECT);

    //s_CreatedObjectCount++;
    //char name[64];
    //sprintf_s(name, "EditorCapsule_%03d", s_CreatedObjectCount);
    //capsule->SetName(name);

    //capsule->SetPosition(position);
    //capsule->SetScale(Vector3(radius, height, radius));
    //capsule->SetTag(GameObjectTag::Item);

    //// CapsuleCollider作成
    //capsule->CreateCapsuleColliderAt(position, radius, height, 1.0f);

    return 0;
}

GameObject* EditorObjectCreator::CreateStaticCube(Vector3 position, Vector3 size) {
    //Scene* scene = Manager::GetScene();

    //auto* staticCube = scene->AddGameObject<Player>(OBJECT);

    //s_CreatedObjectCount++;
    //char name[64];
    //sprintf_s(name, "StaticCube_%03d", s_CreatedObjectCount);
    //staticCube->SetName(name);

    //staticCube->SetPosition(position);
    //staticCube->SetScale(size);
    //staticCube->SetTag(GameObjectTag::Wall);

    //// 質量0で静的オブジェクト
    //staticCube->CreateBoxColliderAt(position, size, 0.0f);

    return 0;
}

GameObject* EditorObjectCreator::CreateDynamicCube(Vector3 position, Vector3 size, float mass) {
    //Scene* scene = Manager::GetScene();

    //auto* dynamicCube = scene->AddGameObject<Player>(OBJECT);

    //s_CreatedObjectCount++;
    //char name[64];
    //sprintf_s(name, "DynamicCube_%03d", s_CreatedObjectCount);
    //dynamicCube->SetName(name);

    //dynamicCube->SetPosition(position);
    //dynamicCube->SetScale(size);
    //dynamicCube->SetTag(GameObjectTag::Item);

    //// 指定質量で動的オブジェクト
    //dynamicCube->CreateBoxColliderAt(position, size, mass);

    return 0;
}

GameObject* EditorObjectCreator::CreateTerrain(Vector3 position) {
    Scene* scene = Manager::GetScene();
    auto* terrain = scene->AddGameObject<MeshField>(OBJECT);
    s_CreatedObjectCount++;
    char name[64];
    sprintf_s(name, "Terrain_%03d", s_CreatedObjectCount);
    terrain->SetName(name);
    terrain->SetPosition(position);
    terrain->SetScale(Vector3(1.0f, 1.0f, 1.0f));
    terrain->SetTag(GameObjectTag::Ground);
    return terrain;
}

Vector3 EditorObjectCreator::GetSafeSpawnPosition() {
    //// カメラから見える位置に配置
    //Scene* scene = Manager::GetScene();
    //auto* camera = scene->GetGameObject<Camera>();

    //if (camera) {
    //    Vector3 camPos = camera->GetPosition();
    //    // カメラの前方5ユニット、高さ+2の位置
    //    return Vector3(camPos.x, camPos.y + 2, camPos.z - 5);
    //}

    //// デフォルト位置
    return Vector3(0, 20, 0);
}