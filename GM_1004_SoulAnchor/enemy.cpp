#include "enemy.h"
#include "manager.h"
#include "modelRenderer.h"
#include "camera.h"
#include "input.h"


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
    SetTag(GameObjectTag::Ground);
    SetName("TreeBlock");

   
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
}

void Enemy::Uninit()
{
    m_AnimationModel->Uninit();

}

void Enemy::Update()
{
    CheckAndCallStart();
    if (m_Started)
    {
        m_AnimationModel->Update();


        if (Input::GetKeyTrigger(KK_G))
        {
            DestroyObject(Vector3(),10.f);
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