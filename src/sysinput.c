/*
 *  Mouse, keyboard and clock services over the Win32 API.
 *
 *  Address range 0x402e00-0x4030ff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: contiguous run of Win32 input/timing wrappers with no string references.
 */
#include "wc1.h"

/* Function start: 0x455A85 */
void SetMousePosition(int x, int y)
{
    SetCursorPos(x, y);
}

/* Function start: 0x455AC8 */
unsigned int PollKeyboardState(void)
{
    g_nClearedKeyboardState_005b3694 = 0;
    g_nHomeKeyState_005b371c = GetAsyncKeyState(VK_HOME);
    g_nUpArrowKeyState_005b3738 = GetAsyncKeyState(VK_UP);
    g_nPageUpKeyState_005b3840 = GetAsyncKeyState(VK_PRIOR);
    g_nLeftArrowKeyState_005b3844 = GetAsyncKeyState(VK_LEFT);
    g_nRightArrowKeyState_005b3728 = GetAsyncKeyState(VK_RIGHT);
    g_nEndKeyState_005b372c = GetAsyncKeyState(VK_END);
    g_nDownArrowKeyState_005b373c = GetAsyncKeyState(VK_DOWN);
    g_nPageDownKeyState_005b3720 = GetAsyncKeyState(VK_NEXT);
    g_nClearKeyState_005b3730 = GetAsyncKeyState(VK_CLEAR);
    g_nOemPeriodKeyState_005b3718 = GetAsyncKeyState(0xbe);
    g_nOemCommaKeyState_005b3724 = GetAsyncKeyState(0xbc);
    g_nInsertKeyState_005b3690 = GetAsyncKeyState(VK_INSERT);
    g_nDeleteKeyState_005b3734 = GetAsyncKeyState(VK_DELETE);

    if (g_nHomeKeyState_005b371c != 0)
        return 0x47;
    if (g_nPageUpKeyState_005b3840 != 0)
        return 0x49;
    if (g_nEndKeyState_005b372c != 0)
        return 0x4f;
    if (g_nPageDownKeyState_005b3720 != 0)
        return 0x51;
    if (g_nInsertKeyState_005b3690 != 0 ||
        g_nOemCommaKeyState_005b3724 != 0)
        return 0x52;
    if (g_nDeleteKeyState_005b3734 != 0 ||
        g_nOemPeriodKeyState_005b3718 != 0)
        return 0x53;
    if (g_nClearKeyState_005b3730 != 0)
        return 0x4c;
    if (g_nUpArrowKeyState_005b3738 != 0) {
        if (g_nLeftArrowKeyState_005b3844 != 0)
            return 0x47;
        return g_nRightArrowKeyState_005b3728 != 0 ? 0x49 : 0x48;
    }
    if (g_nDownArrowKeyState_005b373c != 0) {
        if (g_nLeftArrowKeyState_005b3844 != 0)
            return 0x4f;
        return g_nRightArrowKeyState_005b3728 != 0 ? 0x51 : 0x50;
    }
    if (g_nLeftArrowKeyState_005b3844 != 0)
        return 0x4b;
    return g_nRightArrowKeyState_005b3728 != 0 ? 0x4d : 0;
}

/* Function start: 0x455D2D */
int GetHomeKeyState(void)
{
    return GetAsyncKeyState(VK_HOME);
}

/* Function start: 0x455D48 */
int GetUpArrowKeyState(void)
{
    return GetAsyncKeyState(VK_UP);
}

/* Function start: 0x455D63 */
int GetPageUpKeyState(void)
{
    return GetAsyncKeyState(VK_PRIOR);
}

/* Function start: 0x455D7E */
int GetLeftArrowKeyState(void)
{
    return GetAsyncKeyState(VK_LEFT);
}

/* Function start: 0x455D99 */
int GetRightArrowKeyState(void)
{
    return GetAsyncKeyState(VK_RIGHT);
}

/* Function start: 0x455DB4 */
int GetEndKeyState(void)
{
    return GetAsyncKeyState(VK_END);
}

/* Function start: 0x455DCF */
int GetDownArrowKeyState(void)
{
    return GetAsyncKeyState(VK_DOWN);
}

/* Function start: 0x455DEA */
int GetPageDownKeyState(void)
{
    return GetAsyncKeyState(VK_NEXT);
}

/* Function start: 0x455E05 */
int GetNumLockKeyState(void)
{
    return GetAsyncKeyState(VK_NUMLOCK);
}

/* Function start: 0x455E23 */
int GetNumpadPlusKeyState(void)
{
    return GetAsyncKeyState(VK_ADD);
}

/* Function start: 0x455E3E */
int GetNumpadMinusKeyState(void)
{
    return GetAsyncKeyState(VK_SUBTRACT);
}

/* Function start: 0x455E59 */
int GetReturnKeyState(void)
{
    return GetAsyncKeyState(VK_RETURN);
}

/* Function start: 0x455E74 */
int GetSpaceKeyState(void)
{
    return GetAsyncKeyState(VK_SPACE);
}

/* Function start: 0x455E8F */
int GetShiftKeyState(void)
{
    return GetAsyncKeyState(VK_SHIFT);
}

/* Function start: 0x455EBF */
int GetControlKeyState(void)
{
    return GetAsyncKeyState(VK_CONTROL);
}

/* Function start: 0x455EAA */
unsigned int GetKeyboardModifiers(void)
{
    return g_dwSystemKey_005d10a4;
}

/* Function start: 0x455EF5 */
unsigned int GetGameClockTicks(void)
{
    return (GetTickCount() - g_dwGameClockBase_005d10e8) * 60 / 1000;
}

/* Function start: 0x455F23 */
void InitGameClockRandomEpoch(void)
{
    g_dwGameClockStart_005d12b8 = GetTickCount();
    g_dwGameClockBase_005d10e8 = GetTickCount() + (rand() & 3600000);
}

/* Function start: 0x45641B */
int TakeInputPressCount(void)
{
    int pressCount;

    pressCount = g_nInputPressCount_0049c258;
    g_nInputPressCount_0049c258 = 0;
    return pressCount;
}

/* Function start: 0x455F59 */
void WriteDebugString(const char *s)
{
    if (s != 0) {
        OutputDebugString(s);
        if (g_nDebugFileLoggingEnabled_0049c2d8 != 0) {
            if (g_pDebugLogFile_0049c2dc == 0)
                g_pDebugLogFile_0049c2dc = fopen("log.log", "wt");
            fprintf(g_pDebugLogFile_0049c2dc, "%s", s);
            fflush(g_pDebugLogFile_0049c2dc);
        }
    }
    return;
}

/* Function start: 0x455FD3 */
void SetInputCursorHostPosition(int x, int y)
{
    SetCursorPos(x, y);
    return;
}
