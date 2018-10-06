#include "Module.h"

#define DEFINE_CHAR64(Value) char *Value = new char[64];

extern List<CWeapon> Weapons;
extern List<CAmmo> Ammos;
extern List<CProjectile> Projectiles;
extern List<CEffect> Effects;

extern StringHashMap AnimHashMap;

extern BOOL CanPrecache;
extern BOOL SV_Cheats;
extern cvar_t *CVar_LogPointer;
extern cvar_t *CVar_AMapPointer;
extern cvar_t *CVar_AutoDetectAnimPointer;

char PathAddOn[32] = "cswm";

cell WeaponCount = NULL;
cell AmmoCount = AMMO_MAX_TYPES;

const cell WEAPON_TYPE_ID[] = { CSW_P228, CSW_XM1014, CSW_AK47, CSW_AWP };
const char WEAPON_TYPE_NAME[][8] = { "Pistol", "Shotgun", "Rifle", "Sniper" };
const float WEAPON_DEFAULT_DELAY[] = { 0.0f, 0.2f, 0.0f, 1.3f, 0.0f, 0.3f, 0.0f, 0.1f, 0.1f, 0.0f, 0.1f, 0.2f, 0.1f, 0.3f, 0.1f, 0.1f, 0.1f, 0.2f, 1.5f, 0.1f, 0.1f, 0.9f, 0.1f, 0.1f, 0.3f, 0.0f, 0.2f, 0.1f, 0.1f, 0.0f, 0.1f };

DEFINE_CHAR64(SPR_Trail);
DEFINE_CHAR64(SPR_Explosion);
DEFINE_CHAR64(SPR_Smoke);
DEFINE_CHAR64(SPR_SmokePuff);
DEFINE_CHAR64(SPR_Ring);

int MI_Trail, MI_Explosion, MI_Smoke;
extern int MI_SmokePuff, MI_Ring;

static TraceResult TResult;
static CCleaveDamageInfo CleaveDamageInfo;

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

void DetectAnimation(CWeapon &Weapon, int Type)
{
	edict_t *InfoEdict = CREATE_ENTITY();
	SET_MODEL(InfoEdict, STRING(Weapon.VModel));

	studiohdr_t *Studio = (studiohdr_t *)GET_MODEL_PTR(InfoEdict);

	for (int Index = 0; Index < Studio->numseq; Index++)
	{
		mstudioseqdesc_t *Sequence = (mstudioseqdesc_t *)((byte *)Studio + Studio->seqindex) + Index;

		switch (Type)
		{
			case 0: if (!strncmp(Sequence->label, "draw", 4)) Weapon.AnimD = Index;
			case 1: if (!strncmp(Sequence->label, "shoot", 5)) Weapon.AnimS.Append(Index);
			case 2: if (!strncmp(Sequence->label, "reload", 6)) Weapon.AnimR = Index;
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
	Weapon.ID = WEAPON_TYPE_ID[Type];
	Weapon.Speed = 250.0f;

	if (CVar_LogPointer->value)
		LOG_CONSOLE(PLID, "[CSWM] Creating Weapon: %s (Type=%s)", Weapon.Name, WEAPON_TYPE_NAME[Weapon.Type]);

	Weapons.Append(Weapon);
	WeaponCount++;
	return WeaponCount - 1;
}

void UpdateAmmoList();
cell AMX_NATIVE_CALL CreateAmmo(AMX *Plugin, cell *Params)
{
	if (!Ammos.Size())
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
	char *Path = new char[MAX_PATH];
	sprintf(Path, "sprites/%s.txt", Weapon.GModel);
	PRECACHE_GENERIC(STRING(ALLOC_STRING(Path)));
	delete[] Path;
	return NULL;
}

cell AMX_NATIVE_CALL BuildWeaponAmmunition(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	CWeapon &Weapon = Weapons[Index];
	Weapon.Clip = Params[2];
	Weapon.AmmoID = Params[3];
	return NULL;
}

cell AMX_NATIVE_CALL BuildWeaponDeploy(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	CWeapon &Weapon = Weapons[Index];
	float Duration = CellToFloat(Params[3]);

	if (Params[2])
		Weapon.AnimD = Params[2];
	else
		DetectAnimation(Weapon, 0);

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

	Weapon.Delay = CellToFloat(Params[2]);
	Weapon.Damage = CellToFloat(Params[3]);
	Weapon.Recoil = CellToFloat(Params[4]);

	if (Params[5])
	{
		Weapon.AnimS.Append(Params[5]);

		for (size_t Index = 5; Index <= Params[0] / sizeof(cell); Index++)
			Weapon.AnimS.Append(*MF_GetAmxAddr(Plugin, Params[Index]));
	}
	else
		DetectAnimation(Weapon, WDetectAnim::DETECT_SHOOT);

	return NULL;
}

cell AMX_NATIVE_CALL BuildWeaponReload(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	CWeapon &Weapon = Weapons[Index];
	float Duration = CellToFloat(Params[3]);

	if (Params[2])
		Weapon.AnimR = Params[2];
	else
		DetectAnimation(Weapon, WDetectAnim::DETECT_RELOAD);

	if (Duration > 0.0f)
		Weapon.Reload = Duration;
	else
		Weapon.Reload = Weapon.DurationList[Weapon.AnimR];

	return NULL;
}

cell AMX_NATIVE_CALL BuildWeaponReload2(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	CWeapon &Weapon = Weapons[Index];

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
	Weapon.A2V = new A2V();
	//Weapon.A2V->TRV = NULL;

	switch (Weapon.A2I = Params[2])
	{
		case A2_Zoom:
		{
			Weapon.A2V->WA2_ZOOM_MODE = *MF_GetAmxAddr(Plugin, Params[3]);
			break;
		}
		case A2_Switch:
		{
			Weapon.A2V->WA2_SWITCH_ANIM_A = *MF_GetAmxAddr(Plugin, Params[3]);
			Weapon.A2V->WA2_SWITCH_ANIM_A_DURATION = CellToFloat(*MF_GetAmxAddr(Plugin, Params[4]));
			Weapon.A2V->WA2_SWITCH_ANIM_B = *MF_GetAmxAddr(Plugin, Params[5]);
			Weapon.A2V->WA2_SWITCH_ANIM_B_DURATION = CellToFloat(*MF_GetAmxAddr(Plugin, Params[6]));
			Weapon.A2V->WA2_SWITCH_ANIM_IDLE = *MF_GetAmxAddr(Plugin, Params[7]);
			Weapon.A2V->WA2_SWITCH_ANIM_DRAW = *MF_GetAmxAddr(Plugin, Params[8]);
			Weapon.A2V->WA2_SWITCH_ANIM_DRAW_DURATION = CellToFloat(*MF_GetAmxAddr(Plugin, Params[9]));
			Weapon.A2V->WA2_SWITCH_ANIM_SHOOT = *MF_GetAmxAddr(Plugin, Params[10]);
			Weapon.A2V->WA2_SWITCH_ANIM_SHOOT_DURATION = CellToFloat(*MF_GetAmxAddr(Plugin, Params[11]));
			Weapon.A2V->WA2_SWITCH_ANIM_RELOAD = *MF_GetAmxAddr(Plugin, Params[12]);
			Weapon.A2V->WA2_SWITCH_ANIM_RELOAD_DURATION = CellToFloat(*MF_GetAmxAddr(Plugin, Params[13]));
			Weapon.A2V->WA2_SWITCH_DELAY = CellToFloat(*MF_GetAmxAddr(Plugin, Params[14]));
			Weapon.A2V->WA2_SWITCH_DAMAGE = CellToFloat(*MF_GetAmxAddr(Plugin, Params[15]));
			Weapon.A2V->WA2_SWITCH_RECOIL = CellToFloat(*MF_GetAmxAddr(Plugin, Params[16]));
			PRECACHE_SOUND(Weapon.A2V->WA2_SWITCH_FSOUND = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[17], NULL, NULL))));
			break;
		}
		case A2_Burst:
		{
			Weapon.A2V->WA2_BURST_VALUE = *MF_GetAmxAddr(Plugin, Params[3]);
			break;
		}
		case A2_MultiShot:
		{
			Weapon.A2V->WA2_MULTISHOT_VALUE = *MF_GetAmxAddr(Plugin, Params[3]);
			break;
		}
		case A2_AutoPistol:
		{
			Weapon.A2V->WA2_AUTOPISTOL_ANIM = *MF_GetAmxAddr(Plugin, Params[3]);
			Weapon.A2V->WA2_AUTOPISTOL_DELAY = CellToFloat(*MF_GetAmxAddr(Plugin, Params[4]));
			Weapon.A2V->WA2_AUTOPISTOL_RECOIL = CellToFloat(*MF_GetAmxAddr(Plugin, Params[5]));
			break;
		}
		case A2_KnifeAttack:
		{
			Weapon.A2V->WA2_KNIFEATTACK_ANIMATION = *MF_GetAmxAddr(Plugin, Params[3]);
			Weapon.A2V->WA2_KNIFEATTACK_DELAY = CellToFloat(*MF_GetAmxAddr(Plugin, Params[4]));
			Weapon.A2V->WA2_KNIFEATTACK_DURATION = CellToFloat(*MF_GetAmxAddr(Plugin, Params[5]));
			Weapon.A2V->WA2_KNIFEATTACK_RADIUS = CellToFloat(*MF_GetAmxAddr(Plugin, Params[6]));
			Weapon.A2V->WA2_KNIFEATTACK_DAMAGE_MIN = *MF_GetAmxAddr(Plugin, Params[7]);
			Weapon.A2V->WA2_KNIFEATTACK_DAMAGE_MAX = *MF_GetAmxAddr(Plugin, Params[8]);
			Weapon.A2V->WA2_KNIFEATTACK_KNOCKBACK = CellToFloat(*MF_GetAmxAddr(Plugin, Params[9]));
			PRECACHE_SOUND(Weapon.A2V->WA2_KNIFEATTACK_SOUND = STRING(ALLOC_STRING(MF_GetAmxString(Plugin, Params[10], NULL, NULL))));
			break;
		}
		case A2_RadiusDamage:
		{

			break;
		}
		case A2_Charge:
		{
			break;
		}
		case A2_InstaSwitch:
		{
			Weapon.A2V->WA2_INSTASWITCH_ANIM_SHOOT = *MF_GetAmxAddr(Plugin, Params[3]);
			Weapon.A2V->WA2_INSTASWITCH_DELAY = CellToFloat(*MF_GetAmxAddr(Plugin, Params[4]));
			Weapon.A2V->WA2_INSTASWITCH_DAMAGE = CellToFloat(*MF_GetAmxAddr(Plugin, Params[5]));
			Weapon.A2V->WA2_INSTASWITCH_RECOIL = CellToFloat(*MF_GetAmxAddr(Plugin, Params[6]));
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
	char *String = MF_GetAmxString(Plugin, Params[3], NULL, NULL);

	int Forward = Params[2];

	if (Forward == WForward::DamagePost)
		Weapon.Forwards[Forward] = MF_RegisterSPForwardByName(Plugin, String, FP_CELL, FP_FLOAT, FP_CELL, FP_DONE);
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
	cell PIndex = Params[1];

	if (PIndex < 0 || PIndex > gpGlobals->maxClients)
		return FALSE;

	GiveWeaponByName(INDEXENT(PIndex), (char *)MF_GetAmxString(Plugin, Params[1], NULL, NULL));
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

	GiveWeapon(INDEXENT(PIndex), WIndex);
	return TRUE;
}

static cell AMX_NATIVE_CALL GetWeaponData(AMX *Plugin, cell *Params)
{
	edict_t *WeaponEdict = EDICT_FOR_NUM(Params[1]);

	if (!WeaponEdict)
		return NULL;

	CBasePlayerWeapon *BaseWeapon;

	if (!(BaseWeapon = (CBasePlayerWeapon *)WeaponEdict->pvPrivateData))
		return NULL;

	switch (Params[2])
	{
		case WData::WData_IsCustom: return CUSTOM_WEAPON(BaseWeapon);
		case WData::WData_Attack2: return WEAPON_A2(BaseWeapon);
		case WData::WData_Attack2Offset: return WEAPON_A2_OFFSET(BaseWeapon);
		case WData::WData_InAttack2: return WEAPON_INA2(BaseWeapon);
		case WData::WData_InAttack2Delay: return WEAPON_INA2_DELAY(BaseWeapon);
		case WData::WData_Flags: return WEAPON_A2(BaseWeapon);
		case WData::WData_Key: return WEAPON_KEY(BaseWeapon);
	}

	return NULL;
}

static cell AMX_NATIVE_CALL SetWeaponData(AMX *Plugin, cell *Params)
{
	edict_t *WeaponEdict = EDICT_FOR_NUM(Params[1]);

	if (!WeaponEdict)
		return NULL;

	CBasePlayerWeapon *BaseWeapon;

	if (!(BaseWeapon = (CBasePlayerWeapon *)WeaponEdict->pvPrivateData))
		return NULL;

	int Value = Params[3];

	switch (Params[2])
	{
		//case WData::WData_IsCustom: return CUSTOM_WEAPON(BaseWeapon) = Value; break;
		case WData::WData_Attack2: return WEAPON_A2(BaseWeapon) = Value; break;
		case WData::WData_Attack2Offset: return WEAPON_A2_OFFSET(BaseWeapon) = Value; break;
		case WData::WData_InAttack2: return WEAPON_INA2(BaseWeapon) = Value; break;
		case WData::WData_InAttack2Delay: return WEAPON_INA2_DELAY(BaseWeapon) = Value; break;
		case WData::WData_Flags: return WEAPON_A2(BaseWeapon) = Value; break;
	}

	return NULL;
}

void CallSendWeaponAnim(CBasePlayerWeapon *BaseWeapon, int Anim);
static cell AMX_NATIVE_CALL SendWeaponAnim(AMX *Plugin, cell *Params)
{
	edict_t *WeaponEdict = EDICT_FOR_NUM(Params[1]);

	if (!WeaponEdict)
		return NULL;

	CBasePlayerWeapon *BaseWeapon;

	if (!(BaseWeapon = (CBasePlayerWeapon *)WeaponEdict->pvPrivateData))
		return NULL;

	CallSendWeaponAnim(BaseWeapon, Params[2]);
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
		Projectile.Duration = CellToFloat(*MF_GetAmxAddr(Plugin, Params[5]));

	Projectiles.Append(Projectile);
	return Projectiles.Size() - 1;
}

cell AMX_NATIVE_CALL CreateEffect(AMX *Plugin, cell *Params)
{
	CEffect Effect;
	memset(&Effect, NULL, sizeof(CEffect));
	PRECACHE_MODEL(Effect.Model = STRING(Effect.ModelIndex = ALLOC_STRING(MF_GetAmxString(Plugin, Params[1], NULL, NULL))));
	Effect.Speed = CellToFloat(Params[2]);
	Effect.Forward = MF_RegisterSPForwardByName(Plugin, MF_GetAmxString(Plugin, Params[3], NULL, NULL), FP_CELL, FP_DONE);

	if (Params[0] / sizeof(cell) > 3)
		Effect.Duration = CellToFloat(*MF_GetAmxAddr(Plugin, Params[4]));

	Effects.Append(Effect);
	return Effects.Size() - 1;
}

cell ShootProjectileTimed(edict_t *LauncherEdict, int ProjectileID);
static cell AMX_NATIVE_CALL _ShootProjectileTimed(AMX *Plugin, cell *Params)
{
	return ShootProjectileTimed(EDICT_FOR_NUM(Params[1]), Params[2]);
}

cell ShootProjectileContact(edict_t *LauncherEdict, int ProjectileID);
static cell AMX_NATIVE_CALL _ShootProjectileContact(AMX *Plugin, cell *Params)
{
	return ShootProjectileContact(EDICT_FOR_NUM(Params[1]), Params[2]);
}

cell ShootEffect(edict_t *LauncherEdict, int EffectID);
static cell AMX_NATIVE_CALL _ShootEffect(AMX *Plugin, cell *Params)
{
	return ShootEffect(EDICT_FOR_NUM(Params[1]), Params[2]);
}

int Player_GiveAmmoByID(CBasePlayer *BasePlayer, int AmmoID, int Amount);
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

	for (unsigned int Index = NULL; Index < Ammos.Size(); Index++)
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

cell AMX_NATIVE_CALL SetWeaponPathAddOn(AMX *Plugin, cell *Params)
{
	strcpy_s(PathAddOn, MF_GetAmxString(Plugin, Params[1], NULL, NULL));
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
	Vector Origin = GetVectorFromAddress(MF_GetAmxAddr(Plugin, Params[1]));
	int Flags = Params[2];

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(Origin.x);
	WRITE_COORD(Origin.y);
	WRITE_COORD(Origin.z + 40.0f);
	WRITE_SHORT(MI_Explosion);
	WRITE_BYTE(25);
	WRITE_BYTE(25);
	WRITE_BYTE(Flags);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_WORLDDECAL);
	WRITE_COORD(Origin.x);
	WRITE_COORD(Origin.y);
	WRITE_COORD(Origin.z);
	WRITE_BYTE(RANDOM_LONG(46, 48));
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_SMOKE);
	WRITE_COORD(Origin.x);
	WRITE_COORD(Origin.y);
	WRITE_COORD(Origin.z);
	WRITE_SHORT(MI_Smoke);
	WRITE_BYTE(30);
	WRITE_BYTE(10);
	MESSAGE_END();
	return NULL;
}

void PlayerKnockback(edict_t *VictimEdict, Vector &Origin, float Knockback);
static cell AMX_NATIVE_CALL RadiusDamage2(AMX *Plugin, cell *Params)
{
	Vector Origin = GetVectorFromAddress(MF_GetAmxAddr(Plugin, Params[1]));
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

		Result = BaseEntity->TakeDamage(InflictorVars, AttackerVars, (TargetEdict == OwnerEdict ? 0.4 : 1.0) * ((Damage * Radius) / (TargetOrigin - Origin).Length()), DMG_EXPLOSION);
		
		if (Result & (Flags & RDFlag::Knockback) && BaseEntity->IsPlayer())
			PlayerKnockback(TargetEdict, Origin, RANDOM_FLOAT(120.0f, 150.0f));
	}
	
	return NULL;
}

BOOL InViewCone(edict_t *PlayerEdict, Vector &Origin, BOOL Accurate);
BOOL InViewCone(Vector &SelfOrigin, Vector &VAngles, float FOV, Vector &Origin, BOOL Accurate);
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
	CleaveDamageInfo.Origin = GetVectorFromAddress(MF_GetAmxAddr(Plugin, Params[1]));
	CleaveDamageInfo.VAngles = GetVectorFromAddress(MF_GetAmxAddr(Plugin, Params[2]));
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

cell AMX_NATIVE_CALL GetWeaponPath(AMX *Plugin, cell *Params)
{
	char *Model = (char *)STRING(Weapons[Params[1]].VModel);
	int Length = strlen(Model);
	int Index;
	char OldChar;

	for (Index = Length - 1; Index >= 0; Index--)
	{
		if (Model[Index] == '/' || Model[Index] == '\\')
		{
			OldChar = Model[Index];
			Model[Index] = NULL;
			break;
		}
	}

	MF_SetAmxString(Plugin, Params[2], Model, Params[3]);
	Model[Index] = OldChar;
	return NULL;
}

cell AMX_NATIVE_CALL GetDefaultPath(AMX *Plugin, cell *Params)
{
	char Path[64];
	Q_sprintf(Path, "models/%s", PathAddOn);
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

cell AMX_NATIVE_CALL PrecacheWeaponModel2(AMX *Plugin, cell *Params)
{
	string_t Model = ALLOC_STRING(MF_GetAmxString(Plugin, Params[1], NULL, nullptr));
	PRECACHE_MODEL(STRING(Model));
	return (cell)Model;
}

static int GetSequenceFlags(void *Model, entvars_t *EntVars)
{
	studiohdr_t *Studio = (studiohdr_t *)Model;

	if (!Studio || EntVars->sequence >= Studio->numseq)
	{
		return 0;
	}

	mstudioseqdesc_t *SequenceDesc = (mstudioseqdesc_t *)((byte *)Studio + Studio->seqindex) + EntVars->sequence;
	return SequenceDesc->flags;
}

static void GetSequenceInfo(void *Model, entvars_t *EntVars, float *flFrameRate, float *flGroundSpeed)
{
	studiohdr_t *Studio = (studiohdr_t *)Model;

	if (!Studio)
	{
		return;
	}

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
	//BasePlayer->EntVars->framerate = 1.0f;
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

	//BasePlayer->m_fSequenceLoops = NULL;
	//BasePlayer->m_fSequenceFinished = FALSE;
	BasePlayer->m_Activity = IActivity;
	BasePlayer->m_IdealActivity = IActivity;

	BasePlayer->m_flGroundSpeed = 0.0f;
	BasePlayer->m_flFrameRate = FrameRate;
	BasePlayer->m_flLastEventCheck = gpGlobals->time;
	ResetSequenceInfo(BasePlayer);
}

static cell AMX_NATIVE_CALL _SetAnimation(AMX *Plugin, cell *Params)
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
		case WBuildModel::VModel: PRECACHE_MODEL(STRING(Weapon.VModel = ALLOC_STRING(Model))); break;
		case WBuildModel::PModel: PRECACHE_MODEL(STRING(Weapon.PModel = ALLOC_STRING(Model))); break;
		case WBuildModel::WModel: PRECACHE_MODEL(Weapon.WModel = STRING(ALLOC_STRING(Model))); break;
		case WBuildModel::GModel: PRECACHE_MODEL(Weapon.GModel = STRING(ALLOC_STRING(Model))); break;
	}

	return NULL;
}

cell AMX_NATIVE_CALL BuildWeaponModel2(AMX *Plugin, cell *Params)
{
	cell Index = Params[1];

	if (Index < 0 || Index >= WeaponCount)
		return NULL;

	CWeapon &Weapon = Weapons[Index];
	string_t Model = Params[3];

	switch (Params[2])
	{
		case WBuildModel::VModel: PRECACHE_MODEL(STRING(Weapon.VModel = Model)); break;
		case WBuildModel::PModel: PRECACHE_MODEL(STRING(Weapon.PModel = Model)); break;
		case WBuildModel::WModel: PRECACHE_MODEL(Weapon.WModel = STRING(Model)); break;
		case WBuildModel::GModel: PRECACHE_MODEL(Weapon.GModel = STRING(Model)); break;
	}

	return NULL;
}

static cell AMX_NATIVE_CALL IsEnemy(AMX *Plugin, cell *Params)
{
	edict_t *WeaponOwner = EDICT_FOR_NUM(Params[1])->v.owner;
	edict_t *Victim = EDICT_FOR_NUM(Params[2]);

	if (InvalidEntity(WeaponOwner) || InvalidEntity(Victim))
		return NULL;

	return GetEntityTeam(WeaponOwner) != GetEntityTeam(Victim);
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

static cell AMX_NATIVE_CALL InvalidEntity(AMX *Plugin, cell *Params)
{
	return InvalidEntity(EDICT_FOR_NUM(Params[1]));
}

static cell AMX_NATIVE_CALL InvalidPlayer(AMX *Plugin, cell *Params)
{
	int PlayerID = Params[1];

	if (PlayerID < 1 || PlayerID > gpGlobals->maxClients)
		return TRUE;

	edict_t *PlayerEdict = EDICT_FOR_NUM(PlayerID);

	if (InvalidEntity(PlayerEdict) || !(PlayerEdict->v.flags & FL_CLIENT))
		return TRUE;

	return FALSE;
}

static cell AMX_NATIVE_CALL PlayEmptySound(AMX *Plugin, cell *Params)
{
	((CBasePlayerWeapon *)EDICT_FOR_NUM(Params[1])->pvPrivateData)->PlayEmptySound();
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
	{ "BuildWeaponReload2", BuildWeaponReload2 },
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
	{ "SetWeaponData", SetWeaponData },
	{ "SendWeaponAnim", SendWeaponAnim },
	{ "CreateProjectile", CreateProjectile },
	{ "CreateEffect", CreateEffect },
	{ "ShootProjectileTimed", _ShootProjectileTimed },
	{ "ShootProjectileContact", _ShootProjectileContact },
	{ "ShootEffect", _ShootEffect },
	{ "GiveAmmo", GiveAmmo },
	{ "SetAmmoName", SetAmmoName },
	{ "FindAmmoByName", FindAmmoByName },
	{ "GetWeaponCount", GetWeaponCount },
	{ "GetWeaponName", GetWeaponName },
	{ "SetWeaponPathAddOn", SetWeaponPathAddOn },
	{ "GetWeaponFlags", GetWeaponFlags },
	{ "CreateExplosion", CreateExplosion },
	{ "RadiusDamage2", RadiusDamage2 },
	{ "CleaveDamage", CleaveDamage },
	{ "CleaveDamageByPlayer", CleaveDamageByPlayer },
	{ "CanPrimaryAttack", CanPrimaryAttack },
	{ "SetNextAttack", SetNextAttack },
	{ "SetCustomIdleAnim", SetCustomIdleAnim },
	{ "GetWeaponPath", GetWeaponPath },
	{ "GetDefaultPath", GetDefaultPath },
	{ "SetPlayerViewModel", SetPlayerViewModel },
	{ "SetPlayerWeapModel", SetPlayerWeapModel },
	{ "PrecacheWeaponModel2", PrecacheWeaponModel2 },
	{ "SetAnimation", _SetAnimation },
	{ "GetWeaponDefaultDelay", GetWeaponDefaultDelay },
	{ "GetWeaponAnimDuration", GetWeaponAnimDuration },
	{ "BuildWeaponModel", BuildWeaponModel },
	{ "BuildWeaponModel2", BuildWeaponModel2 },
	{ "IsEnemy", IsEnemy },
	{ "GetWeaponAnimDelay", GetWeaponAnimDelay },
	{ "GetWeaponClip", GetWeaponClip },
	{ "SpendWeaponClip", SpendWeaponClip },
	{ "GainWeaponClip", GainWeaponClip },
	{ "InvalidEntity", InvalidEntity },
	{ "InvalidPlayer", InvalidPlayer },
	{ "PlayEmptySound", PlayEmptySound },
	{ NULL, NULL },
};

void INI_RemoveSpace(char *String)
{
	int Index;
	int JumpRadius = 0;
	int Length = strlen(String);

	for (Index = 0; Index < Length; Index++)
	{
		String[Index - JumpRadius] = String[Index];

		if (String[Index] < 33 || String[Index] > 126 || String[Index] == '"')
			JumpRadius++;
	}

	String[Index - JumpRadius] = NULL;
}

char *INI_GetValuePointer(char *String)
{
	int JumpRadius = 0;
	int Length = strlen(String);

	for (int Index = 0; Index < Length; Index++)
	{
		if (String[Index] == '=')
		{
			String[Index] = NULL;
			return (String + Index + 1);
		}
	}

	return NULL;
}

void OnAmxxAttach(void)
{
	MF_AddNatives(AMXX_NATIVES);

	const char *ConfigsDir = MF_BuildPathname("%s/cswm", MF_GetLocalInfo("amxx_configsdir", "addons/amxmodx/configs"));
	char *PathName = new char[MAX_PATH];
	char *Buffer = new char[128];

	sprintf(PathName, "%s/Anim.lst", ConfigsDir);

	FILE *ConfigsFile = fopen(PathName, "rt");

	if (!ConfigsFile)
	{
		LOG_CONSOLE(PLID, "[CSWM] 'Anim.lst' File Not Found! (./CSWM/Anim.lst)");
		return;
	}

	while (!feof(ConfigsFile))
	{
		fgets(Buffer, 127, ConfigsFile);

		if (!Buffer[0])
			continue;

		Buffer[strlen(Buffer) - 1] = NULL;
		AnimHashMap.Insert(Buffer, AnimHashMap.Length());
	}

	fclose(ConfigsFile);
	sprintf(PathName, "%s/Config.ini", ConfigsDir);

	char SPR_Names[][16] = { "Trail", "Explosion", "Smoke", "SmokePuff", "Ring" };
	char **SPR_Pointers[5] = { &SPR_Trail, &SPR_Explosion, &SPR_Smoke, &SPR_SmokePuff, &SPR_Ring };

	enum
	{
		READING_NONE,
		READING_MAIN,
		READING_MODELS,
	};

	int ReadingDest;
	char *Value;
	ConfigsFile = fopen(PathName, "rt");

	while (!feof(ConfigsFile))
	{
		fgets(Buffer, 127, ConfigsFile);

		if (!Buffer[0])
			continue;

		INI_RemoveSpace(Buffer);

		if (!strncmp(Buffer, "[MAIN]", 6))
		{
			ReadingDest = READING_MAIN;
			continue;
		}
		else if (!strncmp(Buffer, "[MODELS]", 8))
		{
			ReadingDest = READING_MODELS;
			continue;
		}

		Value = INI_GetValuePointer(Buffer);

		switch (ReadingDest)
		{
			case READING_NONE: break;
			case READING_MAIN: break;
			case READING_MODELS:
			{
				for (int Index = 0; Index < 5; Index++)
				{
					if (!strcmp(Buffer, SPR_Names[Index]))
						strcpy(*SPR_Pointers[Index], Value);
				}
				break;
			}
		}
	}

	fclose(ConfigsFile);

	delete[] PathName;
	delete[] Buffer;
}

