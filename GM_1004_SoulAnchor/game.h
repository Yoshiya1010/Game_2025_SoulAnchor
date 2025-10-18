#pragma once
#include "scene.h"
#include "imgui_manager.h"

class Game :public Scene {
private:
	class Audio* m_BGM;
	static ImGuiManager m_ImGuiManager;

	
public:
	void Init() override;
	void Update() override;
	void Uninit() override;

	void DrawImgui(void);


};