#pragma once

#include"main.h"
#include "gameObject.h"
#include "PhysicsManager.h"
#include"FPSPlayer.h"


class PhysicsCollisionManager {
private:
    static PhysicsCollisionManager* s_Instance;

public:
    static PhysicsCollisionManager* GetInstance() {
        if (!s_Instance) {
            s_Instance = new PhysicsCollisionManager();
        }
        return s_Instance;
    }

    static void Init() {
        GetInstance();
    }

    static void Uninit() {
        if (s_Instance) {
            delete s_Instance;
            s_Instance = nullptr;
        }
    }

    void CheckCollisions() {
        auto* world = PhysicsManager::GetWorld();
        if (!world) return;

        world->performDiscreteCollisionDetection();
        btDispatcher* dispatcher = world->getDispatcher();

        // 逆順ループ + 毎回数を取り直す
        for (int i = dispatcher->getNumManifolds() - 1; i >= 0; --i)
        {
            int currentNum = dispatcher->getNumManifolds();
            if (i >= currentNum) continue; // 範囲外になっていたらスキップ

            btPersistentManifold* contactManifold =
                dispatcher->getManifoldByIndexInternal(i);

            const btCollisionObject* objA = contactManifold->getBody0();
            const btCollisionObject* objB = contactManifold->getBody1();

            GameObject* gameObjA =
                static_cast<GameObject*>(objA->getUserPointer());
            GameObject* gameObjB =
                static_cast<GameObject*>(objB->getUserPointer());

            if (!gameObjA || !gameObjB || gameObjA == gameObjB)
                continue;

            int numContacts = contactManifold->getNumContacts();
            for (int j = 0; j < numContacts; ++j)
            {
                btManifoldPoint& pt = contactManifold->getContactPoint(j);

                if (pt.getDistance() < 0.1f)  // ここは今の条件のままでOK
                {
                    Vector3 hitPoint(
                        pt.getPositionWorldOnA().getX(),
                        pt.getPositionWorldOnA().getY(),
                        pt.getPositionWorldOnA().getZ()
                    );

                    ProcessSpecificCollision(gameObjA, gameObjB, hitPoint);
                    break;
                }
            }
        }
    }

private:
    // 　 メイン処理：オブジェクトの組み合わせごとに分岐
    void ProcessSpecificCollision(GameObject* objA, GameObject* objB, const Vector3& hitPoint) {
        GameObjectTag tagA = objA->GetTag();
        GameObjectTag tagB = objB->GetTag();

        printf("Collision: %s (%d) <-> %s (%d)\n",
            objA->GetName().c_str(), (int)tagA,
            objB->GetName().c_str(), (int)tagB);



        // プレイヤー と地面
        if (tagA == GameObjectTag::Player && tagB == GameObjectTag::Ground) {
           HandlePlayerHitGround(objA, objB, hitPoint);
        }
        else if (tagB == GameObjectTag::Player && tagA == GameObjectTag::Ground) {
            HandlePlayerHitGround(objB, objA, hitPoint);
        }

        // その他の衝突
        else {
            HandleGenericCollision(objA, objB, hitPoint);
        }
    }



    void HandlePlayerCollectItem(GameObject* player, GameObject* item, const Vector3& hitPoint) {
        printf("Player collected item: %s\n", item->GetName().c_str());

        // アイテム取得処理（将来的に拡張）
        // Player* playerObj = static_cast<Player*>(player);
        // playerObj->AddItem(item->GetItemType());

        // アイテムを消滅
        item->SetDestroy();
    }

    //プレイヤーと地面が衝突した
    void HandlePlayerHitGround(GameObject* objA, GameObject* objB, const Vector3& hitPoint)
    {
        GameObject* playerObj = (objA->GetTag() == GameObjectTag::Player) ? objA : objB;

        FPSPlayer* player = dynamic_cast<FPSPlayer*>(playerObj);
        if (player)
        {
            player->SetOnGround(true);//地面とプレイヤーが衝突した
        }
    }
   

    void HandleGenericCollision(GameObject* objA, GameObject* objB, const Vector3& hitPoint) {
  

        //// 基本的なコールバック
        objA->OnCollisionEnter(objB, hitPoint);
        objB->OnCollisionEnter(objA, hitPoint);
    }

  



    bool IsStatic(GameObject* obj) {
        GameObjectTag tag = obj->GetTag();
        return tag == GameObjectTag::Wall || tag == GameObjectTag::Ground;
    }

 


 
};