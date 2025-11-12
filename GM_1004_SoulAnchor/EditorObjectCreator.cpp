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
#include"camera.h"
#include"TreeBlock.h"
#include"RockTallBlock_A.h"
#include"sun.h"




GameObject* EditorObjectCreator::CreateGroundBlock(Vector3 position, Vector3 size) {
    
    
    //シーンを取得
    Scene* scene = Manager::GetScene();
    // 新しいGameObjectを作成（適切なクラスに置き換える）
    auto* block = scene->AddGameObject<GroundBlock>(OBJECT);
    block->SetMass(1.0f);
    block->SetPosition(position);
    block->SetScale(size);


    // シーンからユニークな名前をもらう
    block->SetName(scene->GenerateUniqueName("GroundBlock"));


    return block;
}

GameObject* EditorObjectCreator::CreateTree(Vector3 position, Vector3 size) {
   

    //シーンを取得
    Scene* scene = Manager::GetScene();
    // 新しいGameObjectを作成（適切なクラスに置き換える）
    auto* block = scene->AddGameObject<TreeBlock>(OBJECT);
    block->SetMass(1.0f);
    block->SetPosition(position);
    block->SetScale(size);


    // シーンからユニークな名前をもらう
    block->SetName(scene->GenerateUniqueName("TreeBlock"));


    return block;
}


GameObject* EditorObjectCreator::CreateRockTall_A(Vector3 position, Vector3 size) {


    //シーンを取得
    Scene* scene = Manager::GetScene();
    // 新しいGameObjectを作成（適切なクラスに置き換える）
    auto* block = scene->AddGameObject<RockTallBlock_A>(OBJECT);
    block->SetMass(1.0f);
    block->SetPosition(position);
    block->SetScale(size);


    // シーンからユニークな名前をもらう
    block->SetName(scene->GenerateUniqueName("RockTallBlock_A"));


    return block;
}






GameObject* EditorObjectCreator::CreateSun(Vector3 position, Vector3 size) {


    //シーンを取得
    Scene* scene = Manager::GetScene();
    // 新しいGameObjectを作成（適切なクラスに置き換える）
    auto*  sun= scene->AddGameObject<Sun>(OBJECT);
    sun->SetPosition(position);
    sun->SetScale(size);


    // シーンからユニークな名前をもらう
    sun->SetName(scene->GenerateUniqueName("Sun"));


    return sun;
}


GameObject* EditorObjectCreator::CreateEnemy() {


    //シーンを取得
    Scene* scene = Manager::GetScene();
    // 新しいGameObjectを作成（適切なクラスに置き換える）
    auto* enemy = scene->AddGameObject<Enemy>(OBJECT);
    enemy->SetPosition(Vector3{0.f,0.f,0.f});
    enemy->SetScale(Vector3{1.0f,1.0f,1.0f});


    // シーンからユニークな名前をもらう
    enemy->SetName(scene->GenerateUniqueName("Enemy"));


    return enemy;
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
    return Vector3(0, 3, 0);
}