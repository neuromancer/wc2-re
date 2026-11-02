/*
 *  Space palette layout and contiguous palette-entry reservation.
 *
 *  Retail object 0x401840-0x401A0F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x401840 */
void ConfigureDefaultSpacePalette(short mode)
{
    (void)mode;
    g_nSpacePaletteFadeMode_004901e8 = 0x13;
    ApplySpacePaletteModeHook();
}

/* Function start: 0x401859 */
short ReserveContiguousPaletteEntries(short entryCount)
{
    short entry;
    short firstFree;
    short freeEntries;

    firstFree = 0;
    freeEntries = firstFree;
    for (entry = freeEntries; entry < 256; entry++) {
        if (g_awPaletteEntryAllocation_005d4050[entry] != 0) {
            freeEntries = 0;
            firstFree = (short)(entry + 1);
        } else {
            freeEntries++;
        }
        if (entryCount == freeEntries) {
            for (entry = 0; entry < entryCount; entry++) {
                g_awPaletteEntryAllocation_005d4050[entry + firstFree] =
                    entryCount;
            }
            return firstFree;
        }
    }
    return -1;
}

/* Function start: 0x401922 */
void ReleaseContiguousPaletteEntries(short firstEntry)
{
    short entry;
    short count;

    count = g_awPaletteEntryAllocation_005d4050[firstEntry];
    for (entry = 0; count > entry; entry++)
        g_awPaletteEntryAllocation_005d4050[entry + firstEntry] = 0;
}

/* Function start: 0x401978 */
void PrintPaletteAllocationMap(void)
{
    short x;
    short y;
    short entry;

    entry = 0;
    for (y = entry; y < 4; y++) {
        for (x = 0; x < 0x40; x++) {
            printf("%c",
                   g_awPaletteEntryAllocation_005d4050[entry++] == 0 ?
                       '_' : '.');
        }
        printf("\n");
    }
}
