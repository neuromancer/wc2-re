/*
 *  Sets the mouse position.
 *
 *  Retail object 0x4199F0-0x419A3F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x4199F0 */
void SetPersonnelMousePosition(short x, short y)
{
    g_nPersonnelMouseX_005c8d00 = x;
    g_nPersonnelMouseY_005c8d02 = y;
    g_stQueuedMouse_005c83f0.x = x;
    g_stQueuedMouse_005c83f0.y = y;
    SetMouseHomePosition(x, y);
}
