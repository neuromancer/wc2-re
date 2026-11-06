/*
 *  Full-screen DIB viewport setup.
 *
 *  Retail object 0x40FB80-0x40FCCF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x40FB80 */
unsigned short InitializeDIBScreenViewport(
    Viewport *viewport, unsigned short colour)
{
    short row;

    g_pDibScreenViewport = viewport;
    g_pDibScreenAllocationStatePointer =
        &g_nDibScreenChunkState;
    g_nDibScreenChunkState = 0;
    g_pDibScreenViewportMirrorRef = viewport;
    viewport->left = 0;
    viewport->top = 0;
    viewport->right = 319;
    viewport->bottom = 199;
    if (g_nSpacePaletteFadeMode != 0x13) {
        SystemDebugPrintf("== BAD alloc_screen == : type: '%d'\n",
                          (int)(short)g_nSpacePaletteFadeMode);
        exit(1);
    }
    colour = (unsigned short)-1;
    viewport->pixels = GetDIBPixelBuffer();
    viewport->rowOffsets = g_2_awDibScreenRowOffsetsData;
    for (row = 0; row < 202; row++)
        g_2_awDibScreenRowOffsetsData[row] =
            (unsigned short)(row * 320);
    if ((short)colour != -1)
        ClearViewport(viewport, colour);
    return 1;
}

/* Function start: 0x40FC7E */
/* Initialises a 320x200 viewport record (0x13F == 319, 199) then validates it. */
void InitFullScreenViewport(Viewport *viewport, short arg)
{
    g_19_pFullScreenViewport = viewport;
    viewport->left = 0;
    viewport->right = 319;
    viewport->top = 0;
    viewport->bottom = 199;
    AllocateViewport(viewport, arg, 0);
    return;
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
Viewport *g_pDibScreenViewport; /* 0x005d398c */
Viewport *g_19_pFullScreenViewport; /* 0x005d3990 */
unsigned short g_2_awDibScreenRowOffsetsData[202]; /* 0x005d39a0 */
int g_nUnusedScreenSlot[2]; /* 0x005d3b38 */ /* never referenced */
int g_nDibScreenChunkState; /* 0x005d3b40 */
Viewport *g_pDibScreenViewportMirrorRef; /* 0x005d3b44 */
char g_1_nUnusedScreenWord3; /* 0x005d3b48 */ /* never referenced */
int *g_pDibScreenAllocationStatePointer; /* 0x005d3b4c */
int g_nUnusedScreenWordValue[29]; /* 0x005d3b50 */ /* never referenced */
