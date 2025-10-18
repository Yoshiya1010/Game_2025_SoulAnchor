#include	"camera.h"
#include	"Manager.h"
#include	"player.h"
#include	"input.h"
#include	"scene.h"

void	Camera::Init()
{
	//カメラの初期化
	m_Position = Vector3(0.0f, 3.0f, -8.0f);	//カメラ基本座標
	m_UpVector = Vector3(0.0f, 1.0f, 0.0f);		//カメラの上方ベクトル
	m_AtPosition = Vector3(0.0f, 0.0f, 0.0f);	//カメラの注視点座標
	m_Fov = 45.0f;								//画角
	m_nearClip = 1.0f;							//近面クリップ
	m_farClip = 1000.0f;						//遠面クリップ
	m_rot = 0.0f;

	m_playerOffset = Vector3( 0.0f,3.0f,-10.0f);

	// カメラの距離とパラメータ
	m_cameraDistance = 7.0f;
	m_cameraHeight = 2.0f;
	m_sensitivity = 2.0f;  // スティック感度

	SetName("Camera");
}

void	Camera::Uninit()
{

}

void	Camera::Update()
{
	
		// キーボード入力（デバッグ用）
		if (Input::GetKeyPress(KK_RIGHT)) {
			m_Rotation.y += 0.1f;
		}
		if (Input::GetKeyPress(KK_LEFT)) {
			m_Rotation.y -= 0.1f;
		}

		// マウスでカメラ操作モード切替（右クリック）
		bool isRightClick = (Input::GetKeyPress(KK_K));
		static bool prevRightClick = false;
		static bool isCameraControl = false;

		if (isRightClick && !prevRightClick) {
			isCameraControl = !isCameraControl;
			ShowCursor(!isCameraControl);  // カメラ操作中はカーソルを非表示
		}
		prevRightClick = isRightClick;

		// マウス入力処理
		if (isCameraControl) {
			POINT mousePos;
			GetCursorPos(&mousePos);

			static bool firstFrame = true;
			static POINT prevMousePos = { 0, 0 };

			if (firstFrame) {
				prevMousePos = mousePos;
				firstFrame = false;
			}
			else {
				float dx = float(mousePos.x - prevMousePos.x);
				float dy = float(mousePos.y - prevMousePos.y);

				// カメラ回転（マウス感度）
				const float mouseSensitivity = 0.003f;
				m_Rotation.y += dx * mouseSensitivity;
				m_Rotation.x += dy * mouseSensitivity;
			}

			prevMousePos = mousePos;
		}

		// Xboxコントローラーの右スティック入力
		float rightStickX = Input::GetControllerAxisValue(0, CONTROLLER_AXIS_RX);
		float rightStickY = Input::GetControllerAxisValue(0, CONTROLLER_AXIS_RY);

		// デッドゾーン処理
		const float deadZone = 0.2f;
		if (abs(rightStickX) > deadZone) {
			m_Rotation.y += rightStickX * m_sensitivity * 0.02f;
		}
		if (abs(rightStickY) > deadZone) {
			m_Rotation.x += rightStickY * m_sensitivity * 0.02f;
		}

		// 縦方向の回転制限（上下を見すぎないように）
		m_Rotation.x = std::max(-XM_PIDIV2 + 0.1f, std::min(XM_PIDIV2 - 0.1f, m_Rotation.x));

		Player* player = Manager::GetScene()->GetGameObject<Player>();

		// プレイヤーの少し上を注視点に
		m_AtPosition = player->GetPosition() + Vector3(0.0f, m_cameraHeight, 0.0f);

		// カメラの位置計算（プレイヤーを中心に回転）
		float camX = -sinf(m_Rotation.y) * cosf(m_Rotation.x) * m_cameraDistance;
		float camY = sinf(m_Rotation.x) * m_cameraDistance + m_cameraHeight;
		float camZ = -cosf(m_Rotation.y) * cosf(m_Rotation.x) * m_cameraDistance;

		m_Position = m_AtPosition + Vector3(camX, camY, camZ);

	
}

void	Camera::Draw()//3D使用時
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