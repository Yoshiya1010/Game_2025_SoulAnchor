#pragma once

#include "main.h"
#include "renderer.h"
#include"animationModel.h"
#include "gameObject.h"

class Enemy :public GameObject {
private:

	unique_ptr<AnimationModel> m_AnimationModel;


	
public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	AnimationModel* GetAnimationModel();
};