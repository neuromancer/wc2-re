/*
 *  Shape decoding, viewport drawing and raster effects.
 *
 *  Retail object 0x4254C0-0x4288FF (link order; see docs/ORDER.md).
 */
#include "game.h"

unsigned char g_abPaletteTranslation_00496338[256] = {
      0,   1,   2,   3,   4,   5,   6,   7,
      8,   9,  10,  11,  12,  13,  14,  15,
     16,  17,  18,  19,  20,  21,  22,  23,
     24,  25,  26,  27,  28,  29,  30,  31,
     32,  33,  34,  35,  36,  37,  38,  39,
     40,  41,  42,  43,  44,  45,  46,  47,
     48,  49,  50,  51,  52,  53,  54,  55,
     56,  57,  58,  59,  60,  61,  62,  63,
     64,  65,  66,  67,  68,  69,  70,  71,
     72,  73,  74,  75,  76,  77,  78,  79,
     80,  81,  82,  83,  84,  85,  86,  87,
     88,  89,  90,  91,  92,  93,  94,  95,
     96,  97,  98,  99, 100, 101, 102, 103,
    104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 126, 127,
    128, 129, 130, 131, 132, 133, 134, 135,
    136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151,
    152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167,
    168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 181, 182, 183,
    184, 185, 186, 187, 188, 189, 190, 191,
    192, 193, 194, 195, 196, 197, 198, 199,
    200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215,
    216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231,
    232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247,
    248, 249, 250, 251, 252, 253, 254, 255
};
unsigned short g_awAbsoluteCosine_00496438[360] = {
    255, 255, 255, 255, 255, 255, 255, 254, 254, 253, 252, 251,
    250, 250, 248, 247, 246, 245, 243, 242, 241, 239, 237, 236,
    234, 232, 230, 228, 226, 224, 221, 219, 217, 215, 212, 210,
    207, 204, 202, 198, 196, 193, 190, 187, 184, 181, 178, 175,
    171, 168, 165, 161, 158, 154, 150, 147, 143, 139, 136, 131,
    128, 124, 120, 116, 112, 108, 104, 100,  96,  92,  88,  83,
     79,  74,  71,  66,  62,  58,  53,  49,  45,  40,  36,  31,
     27,  22,  18,  13,   9,   4,   0,   4,   9,  13,  18,  22,
     27,  31,  36,  40,  45,  49,  53,  58,  62,  66,  71,  75,
     79,  83,  88,  92,  96, 100, 104, 108, 112, 116, 120, 124,
    128, 131, 136, 139, 143, 147, 150, 154, 158, 161, 165, 168,
    171, 175, 178, 181, 184, 187, 190, 193, 196, 198, 202, 204,
    207, 210, 212, 215, 217, 219, 221, 224, 226, 228, 230, 232,
    234, 236, 237, 239, 241, 242, 243, 245, 246, 247, 248, 250,
    250, 251, 252, 253, 254, 254, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 254, 254, 253, 252, 251,
    250, 250, 248, 247, 246, 245, 243, 242, 241, 239, 237, 236,
    234, 232, 230, 228, 226, 224, 221, 219, 217, 215, 212, 210,
    207, 204, 202, 198, 196, 193, 190, 187, 184, 181, 178, 175,
    171, 168, 165, 161, 158, 154, 150, 147, 143, 139, 136, 131,
    128, 124, 120, 116, 112, 108, 104, 100,  96,  92,  88,  83,
     79,  74,  71,  66,  62,  58,  53,  49,  45,  40,  36,  31,
     27,  22,  18,  13,   9,   4,   0,   4,   9,  13,  18,  22,
     27,  31,  36,  40,  45,  49,  53,  58,  62,  66,  71,  75,
     79,  83,  88,  92,  96, 100, 104, 108, 112, 116, 120, 124,
    128, 131, 136, 139, 143, 147, 150, 154, 158, 161, 165, 168,
    171, 175, 178, 181, 184, 187, 190, 193, 196, 198, 202, 204,
    207, 210, 212, 215, 217, 219, 221, 224, 226, 228, 230, 232,
    234, 236, 237, 239, 241, 242, 243, 245, 246, 247, 248, 250,
    250, 251, 252, 253, 254, 254, 255, 255, 255, 255, 255, 255
};
unsigned short g_awAbsoluteSine_00496708[360] = {
      0,   4,   9,  13,  18,  22,  27,  31,  36,  40,  45,  49,
     53,  58,  62,  66,  71,  75,  79,  83,  88,  92,  96, 100,
    104, 108, 112, 116, 120, 124, 128, 131, 136, 139, 143, 147,
    150, 154, 158, 161, 165, 168, 171, 175, 178, 181, 184, 187,
    190, 193, 196, 198, 202, 204, 207, 210, 212, 215, 217, 219,
    221, 224, 226, 228, 230, 232, 234, 236, 237, 239, 241, 242,
    243, 245, 246, 247, 248, 250, 250, 251, 252, 253, 254, 254,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 254, 254, 253, 252, 251, 250, 250, 248, 247, 246, 245,
    243, 242, 241, 239, 237, 236, 234, 232, 230, 228, 226, 224,
    221, 219, 217, 215, 212, 210, 207, 204, 202, 198, 196, 193,
    190, 187, 184, 181, 178, 175, 171, 168, 165, 161, 158, 154,
    150, 147, 143, 139, 136, 131, 128, 124, 120, 116, 112, 108,
    104, 100,  96,  92,  88,  83,  79,  74,  71,  66,  62,  58,
     53,  49,  45,  40,  36,  31,  27,  22,  18,  13,   9,   4,
      0,   4,   9,  13,  18,  22,  27,  31,  36,  40,  45,  49,
     53,  58,  62,  66,  71,  75,  79,  83,  88,  92,  96, 100,
    104, 108, 112, 116, 120, 124, 128, 131, 136, 139, 143, 147,
    150, 154, 158, 161, 165, 168, 171, 175, 178, 181, 184, 187,
    190, 193, 196, 198, 202, 204, 207, 210, 212, 215, 217, 219,
    221, 224, 226, 228, 230, 232, 234, 236, 237, 239, 241, 242,
    243, 245, 246, 247, 248, 250, 250, 251, 252, 253, 254, 254,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 254, 254, 253, 252, 251, 250, 250, 248, 247, 246, 245,
    243, 242, 241, 239, 237, 236, 234, 232, 230, 228, 226, 224,
    221, 219, 217, 215, 212, 210, 207, 204, 202, 198, 196, 193,
    190, 187, 184, 181, 178, 175, 171, 168, 165, 161, 158, 154,
    150, 147, 143, 139, 136, 131, 128, 124, 120, 116, 112, 108,
    104, 100,  96,  92,  88,  83,  79,  74,  71,  66,  62,  58,
     53,  49,  45,  40,  36,  31,  27,  22,  18,  13,   9,   4
};

#ifdef VPORT_DEBUG
/* Diagnostic helper for `make vport-debug`; not part of the original, so it
 * carries no Function start marker -- there is no WC2 address to hold it to. */
static const char *DescribeUnregisteredViewport(const Viewport *viewport)
{
    static char text[256];
    int entry;
    int shown;
    char *cursor;

    sprintf(text,
            "bad vport vp=%p px=%p rows=%p l=%d t=%d r=%d b=%d alloc=%p"
            " screen=%p dib=%p n=%d:",
            (const void *)viewport, (void *)viewport->pixels,
            (void *)viewport->rowOffsets,
            (int)viewport->left, (int)viewport->top,
            (int)viewport->right, (int)viewport->bottom,
            (void *)viewport->allocation,
            (void *)g_stScreenViewport_005d21a0.pixels,
            (void *)GetDIBPixelBuffer(),
            g_nViewportAllocationCount_005d19bc);
    cursor = text + strlen(text);
    shown = g_nViewportAllocationCount_005d19bc;
    if (shown > 6)
        shown = 6;
    for (entry = 0; entry < shown; entry++) {
        sprintf(cursor, " %p",
                (void *)g_apViewportAllocations_005d19c0[entry]);
        cursor += strlen(cursor);
    }
    return text;
}
#endif

/* Function start: 0x4254C0 */
void CheckHeapBlockSignature(unsigned char *shape)
{
    if (*(int *)(shape - 8) != 0x6666656a) {
        if (IsFreedHeapBlockTracked(shape) != 0)
            exit_squadron("not jefftep (freed)");
        else
            exit_squadron("not jefftep");
    }
}

/* Function start: 0x42550E */
int HasValidShapeAllocationSignature(unsigned char *shape)
{
    if (*(int *)(shape - 8) != 0x6666656a)
        return 0;
    return 1;
}

/* Function start: 0x42553A */
unsigned char *GetPreparedShapeData(unsigned char *shape)
{
#ifdef SDL_PORT
    return *(unsigned char **)(shape - 8 - sizeof(unsigned char *));
#else
    return *(unsigned char **)(shape - 4);
#endif
}

/* Function start: 0x425550 */
short GetShapeFrameCount(unsigned char *shape)
{
    CheckHeapBlockSignature(shape);
    return (short)((*(unsigned short *)(shape + 4) >> 2) - 1);
}

/* Function start: 0x42557C */
void GetShapeFrameExtents(unsigned char *shape, short frame,
                          short *width, short *height,
                          short *leftExtent, short *topExtent)
{
    int right;
    int top;
    int left;
    int bottom;
#ifndef SDL_PORT
    short *frameHeader;
#else
    short frameExtents[4];
#endif

    frame++;
    frame <<= 2;
    if (*(unsigned short *)(shape + 4) > frame) {
#ifdef SDL_PORT
        memcpy(frameExtents, shape + *(int *)(shape + frame),
               sizeof(frameExtents));
        right = frameExtents[0];
        left = frameExtents[1];
        top = frameExtents[2];
        bottom = frameExtents[3];
#else
        frameHeader = (short *)(*(int *)(shape + frame) + shape);
        right = *frameHeader++;
        left = *frameHeader++;
        top = *frameHeader++;
        bottom = *frameHeader++;
#endif
        *width = (short)(left + right + 1);
        *height = (short)(bottom + top + 1);
        *leftExtent = (short)left;
        *topExtent = (short)top;
    }
}

#pragma function(memcpy, memset)
/* Function start: 0x425618 */
void DecodeShapeFrame(unsigned char *shape, short frame,
                      unsigned char *bitmap, int width, int height,
                      int leftExtent, int topExtent)
{
    unsigned char *commands;
    unsigned short rowOp;
    unsigned char *dest;
    unsigned char action;
    unsigned char color;
    unsigned char *pixels;
    short span;
    int endX;
    short yDelta;
    short leftmost;
    short rightmost;
    short minY;
    short maxY;
    int skip;
    int shapeY;
    int screenX;
    short xDelta;
    int row;
    int shapeX;

    shapeX = leftExtent;
    shapeY = topExtent;
    if (shape == 0)
        return;
    if (frame < 0)
        return;
    frame++;
    frame <<= 2;
    if (*(unsigned short *)(shape + 4) > frame) {

        leftmost = 0;
        rightmost = (short)(width - 1);
        minY = 0;
        maxY = (short)(height - 1);
        commands = shape + *(int *)(frame + shape);
        commands += 8;
        pixels = bitmap;
    #ifdef SDL_PORT
        memcpy(&rowOp, commands, sizeof(rowOp));
    #else
        rowOp = *(unsigned short *)commands;
    #endif
        commands += 2;
        while (rowOp != 0) {
    #ifdef SDL_PORT
            memcpy(&xDelta, commands, sizeof(xDelta));
    #else
            xDelta = *(short *)commands;
    #endif
            commands += 2;
    #ifdef SDL_PORT
            memcpy(&yDelta, commands, sizeof(yDelta));
    #else
            yDelta = *(short *)commands;
    #endif
            commands += 2;
            screenX = xDelta + shapeX;
            row = yDelta + shapeY;
            dest = row * width + screenX + pixels;
            if ((rowOp & 1) != 0) {
                rowOp >>= 1;
                while (rowOp > 0) {
                    action = *commands;
                    commands++;
                    if ((action & 1) != 0) {
                        action >>= 1;
                        rowOp = (unsigned short)(rowOp - action);
                        color = *commands;
                        commands++;
                        if (minY <= row && maxY >= row) {
                            endX = action + screenX - 1;
                            if (rightmost >= screenX && leftmost <= endX) {
                                span = action;
                                skip = 0;
                                if (leftmost > screenX) {
                                    span -= leftmost - screenX;
                                    skip += leftmost - screenX;
                                }
                                if (rightmost < endX)
                                    span -= endX - rightmost;
                                memset(skip + dest, color, span);
                            }
                        }
                        dest += action;
                        screenX += action;
                    } else {
                        action >>= 1;
                        rowOp = (unsigned short)(rowOp - action);
                        if (minY <= row && maxY >= row) {
                            endX = action + screenX - 1;
                            if (rightmost >= screenX && leftmost <= endX) {
                                span = action;
                                skip = 0;
                                if (leftmost > screenX) {
                                    span -= leftmost - screenX;
                                    skip += leftmost - screenX;
                                }
                                if (rightmost < endX)
                                    span -= endX - rightmost;
                                memcpy(skip + dest, skip + commands,
                                       span);
                            }
                        }
                        commands += action;
                        dest += action;
                        screenX += action;
                    }
                }
            } else {
                rowOp >>= 1;
                if (minY <= row && maxY >= row) {
                    endX = rowOp + screenX - 1;
                    if (rightmost >= screenX && leftmost <= endX) {
                        span = rowOp;
                        skip = 0;
                        if (leftmost > screenX) {
                            span -= leftmost - screenX;
                            skip += leftmost - screenX;
                        }
                        if (rightmost < endX)
                            span -= endX - rightmost;
                        memcpy(skip + dest, skip + commands, span);
                    }
                }
                commands += rowOp;
            }
    #ifdef SDL_PORT
            memcpy(&rowOp, commands, sizeof(rowOp));
    #else
            rowOp = *(unsigned short *)commands;
    #endif
            commands += 2;
        }
    }
}

#pragma intrinsic(memcpy, memset)
/* Function start: 0x4259E2 */
int SignExtendClipCoord(volatile short v)
{
    if ((unsigned short)v < 0xfdc0)
        return (unsigned short)v;
    return (int)(short)v;
}

void exit_squadron(const char *msg, ...);

/* Function start: 0x425A16 */
void ValidateViewportBounds(Viewport *viewport, RasterSurface *surface,
                            RasterClip *clip)
{
    int nextOffset;
    int rowStrideOffset;
    int offset;

    if (viewport->left < 0)
        return;
    if (viewport->pixels != g_stScreenViewport_005d21a0.pixels) {
        for (offset = 0;
             offset < g_nViewportAllocationCount_005d19bc &&
                 g_apViewportAllocations_005d19c0[offset] !=
                     viewport->pixels;
             offset++) {
        }
        if (offset >= g_nViewportAllocationCount_005d19bc)
#ifdef VPORT_DEBUG
            /* Diagnostic build (make vport-debug): name the viewport that is
             * not in the allocation registry so the caller can be identified
             * from WC2.map.  The reference build keeps the original text. */
            exit_squadron(DescribeUnregisteredViewport(viewport));
#else
            exit_squadron("bad vport");
#endif
    }
    if (viewport->pixels == g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
    offset = SignExtendClipCoord(viewport->rowOffsets[viewport->top]);
    nextOffset = SignExtendClipCoord(
        viewport->rowOffsets[viewport->top + 1]);
    rowStrideOffset = SignExtendClipCoord(
        viewport->rowOffsets[viewport->top]);
    surface->pixels = viewport->pixels + viewport->left + offset;
    surface->maximumX = nextOffset - rowStrideOffset - 1;
    surface->maximumY = viewport->bottom - viewport->top;
    surface->field_C = 0;
    surface->field_10 = 0;
    clip->surface = surface;
    clip->left = 0;
    clip->top = 0;
    clip->right = viewport->right - viewport->left;
    clip->bottom = viewport->bottom - viewport->top;
}

/* Function start: 0x425B9E */
void ClipViewportToScreen(Viewport *viewport)
{
    ValidateViewportBounds(viewport, &g_stRasterSurface_004a2670,
                           &g_stRasterClip_004b2088);
}

#pragma function(memset)
/* Function start: 0x425BBF */
void SetSolidColourTranslation(unsigned char colour)
{
    memset(g_abSolidColourTranslation_004b2710, colour, 255);
    g_abSolidColourTranslation_004b2710[255] = 0xff;
    SetPaletteTranslationTable(g_abSolidColourTranslation_004b2710);
}

#pragma function(memcpy)
/* Function start: 0x425BF6 */
void PrepareShapeRLEData(unsigned char *shape)
{
    int gapRun;
    int runLength;
    unsigned char *code;
    short height;
    unsigned char *bitmap;
    unsigned char *pixel;
    short width;
    unsigned char *offsetTable;
    unsigned char *packed;
    int bytes;
    int frameOffset;
    int endX;
    int row;
    int startX;
    int frameNum;
    unsigned char *emit;
    int todo;
    int startY;
    short leftExtent;
    short topOffset;
    int frameLimit;
    int bottomEdge;

    CheckHeapBlockSignature(shape);
    if (GetPreparedShapeData(shape) != 0)
        return;

    emit = g_abShapeRLEScratch_004b2810;
    memcpy(emit, "1.00", 4);
    emit += 4;
    frameLimit = GetShapeFrameCount(shape);
    memcpy(emit, &frameLimit, 4);
    emit += 4;
    offsetTable = emit;
    memset(emit, 0, (unsigned int)(frameLimit << 3));
    emit += frameLimit << 3;

    for (frameNum = 0; frameNum < frameLimit; frameNum++) {
        frameOffset = (int)(emit - g_abShapeRLEScratch_004b2810);
        memcpy(offsetTable, &frameOffset, 4);
        offsetTable += 8;
        GetShapeFrameExtents(shape, (short)frameNum, &width, &height,
                             &leftExtent, &topOffset);
        memcpy(emit, &height, 2);
        emit += 2;
        memcpy(emit, &width, 2);
        emit += 2;
        memcpy(emit, &topOffset, 2);
        emit += 2;
        memcpy(emit, &leftExtent, 2);
        emit += 2;
        startX = -leftExtent;
        startY = -topOffset;
        endX = width - leftExtent - 1;
        bottomEdge = height - topOffset - 1;
        memcpy(emit, &startX, 4);
        emit += 4;
        memcpy(emit, &startY, 4);
        emit += 4;
        memcpy(emit, &endX, 4);
        emit += 4;
        memcpy(emit, &bottomEdge, 4);
        emit += 4;

        bitmap = AllocateTaggedMemory(
            (unsigned int)((int)height * width), 0);
        pixel = bitmap;
        memset(bitmap, 0xff, (unsigned int)((int)height * width));
        DecodeShapeFrame(shape, (short)frameNum, bitmap, width, height,
                         leftExtent, topOffset);
        for (row = 0; height > row; row++) {
            todo = width;
            while (todo > 0) {
                if (*pixel != 0xff) {
                    code = emit;
                    emit++;
                    for (runLength = 0;
                         todo != 0 && runLength < 0x7f &&
                             *pixel != 0xff;
                         runLength++, todo--, emit++, pixel++) {
                        *emit = *pixel;
                    }
                    *code = (unsigned char)(runLength * 2 + 1);
                } else {
                    for (gapRun = 0;
                         todo != 0 && gapRun < 0xff &&
                             *pixel == 0xff;
                         gapRun++, todo--, pixel++) {
                    }
                    *emit = 1;
                    emit++;
                    *emit = (unsigned char)gapRun;
                    emit++;
                }
            }
            *emit = 0;
            emit++;
        }
        ReleasePacketHandle(bitmap);
    }

    bytes = (int)(emit - g_abShapeRLEScratch_004b2810);
    if (bytes > (int)sizeof(g_abShapeRLEScratch_004b2810))
        exit_squadron("qq copy overflow");
    packed = AllocateTaggedMemory(bytes, 0);
    memcpy(packed, g_abShapeRLEScratch_004b2810, bytes);
#ifdef SDL_PORT
    memcpy(shape - 8 - sizeof(unsigned char *), &packed,
           sizeof(packed));
#else
    memcpy(shape - 4, &packed, 4);
#endif
}

/* Function start: 0x425FB4 */
void DrawSpriteTransformed(Viewport *viewport, int x, int y,
                           unsigned char *shape, int frame,
                           int angle, int scaleX, int scaleY,
                           int flip, int blendMode)
{
    /* Performance-counter samples taken around each stage. */
    LARGE_INTEGER t0;
    LARGE_INTEGER t1;
    LARGE_INTEGER t2;
    LARGE_INTEGER t3;

    if (shape == 0 || frame < 0 || viewport->pixels == 0 ||
        viewport->rowOffsets == 0)
        return;
    if (frame >= GetShapeFrameCount(shape))
        return;
    if (viewport->left < 0)
        return;
    ReadPerformanceCounter(&t0);
    PrepareShapeRLEData(shape);
    ReadPerformanceCounter(&t1);
    ClipViewportToScreen(viewport);
    ReadPerformanceCounter(&t2);
    if (flip != 0) {
        if (flip == 0x10) {
            scaleX = -scaleX;
        } else if (flip == 0x20) {
            scaleY = -scaleY;
        } else if (flip == 0x30) {
            scaleX = -scaleX;
            scaleY = -scaleY;
        } else {
            exit_squadron("bad flip");
        }
    }
    if (blendMode != 0) {
        RotateRLEImage(&g_stRasterClip_004b2088,
                       GetPreparedShapeData(shape), frame,
                       x - viewport->left, y - viewport->top,
                       g_abShapeTransformScratch_004a2688,
                       angle * 10, scaleX * 256, scaleY * 256, 1);
    } else {
        RotateRLEImage(&g_stRasterClip_004b2088,
                       GetPreparedShapeData(shape), frame,
                       x - viewport->left, y - viewport->top,
                       g_abShapeTransformScratch_004a2688,
                       angle * 10, scaleX * 256, scaleY * 256, 0);
    }
    ReadPerformanceCounter(&t3);
    g_nShapePrepareTicks_005d2fd0 += t1.LowPart - t0.LowPart;
    g_nShapeClipTicks_005d2fd4 += t2.LowPart - t1.LowPart;
    g_nShapeDrawTicks_005d2fd8 += t3.LowPart - t2.LowPart;
    if (viewport->pixels == g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
}

/* Function start: 0x4261D5 */
void RasterLineHook(const void *marker)
{
    (void)marker;
    return;
}

/* Function start: 0x4261E5 */
void DrawFontGlyph(char character, TextContext *context, int height,
                   int width, int y)
{
    int column;
    int drawY;
    Viewport *view;
    unsigned char fg;
    unsigned char fontFg;
    unsigned char bg;
    unsigned char fontBg;
    unsigned char translatedPixel;
    unsigned char *dest;
    int dy;
    int dx;
    unsigned char *bitmap;

    drawY = y;
    view = context->viewport;
    fg = context->colour;
    bg = context->backgroundColour;
    fontFg = context->font[2];
    fontBg = context->font[3];
    g_abPaletteTranslation_00496338[fontFg] = fg;
    g_abPaletteTranslation_00496338[fontBg] = bg;
#ifndef SDL_PORT
    /* Preserve the retail Win32 port's signed-char comparisons. */
    if (character == 0x81 || character == 0x84 || character == 0x8e ||
        character == 0x94 || character == 0x99 || character == 0x9a ||
        character == 0xe1) {
        /* The DOS game drew these umlauts from the font workspace; the
         * Win32 port skips them. */
        return;
        bitmap = (*context->fontWorkspace)->pixels;
        width = (*context->fontWorkspace)->width;
        height = (*context->fontWorkspace)->height;
        for (dy = 0; dy < height; dy++) {
            drawY = y + dy;
            if (view->top == drawY) {
                if ((view->rowOffsets[drawY] & 0x8000) != 0)
                    dest = context->cursorX + view->pixels;
                else
                    dest = view->rowOffsets[drawY] + context->cursorX +
                           view->pixels;
            } else {
                dest = view->rowOffsets[drawY] + context->cursorX +
                       view->pixels;
            }
            for (dx = 0; dx < width; dx++)
                dest[dx] = bitmap[width * dy + dx];
        }
        context->cursorX += width;
    } else
#endif
    {
#ifdef SDL_PORT
        bitmap = (context->font[(unsigned char)character + 0x204] << 8) +
                 context->font[(unsigned char)character + 0x104] +
                 context->font;
#else
        bitmap = (context->font[character + 0x204] << 8) +
                 context->font[character + 0x104] + context->font;
#endif
        if (fontFg != fg || fontBg != bg) {
            while (height-- != 0) {
                if (view->top == drawY) {
                    if ((view->rowOffsets[drawY] & 0x8000) != 0)
                        dest = context->cursorX + view->pixels;
                    else
                        dest = view->rowOffsets[drawY] + context->cursorX +
                               view->pixels;
                } else {
                    dest = view->rowOffsets[drawY] + context->cursorX +
                           view->pixels;
                }
                for (column = width; column-- != 0;) {
                    translatedPixel = g_abPaletteTranslation_00496338[*bitmap];
                    if (translatedPixel != 0xff)
                        *dest = translatedPixel;
                    bitmap++;
                    dest++;
                }
                drawY++;
            }
#ifdef SDL_PORT
            context->cursorX += context->font[(unsigned char)character + 4];
#else
            context->cursorX += context->font[character + 4];
#endif
        } else {
            while (height-- != 0) {
                if (view->top == drawY) {
                    if ((view->rowOffsets[drawY] & 0x8000) != 0)
                        dest = context->cursorX + view->pixels;
                    else
                        dest = view->rowOffsets[drawY] + context->cursorX +
                               view->pixels;
                } else {
                    dest = view->rowOffsets[drawY] + context->cursorX +
                           view->pixels;
                }
                for (column = width; column-- != 0;) {
                    if (*bitmap != 0xff)
                        *dest = *bitmap;
                    bitmap++;
                    dest++;
                }
                drawY++;
            }
#ifdef SDL_PORT
            context->cursorX += context->font[(unsigned char)character + 4];
#else
            context->cursorX += context->font[character + 4];
#endif
        }
    }
    g_abPaletteTranslation_00496338[fontFg] = fontFg;
    g_abPaletteTranslation_00496338[fontBg] = fontBg;
    return;
}

/* Function start: 0x42665A */
/* Nothing in the shipped executable calls this; the Win32 port only reports
 * the call to the driver's line copier. */
void __copyline(void)
{
    RasterLineHook("__copyline");
    return;
}

/* Function start: 0x426677 */
/* Nothing in the shipped executable calls this; the Win32 port only reports
 * the call to the driver's line eraser. */
void __wipeline(void)
{
    RasterLineHook("__wipeline");
    return;
}

/* Function start: 0x426694 */
void EstablishColors(Viewport *viewport, unsigned char *active)
{
    RasterLineHook("EstablishColors");
    return;
}

/* Function start: 0x4266B1 */
void GetPaletteEntry(short index, unsigned short *rgb)
{
    GetPaletteEntryAsWords(index, rgb);
}

/* Function start: 0x4266CC */
void SetPaletteEntry(short index, short *rgb)
{
    DIBsetPalette(index, rgb);
}

/* Function start: 0x4266E7 */
void DrawSpriteDefault(Viewport *viewport, short x, short y,
                       unsigned char *shape, short frame)
{
    if (shape == 0)
        return;
    if (frame < 0)
        return;
    if (viewport->left < 0)
        return;
    if (HasValidShapeAllocationSignature(shape) != 0)
        DrawSpriteTransformed(viewport, x, y, shape, frame,
                              0, 0x100, 0x100, 0, 0);
}

short GetTransformedShapeBounds(Viewport *viewport, short x, short y,
                              unsigned char *shape, short frame,
                              short angle, short scale, short flip,
                              short *bounds);
void FillPolygon(Viewport *viewport, ShortPoint *points, int count,
                 char colour);
void triangle(Viewport *viewport, ShortPoint *points, int colour);

/* Function start: 0x426769 */
void CaptureSpriteBackground(Viewport *viewport, unsigned char *background,
                             short x, short y, unsigned char *shape,
                             short frame)
{
    unsigned char *commands;
    unsigned short rowOp;
    unsigned char *screenRow;
    unsigned char *pixels;
    unsigned char action;
    unsigned char color;
    unsigned char *original;
    short span;
    int endX;
    short yDelta;
    short leftmost;
    short rightmost;
    short minY;
    short maxY;
    int screenX;
    short xDelta;
    int row;
    int skip;

    if (viewport->left < 0)
        return;
    if (HasValidShapeAllocationSignature(shape) == 0)
        return;
    original = background;
    if (original == 0)
        return;
    if (shape == 0)
        return;
    if (frame < 0)
        return;
    frame++;
    frame <<= 2;
    if (*(unsigned short *)(shape + 4) > frame) {
        leftmost = viewport->left;
        rightmost = viewport->right;
        minY = viewport->top;
        maxY = viewport->bottom;
        commands = shape + *(int *)(frame + shape);
        commands += 8;
        pixels = viewport->pixels;
#ifdef SDL_PORT
        memcpy(&rowOp, commands, sizeof(rowOp));
#else
        rowOp = *(unsigned short *)commands;
#endif
        commands += 2;
        while (rowOp != 0) {
#ifdef SDL_PORT
            memcpy(&xDelta, commands, sizeof(xDelta));
#else
            xDelta = *(short *)commands;
#endif
            commands += 2;
#ifdef SDL_PORT
            memcpy(&yDelta, commands, sizeof(yDelta));
#else
            yDelta = *(short *)commands;
#endif
            commands += 2;
            screenX = x + xDelta;
            row = y + yDelta;
#ifdef SDL_PORT
            /* WC2 forms the row pointer before it knows the row is on screen,
             * so a sprite straddling the top or bottom edge reads one entry
             * off the end of the row-offset table.  Nothing is copied unless
             * the row passes the same test below, so on the port take the
             * offset only when it is in range. */
            if (minY <= row && maxY >= row)
                screenRow = viewport->rowOffsets[row] + screenX + pixels;
            else
                screenRow = screenX + pixels;
#else
            screenRow = viewport->rowOffsets[row] + screenX + pixels;
#endif
            if ((rowOp & 1) != 0) {
                rowOp >>= 1;
                while (rowOp > 0) {
                    action = *commands;
                    commands++;
                    if ((action & 1) != 0) {
                        action >>= 1;
                        rowOp = (unsigned short)(rowOp - action);
                        color = *commands;
                        commands++;
                        if (minY <= row && maxY >= row) {
                            endX = action + screenX - 1;
                            if (rightmost >= screenX &&
                                leftmost <= endX) {
                                span = action;
                                skip = 0;
                                if (leftmost > screenX) {
                                    span -= leftmost - screenX;
                                    skip += leftmost - screenX;
                                }
                                if (rightmost < endX)
                                    span -= endX - rightmost;
                                memcpy(original, skip + screenRow, span);
                                original += span;
                            }
                        }
                        screenRow += action;
                        screenX += action;
                    } else {
                        action >>= 1;
                        rowOp = (unsigned short)(rowOp - action);
                        if (minY <= row && maxY >= row) {
                            endX = action + screenX - 1;
                            if (rightmost >= screenX &&
                                leftmost <= endX) {
                                span = action;
                                skip = 0;
                                if (leftmost > screenX) {
                                    span -= leftmost - screenX;
                                    skip += leftmost - screenX;
                                }
                                if (rightmost < endX)
                                    span -= endX - rightmost;
                                memcpy(original, skip + screenRow, span);
                                original += span;
                            }
                        }
                        commands += action;
                        screenRow += action;
                        screenX += action;
                    }
                }
            } else {
                rowOp >>= 1;
                if (minY <= row && maxY >= row) {
                    endX = rowOp + screenX - 1;
                    if (rightmost >= screenX && leftmost <= endX) {
                        span = rowOp;
                        skip = 0;
                        if (leftmost > screenX) {
                            span -= leftmost - screenX;
                            skip += leftmost - screenX;
                        }
                        if (rightmost < endX)
                            span -= endX - rightmost;
                        memcpy(original, skip + screenRow, span);
                        original += span;
                    }
                }
                commands += rowOp;
            }
#ifdef SDL_PORT
            memcpy(&rowOp, commands, sizeof(rowOp));
#else
            rowOp = *(unsigned short *)commands;
#endif
            commands += 2;
        }
    }
    (void)color;
}

/* Forward declarations of this file's later drawing routines. */
void DrawFilledViewportRect(Viewport *viewport, short left, short top,
                            short right, short bottom, short color);
void DrawSpriteScaled(Viewport *viewport, short x, short y,
                      unsigned char *shape, short frame, short angle,
                      short scale, short flip);
int HasValidShapeAllocationSignature(unsigned char *shape);

/* Function start: 0x426B96 */
void RestoreSpriteBackground(Viewport *viewport, unsigned char *background,
                             short x, short y, unsigned char *shape,
                             short frame)
{
    unsigned char *commands;
    unsigned short rowOp;
    unsigned char *screenRow;
    unsigned char *pixels;
    unsigned char action;
    unsigned char color;
    unsigned char *original;
    short span;
    int endX;
    short yDelta;
    short leftmost;
    short rightmost;
    short minY;
    short maxY;
    int screenX;
    short xDelta;
    int row;
    int skip;

    original = background;
    if (original == 0)
        return;
    if (shape == 0)
        return;
    if (frame < 0)
        return;
    if (viewport->left < 0)
        return;
    if (HasValidShapeAllocationSignature(shape) == 0)
        return;
    frame++;
    frame <<= 2;
    if (*(unsigned short *)(shape + 4) > frame) {
        leftmost = viewport->left;
        rightmost = viewport->right;
        minY = viewport->top;
        maxY = viewport->bottom;
        commands = shape + *(int *)(frame + shape);
        commands += 8;
        pixels = viewport->pixels;
#ifdef SDL_PORT
        memcpy(&rowOp, commands, sizeof(rowOp));
#else
        rowOp = *(unsigned short *)commands;
#endif
        commands += 2;
        while (rowOp != 0) {
#ifdef SDL_PORT
            memcpy(&xDelta, commands, sizeof(xDelta));
#else
            xDelta = *(short *)commands;
#endif
            commands += 2;
#ifdef SDL_PORT
            memcpy(&yDelta, commands, sizeof(yDelta));
#else
            yDelta = *(short *)commands;
#endif
            commands += 2;
            screenX = x + xDelta;
            row = y + yDelta;
#ifdef SDL_PORT
            /* WC2 forms the row pointer before it knows the row is on screen,
             * so a sprite straddling the top or bottom edge reads one entry
             * off the end of the row-offset table.  Nothing is copied unless
             * the row passes the same test below, so on the port take the
             * offset only when it is in range. */
            if (minY <= row && maxY >= row)
                screenRow = viewport->rowOffsets[row] + screenX + pixels;
            else
                screenRow = screenX + pixels;
#else
            screenRow = viewport->rowOffsets[row] + screenX + pixels;
#endif
            if ((rowOp & 1) != 0) {
                rowOp >>= 1;
                while (rowOp > 0) {
                    action = *commands;
                    commands++;
                    if ((action & 1) != 0) {
                        action >>= 1;
                        rowOp = (unsigned short)(rowOp - action);
                        color = *commands;
                        commands++;
                        if (minY <= row && maxY >= row) {
                            endX = action + screenX - 1;
                            if (rightmost >= screenX &&
                                leftmost <= endX) {
                                span = action;
                                skip = 0;
                                if (leftmost > screenX) {
                                    span -= leftmost - screenX;
                                    skip += leftmost - screenX;
                                }
                                if (rightmost < endX)
                                    span -= endX - rightmost;
                                memcpy(skip + screenRow, original, span);
                                original += span;
                            }
                        }
                        screenRow += action;
                        screenX += action;
                    } else {
                        action >>= 1;
                        rowOp = (unsigned short)(rowOp - action);
                        if (minY <= row && maxY >= row) {
                            endX = action + screenX - 1;
                            if (rightmost >= screenX &&
                                leftmost <= endX) {
                                span = action;
                                skip = 0;
                                if (leftmost > screenX) {
                                    span -= leftmost - screenX;
                                    skip += leftmost - screenX;
                                }
                                if (rightmost < endX)
                                    span -= endX - rightmost;
                                memcpy(skip + screenRow, original, span);
                                original += span;
                            }
                        }
                        commands += action;
                        screenRow += action;
                        screenX += action;
                    }
                }
            } else {
                rowOp >>= 1;
                if (minY <= row && maxY >= row) {
                    endX = rowOp + screenX - 1;
                    if (rightmost >= screenX && leftmost <= endX) {
                        span = rowOp;
                        skip = 0;
                        if (leftmost > screenX) {
                            span -= leftmost - screenX;
                            skip += leftmost - screenX;
                        }
                        if (rightmost < endX)
                            span -= endX - rightmost;
                        memcpy(skip + screenRow, original, span);
                        original += span;
                    }
                }
                commands += rowOp;
            }
#ifdef SDL_PORT
            memcpy(&rowOp, commands, sizeof(rowOp));
#else
            rowOp = *(unsigned short *)commands;
#endif
            commands += 2;
        }
    }
    if (viewport->pixels == g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
    (void)color;
}

/* Function start: 0x426FD9 */
void DrawSolidColourSprite(Viewport *viewport, short x, short y,
                           unsigned char *shape, short frame,
                           unsigned short colour)
{
    if (viewport->left < 0)
        return;
    SetSolidColourTranslation(colour);
    if (HasValidShapeAllocationSignature(shape) != 0)
        DrawSpriteTransformed(viewport, x, y, shape, frame, 0,
                              0x100, 0x100, 0, 1);
}

#pragma function(abs)
/* Function start: 0x427047 */
void CopyViewportContents(Viewport *source, Viewport *destination)
{
    RasterSurface sourceSurface;
    RasterClip targetClip;
    int result;
    RasterClip srcClip;
    RasterSurface dstSurface;

    if (destination->left < 0)
        return;
    if (source->left < 0)
        return;
#ifdef SDL_PORT
    /* ValidateViewportBounds screens left but not top, and the wipe hands it
     * a viewport a row above the buffer as it steps: rowOffsets[-1].  The
     * original read the word in front of the table; skip the copy instead.
     * Screening here rather than in the validator keeps the surface and clip
     * it fills in from being left undefined for the blit below. */
    if (destination->top < 0 || source->top < 0)
        return;
#endif
    ValidateViewportBounds(source, &sourceSurface, &srcClip);
    ValidateViewportBounds(destination, &dstSurface,
                           &targetClip);
    result = BlitRasterClip(
        &srcClip, 0, 0, &targetClip, 0, 0, 0xffffffff);
    if (result != 0)
        abs(result);
    if (g_nCockpitDisplayMode_0049d71c == 9)
        return;
    if (g_bRenderingSpaceFrame_0049d858 != 0 &&
        g_bSceneDisplayUpdateActive_00499bb8 == 0) {
        return;
    }
    if (GetDIBPixelBuffer() == destination->pixels) {
        MarkDibDirty();
        DIBslamReal();
    }
}

/* Function start: 0x427123 */
void ClearViewport(Viewport *viewport, short colour)
{
    if (viewport->pixels != 0 && viewport->rowOffsets != 0) {
        if (viewport->left < 0)
            return;
        ClipViewportToScreen(viewport);
        FillRasterClip(&g_stRasterClip_004b2088, colour);
    }
}

/* Function start: 0x427179 */
void DrawViewportPixel(Viewport *viewport, short x, short y,
                       short colour)
{
    if (viewport->left < 0)
        return;
    ClipViewportToScreen(viewport);
    SetRasterClipPixel(&g_stRasterClip_004b2088, (int)x - viewport->left,
                       (int)y - viewport->top, colour);
}

/* Function start: 0x4271D2 */
short GetViewportPixel(Viewport *viewport, short x, short y)
{
    if (viewport->left < 0)
        return 0;
    ClipViewportToScreen(viewport);
    return (short)ReadRasterClipPixel(&g_stRasterClip_004b2088,
                                      (int)x - viewport->left,
                                      (int)y - viewport->top);
}

/* Function start: 0x42722E */
void DrawViewportLine(Viewport *viewport, short x1, short y1,
                      short x2, short y2, short colour)
{
    if (viewport->left < 0)
        return;
    ClipViewportToScreen(viewport);
    DrawClippedLine(&g_stRasterClip_004b2088,
                    x1 - viewport->left, y1 - viewport->top,
                    x2 - viewport->left, y2 - viewport->top,
                    0, colour);
}

/* Left and right edge of each scanline of the polygon being filled, two
 * ints per row.  Only FillPolygon uses it; WC1's optimized build kept the
 * same 1,648 bytes in gr.c without a reference. */
static int g_anPolygonSpans_004b20a0[412];

/* Function start: 0x4272A5 */
/* Fills a convex polygon: clips it against the viewport's top, right,
 * bottom and left edges in turn, scan-converts the clipped outline into
 * left and right spans, and fills each span.  The Win32 port returns
 * before any of it runs; triangle is its only caller. */
void FillPolygon(Viewport *viewport, ShortPoint *points, int count,
                 char colour)
{
    ShortPoint polyB[8];
    int side;
    int prev;
    unsigned char *row;
    int bottomIndex;
    int stride;
    float denominator;
    int countA;
    int *rowBounds;
    int x;
    ShortPoint polyA[8];
    int topIndex;
    int y1;
    int i;
    int x0;
    int x1;
    int k;
    int y0;
    int countB;
    float distance;
    float fractionStep;
    int j;
    float frac;
    int crossed;
    int y;

    return;
    countA = 0;
    y = viewport->top;
    for (i = 0; i < count; i++) {
        j = (i + 1) % count;
        crossed = 0;
        if (points[i].y >= y) {
            polyA[countA].x = points[i].x;
            polyA[countA].y = points[i].y;
            countA++;
            if (points[j].y <= y)
                crossed = 1;
        } else if (points[j].y >= y)
            crossed = 1;
        if (crossed) {
            distance = (float)(y - points[i].y);
            denominator = (float)(points[j].y - points[i].y);
            frac = distance / denominator;
            distance = (float)points[i].x;
            denominator = (float)(points[j].x - points[i].x);
            x = denominator * frac + distance + 0.5f;
            polyA[countA].x = x;
            polyA[countA].y = y;
            countA++;
        }
    }
    if (countA < 3)
        return;

    countB = 0;
    x = viewport->right;
    for (i = 0; i < countA; i++) {
        j = (i + 1) % countA;
        crossed = 0;
        if (polyA[i].x <= x) {
            polyB[countB].x = polyA[i].x;
            polyB[countB].y = polyA[i].y;
            countB++;
            if (polyA[j].x >= x)
                crossed = 1;
        } else if (polyA[j].x <= x)
            crossed = 1;
        if (crossed) {
            distance = (float)(x - polyA[i].x);
            denominator = (float)(polyA[j].x - polyA[i].x);
            frac = distance / denominator;
            distance = (float)polyA[i].y;
            denominator = (float)(polyA[j].y - polyA[i].y);
            y = denominator * frac + distance + 0.5f;
            polyB[countB].x = x;
            polyB[countB].y = y;
            countB++;
        }
    }
    if (countB < 3)
        return;

    countA = 0;
    y = viewport->bottom;
    for (i = 0; i < countB; i++) {
        j = (i + 1) % countB;
        crossed = 0;
        if (polyB[i].y <= y) {
            polyA[countA].x = polyB[i].x;
            polyA[countA].y = polyB[i].y;
            countA++;
            if (polyB[j].y >= y)
                crossed = 1;
        } else if (polyB[j].y <= y)
            crossed = 1;
        if (crossed) {
            distance = (float)(y - polyB[i].y);
            denominator = (float)(polyB[j].y - polyB[i].y);
            frac = distance / denominator;
            distance = (float)polyB[i].x;
            denominator = (float)(polyB[j].x - polyB[i].x);
            x = denominator * frac + distance + 0.5f;
            polyA[countA].x = x;
            polyA[countA].y = y;
            countA++;
        }
    }
    if (countA < 3)
        return;

    countB = 0;
    x = viewport->left;
    for (i = 0; i < countA; i++) {
        j = (i + 1) % countA;
        crossed = 0;
        if (polyA[i].x >= x) {
            polyB[countB].x = polyA[i].x;
            polyB[countB].y = polyA[i].y;
            countB++;
            if (polyA[j].x <= x)
                crossed = 1;
        } else if (polyA[j].x >= x)
            crossed = 1;
        if (crossed) {
            distance = (float)(x - polyA[i].x);
            denominator = (float)(polyA[j].x - polyA[i].x);
            frac = distance / denominator;
            distance = (float)polyA[i].y;
            denominator = (float)(polyA[j].y - polyA[i].y);
            y = denominator * frac + distance + 0.5f;
            polyB[countB].x = x;
            polyB[countB].y = y;
            countB++;
        }
    }
    if (countB < 3)
        return;

    bottomIndex = topIndex = 0;
    y = countB;
    while (y--) {
        if (polyB[topIndex].y == polyB[y].y) {
            if (polyB[topIndex].x > polyB[y].x)
                topIndex = y;
        } else if (polyB[topIndex].y > polyB[y].y)
            topIndex = y;
        else if (polyB[bottomIndex].y < polyB[y].y)
            bottomIndex = y;
    }

    i = prev = topIndex;
    side = 0;
    crossed = 0;
    for (k = 0; k < countB; k++) {
        if (--prev < 0)
            prev = countB - 1;
        x0 = polyB[i].x;
        y0 = polyB[i].y;
        x1 = polyB[prev].x;
        y1 = polyB[prev].y;
        if (--i < 0)
            i = countB - 1;
        if (crossed == 0 && y1 < y0) {
            side = 1;
            crossed = 1;
        }
        if (x1 == x0) {
            if (y1 < y0) {
                x = x0;
                x0 = x1;
                x1 = x;
                y = y0;
                y0 = y1;
                y1 = y;
            }
            for (y = y0; y <= y1; y++)
                g_anPolygonSpans_004b20a0[y * 2 + side] = x0;
        } else if (y1 == y0) {
            if (x1 > x0) {
                g_anPolygonSpans_004b20a0[y0 * 2] = x0;
                g_anPolygonSpans_004b20a0[y0 * 2 + 1] = x1;
            } else {
                g_anPolygonSpans_004b20a0[y0 * 2] = x1;
                g_anPolygonSpans_004b20a0[y0 * 2 + 1] = x0;
            }
        } else {
            if (y1 < y0) {
                x = x0;
                x0 = x1;
                x1 = x;
                y = y0;
                y0 = y1;
                y1 = y;
            }
            fractionStep = 1.0f / ((y1 - y0) + 1.0f);
            frac = 0;
            g_anPolygonSpans_004b20a0[y0 * 2 + side] = x0;
            for (y = y0; y <= y1; y++) {
                g_anPolygonSpans_004b20a0[y * 2 + side] =
                    (x1 - x0) * frac + x0;
                frac += fractionStep;
            }
            g_anPolygonSpans_004b20a0[y1 * 2 + side] = x1;
        }
    }

    i = polyB[topIndex].y * 2;
    if (g_anPolygonSpans_004b20a0[i + 1] < g_anPolygonSpans_004b20a0[i]) {
        for (y = polyB[topIndex].y; y < polyB[bottomIndex].y; y++) {
            i = y * 2;
            x0 = g_anPolygonSpans_004b20a0[i];
            g_anPolygonSpans_004b20a0[i] = g_anPolygonSpans_004b20a0[i + 1];
            g_anPolygonSpans_004b20a0[i + 1] = x0;
        }
    }

    stride = viewport->rowOffsets[1];
    row = viewport->pixels;
    row += (polyB[topIndex].y - viewport->top) * stride;
    rowBounds = &g_anPolygonSpans_004b20a0[polyB[topIndex].y * 2];
    x = viewport->left;
    for (y = polyB[topIndex].y; y <= polyB[bottomIndex].y; y++) {
        memset(row + (rowBounds[0] - x), colour, abs(rowBounds[1] - rowBounds[0]) + 1);
        row += stride;
        rowBounds += 2;
    }
    if (viewport->pixels == g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
}

/* Function start: 0x427D9F */
/* Nothing in the shipped executable calls this. */
void triangle(Viewport *viewport, ShortPoint *points, int colour)
{
    FillPolygon(viewport, points, 3, colour);
    if (viewport->pixels == g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
    RasterLineHook("triangle");
    return;
}

/* Function start: 0x427DE8 */
/* Nothing in the shipped executable calls this; the Win32 port does not
 * implement the driver's shaded triangle and only reports the call. */
void shaded_triangle(Viewport *viewport)
{
    if (viewport->pixels == g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
    RasterLineHook("shaded_triangle");
    return;
}

/* Function start: 0x427E1B */
void DrawFilledViewportRect(Viewport *viewport, short left, short top,
                            short right, short bottom, short colour)
{
    int y;
    int rows;

    rows = bottom - top;
    if (viewport->left < 0)
        return;
    ClipViewportToScreen(viewport);
    for (y = 0; y <= rows; y++) {
        DrawClippedLine(&g_stRasterClip_004b2088,
                        left - viewport->left,
                        y + top - viewport->top,
                        right - viewport->left,
                        y + top - viewport->top,
                        0, colour);
    }
}

/* Function start: 0x427EC8 */
void DrawViewportBorder(Viewport *viewport, short left, short top,
                        short right, short bottom, short colour)
{
    if (viewport->left < 0)
        return;
    ClipViewportToScreen(viewport);
    DrawClippedLine(&g_stRasterClip_004b2088,
                    left - viewport->left, top - viewport->top,
                    right - viewport->left, top - viewport->top,
                    0, colour);
    DrawClippedLine(&g_stRasterClip_004b2088,
                    left - viewport->left, bottom - viewport->top,
                    right - viewport->left, bottom - viewport->top,
                    0, colour);
    DrawClippedLine(&g_stRasterClip_004b2088,
                    left - viewport->left, top - viewport->top,
                    left - viewport->left, bottom - viewport->top,
                    0, colour);
    DrawClippedLine(&g_stRasterClip_004b2088,
                    right - viewport->left, top - viewport->top,
                    right - viewport->left, bottom - viewport->top,
                    0, colour);
}

/* Function start: 0x428023 */
void DrawViewportEllipse(Viewport *viewport, short x, short y,
                         short verticalRadius, short horizontalRadius,
                         short colour)
{
    if (viewport->left < 0)
        return;
    ClipViewportToScreen(viewport);
    DrawRasterEllipse(&g_stRasterClip_004b2088, x, y, horizontalRadius,
                      verticalRadius, colour);
    if (viewport->pixels == g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
    return;
}

/* Function start: 0x42808F */
void FillViewportEllipse(Viewport *viewport, short x, short y,
                         short verticalRadius, short horizontalRadius,
                         short colour)
{
    if (viewport->left < 0)
        return;
    ClipViewportToScreen(viewport);
    FillRasterEllipse(&g_stRasterClip_004b2088, x, y, horizontalRadius,
                      verticalRadius, colour);
    if (viewport->pixels == g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
    return;
}

/* Function start: 0x4280FB */
void DrawViewportEllipseShadow(Viewport *viewport, short x, short y,
                               short verticalRadius,
                               short horizontalRadius, short colour)
{
    if (viewport->left < 0)
        return;
    ClipViewportToScreen(viewport);
    DrawRasterEllipse(&g_stRasterClip_004b2088, x, y, horizontalRadius,
                      verticalRadius, colour);
    if (viewport->pixels == g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
    return;
}

/* Function start: 0x428167 */
/* Nothing in the shipped executable calls this; the Win32 port does not
 * implement the driver's shadow drawing and only reports the call. */
void shadow_draw(Viewport *viewport)
{
    if (viewport->left < 0)
        return;
    if (viewport->pixels == g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
    RasterLineHook("shadow_draw");
    return;
}

/* Function start: 0x4281AE */
/* Nothing in the shipped executable calls this.  Draws a shape frame with
 * one scale for both axes. */
void DrawShapeFrameScaled(Viewport *viewport, unsigned char *shape,
                          int frame, int x, int y, int angle, int scale)
{
    if (viewport->left < 0)
        return;
    if (shape == 0)
        return;
    if (frame < 0)
        return;
    DrawSpriteTransformed(viewport, x, y, shape, frame, angle, scale, scale,
                          0, 0);
}

/* Function start: 0x428217 */
/* Nothing in the shipped executable calls this.  Stores the four corners of
 * a shape frame's box, placed at (x, y), as a polygon. */
void GetShapeFrameCorners(unsigned char *shape, int frame, int x, int y,
                          short *points, int *count)
{
    unsigned char *base;
    int right;
    int minX;
    int minY;
    int maxY;

    base = shape;
    *count = 0;
    CheckHeapBlockSignature(shape);
    frame++;
    frame <<= 2;
    if (*(unsigned short *)(shape + 4) >= frame) {
        shape += *(int *)(frame + shape);
        right = *(short *)(shape + 2) + x;
        minX = *(short *)shape + x;
        minY = *(short *)(shape + 4) + y;
        maxY = *(short *)(shape + 6) + y;
        points[0] = (short)right;
        points[1] = (short)minY;
        points[2] = (short)right;
        points[3] = (short)maxY;
        points[4] = (short)minX;
        points[5] = (short)maxY;
        points[6] = (short)minX;
        points[7] = (short)minY;
        *count = 4;
    }
}

/* Function start: 0x4282F0 */
void DrawSpriteScaled(Viewport *viewport, short x, short y,
                      unsigned char *shape, short frame, short angle,
                      short scale, short flip)
{
    if (HasValidShapeAllocationSignature(shape) != 0) {
        DrawSpriteTransformed(viewport, x, y, shape, frame, angle,
                              scale, scale, flip, 0);
    }
}

/* Function start: 0x428344 */
void DrawSolidColourSpriteScaled(Viewport *viewport, short x, short y,
                                 unsigned char *shape, short frame,
                                 short angle, short scale, short flip,
                                 unsigned short colour)
{
    SetSolidColourTranslation(colour);
    if (HasValidShapeAllocationSignature(shape) != 0) {
        DrawSpriteTransformed(viewport, x, y, shape, frame, angle,
                              scale, scale, flip, 1);
    }
}

/* Function start: 0x4283A4 */
short GetTransformedShapeBounds(Viewport *viewport, short x, short y,
                              unsigned char *shape, short frame,
                              short angle, short scale, short flip,
                              short *bounds)
{
    int touches;
    ShortRect box;
    int s;
    int transformedHeight;
    int framePos;
    int frameBottom;
    int width;
    int right;
    int widthSum;
    int topExtent;
    int fullHeight;
    unsigned char *frameHeader;
    int topY;
    int frameLeft;
    int screenLeft;
    int cosTerm;

    touches = 0;
    if (shape == 0) {
        if (viewport->left > x || viewport->right < x ||
            viewport->top > y || viewport->bottom < y)
            touches = 0;
        else
            touches = 1;
        return (short)touches;
    }
    if (HasValidShapeAllocationSignature(shape) == 0)
        return 0;
    CheckHeapBlockSignature(shape);
    framePos = frame;
    frameHeader = shape;
    framePos++;
    framePos <<= 2;
    if (*(unsigned short *)(frameHeader + 4) >= framePos) {
        frameHeader += *(int *)(framePos + frameHeader);
        right = *(short *)(frameHeader + 2);
        frameLeft = *(short *)frameHeader;
        topExtent = *(short *)(frameHeader + 4);
        frameBottom = *(short *)(frameHeader + 6);
        cosTerm = g_awAbsoluteCosine_00496438[angle] * scale;
        cosTerm >>= 8;
        s = g_awAbsoluteSine_00496708[angle] * scale;
        s >>= 8;
        if (cosTerm == 0)
            cosTerm++;
        if (s == 0)
            s++;
        widthSum = frameLeft + right;
        fullHeight = frameBottom + topExtent;
        /* WC2 rounds each extent up when the 8.8 fraction is not zero. */
        transformedHeight = s * widthSum + cosTerm * fullHeight;
        if ((transformedHeight & 0xff) != 0)
            transformedHeight += 0x100;
        transformedHeight >>= 8;
        width = s * fullHeight + cosTerm * widthSum;
        if ((width & 0xff) != 0)
            width += 0x100;
        width >>= 8;
        if (cosTerm == 0)
            cosTerm = 1;
        if (s == 0)
            s = 1;
        screenLeft = (s * topExtent >> 8) + x - (cosTerm * right >> 8);
        topY = y - (cosTerm * topExtent >> 8) - (s * right >> 8);
        box.top = (short)topY;
        box.bottom = (short)(box.top + transformedHeight);
        fullHeight = s * fullHeight;
        fullHeight >>= 8;
        fullHeight++;
        fullHeight = screenLeft - fullHeight;
        box.left = (short)fullHeight;
        box.right = (short)(fullHeight + width);
        if (viewport->left > box.right || viewport->right < box.left ||
            viewport->top > box.bottom || box.top > viewport->bottom) {
            touches = 0;
        } else {
            bounds[0] = box.left;
            bounds[2] = box.right;
            bounds[1] = box.top;
            bounds[3] = box.bottom;
            touches = 1;
        }
    }
    (void)flip;
    return (short)touches;
}

/* Function start: 0x428690 */
/* WC2 walks the geometry record as a raw run list: two shorts of header, the
 * source origin, then a destinationX/y/span triple per run ending at
 * -1. */
void CopyViewportRuns(Viewport *source, Viewport *destination,
                 const short *runs)
{
    unsigned short span;
    short srcLeft;
    short srcTop;
    short destinationX;
    short y;

    if (source->pixels == 0 || destination->pixels == 0)
        return;
    if (destination->left < 0)
        return;
    runs += 2;
    srcLeft = *runs;
    runs++;
    srcTop = *runs;
    runs++;
    destinationX = *runs;
    runs++;
    while (destinationX != -1) {
        y = *runs;
        runs++;
        span = (unsigned short)*runs;
        runs++;
        memcpy(destination->rowOffsets[y] + destinationX +
                   destination->pixels,
               source->rowOffsets[y - srcTop] + destinationX +
                   source->pixels - srcLeft,
               span);
        destinationX = *runs;
        runs++;
    }
    if (destination->pixels == g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
}

/* Function start: 0x4287A4 */
/* Nothing in the shipped executable calls this; the Win32 port does not
 * implement the driver's fizzle fade and only reports the call. */
void fizzle_fade(Viewport *source, Viewport *destination)
{
    if (destination->left < 0)
        return;
    if (destination->pixels == g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
    RasterLineHook("fizzle_fade");
    return;
}

/* Function start: 0x4287EB */
void snow_viewport(Viewport *viewport, int effect,
                        unsigned short colour)
{
    int left;
    int yLast;
    int endX;
    unsigned char *dest;
    int topY;
    int rowWidth;
    int y;
    int lines;
    int countdown;

    if (viewport->left < 0)
        return;
    if (viewport->pixels == g_stScreenViewport_005d21a0.pixels) {
        left = viewport->left;
        topY = viewport->top;
        endX = viewport->right;
        yLast = viewport->bottom;
        rowWidth = endX - left + 1;
        for (y = topY; y < yLast; y++) {
            dest = viewport->rowOffsets[y] + viewport->left +
                   viewport->pixels;
            countdown = rowWidth;
            while (countdown-- != 0) {
                if (RandomBelow(g_nViewportStaticRandomRange_0048e060) == 1)
                    *dest = (unsigned char)colour;
                else
                    *dest = g_cSecondaryViewBufferColour_0049cb4c;
                dest++;
            }
        }
        MarkDibDirty();
    }
    RasterLineHook("snow_viewport");
    return;
}
