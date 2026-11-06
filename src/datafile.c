/*
 *  Data file I/O.
 *
 *  Retail object 0x45A010-0x45A2BF (link order; see docs/ORDER.md).
 */
/* This unit owns storage declared in globals.h as statics. */
#define DATAFILE_C
#include "game.h"

short g_nPacketError = 0; /* 0x0049ca90 */

/* Function start: 0x45A010 */
short OpenDataFileOrDie(const char *path)
{
    short fd = (short)_open(path, 0x8002);

    g_nPacketError = (short)errno;
    return fd;
}

/* Function start: 0x45A04A */
void CloseDataFile(unsigned short fd)
{
    g_nPacketError = (short)_close(fd & 0xffff);
}

/* Function start: 0x45A06C */
short WriteDataFileAtOffset(unsigned short fd, int offset,
                            unsigned int length, const void *data)
{
    g_nPacketError = 0;
    if (offset != -1 && _lseek(fd, offset, SEEK_SET) == -1) {
        sprintf(g_szWriteDataFileError, "!lseek %d\n", offset);
        g_nPacketError = (short)errno;
        return 0;
    }
    if (_write(fd, data, length) == -1) {
        sprintf(g_szWriteDataFileError, "!write %d\n", offset);
        g_nPacketError = (short)errno;
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
        sprintf(g_szCreateBlockFileError, "!_open '%s'\n", path);
        g_nPacketError = (short)errno;
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
    g_nPacketError = 0;
    if (offset != -1 && _lseek(fd, offset, SEEK_SET) == -1) {
        sprintf(g_1_szReadDataFileFault, "!lseek %d\n", offset);
        g_nPacketError = (short)errno;
        return 0;
    }
    if (_read(fd, data, length) == -1) {
        sprintf(g_1_szReadDataFileFault, "!lseek %d\n", offset);
        g_nPacketError = (short)errno;
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
        sprintf(g_szSeekDataFileFailure, "!lseek %d\n", offset);
        g_nPacketError = (short)errno;
    }
    return position;
}
