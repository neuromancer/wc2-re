/*
 *  Win32 data-file wrappers, scaled interstitial text, canned sequences,
 *  and the MONODEBG.VXD developer console.
 *
 *  Address range 0x403500-0x403e4f (provisional -- see docs/ORDER.md).
 *  Boundary evidence: MonoDebug_install/MonoDebug_print anchor the final block;
 *  the Mac auto unit starts at visit_the_cinema (0x403e50).
 */
#include "wc1.h"

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
#ifdef WC1_SDL
    /* WC2 stores the descriptor in a word and then compares it against a
     * full -1, so a failed open never reaches the branch above and the caller
     * reports a bare error code instead.  The port says which file it could
     * not create and why, because on a host the reason is usually the
     * directory's permissions. */
    if ((short)fd == -1) {
        fprintf(stderr, "Unable to create '%s' in '%s': %s\n", path,
                Wc1SdlDescribeWorkingDirectory(), strerror(errno));
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

/* Function start: WC2_UNMAPPED */
short GetLineLength(const char *text)
{
    short width;
    char c;

    width = 0;
    for (;;) {
        c = *text;
        text++;
        if (c == 0)
            break;
        if (c >= 'A' && c <= 'z') {
            c = (char)(c - 'A');
            width = width + GetShapeFrameExtent(
                0, 0, g_pIntroFont_005a8960, c, 2);
            width = width + 2;
        } else if (c == '.') {
            width = width + GetShapeFrameExtent(
                0, 0, g_pIntroFont_005a8960, 58, 2);
            width = width + 2;
        } else if (c == ',') {
            width = width + GetShapeFrameExtent(
                0, 0, g_pIntroFont_005a8960, 59, 2);
            width = width + 2;
        } else if (c == ' ') {
            width = width + 6;
        } else if (c == '\n') {
            break;
        }
    }
    return width;
}

/* Function start: WC2_UNMAPPED */
int print_subtitle(Viewport *viewport, short colour, const char *text)
{
    const char *scan;
    short lines;
    short x;
    short y;
    char c;

    (void)colour;
    lines = 1;
    scan = text;
    c = *scan;
    scan++;
    while (c != 0) {
        if (c == '\n')
            lines++;
        c = *scan;
        scan++;
    }
    lines = (short)(lines * 16);
    y = (short)((128 - lines) / 2);
    x = (short)((320 - GetLineLength(text)) >> 1);
    for (;;) {
        c = *text;
        text++;
        if (c == 0)
            break;
        if (c >= 'A' && c <= 'z') {
            c = (char)(c - 'A');
            DrawSpriteDefault(viewport, x, y, g_pIntroFont_005a8960, c);
            x = x + GetShapeFrameExtent(
                0, 0, g_pIntroFont_005a8960, c, 2);
            x = x + 2;
        } else if (c == ' ') {
            x = x + 6;
        } else if (c == '.') {
            DrawSpriteDefault(viewport, x, y, g_pIntroFont_005a8960, 58);
            x = x + GetShapeFrameExtent(
                0, 0, g_pIntroFont_005a8960, 58, 2);
            x = x + 2;
        } else if (c == ',') {
            DrawSpriteDefault(viewport, x, y, g_pIntroFont_005a8960, 59);
            x = x + GetShapeFrameExtent(
                0, 0, g_pIntroFont_005a8960, 59, 2);
            x = x + 2;
        } else if (c == '\n') {
            y = y + 16;
            x = (short)((320 - GetLineLength(text)) >> 1);
        }
    }
    if (viewport->pixels == g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
    return 0;
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
