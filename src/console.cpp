/*
 *  Owner and C wrappers of the Win32 developer overlay console.
 *
 *  Retail object 0x40FCD0-0x41001F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Uninitialized C++ definitions: this unit's .bss, ordered by the
 * identifiers' hash keys. */
DebugOverlayConsole *g_pDiagnosticOverlay; /* 0x004a2548 */
char g_4_szSystemDebugBuffer[256]; /* 0x004a2550 */

/* Function start: 0x40FCD0 */
void CreateDebugOverlayConsole(HINSTANCE module, HWND window,
                               short columns, short rows)
{
    g_pDiagnosticOverlay =
        new DebugOverlayConsole(module, window, columns, rows, 0);
}

/* Function start: 0x40FD73 */
void DestroyGlobalDebugOverlayConsole(void)
{
    delete g_pDiagnosticOverlay;
#ifdef SDL_PORT
    g_pDiagnosticOverlay = 0;
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
        vsprintf(g_4_szSystemDebugBuffer, format, arguments);
    else
        strcpy(g_4_szSystemDebugBuffer, "\n");
    va_end(arguments);
    DebugOverlayPrintf(g_pDiagnosticOverlay,
                       g_4_szSystemDebugBuffer);
    WriteDebugString(g_4_szSystemDebugBuffer);
}

/* Function start: 0x40FE23 */
char PumpMessagesDuringWait(void)
{
    return g_pDiagnosticOverlay->WaitForKey();
}

/* Function start: 0x40FE3E */
unsigned char TakeDebugStepFlag(void)
{
    return g_pDiagnosticOverlay->TakeStepFlag();
}

/* Function start: 0x40FE59 */
void SetDebugTextPosition(short row, short column)
{
    g_pDiagnosticOverlay->SetTextPosition(row, column);
}

/* Function start: 0x40FE7E */
void SetDebugCursorPosition(short column, short row)
{
    g_pDiagnosticOverlay->SetCursorPosition(column, row);
}

/* Function start: 0x40FEA3 */
void ResetDebugOverlay(void)
{
    g_pDiagnosticOverlay->Reset();
}

/* Function start: 0x40FEBE */
void ClearDebugOverlay(void)
{
    g_pDiagnosticOverlay->Clear();
}

/* Function start: 0x40FED9 */
void ClearDebugPauseFlags(void)
{
    g_pDiagnosticOverlay->ClearPauseFlags();
}

/* Function start: 0x40FF40 */
inline void DebugOverlayConsole::SetTextPosition(int row, int column)
{
    cursorColumn = column;
    cursorRow = row;
}

/* Function start: 0x40FF70 */
inline void DebugOverlayConsole::SetCursorPosition(int column, int row)
{
    SetTextPosition(row, column);
}

/* Function start: 0x40FFA0 */
inline void DebugOverlayConsole::Reset(void)
{
    Clear();
}

/* Function start: 0x40FFC0 */
inline unsigned char DebugOverlayConsole::TakeStepFlag(void)
{
    DWORD value;

    value = g_dwDebugOverlayKeyLatch;
    g_dwDebugOverlayKeyLatch = 0;
    return (unsigned char)value;
}

/* Function start: 0x40FFF0 */
inline void DebugOverlayConsole::ClearPauseFlags(void)
{
    g_dwDebugOverlayKeyLatch = 0;
    g_dwDebugOverlayKey = 0;
}
