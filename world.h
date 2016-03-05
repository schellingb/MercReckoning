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

#ifndef _MERCRECKONING_WORLD_
#define _MERCRECKONING_WORLD_

#include "include.h"

//these correspond to tile positions in ui_icons.png - so don't change
enum eWeapon
{
	WEAPON_NONE = 0,
	WEAPON_KNIFE = 1,
	WEAPON_KNUCKLES = 2,
	WEAPON_KICK = 3,
	WEAPON_NAILCLUB = 4,
	WEAPON_NINJASTAR = 5,
	WEAPON_THROWKNIFE = 6,
	WEAPON_CHAINSAW = 7,
	WEAPON_MACHINEGUN = 8,
	WEAPON_BOMB = 9
};

enum eSpecialWeapon
{
	SWEAPON_NONE = 0,
	SWEAPON_GRENADE = 1,
	SWEAPON_RAM = 2
};

//these are stored in savegames so don't change
enum eDude
{
	DUDE_MIKE = 1,
	DUDE_JACK = 2,
	DUDE_LUKE = 3,
	DUDE_BETSY = 4,
	DUDE_FATSTAR = 5,
	DUDE_IVAN = 6,
	DUDE_LEON = 7,
	DUDE_SHIFTER = 8
};

struct sSaveGame1
{
	unsigned char SaveVersion;
	unsigned short Round, RoundHighest;
	int Budget;
	unsigned short NumDudeHealth, NumDudeAmmo, NumDudeHired;
	struct { unsigned short Dude, HealthPercentage; } DudeHealth[16];
	struct { unsigned short Dude, AmmoUsed; } DudeAmmo[16];
	unsigned short DudeHired[16];
};
struct sSaveGame2 : sSaveGame1
{
	unsigned char HintsShown;
};

class cAnimation
{
	static const int ShortBlinkDuration;
	void NextPicture();

public:
	enum eAnimationType { PLAYONCE, LOOP, SHORTBLINK } Type;

	cAnimation() : Type(LOOP), PicIndex(0), Step(1000), Layers(NULL) { }

	unsigned int PicIndex, Step;
	vector<int> Tiles;
	vector<ZL_Surface> *Layers;
	unsigned int NextPicTick;
	bool Done;

	void Setup(vector<ZL_Surface> &Layers, eAnimationType Type = cAnimation::LOOP, int Step = 1000) { Tiles.clear(); this->Layers = &Layers; this->Type = Type; this->Step = Step; }
	void Calculate();
	void Start();
	void Draw(ZL_Vector pos, float viewAngle);
};

class cAnimatedFigure;
struct sWeapon
{
	vector<ZL_Surface> Layers;
	cAnimation ActivateAnimation;
	ZL_Surface OwnerLayer;
	ZL_Sound Sound;
	ZL_ParticleEffect Particles;
	ZL_ParticleBehavior_LinearMove *ParticleMove;

	eWeapon Type;
	ZL_String Name;
	scalar Power, Radius, ArmorPiercing, Range;
	scalar AttackModifier, DefenseModifier, SpeedModifier, AttackSpeedModifier;
	short MaxAmmo, Ammo;
	unsigned int ReactionTime;
	cAnimatedFigure *Owner;
	bool FriendlyFire;
	int Price;
	scalar AmmoPrice;

	sWeapon(eWeapon type);

	bool TryAttack(cAnimatedFigure *pSpecificTarget = NULL);
	bool Attack(scalar radius, float attackPower, bool splashDMG);
	bool AttackTarget(cAnimatedFigure *target, float attackPower);
	int GetPrice();
	void LoadAnimations();
};

struct sSpecialWeapon : sWeapon
{
	void SpecialAttack(ZL_Vector &target);

	sSpecialWeapon(eSpecialWeapon type);
};

struct sPlayer;
class cAnimatedFigure
{
public:
	enum eFigureType { ANIMAL, TEAMMEMBER } FigureType;
	ZL_Surface Picture;
	ZL_Vector Pos;
	scalar ViewAngle;
	scalar HealthMax, Health, HitBoxRadius;
	scalar Strength, Defense, Speed;
	bool FriendlyFire;
	int Price;
	sPlayer *Owner;
	sWeapon Weapon, WeaponUnarmed;
	sSpecialWeapon SpecialWeapon;
	unsigned int MoveReactionTime, NextStepSound, NextMovement, NextAttack;
	bool is_placed;
	unsigned int DizzyUntil, ForceMoveUntil;
	scalar DizzySpeed;
	enum eAction { MOVE = 0, ATTACK, DIZZY, DEAD, STANDING } Action;
	cAnimatedFigure *AttackTarget, *FleeTarget; // Used by AI
	ZL_Vector WanderTarget; // Used by AI
	vector<ZL_Vector> Path;
	scalar PathTotal;
	bool specialAttackLaunched;

	ZL_String Name, Description;
	vector<ZL_Surface> Layers;
	cAnimation AnimationStand, AnimationMove, AnimationAttack, AnimationDeath;
	cAnimation *CurrentAnimation, *PreviousAnimation;
	ZL_Sound Sound;
	int GetMoveAnimStep();
	void SetAction(eAction action);
	void StartAnimation(cAnimation& Animation);
	void ReceiveAttack(cAnimatedFigure *attacker, float attackStrength, float armorPiercing);
	void Calculate();
	virtual void Draw();
	void DrawHealthBar();
	virtual void ThinkMove(bool force_wander = false) = 0;
	virtual void ThinkAttack() = 0;
	bool AddPath(ZL_Vector& point, bool clear_current = false);
	cAnimatedFigure* FindTargetAlongPath(bool OnlyPathBack = false);
	bool CanAttack(sPlayer *player);
	bool IsInRange(cAnimatedFigure *target);
	virtual void Move();
	virtual void Attack();
	virtual void LaunchSpecialAttack(ZL_Vector &target);
	virtual void LoadAnimations() = 0;
	virtual void Die(cAnimatedFigure *killer);
	void LoseControl(const ZL_Vector &direction, scalar unconSpeed, unsigned int unconTime);
	void RegainControl();
	void Bleed(float attack);
	void PathCalculate();

protected:
	cAnimatedFigure(eFigureType FigureType, ZL_Surface Picture, ZL_String Name, ZL_String Description, int Price, scalar HealthMax, scalar HitBoxRadius, scalar Strength, scalar Defense, scalar Speed, bool FriendlyFire, eWeapon WeaponType, unsigned int MoveReactionTime);
	void LoadSound(const char* file);

public:
	virtual ~cAnimatedFigure() { }
};

class cAnimal : public cAnimatedFigure
{
public:
	enum eType { SHEEP, MOTHERSHEEP, CHIPMUNK, OSTRICH, DOG, CHIMP, WOLF, GORILLA, BEAR /*, ALLIGATOR, RHINO, ELEPHANT, TRICERATOPS*/ } Type;
	scalar ArmorPiercing;
	scalar SightRadius; //view distance
	scalar SightAngle; //field of view angle in rad
	scalar Aggressiveness; //0-1, chance of attacking target instead of fleeing from
	unsigned int AttackReactionTime, WanderUntil, FleeUntil;
	bool Moving;

	void Draw();
	virtual void ThinkMove(bool force_wander = false);
	virtual void ThinkAttack();
	virtual void ReactTo(cAnimatedFigure *pDude);
	virtual void LoadAnimations() = 0;

	virtual ~cAnimal() { }
protected:
	cAnimal(const ZL_Vector& InPos, scalar InViewAngle, eType Type, ZL_String Name, ZL_String Description, scalar HealthMax, scalar HitBoxRadius, scalar Strength, scalar Defense, scalar Speed, scalar ArmorPiercing, scalar SightRadius, scalar SightAngle, scalar Aggressiveness, int Reward, unsigned int MoveReactionTime, unsigned int AttackReactionTime)
		: cAnimatedFigure(ANIMAL, ZL_Surface("Data/animal.png"), Name, Description, Reward, HealthMax, HitBoxRadius, Strength, Defense, Speed, false, WEAPON_NONE, MoveReactionTime),
		  Type(Type), ArmorPiercing(ArmorPiercing), SightRadius(SightRadius), SightAngle(SightAngle), Aggressiveness(Aggressiveness),
		  AttackReactionTime(AttackReactionTime), WanderUntil(0), Moving(false)
	{ Pos = InPos; ViewAngle = InViewAngle; AttackTarget = NULL; }
};

struct sTeamMember : public cAnimatedFigure
{
	eDude Dude;
	bool hired;

	void LoadAnimations();
	virtual void Draw();
	void ThinkMove(bool force_wander = false);
	void ThinkAttack();

	sTeamMember(eDude Dude, ZL_Surface Picture, ZL_String Name, ZL_String Description, int Price, scalar Health, scalar Strength, scalar Defense, scalar Speed, eWeapon WeaponType, scalar radius, unsigned int MoveReactionTime);
};

struct sEffect
{
	cAnimation Animation;
	ZL_Vector Pos;

	sEffect(cAnimation animation, ZL_Vector pos)
		: Animation(animation), Pos(pos) {}

	void Draw();
};

struct sObstacle
{
	enum eType { FENCE, ROCK, PALM } Type;
	ZL_RotBB RotPos;
	ZL_Surface Picture;
	int Health;
	sObstacle(eType Type, const ZL_Vector& Pos, scalar Rotation = 0);
	void Draw();
	bool Collide(ZL_Vector& AtPos, scalar Radius, const ZL_Vector* PosMoveFrom = NULL, bool long_move = false);
};

struct sPlayer
{
public:
	bool isHuman;
	int id;
	int allianceMap;
	int Budget;
	struct sRewardCount { int value, count; };
	map<ZL_String, sRewardCount> RewardCounts;
	vector<cAnimatedFigure*> Team;
	int teamAlive;
	void Think();
	void ThinkMove(cAnimatedFigure *figure);
	void ThinkAttack(cAnimatedFigure *figure);
	void AddToTeam(cAnimatedFigure *figure);

	sPlayer(int id = 0, int budget = 0) : isHuman(false), id(1 << id), allianceMap(1 << id), Budget(budget) {}
};

struct sWorld
{
	enum eMode { MODE_PLACEMENT, MODE_PAUSED, MODE_RUNNING } Mode;
	unsigned short Round, RoundHighest, RoundBonus;
	unsigned int Time;
	bool RoundEnded, RoundHasBeenWon;
	int Width, Height;
	ZL_Rectf PlacementArea;

	static const unsigned short FinalRound;
	static const int PricePerHealHP;

	void InitGlobal();
	void Init();
	void StartRound();
	const char* GetRoundName();
	void Calculate();
	void Draw();
	bool PlaceDude(cAnimatedFigure* dude, ZL_Vector pos);
	void SwitchMode(eMode NewMode);
	sPlayer Player;
	vector<sPlayer*> Players;
	vector<sTeamMember*> Dudes;
	vector<sEffect*> Effects;
	vector<sObstacle*> Obstacles;
	bool Collide(ZL_Vector& AtPos, scalar Radius, const ZL_Vector* PosMoveFrom = NULL, bool long_move = false);
	map<ZL_String, ZL_Sound*> Sounds;
	ZL_Sound* LoadSound(const char* file);

	bool SaveGame();
	bool LoadGame();

	enum eHintsShown { HINT_NONE = 0, HINT_TEAM = (1<<0), HINT_PLACEMENT = (1<<1), HINT_PLAYING = (1<<2) };
	unsigned char HintsShown;
	bool HintVisible;
	void ShowHintOnce(eHintsShown which, int FadeInDelay = 0);
	void DrawHint();

	//editor stuff
	int EditorSetWater(int x, int y, int val = -1);
	void EditorResize(int NewWidth, int NewHeight);
	void EditorOutput();
private:
	void InitiateRound();
	void RedrawMapGraphic();
	void GenerateMap(const unsigned short *round_water_data = NULL, int round_water_size = 0);
};

extern sWorld World;
extern cAnimatedFigure *ActivePlayerFigure;

#endif //_MERCRECKONING_WORLD_
