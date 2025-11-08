#pragma once

#include "main.h"
#include "renderer.h"
#include"spriteAnimator2D.h"
#include "gameObject.h"

class PolygonAnimator2D : public GameObject {

private:

	std::unique_ptr<SpriteAnimator2D> m_SpriteAnimator;
public:
	void Init() {}
	void Init(float x, float y, float w, float h, const char* FileName, int cols, int rows);
	void Uninit() override;
	void Update() override;
	void Draw() override;


	void Play() { m_SpriteAnimator->Play(); }
	void SetFrameSpeed(float s) { m_SpriteAnimator->SetFrameSpeed(s); }
	int  GetFrame() const { return m_SpriteAnimator->GetFrame(); }
	int  GetMaxFrame() const { return m_SpriteAnimator->GetMaxFrame(); }
	void SetLoop(bool loop) { m_SpriteAnimator->SetLoop(loop); }
};