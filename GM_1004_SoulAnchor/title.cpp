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
	//îwåi
	AddGameObject<Polygon2D>(UI)->Init(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, "asset\\texture\\TitleScene\\title_background.jpg");


	////É^ÉCÉgÉãÉçÉS
	//auto logo = AddGameObject<SpriteAnimator2D>(UI);
	//logo->Init(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 600, 300,
	//	"asset\\texture\\TitleScene\\title_logo_text_loop.png",
	//	6, 6);
	//logo->SetFrameSpeed(0.08f);

	AddGameObject<Polygon2D>(UI)->Init(900, 600, 300, 100, "asset\\texture\\TitleScene\\UI\\UI_start.png");


	//AÉ{É^Éì
	m_button = AddGameObject<SpriteAnimator2D>(UI);
	m_button->Init(950, 650, 100, 90,
		"asset\\texture\\TitleScene\\UI\\skip_a.png",
		10, 3);
	m_button->SetFrameSpeed(0);
	m_button->SetLoop(false);


}

void Title::Update()
{
	Scene::Update();


	if (m_button->GetMaxFrame()-1 <= m_button->GetFrame())
	{
		Manager::SetScene<Game>();
	}
	if (Input::GetKeyPress(KK_ENTER))
	{
		m_button->Play();
		m_button->SetFrameSpeed(0.2f);
	}

	if (Input::GetKeyRelease(KK_ENTER))
	{
		m_button->Play();
		m_button->SetFrameSpeed(-0.2f);
	}

	
}
