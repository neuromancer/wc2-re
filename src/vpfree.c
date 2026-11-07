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

    g_2_nFreeViewportCalls++;
    allocation = viewport->allocation;
    if (allocation == 0)
        return;

    for (i = 0; i < g_nViewportAllocationCount; i++) {
        if (g_apViewportAllocations[i] == viewport->allocation) {
            g_nViewportAllocationCount--;
            g_apViewportAllocations[i] =
                g_apViewportAllocations[
                    g_nViewportAllocationCount];
            break;
        }
    }

    if (viewport->rowOffsets != 0) {
        ReleasePacketHandle(viewport->rowOffsets);
        viewport->rowOffsets = 0;
    }
    if (g_nSpacePaletteFadeMode != 0x13)
        printf("free_viewport not mcga\n");
    ReleasePacketHandle(allocation);
    viewport->pixels = 0;
    viewport->allocation = 0;
    if (g_101_stScrViewInfo.pixels == allocation)
        g_101_stScrViewInfo.pixels = 0;
    if (g_101_stScrViewInfo.allocation == allocation)
        g_101_stScrViewInfo.allocation = 0;
    if (g_stSecondaryViewBufferBlock.pixels == allocation)
        g_stSecondaryViewBufferBlock.pixels = 0;
    if (g_stSecondaryViewBufferBlock.allocation == allocation)
        g_stSecondaryViewBufferBlock.allocation = 0;
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
int g_nViewportAllocationCount; /* 0x005d19bc */
unsigned char *g_apViewportAllocations[128]; /* 0x005d19c0 */
int g_2_nFreeViewportCalls; /* 0x005d1bc0 */
