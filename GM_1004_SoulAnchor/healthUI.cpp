#include "main.h"
#include "renderer.h"
#include "textureManager.h"
#include"healthUI.h"
#include"scene.h"
#include"manager.h"
#include<memory>


void HealthUI::Init()
{
    //シーンを取得
    Scene* scene = Manager::GetScene();


     //背景を設定
     playerIcon= scene->AddGameObject<Sprite2D>(UI);
     playerIcon->Init(180.f, 100.f, 350.f, 150.f, "asset\\texture\\PlayerUI\\player_icon.png");
     playerIcon->SetName("playerIcon");



     for (int i = 0; i < 3; i++)
     {
         //Hpの背景
         auto* HpBg = scene->AddGameObject<Sprite2D>(UI);
         HpBg->Init(170.0f+(i*60), 105.0f, 50.f, 50.f, "asset\\texture\\PlayerUI\\heart_Bg.png");
         HpBg->SetName("HpBg" + std::to_string(i));
         m_HPBgs.push_back(HpBg);


         //アニメーション
         auto* Hp= scene->AddGameObject<SpriteAnimator2D>(UI);
         Hp->Init(170.0f+(i*60), 105.0f, 120.f, 80.f, "asset\\texture\\PlayerUI\\hp_pinch.png", 10, 3);
         Hp->SetFrameSpeed(0.2);
         Hp->SetLoop(true);
         Hp->SetName("PinchHP"+std::to_string(i));

         m_pinchHPs.push_back(Hp);

     }

     SetName("HealthUI");//自分自身に名前を付ける
}

void HealthUI::Uninit()
{
   
}

void HealthUI::Update()
{
    
    if (Input::GetKeyPress(KK_P)) 
    {
        ChangeHp(-1);
    }

    if (Input::GetKeyPress(KK_O))
    {
        ChangeHp(1);
    }


    if (!m_ChangeHpFlag)return;

    const int maxHearts = static_cast<int>(m_pinchHPs.size());
    if (m_health < 0)           m_health = 0;
    if (m_health > maxHearts)   m_health = maxHearts;
  

    const float spd = CalcHpAnimSpeed(m_health, maxHearts);
    // ハート（アニメ）表示切替：左から m_health 個だけ表示
    for (int i = 0; i < maxHearts; ++i)
    {
        const bool visible = (i < m_health);
        if (m_pinchHPs[i])
        {
            m_pinchHPs[i]->SetDrawFlag(visible);
            m_pinchHPs[i]->SetFrameSpeed(spd);
        
        }
    }

    m_ChangeHpFlag = false;
}

void HealthUI::Draw()
{
    
}
