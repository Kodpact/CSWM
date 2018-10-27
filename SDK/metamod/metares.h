#ifndef INCLUDE_METARES_H
#define INCLUDE_METARES_H

#define SET_META_RESULT(result)			META_GLOBALS->mres=result
#define RETURN_META(result) { META_GLOBALS->mres=result; return; }
#define RETURN_META_VALUE(result, value) { META_GLOBALS->mres=result; return(value); }
#define META_RESULT_STATUS				META_GLOBALS->status
#define META_RESULT_PREVIOUS			META_GLOBALS->prev_mres
#define META_RESULT_ORIG_RET(type)		*(type *)META_GLOBALS->orig_ret
#define META_RESULT_OVERRIDE_RET(type)	*(type *)META_GLOBALS->override_ret

typedef enum
{
	MRES_UNSET = 0,
	MRES_IGNORED,		// plugin didn't take any action
	MRES_HANDLED,		// plugin did something, but real function should still be called
	MRES_OVERRIDE,		// call real function, but use my return value
	MRES_SUPERCEDE,		// skip real function; use my return value
} META_RES;

#endif