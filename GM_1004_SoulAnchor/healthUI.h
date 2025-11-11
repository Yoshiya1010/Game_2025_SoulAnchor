#pragma once

#include "main.h"
#include "renderer.h"
#include"sprite2D.h"
#include"spriteAnimator2D.h"
#include "gameObject.h"
#include"input.h"
#include <memory>
class HealthUI : public GameObject {

private:

	//プレイヤーのアイコン
	Sprite2D *playerIcon;//プレイヤーのアイコン

	
	std::vector<SpriteAnimator2D*> m_pinchHPs;//HPの振動
	

	std::vector<Sprite2D*> m_HPBgs;//HPの背景



	int m_health = 6;//プレイヤーのHP
public:
	
	void Init();
	void Uninit() override;
	void Update() override;
	void Draw() override;
};