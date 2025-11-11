#pragma once


#include "main.h"
#include "renderer.h"
#include"sprite2D.h"
#include"spriteAnimator2D.h"
#include "gameObject.h"
#include"input.h"
#include <memory>
class SoulGaugeUI : public GameObject {

private:





	




public:

	void Init();
	void Uninit() override;
	void Update() override;
	void Draw() override;

	

};