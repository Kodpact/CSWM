#pragma once

#pragma warning (disable: 4127)

//#define MAX_CUSTOM_WEAPONS 64
#define MAX_CUSTOM_WEAPONS Weapons.length()
#define MAX_MODEL_NAME 16
#define MAX_MODEL_PATH_NAME 64
#define MAX_SOUND_NAME MAX_MODEL_NAME + 14

#define GET_CUSTOM_WEAPON(BaseWeapon) BaseWeapon->ammo_338mag
#define SET_CUSTOM_WEAPON(BaseWeapon, Value) BaseWeapon->ammo_338mag = Value

#define GET_WEAPON_FID(BaseWeapon) BaseWeapon->ammo_57mm
#define SET_WEAPON_FID(BaseWeapon, Value) BaseWeapon->ammo_57mm = Value

#define GET_WEAPON_KEY(BaseWeapon) BaseWeapon->ammo_buckshot
#define SET_WEAPON_KEY(BaseWeapon, Value) BaseWeapon->ammo_buckshot = Value
#define GET_ENTITY_KEY(BaseEntity) BaseEntity->ammo_buckshot
#define SET_ENTITY_KEY(BaseEntity, Value) BaseEntity->ammo_buckshot = Value

/* ============= */
#define GET_WEAPON_CURBURST(BaseWeapon) BaseWeapon->ammo_556natobox
#define SET_WEAPON_CURBURST(BaseWeapon, Value) BaseWeapon->ammo_556natobox = Value

#define GET_WEAPON_A2OFFSET(BaseWeapon) BaseWeapon->ammo_9mm
#define SET_WEAPON_A2OFFSET(BaseWeapon, Value) BaseWeapon->ammo_9mm = Value

#define GET_WEAPON_A2DELAY(BaseWeapon) BaseWeapon->maxammo_762nato
#define SET_WEAPON_A2DELAY(BaseWeapon, Value) BaseWeapon->maxammo_762nato = Value

/* ============= */

#define GET_WEAPON_ICON(BaseWeapon) BaseWeapon->ammo_762nato
#define SET_WEAPON_ICON(BaseWeapon, Value) BaseWeapon->ammo_762nato = Value

#define GET_ENTITY_KEY_EX(Entity) Entity->v.iuser3
#define SET_ENTITY_KEY_EX(Entity, Value) Entity->v.iuser3 = Value

#define GET_WEAPON_ATTACK2(BaseWeapon) BaseWeapon->maxammo_57mm
#define SET_WEAPON_ATTACK2(BaseWeapon, Value) BaseWeapon->maxammo_57mm = Value

#define GET_WEAPON_INATTACK2(BaseWeapon) BaseWeapon->maxammo_50ae
#define SET_WEAPON_INATTACK2(BaseWeapon, Value) BaseWeapon->maxammo_50ae = Value

#define GET_WEAPON_INBURST(BaseWeapon) BaseWeapon->ammo_556nato
#define SET_WEAPON_INBURST(BaseWeapon, Value) BaseWeapon->ammo_556nato = Value

#define GET_WEAPON_LASTATTACKINATTACK2(BaseWeapon) BaseWeapon->maxammo_9mm
#define SET_WEAPON_LASTATTACKINATTACK2(BaseWeapon, Value) BaseWeapon->maxammo_9mm = Value

#define GET_WEAPON_OWNER(BaseWeapon) BaseWeapon->pev->iuser2
#define SET_WEAPON_OWNER(BaseWeapon, Value) BaseWeapon->pev->iuser2 = Value
#define SET_WEAPON_OWNER_ED(BaseWeapon, Value) BaseWeapon->pev->owner = Value

#define GET_WEAPON_FLAGS(BaseWeapon) BaseWeapon->maxammo_338mag
#define SET_WEAPON_FLAGS(BaseWeapon, Value) BaseWeapon->maxammo_338mag = Value

#define GET_WEAPON_KEY_EX(BaseWeaponEnt) BaseWeaponEnt->v.iuser3
#define SET_WEAPON_KEY_EX(BaseWeaponEnt, Value) BaseWeapon->v.iuser3 = Value

#define SET_WEAPON_ADDR(BaseWeapon, Value) BaseWeapon->pev->

#define NOISE_DISABLED(BaseWeapon) BaseWeapon->pev->playerclass
#define NOISE_ENABLED(BaseWeapon) !BaseWeapon->pev->playerclass
#define SET_NOISE_DISABLED(BaseWeapon) BaseWeapon->pev->playerclass = FALSE
#define SET_NOISE_ENABLED(BaseWeapon) BaseWeapon->pev->playerclass = TRUE

#define SET_WEAPON_CLIP(BaseWeapon, Value) BaseWeapon->m_iClip = Value
#define SET_WEAPON_AMMOID(BaseWeapon, Value) BaseWeapon->m_iPrimaryAmmoType = Value

#define SECONDARY_WEAPONS_BIT_SUM 67308546
#define DEFAULT_SHOTGUN_RELOAD 3
#define PROJECTILE_ENTITY_KEY BIT(0)
#define SET_PROJECTILE(BaseEntity) BaseEntity->pev->colormap = PROJECTILE_ENTITY_KEY
#define NOT_PROJECTILE(BaseEntity) BaseEntity->pev->colormap != PROJECTILE_ENTITY_KEY
#define GET_PROJECTILE_FORWARD_TOUCH(BaseWeaponEnt) BaseEntity->maxammo_338mag
#define SET_PROJECTILE_FORWARD_TOUCH(BaseWeaponEnt, Value) BaseEntity->maxammo_338mag = Value

enum PType
{
	DEFAULT,
	LIMITED_BOUNCE,
	LIMITED_TIME,
};

/*
int ammo_buckshot;
int ammo_buckshot;
int ammo_9mm;
int ammo_9mm;
int ammo_556nato;
int ammo_556nato;
int ammo_556natobox;
int ammo_556natobox;
int ammo_762nato;
int ammo_762nato;
int ammo_45acp;
int ammo_45acp;
int ammo_50ae;
int ammo_50ae;
int ammo_338mag;
int ammo_338mag;
int ammo_57mm;
int ammo_57mm;
int ammo_357sig;
int ammo_357sig;
*/

enum WType
{
	Pistol,
	Shotgun,
	Rifle,
	Sniper,
};

enum WForward
{
	Purchase,
	DeployPost,
	PrimaryAttackPre,
	PrimaryAttackPost,
	SecondaryAttackPre,
	SecondaryAttackPost,
	ReloadPre,
	ReloadPost,
	HolsterPost,
	DropPost,
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
	Burst,
	CurBurst,
	FID,
	Flags,
	Icon,
	InAttack2,
	InBurst,
	InKnifeAttack,
	Key,
	Owner,
	LastAttackInAttack2,
	InSwitch,
};

#define MAX_WEAPON_TYPES 4
#define MAX_WEAPON_FORWARDS 9

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

	CIcon Icon;

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
	cell Forward_Touch;
};

#define CS_FIRST_ZOOM					40
#define CS_SECOND_AWP_ZOOM				10
#define CS_SECOND_NONAWP_ZOOM			15
#define CS_AUGSG552_ZOOM				55
#define CS_NO_ZOOM						90
/*
#ifndef min
#define min(x, y) x > y ? y : x
#endif

#ifndef max
#define max(x, y) x > y ? x : y
#endif

#ifndef clamp
#define clamp(x, y, z) if(x<y)x=y;if(x>z)x=z;
#endif
*/

#define SET_ONECLIP(x, y) x->v.iuser4 = y;
#define SPEND_ONECLIP(x) x->v.iuser4 -= 1;
#define GET_ONECLIP(x) x->v.iuser4

#define IS_USER_ALIVE(x) (x->v.deadflag == DEAD_NO) && (x->v.health > 0)
#define IS_USER_DEAD(x) (x->v.deadflag != DEAD_NO) || (x->v.health < 1)
#define IS_USER_DEAD_EX(x) (x->deadflag != DEAD_NO) || (x->health < 1)

#define PrivateToEdict(pPrivate) (*(entvars_t **)((char*)pPrivate + 4))->pContainingEntity 

#define RETURN_META_IGNORED() gpMetaGlobals->mres = MRES_IGNORED; return;
#define RETURN_META_SUPERCEDE() gpMetaGlobals->mres = MRES_SUPERCEDE; return;

#define ANGLEVECTORS        (*g_engfuncs.pfnAngleVectors)
#define CLIENT_PRINT        (*g_engfuncs.pfnClientPrintf)
#define CVAR_DIRECTSET      (*g_engfuncs.pfnCvar_DirectSet)
#define GETCLIENTLISTENING  (*g_engfuncs.pfnVoice_GetClientListening)
#define RUNPLAYERMOVE       (*g_engfuncs.pfnRunPlayerMove)
#define SETCLIENTLISTENING  (*g_engfuncs.pfnVoice_SetClientListening)
#define SETCLIENTMAXSPEED   (*g_engfuncs.pfnSetClientMaxspeed)

extern edict_t *SVGame_Edicts;
#define NUM_FOR_EDICT(x) ((int)((edict_t *)(x) - SVGame_Edicts))
#define EDICT_FOR_NUM(x) (SVGame_Edicts + x)

#define WEAPON_MAX_RECOIL 25.0
#define WEAPON_MIN_RECOIL -25.0

#define BIT(x) (1 << x)

enum WFlag
{
	NoHUD = BIT(0),
	AutoReload = BIT(1),
	SoloClip = BIT(2),
	NoDecal = BIT(3),
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
};

#define WEAPON_FLAG_NOHUD (1 << 0)
#define WEAPON_FLAG_AUTORELOAD (1 << 1)
#define WEAPON_FLAG_SOLOCLIP (1 << 2)
#define WEAPON_FLAG_NODECAL (1 << 3)
#define WEAPON_FLAG_NOSMOKE (1 << 4)

void GiveWeaponByName(edict_t *PlayerEdict, const char *Name);
void GiveWeapon(edict_t *PlayerEdict, int Index);




enum xm1014_e
{
	XM1014_IDLE,
	XM1014_FIRE1,
	XM1014_FIRE2,
	XM1014_RELOAD,
	XM1014_PUMP,
	XM1014_START_RELOAD,
	XM1014_DRAW,
};