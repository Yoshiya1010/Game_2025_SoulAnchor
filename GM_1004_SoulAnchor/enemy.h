#pragma once

#include "main.h"
#include "renderer.h"
#include"animationModel.h"
#include "gameObject.h"
#include"fragmentObject.h"

// ‘O•ûéŒ¾
class EnemyState;

class Enemy :public FragmentObject {
private:
    unique_ptr<AnimationModel> m_AnimationModel;
    ID3D11ShaderResourceView* m_CachedTexture = nullptr;

    // StateŠÇ—
    EnemyState* m_CurrentState;
    float m_Health;
    float m_MaxHealth;

public:
    void Init() override;
    void Start() override;
    void Uninit() override;
    void Update() override;
    void Draw() override;
    AnimationModel* GetAnimationModel();

    // StateØ‚è‘Ö‚¦
    void ChangeState(EnemyState* newState);

    // HPŠÇ—
    void Damage(float damage);
    float GetHealth() const { return m_Health; }
    float GetMaxHealth() const { return m_MaxHealth; }
    bool IsDead() const { return m_Health <= 0.0f; }

    // Stateî•ñæ“¾
    const char* GetCurrentStateName() const;

    // HPİ’è
    void SetHealth(float health);
};