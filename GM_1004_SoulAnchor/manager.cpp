
#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "input.h"
#include "title.h"
#include "game.h"
#include "audio.h"

#include"imgui_manager.h"
// ヘッダ上で宣言しても実体は作られないからこっちで定義する必要がある
Scene* Manager::m_Scene = nullptr;
Scene* Manager::m_nextScene = nullptr;



void Manager::Init()
{
	Renderer::Init();
	Input::Init();
	Audio::InitMaster();

	// ゲームシーンを入れる
	m_Scene = new Game();
	m_Scene->Init();
}


void Manager::Uninit()
{
	m_Scene->Uninit();
	delete m_Scene;

	Audio::UninitMaster();
	Input::Uninit();
	Renderer::Uninit();

	
}

void Manager::Update()
{
	Input::Update();

	m_Scene->Update();

	
}

void Manager::Draw()
{
	Renderer::Begin();

	m_Scene->Draw();

	// m_SceneがGameクラスのインスタンスかどうかチェック
	if (m_Scene != nullptr && dynamic_cast<Game*>(m_Scene) != nullptr)
	{
		Game* gameScene = dynamic_cast<Game*>(m_Scene);
		gameScene->DrawImgui();
	}


	Renderer::End();

	// 画面遷移
	if (m_nextScene != nullptr) {
		m_Scene->Uninit();
		delete m_Scene;

		m_Scene = m_nextScene;
		m_Scene->Init();

		m_nextScene = nullptr;
	}
}


