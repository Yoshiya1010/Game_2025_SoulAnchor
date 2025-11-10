#pragma once

#include "main.h"
#include "renderer.h"
#include"sprite2D.h"
#include "gameObject.h"
#include"polygon.h"

class HealthUI : public GameObject {

private:

	std::unique_ptr<Polygon2D> m_player_icon;



	int m_health = 6;//ÉvÉåÉCÉÑÅ[ÇÃHP
public:
	
	void Init();
	void Uninit() override;
	void Update() override;
	void Draw() override;
};