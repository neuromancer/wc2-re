/*
 *  Saves and restores the game palette.
 *
 *  Retail object 0x417550-0x41760F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x417550 */
void SaveGamePalette(void)
{
    short index;

    for (index = 0; index < 256; index++) {
        if ((index & 15) == 0)
            WaitForVerticalBlankThunk();
        GetPaletteEntry(index, g_ausPaletteWords[index]);
    }
}

/* Function start: 0x4175AD */
void RestoreGamePalette(void)
{
    short index;

    for (index = 0; (short)index < 256; index++) {
        if ((index & 15) == 0)
            WaitForVerticalBlankThunk();
        SetPaletteEntry(index, (short *)g_ausPaletteWords[index]);
    }
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
unsigned short g_ausPaletteWords[256][3]; /* 0x005d3220 */
