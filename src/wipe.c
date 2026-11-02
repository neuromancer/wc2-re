/*
 *  Viewport wipe transitions.
 *
 *  Retail object 0x448570-0x44ADDF (link order; see docs/ORDER.md).
 */
#include "game.h"

#pragma function(sqrt)
/* Function start: 0x448570 */
signed char InitializeViewportWipe(Viewport *source,
                                   Viewport *destination,
                                   int wipeType, short duration,
                                   short passFlags, void *workspace)
{
    int halfWidth;
    int halfHeight;
    short scanline;

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
        halfWidth = g_nWipeWidth_005b3448;
        halfHeight = g_nWipeHeight_005b344c;
        g_nWipeTotalSteps_005b3440 = (short)(
            sqrt((double)(halfHeight * halfHeight +
                          halfWidth * halfWidth)) /
            2.0 + 1.0);
        g_nWipeRadialThreshold_005b30fc = g_nWipeTotalSteps_005b3440 + 2;
        g_nWipeMaximumStep_005b34c4 = (short)(
            g_nWipeTotalSteps_005b3440 + 2 < duration
                ? g_nWipeTotalSteps_005b3440 + 2
                : duration);
        g_nWipeRadialStep_005b30f8 = g_nWipeMaximumStep_005b34c4;
        for (scanline = 0; scanline < 200; scanline++) {
            g_pWipeLeadingEdges_005b3438[scanline] = -1;
            g_pWipeTrailingEdges_005b343c[scanline] = -1;
        }
        break;
    case 9:
        g_nWipeRadialAccumulator_005b3100 = 0;
        g_nWipeRadialRowCount_005d17b4 = g_nWipeHeight_005b344c / 2 + 1;
        g_pWipeLeadingEdges_005b3438 =
            g_asWipeRadialLeadingEdges_005b3108;
        g_pWipeTrailingEdges_005b343c =
            g_asWipeRadialTrailingEdges_005b3298;
        halfWidth = g_nWipeWidth_005b3448;
        halfHeight = g_nWipeHeight_005b344c;
        g_nWipeTotalSteps_005b3440 = (short)(
            sqrt((double)(halfHeight * halfHeight +
                          halfWidth * halfWidth)) /
            2.0 + 1.0);
        g_nWipeRadialThreshold_005b30fc = g_nWipeTotalSteps_005b3440 + 1;
        g_nWipeMaximumStep_005b34c4 = (short)(
            g_nWipeTotalSteps_005b3440 + 1 < duration
                ? g_nWipeTotalSteps_005b3440 + 1
                : duration);
        g_nWipeRadialStep_005b30f8 = g_nWipeMaximumStep_005b34c4;
        for (scanline = 0; scanline < g_nWipeRadialRowCount_005d17b4; scanline++) {
            g_pWipeLeadingEdges_005b3438[scanline] = 300;
            g_pWipeTrailingEdges_005b343c[scanline] = 300;
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

#pragma function(memcpy)
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
#ifdef SDL_PORT
            /* upperMiddleOffset is upperRowCount - 1, so on the last band the
             * backwards index reaches rowOffsets[-1].  The original read
             * whatever happened to precede the table; skip the upper half. */
            if (sourceUpperRows - row - 1 >= source->rowOffsets &&
                destinationUpperRows - row - 1 >= destination->rowOffsets)
#endif
            {
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
        }
        row++;
        leading++;
        trailing++;
        upperRowCount--;
        if (upperRowCount == 0)
            break;
    }
}
