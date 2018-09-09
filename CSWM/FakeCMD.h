#pragma once

struct CFakeCMD
{
	char ArgS[256];
	const char *ArgV[3];
	int ArgC;
	BOOL Fake;
	BOOL Notify;
};

void UTIL_FakeClientCommand(edict_t *PlayerEdict, const char *Command, const char *Arg1 = NULL, const char *Arg2 = NULL);
