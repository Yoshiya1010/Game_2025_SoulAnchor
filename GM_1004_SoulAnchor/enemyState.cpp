#include "EnemyState.h"
#include "enemy.h"
#include "manager.h"
#include "scene.h"
#include"FPSPlayer.h"
#include <cmath>

// 待機状態
void EnemyIdleState::Enter()
{
    AnimationModel* model = m_Enemy->GetAnimationModel();
    if (model)
    {
        std::vector<std::string> anims = model->GetAnimationNames();
        // Idle系のアニメーションを探す
        for (const auto& name : anims)
        {
            if (name.find("Idle") != std::string::npos ||
                name.find("idle") != std::string::npos)
            {
                model->Play(name.c_str(), true);
                return;
            }
        }
        // なければ最初のアニメーション
        if (!anims.empty())
        {
            model->Play(anims[0].c_str(), true);
        }
    }
}

void EnemyIdleState::Update()
{
    Scene* scene = Manager::GetScene();
    FPSPlayer* player = scene->GetGameObject<FPSPlayer>();

    if (player)
    {
        Vector3 enemyPos = m_Enemy->GetPosition();
        Vector3 playerPos = player->GetPosition();
        float distance = (playerPos - enemyPos).Length();

        if (distance < 10.0f)
        {
            m_Enemy->ChangeState(new EnemyChaseState(m_Enemy));
        }
    }
}

void EnemyIdleState::Exit()
{
}

// 巡回状態
void EnemyPatrolState::Enter()
{
    m_PatrolTimer = 0.0f;
    m_HasTarget = false;

    AnimationModel* model = m_Enemy->GetAnimationModel();
    if (model)
    {
        std::vector<std::string> anims = model->GetAnimationNames();
        // Walk/Run系のアニメーションを探す
        for (const auto& name : anims)
        {
            if (name.find("Walk") != std::string::npos ||
                name.find("walk") != std::string::npos)
            {
                model->Play(name.c_str(), true);
                return;
            }
        }
    }
}

void EnemyPatrolState::Update()
{
    if (!m_HasTarget)
    {
        Vector3 currentPos = m_Enemy->GetPosition();
        m_PatrolTarget = currentPos + Vector3(
            (rand() % 20 - 10) * 0.5f,
            0.0f,
            (rand() % 20 - 10) * 0.5f
        );
        m_HasTarget = true;
    }

    Vector3 currentPos = m_Enemy->GetPosition();
    Vector3 direction = m_PatrolTarget - currentPos;
    direction.y = 0.0f;
    float distance = direction.Length();

    if (distance > 0.5f)
    {
        direction.Normalize();
        Vector3 newPos = currentPos + direction * 0.05f;
        m_Enemy->SetPosition(newPos);

        float angle = atan2f(direction.x, direction.z);
        m_Enemy->SetRotation(Vector3(0.0f, angle, 0.0f));
    }
    else
    {
        m_HasTarget = false;
        m_PatrolTimer += 0.016f;

        if (m_PatrolTimer > 2.0f)
        {
            m_PatrolTimer = 0.0f;
        }
    }

    Scene* scene = Manager::GetScene();
    FPSPlayer* player = scene->GetGameObject<FPSPlayer>();
    if (player)
    {
        Vector3 playerPos = player->GetPosition();
        float playerDist = (playerPos - currentPos).Length();
        if (playerDist < 10.0f)
        {
            m_Enemy->ChangeState(new EnemyChaseState(m_Enemy));
        }
    }
}

void EnemyPatrolState::Exit()
{
}

// 追跡状態
void EnemyChaseState::Enter()
{
    AnimationModel* model = m_Enemy->GetAnimationModel();
    if (model)
    {
        std::vector<std::string> anims = model->GetAnimationNames();
        // Run系のアニメーションを探す
        for (const auto& name : anims)
        {
            if (name.find("Sprint") != std::string::npos ||
                name.find("sprint") != std::string::npos)
            {
                model->Play(name.c_str(), true);
                return;
            }
        }
    }
}
// 追跡状態
void EnemyChaseState::Update()
{
    Scene* scene = Manager::GetScene();
    FPSPlayer* player = scene->GetGameObject<FPSPlayer>();

    if (!player)
    {
        m_Enemy->ChangeState(new EnemyIdleState(m_Enemy));
        return;
    }

    Vector3 currentPos = m_Enemy->GetPosition();
    Vector3 playerPos = player->GetPosition();
    Vector3 direction = playerPos - currentPos;
    direction.y = 0.0f;
    float distance = direction.Length();

    if (distance < 2.0f)
    {
        m_Enemy->ChangeState(new EnemyAttackState(m_Enemy));
        return;
    }
    else if (distance > 50.0f)
    {
        m_Enemy->ChangeState(new EnemyIdleState(m_Enemy));
        return;
    }

    // プレイヤー方向を向く
    if (direction.Length() > 0.01f)
    {
        direction.Normalize();

        btRigidBody* body = m_Enemy->GetRigidBody();
        if (body)
        {
            // 現在の向き
            Vector3 currentRot = m_Enemy->GetRotation();
            float currentYaw = currentRot.y;

            // 目標の向き
            float targetYaw = atan2f(direction.x, direction.z);

            // 角度差を計算（-π ~ π の範囲に正規化）
            float angleDiff = targetYaw - currentYaw;
            while (angleDiff > XM_PI) angleDiff -= XM_2PI;
            while (angleDiff < -XM_PI) angleDiff += XM_2PI;

            // 角速度を設定（角度差に比例）
            float rotationSpeed = 5.0f; // 回転速度の調整値
            float angularVelocity = angleDiff * rotationSpeed;

            // 角速度を制限（回転が速すぎないように）
            float maxAngularVel = 3.0f;
            if (angularVelocity > maxAngularVel) angularVelocity = maxAngularVel;
            if (angularVelocity < -maxAngularVel) angularVelocity = -maxAngularVel;

            // Y軸回転の角速度を設定
            btVector3 currentAngVel = body->getAngularVelocity();
            body->setAngularVelocity(btVector3(0, angularVelocity, 0));

            // 移動速度を設定
            btVector3 velocity = body->getLinearVelocity();
            float chaseSpeed = 2.0f;

            btVector3 targetVelocity(
                direction.x * chaseSpeed,
                velocity.y(),
                direction.z * chaseSpeed
            );

            body->setLinearVelocity(targetVelocity);
            body->activate();
        }
    }
}

void EnemyChaseState::Exit()
{
    // 追跡終了時に速度と角速度をリセット
    btRigidBody* body = m_Enemy->GetRigidBody();
    if (body)
    {
        btVector3 velocity = body->getLinearVelocity();
        body->setLinearVelocity(btVector3(0, velocity.y(), 0));
        body->setAngularVelocity(btVector3(0, 0, 0));
    }
}
// 攻撃状態
void EnemyAttackState::Enter()
{
    m_AttackCooldown = 0.0f;

    AnimationModel* model = m_Enemy->GetAnimationModel();
    if (model)
    {
        std::vector<std::string> anims = model->GetAnimationNames();
        // Attack系のアニメーションを探す
        for (const auto& name : anims)
        {
            if (name.find("Attack") != std::string::npos ||
                name.find("attack") != std::string::npos ||
                name.find("Punch") != std::string::npos ||
                name.find("punch") != std::string::npos)
            {
                model->Play(name.c_str(), false); // ループなし
                return;
            }
        }
    }
}

void EnemyAttackState::Update()
{
    m_AttackCooldown += 0.016f;

    Scene* scene = Manager::GetScene();
    FPSPlayer* player = scene->GetGameObject<FPSPlayer>();

    if (!player)
    {
        m_Enemy->ChangeState(new EnemyIdleState(m_Enemy));
        return;
    }

    Vector3 currentPos = m_Enemy->GetPosition();
    Vector3 playerPos = player->GetPosition();
    Vector3 direction = playerPos - currentPos;
    direction.y = 0.0f;
    float distance = direction.Length();

    // プレイヤー方向を向く
    if (direction.Length() > 0.01f)
    {
        direction.Normalize();
        float angle = atan2f(direction.x, direction.z);
        m_Enemy->SetRotation(Vector3(0.0f, angle, 0.0f));
    }

    // アニメーション終了確認
    AnimationModel* model = m_Enemy->GetAnimationModel();
    bool animFinished = false;
    if (model)
    {
        animFinished = model->IsAnimationFinished();
    }

    if (m_AttackCooldown > 1.5f || animFinished)
    {
        if (distance < 2.5f)
        {
            // ダメージ処理
            // player->Damage(10.0f); など
        }

        if (distance > 3.0f)
        {
            m_Enemy->ChangeState(new EnemyChaseState(m_Enemy));
        }
        else
        {
            // 次の攻撃準備
            m_AttackCooldown = 0.0f;
            AnimationModel* model = m_Enemy->GetAnimationModel();
            if (model)
            {
                std::vector<std::string> anims = model->GetAnimationNames();
                for (const auto& name : anims)
                {
                    if (name.find("Attack") != std::string::npos ||
                        name.find("attack") != std::string::npos)
                    {
                        model->Play(name.c_str(), false);
                        break;
                    }
                }
            }
        }
    }
}

void EnemyAttackState::Exit()
{
}

// 死亡状態
void EnemyDeadState::Enter()
{
    m_DeathTimer = 0.0f;

    AnimationModel* model = m_Enemy->GetAnimationModel();
    if (model)
    {
        std::vector<std::string> anims = model->GetAnimationNames();
        // Death/Die系のアニメーションを探す
        for (const auto& name : anims)
        {
            if (name.find("Death") != std::string::npos ||
                name.find("death") != std::string::npos ||
                name.find("Die") != std::string::npos ||
                name.find("die") != std::string::npos)
            {
                model->Play(name.c_str(), false);
                return;
            }
        }
    }

   
}

void EnemyDeadState::Update()
{
    m_DeathTimer += 0.016f;

    // アニメーション終了確認
    AnimationModel* model = m_Enemy->GetAnimationModel();
    bool animFinished = false;
    if (model)
    {
        animFinished = model->IsAnimationFinished();
    }

    // アニメーション終了後に削除
    if (animFinished || m_DeathTimer > 1.0f)
    {
        // 死亡時の破壊処理
        m_Enemy->DestroyObject(Vector3(0.0f, 0.0f, 0.0f), 10.0f);
    }
}

void EnemyDeadState::Exit()
{
}