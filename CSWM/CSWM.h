#pragma once

#pragma warning (disable: 4127)

#define CUSTOM_WEAPON(BaseWeapon) BaseWeapon->ammo_338mag
#define WEAPON_FID(BaseWeapon) BaseWeapon->ammo_57mm
#define WEAPON_KEY(BaseWeapon) BaseWeapon->ammo_buckshot
#define WEAPON_CURBURST(BaseWeapon) BaseWeapon->ammo_556nato

#define WEAPON_A2(BaseWeapon) BaseWeapon->maxammo_57mm
#define WEAPON_A2_OFFSET(BaseWeapon) BaseWeapon->ammo_9mm
#define WEAPON_INA2(BaseWeapon) BaseWeapon->maxammo_50ae
#define WEAPON_INA2_DELAY(BaseWeapon) BaseWeapon->maxammo_762nato

#define WEAPON_INBURST(BaseWeapon) BaseWeapon->maxammo_556nato
#define WEAPON_OWNER(BaseWeapon) BaseWeapon->pev->owner
#define WEAPON_FLAGS(BaseWeapon) BaseWeapon->maxammo_338mag
#define WEAPON_KEY_EX(BaseWeaponEnt) BaseWeaponEnt->v.iuser3
#define WEAPON_CLIP(BaseWeapon) BaseWeapon->m_iClip

#define PROJECTILE_TYPE(BaseEntity) BaseEntity->pev->colormap
#define PROJECTILE_FORWARD(BaseWeaponEnt) BaseEntity->maxammo_338mag

enum PType
{
	LIMITED_TOUCH = 1,
	LIMITED_TIME,
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
	IsCustom,
	Attack2,
	Attack2Offset,
	InAttack2,
	InAttack2Delay,
	Flags,
};

enum WLimit
{
	MAX_WEAPON_TYPES = 4,
	MAX_WEAPON_FORWARDS = 13,
	MAX_MODEL_NAME = 16,
	MAX_SODUN_NAME = MAX_MODEL_NAME + 14,
	MAX_MODEL_PATH_NAME = 64
};

struct CWeapon
{
public:
	string_t VModel, PModel;
	const char *Model, *Name, *FireSound, *WModel;
	WType Type;
	int ID;
	int AnimD;
	float AnimD_Duration;
	int AnimS[3];
	float AnimS_Duration;
	int AnimR;
	float AnimR_Duration;
	int Clip;
	int AmmoID;

	float Delay;
	float Damage;
	float Recoil;

	int Flags;

	int A2I;
	A2V *A2V;

	float Speed;

	cell Forwards[MAX_WEAPON_FORWARDS];
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

enum WZoom
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
#define NUM_FOR_EDICT(x) ((int)((edict_t *)(x) - SVGame_Edicts))
#define EDICT_FOR_NUM(x) (SVGame_Edicts + x)

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

	SwitchMode_BarTime = BIT(10),
	SwitchMode_NoText = BIT(11),
	KnifeAttack_ScreenShake = BIT(10),
	KnifeAttack_Penetration = BIT(11),
	KnifeAttack_Accurate = BIT(12),
	KnifeAttack_Knockback = BIT(13),
};

enum PFlag
{
	Penetration = BIT(0),
	NoSelfDamage = BIT(1),
	Knockback = BIT(2),
};

enum WAnimXM1014
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
	if (FNullEnt(Edict) || !Edict->pvPrivateData)
		return TRUE;

	return FALSE;
}

inline BOOL InvalidEntity(entvars_t *EntVars)
{
	if (!EntVars || InvalidEntity(ENT(EntVars)))
		return TRUE;

	return FALSE;
}

void GiveWeaponByName(edict_t *PlayerEdict, const char *Name);
void GiveWeapon(edict_t *PlayerEdict, int Index);