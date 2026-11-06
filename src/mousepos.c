/*
 *  Sets the mouse position.
 *
 *  Retail object 0x4199F0-0x419A3F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x4199F0 */
void SetPersonnelMousePosition(short x, short y)
{
    g_stPersonnelMouseData_005c8d00.x = x;
    g_stPersonnelMouseData_005c8d00.y = y;
    g_stEventQueuePointerData_005c83f0.x = x;
    g_stEventQueuePointerData_005c83f0.y = y;
    SetMouseHomePosition(x, y);
}
