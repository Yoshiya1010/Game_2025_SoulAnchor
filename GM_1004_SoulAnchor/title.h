#pragma once

#include "scene.h"
#include"spriteAnimator2D.h"

class Title :public Scene {
private:

	SpriteAnimator2D* m_button=nullptr;
	int m_NoPressCounter = 0;
public:
	void Init() override;
	void Update() override;
};