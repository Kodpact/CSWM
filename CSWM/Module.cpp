#include "Module.h"
#include <io.h>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define DEFINE_CHAR64(Value) char Value[64]

extern List<CWeapon> Weapons;
extern List<CAmmo> Ammos;
extern List<CProjectile> Projectiles;
extern List<CEffect> Effects;

extern StringHashMap AnimHashMap;

extern BOOL CanPrecache;
extern BOOL ClearWeapons;
extern BOOL SV_Cheats;
extern cvar_t *CVar_LogPointer;
extern cvar_t *CVar_AMapPointer;
extern cvar_t *CVar_AutoDetectAnimPointer;

char PathAddon[32] = "cswm/";

int WeaponCount = 0;
int AmmoCount = AMMO_MAX_TYPES;

const int WEAPON_TYPE_ID[] = { CSW_P228, CSW_XM1014, CSW_AK47, CSW_AWP };
const char WEAPON_TYPE_NAME[][8] = { "Pistol", "Shotgun", "Rifle", "Sniper" };
const float WEAPON_DEFAULT_DELAY[] = { 0.0f, 0.2f, 0.0f, 1.3f, 0.0f, 0.3f, 0.0f, 0.1f, 0.1f, 0.0f, 0.1f, 0.2f, 0.1f, 0.3f, 0.1f, 0.1f, 0.1f, 0.2f, 1.5f, 0.1f, 0.1f, 0.9f, 0.1f, 0.1f, 0.3f, 0.0f, 0.2f, 0.1f, 0.1f, 0.0f, 0.1f };
const int WEAPON_A2_SIZE[] = { 0, 4, 60, 4, 4, 12, 32, 0, 0, 24 };
const float WEAPON_SPEED[] = { 250.0f, 240.0f, 221.0f, 210.0f };

DEFINE_CHAR64(SPR_Trail) = "sprites/laserbeam.spr";
DEFINE_CHAR64(SPR_Explosion) = "sprites/fexplo.spr";
DEFINE_CHAR64(SPR_Smoke) = "sprites/steam1.spr";
DEFINE_CHAR64(SPR_SmokePuff) = "sprites/wall_puff5.spr";
DEFINE_CHAR64(SPR_Ring) = "sprites/shockwave.spr";

int MI_Trail, MI_Explosion, MI_Smoke;
extern int MI_SmokePuff, MI_Ring;

static TraceResult TResult;
static CCleaveDamageInfo CleaveDamageInfo;
static StringHashMap ParamHashMap;

void PrecacheModule(void)
{
	MI_Trail = PRECACHE_MODEL(SPR_Trail);
	MI_Explosion = PRECACHE_MODEL(SPR_Explosion);
	MI_Smoke = PRECACHE_MODEL(SPR_Smoke);
	MI_SmokePuff = PRECACHE_MODEL(SPR_SmokePuff);
	MI_Ring = PRECACHE_MODEL(SPR_Ring);
}

void RecordWeaponDurationList(CWeapon &Weapon)
{
	edict_t *InfoEdict = CREATE_ENTITY();
	SET_MODEL(InfoEdict, STRING(Weapon.VModel));

	studiohdr_t *Studio = (studiohdr_t *)GET_MODEL_PTR(InfoEdict);

	for (int Index = 0; Index < Studio->numseq; Index++)
	{
		mstudioseqdesc_t *Sequence = (mstudioseqdesc_t *)((byte *)Studio + Studio->seqindex) + Index;
		Weapon.DurationList.Append(Sequence->numframes / Sequence->fps);
	}

	REMOVE_ENTITY(InfoEdict);
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

void DetectAnimation(CWeapon &Weapon, int Type)
{
	edict_t *InfoEdict = CREATE_ENTITY();
	SET_MODEL(InfoEdict, STRING(Weapon.VModel));

	studiohdr_t *Studio = (studiohdr_t *)GET_MODEL_PTR(InfoEdict);
	int StartIndex = GetLabelStartIndex(Studio);

	for (int Index = 0; Index < Studio->numseq; Index++)
	{
		mstudioseqdesc_t *Sequence = (mstudioseqdesc_t *)((byte *)Studio + Studio->seqindex) + Index;

		if ((int)strlen(Sequence->label) <= StartIndex)
			break;

		char *Start = Sequence->label + StartIndex;

		switch (Type)
		{
			case DETECT_DRAW: if (!strncmp(Start, "draw", 4)) Weapon.AnimD = Index; break;
			case DETECT_SHOOT: if (!strncmp(Start, "shoot", 5)) Weapon.AnimS.Append(Index); break;
			case DETECT_RELOAD: if (!strncmp(Start, "reload", 6)) Weapon.AnimR = Index; break;
		}
	}

	REMOVE_ENTITY(InfoEdict);
}

Vector &GetVectorFromAddress(cell *Addr)
{
	static Vector Output;
	Output.x = CellToFloat(Addr[0]);
	Output.y = CellToFloat(Addr[1]);
	Output.z = CellToFloat(Addr[2]);
	return Output;
}

cell AMX_NATIVE_CALL CreateWeapon(AMX *Plugin, cell *Params)
{
	if (!CanPrecache)
	{
		LOG_CONSOLE(PLID, "[CSWM] Can Not Register Weapon At This Time.");
		return NULL;
	}

	CWeapon Weapon;
	memset(&Weapon, NULL, sizeof(CWeapon));

	Weapon.Model = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[1], NULL, NULL)));
	Weapon.Name = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[3], NULL, NULL)));
	WType Type = (WType)Params[2];
	Weapon.Type = Type;
	Weapon.Speed = 250.0f;

	if (CVar_LogPointer->value)
		LOG_CONSOLE(PLID, "[CSWM] Creating Weapon: %s (Type=%s)", Weapon.Name, WEAPON_TYPE_NAME[Weapon.Type]);

	Weapons.Append(Weapon);
	WeaponCount++;
	return WeaponCount - 1;
}

cell AMX_NATIVE_CALL CreateAmmo(AMX *Plugin, cell *Params)
{
	if (!Ammos.Length)
		UpdateAmmoList();

	CAmmo Ammo;
	Ammo.Cost = Params[1];
	Ammo.Amount = Params[2];
	Ammo.Max = Params[3];
	Ammos.Append(Ammo);

	AmmoCount++;
	return AmmoCount - 1;
}

cell AMX_NATIVE_CALL BuildWeaponModels(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	CWeapon &Weapon = Weapons[Index];

	PRECACHE_MODEL(STRING(Weapon.VModel = ALLOC_STRING(MF_GetAmxString(Plugin, Params[2], NULL, NULL))));
	PRECACHE_MODEL(STRING(Weapon.PModel = ALLOC_STRING(MF_GetAmxString(Plugin, Params[3], NULL, NULL))));
	PRECACHE_MODEL(Weapon.WModel = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[4], NULL, NULL))));
	RecordWeaponDurationList(Weapon);
	return NULL;
}

cell AMX_NATIVE_CALL BuildWeaponFireSound(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	CWeapon &Weapon = Weapons[Index];

	char *String = MF_GetAmxString(Plugin, Params[2], NULL, NULL);
	Weapon.FireSound = STRING(ALLOC_STRING(String));
	PRECACHE_SOUND(Weapon.FireSound);
	return NULL;
}

cell AMX_NATIVE_CALL BuildWeaponList(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	CWeapon &Weapon = Weapons[Index];

	Weapon.GModel = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[2], NULL, NULL)));

	if (CVar_LogPointer->value)
		LOG_CONSOLE(PLID, "[CSWM] Building Weapon List '%s' For '%s'.", Weapon.GModel, Weapon.Name);

	char Path[MAX_PATH];
	sprintf(Path, "sprites/%s.txt", Weapon.GModel);
	PRECACHE_GENERIC(STRING(ALLOC_STRING(Path)));
	return NULL;
}

cell AMX_NATIVE_CALL BuildWeaponAmmunition(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	CWeapon &Weapon = Weapons[Index];

	if (CVar_LogPointer->value)
		LOG_CONSOLE(PLID, "[CSWM] Building Weapon Ammunition For '%s'.", Weapon.Name);

	Weapon.Clip = Params[2];
	Weapon.AmmoID = (AmmoType)Params[3];
	return NULL;
}

cell AMX_NATIVE_CALL BuildWeaponDeploy(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	CWeapon &Weapon = Weapons[Index];

	if (CVar_LogPointer->value)
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

	return NULL;
}

cell AMX_NATIVE_CALL BuildWeaponPrimaryAttack(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	CWeapon &Weapon = Weapons[Index];

	if (CVar_LogPointer->value)
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

	if (Weapon.AnimS.Length < 1)
	{
		LOG_CONSOLE(PLID, "[CSWM] WARNING: Weapon Has Not Primary Attack Animation.");
		Weapon.AnimS.Append(0);
	}

	return NULL;
}

cell AMX_NATIVE_CALL BuildWeaponReload(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	CWeapon &Weapon = Weapons[Index];

	if (CVar_LogPointer->value)
		LOG_CONSOLE(PLID, "[CSWM] Building Weapon Reload(A) For '%s'.", Weapon.Name);

	float Duration = CellToFloat(Params[3]);

	if (Params[2])
		Weapon.AnimR = Params[2];
	else
		DetectAnimation(Weapon, DETECT_RELOAD);

	if (Duration > 0.0f)
		Weapon.Reload = Duration;
	else
		Weapon.Reload = Weapon.DurationList[Weapon.AnimR];

	return NULL;
}

cell AMX_NATIVE_CALL BuildWeaponReloadShotgun(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	CWeapon &Weapon = Weapons[Index];

	if (CVar_LogPointer->value)
		LOG_CONSOLE(PLID, "[CSWM] Building Weapon Reload(B) For '%s'.", Weapon.Name);

	Weapon.Reload = CellToFloat(Params[2]);
	Weapon.AnimR = Params[3];
	return NULL;
}

cell AMX_NATIVE_CALL BuildWeaponFlags(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	Weapons[Index].Flags = Params[2];
	return NULL;
}

cell AMX_NATIVE_CALL BuildWeaponSecondaryAttack(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	CWeapon &Weapon = Weapons[Index];

	if (CVar_LogPointer->value)
		LOG_CONSOLE(PLID, "[CSWM] Building Weapon Secondary Attack For '%s'.", Weapon.Name);

	Weapon.A2V = (A2V *)new int[WEAPON_A2_SIZE[Params[2]]];

	switch (Weapon.A2I = Params[2])
	{
		case A2_Zoom:
		{
			Weapon.A2V->WA2_ZOOM_MODE = *MF_GetAmxAddr(Plugin, Params[3]);
			break;
		}
		case A2_Switch:
		{
			Weapon.A2V->WA2_SWITCH_ANIM_A = *GetAMXAddr(Plugin, Params[3]);
			Weapon.A2V->WA2_SWITCH_ANIM_A_DURATION = CellToFloat(*GetAMXAddr(Plugin, Params[4]));
			Weapon.A2V->WA2_SWITCH_ANIM_B = *GetAMXAddr(Plugin, Params[5]);
			Weapon.A2V->WA2_SWITCH_ANIM_B_DURATION = CellToFloat(*GetAMXAddr(Plugin, Params[6]));
			Weapon.A2V->WA2_SWITCH_ANIM_IDLE = *GetAMXAddr(Plugin, Params[7]);
			Weapon.A2V->WA2_SWITCH_ANIM_DRAW = *GetAMXAddr(Plugin, Params[8]);
			Weapon.A2V->WA2_SWITCH_ANIM_DRAW_DURATION = CellToFloat(*GetAMXAddr(Plugin, Params[9]));
			Weapon.A2V->WA2_SWITCH_ANIM_SHOOT = *GetAMXAddr(Plugin, Params[10]);
			Weapon.A2V->WA2_SWITCH_ANIM_SHOOT_DURATION = CellToFloat(*GetAMXAddr(Plugin, Params[11]));
			Weapon.A2V->WA2_SWITCH_ANIM_RELOAD = *GetAMXAddr(Plugin, Params[12]);
			Weapon.A2V->WA2_SWITCH_ANIM_RELOAD_DURATION = CellToFloat(*GetAMXAddr(Plugin, Params[13]));
			Weapon.A2V->WA2_SWITCH_DELAY = CellToFloat(*GetAMXAddr(Plugin, Params[14]));
			Weapon.A2V->WA2_SWITCH_DAMAGE = CellToFloat(*GetAMXAddr(Plugin, Params[15]));
			Weapon.A2V->WA2_SWITCH_RECOIL = CellToFloat(*GetAMXAddr(Plugin, Params[16]));
			PRECACHE_SOUND(Weapon.A2V->WA2_SWITCH_FSOUND = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[17], NULL, NULL))));
			break;
		}
		case A2_Burst:
		{
			Weapon.A2V->WA2_BURST_VALUE = *GetAMXAddr(Plugin, Params[3]);
			break;
		}
		case A2_MultiShot:
		{
			Weapon.A2V->WA2_MULTISHOT_VALUE = *GetAMXAddr(Plugin, Params[3]);
			break;
		}
		case A2_AutoPistol:
		{
			Weapon.A2V->WA2_AUTOPISTOL_ANIM = *GetAMXAddr(Plugin, Params[3]);
			Weapon.A2V->WA2_AUTOPISTOL_DELAY = CellToFloat(*GetAMXAddr(Plugin, Params[4]));
			Weapon.A2V->WA2_AUTOPISTOL_RECOIL = CellToFloat(*GetAMXAddr(Plugin, Params[5]));
			break;
		}
		case A2_KnifeAttack:
		{
			Weapon.A2V->WA2_KNIFEATTACK_ANIMATION = *GetAMXAddr(Plugin, Params[3]);
			Weapon.A2V->WA2_KNIFEATTACK_DELAY = CellToFloat(*GetAMXAddr(Plugin, Params[4]));
			Weapon.A2V->WA2_KNIFEATTACK_DURATION = CellToFloat(*GetAMXAddr(Plugin, Params[5]));
			Weapon.A2V->WA2_KNIFEATTACK_RADIUS = CellToFloat(*GetAMXAddr(Plugin, Params[6]));
			Weapon.A2V->WA2_KNIFEATTACK_DAMAGE_MIN = *GetAMXAddr(Plugin, Params[7]);
			Weapon.A2V->WA2_KNIFEATTACK_DAMAGE_MAX = *GetAMXAddr(Plugin, Params[8]);
			Weapon.A2V->WA2_KNIFEATTACK_KNOCKBACK = CellToFloat(*GetAMXAddr(Plugin, Params[9]));
			PRECACHE_SOUND(Weapon.A2V->WA2_KNIFEATTACK_SOUND = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[10], NULL, NULL))));
			break;
		}
		case A2_InstaSwitch:
		{
			Weapon.A2V->WA2_INSTASWITCH_ANIM_SHOOT = *GetAMXAddr(Plugin, Params[3]);
			Weapon.A2V->WA2_INSTASWITCH_DELAY = CellToFloat(*GetAMXAddr(Plugin, Params[4]));
			Weapon.A2V->WA2_INSTASWITCH_DAMAGE = CellToFloat(*GetAMXAddr(Plugin, Params[5]));
			Weapon.A2V->WA2_INSTASWITCH_RECOIL = CellToFloat(*GetAMXAddr(Plugin, Params[6]));
			Weapon.A2V->WA2_INSTASWITCH_NAME = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[7], NULL, NULL)));
			Weapon.A2V->WA2_INSTASWITCH_NAME2 = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[8], NULL, NULL)));
		}
	}

	return NULL;
}

cell AMX_NATIVE_CALL BuildWeaponMaxSpeed(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	Weapons[Index].Speed = CellToFloat(Params[2]);
	return NULL;
}

cell AMX_NATIVE_CALL RegisterWeaponForward(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	CWeapon &Weapon = Weapons[Index];

	if (CVar_LogPointer->value)
		LOG_CONSOLE(PLID, "[CSWM] Registering Weapon Forward For '%s'.", Weapon.Name);

	char *String = MF_GetAmxString(Plugin, Params[3], NULL, NULL);

	int Forward = Params[2];

	if (Forward == WForward::DamagePost)
		Weapon.Forwards[Forward] = MF_RegisterSPForwardByName(Plugin, String, FP_CELL, FP_FLOAT, FP_CELL, FP_DONE);
	else if (Forward == WForward::DropPost)
		Weapon.Forwards[Forward] = MF_RegisterSPForwardByName(Plugin, String, FP_CELL, FP_CELL, FP_DONE);
	else
		Weapon.Forwards[Forward] = MF_RegisterSPForwardByName(Plugin, String, FP_CELL, FP_DONE);

	return NULL;
}

cell AMX_NATIVE_CALL PrecacheWeaponModel(AMX *Plugin, cell *Params)
{
	for (size_t Index = 1; Index <= Params[0] / sizeof(cell); Index++)
		PRECACHE_MODEL(STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[Index], NULL, NULL))));

	return NULL;
}

cell AMX_NATIVE_CALL PrecacheWeaponSound(AMX *Plugin, cell *Params)
{
	for (size_t Index = 1; Index <= Params[0] / sizeof(cell); Index++)
		PRECACHE_SOUND(STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[Index], NULL, NULL))));

	return NULL;
}

cell AMX_NATIVE_CALL PrecacheWeaponGeneric(AMX *Plugin, cell *Params)
{
	for (int Index = 1; Index <= Params[0] / (signed)sizeof(cell); Index++)
		PRECACHE_GENERIC(STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[Index], NULL, NULL))));

	return NULL;
}

cell AMX_NATIVE_CALL FindWeaponByName(AMX *Plugin, cell *Params)
{
	char *Name = MF_GetAmxString(Plugin, Params[1], NULL, NULL);

	for (int Index = NULL; Index < WeaponCount; Index++)
	{
		if (!strcmp(Name, STRING(Weapons[Index].Model)))
			return Index;
	}

	return -1;
}

static cell AMX_NATIVE_CALL GiveWeaponByName(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index > gpGlobals->maxClients)
		return FALSE;

	GiveWeaponByName(EDICT_FOR_NUM(Index), (char *)MF_GetAmxString(Plugin, Params[1], NULL, NULL));
	return TRUE;
}

static cell AMX_NATIVE_CALL GiveWeaponByID(AMX *Plugin, cell *Params)
{
	cell PIndex = Params[1];
	cell WIndex = Params[2];

	if (PIndex < 0 || PIndex > gpGlobals->maxClients)
		return FALSE;

	if (WIndex < 0 || WIndex >= WeaponCount)
		return FALSE;

	GiveWeapon(EDICT_FOR_NUM(PIndex), WIndex);
	return TRUE;
}

#pragma warning (disable : 4701)

static cell AMX_NATIVE_CALL GetWeaponData(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

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
	}

	if (!Data)
		return NULL;

	cell Size = 0;

	if (Type > TYPE_FLOAT)
		Size = *GetAMXAddr(Plugin, Params[4]);

	switch (Type)
	{
		case TYPE_INT:
		{
			return *(int *)Data;
		}
		case TYPE_FLOAT:
		{
			return FloatToCell(*(float *)Data);
		}
		case TYPE_STRING:
		{
			MF_SetAmxString(Plugin, Params[3], (const char *)Data, Size);
			break;
		}
		case TYPE_ARRAY:
		{
			Size = min(Size, ((List<int> *)Data)->Length);

			for (Index = 0; Index < Size; Index++)
			{
				*(GetAMXAddr(Plugin, Params[3]) + Index) = ((List<int> *)Data)->Data[Index];
			};
		}
	}

	return NULL;
}

static cell AMX_NATIVE_CALL SendWeaponAnim(AMX *Plugin, cell *Params)
{
	edict_t *WeaponEdict = EDICT_FOR_NUM(Params[1]);

	if (!WeaponEdict)
		return NULL;

	CBasePlayerWeapon *BaseWeapon = (CBasePlayerWeapon *)WeaponEdict->pvPrivateData;

	if (!BaseWeapon)
		return NULL;

	SendWeaponAnim(BaseWeapon, Params[2]);
	return NULL;
}

cell AMX_NATIVE_CALL CreateProjectile(AMX *Plugin, cell *Params)
{
	CProjectile Projectile;
	memset(&Projectile, NULL, sizeof(CProjectile));
	PRECACHE_MODEL(Projectile.Model = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[1], NULL, NULL))));
	Projectile.Gravity = CellToFloat(Params[2]);
	Projectile.Speed = CellToFloat(Params[3]);
	Projectile.Forward = MF_RegisterSPForwardByName(Plugin, MF_GetAmxString(Plugin, Params[4], NULL, NULL), FP_CELL, FP_DONE);

	if (Params[0] / sizeof(cell) > 4)
		Projectile.Duration = CellToFloat(*GetAMXAddr(Plugin, Params[5]));

	Projectiles.Append(Projectile);
	return Projectiles.Length - 1;
}

cell AMX_NATIVE_CALL CreateEffect(AMX *Plugin, cell *Params)
{
	CEffect Effect;
	memset(&Effect, NULL, sizeof(CEffect));
	PRECACHE_MODEL(Effect.Model = STRING(Effect.ModelIndex = ALLOC_STRING(MF_GetAmxString(Plugin, Params[1], NULL, NULL))));
	Effect.Speed = CellToFloat(Params[2]);
	Effect.Forward = MF_RegisterSPForwardByName(Plugin, MF_GetAmxString(Plugin, Params[3], NULL, NULL), FP_CELL, FP_DONE);

	if (Params[0] / sizeof(cell) > 3)
		Effect.Duration = CellToFloat(*GetAMXAddr(Plugin, Params[4]));

	Effects.Append(Effect);
	return Effects.Length - 1;
}

static cell AMX_NATIVE_CALL ShootProjectileTimed(AMX *Plugin, cell *Params)
{
	edict_t *LauncherEdict = EDICT_FOR_NUM(Params[1]);

	if (InvalidEntity(LauncherEdict))
		return NULL;
	
	return ShootProjectileTimed(LauncherEdict, Params[2]);
}

static cell AMX_NATIVE_CALL ShootProjectileContact(AMX *Plugin, cell *Params)
{
	edict_t *LauncherEdict = EDICT_FOR_NUM(Params[1]);

	if (InvalidEntity(LauncherEdict))
		return NULL;
	
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
		return NULL;

	Player_GiveAmmoByID((CBasePlayer *)PlayerEdict->pvPrivateData, Params[2], Params[3]);
	return NULL;
}

cell AMX_NATIVE_CALL SetAmmoName(AMX *Plugin, cell *Params)
{
	int AmmoIndex = Params[1];

	if (AmmoIndex < 0 || AmmoIndex >= AmmoCount)
		return NULL;

	Ammos[AmmoIndex].Name = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[2], NULL, NULL)));
	return NULL;
}

cell AMX_NATIVE_CALL FindAmmoByName(AMX *Plugin, cell *Params)
{
	char *SearchAmmoName = MF_GetAmxString(Plugin, Params[1], NULL, NULL);
	const char *AmmoName;

	for (int Index = NULL; Index < Ammos.Length; Index++)
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

	return NULL;
}

cell AMX_NATIVE_CALL GetWeaponCount(AMX *Plugin, cell *Params)
{
	return WeaponCount;
}

cell AMX_NATIVE_CALL GetWeaponName(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	MF_SetAmxString(Plugin, Params[2], Weapons[Index].Name, Params[3]);
	return NULL;
}

cell AMX_NATIVE_CALL SetWeaponPathAddon(AMX *Plugin, cell *Params)
{
	strcpy_s(PathAddon, MF_GetAmxString(Plugin, Params[1], NULL, NULL));
	return NULL;
}

cell AMX_NATIVE_CALL GetWeaponFlags(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

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

	return NULL;
}

static cell AMX_NATIVE_CALL RadiusDamageEx(AMX *Plugin, cell *Params)
{
	Vector &Origin = GetVectorFromAddress(GetAMXAddr(Plugin, Params[1]));
	edict_t *InvokerEdict = EDICT_FOR_NUM(Params[2]);
	edict_t *OwnerEdict;
	edict_t *TargetEdict = SVGame_Edicts;
	CBaseEntity *BaseEntity;
	Vector TargetOrigin;
	entvars_t *InflictorVars, *AttackerVars;
	int CastTeam;
	int DamageBits;

	if (InvokerEdict->v.flags & FL_CLIENT)
	{
		InflictorVars = &InvokerEdict->v;
		AttackerVars = &InvokerEdict->v;
		OwnerEdict = InvokerEdict;
		CastTeam = GetEntityTeam(InvokerEdict);
		DamageBits = DMG_SLASH;
	}
	else
	{
		InflictorVars = &InvokerEdict->v;
		AttackerVars = &InvokerEdict->v.owner->v;
		OwnerEdict = InvokerEdict->v.owner;
		CastTeam = ((CGrenade *)InvokerEdict->pvPrivateData)->m_iTeam;
		DamageBits = DMG_EXPLOSION;
	}

	float Radius = CellToFloat(Params[3]);
	float Damage = CellToFloat(Params[4]);
	int Flags = Params[5];
	BOOL Result;

	while ((TargetEdict = FIND_ENTITY_IN_SPHERE(TargetEdict, Origin, Radius)) != SVGame_Edicts)
	{
		if (!TargetEdict->pvPrivateData)
			continue;

		if (!TargetEdict->v.takedamage)
			continue;

		BaseEntity = (CBaseEntity *)TargetEdict->pvPrivateData;

		TargetOrigin = TargetEdict->v.origin;

		if (BaseEntity->IsBSPModel())
			TargetOrigin = TargetOrigin + (TargetEdict->v.mins + TargetEdict->v.maxs) * 0.5f;

		if (!(Flags & RDFlag::Penetration))
		{
			TRACE_LINE(Origin, TargetOrigin, 0, 0, &TResult);

			if (TResult.pHit != TargetEdict && TResult.flFraction < 0.9f)
				continue;
		}

		if ((TargetEdict == OwnerEdict) && (Flags & RDFlag::NoSelfDamage))
			continue;

		Result = BaseEntity->TakeDamage(InflictorVars, AttackerVars, (TargetEdict == OwnerEdict ? 0.4f : 1.0f) * ((Damage * Radius) / (TargetOrigin - Origin).Length()), DMG_EXPLOSION);

		if (Result & (Flags & RDFlag::Knockback) && BaseEntity->IsPlayer())
			PlayerKnockback(TargetEdict, Origin, RANDOM_FLOAT(120.0f, 150.0f));
	}

	return NULL;
}

void PerformCleaveDamage()
{
	edict_t *TargetEdict = SVGame_Edicts;
	CBaseEntity *BaseEntity;
	Vector TargetOrigin;

	while ((TargetEdict = FIND_ENTITY_IN_SPHERE(TargetEdict, CleaveDamageInfo.Origin, CleaveDamageInfo.Radius)) != SVGame_Edicts)
	{
		if (!TargetEdict->pvPrivateData)
			continue;

		if (!TargetEdict->v.takedamage)
			continue;

		BaseEntity = (CBaseEntity *)TargetEdict->pvPrivateData;
		TargetOrigin = TargetEdict->v.origin;

		if (BaseEntity->IsBSPModel())
			TargetOrigin = TargetOrigin + (TargetEdict->v.mins + TargetEdict->v.maxs) * 0.5f;

		if (!InViewCone(CleaveDamageInfo.Origin, CleaveDamageInfo.VAngles, CleaveDamageInfo.FOV, TargetOrigin, CleaveDamageInfo.Accurate))
			continue;

		BaseEntity->TakeDamage(CleaveDamageInfo.Inflictor, CleaveDamageInfo.Attacker, ((CleaveDamageInfo.Damage *CleaveDamageInfo.Radius) / (TargetOrigin - CleaveDamageInfo.Origin).Length()), CleaveDamageInfo.DamageType);
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
	return NULL;
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
	return NULL;
}

static cell AMX_NATIVE_CALL CanPrimaryAttack(AMX *Plugin, cell *Params)
{
	CBasePlayerWeapon *BaseWeapon = (CBasePlayerWeapon *)(EDICT_FOR_NUM(Params[1])->pvPrivateData);
	return ((BaseWeapon->m_flNextPrimaryAttack <= 0.0f) && (BaseWeapon->m_pPlayer->m_flNextAttack <= 0.0f)) ? TRUE : FALSE;
}

static cell AMX_NATIVE_CALL SetNextAttack(AMX *Plugin, cell *Params)
{
	CBasePlayerWeapon *BaseWeapon = (CBasePlayerWeapon *)(EDICT_FOR_NUM(Params[1])->pvPrivateData);
	BaseWeapon->m_flNextPrimaryAttack = BaseWeapon->m_flNextSecondaryAttack = CellToFloat(Params[2]);

	if (Params[3])
		BaseWeapon->m_flTimeWeaponIdle = BaseWeapon->m_flNextPrimaryAttack;

	return NULL;
}

static cell AMX_NATIVE_CALL SetCustomIdleAnim(AMX *Plugin, cell *Params)
{
	((CBasePlayerWeapon *)EDICT_FOR_NUM(Params[1])->pvPrivateData)->m_iFamasShotsFired = Params[2];
	return NULL;
}

#pragma warning (disable : 4701)

cell AMX_NATIVE_CALL GetWeaponPath(AMX *Plugin, cell *Params)
{
	char *Model = (char *)STRING(Weapons[Params[1]].VModel);
	int Length = strlen(Model);
	int Index;
	char OldChar;
	BOOL Format = FALSE;

	for (Index = Length - 1; Index >= 0; Index--)
	{
		if (Model[Index] == '/' || Model[Index] == '\\')
		{
			OldChar = Model[Index];
			Model[Index] = NULL;
			Format = TRUE;
			break;
		}
	}

	if (!Format)
		Model = "";

	MF_SetAmxString(Plugin, Params[2], Model, Params[3]);

	if (Format)
		Model[Index] = OldChar;

	return NULL;
}

cell AMX_NATIVE_CALL GetDefaultPath(AMX *Plugin, cell *Params)
{
	char Path[64];
	sprintf(Path, "models/%s", PathAddon);
	MF_SetAmxString(Plugin, Params[1], Path, Params[2]);
	return NULL;
}

static cell AMX_NATIVE_CALL SetPlayerViewModel(AMX *Plugin, cell *Params)
{
	EDICT_FOR_NUM(Params[1])->v.viewmodel = (string_t)Params[2];
	return NULL;
}

static cell AMX_NATIVE_CALL SetPlayerWeapModel(AMX *Plugin, cell *Params)
{
	EDICT_FOR_NUM(Params[1])->v.weaponmodel = (string_t)Params[2];
	return NULL;
}

cell AMX_NATIVE_CALL PrecacheWeaponModelEx(AMX *Plugin, cell *Params)
{
	string_t Model = ALLOC_STRING(MF_GetAmxString(Plugin, Params[1], NULL, nullptr));
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

static void GetSequenceInfo(void *Model, entvars_t *EntVars, float *flFrameRate, float *flGroundSpeed)
{
	studiohdr_t *Studio = (studiohdr_t *)Model;

	if (!Studio)
		return;

	if (EntVars->sequence >= Studio->numseq)
	{
		*flFrameRate = 0;
		*flGroundSpeed = 0;
		return;
	}

	mstudioseqdesc_t *SequenceDesc = (mstudioseqdesc_t *)((byte *)Studio + Studio->seqindex) + EntVars->sequence;

	if (SequenceDesc->numframes <= 1)
	{
		*flFrameRate = 256.0f;
		*flGroundSpeed = 0.0f;
		return;
	}

	*flFrameRate = SequenceDesc->fps * 256.0f / (SequenceDesc->numframes - 1);
	*flGroundSpeed = SequenceDesc->linearmovement.Length();
	*flGroundSpeed = *flGroundSpeed * SequenceDesc->fps / (SequenceDesc->numframes - 1);
}

static void ResetSequenceInfo(CBasePlayer *BasePlayer)
{
	void *Model = GET_MODEL_PTR(ENT(BasePlayer->pev));

	GetSequenceInfo(Model, BasePlayer->pev, &BasePlayer->m_flFrameRate, &BasePlayer->m_flGroundSpeed);
	BasePlayer->m_fSequenceLoops = ((GetSequenceFlags(Model, BasePlayer->pev) & STUDIO_LOOPING) != 0);
	BasePlayer->pev->animtime = gpGlobals->time;
	BasePlayer->m_fSequenceFinished = FALSE;
	BasePlayer->m_flLastEventCheck = gpGlobals->time;
}

void SetAnimation(edict_t *PlayerEdict, int Animation, Activity IActivity, float FrameRate)
{
	entvars_t *PlayerEntVars = &PlayerEdict->v;
	CBasePlayer *BasePlayer = (CBasePlayer *)PlayerEdict->pvPrivateData;

	if (IActivity == ACT_SWIM || IActivity == ACT_LEAP || IActivity == ACT_HOVER)
		return;

	BasePlayer->m_flLastFired = gpGlobals->time;

	PlayerEntVars->frame = 0.0f;
	PlayerEntVars->framerate = FrameRate;
	PlayerEntVars->sequence = Animation;
	PlayerEntVars->animtime = gpGlobals->time;

	BasePlayer->m_Activity = IActivity;
	BasePlayer->m_IdealActivity = IActivity;

	BasePlayer->m_flGroundSpeed = 0.0f;
	BasePlayer->m_flFrameRate = FrameRate;
	BasePlayer->m_flLastEventCheck = gpGlobals->time;
	ResetSequenceInfo(BasePlayer);
}

static cell AMX_NATIVE_CALL SetAnimation(AMX *Plugin, cell *Params)
{
	edict_t *PlayerEdict = EDICT_FOR_NUM(Params[1]);

	if (!PlayerEdict->pvPrivateData)
		return NULL;

	SetAnimation(PlayerEdict, Params[2], (Activity)Params[3], CellToFloat(Params[4]));
	return NULL;
}

cell AMX_NATIVE_CALL GetWeaponDefaultDelay(AMX *Plugin, cell *Params)
{
	return FloatToCell(WEAPON_DEFAULT_DELAY[Params[1]]);
}

cell AMX_NATIVE_CALL GetWeaponAnimDuration(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	return FloatToCell(Weapons[Index].DurationList.Get(Params[2]));
}

cell AMX_NATIVE_CALL BuildWeaponModel(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	CWeapon &Weapon = Weapons[Index];
	const char *Model = MF_GetAmxString(Plugin, Params[3], NULL, nullptr);

	switch (Params[2])
	{
		case BUILD_VIEW: PRECACHE_MODEL(STRING(Weapon.VModel = ALLOC_STRING(Model))); break;
		case BUILD_WEAP: PRECACHE_MODEL(STRING(Weapon.PModel = ALLOC_STRING(Model))); break;
		case BUILD_WORLD: PRECACHE_MODEL(Weapon.WModel = STRING(ALLOC_STRING(Model))); break;
		case BUILD_LIST: PRECACHE_MODEL(Weapon.GModel = STRING(ALLOC_STRING(Model))); break;
	}

	return NULL;
}

cell AMX_NATIVE_CALL BuildWeaponModelEx(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	CWeapon &Weapon = Weapons[Index];
	string_t Model = Params[3];

	switch (Params[2])
	{
		case BUILD_VIEW: PRECACHE_MODEL(STRING(Weapon.VModel = Model)); break;
		case BUILD_WEAP: PRECACHE_MODEL(STRING(Weapon.PModel = Model)); break;
		case BUILD_WORLD: PRECACHE_MODEL(Weapon.WModel = STRING(Model)); break;
		case BUILD_LIST: PRECACHE_MODEL(Weapon.GModel = STRING(Model)); break;
	}

	return NULL;
}

static cell AMX_NATIVE_CALL GetWeaponAnimDelay(AMX *Plugin, cell *Params)
{
	return FloatToCell(Weapons[Params[1]].DurationList[Params[2]]);
}

static cell AMX_NATIVE_CALL GetWeaponClip(AMX *Plugin, cell *Params)
{
	CBasePlayerWeapon *BaseWeapon = ((CBasePlayerWeapon *)EDICT_FOR_NUM(Params[1])->pvPrivateData);

	if (CUSTOM_WEAPON(BaseWeapon) && WEAPON_FLAGS(BaseWeapon) & WFlag::SoloClip)
		return BaseWeapon->m_pPlayer->m_rgAmmo[BaseWeapon->m_iPrimaryAmmoType];
	else
		return BaseWeapon->m_iClip;
}

static cell AMX_NATIVE_CALL SpendWeaponClip(AMX *Plugin, cell *Params)
{
	CBasePlayerWeapon *BaseWeapon = ((CBasePlayerWeapon *)EDICT_FOR_NUM(Params[1])->pvPrivateData);

	if (CUSTOM_WEAPON(BaseWeapon) && WEAPON_FLAGS(BaseWeapon) & WFlag::SoloClip)
		BaseWeapon->m_pPlayer->m_rgAmmo[BaseWeapon->m_iPrimaryAmmoType] -= Params[2];
	else
		BaseWeapon->m_iClip -= Params[2];

	return NULL;
}

static cell AMX_NATIVE_CALL GainWeaponClip(AMX *Plugin, cell *Params)
{
	CBasePlayerWeapon *BaseWeapon = ((CBasePlayerWeapon *)EDICT_FOR_NUM(Params[1])->pvPrivateData);

	if (CUSTOM_WEAPON(BaseWeapon) && WEAPON_FLAGS(BaseWeapon) & WFlag::SoloClip)
		BaseWeapon->m_pPlayer->m_rgAmmo[BaseWeapon->m_iPrimaryAmmoType] += Params[2];
	else
		BaseWeapon->m_iClip += Params[2];

	return NULL;
}

static cell AMX_NATIVE_CALL StatusIconNumber(AMX *Plugin, cell *Params)
{
	StatusIconNumber(EDICT_FOR_NUM(Params[1]), Params[2], (char)Params[3]);
	return NULL;
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
		return FALSE;

	return WEAPON_KEY(((CBasePlayerWeapon *)WeaponEdict->pvPrivateData));
}

static cell AMX_NATIVE_CALL GetWeaponEntityData(AMX *Plugin, cell *Params)
{
	edict_t *WeaponEdict = EDICT_FOR_NUM(Params[1]);

	if (InvalidEntity(WeaponEdict))
		return NULL;

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

	return NULL;
}

static cell AMX_NATIVE_CALL SetWeaponEntityData(AMX *Plugin, cell *Params)
{
	edict_t *WeaponEdict = EDICT_FOR_NUM(Params[1]);

	if (InvalidEntity(WeaponEdict))
		return NULL;

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

	return NULL;
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
	{ "BuildWeaponReload2", BuildWeaponReloadShotgun },
	{ "BuildWeaponFlags", BuildWeaponFlags },
	{ "BuildWeaponSecondaryAttack", BuildWeaponSecondaryAttack },
	{ "BuildWeaponMaxSpeed", BuildWeaponMaxSpeed },
	{ "RegisterWeaponForward", RegisterWeaponForward },
	{ "PrecacheWeaponModel", PrecacheWeaponModel },
	{ "PrecacheWeaponSound", PrecacheWeaponSound },
	{ "PrecacheWeaponGeneric", PrecacheWeaponGeneric },
	{ "FindWeaponByName", FindWeaponByName },
	{ "GiveWeaponByName", GiveWeaponByName },
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
	{ "SetWeaponPathAddon", SetWeaponPathAddon },
	{ "GetWeaponFlags", GetWeaponFlags },
	{ "CreateExplosion", CreateExplosion },
	{ "RadiusDamageEx", RadiusDamageEx },
	{ "CleaveDamage", CleaveDamage },
	{ "CleaveDamageByPlayer", CleaveDamageByPlayer },
	{ "CanPrimaryAttack", CanPrimaryAttack },
	{ "SetNextAttack", SetNextAttack },
	{ "SetCustomIdleAnim", SetCustomIdleAnim },
	{ "GetWeaponPath", GetWeaponPath },
	{ "GetDefaultPath", GetDefaultPath },
	{ "SetPlayerViewModel", SetPlayerViewModel },
	{ "SetPlayerWeapModel", SetPlayerWeapModel },
	{ "PrecacheWeaponModel2", PrecacheWeaponModelEx },
	{ "SetAnimation", SetAnimation },
	{ "GetWeaponDefaultDelay", GetWeaponDefaultDelay },
	{ "GetWeaponAnimDuration", GetWeaponAnimDuration },
	{ "BuildWeaponModel", BuildWeaponModel },
	{ "BuildWeaponModel2", BuildWeaponModelEx },
	{ "GetWeaponAnimDelay", GetWeaponAnimDelay },
	{ "GetWeaponClip", GetWeaponClip },
	{ "SpendWeaponClip", SpendWeaponClip },
	{ "GainWeaponClip", GainWeaponClip },
	{ "StatusIconNumber", StatusIconNumber },
	{ "IsCustomWeaponEntity", IsCustomWeaponEntity },
	{ "GetWeaponEntityKey", GetWeaponEntityKey },
	{ "GetWeaponEntityData", GetWeaponEntityData },
	{ "SetWeaponEntityData", SetWeaponEntityData },
	{ NULL, NULL },
};

#define LocateCharacter(Buffer) while (*Buffer < 33) { if (*Buffer == NULL || *Buffer == '=') return FALSE; Buffer++; }
#define ReadString(Buffer) while ((*Buffer >= 65 && *Buffer <= 90) || (*Buffer >= 97 && *Buffer <= 122)) { Buffer++; }

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
		if (*Buffer == NULL)
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

void ReadWeaponAttack2(CWeapon &Weapon, char *ValuePack)
{
	char *Value = strtok(ValuePack, ",");

	while (Value)
	{
		int IValue = atoi(Value);
		float FValue = (float)atof(Value);

		switch (Weapon.A2I)
		{
			case A2_None: Weapon.A2I = IValue; Weapon.A2V = (A2V *)new int[WEAPON_A2_SIZE[IValue]]; continue;
			case A2_Zoom:
			{
				Weapon.A2V->WA2_ZOOM_MODE = IValue;
				break;
			}
			case A2_Switch:
			{
				Weapon.A2V->WA2_SWITCH_ANIM_A = IValue;
				Weapon.A2V->WA2_SWITCH_ANIM_A_DURATION = FValue;
				Weapon.A2V->WA2_SWITCH_ANIM_B = IValue;
				Weapon.A2V->WA2_SWITCH_ANIM_B_DURATION = FValue;
				Weapon.A2V->WA2_SWITCH_ANIM_IDLE = IValue;
				Weapon.A2V->WA2_SWITCH_ANIM_DRAW = IValue;
				Weapon.A2V->WA2_SWITCH_ANIM_DRAW_DURATION = FValue;
				Weapon.A2V->WA2_SWITCH_ANIM_SHOOT = IValue;
				Weapon.A2V->WA2_SWITCH_ANIM_SHOOT_DURATION = FValue;
				Weapon.A2V->WA2_SWITCH_ANIM_RELOAD = IValue;
				Weapon.A2V->WA2_SWITCH_ANIM_RELOAD_DURATION = FValue;
				Weapon.A2V->WA2_SWITCH_DELAY = FValue;
				Weapon.A2V->WA2_SWITCH_DAMAGE = FValue;
				Weapon.A2V->WA2_SWITCH_RECOIL = FValue;
				PRECACHE_SOUND(Weapon.A2V->WA2_SWITCH_FSOUND = STRING(ALLOC_STRING(Value)));
				break;
			}
			case A2_Burst:
			{
				Weapon.A2V->WA2_BURST_VALUE = IValue;
				break;
			}
			case A2_MultiShot:
			{
				Weapon.A2V->WA2_MULTISHOT_VALUE = IValue;
				break;
			}
			case A2_AutoPistol:
			{
				Weapon.A2V->WA2_AUTOPISTOL_ANIM = IValue;
				Weapon.A2V->WA2_AUTOPISTOL_DELAY = FValue;
				Weapon.A2V->WA2_AUTOPISTOL_RECOIL = FValue;
				break;
			}
			case A2_KnifeAttack:
			{
				Weapon.A2V->WA2_KNIFEATTACK_ANIMATION = IValue;
				Weapon.A2V->WA2_KNIFEATTACK_DELAY = FValue;
				Weapon.A2V->WA2_KNIFEATTACK_DURATION = FValue;
				Weapon.A2V->WA2_KNIFEATTACK_RADIUS = FValue;
				Weapon.A2V->WA2_KNIFEATTACK_DAMAGE_MIN = IValue;
				Weapon.A2V->WA2_KNIFEATTACK_DAMAGE_MAX = IValue;
				Weapon.A2V->WA2_KNIFEATTACK_KNOCKBACK = FValue;
				PRECACHE_SOUND(Weapon.A2V->WA2_KNIFEATTACK_SOUND = STRING(ALLOC_STRING(Value)));
				break;
			}
			case A2_InstaSwitch:
			{
				Weapon.A2V->WA2_INSTASWITCH_ANIM_SHOOT = IValue;
				Weapon.A2V->WA2_INSTASWITCH_DELAY = FValue;
				Weapon.A2V->WA2_INSTASWITCH_DAMAGE = FValue;
				Weapon.A2V->WA2_INSTASWITCH_RECOIL = FValue;
				Weapon.A2V->WA2_INSTASWITCH_NAME = STRING(ALLOC_STRING(Value));
				Weapon.A2V->WA2_INSTASWITCH_NAME2 = STRING(ALLOC_STRING(Value));
			}
			case A2_ZoomCustom:
			{
				Weapon.A2V->WA2_ZOOM_CUSTOM_FOV = IValue;
				break;
			}
		}

		Value = strtok(NULL, ",");
	}
}

void ReadWeaponArray(List<int> *Array, char *Value)
{
	char *Anim = strtok(Value, ",");

	while (Anim != NULL)
	{
		Array->Append(atoi(Anim));
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

const CParam Params[] =
{
	{ "Model", TYPE_STRING, offsetof(CWeapon, Model) },
	{ "Type", TYPE_OTHER1, 0 },
	{ "Name", TYPE_STRING, offsetof(CWeapon, Name) },
	{ "DeployAnim", TYPE_INT, offsetof(CWeapon, AnimD) },
	{ "DeployTime", TYPE_FLOAT, offsetof(CWeapon, Deploy) },
	{ "ShootAnim", TYPE_ARRAY, offsetof(CWeapon, AnimS) },
	{ "ShootTime", TYPE_FLOAT, offsetof(CWeapon, Delay) },
	{ "Delay", TYPE_FLOAT, offsetof(CWeapon, Delay) },
	{ "Damage", TYPE_FLOAT, offsetof(CWeapon, Damage) },
	{ "Recoil", TYPE_FLOAT, offsetof(CWeapon, Recoil) },
	{ "ReloadAnimation", TYPE_FLOAT, offsetof(CWeapon, AnimR) },
	{ "ReloadDuration", TYPE_FLOAT, offsetof(CWeapon, Reload) },
	{ "Clip", TYPE_INT, offsetof(CWeapon, Clip) },
	{ "AmmoID", TYPE_INT, offsetof(CWeapon, AmmoID) },
	{ "Speed", TYPE_FLOAT, offsetof(CWeapon, Speed) },
	{ "VModel", TYPE_STRINT, offsetof(CWeapon, VModel) },
	{ "PModel", TYPE_STRINT, offsetof(CWeapon, PModel) },
	{ "WModel", TYPE_STRING, offsetof(CWeapon, WModel) },
	{ "GModel", TYPE_STRING, offsetof(CWeapon, GModel) },
	{ "FireSound", TYPE_STRING, offsetof(CWeapon, FireSound) },
	{ "Attack2", TYPE_OTHER2, 0 },
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

		Buffer[strlen(Buffer) - 1] = NULL;
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
			if (CVar_LogPointer->value)
				LOG_CONSOLE(PLID, "[CSWM] WARNING: Unknown Key '%s'. (Config.ini)", Key);

			continue;
		}

		strcpy(Dest, Value);
	}

	fclose(File);
}

void LoadWeapons(void)
{
	char *PathName = (char *)MF_BuildPathname("%s/cswm/weapons/*", MF_GetLocalInfo("amxx_configsdir", "addons/amxmodx/configs"));
	char Buffer[260];
	HANDLE FileHandle;
	WIN32_FIND_DATA FindData;
	FileHandle = FindFirstFile(PathName, &FindData);
	PathName[strlen(PathName) - 2] = 0;

	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			sprintf(Buffer, "%s/%s", PathName, FindData.cFileName);

			if (access(Buffer, NULL) == -1)
				continue;

			FILE *File = fopen(Buffer, "rt");

			if (!File)
				continue;

			if (CVar_LogPointer->value)
				LOG_CONSOLE(PLID, "[CSWM] Loading Weapon From File '%s'...", FindData.cFileName);

			CWeapon Weapon;
			char *WeaponP = (char *)&Weapon;
			memset(WeaponP, NULL, sizeof(CWeapon));

			while (!feof(File))
			{
				fgets(Buffer, sizeof(Buffer), File);

				if (!Buffer[0] || Buffer[0] == '#')
					continue;

				char *Key, *Value;

				if (!INI_ReadKeyValue(Buffer, Key, Value))
					continue;

				intptr_t Output;

				if (!ParamHashMap.Retrieve(Key, &Output))
				{
					if (CVar_LogPointer->value)
						LOG_CONSOLE(PLID, "[CSWM] WARNING: Unknown Parameter %s. (%s)", Key, FindData.cFileName);

					continue;
				}

				CParam *Param = (CParam *)Output;

				switch (Param->Type)
				{
					case TYPE_INT: *(int *)(WeaponP + Param->Offset) = atoi(Value); break;
					case TYPE_FLOAT: *(float *)(WeaponP + Param->Offset) = (float)atof(Value); break;
					case TYPE_STRING:{ *(const char **)(WeaponP + Param->Offset) = STRING(ALLOC_STRING(Value)); break; }
					case TYPE_ARRAY: ReadWeaponArray((List<int> *)(WeaponP + Param->Offset, Value), Value); break;
					case TYPE_STRINT: *(string_t *)(WeaponP + Param->Offset) = ALLOC_STRING(Value); break;
					case TYPE_OTHER1: Weapon.Type = ReadWeaponType(Value); break;
					case TYPE_OTHER2: ReadWeaponAttack2(Weapon, Value); break;
				}
			}

			if (!Weapon.Model)
			{
				LOG_CONSOLE(PLID, "[CSWM] WARNING: Found Weapon Without Model, Ignoring... (%s)", FindData.cFileName);
				continue;
			}

			if (!Weapon.VModel)
			{
				sprintf(Buffer, "models/%s%s/V.mdl", PathAddon, Weapon.Model);
				Weapon.VModel = ALLOC_STRING(Buffer);
			}

			PRECACHE_MODEL(STRING(Weapon.VModel));

			if (!Weapon.PModel)
			{
				sprintf(Buffer, "models/%s%s/P.mdl", PathAddon, Weapon.Model);
				Weapon.PModel = ALLOC_STRING(Buffer);
			}

			PRECACHE_MODEL(STRING(Weapon.PModel));

			if (!Weapon.WModel)
			{
				sprintf(Buffer, "models/%s%s/W.mdl", PathAddon, Weapon.Model);
				Weapon.WModel = STRING(ALLOC_STRING(Buffer));
			}

			PRECACHE_MODEL(Weapon.WModel);

			LOG_CONSOLE(PLID, STRING(Weapon.PModel));
			if (!Weapon.GModel)
			{
				sprintf(Buffer, "weapon_%s", Weapon.Model);
				Weapon.GModel = STRING(ALLOC_STRING(Buffer));
			}

			sprintf(Buffer, "sprites/%s.txt", Weapon.GModel);
			PRECACHE_GENERIC(Buffer);

			if (!Weapon.FireSound)
			{
				if (CVar_LogPointer->value)
					LOG_CONSOLE(PLID, "[CSWM] WARNING: Found Weapon Without Fire Sound, Setting Predicted... (%s)", Weapon.Model);

				sprintf(Buffer, "weapon/%s-1.wav", Weapon.Model);
				Weapon.FireSound = STRING(ALLOC_STRING(Buffer));
			}

			PRECACHE_SOUND(Weapon.FireSound);

			RecordWeaponDurationList(Weapon);

			if (!Weapon.AnimD)
				DetectAnimation(Weapon, DETECT_DRAW);

			if (!Weapon.Deploy)
				Weapon.Deploy = Weapon.DurationList[Weapon.AnimD];

			if (!Weapon.Delay)
				Weapon.Delay = WEAPON_DEFAULT_DELAY[WEAPON_TYPE_ID[Weapon.Type]];

			if (!Weapon.AnimS.Length)
				DetectAnimation(Weapon, DETECT_SHOOT);

			if (!Weapon.AnimR)
				DetectAnimation(Weapon, DETECT_RELOAD);

			if (!Weapon.Reload)
				Weapon.Reload = Weapon.DurationList[Weapon.AnimR];

			if (!Weapon.Speed)
			{
				if (CVar_LogPointer->value)
					LOG_CONSOLE(PLID, "[CSWM] WARNING: Found Weapon Without Speed, Using Default Setting... (%s)", Weapon.Model);

				Weapon.Speed = WEAPON_SPEED[Weapon.Type];
			}

			Weapons.Append(Weapon);
			WeaponCount++;
			fclose(File);
		} while (FindNextFile(FileHandle, &FindData));

		FindClose(FileHandle);
	}
}

void OnAmxxAttach(void)
{
	MF_AddNatives(AMXX_NATIVES);
	CVar_LogPointer = CVAR_GET_POINTER("developer");
	const char *ConfigsDir = MF_BuildPathname("%s/cswm", MF_GetLocalInfo("amxx_configsdir", "addons/amxmodx/configs"));
	char PathName[MAX_PATH];
	sprintf(PathName, "%s/Anim.lst", ConfigsDir);
	LoadAnimList(PathName);
	sprintf(PathName, "%s/Config.ini", ConfigsDir);
	LoadConfig(PathName);

	for (int Index = 0; Index < 21; Index++)
		ParamHashMap.Insert(Params[Index].Name, (intptr_t)(Params + Index));
}

