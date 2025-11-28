#include "enemy.h"
#include "manager.h"
#include "modelRenderer.h"
#include "camera.h"
#include "input.h"
#include"enemyState.h"


void Enemy::Init()
{
    m_AnimationModel = make_unique<AnimationModel>();
    m_AnimationModel->Load("asset\\model\\Enemy\\FBX\\character-skeleton.fbx");
    m_AnimationModel->LoadAllAnimations("asset\\model\\Enemy\\FBX\\character-skeleton.fbx");

    SetShaderType(ShaderType::UNLIT_TEXTURE);


    std::vector<std::string> animNames = m_AnimationModel->GetAnimationNames();
    if (!animNames.empty())
    {
        m_AnimationModel->Play(animNames[0].c_str(), true);
        m_AnimationModel->Update();  // 初回更新で頂点を変換
    }


    //破壊用のモデルのロード
    LoadModel("asset\\model\\Enemy\\OBJ\\character-skeleton.obj");

    // オプション1: Boxコライダー（動的可能、倒れる）
    m_UseTriangleMesh = false;
    SetMass(50.0f);

    // 破壊設定
    SetDestructible(true);
    SetDestructionThreshold(15.0f);
    SetGroupSize(5);
    SetExplosionForce(0.0f);

    m_AutoBoxHalfSize = Vector3(0.3f, 0.9f, 0.3f); // キャラクターのサイズに合わせて調整
    m_ColliderOffset = Vector3(0, 0.9f, 0); // 足元を基準にする場合

    m_Started = false;
    SetTag(GameObjectTag::Enemy);
    SetName("TreeBlock");

    // State初期化
    m_CurrentState = nullptr;
    m_MaxHealth = 100.0f;
    m_Health = m_MaxHealth;

   
}

void Enemy::Start()
{
    if (m_RigidBody) return;
    if (!m_AnimationModel) return;

    SetupCollisionLayer();

    // FBXからバウンディングボックスを計算
    m_AutoBoxHalfSize = CalculateAnimationModelBounds(m_AnimationModel.get());
    m_ColliderOffset = Vector3(0, m_AutoBoxHalfSize.y, 0);

    CreateBoxCollider(m_AutoBoxHalfSize, m_mass);
    RecreateCollider();

    // 初期状態を設定
    m_CurrentState = new EnemyIdleState(this);
    m_CurrentState->Enter();
}

void Enemy::Uninit()
{
    // State削除
    if (m_CurrentState)
    {
        m_CurrentState->Exit();
        delete m_CurrentState;
        m_CurrentState = nullptr;
    }

    m_AnimationModel->Uninit();

}

void Enemy::Update()
{
    CheckAndCallStart();
    if (m_Started)
    {
        m_AnimationModel->Update();

        // State更新
        if (m_CurrentState)
        {
            m_CurrentState->Update();
        }

        if (Input::GetKeyTrigger(KK_G))
        {
            DestroyObject(Vector3(), 10.f);
        }
    }

}

void Enemy::Draw()
{
 

    XMMATRIX S_p = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
    XMMATRIX R_p = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
    XMMATRIX T_p = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
    XMMATRIX parentWorld = S_p * R_p * T_p;
    // ワールド行列を設定
    Renderer::SetWorldMatrix(
        UpdatePhysicsWithModel()
    );

    m_AnimationModel->Draw();

}

AnimationModel* Enemy::GetAnimationModel()
{
    return m_AnimationModel.get();
}

void Enemy::ChangeState(EnemyState* newState)
{
    if (m_CurrentState)
    {
        m_CurrentState->Exit();
        delete m_CurrentState;
    }
    m_CurrentState = newState;
    if (m_CurrentState)
    {
        m_CurrentState->Enter();
    }
}

void Enemy::Damage(float damage)
{
    if (IsDead()) return;

    m_Health -= damage;
    if (m_Health <= 0.0f)
    {
        m_Health = 0.0f;
        ChangeState(new EnemyDeadState(this));
    }
}

const char* Enemy::GetCurrentStateName() const
{
    if (!m_CurrentState)
        return "None";
    return m_CurrentState->GetStateName();
}

void Enemy::SetHealth(float health)
{
    m_Health = std::max(0.0f, std::min(health, m_MaxHealth));

    if (m_Health <= 0.0f && m_CurrentState)
    {
        if (strcmp(m_CurrentState->GetStateName(), "Dead") != 0)
        {
            ChangeState(new EnemyDeadState(this));
        }
    }
}


void Enemy::OnCollisionEnter(GameObject* other, const Vector3& hitPoint)
{
    // 既に破壊されている、または破壊不可能な場合は何もしない
    if (m_IsDestroyed || !m_Destructible) return;
  

    DestroyObject(hitPoint, 10.0f);
}


