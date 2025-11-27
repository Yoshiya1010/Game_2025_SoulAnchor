#pragma once
#include "main.h"
#include "vector3.h"

// 前方宣言
class Enemy;

// Enemyの状態基底クラス
class EnemyState
{
protected:
    Enemy* m_Enemy;

public:
    EnemyState(Enemy* enemy) : m_Enemy(enemy) {}
    virtual ~EnemyState() {}

    // 状態の処理
    virtual void Enter() = 0;
    virtual void Update() = 0;
    virtual void Exit() = 0;

    // 状態名取得（デバッグ用）
    virtual const char* GetStateName() const = 0;
};

// 待機状態
class EnemyIdleState : public EnemyState
{
public:
    EnemyIdleState(Enemy* enemy) : EnemyState(enemy) {}
    void Enter() override;
    void Update() override;
    void Exit() override;
    const char* GetStateName() const override { return "Idle"; }
};

// 巡回状態
class EnemyPatrolState : public EnemyState
{
private:
    float m_PatrolTimer;
    Vector3 m_PatrolTarget;
    bool m_HasTarget;

public:
    EnemyPatrolState(Enemy* enemy) : EnemyState(enemy), m_PatrolTimer(0.0f), m_HasTarget(false) {}
    void Enter() override;
    void Update() override;
    void Exit() override;
    const char* GetStateName() const override { return "Patrol"; }
};

// 追跡状態
class EnemyChaseState : public EnemyState
{
public:
    EnemyChaseState(Enemy* enemy) : EnemyState(enemy) {}
    void Enter() override;
    void Update() override;
    void Exit() override;
    const char* GetStateName() const override { return "Chase"; }
};

// 攻撃状態
class EnemyAttackState : public EnemyState
{
private:
    float m_AttackCooldown;

public:
    EnemyAttackState(Enemy* enemy) : EnemyState(enemy), m_AttackCooldown(0.0f) {}
    void Enter() override;
    void Update() override;
    void Exit() override;
    const char* GetStateName() const override { return "Attack"; }
};

// 死亡状態
class EnemyDeadState : public EnemyState
{
private:
    float m_DeathTimer;

public:
    EnemyDeadState(Enemy* enemy) : EnemyState(enemy), m_DeathTimer(0.0f) {}
    void Enter() override;
    void Update() override;
    void Exit() override;
    const char* GetStateName() const override { return "Dead"; }
};