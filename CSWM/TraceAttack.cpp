#include "Module.h"

int LaserBeamIndex;

void TraceAttack_Precache(void)
{
	LaserBeamIndex = PRECACHE_MODEL("sprites/zbeam4.spr");// PRECACHE_MODEL("sprites/laserbeam.spr");
}

void TraceAttack_LaserBeam(int PlayerID, Vector EndPos, int Time, int Size, int RMin, int RMax, int R, int G, int B)
{
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, nullptr, nullptr);
	WRITE_BYTE(TE_BEAMENTPOINT);
	WRITE_SHORT(PlayerID | 0x1000);
	WRITE_COORD(EndPos.x);
	WRITE_COORD(EndPos.y);
	WRITE_COORD(EndPos.z);
	WRITE_SHORT(LaserBeamIndex);
	WRITE_BYTE(1);
	WRITE_BYTE(5);
	WRITE_BYTE(Time);
	WRITE_BYTE(Size);
	WRITE_BYTE(RANDOM_LONG(RMin, RMax));
	WRITE_BYTE(R);
	WRITE_BYTE(G);
	WRITE_BYTE(B);
	WRITE_BYTE(200);
	WRITE_BYTE(0);
	MESSAGE_END();
}

