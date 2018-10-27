#ifndef INCLUDE_EDICT_H
#define INCLUDE_EDICT_H

#define	MAX_ENT_LEAFS 48

struct edict_s
{
	qboolean free;
	int serialnumber;
	link_t area;
	
	int headnode;
	int num_leafs;
	short leafnums[MAX_ENT_LEAFS];
	
	float freetime;
	
	void* pvPrivateData;
	
	entvars_t v;
};

typedef struct edict_s edict_t;

#endif