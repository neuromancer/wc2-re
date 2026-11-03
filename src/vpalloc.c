/*
 *  Viewport buffer allocation.
 *
 *  Retail object 0x45CA50-0x45CCAF (link order; see docs/ORDER.md).  The
 *  DOS release's vpalloc module holds the size and allocation routines;
 *  the Win32 port puts its display-mode diagnostic in front of them.
 */
#include "game.h"

/* Function start: 0x45CA50 */
void LogDisplayMode(const char *mode)
{
    SystemDebugPrintf("display_mode == '%s'\n", mode);
    ClearDebugPauseFlags();
    PumpMessagesDuringWait();
    exit(1);
}

short AllocateViewport(Viewport *viewport, short clearColour, short flags);
void ClearViewport(Viewport *viewport, short colour);

/* Function start: 0x45CA80 */
short CalcRectangleArea(const Viewport *viewport)
{
    if (g_nSpacePaletteFadeMode_004901e8 != 0x13)
        LogDisplayMode("not MCGA");
    return (short)((viewport->bottom - viewport->top + 1) *
                   (viewport->right - viewport->left + 1));
}

/* Function start: 0x45CAD2 */
short AllocateViewport(Viewport *viewport,
                       short clearColour, short flags)
{
    unsigned short y;
    unsigned short height;
    unsigned short pixelOffset;
    unsigned char *buffer;
    unsigned short firstRow;
    unsigned short width;
    unsigned short left;

    height = viewport->bottom - viewport->top + 1;
    width = viewport->right - viewport->left + 1;
    left = viewport->left;
    firstRow = viewport->top;
    g_nAllocateViewportCalls_005c8dc4++;
    if (g_nSpacePaletteFadeMode_004901e8 != 0x13)
        LogDisplayMode("not MCGA");
    viewport->allocation = AllocateTaggedMemory(
        width * height, (unsigned short)(flags + 2));
    g_apViewportAllocations_005d19c0[
        g_nViewportAllocationCount_005d19bc++] = viewport->allocation;
    buffer = viewport->allocation;
    if (buffer == 0)
        return 0;
    viewport->pixels = buffer;
    pixelOffset = 0;

    viewport->rowOffsets = AllocateTaggedMemory(
        (firstRow + height + 2) * 2, 0);
    if (viewport->rowOffsets == 0) {
        if (g_nSpacePaletteFadeMode_004901e8 != 0x13)
            ReleasePacketHandle(viewport->allocation);
        return 0;
    }

    for (y = 0; y < height + 2; y++) {
        (&viewport->rowOffsets[firstRow])[y] = pixelOffset - left;
        pixelOffset += width;
    }
    if (clearColour != -1)
        ClearViewport(viewport, clearColour);
    return 1;
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
int g_nAllocateViewportCalls_005c8dc4;
