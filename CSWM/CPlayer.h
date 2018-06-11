#pragma once
#ifndef _CPLAYER_
#define _CPLAYER_
#endif

class CPlayerInfo
{
public:
	bool IsValid;
	bool IsAlive;
	bool IsBanned;
	int Index;
	char Address[32];
	edict_t *Edict;

	float BuildDelay;
	edict_t *pOwnEntity;
	float EntDistance;
	float EntOffset[3];
	int LockedEntity[10];
	int LockedEntityAmount;
	int CurObjects;
	int MaxObjects;
	int FlagInfo;
	int Stuck;

	int Money;
	int ZombieSkulls;
	int Experience;
	int ReqExperience;
	int Level;
	
	unsigned long long WeaponBits;
	unsigned long long ZombieBits;
};