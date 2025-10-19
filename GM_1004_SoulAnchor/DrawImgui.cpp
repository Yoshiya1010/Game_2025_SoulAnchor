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
#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "audio.h"
#include"DrawImgui.h"
#include"scene.h"
#include <iostream>

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

// グローバルまたは静的変数として保持
static GameObject* selectedObject = nullptr;

void DrawImguiWindow()
{

	// デモウィンドウ（テスト用）
	static bool showDemo = true;


	static bool showDebugWindow = false;


	//シーンのヒエラルキーの表示
	static bool showSceneHierarchyWindowFlag = true;
	
	static bool showPropertiesWindowFlag = true;

	//セーブのウィンドウ
	static bool saveSceneWindowFlag = false;
	//ロードのウィンドウ
	static bool loadSceneWindowFlag = false;
	if (showDemo) {
	
		
		
		// メインメニューバー
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
			
				if (ImGui::MenuItem("Save Scene", nullptr, &saveSceneWindowFlag));

				if (ImGui::MenuItem("Load Scene", nullptr, &loadSceneWindowFlag));
				
				
			
					
				ImGui::Separator();
				if (ImGui::MenuItem("Exit")) {
					PostQuitMessage(0);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View")) {
				ImGui::MenuItem(" Window", nullptr, &showDemo);
				ImGui::MenuItem("Scene Hierarchy", nullptr, &showSceneHierarchyWindowFlag);
				ImGui::MenuItem("Properties", nullptr, &showPropertiesWindowFlag);

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();

		}

		//Windowsなどを追加していく

		//オブジェクトの作成のWindow
		CreateObjectWindow();

		if (saveSceneWindowFlag)
		{
			SaveSceneWindow();
		}

		if (loadSceneWindowFlag)
		{
			LoadSceneWindow();
		}


		if (showSceneHierarchyWindowFlag)
		{
			ShowSceneHierarchyWindow();
		}

		if (showPropertiesWindowFlag)
		{
			ShowPropertiesWindow();
		}

	}
	


	
}


void CreateObjectWindow()
{
	ImGui::Begin("AddCreateObject");

	// 既存のオブジェクト表示コード...（前回と同じ）

	ImGui::Separator();
	ImGui::Text("Create Objects:");

	if (ImGui::Button("Add Ground")) {
		Vector3 pos = EditorObjectCreator::GetSafeSpawnPosition();
		EditorObjectCreator::CreateGroundBlock(pos, Vector3(1.0f,1.0f,1.f));
	}
	if (ImGui::Button("Add Cube")) {
		Vector3 pos = EditorObjectCreator::GetSafeSpawnPosition();
		 EditorObjectCreator::CreateCube(pos, 1.0f);
	}
	ImGui::SameLine();
	if (ImGui::Button("Add Sphere")) {
		Vector3 pos = EditorObjectCreator::GetSafeSpawnPosition();
		 EditorObjectCreator::CreateSphere(pos, 1.0f);
	}

	if (ImGui::Button("Static Wall")) {
		Vector3 pos = EditorObjectCreator::GetSafeSpawnPosition();
		EditorObjectCreator::CreateStaticCube(pos, Vector3(2, 1, 0.2f));
	}
	ImGui::SameLine();
	if (ImGui::Button("Heavy Box")) {
		Vector3 pos = EditorObjectCreator::GetSafeSpawnPosition();
		EditorObjectCreator::CreateDynamicCube(pos, Vector3(1, 1, 1), 10.0f);
	}

	ImGui::End();
}

void SaveSceneWindow(void)
{
	ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiCond_Once);

	ImGui::Begin("SaveScne");

	ImGui::Separator();
	ImGui::Text(u8"Sceneにある全てのオブジェクトをセーブします。");

	ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.5f);
	static char str0[128] = "";
	ImGui::InputTextWithHint(".json",u8"保存するファイル名", str0,IM_ARRAYSIZE(str0));
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.2f, 1.0f)); // 通常時
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.7f, 1.0f, 1.0f)); // ホバー時
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.5f, 0.9f, 1.0f)); // クリック時
	if (ImGui::Button("Save")) {
		Scene* scene = Manager::GetScene();
		if (scene) {
			scene->SaveScene(str0);
		}
		strcpy_s(str0, "");//リセット
	}
	
	ImGui::PopStyleColor(3);

	ImGui::PopFont();
	ImGui::End();
}


void LoadSceneWindow(void)
{

	
	static std::string SelectFileName;
	
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.5f);
	if (ImGui::Begin("Scene File Browser"))
	{
		const std::string folder_path = "JsonSaveData/";  // 表示したいフォルダ

		// フォルダ内のファイル一覧をループ
		for (const auto& entry : fs::directory_iterator(folder_path))
		{
			std::string name = entry.path().filename().string();

			
			if (fs::is_directory(entry))
			{
				ImGui::Text(u8"%s", name.c_str());
			}
			else
			{
				if (ImGui::Selectable(name.c_str()))
				{
					SelectFileName = name;
				}
			}
		}
	}

	if (SelectFileName != "")
	{
		ImGui::Separator();
		ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), u8"選択中のファイル");
		ImGui::SameLine();
		ImGui::Text(SelectFileName.c_str());
		ImGui::SameLine();
		if (ImGui::Button("Load"))
		{
			Scene* scene = Manager::GetScene();
			if (scene) {
				scene->LoadScene(SelectFileName);
			}
		}
	}
	
		
	ImGui::PopFont();
	ImGui::End();
}

void ShowSceneHierarchyWindow(void)
{
	ImGui::SetNextWindowSize(ImVec2(500, 1000), ImGuiCond_FirstUseEver);
	ImGui::Begin("Scene Hierarchy");
	

	auto* layers = Scene::GetAllGameObjects();

	// レイヤーごとに区切って表示
	for (int i = 0; i < LAYER_NUM; i++)
	{
		// レイヤーの中身が空ならスキップ
		if (layers[i].empty()) continue;

		// 見出しツリー（折りたたみ式）
		std::string layerName = "Layer " + std::to_string(i);
		if (ImGui::TreeNode(layerName.c_str()))
		{
			for (auto& obj : layers[i])
			{
				// オブジェクト名が空のときにImGuiが落ちるのを防止
				std::string objName = obj->GetName();
				if (objName.empty()) objName = "Unnamed Object";

				// 選択可能な行を描画
				if (ImGui::Selectable(objName.c_str(), selectedObject == obj))
				{
					selectedObject = obj;
				}
			}

			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void ShowPropertiesWindow(void)
{
	if (!selectedObject) return;

	ImGui::Begin("Properties");

	ImGui::Text("Editing: %s", selectedObject->GetName().c_str());
	ImGui::Separator();

	Vector3 pos = selectedObject->GetPosition();
	Vector3 rot = selectedObject->GetRotation();
	Vector3 scale = selectedObject->GetScale();

	bool updated = false;

	if (ImGui::DragFloat3("Position", (float*)&pos, 0.1f))
	{
		selectedObject->SetPosition(pos);
		updated = true;
	}

	if (ImGui::DragFloat3("Rotation", (float*)&rot, 1.0f))
	{
		selectedObject->SetRotation(rot);
		updated = true;
	}

	if (ImGui::DragFloat3("Scale", (float*)&scale, 0.1f,0.0f,100.f))
	{
		selectedObject->SetScale(scale);
		if (auto physics = dynamic_cast<PhysicsObject*>(selectedObject))
			physics->RecreateCollider();
		updated = true;
	}

	// 物理オブジェクトなら質量も編集できる
	if (auto physics = dynamic_cast<PhysicsObject*>(selectedObject))
	{
		float mass = physics->GetMass();
		if (ImGui::DragFloat("Mass", &mass, 0.1f, 0.0f, 100.0f))
		{
			physics->SetMass(mass);
			updated = true;
		}

		//位置やスケールを変更したらBulletにも反映！
		if (updated)
		{
			physics->SyncToPhysics();
		}
	}

	ImGui::End();
}