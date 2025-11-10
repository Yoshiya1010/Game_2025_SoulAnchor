#include "main.h"
#include "renderer.h"
#include "textureManager.h"
#include"sprite2D.h"
#include"healthUI.h"
#include"scene.h"
#include"manager.h"

void HealthUI::Init()
{
    //ƒV[ƒ“‚ðŽæ“¾
    Scene* scene = Manager::GetScene();
    Sprite2D *PlayerUi= scene->AddGameObject<Sprite2D>(UI);
    PlayerUi->Init(0.f, 0.f, 100.f, 100.f, "asset\\texture\\PlayerUI\\player_icon.png");
}

void HealthUI::Uninit()
{
   
}

void HealthUI::Update()
{
}

void HealthUI::Draw()
{
    
}
