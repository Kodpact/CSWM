#ifndef INCLUDE_METAGLOBALS_H
#define INCLUDE_METAGLOBALS_H

typedef struct meta_globals_s
{
	META_RES mres;			// writable; plugin's return flag
	META_RES prev_mres;		// readable; return flag of the previous plugin called
	META_RES status;		// readable; "highest" return flag so far
	void *orig_ret;			// readable; return value from "real" function
	void *override_ret;		// readable; return value from overriding/superceding plugin
} meta_globals_t;

#endif