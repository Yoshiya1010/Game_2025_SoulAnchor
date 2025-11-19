#include "game.h"
#include "camera.h"
#include "field.h"
#include "player.h"
#include "enemy.h"
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
#include"editorUICreator.h"
#include"sun.h"
#include"animationModel.h"

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
// utf8ヘルパーマクロ
#define U8(text) reinterpret_cast<const char*>(u8##text)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

// グローバルまたは静的変数として保持
static GameObject* selectedObject = nullptr;

void CheckObjectSelect(GameObject* obj)
{
	if (selectedObject == obj)
	{
		selectedObject = nullptr;
	}
}


void DrawImguiWindow()
{

	if (Scene::GetDebugFlag())return;


	//オブジェクトを追加するWindowの表示
	static bool showAddGameObjectWindowFlag = true;


	//シーンのヒエラルキーの表示
	static bool showSceneHierarchyWindowFlag = true;
	
	static bool showPropertiesWindowFlag = true;

	//アニメーションシステム制御ウィンドウ
	static bool showAnimationWindowFlag = true;

	//シェーダーの設定の設定
	static bool showShaderWindowFlag = true;

	//セーブのウィンドウ
	static bool saveSceneWindowFlag = false;
	//ロードのウィンドウ
	static bool loadSceneWindowFlag = false;
	
	
		
		
		// メインメニューバー
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
			
				if (ImGui::MenuItem("Save Scene", nullptr, &saveSceneWindowFlag));

				if (ImGui::MenuItem("Load Scene", nullptr, &loadSceneWindowFlag));
				
			
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View")) {
				

				ImGui::MenuItem("Scene Hierarchy", nullptr, &showSceneHierarchyWindowFlag);
				ImGui::MenuItem("Properties", nullptr, &showPropertiesWindowFlag);
				ImGui::MenuItem("ShaderWindow", nullptr, &showShaderWindowFlag);
				ImGui::MenuItem("Animation Control", nullptr, &showAnimationWindowFlag);

				ImGui::MenuItem("Add GameObjectWindow", nullptr, &showAddGameObjectWindowFlag);
			
				ImGui::EndMenu();
			}

			//Bullutのあたり判定の描画
			bool drawHitBoxFlag = Scene::GetDrawHitBoxFlag();
			if (ImGui::Checkbox(U8("あたり判定描画"), &drawHitBoxFlag))
			{
				Scene::SetDrawHitBoxFlag(drawHitBoxFlag);
			};

			ImGui::EndMainMenuBar();

		}
	

		//セーブのWindow
		if (saveSceneWindowFlag)
		{
			SaveSceneWindow();
		}
		//ロードのWindow
		if (loadSceneWindowFlag)
		{
			LoadSceneWindow();
		}

		//ヒエラルキーの表示
		if (showSceneHierarchyWindowFlag)
		{
			ShowSceneHierarchyWindow();
		}

		

		//オブジェクトなどの追加を管理しているwindow
		if (showAddGameObjectWindowFlag)
		{
			CreateAddGameObjectWindow();
		}
	

	

		if (showPropertiesWindowFlag)
		{
			ShowPropertiesWindow();
		}

		
	

	

	}
	


	



void SaveSceneWindow(void)
{
	ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiCond_Once);

	ImGui::Begin("SaveScne");
	Scene* scene = Manager::GetScene();
	ImGui::Separator();
	ImGui::Text(U8("Sceneにある全てのオブジェクトをセーブします。"));

	

	ImGui::Text(U8("現在のファイル: "));
	ImGui::SameLine();
	ImGui::Text(scene->GetLoadSceneName().c_str());
	ImGui::SameLine();
	if(ImGui::Button(U8("上書き保存")))
	{
		scene->SaveScene();
	}


	ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.5f);
	static char str0[128] = "";
	ImGui::InputTextWithHint(".json",U8("保存するファイル名"), str0,IM_ARRAYSIZE(str0));
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.2f, 1.0f)); // 通常時
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.7f, 1.0f, 1.0f)); // ホバー時
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.5f, 0.9f, 1.0f)); // クリック時
	if (ImGui::Button(U8("別名保存"))) {
		
		if (scene) {
			scene->SaveSceneAs(str0);
			selectedObject = nullptr;
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
				ImGui::Text("%s", name.c_str());
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
		ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), U8("選択中のファイル"));
		ImGui::SameLine();
		ImGui::Text(SelectFileName.c_str());
		ImGui::SameLine();
		if (ImGui::Button("Load"))
		{
			Scene* scene = Manager::GetScene();
			if (scene) {
				selectedObject = nullptr;
				scene->LoadScene(SelectFileName);
			}
		}
	}
	
		
	ImGui::PopFont();
	ImGui::End();
}


void CreateObjectTab()
{
	if (ImGui::BeginTabItem("Object"))
	{

		// 既存のオブジェクト表示コード

		ImGui::Separator();
		ImGui::Text("Create Objects:");

		if (ImGui::Button("Add Ground")) {
			Vector3 pos = EditorObjectCreator::GetSafeSpawnPosition();
			selectedObject = EditorObjectCreator::CreateGroundBlock(pos, Vector3(1.0f, 1.0f, 1.f));
		}
		if (ImGui::Button("Add Tree")) {
			Vector3 pos = EditorObjectCreator::GetSafeSpawnPosition();
			selectedObject = EditorObjectCreator::CreateTree(pos, Vector3(1.0f, 1.0f, 1.0f));
		}
		if (ImGui::Button("Add RockTall_A")) {
			Vector3 pos = EditorObjectCreator::GetSafeSpawnPosition();
			selectedObject = EditorObjectCreator::CreateRockTall_A(pos, Vector3(1.0f, 1.0f, 1.0f));
		}

		if (ImGui::Button("Add Sun")) {
			Vector3 pos{ 50.f,100.f,50.f };
			selectedObject = EditorObjectCreator::CreateSun(pos, Vector3(1.0f, 1.0f, 1.0f));
		}

		if (ImGui::Button("Add Grass"))
		{
			EditorObjectCreator::CreateGrass();
		}

		if (ImGui::Button("Add Explosion"))
		{
			EditorObjectCreator::CreateExplosion();
		}

		if (ImGui::Button("Add Enemy")) {

			selectedObject = EditorObjectCreator::CreateEnemy();
		}






		ImGui::EndTabItem();
	}

}


void CreateUiTab()
{
	if (ImGui::BeginTabItem("UI"))
	{
		ImGui::Separator();
		ImGui::Text("Create UI:");

		if (ImGui::Button("Add PlayerHPUI"))
		{

			selectedObject = EditorUICreator::CreateUIPlayerHp();
		}


		if (ImGui::Button("Add SoulGauge"))
		{
			selectedObject = EditorUICreator::CreateUISoulGauge();
		}

		ImGui::EndTabItem();
	}
}

void CreateAddGameObjectWindow()
{
	ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
	ImGui::Begin("editor");
	ImGui::BeginTabBar("Editor");
	//オブジェクトの作成のWindow
	CreateObjectTab();

	CreateUiTab();
	ImGui::EndTabBar();
	ImGui::End();
}


void ShowSceneHierarchyTab(void)
{

	if (ImGui::BeginTabItem("Hierarchy")) 
	{

		auto* layers = Scene::GetAllGameObjects();

		for (int i = 0; i < LAYER_NUM; i++)
		{
			if (layers[i].empty()) continue;

			std::string layerName = "Layer " + std::to_string(i);
			if (ImGui::TreeNode(layerName.c_str()))
			{

				if (ImGui::BeginTable(("hier_table_" + std::to_string(i)).c_str(), 2,
					ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
				{
					ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 1.0f);
					ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 90.0f);
					ImGui::TableHeadersRow();

					for (auto& obj : layers[i])
					{
						ImGui::TableNextRow();
						ImGui::PushID(obj);

						//選択
						ImGui::TableSetColumnIndex(0);

						std::string objName = obj->GetName();
						if (objName.empty()) objName = "Unnamed Object";


						bool selected = (selectedObject == obj);
						ImGui::Selectable(objName.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns);
						if (ImGui::IsItemClicked())
						{
							selectedObject = obj;
						}

						//操作ボタン
						ImGui::TableSetColumnIndex(1);

						const char* delLabel = U8("削除");
						ImVec2 textSize = ImGui::CalcTextSize(delLabel);
						ImVec2 pad = ImGui::GetStyle().FramePadding;
						float btnW = textSize.x + pad.x * 2.0f;

						float avail = ImGui::GetContentRegionAvail().x;


						const float margin = 16.0f;
						float x = ImGui::GetCursorPosX() + std::max(0.0f, avail - btnW - margin);
						ImGui::SetCursorPosX(x);

						if (ImGui::SmallButton(delLabel)) {
							obj->SetDestroy();
							if (selectedObject == obj) selectedObject = nullptr;
						}



						ImGui::PopID();
					}

					ImGui::EndTable();
				}

				ImGui::TreePop();
			}
		}

		ImGui::EndTabItem();
	}
}


void DrawShaderManagerTab()
{
	static bool showShaderManager = true;

	if (!showShaderManager) return;

	ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);

	if (ImGui::BeginTabItem("Shader Manager", &showShaderManager))
	{
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
			U8("オブジェクトを選択して変更"));
		ImGui::Separator();

		// 全オブジェクトのシェーダー一覧
		if (ImGui::CollapsingHeader(U8("全てのオブジェクト"), ImGuiTreeNodeFlags_DefaultOpen))
		{
			// 全レイヤーのリストを取得
			auto* allLayers = Scene::GetAllGameObjects();

			// レイヤー名の定義
			const char* layerNames[] = { "SYSTEM", "FIELD", "OBJECT", "EFFECT", "UI" };

			for (int layer = 0; layer < LAYER_NUM; layer++)
			{
				// レイヤーが空ならスキップ
				if (allLayers[layer].empty()) continue;

				if (ImGui::TreeNode(layerNames[layer]))
				{
					for (auto& obj : allLayers[layer])
					{
						ImGui::PushID(obj);

						// シェーダータイプ名と色を取得
						const char* shaderName = "";
						ImVec4 color = ImVec4(1, 1, 1, 1);

						switch (obj->GetShaderType())
						{
						case ShaderType::UNLIT_TEXTURE:
							shaderName = "Tex";
							color = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
							break;
						case ShaderType::UNLIT_COLOR:
							shaderName = "Color";
							color = ImVec4(1.0f, 1.0f, 0.5f, 1.0f);
							break;
						case ShaderType::TOON_SHADOW:
							shaderName = "Toon";
							color = ImVec4(1.0f, 0.5f, 1.0f, 1.0f);
							break;
						case ShaderType::FLAT_RIM:
							shaderName = "FlatRim";
							color = ImVec4(0.5f, 1.0f, 1.0f, 1.0f);
						case ShaderType::CUSTOM:
							shaderName = "Custom";
							color = ImVec4(0.5f, 0.5f, 1.0f, 1.0f);
							break;
						}

						// オブジェクト名
						ImGui::Text("%s:", obj->GetName().c_str());

						// シェーダータイプ表示
						ImGui::SameLine();
						ImGui::TextColored(color, "[%s]", shaderName);


						ImGui::SameLine();

						// クイック切り替えボタン
						ImGui::SameLine();
						if (ImGui::SmallButton("Toon"))
						{
							obj->SetShaderType(ShaderType::TOON_SHADOW);
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("FlatRim"))
						{
							obj->SetShaderType(ShaderType::FLAT_RIM);
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("Tex"))
						{
							obj->SetShaderType(ShaderType::UNLIT_TEXTURE);
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("Col"))
						{
							obj->SetShaderType(ShaderType::UNLIT_COLOR);
						}

						ImGui::PopID();
					}
					ImGui::TreePop();
				}
			}
		}

		ImGui::Separator();

		// 一括変更
		if (ImGui::CollapsingHeader("Batch Operations"))
		{
			static int batchShaderType = 2;  // デフォルトはToon
			const char* types[] = {
				"Unlit Texture",
				"Unlit Color",
				"Toon Shadow",
				"Flat Rim"

			};

			ImGui::Combo(U8("一括変更するシェーダー"), &batchShaderType, types, 4);
			ImGui::Spacing();

			auto* allLayers = Scene::GetAllGameObjects();

			// 各レイヤーごとのボタン
			const char* layerNames[] = { "SYSTEM", "FIELD", "OBJECT", "EFFECT", "UI" };

			for (int layer = 0; layer < LAYER_NUM; layer++)
			{
				std::string buttonLabel = "Apply to " + std::string(layerNames[layer]) + " Layer";

				if (ImGui::Button(buttonLabel.c_str(), ImVec2(-1, 0)))
				{
					for (auto& obj : allLayers[layer])
					{
						obj->SetShaderType((ShaderType)batchShaderType);
					}
				}
			}

			ImGui::Spacing();
			ImGui::Separator();

			if (ImGui::Button(U8("全てのレイヤーのオブジェクトを全て変更"), ImVec2(-1, 0)))
			{
				for (int layer = 0; layer < LAYER_NUM; layer++)
				{
					for (auto& obj : allLayers[layer])
					{
						obj->SetShaderType((ShaderType)batchShaderType);
					}
				}
			}

			ImGui::Text(U8("シェーダーの設定がカスタムのやつはこの変更無視する"));
		}
		ImGui::EndTabItem();
	}
	
}

void ShowSceneHierarchyWindow()
{
	ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
	ImGui::Begin("hierarchy");
	if (ImGui::BeginTabBar(U8("ヒエラルキー"))){
		//ヒエラルキーを描画
		ShowSceneHierarchyTab();
		//シェーダー
		DrawShaderManagerTab();
		ImGui::EndTabBar();
	}
	ImGui::End();
}


void ShowPropertiesTab(void)
{
	

	if (ImGui::BeginTabItem("Properties"))
	{
		// オブジェクトが選択されていない場合
		if (!selectedObject)
		{
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
				U8("オブジェクトを選択してください"));
			ImGui::EndTabItem();
			return;
		}
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

		if (ImGui::DragFloat3("Rotation", (float*)&rot, 0.1f))
		{
			selectedObject->SetRotation(rot);
			if (auto physics = dynamic_cast<PhysicsObject*>(selectedObject))
			{
				physics->m_RotationSyncCountdown = 40;
				physics->SyncToPhysics();
			}
		}

		if (ImGui::DragFloat3("Scale", (float*)&scale, 0.1f, 0.0f, 100.f))
		{
			selectedObject->SetScale(scale);
			if (auto physics = dynamic_cast<PhysicsObject*>(selectedObject))
			{
				physics->RecreateCollider();
			}
			updated = true;
		}

		// 一括スケール変更
		static float allScale = 1.0f;
		if (ImGui::DragFloat("AllScale", &allScale, 0.1f, 0.0f, 100.f))
		{
			scale = Vector3(allScale, allScale, allScale);
			selectedObject->SetScale(scale);
			if (auto physics = dynamic_cast<PhysicsObject*>(selectedObject))
			{
				physics->RecreateCollider();
			}
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


		// 物理オブジェクトなら質量も編集できる
		if (auto sun = dynamic_cast<Sun*>(selectedObject))
		{
			// ターゲット位置の調整
			ImGui::Text("Target");
			ImGui::DragFloat3("Look At", &sun->m_TargetPosition.x, 1.0f, -100.0f, 100.0f);

			ImGui::Separator();

			// ライトの方向（手動調整用）
			ImGui::Text("Light Direction (Manual)");
			ImGui::DragFloat3("Direction", &sun->m_LightDirection.x, 0.01f, -1.0f, 1.0f);

			ImGui::Separator();

			// 光の強さ
			ImGui::Text("Intensity");
			ImGui::SliderFloat("Light", &sun->m_Intensity, 0.0f, 3.0f);
			ImGui::SliderFloat("Ambient", &sun->m_AmbientStrength, 0.0f, 1.0f);

			ImGui::Separator();

			// シャドウマップの範囲
			ImGui::Text("Shadow Settings");
			ImGui::SliderFloat("Ortho Size", &sun->m_OrthoSize, 50.0f, 300.0f);

			ImGui::Separator();
		}

		if (ImGui::Button(U8("選択してるオブジェクトを削除する")))
		{
			if (selectedObject)
			{
				selectedObject->SetDestroy();  // 削除フラグを立てる
				selectedObject = nullptr;

			}
		}

		ImGui::EndTabItem();
	}
}


// アニメーション制御ウィンドウ
void ShowAnimationControlTab()
{


	if (ImGui::BeginTabItem("Animation Control"))
	{
		// オブジェクトが選択されていない場合
		if (!selectedObject)
		{
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
				U8("オブジェクトを選択してください"));
			ImGui::EndTabItem();
			return;
		}

		// EnemyクラスかどうかをチェックしてAnimationModelを取得
		Enemy* enemy = dynamic_cast<Enemy*>(selectedObject);
		if (!enemy)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
				U8("選択中のオブジェクトはアニメーションを持っていません"));
			ImGui::EndTabItem();
			return;
		}

		AnimationModel* animModel = enemy->GetAnimationModel();
		if (!animModel)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
				"AnimationModelが見つかりません");
			ImGui::EndTabItem();
			return;
		}

		// オブジェクト名表示
		ImGui::Text("Object: %s", selectedObject->GetName().c_str());
		ImGui::Separator();

		// アニメーション選択
		ImGui::Text("Animation Selection");

		// 読み込まれているアニメーションのリストを取得
		std::vector<std::string> animNames = animModel->GetAnimationNames();

		if (animNames.empty())
		{
			ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
				"読み込まれているアニメーションがありません");
		}
		else
		{
			// 現在のアニメーション名を表示
			std::string currentAnim = animModel->GetCurrentAnimationName();
			ImGui::Text("Current: %s", currentAnim.c_str());

			ImGui::Separator();

			// アニメーション選択ボタンを表示
			for (const std::string& animName : animNames)
			{
				// 現在再生中のアニメーションは色を変える
				if (animName == currentAnim)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
				}

				if (ImGui::Button(animName.c_str(), ImVec2(-1, 0)))
				{
					// アニメーション再生開始
					animModel->Play(animName.c_str(), true);
				}

				if (animName == currentAnim)
				{
					ImGui::PopStyleColor();
				}
			}
		}

		ImGui::Separator();

		// 再生制御
		ImGui::Text("Playback Control");

		// 再生状態を取得
		AnimationState state = animModel->GetState();
		const char* stateText = "";
		ImVec4 stateColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

		switch (state)
		{
		case AnimationState::PLAYING:
			stateText = "PLAYING";
			stateColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
			break;
		case AnimationState::PAUSED:
			stateText = "PAUSED";
			stateColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
			break;
		case AnimationState::STOPPED:
			stateText = "STOPPED";
			stateColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
			break;
		}

		ImGui::Text("State: ");
		ImGui::SameLine();
		ImGui::TextColored(stateColor, "%s", stateText);

		// 再生ボタン
		if (state == AnimationState::PAUSED)
		{
			if (ImGui::Button("Resume", ImVec2(-1, 0)))
			{
				animModel->Resume();
			}
		}
		else if (state == AnimationState::PLAYING)
		{
			if (ImGui::Button("Pause", ImVec2(-1, 0)))
			{
				animModel->Pause();
			}
		}
		else
		{
			if (ImGui::Button("Play", ImVec2(-1, 0)))
			{
				std::string currentAnim = animModel->GetCurrentAnimationName();
				if (!currentAnim.empty())
				{
					animModel->Play(currentAnim.c_str(), true);
				}
			}
		}

		// 停止ボタン
		if (ImGui::Button("Stop", ImVec2(-1, 0)))
		{
			animModel->Stop();
		}

		ImGui::Separator();

		// 再生スピード調整
		ImGui::Text("Play Speed");
		float speed = animModel->GetPlaySpeed();
		if (ImGui::SliderFloat("Speed", &speed, 0.1f, 5.0f, "%.1f"))
		{
			animModel->SetPlaySpeed(speed);
		}

		// クイック速度設定ボタン
		if (ImGui::Button("0.5x", ImVec2(80, 0)))
		{
			animModel->SetPlaySpeed(0.5f);
		}
		ImGui::SameLine();
		if (ImGui::Button("1.0x", ImVec2(80, 0)))
		{
			animModel->SetPlaySpeed(1.0f);
		}
		ImGui::SameLine();
		if (ImGui::Button("2.0x", ImVec2(80, 0)))
		{
			animModel->SetPlaySpeed(2.0f);
		}

		ImGui::Separator();

		// フレーム情報
		ImGui::Text("Frame Info");
		float currentFrame = animModel->GetCurrentFrame();
		int maxFrame = animModel->GetMaxFrame();
		ImGui::Text("Current Frame: %.1f / %d", currentFrame, maxFrame);

		// フレームスライダー
		if (maxFrame > 0)
		{
			float frame = currentFrame;
			if (ImGui::SliderFloat("Frame", &frame, 0.0f, (float)(maxFrame - 1), "%.1f"))
			{
				animModel->SetFrame(frame);
			}
		}

		ImGui::Separator();

		// ループ設定
		bool isLooping = animModel->IsLooping();
		if (ImGui::Checkbox("Loop Animation", &isLooping))
		{
			// ループ設定はPlay時に指定する必要があるので、現在のアニメーションを再生し直す
			std::string currentAnim = animModel->GetCurrentAnimationName();
			if (!currentAnim.empty())
			{
				float currentFrame = animModel->GetCurrentFrame();
				animModel->Play(currentAnim.c_str(), isLooping);
				animModel->SetFrame(currentFrame);
				if (state == AnimationState::PAUSED)
				{
					animModel->Pause();
				}
				else if (state == AnimationState::STOPPED)
				{
					animModel->Stop();
				}
			}
		}
		ImGui::EndTabItem();
	}

	
}

void ShowPropertiesWindow()
{
	// ウィンドウサイズ設定
	ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
	ImGui::Begin("properties");
	if (ImGui::BeginTabBar("Properties"))
	{


		ShowPropertiesTab();
		ShowAnimationControlTab();
		ImGui::EndTabBar();
	}
	ImGui::End();
}

