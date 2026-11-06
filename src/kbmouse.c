/*
 *  Keyboard mouse pump.
 *
 *  Retail object 0x452450-0x45292F (link order; see docs/ORDER.md).
 */
/* This unit owns storage declared in globals.h as statics. */
#define KBMOUSE_C
#include "game.h"

int g_bKeyboardMouseEnabled = 0; /* 0x0049be68 */

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
    if (g_bKeyboardMouseEnabled == 0)
        return;

    speed = g_cKeypadMouseSpeedValue;
    if (GetShiftKeyState() != 0)
        speed *= 2;
    if (GetNumLockKeyState() != 0)
        g_cKeypadMouseSpeedValue = 1;
    if (GetNumpadPlusKeyState() != 0 && g_cKeypadMouseSpeedValue < 10)
        g_cKeypadMouseSpeedValue++;
    if (GetNumpadMinusKeyState() != 0 && g_cKeypadMouseSpeedValue > 1)
        g_cKeypadMouseSpeedValue--;
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
        g_101_stScrViewInfo.pixels != 0) {
        x = (short)(g_16_stEventQueuePointerData.x + dx);
        y = (short)(g_16_stEventQueuePointerData.y + dy);
        left = g_101_stScrViewInfo.left;
        right = g_101_stScrViewInfo.right;
        top = g_101_stScrViewInfo.top;
        bottom = g_101_stScrViewInfo.bottom;
        if (left > x)
            x = left;
        else if (right < x)
            x = right;
        if (top > y)
            y = top;
        else if (bottom < y)
            y = bottom;
        g_16_stEventQueuePointerData.x = x;
        g_6_stPersonnelMouseData.x = g_16_stEventQueuePointerData.x;
        g_16_stEventQueuePointerData.y = y;
        g_6_stPersonnelMouseData.y = g_16_stEventQueuePointerData.y;
        SetPersonnelCursorPosition(g_16_stEventQueuePointerData.x,
                                   g_16_stEventQueuePointerData.y, 0, 0);
        g_2_bIgnoreNextCursorMoveFlag = 1;
        SetMousePosition(x, y);
    }

    if (buttons != g_nKeyboardMouseButtons) {
        dx = (short)(g_nKeyboardMouseButtons & 2);
        dy = (short)(buttons & 2);
        if (dx == 0 && dy != 0) {
            buttons &= 4;
            QueueInputEvent(1, g_16_stEventQueuePointerData.x,
                            g_16_stEventQueuePointerData.y, 0, 1,
                            buttons, 0, 0, 0);
            SetPersonnelCursorPosition(g_16_stEventQueuePointerData.x,
                                       g_16_stEventQueuePointerData.y, 0, 0);
        } else if (dx != 0 && dy == 0) {
            buttons &= 4;
            QueueInputEvent(2, g_16_stEventQueuePointerData.x,
                            g_16_stEventQueuePointerData.y, 0, 0,
                            buttons, 0, 0, 0);
        }

        dx = (short)(g_nKeyboardMouseButtons & 4);
        dy = (short)(buttons & 4);
        if (dx == 0 && dy != 0) {
            buttons &= 2;
            QueueInputEvent(1, g_16_stEventQueuePointerData.x,
                            g_16_stEventQueuePointerData.y, 0,
                            buttons, 1, 0, 0, 0);
            SetPersonnelCursorPosition(g_16_stEventQueuePointerData.x,
                                       g_16_stEventQueuePointerData.y, 0, 0);
        } else if (dx != 0 && dy == 0) {
            buttons &= 2;
            QueueInputEvent(2, g_16_stEventQueuePointerData.x,
                            g_16_stEventQueuePointerData.y, 0,
                            buttons, 0, 0, 0, 0);
        }
    }
    g_nKeyboardMouseButtons = buttons;
}
