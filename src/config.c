/*
 *  Reads the WINGCMDR.CFG installation configuration.
 *
 *  Retail object 0x401000-0x40111F (link order; see docs/ORDER.md).
 */
#include "game.h"

#pragma function(strcpy)
/* Function start: 0x401000 */
short LoadWingCmdrCfgFile(short argc, char **argv)
{
    FILE *file;
    char *destination;
    short argumentIndex;
    short argumentCount;
    short result;
#ifdef SDL_PORT
    char resolvedPath[PATH_MAX];
#endif

    argumentCount = 0;
    argumentIndex = 1;
    destination = g_szTextScratchBuffer_005d1c40;
#ifdef SDL_PORT
    if (SdlResolvePath("wc2.cfg", resolvedPath,
                          sizeof(resolvedPath)))
        file = fopen(resolvedPath, "rb");
    else
        file = 0;
#else
    file = fopen("wc2.cfg", "rb");
#endif
    while (file != 0) {
        result = (short)fscanf(file, "%s", destination);
        if (result == -1) {
            fclose(file);
            break;
        }
        g_pInitialArguments_005c57f0[argumentCount++] =
            destination;
        destination = strchr(destination, 0);
        destination++;
    }

    while (argc-- != 0) {
        strcpy(destination, argv[argumentIndex++]);
        g_pInitialArguments_005c57f0[argumentCount++] = destination;
        destination = strchr(destination, 0);
        destination++;
    }
    return (short)(argumentCount - 1);
}
