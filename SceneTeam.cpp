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

static sTeamMember* pSelectedDude;
static bool SelectedDudeIsInMyDudes;
static ZL_Rectf recRoundInfo, recDudeGrid, recDudeDetail, recMyDudes, recBtnHire, recMoney, recBtnOK, recBtnQuit, recBtnRoundPrev, recBtnRoundNext, recBtnHeal, recBtnAmmo;
static int DudesPerRow;
static scalar DudeWidth, DudeHeight, MyDudeWidth, Fade;
static const size_t maximumTeamSize = 5;
static ZL_String RoundTitle;
static ZL_Sound sndBuy;

void OnTeamReady()
{
	if (World.Player.Team.size() && World.Player.Team[0]->FigureType == cAnimatedFigure::TEAMMEMBER)
		for (vector<cAnimatedFigure*>::iterator it = World.Player.Team.begin(); it != World.Player.Team.end(); ++it)
		{
			((sTeamMember*)*it)->hired = true;
			(*it)->Owner = &World.Player;
		}
	ZL_SceneManager::GoToScene(SCENE_GAME);
}

void OnDudeClicked()
{
	//ZL_LOG1("SCENE_GAME", "Toggled %s", pSelectedDude->Name.c_str());
	if (find(World.Player.Team.begin(), World.Player.Team.end(), pSelectedDude) != World.Player.Team.end())
	{
		if (!pSelectedDude->hired) World.Player.Budget += pSelectedDude->Price; // Recover money
		else
		{
			if (World.Player.Team.size() == 1) //when firing the last dude, check if there is enough money to buy a dude
			{
				bool canHireMoreDudes = false;
				for (vector<sTeamMember*>::iterator itDude = World.Dudes.begin(); itDude != World.Dudes.end(); ++itDude)
					if ((*itDude)->Health > 0 && (*itDude)->Price <= World.Player.Budget) { canHireMoreDudes = true; break; }
				if (!canHireMoreDudes) return; //would leave no money to continue playing, abort firing
			}
			pSelectedDude->hired = false; //fire
		}
		sndSelect.Play();
		World.Player.Team.erase(find(World.Player.Team.begin(), World.Player.Team.end(), pSelectedDude));
		SelectedDudeIsInMyDudes = false;
	}
	else if (World.Player.Team.size() < maximumTeamSize)
	{
		sndBuy.Play();
		World.Player.AddToTeam(pSelectedDude);
		World.Player.Budget -= World.Player.Team.back()->Price;
		SelectedDudeIsInMyDudes = true;
	}
}

void SelectRound(int Round = World.Round)
{
	if (Round <= 0 || World.RoundHighest < Round || Round > sWorld::FinalRound) return;
	World.Round = Round;
	RoundTitle = ZL_String("- Round #") << Round << ": " << World.GetRoundName() << " -";
}

struct sSceneTeam : public ZL_Scene
{
	sSceneTeam() : ZL_Scene(SCENE_TEAM) { }

	int InitTransitionEnter(ZL_SceneType SceneTypeFrom, void* data)
	{
		for (vector<cAnimatedFigure*>::iterator itDude = World.Player.Team.begin(); itDude != World.Player.Team.end();)
			if ((*itDude)->Health == 0) itDude = World.Player.Team.erase(itDude); // Remove dead dudes from MyDudes
			else ++itDude;
		if (SceneTypeFrom == SCENE_GAME) World.SaveGame();
		SelectRound();
		pSelectedDude = NULL;
		recRoundInfo    = ZL_Rectf(             20, ZLFROMH(60), ZLFROMW(200), ZLFROMH(20));
		recBtnRoundPrev = ZL_Rectf(             20, ZLFROMH(60),        20+44, ZLFROMH(20));
		recBtnRoundNext = ZL_Rectf(ZLFROMW(200+44), ZLFROMH(60), ZLFROMW(200), ZLFROMH(20));
		recDudeGrid     = ZL_Rectf(             20,         120, ZLFROMW(200), ZLFROMH(60));
		recMoney        = ZL_Rectf(   ZLFROMW(200), ZLFROMH(45),  ZLFROMW(20), ZLFROMH(20));
		recDudeDetail   = ZL_Rectf(   ZLFROMW(200),         120,  ZLFROMW(20), ZLFROMH(45));
		recBtnHire      = ZL_Rectf(   ZLFROMW(195),         125,  ZLFROMW(25), 165);
		recBtnHeal      = ZL_Rectf(   ZLFROMW(195),         170, ZLFROMW(113), 210);
		recBtnAmmo      = ZL_Rectf(   ZLFROMW(107),         170,  ZLFROMW(25), 210);
		recMyDudes      = ZL_Rectf(            150,          20, ZLFROMW(150), 120);
		recBtnOK        = ZL_Rectf(   ZLFROMW(150),          20,  ZLFROMW(20), 120);
		recBtnQuit      = ZL_Rectf(             20,          20,          150, 120);
		DudesPerRow = 4; //recDudeGrid.Width() / 130;
		DudeWidth = s(recDudeGrid.Width()-22) / s(DudesPerRow);
		DudeHeight = recDudeGrid.Height() / 2;
		MyDudeWidth = 80;
		World.ShowHintOnce(sWorld::HINT_TEAM, 400);
		ZL_Timer::AddTransitionScalar(&(Fade = 1), 0, 300);
		ZL_Timer::AddTransitionRectf(&recRoundInfo, ZL_Origin::TopCenter, true);
		ZL_Timer::AddTransitionRectf(&recBtnRoundPrev, ZL_Origin::TopCenter, true);
		ZL_Timer::AddTransitionRectf(&recBtnRoundNext, ZL_Origin::TopCenter, true);
		ZL_Timer::AddTransitionRectf(&recMoney, ZL_Origin::TopRight, true);
		ZL_Timer::AddTransitionRectf(&recDudeDetail, ZL_Origin::CenterRight, true);
		ZL_Timer::AddTransitionRectf(&recBtnQuit, ZL_Origin::BottomLeft, true);
		ZL_Timer::AddTransitionRectf(&recMyDudes, ZL_Origin::BottomCenter, true);
		ZL_Timer::AddTransitionRectf(&recBtnOK, ZL_Origin::BottomRight, true);
		ZL_Timer::AddTransitionRectf(&recDudeGrid, ZL_Origin::CenterLeft, true);
		return 800;
	}

	void InitAfterTransition()
	{
		sndBuy = ZL_Sound("Data/buy.ogg");
		ZL_TouchInput::ScrollMode = ZL_TouchInput::SCROLL_SINGULAR;
		ZL_TouchInput::sigClick.connect(this, &sSceneTeam::Click);
		//ZL_TouchInput::sigScroll.connect(this, &sSceneTeam::Scroll);
		ZL_Display::sigKeyDown.connect(this, &sSceneTeam::OnKeyDown);
	}

	int DeInitTransitionLeave(ZL_SceneType SceneTypeTo)
	{
		World.SaveGame();
		pSelectedDude = NULL;
		sndBuy = ZL_Sound();
		ZL_TouchInput::AllSigDisconnect(this);
		ZL_Display::AllSigDisconnect(this);
		ZL_Timer::AddTransitionRectf(&recRoundInfo, ZL_Origin::TopCenter);
		ZL_Timer::AddTransitionRectf(&recBtnRoundPrev, ZL_Origin::TopCenter);
		ZL_Timer::AddTransitionRectf(&recBtnRoundNext, ZL_Origin::TopCenter);
		ZL_Timer::AddTransitionRectf(&recMoney, ZL_Origin::TopRight);
		ZL_Timer::AddTransitionRectf(&recDudeDetail, ZL_Origin::CenterRight);
		ZL_Timer::AddTransitionRectf(&recBtnQuit, ZL_Origin::BottomLeft);
		ZL_Timer::AddTransitionRectf(&recMyDudes, ZL_Origin::BottomCenter);
		ZL_Timer::AddTransitionRectf(&recBtnOK, ZL_Origin::BottomRight);
		ZL_Timer::AddTransitionRectf(&recDudeGrid, ZL_Origin::CenterLeft);
		ZL_Timer::AddTransitionScalar(&(Fade = 0), 1, 300, 500);
		return 800;
	}

	void Click(const ZL_Vector& p, ZL_TouchInput::eClickResult& ClickResult)
	{
		if (World.HintVisible) { World.ShowHintOnce(sWorld::HINT_NONE); return; }
		if (pSelectedDude && pSelectedDude->Health && recBtnHire.Contains(p))
		{
			OnDudeClicked();
			return;
		}
		if (pSelectedDude && SelectedDudeIsInMyDudes && pSelectedDude->Health && pSelectedDude->Health < pSelectedDude->HealthMax && recBtnHeal.Contains(p) && World.Player.Budget >= sWorld::PricePerHealHP)
		{
			sndBuy.Play();
			int cost = (int)((pSelectedDude->HealthMax - pSelectedDude->Health)*s(sWorld::PricePerHealHP));
			if (cost > World.Player.Budget)
			{
				scalar heal = s(World.Player.Budget) / s(sWorld::PricePerHealHP);
				World.Player.Budget = 0;
				pSelectedDude->Health += heal;
			}
			else
			{
				World.Player.Budget -= cost;
				pSelectedDude->Health = pSelectedDude->HealthMax;
			}
			return;
		}
		if (pSelectedDude && SelectedDudeIsInMyDudes && pSelectedDude->Health && pSelectedDude->Weapon.Ammo < pSelectedDude->Weapon.MaxAmmo && recBtnAmmo.Contains(p) && World.Player.Budget >= (int)(s(0.99)+pSelectedDude->Weapon.AmmoPrice))
		{
			sndBuy.Play();
			int cost = (int)(s(pSelectedDude->Weapon.MaxAmmo - pSelectedDude->Weapon.Ammo)*pSelectedDude->Weapon.AmmoPrice);
			if (cost > World.Player.Budget)
			{
				int fill = (int)(s(World.Player.Budget) / pSelectedDude->Weapon.AmmoPrice);
				World.Player.Budget -= (int)(s(fill) * pSelectedDude->Weapon.AmmoPrice);
				pSelectedDude->Weapon.Ammo += fill;
			}
			else
			{
				World.Player.Budget -= cost;
				pSelectedDude->Weapon.Ammo = pSelectedDude->Weapon.MaxAmmo;
			}
			return;
		}
		if (recBtnRoundPrev.Contains(p) && World.Round > 1)
		{
			sndSelect.Play();
			SelectRound(World.Round - 1);
			return;
		}
		if (recBtnRoundNext.Contains(p) && World.Round < World.RoundHighest)
		{
			sndSelect.Play();
			SelectRound(World.Round + 1);
			return;
		}
		if (recBtnQuit.Contains(p))
		{
			sndSelect.Play();
			ZL_SceneManager::GoToScene(SCENE_MENU);
			return;
		}
		if (World.Player.Team.size() > 0 && World.Player.Budget >= 0 && recBtnOK.Contains(p))
		{
			sndSelect.Play();
			OnTeamReady();
			return;
		}
		for (size_t iDude = 0; iDude < World.Dudes.size(); iDude++)
		{
			scalar x = recDudeGrid.left + 22 + (iDude % DudesPerRow) * DudeWidth;
			scalar y = recDudeGrid.high - (iDude / DudesPerRow) * DudeHeight;
			if (ZL_Rectf(x, y - DudeHeight, x + DudeWidth, y).Contains(p))
			{
				sndSelect.Play();
				pSelectedDude = World.Dudes[iDude];
				SelectedDudeIsInMyDudes = (find(World.Player.Team.begin(), World.Player.Team.end(), pSelectedDude) != World.Player.Team.end());
				return;
			}
		}
		for (size_t iMyDude = 0; iMyDude < World.Player.Team.size(); iMyDude++)
		{
			scalar x = recMyDudes.left + 22 + (iMyDude * MyDudeWidth);
			if (ZL_Rectf(x, recMyDudes.low, x + MyDudeWidth, recMyDudes.high).Contains(p))
			{
				sndSelect.Play();
				pSelectedDude = (sTeamMember*)World.Player.Team[iMyDude];
				SelectedDudeIsInMyDudes = true;
				return;
			}
		}
	}

	//void Scroll(const ZL_Vector& startpos, const ZL_Vector& scrollamount)
	//{
	//}

	void OnKeyDown(ZL_KeyboardEvent& e)
	{
		if (World.HintVisible) { World.ShowHintOnce(sWorld::HINT_NONE); return; }
		if (e.key == ZLK_ESCAPE) ZL_SceneManager::GoToScene(SCENE_MENU);
		if (World.Player.Team.size() > 0 && World.Player.Budget >= 0 && (e.key == ZLK_RETURN || e.key == ZLK_SPACE)) OnTeamReady();
	}

	void Draw()
	{
		//BACKGROUND
		static ZL_Vector vecBGMove; vecBGMove += (ZL_Vector(10, 10) * ZLELAPSED);
		ZL_Vector bgzise = srfBGBrown.GetSize();
		ZL_Vector bgoffset = ((vecBGMove.VecMod(bgzise)) -= bgzise);
		srfBGBrown.DrawTo(bgoffset.x, bgoffset.y, ZLWIDTH, ZLHEIGHT);

		//AREA RECTS
		ZL_Display::DrawRect(recRoundInfo, ZL_Color::Black, ZLLUMA(0,.3));
		ZL_Display::DrawRect(recDudeGrid, ZL_Color::Black, ZLLUMA(0,.3));
		ZL_Display::DrawRect(recMoney, ZL_Color::Black, ZLRGBA(.4,.3,0,.3));
		ZL_Display::DrawRect(recDudeDetail, ZL_Color::Black, ZLLUMA(0,(pSelectedDude ? .3 : .1)));
		ZL_Display::DrawRect(recMyDudes, ZL_Color::Black, ZLLUMA(0,.3));
		ZL_Display::DrawRect(recBtnQuit, ZL_Color::Black, ZLRGBA(.3,0,0,.3));
		ZL_Display::DrawRect(recBtnOK, ZL_Color::Black, (World.Player.Team.size() && World.Player.Budget >= 0 ? ZLRGBA(0,.3,0,.3) : (World.Player.Budget < 0 ?  ZLRGBA(.3,0,0,.3) : ZLLUMA(0,.3))));
		//fntMain.Draw(recDudeGrid.MidX()+2, recDudeGrid.high + 20 - 2, "- Manage Your Team - ", s(1.3), s(1.3), ZL_Color::Black, center);
		//fntMain.Draw(recDudeGrid.MidX(), recDudeGrid.high + 20, "- Manage Your Team - ", s(1.3), s(1.3), center);

		//ROUND INFO
		fntMain.Draw(recRoundInfo.MidX()+2, recRoundInfo.MidY(), RoundTitle, s(1.3), s(1.3), ZL_Color::Black, ZL_Origin::Center);
		fntMain.Draw(recRoundInfo.MidX(),   recRoundInfo.MidY(), RoundTitle, s(1.3), s(1.3), ZL_Origin::Center);
		if (World.Round > 1)
		{
			ZL_Display::DrawRect(recBtnRoundPrev, ZL_Color::Black, ZLALPHA(0.3));
			ZL_Display::DrawTriangle(ZL_Vector(recBtnRoundPrev.left+5, recBtnRoundPrev.MidY()), ZL_Vector(recBtnRoundPrev.right-5, recRoundInfo.low+5), ZL_Vector(recBtnRoundPrev.right-5, recRoundInfo.high-5), ZL_Color::Black, ZL_Color::White);
		}
		if (World.Round < World.RoundHighest && World.Round < sWorld::FinalRound)
		{
			ZL_Display::DrawRect(recBtnRoundNext, ZL_Color::Black, ZLALPHA(0.3));
			ZL_Display::DrawTriangle(ZL_Vector(recBtnRoundNext.right-5, recBtnRoundNext.MidY()), ZL_Vector(recBtnRoundNext.left+5, recBtnRoundNext.low+5), ZL_Vector(recBtnRoundNext.left+5, recBtnRoundNext.high-5), ZL_Color::Black, ZL_Color::White);
		}

		//MONEY
		int moneyLeft = World.Player.Budget;
		ZL_Color color;
		if (moneyLeft > 0) color = ZL_Color::Green;
		else if (moneyLeft == 0) color = ZL_Color::White;
		else color = ZL_Color::Red;
		fntMain.Draw(recMoney.Center(), "Money: ", ZL_Origin::CenterRight);
		fntMain.Draw(recMoney.Center(), ZL_String(moneyLeft) << " $", color, ZL_Origin::CenterLeft);

		//DUDES FOR HIRE
		srfBGGreen.DrawTo(recDudeGrid.left, recDudeGrid.low, recDudeGrid.left+22, recDudeGrid.high);
		ZL_Display::DrawRect(recDudeGrid.left, recDudeGrid.low, recDudeGrid.left+22, recDudeGrid.high, ZL_Color::Black);
		ZL_Display::PushMatrix();
		ZL_Display::Translate(recDudeGrid.left + 10, recDudeGrid.MidY());
		ZL_Display::Rotate(PIHALF);
		fntMain.Draw(0, 0, "Mercenaries for Hire", ZL_Origin::Center);
		ZL_Display::PopMatrix();
		for (size_t iDude = 0; iDude < World.Dudes.size(); iDude++)
		{
			sTeamMember &dude = *World.Dudes[iDude];
			scalar x = recDudeGrid.left + 22 + (iDude % DudesPerRow) * DudeWidth;
			scalar y = recDudeGrid.high - (iDude / DudesPerRow) * DudeHeight;
			if (pSelectedDude == &dude) srfBGGreen.DrawTo(x, y, x + DudeWidth, y - DudeHeight);
			ZL_Display::DrawRect(x, y, x + DudeWidth, y - DudeHeight, ZL_Color::Black, ZLLUMA((pSelectedDude == &dude ? .4 : 0),.3));
			ZL_Display::DrawRect(x + 5, y - 29, x + DudeWidth - 5, y - 5, ZL_Color::Black, ZLLUMA(0,.3));
			ZL_Display::DrawRect(x + 5, y - DudeHeight + 5, x + DudeWidth - 5, y - DudeHeight + 29, ZL_Color::Black, ZLLUMA(0,.3));
			scalar xcenter = x + DudeWidth/2;
			fntMain.Draw(xcenter, y - 17, dude.Name, ZL_Origin::Center);
			ZL_Rectf recPic(xcenter, y-(DudeHeight/2), dude.Picture.GetSize()/s(2));
			ZL_Display::DrawRect(recPic.left-2, recPic.low-2, recPic.right+2, recPic.high+2, ZLTRANSPARENT, ZL_Color::Black);
			dude.Picture.DrawTo(recPic);
			fntMain.Draw(xcenter, y - DudeHeight + 17, ZL_String("(") << dude.Price << " $)", ZL_Origin::Center);
			if (dude.Health == 0 || dude.hired) ZL_Display::DrawRect(recPic.left-2, y-(DudeHeight/2)-9, recPic.right+2, y-(DudeHeight/2)+9, ZLTRANSPARENT, ZLLUMA(0,.5));
			if (dude.Health == 0) fntMain.Draw(xcenter, y-(DudeHeight/2), "DEAD",  ZL_Color::Red, ZL_Origin::Center);
			else if (dude.hired) fntMain.Draw(xcenter, y-(DudeHeight/2), "HIRED", ZL_Color::Yellow, ZL_Origin::Center);
		}

		//DUDE DETAIL
		if (pSelectedDude)
		{
			sTeamMember &dude = *pSelectedDude;
			scalar x = recDudeDetail.left + 10;
			scalar y = recDudeDetail.high - 132;
			scalar xcenter = (recDudeDetail.left + recDudeDetail.right) / 2;
			ZL_Display::DrawRect(recDudeDetail.left + 5, recDudeDetail.high - 26, recDudeDetail.right - 5, recDudeDetail.high - 5, ZL_Color::Black, ZLLUMA(0,.3));
			fntMain.Draw(xcenter, recDudeDetail.high - 16, dude.Name, ZL_Origin::Center);
			ZL_Rectf recPic(xcenter - 40, recDudeDetail.high - 108, xcenter + 40, recDudeDetail.high - 32);
			ZL_Display::DrawRect(recPic.left-2, recPic.low-2, recPic.right+2, recPic.high+2, ZLTRANSPARENT, ZL_Color::Black);
			dude.Picture.DrawTo(recPic);
			fntMain.Draw(xcenter, y + 12, dude.Description, ZL_Origin::Center);
			fntMain.Draw(x, y - 18, ZL_String("HP: ") << (int)dude.Health << '/' << (int)dude.HealthMax);
			fntMain.Draw(x, y - 36, ZL_String("Strength: ") << dude.Strength);
			fntMain.Draw(x, y - 54, ZL_String("Defense: ") << dude.Defense);
			fntMain.Draw(x, y - 72, ZL_String("Speed: ") << dude.Speed);
			fntMain.Draw(x, y - 90, (dude.Weapon.Ammo < 0 ? dude.Weapon.Name : dude.Weapon.Name + " (" << dude.Weapon.Ammo << ")"));
			if (dude.Health)
			{
				if (SelectedDudeIsInMyDudes)
				{
					if (dude.Health < dude.HealthMax)
					{
						ZL_Display::DrawRect(recBtnHeal, ZL_Color::Black, ZLRGBA(0,.3,0,.3));
						fntMain.Draw(recBtnHeal.Center() + ZL_Vector(2, 7), "Heal", ZL_Color::Black, ZL_Origin::Center);
						fntMain.Draw(recBtnHeal.Center() + ZL_Vector(0, 9), "Heal", ZL_Origin::Center);
						const ZL_String mon = ZL_String("(") << (int)((dude.HealthMax - dude.Health)*s(sWorld::PricePerHealHP)) << "$)";
						fntMain.Draw(recBtnHeal.Center() + ZL_Vector(2, -10), mon, ZL_Color::Black, ZL_Origin::Center);
						fntMain.Draw(recBtnHeal.Center() + ZL_Vector(0, -8), mon, ZL_Origin::Center);
					}
					if (dude.Weapon.Ammo < dude.Weapon.MaxAmmo)
					{
						ZL_Display::DrawRect(recBtnAmmo, ZL_Color::Black, ZLRGBA(0,.3,0,.3));
						fntMain.Draw(recBtnAmmo.Center() + ZL_Vector(2, 7), "Ammo", ZL_Color::Black, ZL_Origin::Center);
						fntMain.Draw(recBtnAmmo.Center() + ZL_Vector(0, 9), "Ammo", ZL_Origin::Center);
						const ZL_String mon = ZL_String("(") << (int)(s(dude.Weapon.MaxAmmo - dude.Weapon.Ammo)*dude.Weapon.AmmoPrice) << "$)";
						fntMain.Draw(recBtnAmmo.Center() + ZL_Vector(2, -10), mon, ZL_Color::Black, ZL_Origin::Center);
						fntMain.Draw(recBtnAmmo.Center() + ZL_Vector(0, -8), mon, ZL_Origin::Center);
					}
				}
				if (dude.hired)
				{
					ZL_Display::DrawRect(recBtnHire, ZL_Color::Black, ZLRGBA(.3,0,0,.3));
					fntMain.Draw(recBtnHire.Center() + ZL_Vector(2, 2), "Fire Him!", ZL_Color::Black, ZL_Origin::Center);
					fntMain.Draw(recBtnHire.Center(), "Fire Him!", ZL_Origin::Center);
				}
				else if (find(World.Player.Team.begin(), World.Player.Team.end(), pSelectedDude) != World.Player.Team.end())
				{
					ZL_Display::DrawRect(recBtnHire, ZL_Color::Black, ZLLUMA(0,.3));
					fntMain.Draw(recBtnHire.Center() + ZL_Vector(2, 2), "Don't Hire!", ZL_Color::Black, ZL_Origin::Center);
					fntMain.Draw(recBtnHire.Center(), "Don't Hire!", ZL_Origin::Center);
				}
				else if (World.Player.Team.size() == maximumTeamSize)
				{
					fntMain.Draw(recBtnHire.Center(), "Team is full!", ZL_Origin::Center);
				}
				else
				{
					ZL_Display::DrawRect(recBtnHire, ZL_Color::Black, ZLRGBA(0,.3,0,.3));
					fntMain.Draw(recBtnHire.Center() + ZL_Vector(2, 2), "Hire Him!", ZL_Color::Black, ZL_Origin::Center);
					fntMain.Draw(recBtnHire.Center(), "Hire Him!", ZL_Origin::Center);
				}
			}
			else
			{
				fntMain.Draw(recBtnHire.Center(), "Deceased", ZL_Origin::Center);
			}
		}

		//MY DUDES
		srfBGGreen.DrawTo(recMyDudes.left, recMyDudes.low, recMyDudes.left+22 + World.Player.Team.size() * MyDudeWidth, recMyDudes.high);
		ZL_Display::DrawRect(recMyDudes.left, recMyDudes.low, recMyDudes.left+22, recMyDudes.high, ZL_Color::Black);
		ZL_Display::PushMatrix();
		ZL_Display::Translate(recMyDudes.left + 10, recMyDudes.MidY());
		ZL_Display::Rotate(PIHALF);
		fntMain.Draw(0, 0, "My Mercs", ZL_Origin::Center);
		ZL_Display::PopMatrix();
		for (size_t iMyDude = 0; iMyDude < World.Player.Team.size(); iMyDude++)
		{
			cAnimatedFigure *dude = World.Player.Team[iMyDude];
			scalar centerx = recMyDudes.left+22 + iMyDude * MyDudeWidth + MyDudeWidth/2;
			ZL_Display::DrawRect(centerx-MyDudeWidth/2, recMyDudes.low, centerx+MyDudeWidth/2, recMyDudes.high, ZL_Color::Black, ZLLUMA((pSelectedDude == dude ? .4 : 0),.3));
			ZL_Display::DrawRect(centerx-MyDudeWidth/2 + 3, recMyDudes.high - 21, centerx+MyDudeWidth/2 - 3, recMyDudes.high - 3, ZL_Color::Black, ZLLUMA(0,.3));
			fntMain.Draw(centerx, recMyDudes.high - 12, dude->Name, ZL_Origin::Center);
			ZL_Rectf recPic(centerx - MyDudeWidth/2 + 5, recMyDudes.low + 5, centerx + MyDudeWidth/2 - 5, recMyDudes.high - 26);
			bool hired = false;
			if (dude->FigureType == cAnimatedFigure::TEAMMEMBER) hired = ((sTeamMember*)dude)->hired;
			ZL_Display::DrawRect(recPic.left-2, recPic.low-2, recPic.right+2, recPic.high+2, ZLTRANSPARENT, (hired ? ZL_Color::Yellow : ZL_Color::Black));
			dude->Picture.DrawTo(recPic);
		}

		//START BUTTON
		if (World.Player.Team.size() > 0 && World.Player.Budget >= 0)
		{
			fntBig.Draw(recBtnOK.Center() + ZL_Vector(3, -3), "START!", s(0.5), ZL_Color::Black, ZL_Origin::Center);
			fntBig.Draw(recBtnOK.Center(), "START!", s(0.5), ZL_Origin::Center);
		}
		else
		{
			fntBig.Draw(recBtnOK.Center() + ZL_Vector(3, -3), "NOT READY", s(0.5), ZL_Color::Black, ZL_Origin::Center);
			fntBig.Draw(recBtnOK.Center(), "NOT READY", s(0.5), ZL_Origin::Center);
		}

		//QUIT BUTTON
		bool hired = false;
		if (World.Player.Team.size() && World.Player.Team[0]->FigureType == cAnimatedFigure::TEAMMEMBER) hired = ((sTeamMember*)World.Player.Team[0])->hired;
		if (World.RoundHighest > 1 || hired)
		{
			fntBig.Draw(recBtnQuit.Center() + ZL_Vector(3,  15-3), "SAVE", s(0.5), ZL_Color::Black, ZL_Origin::Center);
			fntBig.Draw(recBtnQuit.Center() + ZL_Vector(0,  15), "SAVE", s(0.5), ZL_Origin::Center);
			fntBig.Draw(recBtnQuit.Center() + ZL_Vector(3, -15-3), "QUIT", s(0.5), ZL_Color::Black, ZL_Origin::Center);
			fntBig.Draw(recBtnQuit.Center() + ZL_Vector(0, -15), "QUIT", s(0.5), ZL_Origin::Center);
		}
		else
		{
			fntBig.Draw(recBtnQuit.Center() + ZL_Vector(3, -3), "QUIT", s(0.5), ZL_Color::Black, ZL_Origin::Center);
			fntBig.Draw(recBtnQuit.Center(), "QUIT", s(0.5), ZL_Origin::Center);
		}

		if (Fade) ZL_Display::DrawRect(0, 0, ZLWIDTH, ZLHEIGHT, ZLTRANSPARENT, ZLLUMA(0, Fade));
		if (World.HintVisible) World.DrawHint();
	}
} SceneTeam;
