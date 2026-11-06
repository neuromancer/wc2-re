/*
 *  Win32 entry point, window, joystick, guarded heap and keyboard state.
 *
 *  Retail object 0x453C80-0x4564EF (link order; see docs/ORDER.md).
 */
/* This unit owns storage declared in globals.h as statics. */
#define WINMAIN_C
#include "game.h"

static char g_szWindowClassName[45]; /* 0x0049c278 */
static char g_szWindowTitle[45]; /* 0x0049c2a8 */

/* The debug log writer is defined near the end of this unit. */
void WriteDebugString(const char *s);

/* Function start: 0x453C80 */
unsigned int GetF1KeyLatch(void)
{
    return g_bF1KeyDown;
}

#pragma function(strcmp)
/* Function start: 0x453C95 */
void CheckLauncherAndConfig(void)
{
#ifdef SDL_PORT
    FILE *config;
    char option[100];
    char resolvedPath[PATH_MAX];

    if (ReadCheaterFlagFromRegistry() != 0) {
        *(unsigned char *)&g_nOriginDevUnlock = 1;
        *(unsigned char *)&g_bPlayerDamageEnabled = 0;
        *(unsigned char *)&g_bPlayerCollisionEnabled = 0;
    }

    if (SdlResolvePath("WINGCMDR.CFG", resolvedPath,
                          sizeof(resolvedPath)))
        config = fopen(resolvedPath, "rt");
    else
        config = 0;
    if (config != 0) {
        while (fscanf(config, "%s", option) != EOF) {
            char command;

            if (memcmp(option, "$#SAGA.EXE", 11) == 0)
                g_bApplicationControllerActive = 1;
            command = option[0] == '-' ? option[1] : option[0];
            switch (command) {
            case 'b':
                *(unsigned char *)&g_bPlayerCollisionEnabled = 0;
                break;
            case 'c':
                g_bCockpitEnabled = 0;
                break;
            case 'f':
                g_bShowFrameRate = 1;
                break;
            case 'k':
                *(unsigned char *)&g_bPlayerDamageEnabled = 0;
                break;
            case 'q':
                g_bConfigQuickModeEnabled = 0;
                break;
            }
            if (config == 0)
                return;
        }
        fclose(config);
    }
#else
    int scanned;
    char option[100];
    char command;
    int ctrlFile;
    FILE *config;

    if (ReadCheaterFlagFromRegistry() != 0) {
        *(unsigned char *)&g_bPlayerDamageEnabled = 0;
        *(unsigned char *)&g_bPlayerCollisionEnabled = 0;
        *(unsigned char *)&g_nOriginDevUnlock = 1;
    }

    ctrlFile = _open("ctrl.del", O_BINARY);
    if (ctrlFile != -1) {
        g_bDebugBreakEnabled = 1;
        _close(ctrlFile);
    }

    config = fopen("WC2.CFG", "rt");
    while (config != 0) {
        scanned = fscanf(config, "%s", option);
        if (scanned == -1) {
            fclose(config);
            break;
        }
        if (strcmp("[[log]]", option) == 0)
            g_nDebugFileLoggingEnabled = 1;

        command = option[0];
        if (command == '-')
            command = option[1];
        switch (command) {
        case 'k':
            *(unsigned char *)&g_bPlayerDamageEnabled = 0;
            break;
        case 'b':
            *(unsigned char *)&g_bPlayerCollisionEnabled = 0;
            break;
        case 's':
            break;
        case 'f':
            g_bShowFrameRate = 1;
            break;
        case 'q':
            g_bConfigQuickModeEnabled = 0;
            break;
        case 'l':
            g_bSkipCampaignVideo = 1;
            break;
        case 'w':
            g_bUseHardwarePalette = 0;
            break;
        case 'm':
            g_bConfigMemoryOption = 1;
            break;
        }
    }
#endif
}

#pragma intrinsic(strcmp)
#ifndef SDL_PORT

#pragma function(memset)
/* Function start: 0x453E60 */
int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous,
                   LPSTR commandLine, int showCommand)
{
    MEMORYSTATUS memoryStatus;
    RECT clip;
    HANDLE processHandle;

    if (commandLine != 0 && strchr(commandLine, 's') != 0)
        g_nAudioEnabled = 0;
    g_19_hOneProcessLock =
        CreateSemaphoreA(0, 0, 1, "Wing Commander 2");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxA(0,
                    "Only one instance of Wing Commander 2 for Windows95 may be running at a time",
                    "ATTENTION", MB_ICONERROR);
        exit(0);
    }

    memset(&memoryStatus, 0, sizeof(memoryStatus));
    memoryStatus.dwLength = sizeof(memoryStatus);
    GlobalMemoryStatus(&memoryStatus);
    if (memoryStatus.dwTotalPhys < 0x800000) {
        MessageBoxA(0,
                    "You must have at leat 8 megs of memory available to play Wing Commander 2 for Windows95",
                    "ATTENTION", MB_ICONERROR);
        exit(0);
    }
    if (memoryStatus.dwTotalPageFile < 0x800000) {
        MessageBoxA(0,
                    "You must have at leat 8 megs of virtual memory available to play Wing Commander 2 for Window95",
                    "ATTENTION", MB_ICONERROR);
        exit(0);
    }

    if (waveOutGetNumDevs() < 1)
        g_nAudioEnabled = 0;
    CheckLauncherAndConfig();
    if (!PromptInsertCorrectCd()) {
        CloseHandle(g_19_hOneProcessLock);
        return 0;
    }
    if (!CreateMainWindow(instance, previous, showCommand))
        return 0;

    g_bWindowInactive = 0;
    processHandle = GetCurrentProcess();
    MonoDebug_install();
    SetPriorityClass(processHandle, HIGH_PRIORITY_CLASS);
    InitializeAudioSystem(g_10_hMainWindow);
    InitializeAudioStreamer(g_10_hMainWindow);
    srand((unsigned int)time(0));
    InitGameClockRandomEpoch();
    CreateDebugOverlayConsole(instance, g_10_hMainWindow, 60, 20);
    g_11_dwGameZeroTime = (unsigned int)time(0);
    ShowCursor(FALSE);
    g_176_pfnCurrentInputPump = 0;
    clip.left = 0;
    clip.top = 0;
    clip.right = 320;
    clip.bottom = 200;
    ClipCursor(&clip);

    _onexit((_onexit_t)AbortToDesktop);
    g_196_nInputTimerTicks = 0;
    AllocateApplicationScratchBuffer();
    g_bApplicationControllerActive = 1;
    /* argc is zero, so the argument vector is never read. */
    RunGameApplication(0, (char **)"");
    g_bApplicationShutdownStarted = 1;
    ReleaseApplicationScratchBuffer();

    ClipCursor(0);
    ShowCursor(TRUE);
    g_48_dwGameExitClockValue = (unsigned int)time(0);
    DestroyGlobalDebugOverlayConsole();
    ServiceAudioStream();
    DestroyWindow(g_10_hMainWindow);
    DIBunInstall();
    CloseHandle(g_19_hOneProcessLock);
    return 1;
}

#endif

/* Function start: 0x454103 */
void ShutdownGameWindow(void)
{
    g_bApplicationShutdownStarted = 1;
    g_48_dwGameExitClockValue = (unsigned int)time(0);
#ifdef SDL_PORT
    {
        SDL_Window *window;

        g_16_bMainWindowRunning = 0;
        if ((g_dwStreamerMode & 1) != 0)
            ix_streamer_destroy();
        ServiceAudioStream();
        DestroyGlobalDebugOverlayConsole();
        window = (SDL_Window *)g_10_hMainWindow;
        DIBunInstall();
        SdlShutdownJoysticks();
        if (window != 0)
            SDL_DestroyWindow(window);
        g_10_hMainWindow = 0;
        SDL_Quit();
    }
#else
    ClipCursor(0);
    ShowCursor(TRUE);
    DestroyGlobalDebugOverlayConsole();
    DestroyWindow(g_10_hMainWindow);
    DIBunInstall();
    ClipCursor(0);
    ShowCursor(TRUE);
    {
        HANDLE process;

        process = GetCurrentProcess();
        SetPriorityClass(process, IDLE_PRIORITY_CLASS);
    }
    CloseHandle(g_19_hOneProcessLock);
#endif
    exit(0);
}

/* Function start: 0x45418B */
void ShowNoticeMessageBox(const char *text)
{
    DIBpositionWindow();
    MessageBoxA(0, text, "NOTICE", 0x10);
    exit(0);
    return;
}

#pragma function(strcpy)
/* Function start: 0x4541BD */
void OutputDebugPrintf(const char *format, ...)
{
    va_list arguments;
    char buffer[128];

    va_start(arguments, format);
    if (format != 0)
        vsprintf(buffer, format, arguments);
    else
        strcpy(buffer, va_arg(arguments, const char *));
    va_end(arguments);
    OutputDebugStringA(buffer);
    return;
}

/* Function start: 0x45422D */
unsigned int AbortToDesktop(void)
{
#ifndef SDL_PORT
    HANDLE process;

    g_bApplicationShutdownStarted = 1;
    ClipCursor(0);
    ShowCursor(TRUE);
    process = GetCurrentProcess();
    SetPriorityClass(process, IDLE_PRIORITY_CLASS);
#endif
    sprintf(g_29_abMemoryLoadNotice,
            "Current: %i\nMax    : %i\nTotal : %i\n",
            g_dwGuardedAllocationBytes,
            g_dwGuardedAllocationPeakBytes,
            g_dwGuardedAllocationTotalBytes);
    OutputDebugStringA("Memory Info:\n");
    OutputDebugStringA(g_29_abMemoryLoadNotice);
#ifndef SDL_PORT
    CloseHandle(g_19_hOneProcessLock);
#endif
    return 0;
}

#ifndef SDL_PORT

/* Function start: 0x4542B7 */
int CreateMainWindow(HINSTANCE instance, HINSTANCE previous,
                     int showCommand)
{
    WNDCLASSA windowClass;
    DWORD error;

    g_7_hAppCopy = instance;
    if (previous != 0)
        return 0;

    windowClass.lpszClassName = g_szWindowClassName;
    windowClass.hInstance = g_7_hAppCopy;
    windowClass.lpfnWndProc = MainWindowProc;
    windowClass.hCursor = 0;
    windowClass.hIcon = LoadIconA(0, IDI_APPLICATION);
    windowClass.lpszMenuName = g_szWindowClassName;
    windowClass.hbrBackground = GetStockObject(BLACK_BRUSH);
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    if (RegisterClassA(&windowClass) == 0)
        return 0;

    g_10_hMainWindow = CreateWindowExA(
        0, g_szWindowClassName, g_szWindowTitle,
        WS_POPUP, 0, 0, 320, 200, 0, 0, g_7_hAppCopy, 0);
    if (g_10_hMainWindow == 0) {
        error = GetLastError();
        return 0;
    }

    g_18_hMainWindowContext = GetDC(g_10_hMainWindow);
    SetTextColor(g_18_hMainWindowContext, RGB(255, 0, 0));
    SetBkColor(g_18_hMainWindowContext, RGB(0, 0, 0));
    if (GetDeviceCaps(g_18_hMainWindowContext, BITSPIXEL) < 8) {
        MessageBoxA(0,
                    "You must be running with 256, or more, colors to play Wing Commander",
                    "NOTICE", MB_ICONEXCLAMATION);
        DestroyWindow(g_10_hMainWindow);
        return 0;
    }

    g_1_hMainWndCursor = SetCursor(0);
    ShowWindow(g_10_hMainWindow, showCommand);
    UpdateWindow(g_10_hMainWindow);
    DIBinstall(g_10_hMainWindow);
    g_16_bMainWindowRunning = 1;
    return 1;
}

#endif

/* Function start: 0x45445A */
unsigned int PumpWindowMessages(int skipMessages)
{
#ifndef SDL_PORT
    MSG message;
    /* Retail reserves 28 more bytes around these that nothing touches. */
    DWORD cursor;
    POINT point;
#endif
    int done;
#ifndef SDL_PORT
    RECT windowRect;
#endif

    done = 0;
    g_196_nInputTimerTicks = GetTickCount();
    g_196_nInputTimerTicks -= g_13_lGameClockStartData;
    g_196_nInputTimerTicks *= 60;
    g_196_nInputTimerTicks /= 1000;
    if (skipMessages == 0) {
        if (g_bWindowMessagePumpActive != 0)
            return 1;
        g_bWindowMessagePumpActive = 1;
        if (g_176_pfnCurrentInputPump != 0)
            g_176_pfnCurrentInputPump();
        KeyboardMousePump();
#ifdef SDL_PORT
        SdlPumpEvents();
#else
        done = 0;
        while (done == 0 || g_bWindowInactive != 0) {
            if (g_bWindowInactive != 0) {
                if (GetMessageA(&message, 0, 0, 0) != 0) {
                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                    done = 1;
                } else {
                    LogMemoryUsage();
                    ShutdownGameWindow();
                }
            } else {
                if (PeekMessageA(&message, 0, 0, 0,
                                 PM_NOREMOVE) != 0) {
                    if (GetMessageA(&message, 0, 0, 0) != 0) {
                        TranslateMessage(&message);
                        DispatchMessageA(&message);
                    } else {
                        LogMemoryUsage();
                        ShutdownGameWindow();
                    }
                    done = 0;
                } else {
                    done = 1;
                }
            }
        }
#endif
        g_bWindowMessagePumpActive = 0;
    }
    g_196_nInputTimerTicks = GetTickCount();
    g_196_nInputTimerTicks -= g_13_lGameClockStartData;
    g_196_nInputTimerTicks *= 60;
    g_196_nInputTimerTicks /= 1000;
    return g_16_bMainWindowRunning;
}

#ifndef SDL_PORT

/* Function start: 0x454625 */
LRESULT CALLBACK MainWindowProc(HWND window, UINT message,
                                WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT paint;
    int mouseInput;
    unsigned int menuId;
    unsigned int primaryButtonDown;
    unsigned int secondaryDown;
    unsigned int pointerPositionX;
    unsigned int cursorY;
    unsigned int scanValue;
    MSG textEvent;
    unsigned int character;
    HANDLE processHandle;
    RECT clip;

    menuId = (unsigned int)wParam & 0xffff;
    mouseInput = 0;
    primaryButtonDown = ((unsigned int)wParam & 1) != 0;
    secondaryDown = ((unsigned int)wParam & 2) != 0;
    if (message == WM_MOUSEMOVE || message == WM_RBUTTONUP ||
        message == WM_LBUTTONUP || message == WM_LBUTTONDOWN ||
        message == WM_RBUTTONDOWN) {
        g_nPendingMouseX = LOWORD(lParam);
        pointerPositionX = g_nPendingMouseX;
        g_nPendingMouseY = HIWORD(lParam);
        cursorY = g_nPendingMouseY;
    } else {
        pointerPositionX = g_nPendingMouseX;
        cursorY = g_nPendingMouseY;
    }

    switch (message) {
    case WM_ACTIVATEAPP:
        if (g_bApplicationControllerActive != 0) {
            if (wParam != 0) {
                if (g_bWindowInactive == 1) {
                    clip.left = 0;
                    clip.top = 0;
                    clip.right = 320;
                    clip.bottom = 200;
                    ShowCursor(FALSE);
                    processHandle = GetCurrentProcess();
                    SetPriorityClass(processHandle, HIGH_PRIORITY_CLASS);
                    SetActiveWindow(g_10_hMainWindow);
                    SetForegroundWindow(g_10_hMainWindow);
                    DIBreInstall();
                    ClipCursor(&clip);
                    SetCursorPos(g_3_nSavedOwnerCursorX,
                                 g_102_nStoredWindowCursorYWord);
                    g_bWindowInactive = 0;
                    WriteDebugString("UNPAUSING...\n");
                }
            } else if (g_bWindowInactive == 0 &&
                       g_bApplicationShutdownStarted == 0) {
                g_3_nSavedOwnerCursorX = 160;
                g_102_nStoredWindowCursorYWord = 100;
                ClipCursor(0);
                ShowCursor(TRUE);
                processHandle = GetCurrentProcess();
                SetPriorityClass(processHandle, IDLE_PRIORITY_CLASS);
                g_bWindowInactive = 1;
                WriteDebugString("PAUSING...\n");
            }
            return 0;
        }
        /* fall through */
    case WM_ACTIVATE:
        if (g_bApplicationControllerActive != 0) {
            if (((unsigned int)wParam & 0xffff) != 0) {
                if (HIWORD(wParam) != 0) {
                    g_bWindowInactive = 1;
                    WriteDebugString("MINIMIZED PAUSING...\n");
                    return 0;
                } else if (g_bWindowInactive == 1) {
                    clip.left = 0;
                    clip.top = 0;
                    clip.right = 320;
                    clip.bottom = 200;
                    ShowCursor(FALSE);
                    processHandle = GetCurrentProcess();
                    SetPriorityClass(processHandle, HIGH_PRIORITY_CLASS);
                    SetActiveWindow(g_10_hMainWindow);
                    SetForegroundWindow(g_10_hMainWindow);
                    DIBreInstall();
                    ClipCursor(&clip);
                    SetCursorPos(g_3_nSavedOwnerCursorX,
                                 g_102_nStoredWindowCursorYWord);
                    g_bWindowInactive = 0;
                    WriteDebugString("UNPAUSING...\n");
                }
            } else if (g_bWindowInactive == 0 &&
                       g_bApplicationShutdownStarted == 0) {
                g_3_nSavedOwnerCursorX = 160;
                g_102_nStoredWindowCursorYWord = 100;
                ClipCursor(0);
                ShowCursor(TRUE);
                processHandle = GetCurrentProcess();
                SetPriorityClass(processHandle, IDLE_PRIORITY_CLASS);
                g_bWindowInactive = 1;
                WriteDebugString("PAUSING...\n");
            }
            return 0;
        }
        /* fall through */
    case WM_SETFOCUS:
        SignalAudioMixerWakeEvent();
        return 0;
        /* Retail retains the switch break after this return. */
        break;
    case WM_CREATE:
        break;
    case WM_COMMAND:
        switch (menuId) {
        case 3:
            g_16_bMainWindowRunning = 0;
            PostQuitMessage(0);
            break;
        }
        break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        g_nInputPressCount++;
        QueueInputEvent(1, pointerPositionX,
                        cursorY, 0,
                        primaryButtonDown, secondaryDown, 0, 0, 0);
        mouseInput = 1;
        break;
    case WM_RBUTTONUP:
        g_nInputPressCount--;
        QueueInputEvent(2, pointerPositionX,
                        cursorY, 0, 0, 1, 0, 0, 0);
        mouseInput = 1;
        break;
    case WM_LBUTTONUP:
        g_nInputPressCount--;
        QueueInputEvent(2, pointerPositionX,
                        cursorY, 0, 1, 0, 0, 0, 0);
        mouseInput = 1;
        break;
    case WM_MOUSEMOVE:
        if (g_2_bIgnoreNextCursorMoveFlag == 0) {
            QueueInputEvent(3, pointerPositionX,
                            cursorY, 0,
                            primaryButtonDown, secondaryDown, 0, 0, 0);
            mouseInput = 1;
        } else {
            g_2_bIgnoreNextCursorMoveFlag = 0;
        }
        break;
    case WM_KEYDOWN:
        if (PeekMessageA(&textEvent, window, WM_CHAR, WM_CHAR,
                         PM_REMOVE) != 0) {
            character = (unsigned int)textEvent.wParam;
        } else {
            character = 0;
        }
        if (wParam == VK_F1) {
            if (((unsigned long)lParam & 0x40000000) == 0)
                g_bF1KeyDown = 1;
            else
                g_bF1KeyDown = 0;
        }
        scanValue = ((unsigned long)lParam & 0xff0000) >> 16;
        if (scanValue == 1)
            g_bSceneEscapeRequested = 1;
        if (((unsigned long)lParam & 0x40000000) == 0 &&
            scanValue != 1) {
            g_nInputPressCount++;
        }
        QueueInputEvent(4, pointerPositionX,
                        cursorY, wParam,
                        0, 0, 0, scanValue,
                        character);
        SetInputKeyState((int)scanValue, 1);
        break;
    case WM_KEYUP:
        if (PeekMessageA(&textEvent, window, WM_CHAR, WM_CHAR,
                         PM_REMOVE) != 0) {
            character = (unsigned int)textEvent.wParam;
        } else {
            character = 0;
        }
        if (wParam == VK_F1)
            g_bF1KeyDown = 0;
        scanValue = ((unsigned long)lParam & 0xff0000) >> 16;
        if (scanValue != 0x81) {
            g_nInputPressCount--;
            if (g_nInputPressCount < 0)
                g_nInputPressCount = 0;
        }
        QueueInputEvent(5, pointerPositionX,
                        cursorY, wParam,
                        0, 0, 0, scanValue,
                        character);
        SetInputKeyState((int)scanValue, 0);
        break;
    case WM_SYSKEYUP:
        g_bMenuAHotkey = 0;
        g_4_bMenuBHotkey = 0;
        g_bJoystickCalibrationHotkey = 0;
        g_6_bMenuTHotkeySwitch = 0;
        g_2_bAltHHotkey = 0;
        g_2_bAltFHotkeyFlag = 0;
        g_6_bAltDChordState = 0;
        g_27_nLastAltCommandScanNumberWord = 0;
        g_39_bAltNumpadAddHotkeySwitch = 0;
        g_bAltNumpadSubtractShortcut = 0;
        g_69_dwSysKeyValue = 0;
        break;
    case WM_SYSKEYDOWN:
        g_69_dwSysKeyValue = (unsigned int)wParam;
        switch (wParam) {
        case 'X':
            g_bApplicationShutdownStarted = 1;
            if (((unsigned long)lParam & 0x20000000) != 0)
                PostQuitMessage(0);
            break;
        case 'N':
            ReportSpaceFlightMaxFps(-0.5f);
            break;
        case 'M':
            ReportSpaceFlightMaxFps(0.5f);
            break;
        case 'A':
            if (((unsigned long)lParam & 0x20000000) != 0)
                g_bMenuAHotkey = 1;
            break;
        case 'B':
            if (((unsigned long)lParam & 0x20000000) != 0)
                g_4_bMenuBHotkey = 1;
            break;
        case 'C':
            if (((unsigned long)lParam & 0x20000000) != 0)
                g_bJoystickCalibrationHotkey = 1;
            break;
        case 'T':
            if (((unsigned long)lParam & 0x20000000) != 0)
                g_6_bMenuTHotkeySwitch = 1;
            break;
        case 'H':
            if (((unsigned long)lParam & 0x20000000) != 0)
                g_2_bAltHHotkey = 1;
            break;
        case 'F':
            if (((unsigned long)lParam & 0x20000000) != 0)
                g_2_bAltFHotkeyFlag = 1;
            break;
        case 'D':
            if (((unsigned long)lParam & 0x20000000) != 0)
                g_6_bAltDChordState = 1;
            break;
        case VK_ADD:
            if (((unsigned long)lParam & 0x20000000) != 0)
                g_39_bAltNumpadAddHotkeySwitch = 1;
            break;
        case VK_SUBTRACT:
            if (((unsigned long)lParam & 0x20000000) != 0)
                g_bAltNumpadSubtractShortcut = 1;
            break;
        case 'Z':
            if (((unsigned long)lParam & 0x20000000) != 0 &&
                g_bDebugBreakEnabled != 0) {
                DebugBreak();
            }
            break;
        }
        break;
    case WM_SIZE:
        break;
    case WM_PAINT:
        BeginPaint(g_10_hMainWindow, &paint);
        EndPaint(g_10_hMainWindow, &paint);
        break;
    case WM_CLOSE:
    case WM_DESTROY:
        g_16_bMainWindowRunning = 0;
        ClipCursor(0);
        ShowCursor(TRUE);
        processHandle = GetCurrentProcess();
        SetPriorityClass(processHandle, IDLE_PRIORITY_CLASS);
        PostQuitMessage(0);
        break;
    case WM_QUIT:
        g_16_bMainWindowRunning = 0;
        ClipCursor(0);
        ShowCursor(TRUE);
        processHandle = GetCurrentProcess();
        SetPriorityClass(processHandle, IDLE_PRIORITY_CLASS);
        g_bApplicationShutdownStarted = 1;
        break;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_SCREENSAVE ||
            (wParam & 0xfff0) == SC_MONITORPOWER)
            return 0;
        break;
    }
    if (mouseInput != 0) {
        g_6_stHostMouseTextBlock.x = pointerPositionX;
        g_6_stHostMouseTextBlock.y = cursorY;
        g_6_stHostMouseTextBlock.primaryButton = primaryButtonDown;
        g_6_stHostMouseTextBlock.secondaryButton = secondaryDown;
    }
    return DefWindowProcA(window, message, wParam, lParam);
}

#endif

/* Function start: 0x4551E7 */
void GetJoystickPosition(unsigned int *x, unsigned int *y,
                         int *buttons, unsigned short joystick,
                         unsigned int fallback)
{
    unsigned int slot;
    unsigned int deviceId;
    JOYINFO joyInfo;

    slot = 0;
    if (joystick != 0) {
        deviceId = 1;
        slot = 1;
    } else {
        deviceId = 0;
        slot = 0;
    }
#ifdef SDL_PORT
    if (SdlReadJoystick(
            deviceId, &g_1_aJoystickInfoArray[slot]) != FALSE) {
#else
    if (joyGetPos(deviceId, &g_1_aJoystickInfoArray[slot]) ==
        JOYERR_NOERROR) {
#endif
        *x = g_1_aJoystickInfoArray[slot].wXpos;
        *y = g_1_aJoystickInfoArray[slot].wYpos;
        *buttons = g_1_aJoystickInfoArray[slot].wButtons;
        joyInfo = g_1_aJoystickInfoArray[slot];
        if ((joyInfo.wButtons & 1) != 0) {
            if (g_bJoystickPrimaryButtonLatched == 0)
                g_nInputPressCount++;
            g_bJoystickPrimaryButtonLatched = 1;
        } else {
            g_bJoystickPrimaryButtonLatched = 0;
        }
        if ((joyInfo.wButtons & 2) != 0) {
            if (g_bJoystickSecondaryButtonLatched == 0)
                g_nInputPressCount++;
            g_bJoystickSecondaryButtonLatched = 1;
        } else {
            g_bJoystickSecondaryButtonLatched = 0;
        }
        if (joystick != 0)
            *buttons >>= 2;
        else
            *buttons &= 3;
        return;
    }

    *x = fallback;
    *y = fallback;
    *buttons = fallback;
}

/* Function start: 0x455346 */
short GetJoystickButtons(void)
{
    short buttons;

    buttons = (g_1_aJoystickInfoArray[1].wButtons << 2) |
              g_1_aJoystickInfoArray[0].wButtons;
    return buttons;
}

/* Function start: 0x45536F */
void GetJoystickDevCaps(short joystick, short *xMin, short *xMax,
                        short *yMin, short *yMax)
{
#ifdef SDL_PORT
    unsigned int hostXMin;
    unsigned int hostXMax;
    unsigned int hostYMin;
    unsigned int hostYMax;
#else
    JOYCAPSA devCaps;
#endif
    unsigned int deviceId = joystick != 0;

    *xMin = *xMax = *yMin = *yMax = 0;
#ifdef SDL_PORT
    if (SdlReadJoystickAxisRange(
            deviceId, &hostXMin, &hostXMax, &hostYMin, &hostYMax) == FALSE) {
#else
    if (joyGetDevCapsA(deviceId, &devCaps, sizeof(devCaps)) != JOYERR_NOERROR) {
#endif
        return;
    }

#ifdef SDL_PORT
    *xMin = (short)hostXMin;
    *xMax = (short)hostXMax;
    *yMin = (short)hostYMin;
    *yMax = (short)hostYMax;
#else
    *xMin = (short)devCaps.wXmin;
    *xMax = (short)devCaps.wXmax;
    *yMin = (short)devCaps.wYmin;
    *yMax = (short)devCaps.wYmax;
#endif
}

/* Function start: 0x455427 */
HINSTANCE GetApplicationInstance(void)
{
    return g_7_hAppCopy;
}

/* Function start: 0x45543C */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
HWND GetMainWindowHandle(void)
{
    return g_10_hMainWindow;
}

/* Function start: 0x455451 */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
HDC GetMainWindowDeviceContext(void)
{
    return g_18_hMainWindowContext;
}

#pragma function(memset)
/* Function start: 0x455466 */
void *AllocateGuardedMemory(unsigned int size)
{
    if (g_pGuardedAllocationHead == 0) {
        g_pGuardedAllocationHead =
            malloc(sizeof(GuardedAllocation));
        g_9_pGuardedAllocationLastRef =
            g_pGuardedAllocationHead;
    } else {
        g_9_pGuardedAllocationLastRef->next =
            malloc(sizeof(GuardedAllocation));
        g_9_pGuardedAllocationLastRef =
            g_9_pGuardedAllocationLastRef->next;
    }
    g_9_pGuardedAllocationLastRef->next = 0;
    g_9_pGuardedAllocationLastRef->size = size;
#ifdef SDL_PORT
    /* The 0x400-byte 0xAB fences the original hand-rolls are what ASan's
     * redzones already provide, and ASan reports the write that runs past the
     * block instead of the free that finds the damage long afterwards.  Drop
     * them so the sanitizer, not this bookkeeping, catches an overrun.
     *
     * One word of the tail has to stay, though: the IFF form walk probes for a
     * "FORM" tag one word past the last form and stops when it does not find
     * one, so the original's fence was load-bearing there.  Four bytes is the
     * least that probe needs and still leaves any store wider than a tag --
     * every pointer-width mistake included -- landing in ASan's redzone. */
    g_9_pGuardedAllocationLastRef->block = malloc(size + 4);
    memset(g_9_pGuardedAllocationLastRef->block, 0, size);
    memset((unsigned char *)g_9_pGuardedAllocationLastRef->block + size,
           0xab, 4);
#else
    g_9_pGuardedAllocationLastRef->block = malloc(size + 0x800);

    memset(g_9_pGuardedAllocationLastRef->block, 0xab, 0x400);
    memset((unsigned char *)g_9_pGuardedAllocationLastRef->block +
               0x400,
           0, size);
    memset((unsigned char *)g_9_pGuardedAllocationLastRef->block +
               0x400 + size,
           0xab, 0x400);
#endif
    UntrackFreedHeapBlock(g_9_pGuardedAllocationLastRef->block);
#ifdef SDL_PORT
    return g_9_pGuardedAllocationLastRef->block;
#else
    return (unsigned char *)g_9_pGuardedAllocationLastRef->block +
           0x400;
#endif
}

#pragma intrinsic(memset)
/* Function start: 0x455565 */
void ReportHeapGuardCorruption(void *memory, int count, int overrun)
{
    char text[0x80];

    sprintf(text, "Memory at %p %swritten. Corruption : (%i/%i) %s",
            memory, overrun ? "over" : "under", count, 0x100,
            count > 0x80
                ? (count > 0xc0 ? "EXTREME" : "SEVERE")
                : (count > 0x40 ? "BAD" : "NAUGHTY"));
    MessageBoxA(0, text, "NOTICE", 0x10);
}

/* Function start: 0x455624 */
void CheckAllGuardedAllocations(void)
{
}

/* Function start: 0x45562F */
int IsFreedHeapBlockTracked(void *memory)
{
    FreedHeapBlock *block;

    block = g_pFreedHeapBlockHead;
    while (block != 0) {
        if (block->block == memory)
            return 1;
        if (block->block == (unsigned char *)memory - 8)
            return 1;
        block = block->next;
    }
    return 0;
}

/* Function start: 0x455697 */
int DetectDoubleFree(void *memory)
{
    FreedHeapBlock *block;

    block = g_pFreedHeapBlockHead;
    while (block != 0) {
        if (block->block == memory) {
            OutputDebugStringA("DoubleFree Detected\n");
            return 1;
        }
        if (block->block == (unsigned char *)memory - 8) {
            OutputDebugStringA("DoubleFree (pushed) Detected\n");
            return 1;
        }
        block = block->next;
    }
    return 0;
}

/* Function start: 0x455715 */
void TrackFreedHeapBlock(void *memory)
{
    if (g_pFreedHeapBlockHead == 0) {
        g_pFreedHeapBlockHead = malloc(sizeof(FreedHeapBlock));
        g_pFreedHeapBlockTail = g_pFreedHeapBlockHead;
    } else {
        g_pFreedHeapBlockTail->next = malloc(sizeof(FreedHeapBlock));
        g_pFreedHeapBlockTail = g_pFreedHeapBlockTail->next;
    }
    g_pFreedHeapBlockTail->next = 0;
    g_pFreedHeapBlockTail->block = memory;
    return;
}

/* Function start: 0x455787 */
/* Forget a block that has been handed back out, so a later free of it is not
 * mistaken for a double free. */
void UntrackFreedHeapBlock(void *memory)
{
    FreedHeapBlock *prev;
    FreedHeapBlock *current;

    prev = 0;
    current = g_pFreedHeapBlockHead;
    while (current != 0) {
        if (current->block == memory) {
            if (prev != 0) {
                prev->next = current->next;
                if (current->next == 0)
                    g_pFreedHeapBlockTail = prev;
            } else {
                g_pFreedHeapBlockHead = current->next;
                if (current->next == 0)
                    g_pFreedHeapBlockTail = prev;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
    return;
}

/* Function start: 0x455836 */
/* Nothing in the shipped executable calls this.  Frees the whole list of
 * tracked freed-block records. */
void FreeTrackedHeapBlocks(void)
{
    FreedHeapBlock *entry;
    FreedHeapBlock *freed;

    entry = g_pFreedHeapBlockHead;
    freed = 0;
    while (entry != 0) {
        freed = entry;
        entry = entry->next;
        free(freed);
    }
    return;
}

void ReportHeapGuardCorruption(void *memory, int count, int overrun);
void WriteDebugString(const char *s);

/* Function start: 0x455882 */
void FreeGuardedAllocation(void *memory)
{
    int found;
#ifdef SDL_PORT
    unsigned char *fence;
    unsigned int guardValue;
#else
    unsigned int *fence;
#endif
    GuardedAllocation *previous;
    GuardedAllocation *entry;
    int corrupt;
    int i;

    found = 0;
#ifdef SDL_PORT
    /* No fences to walk: the block starts where the caller's pointer does and
     * ASan owns the redzones on either side of it. */
    fence = (unsigned char *)memory;
#else
    fence = (unsigned int *)((unsigned char *)memory - 0x400);
#endif
    previous = 0;
    entry = g_pGuardedAllocationHead;
    while (entry != 0) {
        if (entry->block == fence) {
#ifndef SDL_PORT
            corrupt = 0;
            for (i = 0; i < 0x100; i++) {
                if (*fence != 0xabababab)
                    corrupt++;
                fence++;
            }
            if (corrupt != 0)
                ReportHeapGuardCorruption(memory, corrupt, 0);

            fence =
                (unsigned int *)((unsigned char *)memory + entry->size);
            corrupt = 0;
            for (i = 0; i < 0x100; i++) {
                if (*fence != 0xabababab)
                    corrupt++;
                fence++;
            }
            if (corrupt != 0)
                ReportHeapGuardCorruption(memory, corrupt, 1);

            fence = (unsigned int *)((unsigned char *)memory - 0x400);
#endif
            TrackFreedHeapBlock(fence);
            free(fence);
            if (previous != 0)
                previous->next = entry->next;
            else
                g_pGuardedAllocationHead = entry->next;
            if (g_9_pGuardedAllocationLastRef == entry) {
                if (previous != 0)
                    g_9_pGuardedAllocationLastRef = previous;
                else
                    g_9_pGuardedAllocationLastRef = 0;
            }
            free(entry);
            entry = 0;
            found = 1;
            break;
        }
        if (entry != 0) {
            previous = entry;
            entry = entry->next;
        }
    }
    if (found == 0 && DetectDoubleFree(fence) == 0) {
        sprintf(g_szGuardedHeapDiagnosticMessage,
                "MEM: Unknown memory lost (%p)\n", fence);
        WriteDebugString(g_szGuardedHeapDiagnosticMessage);
    }
    return;
}

/* Function start: 0x455A85 */
void SetMousePosition(int x, int y)
{
    SetCursorPos(x, y);
}

/* Function start: 0x455A9E */
/* Nothing in the shipped executable calls this.  Latches a scene escape
 * while the Escape key is down. */
void PollEscapeKey(void)
{
    if (GetAsyncKeyState(VK_ESCAPE) != 0)
        g_bSceneEscapeRequested = 1;
    return;
}

/* Function start: 0x455AC8 */
unsigned int PollKeyboardState(void)
{
    g_2_nClearedKeyboardState = 0;
    g_3_nBeginKeyboardState = GetAsyncKeyState(VK_HOME);
    g_13_nUpDirKeyState = GetAsyncKeyState(VK_UP);
    g_nPgUpwardKeyMode = GetAsyncKeyState(VK_PRIOR);
    g_102_nLeftDirKeyboardState = GetAsyncKeyState(VK_LEFT);
    g_nRightDirKeyboardState = GetAsyncKeyState(VK_RIGHT);
    g_18_nEndKeyState = GetAsyncKeyState(VK_END);
    g_15_nDownArrowKeyState = GetAsyncKeyState(VK_DOWN);
    g_1_nPageDownKeyboardMode = GetAsyncKeyState(VK_NEXT);
    g_nClearKeyMode = GetAsyncKeyState(VK_CLEAR);
    g_3_nOemPeriodKeyState = GetAsyncKeyState(0xbe);
    g_nOemCommaKeyState = GetAsyncKeyState(0xbc);
    g_nInsertKeyStatus = GetAsyncKeyState(VK_INSERT);
    g_nDeleteKeyState = GetAsyncKeyState(VK_DELETE);

    if (g_3_nBeginKeyboardState != 0)
        return 0x47;
    if (g_nPgUpwardKeyMode != 0)
        return 0x49;
    if (g_18_nEndKeyState != 0)
        return 0x4f;
    if (g_1_nPageDownKeyboardMode != 0)
        return 0x51;
    if (g_nInsertKeyStatus != 0 ||
        g_nOemCommaKeyState != 0)
        return 0x52;
    if (g_nDeleteKeyState != 0 ||
        g_3_nOemPeriodKeyState != 0)
        return 0x53;
    if (g_nClearKeyMode != 0)
        return 0x4c;
    if (g_13_nUpDirKeyState != 0) {
        if (g_102_nLeftDirKeyboardState != 0)
            return 0x47;
        if (g_nRightDirKeyboardState != 0)
            return 0x49;
        return 0x48;
    }
    if (g_15_nDownArrowKeyState != 0) {
        if (g_102_nLeftDirKeyboardState != 0)
            return 0x4f;
        if (g_nRightDirKeyboardState != 0)
            return 0x51;
        return 0x50;
    }
    if (g_102_nLeftDirKeyboardState != 0)
        return 0x4b;
    if (g_nRightDirKeyboardState != 0)
        return 0x4d;
    return 0;
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

/* Function start: 0x455EAA */
unsigned int GetKeyboardModifiers(void)
{
    return g_69_dwSysKeyValue;
}

/* Function start: 0x455EBF */
int GetControlKeyState(void)
{
    return GetAsyncKeyState(VK_CONTROL);
}

/* Function start: 0x455EDA */
int GetJKeyState(void)
{
    return GetAsyncKeyState(0x4a);
}

/* Function start: 0x455EF5 */
unsigned int GetGameClockTicks(void)
{
    return (GetTickCount() - g_128_dwGameClockBase) * 60 / 1000;
}

/* Function start: 0x455F23 */
void InitGameClockRandomEpoch(void)
{
    g_13_lGameClockStartData = GetTickCount();
    g_128_dwGameClockBase = GetTickCount() + (rand() & 3600000);
    return;
}

/* Function start: 0x455F59 */
void WriteDebugString(const char *s)
{
    if (s != 0) {
        OutputDebugString(s);
        if (g_nDebugFileLoggingEnabled != 0) {
            if (g_pDebugLogFile == 0)
                g_pDebugLogFile = fopen("log.log", "wt");
            fprintf(g_pDebugLogFile, "%s", s);
            fflush(g_pDebugLogFile);
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

#pragma function(memset)

/* Function start: 0x455FF1 */
/* Nothing in the shipped executable calls this.  Allocates a one-entry
 * glyph workspace of 5x5 pixels filled with colour 5. */
FontWorkspace **AllocateFontWorkspace(short fontIndex)
{
    int slot;
    FontWorkspace **workspace;

    workspace = 0;
    workspace = (FontWorkspace **)malloc(sizeof(*workspace));
    *workspace = (FontWorkspace *)malloc(sizeof(**workspace));
    for (slot = 0; slot < 1; slot++) {
        workspace[0][slot].width = 5;
        workspace[0][slot].height = 5;
        workspace[0][slot].pixels = (unsigned char *)malloc(
            (*workspace)->height * (*workspace)->width);
        memset(workspace[0][slot].pixels, 5,
               (*workspace)->height * (*workspace)->width);
    }
    return workspace;
}

/* Function start: 0x4560C0 */
/* Nothing in the shipped executable calls this.  Frees a workspace from
 * AllocateFontWorkspace. */
void FreeFontWorkspace(FontWorkspace **workspace)
{
    int slot;

    for (slot = 0; slot < 1; slot++)
        free(workspace[0][slot].pixels);
    free(*workspace);
    free(workspace);
    return;
}

/* Function start: 0x456123 */
char *LocateStreamsDirOnDisc(void)
{
#ifdef SDL_PORT
    char current[4096];
    const char *directory;
    int length;

    if (GetCurrentDirectoryA(sizeof(current), current) == 0)
        return 0;
    /* Only the final path component identifies GAMEDAT. Its spelling may
     * use any case, and an ancestor may also contain the word "gamedat". */
    directory = current + strlen(current);
    while (directory > current && directory[-1] != '/' &&
           directory[-1] != '\\')
        directory--;
    length = snprintf(g_9_szStreamsPath,
                      sizeof(g_9_szStreamsPath), "%s%s", current,
                      SDL_strcasecmp(directory, "gamedat") == 0
                          ? "/../streams/" : "/streams/");
    if (length < 0 || (size_t)length >= sizeof(g_9_szStreamsPath))
        return 0;
    if (!SetCurrentDirectoryA(g_9_szStreamsPath))
        return 0;
    if (!SetCurrentDirectoryA(current))
        return 0;
    return g_9_szStreamsPath;
#else
    char current[256];
    char *result;
    char drive;

    GetCurrentDirectoryA(0xff, current);
    result = g_9_szStreamsPath;
    drive = FindCdRomDriveByVolumeLabel("<anydisc>", "\\wc2\\streams\\");
    if (drive != 0) {
        sprintf(g_9_szStreamsPath, "%c:%s", drive,
                "\\wc2\\streams\\");
    } else {
        if (strstr(current, "gamedat") != 0)
            sprintf(g_9_szStreamsPath, "%s\\..\\%s", current,
                    "streams\\");
        else
            sprintf(g_9_szStreamsPath, "%s\\%s", current,
                    "streams\\");
        GetCurrentDirectoryA(0xff, current);
        if (!SetCurrentDirectoryA(g_9_szStreamsPath))
            result = 0;
        SetCurrentDirectoryA(current);
    }
    return result;
#endif
}

#pragma function(strcmp)
/* Function start: 0x456236 */
char FindCdRomDriveByVolumeLabel(const char *label,
                                 const char *directory)
{
    char volumeName[256];
    char drivePath[12];
    char systemName[64];
    DWORD volumeFlags;
    DWORD maximumComponentLength;
    int count;
    char drives[26];
    char rootPath[12];
    int device;
    char answer;

    count = 0;
    for (device = 'a'; device <= 'z'; device++) {
        sprintf(rootPath, "%c:\\", (char)device);
        if (GetDriveTypeA(rootPath) == DRIVE_CDROM) {
            drives[count] = (char)device;
            count++;
        }
    }

    answer = 0;
    for (device = 0; device < count; device++) {
        sprintf(drivePath, "%c:\\", drives[device]);
        GetVolumeInformationA(drivePath, volumeName, 0xff, 0,
                              &maximumComponentLength, &volumeFlags,
                              systemName, sizeof(systemName));
        if (strcmp(label, "<anydisc>") == 0) {
            if (SetCurrentDirOnDrive(drives[device], directory) != 0) {
                answer = drives[device];
                break;
            }
        } else if (strcmp(volumeName, label) == 0) {
            if (SetCurrentDirOnDrive(drives[device], directory) != 0) {
                answer = drives[device];
                break;
            }
        }
    }
    return answer;
}

#pragma intrinsic(strcmp)
/* Function start: 0x4563A3 */
int SetCurrentDirOnDrive(char drive, const char *directory)
{
    char path[256];
    char previous[256];
    int result;

    result = 0;
    GetCurrentDirectoryA(0xff, previous);
    sprintf(path, "%c:%s", drive, directory);
    if (SetCurrentDirectoryA(path) != 0)
        result = 1;
    SetCurrentDirectoryA(previous);
    return result;
}

/* Function start: 0x45641B */
int TakeInputPressCount(void)
{
    int pressCount;

    pressCount = g_nInputPressCount;
    g_nInputPressCount = 0;
    return pressCount;
}

#pragma intrinsic(strcpy)
/* Function start: 0x456443 */
int PromptInsertCorrectCd(void)
{
    int discNumber;
    char title[18];
    char message[1024];

    discNumber = 1;
    strcpy(title, "Insert Correct CD");
    sprintf(message,
            "Please place The Kilrathi Saga disc %d into your CD-ROM drive and click OK\n"
            "or click cancel to quit",
            discNumber);
    while (LocateStreamsDirOnDisc() == 0) {
        if (MessageBoxA(0, message, title,
                        MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
            return 0;
    }
    return 1;
}

/* This unit's initialized data, after its functions so that it does not
 * move the functions' symbol keys. */
int g_bDebugBreakEnabled = 0; /* 0x0049c238 */
int g_bApplicationShutdownStarted = 0; /* 0x0049c23c */
int g_bF1KeyDown = 0; /* 0x0049c240 */
int g_nAudioEnabled = 1; /* 0x0049c244 */
int g_bInputEventQueueEnabled = 0; /* 0x0049c248 */
unsigned int g_dwGuardedAllocationTotalBytes = 0; /* 0x0049c24c */
unsigned int g_dwGuardedAllocationBytes = 0; /* 0x0049c250 */
unsigned int g_dwGuardedAllocationPeakBytes = 0; /* 0x0049c254 */
int g_nInputPressCount = 0; /* 0x0049c258 */
int g_bApplicationControllerActive = 0; /* 0x0049c25c */
int g_bShowFrameRate = 0; /* 0x0049c260 */
int g_bConfigQuickModeEnabled = 1; /* 0x0049c264 */
int g_bUseHardwarePalette = 1; /* 0x0049c268 */
int g_bCockpitEnabled = 1; /* 0x0049c26c */
int g_bSkipCampaignVideo = 0; /* 0x0049c270 */
int g_bWindowInactive = 0; /* 0x0049c274 */
static char g_szWindowClassName[45] =
    "Wing Commander II: Vengeance of the Kilrathi"; /* 0x0049c278 */
static char g_szWindowTitle[45] =
    "Wing Commander II: Vengeance of the Kilrathi"; /* 0x0049c2a8 */
int g_nDebugFileLoggingEnabled = 0; /* 0x0049c2d8 */
FILE *g_pDebugLogFile = 0; /* 0x0049c2dc */
int g_bConfigMemoryOption = 0; /* 0x0049c2e0 */
int g_bWindowMessagePumpActive = 0; /* 0x0049c2e4 */
int g_nUnusedWinMainWord1 = 0; /* 0x0049c2e8 */
int g_nPendingMouseX = 0; /* 0x0049c2ec */
int g_nPendingMouseY = 0; /* 0x0049c2f0 */
int g_nUnusedWinMainWord2 = 0; /* 0x0049c2f4 */
int g_bJoystickPrimaryButtonLatched = 0; /* 0x0049c2f8 */
int g_bJoystickSecondaryButtonLatched = 0; /* 0x0049c2fc */
GuardedAllocation *g_pGuardedAllocationHead = 0; /* 0x0049c300 */
FreedHeapBlock *g_pFreedHeapBlockHead = 0; /* 0x0049c304 */
FreedHeapBlock *g_pFreedHeapBlockTail = 0; /* 0x0049c308 */

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
unsigned int g_69_dwSysKeyValue; /* 0x005d10a4 */
JOYINFO g_1_aJoystickInfoArray[2]; /* 0x005d10b0 */
HostMouseMessage g_6_stHostMouseTextBlock; /* 0x005d10d0 */
HWND g_10_hMainWindow; /* 0x005d10e0 */
HANDLE g_19_hOneProcessLock; /* 0x005d10e4 */
unsigned long g_128_dwGameClockBase; /* 0x005d10e8 */
GuardedAllocation *g_9_pGuardedAllocationLastRef; /* 0x005d10ec */
char g_szGuardedHeapDiagnosticMessage[0x80]; /* 0x005d10f0 */
char g_29_abMemoryLoadNotice[0x80]; /* 0x005d1170 */
int g_5_nIdleWinmainWord[32]; /* 0x005d11f0 */ /* never referenced */
int g_39_bAltNumpadAddHotkeySwitch; /* 0x005d1270 */
int g_27_nLastAltCommandScanNumberWord; /* 0x005d1274 */
HDC g_18_hMainWindowContext; /* 0x005d1278 */
int g_2_bAltFHotkeyFlag; /* 0x005d127c */
int g_6_bAltDChordState; /* 0x005d1280 */
int g_bJoystickCalibrationHotkey; /* 0x005d1284 */
HCURSOR g_1_hMainWndCursor; /* 0x005d1288 */
int g_2_bAltHHotkey; /* 0x005d128c */
int g_4_bMenuBHotkey; /* 0x005d1290 */
int g_bMenuAHotkey; /* 0x005d1294 */
int g_6_bMenuTHotkeySwitch; /* 0x005d1298 */
unsigned int g_48_dwGameExitClockValue; /* 0x005d129c */
int g_3_nSavedOwnerCursorX; /* 0x005d12a0 */
int g_102_nStoredWindowCursorYWord; /* 0x005d12a4 */
int g_bAltNumpadSubtractShortcut; /* 0x005d12a8 */
int g_16_bMainWindowRunning; /* 0x005d12ac */
HINSTANCE g_7_hAppCopy; /* 0x005d12b0 */
unsigned int g_11_dwGameZeroTime; /* 0x005d12b4 */
long g_13_lGameClockStartData; /* 0x005d12b8 */
