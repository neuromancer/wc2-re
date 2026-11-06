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

    g_3_stDissolveSourceViewport = *source;
    g_1_stWipeScratchSourcePort =
        g_3_stDissolveSourceViewport;
    g_19_stFadeToView = *destination;
    g_26_stWipeTempDestinationViewport =
        g_19_stFadeToView;
    g_17_nFadeType = wipeType;
    g_3_nWipeRoundFlags = passFlags;
    g_139_nDissolveProgress = 0;
    duration = 15;

    switch (g_17_nFadeType) {
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
        g_15_nFadeWidth =
            destination->right - destination->left + 1;
        g_5_nBlendHeight =
            destination->bottom - destination->top + 1;
        if (g_5_nBlendHeight <= g_15_nFadeWidth) {
            g_1_bWipeWidthAtLeastHeight = 1;
            g_bBlendWidthLessThanHeight = 0;
            g_5_nWipeMinorExtent = g_15_nFadeWidth;
        } else {
            g_1_bWipeWidthAtLeastHeight = 0;
            g_bBlendWidthLessThanHeight = 1;
            g_5_nWipeMinorExtent = g_5_nBlendHeight;
        }
        break;
    }

    switch (g_17_nFadeType) {
    case 0:
        SetViewportVerticalBounds(
            &g_1_stWipeScratchSourcePort,
            g_3_stDissolveSourceViewport.top - 1,
            g_3_stDissolveSourceViewport.top - 1);
        SetViewportVerticalBounds(
            &g_26_stWipeTempDestinationViewport,
            g_19_stFadeToView.top - 1,
            g_19_stFadeToView.top - 1);
        g_nRevealVertThreshold = g_5_nBlendHeight + 1;
        g_18_nWipeTotalStages = g_5_nBlendHeight + 1;
        g_nBlendMaximumStep = (short)(
            g_5_nBlendHeight + 1 < duration
                ? g_5_nBlendHeight + 1
                : duration);
        g_16_nRevealVerticalDelta = g_nBlendMaximumStep;
        break;
    case 1:
        SetViewportHorizontalBounds(
            &g_1_stWipeScratchSourcePort,
            g_3_stDissolveSourceViewport.right + 1,
            g_3_stDissolveSourceViewport.right + 1);
        SetViewportHorizontalBounds(
            &g_26_stWipeTempDestinationViewport,
            g_19_stFadeToView.right + 1,
            g_19_stFadeToView.right + 1);
        g_5_nWipeAcrossThreshold = g_15_nFadeWidth + 1;
        g_18_nWipeTotalStages = g_15_nFadeWidth + 1;
        g_nBlendMaximumStep = (short)(
            g_15_nFadeWidth + 1 < duration
                ? g_15_nFadeWidth + 1
                : duration);
        g_18_nWipeAcrossStride = g_nBlendMaximumStep;
        break;
    case 2:
        SetViewportVerticalBounds(
            &g_1_stWipeScratchSourcePort,
            g_3_stDissolveSourceViewport.bottom + 1,
            g_3_stDissolveSourceViewport.bottom + 1);
        SetViewportVerticalBounds(
            &g_26_stWipeTempDestinationViewport,
            g_19_stFadeToView.bottom + 1,
            g_19_stFadeToView.bottom + 1);
        g_nRevealVertThreshold = g_5_nBlendHeight + 1;
        g_18_nWipeTotalStages = g_5_nBlendHeight + 1;
        g_nBlendMaximumStep = (short)(
            g_5_nBlendHeight + 1 < duration
                ? g_5_nBlendHeight + 1
                : duration);
        g_16_nRevealVerticalDelta = g_nBlendMaximumStep;
        break;
    case 3:
        SetViewportHorizontalBounds(
            &g_1_stWipeScratchSourcePort,
            g_3_stDissolveSourceViewport.left - 1,
            g_3_stDissolveSourceViewport.left - 1);
        SetViewportHorizontalBounds(
            &g_26_stWipeTempDestinationViewport,
            g_19_stFadeToView.left - 1,
            g_19_stFadeToView.left - 1);
        g_5_nWipeAcrossThreshold = g_15_nFadeWidth + 1;
        g_18_nWipeTotalStages = g_15_nFadeWidth + 1;
        g_nBlendMaximumStep = (short)(
            g_15_nFadeWidth + 1 < duration
                ? g_15_nFadeWidth + 1
                : duration);
        g_18_nWipeAcrossStride = g_nBlendMaximumStep;
        break;
    case 4:
        g_2_nWipePreviousAcrossEdge = -1;
        g_105_nFadePreviousVerticalEdge = -1;
        g_5_nWipeAcrossThreshold = g_15_nFadeWidth + 1;
        g_nRevealVertThreshold = g_5_nBlendHeight + 1;
        g_18_nWipeTotalStages = g_15_nFadeWidth + 1;
        g_nBlendMaximumStep =
            (short)(duration < 320 ? duration : 320);
        g_16_nRevealVerticalDelta = g_nBlendMaximumStep;
        g_18_nWipeAcrossStride = g_16_nRevealVerticalDelta;
        break;
    case 5:
        g_2_nWipePreviousAcrossEdge =
            g_3_stDissolveSourceViewport.right -
            g_3_stDissolveSourceViewport.left + 1;
        g_105_nFadePreviousVerticalEdge = -1;
        g_5_nWipeAcrossThreshold = g_15_nFadeWidth + 1;
        g_nRevealVertThreshold = g_5_nBlendHeight + 1;
        g_18_nWipeTotalStages = g_15_nFadeWidth + 1;
        g_nBlendMaximumStep =
            (short)(duration < 320 ? duration : 320);
        g_18_nWipeAcrossStride = g_nBlendMaximumStep;
        g_16_nRevealVerticalDelta = g_nBlendMaximumStep;
        break;
    case 6:
        g_2_nWipePreviousAcrossEdge =
            g_3_stDissolveSourceViewport.right -
            g_3_stDissolveSourceViewport.left + 1;
        g_105_nFadePreviousVerticalEdge =
            g_3_stDissolveSourceViewport.bottom -
            g_3_stDissolveSourceViewport.top + 1;
        g_5_nWipeAcrossThreshold = g_15_nFadeWidth + 1;
        g_nRevealVertThreshold = g_5_nBlendHeight + 1;
        g_18_nWipeTotalStages = g_15_nFadeWidth + 1;
        g_nBlendMaximumStep =
            (short)(duration < 320 ? duration : 320);
        g_16_nRevealVerticalDelta = g_nBlendMaximumStep;
        g_18_nWipeAcrossStride = g_16_nRevealVerticalDelta;
        break;
    case 7:
        g_2_nWipePreviousAcrossEdge = -1;
        g_105_nFadePreviousVerticalEdge =
            g_3_stDissolveSourceViewport.bottom -
            g_3_stDissolveSourceViewport.top + 1;
        g_5_nWipeAcrossThreshold = g_15_nFadeWidth + 1;
        g_nRevealVertThreshold = g_5_nBlendHeight + 1;
        g_18_nWipeTotalStages = g_15_nFadeWidth + 1;
        g_nBlendMaximumStep =
            (short)(duration < 320 ? duration : 320);
        g_16_nRevealVerticalDelta = g_nBlendMaximumStep;
        g_18_nWipeAcrossStride = g_16_nRevealVerticalDelta;
        break;
    case 8:
        g_11_nWipeRadialAccumulator = 0;
        g_nWipeRadialRowCount = g_5_nBlendHeight / 2 + 1;
        g_11_pWipeFrontEdges =
            g_3_asWipeRadialLeadingEdge;
        g_9_pWipeRearEdges =
            g_asWipeRadialTrailingEdges;
        halfWidth = g_15_nFadeWidth;
        halfHeight = g_5_nBlendHeight;
        g_18_nWipeTotalStages = (short)(
            sqrt((double)(halfHeight * halfHeight +
                          halfWidth * halfWidth)) /
            2.0 + 1.0);
        g_1_nFadeRadialThreshold = g_18_nWipeTotalStages + 2;
        g_nBlendMaximumStep = (short)(
            g_18_nWipeTotalStages + 2 < duration
                ? g_18_nWipeTotalStages + 2
                : duration);
        g_18_nBlendRadialStep = g_nBlendMaximumStep;
        for (scanline = 0; scanline < 200; scanline++) {
            g_11_pWipeFrontEdges[scanline] = -1;
            g_9_pWipeRearEdges[scanline] = -1;
        }
        break;
    case 9:
        g_11_nWipeRadialAccumulator = 0;
        g_nWipeRadialRowCount = g_5_nBlendHeight / 2 + 1;
        g_11_pWipeFrontEdges =
            g_3_asWipeRadialLeadingEdge;
        g_9_pWipeRearEdges =
            g_asWipeRadialTrailingEdges;
        halfWidth = g_15_nFadeWidth;
        halfHeight = g_5_nBlendHeight;
        g_18_nWipeTotalStages = (short)(
            sqrt((double)(halfHeight * halfHeight +
                          halfWidth * halfWidth)) /
            2.0 + 1.0);
        g_1_nFadeRadialThreshold = g_18_nWipeTotalStages + 1;
        g_nBlendMaximumStep = (short)(
            g_18_nWipeTotalStages + 1 < duration
                ? g_18_nWipeTotalStages + 1
                : duration);
        g_18_nBlendRadialStep = g_nBlendMaximumStep;
        for (scanline = 0; scanline < g_nWipeRadialRowCount; scanline++) {
            g_11_pWipeFrontEdges[scanline] = 300;
            g_9_pWipeRearEdges[scanline] = 300;
        }
        break;
    }

    g_14_nBlendHorizontalTotal =
        g_18_nWipeAcrossStride >> 2;
    g_108_nTransitionVertSum =
        g_16_nRevealVerticalDelta >> 2;
    g_11_nWipeRadialAccumulator =
        g_18_nBlendRadialStep >> 2;
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

    switch (g_17_nFadeType) {
    case 0:
        verticalCount = 0;
        while (g_nRevealVertThreshold >
                   g_108_nTransitionVertSum &&
               g_18_nWipeTotalStages > g_139_nDissolveProgress) {
            g_108_nTransitionVertSum +=
                g_16_nRevealVerticalDelta;
            verticalCount++;
            g_139_nDissolveProgress++;
        }
        g_108_nTransitionVertSum -=
            g_nRevealVertThreshold;
        if ((g_3_nWipeRoundFlags & 1) != 0) {
            SetViewportVerticalBounds(
                &g_1_stWipeScratchSourcePort,
                g_1_stWipeScratchSourcePort.bottom + 1,
                g_1_stWipeScratchSourcePort.bottom +
                    verticalCount);
            SetViewportVerticalBounds(
                &g_26_stWipeTempDestinationViewport,
                g_26_stWipeTempDestinationViewport.bottom + 1,
                g_26_stWipeTempDestinationViewport.bottom +
                    verticalCount);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
        } else {
            OffsetViewportBounds(
                &g_1_stWipeScratchSourcePort,
                0, 1, 0, verticalCount);
            OffsetViewportBounds(
                &g_26_stWipeTempDestinationViewport,
                0, 1, 0, verticalCount);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
            OffsetViewportBounds(
                &g_1_stWipeScratchSourcePort,
                0, -1, 0, 0);
            OffsetViewportBounds(
                &g_26_stWipeTempDestinationViewport,
                0, -1, 0, 0);
        }
        break;
    case 1:
        horizontalCount = 0;
        while (g_5_nWipeAcrossThreshold >
                   g_14_nBlendHorizontalTotal &&
               g_18_nWipeTotalStages > g_139_nDissolveProgress) {
            g_14_nBlendHorizontalTotal +=
                g_18_nWipeAcrossStride;
            horizontalCount++;
            g_139_nDissolveProgress++;
        }
        g_14_nBlendHorizontalTotal -=
            g_5_nWipeAcrossThreshold;
        if ((g_3_nWipeRoundFlags & 1) != 0) {
            SetViewportHorizontalBounds(
                &g_1_stWipeScratchSourcePort,
                g_1_stWipeScratchSourcePort.left -
                    horizontalCount,
                g_1_stWipeScratchSourcePort.left - 1);
            SetViewportHorizontalBounds(
                &g_26_stWipeTempDestinationViewport,
                g_26_stWipeTempDestinationViewport.left -
                    horizontalCount,
                g_26_stWipeTempDestinationViewport.left - 1);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
        } else {
            OffsetViewportBounds(
                &g_1_stWipeScratchSourcePort,
                -horizontalCount, 0, -1, 0);
            OffsetViewportBounds(
                &g_26_stWipeTempDestinationViewport,
                -horizontalCount, 0, -1, 0);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
            OffsetViewportBounds(
                &g_1_stWipeScratchSourcePort,
                0, 0, 1, 0);
            OffsetViewportBounds(
                &g_26_stWipeTempDestinationViewport,
                0, 0, 1, 0);
        }
        break;
    case 2:
        verticalCount = 0;
        while (g_nRevealVertThreshold >
                   g_108_nTransitionVertSum &&
               g_18_nWipeTotalStages > g_139_nDissolveProgress) {
            g_108_nTransitionVertSum +=
                g_16_nRevealVerticalDelta;
            verticalCount++;
            g_139_nDissolveProgress++;
        }
        g_108_nTransitionVertSum -=
            g_nRevealVertThreshold;
        if ((g_3_nWipeRoundFlags & 1) != 0) {
            SetViewportVerticalBounds(
                &g_1_stWipeScratchSourcePort,
                g_1_stWipeScratchSourcePort.top - verticalCount,
                g_1_stWipeScratchSourcePort.top - 1);
            SetViewportVerticalBounds(
                &g_26_stWipeTempDestinationViewport,
                g_26_stWipeTempDestinationViewport.top -
                    verticalCount,
                g_26_stWipeTempDestinationViewport.top - 1);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
        } else {
            OffsetViewportBounds(
                &g_1_stWipeScratchSourcePort,
                0, -verticalCount, 0, -1);
            OffsetViewportBounds(
                &g_26_stWipeTempDestinationViewport,
                0, -verticalCount, 0, -1);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
            OffsetViewportBounds(
                &g_1_stWipeScratchSourcePort,
                0, 0, 0, 1);
            OffsetViewportBounds(
                &g_26_stWipeTempDestinationViewport,
                0, 0, 0, 1);
        }
        break;
    case 3:
        horizontalCount = 0;
        while (g_5_nWipeAcrossThreshold >
                   g_14_nBlendHorizontalTotal &&
               g_18_nWipeTotalStages > g_139_nDissolveProgress) {
            g_14_nBlendHorizontalTotal +=
                g_18_nWipeAcrossStride;
            horizontalCount++;
            g_139_nDissolveProgress++;
        }
        g_14_nBlendHorizontalTotal -=
            g_5_nWipeAcrossThreshold;
        if ((g_3_nWipeRoundFlags & 1) != 0) {
            SetViewportHorizontalBounds(
                &g_1_stWipeScratchSourcePort,
                g_1_stWipeScratchSourcePort.right + 1,
                g_1_stWipeScratchSourcePort.right +
                    horizontalCount);
            SetViewportHorizontalBounds(
                &g_26_stWipeTempDestinationViewport,
                g_26_stWipeTempDestinationViewport.right + 1,
                g_26_stWipeTempDestinationViewport.right +
                    horizontalCount);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
        } else {
            OffsetViewportBounds(
                &g_1_stWipeScratchSourcePort,
                1, 0, horizontalCount, 0);
            OffsetViewportBounds(
                &g_26_stWipeTempDestinationViewport,
                1, 0, horizontalCount, 0);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
            OffsetViewportBounds(
                &g_1_stWipeScratchSourcePort,
                -1, 0, 0, 0);
            OffsetViewportBounds(
                &g_26_stWipeTempDestinationViewport,
                -1, 0, 0, 0);
        }
        break;
    case 4:
        verticalCount = 0;
        horizontalCount = verticalCount;
        while (g_5_nWipeAcrossThreshold >
                   g_14_nBlendHorizontalTotal &&
               g_18_nWipeTotalStages > g_139_nDissolveProgress) {
            g_14_nBlendHorizontalTotal +=
                g_18_nWipeAcrossStride;
            horizontalCount++;
            g_139_nDissolveProgress++;
        }
        g_14_nBlendHorizontalTotal -=
            g_5_nWipeAcrossThreshold;
        while (g_nRevealVertThreshold >
               g_108_nTransitionVertSum) {
            g_108_nTransitionVertSum +=
                g_16_nRevealVerticalDelta;
            verticalCount++;
        }
        g_108_nTransitionVertSum -=
            g_nRevealVertThreshold;
        g_3_nFadeCurrentHorizontalEdge =
            horizontalCount + g_2_nWipePreviousAcrossEdge;
        g_nWipeCurrentVerticalEdges =
            verticalCount + g_105_nFadePreviousVerticalEdge;
        if ((g_3_nWipeRoundFlags & 1) != 0) {
            if (g_105_nFadePreviousVerticalEdge >= 0 &&
                horizontalCount != 0) {
                SetViewportRectangleBounds(
                    &g_1_stWipeScratchSourcePort,
                    g_3_stDissolveSourceViewport.left +
                        g_2_nWipePreviousAcrossEdge + 1,
                    g_3_stDissolveSourceViewport.top,
                    g_3_stDissolveSourceViewport.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_3_stDissolveSourceViewport.top +
                        g_105_nFadePreviousVerticalEdge);
                SetViewportRectangleBounds(
                    &g_26_stWipeTempDestinationViewport,
                    g_19_stFadeToView.left +
                        g_2_nWipePreviousAcrossEdge + 1,
                    g_19_stFadeToView.top,
                    g_19_stFadeToView.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_19_stFadeToView.top +
                        g_105_nFadePreviousVerticalEdge);
                CopyViewportContents(
                    &g_1_stWipeScratchSourcePort,
                    &g_26_stWipeTempDestinationViewport);
            }
            if (verticalCount != 0) {
                SetViewportRectangleBounds(
                    &g_1_stWipeScratchSourcePort,
                    g_3_stDissolveSourceViewport.left,
                    g_3_stDissolveSourceViewport.top +
                        g_105_nFadePreviousVerticalEdge + 1,
                    g_3_stDissolveSourceViewport.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_3_stDissolveSourceViewport.top +
                        g_nWipeCurrentVerticalEdges);
                SetViewportRectangleBounds(
                    &g_26_stWipeTempDestinationViewport,
                    g_19_stFadeToView.left,
                    g_19_stFadeToView.top +
                        g_105_nFadePreviousVerticalEdge + 1,
                    g_19_stFadeToView.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_19_stFadeToView.top +
                        g_nWipeCurrentVerticalEdges);
                CopyViewportContents(
                    &g_1_stWipeScratchSourcePort,
                    &g_26_stWipeTempDestinationViewport);
            }
        } else {
            SetViewportRectangleBounds(
                &g_1_stWipeScratchSourcePort,
                g_3_stDissolveSourceViewport.left,
                g_3_stDissolveSourceViewport.top,
                g_3_stDissolveSourceViewport.left +
                    g_3_nFadeCurrentHorizontalEdge,
                g_3_stDissolveSourceViewport.top +
                    g_nWipeCurrentVerticalEdges);
            SetViewportRectangleBounds(
                &g_26_stWipeTempDestinationViewport,
                g_19_stFadeToView.left,
                g_19_stFadeToView.top,
                g_19_stFadeToView.left +
                    g_3_nFadeCurrentHorizontalEdge,
                g_19_stFadeToView.top +
                    g_nWipeCurrentVerticalEdges);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
        }
        break;
    case 5:
        verticalCount = 0;
        horizontalCount = verticalCount;
        while (g_5_nWipeAcrossThreshold >
                   g_14_nBlendHorizontalTotal &&
               g_18_nWipeTotalStages > g_139_nDissolveProgress) {
            g_14_nBlendHorizontalTotal +=
                g_18_nWipeAcrossStride;
            horizontalCount++;
            g_139_nDissolveProgress++;
        }
        g_14_nBlendHorizontalTotal -=
            g_5_nWipeAcrossThreshold;
        while (g_nRevealVertThreshold >
               g_108_nTransitionVertSum) {
            g_108_nTransitionVertSum +=
                g_16_nRevealVerticalDelta;
            verticalCount++;
        }
        g_108_nTransitionVertSum -=
            g_nRevealVertThreshold;
        g_3_nFadeCurrentHorizontalEdge =
            g_2_nWipePreviousAcrossEdge - horizontalCount;
        g_nWipeCurrentVerticalEdges =
            verticalCount + g_105_nFadePreviousVerticalEdge;
        if ((g_3_nWipeRoundFlags & 1) != 0) {
            if (g_105_nFadePreviousVerticalEdge >= 0 &&
                horizontalCount != 0) {
                SetViewportRectangleBounds(
                    &g_1_stWipeScratchSourcePort,
                    g_3_stDissolveSourceViewport.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_3_stDissolveSourceViewport.top,
                    g_3_stDissolveSourceViewport.left +
                        g_2_nWipePreviousAcrossEdge - 1,
                    g_3_stDissolveSourceViewport.top +
                        g_105_nFadePreviousVerticalEdge);
                SetViewportRectangleBounds(
                    &g_26_stWipeTempDestinationViewport,
                    g_19_stFadeToView.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_19_stFadeToView.top,
                    g_19_stFadeToView.left +
                        g_2_nWipePreviousAcrossEdge - 1,
                    g_19_stFadeToView.top +
                        g_105_nFadePreviousVerticalEdge);
                CopyViewportContents(
                    &g_1_stWipeScratchSourcePort,
                    &g_26_stWipeTempDestinationViewport);
            }
            if (verticalCount != 0) {
                SetViewportRectangleBounds(
                    &g_1_stWipeScratchSourcePort,
                    g_3_stDissolveSourceViewport.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_3_stDissolveSourceViewport.top +
                        g_105_nFadePreviousVerticalEdge + 1,
                    g_3_stDissolveSourceViewport.right,
                    g_3_stDissolveSourceViewport.top +
                        g_nWipeCurrentVerticalEdges);
                SetViewportRectangleBounds(
                    &g_26_stWipeTempDestinationViewport,
                    g_19_stFadeToView.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_19_stFadeToView.top +
                        g_105_nFadePreviousVerticalEdge + 1,
                    g_19_stFadeToView.right,
                    g_19_stFadeToView.top +
                        g_nWipeCurrentVerticalEdges);
                CopyViewportContents(
                    &g_1_stWipeScratchSourcePort,
                    &g_26_stWipeTempDestinationViewport);
            }
        } else {
            SetViewportRectangleBounds(
                &g_1_stWipeScratchSourcePort,
                g_3_stDissolveSourceViewport.left +
                    g_3_nFadeCurrentHorizontalEdge,
                g_3_stDissolveSourceViewport.top,
                g_3_stDissolveSourceViewport.right,
                g_3_stDissolveSourceViewport.top +
                    g_nWipeCurrentVerticalEdges);
            SetViewportRectangleBounds(
                &g_26_stWipeTempDestinationViewport,
                g_19_stFadeToView.left +
                    g_3_nFadeCurrentHorizontalEdge,
                g_19_stFadeToView.top,
                g_19_stFadeToView.right,
                g_19_stFadeToView.top +
                    g_nWipeCurrentVerticalEdges);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
        }
        break;
    case 6:
        verticalCount = 0;
        horizontalCount = verticalCount;
        while (g_5_nWipeAcrossThreshold >
                   g_14_nBlendHorizontalTotal &&
               g_18_nWipeTotalStages > g_139_nDissolveProgress) {
            g_14_nBlendHorizontalTotal +=
                g_18_nWipeAcrossStride;
            horizontalCount++;
            g_139_nDissolveProgress++;
        }
        g_14_nBlendHorizontalTotal -=
            g_5_nWipeAcrossThreshold;
        while (g_nRevealVertThreshold >
               g_108_nTransitionVertSum) {
            g_108_nTransitionVertSum +=
                g_16_nRevealVerticalDelta;
            verticalCount++;
        }
        g_108_nTransitionVertSum -=
            g_nRevealVertThreshold;
        g_3_nFadeCurrentHorizontalEdge =
            g_2_nWipePreviousAcrossEdge - horizontalCount;
        g_nWipeCurrentVerticalEdges =
            g_105_nFadePreviousVerticalEdge - verticalCount;
        if ((g_3_nWipeRoundFlags & 1) != 0) {
            if (g_105_nFadePreviousVerticalEdge <=
                    g_3_stDissolveSourceViewport.bottom -
                        g_3_stDissolveSourceViewport.top &&
                horizontalCount != 0) {
                SetViewportRectangleBounds(
                    &g_1_stWipeScratchSourcePort,
                    g_3_stDissolveSourceViewport.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_3_stDissolveSourceViewport.top +
                        g_105_nFadePreviousVerticalEdge,
                    g_3_stDissolveSourceViewport.left +
                        g_2_nWipePreviousAcrossEdge - 1,
                    g_3_stDissolveSourceViewport.bottom);
                SetViewportRectangleBounds(
                    &g_26_stWipeTempDestinationViewport,
                    g_19_stFadeToView.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_19_stFadeToView.top +
                        g_105_nFadePreviousVerticalEdge,
                    g_19_stFadeToView.left +
                        g_2_nWipePreviousAcrossEdge - 1,
                    g_19_stFadeToView.bottom);
                CopyViewportContents(
                    &g_1_stWipeScratchSourcePort,
                    &g_26_stWipeTempDestinationViewport);
            }
            if (verticalCount != 0) {
                SetViewportRectangleBounds(
                    &g_1_stWipeScratchSourcePort,
                    g_3_stDissolveSourceViewport.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_3_stDissolveSourceViewport.top +
                        g_nWipeCurrentVerticalEdges,
                    g_3_stDissolveSourceViewport.right,
                    g_3_stDissolveSourceViewport.top +
                        g_105_nFadePreviousVerticalEdge - 1);
                SetViewportRectangleBounds(
                    &g_26_stWipeTempDestinationViewport,
                    g_19_stFadeToView.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_19_stFadeToView.top +
                        g_nWipeCurrentVerticalEdges,
                    g_19_stFadeToView.right,
                    g_19_stFadeToView.top +
                        g_105_nFadePreviousVerticalEdge - 1);
                CopyViewportContents(
                    &g_1_stWipeScratchSourcePort,
                    &g_26_stWipeTempDestinationViewport);
            }
        } else {
            SetViewportRectangleBounds(
                &g_1_stWipeScratchSourcePort,
                g_3_stDissolveSourceViewport.left +
                    g_3_nFadeCurrentHorizontalEdge,
                g_3_stDissolveSourceViewport.top +
                    g_nWipeCurrentVerticalEdges,
                g_3_stDissolveSourceViewport.right,
                g_3_stDissolveSourceViewport.bottom);
            SetViewportRectangleBounds(
                &g_26_stWipeTempDestinationViewport,
                g_19_stFadeToView.left +
                    g_3_nFadeCurrentHorizontalEdge,
                g_19_stFadeToView.top +
                    g_nWipeCurrentVerticalEdges,
                g_19_stFadeToView.right,
                g_19_stFadeToView.bottom);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
        }
        break;
    case 7:
        verticalCount = 0;
        horizontalCount = verticalCount;
        while (g_5_nWipeAcrossThreshold >
                   g_14_nBlendHorizontalTotal &&
               g_18_nWipeTotalStages > g_139_nDissolveProgress) {
            g_14_nBlendHorizontalTotal +=
                g_18_nWipeAcrossStride;
            horizontalCount++;
            g_139_nDissolveProgress++;
        }
        g_14_nBlendHorizontalTotal -=
            g_5_nWipeAcrossThreshold;
        while (g_nRevealVertThreshold >
               g_108_nTransitionVertSum) {
            g_108_nTransitionVertSum +=
                g_16_nRevealVerticalDelta;
            verticalCount++;
        }
        g_108_nTransitionVertSum -=
            g_nRevealVertThreshold;
        g_3_nFadeCurrentHorizontalEdge =
            horizontalCount + g_2_nWipePreviousAcrossEdge;
        g_nWipeCurrentVerticalEdges =
            g_105_nFadePreviousVerticalEdge - verticalCount;
        if ((g_3_nWipeRoundFlags & 1) != 0) {
            if (g_105_nFadePreviousVerticalEdge <=
                    g_3_stDissolveSourceViewport.bottom -
                        g_3_stDissolveSourceViewport.top &&
                horizontalCount != 0) {
                SetViewportRectangleBounds(
                    &g_1_stWipeScratchSourcePort,
                    g_3_stDissolveSourceViewport.left +
                        g_2_nWipePreviousAcrossEdge + 1,
                    g_3_stDissolveSourceViewport.top +
                        g_105_nFadePreviousVerticalEdge,
                    g_3_stDissolveSourceViewport.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_3_stDissolveSourceViewport.bottom);
                SetViewportRectangleBounds(
                    &g_26_stWipeTempDestinationViewport,
                    g_19_stFadeToView.left +
                        g_2_nWipePreviousAcrossEdge + 1,
                    g_19_stFadeToView.top +
                        g_105_nFadePreviousVerticalEdge,
                    g_19_stFadeToView.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_19_stFadeToView.bottom);
                CopyViewportContents(
                    &g_1_stWipeScratchSourcePort,
                    &g_26_stWipeTempDestinationViewport);
            }
            if (verticalCount != 0) {
                SetViewportRectangleBounds(
                    &g_1_stWipeScratchSourcePort,
                    g_3_stDissolveSourceViewport.left,
                    g_3_stDissolveSourceViewport.top +
                        g_nWipeCurrentVerticalEdges,
                    g_3_stDissolveSourceViewport.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_3_stDissolveSourceViewport.top +
                        g_105_nFadePreviousVerticalEdge - 1);
                SetViewportRectangleBounds(
                    &g_26_stWipeTempDestinationViewport,
                    g_19_stFadeToView.left,
                    g_19_stFadeToView.top +
                        g_nWipeCurrentVerticalEdges,
                    g_19_stFadeToView.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_19_stFadeToView.top +
                        g_105_nFadePreviousVerticalEdge - 1);
                CopyViewportContents(
                    &g_1_stWipeScratchSourcePort,
                    &g_26_stWipeTempDestinationViewport);
            }
        } else {
            SetViewportRectangleBounds(
                &g_1_stWipeScratchSourcePort,
                g_3_stDissolveSourceViewport.left,
                g_3_stDissolveSourceViewport.top +
                    g_nWipeCurrentVerticalEdges,
                g_3_stDissolveSourceViewport.left +
                    g_3_nFadeCurrentHorizontalEdge,
                g_3_stDissolveSourceViewport.bottom);
            SetViewportRectangleBounds(
                &g_26_stWipeTempDestinationViewport,
                g_19_stFadeToView.left,
                g_19_stFadeToView.top +
                    g_nWipeCurrentVerticalEdges,
                g_19_stFadeToView.left +
                    g_3_nFadeCurrentHorizontalEdge,
                g_19_stFadeToView.bottom);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
        }
        break;
    case 8:
        while (g_1_nFadeRadialThreshold >
                   g_11_nWipeRadialAccumulator &&
               g_18_nWipeTotalStages > g_139_nDissolveProgress) {
            g_11_nWipeRadialAccumulator +=
                g_18_nBlendRadialStep;
            g_139_nDissolveProgress++;
        }
        g_11_nWipeRadialAccumulator -=
            g_1_nFadeRadialThreshold;
        GenerateWipeRadialEdges(g_11_pWipeFrontEdges,
                                g_139_nDissolveProgress);
        BlitWipeRadialBands(
            &g_1_stWipeScratchSourcePort,
            &g_26_stWipeTempDestinationViewport,
            g_9_pWipeRearEdges,
            g_11_pWipeFrontEdges);
        if ((g_3_nWipeRoundFlags & 1) != 0) {
            edgeSwap = g_9_pWipeRearEdges;
            g_9_pWipeRearEdges = g_11_pWipeFrontEdges;
            g_11_pWipeFrontEdges = edgeSwap;
        }
        break;
    case 9:
        while (g_1_nFadeRadialThreshold >
                   g_11_nWipeRadialAccumulator &&
               g_18_nWipeTotalStages > g_139_nDissolveProgress) {
            g_11_nWipeRadialAccumulator +=
                g_18_nBlendRadialStep;
            g_139_nDissolveProgress++;
        }
        g_11_nWipeRadialAccumulator -=
            g_1_nFadeRadialThreshold;
        GenerateWipeRadialEdges(
            g_9_pWipeRearEdges,
            g_18_nWipeTotalStages - g_139_nDissolveProgress);
        BlitWipeRadialBands(
            &g_1_stWipeScratchSourcePort,
            &g_26_stWipeTempDestinationViewport,
            g_9_pWipeRearEdges,
            g_11_pWipeFrontEdges);
        if ((g_3_nWipeRoundFlags & 1) != 0) {
            edgeSwap = g_9_pWipeRearEdges;
            g_9_pWipeRearEdges = g_11_pWipeFrontEdges;
            g_11_pWipeFrontEdges = edgeSwap;
        }
        break;
    }

    g_2_nWipePreviousAcrossEdge =
        g_3_nFadeCurrentHorizontalEdge;
    g_105_nFadePreviousVerticalEdge =
        g_nWipeCurrentVerticalEdges;
    (void)workspace;
    if (g_18_nWipeTotalStages == g_139_nDissolveProgress)
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
    if (!g_3_nWipeRoundFlags & 1) {
        CopyViewportContents(&g_3_stDissolveSourceViewport,
                             &g_19_stFadeToView);
    } else {
        switch (g_17_nFadeType) {
        case 0:
            height = g_18_nWipeTotalStages -
                g_139_nDissolveProgress;
            SetViewportVerticalBounds(
                &g_1_stWipeScratchSourcePort,
                g_1_stWipeScratchSourcePort.bottom + 1,
                g_1_stWipeScratchSourcePort.bottom + height);
            SetViewportVerticalBounds(
                &g_26_stWipeTempDestinationViewport,
                g_26_stWipeTempDestinationViewport.bottom + 1,
                g_26_stWipeTempDestinationViewport.bottom +
                    height);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
            break;
        case 1:
            width = g_18_nWipeTotalStages -
                g_139_nDissolveProgress;
            SetViewportHorizontalBounds(
                &g_1_stWipeScratchSourcePort,
                g_1_stWipeScratchSourcePort.left - width,
                g_1_stWipeScratchSourcePort.left - 1);
            SetViewportHorizontalBounds(
                &g_26_stWipeTempDestinationViewport,
                g_26_stWipeTempDestinationViewport.left - width,
                g_26_stWipeTempDestinationViewport.left - 1);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
            break;
        case 2:
            height = g_18_nWipeTotalStages -
                g_139_nDissolveProgress;
            SetViewportVerticalBounds(
                &g_1_stWipeScratchSourcePort,
                g_1_stWipeScratchSourcePort.top - height,
                g_1_stWipeScratchSourcePort.top - 1);
            SetViewportVerticalBounds(
                &g_26_stWipeTempDestinationViewport,
                g_26_stWipeTempDestinationViewport.top - height,
                g_26_stWipeTempDestinationViewport.top - 1);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
            break;
        case 3:
            width = g_18_nWipeTotalStages -
                g_139_nDissolveProgress;
            SetViewportHorizontalBounds(
                &g_1_stWipeScratchSourcePort,
                g_1_stWipeScratchSourcePort.right + 1,
                g_1_stWipeScratchSourcePort.right + width);
            SetViewportHorizontalBounds(
                &g_26_stWipeTempDestinationViewport,
                g_26_stWipeTempDestinationViewport.right + 1,
                g_26_stWipeTempDestinationViewport.right + width);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
            break;
        case 4:
            g_3_nFadeCurrentHorizontalEdge =
                g_3_stDissolveSourceViewport.right -
                g_3_stDissolveSourceViewport.left;
            g_nWipeCurrentVerticalEdges =
                g_3_stDissolveSourceViewport.bottom -
                g_3_stDissolveSourceViewport.top;
            if (g_105_nFadePreviousVerticalEdge >= 0) {
                SetViewportRectangleBounds(
                    &g_1_stWipeScratchSourcePort,
                    g_3_stDissolveSourceViewport.left +
                        g_2_nWipePreviousAcrossEdge + 1,
                    g_3_stDissolveSourceViewport.top,
                    g_3_stDissolveSourceViewport.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_3_stDissolveSourceViewport.top +
                        g_105_nFadePreviousVerticalEdge);
                SetViewportRectangleBounds(
                    &g_26_stWipeTempDestinationViewport,
                    g_19_stFadeToView.left +
                        g_2_nWipePreviousAcrossEdge + 1,
                    g_19_stFadeToView.top,
                    g_19_stFadeToView.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_19_stFadeToView.top +
                        g_105_nFadePreviousVerticalEdge);
                CopyViewportContents(
                    &g_1_stWipeScratchSourcePort,
                    &g_26_stWipeTempDestinationViewport);
            }
            SetViewportRectangleBounds(
                &g_1_stWipeScratchSourcePort,
                g_3_stDissolveSourceViewport.left,
                g_3_stDissolveSourceViewport.top +
                    g_105_nFadePreviousVerticalEdge + 1,
                g_3_stDissolveSourceViewport.left +
                    g_3_nFadeCurrentHorizontalEdge,
                g_3_stDissolveSourceViewport.top +
                    g_nWipeCurrentVerticalEdges);
            SetViewportRectangleBounds(
                &g_26_stWipeTempDestinationViewport,
                g_19_stFadeToView.left,
                g_19_stFadeToView.top +
                    g_105_nFadePreviousVerticalEdge + 1,
                g_19_stFadeToView.left +
                    g_3_nFadeCurrentHorizontalEdge,
                g_19_stFadeToView.top +
                    g_nWipeCurrentVerticalEdges);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
            break;
        case 5:
            g_3_nFadeCurrentHorizontalEdge = 0;
            g_nWipeCurrentVerticalEdges =
                g_3_stDissolveSourceViewport.bottom -
                g_3_stDissolveSourceViewport.top;
            if (g_105_nFadePreviousVerticalEdge >= 0) {
                SetViewportRectangleBounds(
                    &g_1_stWipeScratchSourcePort,
                    g_3_stDissolveSourceViewport.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_3_stDissolveSourceViewport.top,
                    g_3_stDissolveSourceViewport.left +
                        g_2_nWipePreviousAcrossEdge - 1,
                    g_3_stDissolveSourceViewport.top +
                        g_105_nFadePreviousVerticalEdge);
                SetViewportRectangleBounds(
                    &g_26_stWipeTempDestinationViewport,
                    g_19_stFadeToView.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_19_stFadeToView.top,
                    g_19_stFadeToView.left +
                        g_2_nWipePreviousAcrossEdge - 1,
                    g_19_stFadeToView.top +
                        g_105_nFadePreviousVerticalEdge);
                CopyViewportContents(
                    &g_1_stWipeScratchSourcePort,
                    &g_26_stWipeTempDestinationViewport);
            }
            SetViewportRectangleBounds(
                &g_1_stWipeScratchSourcePort,
                g_3_stDissolveSourceViewport.left +
                    g_3_nFadeCurrentHorizontalEdge,
                g_3_stDissolveSourceViewport.top +
                    g_105_nFadePreviousVerticalEdge + 1,
                g_3_stDissolveSourceViewport.right,
                g_3_stDissolveSourceViewport.top +
                    g_nWipeCurrentVerticalEdges);
            SetViewportRectangleBounds(
                &g_26_stWipeTempDestinationViewport,
                g_19_stFadeToView.left +
                    g_3_nFadeCurrentHorizontalEdge,
                g_19_stFadeToView.top +
                    g_105_nFadePreviousVerticalEdge + 1,
                g_19_stFadeToView.right,
                g_19_stFadeToView.top +
                    g_nWipeCurrentVerticalEdges);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
            break;
        case 6:
            g_3_nFadeCurrentHorizontalEdge = 0;
            g_nWipeCurrentVerticalEdges = 0;
            if (g_105_nFadePreviousVerticalEdge >= 0) {
                SetViewportRectangleBounds(
                    &g_1_stWipeScratchSourcePort,
                    g_3_stDissolveSourceViewport.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_3_stDissolveSourceViewport.top +
                        g_105_nFadePreviousVerticalEdge,
                    g_3_stDissolveSourceViewport.left +
                        g_2_nWipePreviousAcrossEdge - 1,
                    g_3_stDissolveSourceViewport.bottom);
                SetViewportRectangleBounds(
                    &g_26_stWipeTempDestinationViewport,
                    g_19_stFadeToView.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_19_stFadeToView.top +
                        g_105_nFadePreviousVerticalEdge,
                    g_19_stFadeToView.left +
                        g_2_nWipePreviousAcrossEdge - 1,
                    g_19_stFadeToView.bottom);
                CopyViewportContents(
                    &g_1_stWipeScratchSourcePort,
                    &g_26_stWipeTempDestinationViewport);
            }
            SetViewportRectangleBounds(
                &g_1_stWipeScratchSourcePort,
                g_3_stDissolveSourceViewport.left +
                    g_3_nFadeCurrentHorizontalEdge,
                g_3_stDissolveSourceViewport.top +
                    g_nWipeCurrentVerticalEdges,
                g_3_stDissolveSourceViewport.right,
                g_3_stDissolveSourceViewport.top +
                    g_105_nFadePreviousVerticalEdge + 1);
            SetViewportRectangleBounds(
                &g_26_stWipeTempDestinationViewport,
                g_19_stFadeToView.left +
                    g_3_nFadeCurrentHorizontalEdge,
                g_19_stFadeToView.top +
                    g_nWipeCurrentVerticalEdges,
                g_19_stFadeToView.right,
                g_19_stFadeToView.top +
                    g_105_nFadePreviousVerticalEdge + 1);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
            break;
        case 7:
            g_3_nFadeCurrentHorizontalEdge =
                g_3_stDissolveSourceViewport.right -
                g_3_stDissolveSourceViewport.left;
            g_nWipeCurrentVerticalEdges = 0;
            if (g_105_nFadePreviousVerticalEdge >= 0) {
                SetViewportRectangleBounds(
                    &g_1_stWipeScratchSourcePort,
                    g_3_stDissolveSourceViewport.left +
                        g_2_nWipePreviousAcrossEdge + 1,
                    g_3_stDissolveSourceViewport.top +
                        g_105_nFadePreviousVerticalEdge,
                    g_3_stDissolveSourceViewport.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_3_stDissolveSourceViewport.bottom);
                SetViewportRectangleBounds(
                    &g_26_stWipeTempDestinationViewport,
                    g_19_stFadeToView.left +
                        g_2_nWipePreviousAcrossEdge + 1,
                    g_19_stFadeToView.top +
                        g_105_nFadePreviousVerticalEdge,
                    g_19_stFadeToView.left +
                        g_3_nFadeCurrentHorizontalEdge,
                    g_19_stFadeToView.bottom);
                CopyViewportContents(
                    &g_1_stWipeScratchSourcePort,
                    &g_26_stWipeTempDestinationViewport);
            }
            SetViewportRectangleBounds(
                &g_1_stWipeScratchSourcePort,
                g_3_stDissolveSourceViewport.left,
                g_3_stDissolveSourceViewport.top +
                    g_nWipeCurrentVerticalEdges,
                g_3_stDissolveSourceViewport.left +
                    g_3_nFadeCurrentHorizontalEdge,
                g_3_stDissolveSourceViewport.top +
                    g_105_nFadePreviousVerticalEdge + 1);
            SetViewportRectangleBounds(
                &g_26_stWipeTempDestinationViewport,
                g_19_stFadeToView.left,
                g_19_stFadeToView.top +
                    g_nWipeCurrentVerticalEdges,
                g_19_stFadeToView.left +
                    g_3_nFadeCurrentHorizontalEdge,
                g_19_stFadeToView.top +
                    g_105_nFadePreviousVerticalEdge + 1);
            CopyViewportContents(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport);
            break;
        case 8:
            g_139_nDissolveProgress = g_18_nWipeTotalStages;
            GenerateWipeRadialEdges(g_11_pWipeFrontEdges,
                                    g_139_nDissolveProgress);
            BlitWipeRadialBands(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport,
                g_9_pWipeRearEdges,
                g_11_pWipeFrontEdges);
            break;
        case 9:
            g_139_nDissolveProgress = g_18_nWipeTotalStages;
            GenerateWipeRadialEdges(
                g_9_pWipeRearEdges,
                g_18_nWipeTotalStages - g_139_nDissolveProgress);
            BlitWipeRadialBands(
                &g_1_stWipeScratchSourcePort,
                &g_26_stWipeTempDestinationViewport,
                g_9_pWipeRearEdges,
                g_11_pWipeFrontEdges);
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
    if (g_nWipeRadialRowCount > y)
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
            if (g_nWipeRadialRowCount > y)
                edges[y] = x;
            lastY = y;
        }
    }

    if (radius == 0)
        edges[0] = -1;
    for (blankRow = (short)(radius + 1);
         g_nWipeRadialRowCount > blankRow;
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
short g_nWipeRadialRowCount; /* 0x005d17b4 */
