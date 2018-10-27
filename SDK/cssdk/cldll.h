#ifndef INCLUDE_CLDLL_H
#define INCLUDE_CLDLL_H

#define MAX_WEAPON_SLOTS		5		// hud item selection slots
#define MAX_ITEM_TYPES			6		// hud item selection slots

#define MAX_ITEMS			4		// hard coded item types

#define DEFAULT_FOV			90		// the default field of view

#define HIDEHUD_WEAPONS			(1<<0)
#define HIDEHUD_FLASHLIGHT		(1<<1)
#define HIDEHUD_ALL			(1<<2)
#define HIDEHUD_HEALTH			(1<<3)
#define HIDEHUD_TIMER			(1<<4)
#define HIDEHUD_MONEY			(1<<5)
#define HIDEHUD_CROSSHAIR		(1<<6)
#define HIDEHUD_OBSERVER_CROSSHAIR	(1<<7)

#define STATUSICON_HIDE			0
#define STATUSICON_SHOW			1
#define STATUSICON_FLASH		2

#define HUD_PRINTNOTIFY			1
#define HUD_PRINTCONSOLE		2
#define HUD_PRINTTALK			3
#define HUD_PRINTCENTER			4
#define HUD_PRINTRADIO			5

#define STATUS_NIGHTVISION_ON		1
#define STATUS_NIGHTVISION_OFF		0

#define ITEM_STATUS_NIGHTVISION		(1<<0)
#define ITEM_STATUS_DEFUSER		(1<<1)

#define SCORE_STATUS_DEAD		(1<<0)
#define SCORE_STATUS_BOMB		(1<<1)
#define SCORE_STATUS_VIP		(1<<2)

#define SIGNAL_BUY			(1<<0)
#define SIGNAL_BOMB			(1<<1)
#define SIGNAL_RESCUE			(1<<2)
#define SIGNAL_ESCAPE			(1<<3)
#define SIGNAL_VIPSAFETY		(1<<4)

// player data iuser3
#define PLAYER_CAN_SHOOT		(1<<0)
#define PLAYER_FREEZE_TIME_OVER		(1<<1)
#define PLAYER_IN_BOMB_ZONE		(1<<2)
#define PLAYER_HOLDING_SHIELD		(1<<3)

#define MENU_KEY_1			(1<<0)
#define MENU_KEY_2			(1<<1)
#define MENU_KEY_3			(1<<2)
#define MENU_KEY_4			(1<<3)
#define MENU_KEY_5			(1<<4)
#define MENU_KEY_6			(1<<5)
#define MENU_KEY_7			(1<<6)
#define MENU_KEY_8			(1<<7)
#define MENU_KEY_9			(1<<8)
#define MENU_KEY_0			(1<<9)

#define MAX_AMMO_SLOTS			32		// not really slots

#define HUD_PRINTNOTIFY			1
#define HUD_PRINTCONSOLE		2
#define HUD_PRINTTALK			3
#define HUD_PRINTCENTER			4

#define WEAPON_SUIT			31
#define WEAPON_ALLWEAPONS		(~(1 << WEAPON_SUIT))

// custom enum
enum VGUIMenu
{
	VGUI_Menu_Team = 2,
	VGUI_Menu_MapBriefing = 4,

	VGUI_Menu_Class_T = 26,
	VGUI_Menu_Class_CT,
	VGUI_Menu_Buy,
	VGUI_Menu_Buy_Pistol,
	VGUI_Menu_Buy_ShotGun,
	VGUI_Menu_Buy_Rifle,
	VGUI_Menu_Buy_SubMachineGun,
	VGUI_Menu_Buy_MachineGun,
	VGUI_Menu_Buy_Item,
};

// custom enum
enum VGUIMenuSlot
{
	VGUI_MenuSlot_Buy_Pistol = 1,
	VGUI_MenuSlot_Buy_ShotGun,
	VGUI_MenuSlot_Buy_SubMachineGun,
	VGUI_MenuSlot_Buy_Rifle,
	VGUI_MenuSlot_Buy_MachineGun,
	VGUI_MenuSlot_Buy_PrimAmmo,
	VGUI_MenuSlot_Buy_SecAmmo,
	VGUI_MenuSlot_Buy_Item,
};

#endif