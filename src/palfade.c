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
    short slot;
    short used;
    unsigned short targetColour[3];
    short *goalPalette;
    short *workPalette;

    if (g_nSpacePaletteFadeMode == 0x13) {
        indices = AllocateTaggedMemory(256, 0);
        if (indices == 0)
            return;
        memset(indices, 0, 256);
        used = CollectActivePaletteIndices(viewport, indices, 256);
        workPalette = AllocateTaggedMemory(
            (unsigned int)(used * 6), 0);
        if (workPalette == 0) {
            ReleasePacketHandle(indices);
            return;
        }
        memset(workPalette, 0,
               (unsigned int)(used * 6));
        goalPalette = AllocateTaggedMemory(
            (unsigned int)(used * 6), 0);
        if (goalPalette == 0) {
            ReleasePacketHandle(indices);
            ReleasePacketHandle(workPalette);
            return;
        }
        memset(goalPalette, 0,
               (unsigned int)(used * 6));
        GetPaletteEntry((short)colour, targetColour);
        for (slot = 0; slot < used; slot++) {
            GetPaletteEntry(
                (short)indices[slot],
                (unsigned short *)&workPalette[slot * 3]);
            memcpy(&goalPalette[slot * 3], targetColour, 6);
        }
        while (StepPaletteTransition(
                   workPalette, goalPalette,
                   (short)(used * 3)) != 0) {
            if (enabled != 0) {
                slot = enabled;
                while (slot-- != 0)
                    WaitForVerticalBlankThunk();
            }
            for (slot = 0; slot < used; slot++) {
                SetPaletteEntry((short)indices[slot],
                                &workPalette[slot * 3]);
            }
        }
        ReleasePacketHandle(goalPalette);
        ReleasePacketHandle(workPalette);
        ReleasePacketHandle(indices);
    }
}
