/*
 *  Fades a viewport palette towards a colour.
 *
 *  Retail object 0x453820-0x453A6F (link order; see docs/ORDER.md).
 */
#include "game.h"

#pragma function(memset)
/* Function start: 0x453820 */
void FadeViewportPaletteToColour(Viewport *viewport,
                                 unsigned short colour,
                                 short enabled)
{
    unsigned char *indices;
    short activeCount;
    short index;
    unsigned short target[3];
    short *targetPalette;
    short *currentPalette;

    if (g_nSpacePaletteFadeMode_004901e8 == 0x13) {
        indices = AllocateTaggedMemory(256, 0);
        if (indices == 0)
            return;
        memset(indices, 0, 256);
        activeCount = CollectActivePaletteIndices(viewport, indices, 256);
        currentPalette = AllocateTaggedMemory(
            (unsigned int)(activeCount * 6), 0);
        if (currentPalette == 0) {
            ReleasePacketHandle(indices);
            return;
        }
        memset(currentPalette, 0,
               (unsigned int)(activeCount * 6));
        targetPalette = AllocateTaggedMemory(
            (unsigned int)(activeCount * 6), 0);
        if (targetPalette == 0) {
            ReleasePacketHandle(indices);
            ReleasePacketHandle(currentPalette);
            return;
        }
        memset(targetPalette, 0,
               (unsigned int)(activeCount * 6));
        GetPaletteEntry((short)colour, target);
        for (index = 0; index < activeCount; index++) {
            GetPaletteEntry(
                (short)indices[index],
                (unsigned short *)&currentPalette[index * 3]);
            memcpy(&targetPalette[index * 3], target, 6);
        }
        while (StepPaletteTransition(
                   currentPalette, targetPalette,
                   (short)(activeCount * 3)) != 0) {
            if (enabled != 0) {
                index = enabled;
                while (index-- != 0)
                    WaitForVerticalBlankThunk();
            }
            for (index = 0; index < activeCount; index++) {
                SetPaletteEntry((short)indices[index],
                                &currentPalette[index * 3]);
            }
        }
        ReleasePacketHandle(targetPalette);
        ReleasePacketHandle(currentPalette);
        ReleasePacketHandle(indices);
    }
}
