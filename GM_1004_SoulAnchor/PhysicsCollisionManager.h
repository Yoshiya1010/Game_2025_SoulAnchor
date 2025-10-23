#pragma once

#include"main.h"
#include "gameObject.h"
#include "PhysicsManager.h"


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
        if (!PhysicsManager::GetWorld()) return;

        PhysicsManager::GetWorld()->performDiscreteCollisionDetection();
        btDispatcher* dispatcher = PhysicsManager::GetWorld()->getDispatcher();
        int numManifolds = dispatcher->getNumManifolds();

        for (int i = 0; i < numManifolds; i++) {
            btPersistentManifold* contactManifold = dispatcher->getManifoldByIndexInternal(i);

            const btCollisionObject* objA = contactManifold->getBody0();
            const btCollisionObject* objB = contactManifold->getBody1();

            GameObject* gameObjA = static_cast<GameObject*>(objA->getUserPointer());
            GameObject* gameObjB = static_cast<GameObject*>(objB->getUserPointer());

            if (gameObjA && gameObjB && gameObjA != gameObjB) {
                int numContacts = contactManifold->getNumContacts();
                for (int j = 0; j < numContacts; j++) {
                    btManifoldPoint& pt = contactManifold->getContactPoint(j);

                    if (pt.getDistance() < 0.1f) {
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
    }

private:
    // 　 メイン処理：オブジェクトの組み合わせごとに分岐
    void ProcessSpecificCollision(GameObject* objA, GameObject* objB, const Vector3& hitPoint) {
        GameObjectTag tagA = objA->GetTag();
        GameObjectTag tagB = objB->GetTag();

        printf("Collision: %s (%d) <-> %s (%d)\n",
            objA->GetName().c_str(), (int)tagA,
            objB->GetName().c_str(), (int)tagB);

        // プレイヤーの弾丸 vs Ship
        if (tagA == GameObjectTag::PlayerBullet && tagB == GameObjectTag::Ship) {
            HandlePlayerBulletHitShip(objA, objB, hitPoint);
        }
        else if (tagB == GameObjectTag::PlayerBullet && tagA == GameObjectTag::Ship) {
            HandlePlayerBulletHitShip(objB, objA, hitPoint);
        }

        // 敵の弾丸 vs プレイヤー
        else if (tagA == GameObjectTag::EnemyBullet && tagB == GameObjectTag::Player) {
            HandleEnemyBulletHitPlayer(objA, objB, hitPoint);
        }
        else if (tagB == GameObjectTag::EnemyBullet && tagA == GameObjectTag::Player) {
            HandleEnemyBulletHitPlayer(objB, objA, hitPoint);
        }

        // プレイヤー vs Ship
        else if (tagA == GameObjectTag::Player && tagB == GameObjectTag::Ship) {
            HandlePlayerHitShip(objA, objB, hitPoint);
        }
        else if (tagB == GameObjectTag::Player && tagA == GameObjectTag::Ship) {
            HandlePlayerHitShip(objB, objA, hitPoint);
        }

        // Ship vs Ship
        else if (tagA == GameObjectTag::Ship && tagB == GameObjectTag::Ship) {
            HandleShipHitShip(objA, objB, hitPoint);
        }

        // プレイヤー vs アイテム
        else if (tagA == GameObjectTag::Player && tagB == GameObjectTag::Item) {
            HandlePlayerCollectItem(objA, objB, hitPoint);
        }
        else if (tagB == GameObjectTag::Player && tagA == GameObjectTag::Item) {
            HandlePlayerCollectItem(objB, objA, hitPoint);
        }

        // 弾丸 vs 壁/地面
        else if (IsBulletVsStatic(objA, objB)) {
            HandleBulletHitStatic(objA, objB, hitPoint);
        }

        // その他の衝突
        else {
            HandleGenericCollision(objA, objB, hitPoint);
        }
    }

    // 　 各種衝突処理の実装
    void HandlePlayerBulletHitShip(GameObject* bullet, GameObject* ship, const Vector3& hitPoint) {
        printf("Player bullet hit ship!\n");

        
      

        // 弾丸を消滅
        bullet->SetDestroy();
    }

    void HandleEnemyBulletHitPlayer(GameObject* bullet, GameObject* player, const Vector3& hitPoint) {
        printf("Enemy bullet hit player!\n");

      

        // 弾丸を消滅
        bullet->SetDestroy();
    }

    void HandlePlayerHitShip(GameObject* player, GameObject* ship, const Vector3& hitPoint) {
        printf("Player collided with ship!\n");

       
    }

    void HandleShipHitShip(GameObject* shipA, GameObject* shipB, const Vector3& hitPoint) {
        printf("Ship-to-ship collision!\n");

      
    }

    void HandlePlayerCollectItem(GameObject* player, GameObject* item, const Vector3& hitPoint) {
        printf("Player collected item: %s\n", item->GetName().c_str());

        // アイテム取得処理（将来的に拡張）
        // Player* playerObj = static_cast<Player*>(player);
        // playerObj->AddItem(item->GetItemType());

        // アイテムを消滅
        item->SetDestroy();
    }

    void HandleBulletHitStatic(GameObject* objA, GameObject* objB, const Vector3& hitPoint) {
        GameObject* bullet = GetBulletFromPair(objA, objB);
        GameObject* staticObj = (bullet == objA) ? objB : objA;

        printf("Bullet hit %s - destroyed\n", staticObj->GetName().c_str());

        // 弾丸を消滅
        if (bullet) {
            bullet->SetDestroy();
        }
    }

    void HandleGenericCollision(GameObject* objA, GameObject* objB, const Vector3& hitPoint) {
        printf("Generic collision between %s and %s\n",
            objA->GetName().c_str(), objB->GetName().c_str());

        // 基本的なコールバック
        objA->OnCollisionEnter(objB, hitPoint);
        objB->OnCollisionEnter(objA, hitPoint);
    }

    // 　 ヘルパー関数
    bool IsBulletVsStatic(GameObject* objA, GameObject* objB) {
        return (IsBullet(objA) && IsStatic(objB)) || (IsBullet(objB) && IsStatic(objA));
    }

    bool IsBullet(GameObject* obj) {
        GameObjectTag tag = obj->GetTag();
        return tag == GameObjectTag::PlayerBullet ||
            tag == GameObjectTag::EnemyBullet ||
            tag == GameObjectTag::Bullet;
    }

    bool IsStatic(GameObject* obj) {
        GameObjectTag tag = obj->GetTag();
        return tag == GameObjectTag::Wall || tag == GameObjectTag::Ground;
    }

    GameObject* GetBulletFromPair(GameObject* objA, GameObject* objB) {
        if (IsBullet(objA)) return objA;
        if (IsBullet(objB)) return objB;
        return nullptr;
    }
};