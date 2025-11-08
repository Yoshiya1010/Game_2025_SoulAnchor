#include "main.h"
#include "renderer.h"
#include "Polygon.h"
#include "textureManager.h"
#include"sprite2D.h"

void Polygon2D::Init(float x, float y, float width, float height, const char* FileName)
{
    m_Sprite = std::make_unique<Sprite2D>();
    m_Sprite->Init(x, y, width, height, FileName);
}

void Polygon2D::Uninit()
{
    if (m_Sprite)
    m_Sprite->Uninit();

    m_Sprite.reset();
}

void Polygon2D::Update()
{
}

void Polygon2D::Draw()
{
    m_Sprite->Draw();
}
