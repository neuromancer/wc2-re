/*
 *  Pilot name entry, high scores, inter-scene transitions and ownership of
 *  the Win32 developer overlay console.
 *
 *  Address range 0x425000-0x426fff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: EnterPilotNameAndCallsign/ShowTrainSimHighScores; string band 0x469D74-0x469F98.
 */
#include "game.h"

/* Function start: 0x4333F8 */
int MapCutsceneSoundEffectNumber(int soundIndex)
{
    return soundIndex + 1;
}

/* Function start: 0x40FCD0 */
void CreateDebugOverlayConsole(HINSTANCE module, HWND window,
                               short columns, short rows)
{
    g_pDebugOverlay_004a2548 =
        new DebugOverlayConsole(module, window, columns, rows, 0);
}

/* Function start: 0x40FD73 */
void DestroyGlobalDebugOverlayConsole(void)
{
    DebugOverlayConsole *console;

    console = g_pDebugOverlay_004a2548;
    delete console;
#ifdef SDL_PORT
    g_pDebugOverlay_004a2548 = 0;
#endif
    return;
}

#pragma function(strcpy)
/* Function start: 0x40FDAD */
void SystemDebugPrintf(const char *format, ...)
{
    va_list arguments;

    va_start(arguments, format);
    if (format != 0)
        vsprintf(g_szSystemDebugBuffer_004a2550, format, arguments);
    else
        strcpy(g_szSystemDebugBuffer_004a2550, "\n");
    va_end(arguments);
    DebugOverlayPrintf(g_pDebugOverlay_004a2548,
                       g_szSystemDebugBuffer_004a2550);
    WriteDebugString(g_szSystemDebugBuffer_004a2550);
}

/* Function start: 0x40FE23 */
char PumpMessagesDuringWait(void)
{
    return g_pDebugOverlay_004a2548->WaitForKey();
}

/* Function start: 0x40FE3E */
unsigned char TakeDebugStepFlag(void)
{
    return g_pDebugOverlay_004a2548->TakeStepFlag();
}

/* Function start: 0x40FE59 */
void SetDebugTextPosition(short row, short column)
{
    g_pDebugOverlay_004a2548->SetTextPosition(row, column);
}

/* Function start: 0x40FE7E */
void SetDebugCursorPosition(short column, short row)
{
    g_pDebugOverlay_004a2548->SetCursorPosition(column, row);
}

/* Function start: 0x40FEA3 */
void ResetDebugOverlay(void)
{
    g_pDebugOverlay_004a2548->Reset();
}

/* Function start: 0x40FEBE */
void ClearDebugOverlay(void)
{
    g_pDebugOverlay_004a2548->Clear();
}

/* Function start: 0x40FED9 */
void ClearDebugPauseFlags(void)
{
    g_pDebugOverlay_004a2548->ClearPauseFlags();
}

/* Function start: 0x40FF40 */
void DebugOverlayConsole::SetTextPosition(int row, int column)
{
    cursorColumn = column;
    cursorRow = row;
}

/* Function start: 0x40FF70 */
void DebugOverlayConsole::SetCursorPosition(int column, int row)
{
    SetTextPosition(row, column);
}

/* Function start: 0x40FFA0 */
void DebugOverlayConsole::Reset(void)
{
    Clear();
}

/* Function start: 0x40FFC0 */
unsigned char DebugOverlayConsole::TakeStepFlag(void)
{
    DWORD value;

    value = g_dwDebugOverlayKeyLatch_0049cb2c;
    g_dwDebugOverlayKeyLatch_0049cb2c = 0;
    return (unsigned char)value;
}

/* Function start: 0x40FFF0 */
void DebugOverlayConsole::ClearPauseFlags(void)
{
    g_dwDebugOverlayKeyLatch_0049cb2c = 0;
    g_dwDebugOverlayKey_0049cb28 = 0;
}
