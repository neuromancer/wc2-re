/*
 *  Keyboard mouse pump.
 *
 *  Retail object 0x452450-0x45292F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x452450 */
/* Drive the mouse cursor from the numeric keypad when keyboard-mouse mode is
 * on: the arrows and corners move it by the current step, NumLock resets the
 * step and +/- change it, Shift doubles it, and Return and Space stand in for
 * the two buttons. */
void KeyboardMousePump(void)
{
    short deltaX;
    short deltaY;
    short buttons;
    short step;
    short x;
    short y;
    short left;
    short right;
    short top;
    short bottom;
    short wasDown;
    short isDown;

    deltaX = 0;
    deltaY = 0;
    buttons = 0;
    if (g_bKeyboardMouseEnabled_0049be68 == 0)
        return;

    step = g_nInputDeviceMode_005c83e6;
    if (GetShiftKeyState() != 0)
        step = (short)(step + step);
    if (GetNumLockKeyState() != 0)
        g_nInputDeviceMode_005c83e6 = 1;
    if (GetNumpadPlusKeyState() != 0 && g_nInputDeviceMode_005c83e6 < 10)
        g_nInputDeviceMode_005c83e6++;
    if (GetNumpadMinusKeyState() != 0 && g_nInputDeviceMode_005c83e6 > 1)
        g_nInputDeviceMode_005c83e6--;
    if (GetReturnKeyState() != 0)
        buttons |= 4;
    if (GetSpaceKeyState() != 0)
        buttons |= 2;
    if (GetLeftArrowKeyState() != 0)
        deltaX = (short)-step;
    if (GetRightArrowKeyState() != 0)
        deltaX = (short)(step + deltaX);
    if (GetUpArrowKeyState() != 0)
        deltaY = (short)-step;
    if (GetDownArrowKeyState() != 0)
        deltaY = (short)(step + deltaY);
    if (GetHomeKeyState() != 0) {
        deltaX = (short)(deltaX - step);
        deltaY = (short)(deltaY - step);
    }
    if (GetPageUpKeyState() != 0) {
        deltaX = (short)(step + deltaX);
        deltaY = (short)(deltaY - step);
    }
    if (GetEndKeyState() != 0) {
        deltaX = (short)(deltaX - step);
        deltaY = (short)(step + deltaY);
    }
    if (GetPageDownKeyState() != 0) {
        deltaX = (short)(step + deltaX);
        deltaY = (short)(step + deltaY);
    }

    if ((deltaX != 0 || deltaY != 0) &&
        g_stScreenViewport_005d21a0.pixels != 0) {
        x = (short)(g_nQueuedInputX_005c83f0 + deltaX);
        y = (short)(g_nQueuedInputY_005c83f2 + deltaY);
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

    if (buttons == g_nKeyboardMouseButtons_005b3648)
        return;

    wasDown = (short)(g_nKeyboardMouseButtons_005b3648 & 2);
    isDown = (short)(buttons & 2);
    if (wasDown == 0 && isDown != 0) {
        buttons &= 4;
        QueueInputEvent(1, g_nQueuedInputX_005c83f0,
                        g_nQueuedInputY_005c83f2, 0, 1,
                        buttons, 0, 0, 0);
        SetPersonnelCursorPosition(g_nQueuedInputX_005c83f0,
                                   g_nQueuedInputY_005c83f2, 0, 0);
    } else if (wasDown != 0 && isDown == 0) {
        buttons &= 4;
        QueueInputEvent(2, g_nQueuedInputX_005c83f0,
                        g_nQueuedInputY_005c83f2, 0, 0,
                        buttons, 0, 0, 0);
    }

    wasDown = (short)(g_nKeyboardMouseButtons_005b3648 & 4);
    isDown = (short)(buttons & 4);
    if (wasDown == 0 && isDown != 0) {
        buttons &= 2;
        QueueInputEvent(1, g_nQueuedInputX_005c83f0,
                        g_nQueuedInputY_005c83f2, 0,
                        buttons, 1, 0, 0, 0);
        SetPersonnelCursorPosition(g_nQueuedInputX_005c83f0,
                                   g_nQueuedInputY_005c83f2, 0, 0);
    } else if (wasDown != 0 && isDown == 0) {
        buttons &= 2;
        QueueInputEvent(2, g_nQueuedInputX_005c83f0,
                        g_nQueuedInputY_005c83f2, 0,
                        buttons, 0, 0, 0, 0);
    }
    g_nKeyboardMouseButtons_005b3648 = buttons;
}
