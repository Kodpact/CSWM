#pragma once

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
	A2_AutoPistol,
	A2_MultiShot,
	A2_KnifeAttack,
	A2_RadiusDamage,
	A2_Charge,
	A2_InstaSwitch,
	A2_Custom,
};

enum
{
	ICON_DISABLED,
	ICON_ENABLED,
	ICON_FLASH
};

struct CAutoPistol
{
	int Anim;
	float Delay;
	float Recoil;
};

struct A2V
{
	int IParamA;
	int IParamB;
	int IParamC;
	int IParamD;
	int IParamE;
	int IParamF;

	float FParamA;
	float FParamB;
	float FParamC;
	float FParamD;
	float FParamE;
	float FParamF;
	float FParamG;
	float FParamH;
	
	const char *SParamA;
	const char *SParamB;
};

#define WA2_ZOOM_MODE IParamA

#define WA2_SWITCH_ANIM_A IParamA
#define WA2_SWITCH_ANIM_B IParamB
#define WA2_SWITCH_ANIM_IDLE IParamC
#define WA2_SWITCH_ANIM_DRAW IParamD
#define WA2_SWITCH_ANIM_SHOOT IParamE
#define WA2_SWITCH_ANIM_RELOAD IParamF
#define WA2_SWITCH_ANIM_A_DURATION FParamA
#define WA2_SWITCH_ANIM_B_DURATION FParamB
#define WA2_SWITCH_ANIM_DRAW_DURATION FParamC
#define WA2_SWITCH_ANIM_SHOOT_DURATION FParamD
#define WA2_SWITCH_ANIM_RELOAD_DURATION FParamE
#define WA2_SWITCH_DELAY FParamF
#define WA2_SWITCH_DAMAGE FParamG
#define WA2_SWITCH_RECOIL FParamH
#define WA2_SWITCH_FSOUND SParamA

#define WA2_BURST_VALUE IParamA

#define WA2_MULTISHOT_VALUE IParamA

#define WA2_AUTOPISTOL_ANIM IParamA
#define WA2_AUTOPISTOL_DELAY FParamA
#define WA2_AUTOPISTOL_RECOIL FParamB

#define WA2_KNIFEATTACK_ANIMATION IParamA
#define WA2_KNIFEATTACK_DELAY FParamA
#define WA2_KNIFEATTACK_DURATION FParamB
#define WA2_KNIFEATTACK_RADIUS FParamC
#define WA2_KNIFEATTACK_DAMAGE_MIN IParamB
#define WA2_KNIFEATTACK_DAMAGE_MAX IParamC
#define WA2_KNIFEATTACK_KNOCKBACK FParamD
#define WA2_KNIFEATTACK_MULTI IParamD
#define WA2_KNIFEATTACK_SOUND SParamA

#define WA2_CHARGE_ANIM_A IParamA
#define WA2_CHARGE_ANIM_B IParamB
#define WA2_CHARGE_ANIM_C IParamC
#define WA2_CHARGE_ANIM_D IParamD
#define WA2_CHARGE_ANIM_A_CYCLE FParamA
#define WA2_CHARGE_ANIM_B_CYCLE FParamB
#define WA2_CHARGE_ANIM_C_CYCLE FParamC
#define WA2_CHARGE_ANIM_D_CYCLE FParamD
#define WA2_CHARGE_TIME FParamE
#define WA2_CHARGE_DAMAGE FParamF

#define WA2_INSTASWITCH_ANIM_SHOOT IParamA
#define WA2_INSTASWITCH_DELAY FParamA
#define WA2_INSTASWITCH_DAMAGE FParamB
#define WA2_INSTASWITCH_RECOIL FParamC
#define WA2_INSTASWITCH_NAME SParamA
#define WA2_INSTASWITCH_NAME2 SParamB

#include "CSWM.h"

static void Attack2_Zoom(CBasePlayer *BasePlayer, int ZoomType);
static void Attack2_Switch(CBasePlayer *BasePlayer, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon);
static void Attack2_Burst(CBasePlayer *BasePlayer, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon);
static void Attack2_MultiShot(CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon);
static void Attack2_KnifeAttack(CBasePlayer *BasePlayer, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon);
static void Attack2_KnifeAttackPerform(CBasePlayerWeapon *BaseWeapon);
static void Attack2_InstaSwitch(edict_t *Playeredict, CBasePlayerWeapon *BaseWeapon, CWeapon &Weapon);
