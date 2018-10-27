#ifndef INCLUDE_HUDTEXTPARMS_H
#define INCLUDE_HUDTEXTPARMS_H

typedef struct hudtextparms_s
{
	float x;
	float y;
	int effect;
	byte r1,g1,b1,a1;
	byte r2,g2,b2,a2;
	float fadeinTime;
	float fadeoutTime;
	float holdTime;
	float fxTime;
	int channel;

} hudtextparms_t;

#endif