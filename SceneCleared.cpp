/*
  Mercenary Reckoning
  Copyright (C) 2013,2016 Bernhard Schelling, Leonard Wyrsch, Adrian Keller

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "include.h"

static scalar Pos, ImageScale;
static ZL_Surface Image, srfHole;

struct sSceneCleared : public ZL_Scene
{
	sSceneCleared() : ZL_Scene(SCENE_CLEARED) { }

	int InitTransitionEnter(ZL_SceneType SceneTypeFrom, void* data)
	{
		ZL_Application::SettingsSet("CLEARED", "1");
		ZL_Application::SettingsSynchronize();
		Image = ZL_Surface("Data/cleared.jpg").SetDrawOrigin(ZL_Origin::Center);
		ImageScale = MIN(ZLFROMW(80)/s(Image.GetWidth()), ZLFROMH(80)/s(Image.GetHeight()));
		srfHole = ZL_Surface("Data/hole.png");
		sndMenuMusic = ZL_Sound("Data/menu.ogg");
		sndMenuMusic.Play(true);
		ZL_Timer::AddTransitionScalar(&(Pos = 0), 1, 8000);
		return 1500;
	}

	int DeInitTransitionLeave(ZL_SceneType SceneTypeTo)
	{
		ZL_Display::AllSigDisconnect(this);
		return 3000;
	}

	void DeInitAfterTransition()
	{
		Image = ZL_Surface();
		srfHole = ZL_Surface();
	}

	void InitAfterTransition()
	{
		ZL_Display::sigPointerDown.connect(this, &sSceneCleared::OnPointerDown);
		ZL_Display::sigKeyDown.connect(this, &sSceneCleared::OnKeyDown);
	}

	void OnPointerDown(ZL_PointerPressEvent& e)
	{
		ZL_SceneManager::GoToScene(SCENE_MENU);
	}

	void OnKeyDown(ZL_KeyboardEvent& e)
	{
		ZL_SceneManager::GoToScene(SCENE_MENU);
	}

	void Draw()
	{
		Image.Draw(                   ZLHALFW + ZLHALFW*(1-Pos)/3, ZLHALFH - ZLHALFH*(1-Pos)/3, ImageScale*5, ImageScale*4, ZLLUMA(.5, 1));
		srfHole.DrawTo(0, 0, ZLWIDTH, ZLHEIGHT);
		ZL_Display::DrawRect(ZL_Rectf(ZLHALFW + ZLHALFW*(1-Pos)/2, ZLHALFH - ZLHALFH*(1-Pos)/2, Image.GetSize()*ImageScale*s(0.51)), ZLTRANSPARENT, ZLRGBA(.7,.3,0,1));
		Image.Draw(                   ZLHALFW + ZLHALFW*(1-Pos)/2, ZLHALFH - ZLHALFH*(1-Pos)/2, ImageScale, ImageScale, ZLALPHA(1));
		srfHole.DrawTo(0, 0, ZLWIDTH, ZLHEIGHT);

		scalar y = ZLFROMH(100);
		fntBig.Draw(ZLHALFW - 5, y + 5, "CONGRATULATIONS", s(1.2), s(1.2), ZLLUMA(0,1), ZL_Origin::Center);
		fntBig.Draw(ZLHALFW + 5, y + 5, "CONGRATULATIONS", s(1.2), s(1.2), ZLLUMA(0,1), ZL_Origin::Center);
		fntBig.Draw(ZLHALFW - 5, y - 5, "CONGRATULATIONS", s(1.2), s(1.2), ZLLUMA(0,1), ZL_Origin::Center);
		fntBig.Draw(ZLHALFW + 5, y - 5, "CONGRATULATIONS", s(1.2), s(1.2), ZLLUMA(0,1), ZL_Origin::Center);
		fntBig.Draw(ZLHALFW    , y    , "CONGRATULATIONS", s(1.2), s(1.2), ZLALPHA(1) , ZL_Origin::Center);

		y = 100;
		fntBig.Draw(ZLHALFW - 5, y + 5, "YOU PASSED THE MERCENARY RECKONING", s(.8), s(.8), ZLLUMA(0,1), ZL_Origin::Center);
		fntBig.Draw(ZLHALFW + 5, y + 5, "YOU PASSED THE MERCENARY RECKONING", s(.8), s(.8), ZLLUMA(0,1), ZL_Origin::Center);
		fntBig.Draw(ZLHALFW - 5, y - 5, "YOU PASSED THE MERCENARY RECKONING", s(.8), s(.8), ZLLUMA(0,1), ZL_Origin::Center);
		fntBig.Draw(ZLHALFW + 5, y - 5, "YOU PASSED THE MERCENARY RECKONING", s(.8), s(.8), ZLLUMA(0,1), ZL_Origin::Center);
		fntBig.Draw(ZLHALFW    , y    , "YOU PASSED THE MERCENARY RECKONING", s(.8), s(.8), ZLALPHA(1) , ZL_Origin::Center);
	}
} SceneCleared;
