/*
 *  Keyboard mouse pump.
 *
 *  Retail object 0x452450-0x45292F (link order; see docs/ORDER.md).
 */
#include "game.h"

int g_bKeyboardMouseEnabled_0049be68 = 0;

/* Function start: 0x452450 */
/* Drive the mouse cursor from the numeric keypad when keyboard-mouse mode is
 * on: the arrows and corners move it by the current speed, NumLock resets the
 * speed and +/- change it, Shift doubles it, and Return and Space stand in for
 * the two buttons. */
void KeyboardMousePump(void)
{
    short dx;
    short buttons;
    short speed;
    short right;
    short dy;
    short x;
    short y;
    short left;
    short top;
    short bottom;

    dx = 0;
    dy = 0;
    buttons = 0;
    if (g_bKeyboardMouseEnabled_0049be68 == 0)
        return;

    speed = g_cKeypadMouseSpeed_005c83e6;
    if (GetShiftKeyState() != 0)
        speed *= 2;
    if (GetNumLockKeyState() != 0)
        g_cKeypadMouseSpeed_005c83e6 = 1;
    if (GetNumpadPlusKeyState() != 0 && g_cKeypadMouseSpeed_005c83e6 < 10)
        g_cKeypadMouseSpeed_005c83e6++;
    if (GetNumpadMinusKeyState() != 0 && g_cKeypadMouseSpeed_005c83e6 > 1)
        g_cKeypadMouseSpeed_005c83e6--;
    if (GetReturnKeyState() != 0)
        buttons |= 4;
    if (GetSpaceKeyState() != 0)
        buttons |= 2;
    if (GetLeftArrowKeyState() != 0)
        dx = (short)-speed;
    if (GetRightArrowKeyState() != 0)
        dx = (short)(speed + dx);
    if (GetUpArrowKeyState() != 0)
        dy = (short)-speed;
    if (GetDownArrowKeyState() != 0)
        dy = (short)(speed + dy);
    if (GetHomeKeyState() != 0) {
        dx = (short)(dx - speed);
        dy = (short)(dy - speed);
    }
    if (GetPageUpKeyState() != 0) {
        dx = (short)(speed + dx);
        dy = (short)(dy - speed);
    }
    if (GetEndKeyState() != 0) {
        dx = (short)(dx - speed);
        dy = (short)(speed + dy);
    }
    if (GetPageDownKeyState() != 0) {
        dx = (short)(speed + dx);
        dy = (short)(speed + dy);
    }

    if ((dx != 0 || dy != 0) &&
        g_stScreenViewport_005d21a0.pixels != 0) {
        x = (short)(g_nQueuedInputX_005c83f0 + dx);
        y = (short)(g_nQueuedInputY_005c83f2 + dy);
        left = g_stScreenViewport_005d21a0.left;
        right = g_stScreenViewport_005d21a0.right;
        top = g_stScreenViewport_005d21a0.top;
        bottom = g_stScreenViewport_005d21a0.bottom;
        if (left > x)
            x = left;
        else if (right < x)
            x = right;
        if (top > y)
            y = top;
        else if (bottom < y)
            y = bottom;
        g_nQueuedInputX_005c83f0 = x;
        g_nPersonnelMouseX_005c8d00 = g_nQueuedInputX_005c83f0;
        g_nQueuedInputY_005c83f2 = y;
        g_nPersonnelMouseY_005c8d02 = g_nQueuedInputY_005c83f2;
        SetPersonnelCursorPosition(g_nQueuedInputX_005c83f0,
                                   g_nQueuedInputY_005c83f2, 0, 0);
        g_bSuppressNextMouseMove_005c843c = 1;
        SetMousePosition(x, y);
    }

    if (buttons != g_nKeyboardMouseButtons_005b3648) {
        dx = (short)(g_nKeyboardMouseButtons_005b3648 & 2);
        dy = (short)(buttons & 2);
        if (dx == 0 && dy != 0) {
            buttons &= 4;
            QueueInputEvent(1, g_nQueuedInputX_005c83f0,
                            g_nQueuedInputY_005c83f2, 0, 1,
                            buttons, 0, 0, 0);
            SetPersonnelCursorPosition(g_nQueuedInputX_005c83f0,
                                       g_nQueuedInputY_005c83f2, 0, 0);
        } else if (dx != 0 && dy == 0) {
            buttons &= 4;
            QueueInputEvent(2, g_nQueuedInputX_005c83f0,
                            g_nQueuedInputY_005c83f2, 0, 0,
                            buttons, 0, 0, 0);
        }

        dx = (short)(g_nKeyboardMouseButtons_005b3648 & 4);
        dy = (short)(buttons & 4);
        if (dx == 0 && dy != 0) {
            buttons &= 2;
            QueueInputEvent(1, g_nQueuedInputX_005c83f0,
                            g_nQueuedInputY_005c83f2, 0,
                            buttons, 1, 0, 0, 0);
            SetPersonnelCursorPosition(g_nQueuedInputX_005c83f0,
                                       g_nQueuedInputY_005c83f2, 0, 0);
        } else if (dx != 0 && dy == 0) {
            buttons &= 2;
            QueueInputEvent(2, g_nQueuedInputX_005c83f0,
                            g_nQueuedInputY_005c83f2, 0,
                            buttons, 0, 0, 0, 0);
        }
    }
    g_nKeyboardMouseButtons_005b3648 = buttons;
}
