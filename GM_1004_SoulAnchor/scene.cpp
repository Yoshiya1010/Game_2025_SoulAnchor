#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "input.h"
#include "scene.h"
#include "camera.h"
#include <fstream>
#include "GroundBlock.h"
#include <iostream>
#include <filesystem> 
#include"player.h"
#include"wave.h"
#include"score.h"
#include"enemy.h"

//GameObject* g_GameObjects[4]; //前半を3Dオブジェクト、後半を2Dオブジェクトにする。要するにしっかり分けること
std::list<GameObject*> Scene::m_GameObjects[LAYER_NUM];
float Scene::m_deltaTime;

bool Scene::debugFlag;

bool Scene::m_isPaused = false;


void Scene::Init()
{
	debugFlag = true;
}


void Scene::Uninit()
{

	for (auto& list : m_GameObjects) {

		for (auto& gameObject : list) {
			gameObject->Uninit();
			delete gameObject;
		}
	}


	for (auto& list : m_GameObjects) {
		list.clear();
	}
}

void Scene::Update()
{
	for (auto& list : m_GameObjects) {
		list.remove_if([](GameObject* object) {
			return object->Destroy();
			delete object;
			});
	}

	
	UpdateDeltaTime();

	for (auto& list : m_GameObjects) {

		for (auto& gameObject : list) {
			gameObject->Update();
		}
	}


	if (Input::GetKeyPress(KK_Z))
	{
		debugFlag = !debugFlag;
		
	}

}

void Scene::Draw()
{
	// Zソート
	Camera* camera = GetGameObject<Camera>();

	if (camera != nullptr) {
		Vector3 CameraPosition = camera->GetPosition();

		// &でキャプチャしてやらないとCameraPositionが使えない。ラムダ式も一応関数だから外部の変数を扱うときにキャプチャする必要がある。
		m_GameObjects[OBJECT].sort([&](GameObject* a, GameObject* b) {
			return a->GetDistance(CameraPosition) > b->GetDistance(CameraPosition);
			});
	}


	/// カメラを最初に描画すること。
	for (auto& list : m_GameObjects) {
		for (auto& gameObject : list)
		{
			gameObject->Draw();

			if(debugFlag)
			PhysicsManager::DrawDebugWorld();
		}
	}
	
	

}

void Scene::UpdateDeltaTime()
{
	static auto prevTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();

	std::chrono::duration<float> delta = currentTime - prevTime;
	prevTime = currentTime;

	m_deltaTime = delta.count();
}


void Scene::SaveScene(const std::string& fileName)
{
	json root;

	// LAYER_NUMすべてを走査
	for (auto& list : m_GameObjects)
	{
		for (auto& obj : list)
		{
			root["Objects"].push_back(obj->ToJson());
		}
	}

	std::string path = "JsonSaveData/" + fileName + ".json";
	std::ofstream(path) << root.dump(4);
	
}

void Scene::LoadScene(const std::string& fileName)
{

	std::ifstream ifs("JsonSaveData/" + fileName);
	if (!ifs.is_open()) return;

	json root;
	ifs >> root;

	for (auto& list : m_GameObjects)
		for (auto& obj : list)
			obj->SetDestroy();

	for (auto& item : root["Objects"])
	{
		std::string type = item.value("Type", "GameObject");
		int layer = item.value("Layer", OBJECT);

		GameObject* obj = nullptr;

		if (type == "Player") obj = AddGameObject<Player>(layer);
		else if (type == "Camera") obj = AddGameObject<Camera>(layer);
		else if (type == "Wave") obj = AddGameObject<Wave>(layer);
		else if (type == "Score") obj = AddGameObject<Score>(layer);
		else if (type == "GroundBlock") obj = AddGameObject<GroundBlock>(layer);
		else obj = AddGameObject<GameObject>(layer);

		if (obj)
			obj->FromJson(item);
	}
	
}

