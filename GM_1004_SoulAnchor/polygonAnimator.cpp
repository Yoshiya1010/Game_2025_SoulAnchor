#include "main.h"
#include "renderer.h"
#include"polygonAnimator.h"
#include "textureManager.h"
#include"spriteAnimator2D.h"

void PolygonAnimator2D::Init(float x, float y, float w, float h, const char* FileName, int cols, int rows)
{
    m_SpriteAnimator = std::make_unique<SpriteAnimator2D>();
    m_SpriteAnimator->Init(x, y, w, h, FileName,cols,rows);
}

void PolygonAnimator2D::Uninit()
{
    if (m_SpriteAnimator)
        m_SpriteAnimator->Uninit();

    m_SpriteAnimator.reset();
}

void PolygonAnimator2D::Update()
{
    m_SpriteAnimator->Update();
}

void PolygonAnimator2D::Draw()
{
    m_SpriteAnimator->Draw();
}
