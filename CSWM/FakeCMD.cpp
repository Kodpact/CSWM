#include <Module.h>

char ArgS[256];
const char *ArgV[3];
int ArgC;
BOOL Fake;

const char *Cmd_Args(void)
{
	if (Fake)
		RETURN_META_VALUE(MRES_SUPERCEDE, (ArgC > 1) ? ArgS : ArgV[0]);

	RETURN_META_VALUE(MRES_IGNORED, NULL);
}

const char *Cmd_Argv(int ArgC)
{
	if (Fake)
		RETURN_META_VALUE(MRES_SUPERCEDE, (ArgC < 3) ? ArgV[ArgC] : "");

	RETURN_META_VALUE(MRES_IGNORED, NULL);
}

int	Cmd_Argc(void)
{
	if (Fake)
		RETURN_META_VALUE(MRES_SUPERCEDE, ArgC);

	RETURN_META_VALUE(MRES_IGNORED, 0);
}

void UTIL_FakeClientCommand(edict_t *PlayerEdict, const char *Command, const char *Arg1, const char *Arg2)
{
	ArgV[0] = Command;

	if (Arg2)
	{
		ArgC = 3;
		ArgV[1] = Arg1;
		ArgV[2] = Arg2;
		sprintf(ArgS, "%s %s", Arg1, Arg2);
	}
	else if (Arg1)
	{
		ArgC = 2;
		ArgV[1] = Arg1;
		sprintf(ArgS, "%s", Arg1);
	}
	else
	{
		ArgC = 1;
	}

	Fake = TRUE;
	MDLL_ClientCommand(PlayerEdict);
	Fake = FALSE;
}