/*
 *  Data file I/O.
 *
 *  Retail object 0x45A010-0x45A2BF (link order; see docs/ORDER.md).
 */
/* This unit owns storage declared in globals.h as statics. */
#define DATAFILE_C
#include "game.h"

short g_nPacketError_0049ca90 = 0;

/* Function start: 0x45A010 */
short OpenDataFileOrDie(const char *path)
{
    short fd = (short)_open(path, 0x8002);

    g_nPacketError_0049ca90 = (short)errno;
    return fd;
}

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
        sprintf(g_szCreateBlockFileError_005b3888, "!_open '%s'\n", path);
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
        sprintf(g_szReadDataFileFault_005b3848, "!lseek %d\n", offset);
        g_nPacketError_0049ca90 = (short)errno;
        return 0;
    }
    if (_read(fd, data, length) == -1) {
        sprintf(g_szReadDataFileFault_005b3848, "!lseek %d\n", offset);
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
        sprintf(g_szSeekDataFileFailure_005b3908, "!lseek %d\n", offset);
        g_nPacketError_0049ca90 = (short)errno;
    }
    return position;
}
