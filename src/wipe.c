/*
 *  Viewport wipe transitions.
 *
 *  Retail object 0x448570-0x44ADDF (link order; see docs/ORDER.md).
 */
/* This unit owns the wipe storage declared in globals.h (as statics). */
#define WIPE_C
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

    g_stDissolveSourceViewport_005b3488 = *source;
    g_stWipeScratchSourcePort_005b34c8 =
        g_stDissolveSourceViewport_005b3488;
    g_stFadeToView_005b3450 = *destination;
    g_stWipeTempDestinationViewport_005b34a0 =
        g_stFadeToView_005b3450;
    g_nFadeType_005b3444 = wipeType;
    g_nWipeRoundFlags_005b34b4 = passFlags;
    g_nDissolveProgress_005b3434 = 0;
    duration = 15;

    switch (g_nFadeType_005b3444) {
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
        g_nFadeWidth_005b3448 =
            destination->right - destination->left + 1;
        g_nBlendHeight_005b344c =
            destination->bottom - destination->top + 1;
        if (g_nBlendHeight_005b344c <= g_nFadeWidth_005b3448) {
            g_bWipeWidthAtLeastHeight_005b342c = 1;
            g_bBlendWidthLessThanHeight_005b3430 = 0;
            g_nWipeMinorExtent_005b3428 = g_nFadeWidth_005b3448;
        } else {
            g_bWipeWidthAtLeastHeight_005b342c = 0;
            g_bBlendWidthLessThanHeight_005b3430 = 1;
            g_nWipeMinorExtent_005b3428 = g_nBlendHeight_005b344c;
        }
        break;
    }

    switch (g_nFadeType_005b3444) {
    case 0:
        SetViewportVerticalBounds(
            &g_stWipeScratchSourcePort_005b34c8,
            g_stDissolveSourceViewport_005b3488.top - 1,
            g_stDissolveSourceViewport_005b3488.top - 1);
        SetViewportVerticalBounds(
            &g_stWipeTempDestinationViewport_005b34a0,
            g_stFadeToView_005b3450.top - 1,
            g_stFadeToView_005b3450.top - 1);
        g_nRevealVertThreshold_005b34bc = g_nBlendHeight_005b344c + 1;
        g_nWipeTotalStages_005b3440 = g_nBlendHeight_005b344c + 1;
        g_nBlendMaximumStep_005b34c4 = (short)(
            g_nBlendHeight_005b344c + 1 < duration
                ? g_nBlendHeight_005b344c + 1
                : duration);
        g_nRevealVerticalDelta_005b34b8 = g_nBlendMaximumStep_005b34c4;
        break;
    case 1:
        SetViewportHorizontalBounds(
            &g_stWipeScratchSourcePort_005b34c8,
            g_stDissolveSourceViewport_005b3488.right + 1,
            g_stDissolveSourceViewport_005b3488.right + 1);
        SetViewportHorizontalBounds(
            &g_stWipeTempDestinationViewport_005b34a0,
            g_stFadeToView_005b3450.right + 1,
            g_stFadeToView_005b3450.right + 1);
        g_nWipeAcrossThreshold_005b347c = g_nFadeWidth_005b3448 + 1;
        g_nWipeTotalStages_005b3440 = g_nFadeWidth_005b3448 + 1;
        g_nBlendMaximumStep_005b34c4 = (short)(
            g_nFadeWidth_005b3448 + 1 < duration
                ? g_nFadeWidth_005b3448 + 1
                : duration);
        g_nWipeAcrossStride_005b3478 = g_nBlendMaximumStep_005b34c4;
        break;
    case 2:
        SetViewportVerticalBounds(
            &g_stWipeScratchSourcePort_005b34c8,
            g_stDissolveSourceViewport_005b3488.bottom + 1,
            g_stDissolveSourceViewport_005b3488.bottom + 1);
        SetViewportVerticalBounds(
            &g_stWipeTempDestinationViewport_005b34a0,
            g_stFadeToView_005b3450.bottom + 1,
            g_stFadeToView_005b3450.bottom + 1);
        g_nRevealVertThreshold_005b34bc = g_nBlendHeight_005b344c + 1;
        g_nWipeTotalStages_005b3440 = g_nBlendHeight_005b344c + 1;
        g_nBlendMaximumStep_005b34c4 = (short)(
            g_nBlendHeight_005b344c + 1 < duration
                ? g_nBlendHeight_005b344c + 1
                : duration);
        g_nRevealVerticalDelta_005b34b8 = g_nBlendMaximumStep_005b34c4;
        break;
    case 3:
        SetViewportHorizontalBounds(
            &g_stWipeScratchSourcePort_005b34c8,
            g_stDissolveSourceViewport_005b3488.left - 1,
            g_stDissolveSourceViewport_005b3488.left - 1);
        SetViewportHorizontalBounds(
            &g_stWipeTempDestinationViewport_005b34a0,
            g_stFadeToView_005b3450.left - 1,
            g_stFadeToView_005b3450.left - 1);
        g_nWipeAcrossThreshold_005b347c = g_nFadeWidth_005b3448 + 1;
        g_nWipeTotalStages_005b3440 = g_nFadeWidth_005b3448 + 1;
        g_nBlendMaximumStep_005b34c4 = (short)(
            g_nFadeWidth_005b3448 + 1 < duration
                ? g_nFadeWidth_005b3448 + 1
                : duration);
        g_nWipeAcrossStride_005b3478 = g_nBlendMaximumStep_005b34c4;
        break;
    case 4:
        g_nWipePreviousAcrossEdge_005b346c = -1;
        g_nFadePreviousVerticalEdge_005b3470 = -1;
        g_nWipeAcrossThreshold_005b347c = g_nFadeWidth_005b3448 + 1;
        g_nRevealVertThreshold_005b34bc = g_nBlendHeight_005b344c + 1;
        g_nWipeTotalStages_005b3440 = g_nFadeWidth_005b3448 + 1;
        g_nBlendMaximumStep_005b34c4 =
            (short)(duration < 320 ? duration : 320);
        g_nRevealVerticalDelta_005b34b8 = g_nBlendMaximumStep_005b34c4;
        g_nWipeAcrossStride_005b3478 = g_nRevealVerticalDelta_005b34b8;
        break;
    case 5:
        g_nWipePreviousAcrossEdge_005b346c =
            g_stDissolveSourceViewport_005b3488.right -
            g_stDissolveSourceViewport_005b3488.left + 1;
        g_nFadePreviousVerticalEdge_005b3470 = -1;
        g_nWipeAcrossThreshold_005b347c = g_nFadeWidth_005b3448 + 1;
        g_nRevealVertThreshold_005b34bc = g_nBlendHeight_005b344c + 1;
        g_nWipeTotalStages_005b3440 = g_nFadeWidth_005b3448 + 1;
        g_nBlendMaximumStep_005b34c4 =
            (short)(duration < 320 ? duration : 320);
        g_nWipeAcrossStride_005b3478 = g_nBlendMaximumStep_005b34c4;
        g_nRevealVerticalDelta_005b34b8 = g_nBlendMaximumStep_005b34c4;
        break;
    case 6:
        g_nWipePreviousAcrossEdge_005b346c =
            g_stDissolveSourceViewport_005b3488.right -
            g_stDissolveSourceViewport_005b3488.left + 1;
        g_nFadePreviousVerticalEdge_005b3470 =
            g_stDissolveSourceViewport_005b3488.bottom -
            g_stDissolveSourceViewport_005b3488.top + 1;
        g_nWipeAcrossThreshold_005b347c = g_nFadeWidth_005b3448 + 1;
        g_nRevealVertThreshold_005b34bc = g_nBlendHeight_005b344c + 1;
        g_nWipeTotalStages_005b3440 = g_nFadeWidth_005b3448 + 1;
        g_nBlendMaximumStep_005b34c4 =
            (short)(duration < 320 ? duration : 320);
        g_nRevealVerticalDelta_005b34b8 = g_nBlendMaximumStep_005b34c4;
        g_nWipeAcrossStride_005b3478 = g_nRevealVerticalDelta_005b34b8;
        break;
    case 7:
        g_nWipePreviousAcrossEdge_005b346c = -1;
        g_nFadePreviousVerticalEdge_005b3470 =
            g_stDissolveSourceViewport_005b3488.bottom -
            g_stDissolveSourceViewport_005b3488.top + 1;
        g_nWipeAcrossThreshold_005b347c = g_nFadeWidth_005b3448 + 1;
        g_nRevealVertThreshold_005b34bc = g_nBlendHeight_005b344c + 1;
        g_nWipeTotalStages_005b3440 = g_nFadeWidth_005b3448 + 1;
        g_nBlendMaximumStep_005b34c4 =
            (short)(duration < 320 ? duration : 320);
        g_nRevealVerticalDelta_005b34b8 = g_nBlendMaximumStep_005b34c4;
        g_nWipeAcrossStride_005b3478 = g_nRevealVerticalDelta_005b34b8;
        break;
    case 8:
        g_nWipeRadialAccumulator_005b3100 = 0;
        g_nWipeRadialRowCount_005d17b4 = g_nBlendHeight_005b344c / 2 + 1;
        g_pWipeFrontEdges_005b3438 =
            g_asWipeRadialLeadingEdge_005b3108;
        g_pWipeRearEdges_005b343c =
            g_asWipeRadialTrailingEdges_005b3298;
        halfWidth = g_nFadeWidth_005b3448;
        halfHeight = g_nBlendHeight_005b344c;
        g_nWipeTotalStages_005b3440 = (short)(
            sqrt((double)(halfHeight * halfHeight +
                          halfWidth * halfWidth)) /
            2.0 + 1.0);
        g_nFadeRadialThreshold_005b30fc = g_nWipeTotalStages_005b3440 + 2;
        g_nBlendMaximumStep_005b34c4 = (short)(
            g_nWipeTotalStages_005b3440 + 2 < duration
                ? g_nWipeTotalStages_005b3440 + 2
                : duration);
        g_nBlendRadialStep_005b30f8 = g_nBlendMaximumStep_005b34c4;
        for (scanline = 0; scanline < 200; scanline++) {
            g_pWipeFrontEdges_005b3438[scanline] = -1;
            g_pWipeRearEdges_005b343c[scanline] = -1;
        }
        break;
    case 9:
        g_nWipeRadialAccumulator_005b3100 = 0;
        g_nWipeRadialRowCount_005d17b4 = g_nBlendHeight_005b344c / 2 + 1;
        g_pWipeFrontEdges_005b3438 =
            g_asWipeRadialLeadingEdge_005b3108;
        g_pWipeRearEdges_005b343c =
            g_asWipeRadialTrailingEdges_005b3298;
        halfWidth = g_nFadeWidth_005b3448;
        halfHeight = g_nBlendHeight_005b344c;
        g_nWipeTotalStages_005b3440 = (short)(
            sqrt((double)(halfHeight * halfHeight +
                          halfWidth * halfWidth)) /
            2.0 + 1.0);
        g_nFadeRadialThreshold_005b30fc = g_nWipeTotalStages_005b3440 + 1;
        g_nBlendMaximumStep_005b34c4 = (short)(
            g_nWipeTotalStages_005b3440 + 1 < duration
                ? g_nWipeTotalStages_005b3440 + 1
                : duration);
        g_nBlendRadialStep_005b30f8 = g_nBlendMaximumStep_005b34c4;
        for (scanline = 0; scanline < g_nWipeRadialRowCount_005d17b4; scanline++) {
            g_pWipeFrontEdges_005b3438[scanline] = 300;
            g_pWipeRearEdges_005b343c[scanline] = 300;
        }
        break;
    }

    g_nBlendHorizontalTotal_005b3480 =
        g_nWipeAcrossStride_005b3478 >> 2;
    g_nTransitionVertSum_005b34c0 =
        g_nRevealVerticalDelta_005b34b8 >> 2;
    g_nWipeRadialAccumulator_005b3100 =
        g_nBlendRadialStep_005b30f8 >> 2;
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

    switch (g_nFadeType_005b3444) {
    case 0:
        verticalCount = 0;
        while (g_nRevealVertThreshold_005b34bc >
                   g_nTransitionVertSum_005b34c0 &&
               g_nWipeTotalStages_005b3440 > g_nDissolveProgress_005b3434) {
            g_nTransitionVertSum_005b34c0 +=
                g_nRevealVerticalDelta_005b34b8;
            verticalCount++;
            g_nDissolveProgress_005b3434++;
        }
        g_nTransitionVertSum_005b34c0 -=
            g_nRevealVertThreshold_005b34bc;
        if ((g_nWipeRoundFlags_005b34b4 & 1) != 0) {
            SetViewportVerticalBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                g_stWipeScratchSourcePort_005b34c8.bottom + 1,
                g_stWipeScratchSourcePort_005b34c8.bottom +
                    verticalCount);
            SetViewportVerticalBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                g_stWipeTempDestinationViewport_005b34a0.bottom + 1,
                g_stWipeTempDestinationViewport_005b34a0.bottom +
                    verticalCount);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
        } else {
            OffsetViewportBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                0, 1, 0, verticalCount);
            OffsetViewportBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                0, 1, 0, verticalCount);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
            OffsetViewportBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                0, -1, 0, 0);
            OffsetViewportBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                0, -1, 0, 0);
        }
        break;
    case 1:
        horizontalCount = 0;
        while (g_nWipeAcrossThreshold_005b347c >
                   g_nBlendHorizontalTotal_005b3480 &&
               g_nWipeTotalStages_005b3440 > g_nDissolveProgress_005b3434) {
            g_nBlendHorizontalTotal_005b3480 +=
                g_nWipeAcrossStride_005b3478;
            horizontalCount++;
            g_nDissolveProgress_005b3434++;
        }
        g_nBlendHorizontalTotal_005b3480 -=
            g_nWipeAcrossThreshold_005b347c;
        if ((g_nWipeRoundFlags_005b34b4 & 1) != 0) {
            SetViewportHorizontalBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                g_stWipeScratchSourcePort_005b34c8.left -
                    horizontalCount,
                g_stWipeScratchSourcePort_005b34c8.left - 1);
            SetViewportHorizontalBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                g_stWipeTempDestinationViewport_005b34a0.left -
                    horizontalCount,
                g_stWipeTempDestinationViewport_005b34a0.left - 1);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
        } else {
            OffsetViewportBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                -horizontalCount, 0, -1, 0);
            OffsetViewportBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                -horizontalCount, 0, -1, 0);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
            OffsetViewportBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                0, 0, 1, 0);
            OffsetViewportBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                0, 0, 1, 0);
        }
        break;
    case 2:
        verticalCount = 0;
        while (g_nRevealVertThreshold_005b34bc >
                   g_nTransitionVertSum_005b34c0 &&
               g_nWipeTotalStages_005b3440 > g_nDissolveProgress_005b3434) {
            g_nTransitionVertSum_005b34c0 +=
                g_nRevealVerticalDelta_005b34b8;
            verticalCount++;
            g_nDissolveProgress_005b3434++;
        }
        g_nTransitionVertSum_005b34c0 -=
            g_nRevealVertThreshold_005b34bc;
        if ((g_nWipeRoundFlags_005b34b4 & 1) != 0) {
            SetViewportVerticalBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                g_stWipeScratchSourcePort_005b34c8.top - verticalCount,
                g_stWipeScratchSourcePort_005b34c8.top - 1);
            SetViewportVerticalBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                g_stWipeTempDestinationViewport_005b34a0.top -
                    verticalCount,
                g_stWipeTempDestinationViewport_005b34a0.top - 1);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
        } else {
            OffsetViewportBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                0, -verticalCount, 0, -1);
            OffsetViewportBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                0, -verticalCount, 0, -1);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
            OffsetViewportBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                0, 0, 0, 1);
            OffsetViewportBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                0, 0, 0, 1);
        }
        break;
    case 3:
        horizontalCount = 0;
        while (g_nWipeAcrossThreshold_005b347c >
                   g_nBlendHorizontalTotal_005b3480 &&
               g_nWipeTotalStages_005b3440 > g_nDissolveProgress_005b3434) {
            g_nBlendHorizontalTotal_005b3480 +=
                g_nWipeAcrossStride_005b3478;
            horizontalCount++;
            g_nDissolveProgress_005b3434++;
        }
        g_nBlendHorizontalTotal_005b3480 -=
            g_nWipeAcrossThreshold_005b347c;
        if ((g_nWipeRoundFlags_005b34b4 & 1) != 0) {
            SetViewportHorizontalBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                g_stWipeScratchSourcePort_005b34c8.right + 1,
                g_stWipeScratchSourcePort_005b34c8.right +
                    horizontalCount);
            SetViewportHorizontalBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                g_stWipeTempDestinationViewport_005b34a0.right + 1,
                g_stWipeTempDestinationViewport_005b34a0.right +
                    horizontalCount);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
        } else {
            OffsetViewportBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                1, 0, horizontalCount, 0);
            OffsetViewportBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                1, 0, horizontalCount, 0);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
            OffsetViewportBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                -1, 0, 0, 0);
            OffsetViewportBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                -1, 0, 0, 0);
        }
        break;
    case 4:
        verticalCount = 0;
        horizontalCount = verticalCount;
        while (g_nWipeAcrossThreshold_005b347c >
                   g_nBlendHorizontalTotal_005b3480 &&
               g_nWipeTotalStages_005b3440 > g_nDissolveProgress_005b3434) {
            g_nBlendHorizontalTotal_005b3480 +=
                g_nWipeAcrossStride_005b3478;
            horizontalCount++;
            g_nDissolveProgress_005b3434++;
        }
        g_nBlendHorizontalTotal_005b3480 -=
            g_nWipeAcrossThreshold_005b347c;
        while (g_nRevealVertThreshold_005b34bc >
               g_nTransitionVertSum_005b34c0) {
            g_nTransitionVertSum_005b34c0 +=
                g_nRevealVerticalDelta_005b34b8;
            verticalCount++;
        }
        g_nTransitionVertSum_005b34c0 -=
            g_nRevealVertThreshold_005b34bc;
        g_nFadeCurrentHorizontalEdge_005b3464 =
            horizontalCount + g_nWipePreviousAcrossEdge_005b346c;
        g_nWipeCurrentVerticalEdges_005b3468 =
            verticalCount + g_nFadePreviousVerticalEdge_005b3470;
        if ((g_nWipeRoundFlags_005b34b4 & 1) != 0) {
            if (g_nFadePreviousVerticalEdge_005b3470 >= 0 &&
                horizontalCount != 0) {
                SetViewportRectangleBounds(
                    &g_stWipeScratchSourcePort_005b34c8,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nWipePreviousAcrossEdge_005b346c + 1,
                    g_stDissolveSourceViewport_005b3488.top,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stDissolveSourceViewport_005b3488.top +
                        g_nFadePreviousVerticalEdge_005b3470);
                SetViewportRectangleBounds(
                    &g_stWipeTempDestinationViewport_005b34a0,
                    g_stFadeToView_005b3450.left +
                        g_nWipePreviousAcrossEdge_005b346c + 1,
                    g_stFadeToView_005b3450.top,
                    g_stFadeToView_005b3450.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stFadeToView_005b3450.top +
                        g_nFadePreviousVerticalEdge_005b3470);
                CopyViewportContents(
                    &g_stWipeScratchSourcePort_005b34c8,
                    &g_stWipeTempDestinationViewport_005b34a0);
            }
            if (verticalCount != 0) {
                SetViewportRectangleBounds(
                    &g_stWipeScratchSourcePort_005b34c8,
                    g_stDissolveSourceViewport_005b3488.left,
                    g_stDissolveSourceViewport_005b3488.top +
                        g_nFadePreviousVerticalEdge_005b3470 + 1,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stDissolveSourceViewport_005b3488.top +
                        g_nWipeCurrentVerticalEdges_005b3468);
                SetViewportRectangleBounds(
                    &g_stWipeTempDestinationViewport_005b34a0,
                    g_stFadeToView_005b3450.left,
                    g_stFadeToView_005b3450.top +
                        g_nFadePreviousVerticalEdge_005b3470 + 1,
                    g_stFadeToView_005b3450.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stFadeToView_005b3450.top +
                        g_nWipeCurrentVerticalEdges_005b3468);
                CopyViewportContents(
                    &g_stWipeScratchSourcePort_005b34c8,
                    &g_stWipeTempDestinationViewport_005b34a0);
            }
        } else {
            SetViewportRectangleBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                g_stDissolveSourceViewport_005b3488.left,
                g_stDissolveSourceViewport_005b3488.top,
                g_stDissolveSourceViewport_005b3488.left +
                    g_nFadeCurrentHorizontalEdge_005b3464,
                g_stDissolveSourceViewport_005b3488.top +
                    g_nWipeCurrentVerticalEdges_005b3468);
            SetViewportRectangleBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                g_stFadeToView_005b3450.left,
                g_stFadeToView_005b3450.top,
                g_stFadeToView_005b3450.left +
                    g_nFadeCurrentHorizontalEdge_005b3464,
                g_stFadeToView_005b3450.top +
                    g_nWipeCurrentVerticalEdges_005b3468);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
        }
        break;
    case 5:
        verticalCount = 0;
        horizontalCount = verticalCount;
        while (g_nWipeAcrossThreshold_005b347c >
                   g_nBlendHorizontalTotal_005b3480 &&
               g_nWipeTotalStages_005b3440 > g_nDissolveProgress_005b3434) {
            g_nBlendHorizontalTotal_005b3480 +=
                g_nWipeAcrossStride_005b3478;
            horizontalCount++;
            g_nDissolveProgress_005b3434++;
        }
        g_nBlendHorizontalTotal_005b3480 -=
            g_nWipeAcrossThreshold_005b347c;
        while (g_nRevealVertThreshold_005b34bc >
               g_nTransitionVertSum_005b34c0) {
            g_nTransitionVertSum_005b34c0 +=
                g_nRevealVerticalDelta_005b34b8;
            verticalCount++;
        }
        g_nTransitionVertSum_005b34c0 -=
            g_nRevealVertThreshold_005b34bc;
        g_nFadeCurrentHorizontalEdge_005b3464 =
            g_nWipePreviousAcrossEdge_005b346c - horizontalCount;
        g_nWipeCurrentVerticalEdges_005b3468 =
            verticalCount + g_nFadePreviousVerticalEdge_005b3470;
        if ((g_nWipeRoundFlags_005b34b4 & 1) != 0) {
            if (g_nFadePreviousVerticalEdge_005b3470 >= 0 &&
                horizontalCount != 0) {
                SetViewportRectangleBounds(
                    &g_stWipeScratchSourcePort_005b34c8,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stDissolveSourceViewport_005b3488.top,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nWipePreviousAcrossEdge_005b346c - 1,
                    g_stDissolveSourceViewport_005b3488.top +
                        g_nFadePreviousVerticalEdge_005b3470);
                SetViewportRectangleBounds(
                    &g_stWipeTempDestinationViewport_005b34a0,
                    g_stFadeToView_005b3450.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stFadeToView_005b3450.top,
                    g_stFadeToView_005b3450.left +
                        g_nWipePreviousAcrossEdge_005b346c - 1,
                    g_stFadeToView_005b3450.top +
                        g_nFadePreviousVerticalEdge_005b3470);
                CopyViewportContents(
                    &g_stWipeScratchSourcePort_005b34c8,
                    &g_stWipeTempDestinationViewport_005b34a0);
            }
            if (verticalCount != 0) {
                SetViewportRectangleBounds(
                    &g_stWipeScratchSourcePort_005b34c8,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stDissolveSourceViewport_005b3488.top +
                        g_nFadePreviousVerticalEdge_005b3470 + 1,
                    g_stDissolveSourceViewport_005b3488.right,
                    g_stDissolveSourceViewport_005b3488.top +
                        g_nWipeCurrentVerticalEdges_005b3468);
                SetViewportRectangleBounds(
                    &g_stWipeTempDestinationViewport_005b34a0,
                    g_stFadeToView_005b3450.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stFadeToView_005b3450.top +
                        g_nFadePreviousVerticalEdge_005b3470 + 1,
                    g_stFadeToView_005b3450.right,
                    g_stFadeToView_005b3450.top +
                        g_nWipeCurrentVerticalEdges_005b3468);
                CopyViewportContents(
                    &g_stWipeScratchSourcePort_005b34c8,
                    &g_stWipeTempDestinationViewport_005b34a0);
            }
        } else {
            SetViewportRectangleBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                g_stDissolveSourceViewport_005b3488.left +
                    g_nFadeCurrentHorizontalEdge_005b3464,
                g_stDissolveSourceViewport_005b3488.top,
                g_stDissolveSourceViewport_005b3488.right,
                g_stDissolveSourceViewport_005b3488.top +
                    g_nWipeCurrentVerticalEdges_005b3468);
            SetViewportRectangleBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                g_stFadeToView_005b3450.left +
                    g_nFadeCurrentHorizontalEdge_005b3464,
                g_stFadeToView_005b3450.top,
                g_stFadeToView_005b3450.right,
                g_stFadeToView_005b3450.top +
                    g_nWipeCurrentVerticalEdges_005b3468);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
        }
        break;
    case 6:
        verticalCount = 0;
        horizontalCount = verticalCount;
        while (g_nWipeAcrossThreshold_005b347c >
                   g_nBlendHorizontalTotal_005b3480 &&
               g_nWipeTotalStages_005b3440 > g_nDissolveProgress_005b3434) {
            g_nBlendHorizontalTotal_005b3480 +=
                g_nWipeAcrossStride_005b3478;
            horizontalCount++;
            g_nDissolveProgress_005b3434++;
        }
        g_nBlendHorizontalTotal_005b3480 -=
            g_nWipeAcrossThreshold_005b347c;
        while (g_nRevealVertThreshold_005b34bc >
               g_nTransitionVertSum_005b34c0) {
            g_nTransitionVertSum_005b34c0 +=
                g_nRevealVerticalDelta_005b34b8;
            verticalCount++;
        }
        g_nTransitionVertSum_005b34c0 -=
            g_nRevealVertThreshold_005b34bc;
        g_nFadeCurrentHorizontalEdge_005b3464 =
            g_nWipePreviousAcrossEdge_005b346c - horizontalCount;
        g_nWipeCurrentVerticalEdges_005b3468 =
            g_nFadePreviousVerticalEdge_005b3470 - verticalCount;
        if ((g_nWipeRoundFlags_005b34b4 & 1) != 0) {
            if (g_nFadePreviousVerticalEdge_005b3470 <=
                    g_stDissolveSourceViewport_005b3488.bottom -
                        g_stDissolveSourceViewport_005b3488.top &&
                horizontalCount != 0) {
                SetViewportRectangleBounds(
                    &g_stWipeScratchSourcePort_005b34c8,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stDissolveSourceViewport_005b3488.top +
                        g_nFadePreviousVerticalEdge_005b3470,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nWipePreviousAcrossEdge_005b346c - 1,
                    g_stDissolveSourceViewport_005b3488.bottom);
                SetViewportRectangleBounds(
                    &g_stWipeTempDestinationViewport_005b34a0,
                    g_stFadeToView_005b3450.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stFadeToView_005b3450.top +
                        g_nFadePreviousVerticalEdge_005b3470,
                    g_stFadeToView_005b3450.left +
                        g_nWipePreviousAcrossEdge_005b346c - 1,
                    g_stFadeToView_005b3450.bottom);
                CopyViewportContents(
                    &g_stWipeScratchSourcePort_005b34c8,
                    &g_stWipeTempDestinationViewport_005b34a0);
            }
            if (verticalCount != 0) {
                SetViewportRectangleBounds(
                    &g_stWipeScratchSourcePort_005b34c8,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stDissolveSourceViewport_005b3488.top +
                        g_nWipeCurrentVerticalEdges_005b3468,
                    g_stDissolveSourceViewport_005b3488.right,
                    g_stDissolveSourceViewport_005b3488.top +
                        g_nFadePreviousVerticalEdge_005b3470 - 1);
                SetViewportRectangleBounds(
                    &g_stWipeTempDestinationViewport_005b34a0,
                    g_stFadeToView_005b3450.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stFadeToView_005b3450.top +
                        g_nWipeCurrentVerticalEdges_005b3468,
                    g_stFadeToView_005b3450.right,
                    g_stFadeToView_005b3450.top +
                        g_nFadePreviousVerticalEdge_005b3470 - 1);
                CopyViewportContents(
                    &g_stWipeScratchSourcePort_005b34c8,
                    &g_stWipeTempDestinationViewport_005b34a0);
            }
        } else {
            SetViewportRectangleBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                g_stDissolveSourceViewport_005b3488.left +
                    g_nFadeCurrentHorizontalEdge_005b3464,
                g_stDissolveSourceViewport_005b3488.top +
                    g_nWipeCurrentVerticalEdges_005b3468,
                g_stDissolveSourceViewport_005b3488.right,
                g_stDissolveSourceViewport_005b3488.bottom);
            SetViewportRectangleBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                g_stFadeToView_005b3450.left +
                    g_nFadeCurrentHorizontalEdge_005b3464,
                g_stFadeToView_005b3450.top +
                    g_nWipeCurrentVerticalEdges_005b3468,
                g_stFadeToView_005b3450.right,
                g_stFadeToView_005b3450.bottom);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
        }
        break;
    case 7:
        verticalCount = 0;
        horizontalCount = verticalCount;
        while (g_nWipeAcrossThreshold_005b347c >
                   g_nBlendHorizontalTotal_005b3480 &&
               g_nWipeTotalStages_005b3440 > g_nDissolveProgress_005b3434) {
            g_nBlendHorizontalTotal_005b3480 +=
                g_nWipeAcrossStride_005b3478;
            horizontalCount++;
            g_nDissolveProgress_005b3434++;
        }
        g_nBlendHorizontalTotal_005b3480 -=
            g_nWipeAcrossThreshold_005b347c;
        while (g_nRevealVertThreshold_005b34bc >
               g_nTransitionVertSum_005b34c0) {
            g_nTransitionVertSum_005b34c0 +=
                g_nRevealVerticalDelta_005b34b8;
            verticalCount++;
        }
        g_nTransitionVertSum_005b34c0 -=
            g_nRevealVertThreshold_005b34bc;
        g_nFadeCurrentHorizontalEdge_005b3464 =
            horizontalCount + g_nWipePreviousAcrossEdge_005b346c;
        g_nWipeCurrentVerticalEdges_005b3468 =
            g_nFadePreviousVerticalEdge_005b3470 - verticalCount;
        if ((g_nWipeRoundFlags_005b34b4 & 1) != 0) {
            if (g_nFadePreviousVerticalEdge_005b3470 <=
                    g_stDissolveSourceViewport_005b3488.bottom -
                        g_stDissolveSourceViewport_005b3488.top &&
                horizontalCount != 0) {
                SetViewportRectangleBounds(
                    &g_stWipeScratchSourcePort_005b34c8,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nWipePreviousAcrossEdge_005b346c + 1,
                    g_stDissolveSourceViewport_005b3488.top +
                        g_nFadePreviousVerticalEdge_005b3470,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stDissolveSourceViewport_005b3488.bottom);
                SetViewportRectangleBounds(
                    &g_stWipeTempDestinationViewport_005b34a0,
                    g_stFadeToView_005b3450.left +
                        g_nWipePreviousAcrossEdge_005b346c + 1,
                    g_stFadeToView_005b3450.top +
                        g_nFadePreviousVerticalEdge_005b3470,
                    g_stFadeToView_005b3450.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stFadeToView_005b3450.bottom);
                CopyViewportContents(
                    &g_stWipeScratchSourcePort_005b34c8,
                    &g_stWipeTempDestinationViewport_005b34a0);
            }
            if (verticalCount != 0) {
                SetViewportRectangleBounds(
                    &g_stWipeScratchSourcePort_005b34c8,
                    g_stDissolveSourceViewport_005b3488.left,
                    g_stDissolveSourceViewport_005b3488.top +
                        g_nWipeCurrentVerticalEdges_005b3468,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stDissolveSourceViewport_005b3488.top +
                        g_nFadePreviousVerticalEdge_005b3470 - 1);
                SetViewportRectangleBounds(
                    &g_stWipeTempDestinationViewport_005b34a0,
                    g_stFadeToView_005b3450.left,
                    g_stFadeToView_005b3450.top +
                        g_nWipeCurrentVerticalEdges_005b3468,
                    g_stFadeToView_005b3450.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stFadeToView_005b3450.top +
                        g_nFadePreviousVerticalEdge_005b3470 - 1);
                CopyViewportContents(
                    &g_stWipeScratchSourcePort_005b34c8,
                    &g_stWipeTempDestinationViewport_005b34a0);
            }
        } else {
            SetViewportRectangleBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                g_stDissolveSourceViewport_005b3488.left,
                g_stDissolveSourceViewport_005b3488.top +
                    g_nWipeCurrentVerticalEdges_005b3468,
                g_stDissolveSourceViewport_005b3488.left +
                    g_nFadeCurrentHorizontalEdge_005b3464,
                g_stDissolveSourceViewport_005b3488.bottom);
            SetViewportRectangleBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                g_stFadeToView_005b3450.left,
                g_stFadeToView_005b3450.top +
                    g_nWipeCurrentVerticalEdges_005b3468,
                g_stFadeToView_005b3450.left +
                    g_nFadeCurrentHorizontalEdge_005b3464,
                g_stFadeToView_005b3450.bottom);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
        }
        break;
    case 8:
        while (g_nFadeRadialThreshold_005b30fc >
                   g_nWipeRadialAccumulator_005b3100 &&
               g_nWipeTotalStages_005b3440 > g_nDissolveProgress_005b3434) {
            g_nWipeRadialAccumulator_005b3100 +=
                g_nBlendRadialStep_005b30f8;
            g_nDissolveProgress_005b3434++;
        }
        g_nWipeRadialAccumulator_005b3100 -=
            g_nFadeRadialThreshold_005b30fc;
        GenerateWipeRadialEdges(g_pWipeFrontEdges_005b3438,
                                g_nDissolveProgress_005b3434);
        BlitWipeRadialBands(
            &g_stWipeScratchSourcePort_005b34c8,
            &g_stWipeTempDestinationViewport_005b34a0,
            g_pWipeRearEdges_005b343c,
            g_pWipeFrontEdges_005b3438);
        if ((g_nWipeRoundFlags_005b34b4 & 1) != 0) {
            edgeSwap = g_pWipeRearEdges_005b343c;
            g_pWipeRearEdges_005b343c = g_pWipeFrontEdges_005b3438;
            g_pWipeFrontEdges_005b3438 = edgeSwap;
        }
        break;
    case 9:
        while (g_nFadeRadialThreshold_005b30fc >
                   g_nWipeRadialAccumulator_005b3100 &&
               g_nWipeTotalStages_005b3440 > g_nDissolveProgress_005b3434) {
            g_nWipeRadialAccumulator_005b3100 +=
                g_nBlendRadialStep_005b30f8;
            g_nDissolveProgress_005b3434++;
        }
        g_nWipeRadialAccumulator_005b3100 -=
            g_nFadeRadialThreshold_005b30fc;
        GenerateWipeRadialEdges(
            g_pWipeRearEdges_005b343c,
            g_nWipeTotalStages_005b3440 - g_nDissolveProgress_005b3434);
        BlitWipeRadialBands(
            &g_stWipeScratchSourcePort_005b34c8,
            &g_stWipeTempDestinationViewport_005b34a0,
            g_pWipeRearEdges_005b343c,
            g_pWipeFrontEdges_005b3438);
        if ((g_nWipeRoundFlags_005b34b4 & 1) != 0) {
            edgeSwap = g_pWipeRearEdges_005b343c;
            g_pWipeRearEdges_005b343c = g_pWipeFrontEdges_005b3438;
            g_pWipeFrontEdges_005b3438 = edgeSwap;
        }
        break;
    }

    g_nWipePreviousAcrossEdge_005b346c =
        g_nFadeCurrentHorizontalEdge_005b3464;
    g_nFadePreviousVerticalEdge_005b3470 =
        g_nWipeCurrentVerticalEdges_005b3468;
    (void)workspace;
    if (g_nWipeTotalStages_005b3440 == g_nDissolveProgress_005b3434)
        return 1;
    else
        return 0;
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
    short width;
    short height;

    /* "!flags & 1" binds as "(!flags) & 1"; retail tests it that way. */
    if (!g_nWipeRoundFlags_005b34b4 & 1) {
        CopyViewportContents(&g_stDissolveSourceViewport_005b3488,
                             &g_stFadeToView_005b3450);
    } else {
        switch (g_nFadeType_005b3444) {
        case 0:
            height = g_nWipeTotalStages_005b3440 -
                g_nDissolveProgress_005b3434;
            SetViewportVerticalBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                g_stWipeScratchSourcePort_005b34c8.bottom + 1,
                g_stWipeScratchSourcePort_005b34c8.bottom + height);
            SetViewportVerticalBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                g_stWipeTempDestinationViewport_005b34a0.bottom + 1,
                g_stWipeTempDestinationViewport_005b34a0.bottom +
                    height);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
            break;
        case 1:
            width = g_nWipeTotalStages_005b3440 -
                g_nDissolveProgress_005b3434;
            SetViewportHorizontalBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                g_stWipeScratchSourcePort_005b34c8.left - width,
                g_stWipeScratchSourcePort_005b34c8.left - 1);
            SetViewportHorizontalBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                g_stWipeTempDestinationViewport_005b34a0.left - width,
                g_stWipeTempDestinationViewport_005b34a0.left - 1);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
            break;
        case 2:
            height = g_nWipeTotalStages_005b3440 -
                g_nDissolveProgress_005b3434;
            SetViewportVerticalBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                g_stWipeScratchSourcePort_005b34c8.top - height,
                g_stWipeScratchSourcePort_005b34c8.top - 1);
            SetViewportVerticalBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                g_stWipeTempDestinationViewport_005b34a0.top - height,
                g_stWipeTempDestinationViewport_005b34a0.top - 1);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
            break;
        case 3:
            width = g_nWipeTotalStages_005b3440 -
                g_nDissolveProgress_005b3434;
            SetViewportHorizontalBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                g_stWipeScratchSourcePort_005b34c8.right + 1,
                g_stWipeScratchSourcePort_005b34c8.right + width);
            SetViewportHorizontalBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                g_stWipeTempDestinationViewport_005b34a0.right + 1,
                g_stWipeTempDestinationViewport_005b34a0.right + width);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
            break;
        case 4:
            g_nFadeCurrentHorizontalEdge_005b3464 =
                g_stDissolveSourceViewport_005b3488.right -
                g_stDissolveSourceViewport_005b3488.left;
            g_nWipeCurrentVerticalEdges_005b3468 =
                g_stDissolveSourceViewport_005b3488.bottom -
                g_stDissolveSourceViewport_005b3488.top;
            if (g_nFadePreviousVerticalEdge_005b3470 >= 0) {
                SetViewportRectangleBounds(
                    &g_stWipeScratchSourcePort_005b34c8,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nWipePreviousAcrossEdge_005b346c + 1,
                    g_stDissolveSourceViewport_005b3488.top,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stDissolveSourceViewport_005b3488.top +
                        g_nFadePreviousVerticalEdge_005b3470);
                SetViewportRectangleBounds(
                    &g_stWipeTempDestinationViewport_005b34a0,
                    g_stFadeToView_005b3450.left +
                        g_nWipePreviousAcrossEdge_005b346c + 1,
                    g_stFadeToView_005b3450.top,
                    g_stFadeToView_005b3450.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stFadeToView_005b3450.top +
                        g_nFadePreviousVerticalEdge_005b3470);
                CopyViewportContents(
                    &g_stWipeScratchSourcePort_005b34c8,
                    &g_stWipeTempDestinationViewport_005b34a0);
            }
            SetViewportRectangleBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                g_stDissolveSourceViewport_005b3488.left,
                g_stDissolveSourceViewport_005b3488.top +
                    g_nFadePreviousVerticalEdge_005b3470 + 1,
                g_stDissolveSourceViewport_005b3488.left +
                    g_nFadeCurrentHorizontalEdge_005b3464,
                g_stDissolveSourceViewport_005b3488.top +
                    g_nWipeCurrentVerticalEdges_005b3468);
            SetViewportRectangleBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                g_stFadeToView_005b3450.left,
                g_stFadeToView_005b3450.top +
                    g_nFadePreviousVerticalEdge_005b3470 + 1,
                g_stFadeToView_005b3450.left +
                    g_nFadeCurrentHorizontalEdge_005b3464,
                g_stFadeToView_005b3450.top +
                    g_nWipeCurrentVerticalEdges_005b3468);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
            break;
        case 5:
            g_nFadeCurrentHorizontalEdge_005b3464 = 0;
            g_nWipeCurrentVerticalEdges_005b3468 =
                g_stDissolveSourceViewport_005b3488.bottom -
                g_stDissolveSourceViewport_005b3488.top;
            if (g_nFadePreviousVerticalEdge_005b3470 >= 0) {
                SetViewportRectangleBounds(
                    &g_stWipeScratchSourcePort_005b34c8,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stDissolveSourceViewport_005b3488.top,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nWipePreviousAcrossEdge_005b346c - 1,
                    g_stDissolveSourceViewport_005b3488.top +
                        g_nFadePreviousVerticalEdge_005b3470);
                SetViewportRectangleBounds(
                    &g_stWipeTempDestinationViewport_005b34a0,
                    g_stFadeToView_005b3450.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stFadeToView_005b3450.top,
                    g_stFadeToView_005b3450.left +
                        g_nWipePreviousAcrossEdge_005b346c - 1,
                    g_stFadeToView_005b3450.top +
                        g_nFadePreviousVerticalEdge_005b3470);
                CopyViewportContents(
                    &g_stWipeScratchSourcePort_005b34c8,
                    &g_stWipeTempDestinationViewport_005b34a0);
            }
            SetViewportRectangleBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                g_stDissolveSourceViewport_005b3488.left +
                    g_nFadeCurrentHorizontalEdge_005b3464,
                g_stDissolveSourceViewport_005b3488.top +
                    g_nFadePreviousVerticalEdge_005b3470 + 1,
                g_stDissolveSourceViewport_005b3488.right,
                g_stDissolveSourceViewport_005b3488.top +
                    g_nWipeCurrentVerticalEdges_005b3468);
            SetViewportRectangleBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                g_stFadeToView_005b3450.left +
                    g_nFadeCurrentHorizontalEdge_005b3464,
                g_stFadeToView_005b3450.top +
                    g_nFadePreviousVerticalEdge_005b3470 + 1,
                g_stFadeToView_005b3450.right,
                g_stFadeToView_005b3450.top +
                    g_nWipeCurrentVerticalEdges_005b3468);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
            break;
        case 6:
            g_nFadeCurrentHorizontalEdge_005b3464 = 0;
            g_nWipeCurrentVerticalEdges_005b3468 = 0;
            if (g_nFadePreviousVerticalEdge_005b3470 >= 0) {
                SetViewportRectangleBounds(
                    &g_stWipeScratchSourcePort_005b34c8,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stDissolveSourceViewport_005b3488.top +
                        g_nFadePreviousVerticalEdge_005b3470,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nWipePreviousAcrossEdge_005b346c - 1,
                    g_stDissolveSourceViewport_005b3488.bottom);
                SetViewportRectangleBounds(
                    &g_stWipeTempDestinationViewport_005b34a0,
                    g_stFadeToView_005b3450.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stFadeToView_005b3450.top +
                        g_nFadePreviousVerticalEdge_005b3470,
                    g_stFadeToView_005b3450.left +
                        g_nWipePreviousAcrossEdge_005b346c - 1,
                    g_stFadeToView_005b3450.bottom);
                CopyViewportContents(
                    &g_stWipeScratchSourcePort_005b34c8,
                    &g_stWipeTempDestinationViewport_005b34a0);
            }
            SetViewportRectangleBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                g_stDissolveSourceViewport_005b3488.left +
                    g_nFadeCurrentHorizontalEdge_005b3464,
                g_stDissolveSourceViewport_005b3488.top +
                    g_nWipeCurrentVerticalEdges_005b3468,
                g_stDissolveSourceViewport_005b3488.right,
                g_stDissolveSourceViewport_005b3488.top +
                    g_nFadePreviousVerticalEdge_005b3470 + 1);
            SetViewportRectangleBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                g_stFadeToView_005b3450.left +
                    g_nFadeCurrentHorizontalEdge_005b3464,
                g_stFadeToView_005b3450.top +
                    g_nWipeCurrentVerticalEdges_005b3468,
                g_stFadeToView_005b3450.right,
                g_stFadeToView_005b3450.top +
                    g_nFadePreviousVerticalEdge_005b3470 + 1);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
            break;
        case 7:
            g_nFadeCurrentHorizontalEdge_005b3464 =
                g_stDissolveSourceViewport_005b3488.right -
                g_stDissolveSourceViewport_005b3488.left;
            g_nWipeCurrentVerticalEdges_005b3468 = 0;
            if (g_nFadePreviousVerticalEdge_005b3470 >= 0) {
                SetViewportRectangleBounds(
                    &g_stWipeScratchSourcePort_005b34c8,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nWipePreviousAcrossEdge_005b346c + 1,
                    g_stDissolveSourceViewport_005b3488.top +
                        g_nFadePreviousVerticalEdge_005b3470,
                    g_stDissolveSourceViewport_005b3488.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stDissolveSourceViewport_005b3488.bottom);
                SetViewportRectangleBounds(
                    &g_stWipeTempDestinationViewport_005b34a0,
                    g_stFadeToView_005b3450.left +
                        g_nWipePreviousAcrossEdge_005b346c + 1,
                    g_stFadeToView_005b3450.top +
                        g_nFadePreviousVerticalEdge_005b3470,
                    g_stFadeToView_005b3450.left +
                        g_nFadeCurrentHorizontalEdge_005b3464,
                    g_stFadeToView_005b3450.bottom);
                CopyViewportContents(
                    &g_stWipeScratchSourcePort_005b34c8,
                    &g_stWipeTempDestinationViewport_005b34a0);
            }
            SetViewportRectangleBounds(
                &g_stWipeScratchSourcePort_005b34c8,
                g_stDissolveSourceViewport_005b3488.left,
                g_stDissolveSourceViewport_005b3488.top +
                    g_nWipeCurrentVerticalEdges_005b3468,
                g_stDissolveSourceViewport_005b3488.left +
                    g_nFadeCurrentHorizontalEdge_005b3464,
                g_stDissolveSourceViewport_005b3488.top +
                    g_nFadePreviousVerticalEdge_005b3470 + 1);
            SetViewportRectangleBounds(
                &g_stWipeTempDestinationViewport_005b34a0,
                g_stFadeToView_005b3450.left,
                g_stFadeToView_005b3450.top +
                    g_nWipeCurrentVerticalEdges_005b3468,
                g_stFadeToView_005b3450.left +
                    g_nFadeCurrentHorizontalEdge_005b3464,
                g_stFadeToView_005b3450.top +
                    g_nFadePreviousVerticalEdge_005b3470 + 1);
            CopyViewportContents(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0);
            break;
        case 8:
            g_nDissolveProgress_005b3434 = g_nWipeTotalStages_005b3440;
            GenerateWipeRadialEdges(g_pWipeFrontEdges_005b3438,
                                    g_nDissolveProgress_005b3434);
            BlitWipeRadialBands(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0,
                g_pWipeRearEdges_005b343c,
                g_pWipeFrontEdges_005b3438);
            break;
        case 9:
            g_nDissolveProgress_005b3434 = g_nWipeTotalStages_005b3440;
            GenerateWipeRadialEdges(
                g_pWipeRearEdges_005b343c,
                g_nWipeTotalStages_005b3440 - g_nDissolveProgress_005b3434);
            BlitWipeRadialBands(
                &g_stWipeScratchSourcePort_005b34c8,
                &g_stWipeTempDestinationViewport_005b34a0,
                g_pWipeRearEdges_005b343c,
                g_pWipeFrontEdges_005b3438);
            break;
        }
    }
    (void)workspace;
}

/* Function start: 0x44A910 */
void GenerateWipeRadialEdges(short *edges, int radius)
{
    short y;
    short err;
    short lastY;
    short decision;
    short blankRow;
    short x;

    x = 0;
    y = (short)radius;
    err = (short)((1 - radius) * 2);
    if (g_nWipeRadialRowCount_005d17b4 > y)
        edges[y] = x;
    lastY = y;

    while (y > 0) {
        if (err < 0) {
            decision = (short)((err + y) * 2 - 1);
            if (decision > 0) {
                x++;
                y--;
                err += 2 * (x - y + 1);
            } else {
                x++;
                err += x * 2 + 1;
            }
        } else if (err > 0) {
            decision = (short)((err - x) * 2 - 1);
            if (decision > 0) {
                y--;
                err += 1 - y * 2;
            } else {
                x++;
                y--;
                err += 2 * (x - y + 1);
            }
        } else {
            x++;
            y--;
            err += 2 * (x - y + 1);
        }
        if (lastY != y) {
            if (g_nWipeRadialRowCount_005d17b4 > y)
                edges[y] = x;
            lastY = y;
        }
    }

    if (radius == 0)
        edges[0] = -1;
    for (blankRow = (short)(radius + 1);
         g_nWipeRadialRowCount_005d17b4 > blankRow;
         blankRow++) {
        edges[blankRow] = -1;
    }
}

#pragma function(memcpy)
/* Function start: 0x44AAE5 */
void BlitWipeRadialBands(Viewport *source, Viewport *destination,
                         short *trailingEdges, short *leadingEdges)
{
    short src_left_center;
    unsigned short *dst_row_offsets_lower;
    short radial_right_half_end;
    short half_source_width;
    short source_right_x;
    short dest_right_x;
    short wipe_row_count;
    short remaining_upper_rows;
    short wipe_width;
    int lead_width;
    short top_center_row;
    int bands_processed;
    short *wipe_trailing_edge;
    short *wipe_leading_widths;
    short wipe_left_half_extent;
    unsigned short *lower_src_row_offsets;
    short lower_center_offset;
    short band_width;
    short dst_midpoint_left;
    unsigned short *src_top_rows;
    unsigned short *destination_top_rows;

    wipe_width = source->right - source->left + 1;
    half_source_width = (wipe_width + 1) >> 1;
    wipe_left_half_extent = half_source_width - 1;
    radial_right_half_end = wipe_left_half_extent;
    wipe_row_count = source->bottom - source->top + 1;
    remaining_upper_rows = (wipe_row_count + 1) >> 1;
    top_center_row = remaining_upper_rows - 1;
    lower_center_offset = wipe_row_count >> 1;
    src_left_center = source->left + wipe_left_half_extent;
    source_right_x = source->left + radial_right_half_end;
    src_top_rows = source->rowOffsets +
        (source->top + top_center_row);
    lower_src_row_offsets = source->rowOffsets +
        (source->top + lower_center_offset);
    dst_midpoint_left = destination->left + wipe_left_half_extent;
    dest_right_x = destination->left + radial_right_half_end;
    destination_top_rows = destination->rowOffsets +
        (destination->top + top_center_row);
    dst_row_offsets_lower = destination->rowOffsets +
        (destination->top + lower_center_offset);
    bands_processed = 0;
    wipe_leading_widths = leadingEdges;
    wipe_trailing_edge = trailingEdges;

    while ((lead_width = *wipe_leading_widths) >= 0) {
        if (lead_width >= half_source_width)
            lead_width = half_source_width - 1;
        if (*wipe_trailing_edge < half_source_width) {
            band_width = (short)(lead_width - *wipe_trailing_edge);
            memmove(destination->pixels + dst_row_offsets_lower[bands_processed] +
                       *wipe_trailing_edge + dest_right_x + 1,
                   source->pixels + lower_src_row_offsets[bands_processed] +
                       *wipe_trailing_edge + source_right_x + 1,
                   band_width);
            memmove(destination->pixels +
                       (dst_row_offsets_lower[bands_processed] + dst_midpoint_left -
                        1 - *wipe_trailing_edge) - band_width,
                   source->pixels +
                       (lower_src_row_offsets[bands_processed] + src_left_center - 1 -
                        *wipe_trailing_edge) - band_width,
                   band_width);
#ifdef SDL_PORT
            /* Odd heights reach rowOffsets[-1] on the last upper band. */
            if (lower_src_row_offsets - bands_processed - 1 >= source->rowOffsets &&
                dst_row_offsets_lower - bands_processed - 1 >= destination->rowOffsets)
#endif
            {
            memmove(destination->pixels + *(dst_row_offsets_lower - bands_processed - 1) +
                       *wipe_trailing_edge + dest_right_x,
                   source->pixels + *(lower_src_row_offsets - bands_processed - 1) +
                       *wipe_trailing_edge + source_right_x,
                   band_width);
            memmove(destination->pixels +
                       (*(dst_row_offsets_lower - bands_processed - 1) +
                        dst_midpoint_left - *wipe_trailing_edge) - band_width,
                   source->pixels +
                       (*(lower_src_row_offsets - bands_processed - 1) + src_left_center -
                        *wipe_trailing_edge) - band_width,
                   band_width);
            }
        }
        bands_processed++;
        wipe_leading_widths++;
        wipe_trailing_edge++;
        remaining_upper_rows--;
        if (remaining_upper_rows == 0)
            break;
    }
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short g_nWipeRadialRowCount_005d17b4;
