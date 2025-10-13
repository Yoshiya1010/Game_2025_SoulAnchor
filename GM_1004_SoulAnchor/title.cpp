#include "main.h"
#include "manager.h"
#include "renderer.h"

#include "input.h"
#include "title.h"
#include "game.h"
#include "Polygon.h"

void Title::Init()
{
	AddGameObject<Polygon2D>(UI)->Init(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, "asset\\texture\\Title_BG.png");
}

void Title::Update()
{
	Scene::Update();

	if (Input::GetKeyTrigger(KK_ENTER)) {
		Manager::SetScene<Game>();
	}
}
