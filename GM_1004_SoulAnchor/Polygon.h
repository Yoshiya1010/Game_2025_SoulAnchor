#pragma once

#include "main.h"
#include "renderer.h"
#include"sprite2D.h"
#include "gameObject.h"

class Polygon2D : public GameObject{

private:

	std::unique_ptr<Sprite2D> m_Sprite;
public:
	void Init() {}
	void Init(float x, float y, float width, float height, const char* FileName);
	void Uninit() override;
	void Update() override;
	void Draw() override;
};