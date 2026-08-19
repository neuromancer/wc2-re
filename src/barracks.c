/*
 *  Barracks and campaign save/load room.
 *
 *  Address range 0x41ada0-0x41c75f (provisional -- see docs/ORDER.md).
 *  Boundary evidence: contiguous SAVEGAME.WLD helpers ending at BarracksScreen;
 *  the Win32 debug-overlay compilation unit begins at 0x41c760.
 */
#include "wc1.h"

/* Function start: 0x46138D */
void DrawUnreferencedPilotHandFrame(Viewport *viewport, unsigned char *shape,
                       BarracksAnimationState *state)
{
    short x;
    short y;

    x = (short)(g_stPilotHandOrigin_0049af90.x -
                g_stCockpitViewport_005d2160.left);
    y = (short)(g_stPilotHandOrigin_0049af90.y -
                g_stCockpitViewport_005d2160.top);
    CopyViewportContents(&g_stPilotHandBackgroundViewport_005d2b40,
                         &g_stPilotHandViewport_005d2c70);
    DrawSpriteDefault(&g_stPilotHandViewport_005d2c70, x, y,
                      g_pPilotHandAnimationShape_005d2c64,
                      (short)(signed char)g_cPilotHandFrame_005d1c30);
    DrawSpriteDefault(
        &g_stPilotHandViewport_005d2c70,
        (short)(x + g_asPilotHandOffsets_0049aff8[
            (signed char)g_cPilotHandFrame_005d1c30 * 2]),
        (short)(y + g_asPilotHandOffsets_0049aff8[
            (signed char)g_cPilotHandFrame_005d1c30 * 2 + 1]),
        g_pPilotHandAnimationShape_005d2c64, 0x11);
    CopyViewportContents(&g_stPilotHandViewport_005d2c70,
                         &g_stCockpitViewport_005d2160);
    g_cLastPilotHandFrame_0049aff4 = g_cPilotHandFrame_005d1c30;
}

/* Function start: 0x418DAA */
short RunCampaignSelectionFrame(void *scene, void *field)
{
    short selection;

    selection = 0;
    do {
        selection = PollSceneHotspotInput(scene, 0, 0, 0, 0);
        if (selection == 0) {
            DisableMouseCursorDrawing();
            DrawSpriteDefault(
                &g_stSecondaryViewBuffer_005d2c90, 0, 0, field, 0);
            DrawConstellationField();
            DrawSpriteDefault(
                &g_stSecondaryViewBuffer_005d2c90, 0, 0, scene, 0);
            if (g_pszPersonnelFooter_00492658 != 0) {
                g_stDefaultTextContext_005d2d20.viewport =
                    &g_stSecondaryViewBuffer_005d2c90;
                InitializeTextContextFromFont(
                    &g_stDefaultTextContext_005d2d20, 1,
                    g_bPrimaryViewBufferColour_0049cb50, -1);
                SetTextCursor(
                    (unsigned short)((320 -
                        MeasureTextPixelWidthClamped(
                            g_pszPersonnelFooter_00492658)) >> 1),
                    180);
                DrawFormattedText("%S", g_pszPersonnelFooter_00492658);
                g_stDefaultTextContext_005d2d20.viewport =
                    &g_stScreenViewport_005d21a0;
            }
            EnableMouseCursorDrawing();
            WaitForVerticalBlankThunk();
            CopyViewportContents(&g_stSecondaryViewBuffer_005d2c90,
                                 &g_stScreenViewport_005d21a0);
        }
    } while (selection == 0);
    if (g_bSceneEscapeRequested_0049d4b0 != 0)
        selection = 0;
    return selection;
}

/* Function start: 0x419831 */
short BarracksScreen(void)
{
    signed char selection;
    void *campaignScene;
    void *field;

    selection = 0;
    campaignScene = 0;
    field = 0;
    DisableMouseCursorDrawing();
    ClearViewport(&g_stModalSourceViewport_005d2c50,
                  g_cSecondaryViewBufferColour_0049cb4c);
    g_stSecondaryViewBuffer_005d2c90.left = 0;
    g_stSecondaryViewBuffer_005d2c90.right = 319;
    g_stSecondaryViewBuffer_005d2c90.top = 0;
    g_stSecondaryViewBuffer_005d2c90.bottom = 199;
    if ((short)AllocateViewport(
            &g_stSecondaryViewBuffer_005d2c90,
            g_cSecondaryViewBufferColour_0049cb4c, 0) == 0) {
        ReportFatalErrorCode("033");
    }
    ClearViewport(&g_stSecondaryViewBuffer_005d2c90, 0);
    campaignScene = LoadNamedPacket(
        "campaign.vga", 0, 0, 0, 0, 1);
    if (campaignScene != 0 && WaitForSceneAdvance(campaignScene) != 0) {
        field = FetchDiskPacketRetrying("field.v00", 1, 0);
        init_constellation(0);
        InitializeConstellationField(
            &g_stSecondaryViewBuffer_005d2c90, 0, 16);
        SetMenuInputPump();
        SetInputViewport(&g_stSecondaryViewBuffer_005d2c90);
        SetPersonnelMousePosition(159, 99);
        EnableMouseCursorDrawing();
        while (selection == 0) {
            selection = (signed char)RunCampaignSelectionFrame(
                campaignScene, field);
        }
        FreePacketAndClear(&g_pConstellationShape_005d2c4c, 0);
        FreePacketAndClear(&campaignScene, 0);
        FreePacketAndClear(&field, 0);
        free_viewport(&g_stSecondaryViewBuffer_005d2c90);
        return (short)(selection - 1);
    }
    FreePacketAndClear(&campaignScene, 0);
    free_viewport(&g_stSecondaryViewBuffer_005d2c90);
    return 0;
}

/* Function start: 0x4225A0 */
short StepPaletteTransition(short *current,
                            const short *target,
                            short componentCount)
{
    short difference;
    short index;
    short previousCountdown;

    if (g_nPaletteTransitionInitialise_0049305c != 0) {
        g_pPaletteTransitionAccumulator_005d301c =
            AllocateTaggedMemory((unsigned int)(componentCount * 2), 0);
        g_pPaletteTransitionDelta_005d3014 =
            AllocateTaggedMemory((unsigned int)(componentCount * 2), 0);
        g_pPaletteTransitionDirection_005d3010 =
            AllocateTaggedMemory((unsigned int)(componentCount * 2), 0);
        if (g_pPaletteTransitionAccumulator_005d301c == 0 ||
            g_pPaletteTransitionDelta_005d3014 == 0 ||
            g_pPaletteTransitionDirection_005d3010 == 0) {
            if (g_pPaletteTransitionAccumulator_005d301c != 0)
                ReleasePacketHandle(g_pPaletteTransitionAccumulator_005d301c);
            if (g_pPaletteTransitionDelta_005d3014 != 0)
                ReleasePacketHandle(g_pPaletteTransitionDelta_005d3014);
            if (g_pPaletteTransitionDirection_005d3010 != 0)
                ReleasePacketHandle(g_pPaletteTransitionDirection_005d3010);
            return 0;
        }

        g_nPaletteTransitionMaxDelta_005d3018 = 0;
        for (index = 0; index < componentCount; index++) {
            difference = (short)(current[index] - target[index]);
            if (difference < 0) {
                difference = (short)-difference;
                g_pPaletteTransitionDirection_005d3010[index] = 1;
            } else {
                g_pPaletteTransitionDirection_005d3010[index] = -1;
            }
            g_pPaletteTransitionDelta_005d3014[index] = difference;
            if (g_nPaletteTransitionMaxDelta_005d3018 < difference)
                g_nPaletteTransitionMaxDelta_005d3018 = difference;
        }

        difference = (short)(g_nPaletteTransitionMaxDelta_005d3018 / 2);
        for (index = 0; index < componentCount; index++)
            g_pPaletteTransitionAccumulator_005d301c[index] = difference;
        g_nPaletteTransitionCountdown_005d3020 =
            g_nPaletteTransitionMaxDelta_005d3018;
        g_nPaletteTransitionInitialise_0049305c = 0;
    }

    previousCountdown = g_nPaletteTransitionCountdown_005d3020;
    g_nPaletteTransitionCountdown_005d3020--;
    if (previousCountdown == 0) {
        ReleasePacketHandle(g_pPaletteTransitionAccumulator_005d301c);
        ReleasePacketHandle(g_pPaletteTransitionDelta_005d3014);
        ReleasePacketHandle(g_pPaletteTransitionDirection_005d3010);
        g_nPaletteTransitionInitialise_0049305c = 1;
        return 0;
    }

    for (index = 0; index < componentCount; index++) {
        g_pPaletteTransitionAccumulator_005d301c[index] =
            (short)(g_pPaletteTransitionAccumulator_005d301c[index] +
                    g_pPaletteTransitionDelta_005d3014[index]);
        if (g_pPaletteTransitionAccumulator_005d301c[index] >
            g_nPaletteTransitionMaxDelta_005d3018) {
            g_pPaletteTransitionAccumulator_005d301c[index] =
                (short)(g_pPaletteTransitionAccumulator_005d301c[index] -
                        g_nPaletteTransitionMaxDelta_005d3018);
            current[index] =
                (short)(current[index] +
                        g_pPaletteTransitionDirection_005d3010[index]);
        }
    }
    return 1;
}

/* Function start: 0x446910 */
char *DosStrcat(char *destination, const char *source)
{
    char *end;

    end = DosStrchr(destination, 0);
    DosStrcpy(end, source);
    return destination;
}
