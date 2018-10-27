#ifndef INCLUDE_UTIL_H
#define INCLUDE_UTIL_H

#define FStrEq(x,y) (strcmp(x,y) == 0)
#define ENT(x) x->pContainingEntity
#define EMIT_SOUND_SHORT(ent, chan, snd) EMIT_SOUND(ent, chan, snd, VOL_NORM, ATTN_NORM, 0, PITCH_NORM)
#define STOP_SOUND(ent, chan, snd) EMIT_SOUND(ent, chan, snd, 0, 0, SND_STOP, PITCH_NORM)

#endif