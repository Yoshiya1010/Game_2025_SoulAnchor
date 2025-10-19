#pragma once

#include <chrono>
#include <vector>
#include <list>
#include "gameObject.h"
#include"json.hpp"
#include"Layer.h"
using json = nlohmann::json;





class Scene
{
private:
	static std::list<GameObject*> m_GameObjects[LAYER_NUM];
	static float m_deltaTime;

	static void UpdateDeltaTime();


	static bool debugFlag;
	
	static bool m_isPaused;   // ポーズ中フラグ

public:
	virtual void Init();
	virtual void Uninit();
	virtual void Update();
	virtual void Draw();

	static void SetPaused(bool pause) { m_isPaused = pause; }
	static bool IsPaused() { return m_isPaused; }


	void SaveScene(const std::string& fileName);
	void LoadScene(const std::string& fileName);

	

	template <typename T>
	T* AddGameObject(int Layer)
	{
		T* gameObject = new T();
		gameObject->Init();
		gameObject->SetLayer(Layer);

		m_GameObjects[Layer].push_back(gameObject);

	

		return gameObject;
	}

	template <typename T>
	T* GetGameObject()
	{
		for (auto& list : m_GameObjects) {

			for (auto& gameObject : list) {

				T* find = dynamic_cast<T*>(gameObject); // T*型にあてはめられない場合、nullが入る

				if (find) {
					return find;
				}
			}
		}

		return nullptr;
	}

	template <typename T>
	std::vector<T*> GetGameObjects()
	{

		std::vector<T*> finds;

		for (auto& list : m_GameObjects) {

			for (auto& gameObject : list) {
				T* find = dynamic_cast<T*>(gameObject); // T*型にあてはめられない場合、nullが入る

				if (find) {
					finds.push_back(find);
				}
			}
		}


		return finds;
	}
	static std::list<GameObject*>* GetAllGameObjects() { return m_GameObjects; }


	static float GetDeltaTime() { return m_deltaTime; }
};

