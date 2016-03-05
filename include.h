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

#ifndef _MERCRECKONING_INCLUDE_
#define _MERCRECKONING_INCLUDE_

#include <ZL_Application.h>
#include <ZL_Display.h>
#include <ZL_Surface.h>
#include <ZL_Signal.h>
#include <ZL_Audio.h>
#include <ZL_Font.h>
#include <ZL_Scene.h>
#include <ZL_Timer.h>
#include <ZL_Particles.h>
#include <ZL_Math.h>
#include <ZL_Data.h>
#include <ZL_Network.h>
#include <../Opt/ZL_TouchInput.h>

#include <map>
#include <algorithm>
#include <vector>
#include <list>
#include <cassert>

#if defined(_MSC_VER)
#pragma warning(disable:4244) //'argument' : conversion from 'int' to 'scalar', possible loss of data
#endif
using namespace std;

extern ZL_Font fntMain, fntBig;
extern ZL_Surface srfBGBrown, srfBGGreen, srfPalm;
extern ZL_Sound sndMenuMusic, sndSelect;

#define SCENE_MENU 1
#define SCENE_INTRO 2
#define SCENE_TEAM 3
#define SCENE_GAME 4
#define SCENE_CLEARED 5

#ifdef ZILLALOG
#define DEBUG 1
#define SCENE_EDITOR 99
#else
#define DEBUG 0
#endif

#endif //_MERCRECKONING_INCLUDE_
