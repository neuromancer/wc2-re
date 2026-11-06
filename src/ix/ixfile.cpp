/*
 *  ix -- stdio wrappers used by the streamer and its thread.
 *
 *  Original address range: 0x0048C400 - 0x0048C589
 *  A separate object from the LZO decoder before it, pulled from a library
 *  of its own after the decoder's.
 */
#include "ix.h"
#include <stdlib.h>
#include <string.h>
#ifdef SDL_PORT
#include <errno.h>
#include <limits.h>
#endif

#pragma function(strcat)

/* Function start: 0x48C400 */
/* Unreferenced, like the matching pair in WC1: the file module's empty init
 * and shutdown entry points.  Nothing calls them, so ix.h does not declare
 * them. */
extern "C" void ix_file_init_hook(void)
{
}

/* Function start: 0x48C410 */
extern "C" void ix_file_shutdown_hook(void)
{
}

/* Function start: 0x48C420 */
extern "C" FILE *ix_file_open(const char *path, int mode)
{
    char modeString[4];
#ifdef SDL_PORT
    char resolvedPath[PATH_MAX];
    FILE *file;
#endif

    modeString[0] = 0;
    if ((mode & 1) != 0)
        strcat(modeString, "r");
    if ((mode & 2) != 0)
        strcat(modeString, "w");
    strcat(modeString, "b");
#ifdef SDL_PORT
    if (!SdlResolvePath(path, resolvedPath, sizeof(resolvedPath))) {
        fprintf(stderr, "Unable to resolve audio file path '%s'.\n",
                path != 0 ? path : "(null)");
        return 0;
    }
    file = fopen(resolvedPath, modeString);
    if (file == 0)
        fprintf(stderr, "Unable to open audio file '%s': %s\n",
                resolvedPath, strerror(errno));
    return file;
#else
    return fopen(path, modeString);
#endif
}

/* Function start: 0x48C48E */
extern "C" void ix_file_close(FILE *file)
{
    fclose(file);
}

/* Function start: 0x48C4AA */
extern "C" void ix_file_seek(FILE *file, long position)
{
    fseek(file, position, SEEK_SET);
}

/* Function start: 0x48C4CC */
extern "C" long ix_file_tell(FILE *file)
{
    return ftell(file);
}

/* Function start: 0x48C4E8 */
extern "C" unsigned int ix_file_read(FILE *file, void *destination,
                                      unsigned int bytes)
{
    return fread(destination, 1, bytes, file);
}

/* Function start: 0x48C50E */
/* Unreferenced in WC2. */
extern "C" unsigned int ix_file_write(FILE *file, const void *source,
                                       unsigned int bytes)
{
    return fwrite(source, 1, bytes, file);
}

/* Function start: 0x48C534 */
extern "C" long ix_file_size(FILE *file)
{
    long position;
    long size;

    position = ftell(file);
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, position, SEEK_SET);
    return size;
}
