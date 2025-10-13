#pragma once
#include <btBulletDynamicsCommon.h> 
#include <vector>



class BulletDebugDrawer; // 前方宣言

class PhysicsManager {
private:
    static btDiscreteDynamicsWorld* s_DynamicsWorld;
    static btBroadphaseInterface* s_Broadphase;
    static btDefaultCollisionConfiguration* s_CollisionConfig;
    static btCollisionDispatcher* s_Dispatcher;
    static btSequentialImpulseConstraintSolver* s_Solver;

    static BulletDebugDrawer* s_DebugDrawer;

public:
    static void Init();
    static void Uninit();
    static void Update();
    static btDiscreteDynamicsWorld* GetWorld() { return s_DynamicsWorld; }



    static BulletDebugDrawer* GetDebugDrawer() { return s_DebugDrawer; }
    static void SetDebugDrawEnabled(bool enabled);
    static void DrawDebugWorld(); // デバッグ描画実行
};