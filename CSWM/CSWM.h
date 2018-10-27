#pragma once

#include "List.h"
#include "182/AMXX.h"

#define CUSTOM_WEAPON(BaseWeapon) BaseWeapon->ammo_338mag
#define WEAPON_FID(BaseWeapon) BaseWeapon->ammo_57mm
#define WEAPON_KEY(BaseWeapon) BaseWeapon->ammo_buckshot
#define WEAPON_CURBURST(BaseWeapon) BaseWeapon->ammo_556nato

#define WEAPON_A2(BaseWeapon) BaseWeapon->maxammo_57mm
#define WEAPON_A2_OFFSET(BaseWeapon) BaseWeapon->ammo_9mm
#define WEAPON_INA2(BaseWeapon) BaseWeapon->maxammo_50ae
#define WEAPON_INA2_DELAY(BaseWeapon) BaseWeapon->maxammo_762nato

#define WEAPON_INBURST(BaseWeapon) BaseWeapon->maxammo_556nato
#define WEAPON_FLAGS(BaseWeapon) BaseWeapon->maxammo_338mag
#define WEAPON_KEY_EX(BaseWeaponEnt) BaseWeaponEnt->v.iuser3
#define WEAPON_CLIP(BaseWeapon) BaseWeapon->m_iClip

#define PROJECTILE_TYPE(BaseEntity) BaseEntity->ammo_338mag
#define PROJECTILE_FORWARD(BaseWeaponEnt) BaseEntity->maxammo_338mag
#define EFFECT_TYPE(BaseEntity) BaseEntity->ammo_338mag
#define EFFECT_FORWARD(BaseWeaponEnt) BaseEntity->maxammo_338mag
#define EFFECT_EXPIRE_TIME(BaseEntity) BaseEntity->pev->fuser4
#define EFFECT_MAX_FRAMES(BaseEntity) BaseEntity->pev->fuser3
#define EFFECT_LAST_TIME(BaseEntity) BaseEntity->pev->fuser2

enum
{
	DETECT_DRAW,
	DETECT_SHOOT,
	DETECT_RELOAD,
};

enum
{
	BUILD_VIEW,
	BUILD_WEAP,
	BUILD_WORLD,
	BUILD_LIST,
};

enum
{
	PLIMITED_TOUCH = 1,
	PLIMITED_TIME,
};

enum
{
	ELIMITED_TIME = 1,
};

enum WType
{
	Pistol,
	Shotgun,
	Rifle,
	Sniper,
};

enum WForward
{
	SpawnPost,
	DeployPost,
	PrimaryAttackPre,
	PrimaryAttackPrePost,
	PrimaryAttackPost,
	SecondaryAttackPre,
	SecondaryAttackPost,
	ReloadPre,
	ReloadPost,
	HolsterPost,
	DropPost,
	DamagePre,
	DamagePost,
};

enum WReturn
{
	IGNORED,
	SUPERCEDE,
};

enum WShotgunReloadType
{
	M3Style = -1,
	XM1014Style,
	RifleStyle,
};

enum WData
{
	WD_VModel,
	WD_PModel,
	WD_Model,
	WD_Name,
	WD_FireSound,
	WD_WModel,
	WD_GModel,
	WD_Type,
	WD_AnimD,
	WD_AnimS,
	WD_AnimR,
	WD_Clip,
	WD_AmmoID,
	WD_Deploy,
	WD_Reload,
	WD_Delay,
	WD_Damage,
	WD_Recoil,
	WD_Flags,
	WD_A2I,
	WD_Speed,
	WD_Forwards,
	WD_DurationList,
};

enum WEData
{
	WED_Custom,
	WED_FID,
	WED_Key,
	WED_CurBurst,
	WED_A2,
	WED_A2_Offset,
	WED_INA2,
	WED_INA2_Delay,
	WED_INBurst,
	WED_Flags,
};

enum WLimit
{
	MAX_WEAPON_TYPES = 4,
	MAX_HOOKS = 12,
	MAX_WEAPON_FORWARDS = 13,
	MAX_MODEL_NAME = 24,
	MAX_MODEL_PATH_NAME = 64
};

struct CWeapon
{
	string_t VModel, PModel;
	const char *Model, *Name, *FireSound, *WModel, *GModel;
	WType Type;
	int AnimD;
	List<int> AnimS;
	int AnimR;

	int Clip;
	AmmoType AmmoID;

	float Deploy;
	float Reload;
	float Delay;
	float Damage;
	float Recoil;

	int Flags;
	int A2I;
	A2V *A2V;
	float Speed;
	cell Forwards[MAX_WEAPON_FORWARDS];
	List<float> DurationList;
};

struct CAmmo
{
	int Cost;
	int Amount;
	int Max;
	const char *Name;
};

struct CProjectile
{
	const char *Model;
	float Gravity;
	float Speed;
	float Duration;
	cell Forward;
};

struct CEffect
{
	const char *Model;
	string_t ModelIndex;
	float Speed;
	float Duration;
	cell Forward;
};

struct CCleaveDamageInfo
{
	Vector Origin;
	Vector VAngles;
	float FOV;
	BOOL Accurate;
	float Damage;
	float Radius;
	entvars_t *Inflictor;
	entvars_t *Attacker;
	int DamageType;
};

struct CParam
{
	char *Name;
	int Type;
	int Offset;
};

enum ParamType
{
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_STRING,
	TYPE_ARRAY,
	TYPE_STRINT,
	TYPE_OTHER1,
	TYPE_OTHER2,
};

enum ZoomType
{
	CS_SECOND_AWP_ZOOM = 10,
	CS_SECOND_NONAWP_ZOOM = 15,
	CS_FIRST_ZOOM = 40,
	CS_AUGSG552_ZOOM = 54,
	CS_NO_ZOOM = 90,
};

#define IS_USER_ALIVE(x) (x->v.deadflag == DEAD_NO) && (x->v.health > 0)
#define IS_USER_DEAD(x) (x->v.deadflag != DEAD_NO) || (x->v.health < 1)

extern edict_t *SVGame_Edicts;
#define NUM_FOR_EDICT(Edict) (int)(Edict - SVGame_Edicts)
#define EDICT_FOR_NUM(Edict) (SVGame_Edicts + Edict)

#define BIT(x) (1 << x)

enum WFlag
{
	NoHUD = BIT(0),
	AutoReload = BIT(1),
	NoDecal = BIT(2),
	NoSmoke = BIT(3),
	ShotgunCustomReloadSound = BIT(4),
	CustomPrimaryAttack = BIT(5),
	AutoSniper = BIT(6),
	CustomIdleAnim = BIT(7),
	SoloClip = BIT(8),

	SwitchMode_BarTime = BIT(10),
	SwitchMode_NoText = BIT(11),
	KnifeAttack_ScreenShake = BIT(10),
	KnifeAttack_Penetration = BIT(11),
	KnifeAttack_Accurate = BIT(12),
	KnifeAttack_Knockback = BIT(13),
};

enum RDFlag
{
	Penetration = BIT(0),
	NoSelfDamage = BIT(1),
	Knockback = BIT(2),
};

enum
{
	XM1014_IDLE,
	XM1014_FIRE1,
	XM1014_FIRE2,
	XM1014_RELOAD,
	XM1014_PUMP,
	XM1014_START_RELOAD,
	XM1014_DRAW,
};

inline BOOL InvalidEntity(edict_t *Edict)
{
	if (!Edict || !Edict->pvPrivateData)
		return TRUE;

	return FALSE;
}

inline int GetEntityTeam(void *PrivateData)
{
	return *((int *)PrivateData + 114);
}

inline int GetEntityTeam(edict_t *Edict)
{
	return GetEntityTeam(Edict->pvPrivateData);
}

#define GetAMXAddr(ValueA, ValueB) (cell *)(ValueA->base + (int)(((AMX_HEADER *)ValueA->base)->dat + ValueB))
#define CellToFloat(Value) (*(REAL *)&Value)
#define FloatToCell(Value) (*(cell *)&Value)

#ifndef VectorSub
inline void VectorSub(Vector &InA, Vector &InB, Vector &Out)
{
	Out.x = InA.x - InB.x;
	Out.y = InA.y - InB.y;
	Out.z = InA.z - InB.z;
}
#endif

#ifndef VectorMulScalar
inline void VectorMulScalar(Vector &InA, float Scale, Vector &Out)
{
	Out.x = InA.x * Scale;
	Out.y = InA.y * Scale;
	Out.z = InA.z * Scale;
}
#endif

#ifndef VectorSum
inline void VectorSum(Vector &InA, Vector &InB, Vector &Out)
{
	Out.x = InA.x + InB.x;
	Out.y = InA.y + InB.y;
	Out.z = InA.z + InB.z;
}
#endif

// CSWM.cpp

void GiveWeaponByName(edict_t *PlayerEdict, const char *Name);
void GiveWeapon(edict_t *PlayerEdict, int Index);
void UpdateAmmoList();
void SendWeaponAnim(CBasePlayerWeapon *BaseWeapon, int Anim);
cell ShootProjectileTimed(edict_t *LauncherEdict, int ProjectileID);
cell ShootProjectileContact(edict_t *LauncherEdict, int ProjectileID);
cell ShootEffect(edict_t *LauncherEdict, int EffectID);
int Player_GiveAmmoByID(CBasePlayer *BasePlayer, int AmmoID, int Amount);
void PlayerKnockback(edict_t *VictimEdict, Vector &Origin, float Knockback);
BOOL InViewCone(edict_t *PlayerEdict, Vector &Origin, BOOL Accurate);
BOOL InViewCone(Vector &SelfOrigin, Vector &VAngles, float FOV, Vector &Origin, BOOL Accurate);
void StatusIconNumber(edict_t *PlayerEdict, BOOL Status, char Number);
inline int *GetPlayerAmmo(CBasePlayer *BasePlayer, int AmmoID);

// Module.cpp

void LoadWeapons(void);
void SetAnimation(edict_t *PlayerEdict, int Animation, Activity IActivity, float FrameRate = 1.0);
