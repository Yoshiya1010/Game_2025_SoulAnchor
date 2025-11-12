#pragma once

#include "main.h"
#include "renderer.h"
#include "gameObject.h"
#include"ModelFBX.h"
#include<memory>
#include"fragmentObject.h"



class GroundBlock :public FragmentObject {
public:
	void Init() override;
	void Start()override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	
};