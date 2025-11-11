#include "main.h"
#include "renderer.h"
#include "textureManager.h"
#include"soulGaugeUI.h"
#include"scene.h"
#include"manager.h"
#include<memory>
#include"input.h"


void SoulGaugeUI::Init()
{
    //シーンを取得
    Scene* scene = Manager::GetScene();

    Vector3 position{ 180.f,500.f,0.f };
    Vector3 size{ 260.f,450.f,0.f };

    Vector3 GaugePosition{ 140.0f,475.0f,0.f };
    Vector3 GaugeSize{ 55.f,300.f,1.f };

    //背景を設定

    //ゲージのボーダー
    auto soulGaugeBorder = scene->AddGameObject<Sprite2D>(UI);
    soulGaugeBorder->Init(position.x, position.y, size.x, size.y, "asset\\texture\\SoulGauge\\gauge_border.png");
    soulGaugeBorder->SetName("GaugeBorder");

    //ゲージ
    m_Gauge = scene->AddGameObject<SpriteSoulGauge>(UI);
    m_Gauge->Init(GaugePosition.x, GaugePosition.y, GaugeSize.x, GaugeSize.y, "asset\\texture\\SoulGauge\\gauge_blue.png","asset\\texture\\SoulGauge\\gauge_yellow.png", "asset\\texture\\SoulGauge\\gauge_red.png");
    m_Gauge->SetName("GaugeYellow");
    m_Gauge->SetValue(0.f);


    //ゲージの仕切り
    auto soulGaugeDivision = scene->AddGameObject<Sprite2D>(UI);
    soulGaugeDivision->Init(position.x, position.y, size.x, size.y, "asset\\texture\\SoulGauge\\gauge_division.png");
    soulGaugeDivision->SetName("GaugeDivision");

    // 外枠の装飾
    auto soulGaugeOutLine = scene->AddGameObject<Sprite2D>(UI);
    soulGaugeOutLine->Init(position.x, position.y, size.x, size.y, "asset\\texture\\SoulGauge\\gauge_out_line.png");
    soulGaugeOutLine->SetName("GaugeOutLine");

    //　レベル表示の際の右端の羽のBG
    auto soulGaugeLevelBg = scene->AddGameObject<Sprite2D>(UI);
    soulGaugeLevelBg->Init(position.x, position.y, size.x, size.y, "asset\\texture\\SoulGauge\\level_bg.png");
    soulGaugeLevelBg->SetName("GaugeLevelBg");
    
    //　レベル表示の際の右端の羽のアウトライン
    auto soulGaugeLevelOutLine = scene->AddGameObject<Sprite2D>(UI);
    soulGaugeLevelOutLine->Init(position.x, position.y, size.x, size.y, "asset\\texture\\SoulGauge\\level_line.png");
    soulGaugeLevelOutLine->SetName("GaugeLevelOutLine");


    //宝石をはめるスロット
    auto soulGaugeSlot = scene->AddGameObject<Sprite2D>(UI);
    soulGaugeSlot->Init(position.x, position.y, size.x, size.y, "asset\\texture\\SoulGauge\\gauge_slot.png");
    soulGaugeSlot->SetName("GaugeSlot");
     

    //宝石　青
    auto blueJewel = scene->AddGameObject<Sprite2D>(UI);
    blueJewel->Init(position.x, position.y, size.x, size.y, "asset\\texture\\SoulGauge\\blue_jewel.png");
    blueJewel->SetName("blueJewel");

    //宝石　赤
    auto redJewel = scene->AddGameObject<Sprite2D>(UI);
    redJewel->Init(position.x, position.y, size.x, size.y, "asset\\texture\\SoulGauge\\red_jewel.png");
    redJewel->SetName("redJewel");

    //宝石　黄色
    auto yellowJewel = scene->AddGameObject<Sprite2D>(UI);
    yellowJewel->Init(position.x, position.y, size.x, size.y, "asset\\texture\\SoulGauge\\yellow_jewel.png");
    yellowJewel->SetName("yellowJewel");

    
    


}

void SoulGaugeUI::Uninit()
{

}

void SoulGaugeUI::Update()
{
    if (Input::GetKeyTrigger(KK_O)) 
    {
        m_Gauge->SetTargetValue(m_Gauge->GetTargetValue()+10.f);
    }

    if (Input::GetKeyTrigger(KK_L))
    {
        m_Gauge->SetTargetValue(m_Gauge->GetTargetValue() - 10.f);
    }
   
}

void SoulGaugeUI::Draw()
{

}
