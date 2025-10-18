#pragma once

#include <chrono>
#include <vector>
#include <list>
#include "gameObject.h"
#include"json.hpp"
using json = nlohmann::json;



enum LAYER {
	SYSTEM = 0,
	FIELD,
	OBJECT,
	EFFECT,
	UI,
	LAYER_NUM, // この列挙体の要素数
};

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

	void AddExistingGameObject(int Layer, GameObject* obj)
	{
		m_GameObjects[Layer].push_back(obj);
	}

	template <typename T>
	T* AddGameObject(int Layer)
	{
		T* gameObject = new T();
		gameObject->Init();
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

	static float GetDeltaTime() { return m_deltaTime; }
};

std::unique_ptr<GameObject> CreateGameObjectFromJson(const json& j);