/*
 *  Queries the graphics mode.
 *
 *  Retail object 0x45B7E0-0x45B80F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x45B7E0 */
short QueryCurrentGraphicsMode(void)
{
    if ((short)g_nSpacePaletteFadeMode != -1)
        return (short)g_nSpacePaletteFadeMode;
    return 0x13;
}
