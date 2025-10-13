#pragma once
#include "scene.h"

class Game :public Scene {
private:
	class Audio* m_BGM;
	static ImGuiManager m_ImGuiManager;

	static GameObject* selectedObject;
public:
	void Init() override;
	void Update() override;
	void Uninit() override;

	void DrawImgui(void);

	void ShowSceneHierarchy();
};