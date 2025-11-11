#include "main.h"
#include "renderer.h"
#include "textureManager.h"
#include"soulGaugeUI.h"
#include"scene.h"
#include"manager.h"
#include<memory>


void SoulGaugeUI::Init()
{
    //ƒV[ƒ“‚ðŽæ“¾
    Scene* scene = Manager::GetScene();


    //”wŒi‚ðÝ’è
    playerIcon = scene->AddGameObject<Sprite2D>(UI);
    playerIcon->Init(180.f, 100.f, 350.f, 150.f, "asset\\texture\\PlayerUI\\player_icon.png");
    playerIcon->SetName("playerIcon");



    for (int i = 0; i < 3; i++)
    {
        //Hp‚Ì”wŒi
        auto* HpBg = scene->AddGameObject<Sprite2D>(UI);
        HpBg->Init(170.0f + (i * 60), 105.0f, 50.f, 50.f, "asset\\texture\\PlayerUI\\heart_Bg.png");
        HpBg->SetName("HpBg" + std::to_string(i));
        m_HPBgs.push_back(HpBg);


        //ƒAƒjƒ[ƒVƒ‡ƒ“
        auto* Hp = scene->AddGameObject<SpriteAnimator2D>(UI);
        Hp->Init(170.0f + (i * 60), 105.0f, 120.f, 80.f, "asset\\texture\\PlayerUI\\hp_pinch.png", 10, 3);
        Hp->SetFrameSpeed(0.2);
        Hp->SetLoop(true);
        Hp->SetName("PinchHP" + std::to_string(i));

        m_pinchHPs.push_back(Hp);

    }

    
}

void SoulGaugeUI::Uninit()
{

}

void SoulGaugeUI::Update()
{

   
}

void SoulGaugeUI::Draw()
{

}
