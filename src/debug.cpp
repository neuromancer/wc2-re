/*
 *  Win32 developer overlay console.
 *
 *  Address range 0x41c760-0x41d0bf (exact).  The new/delete call sites,
 *  implicit ECX object parameter, and callee-cleanup member arguments prove
 *  this isolated utility was compiled as C++ despite the surrounding game
 *  sources being C.
 */
#include "game.h"
#include <stdarg.h>

#pragma function(memcpy, memset)

/* Function start: 0x45AC50 */
DebugOverlayConsole::DebugOverlayConsole(HINSTANCE module,
                                         HWND targetWindow,
                                         int columnCount,
                                         int rowCount,
                                         int waitMode)
{
#ifndef SDL_PORT
    TEXTMETRICA metrics;
    HDC deviceContext;
    HGDIOBJ font;
#endif

    busyWait = waitMode;
    g_nDebugOverlayConsoleCount_0049cb24++;
    window = targetWindow;
    columns = columnCount;
    rows = rowCount;
    cursorRow = 0;
    cursorColumn = cursorRow;
    textBuffer = (char *)malloc(rows * columns);
    dirtyLines = (char *)malloc(rows);
    memset(textBuffer, ' ', rows * columns);
    memset(dirtyLines, 1, rows);

#ifdef SDL_PORT
    (void)module;
    characterWidth = 8;
    characterHeight = 10;
#else
    deviceContext = GetDC(window);
    font = CreateFontA(10, 10, 0, 0, 400, 0, 0, 0, 0, 2, 0, 0,
                       0x30, g_szDebugOverlayFontName_0049cb34);
    SelectObject(deviceContext, font);
#endif
    textColor = 0xffffff;
    backgroundColor = 0;
    backgroundMode = OPAQUE;
#ifndef SDL_PORT
    GetTextMetricsA(deviceContext, &metrics);
    SetTextColor(deviceContext, 0xffffff);
    SetBkColor(deviceContext, 0);
    ReleaseDC(window, deviceContext);
    characterWidth = metrics.tmMaxCharWidth;
    characterHeight = metrics.tmHeight;

    if (g_hDebugKeyboardHook_0049cb30 == 0) {
        g_hDebugKeyboardHook_0049cb30 =
            SetWindowsHookExA(WH_KEYBOARD, (HOOKPROC)DebugKeyboardHookProc,
                              module, 0);
    }
#endif
    reverseVideo = 0;
#ifdef SDL_PORT
    mutex = 0;
#else
    mutex = CreateMutexA(0, FALSE, 0);
#endif
    spinnerIndex = 0;
    animationState = 1;
    spinnerCharacters = (char *)malloc(5);
    strcpy(spinnerCharacters, g_szDebugOverlaySpinner_0049cb3c);
}

/* Function start: 0x45AEE4 */
extern "C" DWORD WINAPI DebugOverlayWorkerProc(void *parameter)
{
    DebugOverlayConsole *console;
#ifndef SDL_PORT
    /* Retail compares against this stack slot before its first assignment. */
    DWORD timer;
    DWORD waitResult;
    HDC deviceContext;
#endif

    console = (DebugOverlayConsole *)parameter;
#ifdef SDL_PORT
    while (console->animationState != 2)
        SDL_Delay(10);
    return 0;
#else
    while (console->animationState != 2) {
        if (timeGetTime() > timer + 500) {
            waitResult = WaitForSingleObject(console->mutex, 500);
            if (waitResult == WAIT_FAILED) {
                ExitThread(1);
            } else if (waitResult == WAIT_TIMEOUT) {
                continue;
            }
            if (console->animationState != 0) {
                deviceContext = GetDC(console->window);
                TextOutA(deviceContext,
                         console->characterWidth * console->cursorColumn,
                         console->characterHeight * console->cursorRow,
                         console->spinnerCharacters + console->spinnerIndex,
                         1);
                console->spinnerIndex++;
                if ((int)strlen(console->spinnerCharacters) ==
                    console->spinnerIndex) {
                    console->spinnerIndex = 0;
                }
                ReleaseDC(console->window, deviceContext);
            }
            ReleaseMutex(console->mutex);
            timer = timeGetTime();
        }
    }
    ExitThread(0);
    return 0;
#endif
}

/* Function start: 0x45B04E */
extern "C" LRESULT CALLBACK DebugKeyboardHookProc(int code, WPARAM key,
                                                    LPARAM flags)
{
#ifdef SDL_PORT
    (void)code;
    if ((flags & 0x40000000) != 0) {
        g_dwDebugOverlayKey_0049cb28 = (DWORD)key;
        g_dwDebugOverlayKeyLatch_0049cb2c = (DWORD)key;
    }
    return 0;
#else
    if (code < 0)
        return CallNextHookEx(g_hDebugKeyboardHook_0049cb30,
                              code, key, flags);
    if ((flags & 0x40000000) != 0) {
        g_dwDebugOverlayKey_0049cb28 = key;
        g_dwDebugOverlayKeyLatch_0049cb2c = key;
    }
    return CallNextHookEx(g_hDebugKeyboardHook_0049cb30,
                          code, key, flags);
#endif
}

#pragma function(strcpy, strlen)

/* Function start: 0x45B0BB */
extern "C" void DebugOverlayPrintf(DebugOverlayConsole *console,
                                     const char *format, ...)
{
    va_list arguments;
    int length;
    int index;

    va_start(arguments, format);
    if (format != 0)
        vsprintf(console->formatBuffer, format, arguments);
    else
        strcpy(console->formatBuffer, g_szDebugOverlayNewline_0049cb44);
    va_end(arguments);
    length = strlen(console->formatBuffer);
    for (index = 0; length > index; index++) {
        console->dirtyLines[console->cursorRow] = 1;
        if (console->formatBuffer[index] >= ' ' &&
            console->formatBuffer[index] <= '~') {
            console->textBuffer[console->cursorRow * console->columns +
                                console->cursorColumn] =
                console->formatBuffer[index];
            console->cursorColumn++;
            if (console->cursorColumn == console->columns) {
                console->cursorColumn = 0;
                console->cursorRow++;
                if (console->rows == console->cursorRow)
                    console->Scroll();
            }
        } else {
            switch (console->formatBuffer[index]) {
            case '\n':
            case '\r':
                console->cursorColumn = 0;
                console->cursorRow++;
                if (console->rows == console->cursorRow)
                    console->Scroll();
                break;
            case '\a':
#ifdef SDL_PORT
                fputc('\a', stderr);
#else
                Beep(0, 0);
#endif
                break;
            case '\b':
                console->cursorColumn--;
                if (console->cursorColumn < 0) {
                    console->cursorColumn = 0;
                    console->cursorRow--;
                    if (console->cursorRow < 0)
                        console->cursorRow = 0;
                }
                console->textBuffer[
                    console->cursorRow * console->columns +
                    console->cursorColumn] = ' ';
                break;
            }
        }
    }
    console->DrawPendingLines();
}

/* Function start: 0x45AE6C */
DebugOverlayConsole::~DebugOverlayConsole(void)
{
    int shutdownKey;

    animationState = 2;
    shutdownKey = 0x103;
    if (--g_nDebugOverlayConsoleCount_0049cb24 == 0) {
#ifndef SDL_PORT
        UnhookWindowsHookEx(g_hDebugKeyboardHook_0049cb30);
#endif
        free(textBuffer);
        free(dirtyLines);
        free(spinnerCharacters);
    }
    (void)shutdownKey;
    return;
}

#pragma intrinsic(strcpy, strlen)

/* Function start: 0x45B2E0 */
void DebugOverlayConsole::Clear(void)
{
    cursorRow = 0;
    cursorColumn = cursorRow;
    memset(textBuffer, ' ', rows * columns);
    memset(dirtyLines, 1, rows);
    return;
}

/* Function start: 0x45B348 */
void DebugOverlayConsole::Scroll(void)
{
    memmove(textBuffer, columns + textBuffer, (rows - 1) * columns);
    cursorRow = rows - 1;
    memset((rows - 1) * columns + textBuffer, ' ', columns);
    memset(dirtyLines, 1, rows);
    return;
}

/* Function start: 0x45B3DC */
void DebugOverlayConsole::DrawPendingLines(void)
{
#ifndef SDL_PORT
    HDC deviceContext;
    int row;

    deviceContext = GetDC(window);
    for (row = 0; rows > row; row++) {
        if (dirtyLines[row] != 0) {
            TextOutA(deviceContext, 0, characterHeight * row,
                     columns * row + textBuffer, columns);
        }
    }
    ReleaseDC(window, deviceContext);
#endif
    memset(dirtyLines, 0, rows);
    return;
}

/* Function start: 0x45B49C */
char DebugOverlayConsole::WaitForKey(void)
{
#ifdef SDL_PORT
    char key;

    while (g_dwDebugOverlayKey_0049cb28 == 0 &&
           PumpWindowMessages(0) != 0) {
        if (busyWait == 0)
            SDL_Delay(1);
    }
    if (g_dwDebugOverlayKey_0049cb28 == 0)
        return 0x1b;
    key = (char)g_dwDebugOverlayKey_0049cb28;
    g_dwDebugOverlayKey_0049cb28 = 0;
    return key;
#else
    MSG message;
    DWORD key;

    if (busyWait == 0) {
        while (g_dwDebugOverlayKey_0049cb28 == 0) {
            while (PeekMessageA(&message, 0, 0, 0, 0) != 0) {
                if (message.message == WM_QUIT)
                    return 0;
                if (GetMessageA(&message, 0, 0, 0) != 0) {
                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                }
            }
        }
    } else {
        while (g_dwDebugOverlayKey_0049cb28 == 0) {
        }
    }
    key = g_dwDebugOverlayKey_0049cb28;
    g_dwDebugOverlayKey_0049cb28 = 0;
    return (char)key;
#endif
}

/* Function start: 0x45B55C */
void DebugOverlayConsole::EnableReverseVideo(void)
{
#ifndef SDL_PORT
    HDC deviceContext;
#endif

    if (reverseVideo == 0) {
#ifndef SDL_PORT
        deviceContext = GetDC(window);
        SetBkColor(deviceContext, textColor);
        SetTextColor(deviceContext, backgroundColor);
#endif
        reverseVideo = 1;
#ifndef SDL_PORT
        ReleaseDC(window, deviceContext);
#endif
    }
}

/* Function start: 0x45B5D8 */
void DebugOverlayConsole::DisableReverseVideo(void)
{
#ifndef SDL_PORT
    HDC deviceContext;
#endif

    if (reverseVideo != 0) {
#ifndef SDL_PORT
        deviceContext = GetDC(window);
        SetTextColor(deviceContext, textColor);
        SetBkColor(deviceContext, backgroundColor);
#endif
        reverseVideo = 0;
#ifndef SDL_PORT
        ReleaseDC(window, deviceContext);
#endif
    }
}

/* Function start: 0x45B654 */
void DebugOverlayConsole::SetOverlayTextColor(int red, int green, int blue)
{
#ifndef SDL_PORT
    HDC deviceContext;

    deviceContext = GetDC(window);
#endif
    textColor = red + (blue * 0x100 + green) * 0x100;
#ifndef SDL_PORT
    SetTextColor(deviceContext, textColor);
    ReleaseDC(window, deviceContext);
#endif
}

/* Function start: 0x45B6BB */
void DebugOverlayConsole::SetOverlayBackgroundColor(int red, int green,
                                                    int blue)
{
#ifndef SDL_PORT
    HDC deviceContext;

    deviceContext = GetDC(window);
#endif
    backgroundColor = red + (blue * 0x100 + green) * 0x100;
#ifndef SDL_PORT
    SetBkColor(deviceContext, backgroundColor);
    ReleaseDC(window, deviceContext);
#endif
}

/* Function start: 0x45B722 */
void DebugOverlayConsole::SetTransparentBackground(void)
{
#ifndef SDL_PORT
    HDC deviceContext;
#endif

    backgroundMode = TRANSPARENT;
#ifndef SDL_PORT
    deviceContext = GetDC(window);
    SetBkMode(deviceContext, backgroundMode);
    ReleaseDC(window, deviceContext);
#endif
}
