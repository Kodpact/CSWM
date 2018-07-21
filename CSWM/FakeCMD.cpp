#include <Module.h>
#include <UTIL.h>

FakeCMD gFakeCMD;

const char *Cmd_Args(void)
{
	if (gFakeCMD.Fake)
		RETURN_META_VALUE(MRES_SUPERCEDE, (gFakeCMD.ArgC > 1) ? gFakeCMD.ArgS : gFakeCMD.ArgV[0]);

	RETURN_META_VALUE(MRES_IGNORED, NULL);
}

const char *Cmd_Argv(int ArgC)
{
	if (gFakeCMD.Fake)
		RETURN_META_VALUE(MRES_SUPERCEDE, (ArgC < 3) ? gFakeCMD.ArgV[ArgC] : "");

	RETURN_META_VALUE(MRES_IGNORED, NULL);
}

int	Cmd_Argc(void)
{
	if (gFakeCMD.Fake)
		RETURN_META_VALUE(MRES_SUPERCEDE, gFakeCMD.ArgC);

	RETURN_META_VALUE(MRES_IGNORED, 0);
}

void UTIL_FakeClientCommand(edict_t *pedict, const char *Command, const char *Arg1, const char *Arg2)
{
	if (!Command)
		return;

	gFakeCMD.ArgV[0] = Command;

	if (!Arg1 && Arg2)
	{
		Arg1 = Arg2;
		Arg2 = NULL;
	}

	if (Arg2)
	{
		gFakeCMD.ArgC = 3;
		gFakeCMD.ArgV[1] = Arg1;
		gFakeCMD.ArgV[2] = Arg2;
		sprintf_s(gFakeCMD.ArgS, sizeof(gFakeCMD.ArgS), "%s %s", Arg1, Arg2);
	}
	else if (Arg1)
	{
		gFakeCMD.ArgC = 2;
		gFakeCMD.ArgV[1] = Arg1;
		sprintf_s(gFakeCMD.ArgS, sizeof(gFakeCMD.ArgS), "%s", Arg1);
	}
	else
	{
		gFakeCMD.ArgC = 1;
	}

	gFakeCMD.Fake = true;
	MDLL_ClientCommand(pedict);
	gFakeCMD.Fake = false;
}