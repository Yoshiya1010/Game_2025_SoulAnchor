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



void DrawImguiWindow()
{


	// デモウィンドウ（テスト用）
	static bool showDemo = true;
	static bool showSceneHierarchy = false;
	static bool showProperties = false;
	static bool showDebugWindow = false;
	if (showDemo) {
	
		
		
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
				ImGui::MenuItem(" Window", nullptr, &showDemo);
				ImGui::MenuItem("Scene Hierarchy", nullptr, &showSceneHierarchy);
				ImGui::MenuItem("Properties", nullptr, &showProperties);
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();

		}

		//Windowsなどを追加していく
		CreateObjectWindow();
		
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
		EditorObjectCreator::CreateGroundBlock(pos, 100.0f);
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