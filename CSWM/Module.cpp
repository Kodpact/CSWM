#include "Module.h"

#if defined _WIN32
#include <io.h>
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define DEFINE_CHAR64(Value) char Value[64]

#define LocateCharacter(Buffer) while (*Buffer < 33) { if (*Buffer == 0 || *Buffer == '=') return FALSE; Buffer++; }
#define ReadString(Buffer) while ((*Buffer >= 65 && *Buffer <= 90) || (*Buffer >= 97 && *Buffer <= 122) || (*Buffer >= 48 && *Buffer <= 57)) { Buffer++; }
#define ReadWord(Buffer) while (*Buffer > 32) { Buffer++; }

extern List<CWeapon> Weapons;
extern List<CAmmo> Ammos;
extern List<CProjectile> Projectiles;
extern List<CEffect> Effects;
extern List<int> TakeDamageFWs;

extern StringHashMap AnimHashMap;

extern BOOL CanPrecache;
extern BOOL ClearWeapons;
extern BOOL SV_Cheats;
extern float *CVar_LogPointer;
extern float *CVar_AMapPointer;
extern float *CVar_AutoDetectAnimPointer;

extern void *FPlayer_TakeDamage, *FPlayerBot_TakeDamage, *FEntity_TakeDamage;

char PathAddon[32] = "cswm/";

int WeaponCount = 0;
int AmmoCount = AMMO_MAX_TYPES;

const int WEAPON_TYPE_ID[] = { CSW_P228, CSW_XM1014, CSW_AK47, CSW_AWP };
const char WEAPON_TYPE_NAME[][8] = { "Pistol", "Shotgun", "Rifle", "Sniper" };
const int WEAPON_TYPE_AMMO[] = { 9, 5, 2, 1 };
const int WEAPON_TYPE_CLIP[] = { 13, 7, 30, 10 };
const float WEAPON_DEFAULT_DELAY[] = { 0.0f, 0.2f, 0.0f, 1.3f, 0.0f, 0.3f, 0.0f, 0.1f, 0.1f, 0.0f, 0.1f, 0.2f, 0.1f, 0.3f, 0.1f, 0.1f, 0.1f, 0.2f, 1.5f, 0.1f, 0.1f, 0.9f, 0.1f, 0.1f, 0.3f, 0.0f, 0.2f, 0.1f, 0.1f, 0.0f, 0.1f };
const int WEAPON_A2_SIZE[] = { 0, 4, 60, 4, 4, 12, 36, 24, 4 };
const float WEAPON_SPEED[] = { 250.0f, 240.0f, 221.0f, 210.0f };

DEFINE_CHAR64(SPR_Trail) = "sprites/laserbeam.spr";
DEFINE_CHAR64(SPR_Explosion) = "sprites/fexplo.spr";
DEFINE_CHAR64(SPR_Smoke) = "sprites/steam1.spr";
DEFINE_CHAR64(SPR_SmokePuff) = "sprites/wall_puff4.spr";
DEFINE_CHAR64(SPR_Ring) = "sprites/shockwave.spr";

int MI_Trail, MI_Explosion, MI_Smoke;
extern int MI_SmokePuff, MI_Ring;

TraceResult TResult;
CCleaveDamageInfo CleaveDamageInfo = { Vector(0,0,0), Vector(0,0,0), 0.0f, FALSE, 0.0f, 0.0f, NULL, NULL, 0 };
CKnockbackInfo KnockbackInfo = { 1.0f, 1.0f };
StringHashMap AParamHashMap, WParamHashMap;

void PrecacheModule(void)
{
	MI_Trail = PRECACHE_MODEL(SPR_Trail);
	MI_Explosion = PRECACHE_MODEL(SPR_Explosion);
	MI_Smoke = PRECACHE_MODEL(SPR_Smoke);
	MI_SmokePuff = PRECACHE_MODEL(SPR_SmokePuff);
	MI_Ring = PRECACHE_MODEL(SPR_Ring);
}

static int GetLabelStartIndex(studiohdr_t *Studio)
{
	mstudioseqdesc_t *SequenceIdle = (mstudioseqdesc_t *)((byte *)Studio + Studio->seqindex);
	const char *Idle = SequenceIdle->label;
	int LengthI = strlen(Idle);

	for (int Index = 0; Index < LengthI; Index++)
	{
		if (!strncmp(Idle + Index, "idle", 4))
			return Index;
	}

	return 0;
}

BOOL ValidAttackAnim(char *Value)
{
	if (!Value)
		return TRUE;

	if (Value[1] && ((Value[1] >= 'a' && Value[1] <= 'z') || (Value[1] >= 'A' && Value[1] <= 'Z')))
		return FALSE;

	return TRUE;
}

void DetectAnimation(CWeapon &Weapon, int Type)
{
	edict_t *InfoEdict = CREATE_ENTITY();
	SET_MODEL(InfoEdict, STRING(Weapon.VModel));
	studiohdr_t *Studio = (studiohdr_t *)GET_MODEL_PTR(InfoEdict);
	REMOVE_ENTITY(InfoEdict);
	int StartIndex = GetLabelStartIndex(Studio);
	
	if (Type == DETECT_SHOOT && Weapon.AnimS.Length)
		Weapon.AnimS.Clear();

	for (int Index = 0; Index < Studio->numseq; Index++)
	{
		mstudioseqdesc_t *Sequence = (mstudioseqdesc_t *)((byte *)Studio + Studio->seqindex) + Index;

		if ((int)strlen(Sequence->label) <= StartIndex)
			break;

		char *Start = Sequence->label + StartIndex;

		switch (Type)
		{
			case DETECT_DRAW: if (!strnicmp(Start, "draw", 4)) Weapon.AnimD = Index; break;
			case DETECT_SHOOT: if (!strnicmp(Start, "shoot", 5) && ValidAttackAnim(Start + 5)) Weapon.AnimS.Append(Index); break;
			case DETECT_RELOAD: if (!strnicmp(Start, "reload", 6)) Weapon.AnimR = Index; break;
		}
	}
}

BOOL InvalidAnimation(CWeapon &Weapon, int Animation)
{
	if (Weapon.DurationList.Length <= Animation || Animation < 0)
	{
		if (*CVar_LogPointer)
			LOG_CONSOLE(PLID, "[CSWM] Weapon %s Has Invalid Animation (%i), Trying To Reset...", Weapon.Model, Animation);

		return TRUE;
	}

	return FALSE;
}

void CheckAmmo(CAmmo &Ammo, int Index)
{
	if (!Ammo.Cost)
	{
		if (*CVar_LogPointer)
			LOG_CONSOLE(PLID, "[CSWM] Found Ammo Without Cost, Setting Default... (ID=%i)", Index);

		Ammo.Cost = 30;
	}

	if (!Ammo.Amount)
	{
		if (*CVar_LogPointer)
			LOG_CONSOLE(PLID, "[CSWM] Found Ammo Without Clip Size, Setting Default... (ID=%i)", Index);

		Ammo.Amount = 30;
	}

	if (!Ammo.Max)
	{
		if (*CVar_LogPointer)
			LOG_CONSOLE(PLID, "[CSWM] Found Ammo Without Max Count, Setting Default... (ID=%i)", Index);

		Ammo.Cost = 90;
	}

	if (!Ammo.Name)
	{
		char Name[32];
		sprintf(Name, "Ammo%i", AmmoCount);
		Ammo.Name = STRING(ALLOC_STRING(Name));
	}
}

void CheckWeapon(CWeapon &Weapon)
{
	char Buffer[260];

	if (!Weapon.VModel)
	{
		if (*CVar_LogPointer)
			LOG_CONSOLE(PLID, "[CSWM] Weapon %s Has Invalid View Model, Trying To Detect...", Weapon.Model);

		sprintf(Buffer, "models/%s%s/V.mdl", PathAddon, Weapon.Model);
		Weapon.VModel = ALLOC_STRING(Buffer);
	}

	PRECACHE_MODEL(STRING(Weapon.VModel));

	if (!Weapon.PModel)
	{
		if (*CVar_LogPointer)
			LOG_CONSOLE(PLID, "[CSWM] Weapon %s Has Invalid Weap Model, Trying To Detect...", Weapon.Model);

		sprintf(Buffer, "models/%s%s/P.mdl", PathAddon, Weapon.Model);
		Weapon.PModel = ALLOC_STRING(Buffer);
	}

	PRECACHE_MODEL(STRING(Weapon.PModel));

	if (!Weapon.WModel)
	{
		if (*CVar_LogPointer)
			LOG_CONSOLE(PLID, "[CSWM] Weapon %s Has Invalid World Model, Trying To Detect...", Weapon.Model);

		sprintf(Buffer, "models/%s%s/W.mdl", PathAddon, Weapon.Model);
		Weapon.WModel = STRING(ALLOC_STRING(Buffer));
	}

	PRECACHE_MODEL(Weapon.WModel);

	if (!Weapon.GModel)
	{
		if (*CVar_LogPointer)
			LOG_CONSOLE(PLID, "[CSWM] Weapon %s Has Invalid WeaponList, Trying To Detect...", Weapon.Model);

		sprintf(Buffer, "weapon_%s", Weapon.Model);
		Weapon.GModel = STRING(ALLOC_STRING(Buffer));
	}

	sprintf(Buffer, "sprites/%s.txt", Weapon.GModel);
	PRECACHE_GENERIC(STRING(ALLOC_STRING(Buffer)));

	if (!Weapon.FireSound)
	{
		if (*CVar_LogPointer)
			LOG_CONSOLE(PLID, "[CSWM] Weapon %s Has Invalid Fire Sound, Trying To Detect...", Weapon.Model);

		sprintf(Buffer, "weapons/%s-1.wav", Weapon.Model);
		Weapon.FireSound = STRING(ALLOC_STRING(Buffer));
	}

	PRECACHE_SOUND(Weapon.FireSound);

	if (!Weapon.AmmoID)
	{
		if (*CVar_LogPointer)
			LOG_CONSOLE(PLID, "[CSWM] Weapon %s Has Invalid Ammo Type, Setting Default...", Weapon.Model);

		Weapon.AmmoID = (AmmoType)WEAPON_TYPE_AMMO[Weapon.Type];
	}

	if (!Weapon.Clip)
	{
		if (*CVar_LogPointer)
			LOG_CONSOLE(PLID, "[CSWM] Weapon %s Has Invalid Clip, Setting Default...", Weapon.Model);

		Weapon.Clip = WEAPON_TYPE_CLIP[Weapon.Type];
	}

	RecordWeaponDurationList(Weapon);

	if (!Weapon.AnimD || InvalidAnimation(Weapon, Weapon.AnimD))
		DetectAnimation(Weapon, DETECT_DRAW);

	if (!Weapon.Deploy)
		Weapon.Deploy = Weapon.DurationList[Weapon.AnimD];

	if (!Weapon.Delay)
		Weapon.Delay = WEAPON_DEFAULT_DELAY[WEAPON_TYPE_ID[Weapon.Type]];

	if (Weapon.AnimS.Length < 1 || InvalidAnimation(Weapon, Weapon.AnimS[0]))
	{
		DetectAnimation(Weapon, DETECT_SHOOT);

		if (Weapon.AnimS.Length < 1)
		{
			if (*CVar_LogPointer)
				LOG_CONSOLE(PLID, "[CSWM] Can Not Detect Weapon Primary Attack Animation. (%s)", Weapon.Model);
			
			Weapon.AnimS.Append(0);
		}
	}

	if (!Weapon.AnimR || InvalidAnimation(Weapon, Weapon.AnimR))
		DetectAnimation(Weapon, DETECT_RELOAD);

	if (!Weapon.Reload)
		Weapon.Reload = Weapon.DurationList[Weapon.AnimR];

	if (!Weapon.Speed)
	{
		if (*CVar_LogPointer)
			LOG_CONSOLE(PLID, "[CSWM] Found Weapon Without Speed, Using Default Setting... (%s)", Weapon.Model);

		Weapon.Speed = WEAPON_SPEED[Weapon.Type];
	}

	if (Weapon.AnimD <= 0 || Weapon.AnimD > 10.0f)
	{
		if (*CVar_LogPointer)
			LOG_CONSOLE(PLID, "[CSWM] Weapon %s Has Draw Animation With %.2f Delay!", Weapon.Model, Weapon.AnimD);
	}

	if (Weapon.Delay <= 0 || Weapon.Delay > 10.0f)
	{
		if (*CVar_LogPointer)
			LOG_CONSOLE(PLID, "[CSWM] Warning: Weapon %s Has Shoot Animation With %.2f Delay!", Weapon.Model, Weapon.AnimD);
	}

	if (Weapon.AnimR <= 0 || Weapon.AnimR > 10.0f)
	{
		if (*CVar_LogPointer)
			LOG_CONSOLE(PLID, "[CSWM] Warning: Weapon %s Has Reload Animation With %.2f Delay!", Weapon.Model, Weapon.AnimD);
	}
}

void RecordWeaponDurationList(CWeapon &Weapon)
{
	edict_t *InfoEdict = CREATE_ENTITY();
	SET_MODEL(InfoEdict, STRING(Weapon.VModel));
	studiohdr_t *Studio = (studiohdr_t *)GET_MODEL_PTR(InfoEdict);
	REMOVE_ENTITY(InfoEdict);

	for (int Index = 0; Index < Studio->numseq; Index++)
	{
		mstudioseqdesc_t *Sequence = (mstudioseqdesc_t *)((byte *)Studio + Studio->seqindex) + Index;
		Weapon.DurationList.Append(Sequence->numframes / Sequence->fps);
	}
}

Vector &GetVectorFromAddress(cell *Addr)
{
	static Vector Output;
	Output.x = CellToFloat(Addr[0]);
	Output.y = CellToFloat(Addr[1]);
	Output.z = CellToFloat(Addr[2]);
	return Output;
}

static cell AMX_NATIVE_CALL CreateWeapon(AMX *Plugin, cell *Params)
{
	if (!CanPrecache)
	{
		LOG_CONSOLE(PLID, "[CSWM] Can Not Register Weapon At This Time.");
		return 0;
	}

	CWeapon Weapon;
	memset(&Weapon, 0, sizeof(CWeapon));

	Weapon.Model = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[1], 0, NULL)));
	Weapon.Name = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[3], 0, NULL)));
	WType Type = (WType)Params[2];
	Weapon.Type = Type;
	Weapon.Speed = 250.0f;

	if (*CVar_LogPointer)
		LOG_CONSOLE(PLID, "[CSWM] Creating Weapon: %s (Type = '%s')", Weapon.Name, WEAPON_TYPE_NAME[Weapon.Type]);

	Weapons.Append(Weapon);
	WeaponCount++;
	return WeaponCount - 1;
}

static cell AMX_NATIVE_CALL CreateAmmo(AMX *Plugin, cell *Params)
{
	CAmmo Ammo;
	Ammo.Cost = Params[1];
	Ammo.Amount = Params[2];
	Ammo.Max = Params[3];
	Ammo.Name = NULL;

	Ammos.Append(Ammo);
	AmmoCount++;
	return AmmoCount - 1;
}

static cell AMX_NATIVE_CALL BuildWeaponModels(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	CWeapon &Weapon = Weapons[Index];

	PRECACHE_MODEL(STRING(Weapon.VModel = ALLOC_STRING(MF_GetAmxString(Plugin, Params[2], 0, NULL))));
	PRECACHE_MODEL(STRING(Weapon.PModel = ALLOC_STRING(MF_GetAmxString(Plugin, Params[3], 0, NULL))));
	PRECACHE_MODEL(Weapon.WModel = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[4], 0, NULL))));
	RecordWeaponDurationList(Weapon);
	return 0;
}

static cell AMX_NATIVE_CALL BuildWeaponFireSound(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	CWeapon &Weapon = Weapons[Index];

	char *String = MF_GetAmxString(Plugin, Params[2], 0, NULL);
	Weapon.FireSound = STRING(ALLOC_STRING(String));
	PRECACHE_SOUND(Weapon.FireSound);
	return 0;
}

static cell AMX_NATIVE_CALL BuildWeaponList(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	CWeapon &Weapon = Weapons[Index];

	Weapon.GModel = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[2], 0, NULL)));

	if (*CVar_LogPointer)
		LOG_CONSOLE(PLID, "[CSWM] Building Weapon List '%s' For '%s'.", Weapon.GModel, Weapon.Name);

	char Path[MAX_PATH];
	sprintf(Path, "sprites/%s.txt", Weapon.GModel);
	PRECACHE_GENERIC(STRING(ALLOC_STRING(Path)));
	return 0;
}

static cell AMX_NATIVE_CALL BuildWeaponAmmunition(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	CWeapon &Weapon = Weapons[Index];

	if (*CVar_LogPointer)
		LOG_CONSOLE(PLID, "[CSWM] Building Weapon Ammunition For '%s'.", Weapon.Name);

	Weapon.Clip = Params[2];
	Weapon.AmmoID = (AmmoType)Params[3];
	return 0;
}

static cell AMX_NATIVE_CALL BuildWeaponDeploy(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	CWeapon &Weapon = Weapons[Index];

	if (*CVar_LogPointer)
		LOG_CONSOLE(PLID, "[CSWM] Building Weapon Deploy For '%s'.", Weapon.Name);

	float Duration = CellToFloat(Params[3]);

	if (Params[2])
		Weapon.AnimD = Params[2];
	else
		DetectAnimation(Weapon, DETECT_DRAW);

	if (Duration > 0.0f)
		Weapon.Deploy = Duration;
	else
		Weapon.Deploy = Weapon.DurationList[Weapon.AnimD];

	return 0;
}

static cell AMX_NATIVE_CALL BuildWeaponPrimaryAttack(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	CWeapon &Weapon = Weapons[Index];

	if (*CVar_LogPointer)
		LOG_CONSOLE(PLID, "[CSWM] Building Weapon Priamry Attack For '%s'.", Weapon.Name);

	Weapon.Delay = CellToFloat(Params[2]);
	Weapon.Damage = CellToFloat(Params[3]);
	Weapon.Recoil = CellToFloat(Params[4]);

	if ((Params[0] / sizeof(cell)) >= 5 && *GetAMXAddr(Plugin, Params[5]))
	{
		Weapon.AnimS.Append(Params[5]);

		for (Index = 5; Index <= Params[0] / (cell)sizeof(cell); Index++)
			Weapon.AnimS.Append(*GetAMXAddr(Plugin, Params[Index]));
	}
	else
		DetectAnimation(Weapon, DETECT_SHOOT);

	return 0;
}

static cell AMX_NATIVE_CALL BuildWeaponReload(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	CWeapon &Weapon = Weapons[Index];

	if (*CVar_LogPointer)
		LOG_CONSOLE(PLID, "[CSWM] Building Weapon Reload(Rifle) For '%s'.", Weapon.Name);

	float Duration = CellToFloat(Params[3]);

	if (Params[2])
		Weapon.AnimR = Params[2];
	else
		DetectAnimation(Weapon, DETECT_RELOAD);

	if (Duration > 0.0f)
		Weapon.Reload = Duration;
	else
		Weapon.Reload = Weapon.DurationList[Weapon.AnimR];

	return 0;
}

static cell AMX_NATIVE_CALL BuildWeaponReloadShotgun(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	CWeapon &Weapon = Weapons[Index];

	if (*CVar_LogPointer)
		LOG_CONSOLE(PLID, "[CSWM] Building Weapon Reload(Shotgun) For '%s'.", Weapon.Name);

	Weapon.Reload = CellToFloat(Params[2]);
	Weapon.AnimR = Params[3];
	return 0;
}

static cell AMX_NATIVE_CALL BuildWeaponFlags(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	Weapons[Index].Flags |= Params[2];
	return 0;
}

static cell AMX_NATIVE_CALL BuildWeaponSecondaryAttack(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	CWeapon &Weapon = Weapons[Index];

	if (*CVar_LogPointer)
		LOG_CONSOLE(PLID, "[CSWM] Building Weapon Secondary Attack For '%s'.", Weapon.Name);

	Weapon.A2V = (Attack2 *)new int[WEAPON_A2_SIZE[Params[2]]];

	switch (Weapon.A2I = Params[2])
	{
		case A2_Zoom:
		{
			GetAttack2Data(int, Weapon.A2V, WA2_ZOOM_MODE) = *MF_GetAmxAddr(Plugin, Params[3]);
			break;
		}
		case A2_Switch:
		{
			GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_A) = *MF_GetAmxAddr(Plugin, Params[3]);
			GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_ANIM_A_DURATION) = CellToFloat(*GetAMXAddr(Plugin, Params[4]));
			GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_B) = *MF_GetAmxAddr(Plugin, Params[5]);
			GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_ANIM_B_DURATION) = CellToFloat(*GetAMXAddr(Plugin, Params[6]));
			GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_IDLE) = *MF_GetAmxAddr(Plugin, Params[7]);
			GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_DRAW) = *MF_GetAmxAddr(Plugin, Params[8]);
			GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_ANIM_DRAW_DURATION) = CellToFloat(*GetAMXAddr(Plugin, Params[9]));
			GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_SHOOT) = *MF_GetAmxAddr(Plugin, Params[10]);
			GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_ANIM_SHOOT_DURATION) = CellToFloat(*GetAMXAddr(Plugin, Params[11]));
			GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_RELOAD) = *MF_GetAmxAddr(Plugin, Params[12]);
			GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_ANIM_RELOAD_DURATION) = CellToFloat(*GetAMXAddr(Plugin, Params[13]));
			GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_DELAY) = CellToFloat(*GetAMXAddr(Plugin, Params[14]));
			GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_DAMAGE) = CellToFloat(*GetAMXAddr(Plugin, Params[15]));
			GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_RECOIL) = CellToFloat(*GetAMXAddr(Plugin, Params[16]));
			PRECACHE_SOUND(GetAttack2Data(const char *, Weapon.A2V, WA2_SWITCH_FSOUND) = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[17], 0, NULL))));
			break;
		}
		case A2_Burst:
		{
			GetAttack2Data(int, Weapon.A2V, WA2_BURST_VALUE) = *MF_GetAmxAddr(Plugin, Params[3]);
			break;
		}
		case A2_MultiShot:
		{
			GetAttack2Data(int, Weapon.A2V, WA2_MULTISHOT_VALUE) = *MF_GetAmxAddr(Plugin, Params[3]);
			break;
		}
		case A2_AutoPistol:
		{
			GetAttack2Data(int, Weapon.A2V, WA2_AUTOPISTOL_ANIM) = *MF_GetAmxAddr(Plugin, Params[3]);
			GetAttack2Data(float, Weapon.A2V, WA2_AUTOPISTOL_DELAY) = CellToFloat(*GetAMXAddr(Plugin, Params[4]));
			GetAttack2Data(float, Weapon.A2V, WA2_AUTOPISTOL_RECOIL) = CellToFloat(*GetAMXAddr(Plugin, Params[5]));
			break;
		}
		case A2_KnifeAttack:
		{
			GetAttack2Data(int, Weapon.A2V, WA2_KNIFEATTACK_ANIMATION) = *MF_GetAmxAddr(Plugin, Params[3]);
			GetAttack2Data(float, Weapon.A2V, WA2_KNIFEATTACK_DELAY) = CellToFloat(*GetAMXAddr(Plugin, Params[4]));
			GetAttack2Data(float, Weapon.A2V, WA2_KNIFEATTACK_DURATION) = CellToFloat(*GetAMXAddr(Plugin, Params[5]));
			GetAttack2Data(float, Weapon.A2V, WA2_KNIFEATTACK_RADIUS) = CellToFloat(*GetAMXAddr(Plugin, Params[6]));
			GetAttack2Data(int, Weapon.A2V, WA2_KNIFEATTACK_DAMAGE_MIN) = *MF_GetAmxAddr(Plugin, Params[7]);
			GetAttack2Data(int, Weapon.A2V, WA2_KNIFEATTACK_DAMAGE_MAX) = *MF_GetAmxAddr(Plugin, Params[8]);
			GetAttack2Data(float, Weapon.A2V, WA2_KNIFEATTACK_KNOCKBACK) = CellToFloat(*GetAMXAddr(Plugin, Params[9]));
			PRECACHE_SOUND(GetAttack2Data(const char *, Weapon.A2V, WA2_KNIFEATTACK_SOUND) = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[10], 0, NULL))));
			break;
		}
		case A2_InstaSwitch:
		{
			GetAttack2Data(int, Weapon.A2V, WA2_INSTASWITCH_ANIM_SHOOT) = *MF_GetAmxAddr(Plugin, Params[3]);
			GetAttack2Data(float, Weapon.A2V, WA2_INSTASWITCH_DELAY) = CellToFloat(*GetAMXAddr(Plugin, Params[4]));
			GetAttack2Data(float, Weapon.A2V, WA2_INSTASWITCH_DAMAGE) = CellToFloat(*GetAMXAddr(Plugin, Params[5]));
			GetAttack2Data(float, Weapon.A2V, WA2_INSTASWITCH_RECOIL) = CellToFloat(*GetAMXAddr(Plugin, Params[6]));
			GetAttack2Data(const char *, Weapon.A2V, WA2_INSTASWITCH_NAME) = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[7], 0, NULL)));
			GetAttack2Data(const char *, Weapon.A2V, WA2_INSTASWITCH_NAME2) = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[8], 0, NULL)));
		}
		case A2_ZoomCustom:
		{
			GetAttack2Data(int, Weapon.A2V, WA2_ZOOM_CUSTOM_FOV) = *MF_GetAmxAddr(Plugin, Params[3]);
			break;
		}
		default:
		{
			if (*CVar_LogPointer)
				LOG_CONSOLE(PLID, "[CSWM] Invalid Attack2 Type: %i, Resetting...", Weapon.A2I);

			Weapon.A2I = 0;
		}
	}

	return 0;
}

static cell AMX_NATIVE_CALL BuildWeaponMaxSpeed(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	Weapons[Index].Speed = CellToFloat(Params[2]);
	return 0;
}

static cell AMX_NATIVE_CALL BuildWeaponSpecs(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	CWeapon &Weapon = Weapons[Index];
	Weapon.Delay = CellToFloat(Params[2]);
	Weapon.Recoil = CellToFloat(Params[3]);
	Weapon.Damage = CellToFloat(Params[4]);
	return 0;
}

static cell AMX_NATIVE_CALL CheckWeaponParams(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	CheckWeapon(Weapons[Index]);
	return 0;
}

static cell AMX_NATIVE_CALL RegisterWeaponForward(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	CWeapon &Weapon = Weapons[Index];

	if (*CVar_LogPointer)
		LOG_CONSOLE(PLID, "[CSWM] Registering Weapon Forward For '%s'.", Weapon.Name);

	char *String = MF_GetAmxString(Plugin, Params[3], 0, NULL);

	int Forward = Params[2];

	if (Forward == WForward::DamagePost)
		Weapon.Forwards[Forward] = MF_RegisterSPForwardByName(Plugin, String, FP_CELL, FP_FLOAT, FP_CELL, FP_DONE);
	else if (Forward == WForward::DropPost)
		Weapon.Forwards[Forward] = MF_RegisterSPForwardByName(Plugin, String, FP_CELL, FP_CELL, FP_DONE);
	else
		Weapon.Forwards[Forward] = MF_RegisterSPForwardByName(Plugin, String, FP_CELL, FP_DONE);
	
	return 0;
}

static cell AMX_NATIVE_CALL PrecacheWeaponModelSounds(AMX *AMX, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	edict_t *InfoEdict = CREATE_ENTITY();
	SET_MODEL(InfoEdict, STRING(Weapons[Index].VModel));
	studiohdr_t *Studio = (studiohdr_t *)GET_MODEL_PTR(InfoEdict);
	REMOVE_ENTITY(InfoEdict);
	mstudioseqdesc_t *SeqDesc = (mstudioseqdesc_t *)((uintptr_t)Studio + Studio->seqindex);

	for (int index = 0; index < Studio->numseq; index++)
	{
		mstudioevent_t *Event = (mstudioevent_t *)((uintptr_t)Studio + SeqDesc[index].eventindex);

		for (int i = 0; i < SeqDesc[index].numevents; i++)
		{
			if (Event[i].event != 5004)
				continue;

			if (Event[i].options[0] == '\0')
				continue;
			
			PRECACHE_SOUND(Event[i].options);
		}
	}

	return 0;
}

static cell AMX_NATIVE_CALL PrecacheWeaponModelSoundsAsGenerics(AMX *AMX, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	edict_t *InfoEdict = CREATE_ENTITY();
	SET_MODEL(InfoEdict, STRING(Weapons[Index].VModel));
	studiohdr_t *Studio = (studiohdr_t *)GET_MODEL_PTR(InfoEdict);
	REMOVE_ENTITY(InfoEdict);
	mstudioseqdesc_t *SeqDesc = (mstudioseqdesc_t *)((uintptr_t)Studio + Studio->seqindex);

	for (int index = 0; index < Studio->numseq; index++)
	{
		mstudioevent_t *Event = (mstudioevent_t *)((uintptr_t)Studio + SeqDesc[index].eventindex);

		for (int i = 0; i < SeqDesc[index].numevents; i++)
		{
			if (Event[i].event != 5004)
				continue;

			if (Event[i].options[0] == '\0')
				continue;

			char Buffer[MAX_PATH];
			sprintf(Buffer, "weapons/%s", Event[i].options);
			PRECACHE_SOUND(STRING(ALLOC_STRING(Buffer)));
		}
	}

	return 0;
}

BOOL LocateSprite(char *&Text)
{
	int Level = 0;

	while (*Text)
	{
		if (!Level)
		{
			if (*Text > 32)
			{
				Level = 1;
				
				if ((strnicmp(Text, "zoom", 4) || Text[4] == '_') &&
					strnicmp(Text, "weapon", 6) && strnicmp(Text, "ammo", 4))
					return FALSE;
			}
		}
		else if (Level > 0)
		{
			if (*Text <= 32)
				Level = -(Level + 1);
		}
		else if (Level < 0)
		{
			if (*Text > 32)
				Level = -(Level - 1);

			if (Level == 5)
				break;
		}

		Text++;
	}
	
	if (*Text == 0)
		return FALSE;

	return TRUE;
}

static cell AMX_NATIVE_CALL PrecacheWeaponListSprites(AMX *AMX, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	const char *Path = MF_BuildPathname("sprites/%s.txt", Weapons[Index].GModel);
	FILE *HUD = fopen(Path, "rt");
	
	if (!HUD)
		return 0;

	char Buffer[260];

	while (!feof(HUD))
	{
		fgets(Buffer, 260, HUD);

		if (!Buffer[0])
			continue;

		char *Temp = Buffer;
		
		if (!LocateSprite(Temp))
			continue;

		char *Start = Temp;
		ReadWord(Temp);
		
		if (Temp == Start)
			continue;

		if (*Temp != 0)
			*Temp = 0;

		sprintf(Buffer, "sprites/%s.spr", Start);
		PRECACHE_GENERIC(STRING(ALLOC_STRING(Buffer)));
	}

	fclose(HUD);
	return 1;
}

static cell AMX_NATIVE_CALL FindWeaponByModel(AMX *Plugin, cell *Params)
{
	char *Name = MF_GetAmxString(Plugin, Params[1], 0, NULL);

	for (int Index = 0; Index < WeaponCount; Index++)
	{
		if (!strcmp(Name, Weapons[Index].Model))
			return Index;
	}

	return -1;
}

static cell AMX_NATIVE_CALL FindWeaponByName(AMX *Plugin, cell *Params)
{
	char *Name = MF_GetAmxString(Plugin, Params[1], 0, NULL);

	for (int Index = 0; Index < WeaponCount; Index++)
	{
		if (!strcmp(Name, Weapons[Index].Name))
			return Index;
	}

	return -1;
}

static cell AMX_NATIVE_CALL GiveWeaponByModel(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index > gpGlobals->maxClients)
		return FALSE;

	return NUM_FOR_EDICT(GiveWeaponByName(EDICT_FOR_NUM(Index), (char *)MF_GetAmxString(Plugin, Params[2], 0, NULL)));
}

static cell AMX_NATIVE_CALL GiveWeaponByID(AMX *Plugin, cell *Params)
{
	cell PIndex = Params[1];
	cell WIndex = Params[2];

	if (PIndex < 0 || PIndex > gpGlobals->maxClients)
		return 0;

	if (WIndex < 0 || WIndex >= WeaponCount)
		return 0;

	edict_t *Result = GiveWeapon(EDICT_FOR_NUM(PIndex), WIndex);

	if (!Result)
		return 0;

	return NUM_FOR_EDICT(Result);
}

#pragma warning (disable : 4701)

static cell AMX_NATIVE_CALL GetWeaponData(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	CWeapon &Weapon = Weapons[Index];
	int Type;
	void *Data = NULL;
	
	switch (Params[2])
	{
		case WData::WD_VModel: Type = TYPE_STRING; Data = (void *)Weapon.VModel; break;
		case WData::WD_PModel: Type = TYPE_STRING; Data = (void *)Weapon.PModel; break;
		case WData::WD_Model: Type = TYPE_STRING; Data = (void *)Weapon.Model; break;
		case WData::WD_Name: Type = TYPE_STRING; Data = (void *)Weapon.Name; break;
		case WData::WD_FireSound: Type = TYPE_STRING; Data = (void *)Weapon.FireSound; break;
		case WData::WD_WModel: Type = TYPE_STRING; Data = (void *)Weapon.WModel; break;
		case WData::WD_GModel: Type = TYPE_STRING; Data = (void *)Weapon.GModel; break;
		case WData::WD_Type: Type = TYPE_INT; Data = &Weapon.Type; break;
		case WData::WD_AnimD: Type = TYPE_INT; Data = &Weapon.AnimD; break;
		case WData::WD_AnimS: Type = TYPE_ARRAY; Data = &Weapon.AnimS; break;
		case WData::WD_AnimR: Type = TYPE_INT; Data = &Weapon.AnimR; break;
		case WData::WD_Clip: Type = TYPE_INT; Data = &Weapon.Clip; break;
		case WData::WD_AmmoID: Type = TYPE_INT; Data = &Weapon.AmmoID; break;
		case WData::WD_Deploy: Type = TYPE_FLOAT; Data = &Weapon.Deploy; break;
		case WData::WD_Reload: Type = TYPE_FLOAT; Data = &Weapon.Reload; break;
		case WData::WD_Delay: Type = TYPE_FLOAT; Data = &Weapon.Delay; break;
		case WData::WD_Damage: Type = TYPE_FLOAT; Data = &Weapon.Damage; break;
		case WData::WD_Recoil: Type = TYPE_FLOAT; Data = &Weapon.Recoil; break;
		case WData::WD_Flags: Type = TYPE_INT; Data = &Weapon.Flags; break;
		case WData::WD_A2I: Type = TYPE_INT; Data = &Weapon.A2I; break;
		case WData::WD_Speed: Type = TYPE_FLOAT; Data = &Weapon.Speed; break;
		case WData::WD_Forwards: Type = TYPE_ARRAY; Data = &Weapon.Forwards; break;
		case WData::WD_DurationList: Type = TYPE_ARRAY; Data = &Weapon.DurationList; break;
		default: return 0;
	}
	
	if (!Data)
		return 0;

	cell Size = 0;

	if (Type > TYPE_FLOAT)
		Size = *GetAMXAddr(Plugin, Params[4]);
	
	if (Type <= TYPE_FLOAT)
		return *(int *)Data;
	else if (Type == TYPE_STRING)
		MF_SetAmxString(Plugin, Params[3], (const char *)Data, Size);
	else if (Type == TYPE_ARRAY)
	{
		Size = min(Size, ((List<int> *)Data)->Length);
		int *Variable = GetAMXAddr(Plugin, Params[3]);

		for (Index = 0; Index < Size; Index++)
			*(Variable + Index) = ((List<int> *)Data)->Data[Index];
	}

	return 0;
}

static cell AMX_NATIVE_CALL SendWeaponAnim(AMX *Plugin, cell *Params)
{
	edict_t *WeaponEdict = EDICT_FOR_NUM(Params[1]);

	if (!WeaponEdict)
		return 0;

	CBasePlayerWeapon *BaseWeapon = (CBasePlayerWeapon *)WeaponEdict->pvPrivateData;

	if (!BaseWeapon)
		return 0;

	SendWeaponAnim(BaseWeapon, Params[2]);
	return 0;
}

static cell AMX_NATIVE_CALL CreateProjectile(AMX *Plugin, cell *Params)
{
	CProjectile Projectile;
	memset(&Projectile, 0, sizeof(CProjectile));
	PRECACHE_MODEL(Projectile.Model = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[1], 0, NULL))));
	Projectile.Gravity = CellToFloat(Params[2]);
	Projectile.Speed = CellToFloat(Params[3]);
	Projectile.Forward = MF_RegisterSPForwardByName(Plugin, MF_GetAmxString(Plugin, Params[4], 0, NULL), FP_CELL, FP_DONE);
	Projectile.Duration = CellToFloat(Params[5]);
	Projectiles.Append(Projectile);
	return Projectiles.Length - 1;
}

static cell AMX_NATIVE_CALL CreateEffect(AMX *Plugin, cell *Params)
{
	CEffect Effect;
	memset(&Effect, 0, sizeof(CEffect));
	PRECACHE_MODEL(Effect.Model = STRING(Effect.ModelIndex = ALLOC_STRING(MF_GetAmxString(Plugin, Params[1], 0, NULL))));
	Effect.Speed = CellToFloat(Params[2]);
	Effect.Forward = MF_RegisterSPForwardByName(Plugin, MF_GetAmxString(Plugin, Params[3], 0, NULL), FP_CELL, FP_DONE);
	Effect.Duration = CellToFloat(Params[4]);
	Effects.Append(Effect);
	return Effects.Length - 1;
}

static cell AMX_NATIVE_CALL ShootProjectileTimed(AMX *Plugin, cell *Params)
{
	edict_t *LauncherEdict = EDICT_FOR_NUM(Params[1]);

	if (InvalidEntity(LauncherEdict))
		return 0;

	int ProjectileID = Params[2];

	if (ProjectileID < 0 || ProjectileID >= Projectiles.Length)
		return 0;

	return ShootProjectileTimed(LauncherEdict, ProjectileID);
}

static cell AMX_NATIVE_CALL ShootProjectileContact(AMX *Plugin, cell *Params)
{
	edict_t *LauncherEdict = EDICT_FOR_NUM(Params[1]);

	if (InvalidEntity(LauncherEdict))
		return 0;

	return ShootProjectileContact(LauncherEdict, Params[2]);
}

static cell AMX_NATIVE_CALL ShootEffect(AMX *Plugin, cell *Params)
{
	return ShootEffect(EDICT_FOR_NUM(Params[1]), Params[2]);
}

static cell AMX_NATIVE_CALL GiveAmmo(AMX *Plugin, cell *Params)
{
	edict_t *PlayerEdict = EDICT_FOR_NUM(Params[1]);

	if (InvalidEntity(PlayerEdict), IS_USER_DEAD(PlayerEdict))
		return 0;

	Player_GiveAmmoByID((CBasePlayer *)PlayerEdict->pvPrivateData, Params[2], Params[3]);
	return 0;
}

static cell AMX_NATIVE_CALL SetAmmoName(AMX *Plugin, cell *Params)
{
	int AmmoIndex = Params[1];

	if (AmmoIndex < 0 || AmmoIndex >= AmmoCount)
		return 0;

	Ammos[AmmoIndex].Name = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[2], 0, NULL)));
	return 0;
}

static cell AMX_NATIVE_CALL FindAmmoByName(AMX *Plugin, cell *Params)
{
	char *SearchAmmoName = MF_GetAmxString(Plugin, Params[1], 0, NULL);
	const char *AmmoName;

	for (int Index = 0; Index < Ammos.Length; Index++)
	{
		AmmoName = Ammos[Index].Name;

		if (Params[2])
		{
			if (!strcmp(SearchAmmoName, AmmoName))
				return (cell)Index;
		}
		else
		{
			if (!stricmp(SearchAmmoName, AmmoName))
				return (cell)Index;
		}
	}

	return 0;
}

static cell AMX_NATIVE_CALL GetWeaponCount(AMX *Plugin, cell *Params)
{
	return WeaponCount;
}

static cell AMX_NATIVE_CALL GetWeaponName(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return FALSE;

	const char *Name = Weapons[Index].Name;

	if (!Name)
		return FALSE;

	MF_SetAmxString(Plugin, Params[2], Name, Params[3]);
	return TRUE;
}

static cell AMX_NATIVE_CALL GetWeaponModel(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return FALSE;

	const char *Model = Weapons[Index].Model;

	if (!Model)
		return FALSE;

	MF_SetAmxString(Plugin, Params[2], Model, Params[3]);
	return TRUE;
}

static cell AMX_NATIVE_CALL SetWeaponPathAddon(AMX *Plugin, cell *Params)
{
	char *Path = MF_GetAmxString(Plugin, Params[1], 0, NULL);

	if (strlen(Path) > 31)
		Path[31] = 0;

	strcpy(PathAddon, Path);
	return 0;
}

static cell AMX_NATIVE_CALL GetWeaponFlags(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	return Weapons[Index].Flags;
}

static cell AMX_NATIVE_CALL CreateExplosion(AMX *Plugin, cell *Params)
{
	Vector &Origin = GetVectorFromAddress(GetAMXAddr(Plugin, Params[1]));
	int Flags = Params[2];

	if (MI_Explosion)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, NULL, NULL);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(Origin.x);
		WRITE_COORD(Origin.y);
		WRITE_COORD(Origin.z + 40.0f);
		WRITE_SHORT(MI_Explosion);
		WRITE_BYTE(25);
		WRITE_BYTE(25);
		WRITE_BYTE(Flags);
		MESSAGE_END();
	}

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, NULL, NULL);
	WRITE_BYTE(TE_WORLDDECAL);
	WRITE_COORD(Origin.x);
	WRITE_COORD(Origin.y);
	WRITE_COORD(Origin.z);
	WRITE_BYTE(RANDOM_LONG(46, 48));
	MESSAGE_END();

	if (MI_Smoke)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, NULL, NULL);
		WRITE_BYTE(TE_SMOKE);
		WRITE_COORD(Origin.x);
		WRITE_COORD(Origin.y);
		WRITE_COORD(Origin.z);
		WRITE_SHORT(MI_Smoke);
		WRITE_BYTE(30);
		WRITE_BYTE(10);
		MESSAGE_END();
	}

	return 0;
}

static cell AMX_NATIVE_CALL RadiusDamageEx(AMX *Plugin, cell *Params)
{
	Vector &Origin = GetVectorFromAddress(GetAMXAddr(Plugin, Params[1]));
	float Radius = CellToFloat(Params[2]);
	float Damage = CellToFloat(Params[3]);
	edict_t *TargetEdict = SVGame_Edicts;
	CBaseEntity *BaseEntity;
	entvars_t *EntityVars;
	Vector TargetOrigin;
	entvars_t *InflictorVars = &EDICT_FOR_NUM(Params[4])->v;
	entvars_t *AttackerVars = &EDICT_FOR_NUM(Params[5])->v;
	int DamageBits = Params[6];
	int Flags = Params[7];
	edict_t *SelfEdict = AttackerVars->flags & FL_CLIENT ? AttackerVars->pContainingEntity : AttackerVars->owner;
	float AdjustedDamage;
	BOOL Result;
	
	while ((TargetEdict = FIND_ENTITY_IN_SPHERE(TargetEdict, Origin, Radius)) != SVGame_Edicts)
	{
		if (!TargetEdict->pvPrivateData)
			continue;

		if (!TargetEdict->v.takedamage)
			continue;

		BaseEntity = (CBaseEntity *)TargetEdict->pvPrivateData;
		EntityVars = EV_FROM_PD(BaseEntity);
		TargetOrigin = TargetEdict->v.origin;
		
		if (EntityVars->solid == SOLID_BSP)
			TargetOrigin = TargetOrigin + (TargetEdict->v.mins + TargetEdict->v.maxs) * 0.5f;

		if (Flags & RDFlag::Penetration)
		{
			TRACE_LINE(Origin, TargetOrigin, 0, 0, &TResult);

			if (TResult.pHit != TargetEdict && TResult.flFraction < 0.9f)
				continue;
		}

		if (Flags & RDFlag::IgnoreSelf && TargetEdict == SelfEdict)
		{
			if (Flags & RDFlag::KnockAny)
				goto TryKnock;

			continue;
		}

		AdjustedDamage = sin((1 - (clamp((TargetOrigin - Origin).Length(), 0.0f, Radius) / Radius)) * (3.14159f / 2)) * Damage;

		if (AdjustedDamage <= 0.0f)
			continue;
		
		Result = CALLFUNC_GAME(FN_TAKEDAMAGE, (*(void ***)BaseEntity)[EO_TakeDamage], BaseEntity, InflictorVars, AttackerVars, AdjustedDamage, DamageBits | DMG_CUSTOM);

		if (!(Flags & RDFlag::KnockAny) && !Result)
			continue;

	TryKnock:
		if (EntityVars->flags & FL_CLIENT)
			PlayerKnockback(EntityVars->pContainingEntity, Origin);
	}

	return 0;
}

void PerformCleaveDamage()
{
	edict_t *TargetEdict = SVGame_Edicts;
	CBaseEntity *BaseEntity;
	entvars_t *EntityVars;
	Vector TargetOrigin;

	while ((TargetEdict = FIND_ENTITY_IN_SPHERE(TargetEdict, CleaveDamageInfo.Origin, CleaveDamageInfo.Radius)) != SVGame_Edicts)
	{
		if (!TargetEdict->pvPrivateData)
			continue;

		if (!TargetEdict->v.takedamage)
			continue;

		BaseEntity = (CBaseEntity *)TargetEdict->pvPrivateData;
		EntityVars = EV_FROM_PD(BaseEntity);
		TargetOrigin = TargetEdict->v.origin;

		if (EntityVars->solid == SOLID_BSP)
			TargetOrigin = TargetOrigin + (TargetEdict->v.mins + TargetEdict->v.maxs) * 0.5f;

		if (!InViewCone(CleaveDamageInfo.Origin, CleaveDamageInfo.VAngles, CleaveDamageInfo.FOV, TargetOrigin, CleaveDamageInfo.Accurate))
			continue;

		CALLFUNC_GAME(FN_TAKEDAMAGE, (*(void ***)BaseEntity)[EO_TakeDamage], BaseEntity,
			CleaveDamageInfo.Inflictor, CleaveDamageInfo.Attacker,
			((CleaveDamageInfo.Damage *CleaveDamageInfo.Radius) / (TargetOrigin - CleaveDamageInfo.Origin).Length()),
			CleaveDamageInfo.DamageType | DMG_CUSTOM);
	}
}

static cell AMX_NATIVE_CALL CleaveDamage(AMX *Plugin, cell *Params)
{
	CleaveDamageInfo.Origin = GetVectorFromAddress(GetAMXAddr(Plugin, Params[1]));
	CleaveDamageInfo.VAngles = GetVectorFromAddress(GetAMXAddr(Plugin, Params[2]));
	CleaveDamageInfo.FOV = CellToFloat(Params[3]);
	CleaveDamageInfo.Accurate = Params[4];
	CleaveDamageInfo.Damage = CellToFloat(Params[5]);
	CleaveDamageInfo.Radius = CellToFloat(Params[6]);
	CleaveDamageInfo.Inflictor = &EDICT_FOR_NUM(Params[7])->v;
	CleaveDamageInfo.Attacker = &EDICT_FOR_NUM(Params[8])->v;
	CleaveDamageInfo.DamageType = Params[9];
	PerformCleaveDamage();
	return 0;
}

static cell AMX_NATIVE_CALL CleaveDamageByPlayer(AMX *Plugin, cell *Params)
{
	edict_t *PlayerEdict = EDICT_FOR_NUM(Params[1]);
	CleaveDamageInfo.Origin = (PlayerEdict->v.origin + PlayerEdict->v.view_ofs);
	CleaveDamageInfo.VAngles = PlayerEdict->v.v_angle;
	CleaveDamageInfo.FOV = PlayerEdict->v.fov;
	CleaveDamageInfo.Accurate = FALSE;
	CleaveDamageInfo.Damage = CellToFloat(Params[2]);
	CleaveDamageInfo.Radius = CellToFloat(Params[3]);
	CleaveDamageInfo.Inflictor = &PlayerEdict->v;
	CleaveDamageInfo.Attacker = &PlayerEdict->v;
	CleaveDamageInfo.DamageType = DMG_SLASH;
	PerformCleaveDamage();
	return 0;
}

static cell AMX_NATIVE_CALL CanPrimaryAttack(AMX *Plugin, cell *Params)
{
	CBasePlayerWeapon *BaseWeapon = (CBasePlayerWeapon *)(EDICT_FOR_NUM(Params[1])->pvPrivateData);
	return ((GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextPrimaryAttack, 4) <= 0.0f)
		&& (GetPrivateData(float, GetPrivateData(CBasePlayer *, BaseWeapon, CBasePlayerItem_Player, 4), CBaseMonster_NextAttack, 5) <= 0.0f)) ? TRUE : FALSE;
}

static cell AMX_NATIVE_CALL SetNextAttack(AMX *Plugin, cell *Params)
{
	CBasePlayerWeapon *BaseWeapon = (CBasePlayerWeapon *)(EDICT_FOR_NUM(Params[1])->pvPrivateData);
	GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextPrimaryAttack, 4)
		= GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextSecondaryAttack, 4) = CellToFloat(Params[2]);

	if (Params[3])
		GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_TimeWeaponIdle, 4)
		= GetPrivateData(float, BaseWeapon, CBasePlayerWeapon_NextPrimaryAttack, 4);

	return 0;
}

static cell AMX_NATIVE_CALL SetWeaponIdleAnim(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	CWeapon &Weapon = Weapons[Index];
	Weapon.AnimI = Params[2];
	Weapon.Flags |= WFlag::CustomIdleAnim;
	return 0;
}

#pragma warning (disable : 4701)

static cell AMX_NATIVE_CALL GetWeaponPathAddon(AMX *Plugin, cell *Params)
{
	MF_SetAmxString(Plugin, Params[1], PathAddon, Params[2]);
	return 0;
}

static cell AMX_NATIVE_CALL SetPlayerViewModel(AMX *Plugin, cell *Params)
{
	EDICT_FOR_NUM(Params[1])->v.viewmodel = (string_t)Params[2];
	return 0;
}

static cell AMX_NATIVE_CALL SetPlayerWeapModel(AMX *Plugin, cell *Params)
{
	EDICT_FOR_NUM(Params[1])->v.weaponmodel = (string_t)Params[2];
	return 0;
}

static cell AMX_NATIVE_CALL PrecacheWeaponModelEx(AMX *Plugin, cell *Params)
{
	string_t Model = ALLOC_STRING(MF_GetAmxString(Plugin, Params[1], 0, NULL));
	PRECACHE_MODEL(STRING(Model));
	return (cell)Model;
}

static int GetSequenceFlags(void *Model, entvars_t *EntVars)
{
	studiohdr_t *Studio = (studiohdr_t *)Model;

	if (!Studio || EntVars->sequence >= Studio->numseq)
		return 0;

	mstudioseqdesc_t *SequenceDesc = (mstudioseqdesc_t *)((byte *)Studio + Studio->seqindex) + EntVars->sequence;
	return SequenceDesc->flags;
}

static void GetSequenceInfo(void *Model, entvars_t *EntVars, float *FrameRate, float *GroundSpeed)
{
	studiohdr_t *Studio = (studiohdr_t *)Model;

	if (!Studio)
		return;

	if (EntVars->sequence >= Studio->numseq)
	{
		*FrameRate = 0;
		*GroundSpeed = 0;
		return;
	}

	mstudioseqdesc_t *SequenceDesc = (mstudioseqdesc_t *)((byte *)Studio + Studio->seqindex) + EntVars->sequence;

	if (SequenceDesc->numframes <= 1)
	{
		*FrameRate = 256.0f;
		*GroundSpeed = 0.0f;
		return;
	}

	*FrameRate = SequenceDesc->fps * 256.0f / (SequenceDesc->numframes - 1);
	*GroundSpeed = SequenceDesc->linearmovement.Length() * SequenceDesc->fps / (SequenceDesc->numframes - 1);
}

static void ResetSequenceInfo(CBasePlayer *BasePlayer)
{
	entvars_t *PlayerVars = EV_FROM_PD(BasePlayer);
	void *Model = GET_MODEL_PTR(ENT(PlayerVars));

	GetSequenceInfo(Model, PlayerVars,
		GetPrivateDataPointer(float, BasePlayer, CBaseAnimating_FrameRate, 5),
		GetPrivateDataPointer(float, BasePlayer, CBaseAnimating_GroundSpeed, 5));

	PlayerVars->animtime = gpGlobals->time;

	GetPrivateData(int, BasePlayer, CBaseAnimating_SequenceLoops, 5) = ((GetSequenceFlags(Model, PlayerVars) & STUDIO_LOOPING) != 0);
	GetPrivateData(int, BasePlayer, CBaseAnimating_SequenceFinished, 5) = FALSE;
	GetPrivateData(float, BasePlayer, CBaseAnimating_LastEventCheck, 5) = gpGlobals->time;
}

void SetAnimation(edict_t *PlayerEdict, int Animation, Activity ACT, float FrameRate)
{
	entvars_t *PlayerEntVars = &PlayerEdict->v;
	CBasePlayer *BasePlayer = (CBasePlayer *)PlayerEdict->pvPrivateData;

	GetPrivateData(float, BasePlayer, CBasePlayer_LastFired, 5) = gpGlobals->time;

	PlayerEntVars->frame = 0.0f;
	PlayerEntVars->framerate = FrameRate;
	PlayerEntVars->sequence = Animation;
	PlayerEntVars->animtime = gpGlobals->time;

	GetPrivateData(int, BasePlayer, CBaseMonster_Activity, 5) = ACT;
	GetPrivateData(int, BasePlayer, CBaseMonster_IdealActivity, 5) = ACT;
	GetPrivateData(float, BasePlayer, CBaseAnimating_LastEventCheck, 5) = gpGlobals->time;
	ResetSequenceInfo(BasePlayer);
	GetPrivateData(float, BasePlayer, CBaseAnimating_FrameRate, 5) *= FrameRate;
}

static cell AMX_NATIVE_CALL SetAnimation(AMX *Plugin, cell *Params)
{
	edict_t *PlayerEdict = EDICT_FOR_NUM(Params[1]);

	if (!PlayerEdict->pvPrivateData)
		return 0;

	SetAnimation(PlayerEdict, Params[2], (Activity)Params[3], CellToFloat(Params[4]));
	return 0;
}

static cell AMX_NATIVE_CALL GetWeaponDefaultDelay(AMX *Plugin, cell *Params)
{
	return FloatToCell(WEAPON_DEFAULT_DELAY[Params[1]]);
}

static cell AMX_NATIVE_CALL GetWeaponAnimDuration(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	return FloatToCell(Weapons[Index].DurationList.Get(Params[2]));
}

static cell AMX_NATIVE_CALL BuildWeaponModel(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	CWeapon &Weapon = Weapons[Index];
	const char *Model = MF_GetAmxString(Plugin, Params[3], 0, NULL);

	switch (Params[2])
	{
		case BUILD_VIEW: PRECACHE_MODEL(STRING(Weapon.VModel = ALLOC_STRING(Model))); break;
		case BUILD_WEAP: PRECACHE_MODEL(STRING(Weapon.PModel = ALLOC_STRING(Model))); break;
		case BUILD_WORLD: PRECACHE_MODEL(Weapon.WModel = STRING(ALLOC_STRING(Model))); break;
		case BUILD_LIST: PRECACHE_MODEL(Weapon.GModel = STRING(ALLOC_STRING(Model))); break;
	}

	return 0;
}

static cell AMX_NATIVE_CALL BuildWeaponModelEx(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	CWeapon &Weapon = Weapons[Index];
	string_t Model = Params[3];

	switch (Params[2])
	{
		case BUILD_VIEW: PRECACHE_MODEL(STRING(Weapon.VModel = Model)); break;
		case BUILD_WEAP: PRECACHE_MODEL(STRING(Weapon.PModel = Model)); break;
		case BUILD_WORLD: PRECACHE_MODEL(Weapon.WModel = STRING(Model)); break;
		case BUILD_LIST: PRECACHE_MODEL(Weapon.GModel = STRING(Model)); break;
	}

	return 0;
}

static cell AMX_NATIVE_CALL GetWeaponClip(AMX *Plugin, cell *Params)
{
	CBasePlayerWeapon *BaseWeapon = ((CBasePlayerWeapon *)EDICT_FOR_NUM(Params[1])->pvPrivateData);
	CBasePlayer *BasePlayer = GetPrivateData(CBasePlayer *, BaseWeapon, CBasePlayerItem_Player, 4);

	if (CUSTOM_WEAPON(BaseWeapon) && WEAPON_FLAGS(BaseWeapon) & WFlag::SoloClip)
		return GetPrivateDataEx(int, BasePlayer, CBasePlayer_Ammo, GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_PrimaryAmmoType, 4), 5);
	else
		return WEAPON_CLIP(BaseWeapon);
}

static cell AMX_NATIVE_CALL SetWeaponClip(AMX *Plugin, cell *Params)
{
	CBasePlayerWeapon *BaseWeapon = ((CBasePlayerWeapon *)EDICT_FOR_NUM(Params[1])->pvPrivateData);
	CBasePlayer *BasePlayer = GetPrivateData(CBasePlayer *, BaseWeapon, CBasePlayerItem_Player, 4);

	if (CUSTOM_WEAPON(BaseWeapon) && WEAPON_FLAGS(BaseWeapon) & WFlag::SoloClip)
		GetPrivateDataEx(int, BasePlayer, CBasePlayer_Ammo, GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_PrimaryAmmoType, 4), 5) = Params[2];
	else
		WEAPON_CLIP(BaseWeapon) = Params[2];

	return 0;
}

static cell AMX_NATIVE_CALL IsCustomWeaponEntity(AMX *Plugin, cell *Params)
{
	edict_t *WeaponEdict = EDICT_FOR_NUM(Params[1]);

	if (InvalidEntity(WeaponEdict))
		return FALSE;

	return CUSTOM_WEAPON(((CBasePlayerWeapon *)WeaponEdict->pvPrivateData));
}

static cell AMX_NATIVE_CALL GetWeaponEntityKey(AMX *Plugin, cell *Params)
{
	edict_t *WeaponEdict = EDICT_FOR_NUM(Params[1]);

	if (InvalidEntity(WeaponEdict))
		return -1;

	CBasePlayerWeapon *BaseWeapon = (CBasePlayerWeapon *)WeaponEdict->pvPrivateData;

	if (!CUSTOM_WEAPON(BaseWeapon))
		return -1;

	return WEAPON_KEY(BaseWeapon);
}

static cell AMX_NATIVE_CALL GetWeaponEntityData(AMX *Plugin, cell *Params)
{
	edict_t *WeaponEdict = EDICT_FOR_NUM(Params[1]);

	if (InvalidEntity(WeaponEdict))
		return 0;

	CBasePlayerWeapon *BaseWeapon = (CBasePlayerWeapon *)WeaponEdict->pvPrivateData;

	switch (Params[2])
	{
		case WEData::WED_Custom: return CUSTOM_WEAPON(BaseWeapon);
		case WEData::WED_FID: return WEAPON_FID(BaseWeapon);
		case WEData::WED_Key: return WEAPON_KEY(BaseWeapon);
		case WEData::WED_CurBurst: return WEAPON_CURBURST(BaseWeapon);
		case WEData::WED_A2: return WEAPON_A2(BaseWeapon);
		case WEData::WED_A2_Offset: return WEAPON_A2_OFFSET(BaseWeapon);
		case WEData::WED_INA2: return WEAPON_INA2(BaseWeapon);
		case WEData::WED_INA2_Delay: return WEAPON_INA2_DELAY(BaseWeapon);
		case WEData::WED_INBurst: return WEAPON_INBURST(BaseWeapon);
		case WEData::WED_Flags: return WEAPON_FLAGS(BaseWeapon);
	}

	return 0;
}

static cell AMX_NATIVE_CALL SetWeaponEntityData(AMX *Plugin, cell *Params)
{
	edict_t *WeaponEdict = EDICT_FOR_NUM(Params[1]);

	if (InvalidEntity(WeaponEdict))
		return 0;

	CBasePlayerWeapon *BaseWeapon = (CBasePlayerWeapon *)WeaponEdict->pvPrivateData;
	int Value = Params[3];

	switch (Params[2])
	{
		case WEData::WED_Custom: CUSTOM_WEAPON(BaseWeapon) = Value; break;
		case WEData::WED_FID: WEAPON_FID(BaseWeapon) = Value; break;
		case WEData::WED_Key: WEAPON_KEY(BaseWeapon) = Value; break;
		case WEData::WED_CurBurst: WEAPON_CURBURST(BaseWeapon) = Value; break;
		case WEData::WED_A2: WEAPON_A2(BaseWeapon) = Value; break;
		case WEData::WED_A2_Offset: WEAPON_A2_OFFSET(BaseWeapon) = Value; break;
		case WEData::WED_INA2: WEAPON_INA2(BaseWeapon) = Value; break;
		case WEData::WED_INA2_Delay: WEAPON_INA2_DELAY(BaseWeapon) = Value; break;
		case WEData::WED_INBurst: WEAPON_INBURST(BaseWeapon) = Value; break;
		case WEData::WED_Flags: WEAPON_FLAGS(BaseWeapon) = Value; break;
	}

	return 0;
}

static cell AMX_NATIVE_CALL SetWeaponWBody(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return 0;

	CWeapon &Weapon = Weapons[Index];
	Weapon.WBody = Params[2];
	return 0;
}

static cell AMX_NATIVE_CALL SetKnockbackInfo(AMX *Plugin, cell *Params)
{
	KnockbackInfo.PushPower = CellToFloat(Params[1]);
	KnockbackInfo.JumpPower = CellToFloat(Params[2]);
	return 0;
}

static cell AMX_NATIVE_CALL PlayerKnockback(AMX *Plugin, cell *Params)
{
	edict_t *PlayerEdict = EDICT_FOR_NUM(Params[1]);

	if (InvalidEntity(PlayerEdict))
		return 0;

	PlayerKnockback(PlayerEdict, GetVectorFromAddress(GetAMXAddr(Plugin, Params[2])));
	return 0;
}

static cell AMX_NATIVE_CALL RegisterTakeDamageForward(AMX *Plugin, cell *Params)
{
	char *Callback = MF_GetAmxString(Plugin, Params[1], 0, NULL);
	int Forward = MF_RegisterSPForwardByName(Plugin, Callback, FP_CELL, FP_CELL, FP_CELL, FP_FLOAT, FP_CELL, FP_DONE);
	TakeDamageFWs.Append(Forward);
	return 0;
}

AMX_NATIVE_INFO AMXX_NATIVES[] =
{
	{ "CreateWeapon", CreateWeapon },
	{ "CreateAmmo", CreateAmmo },
	{ "BuildWeaponModels", BuildWeaponModels },
	{ "BuildWeaponFireSound", BuildWeaponFireSound },
	{ "BuildWeaponList", BuildWeaponList },
	{ "BuildWeaponAmmunition", BuildWeaponAmmunition },
	{ "BuildWeaponDeploy", BuildWeaponDeploy },
	{ "BuildWeaponPrimaryAttack", BuildWeaponPrimaryAttack },
	{ "BuildWeaponReload", BuildWeaponReload },
	{ "BuildWeaponReloadShotgun", BuildWeaponReloadShotgun },
	{ "BuildWeaponFlags", BuildWeaponFlags },
	{ "BuildWeaponSecondaryAttack", BuildWeaponSecondaryAttack },
	{ "BuildWeaponMaxSpeed", BuildWeaponMaxSpeed },
	{ "BuildWeaponSpecs", BuildWeaponSpecs },
	{ "CheckWeaponParams", CheckWeaponParams },
	{ "RegisterWeaponForward", RegisterWeaponForward },
	{ "PrecacheWeaponModelSounds", PrecacheWeaponModelSounds },
	{ "PrecacheWeaponModelSoundsAsGenerics", PrecacheWeaponModelSoundsAsGenerics },
	{ "PrecacheWeaponListSprites", PrecacheWeaponListSprites },
	{ "FindWeaponByModel", FindWeaponByModel },
	{ "FindWeaponByName", FindWeaponByName },
	{ "GiveWeaponByModel", GiveWeaponByModel },
	{ "GiveWeaponByID", GiveWeaponByID },
	{ "GetWeaponData", GetWeaponData },
	{ "SendWeaponAnim", SendWeaponAnim },
	{ "CreateProjectile", CreateProjectile },
	{ "CreateEffect", CreateEffect },
	{ "ShootProjectileTimed", ShootProjectileTimed },
	{ "ShootProjectileContact", ShootProjectileContact },
	{ "ShootEffect", ShootEffect },
	{ "GiveAmmo", GiveAmmo },
	{ "SetAmmoName", SetAmmoName },
	{ "FindAmmoByName", FindAmmoByName },
	{ "GetWeaponCount", GetWeaponCount },
	{ "GetWeaponName", GetWeaponName },
	{ "GetWeaponModel", GetWeaponModel },
	{ "SetWeaponPathAddon", SetWeaponPathAddon },
	{ "GetWeaponFlags", GetWeaponFlags },
	{ "CreateExplosion", CreateExplosion },
	{ "RadiusDamageEx", RadiusDamageEx },
	{ "CleaveDamage", CleaveDamage },
	{ "CleaveDamageByPlayer", CleaveDamageByPlayer },
	{ "CanPrimaryAttack", CanPrimaryAttack },
	{ "SetNextAttack", SetNextAttack },
	{ "SetWeaponIdleAnim", SetWeaponIdleAnim },
	{ "GetWeaponPathAddon", GetWeaponPathAddon },
	{ "SetPlayerViewModel", SetPlayerViewModel },
	{ "SetPlayerWeapModel", SetPlayerWeapModel },
	{ "PrecacheWeaponModelEx", PrecacheWeaponModelEx },
	{ "SetAnimation", SetAnimation },
	{ "GetWeaponDefaultDelay", GetWeaponDefaultDelay },
	{ "GetWeaponAnimDuration", GetWeaponAnimDuration },
	{ "BuildWeaponModel", BuildWeaponModel },
	{ "BuildWeaponModelEx", BuildWeaponModelEx },
	{ "GetWeaponClip", GetWeaponClip },
	{ "SetWeaponClip", SetWeaponClip },
	{ "IsCustomWeaponEntity", IsCustomWeaponEntity },
	{ "GetWeaponEntityKey", GetWeaponEntityKey },
	{ "GetWeaponEntityData", GetWeaponEntityData },
	{ "SetWeaponEntityData", SetWeaponEntityData },
	{ "SetWeaponWBody", SetWeaponWBody },
	{ "SetKnockbackInfo", SetKnockbackInfo },
	{ "PlayerKnockback" , PlayerKnockback },
	{ "RegisterTakeDamageForward", RegisterTakeDamageForward },
	{ NULL, NULL },
};

BOOL INI_ReadQuotedString(char *Buffer)
{
	int Length = 0;

	while (*Buffer)
	{
		if (*Buffer == '"')
		{
			if (!Length)
				return FALSE;

			*Buffer = 0;
			return TRUE;
		}

		Length++;
		Buffer++;
	}

	return FALSE;
}

BOOL INI_ReadKeyValue(char *Buffer, char *&Key, char *&Value)
{
	LocateCharacter(Buffer);
	Key = Buffer++;
	ReadString(Buffer);

	if (*Buffer == '=')
		goto IgnoreLoop;

	*Buffer++ = 0;

	while (*Buffer < 33)
	{
		if (*Buffer == 0)
			return FALSE;

		Buffer++;
	}

	if (*Buffer != '=')
		return FALSE;

IgnoreLoop:
	*Buffer++ = 0;
	LocateCharacter(Buffer);

	if (*Buffer == '"')
	{
		Buffer++;

		if (!INI_ReadQuotedString(Buffer))
			return FALSE;

		Value = Buffer;
		return TRUE;
	}
	else
	{
		Value = Buffer;

		while (*Buffer)
		{
			if (*Buffer < 33)
			{
				*Buffer = 0;
				break;
			}

			Buffer++;
		}

		return TRUE;
	}
}

WType ReadWeaponType(char *Value)
{
	if (!stricmp(Value, "Pistol"))
		return WType::Pistol;
	else if (!stricmp(Value, "Shotgun"))
		return WType::Shotgun;
	else if (!strcmp(Value, "Sniper"))
		return WType::Sniper;

	return WType::Rifle;
}

int StrToInt(const char *Str)
{
	int Result = 0;

	while (*Str)
		Result = Result * 10 + (*Str++ - '0');

	return Result;
}
void ReadWeaponAttack2(CWeapon &Weapon, char *ValuePack)
{
	char *Value = strtok(ValuePack, ",");
	int IValue[16];
	float FValue[16];
	char *SValue[16];
	int ArgumentCount = 0;

	while (Value)
	{
		IValue[ArgumentCount] = StrToInt(Value);
		FValue[ArgumentCount] = (float)atof(Value);
		SValue[ArgumentCount] = Value;
		ArgumentCount++;
		Value = strtok(NULL, ",");
	}

	Weapon.A2I = IValue[0];

	if (Weapon.A2I < 1 || Weapon.A2I > 8)
	{
		if (*CVar_LogPointer)
			LOG_CONSOLE(PLID, "[CSWM] Invalid Attack2 Type: %i (%s)", Weapon.A2I, Weapon.Model);

		return;
	}

	Weapon.A2V = (Attack2 *)new int[WEAPON_A2_SIZE[Weapon.A2I]];

	switch (Weapon.A2I)
	{
		case A2_Zoom:
		{
			GetAttack2Data(int, Weapon.A2V, WA2_ZOOM_MODE) = IValue[1];
			break;
		}
		case A2_Switch:
		{
			GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_A) = IValue[1];
			GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_ANIM_A_DURATION) = FValue[2];
			GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_B) = IValue[3];
			GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_ANIM_B_DURATION) = FValue[4];
			GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_IDLE) = IValue[5];
			GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_DRAW) = IValue[6];
			GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_ANIM_DRAW_DURATION) = FValue[7];
			GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_SHOOT) = IValue[8];
			GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_ANIM_SHOOT_DURATION) = FValue[9];
			GetAttack2Data(int, Weapon.A2V, WA2_SWITCH_ANIM_RELOAD) = IValue[10];
			GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_ANIM_RELOAD_DURATION) = FValue[11];
			GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_DELAY) = FValue[12];
			GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_DAMAGE) = FValue[13];
			GetAttack2Data(float, Weapon.A2V, WA2_SWITCH_RECOIL) = FValue[14];
			PRECACHE_SOUND(GetAttack2Data(const char *, Weapon.A2V, WA2_SWITCH_FSOUND) = STRING(ALLOC_STRING(SValue[15])));
			break;
		}
		case A2_Burst:
		{
			GetAttack2Data(int, Weapon.A2V, WA2_BURST_VALUE) = IValue[1];
			break;
		}
		case A2_MultiShot:
		{
			GetAttack2Data(int, Weapon.A2V, WA2_MULTISHOT_VALUE) = IValue[1];
			break;
		}
		case A2_AutoPistol:
		{
			GetAttack2Data(int, Weapon.A2V, WA2_AUTOPISTOL_ANIM) = IValue[1];
			GetAttack2Data(float, Weapon.A2V, WA2_AUTOPISTOL_DELAY) = FValue[2];
			GetAttack2Data(float, Weapon.A2V, WA2_AUTOPISTOL_RECOIL) = FValue[3];
			break;
		}
		case A2_KnifeAttack:
		{
			GetAttack2Data(int, Weapon.A2V, WA2_KNIFEATTACK_ANIMATION) = IValue[1];
			GetAttack2Data(float, Weapon.A2V, WA2_KNIFEATTACK_DELAY) = FValue[2];
			GetAttack2Data(float, Weapon.A2V, WA2_KNIFEATTACK_DURATION) = FValue[3];
			GetAttack2Data(float, Weapon.A2V, WA2_KNIFEATTACK_RADIUS) = FValue[4];
			GetAttack2Data(int, Weapon.A2V, WA2_KNIFEATTACK_DAMAGE_MIN) = IValue[5];
			GetAttack2Data(int, Weapon.A2V, WA2_KNIFEATTACK_DAMAGE_MAX) = IValue[6];
			GetAttack2Data(float, Weapon.A2V, WA2_KNIFEATTACK_KNOCKBACK) = FValue[7];
			PRECACHE_SOUND(GetAttack2Data(const char *, Weapon.A2V, WA2_KNIFEATTACK_SOUND) = STRING(ALLOC_STRING(SValue[8])));
			break;
		}
		case A2_InstaSwitch:
		{
			GetAttack2Data(int, Weapon.A2V, WA2_INSTASWITCH_ANIM_SHOOT) = IValue[1];
			GetAttack2Data(float, Weapon.A2V, WA2_INSTASWITCH_DELAY) = FValue[2];
			GetAttack2Data(float, Weapon.A2V, WA2_INSTASWITCH_DAMAGE) = FValue[3];
			GetAttack2Data(float, Weapon.A2V, WA2_INSTASWITCH_RECOIL) = FValue[4];
			GetAttack2Data(const char *, Weapon.A2V, WA2_INSTASWITCH_NAME) = STRING(ALLOC_STRING(SValue[5]));
			GetAttack2Data(const char *, Weapon.A2V, WA2_INSTASWITCH_NAME2) = STRING(ALLOC_STRING(SValue[6]));
		}
		case A2_ZoomCustom:
		{
			GetAttack2Data(int, Weapon.A2V, WA2_ZOOM_CUSTOM_FOV) = IValue[1];
			break;
		}
	}
}

void ReadWeaponAmmo(CWeapon &Weapon, char *Value)
{
	for (int Index = 1; Index < AmmoCount; Index++)
	{
		const char *Name = Ammos[Index].Name;

		if (!Name)
			continue;

		if (!stricmp(Value, Name))
		{
			Weapon.AmmoID = (AmmoType)Index;
			break;
		}
	}
}

void ReadWeaponArray(List<int> *Array, char *Value)
{
	char *Anim = strtok(Value, ",");

	while (Anim != NULL)
	{
		Array->Append(StrToInt(Anim));
		Anim = strtok(NULL, " ,.-");
	}
}

AmmoType ReadWeaponAmmoID(char *Value)
{
	for (int Index = 0; Index < Ammos.Length; Index++)
	{
		if (!Ammos[Index].Name || strlen(Ammos[Index].Name) < 1)
			continue;

		if (!stricmp(Value, Ammos[Index].Name))
			return (AmmoType)Index;
	}

	return (AmmoType)0;
}

const CParam AmmoParams[] =
{
	{ "Cost", TYPE_INT, offsetof(CAmmo, Cost) },
	{ "Amount", TYPE_INT, offsetof(CAmmo, Amount) },
	{ "Max", TYPE_INT, offsetof(CAmmo, Max) },
	{ "Name", TYPE_STRING, offsetof(CAmmo, Name) },
};

const CParam WeaponParams[] =
{
	{ "Model", TYPE_STRING, offsetof(CWeapon, Model) },
	{ "Type", TYPE_OTHER1, 0 },
	{ "Name", TYPE_STRING, offsetof(CWeapon, Name) },
	{ "DeployAnim", TYPE_INT, offsetof(CWeapon, AnimD) },
	{ "DeployDuration", TYPE_FLOAT, offsetof(CWeapon, Reload) },
	{ "DeployTime", TYPE_FLOAT, offsetof(CWeapon, Deploy) },
	{ "ShootAnim", TYPE_ARRAY, offsetof(CWeapon, AnimS) },
	{ "ShootDuration", TYPE_FLOAT, offsetof(CWeapon, Delay) },
	{ "ShootTime", TYPE_FLOAT, offsetof(CWeapon, Delay) },
	{ "Delay", TYPE_FLOAT, offsetof(CWeapon, Delay) },
	{ "Damage", TYPE_FLOAT, offsetof(CWeapon, Damage) },
	{ "Recoil", TYPE_FLOAT, offsetof(CWeapon, Recoil) },
	{ "ReloadAnimation", TYPE_FLOAT, offsetof(CWeapon, AnimR) },
	{ "ReloadDuration", TYPE_FLOAT, offsetof(CWeapon, Reload) },
	{ "ReloadTime", TYPE_FLOAT, offsetof(CWeapon, Reload) },
	{ "ReloadType", TYPE_INT, offsetof(CWeapon, AnimR)  },
	{ "Clip", TYPE_INT, offsetof(CWeapon, Clip) },
	{ "AmmoID", TYPE_INT, offsetof(CWeapon, AmmoID) },
	{ "Ammo", TYPE_OTHER3, 0 },
	{ "Speed", TYPE_FLOAT, offsetof(CWeapon, Speed) },
	{ "VModel", TYPE_STRINT, offsetof(CWeapon, VModel) },
	{ "PModel", TYPE_STRINT, offsetof(CWeapon, PModel) },
	{ "WModel", TYPE_STRING, offsetof(CWeapon, WModel) },
	{ "ViewModel", TYPE_STRINT, offsetof(CWeapon, VModel) },
	{ "WeaponModel", TYPE_STRINT, offsetof(CWeapon, PModel) },
	{ "WorldModel", TYPE_STRING, offsetof(CWeapon, WModel) },
	{ "GModel", TYPE_STRING, offsetof(CWeapon, GModel) },
	{ "WeaponList", TYPE_STRING, offsetof(CWeapon, GModel) },
	{ "FireSound", TYPE_STRING, offsetof(CWeapon, FireSound) },
	{ "Attack2", TYPE_OTHER2, 0 },
	{ "WBody", TYPE_INT, offsetof(CWeapon, WBody) },
	{ "WorldModelBody", TYPE_INT, offsetof(CWeapon, WBody) },
};

void LoadAnimList(char *PathName)
{
	FILE *File = fopen(PathName, "rt");
	char Buffer[128];

	if (!File)
	{
		LOG_CONSOLE(PLID, "[CSWM] 'Anim.lst' File Not Found! (./CSWM/Anim.lst)");
		return;
	}

	while (!feof(File))
	{
		fgets(Buffer, sizeof(File), File);

		if (!Buffer[0])
			continue;

		Buffer[strlen(Buffer) - 1] = 0;
		AnimHashMap.Insert(Buffer, AnimHashMap.Length());
	}

	fclose(File);
}

void LoadConfig(char *PathName)
{
	FILE *File = fopen(PathName, "rt");

	if (!File)
	{
		LOG_CONSOLE(PLID, "[CSWM] 'Config.ini' File Not Found! (./CSWM/Config.ini)");
		LOG_CONSOLE(PLID, "[CSWM] Using Default Settings...");
		return;
	}

	char Buffer[260];

	while (!feof(File))
	{
		fgets(Buffer, sizeof(Buffer), File);

		if (!Buffer[0] || Buffer[0] == '#')
			continue;

		char *Key, *Value;

		if (!INI_ReadKeyValue(Buffer, Key, Value))
			continue;

		char *Dest = NULL;

		if (!strcmp(Key, "PathAddon"))
			Dest = PathAddon;
		else if (!strcmp(Key, "Trail"))
			Dest = SPR_Trail;
		else if (!strcmp(Key, "Explosion"))
			Dest = SPR_Explosion;
		else if (!strcmp(Key, "Smoke"))
			Dest = SPR_Smoke;
		else if (!strcmp(Key, "SmokePuff"))
			Dest = SPR_SmokePuff;
		else if (!strcmp(Key, "Ring"))
			Dest = SPR_Ring;

		if (!Dest)
		{
			if (*CVar_LogPointer)
				LOG_CONSOLE(PLID, "[CSWM] Unknown Key '%s'. (Config.ini)", Key);

			continue;
		}

		strcpy(Dest, Value);
	}

	fclose(File);
}

void LoadAmmo(char *Directory, char *FileName)
{
	char Buffer[260];
	sprintf(Buffer, "%s/%s", Directory, FileName);

	if (access(Buffer, 0) == -1)
		return;

	FILE *File = fopen(Buffer, "rt");
	
	if (!File)
		return;
	
	if (*CVar_LogPointer)
		LOG_CONSOLE(PLID, "[CSWM] Loading Ammo From File '%s'...", FileName);
	
	CAmmo Ammo;
	char *AmmoP = (char *)&Ammo;
	memset(AmmoP, 0, sizeof(CAmmo));
	
	while (!feof(File))
	{
		fgets(Buffer, sizeof(Buffer), File);
		
		if (!Buffer[0] || Buffer[0] == '#')
			continue;

		char *Key, *Value;

		if (!INI_ReadKeyValue(Buffer, Key, Value))
			continue;

		intptr_t Output;

		if (!AParamHashMap.Retrieve(Key, (int *)&Output))
		{
			if (*CVar_LogPointer)
				LOG_CONSOLE(PLID, "[CSWM] Unknown Parameter %s. (%s)", Key, FileName);

			continue;
		}

		CParam *Param = (CParam *)Output;

		if (Param->Type == TYPE_INT)
			*(int *)(AmmoP + Param->Offset) = StrToInt(Value);
		else 
			*(const char **)(AmmoP + Param->Offset) = STRING(ALLOC_STRING(Value));
	}

	CheckAmmo(Ammo, AmmoCount);
	Ammos.Append(Ammo);
	AmmoCount++;
	fclose(File);
}

void LoadWeapon(char *Directory, char *FileName)
{
	char Buffer[260];
	sprintf(Buffer, "%s/%s", Directory, FileName);

	if (access(Buffer, 0) == -1)
		return;

	FILE *File = fopen(Buffer, "rt");

	if (!File)
		return;

	if (*CVar_LogPointer)
		LOG_CONSOLE(PLID, "[CSWM] Loading Weapon From File '%s'...", FileName);

	CWeapon Weapon;
	char *WeaponP = (char *)&Weapon;
	memset(WeaponP, 0, sizeof(CWeapon));

	while (!feof(File))
	{
		fgets(Buffer, sizeof(Buffer), File);

		if (!Buffer[0] || Buffer[0] == '#')
			continue;

		char *Key, *Value;

		if (!INI_ReadKeyValue(Buffer, Key, Value))
			continue;

		intptr_t Output;

		if (!WParamHashMap.Retrieve(Key, (int *)&Output))
		{
			if (*CVar_LogPointer)
				LOG_CONSOLE(PLID, "[CSWM] Unknown Parameter %s. (%s)", Key, FileName);

			continue;
		}

		CParam *Param = (CParam *)Output;

		switch (Param->Type)
		{
			case TYPE_INT: *(int *)(WeaponP + Param->Offset) = StrToInt(Value); break;
			case TYPE_FLOAT: *(float *)(WeaponP + Param->Offset) = (float)atof(Value); break;
			case TYPE_STRING: *(const char **)(WeaponP + Param->Offset) = STRING(ALLOC_STRING(Value)); break;
			case TYPE_ARRAY: ReadWeaponArray((List<int> *)(WeaponP + Param->Offset, Value), Value); break;
			case TYPE_STRINT: *(string_t *)(WeaponP + Param->Offset) = ALLOC_STRING(Value); break;
			case TYPE_OTHER1: Weapon.Type = ReadWeaponType(Value); break;
			case TYPE_OTHER2: ReadWeaponAttack2(Weapon, Value); break;
			case TYPE_OTHER3: ReadWeaponAmmo(Weapon, Value);
		}
	}

	if (!Weapon.Model)
	{
		LOG_CONSOLE(PLID, "[CSWM] Found Weapon Without Model, Ignoring... (%s)", FileName);
		return;
	}

	CheckWeapon(Weapon);
	Weapons.Append(Weapon);
	WeaponCount++;
	fclose(File);
}

#if defined _WIN32
void LoadAmmos(void)
{
	char *PathName = (char *)MF_BuildPathname("%s/cswm/ammos/*", MF_GetLocalInfo("amxx_configsdir", "addons/amxmodx/configs"));
	HANDLE FileHandle;
	WIN32_FIND_DATA FindData;
	FileHandle = FindFirstFile(PathName, &FindData);
	PathName[strlen(PathName) - 2] = 0;

	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FindData.cFileName[0] == '.')
				continue;
			
			LoadAmmo(PathName, FindData.cFileName);
		} while (FindNextFile(FileHandle, &FindData));

		FindClose(FileHandle);
	}
}

void LoadWeapons(void)
{
	char *PathName = (char *)MF_BuildPathname("%s/cswm/weapons/*", MF_GetLocalInfo("amxx_configsdir", "addons/amxmodx/configs"));
	HANDLE FileHandle;
	WIN32_FIND_DATA FindData;
	FileHandle = FindFirstFile(PathName, &FindData);
	PathName[strlen(PathName) - 2] = 0;

	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FindData.cFileName[0] == '.')
				continue;
			
			LoadWeapon(PathName, FindData.cFileName);
		} while (FindNextFile(FileHandle, &FindData));

		FindClose(FileHandle);
	}
}
#else
#include <dirent.h>

void LoadAmmos(void)
{
	char *PathName = (char *)MF_BuildPathname("%s/cswm/ammos", MF_GetLocalInfo("amxx_configsdir", "addons/amxmodx/configs"));
	DIR *Directory;
	struct dirent *Ent;
	
	if ((Directory = opendir(PathName)))
	{
		while ((Ent = readdir(Directory)))
		{
			if (Ent->d_name[0] == '.')
				continue;
			
			LoadAmmo(PathName, Ent->d_name);
		}

		closedir(Directory);
	}
}

void LoadWeapons(void)
{
	char *PathName = (char *)MF_BuildPathname("%s/cswm/weapons", MF_GetLocalInfo("amxx_configsdir", "addons/amxmodx/configs"));
	DIR *Directory;
	struct dirent *Ent;

	if ((Directory = opendir(PathName)))
	{
		while ((Ent = readdir(Directory)))
		{
			if (Ent->d_name[0] == '.')
				continue;
			
			LoadWeapon(PathName, Ent->d_name);
		}
		
		closedir(Directory);
	}
}
#endif

void WeaponList()
{
	for (int Index = 0; Index < WeaponCount; Index++)
	{
		LOG_CONSOLE(PLID, "%i) %s", Index + 1, Weapons[Index].Model);
	}

	LOG_CONSOLE(PLID, "Total Weapons: %i", WeaponCount);
}

void OnAmxxAttach(void)
{
	MF_AddNatives(AMXX_NATIVES);
	CVar_LogPointer = &CVAR_GET_POINTER("developer")->value;
	const char *ConfigsDir = MF_BuildPathname("%s/cswm", MF_GetLocalInfo("amxx_configsdir", "addons/amxmodx/configs"));
	char PathName[MAX_PATH];
	sprintf(PathName, "%s/Anim.lst", ConfigsDir);
	LoadAnimList(PathName);
	sprintf(PathName, "%s/Config.ini", ConfigsDir);
	LoadConfig(PathName);

	for (int Index = 0; Index < (sizeof(AmmoParams) / sizeof(AmmoParams[0])); Index++)
		AParamHashMap.Insert(AmmoParams[Index].Name, (intptr_t)(AmmoParams + Index));
	
	for (int Index = 0; Index < (sizeof(WeaponParams) / sizeof(WeaponParams[0])); Index++)
		WParamHashMap.Insert(WeaponParams[Index].Name, (intptr_t)(WeaponParams + Index));

	UpdateAmmoList();
	ADD_SERVER_COMMAND("cswm_list", WeaponList);
}

