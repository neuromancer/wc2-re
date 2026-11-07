/*
 *  MONODEBG developer channel.
 *
 *  Retail object 0x4377F0-0x4379EF (link order; see docs/ORDER.md).
 */
/* This unit owns the mono storage declared in globals.h (as statics). */
#define MONO_C
#include "game.h"

extern char g_12_szSoundTraceScratch[0x200]; /* 0x005d1f00 */

/* Function start: 0x4377F0 */
void MonoDebug_install(void)
{
    unsigned int version;

    g_hMonoDebugDevice =
        CreateFileA("\\\\.\\MONODEBG.VXD", 0, 0, 0, CREATE_ALWAYS,
                    FILE_FLAG_DELETE_ON_CLOSE, 0);
    if (g_hMonoDebugDevice == INVALID_HANDLE_VALUE)
        return;

    if (!DeviceIoControl(g_hMonoDebugDevice, 1, 0, 0,
                         &version, sizeof(version), 0, 0)) {
        CloseHandle(g_hMonoDebugDevice);
        return;
    }
    if (version != 0x20004) {
        CloseHandle(g_hMonoDebugDevice);
        exit_squadron("MonoDebug__install expecting version");
        return;
    }
    if (!DeviceIoControl(g_hMonoDebugDevice, 2, 0, 0,
                         0, 0, 0, 0)) {
        CloseHandle(g_hMonoDebugDevice);
        exit_squadron("MonoDebug__install init failed");
        return;
    }
    g_bMonoDebugInstalled = 1;
}

/* Function start: 0x4378D9 */
void MonoDebug_remove(void)
{
    if (g_bMonoDebugInstalled != 0) {
        CloseHandle(g_hMonoDebugDevice);
        g_bMonoDebugInstalled = 0;
    }
}

/* Function start: 0x437907 */
/* Nothing in the shipped executable calls this.  It sends control code 3 to
 * MONODEBG.VXD (install uses 1 and 2, print uses 9) and removes the channel
 * when the driver rejects it; what code 3 asks the driver to do is unknown. */
void SendMonoDebugCommand3(void)
{
    if (g_bMonoDebugInstalled != 0) {
        if (!DeviceIoControl(g_hMonoDebugDevice, 3, 0, 0, 0, 0, 0, 0))
            MonoDebug_remove();
    }
}

/* Function start: 0x437946 */
void SoundDebugPrintf(const char *fmt, ...)
{
    va_list arguments;

    va_start(arguments, fmt);
    vsprintf(g_12_szSoundTraceScratch, fmt, arguments);
    va_end(arguments);
    MonoDebug_print(g_12_szSoundTraceScratch);
}

/* Function start: 0x437983 */
void MonoDebug_print(const char *text)
{
    if (g_bMonoDebugInstalled != 0) {
        if (!DeviceIoControl(g_hMonoDebugDevice, 9,
                             (void *)text, 0xfa0, 0, 0, 0, 0)) {
            exit_squadron("MonoDebug::print failed (buffer possibly on stack?!)");
            MonoDebug_remove();
        }
    }
}

/* Function start: 0x4379D4 */
void ReadPerformanceCounter(LARGE_INTEGER *p)
{
    QueryPerformanceCounter(p);
}
