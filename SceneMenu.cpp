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

static ZL_Surface srfTitle1, srfTitle2, srfTitle3, srf1GAMLogo, srfHole;
static ZL_Rectf recStart, recContinue;
#ifdef SCENE_EDITOR //DEBUGMODE ONLY SCENE
static ZL_Rectf recEditor;
#endif
static bool has_save;

static void OnStartPress()
{
	sndSelect.Play();
	World.HintsShown = 0; //show all hints again
	World.Init();
	ZL_SceneManager::GoToScene(SCENE_INTRO);
}

static void OnContinuePress()
{
	if (!World.LoadGame()) return;
	sndSelect.Play();
	sndMenuMusic = ZL_Sound();
	ZL_SceneManager::GoToScene(SCENE_TEAM);
}

#ifdef SCENE_EDITOR
static void OnEditorPress()
{
	sndMenuMusic = ZL_Sound();
	sndSelect.Play();
	World.Init();
	ZL_SceneManager::GoToScene(SCENE_EDITOR);
}
#endif

struct sSceneMenu : public ZL_Scene
{
	sSceneMenu() : ZL_Scene(SCENE_MENU) { }

	void InitGlobal()
	{
		ZL_Display::PointerX = 0;
		ZL_Display::PointerY = 0;
	}

	int InitTransitionEnter(ZL_SceneType SceneTypeFrom, void* data)
	{
		//srfTitle = ZL_Surface("Data/title.png").SetDrawOrigin(ZL_Origin::Center);
		srfTitle1 = ZL_Surface("Data/titletasa.png").SetDrawOrigin(ZL_Origin::Center);
		srfTitle2 = ZL_Surface("Data/titlemerc.png").SetDrawOrigin(ZL_Origin::Center);
		srfTitle3 = ZL_Surface("Data/titlereck.png").SetDrawOrigin(ZL_Origin::Center);
		srf1GAMLogo = ZL_Surface("Data/1gamlogo.png").SetDrawOrigin(ZL_Origin::Center).SetTextureFilterMode().SetScale(2);
		srfHole = ZL_Surface("Data/hole.png");
		#ifdef SCENE_EDITOR //DEBUGMODE ONLY SCENE
		recStart    = ZL_Rectf(ZLHALFW - 250, 180, ZLHALFW + 250, 230);
		recContinue = ZL_Rectf(ZLHALFW - 250, 115, ZLHALFW + 250, 165);
		recEditor   = ZL_Rectf(ZLHALFW - 250, 50, ZLHALFW + 250, 100);
		#else
		recStart    = ZL_Rectf(ZLHALFW - 250, 150, ZLHALFW + 250, 210);
		recContinue = ZL_Rectf(ZLHALFW - 250,  70, ZLHALFW + 250, 130);
		#endif

		if (!sndMenuMusic)
		{
			sndMenuMusic = ZL_Sound("Data/menu.ogg");
			sndMenuMusic.Play(true);
		}

		has_save = ZL_Application::SettingsHas("SAVEGAME");

		return 400;
	}

	int DeInitTransitionLeave(ZL_SceneType SceneTypeTo)
	{
		ZL_Display::AllSigDisconnect(this);
		return 400;
	}

	void DeInitAfterTransition()
	{
		//srfTitle = ZL_Surface();
		srfTitle1 = ZL_Surface();
		srfTitle2 = ZL_Surface();
		srfTitle3 = ZL_Surface();
		srf1GAMLogo = ZL_Surface();
	}

	void InitAfterTransition()
	{
		ZL_Display::sigPointerUp.connect(this, &sSceneMenu::OnPointerUp);
		ZL_Display::sigKeyDown.connect(this, &sSceneMenu::OnKeyDown);
	}

	void OnPointerUp(ZL_PointerPressEvent& e)
	{
		if      (recStart.Contains(e)) OnStartPress();
		else if (recContinue.Contains(e)) OnContinuePress();
		#ifdef SCENE_EDITOR
		else if (recEditor.Contains(e)) OnEditorPress();
		#endif
		else if (e.x >= ZLFROMW(300) && e.y >= ZLFROMH(45)) ZL_Application::OpenExternalUrl("http://www.onegameamonth.com/");
		else if (e.y <= 40) ZL_Application::OpenExternalUrl("https://zillalib.github.io/");
	}

	void OnKeyDown(ZL_KeyboardEvent& e)
	{
		if (e.key == ZLK_ESCAPE) ZL_Application::Quit();
	}

	void Draw()
	{
		scalar scale = s(1)+ssin(s(ZLTICKS)*s(0.001))*s(0.02);

		ZL_Vector vecOffset((ZL_Display::PointerX-ZLHALFW)/ZLHALFW, (ZL_Display::PointerY-ZLHALFH)/ZLHALFH);
		static ZL_Vector vecBGMove; vecBGMove -= (vecOffset * ZLELAPSEDF(200));
		ZL_Vector bgzise = srfBGBrown.GetSize();
		ZL_Vector bgoffset = ((vecBGMove.VecMod(bgzise)) -= bgzise);
		bool inv = (ZLTICKS>>11)&1;
		(inv ? srfBGBrown : srfBGGreen).DrawTo(bgoffset.x, bgoffset.y, ZLWIDTH, ZLHEIGHT);
		(inv ? srfBGGreen : srfBGBrown).DrawTo(bgoffset.x, bgoffset.y, ZLWIDTH, ZLHEIGHT, ZLALPHA(s(ZLTICKS&((1<<11)-1))/s(1<<11)));

		scalar rot = s(ZLTICKS)/s(5000);
		srfPalm.SetScale(1+scale, 2+(1-scale));
		srfPalm.Draw(        15*scale , ZLFROMH(15*scale), rot);
		srfPalm.Draw(ZLFROMW(15*scale), ZLFROMH(15*scale), rot);
		srfPalm.Draw(        15*scale ,         15*scale , rot);
		srfPalm.Draw(ZLFROMW(15*scale),         15*scale , rot);
		srfPalm.SetScale(1);

		srfHole.DrawTo(0, 0, ZLWIDTH, ZLHEIGHT);

		ZL_Vector vecTitle1(ZLHALFW, ZLFROMH(45)), vecTitle2(ZLHALFW, ZLFROMH(120)), vecTitle3(ZLHALFW, ZLFROMH(205));
		srfTitle1.Draw(vecTitle1 - vecOffset*9, scale, scale, ZLLUMA(0,.5));
		srfTitle1.Draw(vecTitle1 - vecOffset*5, scale, scale);
		srfTitle2.Draw(vecTitle2 - vecOffset*8, scale, scale, ZLLUMA(0,.5));
		srfTitle2.Draw(vecTitle2 - vecOffset*3, scale, scale);
		srfTitle3.Draw(vecTitle3 - vecOffset*5, scale, scale, ZLLUMA(0,.5));
		srfTitle3.Draw(vecTitle3 - vecOffset*1, scale, scale);

		ZL_Display::PushMatrix();
		ZL_Display::Translate(ZLHALFW, ZLHALFH);
		ZL_Display::Rotate(scos(ZLTICKS*0.001)*0.02);
		ZL_Display::Scale(scale);
		ZL_Display::Translate(-ZLHALFW, -ZLHALFH);

		ZL_Display::DrawRect(recStart, ZLBLACK, ZLLUMA(0,.5));
		fntBig.Draw(recStart.MidX()-1, recStart.MidY()-5, "START GAME", ZLBLACK, ZL_Origin::Center);
		fntBig.Draw(recStart.MidX()-1, recStart.MidY()-3, "START GAME", ZLBLACK, ZL_Origin::Center);
		fntBig.Draw(recStart.MidX()+1, recStart.MidY()-5, "START GAME", ZLBLACK, ZL_Origin::Center);
		fntBig.Draw(recStart.MidX()+1, recStart.MidY()-3, "START GAME", ZLBLACK, ZL_Origin::Center);
		fntBig.Draw(recStart.MidX(), recStart.MidY()-4, "START GAME", ZLRGBA(.6,.4,.2,.8), ZL_Origin::Center);

		ZL_Display::DrawRect(recContinue, ZLBLACK, ZLLUMA(0,(has_save ? .5 : .3)));
		fntBig.Draw(recContinue.MidX()-1, recContinue.MidY()-5, "CONTINUE GAME", (has_save ? ZLBLACK : ZLLUMA(0, 0.3)), ZL_Origin::Center);
		fntBig.Draw(recContinue.MidX()-1, recContinue.MidY()-3, "CONTINUE GAME", (has_save ? ZLBLACK : ZLLUMA(0, 0.3)), ZL_Origin::Center);
		fntBig.Draw(recContinue.MidX()+1, recContinue.MidY()-5, "CONTINUE GAME", (has_save ? ZLBLACK : ZLLUMA(0, 0.3)), ZL_Origin::Center);
		fntBig.Draw(recContinue.MidX()+1, recContinue.MidY()-3, "CONTINUE GAME", (has_save ? ZLBLACK : ZLLUMA(0, 0.3)), ZL_Origin::Center);
		fntBig.Draw(recContinue.MidX(), recContinue.MidY()-4, "CONTINUE GAME", (has_save ? ZLRGBA(.6,.4,.2,.8) : ZLRGBA(.9,.9,.9,.2)), ZL_Origin::Center);

		#ifdef SCENE_EDITOR
		ZL_Display::DrawRect(recEditor, ZLBLACK, ZLLUMA(0,.5));
		fntBig.Draw(recEditor.MidX()-1, recEditor.MidY()-5, "EDITOR", ZLBLACK, ZL_Origin::Center);
		fntBig.Draw(recEditor.MidX()-1, recEditor.MidY()-3, "EDITOR", ZLBLACK, ZL_Origin::Center);
		fntBig.Draw(recEditor.MidX()+1, recEditor.MidY()-5, "EDITOR", ZLBLACK, ZL_Origin::Center);
		fntBig.Draw(recEditor.MidX()+1, recEditor.MidY()-3, "EDITOR", ZLBLACK, ZL_Origin::Center);
		fntBig.Draw(recEditor.MidX(), recEditor.MidY()-4, "EDITOR", ZLRGBA(.6,.4,.2,.8), ZL_Origin::Center);
		#endif

		ZL_Display::PopMatrix();

		srf1GAMLogo.Draw(ZLFROMW(srf1GAMLogo.GetWidth()+10)-vecOffset.x*3, ZLFROMH(18)-vecOffset.y*3, ZLLUMA(0,.5));
		srf1GAMLogo.Draw(ZLFROMW(srf1GAMLogo.GetWidth()+10)              , ZLFROMH(18)             , ZLALPHA(0.8));

		fntMain.Draw(ZLHALFW+1, 20-1, "(c) 2013 Bernhard Schelling, Leonard Wyrsch, Adrian Keller - Nukular Design", ZL_Color::Black, ZL_Origin::Center);
		fntMain.Draw(ZLHALFW+1, 20+1, "(c) 2013 Bernhard Schelling, Leonard Wyrsch, Adrian Keller - Nukular Design", ZL_Color::Black, ZL_Origin::Center);
		fntMain.Draw(ZLHALFW-1, 20-1, "(c) 2013 Bernhard Schelling, Leonard Wyrsch, Adrian Keller - Nukular Design", ZL_Color::Black, ZL_Origin::Center);
		fntMain.Draw(ZLHALFW-1, 20+1, "(c) 2013 Bernhard Schelling, Leonard Wyrsch, Adrian Keller - Nukular Design", ZL_Color::Black, ZL_Origin::Center);
		fntMain.Draw(ZLHALFW, 20, "(c) 2013 Bernhard Schelling, Leonard Wyrsch, Adrian Keller - Nukular Design", ZL_Origin::Center);
	}
} SceneMenu;
