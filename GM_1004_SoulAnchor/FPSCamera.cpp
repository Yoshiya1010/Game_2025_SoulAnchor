#include	"FPSCamera.h"
#include	"Manager.h"
#include	"FPSPlayer.h"
#include	"input.h"
#include	"scene.h"
#include <algorithm> 

void	FPSCamera::Init()
{
	//カメラの初期化
	m_Position = Vector3(0.0f, 3.0f, -8.0f);	//カメラ基本座標
	m_UpVector = Vector3(0.0f, 1.0f, 0.0f);		//カメラの上方ベクトル
	m_AtPosition = Vector3(0.0f, 0.0f, 0.0f);	//カメラの注視点座標
	m_Fov = 45.0f;								//画角
	m_nearClip = 1.0f;							//近面クリップ
	m_farClip = 1000.0f;						//遠面クリップ
	m_rot = 0.0f;

	m_playerOffset = Vector3(0.0f, 3.0f, -10.0f);

	// カメラの距離とパラメータ
	m_cameraDistance = 7.0f;
	m_cameraHeight = 2.0f;
	m_sensitivity = 2.0f;  // スティック感度

	SetName("FPSCamera");

	
}

void	FPSCamera::Uninit()
{

}

void	FPSCamera::Update()
{
	// 右クリックでマウスロック切り替え
	static bool mouseLocked = false;

	FPSPlayer* player = Manager::GetScene()->GetGameObject<FPSPlayer>();
	if (!player) return;

	if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
		mouseLocked = !mouseLocked;
		
	}

	if (mouseLocked)  // 右クリック押下判定
	{
		//カメラ操作（マウスによる視点回転
		POINT mousePos;
		GetCursorPos(&mousePos);
		static POINT prevMousePos = mousePos;

		float dx = (float)(mousePos.x - prevMousePos.x);
		float dy = (float)(mousePos.y - prevMousePos.y);
		prevMousePos = mousePos;

		const float mouseSensitivity = 0.003f;
		m_Rotation.y += dx * mouseSensitivity;
		m_Rotation.x -= dy * mouseSensitivity;

		float minPitch = -XM_PIDIV2 + 0.1f;
		float maxPitch = XM_PIDIV2 - 0.1f;

		// 上下の回転制限
		m_Rotation.x = std::max(minPitch, std::min(m_Rotation.x, maxPitch));

		// カメラの位置をプレイヤーの頭に合わせる
		Vector3 playerPos = player->GetPosition();
		m_Position = playerPos + Vector3(0.0f, 1.8f, 0.0f); // 頭の高さ

		//カメラの注視点を向いている方向に設定
		Vector3 forward;
		forward.x = sinf(m_Rotation.y) * cosf(m_Rotation.x);
		forward.y = sinf(m_Rotation.x);
		forward.z = cosf(m_Rotation.y) * cosf(m_Rotation.x);

		m_AtPosition = m_Position + forward;

		// View行列・Projection行列設定
		m_ViewMatrix = XMMatrixLookAtLH(XMLoadFloat3((XMFLOAT3*)&m_Position),
			XMLoadFloat3((XMFLOAT3*)&m_AtPosition),
			XMLoadFloat3((XMFLOAT3*)&m_UpVector));
		Renderer::SetViewMatrix(m_ViewMatrix);
	}
}

void	FPSCamera::Draw()//3D使用時
{
	//プロジェクション行列を作成
	m_ProjectionMatrix =
		XMMatrixPerspectiveFovLH(
			XMConvertToRadians(m_Fov),
			(float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
			m_nearClip,
			m_farClip
		);

	//プロジェクション行列をセット
	Renderer::SetProjectionMatrix(m_ProjectionMatrix);

	//カメラ行列を作成
	XMVECTOR	eyev = XMLoadFloat3((XMFLOAT3*)&m_AtPosition);
	XMVECTOR	pos = XMLoadFloat3((XMFLOAT3*)&m_Position);
	XMVECTOR	up = XMLoadFloat3((XMFLOAT3*)&m_UpVector);
	m_ViewMatrix = XMMatrixLookAtLH(pos, eyev, up);

	//カメラ行列をセット
	Renderer::SetViewMatrix(m_ViewMatrix);
}