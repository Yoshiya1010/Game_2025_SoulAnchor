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



	bool m_ChangeHpFlag=false;
	int m_health = 3;//プレイヤーのHP


	float CalcHpAnimSpeed(int hp, int maxHearts)
	{
		if (hp <= 0) return 0.0f;          
		if (maxHearts <= 1) return 0.3f;   

		const float slow = 0.08f;//HPフル
		const float fast = 0.50f;//瀕死
	
		float t = 1.0f - float(hp - 1) / float(maxHearts - 1);
		return slow + (fast - slow) * t;
	}
public:

	void Init();
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void ChangeHp(int hp)//ダメージをくらった時はマイナスを入れて
	{
		m_health += hp;
		m_ChangeHpFlag = true;
	}

};