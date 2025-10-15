#pragma once

#include <chrono>
#include <vector>
#include <list>
#include "gameObject.h"


enum LAYER {
	SYSTEM = 0,
	FIELD,
	OBJECT,
	EFFECT,
	UI,
	LAYER_NUM, // ‚±‚Ì—ñ‹“‘Ì‚Ì—v‘f”
};

class Scene {
private:
	static std::list<GameObject*> m_GameObjects[LAYER_NUM];
	static float m_deltaTime;

	static void UpdateDeltaTime();


	static bool debugFlag;
	
public:
	virtual void Init();
	virtual void Uninit();
	virtual void Update();
	virtual void Draw();

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

				T* find = dynamic_cast<T*>(gameObject); // T*Œ^‚É‚ ‚Ä‚Í‚ß‚ç‚ê‚È‚¢ê‡Anull‚ª“ü‚é

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
				T* find = dynamic_cast<T*>(gameObject); // T*Œ^‚É‚ ‚Ä‚Í‚ß‚ç‚ê‚È‚¢ê‡Anull‚ª“ü‚é

				if (find) {
					finds.push_back(find);
				}
			}
		}


		return finds;
	}

	static float GetDeltaTime() { return m_deltaTime; }
};