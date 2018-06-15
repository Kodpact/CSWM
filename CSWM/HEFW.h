#pragma once

void HookEntityFW(const char ClassName[], int Offset, void *Function, void *&Forward);
void HookEntityFWByVTable(void **VTable, int Offset, void *Function, void *&Forward);
void ResetEntityFW(const char ClassName[], int Offset, void *Function, void *Forward);
void ResetEntityFWByVTable(void **VTable, int Offset, void *Function, void *&Forward);
void *GetEntityFW(const char ClassName[], int Offset);

#include "CStrike.h"

/*===========================

pev 4
base 0x0

spawn 0
precache 1
keyvalue 3
objectcaps 6
activate 7
setobjectcollisionbox 8
classify 9
deathnotice 10
traceattack 11
takedamage 12
takehealth 13
killed 14
bloodcolor 15
tracebleed 16
istriggered 17
mymonsterpointer 18
mysquadmonsterpointer 19
gettogglestate 20
addpoints 21
addpointstoteam 22
addplayeritem 23
removeplayeritem 24
giveammo 25
getdelay 26
ismoving 27
overridereset 28
damagedecal 29
settogglestate 30
startsneaking 31
stopsneaking 32
oncontrols 33
issneaking 34
isalive 35
isbspmodel 36
reflectgauss 37
hastarget 38
isinworld 39
isplayer 40
isnetclient 41
teamid 42
getnexttarget 43
think 44
touch 45
use 46
blocked 47
respawn 48
updateowner 49
fbecomeprone 50
center 51
eyeposition 52
earposition 53
bodytarget 54
illumination 55
fvecvisible 56
fvisible 57
changeyaw 59
hashumangibs 60
hasaliengibs 61
fademonster 62
gibmonster 63
getdeathactivity 64
becomedead 65
irelationship 67
painsound 68
reportaistate 70
monsterinitdead 71
look 72
bestvisibleenemy 73
finviewcone 75
fvecinviewcone 74

player_jump 76
player_duck 77
player_prethink 78
player_postthink 79
player_getgunposition 80
player_shouldfadeondeath 66
player_impulsecommands 83
player_updateclientdata 82

item_addtoplayer 59
item_addduplicate 60
item_getiteminfo 61
item_candeploy 62
item_deploy 64
item_canholster 66
item_holster 67
item_updateiteminfo 68
item_preframe 69
item_postframe 70
item_drop 71
item_kill 72
item_attachtoplayer 73
item_primaryammoindex 74
item_secondaryammoindex 75
item_updateclientdata 76
item_getweaponptr 77
item_itemslot 79

weapon_extractammo 80
weapon_extractclipammo 81
weapon_addweapon 82
weapon_playemptysound 83
weapon_resetemptysound 84
weapon_isusable 86
weapon_primaryattack 87
weapon_secondaryattack 88
weapon_reload 89
weapon_weaponidle 90
weapon_retireweapon 91
weapon_shouldweaponidle 92
weapon_usedecrement 93

cstrike_restart 2
cstrike_roundrespawn 84
cstrike_item_candrop 63
cstrike_item_isweapon 65
cstrike_item_getmaxspeed 78
cstrike_weapon_sendweaponanim 85
cstrike_player_resetmaxspeed 69
cstrike_player_isbot 81
cstrike_player_getautoaimvector 85
cstrike_player_blind 86
cstrike_player_ontouchingweapon 87

=========================== */