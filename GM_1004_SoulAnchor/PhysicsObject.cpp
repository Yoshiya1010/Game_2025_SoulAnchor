
#include"PhysicsObject.h"
#include"gameObject.h"
#include"VoxelFragment.h"
#include"random"
#include"scene.h"
#include "manager.h"

void PhysicsObject::Destroy(Vector3 impactPoint, Vector3 impactForce)
{
    // ========== ステップ1: 準備 ==========

    // 分割数を取得（例: 4x4x4 = 64個）
    int gridX = m_VoxelGridX;  // X方向に4分割
    int gridY = m_VoxelGridY;  // Y方向に4分割
    int gridZ = m_VoxelGridZ;  // Z方向に4分割

    // 各破片のサイズを計算
    // 例: ブロックが (10, 10, 10) なら、破片は (2.5, 2.5, 2.5)
    Vector3 fragmentSize = Vector3(m_Scale.x / m_VoxelGridX, m_Scale.x / m_VoxelGridX, m_Scale.x / m_VoxelGridX);


    // ========== ステップ2: ランダム生成器の準備 ==========

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> randomRotation(-10.0f, 10.0f);  // 回転
    std::uniform_real_distribution<float> randomColorVariation(0.8f, 1.2f);  // 色の変化


    // ========== ステップ3: 衝撃方向を正規化 ==========

    Vector3 impactDir = impactForce;  // アンカーの速度ベクトル
    if (impactDir.Length() > 0) {
        impactDir.Normalize();  // 長さを1にする
    }
    else {
        impactDir = Vector3(0, 1, 0);  // 速度が0なら上方向
    }


    // ========== ステップ4: グリッド分割してループ ==========

    for (int x = 0; x < gridX; x++) {        // X方向に4回
        for (int y = 0; y < gridY; y++) {    // Y方向に4回
            for (int z = 0; z < gridZ; z++) {  // Z方向に4回

                // --- 4.1: 破片の位置を計算 ---

                // ブロックの中心を(0,0,0)として、各グリッドの位置を計算
                Vector3 localOffset = Vector3(
                    (x + 0.5f) * fragmentSize.x - m_Scale.x * 0.5f,
                    (y + 0.5f) * fragmentSize.y - m_Scale.y * 0.5f,
                    (z + 0.5f) * fragmentSize.z - m_Scale.z * 0.5f
                );

                // ワールド座標に変換
                Vector3 worldPos = m_Position + localOffset;


                // --- 4.2: 破片オブジェクトを生成 ---

                auto* fragment = Manager::GetScene()->AddGameObject<VoxelFragment>(OBJECT);
                fragment->SetPosition(worldPos);
                fragment->SetScale(fragmentSize * 0.9f);  // 0.9倍（少し隙間を作る）


                // --- 4.3: ランダムな色をつける ---

                float variation = randomColorVariation(gen);  // 0.8〜1.2のランダム値
                XMFLOAT4 color = XMFLOAT4(
                    m_FragmentColor.x * variation,  // 赤成分
                    m_FragmentColor.y * variation,  // 緑成分
                    m_FragmentColor.z * variation,  // 青成分
                    m_FragmentColor.w               // アルファ
                );
                fragment->SetColor(color);


                // --- 4.4: 力の方向を計算 ---

                // 衝撃点から破片への方向ベクトル
                Vector3 fragmentToImpact = worldPos - impactPoint;
                float distance = fragmentToImpact.Length();

                if (distance > 0.01f) {
                    fragmentToImpact.Normalize();
                }
                else {
                    fragmentToImpact = Vector3(0, 1, 0);  // 真上
                }

                // 衝撃方向50% + 放射方向50%
                Vector3 forceDir = impactDir * 0.5f + fragmentToImpact * 0.5f;
                forceDir.y += 0.3f;  // 少し上向きに（重力で落ちるので）
                forceDir.Normalize();


                // --- 4.5: 力の大きさを計算 ---

                // 衝撃点に近いほど強く飛ぶ
                float forceMagnitude = 500.0f / (distance + 1.0f);
                forceMagnitude = std::max(forceMagnitude, 100.0f);  // 最低100

                Vector3 force = forceDir * forceMagnitude;


                // --- 4.6: 物理演算で力を加える ---

                if (fragment->GetRigidBody()) {
                    // 直線運動の力
                    fragment->GetRigidBody()->applyCentralImpulse(
                        btVector3(force.x, force.y, force.z)
                    );

                    // ランダムな回転運動
                    fragment->GetRigidBody()->applyTorqueImpulse(
                        btVector3(
                            randomRotation(gen),  // X軸回転
                            randomRotation(gen),  // Y軸回転
                            randomRotation(gen)   // Z軸回転
                        )
                    );

                    fragment->GetRigidBody()->activate(true);
                }
            }
        }
    }

    // ========== ステップ5: 元のオブジェクトを削除 ==========
    SetDestroy();
}
void PhysicsObject::CheckDestruction(GameObject* other, const Vector3& hitPoint)
{
    // ========== ステップ1: 破壊可能かチェック ==========
    if (!m_IsDestructible) return;  // 破壊不可なら何もしない


    // ========== ステップ2: 衝突相手の速度を取得 ==========

    // 衝突相手をPhysicsObjectにキャスト
    PhysicsObject* otherPhysics = dynamic_cast<PhysicsObject*>(other);
    if (!otherPhysics) return;  // PhysicsObjectじゃなければ終了

    // 相手の速度ベクトルを取得
    Vector3 velocity = otherPhysics->GetVelocity();

    // 速度の大きさ（スピード）を計算
    float speed = velocity.Length();


    // ========== ステップ3: 速度が閾値を超えたら破壊 ==========

    if (speed > m_DestructionThreshold) {  // 例: 20.0f より速い？
        printf("[%s] Destroyed by impact! Speed: %.2f\n",
            GetName().c_str(), speed);

        // 破壊実行！
        Destroy(hitPoint, velocity);
    }
}