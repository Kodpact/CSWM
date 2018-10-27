#ifndef INCLUDE_LOCKSOUND_H
#define INCLUDE_LOCKSOUND_H

typedef struct locksounds
{
	string_t sLockedSound;
	string_t sLockedSentence;
	string_t sUnlockedSound;
	string_t sUnlockedSentence;
	int iLockedSentence;
	int iUnlockedSentence;
	float flwaitSound;
	float flwaitSentence;
	byte bEOFLocked;
	byte bEOFUnlocked;

} locksound_t;

#endif