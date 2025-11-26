#pragma once

#include "main.h"
#include "renderer.h"
#include"animationModel.h"
#include "gameObject.h"
#include"fragmentObject.h"

class Enemy :public FragmentObject {
private:

	unique_ptr<AnimationModel> m_AnimationModel;


	
public:
	void Init() override;
	void Start()override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	AnimationModel* GetAnimationModel();
};