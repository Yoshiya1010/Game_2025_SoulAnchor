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
	if (m_isPaused) return; 
	UpdateDeltaTime();

	for (auto& list : m_GameObjects) {

		for (auto& gameObject : list) {
			gameObject->Update();
		}
	}


	for (auto& list : m_GameObjects) {
		list.remove_if([](GameObject* object) {
			return object->Destroy();
			delete object;
			});
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

	
	std::ifstream ifs("JsonSaveData/"+fileName);
	if (!ifs.is_open()) {
		//Jsonファイルないわ
		return;
	}

	json root;
	ifs >> root;
	Scene::SetPaused(true);

	for (auto& list : m_GameObjects) {
		for (auto& obj : list) {
			obj->SetDestroy(); // ← ここで予約
		}
	}

	// 新しいオブジェクトを生成
	for (auto& item : root["Objects"]) {
		auto obj = CreateGameObjectFromJson(item);
		AddExistingGameObject(OBJECT, obj.release());
	}
	Scene::SetPaused(false);

	
}

std::unique_ptr<GameObject> CreateGameObjectFromJson(const json& j)
{
	std::string type = j.value("Type", "GameObject");
	std::unique_ptr<GameObject> obj;

	if (type == "GroundBlock")      obj = std::make_unique<GroundBlock>();
	else                            obj = std::make_unique<GameObject>();

	obj->FromJson(j);
	return obj;
}