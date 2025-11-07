// PhysicsObject.cpp に追加

#include "FBXMeshSlicer.h"
#include "SlicedMeshFragment.h"
#include "ModelFBX.h"
#include"scene.h"
#include"manager.h"
#include <random>

// メッシュを実際に分割して破壊
void PhysicsObject::DestroyWithMeshSlicing(Vector3 impactPoint, Vector3 impactForce)
{
    printf("[%s] Mesh Slicing Destroy at impact: (%.2f, %.2f, %.2f)\n",
        GetName().c_str(), impactPoint.x, impactPoint.y, impactPoint.z);

    // FBXのaiSceneを取得する必要がある
    // ※ これにはStaticFBXModelにaiSceneへのアクセス方法を追加する必要がある
    // 今回は簡易的に、外部からaiSceneを渡す形にする

    // ここでは仮の実装として、m_ModelPath から再読み込み
    const aiScene* scene = aiImportFile(
        m_ModelPath.c_str(),
        aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded
    );

    if (!scene) {
        printf("[%s] Failed to load FBX for slicing: %s\n",
            GetName().c_str(), m_ModelPath.c_str());
        return;
    }

    // メッシュを分割
    std::vector<SlicedMesh> slicedMeshes = FBXMeshSlicer::SliceFBX(
        scene,
        m_Scale,
        m_VoxelGridX,
        m_VoxelGridY,
        m_VoxelGridZ
    );

    printf("[%s] Sliced into %zu mesh fragments\n",
        GetName().c_str(), slicedMeshes.size());

    // ランダム生成器
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> randomRotation(-15.0f, 15.0f);

    // 衝撃方向を正規化
    Vector3 impactDir = impactForce;
    if (impactDir.Length() > 0.001f) {
        impactDir.Normalize();
    }
    else {
        impactDir = Vector3(0, 1, 0);
    }

    // 各分割メッシュを破片として生成
    for (const auto& meshData : slicedMeshes) {
        auto* fragment = Manager::GetScene()->AddGameObject<SlicedMeshFragment>(OBJECT);

        // メッシュデータを設定（位置とスケールもここで設定される）
        fragment->SetMeshData(meshData);

        // ワールド空間の位置に変換
        Vector3 worldPos = m_Position + meshData.center;
        fragment->SetPosition(worldPos);

        // 力の計算
        Vector3 toFragment = worldPos - impactPoint;
        float distance = toFragment.Length();

        if (distance > 0.01f) {
            toFragment.Normalize();
        }
        else {
            toFragment = Vector3(0, 1, 0);
        }

        // 衝撃方向とランダム要素を混ぜる
        Vector3 forceDir = impactDir * 0.4f + toFragment * 0.6f;
        forceDir.y += 0.4f;
        forceDir.Normalize();

        // 距離に応じて力を減衰
        float forceMagnitude = 600.0f / (distance + 1.0f);
        forceMagnitude = std::max(forceMagnitude, 150.0f);
        forceMagnitude = std::min(forceMagnitude, 800.0f);

        Vector3 force = forceDir * forceMagnitude;

        // RigidBodyに力を加える（次フレームで適用）
        if (fragment->GetRigidBody()) {
            fragment->GetRigidBody()->applyCentralImpulse(
                btVector3(force.x, force.y, force.z)
            );

            fragment->GetRigidBody()->applyTorqueImpulse(
                btVector3(
                    randomRotation(gen),
                    randomRotation(gen),
                    randomRotation(gen)
                )
            );

            fragment->GetRigidBody()->activate(true);
        }
    }

    // シーンを解放
    aiReleaseImport(scene);

    // 元のオブジェクトを削除
    SetDestroy();

    printf("[%s] Mesh slicing complete!\n", GetName().c_str());
}

void PhysicsObject::SetDestructible(bool enable, float threshold) {
    m_IsDestructible = enable;
    m_DestructionThreshold = threshold;
}

void PhysicsObject::SetVoxelGrid(int x, int y, int z) {
    m_VoxelGridX = x;
    m_VoxelGridY = y;
    m_VoxelGridZ = z;
}

void PhysicsObject::SetFBXDestructionModel(const std::string& path, float modelScale) {
    m_ModelPath = path;
    m_ModelScale = modelScale;
}

bool PhysicsObject::IsDestructible() const {
    return m_IsDestructible;
}
// ========== PhysicsObject.cpp の最後に追加 ==========

#include "FBXFragment.h"
#include <random>
#include "manager.h"
#include "scene.h"

// 衝突チェック
void PhysicsObject::CheckDestruction(GameObject* other, const Vector3& hitPoint)
{
    if (!m_IsDestructible) return;

    PhysicsObject* otherPhysics = dynamic_cast<PhysicsObject*>(other);
    if (!otherPhysics) return;

    Vector3 velocity = otherPhysics->GetVelocity();
    float speed = velocity.Length();

    if (speed > m_DestructionThreshold) {
        printf("[%s] Destroyed by impact! Speed: %.2f\n", GetName().c_str(), speed);
        DestroyWithFBX(hitPoint, velocity);
    }
}

// FBX破壊
void PhysicsObject::DestroyWithFBX(Vector3 impactPoint, Vector3 impactForce)
{
    printf("[%s] FBX Destroy at impact: (%.2f, %.2f, %.2f)\n",
        GetName().c_str(), impactPoint.x, impactPoint.y, impactPoint.z);

    int gridX = m_VoxelGridX;
    int gridY = m_VoxelGridY;
    int gridZ = m_VoxelGridZ;

    Vector3 fragmentSize = Vector3(
        m_Scale.x / (float)gridX,
        m_Scale.y / (float)gridY,
        m_Scale.z / (float)gridZ
    );

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> randomRotation(-15.0f, 15.0f);
    std::uniform_real_distribution<float> randomScale(0.85f, 1.15f);

    Vector3 impactDir = impactForce;
    if (impactDir.Length() > 0.001f) {
        impactDir.Normalize();
    }
    else {
        impactDir = Vector3(0, 1, 0);
    }

    Vector3 objectCenter = m_Position;

    int fragmentCount = 0;
    for (int x = 0; x < gridX; x++) {
        for (int y = 0; y < gridY; y++) {
            for (int z = 0; z < gridZ; z++) {
                Vector3 localOffset = Vector3(
                    (x + 0.5f) * fragmentSize.x - m_Scale.x * 0.5f,
                    (y + 0.5f) * fragmentSize.y - m_Scale.y * 0.5f,
                    (z + 0.5f) * fragmentSize.z - m_Scale.z * 0.5f
                );

                Vector3 worldPos = objectCenter + localOffset;

                auto* fragment = Manager::GetScene()->AddGameObject<FBXFragment>(OBJECT);
                fragment->SetModelPath(m_ModelPath);
                fragment->SetModelScale(m_ModelScale);
                fragment->SetPosition(worldPos);

                float scaleVar = randomScale(gen);
                Vector3 fragScale = fragmentSize * 0.9f * scaleVar;
                fragment->SetScale(fragScale);

                fragment->SetLifetime(3.0f + randomScale(gen) * 2.0f);

                Vector3 toFragment = worldPos - impactPoint;
                float distance = toFragment.Length();

                if (distance > 0.01f) {
                    toFragment.Normalize();
                }
                else {
                    toFragment = Vector3(0, 1, 0);
                }

                Vector3 forceDir = impactDir * 0.4f + toFragment * 0.6f;
                forceDir.y += 0.4f;
                forceDir.Normalize();

                float forceMagnitude = 600.0f / (distance + 1.0f);
                forceMagnitude = std::max(forceMagnitude, 150.0f);
                forceMagnitude = std::min(forceMagnitude, 800.0f);

                Vector3 force = forceDir * forceMagnitude;

                if (fragment->GetRigidBody()) {
                    fragment->GetRigidBody()->applyCentralImpulse(
                        btVector3(force.x, force.y, force.z)
                    );

                    fragment->GetRigidBody()->applyTorqueImpulse(
                        btVector3(
                            randomRotation(gen),
                            randomRotation(gen),
                            randomRotation(gen)
                        )
                    );

                    fragment->GetRigidBody()->activate(true);
                }

                fragmentCount++;
            }
        }
    }

    SetDestroy();

    printf("[%s] Successfully destroyed into %d FBX fragments\n",
        GetName().c_str(), fragmentCount);
}