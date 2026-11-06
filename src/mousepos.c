/*
 *  Sets the mouse position.
 *
 *  Retail object 0x4199F0-0x419A3F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x4199F0 */
void SetPersonnelMousePosition(short x, short y)
{
    g_6_stPersonnelMouseData.x = x;
    g_6_stPersonnelMouseData.y = y;
    g_16_stEventQueuePointerData.x = x;
    g_16_stEventQueuePointerData.y = y;
    SetMouseHomePosition(x, y);
}
