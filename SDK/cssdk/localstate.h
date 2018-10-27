#ifndef INCLUDE_LOCALSTATE_H
#define INCLUDE_LOCALSTATE_H

typedef struct local_state_s
{
	entity_state_t	playerstate;
	clientdata_t	client;
	weapon_data_t	weapondata[ 64 ];
} local_state_t;

#endif