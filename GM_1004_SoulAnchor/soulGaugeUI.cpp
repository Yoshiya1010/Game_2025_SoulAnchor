#include "main.h"
#include "renderer.h"
#include "textureManager.h"
#include"soulGaugeUI.h"
#include"scene.h"
#include"manager.h"
#include<memory>


void SoulGaugeUI::Init()
{
    //シーンを取得
    Scene* scene = Manager::GetScene();

    Vector3 position{ 180.f,500.f,0.f };
    Vector3 size{ 260.f,450.f,0.f };

    //背景を設定

    //ゲージのボーダー
    auto soulGaugeBorder = scene->AddGameObject<Sprite2D>(UI);
    soulGaugeBorder->Init(position.x, position.y, size.x, size.y, "asset\\texture\\SoulGauge\\gauge_border.png");
    soulGaugeBorder->SetName("GaugeBorder");

    //ゲージ
    auto soulGaugeYellow = scene->AddGameObject<Sprite2D>(UI);
    soulGaugeYellow->Init(position.x, position.y, size.x, size.y, "asset\\texture\\SoulGauge\\gauge_yellow.png");
    soulGaugeYellow->SetName("GaugeYellow");

    //ゲージの仕切り
    auto soulGaugeDivision = scene->AddGameObject<Sprite2D>(UI);
    soulGaugeDivision->Init(position.x, position.y, size.x, size.y, "asset\\texture\\SoulGauge\\gauge_division.png");
    soulGaugeDivision->SetName("GaugeDivision");

    // 外枠の装飾
    auto soulGaugeOutLine = scene->AddGameObject<Sprite2D>(UI);
    soulGaugeOutLine->Init(position.x, position.y, size.x, size.y, "asset\\texture\\SoulGauge\\gauge_out_line.png");
    soulGaugeOutLine->SetName("GaugeOutLine");

    auto soulGaugeLevelBg = scene->AddGameObject<Sprite2D>(UI);
    soulGaugeLevelBg->Init(position.x, position.y, size.x, size.y, "asset\\texture\\SoulGauge\\level_bg.png");
    soulGaugeLevelBg->SetName("GaugeLevelBg");
    

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

   
}

void SoulGaugeUI::Draw()
{

}
