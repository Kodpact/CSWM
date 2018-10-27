#ifndef INCLUDE_CLIENTTEXTMESSAGE_H
#define INCLUDE_CLIENTTEXTMESSAGE_H

typedef struct client_textmessage_s
{
	int		effect;
	byte	r1, g1, b1, a1;
	byte	r2, g2, b2, a2;
	float	x;
	float	y;
	float	fadein;
	float	fadeout;
	float	holdtime;
	float	fxtime;
	const char *pName;
	const char *pMessage;
} client_textmessage_t;

#endif