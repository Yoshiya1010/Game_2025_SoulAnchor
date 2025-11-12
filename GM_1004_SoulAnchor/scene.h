#pragma once

#include <chrono>
#include <vector>
#include <list>
#include "gameObject.h"
#include"json.hpp"
#include"Layer.h"
#include"main.h"
#include <fstream>

using json = nlohmann::json;





class Scene
{
private:
	static std::list<GameObject*> m_GameObjects[LAYER_NUM];
	static float m_deltaTime;

	static void UpdateDeltaTime();


	static bool debugFlag;
	
	static bool m_isPaused;   // ポーズ中フラグ

	std::string m_LoadSceneName{};


	
	std::unordered_map<std::string, int> m_NameCounter; // 各種類の連番管理


	// Toon+Shadowシェーダー（OBJECT層用）
	static ID3D11VertexShader* m_ToonVS;
	static ID3D11PixelShader* m_ToonPS;
	static ID3D11InputLayout* m_ToonLayout;

	static ID3D11VertexShader* m_ShadowVS;
	static ID3D11PixelShader* m_ShadowPS;
	static ID3D11InputLayout* m_ShadowLayout;

public:
	virtual void Init();
	virtual void Uninit();
	virtual void Update();
	virtual void Draw();

	static void InitToonShaders();
	static void UninitToonShaders();

	static void SetPaused(bool pause) { m_isPaused = pause; }
	static bool IsPaused() { return m_isPaused; }


	void SaveScene();
	void SaveSceneAs(const std::string& fileName);
	void LoadScene(const std::string& fileName);

	static void SetDebugFlag(bool flag) { debugFlag = flag; }
	static bool GetDebugFlag(void) { return debugFlag; }

	const std::string& GetLoadSceneName()const { return m_LoadSceneName; }

	template <typename T>
	T* AddGameObject(int Layer)
	{
		T* gameObject = DEBUG_NEW T();
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
	
	std::string GenerateUniqueName(const std::string& baseName);
	
};

