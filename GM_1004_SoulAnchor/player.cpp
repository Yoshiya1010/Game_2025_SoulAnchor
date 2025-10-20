#include "player.h"
#include "manager.h"
#include "modelRenderer.h"
#include "audio.h"

#include "camera.h"
#include "input.h"
#include "bullet.h"
#include "scene.h"
#include "animationModel.h"

#include "meshField.h"

void Player::Init()
{
    m_ModelRenderer = make_unique<ModelRenderer>();
    m_ModelRenderer->Load("asset\\model\\player.obj");

    // モデルのロード
    m_AnimationModel = make_unique<AnimationModel>();
    m_AnimationModel->Load("asset\\model\\Akai.fbx");

    // モデルのアニメーションをロード
    m_AnimationModel->LoadAnimation("asset\\model\\Akai_Run.fbx", "Run");
    m_AnimationModel->LoadAnimation("asset\\model\\Akai_Idle.fbx", "Idle");

    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");

    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");

    // Audioの初期化
    m_SE = make_unique<Audio>();
    m_SE->Load("asset\\audio\\wan.wav");

    m_Scale = Vector3(0.015f, 0.015f, 0.015f);

    m_Frame = 0;

    m_AnimationName = "Idle";
    m_AnimationNameNext = "Run";
    m_AnimationBlend = 0.0f;


    

    SetName("Player");
}

void Player::Uninit()
{
    m_SE->Uninit();
    m_SE.reset();

    //delete m_ModelRenderer;
    m_AnimationModel->Uninit();
    m_AnimationModel.reset();

    if (m_VertexLayout)     m_VertexLayout->Release();
    if (m_VertexShader)     m_VertexShader->Release();
    if (m_PixelShader)      m_PixelShader->Release();
}

void Player::Update()
{
    Camera* camera = Manager::GetScene()->GetGameObject<Camera>();
    
    Vector3 moveDirection = Vector3(0.0f, 0.0f, 0.0f);
    bool isMoving = false;

    if (Input::GetKeyPress(KK_A)) {
        m_Position -= camera->GetRight() * 0.1f;
        m_Rotation.y = camera->GetRotation().y - XM_PIDIV2;
        isMoving = true;
    }

    if (Input::GetKeyPress(KK_D)) {
        m_Position += camera->GetRight() * 0.1f;
        m_Rotation.y = camera->GetRotation().y + XM_PIDIV2;
        isMoving = true;
    }

    if (Input::GetKeyPress(KK_W)) {
        m_Position += camera->GetForward() * 0.1f;
        m_Rotation.y = camera->GetRotation().y;
        isMoving = true;
    }

    if (Input::GetKeyPress(KK_S)) {
        m_Position -= camera->GetForward() * 0.1f;
        m_Rotation.y = camera->GetRotation().y + XM_PI;
        isMoving = true;
    }

    // Xboxコントローラーの左スティック入力
    float leftStickX = Input::GetControllerAxisValue(0, CONTROLLER_AXIS_LX);
    float leftStickY = Input::GetControllerAxisValue(0, CONTROLLER_AXIS_LY);

    // デッドゾーン処理
    const float deadZone = 0.2f;
    if (abs(leftStickX) > deadZone || abs(leftStickY) > deadZone) {
        // スティックの入力を正規化
        float magnitude = sqrt(leftStickX * leftStickX + leftStickY * leftStickY);
        if (magnitude > 1.0f) {
            leftStickX /= magnitude;
            leftStickY /= magnitude;
        }

        // カメラの向きに合わせて移動方向を計算（Y軸は固定）
        Vector3 stickDirection = camera->GetForward() * leftStickY + camera->GetRight() * leftStickX;
        stickDirection.y = 0.0f; // Y軸の移動を無効化（水平移動のみ）
        moveDirection += stickDirection;
        isMoving = true;
	}

	// 移動処理
	if (isMoving) {
		// 移動方向を正規化（XZ平面のみ）
		float length = sqrt(moveDirection.x * moveDirection.x + moveDirection.z * moveDirection.z);
		if (length > 0.0f) {
			moveDirection.x /= length;
			moveDirection.z /= length;
			moveDirection.y = 0.0f; // Y軸の移動を確実に無効化

			// プレイヤーの位置を更新（Y座標は変更しない）
			m_Position.x += moveDirection.x * 0.1f;
			m_Position.z += moveDirection.z * 0.1f;

			// プレイヤーの向きを移動方向に合わせる
			m_Rotation.y = atan2f(moveDirection.x, moveDirection.z);
		}

        // Runアニメーションにする
        if (m_AnimationNameNext != "Run") {
            m_AnimationName = m_AnimationNameNext;
            m_AnimationNameNext = "Run";
            m_AnimationBlend = 0.0f;
        }
    }
    else {
        // Idleアニメーションにする
        if (m_AnimationNameNext != "Idle") {
            m_AnimationName = m_AnimationNameNext;
            m_AnimationNameNext = "Idle";
            m_AnimationBlend = 0.0f;
        }
    }

    MeshField* meshField = Manager::GetScene()->GetGameObject<MeshField>();
    if(meshField)
    m_Position.y = meshField->GetHeight(m_Position);


	// 弾を発射する
	if (Input::GetKeyTrigger(KK_SPACE)) {
		Bullet* bullet = Manager::GetScene()->AddGameObject<Bullet>(OBJECT);
		bullet->SetPosition(m_Position);
		bullet->SetVelocity(GetForward() * 0.5f);

		// SEを再生する
		m_SE->Play();
	}

    // m_AnimationModel->Update(m_AnimationName.c_str(), m_Frame);
    m_AnimationModel->Update(m_AnimationName.c_str(), m_Frame, m_AnimationNameNext.c_str(), m_Frame, m_AnimationBlend);
    m_Frame++;

    m_AnimationBlend += 0.1f;
    if (m_AnimationBlend > 1.0f) {
        m_AnimationBlend = 1.0f;
    }


   
}

void Player::Draw()
{
 
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);
    
    {
        XMMATRIX S_p = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
        XMMATRIX R_p = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
        XMMATRIX T_p = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
        XMMATRIX parentWorld = S_p * R_p * T_p;

        Renderer::SetWorldMatrix(parentWorld);
        m_AnimationModel->Draw();

        // 子のローカル変換（親からの相対だけを入れる）
        Vector3 childLocalPos = { 0.0f, 0.0f, 0.0f };   // 例：親の前方50cm
        Vector3 childLocalRot = { 0.0f, 0.0f, 0.0f };   // 例：回転なし
        Vector3 childLocalScale = { m_Scale.x * 1000.0f, m_Scale.y * 1000.0f, m_Scale.z * 1000.0f }; // ←行列に*100はしない！

        XMMATRIX S_c = XMMatrixScaling(childLocalScale.x, childLocalScale.y, childLocalScale.z);
        XMMATRIX R_c = XMMatrixRotationRollPitchYaw(childLocalRot.x, childLocalRot.y, childLocalRot.z);
        XMMATRIX T_c = XMMatrixTranslation(childLocalPos.x, childLocalPos.y, childLocalPos.z);

        // 子の最終ワールド： 子ローカル * 親ワールド
        XMMATRIX childWorld = S_c * R_c * T_c * parentWorld;

        Renderer::SetWorldMatrix(childWorld);
        m_ModelRenderer->Draw();

      
    }
}