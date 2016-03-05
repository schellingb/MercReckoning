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

#include "world.h"
#include "animals.h"

sWorld World;
cAnimatedFigure *ActivePlayerFigure = NULL;

// Textures
// Objects
static ZL_Surface srfFence, srfRock;
// Decals
static ZL_Surface srfBlood;
// Misc
static ZL_Surface Graphic, Background, srfShadow, srfPath, srfPathTarget, srfAttackCursor;

//sounds
static ZL_Sound sndStep;

//other
static ZL_ParticleEffect particleBlood;
static unsigned char *water_map = NULL;
static unsigned int water_map_width = 0, water_map_height = 0;
static ZL_Shader WaterShader;

//hint stuff
static scalar hint_fade;
static ZL_Surface hint_surface, hint_surface2;

//consts
int const cAnimation::ShortBlinkDuration = 150;
const unsigned short sWorld::FinalRound = 10;
int const sWorld::PricePerHealHP = 2;

void sWorld::InitGlobal()
{
	srfFence = ZL_Surface("Data/fence.png").SetDrawOrigin(ZL_Origin::Center);
	srfRock = ZL_Surface("Data/rock.png").SetDrawOrigin(ZL_Origin::Center);
	srfBlood = ZL_Surface("Data/blood_splatter.png").SetDrawOrigin(ZL_Origin::Center);
	srfShadow = ZL_Surface("Data/shadow.png").SetDrawOrigin(ZL_Origin::Center);
	srfPath = ZL_Surface("Data/path.png").SetDrawOrigin(ZL_Origin::Center);
	srfPathTarget = ZL_Surface("Data/pathtarget.png").SetDrawOrigin(ZL_Origin::Center);
	srfAttackCursor = ZL_Surface("Data/cursor.png").SetDrawOrigin(ZL_Origin::Center);
	Background = ZL_Surface(32, 32).SetTextureRepeatMode();

	sndStep = ZL_Sound("Data/step.ogg");

	particleBlood = ZL_ParticleEffect(1000, 200); //random lifetime 800 - 1200
	particleBlood.AddParticleImage(ZL_Surface("Data/particle_blood.png"), 1000);
	particleBlood.AddBehavior(new ZL_ParticleBehavior_LinearMove(60, 20));
	particleBlood.AddBehavior(new ZL_ParticleBehavior_LinearImageProperties(1, 0, s(1.0), s(0.5)));

	WaterShader = ZL_Shader("#ifdef GL_ES\nprecision highp float;\n#endif\n"
		"uniform "/*"mediump "*/"sampler2D u_texture;varying "/*"highp "*/"vec2 v_texcoord;"
		"void main()"
		"{"
			"gl_FragColor = texture2D(u_texture, v_texcoord);"
			""/*"highp "*/"float f = 1.0-((v_texcoord.x-256.0)*(v_texcoord.x-256.0)+(v_texcoord.y-256.0)*(v_texcoord.y-256.0))/(256.0*256.0);"
			"gl_FragColor.r *= f; gl_FragColor.g *= f; gl_FragColor.b *= f;"
		"}");

	HintsShown = 0;
	HintVisible = false;
}

void sWorld::Init()
{
	Player = sPlayer(0, 2000); // human controlled player
	Player.isHuman = true;

	Round = RoundHighest = 1;

	for (vector<sTeamMember*>::iterator itDude = Dudes.begin(); itDude != Dudes.end(); ++itDude)
		delete(*itDude);
	Dudes.clear();
	Player.Team.clear();
	// Todo: Give dudes the correct radius; Fatso should have a large radius, thus be easier to hit
	//                                ID              Picture                           Name,      Description,     Price, Health, Strength, Defense, Speed,  Weapon,            Radius, Reaction time
	Dudes.push_back(new sTeamMember(DUDE_MIKE,    ZL_Surface("Data/Dude_Mike.png"),    "Mike",    "He is the best", 5000,  300,    100,        70,     90,    WEAPON_MACHINEGUN, 20,      100));
	Dudes.push_back(new sTeamMember(DUDE_JACK,    ZL_Surface("Data/Dude_Jack.png"),    "Jack",    "Knife Jack",     1000,  150,     50,        30,    120,    WEAPON_THROWKNIFE, 20,      200));
	Dudes.push_back(new sTeamMember(DUDE_LUKE,    ZL_Surface("Data/Dude_Luke.png"),    "Luke",    "Knuckle Luke",    500,  100,     30,        10,     90,    WEAPON_KNUCKLES,   20,        0));
	Dudes.push_back(new sTeamMember(DUDE_BETSY,   ZL_Surface("Data/Dude_Betsy.png"),   "Betsy",   "Chainsaw Betsy", 3000,  120,     80,        50,     80,    WEAPON_CHAINSAW,   20,      400));
	Dudes.push_back(new sTeamMember(DUDE_FATSTAR, ZL_Surface("Data/Dude_Fatty.png"),   "FatStar", "McFatty",         200,  300,     20,        40,     30,    WEAPON_NAILCLUB,   25,     2000));
	Dudes.push_back(new sTeamMember(DUDE_IVAN,    ZL_Surface("Data/Dude_Ivan.png"),    "Ivan",    "Crazy Ivan",      500,   50,      0,      -100,     70,    WEAPON_BOMB,       20,      300));
	Dudes.push_back(new sTeamMember(DUDE_LEON,    ZL_Surface("Data/Dude_Leon.png"),    "Leon",    "The French",      400,  100,     30,        30,     90,    WEAPON_KICK,       20,      200));
	Dudes.push_back(new sTeamMember(DUDE_SHIFTER, ZL_Surface("Data/Dude_Shifter.png"), "Shifter", "Silent Death",   1500,  200,     60,        30,    100,    WEAPON_NINJASTAR,  20,      200));
	//#if DEBUG
	//Dudes.push_back(new sTeamMember(ZL_Surface("Data/Dude_Fatty.png"), "Debby Ugger", "DebUgger",          1,  999,      0,       100,    200,    WEAPON_NONE,       20,        0)); // Immortal Debby
	//#endif
}

const char* sWorld::GetRoundName()
{
	switch (Round)
	{
		case  1: return "The Sheep";
		case  2: return "The Sheep Army";
		case  3: return "CHIPMUNKS!!!";
		case  4: return "The Ostrich";
		case  5: return "The Mothersheep";
		case  6: return "The Dog";
		case  7: return "Chimps";
		case  8: return "The Wolf";
		case  9: return "The Gorilla";
		case 10: return "FINAL! The Bear";
		default: return "";
	}
}

void sWorld::InitiateRound()
{
	Players.push_back(&Player);
	sPlayer *enemy = new sPlayer(1);
	switch (Round)
	{
		// The Sheep
		case 1:{
			Width = 768;
			Height = 448;
			PlacementArea = ZL_Rectf(s(178), s(20), s(642), s(172));
			enemy->AddToTeam(new cSheep(ZL_Vector(s(417), s(330)), s(1.567256)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(626), s(388)), s(-1.336487)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(645), s(272)), s(-1.530000)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(667), s(148)), s(1.890000)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(738), s(91)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(257), s(406)), s(4.353183)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(218), s(349)), s(2.290000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(51), s(210)), s(5.643183)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(114), s(175)), s(0.640000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(44), s(128)), s(3.933185)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(97), s(105)), s(5.383184)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(39), s(54)), s(2.160001)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(92), s(38)), s(2.589999)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(754), s(46)), s(0.470000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(561), s(412)), s(0.000000)));
			static const unsigned short water_round_1[] = { 0,0,0,0,0,0,0,192,0xc000,49,0x3fc0,0xc000,63,0xfc0,0xc000,31,0x3fe0,0xe000,127,0x7fe0,0xe000 };
			RoundBonus = 350;
			GenerateMap(water_round_1, sizeof(water_round_1));
			}break;

		// Sheep army
		case 2:{
			Width = 864;
			Height = 672;
			PlacementArea = ZL_Rectf(s(263), s(449), s(732), s(592));
			enemy->AddToTeam(new cSheep(ZL_Vector(s(239), s(133)), s(1.438849)));
			enemy->AddToTeam(new cSheep(ZL_Vector(s(666), s(199)), s(4.819203)));
			enemy->AddToTeam(new cSheep(ZL_Vector(s(629), s(122)), s(5.372124)));
			enemy->AddToTeam(new cSheep(ZL_Vector(s(201), s(192)), s(0.219911)));
			enemy->AddToTeam(new cSheep(ZL_Vector(s(155), s(139)), s(6.193717)));
			enemy->AddToTeam(new cSheep(ZL_Vector(s(742), s(126)), s(2.111150)));
			enemy->AddToTeam(new cSheep(ZL_Vector(s(78), s(150)), s(0.791681)));
			enemy->AddToTeam(new cSheep(ZL_Vector(s(797), s(174)), s(4.090354)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(73), s(265)), s(0.050000)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(205), s(268)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(85), s(246)), s(0.050000)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(216), s(246)), s(6.273186)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(322), s(190)), s(5.433184)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(317), s(226)), s(5.483184)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(748), s(637)), s(4.663178)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(784), s(543)), s(4.563183)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(817), s(612)), s(0.420000)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(708), s(370)), s(1.570000)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(818), s(411)), s(5.133182)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(767), s(383)), s(1.020000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(565), s(358)), s(5.023184)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(209), s(534)), s(6.153184)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(145), s(495)), s(6.153184)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(200), s(461)), s(0.500000)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(83), s(466)), s(0.390000)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(131), s(431)), s(5.943186)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(56), s(317)), s(5.293184)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(141), s(316)), s(1.410000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(241), s(314)), s(5.553184)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(839), s(261)), s(0.000000)));
			RoundBonus = 400;
			static const unsigned short water_round_2[] = { 0xf1ff,0xcfff,0xf800,96,0,0,0,0,0,0,4,0,0,0,0xff00,3,0x3ff8,0,0x8302,0x1003,0x1e10,0x81c0,0,0x400,0,112,0xc000,15,0x7c00,0,0xfc0,0,254,0xf000,0x8187,67 };
			GenerateMap(water_round_2, sizeof(water_round_2));
			}break;

		// CHIPMUNKS!!!
		case 3:{
			Width = 768;
			Height = 1120;
			PlacementArea = ZL_Rectf(s(470), s(0), s(784), s(343));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(172), s(844)), s(5.109911)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(269), s(989)), s(5.476017)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(284), s(1034)), s(4.134336)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(339), s(1000)), s(5.360087)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(361), s(898)), s(4.578583)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(435), s(890)), s(4.428404)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(289), s(811)), s(5.457167)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(180), s(1009)), s(5.318937)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(320), s(935)), s(4.848582)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(400), s(1013)), s(3.459734)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(415), s(958)), s(4.056549)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(241), s(854)), s(5.113099)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(173), s(899)), s(5.412211)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(197), s(962)), s(5.780618)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(348), s(1040)), s(4.812920)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(140), s(558)), s(6.043185)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(76), s(572)), s(0.039999)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(91), s(503)), s(0.290000)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(652), s(847)), s(0.120000)));
			RoundBonus = 550;
			static const unsigned short water_round_3[] = { 0xffff,0xffc0,0x80ff,0xffff,0xff81,0x81ff,0xffff,0xff00,255,0x7fff,0xff00,127,0x3fff,0xff00,63,0x1fff,0xff00,0x801f,0x7ff,0xff80,0x8001,247,0x61c0,0xc000,41,480,0xf000,1,496,0xf800,1,252,0xfe00,0x8000,0x5ff,0xffe0,3,0x696,0xc700,0x800c,0xfc7,0xff80,0x818f,0xfff,0x7f00,7,0x71f,0xbf00,0x8007,0x7fff,0xffe0,0xffcf,255 };
			GenerateMap(water_round_3, sizeof(water_round_3));
			}break;


		// The Ostrich
		case 4:{
			Width = 800;
			Height = 576;
			PlacementArea = ZL_Rectf(s(360), s(12), s(687), s(215));
			enemy->AddToTeam(new cOstrich(ZL_Vector(s(648), s(470)), s(3.689999)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(150), s(72)), s(0.156726)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(152), s(145)), s(6.139915)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(46), s(213)), s(0.650000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(42), s(146)), s(0.590000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(245), s(451)), s(1.890000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(159), s(408)), s(1.550000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(120), s(332)), s(0.650000)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(558), s(31)), s(0.100000)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(717), s(62)), s(0.109999)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(618), s(201)), s(6.103184)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(513), s(228)), s(6.243186)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(721), s(139)), s(2.379998)));
			RoundBonus = 900;
			static const unsigned short water_round_4[] = { 63,0x7f0,0,4,0,0,0,0x1000,0,32,0x6003,0xc00,508,0xfff0,15,0x1ffe,0xf000,63,0x7c00,0,496,0xc000,15,0xbf80,1,0xfff,0xff00,0xbfb3,1 };
			GenerateMap(water_round_4, sizeof(water_round_4));
			}break;

		// Mothersheep
		case 5:{
			Width = 800;
			Height = 768;
			PlacementArea = ZL_Rectf(s(250), s(26), s(550), s(248));
			enemy->AddToTeam(new cMomSheep(ZL_Vector(s(409), s(645)), s(1.542390)));
			enemy->AddToTeam(new cSheep(ZL_Vector(s(403), s(392)), s(4.703717)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(87), s(654)), s(4.423186)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(34), s(554)), s(4.813185)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(50), s(584)), s(4.833186)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(82), s(478)), s(4.773185)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(714), s(534)), s(1.739999)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(687), s(588)), s(1.760000)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(734), s(585)), s(1.630000)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(682), s(687)), s(1.870000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(273), s(362)), s(0.390000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(535), s(365)), s(5.773183)));
			RoundBonus = 1550;
			static const unsigned short water_round_5[] = { 0x18ff,0xffff,0xf800,511,0xfff0,0xe003,0x7ff,0x7dc0,0x800f,0x1e73,0xef00,61,0xfbff,0xfe00,0x7ff,0xfffe,0xf001,0x47f,0x3fe0,0,60,0x7000,0,64,0x8000,0,0,0,0,0,0,24,0x3000,0,112,0xf000,0x8417,225 };
			GenerateMap(water_round_5, sizeof(water_round_5));
			}break;

		// The Dog
		case 6:{
			Width = 832;
			Height = 384;
			PlacementArea = ZL_Rectf(s(15), s(18), s(215), s(363));
			enemy->AddToTeam(new cDog(ZL_Vector(s(640), s(221)), s(3.170002)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(533), s(235)), s(3.149911)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(739), s(237)), s(3.159911)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(593), s(126)), s(3.329910)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(638), s(306)), s(3.169911)));
			enemy->AddToTeam(new cChipmunk(ZL_Vector(s(711), s(124)), s(3.259911)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(249), s(362)), s(3.803188)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(325), s(358)), s(0.860000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(415), s(63)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(829), s(12)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(559), s(396)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(202), s(214)), s(0.000000)));
			RoundBonus = 1000;
			static const unsigned short water_round_6[] = { 0xffc1,0x84ff,0x1f3f,0x3c04,0x2000,0,0,0,0,0,0,0,0,0,0,128,24,0xf007,0x3c11,242 };
			GenerateMap(water_round_6, sizeof(water_round_6));
			}break;

		// Chimps
		case 7:{
			Width = 800;
			Height = 480;
			PlacementArea = ZL_Rectf(s(15), s(18), s(302), s(213));
			enemy->AddToTeam(new cChimp(ZL_Vector(s(156), s(418)), s(0.409999)));
			enemy->AddToTeam(new cChimp(ZL_Vector(s(484), s(383)), s(2.070001)));
			enemy->AddToTeam(new cChimp(ZL_Vector(s(485), s(181)), s(1.746816)));
			enemy->AddToTeam(new cChimp(ZL_Vector(s(216), s(323)), s(0.836816)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(512), s(310)), s(0.040001)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(156), s(205)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(271), s(182)), s(5.603181)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(2), s(471)), s(0.830000)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(-4), s(425)), s(1.429999)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(-3), s(371)), s(2.049999)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(-140), s(208)), s(5.833183)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(629), s(649)), s(4.443184)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(273), s(-95)), s(0.680000)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(945), s(374)), s(5.583186)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(793), s(121)), s(5.433182)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(793), s(47)), s(0.970000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(379), s(205)), s(1.090000)));
			RoundBonus = 1400;
			static const unsigned short water_round_7[] = { 258,0x600,0,0xc000,63,0xc060,0x4000,0,128,96,0xc001,0x3ff,0xf300,0xfff,0x3832,48,0xc010,0x2000,256,0,0,0,780,3 };
			GenerateMap(water_round_7, sizeof(water_round_7));
			}break;

		// The Wolf
		case 8:{
			Width = 800;
			Height = 480;
			PlacementArea = ZL_Rectf(s(15), s(16), s(331), s(336));
			enemy->AddToTeam(new cWolf(ZL_Vector(s(626), s(400)), s(3.839998)));
			enemy->AddToTeam(new cDog(ZL_Vector(s(696), s(356)), s(4.390005)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(729), s(471)), s(0.989999)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(774), s(437)), s(3.613186)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(462), s(30)), s(6.073185)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(-15), s(235)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(-12), s(219)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(-8), s(203)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(-5), s(188)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(-2), s(173)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(-44), s(293)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(147), s(384)), s(0.799999)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(234), s(423)), s(4.773182)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(325), s(452)), s(5.053183)));
			RoundBonus = 1700;
			static const unsigned short water_round_8[] = { 0x1987,0x7e0,28,2,0,0,0,0x2000,0,64,0xc010,0x2000,128,192,0x8000,0,0,56,0xf000,0,0x7e0,0xe000,127,96 };
			GenerateMap(water_round_8, sizeof(water_round_8));
			}break;

		// The Gorilla
		case 9:{
			Width = 576;
			Height = 1024;
			PlacementArea = ZL_Rectf(s(79), s(13), s(335), s(264));
			enemy->AddToTeam(new cChimp(ZL_Vector(s(201), s(918)), s(4.446816)));
			enemy->AddToTeam(new cChimp(ZL_Vector(s(373), s(866)), s(4.286815)));
			enemy->AddToTeam(new cGorilla(ZL_Vector(s(286), s(894)), s(4.330000)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(496), s(955)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(447), s(1001)), s(0.600000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(545), s(917)), s(0.570000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(116), s(998)), s(4.713184)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(508), s(723)), s(5.183182)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(524), s(853)), s(1.559999)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(506), s(795)), s(1.630000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(72), s(764)), s(5.793184)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(48), s(845)), s(0.660001)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(78), s(921)), s(0.460000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(367), s(699)), s(0.540000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(477), s(388)), s(3.883182)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(141), s(548)), s(5.733185)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(355), s(582)), s(5.733185)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(386), s(233)), s(5.323187)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(456), s(308)), s(4.713184)));
			RoundBonus = 2600;
			static const unsigned short water_round_9[] = { 0xfe27,0xf81f,0xc03f,255,0x3ff,0xffc,0x7fe0,0xff80,0xfc03,0xc03f,255,0x7ff,0x3ff8,0xfce0,0xfb00,0xde01,0xfc03,0xf807,0xe00f,0xc03f,255,511,0x7fb,0x1fa6,0x7ec8,0x6000,0x8000,1,2,8,0,256,0x400,0x3000,0xe000,0xc3f0 };
			GenerateMap(water_round_9, sizeof(water_round_9));
			}break;

		// The Bear
		case 10:{
			Width = 1664;
			Height = 704;
			PlacementArea = ZL_Rectf(s(1375), s(226), s(1646), s(460));
			enemy->AddToTeam(new cBear(ZL_Vector(s(241), s(229)), s(0.356815)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(770), s(261)), s(0.850000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(736), s(505)), s(5.183184)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(853), s(440)), s(5.753185)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(719), s(434)), s(1.490000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(769), s(393)), s(0.770000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(689), s(358)), s(1.630000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(833), s(284)), s(5.633184)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(789), s(474)), s(5.303185)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(755), s(312)), s(5.573182)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(824), s(361)), s(0.870000)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(686), s(280)), s(0.540000)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(1659), s(317)), s(4.713187)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(1669), s(317)), s(4.603185)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(1683), s(316)), s(4.563182)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(1691), s(315)), s(4.413181)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(1722), s(304)), s(4.093185)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(1701), s(311)), s(4.303183)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(1715), s(306)), s(4.293186)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(1735), s(294)), s(4.103185)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(1748), s(285)), s(4.083182)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(1761), s(277)), s(4.013182)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(1688), s(222)), s(0.960000)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(1704), s(420)), s(1.229998)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(1795), s(384)), s(4.313185)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(1844), s(284)), s(4.743185)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(1779), s(227)), s(4.443184)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(1562), s(179)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(1178), s(122)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(1276), s(116)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::FENCE, ZL_Vector(s(1209), s(129)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(1375), s(162)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(169), s(58)), s(3.259999)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(122), s(115)), s(2.550000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(77), s(165)), s(1.850000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(55), s(245)), s(5.273183)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(59), s(346)), s(5.343184)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(36), s(436)), s(6.183186)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(90), s(581)), s(0.459999)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(68), s(509)), s(1.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::ROCK, ZL_Vector(s(1136), s(619)), s(0.000000)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(1063), s(631)), s(0.259999)));
			Obstacles.push_back(new sObstacle(sObstacle::PALM, ZL_Vector(s(1210), s(607)), s(0.900000)));
			RoundBonus = 5000;
			static const unsigned short water_round_10[] = { 0xfeff,0xfff3,0xffff,255,0xf82c,0xffff,0x7ff,0,0xfffc,0x1fff,0,0x3e00,0xffc0,1,0,0xe000,19,0,0,259,0,0,0x1000,0,0,0,0,0,0,16,0,0,256,0,0,0x1000,0,0,0,0,0,0,0,0,0,0,0,0,0x1800,0,0,0xf000,3,0,0xe200,31,0,0xf000,0x71ff,0,0x8000,0x7fff,15,0,0xfff8,0x1fff,0x2000,0xfff6,0xffff,0xb0ff,0xffff,255 };
			GenerateMap(water_round_10, sizeof(water_round_10));
			}break;

		// Restart game
		default:
			Round = 1;
			InitiateRound();
			return;
	}
	Players.push_back(enemy);
	for (vector<cAnimatedFigure*>::iterator itEnemy = enemy->Team.begin(); itEnemy != enemy->Team.end(); ++itEnemy)
		(*itEnemy)->is_placed = true;
}

ZL_Sound* sWorld::LoadSound(const char* file)
{
	if (Sounds.find(file) != Sounds.end()) return Sounds[file];
	ZL_Sound *output = new ZL_Sound(file);
	Sounds[file] = output;
	return output;
}

void sPlayer::Think()
{
	teamAlive = 0;
	for (vector<cAnimatedFigure*>::iterator itFigure = Team.begin(); itFigure != Team.end(); ++itFigure)
	{
		if (!isHuman && (*itFigure)->Health) // Have AI think about move
		{
			(*itFigure)->ThinkMove();
			(*itFigure)->ThinkAttack();
		}
		(*itFigure)->Calculate(); // Calculate/execute current figure's moves
		if ((*itFigure)->Health || (*itFigure)->CurrentAnimation) teamAlive++; // Determine if figure is still alive
	}
}

void sPlayer::AddToTeam(cAnimatedFigure *figure)
{
	Team.push_back(figure);
	figure->Owner = this;
}

void cAnimation::Start()
{
	Done = false;
	PicIndex = 0;
	NextPicture();
}

void cAnimation::NextPicture()
{
	if (Type == LOOP || Type == PLAYONCE) NextPicTick = World.Time + Step;
	else if (Type == SHORTBLINK)
	{
		if (PicIndex == 0)
			NextPicTick = World.Time + rand() % (Step + 1000);
		else
			NextPicTick = World.Time + ShortBlinkDuration;
	}
}

void cAnimation::Calculate()
{
	if (World.Time < NextPicTick) return;
	if (PicIndex+1 >= Tiles.size())
	{
		if (Type == PLAYONCE) { Done = true; return; }
		PicIndex = 0;
	}
	else PicIndex++;
	NextPicture();
}

void cAnimation::Draw(ZL_Vector pos, float viewAngle)
{
	if (Layers && Layers->size() != 0)
		for (vector<ZL_Surface>::iterator it = Layers->begin(); it != Layers->end(); ++it)
			it->SetTilesetIndex(Tiles[PicIndex]).Draw(pos, viewAngle);
}

cAnimatedFigure::cAnimatedFigure(eFigureType FigureType, ZL_Surface Picture, ZL_String Name, ZL_String Description, int Price,
								 scalar HealthMax, scalar HitBoxRadius, scalar Strength, scalar Defense, scalar Speed,
								 bool FriendlyFire, eWeapon WeaponType, unsigned int MoveReactionTime)
	: FigureType(FigureType), Picture(Picture), HealthMax(HealthMax), Health(HealthMax), HitBoxRadius(HitBoxRadius), Strength(Strength), Defense(Defense), Speed(Speed),
	  FriendlyFire(FriendlyFire), Price(Price), Owner(NULL), Weapon(WeaponType), WeaponUnarmed(WeaponType == WEAPON_THROWKNIFE ? WEAPON_KNIFE : WEAPON_NONE),
	  SpecialWeapon(SWEAPON_NONE), MoveReactionTime(MoveReactionTime), NextMovement(World.Time-(rand()%3000)), NextAttack(World.Time),
	  AttackTarget(NULL), FleeTarget(NULL), specialAttackLaunched(false), Name(Name), Description(Description), CurrentAnimation(NULL), PreviousAnimation(NULL)
{ Weapon.Owner = this; WeaponUnarmed.Owner = this; }

void cAnimatedFigure::LoadSound(const char* file)
{
	if (!Sound) Sound = *World.LoadSound(file);
}

int cAnimatedFigure::GetMoveAnimStep()
{
	return 2 * (200 - Speed);
}

void cAnimatedFigure::SetAction(eAction action)
{
	if (Action == action) return;

	Action = action;
	switch (action)
	{
	case cAnimatedFigure::MOVE:
		StartAnimation(AnimationMove);
		break;
	case cAnimatedFigure::ATTACK:
		StartAnimation(AnimationAttack);
		break;
	case cAnimatedFigure::DIZZY:
		// Maybe add a few rotating stars above figure's head?
		break;
	case cAnimatedFigure::DEAD:
		StartAnimation(AnimationDeath);
		break;
	case cAnimatedFigure::STANDING:
		StartAnimation(AnimationStand);
		break;
	default:
		break;
	}
}

void cAnimatedFigure::StartAnimation(cAnimation& Animation)
{
	if (CurrentAnimation == &Animation || !Animation.Layers || !Animation.Tiles.size()) return;
	if (Health == 0 && &Animation != &AnimationDeath) return;
	if (CurrentAnimation && CurrentAnimation->Type != cAnimation::PLAYONCE) PreviousAnimation = CurrentAnimation;
	CurrentAnimation = &Animation;
	CurrentAnimation->Start();
}

void cAnimatedFigure::DrawHealthBar()
{
	ZL_Vector pos(Pos.x - 26, Pos.y + HitBoxRadius + 5);
	scalar healthWidth = (s(50 )* Health / HealthMax);
	if (healthWidth < 0) healthWidth = 0;
	ZL_Display::DrawRect(ZL_Rectf(pos, 52, 5), ZL_Color::Black, ZL_Color::Red);
	ZL_Display::DrawRect(ZL_Rectf(ZL_Vector(pos.x + 1, pos.y + 1), healthWidth, 3), ZL_Color::Green, ZL_Color::Green);
}

bool cAnimatedFigure::AddPath(ZL_Vector& point, bool clear_current)
{
	if (!is_placed || Action == DIZZY) return false;
	if (clear_current || Path.size() <= 1)
	{
		Path.clear();
		Path.push_back(Pos);
		ViewAngle = (point - Pos).GetAngle();
		NextMovement = World.Time + MoveReactionTime;
		AttackTarget = NULL;
		for (vector<sPlayer*>::iterator itPlayer = World.Players.begin(); itPlayer != World.Players.end(); ++itPlayer)
		{
			if (!CanAttack(*itPlayer)) continue;
			for (vector<cAnimatedFigure*>::iterator itTarget = (*itPlayer)->Team.begin(); itTarget != (*itPlayer)->Team.end(); ++itTarget)
				if ((*itTarget)->Health && ((*itTarget)->Pos - point) <= 5 + (*itTarget)->HitBoxRadius)
					AttackTarget = *itTarget;
		}
		ForceMoveUntil = (AttackTarget ? 0 : NextMovement + 500);
		NextStepSound = NextMovement;
	}
	//check if the added path part would be colliding with an obstacle. If so, cancel path inside the obstacle
	if ((point - Path.back()) < 3) return false;
	ZL_Vector CollisionCheckStartPos = Path.back(), CollisionCheckPos = point;
	while (World.Collide(CollisionCheckPos, HitBoxRadius+2, &CollisionCheckStartPos, true))
	{
		if (CollisionCheckPos == CollisionCheckStartPos)
		{
			CollisionCheckPos = point;
			CollisionCheckStartPos += (point - CollisionCheckStartPos).Norm();
			if ((CollisionCheckStartPos - CollisionCheckPos) <= 3) return false;
		}
		else if ((CollisionCheckStartPos - CollisionCheckPos) <= 3) return false;
		else { point = CollisionCheckPos; break; }
	}
	//Path has no collisions
	Path.push_back(point);
	//Find a AttackTarget to the added path point
	if (!AttackTarget && (AttackTarget = FindTargetAlongPath(!clear_current)) && (ForceMoveUntil)) ForceMoveUntil = 0;
	SetAction(MOVE);
	return true;
}

cAnimatedFigure* cAnimatedFigure::FindTargetAlongPath(bool OnlyPathBack)
{
	if (Path.size() < 2) return NULL;
	scalar until = (OnlyPathBack ? 10 : 0);
	vector<ZL_Vector>::iterator it = (OnlyPathBack ? Path.end()-2 : Path.begin());
	for (ZL_Vector* pPrev = &*it; ++it != Path.end(); pPrev = &*it)
	{
		ZL_Vector diff(*it - *pPrev);
		scalar difflen = diff.GetLength();
		while ((until + difflen) >= 10)
		{
			ZL_Vector checkpos = *pPrev + diff * ((10 - until) / difflen);
			until -= 10;
			for (vector<sPlayer*>::iterator itPlayer = World.Players.begin(); itPlayer != World.Players.end(); ++itPlayer)
			{
				if (!CanAttack(*itPlayer)) continue;
				for (vector<cAnimatedFigure*>::iterator itTarget = (*itPlayer)->Team.begin(); itTarget != (*itPlayer)->Team.end(); ++itTarget)
					if ((*itTarget)->Health && ((*itTarget)->Pos - checkpos) <= 5 + (*itTarget)->HitBoxRadius)
						return *itTarget;
			}
		}
		until += difflen;
	}
	return NULL;
}

bool cAnimatedFigure::CanAttack(sPlayer *player)
{
	return FriendlyFire || player != Owner ||
		   !(player->id & Owner->allianceMap);
}

void cAnimatedFigure::Draw()
{
	if (!is_placed || !CurrentAnimation) return;
	if (CurrentAnimation->Done) CurrentAnimation = PreviousAnimation;
	if (!CurrentAnimation) return;
	scalar ShadowScale = HitBoxRadius/s(25);
	srfShadow.Draw(Pos, ShadowScale, ShadowScale);
	CurrentAnimation->Draw(Pos, ViewAngle);
	if (FigureType == TEAMMEMBER && Weapon.Particles)
		Weapon.Particles.Draw();
}

void cAnimatedFigure::Calculate()
{
	if (CurrentAnimation) CurrentAnimation->Calculate();
	if (Health <= 0) return;
	if (World.Time >= NextAttack) Attack(); // Do not react instantly
	if (World.Time >= NextMovement) Move(); // Do not react instantly
}

bool cAnimatedFigure::IsInRange(cAnimatedFigure *target)
{
	return Pos - target->Pos <= HitBoxRadius + target->HitBoxRadius + 1 + (Weapon.Ammo ? Weapon.Range : WeaponUnarmed.Range);
}

void cAnimatedFigure::Move()
{
	if (!AttackTarget && !FleeTarget && !Path.size())
	{
		StartAnimation(AnimationStand);
		return;
	}

	if (Action == DIZZY && DizzyUntil && World.Time >= DizzyUntil) RegainControl();
	if (Action == MOVE && World.Time > ForceMoveUntil)
		for (vector<sPlayer*>::iterator itPlayer = World.Players.begin(); itPlayer != World.Players.end(); ++itPlayer)
		{
			if (!CanAttack(*itPlayer)) continue;
			for (vector<cAnimatedFigure*>::iterator itTarget = (*itPlayer)->Team.begin(); itTarget != (*itPlayer)->Team.end(); ++itTarget)
				if ((*itTarget)->Health && ((*itTarget)->Pos - Pos) <= HitBoxRadius + (*itTarget)->HitBoxRadius)
				{
					AttackTarget = *itTarget;
					SetAction(STANDING);
					ViewAngle = (AttackTarget->Pos - Pos).GetAngle();
					break;
				}
		}

	if (Path.size() > 0)
	{
		if (PathTotal <= 0 || Path.size() == 1)
		{
			Pos = *Path.rbegin();
			SetAction(STANDING);
			Path.clear();
		}
		else if (AttackTarget && IsInRange(AttackTarget))
		{
			if (CurrentAnimation == &AnimationMove) StartAnimation(AnimationStand);
		}
		else
		{
			scalar until = 0;
			scalar speed = (Action != DIZZY ? Speed : DizzySpeed);
			if (Action != DIZZY) SetAction(MOVE);
			while (Path.size() > 1)
			{
				float elapsed = ZLELAPSEDF(speed * Weapon.SpeedModifier);
				ZL_Vector diff(*(Path.begin() + 1) - Path.front());
				scalar difflen = diff.GetLength();
				if ((until + difflen) >= elapsed)
				{
					ViewAngle = diff.GetAngle();
					Pos = Path.front() + (diff * (elapsed - until) / difflen);
					if (World.Collide(Pos, HitBoxRadius, &Path.front()))
					{
						Path.clear();
						StartAnimation(AnimationStand);
						if (Action == DIZZY) RegainControl();
					}
					else
					{
						Path.front() = Pos;
						PathTotal -= elapsed;
					}
					break;
				}
				until += difflen;
				Path.erase(Path.begin());
			}
			if (World.Time >= NextStepSound)
			{
				sndStep.Play();
				NextStepSound = World.Time + 48000 / speed;
				if (!AttackTarget) AttackTarget = FindTargetAlongPath();
			}
		}
	}
}

void cAnimatedFigure::Attack()
{
	if (AttackTarget && AttackTarget->Health <= 0) AttackTarget = NULL;
	if (!AttackTarget) return;
	sWeapon *UseWeapon = (Weapon.Ammo ? &Weapon : &WeaponUnarmed);
	if (!IsInRange(AttackTarget)) return;
	if (AttackTarget->AttackTarget == NULL && AttackTarget->ForceMoveUntil <= World.Time) AttackTarget->AttackTarget = this;

	if (UseWeapon->TryAttack(AttackTarget))
	{
		SetAction(ATTACK);
		NextAttack = World.Time + UseWeapon->ReactionTime - (Speed * UseWeapon->AttackSpeedModifier);
		NextMovement = NextAttack + MoveReactionTime;
	}
}

void cAnimatedFigure::LaunchSpecialAttack(ZL_Vector &target)
{
	specialAttackLaunched = true;
	SpecialWeapon.SpecialAttack(target);
}

void cAnimatedFigure::ReceiveAttack(cAnimatedFigure *attacker, float attackStrength, float armorPiercing)
{
	assert(Health > 0);
	if (!Sound.IsPlaying()) Sound.Play();
	float attack = attackStrength * (100 - (Defense * Weapon.DefenseModifier)) / 100 + armorPiercing;
	if (attack <= 0) return;
	Bleed(attack);
	Health -= attack;
	#ifdef ZILLALOG
	ZL_LOG0(Name, ZL_String("Damage received: ") << attack);
	ZL_LOG0(Name, ZL_String("Health remaining: ") << Health);
	#endif
	if (Health <= 0)
	{
		Health = 0;
		Die(attacker);
		CurrentAnimation = PreviousAnimation = NULL; //hide when death animation is done or no death animation defined
		StartAnimation(AnimationDeath);
	}
}

void cAnimatedFigure::LoseControl(const ZL_Vector &direction, scalar unconSpeed, unsigned int unconTime)
{
	SetAction(DIZZY);
	AttackTarget = NULL;
	Path.clear();
	if (!direction)
	{
		// TODO: Implement random path walking
	}
	else if (direction.GetLength() != 0)
	{
		ZL_Vector moveVec = direction.VecNorm() * unconSpeed * Weapon.SpeedModifier;
		Path.push_back(Pos);
		Path.push_back(Pos + ZL_Vector(moveVec));
		PathTotal = moveVec.GetLength();
	}
	DizzySpeed = unconSpeed;
	DizzyUntil = unconTime;
}

void cAnimatedFigure::RegainControl()
{
	SetAction(STANDING);
	Path.clear();
}

void cAnimatedFigure::Bleed(float attack)
{
	if (attack <= 0) return;
	int bloodAmount = 100 * attack / HealthMax;
	if (bloodAmount > 100) bloodAmount = 100;
	if (bloodAmount < 3) bloodAmount = 3;
	particleBlood.Spawn(bloodAmount, Pos, 0, 10, 10);
	Graphic.RenderToBegin();
	for (int i = 0; i < bloodAmount; ++i)
	{
		ZL_Vector pos = (Pos + (ZL_Vector(1, 0) *= RAND_RANGE(5, HitBoxRadius+10)).Rotate(RAND_FACTOR*PI2));
		if (pos.x < 10) pos.x = 10;
		if (pos.y < 10) pos.y = 10;
		if (pos.x > World.Width-10) pos.x = World.Width-10;
		if (pos.y > World.Height-10) pos.y = World.Height-10;
		scalar scale = RAND_RANGE(0.5, 1.1);
		srfBlood.Draw(pos, RAND_FACTOR*PI2, scale, scale);
	}
	Graphic.RenderToEnd();
}

void cAnimatedFigure::Die(cAnimatedFigure *killer)
{
	ZL_LOG0(Name, "died");
	int reward = Price / 10;

	if (Weapon.Type == WEAPON_BOMB)
		Weapon.TryAttack(); // When figure dies with bomb equipped, bomb always explodes (even when figure is dizzy)
	if (Owner) Owner->Budget += Weapon.GetPrice(); // Retrieve money
	Path.clear();
	SetAction(DEAD);

	if (!killer->Owner) return;
	killer->Owner->Budget += reward;
	if (killer->Owner->RewardCounts.find(Name.to_upper()) != killer->Owner->RewardCounts.end()) killer->Owner->RewardCounts[Name.to_upper()].count++;
	else { sPlayer::sRewardCount rc = { reward, 1 }; killer->Owner->RewardCounts[Name.to_upper()] = rc; }
	killer->AttackTarget = killer->FindTargetAlongPath();
}

void cAnimatedFigure::PathCalculate()
{
	if (!Health) return;
	PathTotal = 0;
	if (!Path.size()) return;
	for (vector<ZL_Vector>::iterator itPrev = Path.begin(), it = Path.begin()+1 ; it != Path.end(); ++it, ++itPrev)
		PathTotal += (*it - *itPrev).GetLength();
	StartAnimation(AnimationMove);
}

void cAnimal::Draw()
{
	cAnimatedFigure::Draw();
	if (!Health) return;
	#if DEBUG
	ZL_Display::DrawCircle(Pos, HitBoxRadius, ZL_Color::Red); // Hit radius
	ZL_Display::DrawCircle(Pos, SightRadius, ZL_Color::Yellow); // Offensive radius
	if (SightAngle < PI2) ZL_Display::DrawLine(Pos, Pos+ZL_Vector::Right.VecRotate(ViewAngle+SightAngle/2)*SightRadius, ZL_Color::Yellow);
	if (SightAngle < PI2) ZL_Display::DrawLine(Pos, Pos+ZL_Vector::Right.VecRotate(ViewAngle-SightAngle/2)*SightRadius, ZL_Color::Yellow);
	#endif
	DrawHealthBar();
}

void cAnimal::ThinkMove(bool force_wander)
{
	if (AttackTarget && (AttackTarget->Health == 0 || (Pos - AttackTarget->Pos) > SightRadius*3)) AttackTarget = NULL;
	if (FleeTarget && (AttackTarget || FleeTarget->Health == 0)) FleeTarget = NULL;
	if (FleeTarget)
	{
		scalar StopFleeLength = SightRadius*2;
		if (World.Time >= FleeUntil) StopFleeLength = HitBoxRadius + FleeTarget->HitBoxRadius + 50;
		if ((Pos - FleeTarget->Pos) > StopFleeLength) FleeTarget = NULL;
	}
	if (!force_wander && !AttackTarget && !FleeTarget)
	{
		for (vector<sPlayer*>::iterator itPlayer = World.Players.begin(); itPlayer != World.Players.end(); ++itPlayer)
		{
			if (!CanAttack(*itPlayer)) continue;
			for (vector<cAnimatedFigure*>::iterator itFigure = (*itPlayer)->Team.begin(); itFigure != (*itPlayer)->Team.end(); ++itFigure)
			{
				if ((*itFigure)->Health == 0) continue;
				ZL_Vector diff((*itFigure)->Pos - Pos);
				if (diff > SightRadius) continue;
				scalar rel = diff.GetRelAngle(ViewAngle);
				if (rel < -SightAngle/2 || rel > SightAngle/2) continue;
				ReactTo(*itFigure);
				break;
			}
		}
	}
	if (force_wander || (!AttackTarget && !FleeTarget && NextMovement < World.Time - 3000))
	{
		ZL_LOG0(Name, "Decided to wander");
		NextMovement = World.Time + (rand()%1000);
		WanderUntil = NextMovement + 1000 + (rand()%2000);
		ZL_Vector WanderTarget = ZL_Vector(World.Width*RAND_FACTOR, World.Height*RAND_FACTOR);
		for (int retry = 0; retry < 100 && World.Collide(WanderTarget, HitBoxRadius+2, &Pos, true); retry++)
			WanderTarget = ZL_Vector(World.Width*RAND_FACTOR, World.Height*RAND_FACTOR);
		AddPath(WanderTarget, true);
		PathCalculate();
	}
}

void cAnimal::ThinkAttack()
{
	// TODO: Implement animal's default attack AI
}

void cAnimal::ReactTo(cAnimatedFigure *figure)
{
	if (AttackTarget || FleeTarget) return; //already busy
	ZL_Vector PosTarget = figure->Pos;
	if (World.Collide(PosTarget, HitBoxRadius+2, &Pos, true))
	{
		//no path to target, ignore and run somewere
		if (WanderUntil < World.Time) ThinkMove(true);
		return;
	}
	if (Speed && RAND_FACTOR > Aggressiveness)
	{
		FleeTarget = figure; //   Know who you're fleeing from
		ZL_Vector postmp = (Pos - figure->Pos).VecNorm() * 10;
		AddPath(postmp, true); // Know where you're fleeing to
		PathCalculate();
		FleeUntil = World.Time + 1000 + (rand()%2000);
	}
	else AttackTarget = figure;
	// Attack/Flee target can't be anyone other than target here, or we would have left the method long ago
	ZL_LOG1(Name, (AttackTarget ? "Decided to attack %s" : "Decided to flee from %s"), figure->Name.c_str());
	if (WanderUntil <= World.Time) NextMovement = World.Time + MoveReactionTime; //only react slow when not wandering into enemy
	NextAttack = World.Time + AttackReactionTime;
}

sWeapon::sWeapon(eWeapon type)
{
	Type = type;
	//defaults
	Range = 0;
	Radius = 0;
	AmmoPrice = s(0.0);
	switch (type)
	{
	case WEAPON_MACHINEGUN:
		Name                = "MG";
		ReactionTime        = 100;
		MaxAmmo             = 600;
		Power               = s(21);
		ArmorPiercing       = s(0.1);
		AttackModifier      = s(0.0);
		DefenseModifier     = s(0.7);
		SpeedModifier       = s(0.2);
		AttackSpeedModifier = s(0.0);
		FriendlyFire        = false;
		Price               = 1500;
		AmmoPrice           = s(2.0);
		Range               = s(150.0);
		break;
	case WEAPON_CHAINSAW:
		Name                = "Chainsaw";
		ReactionTime        = 10;
		MaxAmmo             = 6000;
		Power               = s(1);
		ArmorPiercing       = s(1);
		AttackModifier      = s(0.01);
		DefenseModifier     = s(0.9);
		SpeedModifier       = s(0.3);
		AttackSpeedModifier = s(0.0);
		FriendlyFire        = false;
		Price               = 1000;
		AmmoPrice           = s(0.1);
		break;
	case WEAPON_KNIFE:
		Name                = "Knife";
		ReactionTime        = 700;
		MaxAmmo             = -1;
		Power               = s(5);
		ArmorPiercing       = s(0.0);
		AttackModifier      = s(1.2);
		DefenseModifier     = s(1.0);
		SpeedModifier       = s(1.0);
		AttackSpeedModifier = s(0.9);
		FriendlyFire        = false;
		Price               = 12;
		break;
	case WEAPON_KNUCKLES:
		Name                = "Knuckles";
		ReactionTime        = 400;
		MaxAmmo             = -1;
		Power               = s(3);
		ArmorPiercing       = s(0);
		AttackModifier      = s(1.3);
		DefenseModifier     = s(1.1);
		SpeedModifier       = s(1.0);
		AttackSpeedModifier = s(1.0);
		FriendlyFire        = false;
		Price               = 10;
		break;
	case WEAPON_NAILCLUB:
		Name                = "Nail club";
		ReactionTime        = 1400;
		MaxAmmo             = -1;
		Power               = s(5);
		ArmorPiercing       = s(0.01);
		AttackModifier      = s(1.3);
		DefenseModifier     = s(1.1);
		SpeedModifier       = s(0.9);
		AttackSpeedModifier = s(0.3);
		FriendlyFire        = false;
		Price               = 7;
		break;
	case WEAPON_KICK:
		Name                = "Kick";
		ReactionTime        = 600;
		MaxAmmo             = -1;
		Power               = s(0);
		ArmorPiercing       = s(0);
		AttackModifier      = s(1.4);
		DefenseModifier     = s(1.0);
		SpeedModifier       = s(1.0);
		AttackSpeedModifier = s(0.7);
		FriendlyFire        = false;
		Price               = 0;
		break;
	case WEAPON_NONE:
		Name                = "Unarmed";
		ReactionTime        = 400;
		MaxAmmo             = -1;
		Power               = s(0);
		ArmorPiercing       = s(0);
		AttackModifier      = s(1.0);
		DefenseModifier     = s(1.0);
		SpeedModifier       = s(1.0);
		AttackSpeedModifier = s(1.0);
		FriendlyFire        = false;
		Price               = 0;
		break;
	case WEAPON_NINJASTAR:
		Name                = "Shuriken";
		ReactionTime        = 850;
		MaxAmmo             = 200;
		Power               = s(3);
		ArmorPiercing       = s(0);
		AttackModifier      = s(1.0);
		DefenseModifier     = s(1.0);
		SpeedModifier       = s(1.0);
		AttackSpeedModifier = s(0.6);
		FriendlyFire        = false;
		Price               = 0;
		AmmoPrice           = s(3.0);
		Range               = s(150.0);
		break;
	case WEAPON_THROWKNIFE:
		Name                = "Throwing knives";
		ReactionTime        = 700;
		MaxAmmo             = 150;
		Power               = s(3);
		ArmorPiercing       = s(0.01);
		AttackModifier      = s(1.1);
		DefenseModifier     = s(1.0);
		SpeedModifier       = s(1.0);
		AttackSpeedModifier = s(0.6);
		FriendlyFire        = false;
		Price               = 0;
		AmmoPrice           = s(4);
		Range               = s(100.0);
		break;
	case WEAPON_BOMB:
		Name                = "Suicide bomb";
		ReactionTime        = 1;
		MaxAmmo             = 1;
		Power               = s(1000);
		ArmorPiercing       = s(200);
		AttackModifier      = s(0.0);
		DefenseModifier     = s(1.0);
		SpeedModifier       = s(0.8);
		AttackSpeedModifier = s(0.0);
		Radius              = s(80.0);
		FriendlyFire        = true;
		Price               = 0;
		AmmoPrice           = s(1000.0);
		break;
	default:
		Name                = "UnknoWeapon";
		ReactionTime        = 0;
		MaxAmmo             = -1;
		Power               = s(999999);
		ArmorPiercing       = s(999999);
		AttackModifier      = s(1.0);
		DefenseModifier     = s(1.0);
		SpeedModifier       = s(1.0);
		AttackSpeedModifier = s(1.0);
		FriendlyFire        = false;
		Price               = 999999;
	}
	Ammo = MaxAmmo;
	LoadAnimations();
}

void sWeapon::LoadAnimations()
{
	ActivateAnimation.Layers = NULL;
	ActivateAnimation.Tiles.clear();
	Layers.clear();
	OwnerLayer = ZL_Surface();
	Sound = ZL_Sound();
	Particles = ZL_ParticleEffect();
	switch(Type)
	{
		case WEAPON_MACHINEGUN:
			OwnerLayer = ZL_Surface("Data/weapon_machinegun.png").SetDrawOrigin(ZL_Origin::TopLeft).SetRotateOrigin(ZL_Origin::TopLeft).SetTilesetClipping(2,2);
			Sound = *World.LoadSound("Data/shoot.ogg");
			break;
		case WEAPON_KNIFE:
			OwnerLayer = ZL_Surface("Data/weapon_knife.png").SetDrawOrigin(ZL_Origin::TopLeft).SetRotateOrigin(ZL_Origin::TopLeft).SetTilesetClipping(2,2);
			Sound = *World.LoadSound("Data/knife.ogg");
			break;
		case WEAPON_THROWKNIFE:
			OwnerLayer = ZL_Surface("Data/weapon_knife.png").SetDrawOrigin(ZL_Origin::TopLeft).SetRotateOrigin(ZL_Origin::TopLeft).SetTilesetClipping(2,2);
			Sound = *World.LoadSound("Data/knife.ogg");
			Particles = ZL_ParticleEffect(150, 10);
			Particles.AddParticleImage(ZL_Surface("Data/particle_knife.png"), 10);
			Particles.AddBehavior(ParticleMove = new ZL_ParticleBehavior_LinearMove(Range, 10, 0, 0, true));
			break;
		case WEAPON_NINJASTAR:
			OwnerLayer = ZL_Surface("Data/weapon_ninjastar.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(2,2);
			Sound = *World.LoadSound("Data/knife.ogg");
			Particles = ZL_ParticleEffect(100, 10);
			Particles.AddParticleImage(ZL_Surface("Data/particle_ninjastar.png"), 10);
			Particles.AddBehavior(ParticleMove = new ZL_ParticleBehavior_LinearMove(Range, 10, 0, 0, true));
			break;
		case WEAPON_KNUCKLES:
			OwnerLayer = ZL_Surface("Data/weapon_knuckles.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(2,2);
			Sound = *World.LoadSound("Data/knife.ogg");
			break;
		case WEAPON_CHAINSAW:
			OwnerLayer = ZL_Surface("Data/weapon_chainsaw.png").SetDrawOrigin(ZL_Origin::CenterLeft).SetRotateOrigin(ZL_Origin::CenterLeft).SetTilesetClipping(2,2);
			Sound = *World.LoadSound("Data/chainsaw.ogg");
			break;
		case WEAPON_NAILCLUB:
			OwnerLayer = ZL_Surface("Data/weapon_nailclub.png").SetDrawOrigin(ZL_Origin::CenterLeft).SetRotateOrigin(ZL_Origin::CenterLeft).SetTilesetClipping(2,2);
			Sound = *World.LoadSound("Data/knife.ogg");
			break;
		case WEAPON_BOMB:
			OwnerLayer = ZL_Surface("Data/weapon_bomb.png").SetDrawOrigin(ZL_Origin::CenterLeft).SetRotateOrigin(ZL_Origin::CenterLeft).SetTilesetClipping(2,2);
			Sound = *World.LoadSound("Data/explosion.ogg");
			ActivateAnimation.Setup(Layers, cAnimation::PLAYONCE, 32);
			Layers.push_back(ZL_Surface("Data/explosion.zltex").SetDrawOrigin(ZL_Origin::Center).SetScale(2).SetTilesetClipping(4, 3));
			{for (int i = 0; i < 4*3; i++) ActivateAnimation.Tiles.push_back(i);}
			break;
		default:
			break;
	}
}

bool sWeapon::TryAttack(cAnimatedFigure *pSpecificTarget)
{
	float attackPower = Owner->Strength * AttackModifier + Power;
	bool attackLaunched = Attack(Owner->HitBoxRadius + 2, attackPower, false);
	if (!attackLaunched && Range && pSpecificTarget)
	{
		attackLaunched = AttackTarget(pSpecificTarget, attackPower); // Did we hit something?
	}
	if (attackLaunched)
	{
		if (Sound) Sound.Play();
		if (Particles) { ParticleMove->behaviorAngle = Owner->ViewAngle; Particles.Spawn(1, Owner->Pos); }
		if (Radius > 0.0f)
			Attack(Owner->HitBoxRadius + Radius + 2, attackPower, true); // Attack this time in a larger radius
		if (Ammo > 0)
			Ammo--;
	}
	return attackLaunched;
}

bool sWeapon::Attack(scalar radius, float attackPower, bool splashDMG)
{
	bool attackLaunched = false;
	for (vector<sPlayer*>::iterator itPlayer = World.Players.begin(); itPlayer != World.Players.end(); ++itPlayer)
		for (vector<cAnimatedFigure*>::iterator itTarget = (*itPlayer)->Team.begin(); itTarget != (*itPlayer)->Team.end(); ++itTarget)
		{
			if ((*itTarget)->Health <= 0 || (!Owner->CanAttack(*itPlayer) && !FriendlyFire)) continue;
			scalar distance = Owner->Pos.GetDistance((*itTarget)->Pos);
			if (distance <= (radius + (*itTarget)->HitBoxRadius))
			{
				scalar dmg = attackPower;
				if (splashDMG)
					dmg -= attackPower * (distance / radius);
				attackLaunched |= AttackTarget(*itTarget, dmg);
			}
		}

	if (attackLaunched && ActivateAnimation.Layers)
	{
		ActivateAnimation.Start();
		World.Effects.push_back(new sEffect(ActivateAnimation, Owner->Pos));
	}
	return attackLaunched;
}

bool sWeapon::AttackTarget(cAnimatedFigure *target, float attackPower)
{
	if (Ammo != 0) // Do not attack, if Ammo == 0 (negative Ammo => unlimited Ammo)
	{
		Owner->ViewAngle = (target->Pos - Owner->Pos).GetAngle();
		ZL_LOG0(Owner->Name, ZL_String("Attacking " + target->Name + ": ") << attackPower);
		Owner->StartAnimation(Owner->AnimationAttack);
		target->ReceiveAttack(Owner, attackPower, ArmorPiercing);
		if (target->FigureType == cAnimatedFigure::ANIMAL) ((cAnimal*)target)->ReactTo(Owner);
		return true;
	}
	return false;
}

int sWeapon::GetPrice()
{
	return Price + (AmmoPrice * Ammo);
}

sSpecialWeapon::sSpecialWeapon(eSpecialWeapon type) : sWeapon(WEAPON_NONE)
{
}

void sSpecialWeapon::SpecialAttack(ZL_Vector &target)
{
}

sTeamMember::sTeamMember(eDude Dude, ZL_Surface Picture, ZL_String Name, ZL_String Description, int Price,
						 scalar Health, scalar Strength, scalar Defense, scalar Speed, eWeapon WeaponType, scalar radius, unsigned int MoveReactionTime)
: cAnimatedFigure(TEAMMEMBER, Picture, Name, Description, Price,
				  Health, radius, Strength, Defense, Speed, false, WeaponType, MoveReactionTime), Dude(Dude)
{
	ViewAngle = PIHALF;
	LoadAnimations();
	hired = false;
}

void sTeamMember::LoadAnimations()
{
	LoadSound("Data/ouch.ogg");

	Layers.clear();
	Layers.push_back(ZL_Surface("Data/man_pants.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(2,2));
	Layers.push_back(ZL_Surface("Data/man_body.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(2,2));

	if (Dude == DUDE_MIKE)
	{
		Layers.push_back(ZL_Surface("Data/man_shirt.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(2,2));
		Layers[0].SetColor(ZLRGBFF(208, 182, 146)); //pants color
		Layers[1].SetColor(ZLRGBFF(255, 158, 72)); //body color
		Layers[2].SetColor(ZLRGBFF(208, 182, 146)); //armband/shirt color
		Layers.push_back(ZL_Surface("Data/man_head.png").SetDrawOrigin(ZL_Origin::Center).SetColor(ZLRGBFF(35,26,6)));
	}
	else if (Dude == DUDE_JACK)
	{
		Layers.push_back(ZL_Surface("Data/man_shirt.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(2,2));
		Layers[0].SetColor(ZLRGB(1  ,  .3,  .3)); //pants color
		Layers[1].SetColor(ZLRGB(1  ,  .7,  .4)); //body color
		Layers[2].SetColor(ZLRGBFF(255, 179, 15)); //armband/shirt color
		Layers.push_back(ZL_Surface("Data/man_head.png").SetDrawOrigin(ZL_Origin::Center).SetColor(ZLRGBFF(167,101,47)));
	}
	else if (Dude == DUDE_LUKE)
	{
		Layers.push_back(ZL_Surface("Data/man_shirt.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(2,2));
		Layers[0].SetColor(ZLRGB(1  ,  .3,  .3)); //pants color
		Layers[1].SetColor(ZLRGB(1  ,  .7,  .4)); //body color
		Layers[2].SetColor(ZLRGBFF(225, 109, 15)); //armband/shirt color
		Layers.push_back(ZL_Surface("Data/man_hairluke.png").SetDrawOrigin(ZL_Origin::Center).SetColor(ZLRGBFF(167,101,47)));
	}
	else if (Dude == DUDE_BETSY)
	{
		Layers.push_back(ZL_Surface("Data/man_shirt.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(2,2));
		Layers[0].SetColor(ZLRGBFF(30,  20, 100)); //pants color
		Layers[1].SetColor(ZLRGBFF(250, 170, 100)); //body color
		Layers[2].SetColor(ZLRGBFF(30, 10, 3)); //armband/shirt color
		Layers.push_back(ZL_Surface("Data/man_headbetsy.png").SetDrawOrigin(ZL_Origin::Center));
	}
	else if (Dude == DUDE_FATSTAR)
	{
		Layers.push_back(ZL_Surface("Data/man_shirt.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(2,2));
		Layers[0].SetColor(ZLRGB(1  ,  .3,  .3)).SetScale(s(1.3), s(1.1)); //pants color
		Layers[1].SetColor(ZLRGB(1  ,  .7,  .4)).SetScale(s(1.3), s(1.1)); //body color
		Layers[2].SetColor(ZLRGB(0.4  , 0.2  , 0  )).SetScale(s(1.3), s(1.1)); //armband/shirt color
		Layers.push_back(ZL_Surface("Data/man_head.png").SetDrawOrigin(ZL_Origin::Center).SetColor(ZLRGB(.01,.01,.01)));
	}
	else if (Dude == DUDE_IVAN)
	{
		Layers.push_back(ZL_Surface("Data/man_shirt.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(2,2));
		Layers[0].SetColor(ZLRGBFF(100,  20, 100)); //pants color
		Layers[1].SetColor(ZLRGBFF(250, 170, 100)); //body color
		Layers[2].SetColor(ZLRGBFF(30,  20, 100)); //armband/shirt color
		Layers.push_back(ZL_Surface("Data/man_headivan.png").SetDrawOrigin(ZL_Origin::Center));
	}
	else if (Dude == DUDE_LEON)
	{
		Layers.push_back(ZL_Surface("Data/man_shirt.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(2,2));
		Layers[0].SetColor(ZLRGBFF(30,  100, 20)); //pants color
		Layers[1].SetColor(ZLRGBFF(250, 170, 100)); //body color
		Layers[2].SetColor(ZLRGBFF(30,  100, 20)); //armband/shirt color
		Layers.push_back(ZL_Surface("Data/man_headivan.png").SetDrawOrigin(ZL_Origin::Center).SetColor(ZLRGB(.01,.01,.01)));
	}
	else if (Dude == DUDE_SHIFTER)
	{
		Layers.push_back(ZL_Surface("Data/man_shirt.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(2,2));
		Layers[0].SetColor(ZLRGBFF(200, 97, 60)); //pants color
		Layers[1].SetColor(ZLRGBFF(250, 170, 100)); //body color
		Layers[2].SetColor(ZLRGBFF(200, 97, 60)); //armband/shirt color
		Layers.push_back(ZL_Surface("Data/man_headshifter.png").SetDrawOrigin(ZL_Origin::Center));
	}

	else
	{
		Layers.push_back(ZL_Surface("Data/man_armband.png").SetDrawOrigin(ZL_Origin::Center).SetTilesetClipping(2,2));
		Layers[0].SetColor(ZLRGB(1  ,  .3,  .3)); //pants color
		Layers[1].SetColor(ZLRGB(1  ,  .7,  .4)); //body color
		Layers[2].SetColor(ZLRGB(1  , 0  , 0  )); //armband/shirt color
		Layers.push_back(ZL_Surface("Data/man_hat.png").SetDrawOrigin(ZL_Origin::Center).SetColor(ZLRGB(1,0,0)));
	}

	if (Weapon.OwnerLayer) Layers.push_back(Weapon.OwnerLayer.SetScale(Layers[1].GetScaleW(), Layers[1].GetScaleH()));

	AnimationStand.Setup(Layers);
	AnimationStand.Tiles.push_back(0);

	AnimationMove.Setup(Layers, cAnimation::LOOP, GetMoveAnimStep());
	AnimationMove.Tiles.push_back(2);
	AnimationMove.Tiles.push_back(3);

	AnimationAttack.Setup(Layers, cAnimation::PLAYONCE, Weapon.ReactionTime / 2);
	AnimationAttack.Tiles.push_back(1);

	StartAnimation(AnimationStand);
}

void sTeamMember::Draw()
{
	if (!is_placed) return;
	if (Health && Path.size() && Action != DIZZY)
	{
		if (Path.size() > 1)
		{
			scalar until = 0;
			int i = (World.RoundEnded ? World.Time : ZLTICKS)/100;
			ZL_Color pathcol = (ActivePlayerFigure == this ? ZL_Color::Green : ZLBLACK);
			ZL_Vector diff;
			vector<ZL_Vector>::reverse_iterator it = Path.rbegin();
			scalar lastdir = (*it - *(it+1)).GetAngle();
			for (ZL_Vector* pPrev = &*it; ++it != Path.rend(); pPrev = &*it)
			{
				ZL_Vector diff = *it; diff -= *pPrev;
				scalar difflen = diff.GetLength();
				while ((until + difflen) >= 10)
				{
					ZL_Vector checkpos = *pPrev + diff * ((10 - until) / difflen);
					pathcol.a = s(0.2)+(s(0.2)*s(((i++)%5)));
					srfPath.Draw(checkpos, pathcol);
					until -= 10;
				}
				until += difflen;
			}
			pathcol.a = s(0.9);
			srfPathTarget.SetScale(Layers[0].GetScaleW());
			srfPathTarget.Draw(Path.back(), lastdir, pathcol);
		}
	}
	cAnimatedFigure::Draw();
	if (!Health) return;
	#if DEBUG
	ZL_Display::DrawCircle(Pos, HitBoxRadius + Weapon.Radius, ZL_Color::Yellow); // Weapon radius
	ZL_Display::DrawCircle(Pos, HitBoxRadius, ZL_Color::Red); // Hit radius
	#endif
	cAnimatedFigure::DrawHealthBar();
	if (World.Mode == sWorld::MODE_PAUSED && ActivePlayerFigure == this)
	{
		ZL_Display::DrawCircle(Pos, HitBoxRadius+10, ZLRGBA(0,1,0,.4), ZLRGBA(0,1,0,.1));
		ZL_Display::DrawCircle(Pos, HitBoxRadius+5, ZLTRANSPARENT, ZLRGBA(0,1,0,.1));
		ZL_Display::DrawCircle(Pos, HitBoxRadius+1, ZLTRANSPARENT, ZLRGBA(0,1,0,.1));
	}
	if (AttackTarget) srfAttackCursor.Draw(AttackTarget->Pos, (ActivePlayerFigure == this ? ZL_Color::Green : ZLLUMA(.5, .5)));

	/*
	if (Weapon->Ammo >= 0)
	{
		ZL_String ammo = ZL_String(Weapon->Ammo);
		scalar treshold = Weapon->MaxAmmo / 3; // Define treshold; (Ammo > treshold) -> green, (Ammo == treshold) -> yellow, (Ammo < treshold) -> red

		scalar r = (Weapon->MaxAmmo - Weapon->Ammo) / treshold;
		scalar g = Weapon->Ammo / treshold;
		fntMain.Draw(Pos.x - (fntMain.GetDimensions(ammo).x / 2), Pos.y - 30, ammo, ZL_Color(r, g, 0));
	}
	*/
}

void sTeamMember::ThinkMove(bool force_wander)
{
	//TODO: Implement
}

void sTeamMember::ThinkAttack()
{
	//TODO: Implement
}

void sEffect::Draw()
{
	Animation.Draw(Pos, 0);
	Animation.Calculate();
}

#ifdef SCENE_EDITOR
const char* EditorFenceType(sObstacle::eType type)
{
	switch (type)
	{
		case sObstacle::FENCE: return "FENCE";
		case sObstacle::ROCK:  return "ROCK";
		case sObstacle::PALM:  return "PALM";
		default: assert(false); return "UNKNOWN";
	}
}
#endif

sObstacle::sObstacle(eType Type, const ZL_Vector& Pos, scalar Rotation) : Type(Type), RotPos(Pos, 0, 0, Rotation)
{
	switch (Type)
	{
		case FENCE:
			Picture = srfFence;
			Health = 150;
			break;
		case ROCK:
			Picture = srfRock;
			Health = 550;
			break;
		case PALM:
			Picture = srfPalm;
			Health = 250;
			RotPos.E = ZL_Vector(10, 10);
			break;
		default:
			Picture = srfBlood;
			RotPos.E = ZL_Vector(Picture.GetWidth()/2, Picture.GetHeight()/2);
	}
}

bool sObstacle::Collide(ZL_Vector& AtPos, scalar Radius, const ZL_Vector* PosMoveFrom, bool long_move)
{
	if (Health <= 0) return false;
	if ((!PosMoveFrom || !long_move) && !RotPos.Overlaps(AtPos, Radius)) return false;
	if (!PosMoveFrom || (AtPos.x == PosMoveFrom->x && AtPos.y == PosMoveFrom->y)) return true;
	if (!ZL_Math::CircleRotBBSweep(*PosMoveFrom, AtPos, Radius, RotPos, &AtPos)) return false;
	if (AtPos.x != PosMoveFrom->x || AtPos.y != PosMoveFrom->y) AtPos -= (AtPos - *PosMoveFrom).Norm();
	return true;
}

void sObstacle::Draw()
{
	if (Health <= 0) return;
	Picture.Draw(RotPos.P, RotPos.A);
	#if DEBUG
	ZL_Display::PushMatrix();
	ZL_Display::Translate(RotPos.P);
	ZL_Display::Rotate(RotPos.A);
	ZL_Display::Translate(-RotPos.P);
	ZL_Display::DrawRect(ZL_Rectf(RotPos.P, RotPos.E), ZL_Color::Red);
	ZL_Display::PopMatrix();
	#endif
}

void sWorld::StartRound()
{
	RoundEnded = RoundHasBeenWon = false;

	for (vector<sPlayer*>::iterator itPlayer = Players.begin(); itPlayer != Players.end(); ++itPlayer)
	{
		if ((*itPlayer) == &Player) continue; // do not reset human player
		for (vector<cAnimatedFigure*>::iterator itEnemy = (*itPlayer)->Team.begin(); itEnemy != (*itPlayer)->Team.end(); ++itEnemy) delete(*itEnemy);
		delete(*itPlayer);
	}
	Players.clear();
	for (vector<sObstacle*>::iterator itObstacle = Obstacles.begin(); itObstacle != Obstacles.end(); ++itObstacle) delete(*itObstacle);
	Obstacles.clear();
	for (vector<sEffect*>::iterator itEffect = Effects.begin(); itEffect != Effects.end(); ++itEffect) delete(*itEffect);
	Effects.clear();

	Player.RewardCounts.clear();

	//Defaults
	Width = 800;
	Height = 480;
	PlacementArea = ZL_Rectf(0, 0, Width, 200);

	//Reset round timer (use a fixed number from which can be substracted for random NextMovemenent init times
	Time = 10000;

	//Setup level
	InitiateRound();

	for (vector<cAnimatedFigure*>::iterator itDude = Player.Team.begin(); itDude != Player.Team.end(); ++itDude)
	{
		(*itDude)->CurrentAnimation = (*itDude)->PreviousAnimation = NULL;
		(*itDude)->StartAnimation((*itDude)->AnimationStand);
		(*itDude)->is_placed = false;
		(*itDude)->Path.clear();
		(*itDude)->AttackTarget = NULL;
		(*itDude)->SetAction(cAnimatedFigure::STANDING);
		(*itDude)->ForceMoveUntil = 0;
		(*itDude)->NextMovement = World.Time + (*itDude)->MoveReactionTime;
		(*itDude)->NextAttack = 0;
	}

	Mode = MODE_PLACEMENT;
}

void sWorld::Calculate()
{
	if (Mode != MODE_RUNNING || RoundEnded || HintVisible) return;
	Time += ZLELAPSEDTICKS; // Update world time
	// Process dudes and enemies
	int playersAlive = 0;
	for (vector<sPlayer*>::iterator itPlayer = Players.begin(); itPlayer != Players.end(); ++itPlayer)
	{
		(*itPlayer)->Think();
		playersAlive += ((*itPlayer)->teamAlive > 0);
	}
	if (playersAlive <= 1)
	{
		RoundEnded = true;
		RoundHasBeenWon = (Player.teamAlive != 0);
		if (RoundHasBeenWon && Round == RoundHighest)
		{
			RoundHighest = Round + 1; //can be higher than FinalRound which means game cleared
			Player.Budget += RoundBonus;
		}
		else RoundBonus = 0; //no bonus on lose or replaying a previous round
	}
}

void sWorld::Draw()
{
	WaterShader.Activate();
	Background.DrawTo((Width/2)-8192, (Width/2)-8192, (Width/2)+8192, (Width/2)+8192);
	WaterShader.Deactivate();
	Graphic.Draw(0, 0);
	if (Mode == MODE_PLACEMENT) ZL_Display::DrawRect(PlacementArea, ZL_Color::Green, ZLRGBA(0,1,0,.2));
	for (vector<sPlayer*>::iterator itPlayer = Players.begin(); itPlayer != Players.end(); ++itPlayer)
		for (vector<cAnimatedFigure*>::iterator itFigure = (*itPlayer)->Team.begin(); itFigure != (*itPlayer)->Team.end(); ++itFigure)
			(*itFigure)->Draw();
	particleBlood.Draw();
	for (vector<sObstacle*>::iterator itObstacle = Obstacles.begin(); itObstacle != Obstacles.end(); ++itObstacle)
		(*itObstacle)->Draw();
	for (vector<sEffect*>::iterator itEffect = Effects.begin(); itEffect != Effects.end();)
	{
		(*itEffect)->Draw();
		if (!(*itEffect)->Animation.Done) ++itEffect;
		else { delete(*itEffect); itEffect = Effects.erase(itEffect); }
	}
}

bool sWorld::PlaceDude(cAnimatedFigure* dude, ZL_Vector pos)
{
	if (!PlacementArea.Contains(pos)) return false;
	if (Collide(pos, dude->HitBoxRadius+1)) return false;
	dude->Pos.x = pos.x;
	dude->Pos.y = pos.y;
	dude->is_placed = true;
	dude->ViewAngle = PIHALF;
	for (vector<cAnimatedFigure*>::iterator it = Player.Team.begin(); it != Player.Team.end(); ++it)
		if (!(*it)->is_placed) return true;
	//all dudes have been placed
	Mode = MODE_PAUSED;
	return true;
}

void sWorld::SwitchMode(eMode NewMode)
{
	if (Mode == MODE_PAUSED && NewMode == MODE_RUNNING)
	{
		for (vector<cAnimatedFigure*>::iterator itDude = Player.Team.begin(); itDude != Player.Team.end(); ++itDude)
			(*itDude)->PathCalculate();
	}
	Mode = NewMode;
}

bool CheckWaterXY(int x, int y, ZL_Vector& AtPos, const scalar& Radius, const ZL_Vector* PosMoveFrom = NULL)
{
	if (!water_map[(y/32*water_map_width)+x/32]) return false;
	ZL_AABB tilebb(ZL_Vector(s(x/32*32)+s(16), s(y/32*32)+s(16)), s(32), s(32));
	if (PosMoveFrom) return ZL_Math::CircleAABBSweep(*PosMoveFrom, AtPos, Radius, tilebb, &AtPos);
	if (tilebb.Overlaps(AtPos, Radius)) return true;
	return false;
}

bool CheckWaterPoint(const ZL_Vector& CheckPos, ZL_Vector& AtPos, scalar Radius, const ZL_Vector* PosMoveFrom = NULL)
{
	bool coll = false;
	if (CheckWaterXY(CheckPos.x+Radius, CheckPos.y, AtPos, Radius, PosMoveFrom)) coll = true;
	if (CheckWaterXY(CheckPos.x, CheckPos.y+Radius, AtPos, Radius, PosMoveFrom)) coll = true;
	if (CheckWaterXY(CheckPos.x-Radius, CheckPos.y, AtPos, Radius, PosMoveFrom)) coll = true;
	if (CheckWaterXY(CheckPos.x, CheckPos.y-Radius, AtPos, Radius, PosMoveFrom)) coll = true;
	if (CheckWaterXY(CheckPos.x+Radius, CheckPos.y+Radius, AtPos, Radius, PosMoveFrom)) coll = true;
	if (CheckWaterXY(CheckPos.x+Radius, CheckPos.y-Radius, AtPos, Radius, PosMoveFrom)) coll = true;
	if (CheckWaterXY(CheckPos.x-Radius, CheckPos.y+Radius, AtPos, Radius, PosMoveFrom)) coll = true;
	if (CheckWaterXY(CheckPos.x-Radius, CheckPos.y-Radius, AtPos, Radius, PosMoveFrom)) coll = true;
	if (coll && PosMoveFrom && (AtPos.x != PosMoveFrom->x || AtPos.y != PosMoveFrom->y)) AtPos -= (AtPos - *PosMoveFrom).Norm();
	return coll;
}

bool CheckWaterMove(ZL_Vector& AtPos, scalar Radius, const ZL_Vector* PosMoveFrom = NULL, bool long_move = false)
{
	if (long_move && PosMoveFrom)
	{
		ZL_Vector diff = (AtPos - *PosMoveFrom);
		if (diff > s(16))
		{
			scalar length = diff.GetLength(), checklen = s(16);
			for (ZL_Vector step = diff*(s(16)/length), check = *PosMoveFrom + step; checklen < length; check += step, checklen += s(16))
				if (CheckWaterPoint(check, AtPos, Radius, PosMoveFrom)) return true;
		}
	}
	return CheckWaterPoint(AtPos, AtPos, Radius, PosMoveFrom);
}

bool sWorld::Collide(ZL_Vector& AtPos, scalar Radius, const ZL_Vector* PosMoveFrom, bool long_move)
{
	bool coll = false;
	if (AtPos.x - Radius < 0) { AtPos.x = Radius; coll = true; }
	if (AtPos.y - Radius < 0) { AtPos.y = Radius; coll = true; }
	if (AtPos.x + Radius >= Width) { AtPos.x = Width - Radius - s(0.001); coll = true; }
	if (AtPos.y + Radius >= Height) { AtPos.y = Height - Radius - s(0.001); coll = true; }
	if (CheckWaterMove(AtPos, Radius, PosMoveFrom, long_move))
	{
		coll = true;
		while (PosMoveFrom && CheckWaterPoint(AtPos, AtPos, Radius, PosMoveFrom) && (AtPos - *PosMoveFrom) >= 2) { } //move away of any further collisions
	}
	for (vector<sObstacle*>::iterator itObstacle = Obstacles.begin(); itObstacle != Obstacles.end(); ++itObstacle)
		if ((*itObstacle)->Collide(AtPos, Radius, PosMoveFrom, long_move)) coll = true;
	return coll;
}

bool sWorld::SaveGame()
{
	sSaveGame2 sg;
	memset(&sg, 0, sizeof(sg));
	sg.SaveVersion = 2;
	sg.HintsShown = HintsShown;
	sg.Round = Round;
	sg.RoundHighest = RoundHighest;
	sg.Budget = Player.Budget;
	for (vector<cAnimatedFigure*>::iterator itMy = Player.Team.begin(); itMy != Player.Team.end(); ++itMy)
		if (((sTeamMember*)*itMy)->hired == false) sg.Budget += (*itMy)->Price; //temp hire
	for (vector<sTeamMember*>::iterator it = Dudes.begin(); it != Dudes.end(); ++it)
	{
		if ((*it)->Health < (*it)->HealthMax)
		{
			sg.DudeHealth[sg.NumDudeHealth].Dude = (unsigned short)(*it)->Dude;
			sg.DudeHealth[sg.NumDudeHealth].HealthPercentage = (unsigned short)((*it)->Health * s(0xFFFF) / (*it)->HealthMax);
			sg.NumDudeHealth++;
		}
		if ((*it)->Health && (*it)->Weapon.MaxAmmo > 0 && (*it)->Weapon.Ammo < (*it)->Weapon.MaxAmmo)
		{
			sg.DudeAmmo[sg.NumDudeAmmo].Dude = (unsigned short)(*it)->Dude;
			sg.DudeAmmo[sg.NumDudeAmmo].AmmoUsed = (unsigned short)((*it)->Weapon.MaxAmmo - (*it)->Weapon.Ammo);
			sg.NumDudeAmmo++;
		}
		if ((*it)->Health && (*it)->hired)
			sg.DudeHired[sg.NumDudeHired++] = (unsigned short)(*it)->Dude;
	}
	if (sg.Round == 1 && sg.NumDudeHealth == 0 && sg.NumDudeAmmo == 0 && sg.NumDudeHired == 0) return false;
	ZL_Application::SettingsSet("SAVEGAME", ZL_Base64::Encode(&sg, sizeof(sg)));
	ZL_Application::SettingsSynchronize();
	return true;
}

bool sWorld::LoadGame()
{
	sSaveGame2 sg;
	ZL_String Base64sg = ZL_Application::SettingsGet("SAVEGAME");
	if (!Base64sg.length()) return false;
	if (ZL_Base64::Decode(Base64sg, &sg, sizeof(sg)) < sizeof(sSaveGame1)) return false;
	if (sg.SaveVersion < 1 || sg.SaveVersion > 2) return false;
	HintsShown = (sg.SaveVersion == 1 ? 0 : sg.HintsShown);
	if (sg.Round == 1 && sg.NumDudeHealth == 0 && sg.NumDudeAmmo == 0 && sg.NumDudeHired == 0) return false;
	Init();
	Round = sg.Round;
	RoundHighest = sg.RoundHighest;
	Player.Budget = sg.Budget;
	for (vector<sTeamMember*>::iterator it = Dudes.begin(); it != Dudes.end(); ++it)
	{
		size_t i;
		for (i = 0; i < sg.NumDudeHealth; i++)
			if (sg.DudeHealth[i].Dude == (unsigned short)(*it)->Dude)
				(*it)->Health = (*it)->HealthMax * s(sg.DudeHealth[i].HealthPercentage) / s(0xFFFF);
		if ((*it)->Health <= 0) continue;
		for (i = 0; i < sg.NumDudeAmmo; i++)
			if (sg.DudeAmmo[i].Dude == (unsigned short)(*it)->Dude && (*it)->Weapon.MaxAmmo)
				(*it)->Weapon.Ammo = MAX(0, ((*it)->Weapon.MaxAmmo - (int)(sg.DudeAmmo[i].AmmoUsed)));
		for (i = 0; i < sg.NumDudeHired; i++)
			if (sg.DudeHired[i] == (unsigned short)(*it)->Dude && find(Player.Team.begin(), Player.Team.end(), *it) == Player.Team.end())
			{
				(*it)->hired = true;
				Player.Team.push_back(*it);
				break;
			}
	}
	return true;
}

void DrawRandomMud(int w32, int h32)
{
	#ifdef SCENE_EDITOR
	if (ZL_SceneManager::GetCurrent()->SceneType == SCENE_EDITOR) return;
	#endif

	scalar *frac = new scalar[w32*h32];
	memset(frac, 0, sizeof(frac[0])*w32*h32);
	for (int i = (w32*h32)/5; i; i--)
	{
		int x = rand()%w32, y = rand()%h32, size = (rand()%(10));
		for (int xoff = -size; xoff <= size; xoff++)
		{
			int xoffsize = (size-abs(xoff));
			for (int xx = x+xoff, yoff = -xoffsize; yoff <= xoffsize; yoff++)
			{
				int yoffsize = (size-abs(yoff)), yy = y+yoff;
				if (xx < 0 || xx >= w32 || yy < 0 || yy >= h32) continue;
				frac[yy*w32+xx] += 1+xoffsize+yoffsize;
			}
		}
	}

	int w16 = w32*2, h16 = h32*2, mx, my;
	bool *mud_map = new bool[w16*h16];

	//init mud_map from random fractal map by threshold
	for (my = 0; my < h16; my++) for (mx = 0; mx < w16; mx++)
		mud_map[my*w16+mx] = (frac[(my/2)*(w16/2)+(mx/2)] > 140);

	//smooth mud_map edges by adding where 1 space to two sides are set and 2 spaces to opposite sides are free
	for (my = 0; my < h16-1; my+=2) for (bool* p = &mud_map[my*w16], *pEnd = &mud_map[my*w16+w16-1]; p < pEnd; p+=2)
	{
		if (*p) continue;
		bool hasleft = (p > &mud_map[my*w16]), hasright = (p < pEnd-2);
		if (hasleft)
		{
			bool up = (my < h16-2 && p[  w16+w16]), down = (my && p[  -w16]);
			if      (p[-1    ] && down && !up   && (!hasright || !p[ 2    ])) p[    0] = true;
			else if (p[-1+w16] &&   up && !down && (!hasright || !p[ 2+w16])) p[w16  ] = true;
		}
		if (hasright)
		{
			bool up = (my < h16-2 && p[1+w16+w16]), down = (my && p[1-w16]);
			if      (p[ 2    ] && down && !up   && (!hasleft || !p[-1    ])) p[    1] = true;
			else if (p[ 2+w16] &&   up && !down && (!hasleft || !p[-1+w16])) p[w16+1] = true;
		}
	}

	//smooth mud_map edges by removing when 3 cross connected are free
	for (my = 0; my < h16-1; my+=2) for (bool* p = &mud_map[my*w16], *pEnd = &mud_map[my*w16+w16-1]; p < pEnd; p+=2)
	{
		if (!p[0] && !p[1] && !p[w16] && !p[w16+1]) continue;
		bool hasleft = (p > &mud_map[my*w16]), hasright = (p < pEnd-2), hasup = (my < h16-2), hasdown = (my > 0);
		if (hasleft  && hasdown && !p[-1+w16] && !p[-1-w16    ] && !p[1    -w16]) p[    0] = false;
		if (hasleft  && hasup   && !p[-1    ] && !p[-1+w16+w16] && !p[1+w16+w16]) p[w16  ] = false;
		if (hasright && hasdown && !p[ 2+w16] && !p[ 2-w16    ] && !p[     -w16]) p[    1] = false;
		if (hasright && hasup   && !p[ 2    ] && !p[ 2+w16+w16] && !p[  w16+w16]) p[w16+1] = false;
	}

	ZL_Surface srfMudTile("Data/mudt.png");
	srfMudTile.SetTilesetClipping(4, 4);
	for (my = 0; my < h16; my++) for (mx = 0; mx < w16; mx++)
	{
		//if (frac[(my/2)*w32+(mx/2)] > 140) { ZL_Display::DrawRect(mx/2*32, my/2*32, mx/2*32+32, my/2*32+32, ZL_Color::Red); }
		bool* p = &mud_map[my*w16+mx];
		if (*p) { srfMudTile.SetTilesetIndex((mx&1)+((my&1)<<1)).Draw(mx*16, my*16); continue; }
		bool l  = (mx > 0 && p[-1]), r  = (mx < w16-1 && p[1]), lo = (my > 0 && p[-w16]), hi = (my < h16-1 && p[w16]);
		assert(!(r && l) && !(lo && hi)); //if two opposite sides both are set, something bad was generated
		int t = (r&&lo ? 8 : (l&&hi ? 9 :(l&&lo ? 10 : (r&&hi ? 11 : (r ? 4 : (l ? 5 : (lo ? 6 : (hi ? 7 : 0))))))));
		if (t) { srfMudTile.SetTilesetIndex(t).Draw(mx*16, my*16); continue; }
		if      (mx >     0 && my < h16-1 && p[-1+w16]) srfMudTile.SetTilesetIndex(12).Draw(mx*16, my*16);
		else if (mx < w16-1 && my < h16-1 && p[+1+w16]) srfMudTile.SetTilesetIndex(14).Draw(mx*16, my*16);
		if      (mx < w16-1 && my >     0 && p[+1-w16]) srfMudTile.SetTilesetIndex(13).Draw(mx*16, my*16);
		else if (mx >     0 && my >     0 && p[-1-w16]) srfMudTile.SetTilesetIndex(15).Draw(mx*16, my*16);
	}

	delete mud_map;
	delete frac;
}

void DrawTileMap(unsigned char *map32, int w32, int h32, ZL_Surface& srf, bool fill_outside = true)
{
	srf.SetTilesetClipping(4, 4);

	//int w16 = w32*2, h16 = h32*2, mx, my;
	int w16 = 2+w32*2, h16 = 2+h32*2, mx, my;
	bool *map16 = new bool[w16*h16];
	bool *p, *pEnd, l, r, lo, hi, up, down;

	//init map16 from water_map32
	//for (my = 0; my < h16; my++) for (mx = 0; mx < w16; mx++)
	//	map16[my*w16+mx] = (map32[(my/2)*(w16/2)+(mx/2)]>0);
	for (my = 1; my < h16-1; my++) for (mx = 1; mx < w16-1; mx++)
		map16[my*w16+mx] = (map32[((my-1)/2)*(w32)+((mx-1)/2)]>0);

	//fill outside if set
	for (my = 0; my < h16  ; my++) map16[my*w16] = map16[my*w16+w16-1] = fill_outside;
	for (mx = 1; mx < w16-1; mx++) map16[mx] = map16[h16*w16-w16+mx] = fill_outside;

	//smooth map16 edges by adding where 1 space to two sides are set and 2 spaces to opposite sides are free
	for (my = 1; my < h16-2; my+=2) for (p = &map16[my*w16+1], pEnd = &map16[my*w16+w16-2]; p < pEnd; p+=2)
	{
		if (*p) continue;
		up = (p[  w16+w16]), down = (p[  -w16]);
		if      (p[-1    ] && down && !up   && !p[ 2    ]) p[    0] = true;
		else if (p[-1+w16] &&   up && !down && !p[ 2+w16]) p[w16  ] = true;
		up = (p[1+w16+w16]), down = (p[1-w16]);
		if      (p[ 2    ] && down && !up   && !p[-1    ]) p[    1] = true;
		else if (p[ 2+w16] &&   up && !down && !p[-1+w16]) p[w16+1] = true;
	}

	//smooth map16 edges by removing when 3 cross connected are free
	for (my = 1; my < h16-2; my+=2) for (p = &map16[my*w16+1], pEnd = &map16[my*w16+w16-2]; p < pEnd; p+=2)
	{
		if (!p[0] && !p[1] && !p[w16] && !p[w16+1]) continue;
		if (!p[-1+w16] && !p[-1-w16    ] && !p[1    -w16]) p[    0] = false;
		if (!p[-1    ] && !p[-1+w16+w16] && !p[1+w16+w16]) p[w16  ] = false;
		if (!p[ 2+w16] && !p[ 2-w16    ] && !p[     -w16]) p[    1] = false;
		if (!p[ 2    ] && !p[ 2+w16+w16] && !p[  w16+w16]) p[w16+1] = false;
	}

	for (my = 1; my < h16-1; my++) for (mx = 1; mx < w16-1; mx++)
	{
		//if (map32[(my/2)*w32+(mx/2)] > 0) { ZL_Display::DrawRect(mx/2*32, my/2*32, mx/2*32+32, my/2*32+32, ZL_Color::Red); }
		p = &map16[my*w16+mx];
		if (*p) { srf.SetTilesetIndex((mx&1)+((my&1)<<1)).Draw(mx*16-16, my*16-16); continue; }
		//if (fill_outside) l  = (mx == 0 || p[-1]), r  = (mx == w16-1 || p[1]), lo = (my == 0 || p[-w16]), hi = (my == h16-1 && p[w16]);
		//else              l  = (mx >  0 && p[-1]), r  = (mx <  w16-1 && p[1]), lo = (my >  0 && p[-w16]), hi = (my <  h16-1 && p[w16]);
		l  = p[-1], r  = p[1], lo = p[-w16], hi = p[w16];
		assert(!(r && l) && !(lo && hi)); //if two opposite sides both are set, something bad was generated
		int t = (r&&lo ? 8 : (l&&hi ? 9 :(l&&lo ? 10 : (r&&hi ? 11 : (r ? 4 : (l ? 5 : (lo ? 6 : (hi ? 7 : 0))))))));
		if (t) { srf.SetTilesetIndex(t).Draw(mx*16-16, my*16-16); continue; }
		if      (mx >     0 && my < h16-1 && p[-1+w16]) srf.SetTilesetIndex(12).Draw(mx*16-16, my*16-16);
		else if (mx < w16-1 && my < h16-1 && p[+1+w16]) srf.SetTilesetIndex(14).Draw(mx*16-16, my*16-16);
		if      (mx < w16-1 && my >     0 && p[+1-w16]) srf.SetTilesetIndex(13).Draw(mx*16-16, my*16-16);
		else if (mx >     0 && my >     0 && p[-1-w16]) srf.SetTilesetIndex(15).Draw(mx*16-16, my*16-16);
	}

	#ifdef SCENE_EDITOR
	if (ZL_SceneManager::GetCurrent()->SceneType == SCENE_EDITOR)
		for (my = 1; my < h16-1; my++) for (mx = 1; mx < w16-1; mx++)
			if (map32[(my/2)*w32+(mx/2)] > 0) { ZL_Display::DrawRect(mx/2*32, my/2*32, mx/2*32+32, my/2*32+32, ZL_Color::Blue, ZLRGBA(0,0,255,.1)); }
	#endif

	delete map16;
}

/*
void DrawWaterMap(unsigned char *water_map, int w32, int h32, bool outside_water = true)
{
	#ifdef SCENE_EDITOR
	if (ZL_SceneManager::GetCurrent()->SceneType == SCENE_EDITOR)
	{
		//draw water editor overlay
		for (int wy = 0; wy < h32; wy++) for (int wx = 0; wx < w32; wx++)
			if (water_map[wy*w32+wx]) srfWater.SetTilesetIndex(0).Draw(32*wx, 32*wy, ZLALPHA(0.2));
	}
	#endif
	unsigned char o = (outside_water ? 1 : 0);
	for (int wy = -1; wy <= h32; wy++) for (int wx = -1; wx <= w32; wx++)
	{
		unsigned char* w = water_map+(wy*w32)+wx;
		unsigned int ww = (wy>=h32-1 ? (o?224:0) : ((wx<=0 ? o : *(w+w32-1))<<5) + ((wx==-1||wx==w32 ? o : *(w+w32))<<6) + ((wx>=w32-1 ? o : *(w+w32+1))<<7)) + // 5 6 7  32 64 128
		            (wy==h32||wy==-1 ? (o? 24:0) : ((wx<=0 ? o : *(w    -1))<<3) +                                         ((wx>=w32-1 ? o : *(w    +1))<<4)) + // 3 w 4   8     16
		                    (wy<=0   ? (o?  7:0) : ((wx<=0 ? o : *(w-w32-1))<<0) + ((wx==-1||wx==w32 ? o : *(w-w32))<<1) + ((wx>=w32-1 ? o : *(w-w32+1))<<2));  // 0 1 2   1  2   4
		if (o)
		{
			if (wx==-1 || wx==w32   || wy== -1 || wy==h32  ) continue;
			if (wx== 0 || wx>=w32-2) { w = &o; ww |= 1<<1|1<<6; }
			if (wy== 0 || wy>=h32-2) { w = &o; ww |= 1<<3|1<<4; }
			if (wx== 1 || wx>=w32-3) ww |= (wx==1 ? 1<<0|1<<3|1<<5 : 1<<2|1<<4|1<<7);
			if (wy== 1 || wy>=h32-3) ww |= (wy==1 ? 1<<0|1<<1|1<<2 : 1<<5|1<<6|1<<7);
		}
		if (wx>=0 && wx < w32 && wy >= 0 && wy < h32 && *w)
		{
			if (ww == 255)                  srfWater.SetTilesetIndex( 0).Draw(32*(wx)  , 32*(wy)  );
			if (ww ==   0)                  srfWater.SetTilesetIndex( 0).Draw(32*(wx)  , 32*(wy)  );
			continue;
		}
		if (ww& 8 && ww& 32 && ww& 64)      srfWater.SetTilesetIndex(14).Draw(32*(wx-1), 32*(wy+1));
		if (ww&64 && ww&128 && ww& 16)      srfWater.SetTilesetIndex(15).Draw(32*(wx+1), 32*(wy+1));
		if (ww&16 && ww&  4 && ww&  2)      srfWater.SetTilesetIndex(12).Draw(32*(wx+1), 32*(wy-1));
		if (ww& 2 && ww&  1 && ww&  8)      srfWater.SetTilesetIndex(13).Draw(32*(wx-1), 32*(wy-1));
		if (ww&32 && ww& 64 && ww&128)      srfWater.SetTilesetIndex( 7).Draw(32*(wx  ), 32*(wy+1));
		if (ww& 1 && ww&  2 && ww&  4)      srfWater.SetTilesetIndex( 4).Draw(32*(wx  ), 32*(wy-1));
		if (ww& 1 && ww&  8 && ww& 32)      srfWater.SetTilesetIndex( 6).Draw(32*(wx-1), 32*(wy  ));
		if (ww& 4 && ww& 16 && ww&128)      srfWater.SetTilesetIndex( 5).Draw(32*(wx+1), 32*(wy  ));
		if (!(ww&8 ) && ww&32  && !(ww&64)) srfWater.SetTilesetIndex(10).Draw(32*(wx-1), 32*(wy+1));
		if (!(ww&64) && ww&128 && !(ww&16)) srfWater.SetTilesetIndex(11).Draw(32*(wx+1), 32*(wy+1));
		if (!(ww&16) && ww&4   && !(ww&2 )) srfWater.SetTilesetIndex( 8).Draw(32*(wx+1), 32*(wy-1));
		if (!(ww&2 ) && ww&1   && !(ww&8 )) srfWater.SetTilesetIndex( 9).Draw(32*(wx-1), 32*(wy-1));
	}
}
*/
void DrawRandomPlants()
{
	ZL_Surface plant("Data/plant.png");
	for (int i = ((World.Width*World.Height)/2000); i; i--)
		plant.Draw(RAND_RANGE(16, World.Width-16), RAND_RANGE(16, World.Height-16), RAND_FACTOR*PI2);
}
void sWorld::RedrawMapGraphic()
{
	ZL_Surface ground;
	bool AddRandomMud = false, AddRandomPlants = false;
	switch (Round)
	{
	case 8:
		AddRandomMud = true;
	case 4:
		ground = ZL_Surface("Data/sand.png").SetTextureRepeatMode();
		break;
	case 3:
	case 7:
		ground = ZL_Surface("Data/mud.png").SetTextureRepeatMode();
		AddRandomPlants = true;
		break;
	default:
		ground = ZL_Surface("Data/grass.png").SetTextureRepeatMode();
		AddRandomMud = true;
		AddRandomPlants = true;
	}

	ZL_Surface srfTileWater = ZL_Surface("Data/watert.png").SetTilesetClipping(4, 4);

	Background.RenderToBegin(0);
	srfTileWater.SetTilesetIndex(0).Draw(0, 16);
	srfTileWater.SetTilesetIndex(1).Draw(16, 16);
	srfTileWater.SetTilesetIndex(2).Draw(0, 0);
	srfTileWater.SetTilesetIndex(3).Draw(16, 0);
	Background.RenderToEnd();

	Graphic.RenderToBegin();
	ground.DrawTo(0, 0, Width, Height);
	if (AddRandomPlants) DrawRandomPlants();
	if (AddRandomMud) DrawRandomMud(water_map_width, water_map_height);
	ZL_Surface("Data/overlay.png").SetClipping(ZL_Rect(0,0,0+Width/16,0+Height/16)).DrawTo(0, 0, Width, Height);
	//DrawWaterMap(water_map, water_map_width, water_map_height, true);
	ZL_Surface watert("Data/watert.png");
	DrawTileMap(water_map, water_map_width, water_map_height, watert, true);
	Graphic.RenderToEnd();
}

void sWorld::GenerateMap(const unsigned short *round_water_data, int round_water_size)
{
	Graphic = ZL_Surface(Width, Height);

	if (water_map) delete water_map;
	water_map_width = (Width/32);
	water_map_height = (Height/32);
	water_map = new unsigned char[water_map_width*water_map_height];
	memset(water_map, 0, water_map_width*water_map_height);
	if (round_water_data)
		for (int iw = 0, iwend = MIN(round_water_size<<3,(int)(water_map_width*water_map_height)) ; iw < iwend; iw++)
			water_map[iw] = ((round_water_data[iw>>4] & (1<<(iw&15))) ? 1 : 0);

	RedrawMapGraphic();
}

#ifdef SCENE_EDITOR
void sWorld::EditorResize(int NewWidth, int NewHeight)
{
	if (NewWidth < 64) NewWidth = 64;
	if (NewHeight < 64) NewHeight = 64;
	if (NewWidth > 2048) NewWidth = 2048;
	if (NewHeight > 2048) NewHeight = 2048;
	if ((Width/32) != (int)water_map_width || (Height/32) != (int)water_map_height)
	{
		unsigned int new_water_map_width = (Width/32), new_water_map_height = (Height/32);
		unsigned char *new_water_map = new unsigned char[new_water_map_width*new_water_map_height];
		memset(new_water_map, 0, new_water_map_width*new_water_map_height);
		for (int wy = MIN(water_map_height, new_water_map_height)-1; wy >= 0; wy--)
			memcpy(&new_water_map[wy*new_water_map_width], &water_map[wy*water_map_width], MIN(water_map_width, new_water_map_width));
		delete water_map;
		water_map = new_water_map;
		water_map_width = new_water_map_width;
		water_map_height = new_water_map_height;

		Graphic = ZL_Surface(water_map_width*32, water_map_height*32);
		RedrawMapGraphic();
	}
	Width = NewWidth;
	Height = NewHeight;
}

int sWorld::EditorSetWater(int x, int y, int value)
{
	if (x < 0 || y < 0) return 1;
	unsigned int wx = x/32, wy = y/32;
	if (wx >= water_map_width || wy >= water_map_height) return 1;
	water_map[wy*water_map_width+wx] = (value == 0 ? 0 : (value == 1 ? 1 : !water_map[wy*water_map_width+wx]));
	RedrawMapGraphic();
	return water_map[wy*water_map_width+wx];
}

void sWorld::EditorOutput()
{
	ZL_String strWater;
	unsigned int i, sum = 0;
	for (i = 0; i < (unsigned int)(water_map_width*water_map_height); i++)
	{
		if (i && !(i&15)) { strWater << ZL_String::format((sum < 1000 ? "%d," : "0x%x,"), sum); sum = 0; }
		sum += ((unsigned int)water_map[i])<<(i&15);
	}
	if (sum) strWater << ZL_String::format((sum < 1000 ? "%d" : "0x%x"), sum);
	while (strWater.size() && *(strWater.end()-1) == ',') strWater.erase(strWater.length()-1); //remove trailing ,
	while (strWater.size() > 1 && *(strWater.end()-2) == ',' && *(strWater.end()-1) == '0') strWater.erase(strWater.length()-2); //remove trailing ,0

	#ifndef __NACL__
	printf("\t\tcase %d:{\n", Round);

	printf("\t\t\tWidth = %d;\n", water_map_width*32);
	printf("\t\t\tHeight = %d;\n", water_map_height*32);
	printf("\t\t\tPlacementArea = ZL_Rectf(s(%d), s(%d), s(%d), s(%d));\n", (int)PlacementArea.left, (int)PlacementArea.low, (int)PlacementArea.right, (int)PlacementArea.high);

	extern const char* EditorGetAnimalClass(cAnimal::eType);
	for (vector<cAnimatedFigure*>::iterator itEnemy = Players[1]->Team.begin(); itEnemy != Players[1]->Team.end(); ++itEnemy)
		printf("\t\t\tEnemies.push_back(new %s(ZL_Vector(s(%d), s(%d)), s(%f)));\n", EditorGetAnimalClass(((cAnimal*)*itEnemy)->Type), (int)(*itEnemy)->Pos.x, (int)(*itEnemy)->Pos.y, (*itEnemy)->ViewAngle);

	extern const char* EditorFenceType(sObstacle::eType);
	for (vector<sObstacle*>::iterator itObstacle = Obstacles.begin(); itObstacle != Obstacles.end(); ++itObstacle)
		printf("\t\t\tObstacles.push_back(new sObstacle(sObstacle::%s, ZL_Vector(s(%d), s(%d)), s(%f)));\n", EditorFenceType((*itObstacle)->Type),
			(int)(*itObstacle)->RotPos.P.x, (int)(*itObstacle)->RotPos.P.y, (*itObstacle)->RotPos.A);

	printf("\t\t\tRoundBonus = %d;\n", RoundBonus);

	if (strWater.size() > 1 || (strWater.size() == 1 && *strWater.begin() != '0'))
		printf("\t\t\tstatic const unsigned short water_round_%d[] = { %s };\n"
			"\t\t\tGenerateMap(water_round_%d, sizeof(water_round_%d));\n", Round, strWater.c_str(), Round, Round);
	else
		printf("\t\t\tGenerateMap();\n");

	printf("\t\t\t}break;\n\n");
	#endif
}

#endif

void sWorld::ShowHintOnce(eHintsShown which, int FadeInDelay)
{
	if (which == HINT_NONE || HintVisible)
	{
		ZL_Timer::EndTransitionFor(&hint_fade);
		ZL_Timer::AddTransitionScalar(&hint_fade, 0, hint_fade*500);
		return;
	}
	if (HintsShown & (unsigned char)which) return;
	HintVisible = true;
	HintsShown |= (unsigned char)which;
	switch (which)
	{
		case HINT_TEAM:      hint_surface = ZL_Surface("Data/hint_team.zltex");      break;
		case HINT_PLACEMENT: hint_surface = ZL_Surface("Data/hint_placement.zltex"); break;
		case HINT_PLAYING:   hint_surface = ZL_Surface("Data/hint_playing.zltex");
		                     #ifdef __SMARTPHONE__
			                 hint_surface2 = ZL_Surface("Data/hint_playing_smp.png");
		                     #else
			                 hint_surface2 = ZL_Surface("Data/hint_playing_pc.png");
		                     #endif
							 break;
		default:             break;
	}
	ZL_Timer::AddTransitionScalar(&(hint_fade = 0), 1, 1300, FadeInDelay);
}

void sWorld::DrawHint()
{
	if (hint_fade == 0 && (ZL_Timer::GetActiveTransitionFor(&hint_fade) == NULL))
	{
		hint_surface = hint_surface2 = ZL_Surface();
		HintVisible = false;
		return;
	}
	ZL_Display::DrawRect(0, 0, ZLWIDTH, ZLHEIGHT, ZLTRANSPARENT, ZLLUMA(0, hint_fade*s(0.6)));
	hint_surface.DrawTo(0, 0, ZLWIDTH, ZLHEIGHT, ZLALPHA(hint_fade));
	hint_surface2.DrawTo(0, 0, ZLWIDTH, ZLHEIGHT, ZLALPHA(hint_fade));
}
