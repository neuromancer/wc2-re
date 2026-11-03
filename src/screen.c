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

    g_pDibScreenViewport_005d398c = viewport;
    g_pDibScreenAllocationStatePointer_005d3b4c =
        &g_nDibScreenChunkState_005d3b40;
    g_nDibScreenChunkState_005d3b40 = 0;
    g_pDibScreenViewportMirrorRef_005d3b44 = viewport;
    viewport->left = 0;
    viewport->top = 0;
    viewport->right = 319;
    viewport->bottom = 199;
    if (g_nSpacePaletteFadeMode_004901e8 != 0x13) {
        SystemDebugPrintf("== BAD alloc_screen == : type: '%d'\n",
                          (int)(short)g_nSpacePaletteFadeMode_004901e8);
        exit(1);
    }
    colour = (unsigned short)-1;
    viewport->pixels = GetDIBPixelBuffer();
    viewport->rowOffsets = g_awDibScreenRowOffsetsData_005d39a0;
    for (row = 0; row < 202; row++)
        g_awDibScreenRowOffsetsData_005d39a0[row] =
            (unsigned short)(row * 320);
    if ((short)colour != -1)
        ClearViewport(viewport, colour);
    return 1;
}

/* Function start: 0x40FC7E */
/* Initialises a 320x200 viewport record (0x13F == 319, 199) then validates it. */
void InitFullScreenViewport(Viewport *viewport, short arg)
{
    g_pFullScreenViewport_005d3990 = viewport;
    viewport->left = 0;
    viewport->right = 319;
    viewport->top = 0;
    viewport->bottom = 199;
    AllocateViewport(viewport, arg, 0);
    return;
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
Viewport *g_pDibScreenViewport_005d398c;
Viewport *g_pFullScreenViewport_005d3990;
unsigned short g_awDibScreenRowOffsetsData_005d39a0[202];
int g_nUnusedScreenSlot_005d3b38[2]; /* never referenced */
int g_nDibScreenChunkState_005d3b40;
Viewport *g_pDibScreenViewportMirrorRef_005d3b44;
char g_nUnusedScreenWord_005d3b48; /* never referenced */
int *g_pDibScreenAllocationStatePointer_005d3b4c;
int g_nUnusedScreenWordValue_005d3b50[29]; /* never referenced */
