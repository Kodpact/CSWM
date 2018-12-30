#ifndef INCLUDE_CSWM_H
#define INCLUDE_CSWM_H

#include "List.h"
#include "182/AMXX.h"

#if defined WIN32
#define __FC __fastcall
#else
#define __FC
#endif

typedef void(__FC *FN_WEAPON_SPAWN) (CBasePlayerWeapon *);
typedef BOOL(__FC *FN_WEAPON_DEPLOY) (CBasePlayerWeapon *);
typedef void(__FC *FN_WEAPON_PRIMARYATTACK) (CBasePlayerWeapon *);
typedef void(__FC *FN_WEAPON_SECONDARYATTACK) (CBasePlayerWeapon *);
typedef void(__FC *FN_WEAPON_RELOAD) (CBasePlayerWeapon *);
typedef void(__FC *FN_WEAPON_POSTFRAME) (CBasePlayerWeapon *);
typedef void(__FC *FN_WEAPON_IDLE) (CBasePlayerWeapon *);
typedef float(__FC *FN_WEAPON_GET_MAXSPEED) (CBasePlayerWeapon *);
typedef void(__FC *FN_WEAPON_PLAY_EMPTY_SOUND) (CBasePlayerWeapon *);
typedef void(__FC *FN_WEAPON_RESET_EMPTY_SOUND) (CBasePlayerWeapon *);
typedef void(__FC *FN_WEAPONBOX_SPAWN) (CWeaponBox *);
typedef void(__FC *FN_GRENADE_SPAWN) (CGrenade *);
typedef void(__FC *FN_THINK) (CBaseEntity *);

#if defined _WIN32
typedef BOOL(__FC *FN_WEAPON_ADDTOPLAYER) (CBasePlayerWeapon *, int, CBasePlayer *);
typedef void(__fastcall *FN_WEAPON_HOLSTER) (CBasePlayerWeapon *, int, int);
typedef int(__fastcall *FN_WEAPON_EXTRACT_AMMO) (CBasePlayerWeapon *, int, CBasePlayerWeapon *);
typedef void(__fastcall *FN_WEAPON_SEND_WEAPON_ANIM) (CBasePlayerWeapon *, int, int, int);
typedef BOOL(__fastcall *FN_TAKEDAMAGE) (CBaseEntity *, int, entvars_t *, entvars_t *, float, int);
typedef void(__fastcall *FN_TRACEATTACK) (CBaseEntity *, int, entvars_t *, float, Vector, TraceResult *, int);
typedef void(__fastcall *FN_TOUCH) (CBaseEntity *, int, CBaseEntity *);
#else
typedef BOOL(__FC *FN_WEAPON_ADDTOPLAYER) (CBasePlayerWeapon *, CBasePlayer *);
typedef void(*FN_WEAPON_HOLSTER) (CBasePlayerWeapon *, int);
typedef int(*FN_WEAPON_EXTRACT_AMMO) (CBasePlayerWeapon *, CBasePlayerWeapon *);
typedef void(*FN_WEAPON_SEND_WEAPON_ANIM) (CBasePlayerWeapon *, int, int);
typedef BOOL(*FN_TAKEDAMAGE) (CBaseEntity *, entvars_t *, entvars_t *, float, int);
typedef void(*FN_TRACEATTACK) (CBaseEntity *, entvars_t *, float, Vector, TraceResult *, int);
typedef void(*FN_TOUCH) (CBaseEntity *, CBaseEntity *);
#endif

/* Only Used To Call/Declare Multiple Argument Based Functions */

#if defined _WIN32
#define DECLFUNC_OS(Return, Name, First, ...) Return __fastcall Name(First, int, ##__VA_ARGS__)
#define CALLFUNC_OS(Function, First, ...) Function(First, 0, ##__VA_ARGS__)
#define CALLFUNC_GAME(Type, Function, Caller, ...) ((Type)Function)(Caller, 0, ##__VA_ARGS__)
#else
#define DECLFUNC_OS(Return, Name, First, ...) Return Name(First, ##__VA_ARGS__)
#define CALLFUNC_OS(Function, First, ...) Function(First, ##__VA_ARGS__)
#define CALLFUNC_GAME(Type, Function, Caller, ...) ((Type)Function)(Caller, ##__VA_ARGS__)
#endif

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
	SpawnPre,
	SpawnPost,
	DeployPre,
	DeployPrePost,
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
	MAX_WEAPON_FORWARDS,
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
	MAX_MODEL_NAME = 24,
	MAX_MODEL_PATH_NAME = 64,
};

enum
{
	Zoom_Rifle,
	Zoom_SniperF,
	Zoom_SniperS,
	Zoom_SniperB,
};

enum WAttack2
{
	A2_None,
	A2_Zoom,
	A2_Switch,
	A2_Burst,
	A2_MultiShot,
	A2_AutoPistol,
	A2_KnifeAttack,
	A2_InstaSwitch,
	A2_ZoomCustom,
};

struct Attack2;

#define WA2_ZOOM_MODE 0

#define WA2_SWITCH_ANIM_A 0
#define WA2_SWITCH_ANIM_B 1
#define WA2_SWITCH_ANIM_IDLE 2
#define WA2_SWITCH_ANIM_DRAW 3
#define WA2_SWITCH_ANIM_SHOOT 4
#define WA2_SWITCH_ANIM_RELOAD 5
#define WA2_SWITCH_ANIM_A_DURATION 6
#define WA2_SWITCH_ANIM_B_DURATION 7
#define WA2_SWITCH_ANIM_DRAW_DURATION 8
#define WA2_SWITCH_ANIM_SHOOT_DURATION 9
#define WA2_SWITCH_ANIM_RELOAD_DURATION 10
#define WA2_SWITCH_DELAY 11
#define WA2_SWITCH_DAMAGE 12
#define WA2_SWITCH_RECOIL 13
#define WA2_SWITCH_FSOUND 14

#define WA2_BURST_VALUE 0

#define WA2_MULTISHOT_VALUE 0

#define WA2_AUTOPISTOL_ANIM 0
#define WA2_AUTOPISTOL_DELAY 1
#define WA2_AUTOPISTOL_RECOIL 2

#define WA2_KNIFEATTACK_ANIMATION 0
#define WA2_KNIFEATTACK_DELAY 1
#define WA2_KNIFEATTACK_DURATION 2
#define WA2_KNIFEATTACK_RADIUS 3
#define WA2_KNIFEATTACK_DAMAGE_MIN 4
#define WA2_KNIFEATTACK_DAMAGE_MAX 5
#define WA2_KNIFEATTACK_KNOCKBACK 6
#define WA2_KNIFEATTACK_MULTI 7
#define WA2_KNIFEATTACK_SOUND 8

#define WA2_INSTASWITCH_ANIM_SHOOT 0
#define WA2_INSTASWITCH_DELAY 1
#define WA2_INSTASWITCH_DAMAGE 2
#define WA2_INSTASWITCH_RECOIL 3
#define WA2_INSTASWITCH_NAME 4
#define WA2_INSTASWITCH_NAME2 5

#define WA2_ZOOM_CUSTOM_FOV 0

#define GetAttack2Data(Type, Data, Offset) *(Type *)((int *)Data + Offset)

struct CWeapon
{
	string_t VModel, PModel;
	const char *Model, *Name, *FireSound, *WModel, *GModel;
	WType Type;
	int AnimI;
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
	Attack2 *A2V;
	float Speed;
	cell Forwards[MAX_WEAPON_FORWARDS];
	List<float> DurationList;
	int WBody;
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

struct CKnockbackInfo
{
	float PushPower;
	float JumpPower;
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
	TYPE_OTHER3,
};

enum ZoomType
{
	CS_SECOND_AWP_ZOOM = 10,
	CS_SECOND_NONAWP_ZOOM = 15,
	CS_FIRST_ZOOM = 40,
	CS_AUGSG552_ZOOM = 55,
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
	DisableReload = BIT(9),
	ReloadKeepFOV = BIT(10),

	Zoom_NoSound = BIT(10),
	ZoomCustom_NoSound = BIT(10),
	SwitchMode_BarTime = BIT(10),
	SwitchMode_NoText = BIT(11),
	AutoPistol_NoSceenShake = BIT(10),
	KnifeAttack_ScreenShake = BIT(10),
	KnifeAttack_Penetration = BIT(11),
	KnifeAttack_Accurate = BIT(12),
	KnifeAttack_Knockback = BIT(13),
	KnifeAttack_NoSound = BIT(14),
};

enum RDFlag
{
	Penetration = BIT(0),
	IgnoreSelf = BIT(1),
	Knockback = BIT(2),
	KnockAny = BIT(3),
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


extern size_t PEV_Offset;
/* Entity Vars From Private Data */
#define EV_FROM_PD(Data) (*(entvars_t **)((char *)Data + PEV_Offset))
/* Edict From Private Data */
#define ED_FROM_PD(Data) (*(entvars_t **)((char *)Data + PEV_Offset))->pContainingEntity
/* Edict Index From Private Data */
#define EDI_FROM_PD(Data) NUM_FOR_EDICT((*(entvars_t **)((char *)Data + PEV_Offset))->pContainingEntity)

#ifdef _WIN32
#define GetPrivateData(Type, Data, Offset, Diff)  (*(Type *)((char *)Data + Offset))
#define GetPrivateDataEx(Type, Data, Offset1, Offset2, Diff)  (*(Type *)((char *)Data + Offset1 + Offset2 * 4))
#define GetPrivateDataPointer(Type, Data, Offset, Diff)  (Type *)((char *)Data + Offset)
#else
#define GetPrivateData(Type, Data, Offset, Diff)  (*(Type *)((char *)Data + Offset + (Diff * 4)))
#define GetPrivateDataEx(Type, Data, Offset1, Offset2, Diff)  (*(Type *)((char *)Data + Offset1 + Offset2 * 4 + (Diff * 4)))
#define GetPrivateDataPointer(Type, Data, Offset, Diff)  (Type *)((char *)Data + Offset + (Diff * 4))
#endif

#define CUSTOM_WEAPON(BaseWeapon) GetPrivateData(int, BaseWeapon, CBaseEntity_Ammo338MAG, 5)
#define WEAPON_FID(BaseWeapon) GetPrivateData(int, BaseWeapon, CBaseEntity_Ammo57MM, 5)
#define WEAPON_KEY(BaseWeapon) GetPrivateData(int, BaseWeapon, CBaseEntity_AmmoBuckshot, 5)
#define WEAPON_CURBURST(BaseWeapon) GetPrivateData(int, BaseWeapon, CBaseEntity_Ammo556Nato, 5)

#define WEAPON_A2(BaseWeapon) GetPrivateData(int, BaseWeapon, CBaseEntity_MaxAmmo57MM, 5)
#define WEAPON_A2_OFFSET(BaseWeapon) GetPrivateData(int, BaseWeapon, CBaseEntity_Ammo9MM, 5)
#define WEAPON_INA2(BaseWeapon) GetPrivateData(int, BaseWeapon, CBaseEntity_MaxAmmo50AE, 5)
#define WEAPON_INA2_DELAY(BaseWeapon) GetPrivateData(int, BaseWeapon, CBaseEntity_MaxAmmo762Nato, 5)

#define WEAPON_INBURST(BaseWeapon) GetPrivateData(int, BaseWeapon, CBaseEntity_MaxAmmo556Nato, 5)
#define WEAPON_FLAGS(BaseWeapon) GetPrivateData(int, BaseWeapon, CBaseEntity_MaxAmmo338MAG, 5)
#define WEAPON_KEY_EX(BaseWeaponEnt) BaseWeaponEnt->v.iuser3
#define WEAPON_CLIP(BaseWeapon) GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_Clip, 4)
#define WEAPON_ID(BaseWeapon) GetPrivateData(int, BaseWeapon, CBasePlayerItem_ID, 4)
#define WEAPON_AMMO_TYPE(BaseWeapon) GetPrivateData(int, BaseWeapon, CBasePlayerWeapon_PrimaryAmmoType, 4)

#define PROJECTILE_TYPE(BaseEntity) GetPrivateData(int, BaseEntity, CBaseEntity_Ammo338MAG, 5)
#define PROJECTILE_FORWARD(BaseWeaponEnt) GetPrivateData(int, BaseEntity, CBaseEntity_MaxAmmo338MAG, 5)
#define EFFECT_TYPE(BaseEntity) GetPrivateData(int, BaseEntity, CBaseEntity_Ammo338MAG, 5)
#define EFFECT_FORWARD(BaseWeaponEnt) GetPrivateData(int, BaseEntity, CBaseEntity_MaxAmmo338MAG, 5)
#define EFFECT_EXPIRE_TIME(BaseEntity) EV_FROM_PD(BaseEntity)->fuser4
#define EFFECT_MAX_FRAMES(EntityVars) EntityVars->fuser3
#define EFFECT_LAST_TIME(EntityVars) EntityVars->fuser2


// CSWM.cpp

edict_t *GiveWeaponByName(edict_t *PlayerEdict, const char *Name);
edict_t *GiveWeapon(edict_t *PlayerEdict, int Index);
void UpdateAmmoList();
void SendWeaponAnim(CBasePlayerWeapon *BaseWeapon, int Anim);
cell ShootProjectileTimed(edict_t *LauncherEdict, int ProjectileID);
cell ShootProjectileContact(edict_t *LauncherEdict, int ProjectileID);
cell ShootEffect(edict_t *LauncherEdict, int EffectID);
int Player_GiveAmmoByID(CBasePlayer *BasePlayer, int AmmoID, int Amount);
void PlayerKnockback(edict_t *VictimEdict, Vector &Origin);
BOOL InViewCone(edict_t *PlayerEdict, Vector &Origin, BOOL Accurate);
BOOL InViewCone(Vector &SelfOrigin, Vector &VAngles, float FOV, Vector &Origin, BOOL Accurate);

// Attack2-CSWM.cpp

static void Attack2_Zoom(CBasePlayer *BasePlayer, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon);
static void Attack2_Switch(CBasePlayer *BasePlayer, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon);
static void Attack2_Burst(CBasePlayer *BasePlayer, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon);
static void Attack2_MultiShot(CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon);
static void Attack2_KnifeAttack(CBasePlayer *BasePlayer, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon);
static void Attack2_KnifeAttackPerform(CBasePlayerWeapon *BaseWeapon);
static void Attack2_InstaSwitch(edict_t *Playeredict, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon);
static void Attack2_ZoomCustom(CBasePlayer *BasePlayer, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon);

// Module.cpp

void LoadAmmos(void);
void LoadWeapons(void);
void SetAnimation(edict_t *PlayerEdict, int Animation, Activity ACT, float FrameRate);
void CheckAmmo(CAmmo &Ammo, int Index);
void CheckWeapon(CWeapon &Weapon);
void RecordWeaponDurationList(CWeapon &Weapon);

#endif