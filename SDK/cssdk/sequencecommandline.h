#ifndef INCLUDE_SEQUENCECOMMANDLINE_H
#define INCLUDE_SEQUENCECOMMANDLINE_H

typedef struct sequenceCommandLine sequenceCommandLine_t;

struct sequenceCommandLine
{
	int commandType;
	client_textmessage_t clientMessage;
	char* speakerName;
	char* listenerName;
	char* soundFileName;
	char* sentenceName;
	char* fireTargetNames;
	char* killTargetNames;
	float delay;
	int repeatCount;
	int textChannel;
	int modifierBitField;
	sequenceCommandLine_t*	nextCommandLine;
};

#endif