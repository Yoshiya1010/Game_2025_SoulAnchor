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

ImGuiManager Game::m_ImGuiManager;

GameObject* Game::selectedObject = nullptr; //現在選択しているオブジェクト

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
	AddGameObject<Camera>(SYSTEM);
	//AddGameObject<MeshField>(FIELD);
	AddGameObject<Player>(OBJECT)->SetPosition({ -2.0f,0.0f,3.0f });
	//AddGameObject<Enemy>(OBJECT)->SetPosition({ -2.0f,0.0f,3.0f });
	//AddGameObject<Enemy>(OBJECT)->SetPosition({ 0.0f,0.0f,3.0f });
	/*AddGameObject<Enemy>(OBJECT)->SetPosition({ 2.0f,0.0f,3.0f });*/

	/*AddGameObject<GroundBlock>(OBJECT);*/

	AddGameObject<Wave>(FIELD)->SetPosition({ 0.0f,0.5f,0.0f });
	AddGameObject<Score>(UI)->SetPosition({ 50.0f,50.0f,0.0f });
	
	

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

	// デモウィンドウ（テスト用）
	static bool showDemo = true;
	static bool showSceneHierarchy = false;
	static bool showProperties = false;
	static bool showDebugWindow = false;
	if (showDemo) {
		ImGui::ShowDemoWindow(&showDemo);

		// メインメニューバー
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New Level")) {
					// 新規レベル作成（後で実装）
				}
				if (ImGui::MenuItem("Open Level...")) {
					// レベル読み込み（後で実装）
				}
				if (ImGui::MenuItem("Save Level")) {
					// レベル保存（後で実装）
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Exit")) {
					PostQuitMessage(0);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View")) {
				ImGui::MenuItem("aaaaaaaaaaaaa Window", nullptr, &showDemo);
				ImGui::MenuItem("Scene Hierarchy", nullptr, &showSceneHierarchy);
				ImGui::MenuItem("Properties", nullptr, &showProperties);
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}
	// 基本エディターウィンドウ
	if (showSceneHierarchy) {
		ShowSceneHierarchy();
	}

	//if (showProperties) {
	//	ShowPropertiesPanel();
	//}


	//if (showDebugWindow) {
	//	ShowDebugSettings();
	//}


	// ImGui描画（3Dシーンの上にオーバーレイ）
	m_ImGuiManager.EndFrame();
}

void Game::Uninit()
{
	m_BGM->Uninit();
	delete m_BGM;

	Scene::Uninit();

	// ImGui終了処理
	m_ImGuiManager.Shutdown();
}


void Game::ShowSceneHierarchy()
{
	ImGui::Begin("Scene Hierarchy");

	// 既存のオブジェクト表示コード...（前回と同じ）

	ImGui::Separator();
	ImGui::Text("Create Objects:");

	if (ImGui::Button("Add Ground")) {
		Vector3 pos = EditorObjectCreator::GetSafeSpawnPosition();
		selectedObject = EditorObjectCreator::CreateGroundBlock(pos, 100.0f);
	}
	if (ImGui::Button("Add Cube")) {
		Vector3 pos = EditorObjectCreator::GetSafeSpawnPosition();
		selectedObject = EditorObjectCreator::CreateCube(pos, 1.0f);
	}
	ImGui::SameLine();
	if (ImGui::Button("Add Sphere")) {
		Vector3 pos = EditorObjectCreator::GetSafeSpawnPosition();
		selectedObject = EditorObjectCreator::CreateSphere(pos, 1.0f);
	}

	if (ImGui::Button("Static Wall")) {
		Vector3 pos = EditorObjectCreator::GetSafeSpawnPosition();
		selectedObject = EditorObjectCreator::CreateStaticCube(pos, Vector3(2, 1, 0.2f));
	}
	ImGui::SameLine();
	if (ImGui::Button("Heavy Box")) {
		Vector3 pos = EditorObjectCreator::GetSafeSpawnPosition();
		selectedObject = EditorObjectCreator::CreateDynamicCube(pos, Vector3(1, 1, 1), 10.0f);
	}

	ImGui::End();
}