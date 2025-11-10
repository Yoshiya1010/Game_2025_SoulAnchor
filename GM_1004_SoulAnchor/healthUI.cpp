#include "main.h"
#include "renderer.h"
#include "Polygon.h"
#include "textureManager.h"
#include"sprite2D.h"
#include"healthUI.h"

void HealthUI::Init()
{
    m_player_icon = std::make_unique<Sprite2D>();
    m_player_icon->Init(0, 0, 100, 100,"asset\\texture\\PlayerUI\\player_icon.png");
}

void HealthUI::Uninit()
{
    if (m_player_icon)
        m_player_icon->Uninit();

    m_player_icon.reset();
}

void HealthUI::Update()
{
}

void HealthUI::Draw()
{
    m_player_icon->Draw();
}
