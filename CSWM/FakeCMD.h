#pragma once

struct FakeCMD
{
	char ArgS[256];
	const char *ArgV[3];
	int ArgC;
	bool Fake;
	bool Notify;
};

void UTIL_FakeClientCommand(edict_t *pedict, const char *Command, const char *Arg1 = NULL, const char *Arg2 = NULL);
