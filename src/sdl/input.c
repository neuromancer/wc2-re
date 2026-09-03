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

BOOL TextOutA(HDC dc, int x, int y, const char *text, int length)
{
    (void)dc;
    (void)x;
    (void)y;
    (void)text;
    (void)length;
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

int SdlGetAsyncKeyState(int virtualKey)
{
    const Uint8 *keys;
    SDL_Scancode scanCode;

    SDL_PumpEvents();
    keys = SDL_GetKeyboardState(0);
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
    case VK_PRIOR:
        scanCode = SDL_SCANCODE_PAGEUP;
        break;
    case VK_NEXT:
        scanCode = SDL_SCANCODE_PAGEDOWN;
        break;
    case VK_END:
        scanCode = SDL_SCANCODE_END;
        break;
    case VK_HOME:
        scanCode = SDL_SCANCODE_HOME;
        break;
    case VK_LEFT:
        scanCode = SDL_SCANCODE_LEFT;
        break;
    case VK_UP:
        scanCode = SDL_SCANCODE_UP;
        break;
    case VK_RIGHT:
        scanCode = SDL_SCANCODE_RIGHT;
        break;
    case VK_DOWN:
        scanCode = SDL_SCANCODE_DOWN;
        break;
    case VK_INSERT:
        scanCode = SDL_SCANCODE_INSERT;
        break;
    case VK_DELETE:
        scanCode = SDL_SCANCODE_DELETE;
        break;
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
