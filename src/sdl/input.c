#include "sdl_port.h"

#include "video_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HANDLE CreateFileA(const char *path, DWORD desiredAccess, DWORD shareMode,
                   LPVOID security, DWORD creationDisposition,
                   DWORD flags, HANDLE templateFile)
{
    (void)path;
    (void)desiredAccess;
    (void)shareMode;
    (void)security;
    (void)creationDisposition;
    (void)flags;
    (void)templateFile;
    return INVALID_HANDLE_VALUE;
}

BOOL DeviceIoControl(HANDLE device, DWORD controlCode, LPVOID input,
                     DWORD inputBytes, LPVOID output, DWORD outputBytes,
                     DWORD *returnedBytes, LPVOID overlapped)
{
    (void)device;
    (void)controlCode;
    (void)input;
    (void)inputBytes;
    (void)output;
    (void)outputBytes;
    (void)overlapped;
    if (returnedBytes != 0)
        *returnedBytes = 0;
    return FALSE;
}

DWORD GetCurrentDirectoryA(DWORD size, char *path)
{
#ifdef _WIN32
    wchar_t *widePath;
    char *utf8Path;
    size_t pathLength;

    /* This is the port shim behind the game's Win32 spelling.  Its callers
     * pass the result to SDL/Slint, so return UTF-8 rather than ANSI. */
    if (path == 0 || size == 0)
        return 0;
    widePath = _wgetcwd(0, 0);
    if (widePath == 0)
        return 0;
    utf8Path = SDL_iconv_wchar_utf8(widePath);
    free(widePath);
    if (utf8Path == 0)
        return 0;
    pathLength = strlen(utf8Path);
    if (pathLength + 1 > size) {
        SDL_free(utf8Path);
        return 0;
    }
    memcpy(path, utf8Path, pathLength + 1);
    SDL_free(utf8Path);
    return (DWORD)pathLength;
#else
    if (path == 0 || size == 0 || getcwd(path, size) == 0)
        return 0;
    return (DWORD)strlen(path);
#endif
}

UINT GetDriveTypeA(const char *root)
{
    (void)root;
    return 0;
}

BOOL GetVolumeInformationA(const char *root, char *volume,
                           DWORD volumeSize, DWORD *serial,
                           DWORD *maximumComponentLength, DWORD *flags,
                           char *filesystem, DWORD filesystemSize)
{
    (void)root;
    (void)volume;
    (void)volumeSize;
    (void)serial;
    (void)maximumComponentLength;
    (void)flags;
    (void)filesystem;
    (void)filesystemSize;
    return FALSE;
}

int MessageBoxA(HWND window, const char *text, const char *title, UINT type)
{
    Uint32 messageFlags;

    messageFlags = (type & MB_ICONERROR) != 0
                       ? SDL_MESSAGEBOX_ERROR
                       : SDL_MESSAGEBOX_WARNING;
    SDL_ShowSimpleMessageBox(messageFlags, title, text,
                             (SDL_Window *)window);
    return (type & MB_OKCANCEL) != 0 ? IDCANCEL : IDOK;
}

BOOL QueryPerformanceCounter(LARGE_INTEGER *counter)
{
    counter->QuadPart = (int64_t)SDL_GetPerformanceCounter();
    return TRUE;
}

BOOL SetCurrentDirectoryA(const char *path)
{
    return SdlChangeDirectory(path) == 0;
}

/* ThrottleFrameAndDrawFps is the only caller: it draws the -f frame rate
 * over the window. There is no window DC here, so show it in the title. */
BOOL TextOutA(HDC dc, int x, int y, const char *text, int length)
{
    (void)dc;
    (void)x;
    (void)y;
    if (text != 0 && length > 0)
        SdlShowFrameRateText(text);
    return TRUE;
}

void InitializeCriticalSection(CRITICAL_SECTION *criticalSection)
{
    criticalSection->mutex = SDL_CreateMutex();
}

void DeleteCriticalSection(CRITICAL_SECTION *criticalSection)
{
    if (criticalSection->mutex != 0) {
        SDL_DestroyMutex(criticalSection->mutex);
        criticalSection->mutex = 0;
    }
}

void EnterCriticalSection(CRITICAL_SECTION *criticalSection)
{
    SDL_LockMutex(criticalSection->mutex);
}

void LeaveCriticalSection(CRITICAL_SECTION *criticalSection)
{
    SDL_UnlockMutex(criticalSection->mutex);
}

DWORD SdlGetTicks(void)
{
    return SDL_GetTicks();
}

static const Uint8 *g_pSdlKeyboardSnapshot;

/* SDL_PushEvent does not update SDL's polled keyboard state. Native flight
 * tests supply the same held keys through both input paths, as in WC1. */
void SdlSetKeyboardSnapshot(const Uint8 *keys)
{
    g_pSdlKeyboardSnapshot = keys;
}

int SdlGetAsyncKeyState(int virtualKey)
{
    const Uint8 *keys;
    SDL_Scancode scanCode;

    keys = g_pSdlKeyboardSnapshot;
    if (keys == 0) {
        SDL_PumpEvents();
        keys = SDL_GetKeyboardState(0);
    }
    /* In flight, rebound keys hold the keys the game polls. */
    keys = SdlPadKeyboardState(SdlKeymapKeyboardState(keys));
    switch (virtualKey) {
    case VK_SHIFT:
        return (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT])
            ? 0x8000 : 0;
    case VK_CONTROL:
        return (keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL])
            ? 0x8000 : 0;
    case VK_CLEAR:
        scanCode = SDL_SCANCODE_KP_5;
        break;
    /* The keypad steers as these keys do in SdlTranslateScanCode, as it
     * does on Windows with Num Lock off. */
    case VK_PRIOR:
        return (keys[SDL_SCANCODE_PAGEUP] || keys[SDL_SCANCODE_KP_9])
            ? 0x8000 : 0;
    case VK_NEXT:
        return (keys[SDL_SCANCODE_PAGEDOWN] || keys[SDL_SCANCODE_KP_3])
            ? 0x8000 : 0;
    case VK_END:
        return (keys[SDL_SCANCODE_END] || keys[SDL_SCANCODE_KP_1])
            ? 0x8000 : 0;
    case VK_HOME:
        return (keys[SDL_SCANCODE_HOME] || keys[SDL_SCANCODE_KP_7])
            ? 0x8000 : 0;
    case VK_LEFT:
        return (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_KP_4])
            ? 0x8000 : 0;
    case VK_UP:
        return (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_KP_8])
            ? 0x8000 : 0;
    case VK_RIGHT:
        return (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_KP_6])
            ? 0x8000 : 0;
    case VK_DOWN:
        return (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_KP_2])
            ? 0x8000 : 0;
    case VK_INSERT:
        return (keys[SDL_SCANCODE_INSERT] || keys[SDL_SCANCODE_KP_0])
            ? 0x8000 : 0;
    case VK_DELETE:
        return (keys[SDL_SCANCODE_DELETE] || keys[SDL_SCANCODE_KP_PERIOD])
            ? 0x8000 : 0;
    case 0xbc:
        scanCode = SDL_SCANCODE_COMMA;
        break;
    case 0xbe:
        scanCode = SDL_SCANCODE_PERIOD;
        break;
    /* WC2 polls five keys WC1 never did.  Space and Return are the fire and
     * lock buttons the flight loop reads through KeyboardMousePump, and the
     * keypad three set the cursor step; without them the shim answered "up"
     * for every one of them. */
    case VK_SPACE:
        scanCode = SDL_SCANCODE_SPACE;
        break;
    case VK_RETURN:
        return (keys[SDL_SCANCODE_RETURN] ||
                keys[SDL_SCANCODE_KP_ENTER]) ? 0x8000 : 0;
    case VK_NUMLOCK:
        scanCode = SDL_SCANCODE_NUMLOCKCLEAR;
        break;
    case VK_ADD:
        scanCode = SDL_SCANCODE_KP_PLUS;
        break;
    case VK_SUBTRACT:
        scanCode = SDL_SCANCODE_KP_MINUS;
        break;
    case VK_F1:
        scanCode = SDL_SCANCODE_F1;
        break;
    default:
        return 0;
    }
    return keys[scanCode] ? 0x8000 : 0;
}

void SdlOutputDebugString(const char *text)
{
    fputs(text, stderr);
    fflush(stderr);
}

int SdlSetCursorPosition(int x, int y)
{
    SDL_Window *window;
    int windowX;
    int windowY;

    window = SDL_GetKeyboardFocus();
    if (window == 0)
        window = SDL_GetMouseFocus();
    if (window == 0)
        return FALSE;
    if (!SdlMapLogicalToWindow(
            window, x, y, &windowX, &windowY))
        return FALSE;
    SDL_WarpMouseInWindow(window, windowX, windowY);
    return TRUE;
}

void SdlSleep(DWORD milliseconds)
{
    SDL_Delay(milliseconds);
}
