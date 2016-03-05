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
#ifdef SCENE_EDITOR //DEBUGMODE ONLY SCENE
#include "world.h"

static ZL_Vector scroll;
static ZL_Rect screen;
static scalar zoom;
struct sEditorButton
{
	enum eType { PREVROUND, NEXTROUND, WATER, FENCE, ROCK, PALM, MOVE, ROTATE, DELETE, PLACEAREA, MAPSIZE, OUTPUT } type;
	ZL_Rectf rec;
	ZL_String title;
	void Draw();
};
static vector<sEditorButton> buttons;
sEditorButton* pActiveButton = NULL;
ZL_Rectf recMenuHeader;
static void AddButton(sEditorButton::eType type, const ZL_String& title)
{
	sEditorButton btn;
	btn.type = type;
	btn.title = title;
	btn.rec = ZL_Rectf(recMenuHeader.left, recMenuHeader.low-35*buttons.size()-35, recMenuHeader.right, recMenuHeader.low-35*buttons.size());
	buttons.push_back(btn);
}
void sEditorButton::Draw()
{
	ZL_Display::DrawRect(rec, ZL_Color::Black, (pActiveButton == this ? ZLLUMA(.4,0.8) : ZLLUMA(0,0.8)));
	fntMain.Draw(rec.Center(), title, ZL_Origin::Center);
}
int DragWaterValue;
sObstacle *DragObstacle;
cAnimal *DragEnemy;

struct sSceneEditor : public ZL_Scene
{
	sSceneEditor() : ZL_Scene(SCENE_EDITOR) { }

	void InitEnter(ZL_SceneType SceneTypeFrom, void* data)
	{
		if (SceneTypeFrom != SCENE_GAME) { World.Round = 1; World.StartRound(); }
		zoom = s(1);
		scroll = ZL_Vector(World.Width/2 - ZLHALFW, World.Height/2 - ZLHALFH);
		recMenuHeader = ZL_Rectf(0, ZLFROMH(18), 100, ZLHEIGHT);
		AddButton(sEditorButton::PREVROUND, "Round <-");
		AddButton(sEditorButton::NEXTROUND, "Round ->");
		AddButton(sEditorButton::WATER, "Water");
		AddButton(sEditorButton::FENCE, "Fence");
		AddButton(sEditorButton::PALM, "Palm");
		AddButton(sEditorButton::ROCK, "Rock");
		AddButton(sEditorButton::MOVE, "Move");
		AddButton(sEditorButton::ROTATE, "Rotate");
		AddButton(sEditorButton::DELETE, "Delete");
		AddButton(sEditorButton::PLACEAREA, "PlaceArea");
		AddButton(sEditorButton::MAPSIZE, "MapSize");
		AddButton(sEditorButton::OUTPUT, "Output");
		UpdateScreen();
	}

	void InitAfterTransition()
	{
		ZL_TouchInput::ScrollMode = ZL_TouchInput::SCROLL_DUAL_OR_RIGHTMB;
		ZL_TouchInput::sigClick.connect(this, &sSceneEditor::Click);
		ZL_TouchInput::sigScroll.connect(this, &sSceneEditor::Scroll);
		ZL_TouchInput::sigZoom.connect(this, &sSceneEditor::Zoom);
		ZL_TouchInput::sigDrag.connect(this, &sSceneEditor::Drag);
		ZL_Display::sigKeyDown.connect(this, &sSceneEditor::OnKeyDown);
		ZL_Display::sigActivated.connect(this, &sSceneEditor::OnActivated);
	}

	void DeInitLeave(ZL_SceneType SceneTypeTo)
	{
		ZL_Display::AllSigDisconnect(this);
		ZL_TouchInput::AllSigDisconnect(this);
		buttons.clear();
		pActiveButton = NULL;
	}

	void UpdateScreen()
	{
		screen = ZL_Rectf(scroll, ZLWIDTH*zoom, ZLHEIGHT*zoom);
	}

	void Click(const ZL_Vector& p, ZL_TouchInput::eClickResult& ClickResult)
	{
		DragObstacle = NULL;
		DragEnemy = NULL;
		for (vector<sEditorButton>::iterator itButton = buttons.begin(); itButton != buttons.end(); ++itButton)
			if (itButton->rec.Contains(p))
			{
				if (itButton->type == sEditorButton::OUTPUT) World.EditorOutput();
				else if (itButton->type == sEditorButton::PREVROUND) { World.Round--; World.StartRound(); }
				else if (itButton->type == sEditorButton::NEXTROUND) { World.Round++; World.StartRound(); }
				else pActiveButton = &*itButton;
				return;
			}
		ZL_Vector pos = scroll + p*zoom;
		if (pActiveButton && pActiveButton->type == sEditorButton::WATER)
		{
			DragWaterValue = World.EditorSetWater(pos.x, pos.y);
		}
		else if (pActiveButton && (pActiveButton->type == sEditorButton::MOVE || pActiveButton->type == sEditorButton::ROTATE))
		{
			for (vector<sObstacle*>::iterator itObstacle = World.Obstacles.begin(); itObstacle != World.Obstacles.end(); ++itObstacle)
				if ((*itObstacle)->Collide(pos, 10) && (!DragObstacle || (pos - (*itObstacle)->RotPos.P < pos - DragObstacle->RotPos.P))) DragObstacle = *itObstacle;
			if (DragObstacle == NULL)
				for (vector<cAnimatedFigure*>::iterator itEnemy = World.Players[1]->Team.begin(); itEnemy != World.Players[1]->Team.end(); ++itEnemy)
					if (((*itEnemy)->Pos - pos < (*itEnemy)->HitBoxRadius + 10) && (!DragEnemy || (pos - (*itEnemy)->Pos < pos - DragEnemy->Pos))) DragEnemy = (cAnimal*)*itEnemy;
		}
		else if (pActiveButton && pActiveButton->type == sEditorButton::FENCE)
		{
			World.Obstacles.push_back(DragObstacle = new sObstacle(sObstacle::FENCE, pos, 0));
		}
		else if (pActiveButton && pActiveButton->type == sEditorButton::ROCK)
		{
			World.Obstacles.push_back(DragObstacle = new sObstacle(sObstacle::ROCK, pos, 0));
		}
		else if (pActiveButton && pActiveButton->type == sEditorButton::PALM)
		{
			World.Obstacles.push_back(DragObstacle = new sObstacle(sObstacle::PALM, pos, 0));
		}
		else if (pActiveButton && pActiveButton->type == sEditorButton::DELETE)
		{
			for (vector<sObstacle*>::iterator itObstacle = World.Obstacles.begin(); itObstacle != World.Obstacles.end(); ++itObstacle)
				if ((*itObstacle)->Collide(pos, 30)) { delete *itObstacle; World.Obstacles.erase(itObstacle); return; }
		}
		else if (pActiveButton && pActiveButton->type == sEditorButton::PLACEAREA)
		{
			World.PlacementArea = ZL_Rectf(pos, 20, 20);
		}
	}

	void Drag(const ZL_Vector& startpos, const ZL_Vector& p, const ZL_Vector& dragamount)
	{
		ZL_Vector pos = scroll + p*zoom;
		if (pActiveButton && pActiveButton->type == sEditorButton::WATER)
		{
			World.EditorSetWater(pos.x, pos.y, DragWaterValue);
		}
		else if (pActiveButton && pActiveButton->type == sEditorButton::ROTATE && DragObstacle)
		{
			DragObstacle->RotPos.A = ZL_Math::Angle(DragObstacle->RotPos.A + dragamount.y/s(100));
		}
		else if (DragObstacle)
		{
			DragObstacle->RotPos.P += dragamount*zoom;
		}
		else if (pActiveButton && pActiveButton->type == sEditorButton::ROTATE && DragEnemy)
		{
			DragEnemy->ViewAngle = ZL_Math::Angle(DragEnemy->ViewAngle + dragamount.y/s(100));
		}
		else if (DragEnemy)
		{
			DragEnemy->Pos += dragamount*zoom;
		}
		else if (pActiveButton && pActiveButton->type == sEditorButton::PLACEAREA)
		{
			ZL_Vector ScrollStartPos = scroll + startpos*zoom;
			World.PlacementArea = ZL_Rectf(MAX(0,MIN(World.Width,MIN(ScrollStartPos.x, pos.x))), MAX(0,MIN(World.Height,MIN(ScrollStartPos.y, pos.y))), MAX(0,MIN(World.Width, MAX(ScrollStartPos.x, pos.x))), MAX(0,MIN(World.Height, MAX(ScrollStartPos.y, pos.y))));
		}
		else if (pActiveButton && pActiveButton->type == sEditorButton::MAPSIZE)
		{
			World.EditorResize(World.Width + dragamount.x*zoom, World.Height + dragamount.y*zoom);
		}
	}

	void Scroll(const ZL_Vector& startpos, const ZL_Vector& scrollamount)
	{
		scroll -= scrollamount*zoom;
		UpdateScreen();
	}

	void Zoom(const ZL_Vector& startpos, scalar factor, const ZL_Vector& originp)
	{
		scroll += (originp*zoom*(-factor+1));
		zoom *= factor;
		UpdateScreen();
	}

	void OnKeyDown(ZL_KeyboardEvent& e)
	{
		if (e.key == ZLK_ESCAPE) ZL_SceneManager::GoToScene(SCENE_MENU);
		if ((e.key == ZLK_F5 || e.key == ZLK_VOLUMEDOWN || e.key == ZLK_VOLUMEUP) && World.Player.Team.size()) ZL_SceneManager::GoToScene(SCENE_GAME);
	}

	void OnActivated(ZL_WindowActivateEvent& e)
	{
	}

	void Calculate()
	{
	}

	void Draw()
	{
		ZL_Display::ClearFill(ZL_Color::Black);
		ZL_Display::PushOrtho(screen);
		World.Draw();
		ZL_Display::DrawRect(World.PlacementArea, ZL_Color::Green, ZLRGBA(0,1,0,.2));
		ZL_Display::PopOrtho();
		ZL_Display::DrawRect(recMenuHeader, ZL_Color::Black, ZL_Color::Black);
		fntMain.Draw(recMenuHeader.Center(), ZL_String("Round ")<<World.Round, ZL_Origin::Center);
		for (vector<sEditorButton>::iterator itButton = buttons.begin(); itButton != buttons.end(); ++itButton)
			itButton->Draw();
	}
} SceneEditor;

#endif //DEBUGMODE ONLY SCENE
