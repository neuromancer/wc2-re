/*
 *  Waits for a queued input press.
 *
 *  Retail object 0x465730-0x46577F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x465730 */
short WaitForQueuedInputPress(void)
{
    int savedQueueState;
    short key;

    savedQueueState = g_bInputEventQueueEnabled;
    g_bInputEventQueueEnabled = 1;
    while ((key = (short)WaitForAnyInputPress()) == 0)
        ;
    g_bInputEventQueueEnabled = savedQueueState;
    return key;
}
