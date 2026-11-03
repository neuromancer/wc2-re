/*
 *  Frees a viewport.
 *
 *  Retail object 0x4471B0-0x4472FF (link order; see docs/ORDER.md).
 */
#include "game.h"
#ifndef SDL_PORT
#include <malloc.h>
#endif

/* Function start: 0x4471B0 */
void free_viewport(Viewport *viewport)
{
    int i;
    unsigned char *allocation;

    g_nFreeViewportCalls_005d1bc0++;
    allocation = viewport->allocation;
    if (allocation == 0)
        return;

    for (i = 0; i < g_nViewportAllocationCount_005d19bc; i++) {
        if (g_apViewportAllocations_005d19c0[i] == viewport->allocation) {
            g_nViewportAllocationCount_005d19bc--;
            g_apViewportAllocations_005d19c0[i] =
                g_apViewportAllocations_005d19c0[
                    g_nViewportAllocationCount_005d19bc];
            break;
        }
    }

    if (viewport->rowOffsets != 0) {
        ReleasePacketHandle(viewport->rowOffsets);
        viewport->rowOffsets = 0;
    }
    if (g_nSpacePaletteFadeMode_004901e8 != 0x13)
        printf("free_viewport not mcga\n");
    ReleasePacketHandle(allocation);
    viewport->pixels = 0;
    viewport->allocation = 0;
    if (g_stScrViewInfo_005d21a0.pixels == allocation)
        g_stScrViewInfo_005d21a0.pixels = 0;
    if (g_stScrViewInfo_005d21a0.allocation == allocation)
        g_stScrViewInfo_005d21a0.allocation = 0;
    if (g_stSecondaryViewBufferBlock_005d2c90.pixels == allocation)
        g_stSecondaryViewBufferBlock_005d2c90.pixels = 0;
    if (g_stSecondaryViewBufferBlock_005d2c90.allocation == allocation)
        g_stSecondaryViewBufferBlock_005d2c90.allocation = 0;
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
int g_nViewportAllocationCount_005d19bc;
unsigned char *g_apViewportAllocations_005d19c0[128];
int g_nFreeViewportCalls_005d1bc0;
