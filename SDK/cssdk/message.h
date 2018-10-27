#ifndef INCLUDE_MESSAGE_H
#define INCLUDE_MESSAGE_H

#define MSG_BROADCAST		0		// unreliable to all
#define MSG_ONE				1		// reliable to one (msg_entity)
#define MSG_ALL				2		// reliable to all
#define MSG_INIT			3		// write to the init string
#define MSG_PVS				4		// Ents in PVS of org
#define MSG_PAS				5		// Ents in PAS of org
#define MSG_PVS_R			6		// Reliable to PVS
#define MSG_PAS_R			7		// Reliable to PAS
#define MSG_ONE_UNRELIABLE	8		// Send to one client, but don't put in reliable stream, put in unreliable datagram ( could be dropped )
#define MSG_SPEC			9		// Sends to all spectator proxies

#endif