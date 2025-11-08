#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "input.h"
#include "title.h"
#include "game.h"
#include "Polygon.h"
#include"spriteAnimator2D.h"

void Title::Init()
{

	AddGameObject<Polygon2D>(UI)->Init(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, "asset\\texture\\TitleScene\\title_background.jpg");

	auto logo = AddGameObject<SpriteAnimator2D>(UI);
	logo->Init(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 600, 300,
		"asset\\texture\\TitleScene\\title_logo_text_loop.png",
		6, 6);
	logo->SetFrameSpeed(0.08f);
}

void Title::Update()
{
	Scene::Update();

	if (Input::GetKeyTrigger(KK_ENTER)) {
		Manager::SetScene<Game>();
	}
}
