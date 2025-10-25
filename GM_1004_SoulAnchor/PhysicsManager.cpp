// PhysicsManager.cpp
#include "PhysicsManager.h"
#include <iostream>
#include "BulletDebugDrawer.h"
#include"input.h"
#include"scene.h"

// 静的メンバーの定義
btDiscreteDynamicsWorld* PhysicsManager::s_DynamicsWorld = nullptr;
btBroadphaseInterface* PhysicsManager::s_Broadphase = nullptr;
btDefaultCollisionConfiguration* PhysicsManager::s_CollisionConfig = nullptr;
btCollisionDispatcher* PhysicsManager::s_Dispatcher = nullptr;
btSequentialImpulseConstraintSolver* PhysicsManager::s_Solver = nullptr;

BulletDebugDrawer* PhysicsManager::s_DebugDrawer = nullptr;


void PhysicsManager::Init() {
    // 衝突検出の設定
    s_CollisionConfig = new btDefaultCollisionConfiguration();
    s_Dispatcher = new btCollisionDispatcher(s_CollisionConfig);

    // ブロードフェーズ（大雑把な衝突検出）
    s_Broadphase = new btDbvtBroadphase();

    // 制約ソルバー
    s_Solver = new btSequentialImpulseConstraintSolver();

    // 物理世界の作成
    s_DynamicsWorld = new btDiscreteDynamicsWorld(
        s_Dispatcher, s_Broadphase, s_Solver, s_CollisionConfig);

    // 重力設定（海戦ゲームなので下向き）
    s_DynamicsWorld->setGravity(btVector3(0, -9.81f, 0));

    btTransform worldTransform;
    worldTransform.setIdentity();
    worldTransform.setBasis(btMatrix3x3(
        1, 0, 0,
        0, 1, 0,
        0, 0, -1  // Z反転で左手化にして描画と揃えた
    ));

    // デバッグドローワー作成・設定
    s_DebugDrawer = new BulletDebugDrawer();
    s_DebugDrawer->Init();
    s_DynamicsWorld->setDebugDrawer(s_DebugDrawer);

    // デフォルトではワイヤーフレーム表示を有効に
    s_DebugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
    s_DebugDrawer->SetEnabled(true);


    std::cout << "Physics world initialized!" << std::endl;
}

void PhysicsManager::Uninit() {

    if (s_DebugDrawer)
    {
        s_DebugDrawer->Uninit();
        delete s_DebugDrawer;
        s_DebugDrawer = nullptr;
    }


    // 逆順で削除
    if (s_DynamicsWorld) {
        delete s_DynamicsWorld;
        s_DynamicsWorld = nullptr;
    }
    if (s_Solver) {
        delete s_Solver;
        s_Solver = nullptr;
    }
    if (s_Broadphase) {
        delete s_Broadphase;
        s_Broadphase = nullptr;
    }
    if (s_Dispatcher) {
        delete s_Dispatcher;
        s_Dispatcher = nullptr;
    }
    if (s_CollisionConfig) {
        delete s_CollisionConfig;
        s_CollisionConfig = nullptr;
    }

    std::cout << "Physics world cleaned up!" << std::endl;
}

void PhysicsManager::SetDebugDrawEnabled(bool enabled)
{
    if (s_DebugDrawer)
    {
        s_DebugDrawer->SetEnabled(enabled);
    }
}

void PhysicsManager::DrawDebugWorld()
{
    if (s_DebugDrawer && s_DynamicsWorld)
    {
        s_DebugDrawer->Begin();
        s_DynamicsWorld->debugDrawWorld();
        s_DebugDrawer->End();
    }
}

void PhysicsManager::Update() {
    if (s_DynamicsWorld) {
        // 物理シミュレーションを1フレーム進める
        s_DynamicsWorld->stepSimulation( Scene::GetDeltaTime(), 10);
    }

    if (Input::GetKeyTrigger(KK_F1))
    {
        static bool debugEnabled = true;
        debugEnabled = !debugEnabled;
        PhysicsManager::SetDebugDrawEnabled(debugEnabled);

        printf("Physics Debug Draw: %s\n", debugEnabled ? "ON" : "OFF");
    }

    // より詳細な表示モード切り替え（F2キー）
    if (Input::GetKeyTrigger(KK_F2))
    {
        auto debugDrawer = PhysicsManager::GetDebugDrawer();
        if (debugDrawer)
        {
            int currentMode = debugDrawer->getDebugMode();
            int newMode = btIDebugDraw::DBG_DrawWireframe |
                btIDebugDraw::DBG_DrawContactPoints |
                btIDebugDraw::DBG_DrawConstraints |
                btIDebugDraw::DBG_DrawConstraintLimits;

            debugDrawer->setDebugMode(newMode);
        }
    }
}