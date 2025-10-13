#include "main.h"
#include "manager.h"
#include "renderer.h"

#include "input.h"
#include "result.h"
#include "title.h"
#include "Polygon.h"

void Result::Init()
{
	AddGameObject<Polygon2D>(UI)->Init(0.0f,0.0f,SCREEN_WIDTH,SCREEN_HEIGHT,"asset\\texture\\Result_BG.png");
}

void Result::Update()
{
	Scene::Update();

	if (Input::GetKeyTrigger(KK_ENTER)) {
		Manager::SetScene<Title>();
	}
}
