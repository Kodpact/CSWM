#ifndef INCLUDE_PLAYER_H
#define INCLUDE_PLAYER_H

#include <string.h>

#define MIN_BUY_TIME				15	// the minimum threshold values for cvar mp_buytime 15 sec's
#define MAX_BUFFER_MENU				175
#define MAX_BUFFER_MENU_BRIEFING	50
#define MAX_PLAYER_NAME_LENGTH		32
#define MAX_AUTOBUY_LENGTH			256
#define MAX_REBUY_LENGTH			256
#define MAX_LOCATION_LENGTH			32
#define MAX_RECENT_PATH				20
#define MAX_HOSTAGE_ICON			4	// the maximum number of icons of the hostages in the HUD

#define SUITUPDATETIME 	3.5
#define SUITFIRSTUPDATETIME 0.1

#define PLAYER_FATAL_FALL_SPEED 1100.0f
#define PLAYER_MAX_SAFE_FALL_SPEED	500.0f
#define PLAYER_USE_RADIUS 64.0f

#define ARMOR_RATIO 	0.5	// Armor Takes 50% of the damage
#define ARMOR_BONUS 	0.5	// Each Point of Armor is work 1/x points of health

#define FLASH_DRAIN_TIME 1.2	// 100 units/3 minutes
#define FLASH_CHARGE_TIME 0.2	// 100 units/20 seconds  (seconds per unit)

// damage per unit per second.
#define DAMAGE_FOR_FALL_SPEED 100.0f / (PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED)
#define PLAYER_MIN_BOUNCE_SPEED 350.0f

// won't punch player's screen/make scrape noise unless player falling at least this fast.
#define PLAYER_FALL_PUNCH_THRESHHOLD	250.0f

// Money blinks few of times on the freeze period
// NOTE: It works for CZ
#define MONEY_BLINK_AMOUNT 30

// Player physics flags bits
// CBasePlayer::m_afPhysicsFlags
#define PFLAG_ONLADDER 	(1<<0)
#define PFLAG_ONSWING 	(1<<0)
#define PFLAG_ONTRAIN 	(1<<1)
#define PFLAG_ONBARNACLE (1<<2)
#define PFLAG_DUCKING 	(1<<3)	// In the process of ducking, but totally squatted yet
#define PFLAG_USING 	(1<<4)	// Using a continuous entity
#define PFLAG_OBSERVER 	(1<<5)	// player is locked in stationary cam mode. Spectators can move, observers can't.

#define TRAIN_OFF 	0x00
#define TRAIN_NEUTRAL 	0x01
#define TRAIN_SLOW 	0x02
#define TRAIN_MEDIUM 	0x03
#define TRAIN_FAST 	0x04
#define TRAIN_BACK 	0x05

#define TRAIN_ACTIVE 	0x80
#define TRAIN_NEW 	0xc0

#define SIGNAL_BUY 	(1<<0)
#define SIGNAL_BOMB 	(1<<1)
#define SIGNAL_RESCUE 	(1<<2)
#define SIGNAL_ESCAPE 	(1<<3)
#define SIGNAL_VIPSAFETY (1<<4)

#define IGNOREMSG_NONE 	0
#define IGNOREMSG_ENEMY 	1
#define IGNOREMSG_TEAM 	2

// max of 4 suit sentences queued up at any time
#define CSUITPLAYLIST 	4

#define SUIT_GROUP 	TRUE
#define SUIT_SENTENCE 	FALSE

#define SUIT_REPEAT_OK 	0
#define SUIT_NEXT_IN_30SEC 30
#define SUIT_NEXT_IN_1MIN 60
#define SUIT_NEXT_IN_5MIN 300
#define SUIT_NEXT_IN_10MIN 600
#define SUIT_NEXT_IN_30MIN 1800
#define SUIT_NEXT_IN_1HOUR 3600

#define TEAM_NAME_LENGTH 16

#define MAX_ID_RANGE 	2048.0f
#define MAX_SPECTATOR_ID_RANGE 8192.0f
#define SBAR_STRING_SIZE 128

#define SBAR_TARGETTYPE_TEAMMATE	1
#define SBAR_TARGETTYPE_ENEMY 2
#define SBAR_TARGETTYPE_HOSTAGE 3

#define CHAT_INTERVAL 	1.0f
#define CSUITNOREPEAT 	32

#define AUTOAIM_2DEGREES 0.0348994967025f
#define AUTOAIM_5DEGREES 0.08715574274766f
#define AUTOAIM_8DEGREES 0.1391731009601f
#define AUTOAIM_10DEGREES 0.1736481776669f

#define SOUND_FLASHLIGHT_ON "items/flashlight1.wav"
#define SOUND_FLASHLIGHT_OFF "items/flashlight1.wav"

// custom enum
enum RewardType
{
	RT_NONE,
	RT_ROUND_BONUS,
	RT_PLAYER_RESET,
	RT_PLAYER_JOIN,
	RT_PLAYER_SPEC_JOIN,
	RT_PLAYER_BOUGHT_SOMETHING,
	RT_HOSTAGE_TOOK,
	RT_HOSTAGE_RESCUED,
	RT_HOSTAGE_DAMAGED,
	RT_HOSTAGE_KILLED,
	RT_TEAMMATES_KILLED,
	RT_ENEMY_KILLED,
	RT_INTO_GAME,
	RT_VIP_KILLED,
	RT_VIP_RESCUED_MYSELF
};

enum PLAYER_ANIM
{
	PLAYER_IDLE,
	PLAYER_WALK,
	PLAYER_JUMP,
	PLAYER_SUPERJUMP,
	PLAYER_DIE,
	PLAYER_ATTACK1,
	PLAYER_ATTACK2,
	PLAYER_FLINCH,
	PLAYER_LARGE_FLINCH,
	PLAYER_RELOAD,
	PLAYER_HOLDBOMB
};

enum _Menu
{
	Menu_OFF,
	Menu_ChooseTeam,
	Menu_IGChooseTeam,
	Menu_ChooseAppearance,
	Menu_Buy,
	Menu_BuyPistol,
	Menu_BuyRifle,
	Menu_BuyMachineGun,
	Menu_BuyShotgun,
	Menu_BuySubMachineGun,
	Menu_BuyItem,
	Menu_Radio1,
	Menu_Radio2,
	Menu_Radio3,
	Menu_ClientBuy
};

enum TeamName
{
	UNASSIGNED,
	TERRORIST,
	CT,
	SPECTATOR,
};

enum ModelName
{
	MODEL_UNASSIGNED,
	MODEL_URBAN,
	MODEL_TERROR,
	MODEL_LEET,
	MODEL_ARCTIC,
	MODEL_GSG9,
	MODEL_GIGN,
	MODEL_SAS,
	MODEL_GUERILLA,
	MODEL_VIP,
	MODEL_MILITIA,
	MODEL_SPETSNAZ,
	MODEL_AUTO
};

enum JoinState
{
	JOINED,
	SHOWLTEXT,
	READINGLTEXT,
	SHOWTEAMSELECT,
	PICKINGTEAM,
	GETINTOGAME
};

enum TrackCommands
{
	CMD_SAY = 0,
	CMD_SAYTEAM,
	CMD_FULLUPDATE,
	CMD_VOTE,
	CMD_VOTEMAP,
	CMD_LISTMAPS,
	CMD_LISTPLAYERS,
	CMD_NIGHTVISION,
	COMMANDS_TO_TRACK,
};

struct RebuyStruct
{
	int m_primaryWeapon;
	int m_primaryAmmo;
	int m_secondaryWeapon;
	int m_secondaryAmmo;
	int m_heGrenade;
	int m_flashbang;
	int m_smokeGrenade;
	int m_defuser;
	int m_nightVision;
	ArmorType m_armor;
};

enum ThrowDirection
{
	THROW_NONE,
	THROW_FORWARD,
	THROW_BACKWARD,
	THROW_HITVEL,
	THROW_BOMB,
	THROW_GRENADE,
	THROW_HITVEL_MINUS_AIRVEL
};

enum sbar_data
{
	SBAR_ID_TARGETTYPE = 1,
	SBAR_ID_TARGETNAME,
	SBAR_ID_TARGETHEALTH,
	SBAR_END
};

enum MusicState { SILENT, CALM, INTENSE };

class CCSPlayer;

class CStripWeapons;
class CInfoIntermission;
class CDeadHEV;
class CSprayCan;
class CBloodSplat;
class CBasePlayer;
class CWShield;

#endif