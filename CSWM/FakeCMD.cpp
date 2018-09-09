#include <Module.h>

CFakeCMD FakeCMD;

const char *Cmd_Args(void)
{
	if (FakeCMD.Fake)
		RETURN_META_VALUE(MRES_SUPERCEDE, (FakeCMD.ArgC > 1) ? FakeCMD.ArgS : FakeCMD.ArgV[0]);

	RETURN_META_VALUE(MRES_IGNORED, NULL);
}

const char *Cmd_Argv(int ArgC)
{
	if (FakeCMD.Fake)
		RETURN_META_VALUE(MRES_SUPERCEDE, (ArgC < 3) ? FakeCMD.ArgV[ArgC] : "");

	RETURN_META_VALUE(MRES_IGNORED, NULL);
}

int	Cmd_Argc(void)
{
	if (FakeCMD.Fake)
		RETURN_META_VALUE(MRES_SUPERCEDE, FakeCMD.ArgC);

	RETURN_META_VALUE(MRES_IGNORED, 0);
}

void UTIL_FakeClientCommand(edict_t *PlayerEdict, const char *Command, const char *Arg1, const char *Arg2)
{
	if (!Command)
		return;

	FakeCMD.ArgV[0] = Command;

	if (!Arg1 && Arg2)
	{
		Arg1 = Arg2;
		Arg2 = NULL;
	}

	if (Arg2)
	{
		FakeCMD.ArgC = 3;
		FakeCMD.ArgV[1] = Arg1;
		FakeCMD.ArgV[2] = Arg2;
		sprintf_s(FakeCMD.ArgS, sizeof(FakeCMD.ArgS), "%s %s", Arg1, Arg2);
	}
	else if (Arg1)
	{
		FakeCMD.ArgC = 2;
		FakeCMD.ArgV[1] = Arg1;
		sprintf_s(FakeCMD.ArgS, sizeof(FakeCMD.ArgS), "%s", Arg1);
	}
	else
	{
		FakeCMD.ArgC = 1;
	}

	FakeCMD.Fake = TRUE;
	MDLL_ClientCommand(PlayerEdict);
	FakeCMD.Fake = FALSE;
}