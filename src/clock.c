/*
 *  Splits game clock ticks.
 *
 *  Retail object 0x437760-0x4377EF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x437760 */
void SplitGameClockTicks(unsigned char *parts)
{
    int ticks = (int)GetGameClockTicks();

    parts[0] = (unsigned char)(ticks % 60);
    ticks = ticks / 60;
    parts[1] = (unsigned char)(ticks % 60);
    ticks = ticks / 60;
    parts[2] = (unsigned char)(ticks % 60);
    ticks = ticks / 60;
    parts[3] = (unsigned char)(ticks % 24);
}
