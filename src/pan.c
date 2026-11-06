/*
 *  Pans between screens.
 *
 *  Retail object 0x433410-0x43368F (link order; see docs/ORDER.md).
 */
#include "game.h"

#pragma function(memset)
/* Function start: 0x433410 */
void PanToScreen(Viewport *source, Viewport *destination)
{
    unsigned char *indices;
    short count;
    short i;
    unsigned short destRgb[3];
    short *basePalette;
    short *fadePalette;

    if (g_nSpacePaletteFadeMode == 0x13) {
        indices = AllocateTaggedMemory(256, 0);
        if (indices == 0)
            return;
        memset(indices, 0, 256);
        count = CollectActivePaletteIndices(source, indices, 256);
        basePalette = AllocateTaggedMemory(
            (unsigned int)(count * 6), 0);
        fadePalette = AllocateTaggedMemory(
            (unsigned int)(count * 6), 0);
        if (basePalette == 0 || fadePalette == 0) {
            ReleasePacketHandle(indices);
            if (basePalette != 0)
                ReleasePacketHandle(basePalette);
            if (fadePalette != 0)
                ReleasePacketHandle(fadePalette);
            return;
        }
        memset(basePalette, 0,
               (unsigned int)(count * 6));
        memset(fadePalette, 0,
               (unsigned int)(count * 6));
        GetPaletteEntry(
            (short)GetViewportPixel(destination,
                                    destination->left,
                                    destination->top),
            destRgb);
        WaitForVerticalBlankThunk();
        for (i = 0; i < count; i++) {
            GetPaletteEntry(
                (short)indices[i],
                (unsigned short *)&basePalette[i * 3]);
            SetPaletteEntry((short)indices[i], (short *)destRgb);
            *(PaletteRgb *)&fadePalette[i * 3] =
                *(PaletteRgb *)destRgb;
        }
        CopyViewportContents(source, destination);
        while (StepPaletteTransition(
                   fadePalette, basePalette,
                   (short)(count * 3)) != 0) {
            WaitForVerticalBlankThunk();
            for (i = 0; i < count; i++) {
                SetPaletteEntry(
                    (short)indices[i],
                    &fadePalette[i * 3]);
            }
        }
        ReleasePacketHandle(fadePalette);
        ReleasePacketHandle(basePalette);
        ReleasePacketHandle(indices);
    } else {
        CopyViewportContents(source, destination);
    }
}
