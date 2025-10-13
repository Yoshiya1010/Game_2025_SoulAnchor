#pragma once
#include <chrono>
#include <list>
#include<vector>
#include "gameObject.h"
#include"imgui_manager.h"
class Manager
{
private:
	static class Scene* m_Scene; // ポインタ変数なら前方宣言で解決できる。循環参照を避けるためになるべくこの形の方がいいかも
	static class Scene* m_nextScene;

	
public:
	static void Init();
	static void Uninit();
	static void Update();
	static void Draw();

	static Scene* GetScene() { return m_Scene; }

	template <typename T>
	static void SetScene()
	{
		m_nextScene = new T();
	}


};