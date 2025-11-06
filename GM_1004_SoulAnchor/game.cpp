#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "audio.h"
#include "game.h"
#include "camera.h"
#include "field.h"
#include "player.h"
#include "enemy.h"
#include "Polygon.h"
#include "title.h"
#include "input.h"
#include "result.h"
#include "score.h"
#include "meshField.h"
#include "wave.h"
#include"GroundBlock.h"
#include"imgui.h"
#include"PhysicsCollisionManager.h"
#include"PhysicsManager.h"
#include"EditorObjectCreator.h"
#include"DrawImgui.h"
#include"FPSCamera.h"
#include"FPSPlayer.h"
#include"SkyBox.h"
#include "DebugImguiWindow.h"

ImGuiManager Game::m_ImGuiManager;



void Game::Init()
{
	// 物理世界の初期化
	PhysicsManager::Init();

	////デバック用のワイヤーフレーム
	//PhysicsDebugRenderer::Init();

	//衝突管理マネージャーの初期化
	PhysicsCollisionManager::Init();

	// audioのロード
	m_BGM = new Audio();
	m_BGM->Load("asset\\audio\\bgm.wav");
	m_BGM->Play(true); // 曲を再生

	/// カメラを最初に入れる
	AddGameObject<FPSCamera>(SYSTEM);
	AddGameObject<FPSPlayer>(OBJECT)->SetPosition({ -2.0f,0.0f,3.0f });
	AddGameObject<Score>(UI)->SetPosition({ 50.0f,50.0f,0.0f });

	AddGameObject<SkyBox>(OBJECT);
	
	//初期値のシーンをロード　今はデバック用をいれるけど
	Scene* scene = Manager::GetScene();
	if (scene)
	{
		scene->LoadScene("TestScene.json");
	}
	
	// ImGui初期化（Renderer::Init()の後）
	HWND hwnd = GetWindow();
	ID3D11Device* device = Renderer::GetDevice();
	ID3D11DeviceContext* context = Renderer::GetDeviceContext();
	if (!m_ImGuiManager.Initialize(hwnd, device, context)) {
		MessageBox(hwnd, TEXT("ImGui initialization failed!"), TEXT("Error"), MB_OK);
	}

}

void Game::Update()
{
	Scene::Update();

	//if (Scene::GetGameObject<Enemy>() == nullptr) {
	//	Manager::SetScene<Result>();
	//}

	if (Scene::IsPaused()==true) return;
	// 物理世界の更新
	PhysicsManager::Update();

	//衝突検知を毎フレーム実行
	PhysicsCollisionManager::GetInstance()->CheckCollisions();
}

void Game::DrawImgui()
{

	PhysicsManager::DrawDebugWorld();

	// ImGuiフレーム開始
	m_ImGuiManager.BeginFrame();
	DrawImguiWindow();

	DebugImguiWindow::Draw();
	// ImGui描画（3Dシーンの上にオーバーレイ）
	m_ImGuiManager.EndFrame();

}

void Game::Uninit()
{
	


	m_BGM->Uninit();
	delete m_BGM;

	Scene::Uninit();

	// 物理世界の終了処理
	PhysicsManager::Uninit();

	//衝突管理マネージャーの終了処理
	PhysicsCollisionManager::Uninit();

	// ImGui終了処理
	m_ImGuiManager.Shutdown();
}


