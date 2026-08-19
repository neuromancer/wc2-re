/*
 *  Rasteriser primitives and screen-space effects.
 *
 *  Address range 0x440c00-0x44274f (provisional -- see docs/ORDER.md).
 *  Boundary evidence: PROVEN by name: shadow_draw, fizzle_fade, snow_viewport.
 */
#include "wc1.h"

#pragma function(abs, memcpy, memset, sqrt)

/* Function start: 0x425A16 */
void ValidateViewportBounds(Viewport *viewport, RasterSurface *surface,
                            RasterClip *clip)
{
    int allocation;
    int topOffset;
    int nextOffset;
    int rowStrideOffset;

    if (viewport->pixels != g_stScreenViewport_005d21a0.pixels) {
        allocation = 0;
        while (allocation < g_nViewportAllocationCount_005d19bc) {
            if (g_apViewportAllocations_005a7f10[allocation] ==
                viewport->pixels)
                break;
            allocation++;
        }
        if (allocation >= g_nViewportAllocationCount_005d19bc)
            exit_squadron(g_szBadViewport_004969f8);
    }
    if (viewport->pixels == g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
    topOffset = SignExtendClipCoord(viewport->rowOffsets[viewport->top]);
    nextOffset = SignExtendClipCoord(
        viewport->rowOffsets[viewport->top + 1]);
    rowStrideOffset = SignExtendClipCoord(
        viewport->rowOffsets[viewport->top]);
    surface->pixels = viewport->pixels + viewport->left + topOffset;
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

/* Function start: 0x448570 */
signed char InitializeViewportWipe(Viewport *source,
                                   Viewport *destination,
                                   int wipeType, short duration,
                                   short passFlags, void *workspace)
{
    int radialWidth;
    int radialHeight;
    short edge;

    g_stWipeSourceViewport_005b3488 = *source;
    g_stWipeWorkingSourceViewport_005b34c8 =
        g_stWipeSourceViewport_005b3488;
    g_stWipeDestinationViewport_005b3450 = *destination;
    g_stWipeWorkingDestinationViewport_005b34a0 =
        g_stWipeDestinationViewport_005b3450;
    g_nWipeType_005b3444 = wipeType;
    g_nWipePassFlags_005b34b4 = passFlags;
    g_nWipeProgress_005b3434 = 0;
    duration = 15;

    switch (g_nWipeType_005b3444) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        g_nWipeWidth_005b3448 =
            destination->right - destination->left + 1;
        g_nWipeHeight_005b344c =
            destination->bottom - destination->top + 1;
        if (g_nWipeHeight_005b344c <= g_nWipeWidth_005b3448) {
            g_bWipeWidthAtLeastHeight_005b342c = 1;
            g_bWipeWidthLessThanHeight_005b3430 = 0;
            g_nWipeShortDimension_005b3428 = g_nWipeWidth_005b3448;
        } else {
            g_bWipeWidthAtLeastHeight_005b342c = 0;
            g_bWipeWidthLessThanHeight_005b3430 = 1;
            g_nWipeShortDimension_005b3428 = g_nWipeHeight_005b344c;
        }
        break;
    }

    switch (g_nWipeType_005b3444) {
    case 0:
        SetViewportVerticalBounds(
            &g_stWipeWorkingSourceViewport_005b34c8,
            g_stWipeSourceViewport_005b3488.top - 1,
            g_stWipeSourceViewport_005b3488.top - 1);
        SetViewportVerticalBounds(
            &g_stWipeWorkingDestinationViewport_005b34a0,
            g_stWipeDestinationViewport_005b3450.top - 1,
            g_stWipeDestinationViewport_005b3450.top - 1);
        g_nWipeVerticalThreshold_005b34bc = g_nWipeHeight_005b344c + 1;
        g_nWipeTotalSteps_005b3440 = g_nWipeHeight_005b344c + 1;
        g_nWipeMaximumStep_005b34c4 = (short)(
            g_nWipeHeight_005b344c + 1 < duration
                ? g_nWipeHeight_005b344c + 1
                : duration);
        g_nWipeVerticalStep_005b34b8 = g_nWipeMaximumStep_005b34c4;
        break;
    case 1:
        SetViewportHorizontalBounds(
            &g_stWipeWorkingSourceViewport_005b34c8,
            g_stWipeSourceViewport_005b3488.right + 1,
            g_stWipeSourceViewport_005b3488.right + 1);
        SetViewportHorizontalBounds(
            &g_stWipeWorkingDestinationViewport_005b34a0,
            g_stWipeDestinationViewport_005b3450.right + 1,
            g_stWipeDestinationViewport_005b3450.right + 1);
        g_nWipeHorizontalThreshold_005b347c = g_nWipeWidth_005b3448 + 1;
        g_nWipeTotalSteps_005b3440 = g_nWipeWidth_005b3448 + 1;
        g_nWipeMaximumStep_005b34c4 = (short)(
            g_nWipeWidth_005b3448 + 1 < duration
                ? g_nWipeWidth_005b3448 + 1
                : duration);
        g_nWipeHorizontalStep_005b3478 = g_nWipeMaximumStep_005b34c4;
        break;
    case 2:
        SetViewportVerticalBounds(
            &g_stWipeWorkingSourceViewport_005b34c8,
            g_stWipeSourceViewport_005b3488.bottom + 1,
            g_stWipeSourceViewport_005b3488.bottom + 1);
        SetViewportVerticalBounds(
            &g_stWipeWorkingDestinationViewport_005b34a0,
            g_stWipeDestinationViewport_005b3450.bottom + 1,
            g_stWipeDestinationViewport_005b3450.bottom + 1);
        g_nWipeVerticalThreshold_005b34bc = g_nWipeHeight_005b344c + 1;
        g_nWipeTotalSteps_005b3440 = g_nWipeHeight_005b344c + 1;
        g_nWipeMaximumStep_005b34c4 = (short)(
            g_nWipeHeight_005b344c + 1 < duration
                ? g_nWipeHeight_005b344c + 1
                : duration);
        g_nWipeVerticalStep_005b34b8 = g_nWipeMaximumStep_005b34c4;
        break;
    case 3:
        SetViewportHorizontalBounds(
            &g_stWipeWorkingSourceViewport_005b34c8,
            g_stWipeSourceViewport_005b3488.left - 1,
            g_stWipeSourceViewport_005b3488.left - 1);
        SetViewportHorizontalBounds(
            &g_stWipeWorkingDestinationViewport_005b34a0,
            g_stWipeDestinationViewport_005b3450.left - 1,
            g_stWipeDestinationViewport_005b3450.left - 1);
        g_nWipeHorizontalThreshold_005b347c = g_nWipeWidth_005b3448 + 1;
        g_nWipeTotalSteps_005b3440 = g_nWipeWidth_005b3448 + 1;
        g_nWipeMaximumStep_005b34c4 = (short)(
            g_nWipeWidth_005b3448 + 1 < duration
                ? g_nWipeWidth_005b3448 + 1
                : duration);
        g_nWipeHorizontalStep_005b3478 = g_nWipeMaximumStep_005b34c4;
        break;
    case 4:
        g_nWipePreviousHorizontalEdge_005b346c = -1;
        g_nWipePreviousVerticalEdge_005b3470 = -1;
        g_nWipeHorizontalThreshold_005b347c = g_nWipeWidth_005b3448 + 1;
        g_nWipeVerticalThreshold_005b34bc = g_nWipeHeight_005b344c + 1;
        g_nWipeTotalSteps_005b3440 = g_nWipeWidth_005b3448 + 1;
        g_nWipeMaximumStep_005b34c4 =
            (short)(duration < 320 ? duration : 320);
        g_nWipeVerticalStep_005b34b8 = g_nWipeMaximumStep_005b34c4;
        g_nWipeHorizontalStep_005b3478 = g_nWipeVerticalStep_005b34b8;
        break;
    case 5:
        g_nWipePreviousHorizontalEdge_005b346c =
            g_stWipeSourceViewport_005b3488.right -
            g_stWipeSourceViewport_005b3488.left + 1;
        g_nWipePreviousVerticalEdge_005b3470 = -1;
        g_nWipeHorizontalThreshold_005b347c = g_nWipeWidth_005b3448 + 1;
        g_nWipeVerticalThreshold_005b34bc = g_nWipeHeight_005b344c + 1;
        g_nWipeTotalSteps_005b3440 = g_nWipeWidth_005b3448 + 1;
        g_nWipeMaximumStep_005b34c4 =
            (short)(duration < 320 ? duration : 320);
        g_nWipeHorizontalStep_005b3478 = g_nWipeMaximumStep_005b34c4;
        g_nWipeVerticalStep_005b34b8 = g_nWipeMaximumStep_005b34c4;
        break;
    case 6:
        g_nWipePreviousHorizontalEdge_005b346c =
            g_stWipeSourceViewport_005b3488.right -
            g_stWipeSourceViewport_005b3488.left + 1;
        g_nWipePreviousVerticalEdge_005b3470 =
            g_stWipeSourceViewport_005b3488.bottom -
            g_stWipeSourceViewport_005b3488.top + 1;
        g_nWipeHorizontalThreshold_005b347c = g_nWipeWidth_005b3448 + 1;
        g_nWipeVerticalThreshold_005b34bc = g_nWipeHeight_005b344c + 1;
        g_nWipeTotalSteps_005b3440 = g_nWipeWidth_005b3448 + 1;
        g_nWipeMaximumStep_005b34c4 =
            (short)(duration < 320 ? duration : 320);
        g_nWipeVerticalStep_005b34b8 = g_nWipeMaximumStep_005b34c4;
        g_nWipeHorizontalStep_005b3478 = g_nWipeVerticalStep_005b34b8;
        break;
    case 7:
        g_nWipePreviousHorizontalEdge_005b346c = -1;
        g_nWipePreviousVerticalEdge_005b3470 =
            g_stWipeSourceViewport_005b3488.bottom -
            g_stWipeSourceViewport_005b3488.top + 1;
        g_nWipeHorizontalThreshold_005b347c = g_nWipeWidth_005b3448 + 1;
        g_nWipeVerticalThreshold_005b34bc = g_nWipeHeight_005b344c + 1;
        g_nWipeTotalSteps_005b3440 = g_nWipeWidth_005b3448 + 1;
        g_nWipeMaximumStep_005b34c4 =
            (short)(duration < 320 ? duration : 320);
        g_nWipeVerticalStep_005b34b8 = g_nWipeMaximumStep_005b34c4;
        g_nWipeHorizontalStep_005b3478 = g_nWipeVerticalStep_005b34b8;
        break;
    case 8:
        g_nWipeRadialAccumulator_005b3100 = 0;
        g_nWipeRadialRowCount_005d17b4 = g_nWipeHeight_005b344c / 2 + 1;
        g_pWipeLeadingEdges_005b3438 =
            g_asWipeRadialLeadingEdges_005b3108;
        g_pWipeTrailingEdges_005b343c =
            g_asWipeRadialTrailingEdges_005b3298;
        radialWidth = g_nWipeWidth_005b3448;
        radialHeight = g_nWipeHeight_005b344c;
        g_nWipeTotalSteps_005b3440 = (short)(
            sqrt((double)(radialHeight * radialHeight +
                          radialWidth * radialWidth)) /
            2.0 + 1.0);
        g_nWipeRadialThreshold_005b30fc = g_nWipeTotalSteps_005b3440 + 2;
        g_nWipeMaximumStep_005b34c4 = (short)(
            g_nWipeTotalSteps_005b3440 + 2 < duration
                ? g_nWipeTotalSteps_005b3440 + 2
                : duration);
        g_nWipeRadialStep_005b30f8 = g_nWipeMaximumStep_005b34c4;
        for (edge = 0; edge < 200; edge++) {
            g_pWipeLeadingEdges_005b3438[edge] = -1;
            g_pWipeTrailingEdges_005b343c[edge] = -1;
        }
        break;
    case 9:
        g_nWipeRadialAccumulator_005b3100 = 0;
        g_nWipeRadialRowCount_005d17b4 = g_nWipeHeight_005b344c / 2 + 1;
        g_pWipeLeadingEdges_005b3438 =
            g_asWipeRadialLeadingEdges_005b3108;
        g_pWipeTrailingEdges_005b343c =
            g_asWipeRadialTrailingEdges_005b3298;
        radialWidth = g_nWipeWidth_005b3448;
        radialHeight = g_nWipeHeight_005b344c;
        g_nWipeTotalSteps_005b3440 = (short)(
            sqrt((double)(radialHeight * radialHeight +
                          radialWidth * radialWidth)) /
            2.0 + 1.0);
        g_nWipeRadialThreshold_005b30fc = g_nWipeTotalSteps_005b3440 + 1;
        g_nWipeMaximumStep_005b34c4 = (short)(
            g_nWipeTotalSteps_005b3440 + 1 < duration
                ? g_nWipeTotalSteps_005b3440 + 1
                : duration);
        g_nWipeRadialStep_005b30f8 = g_nWipeMaximumStep_005b34c4;
        for (edge = 0; edge < g_nWipeRadialRowCount_005d17b4; edge++) {
            g_pWipeLeadingEdges_005b3438[edge] = 300;
            g_pWipeTrailingEdges_005b343c[edge] = 300;
        }
        break;
    }

    g_nWipeHorizontalAccumulator_005b3480 =
        g_nWipeHorizontalStep_005b3478 >> 2;
    g_nWipeVerticalAccumulator_005b34c0 =
        g_nWipeVerticalStep_005b34b8 >> 2;
    g_nWipeRadialAccumulator_005b3100 =
        g_nWipeRadialStep_005b30f8 >> 2;
    (void)workspace;
    return 1;
}

/* Function start: 0x448D1A */
void SetViewportHorizontalBounds(Viewport *viewport, int left, int right)
{
    viewport->left = (short)left;
    viewport->right = (short)right;
}

/* Function start: 0x448D39 */
void SetViewportVerticalBounds(Viewport *viewport, int top, int bottom)
{
    viewport->top = (short)top;
    viewport->bottom = (short)bottom;
}

/* Function start: 0x448D58 */
signed char AdvanceViewportWipe(void *workspace)
{
    short verticalCount;
    short horizontalCount;
    short *edgeSwap;

    switch (g_nWipeType_005b3444) {
    case 0:
        verticalCount = 0;
        while (g_nWipeVerticalThreshold_005b34bc >
                   g_nWipeVerticalAccumulator_005b34c0 &&
               g_nWipeTotalSteps_005b3440 > g_nWipeProgress_005b3434) {
            g_nWipeVerticalAccumulator_005b34c0 +=
                g_nWipeVerticalStep_005b34b8;
            verticalCount++;
            g_nWipeProgress_005b3434++;
        }
        g_nWipeVerticalAccumulator_005b34c0 -=
            g_nWipeVerticalThreshold_005b34bc;
        if ((g_nWipePassFlags_005b34b4 & 1) != 0) {
            SetViewportVerticalBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                g_stWipeWorkingSourceViewport_005b34c8.bottom + 1,
                g_stWipeWorkingSourceViewport_005b34c8.bottom +
                    verticalCount);
            SetViewportVerticalBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                g_stWipeWorkingDestinationViewport_005b34a0.bottom + 1,
                g_stWipeWorkingDestinationViewport_005b34a0.bottom +
                    verticalCount);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
        } else {
            OffsetViewportBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                0, 1, 0, verticalCount);
            OffsetViewportBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                0, 1, 0, verticalCount);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
            OffsetViewportBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                0, -1, 0, 0);
            OffsetViewportBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                0, -1, 0, 0);
        }
        break;
    case 1:
        horizontalCount = 0;
        while (g_nWipeHorizontalThreshold_005b347c >
                   g_nWipeHorizontalAccumulator_005b3480 &&
               g_nWipeTotalSteps_005b3440 > g_nWipeProgress_005b3434) {
            g_nWipeHorizontalAccumulator_005b3480 +=
                g_nWipeHorizontalStep_005b3478;
            horizontalCount++;
            g_nWipeProgress_005b3434++;
        }
        g_nWipeHorizontalAccumulator_005b3480 -=
            g_nWipeHorizontalThreshold_005b347c;
        if ((g_nWipePassFlags_005b34b4 & 1) != 0) {
            SetViewportHorizontalBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                g_stWipeWorkingSourceViewport_005b34c8.left -
                    horizontalCount,
                g_stWipeWorkingSourceViewport_005b34c8.left - 1);
            SetViewportHorizontalBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                g_stWipeWorkingDestinationViewport_005b34a0.left -
                    horizontalCount,
                g_stWipeWorkingDestinationViewport_005b34a0.left - 1);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
        } else {
            OffsetViewportBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                -horizontalCount, 0, -1, 0);
            OffsetViewportBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                -horizontalCount, 0, -1, 0);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
            OffsetViewportBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                0, 0, 1, 0);
            OffsetViewportBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                0, 0, 1, 0);
        }
        break;
    case 2:
        verticalCount = 0;
        while (g_nWipeVerticalThreshold_005b34bc >
                   g_nWipeVerticalAccumulator_005b34c0 &&
               g_nWipeTotalSteps_005b3440 > g_nWipeProgress_005b3434) {
            g_nWipeVerticalAccumulator_005b34c0 +=
                g_nWipeVerticalStep_005b34b8;
            verticalCount++;
            g_nWipeProgress_005b3434++;
        }
        g_nWipeVerticalAccumulator_005b34c0 -=
            g_nWipeVerticalThreshold_005b34bc;
        if ((g_nWipePassFlags_005b34b4 & 1) != 0) {
            SetViewportVerticalBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                g_stWipeWorkingSourceViewport_005b34c8.top - verticalCount,
                g_stWipeWorkingSourceViewport_005b34c8.top - 1);
            SetViewportVerticalBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                g_stWipeWorkingDestinationViewport_005b34a0.top -
                    verticalCount,
                g_stWipeWorkingDestinationViewport_005b34a0.top - 1);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
        } else {
            OffsetViewportBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                0, -verticalCount, 0, -1);
            OffsetViewportBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                0, -verticalCount, 0, -1);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
            OffsetViewportBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                0, 0, 0, 1);
            OffsetViewportBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                0, 0, 0, 1);
        }
        break;
    case 3:
        horizontalCount = 0;
        while (g_nWipeHorizontalThreshold_005b347c >
                   g_nWipeHorizontalAccumulator_005b3480 &&
               g_nWipeTotalSteps_005b3440 > g_nWipeProgress_005b3434) {
            g_nWipeHorizontalAccumulator_005b3480 +=
                g_nWipeHorizontalStep_005b3478;
            horizontalCount++;
            g_nWipeProgress_005b3434++;
        }
        g_nWipeHorizontalAccumulator_005b3480 -=
            g_nWipeHorizontalThreshold_005b347c;
        if ((g_nWipePassFlags_005b34b4 & 1) != 0) {
            SetViewportHorizontalBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                g_stWipeWorkingSourceViewport_005b34c8.right + 1,
                g_stWipeWorkingSourceViewport_005b34c8.right +
                    horizontalCount);
            SetViewportHorizontalBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                g_stWipeWorkingDestinationViewport_005b34a0.right + 1,
                g_stWipeWorkingDestinationViewport_005b34a0.right +
                    horizontalCount);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
        } else {
            OffsetViewportBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                1, 0, horizontalCount, 0);
            OffsetViewportBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                1, 0, horizontalCount, 0);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
            OffsetViewportBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                -1, 0, 0, 0);
            OffsetViewportBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                -1, 0, 0, 0);
        }
        break;
    case 4:
        verticalCount = 0;
        horizontalCount = verticalCount;
        while (g_nWipeHorizontalThreshold_005b347c >
                   g_nWipeHorizontalAccumulator_005b3480 &&
               g_nWipeTotalSteps_005b3440 > g_nWipeProgress_005b3434) {
            g_nWipeHorizontalAccumulator_005b3480 +=
                g_nWipeHorizontalStep_005b3478;
            horizontalCount++;
            g_nWipeProgress_005b3434++;
        }
        g_nWipeHorizontalAccumulator_005b3480 -=
            g_nWipeHorizontalThreshold_005b347c;
        while (g_nWipeVerticalThreshold_005b34bc >
               g_nWipeVerticalAccumulator_005b34c0) {
            verticalCount++;
            g_nWipeVerticalAccumulator_005b34c0 +=
                g_nWipeVerticalStep_005b34b8;
        }
        g_nWipeVerticalAccumulator_005b34c0 -=
            g_nWipeVerticalThreshold_005b34bc;
        g_nWipeCurrentHorizontalEdge_005b3464 =
            horizontalCount + g_nWipePreviousHorizontalEdge_005b346c;
        g_nWipeCurrentVerticalEdge_005b3468 =
            verticalCount + g_nWipePreviousVerticalEdge_005b3470;
        if ((g_nWipePassFlags_005b34b4 & 1) != 0) {
            if (g_nWipePreviousVerticalEdge_005b3470 >= 0 &&
                horizontalCount != 0) {
                SetViewportRectangleBounds(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    g_stWipeSourceViewport_005b3488.left +
                        g_nWipePreviousHorizontalEdge_005b346c + 1,
                    g_stWipeSourceViewport_005b3488.top,
                    g_stWipeSourceViewport_005b3488.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeSourceViewport_005b3488.top +
                        g_nWipePreviousVerticalEdge_005b3470);
                SetViewportRectangleBounds(
                    &g_stWipeWorkingDestinationViewport_005b34a0,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipePreviousHorizontalEdge_005b346c + 1,
                    g_stWipeDestinationViewport_005b3450.top,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeDestinationViewport_005b3450.top +
                        g_nWipePreviousVerticalEdge_005b3470);
                CopyViewportContents(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    &g_stWipeWorkingDestinationViewport_005b34a0);
            }
            if (verticalCount != 0) {
                SetViewportRectangleBounds(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    g_stWipeSourceViewport_005b3488.left,
                    g_stWipeSourceViewport_005b3488.top +
                        g_nWipePreviousVerticalEdge_005b3470 + 1,
                    g_stWipeSourceViewport_005b3488.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeSourceViewport_005b3488.top +
                        g_nWipeCurrentVerticalEdge_005b3468);
                SetViewportRectangleBounds(
                    &g_stWipeWorkingDestinationViewport_005b34a0,
                    g_stWipeDestinationViewport_005b3450.left,
                    g_stWipeDestinationViewport_005b3450.top +
                        g_nWipePreviousVerticalEdge_005b3470 + 1,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeDestinationViewport_005b3450.top +
                        g_nWipeCurrentVerticalEdge_005b3468);
                CopyViewportContents(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    &g_stWipeWorkingDestinationViewport_005b34a0);
            }
        } else {
            SetViewportRectangleBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                g_stWipeSourceViewport_005b3488.left,
                g_stWipeSourceViewport_005b3488.top,
                g_stWipeSourceViewport_005b3488.left +
                    g_nWipeCurrentHorizontalEdge_005b3464,
                g_stWipeSourceViewport_005b3488.top +
                    g_nWipeCurrentVerticalEdge_005b3468);
            SetViewportRectangleBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                g_stWipeDestinationViewport_005b3450.left,
                g_stWipeDestinationViewport_005b3450.top,
                g_stWipeDestinationViewport_005b3450.left +
                    g_nWipeCurrentHorizontalEdge_005b3464,
                g_stWipeDestinationViewport_005b3450.top +
                    g_nWipeCurrentVerticalEdge_005b3468);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
        }
        break;
    case 5:
        verticalCount = 0;
        horizontalCount = verticalCount;
        while (g_nWipeHorizontalThreshold_005b347c >
                   g_nWipeHorizontalAccumulator_005b3480 &&
               g_nWipeTotalSteps_005b3440 > g_nWipeProgress_005b3434) {
            g_nWipeHorizontalAccumulator_005b3480 +=
                g_nWipeHorizontalStep_005b3478;
            horizontalCount++;
            g_nWipeProgress_005b3434++;
        }
        g_nWipeHorizontalAccumulator_005b3480 -=
            g_nWipeHorizontalThreshold_005b347c;
        while (g_nWipeVerticalThreshold_005b34bc >
               g_nWipeVerticalAccumulator_005b34c0) {
            verticalCount++;
            g_nWipeVerticalAccumulator_005b34c0 +=
                g_nWipeVerticalStep_005b34b8;
        }
        g_nWipeVerticalAccumulator_005b34c0 -=
            g_nWipeVerticalThreshold_005b34bc;
        g_nWipeCurrentHorizontalEdge_005b3464 =
            g_nWipePreviousHorizontalEdge_005b346c - horizontalCount;
        g_nWipeCurrentVerticalEdge_005b3468 =
            verticalCount + g_nWipePreviousVerticalEdge_005b3470;
        if ((g_nWipePassFlags_005b34b4 & 1) != 0) {
            if (g_nWipePreviousVerticalEdge_005b3470 >= 0 &&
                horizontalCount != 0) {
                SetViewportRectangleBounds(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    g_stWipeSourceViewport_005b3488.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeSourceViewport_005b3488.top,
                    g_stWipeSourceViewport_005b3488.left +
                        g_nWipePreviousHorizontalEdge_005b346c - 1,
                    g_stWipeSourceViewport_005b3488.top +
                        g_nWipePreviousVerticalEdge_005b3470);
                SetViewportRectangleBounds(
                    &g_stWipeWorkingDestinationViewport_005b34a0,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeDestinationViewport_005b3450.top,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipePreviousHorizontalEdge_005b346c - 1,
                    g_stWipeDestinationViewport_005b3450.top +
                        g_nWipePreviousVerticalEdge_005b3470);
                CopyViewportContents(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    &g_stWipeWorkingDestinationViewport_005b34a0);
            }
            if (verticalCount != 0) {
                SetViewportRectangleBounds(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    g_stWipeSourceViewport_005b3488.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeSourceViewport_005b3488.top +
                        g_nWipePreviousVerticalEdge_005b3470 + 1,
                    g_stWipeSourceViewport_005b3488.right,
                    g_stWipeSourceViewport_005b3488.top +
                        g_nWipeCurrentVerticalEdge_005b3468);
                SetViewportRectangleBounds(
                    &g_stWipeWorkingDestinationViewport_005b34a0,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeDestinationViewport_005b3450.top +
                        g_nWipePreviousVerticalEdge_005b3470 + 1,
                    g_stWipeDestinationViewport_005b3450.right,
                    g_stWipeDestinationViewport_005b3450.top +
                        g_nWipeCurrentVerticalEdge_005b3468);
                CopyViewportContents(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    &g_stWipeWorkingDestinationViewport_005b34a0);
            }
        } else {
            SetViewportRectangleBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                g_stWipeSourceViewport_005b3488.left +
                    g_nWipeCurrentHorizontalEdge_005b3464,
                g_stWipeSourceViewport_005b3488.top,
                g_stWipeSourceViewport_005b3488.right,
                g_stWipeSourceViewport_005b3488.top +
                    g_nWipeCurrentVerticalEdge_005b3468);
            SetViewportRectangleBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                g_stWipeDestinationViewport_005b3450.left +
                    g_nWipeCurrentHorizontalEdge_005b3464,
                g_stWipeDestinationViewport_005b3450.top,
                g_stWipeDestinationViewport_005b3450.right,
                g_stWipeDestinationViewport_005b3450.top +
                    g_nWipeCurrentVerticalEdge_005b3468);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
        }
        break;
    case 6:
        verticalCount = 0;
        horizontalCount = verticalCount;
        while (g_nWipeHorizontalThreshold_005b347c >
                   g_nWipeHorizontalAccumulator_005b3480 &&
               g_nWipeTotalSteps_005b3440 > g_nWipeProgress_005b3434) {
            g_nWipeHorizontalAccumulator_005b3480 +=
                g_nWipeHorizontalStep_005b3478;
            horizontalCount++;
            g_nWipeProgress_005b3434++;
        }
        g_nWipeHorizontalAccumulator_005b3480 -=
            g_nWipeHorizontalThreshold_005b347c;
        while (g_nWipeVerticalThreshold_005b34bc >
               g_nWipeVerticalAccumulator_005b34c0) {
            verticalCount++;
            g_nWipeVerticalAccumulator_005b34c0 +=
                g_nWipeVerticalStep_005b34b8;
        }
        g_nWipeVerticalAccumulator_005b34c0 -=
            g_nWipeVerticalThreshold_005b34bc;
        g_nWipeCurrentHorizontalEdge_005b3464 =
            g_nWipePreviousHorizontalEdge_005b346c - horizontalCount;
        g_nWipeCurrentVerticalEdge_005b3468 =
            g_nWipePreviousVerticalEdge_005b3470 - verticalCount;
        if ((g_nWipePassFlags_005b34b4 & 1) != 0) {
            if (g_nWipePreviousVerticalEdge_005b3470 <=
                    g_stWipeSourceViewport_005b3488.bottom -
                        g_stWipeSourceViewport_005b3488.top &&
                horizontalCount != 0) {
                SetViewportRectangleBounds(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    g_stWipeSourceViewport_005b3488.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeSourceViewport_005b3488.top +
                        g_nWipePreviousVerticalEdge_005b3470,
                    g_stWipeSourceViewport_005b3488.left +
                        g_nWipePreviousHorizontalEdge_005b346c - 1,
                    g_stWipeSourceViewport_005b3488.bottom);
                SetViewportRectangleBounds(
                    &g_stWipeWorkingDestinationViewport_005b34a0,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeDestinationViewport_005b3450.top +
                        g_nWipePreviousVerticalEdge_005b3470,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipePreviousHorizontalEdge_005b346c - 1,
                    g_stWipeDestinationViewport_005b3450.bottom);
                CopyViewportContents(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    &g_stWipeWorkingDestinationViewport_005b34a0);
            }
            if (verticalCount != 0) {
                SetViewportRectangleBounds(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    g_stWipeSourceViewport_005b3488.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeSourceViewport_005b3488.top +
                        g_nWipeCurrentVerticalEdge_005b3468,
                    g_stWipeSourceViewport_005b3488.right,
                    g_stWipeSourceViewport_005b3488.top +
                        g_nWipePreviousVerticalEdge_005b3470 - 1);
                SetViewportRectangleBounds(
                    &g_stWipeWorkingDestinationViewport_005b34a0,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeDestinationViewport_005b3450.top +
                        g_nWipeCurrentVerticalEdge_005b3468,
                    g_stWipeDestinationViewport_005b3450.right,
                    g_stWipeDestinationViewport_005b3450.top +
                        g_nWipePreviousVerticalEdge_005b3470 - 1);
                CopyViewportContents(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    &g_stWipeWorkingDestinationViewport_005b34a0);
            }
        } else {
            SetViewportRectangleBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                g_stWipeSourceViewport_005b3488.left +
                    g_nWipeCurrentHorizontalEdge_005b3464,
                g_stWipeSourceViewport_005b3488.top +
                    g_nWipeCurrentVerticalEdge_005b3468,
                g_stWipeSourceViewport_005b3488.right,
                g_stWipeSourceViewport_005b3488.bottom);
            SetViewportRectangleBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                g_stWipeDestinationViewport_005b3450.left +
                    g_nWipeCurrentHorizontalEdge_005b3464,
                g_stWipeDestinationViewport_005b3450.top +
                    g_nWipeCurrentVerticalEdge_005b3468,
                g_stWipeDestinationViewport_005b3450.right,
                g_stWipeDestinationViewport_005b3450.bottom);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
        }
        break;
    case 7:
        verticalCount = 0;
        horizontalCount = verticalCount;
        while (g_nWipeHorizontalThreshold_005b347c >
                   g_nWipeHorizontalAccumulator_005b3480 &&
               g_nWipeTotalSteps_005b3440 > g_nWipeProgress_005b3434) {
            g_nWipeHorizontalAccumulator_005b3480 +=
                g_nWipeHorizontalStep_005b3478;
            horizontalCount++;
            g_nWipeProgress_005b3434++;
        }
        g_nWipeHorizontalAccumulator_005b3480 -=
            g_nWipeHorizontalThreshold_005b347c;
        while (g_nWipeVerticalThreshold_005b34bc >
               g_nWipeVerticalAccumulator_005b34c0) {
            verticalCount++;
            g_nWipeVerticalAccumulator_005b34c0 +=
                g_nWipeVerticalStep_005b34b8;
        }
        g_nWipeVerticalAccumulator_005b34c0 -=
            g_nWipeVerticalThreshold_005b34bc;
        g_nWipeCurrentHorizontalEdge_005b3464 =
            horizontalCount + g_nWipePreviousHorizontalEdge_005b346c;
        g_nWipeCurrentVerticalEdge_005b3468 =
            g_nWipePreviousVerticalEdge_005b3470 - verticalCount;
        if ((g_nWipePassFlags_005b34b4 & 1) != 0) {
            if (g_nWipePreviousVerticalEdge_005b3470 <=
                    g_stWipeSourceViewport_005b3488.bottom -
                        g_stWipeSourceViewport_005b3488.top &&
                horizontalCount != 0) {
                SetViewportRectangleBounds(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    g_stWipeSourceViewport_005b3488.left +
                        g_nWipePreviousHorizontalEdge_005b346c + 1,
                    g_stWipeSourceViewport_005b3488.top +
                        g_nWipePreviousVerticalEdge_005b3470,
                    g_stWipeSourceViewport_005b3488.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeSourceViewport_005b3488.bottom);
                SetViewportRectangleBounds(
                    &g_stWipeWorkingDestinationViewport_005b34a0,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipePreviousHorizontalEdge_005b346c + 1,
                    g_stWipeDestinationViewport_005b3450.top +
                        g_nWipePreviousVerticalEdge_005b3470,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeDestinationViewport_005b3450.bottom);
                CopyViewportContents(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    &g_stWipeWorkingDestinationViewport_005b34a0);
            }
            if (verticalCount != 0) {
                SetViewportRectangleBounds(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    g_stWipeSourceViewport_005b3488.left,
                    g_stWipeSourceViewport_005b3488.top +
                        g_nWipeCurrentVerticalEdge_005b3468,
                    g_stWipeSourceViewport_005b3488.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeSourceViewport_005b3488.top +
                        g_nWipePreviousVerticalEdge_005b3470 - 1);
                SetViewportRectangleBounds(
                    &g_stWipeWorkingDestinationViewport_005b34a0,
                    g_stWipeDestinationViewport_005b3450.left,
                    g_stWipeDestinationViewport_005b3450.top +
                        g_nWipeCurrentVerticalEdge_005b3468,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeDestinationViewport_005b3450.top +
                        g_nWipePreviousVerticalEdge_005b3470 - 1);
                CopyViewportContents(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    &g_stWipeWorkingDestinationViewport_005b34a0);
            }
        } else {
            SetViewportRectangleBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                g_stWipeSourceViewport_005b3488.left,
                g_stWipeSourceViewport_005b3488.top +
                    g_nWipeCurrentVerticalEdge_005b3468,
                g_stWipeSourceViewport_005b3488.left +
                    g_nWipeCurrentHorizontalEdge_005b3464,
                g_stWipeSourceViewport_005b3488.bottom);
            SetViewportRectangleBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                g_stWipeDestinationViewport_005b3450.left,
                g_stWipeDestinationViewport_005b3450.top +
                    g_nWipeCurrentVerticalEdge_005b3468,
                g_stWipeDestinationViewport_005b3450.left +
                    g_nWipeCurrentHorizontalEdge_005b3464,
                g_stWipeDestinationViewport_005b3450.bottom);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
        }
        break;
    case 8:
        while (g_nWipeRadialThreshold_005b30fc >
                   g_nWipeRadialAccumulator_005b3100 &&
               g_nWipeTotalSteps_005b3440 > g_nWipeProgress_005b3434) {
            g_nWipeRadialAccumulator_005b3100 +=
                g_nWipeRadialStep_005b30f8;
            g_nWipeProgress_005b3434++;
        }
        g_nWipeRadialAccumulator_005b3100 -=
            g_nWipeRadialThreshold_005b30fc;
        GenerateWipeRadialEdges(g_pWipeLeadingEdges_005b3438,
                                g_nWipeProgress_005b3434);
        BlitWipeRadialBands(
            &g_stWipeWorkingSourceViewport_005b34c8,
            &g_stWipeWorkingDestinationViewport_005b34a0,
            g_pWipeTrailingEdges_005b343c,
            g_pWipeLeadingEdges_005b3438);
        edgeSwap = g_pWipeTrailingEdges_005b343c;
        if ((g_nWipePassFlags_005b34b4 & 1) != 0) {
            g_pWipeTrailingEdges_005b343c = g_pWipeLeadingEdges_005b3438;
            g_pWipeLeadingEdges_005b3438 = edgeSwap;
        }
        break;
    case 9:
        while (g_nWipeRadialThreshold_005b30fc >
                   g_nWipeRadialAccumulator_005b3100 &&
               g_nWipeTotalSteps_005b3440 > g_nWipeProgress_005b3434) {
            g_nWipeRadialAccumulator_005b3100 +=
                g_nWipeRadialStep_005b30f8;
            g_nWipeProgress_005b3434++;
        }
        g_nWipeRadialAccumulator_005b3100 -=
            g_nWipeRadialThreshold_005b30fc;
        GenerateWipeRadialEdges(
            g_pWipeTrailingEdges_005b343c,
            g_nWipeTotalSteps_005b3440 - g_nWipeProgress_005b3434);
        BlitWipeRadialBands(
            &g_stWipeWorkingSourceViewport_005b34c8,
            &g_stWipeWorkingDestinationViewport_005b34a0,
            g_pWipeTrailingEdges_005b343c,
            g_pWipeLeadingEdges_005b3438);
        edgeSwap = g_pWipeTrailingEdges_005b343c;
        if ((g_nWipePassFlags_005b34b4 & 1) != 0) {
            g_pWipeTrailingEdges_005b343c = g_pWipeLeadingEdges_005b3438;
            g_pWipeLeadingEdges_005b3438 = edgeSwap;
        }
        break;
    }

    g_nWipePreviousHorizontalEdge_005b346c =
        g_nWipeCurrentHorizontalEdge_005b3464;
    g_nWipePreviousVerticalEdge_005b3470 =
        g_nWipeCurrentVerticalEdge_005b3468;
    (void)workspace;
    return g_nWipeTotalSteps_005b3440 == g_nWipeProgress_005b3434;
}

/* Function start: 0x449FAE */
void OffsetViewportBounds(Viewport *viewport, int left,
                          int top, int right, int bottom)
{
    viewport->left += (short)left;
    viewport->top += (short)top;
    viewport->right += (short)right;
    viewport->bottom += (short)bottom;
}

/* Function start: 0x44A009 */
void SetViewportRectangleBounds(Viewport *viewport, int left, int top,
                       int right, int bottom)
{
    viewport->left = (short)left;
    viewport->top = (short)top;
    viewport->right = (short)right;
    viewport->bottom = (short)bottom;
}

/* Function start: 0x44A03C */
void FinishViewportWipe(void *workspace)
{
    short remaining;

    if (g_nWipePassFlags_005b34b4 == 0) {
        CopyViewportContents(&g_stWipeSourceViewport_005b3488,
                             &g_stWipeDestinationViewport_005b3450);
    } else {
        switch (g_nWipeType_005b3444) {
        case 0:
            remaining = g_nWipeTotalSteps_005b3440 -
                g_nWipeProgress_005b3434;
            SetViewportVerticalBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                g_stWipeWorkingSourceViewport_005b34c8.bottom + 1,
                g_stWipeWorkingSourceViewport_005b34c8.bottom + remaining);
            SetViewportVerticalBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                g_stWipeWorkingDestinationViewport_005b34a0.bottom + 1,
                g_stWipeWorkingDestinationViewport_005b34a0.bottom +
                    remaining);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
            break;
        case 1:
            remaining = g_nWipeTotalSteps_005b3440 -
                g_nWipeProgress_005b3434;
            SetViewportHorizontalBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                g_stWipeWorkingSourceViewport_005b34c8.left - remaining,
                g_stWipeWorkingSourceViewport_005b34c8.left - 1);
            SetViewportHorizontalBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                g_stWipeWorkingDestinationViewport_005b34a0.left - remaining,
                g_stWipeWorkingDestinationViewport_005b34a0.left - 1);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
            break;
        case 2:
            remaining = g_nWipeTotalSteps_005b3440 -
                g_nWipeProgress_005b3434;
            SetViewportVerticalBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                g_stWipeWorkingSourceViewport_005b34c8.top - remaining,
                g_stWipeWorkingSourceViewport_005b34c8.top - 1);
            SetViewportVerticalBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                g_stWipeWorkingDestinationViewport_005b34a0.top - remaining,
                g_stWipeWorkingDestinationViewport_005b34a0.top - 1);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
            break;
        case 3:
            remaining = g_nWipeTotalSteps_005b3440 -
                g_nWipeProgress_005b3434;
            SetViewportHorizontalBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                g_stWipeWorkingSourceViewport_005b34c8.right + 1,
                g_stWipeWorkingSourceViewport_005b34c8.right + remaining);
            SetViewportHorizontalBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                g_stWipeWorkingDestinationViewport_005b34a0.right + 1,
                g_stWipeWorkingDestinationViewport_005b34a0.right + remaining);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
            break;
        case 4:
            g_nWipeCurrentHorizontalEdge_005b3464 =
                g_stWipeSourceViewport_005b3488.right -
                g_stWipeSourceViewport_005b3488.left;
            g_nWipeCurrentVerticalEdge_005b3468 =
                g_stWipeSourceViewport_005b3488.bottom -
                g_stWipeSourceViewport_005b3488.top;
            if (g_nWipePreviousVerticalEdge_005b3470 >= 0) {
                SetViewportRectangleBounds(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    g_stWipeSourceViewport_005b3488.left +
                        g_nWipePreviousHorizontalEdge_005b346c + 1,
                    g_stWipeSourceViewport_005b3488.top,
                    g_stWipeSourceViewport_005b3488.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeSourceViewport_005b3488.top +
                        g_nWipePreviousVerticalEdge_005b3470);
                SetViewportRectangleBounds(
                    &g_stWipeWorkingDestinationViewport_005b34a0,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipePreviousHorizontalEdge_005b346c + 1,
                    g_stWipeDestinationViewport_005b3450.top,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeDestinationViewport_005b3450.top +
                        g_nWipePreviousVerticalEdge_005b3470);
                CopyViewportContents(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    &g_stWipeWorkingDestinationViewport_005b34a0);
            }
            SetViewportRectangleBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                g_stWipeSourceViewport_005b3488.left,
                g_stWipeSourceViewport_005b3488.top +
                    g_nWipePreviousVerticalEdge_005b3470 + 1,
                g_stWipeSourceViewport_005b3488.left +
                    g_nWipeCurrentHorizontalEdge_005b3464,
                g_stWipeSourceViewport_005b3488.top +
                    g_nWipeCurrentVerticalEdge_005b3468);
            SetViewportRectangleBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                g_stWipeDestinationViewport_005b3450.left,
                g_stWipeDestinationViewport_005b3450.top +
                    g_nWipePreviousVerticalEdge_005b3470 + 1,
                g_stWipeDestinationViewport_005b3450.left +
                    g_nWipeCurrentHorizontalEdge_005b3464,
                g_stWipeDestinationViewport_005b3450.top +
                    g_nWipeCurrentVerticalEdge_005b3468);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
            break;
        case 5:
            g_nWipeCurrentHorizontalEdge_005b3464 = 0;
            g_nWipeCurrentVerticalEdge_005b3468 =
                g_stWipeSourceViewport_005b3488.bottom -
                g_stWipeSourceViewport_005b3488.top;
            if (g_nWipePreviousVerticalEdge_005b3470 >= 0) {
                SetViewportRectangleBounds(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    g_stWipeSourceViewport_005b3488.left,
                    g_stWipeSourceViewport_005b3488.top,
                    g_stWipeSourceViewport_005b3488.left +
                        g_nWipePreviousHorizontalEdge_005b346c - 1,
                    g_stWipeSourceViewport_005b3488.top +
                        g_nWipePreviousVerticalEdge_005b3470);
                SetViewportRectangleBounds(
                    &g_stWipeWorkingDestinationViewport_005b34a0,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeDestinationViewport_005b3450.top,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipePreviousHorizontalEdge_005b346c - 1,
                    g_stWipeDestinationViewport_005b3450.top +
                        g_nWipePreviousVerticalEdge_005b3470);
                CopyViewportContents(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    &g_stWipeWorkingDestinationViewport_005b34a0);
            }
            SetViewportRectangleBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                g_stWipeSourceViewport_005b3488.left +
                    g_nWipeCurrentHorizontalEdge_005b3464,
                g_stWipeSourceViewport_005b3488.top +
                    g_nWipePreviousVerticalEdge_005b3470 + 1,
                g_stWipeSourceViewport_005b3488.right,
                g_stWipeSourceViewport_005b3488.top +
                    g_nWipeCurrentVerticalEdge_005b3468);
            SetViewportRectangleBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                g_stWipeDestinationViewport_005b3450.left +
                    g_nWipeCurrentHorizontalEdge_005b3464,
                g_stWipeDestinationViewport_005b3450.top +
                    g_nWipePreviousVerticalEdge_005b3470 + 1,
                g_stWipeDestinationViewport_005b3450.right,
                g_stWipeDestinationViewport_005b3450.top +
                    g_nWipeCurrentVerticalEdge_005b3468);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
            break;
        case 6:
            g_nWipeCurrentHorizontalEdge_005b3464 = 0;
            g_nWipeCurrentVerticalEdge_005b3468 = 0;
            if (g_nWipePreviousVerticalEdge_005b3470 >= 0) {
                SetViewportRectangleBounds(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    g_stWipeSourceViewport_005b3488.left,
                    g_stWipeSourceViewport_005b3488.top +
                        g_nWipePreviousVerticalEdge_005b3470,
                    g_stWipeSourceViewport_005b3488.left +
                        g_nWipePreviousHorizontalEdge_005b346c - 1,
                    g_stWipeSourceViewport_005b3488.bottom);
                SetViewportRectangleBounds(
                    &g_stWipeWorkingDestinationViewport_005b34a0,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeDestinationViewport_005b3450.top +
                        g_nWipePreviousVerticalEdge_005b3470,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipePreviousHorizontalEdge_005b346c - 1,
                    g_stWipeDestinationViewport_005b3450.bottom);
                CopyViewportContents(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    &g_stWipeWorkingDestinationViewport_005b34a0);
            }
            SetViewportRectangleBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                g_stWipeSourceViewport_005b3488.left +
                    g_nWipeCurrentHorizontalEdge_005b3464,
                g_stWipeSourceViewport_005b3488.top +
                    g_nWipeCurrentVerticalEdge_005b3468,
                g_stWipeSourceViewport_005b3488.right,
                g_stWipeSourceViewport_005b3488.top +
                    g_nWipePreviousVerticalEdge_005b3470 + 1);
            SetViewportRectangleBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                g_stWipeDestinationViewport_005b3450.left +
                    g_nWipeCurrentHorizontalEdge_005b3464,
                g_stWipeDestinationViewport_005b3450.top +
                    g_nWipeCurrentVerticalEdge_005b3468,
                g_stWipeDestinationViewport_005b3450.right,
                g_stWipeDestinationViewport_005b3450.top +
                    g_nWipePreviousVerticalEdge_005b3470 + 1);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
            break;
        case 7:
            g_nWipeCurrentHorizontalEdge_005b3464 =
                g_stWipeSourceViewport_005b3488.right -
                g_stWipeSourceViewport_005b3488.left;
            g_nWipeCurrentVerticalEdge_005b3468 = 0;
            if (g_nWipePreviousVerticalEdge_005b3470 >= 0) {
                SetViewportRectangleBounds(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    g_stWipeSourceViewport_005b3488.left +
                        g_nWipePreviousHorizontalEdge_005b346c + 1,
                    g_stWipeSourceViewport_005b3488.top +
                        g_nWipePreviousVerticalEdge_005b3470,
                    g_stWipeSourceViewport_005b3488.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeSourceViewport_005b3488.bottom);
                SetViewportRectangleBounds(
                    &g_stWipeWorkingDestinationViewport_005b34a0,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipePreviousHorizontalEdge_005b346c + 1,
                    g_stWipeDestinationViewport_005b3450.top +
                        g_nWipePreviousVerticalEdge_005b3470,
                    g_stWipeDestinationViewport_005b3450.left +
                        g_nWipeCurrentHorizontalEdge_005b3464,
                    g_stWipeDestinationViewport_005b3450.bottom);
                CopyViewportContents(
                    &g_stWipeWorkingSourceViewport_005b34c8,
                    &g_stWipeWorkingDestinationViewport_005b34a0);
            }
            SetViewportRectangleBounds(
                &g_stWipeWorkingSourceViewport_005b34c8,
                g_stWipeSourceViewport_005b3488.left,
                g_stWipeSourceViewport_005b3488.top +
                    g_nWipeCurrentVerticalEdge_005b3468,
                g_stWipeSourceViewport_005b3488.left +
                    g_nWipeCurrentHorizontalEdge_005b3464,
                g_stWipeSourceViewport_005b3488.top +
                    g_nWipePreviousVerticalEdge_005b3470 + 1);
            SetViewportRectangleBounds(
                &g_stWipeWorkingDestinationViewport_005b34a0,
                g_stWipeDestinationViewport_005b3450.left,
                g_stWipeDestinationViewport_005b3450.top +
                    g_nWipeCurrentVerticalEdge_005b3468,
                g_stWipeDestinationViewport_005b3450.left +
                    g_nWipeCurrentHorizontalEdge_005b3464,
                g_stWipeDestinationViewport_005b3450.top +
                    g_nWipePreviousVerticalEdge_005b3470 + 1);
            CopyViewportContents(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0);
            break;
        case 8:
            g_nWipeProgress_005b3434 = g_nWipeTotalSteps_005b3440;
            GenerateWipeRadialEdges(g_pWipeLeadingEdges_005b3438,
                                    g_nWipeTotalSteps_005b3440);
            BlitWipeRadialBands(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0,
                g_pWipeTrailingEdges_005b343c,
                g_pWipeLeadingEdges_005b3438);
            break;
        case 9:
            g_nWipeProgress_005b3434 = g_nWipeTotalSteps_005b3440;
            GenerateWipeRadialEdges(
                g_pWipeTrailingEdges_005b343c,
                g_nWipeTotalSteps_005b3440 - g_nWipeTotalSteps_005b3440);
            BlitWipeRadialBands(
                &g_stWipeWorkingSourceViewport_005b34c8,
                &g_stWipeWorkingDestinationViewport_005b34a0,
                g_pWipeTrailingEdges_005b343c,
                g_pWipeLeadingEdges_005b3438);
            break;
        }
    }
    (void)workspace;
}

/* Function start: 0x44A910 */
void GenerateWipeRadialEdges(short *edges, int radius)
{
    short x;
    short y;
    short error;
    short previousY;
    short fillRadius;
    short decision;

    x = 0;
    y = (short)radius;
    error = (short)((1 - radius) * 2);
    if (g_nWipeRadialRowCount_005d17b4 > y)
        edges[y] = x;
    previousY = y;

    while (y > 0) {
        if (error < 0) {
            decision = (short)((error + y) * 2 - 1);
            if (decision > 0) {
                x++;
                y--;
                error += 2 + (x - y) * 2;
            } else {
                x++;
                error += x * 2 + 1;
            }
        } else if (error > 0) {
            decision = (short)((error - x) * 2 - 1);
            if (decision > 0) {
                y--;
                error += 1 - y * 2;
            } else {
                x++;
                y--;
                error += 2 + (x - y) * 2;
            }
        } else {
            x++;
            y--;
            error += 2 + (x - y) * 2;
        }
        if (previousY != y) {
            if (g_nWipeRadialRowCount_005d17b4 > y)
                edges[y] = x;
            previousY = y;
        }
    }

    if (radius == 0)
        edges[0] = -1;
    for (fillRadius = (short)(radius + 1);
         g_nWipeRadialRowCount_005d17b4 > fillRadius;
         fillRadius++) {
        edges[fillRadius] = -1;
    }
}

/* Function start: 0x44AAE5 */
void BlitWipeRadialBands(Viewport *source, Viewport *destination,
                         short *trailingEdges, short *leadingEdges)
{
    short sourceLeftCenter;
    unsigned short *destinationLowerRows;
    short rightHalfWidthMinusOne;
    short halfWidth;
    short sourceRightCenter;
    short destinationRightCenter;
    short height;
    short upperRowCount;
    short width;
    int leadingWidth;
    short upperMiddleOffset;
    int row;
    short *trailing;
    short *leading;
    short halfWidthMinusOne;
    unsigned short *sourceLowerRows;
    short lowerMiddleOffset;
    short copyWidth;
    short destinationLeftCenter;
    unsigned short *sourceUpperRows;
    unsigned short *destinationUpperRows;

    width = source->right - source->left + 1;
    halfWidth = (width + 1) >> 1;
    halfWidthMinusOne = halfWidth - 1;
    rightHalfWidthMinusOne = halfWidthMinusOne;
    height = source->bottom - source->top + 1;
    upperRowCount = (height + 1) >> 1;
    upperMiddleOffset = upperRowCount - 1;
    lowerMiddleOffset = height >> 1;
    sourceLeftCenter = source->left + halfWidthMinusOne;
    sourceRightCenter = source->left + rightHalfWidthMinusOne;
    sourceUpperRows = source->rowOffsets +
        (source->top + upperMiddleOffset);
    sourceLowerRows = source->rowOffsets +
        (source->top + lowerMiddleOffset);
    destinationLeftCenter = destination->left + halfWidthMinusOne;
    destinationRightCenter = destination->left + rightHalfWidthMinusOne;
    destinationUpperRows = destination->rowOffsets +
        (destination->top + upperMiddleOffset);
    destinationLowerRows = destination->rowOffsets +
        (destination->top + lowerMiddleOffset);
    row = 0;
    leading = leadingEdges;
    trailing = trailingEdges;

    while ((leadingWidth = *leading) >= 0) {
        if (leadingWidth >= halfWidth)
            leadingWidth = halfWidth - 1;
        if (*trailing < halfWidth) {
            copyWidth = (short)(leadingWidth - *trailing);
            memcpy(destination->pixels + destinationLowerRows[row] +
                       *trailing + destinationRightCenter + 1,
                   source->pixels + sourceLowerRows[row] +
                       *trailing + sourceRightCenter + 1,
                   copyWidth);
            memcpy(destination->pixels +
                       (destinationLowerRows[row] + destinationLeftCenter -
                        1 - *trailing) - copyWidth,
                   source->pixels +
                       (sourceLowerRows[row] + sourceLeftCenter - 1 -
                        *trailing) - copyWidth,
                   copyWidth);
            memcpy(destination->pixels + *(destinationUpperRows - row - 1) +
                       *trailing + destinationRightCenter,
                   source->pixels + *(sourceUpperRows - row - 1) +
                       *trailing + sourceRightCenter,
                   copyWidth);
            memcpy(destination->pixels +
                       (*(destinationUpperRows - row - 1) +
                        destinationLeftCenter - *trailing) - copyWidth,
                   source->pixels +
                       (*(sourceUpperRows - row - 1) + sourceLeftCenter -
                        *trailing) - copyWidth,
                   copyWidth);
        }
        row++;
        leading++;
        trailing++;
        upperRowCount--;
        if (upperRowCount == 0)
            break;
    }
}

/* Function start: 0x425BBF */
void SetSolidColourTranslation(unsigned char colour)
{
    memset(g_abSolidColourTranslation_004b2710, colour, 255);
    g_abSolidColourTranslation_004b2710[255] = 0xff;
    SetPaletteTranslationTable(g_abSolidColourTranslation_004b2710);
}

/* Function start: 0x425BF6 */
void PrepareShapeRLEData(unsigned char *shape)
{
    RLEFrameHeader *frameHeader;
    unsigned char *bitmap;
    unsigned char *pixel;
    unsigned char *output;
    unsigned char *preparedShape;
    int *frameOffset;
    int preparedSize;
    int frameCount;
    short width;
    short height;
    short leftExtent;
    short topExtent;
    int frame;
    int row;
    int remaining;
    int runLength;
    int frameLeft;
    int frameTop;
    int frameRight;
    int frameBottom;

    CheckHeapBlockSignature(shape);
    if (GetPreparedShapeData(shape) != 0)
        return;

    *(int *)g_abShapeRLEScratch_004b2810 =
        *(const int *)g_szShapeRLEVersion_00496a04;
    frameCount = GetShapeFrameCount(shape);
    *(int *)(g_abShapeRLEScratch_004b2810 + 4) = frameCount;
    memset(g_abShapeRLEScratch_004b2810 + 8, 0,
           (unsigned int)(frameCount << 3));
    frameOffset = (int *)(g_abShapeRLEScratch_004b2810 + 8);
    output = g_abShapeRLEScratch_004b2810 + 8 + (frameCount << 3);

    for (frame = 0; frame < frameCount; frame++) {
        *frameOffset = (int)(output - g_abShapeRLEScratch_004b2810);
        frameOffset += 2;
        GetShapeFrameExtents(shape, (short)frame, &width, &height,
                             &leftExtent, &topExtent);
        frameHeader = (RLEFrameHeader *)output;
        frameHeader->height = height;
        frameHeader->width = width;
        frameHeader->topExtent = topExtent;
        frameHeader->leftExtent = leftExtent;
        frameLeft = -leftExtent;
        frameTop = -topExtent;
        frameRight = width - leftExtent - 1;
        frameBottom = height - topExtent - 1;
        frameHeader->left = frameLeft;
        frameHeader->top = frameTop;
        frameHeader->right = frameRight;
        frameHeader->bottom = frameBottom;
        output += sizeof(RLEFrameHeader);

        bitmap = AllocateTaggedMemory(
            (unsigned int)((int)width * height), 0);
        pixel = bitmap;
        memset(bitmap, 0xff, (unsigned int)((int)width * height));
        DecodeShapeFrame(shape, (short)frame, bitmap, width, height,
                         leftExtent, topExtent);
        for (row = 0; row < height; row++) {
            remaining = width;
            while (remaining > 0) {
                if (*pixel != 0xff) {
                    unsigned char *runCode;

                    runLength = 0;
                    runCode = output++;
                    while (remaining > 0 && runLength < 0x7f &&
                           *pixel != 0xff) {
                        *output = *pixel;
                        runLength++;
                        remaining--;
                        output++;
                        pixel++;
                    }
                    *runCode = (unsigned char)(runLength * 2 + 1);
                } else {
                    runLength = 0;
                    while (remaining > 0 && runLength < 0xff &&
                           *pixel == 0xff) {
                        runLength++;
                        remaining--;
                        pixel++;
                    }
                    *output++ = 1;
                    *output++ = (unsigned char)runLength;
                }
            }
            *output++ = 0;
        }
        ReleasePacketHandle(bitmap);
    }

    preparedSize = (int)(output - g_abShapeRLEScratch_004b2810);
    if (preparedSize > (int)sizeof(g_abShapeRLEScratch_004b2810))
        exit_squadron(g_szShapeRLEOverflow_00496a0c);
    preparedShape = AllocateTaggedMemory(preparedSize, 0);
    memcpy(preparedShape, g_abShapeRLEScratch_004b2810, preparedSize);
#ifdef WC1_SDL
    *(unsigned char **)(shape - 8 - sizeof(unsigned char *)) = preparedShape;
#else
    *(unsigned char **)(shape - 4) = preparedShape;
#endif
}

/* Function start: 0x425FB4 */
void DrawSpriteTransformed(Viewport *viewport, int x, int y,
                           unsigned char *shape, int frame,
                           int angle, int scaleX, int scaleY,
                           int flip, int blendMode)
{
    LARGE_INTEGER prepareStart;
    LARGE_INTEGER prepareEnd;
    LARGE_INTEGER clipEnd;
    LARGE_INTEGER drawEnd;

    if (shape != 0 && frame >= 0 && viewport->pixels != 0 &&
        viewport->rowOffsets != 0 && frame < GetShapeFrameCount(shape) &&
        viewport->left >= 0) {
        ReadPerformanceCounter(&prepareStart);
        PrepareShapeRLEData(shape);
        ReadPerformanceCounter(&prepareEnd);
        ClipViewportToScreen(viewport);
        ReadPerformanceCounter(&clipEnd);
        if (flip != 0) {
            if (flip == 0x10) {
                scaleX = -scaleX;
            } else if (flip == 0x20) {
                scaleY = -scaleY;
            } else if (flip == 0x30) {
                scaleX = -scaleX;
                scaleY = -scaleY;
            } else {
                exit_squadron(g_szBadShapeFlip_00496a20);
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
        ReadPerformanceCounter(&drawEnd);
        g_nShapePrepareTicks_005d2fd0 +=
            prepareEnd.LowPart - prepareStart.LowPart;
        g_nShapeClipTicks_005d2fd4 +=
            clipEnd.LowPart - prepareEnd.LowPart;
        g_nShapeDrawTicks_005d2fd8 +=
            drawEnd.LowPart - clipEnd.LowPart;
        if (viewport->pixels == g_stScreenViewport_005d21a0.pixels)
            MarkDibDirty();
    }
}

/* Function start: 0x4261D5 */
void RasterLineHook(const void *marker)
{
    (void)marker;
}

/* Function start: 0x4261E5 */
void DrawFontGlyph(char character, TextContext *context, int height,
                   int width, int y)
{
    unsigned char *font = context->font;
    Viewport *viewport = context->viewport;
    unsigned char colour = context->colour;
    unsigned char background = context->backgroundColour;
    unsigned char fontColour = font[2];
    unsigned char fontBackground = font[3];
    int row = y;
    unsigned char *characterData;
    unsigned char *source;
    unsigned char *destination;
    unsigned char translated;
    unsigned int bitmapOffset;
    unsigned int destinationOffset;
    short characterIndex;
    int column;

    g_abPaletteTranslation_00496338[fontColour] = colour;
    g_abPaletteTranslation_00496338[fontBackground] = background;
    characterIndex = (short)(signed char)character;
    if (characterIndex != 0x81 && characterIndex != 0x84 &&
        characterIndex != 0x8e && characterIndex != 0x94 &&
        characterIndex != 0x99 && characterIndex != 0x9a &&
        characterIndex != 0xe1) {
        characterData = font + characterIndex;
        bitmapOffset = ((unsigned int)characterData[0x204] << 8) +
                       characterData[0x104];
        source = font + bitmapOffset;
        if (fontColour == colour && fontBackground == background) {
            for (; height-- != 0; row++) {
                if (viewport->top == row &&
                    (viewport->rowOffsets[row] & 0x8000) != 0)
                    destinationOffset = (unsigned int)context->cursorX;
                else
                    destinationOffset = viewport->rowOffsets[row] +
                                        (unsigned int)context->cursorX;
                destination = viewport->pixels + destinationOffset;
                for (column = width; column-- != 0;) {
                    if (*source != 0xff)
                        *destination = *source;
                    source++;
                    destination++;
                }
            }
        } else {
            for (; height-- != 0; row++) {
                if (viewport->top == row &&
                    (viewport->rowOffsets[row] & 0x8000) != 0)
                    destinationOffset = (unsigned int)context->cursorX;
                else
                    destinationOffset = viewport->rowOffsets[row] +
                                        (unsigned int)context->cursorX;
                destination = viewport->pixels + destinationOffset;
                for (column = width; column-- != 0;) {
                    translated = g_abPaletteTranslation_00496338[*source];
                    if (translated != 0xff)
                        *destination = translated;
                    source++;
                    destination++;
                }
            }
        }
        context->cursorX += font[4 + characterIndex];
        g_abPaletteTranslation_00496338[fontColour] = fontColour;
        g_abPaletteTranslation_00496338[fontBackground] = fontBackground;
    }
}

/* Function start: 0x426694 */
void MarkActivePaletteEntries(Viewport *viewport, unsigned char *active)
{
#if 0
    unsigned short rgb[3];
    int index;

    (void)viewport;
    index = 0;
    do {
        GetPaletteEntry((short)index, rgb);
        if (rgb[0] != 0 || rgb[1] != 0 || rgb[2] != 0)
            active[index] = 1;
        index++;
    } while (index < 256);
#endif
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
#if 0
    if (shape != 0 && frame >= 0)
        DrawSpriteTransformed(viewport, x, y, shape, frame,
                              0, 0x100, 0x100, 0, 0);
#else
    if (shape == 0)
        return;
    if (frame < 0)
        return;
    if (viewport->left < 0)
        return;
    if (HasValidShapeAllocationSignature(shape) == 0)
        return;
    DrawSpriteTransformed(viewport, x, y, shape, frame,
                          0, 0x100, 0x100, 0, 0);
#endif
}

/* Function start: 0x426769 */
void CaptureSpriteBackground(Viewport *viewport, unsigned char *background,
                             short x, short y, unsigned char *shape,
                             short frame)
{
    unsigned char *commands;
    unsigned char *saved;
    unsigned char *screen;
    unsigned char *pixels;
    unsigned char code;
    short left;
    short top;
    short right;
    short bottom;
    short copyLength;
    int frameOffset;
    int drawX;
    int drawY;
    int endX;
    int skip;
    unsigned short count;
    unsigned short runLength;
#ifdef WC1_SDL
    short coordinate;
#endif

    saved = background;
    if (background == 0)
        return;
    if (shape == 0)
        return;
    if (frame < 0)
        return;
    frameOffset = (short)(frame * 4 + 4);
    if ((int)*(unsigned short *)(shape + 4) <= frameOffset)
        return;
    left = viewport->left;
    right = viewport->right;
    top = viewport->top;
    bottom = viewport->bottom;
    commands = shape + *(int *)(shape + frameOffset) + 8;
#ifdef WC1_SDL
    memcpy(&count, commands, sizeof(count));
#else
    count = *(unsigned short *)commands;
#endif
    pixels = viewport->pixels;
    commands += 2;
    while (count != 0) {
#ifdef WC1_SDL
        memcpy(&coordinate, commands, sizeof(coordinate));
        drawX = x + coordinate;
        commands += 2;
        memcpy(&coordinate, commands, sizeof(coordinate));
        drawY = y + coordinate;
        commands += 2;
#else
        drawX = x + *(short *)commands;
        commands += 2;
        drawY = y + *(short *)commands;
        commands += 2;
#endif
        screen = pixels + (WC1_SPRITE_ROW_OFFSET(viewport, drawY) + drawX);
        if ((count & 1) != 0) {
            count >>= 1;
            while (count != 0) {
                code = *commands;
                commands++;
                if ((code & 1) != 0) {
                    runLength = (unsigned short)(code >> 1);
                    commands++;
                    count -= runLength;
                    endX = drawX + runLength - 1;
                    if (top <= drawY && bottom >= drawY && drawX <= right &&
                        left <= endX) {
                        copyLength = (short)runLength;
                        skip = 0;
                        if (drawX < left) {
                            copyLength = copyLength - left + drawX;
                            skip = left - drawX;
                        }
                        if (right < endX)
                            copyLength = copyLength - endX + right;
                        memcpy(saved, screen + skip, copyLength);
                        saved += copyLength;
                    }
                } else {
                    runLength = (unsigned short)(code >> 1);
                    count -= runLength;
                    endX = drawX + runLength - 1;
                    if (top <= drawY && bottom >= drawY && drawX <= right &&
                        left <= endX) {
                        copyLength = (short)runLength;
                        skip = 0;
                        if (drawX < left) {
                            copyLength = copyLength - left + drawX;
                            skip = left - drawX;
                        }
                        if (right < endX)
                            copyLength = copyLength - endX + right;
                        memcpy(saved, screen + skip, copyLength);
                        saved += copyLength;
                    }
                    commands += runLength;
                }
                drawX += runLength;
                screen += runLength;
            }
        } else {
            count >>= 1;
            endX = drawX + count - 1;
            if (top <= drawY && bottom >= drawY && drawX <= right &&
                left <= endX) {
                copyLength = (short)count;
                skip = 0;
                if (drawX < left) {
                    copyLength = copyLength - left + drawX;
                    skip = left - drawX;
                }
                if (right < endX)
                    copyLength = copyLength - endX + right;
                memcpy(saved, screen + skip, copyLength);
                saved += copyLength;
            }
            commands += count;
        }
#ifdef WC1_SDL
        memcpy(&count, commands, sizeof(count));
#else
        count = *(unsigned short *)commands;
#endif
        commands += 2;
    }
}

/* Function start: 0x426B96 */
void RestoreSpriteBackground(Viewport *viewport, unsigned char *background,
                             short x, short y, unsigned char *shape,
                             short frame)
{
    unsigned char *commands;
    unsigned char *saved;
    unsigned char *screen;
    unsigned char *pixels;
    unsigned char code;
    short left;
    short top;
    short right;
    short bottom;
    short copyLength;
    int frameOffset;
    int drawX;
    int drawY;
    int endX;
    int skip;
    unsigned short count;
    unsigned short runLength;
#ifdef WC1_SDL
    short coordinate;
#endif

    saved = background;
    if (background == 0)
        return;
    if (shape == 0)
        return;
    if (frame < 0)
        return;
    frameOffset = (short)(frame * 4 + 4);
    if ((int)*(unsigned short *)(shape + 4) <= frameOffset)
        return;
    left = viewport->left;
    right = viewport->right;
    top = viewport->top;
    bottom = viewport->bottom;
    commands = shape + *(int *)(shape + frameOffset) + 8;
#ifdef WC1_SDL
    memcpy(&count, commands, sizeof(count));
#else
    count = *(unsigned short *)commands;
#endif
    pixels = viewport->pixels;
    commands += 2;
    while (count != 0) {
#ifdef WC1_SDL
        memcpy(&coordinate, commands, sizeof(coordinate));
        drawX = x + coordinate;
        commands += 2;
        memcpy(&coordinate, commands, sizeof(coordinate));
        drawY = y + coordinate;
        commands += 2;
#else
        drawX = x + *(short *)commands;
        commands += 2;
        drawY = y + *(short *)commands;
        commands += 2;
#endif
        screen = pixels + (WC1_SPRITE_ROW_OFFSET(viewport, drawY) + drawX);
        if ((count & 1) != 0) {
            count >>= 1;
            while (count != 0) {
                code = *commands;
                commands++;
                if ((code & 1) != 0) {
                    runLength = (unsigned short)(code >> 1);
                    commands++;
                    count -= runLength;
                    endX = drawX + runLength - 1;
                    if (top <= drawY && bottom >= drawY && drawX <= right &&
                        left <= endX) {
                        copyLength = (short)runLength;
                        skip = 0;
                        if (drawX < left) {
                            copyLength = copyLength - left + drawX;
                            skip = left - drawX;
                        }
                        if (right < endX)
                            copyLength = copyLength - endX + right;
                        memcpy(screen + skip, saved, copyLength);
                        saved += copyLength;
                    }
                } else {
                    runLength = (unsigned short)(code >> 1);
                    count -= runLength;
                    endX = drawX + runLength - 1;
                    if (top <= drawY && bottom >= drawY && drawX <= right &&
                        left <= endX) {
                        copyLength = (short)runLength;
                        skip = 0;
                        if (drawX < left) {
                            copyLength = copyLength - left + drawX;
                            skip = left - drawX;
                        }
                        if (right < endX)
                            copyLength = copyLength - endX + right;
                        memcpy(screen + skip, saved, copyLength);
                        saved += copyLength;
                    }
                    commands += runLength;
                }
                drawX += runLength;
                screen += runLength;
            }
        } else {
            count >>= 1;
            endX = drawX + count - 1;
            if (top <= drawY && bottom >= drawY && drawX <= right &&
                left <= endX) {
                copyLength = (short)count;
                skip = 0;
                if (drawX < left) {
                    copyLength = copyLength - left + drawX;
                    skip = left - drawX;
                }
                if (right < endX)
                    copyLength = copyLength - endX + right;
                memcpy(screen + skip, saved, copyLength);
                saved += copyLength;
            }
            commands += count;
        }
#ifdef WC1_SDL
        memcpy(&count, commands, sizeof(count));
#else
        count = *(unsigned short *)commands;
#endif
        commands += 2;
    }
    if (viewport->pixels == g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
}

/* Function start: 0x426FD9 */
void DrawSolidColourSprite(Viewport *viewport, short x, short y,
                           unsigned char *shape, short frame,
                           unsigned char colour)
{
#if 0
    SetSolidColourTranslation(colour);
    DrawSpriteTransformed(viewport, x, y, shape, frame, 0,
                          0x100, 0x100, 0, 1);
#else
    if (viewport->left >= 0) {
        SetSolidColourTranslation(colour);
        if (HasValidShapeAllocationSignature(shape) != 0)
            DrawSpriteTransformed(viewport, x, y, shape, frame, 0,
                                  0x100, 0x100, 0, 1);
    }
#endif
}

/* Function start: 0x427047 */
void CopyViewportContents(Viewport *source, Viewport *destination)
{
    RasterSurface sourceSurface;
    RasterClip destinationClip;
    int blitResult;
    RasterClip sourceClip;
    RasterSurface destinationSurface;

    if (destination->left < 0)
        return;
    if (source->left < 0)
        return;
    ValidateViewportBounds(source, &sourceSurface, &sourceClip);
    ValidateViewportBounds(destination, &destinationSurface,
                           &destinationClip);
    blitResult = BlitRasterClip(
        &sourceClip, 0, 0, &destinationClip, 0, 0, 0xffffffff);
    if (blitResult != 0)
        abs(blitResult);
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
#if 0
    if (viewport->pixels != 0 && viewport->rowOffsets != 0) {
        ClipViewportToScreen(viewport);
        FillRasterClip(&g_stRasterClip_004b2088, colour);
    }
    if (viewport == &g_stScreenViewport_005d21a0) {
        MarkDibDirty();
        DIBslamReal();
    }
#else
    if (viewport->pixels != 0 && viewport->rowOffsets != 0 &&
        viewport->left >= 0) {
        ClipViewportToScreen(viewport);
        FillRasterClip(&g_stRasterClip_004b2088, colour);
    }
#endif
}

/* Function start: 0x427179 */
void DrawViewportPixel(Viewport *viewport, short x, short y,
                       short colour)
{
    if (viewport->left >= 0) {
        ClipViewportToScreen(viewport);
        SetRasterClipPixel(&g_stRasterClip_004b2088,
                           (int)x - viewport->left,
                           (int)y - viewport->top, colour);
    }
}

/* Function start: 0x4271D2 */
int GetViewportPixel(Viewport *viewport, short x, short y)
{
    ClipViewportToScreen(viewport);
    return ReadRasterClipPixel(&g_stRasterClip_004b2088,
                               (int)x - viewport->left,
                               (int)y - viewport->top);
}

/* Function start: 0x42722E */
void DrawViewportLine(Viewport *viewport, short x1, short y1,
                      short x2, short y2, short colour)
{
    if (viewport->left >= 0) {
        ClipViewportToScreen(viewport);
        DrawClippedLine(&g_stRasterClip_004b2088,
                        x1 - viewport->left, y1 - viewport->top,
                        x2 - viewport->left, y2 - viewport->top,
                        0, colour);
    }
}

/* Function start: 0x4272A5 */
void TriangleRasterizerHook(void)
{
}

/* Function start: 0x427E1B */
void DrawFilledViewportRect(Viewport *viewport, short left, short top,
                            short right, short bottom, short colour)
{
    int row;
    int height;

    height = bottom - top;
    ClipViewportToScreen(viewport);
    for (row = 0; row <= height; row++) {
        DrawClippedLine(&g_stRasterClip_004b2088,
                        left - viewport->left,
                        row + top - viewport->top,
                        right - viewport->left,
                        row + top - viewport->top,
                        0, colour);
    }
}

/* Function start: 0x427EC8 */
void DrawViewportBorder(Viewport *viewport, short left, short top,
                        short right, short bottom, short colour)
{
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
    if (viewport->left >= 0) {
        ClipViewportToScreen(viewport);
        DrawRasterEllipse(&g_stRasterClip_004b2088, x, y,
                          horizontalRadius, verticalRadius, colour);
        if (viewport->pixels == g_stScreenViewport_005d21a0.pixels)
            MarkDibDirty();
    }
}

/* Function start: 0x42808F */
void FillViewportEllipse(Viewport *viewport, short x, short y,
                         short verticalRadius, short horizontalRadius,
                         short colour)
{
    if (viewport->left >= 0) {
        ClipViewportToScreen(viewport);
        FillRasterEllipse(&g_stRasterClip_004b2088, x, y,
                          horizontalRadius, verticalRadius, colour);
        if (viewport->pixels == g_stScreenViewport_005d21a0.pixels)
            MarkDibDirty();
    }
}

/* Function start: 0x4280FB */
void DrawViewportEllipseShadow(Viewport *viewport, short x, short y,
                               short verticalRadius,
                               short horizontalRadius, short colour)
{
    if (viewport->left >= 0) {
        ClipViewportToScreen(viewport);
        DrawRasterEllipse(&g_stRasterClip_004b2088, x, y,
                          horizontalRadius, verticalRadius, colour);
        if (viewport->pixels == g_stScreenViewport_005d21a0.pixels)
            MarkDibDirty();
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
                                 unsigned char colour)
{
    SetSolidColourTranslation(colour);
    if (HasValidShapeAllocationSignature(shape) != 0) {
        DrawSpriteTransformed(viewport, x, y, shape, frame, angle,
                              scale, scale, flip, 1);
    }
}

/* Function start: 0x4283A4 */
int GetTransformedShapeBounds(Viewport *viewport, short x, short y,
                              unsigned char *shape, short frame,
                              short angle, short scale, int flip,
                              short *bounds)
{
    short *frameData;
    int frameOffset;
    int leftExtent;
    int topExtent;
    int absoluteCosine;
    int absoluteSine;
    int horizontalExtent;
    int transformedHeight;
    int transformedWidth;
    short left;
    short top;
    short right;
    short bottom;

    if (shape == 0) {
        if (viewport->left <= x && x <= viewport->right &&
            viewport->top <= y && y <= viewport->bottom)
            return 1;
        return 0;
    }
    CheckHeapBlockSignature(shape);
    frameOffset = frame * 4 + 4;
    if (frameOffset <= (int)*(unsigned short *)(shape + 4)) {
        frameData = (short *)(shape + *(int *)(shape + frameOffset));
        leftExtent = frameData[1];
        topExtent = frameData[2];
        absoluteCosine =
            (int)(g_awAbsoluteCosine_00496438[angle] * scale) >> 8;
        absoluteSine =
            (int)(g_awAbsoluteSine_00496708[angle] * scale) >> 8;
        if (absoluteCosine == 0)
            absoluteCosine = 1;
        if (absoluteSine == 0)
            absoluteSine = 1;
        horizontalExtent = topExtent + frameData[3];
        transformedHeight =
            absoluteSine * (frameData[0] + leftExtent) +
            absoluteCosine * horizontalExtent;
        if ((char)transformedHeight != 0)
            transformedHeight += 0x100;
        transformedHeight >>= 8;
        transformedWidth =
            absoluteCosine * (frameData[0] + leftExtent) +
            absoluteSine * horizontalExtent;
        if ((char)transformedWidth != 0)
            transformedWidth += 0x100;
        transformedWidth >>= 8;
        if (absoluteCosine == 0)
            absoluteCosine = 1;
        if (absoluteSine == 0)
            absoluteSine = 1;
        top = (short)(y - (absoluteSine * leftExtent >> 8) -
                      (absoluteCosine * topExtent >> 8));
        bottom = (short)(transformedHeight + top);
        left = (short)(((absoluteSine * topExtent >> 8) -
                        (absoluteCosine * leftExtent >> 8) + x) -
                       ((absoluteSine * horizontalExtent >> 8) + 1));
        right = (short)(transformedWidth + left);
        if (viewport->left <= right && left <= viewport->right &&
            viewport->top <= bottom && top <= viewport->bottom) {
            bounds[0] = left;
            bounds[2] = right;
            bounds[1] = top;
            bounds[3] = bottom;
            return 1;
        }
    }
    (void)flip;
    return 0;
}

/* Function start: 0x428690 */
void fizzle_fade(Viewport *source, Viewport *destination,
                 const ScreenViewportGeometry *geometry)
{
    const short *run;
    unsigned char *sourcePixels;
    unsigned char *destinationPixels;
    unsigned short width;
    short sourceLeft;
    short sourceTop;
    short destinationX;
    short sourceY;

    if (source->pixels != 0 && destination->pixels != 0) {
        run = &geometry->originX;
        sourceLeft = *run++;
        sourceTop = *run++;
        destinationX = *run++;
        if (destinationX != -1) {
            do {
                sourceY = *run++;
                width = (unsigned short)*run++;
                sourcePixels = source->pixels +
                    source->rowOffsets[sourceY - sourceTop] - sourceLeft +
                    destinationX;
                destinationPixels = destination->pixels +
                    destination->rowOffsets[sourceY] + destinationX;
                memcpy(destinationPixels, sourcePixels, width);
                destinationX = *run++;
            } while (destinationX != -1);
        }
        if (destination->pixels == g_stScreenViewport_005d21a0.pixels)
            MarkDibDirty();
    }
}

/* Function start: 0x427DE8 */
void snow_viewport(Viewport *viewport, int effect, unsigned short colour)
{
    (void)effect;
    (void)colour;
    if (viewport->pixels == g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
    RasterLineHook(g_szShadedTriangle_00496a60);
}

/* Function start: 0x428979 */
void UpdateStreamerStoppedFlag(void)
{
    if (g_nAudioEnabled_0049c244 != 0) {
        if ((GetStreamerState() & 4) == 0)
            g_nMusicTrackComplete_0049be88 = 1;
        else
            g_nMusicTrackComplete_0049be88 = 0;
    }
    return;
}

/* Function start: 0x42894B */
void SignalAudioMixerWakeEvent(void)
{
#if 0
    if (g_nAudioEnabled_0049c244 != 0)
        ix_streamer_configure(2, 0);
#else
    if (g_nAudioEnabled_0049c244 == 0)
        return;
    ix_streamer_configure(2, 0);
    return;
#endif
}

/* Function start: 0x428900 */
void InitializeAudioStreamer(HWND window)
{
#if 0
    if (g_nAudioEnabled_0049c244 != 0) {
        ix_streamer_configure(3, (void *)1);
        ix_streamer_configure(0, window);
        ix_streamer_init();
        ix_streamer_set_intensity(0x19);
    }
#else
    if (g_nAudioEnabled_0049c244 == 0)
        return;
    ix_streamer_configure(3, (void *)1);
    ix_streamer_configure(0, window);
    ix_streamer_init();
    ix_streamer_set_intensity(0x19);
    return;
#endif
}

/* Function start: 0x4289BF */
void Streamer_open(const char *streamName)
{
    char *streamsDirectory;

    if (g_nAudioEnabled_0049c244 != 0) {
        streamsDirectory = LocateStreamsDirOnDisc();
#if 0
        if (streamsDirectory == 0) {
            ShowNoticeMessageBox("Unable to locate streamed music.\n");
            exit_squadron("");
        }
#endif
        sprintf(g_szStreamerPath_005b2818, "%s%s",
                streamsDirectory, streamName);
        SoundDebugPrintf("Streamer_open %s", g_szStreamerPath_005b2818);
        ix_streamer_open_stream_file(g_szStreamerPath_005b2818);
        g_nStreamerAudioPlaying_005b2810 = 0;
    }
    return;
}

/* Function start: 0x428A2F */
void Streamer_play(void)
{
#if 0
    if (g_nAudioEnabled_0049c244 != 0 && g_nStreamerAudioPlaying_005b2810 == 0) {
        g_nStreamerAudioPlaying_005b2810 = 1;
        SoundDebugPrintf("Streamer_play");
        ix_streamer_audio_play();
    }
#else
    if (g_nAudioEnabled_0049c244 == 0)
        return;
    if (g_nStreamerAudioPlaying_005b2810 == 0) {
        g_nStreamerAudioPlaying_005b2810 = 1;
        ix_streamer_audio_play();
    }
    return;
#endif
}

/* Function start: 0x428A6D */
void Streamer_stop(void)
{
#if 0
    if (g_nAudioEnabled_0049c244 != 0 && g_nStreamerAudioPlaying_005b2810 != 0) {
        SoundDebugPrintf("Streamer_stop");
        ix_streamer_audio_stop();
        g_nStreamerAudioPlaying_005b2810 = 0;
    }
#else
    if (g_nAudioEnabled_0049c244 == 0)
        return;
    if (g_nStreamerAudioPlaying_005b2810 != 0) {
        ix_streamer_audio_stop();
        g_nStreamerAudioPlaying_005b2810 = 0;
    }
    return;
#endif
}

/* Function start: WC2_UNMAPPED */
void ClearStreamerTrigger(void)
{
    if (g_nAudioEnabled_0049c244 != 0)
        ix_streamer_set_trigger(-1);
}

/* Function start: 0x428AEB */
void SetStreamerTrigger(int trigger)
{
#if 0
    if (g_nAudioEnabled_0049c244 != 0) {
        if (g_nStreamerAudioPlaying_005b2810 == 0) {
            ForceStreamerTrigger(trigger);
            return;
        }
        SoundDebugPrintf("Streamer_trigger %d", trigger);
        if (trigger >= 0)
            ix_streamer_set_trigger((char)trigger);
        if (g_nStreamerAudioPlaying_005b2810 == 0)
            Streamer_play();
    }
#else
    if (g_nAudioEnabled_0049c244 != 0) {
        ix_streamer_set_trigger((char)trigger);
        if (trigger >= 0 && g_nStreamerAudioPlaying_005b2810 == 0)
            ForceStreamerTrigger(trigger);
    }
    return;
#endif
}

/* Function start: 0x428AAB */
void SetStreamerIntensity(int intensity)
{
    if (g_nAudioEnabled_0049c244 != 0) {
        ix_streamer_set_intensity((unsigned char)intensity);
        if (g_nStreamerAudioPlaying_005b2810 == 0)
            Streamer_play();
    }
    return;
}

/* Function start: 0x428B3C */
void ForceStreamerTrigger(int trigger)
{
    if (g_nAudioEnabled_0049c244 != 0) {
#if 0
        SoundDebugPrintf("FORCE");
#endif
        if (trigger > 0)
            ix_streamer_force_trigger((char)trigger);
        if (g_nStreamerAudioPlaying_005b2810 == 0)
            Streamer_play();
    }
    return;
}

/* Function start: 0x428DA3 */
void SetMusicStreamVolume(unsigned short volume)
{
#if 0
    if (g_nAudioEnabled_0049c244 != 0)
        ix_streamer_set_volume(volume);
#else
    ix_streamer_set_volume(volume);
    return;
#endif
}

/* Function start: 0x428DBF */
unsigned int GetMusicStreamVolume(void)
{
    return ix_streamer_get_volume();
}

/* Function start: 0x428DE0 */
unsigned int GetStreamerState(void)
{
    return g_dwStreamerState_005c4c38;
}

/* Function start: 0x428B86 */
void Streamer_close(void)
{
#if 0
    if (g_nAudioEnabled_0049c244 != 0) {
        SoundDebugPrintf("Streamer_close");
        ix_streamer_close_stream_file();
    }
#else
    if (g_nAudioEnabled_0049c244 == 0)
        return;
    ix_streamer_close_stream_file();
    return;
#endif
}

/* Function start: 0x428BAD */
void DestroyAudioStreamer(void)
{
    if (g_nAudioEnabled_0049c244 == 0)
        return;
    ix_streamer_destroy();
    return;
}

/* Function start: 0x428D09 */
int ReadCheaterFlagFromRegistry(void)
{
    HKEY key;
    DWORD type;
    DWORD size;
    int enabled = 0;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "Software\\Origin Systems\\WC: Kilrathi Saga",
                      0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS) {
        type = REG_DWORD;
        size = sizeof(int);
        RegQueryValueExA(key, "Cheater", 0, &type,
                         (LPBYTE)&enabled, &size);
        RegCloseKey(key);
    }
    if (enabled != 0)
        OutputDebugStringA("reg cheating\n");
    else
        OutputDebugStringA("reg not cheating\n");
    return enabled;
}
