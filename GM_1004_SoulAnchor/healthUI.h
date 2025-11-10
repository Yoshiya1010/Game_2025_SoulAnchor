#pragma once

#include "main.h"
#include "renderer.h"
#include"sprite2D.h"
#include "gameObject.h"
class HealthUI : public GameObject {

private:





	int m_health = 6;//ƒvƒŒƒCƒ„[‚ÌHP
public:
	
	void Init();
	void Uninit() override;
	void Update() override;
	void Draw() override;
};