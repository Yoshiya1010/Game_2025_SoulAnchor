#include "EditorObjectCreator.h"
#include "manager.h"
#include "player.h"
#include"enemy.h"
#include"MeshField.h"
#include<iostream>
#include"json.hpp"
#include <fstream>
#include"GroundBlock.h"

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


    nlohmann::json m_json = {
    {"Name","Ground"},
    {"Position.x",cube->GetPosition().x},
    {"Position.y",cube->GetPosition().y},
    {"Position.z",cube->GetPosition().z},
    {"size",cube->GetScale().x},
    {"size",cube->GetScale().y},
    {"size",cube->GetScale().z},
    {"Tag",cube->GetTag()}
    };


    //ここでファイルを作成
    

    std::string filename = m_json["Name"];
    std::string Extension = ".json";
    filename += Extension;

    //作成したファイルに内容を書き込む
    std::ofstream writing_file;
    writing_file.open(filename, std::ios::out);
    writing_file << m_json.dump() << std::endl;
    writing_file.close();

    return cube;
}

GameObject* EditorObjectCreator::CreateCube(Vector3 position, float size) {
    Scene* scene = Manager::GetScene();

    // 新しいGameObjectを作成（適切なクラスに置き換える）
    auto* cube = scene->AddGameObject<Enemy>(OBJECT);

    // 名前設定
    s_CreatedObjectCount++;
    char name[64];
    sprintf_s(name, "EditorCube_%03d", s_CreatedObjectCount);
    cube->SetName(name);

    // 位置・スケール設定
    cube->SetPosition(position);
    cube->SetScale(Vector3(size, size, size));
    cube->SetTag(GameObjectTag::Item);

    // BoxCollider作成
    cube->CreateBoxColliderAt(position, Vector3(size, size, size), 1.0f);



    return cube;
}

GameObject* EditorObjectCreator::CreateSphere(Vector3 position, float radius) {
    Scene* scene = Manager::GetScene();

    auto* sphere = scene->AddGameObject<Player>(OBJECT);

    s_CreatedObjectCount++;
    char name[64];
    sprintf_s(name, "EditorSphere_%03d", s_CreatedObjectCount);
    sphere->SetName(name);

    sphere->SetPosition(position);
    sphere->SetScale(Vector3(radius, radius, radius));
    sphere->SetTag(GameObjectTag::Item);

    // SphereCollider作成
    sphere->CreateSphereColliderAt(position, radius, 1.0f);

    return sphere;
}

GameObject* EditorObjectCreator::CreateCapsule(Vector3 position, float radius, float height) {
    Scene* scene = Manager::GetScene();

    auto* capsule = scene->AddGameObject<Player>(OBJECT);

    s_CreatedObjectCount++;
    char name[64];
    sprintf_s(name, "EditorCapsule_%03d", s_CreatedObjectCount);
    capsule->SetName(name);

    capsule->SetPosition(position);
    capsule->SetScale(Vector3(radius, height, radius));
    capsule->SetTag(GameObjectTag::Item);

    // CapsuleCollider作成
    capsule->CreateCapsuleColliderAt(position, radius, height, 1.0f);

    return capsule;
}

GameObject* EditorObjectCreator::CreateStaticCube(Vector3 position, Vector3 size) {
    Scene* scene = Manager::GetScene();

    auto* staticCube = scene->AddGameObject<Player>(OBJECT);

    s_CreatedObjectCount++;
    char name[64];
    sprintf_s(name, "StaticCube_%03d", s_CreatedObjectCount);
    staticCube->SetName(name);

    staticCube->SetPosition(position);
    staticCube->SetScale(size);
    staticCube->SetTag(GameObjectTag::Wall);

    // 質量0で静的オブジェクト
    staticCube->CreateBoxColliderAt(position, size, 0.0f);

    return staticCube;
}

GameObject* EditorObjectCreator::CreateDynamicCube(Vector3 position, Vector3 size, float mass) {
    Scene* scene = Manager::GetScene();

    auto* dynamicCube = scene->AddGameObject<Player>(OBJECT);

    s_CreatedObjectCount++;
    char name[64];
    sprintf_s(name, "DynamicCube_%03d", s_CreatedObjectCount);
    dynamicCube->SetName(name);

    dynamicCube->SetPosition(position);
    dynamicCube->SetScale(size);
    dynamicCube->SetTag(GameObjectTag::Item);

    // 指定質量で動的オブジェクト
    dynamicCube->CreateBoxColliderAt(position, size, mass);

    return dynamicCube;
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