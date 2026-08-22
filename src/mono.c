/*
 *  Win32 data-file wrappers, scaled interstitial text, canned sequences,
 *  and the MONODEBG.VXD developer console.
 *
 *  Address range 0x403500-0x403e4f (provisional -- see docs/ORDER.md).
 *  Boundary evidence: MonoDebug_install/MonoDebug_print anchor the final block;
 *  the Mac auto unit starts at visit_the_cinema (0x403e50).
 */
#include "game.h"

static char g_szSoundDebugBuffer_005d1f00[0xfa0];

/* Function start: 0x45A04A */
void CloseDataFile(unsigned short fd)
{
    g_nPacketError_0049ca90 = (short)_close(fd & 0xffff);
}

/* Function start: 0x45A06C */
short WriteDataFileAtOffset(unsigned short fd, int offset,
                            unsigned int length, const void *data)
{
    g_nPacketError_0049ca90 = 0;
    if (offset != -1 && _lseek(fd, offset, SEEK_SET) == -1) {
        sprintf(g_szWriteDataFileError_005b38c8, "!lseek %d\n", offset);
        g_nPacketError_0049ca90 = (short)errno;
        return 0;
    }
    if (_write(fd, data, length) == -1) {
        sprintf(g_szWriteDataFileError_005b38c8, "!write %d\n", offset);
        g_nPacketError_0049ca90 = (short)errno;
        return 0;
    }
    return 1;
}

/* Function start: 0x45A12D */
short CreateDataFile(const char *path)
{
    unsigned short fd;

    fd = (unsigned short)_open(path, 0x8101, 0x180);
    if ((unsigned int)fd == (unsigned int)-1) {
        sprintf(g_szCreateDataFileError_005b3888, "!_open '%s'\n", path);
        g_nPacketError_0049ca90 = (short)errno;
        return 0;
    }
#ifdef SDL_PORT
    /* WC2 stores the descriptor in a word and then compares it against a
     * full -1, so a failed open never reaches the branch above and the caller
     * reports a bare error code instead.  The port says which file it could
     * not create and why, because on a host the reason is usually the
     * directory's permissions. */
    if ((short)fd == -1) {
        fprintf(stderr, "Unable to create '%s' in '%s': %s\n", path,
                SdlDescribeWorkingDirectory(), strerror(errno));
    }
#endif
    return (short)fd;
}

/* Function start: 0x45A19B */
int ReadDataFileAtOffset(unsigned short fd, int offset,
                         unsigned int length, void *data)
{
    g_nPacketError_0049ca90 = 0;
    if (offset != -1 && _lseek(fd, offset, SEEK_SET) == -1) {
        sprintf(g_szReadDataFileError_005b3848, "!lseek %d\n", offset);
        g_nPacketError_0049ca90 = (short)errno;
        return 0;
    }
    if (_read(fd, data, length) == -1) {
        sprintf(g_szReadDataFileError_005b3848, "!lseek %d\n", offset);
        g_nPacketError_0049ca90 = (short)errno;
        return 0;
    }
    return 1;
}

/* Function start: 0x45A259 */
int SeekDataFile(unsigned short fd, int offset,
                 unsigned int origin)
{
    int position;

    position = _lseek(fd, offset, origin & 0xffff);
    if (position == -1) {
        sprintf(g_szSeekDataFileError_005b3908, "!lseek %d\n", offset);
        g_nPacketError_0049ca90 = (short)errno;
    }
    return position;
}

/* Function start: 0x437760 */
void SplitGameClockTicks(unsigned char *parts)
{
    int ticks = (int)GetGameClockTicks();

    parts[0] = (unsigned char)(ticks % 60);
    ticks = ticks / 60;
    parts[1] = (unsigned char)(ticks % 60);
    ticks = ticks / 60;
    parts[2] = (unsigned char)(ticks % 60);
    ticks = ticks / 60;
    parts[3] = (unsigned char)(ticks % 24);
}

/* Function start: 0x4377F0 */
void MonoDebug_install(void)
{
    unsigned int version;

    g_hMonoDebugDevice_005b30ec =
        CreateFileA("\\\\.\\MONODEBG.VXD", 0, 0, 0, CREATE_ALWAYS,
                    FILE_FLAG_DELETE_ON_CLOSE, 0);
    if (g_hMonoDebugDevice_005b30ec == INVALID_HANDLE_VALUE)
        return;

    if (!DeviceIoControl(g_hMonoDebugDevice_005b30ec, 1, 0, 0,
                         &version, sizeof(version), 0, 0)) {
        CloseHandle(g_hMonoDebugDevice_005b30ec);
        return;
    }
    if (version != 0x20004) {
        CloseHandle(g_hMonoDebugDevice_005b30ec);
        exit_squadron("MonoDebug__install expecting version");
        return;
    }
    if (!DeviceIoControl(g_hMonoDebugDevice_005b30ec, 2, 0, 0,
                         0, 0, 0, 0)) {
        CloseHandle(g_hMonoDebugDevice_005b30ec);
        exit_squadron("MonoDebug__install init failed");
        return;
    }
    g_bMonoDebugInstalled_005b30e8 = 1;
}

/* Function start: 0x4378D9 */
void MonoDebug_remove(void)
{
    if (g_bMonoDebugInstalled_005b30e8 != 0) {
        CloseHandle(g_hMonoDebugDevice_005b30ec);
        g_bMonoDebugInstalled_005b30e8 = 0;
    }
}

/* Function start: 0x437946 */
void SoundDebugPrintf(const char *fmt, ...)
{
    va_list arguments;

    va_start(arguments, fmt);
    vsprintf(g_szSoundDebugBuffer_005d1f00, fmt, arguments);
    va_end(arguments);
    MonoDebug_print(g_szSoundDebugBuffer_005d1f00);
}

/* Function start: 0x437983 */
void MonoDebug_print(const char *text)
{
    if (g_bMonoDebugInstalled_005b30e8 != 0) {
        if (!DeviceIoControl(g_hMonoDebugDevice_005b30ec, 9,
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
