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

#ifndef _MERCRECKONING_ANIMALS_
#define _MERCRECKONING_ANIMALS_
#include "world.h"

class cSheep : public cAnimal
{
public:
	cSheep(const ZL_Vector& pos, scalar viewAngle);
	void LoadAnimations();
};

class cMomSheep : public cSheep
{
public:
	cMomSheep(const ZL_Vector& pos, scalar viewAngle);
	void Attack();
	void LoadAnimations();
};

class cChipmunk : public cAnimal
{
public:
	cChipmunk(const ZL_Vector& pos, scalar viewAngle);
	void LoadAnimations();
};

class cOstrich : public cAnimal
{
public:
	cOstrich(const ZL_Vector& pos, scalar viewAngle);
	void LoadAnimations();
};

class cDog : public cAnimal
{
public:
	cDog(const ZL_Vector& pos, scalar viewAngle);
	void LoadAnimations();
};

class cChimp : public cAnimal
{
public:
	cChimp(const ZL_Vector& pos, scalar viewAngle);
	void LoadAnimations();
};

class cWolf : public cAnimal
{
public:
	cWolf(const ZL_Vector& pos, scalar viewAngle);
	void LoadAnimations();
};

class cGorilla : public cAnimal
{
public:
	cGorilla(const ZL_Vector& pos, scalar viewAngle);
	void LoadAnimations();
};

class cBear : public cAnimal
{
public:
	cBear(const ZL_Vector& pos, scalar viewAngle);
	void LoadAnimations();
};

/*
class cAlligator : public cAnimal
{
public:
	cAlligator(const ZL_Vector& pos, scalar viewAngle);
	void LoadAnimations();
};

class cRhino : public cAnimal
{
public:
	cRhino(const ZL_Vector& pos, scalar viewAngle);
	void LoadAnimations();
};

class cElephant : public cAnimal
{
public:
	cElephant(const ZL_Vector& pos, scalar viewAngle);
	void LoadAnimations();
};

class cTriceratops : public cAnimal
{
public:
	cTriceratops(const ZL_Vector& pos, scalar viewAngle);
	void LoadAnimations();
};
*/

#endif //_MERCRECKONING_ANIMALS_
