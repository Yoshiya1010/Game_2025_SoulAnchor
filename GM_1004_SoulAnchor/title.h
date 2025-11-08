#pragma once

#include "scene.h"
#include"polygonAnimator.h"

class Title :public Scene {
private:

	PolygonAnimator2D* m_button=nullptr;
	int m_NoPressCounter = 0;
public:
	void Init() override;
	void Update() override;
};