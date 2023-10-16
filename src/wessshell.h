#ifndef _FUNQUEUE_H
#define _FUNQUEUE_H

#include "doomlib.h"
#include <n_libaudio_sc.h>

extern ALPlayer wessnode;   // 800B4140
extern ALPlayer *wessstate; // 800B4154

extern ALMicroTime __wessVoiceHandler(void *node); // 8002F154
extern void SSP_SeqpNew(void) SEC_STARTUP; // 8002F100

#endif
