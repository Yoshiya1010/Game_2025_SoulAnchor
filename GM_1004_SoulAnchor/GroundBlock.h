#pragma once

#include "main.h"
#include "renderer.h"
#include "gameObject.h"
#include"ModelFBX.h"
#include<memory>
#include"PhysicsObject.h"



class GroundBlock :public PhysicsObject {
private:
	


	//ƒ‚ƒfƒ‹
	unique_ptr<StaticFBXModel> m_ModelRenderer = nullptr;
	const float m_modelScale = 2.0f;

public:
	void Init() override;
	void Start()override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	
};