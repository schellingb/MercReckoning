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

#include "animals.h"

//                                                                                        Type,      Name,                     Description,                    Health, HitBoxRad, Str, Def, Spd, ArmorPrc, SightRad, SightAng, Aggr, Price, MoveReac, AttackReac
cSheep::cSheep(const ZL_Vector& pos, scalar angle)             : cAnimal(pos,angle,      SHEEP,    "Sheep",                          "It's a sheep, baaah~~",  500,      30,     13,   10, s( 60),  0,       100,       2,   s(.2),   500,     1000,     800) { LoadAnimations(); StartAnimation(AnimationStand); }
cChipmunk::cChipmunk(const ZL_Vector& pos, scalar angle)       : cAnimal(pos,angle,   CHIPMUNK, "Chipmunk",             "Extremely weak, extremely annoying",   75,      10,      2,   10, s(220),  0,       750,   PI2-1,   s(.9),   100,       50,      80) { LoadAnimations(); StartAnimation(AnimationStand); }
cOstrich::cOstrich(const ZL_Vector& pos, scalar angle)         : cAnimal(pos,angle,    OSTRICH,  "Ostrich",                            "Beware of the beak!", 1200,      30,     15,   10, s(160),  0,       200,       4,   s(.4),  2400,      600,     350) { LoadAnimations(); StartAnimation(AnimationStand); }
cDog::cDog(const ZL_Vector& pos, scalar angle)                 : cAnimal(pos,angle,        DOG,      "Dog",                    "Caution! Barks AND bites!!!", 2500,      35,     20,   10, s(150),  0,       300,       2,   s(.5),  3500,      400,     800) { LoadAnimations(); StartAnimation(AnimationStand); }
cChimp::cChimp(const ZL_Vector& pos, scalar angle)             : cAnimal(pos,angle,      CHIMP,    "Chimp",                                       "Ook ook?",  800,      30,     25,   10, s(100),  0,       200,       2,   s(.5),  1500,      500,     800) { LoadAnimations(); StartAnimation(AnimationStand); }
cWolf::cWolf(const ZL_Vector& pos, scalar angle)               : cAnimal(pos,angle,       WOLF,     "Wolf",                      "There is no \"I\" in wolf", 3200,      45,     30,   10, s(160),  0,       300,       3,   s(.7),  4500,      300,     800) { LoadAnimations(); StartAnimation(AnimationStand); }
cGorilla::cGorilla(const ZL_Vector& pos, scalar angle)         : cAnimal(pos,angle,    GORILLA,  "Gorilla",                           "Barrels not included", 4300,      50,     35,   10, s(110),  0,       350,      PI,   s(.8),  6000,      300,     800) { LoadAnimations(); StartAnimation(AnimationStand); }
cBear::cBear(const ZL_Vector& pos, scalar angle)               : cAnimal(pos,angle,       BEAR,     "Bear", "Playing dead - unfortunately - doesn't help...", 5400,      65,     40,   10, s(110),  0,       400,      PI,   s( 1), 15000,      500,     800) { LoadAnimations(); StartAnimation(AnimationStand); }
/*
cAlligator::cAlligator(const ZL_Vector& pos, scalar angle)     : cAnimal(pos,angle,  ALLIGATOR, 5400,      30,     40,   10, s(0.3),  0,       300,       2,   s(70),   400,     1000,     800) { LoadAnimations(); StartAnimation(AnimationStand); }
cRhino::cRhino(const ZL_Vector& pos, scalar angle)             : cAnimal(pos,angle,      RHINO, 6500,      30,     45,   10, s(1.2),  0,       300,     PI2,   s(80),   500,     1000,     800) { LoadAnimations(); StartAnimation(AnimationStand); }
cElephant::cElephant(const ZL_Vector& pos, scalar angle)       : cAnimal(pos,angle,   ELEPHANT, 7800,      30,     50,   10, s(0.4),  0,       300,     PI2,   s(90),   600,     1000,     800) { LoadAnimations(); StartAnimation(AnimationStand); }
cTriceratops::cTriceratops(const ZL_Vector& pos, scalar angle) : cAnimal(pos,angle,TRICERATOPS, 8200,      30,     60,   10, s(0.9),  0,       300,     PI2,   s(100),  1000,     1000,     800) { LoadAnimations(); StartAnimation(AnimationStand); }
*/

#ifdef SCENE_EDITOR
const char* EditorGetAnimalClass(cAnimal::eType type)
{
	switch (type)
	{
		case cAnimal::      SHEEP: return "cSheep";
		case cAnimal::MOTHERSHEEP: return "cMomSheep";
		case cAnimal::   CHIPMUNK: return "cChipmunk";
		case cAnimal::    OSTRICH: return "cOstrich";
		case cAnimal::        DOG: return "cDog";
		case cAnimal::      CHIMP: return "cChimp";
		case cAnimal::       WOLF: return "cWolf";
		case cAnimal::    GORILLA: return "cGorilla";
		case cAnimal::       BEAR: return "cBear";
		/*
		case cAnimal::  ALLIGATOR: return "cAlligator";
		case cAnimal::      RHINO: return "cRhino";
		case cAnimal::   ELEPHANT: return "cElephant";
		case cAnimal::TRICERATOPS: return "cTriceratops";
		*/
		default: assert(false); return "cUNKNOWN";
	}
}
#endif

void cSheep::LoadAnimations()
{
	Layers.push_back(ZL_Surface("Data/sheep.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(3,2));

	AnimationStand.Setup(Layers, cAnimation::SHORTBLINK, 5000);
	AnimationStand.Tiles.push_back(0);
	AnimationStand.Tiles.push_back(1);

	AnimationMove.Setup(Layers, cAnimation::LOOP, GetMoveAnimStep());
	AnimationMove.Tiles.push_back(3);
	AnimationMove.Tiles.push_back(4);
	AnimationMove.Tiles.push_back(5);

	AnimationAttack.Setup(Layers, cAnimation::PLAYONCE, AttackReactionTime / 2);
	AnimationAttack.Tiles.push_back(2);

	AnimationDeath.Setup(Layers, cAnimation::PLAYONCE, 1000);
	AnimationDeath.Tiles.push_back(1);

	LoadSound("Data/sheep.ogg");
}

cMomSheep::cMomSheep(const ZL_Vector& pos, scalar viewAngle) : cSheep(pos, viewAngle)
{
	Type = MOTHERSHEEP;
	Name = "Mothersheep";
	Health = HealthMax = 1500;
	Strength = 25;
	Defense = 70;
	Speed = s(0.2);
	Aggressiveness = 1;
	MoveReactionTime = 0;
	AttackReactionTime = 1200;
	HitBoxRadius = 60;
	SightRadius = 200;
	Price = 4000;
}

void cMomSheep::LoadAnimations()
{
	cSheep::LoadAnimations();
	Layers.front().SetScale(s(2.0));
}

void cMomSheep::Attack()
{
	bool attackLaunched = false;
	bool superAttack = false;
	scalar attackPower = Strength;
	for (vector<cAnimatedFigure*>::iterator it = World.Player.Team.begin(); it != World.Player.Team.end(); ++it)
		if ((*it)->Health && Pos.GetDistance((*it)->Pos) < HitBoxRadius + (*it)->HitBoxRadius + 2)
		{
			if (!attackLaunched)
			{
				superAttack = rand() % 100 < 20;
				if (superAttack)
					attackPower *= 3.0f;
			}
			ZL_LOG0("Mothersheep", ZL_String(superAttack ? "Super attack: " : "Attack: ") << attackPower);
			StartAnimation(AnimationAttack);
			(*it)->ReceiveAttack(this, attackPower, ArmorPiercing);
			if (superAttack)
				(*it)->LoseControl((((*it)->Pos - Pos)), 300.0f, 0);
			attackLaunched = true;
		}
	if (attackLaunched)
		NextAttack = World.Time + (superAttack ? AttackReactionTime * 1.3f : AttackReactionTime);
}

void cChipmunk::LoadAnimations()
{
	Layers.push_back(ZL_Surface("Data/chipmunk.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(1,2));

	AnimationStand.Setup(Layers);
	AnimationStand.Tiles.push_back(0);

	AnimationMove.Setup(Layers, cAnimation::LOOP, GetMoveAnimStep());
	AnimationMove.Tiles.push_back(0);
	AnimationMove.Tiles.push_back(1);

	LoadSound("Data/chipmunk.ogg");
}

void cOstrich::LoadAnimations()
{
	Layers.push_back(ZL_Surface("Data/ostrich.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(2,2).SetScale(s(0.6)));

	AnimationStand.Setup(Layers);
	AnimationStand.Tiles.push_back(0);

	AnimationMove.Setup(Layers, cAnimation::LOOP, GetMoveAnimStep());
	AnimationMove.Tiles.push_back(1);
	AnimationMove.Tiles.push_back(0);

	AnimationAttack.Setup(Layers, cAnimation::PLAYONCE, AttackReactionTime / 2);
	AnimationAttack.Tiles.push_back(2);

	AnimationDeath.Setup(Layers, cAnimation::PLAYONCE, 1000);
	AnimationDeath.Tiles.push_back(3);

	LoadSound("Data/ostrich.ogg");
}

void cDog::LoadAnimations()
{
	Layers.push_back(ZL_Surface("Data/dog.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(3,2).SetScale(s(0.4)));

	AnimationStand.Setup(Layers, cAnimation::LOOP, 300);
	AnimationStand.Tiles.push_back(0);
	AnimationStand.Tiles.push_back(1);

	AnimationMove.Setup(Layers, cAnimation::LOOP, GetMoveAnimStep());
	AnimationMove.Tiles.push_back(3);
	AnimationMove.Tiles.push_back(4);

	AnimationAttack.Setup(Layers, cAnimation::PLAYONCE, AttackReactionTime / 2);
	AnimationAttack.Tiles.push_back(2);

	AnimationDeath.Setup(Layers, cAnimation::PLAYONCE, 1000);
	AnimationDeath.Tiles.push_back(5);

	LoadSound("Data/dog.ogg");
}

void cChimp::LoadAnimations()
{
	Layers.push_back(ZL_Surface("Data/chimp.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(2,3).SetScale(s(0.6)));

	AnimationStand.Setup(Layers, cAnimation::SHORTBLINK, 5000);
	AnimationStand.Tiles.push_back(0);
	AnimationStand.Tiles.push_back(1);

	AnimationMove.Setup(Layers, cAnimation::LOOP, GetMoveAnimStep());
	AnimationMove.Tiles.push_back(2);
	AnimationMove.Tiles.push_back(3);

	AnimationAttack.Setup(Layers, cAnimation::PLAYONCE, AttackReactionTime / 2);
	AnimationAttack.Tiles.push_back(4);

	AnimationDeath.Setup(Layers, cAnimation::PLAYONCE, 1000);
	AnimationDeath.Tiles.push_back(5);

	LoadSound("Data/monkey.ogg");
}

void cWolf::LoadAnimations()
{
	Layers.push_back(ZL_Surface("Data/wolf.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(3,2).SetScale(s(0.5)));

	AnimationStand.Setup(Layers, cAnimation::LOOP, 300);
	AnimationStand.Tiles.push_back(0);
	AnimationStand.Tiles.push_back(1);

	AnimationMove.Setup(Layers, cAnimation::LOOP, GetMoveAnimStep());
	AnimationMove.Tiles.push_back(3);
	AnimationMove.Tiles.push_back(4);

	AnimationAttack.Setup(Layers, cAnimation::PLAYONCE, AttackReactionTime / 2);
	AnimationAttack.Tiles.push_back(2);

	AnimationDeath.Setup(Layers, cAnimation::PLAYONCE, 1000);
	AnimationDeath.Tiles.push_back(5);

	LoadSound("Data/dog.ogg"); Sound.SetSpeedFactor(s(0.9));
}

void cGorilla::LoadAnimations()
{
	Layers.push_back(ZL_Surface("Data/gorilla.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(2,3));

	AnimationStand.Setup(Layers, cAnimation::SHORTBLINK, 5000);
	AnimationStand.Tiles.push_back(0);
	AnimationStand.Tiles.push_back(1);

	AnimationMove.Setup(Layers, cAnimation::LOOP, GetMoveAnimStep());
	AnimationMove.Tiles.push_back(2);
	AnimationMove.Tiles.push_back(3);

	AnimationAttack.Setup(Layers, cAnimation::PLAYONCE, AttackReactionTime / 2);
	AnimationAttack.Tiles.push_back(4);

	AnimationDeath.Setup(Layers, cAnimation::PLAYONCE, 1000);
	AnimationDeath.Tiles.push_back(5);

	LoadSound("Data/monkey.ogg"); Sound.SetSpeedFactor(s(0.8));
}

void cBear::LoadAnimations()
{
	Layers.push_back(ZL_Surface("Data/bear.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(2,3));

	AnimationStand.Setup(Layers, cAnimation::SHORTBLINK, 5000);
	AnimationStand.Tiles.push_back(0);
	AnimationStand.Tiles.push_back(1);

	AnimationMove.Setup(Layers, cAnimation::LOOP, GetMoveAnimStep());
	AnimationMove.Tiles.push_back(2);
	AnimationMove.Tiles.push_back(3);

	AnimationAttack.Setup(Layers, cAnimation::PLAYONCE, AttackReactionTime / 2);
	AnimationAttack.Tiles.push_back(4);

	AnimationDeath.Setup(Layers, cAnimation::PLAYONCE, 1000);
	AnimationDeath.Tiles.push_back(5);

	LoadSound("Data/bear.ogg");
}

/*
void cAlligator::LoadAnimations()
{
	Layers.push_back(ZL_Surface("Data/animal.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(1,1));
	AnimationStand.Setup(Layers);
	AnimationStand.Tiles.push_back(0);
	Layers.front().SetColor(ZLRGB(.1,.7,.1));
}
void cRhino::LoadAnimations()
{
	Layers.push_back(ZL_Surface("Data/animal.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(1,1));
	AnimationStand.Setup(Layers);
	AnimationStand.Tiles.push_back(0);
	Layers.front().SetColor(ZLRGB(.6,.6,.6));
}
void cElephant::LoadAnimations()
{
	Layers.push_back(ZL_Surface("Data/animal.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(1,1));
	AnimationStand.Setup(Layers);
	AnimationStand.Tiles.push_back(0);
	Layers.front().SetColor(ZLRGB(.7,.7,.7));
}
void cTriceratops::LoadAnimations()
{
	Layers.push_back(ZL_Surface("Data/animal.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(1,1));
	AnimationStand.Setup(Layers);
	AnimationStand.Tiles.push_back(0);
	Layers.front().SetColor(ZLRGB(.1,.9,.2));
}
*/
