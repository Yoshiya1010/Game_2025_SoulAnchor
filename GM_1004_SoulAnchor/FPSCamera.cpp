#include	"FPSCamera.h"
#include	"Manager.h"
#include	"FPSPlayer.h"
#include	"input.h"
#include	"scene.h"
#include <algorithm> 
#include"main.h"

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

	g_hWnd = GetWindow();
	
}

void	FPSCamera::Uninit()
{

}

void	FPSCamera::Update()
{


    FPSPlayer* player = Manager::GetScene()->GetGameObject<FPSPlayer>();
    if (!player) return;

    bool fpsMode = Scene::GetCameraModeFlag();
    static bool prevFpsMode = false;  // 前フレームの状態を覚えておく

    //状態遷移の処理 
    if (fpsMode && !prevFpsMode)
    {
       

        // カーソルを確実に非表示にする
        while (ShowCursor(FALSE) >= 0) {}   // カウンタが -1 になるまで減らす

        // ウィンドウ矩形 → スクリーン座標に変換
        RECT rc;
        GetClientRect(g_hWnd, &rc);
        POINT lt{ rc.left, rc.top };
        POINT rb{ rc.right, rc.bottom };
        ClientToScreen(g_hWnd, &lt);
        ClientToScreen(g_hWnd, &rb);

        // このウィンドウ内にマウスを閉じ込める
        RECT clip{ lt.x, lt.y, rb.x, rb.y };
        ClipCursor(&clip);

        // マウスを中央へ
        POINT center{
            (lt.x + rb.x) / 2,
            (lt.y + rb.y) / 2
        };
        SetCursorPos(center.x, center.y);
    }
    else if (!fpsMode && prevFpsMode)
    {
        //FPSモードを抜けた時

        ClipCursor(nullptr);// 拘束解除

        //カーソルを確実に表示にする
        while (ShowCursor(TRUE) < 0) {}//カウンターを表示する
    }

    //FPSモード
    if (fpsMode)
    {
        // ウィンドウ矩形 → スクリーン座標に変換
        RECT rc;
        GetClientRect(g_hWnd, &rc);
        POINT lt{ rc.left, rc.top };
        POINT rb{ rc.right, rc.bottom };
        ClientToScreen(g_hWnd, &lt);
        ClientToScreen(g_hWnd, &rb);

        POINT center{
            (lt.x + rb.x) / 2,
            (lt.y + rb.y) / 2
        };

        // 現在位置を取得
        POINT mousePos;
        GetCursorPos(&mousePos);

        // 中央からの差分を移動量として使う
        float dx = static_cast<float>(mousePos.x - center.x);
        float dy = static_cast<float>(mousePos.y - center.y);

        // 毎フレーム中央に戻す
        SetCursorPos(center.x, center.y);

        // カメラ回転
        const float mouseSensitivity = 0.003f;
        m_Rotation.y += dx * mouseSensitivity;  // yaw
        m_Rotation.x -= dy * mouseSensitivity;  // pitch

        float minPitch = -XM_PIDIV2 + 0.1f;
        float maxPitch = XM_PIDIV2 - 0.1f;
        m_Rotation.x = std::max(minPitch, std::min(m_Rotation.x, maxPitch));

        // カメラ位置
        Vector3 playerPos = player->GetPosition();
        m_Position = playerPos + Vector3(0.0f, 1.8f, 0.0f); // 頭の高さ

        // 向きベクトル
        m_Forward.x = sinf(m_Rotation.y) * cosf(m_Rotation.x);
        m_Forward.y = sinf(m_Rotation.x);
        m_Forward.z = cosf(m_Rotation.y) * cosf(m_Rotation.x);
        m_AtPosition = m_Position + m_Forward;

        // ビュー行列設定
        m_ViewMatrix = XMMatrixLookAtLH(
            XMLoadFloat3((XMFLOAT3*)&m_Position),
            XMLoadFloat3((XMFLOAT3*)&m_AtPosition),
            XMLoadFloat3((XMFLOAT3*)&m_UpVector)
        );
        Renderer::SetViewMatrix(m_ViewMatrix);
    }

    // 最後に状態を保存
    prevFpsMode = fpsMode;

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

//視錐台カリング
bool FPSCamera::CheckView(Vector3 position,float Size)
{
    XMMATRIX vp;
    vp = m_ViewMatrix * m_ProjectionMatrix;

    XMMATRIX invVp;
    invVp = XMMatrixInverse(nullptr, vp);

    XMFLOAT3 vpos[4];
    vpos[0] = XMFLOAT3(-1.0f, 1.0f, 1.0f);
    vpos[1] = XMFLOAT3(1.0f, 1.0f, 1.0f);
    vpos[2] = XMFLOAT3(-1.0f, -1.0f, 1.0f);
    vpos[3] = XMFLOAT3(1.0f, -1.0f, 1.0f);


    XMVECTOR vposv[4];
    vposv[0] = XMLoadFloat3(&vpos[0]);
    vposv[1] = XMLoadFloat3(&vpos[1]);
    vposv[2] = XMLoadFloat3(&vpos[2]);
    vposv[3] = XMLoadFloat3(&vpos[3]);
    XMVECTOR wposv[4];
    wposv[0] = XMVector3TransformCoord(vposv[0], invVp);
    wposv[1] = XMVector3TransformCoord(vposv[1], invVp);
    wposv[2] = XMVector3TransformCoord(vposv[2], invVp);
    wposv[3] = XMVector3TransformCoord(vposv[3], invVp);

    XMFLOAT3 wpos[4];
    XMStoreFloat3(&wpos[0], wposv[0]);
    XMStoreFloat3(&wpos[1], wposv[1]);
    XMStoreFloat3(&wpos[2], wposv[2]);
    XMStoreFloat3(&wpos[3], wposv[3]);


    Vector3 v;
    v = position - m_Position;

    Vector3 wp[4];

    wp[0] = Vector3(wpos[0].x, wpos[0].y, wpos[0].z);
    wp[1] = Vector3(wpos[1].x, wpos[1].y, wpos[1].z);
    wp[2] = Vector3(wpos[2].x, wpos[2].y, wpos[2].z);
    wp[3] = Vector3(wpos[3].x, wpos[3].y, wpos[3].z);


    //左面
    {
        Vector3 v1, v2;
        v1 = wp[0] - m_Position;
        v2 = wp[2] - m_Position;


        Vector3 n;
        n = Vector3::Cross(v1, v2);

        float d;
        d = Vector3::Dot(n, v);//面からの距離

        if (d < Size)
            return false;

    }


    // 右面 
    {
        Vector3 v1 = wp[3] - m_Position; // 右下
        Vector3 v2 = wp[1] - m_Position; // 右上
        Vector3 n = Vector3::Cross(v1, v2);
        float d;
        d = Vector3::Dot(n, v);//面からの距離

        if (d < Size)
            return false;
    }

    // 上面 
    {
        Vector3 v1 = wp[1] - m_Position; // 右上
        Vector3 v2 = wp[0] - m_Position; // 左上
        Vector3 n = Vector3::Cross(v1, v2);
        float d;
        d = Vector3::Dot(n, v);//面からの距離

        if (d < Size)
            return false;
    }

    // 下面 
    {
        Vector3 v1 = wp[2] - m_Position; // 左下
        Vector3 v2 = wp[3] - m_Position; // 右下
        Vector3 n = Vector3::Cross(v1, v2);
        float d;
        d = Vector3::Dot(n, v);//面からの距離

        if (d < Size)
            return false;
    }

    return true;
}



