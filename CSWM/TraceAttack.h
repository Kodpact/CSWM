#pragma once

enum
{
	TR_None,
	TR_LaserBeam
};

struct TRV
{
	int Time;
	int Size;
	int RMin;
	int RMax;
	int R;
	int G;
	int B;
};

void TraceAttack_Precache(void);
void TraceAttack_LaserBeam(int PlayerID, Vector End, int Time, int Size, int RMin, int RMax, int R, int G, int B);
