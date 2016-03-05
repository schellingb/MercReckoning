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

static ZL_Vector scroll;
static ZL_Rect screen;
static scalar zoom;
static ZL_Surface srfUISideBar, srfUIActiveDude, srfUIInactiveDude, srfUIMenuButton, srfUIModeButton, srfUIIcons;
//static unsigned int NextSceneDelay;
static const scalar SideBarWidth = s(165);
static scalar SideBarScroll = 0;
static bool ResumeAfterDragPath = false, ClearPathOnDrag = false;
static scalar SideBarSlide, RewardSlide, Fade;
static int RewardDisplayCount;
static ZL_Rectf recContinue;
static bool MenuVisible = false;

static void MenuToggle();
static void MenuDraw();
static bool MenuClick(ZL_Vector pos);

static void FinishContinuePress()
{
	bool stillDudesLeft = false;
	for (vector<sTeamMember*>::iterator itDude = World.Dudes.begin(); itDude != World.Dudes.end(); ++itDude)
		if ((*itDude)->Health > 0 && ((*itDude)->hired || (*itDude)->Price <= World.Player.Budget)) { stillDudesLeft = true; break; }

	sndSelect.Play();
	if (World.RoundHasBeenWon && World.Round == sWorld::FinalRound) { World.SaveGame(); ZL_SceneManager::GoToScene(SCENE_CLEARED); } //cleared
	else if (World.RoundHasBeenWon && World.Round < World.RoundHighest) { World.Round++;  ZL_SceneManager::GoToScene(SCENE_TEAM); } //win
	else if (!stillDudesLeft) ZL_SceneManager ::GoToScene(SCENE_MENU); // game over
	else ZL_SceneManager::GoToScene(SCENE_TEAM); //lose
}

struct sSceneGame : public ZL_Scene
{
	sSceneGame() : ZL_Scene(SCENE_GAME) { }

	void InitGlobal()
	{
		srfUISideBar = ZL_Surface("Data/ui_sidebar.png").SetTextureRepeatMode();
		srfUIActiveDude = ZL_Surface("Data/ui_activedude.png");
		srfUIInactiveDude = ZL_Surface("Data/ui_inactivedude.png");
		srfUIMenuButton = ZL_Surface("Data/ui_menubutton.png");
		srfUIModeButton = ZL_Surface("Data/ui_modebutton.png").SetTilesetClipping(1, 3);
		srfUIIcons = ZL_Surface("Data/ui_icons.png").SetTilesetClipping(4, 5);
	}

	void InitEnter(ZL_SceneType SceneTypeFrom, void* data)
	{
		#ifdef SCENE_EDITOR
		if (SceneTypeFrom != SCENE_EDITOR)
		#endif
		World.StartRound();
		//NextSceneDelay = 0;
		ActivePlayerFigure = World.Player.Team[0];
		zoom = s(1);
		scroll = ZL_Vector(World.PlacementArea.MidX() - ZLFROMW(SideBarWidth)/2, World.PlacementArea.MidY() - ZLHALFH);
		scroll.x = MIN(World.Width - ZLFROMW(SideBarWidth), MAX(0, scroll.x));
		scroll.y = MIN(World.Height - ZLHEIGHT, MAX(0, scroll.y));
		UpdateScreen();
	}

	int InitTransitionEnter(ZL_SceneType SceneTypeFrom, void* data)
	{
		World.ShowHintOnce(sWorld::HINT_PLACEMENT, 250);
		ZL_Timer::AddTransitionScalar(&(Fade = 1), 0, 300);
		ZL_Timer::AddTransitionScalar(&(SideBarSlide = 0), 1, 500);
		return 500;
	}

	void InitAfterTransition()
	{
		ZL_TouchInput::ScrollMode = ZL_TouchInput::SCROLL_DUAL_OR_RIGHTMB;
		ZL_TouchInput::sigClick.connect(this, &sSceneGame::Click);
		ZL_TouchInput::sigScroll.connect(this, &sSceneGame::Scroll);
		ZL_TouchInput::sigZoom.connect(this, &sSceneGame::Zoom);
		ZL_TouchInput::sigDrag.connect(this, &sSceneGame::Drag);
		ZL_TouchInput::sigDragEnd.connect(this, &sSceneGame::DragEnd);
		ZL_Display::sigKeyDown.connect(this, &sSceneGame::OnKeyDown);
		ZL_Display::sigActivated.connect(this, &sSceneGame::OnActivated);
	}

	int DeInitTransitionLeave(ZL_SceneType SceneTypeTo)
	{
		ZL_Display::AllSigDisconnect(this);
		ZL_TouchInput::AllSigDisconnect(this);
		ZL_Timer::AddTransitionScalar(&(SideBarSlide = 1), 0, 500);
		ZL_Timer::AddTransitionScalar(&(Fade = 0), 1, 300, 200);
		return 500;
	}

	void DeInitAfterTransition()
	{
		ZL_Timer::EndTransitionFor(&RewardSlide);
	}

	void UpdateScreen()
	{
		screen = ZL_Rectf(scroll, ZLWIDTH*zoom, ZLHEIGHT*zoom);
	}

	void Click(const ZL_Vector& p, ZL_TouchInput::eClickResult& ClickResult)
	{
		if (World.HintVisible) { World.ShowHintOnce(sWorld::HINT_NONE); return; }
		if (World.RoundEnded)
		{
			if (recContinue.Contains(p)) FinishContinuePress();
			return;
		}
		if (MenuClick(p)) { ClickResult = ZL_TouchInput::CANCEL_DRAG; return; }
		if (p.x >= ZLFROMW(SideBarWidth)) { SideBarClick(p); ClickResult = ZL_TouchInput::START_SCROLL; return; }
		ZL_Vector pos = scroll + p*zoom;
		if (World.Mode == sWorld::MODE_PLACEMENT)
		{
			if (World.PlaceDude(ActivePlayerFigure, pos))
			{
				ActivePlayerFigure = NULL;
				for (vector<cAnimatedFigure*>::iterator it = World.Player.Team.begin(); it != World.Player.Team.end(); ++it)
					if (!(*it)->is_placed) { ActivePlayerFigure = *it; break; }
			}
			ClickResult = ZL_TouchInput::CANCEL_DRAG;
		}
		else if (World.Mode == sWorld::MODE_PAUSED && ActivePlayerFigure)
		{
			ResumeAfterDragPath = false;
			cAnimatedFigure* new_ActivePlayerFigure = NULL;
			for (vector<cAnimatedFigure*>::iterator it = World.Player.Team.begin(); it != World.Player.Team.end(); ++it)
				if ((*it)->Health && (pos - (*it)->Pos) < ((*it)->HitBoxRadius + 5*zoom))
					new_ActivePlayerFigure = *it;
			if (new_ActivePlayerFigure)
			{
				ActivePlayerFigure = new_ActivePlayerFigure;
				ClearPathOnDrag = true;
			}
			else ActivePlayerFigure->AddPath(pos, true);
		}
		else if (World.Mode == sWorld::MODE_PAUSED)
		{
			ResumeAfterDragPath = false;
			for (vector<cAnimatedFigure*>::iterator it = World.Player.Team.begin(); it != World.Player.Team.end(); ++it)
				if ((*it)->Health && (pos - (*it)->Pos) < ((*it)->HitBoxRadius + (ActivePlayerFigure ? 5 : 15)*zoom))
					ActivePlayerFigure = *it;
			if (ActivePlayerFigure) ClearPathOnDrag = true;
			else ClickResult = ZL_TouchInput::START_SCROLL;
		}
		else if (World.Mode == sWorld::MODE_RUNNING)
		{
			for (vector<cAnimatedFigure*>::iterator it = World.Player.Team.begin(); it != World.Player.Team.end(); ++it)
				if ((*it)->Health && (pos - (*it)->Pos) < ((*it)->HitBoxRadius + 15*zoom))
					ActivePlayerFigure = *it;
			if (ActivePlayerFigure)
			{
				ResumeAfterDragPath = true;
				ClearPathOnDrag = true;
				World.SwitchMode(sWorld::MODE_PAUSED);
			}
			else ClickResult = ZL_TouchInput::START_SCROLL;
		}
	}

	void Drag(const ZL_Vector& startpos, const ZL_Vector& p, const ZL_Vector& dragamount)
	{
		if (World.RoundEnded || World.HintVisible) return;
		if (startpos.x >= ZLFROMW(SideBarWidth)) return;
		if (World.Mode == sWorld::MODE_PAUSED && ActivePlayerFigure)
		{
			ZL_Vector pos = scroll + p*zoom;
			ActivePlayerFigure->AddPath(pos, ClearPathOnDrag);
			ClearPathOnDrag = false;
		}
	}

	void DragEnd(const ZL_Vector& startpos, const ZL_Vector& endpos)
	{
		if (World.RoundEnded || World.HintVisible) return;
		if (startpos.x >= ZLFROMW(SideBarWidth)) return;
		if (ResumeAfterDragPath)
		{
			World.SwitchMode(sWorld::MODE_RUNNING);
			ActivePlayerFigure = NULL;
			ResumeAfterDragPath = false;
		}
	}

	void Scroll(const ZL_Vector& startpos, const ZL_Vector& scrollamount)
	{
		if (World.RoundEnded || World.HintVisible) return;
		if (startpos.x >= ZLFROMW(SideBarWidth)) { /*SideBarScroll = MAX(0, SideBarScroll-scrollamount.y);*/ return; }
		scroll -= scrollamount*zoom;
		UpdateScreen();
	}

	void Zoom(const ZL_Vector& startpos, scalar factor, const ZL_Vector& originp)
	{
		if (World.RoundEnded || World.HintVisible) return;
		if (startpos.x >= ZLFROMW(SideBarWidth)) return;
		if ((zoom <= s(0.1) && factor < 1) || (zoom >= s(12) && factor > 1)) return;
		scroll += (originp*zoom*(-factor+1));
		zoom *= factor;
		UpdateScreen();
	}

	void OnKeyDown(ZL_KeyboardEvent& e)
	{
		if (World.HintVisible) { World.ShowHintOnce(sWorld::HINT_NONE); return; }
		if (World.RoundEnded) { FinishContinuePress(); return; }
		if (e.key == ZLK_ESCAPE) { MenuToggle(); }
		if ((e.key == ZLK_KP_PLUS || e.key == ZLK_PAGEDOWN) && (zoom > s(.1))) { scroll += (ZL_Vector(ZLFROMW(SideBarWidth)/2,ZLHALFH)*zoom*s( 0.2 )); zoom *= s(0.8 ); UpdateScreen(); }
		if ((e.key == ZLK_KP_MINUS ||  e.key == ZLK_PAGEUP) && (zoom < s(12))) { scroll += (ZL_Vector(ZLFROMW(SideBarWidth)/2,ZLHALFH)*zoom*s(-0.25)); zoom *= s(1.25); UpdateScreen(); }

		#ifdef SCENE_EDITOR
		if (e.key == ZLK_F5 || e.key == ZLK_VOLUMEDOWN || e.key == ZLK_VOLUMEUP) ZL_SceneManager::GoToScene(SCENE_EDITOR);
		#endif

		if (e.key == ZLK_SPACE && World.Mode != sWorld::MODE_PLACEMENT)
		{
			World.SwitchMode(World.Mode == sWorld::MODE_PAUSED ? sWorld::MODE_RUNNING : sWorld::MODE_PAUSED);
			ActivePlayerFigure = NULL;
			if (World.Mode == sWorld::MODE_RUNNING) World.ShowHintOnce(sWorld::HINT_PLAYING); //world will pause game while hint is visible
		}
	}

	void OnActivated(ZL_WindowActivateEvent& e)
	{
		if (World.Mode == sWorld::MODE_RUNNING && (e.minimized || !e.key_focus || !e.mouse_focus))
			World.SwitchMode(sWorld::MODE_PAUSED);
	}

	void Calculate()
	{
		if (World.RoundEnded) return;
		World.Calculate();
		if (World.RoundEnded)
		{
			if (MenuVisible) MenuToggle(); //hide menu
			ActivePlayerFigure = NULL;
			RewardDisplayCount = -1;
		}
		if (ActivePlayerFigure && ActivePlayerFigure->Health == 0) ActivePlayerFigure = NULL;
	}

	void SideBarClick(const ZL_Vector& p)
	{
		scalar x = ZLFROMW(SideBarWidth);
		scalar y = ZLFROMH(47);
		if (p.y >= y)
		{
			if (p.x >= x+119) //menu button
			{
				MenuToggle();
			}
			else if (World.Mode != sWorld::MODE_PLACEMENT)
			{
				World.SwitchMode(World.Mode == sWorld::MODE_PAUSED ? sWorld::MODE_RUNNING : sWorld::MODE_PAUSED);
				ActivePlayerFigure = NULL;
				if (World.Mode == sWorld::MODE_RUNNING) World.ShowHintOnce(sWorld::HINT_PLAYING); //world will pause game while hint is visible
			}
			return;
		}
		y -= SideBarScroll;
		for (vector<cAnimatedFigure*>::iterator it = World.Player.Team.begin(); it != World.Player.Team.end(); ++it)
		{
			y -= 89;
			if (p.y < y) continue;
			if (World.Mode == sWorld::MODE_PLACEMENT && (*it)->is_placed) break;
			if ((*it)->Health == 0) break;
			ActivePlayerFigure = *it;
			if (World.Mode == sWorld::MODE_RUNNING) World.SwitchMode(sWorld::MODE_PAUSED);
			break;
		}
	}

	void DrawSideBar()
	{
		scalar x = ZLFROMW(SideBarWidth*SideBarSlide);
		scalar y = ZLFROMH(47);
		srfUISideBar.DrawTo(x-1, 0, ZLWIDTH, ZLHEIGHT);
		srfUIModeButton.SetTilesetIndex((int)World.Mode).Draw(x, y);
		srfUIMenuButton.Draw(x+119, y);
		y -= SideBarScroll;
		for (vector<cAnimatedFigure*>::iterator it = World.Player.Team.begin(); it != World.Player.Team.end(); ++it)
		{
			y -= 89;
			cAnimatedFigure &dude = *(*it);
			if (World.Mode == sWorld::MODE_RUNNING) srfUIInactiveDude.Draw(x, y);
			if (World.Mode != sWorld::MODE_RUNNING && *it == ActivePlayerFigure) srfUIActiveDude.Draw(x, y);
			dude.Picture.DrawTo(x+9, y+28, x+61, y+85);
			fntMain.Draw(x+34, y+15, dude.Name, ZL_Origin::Center);

			ZL_Display::DrawRect(x+66, y+71, x+66+(s(95)*dude.Health/dude.HealthMax), y+71+14, ZLTRANSPARENT, ZLRGBA(1,0,0,.5));
			if (dude.Weapon.MaxAmmo > 0)
				 ZL_Display::DrawRect(x+66, y+54, x+66+(s(95)*s(dude.Weapon.Ammo)/s(dude.Weapon.MaxAmmo)), y+54+14, ZLTRANSPARENT, ZLRGBA(0,0,1,.7));
			else ZL_Display::DrawRect(x+66, y+54, x+66+95                                                  , y+54+14, ZLTRANSPARENT, ZLRGBA(0,0,0,.4));

			int iActionTile = (int)dude.Action;
			if (World.Mode == sWorld::MODE_PLACEMENT) iActionTile =  (ActivePlayerFigure == &dude ? 5 : 6);
			srfUIIcons.SetTilesetIndex(iActionTile).Draw(x+66, y+6);
			srfUIIcons.SetTilesetIndex(8+(int)dude.Weapon.Type).Draw(x+116, y+6);

			fntMain.Draw(x+s(66)+s(95)/s(2), y+71+6, ZL_String((int)dude.Health) << '/' << dude.HealthMax, s(0.8), ZL_Origin::Center);
			if (dude.Weapon.MaxAmmo > 0)
				fntMain.Draw(x+s(66)+s(95)/s(2), y+54+6, ZL_String(dude.Weapon.Ammo) << '/' << dude.Weapon.MaxAmmo, s(0.8), ZL_Origin::Center);
			else
				fntMain.Draw(x+s(66)+s(95)/s(2), y+54+6, dude.Weapon.Name, s(0.8), ZL_Origin::Center);

			if (World.Mode != sWorld::MODE_RUNNING && *it != ActivePlayerFigure) srfUIInactiveDude.Draw(x, y);
		}
	}

	void DrawFinishScreen()
	{
		if (RewardDisplayCount == -1 || RewardSlide == 0)
		{
			RewardDisplayCount++;
			ZL_Timer::AddTransitionScalar(&(RewardSlide = -1), 0, 250);
			scalar btnx = ZLFROMW(SideBarWidth)/s(2);
			scalar btny = 120; //ZLFROMH(80) - 100 - World.RewardCounts.size()*45;
			recContinue = ZL_Rectf(btnx - 200, btny - 110, btnx + 200, btny - 40);
		}

		scalar fadebg = (RewardDisplayCount == 0 ? (1+RewardSlide) : 1);
		srfBGGreen.DrawTo(0, 0, ZLWIDTH, ZLHEIGHT, ZLALPHA(fadebg*0.5));

		bool has_rewards = (World.Player.RewardCounts.size() > 0 || World.RoundBonus);
		scalar y = ZLFROMH(has_rewards ? 80 : 200);  //center in screen if no rewards are to be displayed

		bool stillDudesLeft = false;
		for (vector<sTeamMember*>::iterator itDude = World.Dudes.begin(); itDude != World.Dudes.end(); ++itDude)
			if ((*itDude)->Health > 0 && ((*itDude)->hired || (*itDude)->Price <= World.Player.Budget)) { stillDudesLeft = true; break; }

		scalar offmsg = (RewardDisplayCount == 0 ? ZLWIDTH*RewardSlide : 0);
		const char *roundEndMessage = (!stillDudesLeft ? "GAME OVER" : (World.RoundHasBeenWon ? (World.Round == sWorld::FinalRound ? "YOU WIN!!!" : "YOU WIN") : "YOU LOSE"));
		fntBig.Draw(offmsg+ZLHALFW-(SideBarWidth/2)+5, y-5, roundEndMessage, 2.1f, ZL_Color::Black, ZL_Origin::Center);
		fntBig.Draw(offmsg+ZLHALFW-(SideBarWidth/2)  , y  , roundEndMessage, 2.1f, ZL_Color::Red, ZL_Origin::Center);

		scalar fadecont = (RewardDisplayCount == 0 ? (1+RewardSlide)*s(0.5) : (RewardDisplayCount == 1 ? s(0.5)+(1+RewardSlide)*s(0.5) : 1));
		ZL_Display::DrawRect(recContinue, ZLLUMA(0, fadecont), ZLLUMA(0,.5*fadecont));

		const char* btn = (!stillDudesLeft ? "GAME OVER" : (World.RoundHasBeenWon && World.Round == sWorld::FinalRound ? "FINISH" : "CONTINUE"));
		fntBig.Draw(recContinue.MidX()-1, recContinue.MidY()-5, btn, ZLLUMA(0, fadecont), ZL_Origin::Center);
		fntBig.Draw(recContinue.MidX()-1, recContinue.MidY()-3, btn, ZLLUMA(0, fadecont), ZL_Origin::Center);
		fntBig.Draw(recContinue.MidX()+1, recContinue.MidY()-5, btn, ZLLUMA(0, fadecont), ZL_Origin::Center);
		fntBig.Draw(recContinue.MidX()+1, recContinue.MidY()-3, btn, ZLLUMA(0, fadecont), ZL_Origin::Center);
		fntBig.Draw(recContinue.MidX(), recContinue.MidY()-4, btn, ZLALPHA(fadecont), ZL_Origin::Center);

		if (!has_rewards) return;

		y -= 100;
		int i = 0, tcount = 0, ttotal = 0;
		for (map<ZL_String, sPlayer::sRewardCount>::iterator it = World.Player.RewardCounts.begin(); i < RewardDisplayCount; i++)
		{
			const char *name;
			ZL_String num("X"), total;
			bool drawline = false;
			if (it == World.Player.RewardCounts.end())
			{
				int over = (i - (int)World.Player.RewardCounts.size());
				if (over == 0 && World.RoundBonus)
				{
					name = "CLEAR BONUS";
					num = "";
					total << World.RoundBonus << "S";
					tcount++;
					ttotal += World.RoundBonus;
				}
				else if ((over == 1 && World.RoundBonus) || (over == 0))
				{
					if (tcount == 0) break;
					name = "TOTAL";
					num << tcount;
					total << ttotal << "S";
					y -= 20;
					drawline = true;
				}
				else break;
			}
			else
			{
				name = it->first.c_str();
				num << it->second.count;
				total << (it->second.count * it->second.value) << "S";
				tcount += it->second.count;
				ttotal += (it->second.count * it->second.value);
				++it;
			}
			scalar offnm = (i+1 == RewardDisplayCount ? ZLWIDTH*RewardSlide : 0);
			scalar offnu = (i+1 == RewardDisplayCount ? ZLWIDTH*MIN(0,RewardSlide+0.05) : 0);
			scalar offto = (i+1 == RewardDisplayCount ? ZLWIDTH*MIN(0,RewardSlide+0.1) : 0);
			fntBig.Draw(offnm+ZLWIDTH*s(0.01)+2, y-2, name, ZLBLACK, ZL_Origin::BottomLeft);
			fntBig.Draw(offnu+ZLWIDTH*s(0.50)+2, y-2, num, ZLBLACK, ZL_Origin::BottomRight);
			fntBig.Draw(offto+ZLWIDTH*s(0.78)+2, y-2, total, ZLBLACK, ZL_Origin::BottomRight);
			ZL_Display::DrawRect(offto+ZLWIDTH*s(0.78)-10+2, y-5-2, offto+ZLWIDTH*s(0.78)-15+2, y+42-2, ZLTRANSPARENT, ZLBLACK);
			fntBig.Draw(offnm+ZLWIDTH*s(0.01), y, name, ZLWHITE, ZL_Origin::BottomLeft);
			fntBig.Draw(offnu+ZLWIDTH*s(0.50), y, num, ZLWHITE, ZL_Origin::BottomRight);
			fntBig.Draw(offto+ZLWIDTH*s(0.78), y, total, ZLWHITE, ZL_Origin::BottomRight);
			ZL_Display::DrawRect(offto+ZLWIDTH*s(0.78)-10, y-5, offto+ZLWIDTH*s(0.78)-15, y+42, ZLTRANSPARENT, ZLWHITE);
			if (drawline)
			{
				ZL_Display::DrawRect(offto+5+2, y+50-2, offto+ZLWIDTH*s(0.78)+5+2, y+54-2, ZLTRANSPARENT, ZLBLACK);
				ZL_Display::DrawRect(offto+5, y+50, offto+ZLWIDTH*s(0.78)+5, y+54, ZLTRANSPARENT, ZLWHITE);
			}
			y -= 45;
		}
	}

	void Draw()
	{
		ZL_Display::ClearFill(ZL_Color::Black);
		ZL_Display::PushOrtho(screen);
		ZL_Display::FillGradient(0, 0, ZLWIDTH, ZLHEIGHT, ZLRGB(0,0,.3), ZLRGB(0,0,.3), ZLRGB(.4,.4,.4), ZLRGB(.4,.4,.4));
		World.Draw();
		ZL_Display::PopOrtho();
		DrawSideBar();
		MenuDraw();
		if (World.RoundEnded) DrawFinishScreen();
		if (Fade) ZL_Display::DrawRect(0, 0, ZLWIDTH, ZLHEIGHT, ZLTRANSPARENT, ZLLUMA(0, Fade));
		if (World.HintVisible) World.DrawHint();
	}
} SceneGame;

static scalar MenuFade;
static ZL_Rectf recMenu, recMenuBtnResume, recMenuBtnSaveQuit, recMenuBtnRetreat;

void MenuToggle()
{
	if (!MenuVisible && MenuFade) return;
	sndSelect.Play();
	if (MenuVisible)
	{
		MenuVisible = false;
		ZL_Timer::AddTransitionFloat(&(MenuFade = 1), 0, 500);
	}
	else
	{
		if (World.Mode == sWorld::MODE_RUNNING) World.SwitchMode(sWorld::MODE_PAUSED);
		MenuVisible = true;
		recMenu = ZL_Rectf(ZLHALFW, ZLHALFH, ZL_Vector(150, 150));
		recMenuBtnResume = ZL_Rectf(recMenu.left + 10, recMenu.low + 110, recMenu.right - 10, recMenu.low + 150);
		recMenuBtnSaveQuit = ZL_Rectf(recMenu.left + 10, recMenu.low + 60, recMenu.right - 10, recMenu.low + 100);
		recMenuBtnRetreat = ZL_Rectf(recMenu.left + 10, recMenu.low + 10, recMenu.right - 10, recMenu.low + 50);
		ZL_Timer::AddTransitionFloat(&(MenuFade = 0), 1, 500);
	}
}

void MenuDraw()
{
	if (!MenuVisible && !MenuFade) return;
	ZL_Display::DrawRect(-10000, -10000, 10000, 10000, ZLTRANSPARENT, ZLLUMA(0, MenuFade/2));
	ZL_Display::PushMatrix();
	ZL_Display::Translate(ZLHALFW, ZLHALFH);
	ZL_Display::Scale(MenuFade+ssin(ZLTICKS*0.001)*0.02);
	ZL_Display::Rotate(scos(ZLTICKS*0.001)*0.02);
	ZL_Display::Translate(-ZLHALFW, -ZLHALFH);

	ZL_Display::DrawRect(recMenu.left-2, recMenu.low-2, recMenu.right+2, recMenu.high+2, ZLBLACK, ZLBLACK);
	srfBGBrown.DrawTo(recMenu);
	fntBig.Draw(recMenu.MidX()+2, recMenu.high-40-2, "RESTING", ZLBLACK, ZL_Origin::Center);
	fntBig.Draw(recMenu.MidX(), recMenu.high-40, "RESTING", ZL_Origin::Center);
	ZL_Display::DrawRect(recMenu.left+30+2, recMenu.high-70-2, recMenu.right-30+2, recMenu.high-72-2, ZLTRANSPARENT, ZLBLACK);
	ZL_Display::DrawRect(recMenu.left+30, recMenu.high-70, recMenu.right-30, recMenu.high-72, ZLTRANSPARENT, ZLWHITE);
	int enemiesLeft = 0;
	for (vector<sPlayer*>::iterator itEnemy = World.Players.begin(); itEnemy != World.Players.end(); ++itEnemy)
		if (*itEnemy != &World.Player) enemiesLeft += (int)(*itEnemy)->Team.size();
	fntMain.Draw(recMenu.MidX()+2, recMenu.high-90-2, ZL_String("Enemies left: ") << enemiesLeft, ZLBLACK, ZL_Origin::Center);
	fntMain.Draw(recMenu.MidX(), recMenu.high-90, ZL_String("Enemies left: ") << enemiesLeft, ZL_Origin::Center);
	fntMain.Draw(recMenu.MidX()+2, recMenu.high-120-2, ZL_String("Money: ") << World.Player.Budget, ZLBLACK, ZL_Origin::Center);
	fntMain.Draw(recMenu.MidX(), recMenu.high-120, ZL_String("Money: ") << World.Player.Budget, ZL_Origin::Center);

	ZL_Display::DrawRect(recMenuBtnResume, ZL_Color::Black, ZLLUMA(0,.5));
	fntMain.Draw(recMenuBtnResume.Center(), "Resume", ZL_Origin::Center);
	ZL_Display::DrawRect(recMenuBtnSaveQuit, ZL_Color::Black, ZLLUMA(0,.5));
	fntMain.Draw(recMenuBtnSaveQuit.Center(), "Save & Quit", ZL_Origin::Center);
	ZL_Display::DrawRect(recMenuBtnRetreat, ZL_Color::Black, ZLLUMA(0,.5));
	fntMain.Draw(recMenuBtnRetreat.Center(), "Retreat!", ZL_Origin::Center);

	ZL_Display::PopMatrix();

}

bool MenuClick(ZL_Vector pos)
{
	if (!MenuVisible) return false;
	if (MenuFade != 1) return true;
	if (!recMenu.Contains(pos) || recMenuBtnResume.Contains(pos))
	{
		MenuToggle();
	}
	else if (recMenuBtnSaveQuit.Contains(pos))
	{
		MenuFade = 0;
		MenuVisible = false;
		ZL_SceneManager::GoToScene(SCENE_MENU);
	}
	else if (recMenuBtnRetreat.Contains(pos))
	{
		MenuFade = 0;
		MenuVisible = false;
		ZL_SceneManager::GoToScene(SCENE_TEAM);
	}
	return true;
}
