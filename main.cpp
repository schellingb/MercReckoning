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
#include "world.h"

ZL_TouchInput TouchInput;
ZL_Font fntMain, fntBig;
ZL_Surface srfBGBrown, srfBGGreen, srfPalm;
ZL_Sound sndMenuMusic, sndSelect;

#define ZL_OPT_DO_IMPLEMENTATION
#include <../Opt/ZL_TouchInput.h>

#ifdef SCENE_EDITOR
static const int DisplayFlags = ZL_DISPLAY_ALLOWRESIZEVERTICAL | ZL_DISPLAY_OVERRIDEANDROIDVOLUMEKEYS;
#else
static const int DisplayFlags = ZL_DISPLAY_ALLOWRESIZEVERTICAL;
#endif

struct sAnimalPunch : public ZL_Application
{
	sAnimalPunch() : ZL_Application(60) { }

	virtual void Load(int argc, char *argv[])
	{
		if (!ZL_Application::LoadReleaseDesktopDataBundle()) return;
		if (!ZL_Display::Init("Mercenary Reckoning", 800, 480, DisplayFlags)) return;
		ZL_Display::ClearFill(ZL_Color::White);
		ZL_Display::SetAA(true);
		ZL_Audio::Init();
		ZL_Application::SettingsInit("MercReckoning");
		ZL_Timer::Init();
		ZL_TouchInput::Init();

		fntMain = ZL_Font("Data/fntMain.png");
		fntBig = ZL_Font("Data/matchbox.ttf.zip", 50);
		srfBGBrown = ZL_Surface("Data/ui_brownbg.jpg").SetTextureRepeatMode();
		srfBGGreen = ZL_Surface("Data/ui_greenbg.jpg").SetTextureRepeatMode();
		srfPalm = ZL_Surface("Data/palm.png").SetDrawOrigin(ZL_Origin::Center);
		sndSelect = ZL_Sound("Data/select.ogg");

		World.InitGlobal();

		ZL_SceneManager::Init(SCENE_MENU);

		/*
		//QUICKSTART DEBUG
		World.Init();
		World.Round = World.RoundHighest = 3;
		//World.RoundHighest++;
		//World.Player.Team.push_back(World.Dudes[0]);
		World.Player.Team.push_back(World.Dudes[7]);
		World.Dudes[7]->Health = World.Dudes[7]->HealthMax = 10000;
		//World.Player.Team.push_back(World.Dudes[1]);
		//World.Player.Team.front()->Health = 100;
		//World.Player.Team.front()->Weapon->Ammo = 3; //debug no ammo
		//ZL_SceneManager::Init(SCENE_TEAM);
		//for (vector<sTeamMember*>::iterator it = World.Dudes.begin()+3; it != World.Dudes.end(); ++it) { World.Player.Team.push_back(*it); (*it)->hired = true; }
		ZL_SceneManager::Init(SCENE_GAME);
		//for (vector<cAnimal*>::iterator e = World.Enemies.begin(); e != World.Enemies.end(); ++e)
		//	(*e)->ReceiveAttack(10000, 1);
		//for (int i = 0; i < World.Player.Team.size(); i++) { World.PlaceDude(World.Player.Team[i], ZL_Vector(50*(1+(i%4)), 20+65*(1+i/4))); World.Player.Team[i]->ViewAngle = -PIHALF; }
		*/

		#if 0 //OUTPUT ROUND MONEY REWARD STATS ONLY THEN QUIT
		for (World.Round = 1; World.Round <= sWorld::FinalRound; World.Round++)
		{
			World.StartRound();
			int cnum = 0, ctotal = 0;
			for (vector<cAnimal*>::iterator e = World.Enemies.begin(); e != World.Enemies.end(); ++e)
			{ cnum++; ctotal += (*e)->Reward; }
			printf("Round %2d: Enemies = %2d - Total with Bonus = %5d - Total no Bonus = %5d (%s)\n", World.Round, cnum, ctotal+World.RoundBonus, ctotal, World.GetRoundName());
		}
		exit(0);
		#endif
	}

	//display fps
	//virtual void AfterFrame() { fntMain.Draw(ZLFROMW(50), ZLFROMH(30), ZL_String::format("%d", FPS)); }
} AnimalPunch;
