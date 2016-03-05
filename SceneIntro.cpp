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

static const char* ArrayStoryText[] = {
	"To change the fates of\ncivil wars around the\nworld mercenaries\nare hired",
	"But before being trusted\nby crooked governments\nand corporations there\nis one final challenge",
	"Known as Merc Reckoning\nA survival deathmatch\nagainst nature\non offshore islands",
	"Lead your team\nof freshmen mercenaries\nto victory!",
};
static const char* ArrayStoryImages[] = {
	"Data/intro1.jpg",
	"Data/intro2.jpg",
	"Data/intro3.jpg",
	"Data/intro4.jpg",
};

static scalar TextScale, ImageScale, LastImageScale;
static vector<ZL_TextBuffer> TextBufs;
static ZL_Surface Image, LastImage, srfHole;
static ticks_t TimeStart, TimeEnd;
static int CurrentText;

struct sSceneIntro : public ZL_Scene
{
	sSceneIntro() : ZL_Scene(SCENE_INTRO) { }

	void InitText(int n)
	{
		CurrentText = n;
		TextBufs.clear();
		if (CurrentText >= (int)(sizeof(ArrayStoryText)/sizeof(ArrayStoryText[0])))
		{
			ZL_SceneManager::GoToScene(SCENE_TEAM);
			return;
		}
		vector<ZL_String> lines = ZL_String::to_upper(ArrayStoryText[n]).split("\n");
		TextScale = 100;
		for (vector<ZL_String>::iterator it = lines.begin(); it != lines.end(); ++it)
		{
			ZL_TextBuffer TextBuf(fntBig, *it);
			scalar w = ZLFROMW(80)/s(TextBuf.GetWidth());
			if (w < TextScale) TextScale = w;
			TextBufs.push_back(TextBuf);
		}
		LastImage = Image;
		LastImageScale = ImageScale;
		Image = ZL_Surface(ArrayStoryImages[n]).SetDrawOrigin(ZL_Origin::Center);
		ImageScale = MIN(ZLFROMW(80)/s(Image.GetWidth()), ZLFROMH(80)/s(Image.GetHeight()));
		TimeStart = ZLTICKS;
		TimeEnd = ZLTICKS + 1500 + (ticks_t)TextBufs.size() * 1500;
	}

	int InitTransitionEnter(ZL_SceneType SceneTypeFrom, void* data)
	{
		TimeEnd = 0;
		CurrentText = -1;
		srfHole = ZL_Surface("Data/hole.png");
		return 400;
	}

	int DeInitTransitionLeave(ZL_SceneType SceneTypeTo)
	{
		ZL_Display::AllSigDisconnect(this);
		return 400;
	}

	void DeInitAfterTransition()
	{
		Image = ZL_Surface();
		LastImage = ZL_Surface();
		sndMenuMusic = ZL_Sound();
		srfHole = ZL_Surface();
	}

	void InitAfterTransition()
	{
		ZL_Display::sigPointerDown.connect(this, &sSceneIntro::OnPointerDown);
		ZL_Display::sigKeyDown.connect(this, &sSceneIntro::OnKeyDown);
	}

	void OnPointerDown(ZL_PointerPressEvent& e)
	{
		scalar imgdone = s(ZLTICKS - TimeStart) / s(TimeEnd - TimeStart);
		TimeStart = ZLTICKS - (imgdone * 500);
		TimeEnd = ZLTICKS + ((1-imgdone) * 500);
	}

	void OnKeyDown(ZL_KeyboardEvent& e)
	{
		if (e.key == ZLK_ESCAPE) ZL_SceneManager::GoToScene(SCENE_TEAM);
		scalar imgdone = s(ZLTICKS - TimeStart) / s(TimeEnd - TimeStart);
		TimeStart = ZLTICKS - (imgdone * 500);
		TimeEnd = ZLTICKS + ((1-imgdone) * 500);
	}

	void Draw()
	{
		if (CurrentText == -1 || TextBufs.size())
		{
			if (ZLTICKS >= TimeEnd) InitText(CurrentText+1);
			scalar imgpos = s(ZLTICKS - TimeStart) / s(TimeEnd - TimeStart);
			scalar bigimgalpha = (imgpos < 0.1 ? imgpos*10 : 1);
			scalar imgalpha = (imgpos > 0.9 ? (1-imgpos)*10 : bigimgalpha);
			LastImage.Draw(ZLHALFW - ZLWIDTH*(imgpos+0.5)/3, ZLHALFH + ZLHEIGHT*(imgpos+0.5)/3, LastImageScale*5, LastImageScale*4, ZLLUM(.5));
			Image.Draw(    ZLHALFW - ZLWIDTH*(imgpos-0.5)/3, ZLHALFH + ZLHEIGHT*(imgpos-0.5)/3, ImageScale*5, ImageScale*4, ZLLUMA(.5, bigimgalpha));
			srfHole.DrawTo(0, 0, ZLWIDTH, ZLHEIGHT);
			ZL_Display::DrawRect(ZL_Rectf(ZLHALFW - ZLWIDTH*(imgpos-0.5)/2, ZLHALFH + ZLHEIGHT*(imgpos-0.5)/2, Image.GetSize()*ImageScale*s(0.51)), ZLTRANSPARENT, ZLRGBA(.7,.3,0,imgalpha));
			Image.Draw(ZLHALFW - ZLWIDTH*(imgpos-0.5)/2, ZLHALFH + ZLHEIGHT*(imgpos-0.5)/2, ImageScale, ImageScale, ZLALPHA(imgalpha));
			srfHole.DrawTo(0, 0, ZLWIDTH, ZLHEIGHT);

			scalar showpos = 1, anim = imgpos * (2.5+TextBufs.size()) - 1.5;
			for (vector<ZL_TextBuffer>::iterator it = TextBufs.begin(); it != TextBufs.end(); ++it, showpos++)
			{
				scalar show = (s(2.5) - sabs(anim - showpos)) / s(2.5);
				if (show <= 0) continue;
				scalar x = ZLHALFW, y = ZLHALFH - 50 + showpos*-60 + anim * 60, scalex = TextScale*(s(.5)+s(0.5)*show), scaley = TextScale*show, shadowoff = 5*show, shadowalpha = show/2;
				it->Draw(x - shadowoff, y + shadowoff, scalex, scaley, ZLLUMA(0,shadowalpha), ZL_Origin::Center);
				it->Draw(x + shadowoff, y + shadowoff, scalex, scaley, ZLLUMA(0,shadowalpha), ZL_Origin::Center);
				it->Draw(x - shadowoff, y - shadowoff, scalex, scaley, ZLLUMA(0,shadowalpha), ZL_Origin::Center);
				it->Draw(x + shadowoff, y - shadowoff, scalex, scaley, ZLLUMA(0,shadowalpha), ZL_Origin::Center);
				it->Draw(x            , y            , scalex, scaley, ZLALPHA(show)        , ZL_Origin::Center);
			}
		}
		else
		{
			scalar imgpos = s(ZLTICKS - TimeStart) / s(TimeEnd - TimeStart);
			Image.Draw(    ZLHALFW - ZLWIDTH*(imgpos-0.5)/3, ZLHALFH + ZLHEIGHT*(imgpos-0.5)/3, ImageScale*5, ImageScale*4, ZLLUM(.5));
			srfHole.DrawTo(0, 0, ZLWIDTH, ZLHEIGHT);
			srfHole.DrawTo(0, 0, ZLWIDTH, ZLHEIGHT);
		}
	}
} SceneIntro;
