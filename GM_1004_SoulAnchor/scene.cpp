#include "main.h"
#include "manager.h"
#include "renderer.h"

#include "scene.h"
#include "camera.h"

//GameObject* g_GameObjects[4]; //前半を3Dオブジェクト、後半を2Dオブジェクトにする。要するにしっかり分けること
std::list<GameObject*> Scene::m_GameObjects[LAYER_NUM];
float Scene::m_deltaTime;

void Scene::Init()
{

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
	//for (int i = 0; i < 4; i++) {
	//	g_GameObjects[i]->Update();
	//}

	UpdateDeltaTime();

	for (auto& list : m_GameObjects) {

		for (auto& gameObject : list) {
			gameObject->Update();
		}
	}


	for (auto& list : m_GameObjects) {
		list.remove_if([](GameObject* object) {
			return object->Destroy();
			});
	}
	//for (int i = 0; i < 3; i++) {

	//	m_GameObjects[i].remove_if([](GameObject* object) {
	//		return object->Destroy();
	//		});
	//}


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
		for (auto& gameObject : list) {
			gameObject->Draw();
			PhysicsManager::DrawDebugWorld();
			
		}
	}
	PhysicsManager::DrawDebugWorld();

}

void Scene::UpdateDeltaTime()
{
	static auto prevTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();

	std::chrono::duration<float> delta = currentTime - prevTime;
	prevTime = currentTime;

	m_deltaTime = delta.count();
}