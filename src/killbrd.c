/*
 *  Kill board, conversation scenes and save-slot flags.
 *
 *  Address range 0x43c000-0x440bff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: ShowTigersClawKillBoard/RunConversationScene; string band 0x4705DC-0x470668.
 */
#include "wc1.h"

/* Function start: WC2_UNMAPPED */
void CorrectPointers(void)
{
    short pilot;

    pilot = 0;
    do {
        g_apWingmanPilots_00598a30[pilot] =
            &g_aPilotRecords_005988d0[pilot];
        pilot++;
    } while (pilot < 8);
    g_stCampaignState_0059ca50.currentPilot =
        &g_aPilotRecords_005988d0[8];
    g_pCurrentCampaignDate_005a86a8 =
        &g_stCampaignState_0059ca50.currentDate;
    g_pElapsedCampaignDate_005d3e8c =
        &g_stCampaignState_0059ca50.elapsedDate;
}

/* Function start: WC2_UNMAPPED */
void ClearRoomMenuLabel(void)
{
    g_pszCurrentRoomMenuLabel_00598aba = 0;
}

/* Function start: WC2_UNMAPPED */
int IsRoomMenuLabelEmpty(void)
{
    return g_pszCurrentRoomMenuLabel_00598aba == 0;
}

/* Function start: WC2_UNMAPPED */
void DrawRoomMenuLabel(TextContext *context, const char *label)
{
    DosStrcpy(g_szTextScratchBuffer_005d1c40, label);
    SetTextContext(context);
    FormatTextBufferFromStart(g_szRoomMenuLabelFormat_004705dc_WC1_UNMAPPED,
                              0, 188,
                              g_szTextScratchBuffer_005d1c40);
}

/* Function start: WC2_UNMAPPED */
void RefreshRoomMenuLabel(void)
{
    if (IsRoomMenuLabelEmpty())
        g_pszCurrentRoomMenuLabel_00598aba =
            g_pszBlankRoomMenuLabel_00470090_WC1_UNMAPPED;
    DrawRoomMenuLabel(&g_stRoomMenuTextContext_00598abe,
                      g_pszCurrentRoomMenuLabel_00598aba);
}

/* Function start: WC2_UNMAPPED */
void ClearRoomMenuCursorFrame(void)
{
    g_nRoomMenuCursorFrame_00598ab0 = 0;
}

/* Function start: WC2_UNMAPPED */
void SelectRoomMenuLabel(short i)
{
    const char *label;

    label = g_ppszRoomMenuLabels_00598ab6[i];
    if (label != 0)
        g_pszCurrentRoomMenuLabel_00598aba = label;
}

/* Function start: WC2_UNMAPPED */
void InitializeRoomMenu(TitleMenuRegion *regions, char **labels,
                        Viewport *viewport, char *text,
                        unsigned char alignment)
{
    g_pRoomMenuRegions_00598ab2 = regions;
    g_ppszRoomMenuLabels_00598ab6 = labels;
    g_pszCurrentRoomMenuLabel_00598aba =
        g_pszBlankRoomMenuLabel_00470090_WC1_UNMAPPED;
    g_stRoomMenuTextContext_00598abe.viewport = viewport;
    g_stRoomMenuTextContext_00598abe.text = text;
    g_stRoomMenuTextContext_00598abe.textCursor = text;
    g_stRoomMenuTextContext_00598abe.alignment = alignment;
    *text = g_szEmptyRoomMenuLabel_004705e8_WC1_UNMAPPED[0];
    InitializeTextContextFromFont(&g_stRoomMenuTextContext_00598abe,
                                  0, g_bPrimaryViewBufferColour_0049cb50,
                                  -1);
    ClearRoomMenuCursorFrame();
}

/* Function start: WC2_UNMAPPED */
int FindMenuRegionAtPoint(short x, short y,
                          const TitleMenuRegion *regions)
{
    short index;

    index = 0;
    while (regions->frame != -1) {
        if (IsPointInRect(x, y, &regions->left) != 0)
            return index;
        regions++;
        index++;
    }
    return -1;
}

/* Function start: 0x45C128 */
void ResetGameTextContexts(void)
{
    g_stDefaultTextContext_005d2d20.viewport =
        &g_stModalSourceViewport_005d2c50;
    g_stDefaultTextContext_005d2d20.text =
        g_szDefaultTextBuffer_005d2b80;
    InitializeTextContextFromFont(&g_stDefaultTextContext_005d2d20, 1,
                                  g_ucPrimaryTextColour_0049cb64,
                                  g_cSecondaryViewBufferColour_0049cb4c);
    SetTextContext(&g_stDefaultTextContext_005d2d20);
    g_stSpaceTextContext_005d21c0.viewport = &g_stViewBuffer_005d2b00;
    g_stSpaceTextContext_005d21c0.text = g_szDefaultTextBuffer_005d2b80;
    g_stSpaceTextContext_005d21c0.alignment = 2;
    InitializeTextContextFromFont(&g_stSpaceTextContext_005d21c0, 1,
                                  g_abGamePaletteReservedColours_0049cb54[8], -1);
}

/* Function start: 0x418ECD */
short RunTitleMenuInputLoop(unsigned char *buttons,
                            unsigned char *logo,
                            unsigned char *background,
                            short showSecondButton)
{
    short selection;
    short maximumSelection;
    int previousKeyboardMouseEnabled;

    selection = 0;
    maximumSelection = 0;
    if (showSecondButton == 0)
        maximumSelection = 1;
    g_bKeyboardMouseEnabled_0049be68 = 1;
    ClearDebugPauseFlags();
    ReleaseInputEventQueue();
    while (selection == 0) {
        selection = PollSceneHotspotInput(buttons, 0, 0, 0,
                                          maximumSelection);
        if (selection != 0)
            break;
        PumpWindowMessages(0);
        DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 0, 0,
                          background, 0);
        DrawConstellationField();
        DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 160, 100,
                          logo, 0x19);
        DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 160, 100,
                          logo, 0x1f);
        DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 160, 100,
                          logo, 0x20);
        DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 0, 0,
                          buttons, 0);
        if (showSecondButton != 0)
            DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 0, 0,
                              buttons, 1);
        RefreshMemoryStatusOverlay();
    }
    g_bKeyboardMouseEnabled_0049be68 = previousKeyboardMouseEnabled;
    return selection;
}

/* Function start: 0x418FFC */
short RunTitleScreen(void)
{
    int soundHandle;
    unsigned char *background;
    unsigned char *logo;
    unsigned char *buttons;
    signed char menuChoice;
    short savedCampaignPresent;
    short frame;
    Viewport screenBuffer;

    menuChoice = 0;
    buttons = 0;
    logo = 0;
    background = 0;
    soundHandle = 0;
    g_nUiInputMode_005c8d3c = 0;
    SetMenuInputPump();
    DisableMouseCursorDrawing();
    g_nUiInputMode_005c8d3c = 0;
    ClearViewport(&g_stModalSourceViewport_005d2c50,
                  g_cSecondaryViewBufferColour_0049cb4c);
    g_stSecondaryViewBuffer_005d2c90.left = 0;
    g_stSecondaryViewBuffer_005d2c90.top = 0;
    g_stSecondaryViewBuffer_005d2c90.right = 319;
    g_stSecondaryViewBuffer_005d2c90.bottom = 199;
    if (AllocateViewport(&g_stSecondaryViewBuffer_005d2c90,
                         g_cSecondaryViewBufferColour_0049cb4c, 0) == 0)
        ReportFatalErrorCode("031");
    screenBuffer.left = 0;
    screenBuffer.top = 0;
    screenBuffer.right = 319;
    screenBuffer.bottom = 199;
    if (AllocateViewport(&screenBuffer,
                         g_cSecondaryViewBufferColour_0049cb4c, 0) == 0)
        ReportFatalErrorCode("032");
    PreloadMusicTrack(0x36);
    spacetrack(0x36, 1, 1);
    init_constellation(0);
    InitializeConstellationField(
        &g_stSecondaryViewBuffer_005d2c90, 0, 0x10);
    buttons = FetchDiskPacketRetrying("buttons.v00", 0, 0);
    logo = FetchDiskPacketRetrying("wc2logo.vga", 0, 0);
    background = FetchDiskPacketRetrying("field.v00", 1, 0);
    FlushInputEvents();
    ClearDebugPauseFlags();
    if (WaitForInputKey() != 0)
        g_bSceneEscapeRequested_0049d4b0 = 1;
    if (g_bSceneEscapeRequested_0049d4b0 == 0)
        PlaySfxWaveFileByNumber(0x28, -1, 0);
    if (g_bSceneEscapeRequested_0049d4b0 == 0) {
        for (frame = 0; frame < 0x1a; frame++) {
            DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                              0, 0, background, 0);
            DrawConstellationField();
            DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                              160, 100, logo, frame);
            RefreshMemoryStatusOverlay();
            if (WaitForInputKey() != 0) {
                g_bSceneEscapeRequested_0049d4b0 = 1;
                break;
            }
            SetFrameTimerAndWait(4);
        }
    }
    if (soundHandle != 0) {
        ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(
            soundHandle, 0);
    }
    soundHandle = 0;
    if (g_bSceneEscapeRequested_0049d4b0 == 0) {
        ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(0, 0);
        PlaySfxWaveFileByNumber(99, -1, 0);
    }
    if (g_bSceneEscapeRequested_0049d4b0 == 0) {
        for (frame = 0; frame < 10; frame++) {
            DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                              0, 0, background, 0);
            DrawConstellationField();
            DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                              160, 100, logo, 0x19);
            RefreshMemoryStatusOverlay();
            if (WaitForInputKey() != 0) {
                g_bSceneEscapeRequested_0049d4b0 = 1;
                break;
            }
            SetFrameTimerAndWait(4);
        }
    }
    if (soundHandle != 0) {
        ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(
            soundHandle, 0);
    }
    soundHandle = 0;
    PlaySfxWaveFileByNumber(6, -1, 0);
    if (g_bSceneEscapeRequested_0049d4b0 == 0) {
        for (frame = 0x1a; frame < 0x20; frame++) {
            DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                              0, 0, background, 0);
            DrawConstellationField();
            DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                              160, 100, logo, 0x19);
            DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                              160, 100, logo, frame);
            RefreshMemoryStatusOverlay();
            if (WaitForInputKey() != 0) {
                g_bSceneEscapeRequested_0049d4b0 = 1;
                break;
            }
            SetFrameTimerAndWait(4);
        }
    }
    if (soundHandle != 0) {
        ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(
            soundHandle, 0);
    }
    soundHandle = 0;
    if (g_bSceneEscapeRequested_0049d4b0 == 0)
        PlaySfxWaveFileByNumber(99, -1, 0);
    if (g_bSceneEscapeRequested_0049d4b0 == 0) {
        for (frame = 0; frame < 10; frame++) {
            DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                              0, 0, background, 0);
            DrawConstellationField();
            DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                              160, 100, logo, 0x19);
            DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                              160, 100, logo, 0x1f);
            RefreshMemoryStatusOverlay();
            if (WaitForInputKey() != 0) {
                g_bSceneEscapeRequested_0049d4b0 = 1;
                break;
            }
            SetFrameTimerAndWait(4);
        }
    }
    if (soundHandle != 0) {
        ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(
            soundHandle, 0);
    }
    soundHandle = 0;
    if (g_bSceneEscapeRequested_0049d4b0 == 0)
        PlaySfxWaveFileByNumber(100, -1, 0);
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                      0, 0, background, 0);
    DrawConstellationField();
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                      160, 100, logo, 0x19);
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                      160, 100, logo, 0x1f);
    RefreshMemoryStatusOverlay();
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                      160, 100, logo, 0x20);
    CopyViewportContents(&g_stScreenViewport_005d21a0,
                         &screenBuffer);
    MarkDibDirty();
    DIBslamReal();
    g_bRoomTransitionAnimationEnabled_00499c00 = 1;
    CopyViewportContents(&g_stSecondaryViewBuffer_005d2c90,
                         &g_stScreenViewport_005d21a0);
    MarkDibDirty();
    DIBslamReal();
    if (soundHandle != 0) {
        ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(
            soundHandle, 0);
    }
    soundHandle = 0;
    savedCampaignPresent = (short)HasSavedPilotCampaign();
    SetMenuInputPump();
    g_nUiInputMode_005c8d3c = 1;
    SetInputViewport(&g_stSecondaryViewBuffer_005d2c90);
    SetPersonnelMousePosition(159, 159);
    EnableMouseCursorDrawing();
    ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(
        soundHandle, 0);
    g_bNewPilotCampaignInitialized_004926c0 = 0;
    free_viewport(&screenBuffer);
    if (g_bSceneEscapeRequested_0049d4b0 != 0)
        g_bSceneEscapeRequested_0049d4b0 = 0;
    PumpWindowMessages(0);
    while (TakeInputPressCount() != 0) {
        PumpWindowMessages(0);
        FlushInputEvents();
    }
    FlushInputEvents();
    g_bSceneEscapeRequested_0049d4b0 = 0;
    menuChoice = 0;
    while (menuChoice == 0) {
        menuChoice = (signed char)RunTitleMenuInputLoop(
            buttons, logo, background, savedCampaignPresent);
    }
    DisableMouseCursorDrawing();
    FreePacketAndClear(&buttons, 0);
    FreePacketAndClear(&logo, 0);
    FreePacketAndClear(&background, 0);
    free_viewport(&g_stSecondaryViewBuffer_005d2c90);
    FreePacketAndClear(&g_pConstellationShape_005d2c4c, 0);
    StopMusicUnlessSuppressed();
    ReleaseMusicTrack(0x36);
    g_bSceneEscapeRequested_0049d4b0 = 0;
    return (short)(menuChoice - 1);
}

/* Function start: WC2_UNMAPPED */
short RecRoom(void)
{
    InputEventState event;
    ShortRect firstPilotBounds;
    ShortRect nextFrameBounds;
    ShortRect secondPilotBounds;
    Viewport bottomDestination;
    Viewport bottomSource;
    Viewport pilotDestination;
    Viewport pilotWork;
    Viewport shotglassDestination;
    Viewport shotglassWork;
    signed char animationIds[3];
    signed char *animations[3];
    unsigned char clicked;
    unsigned char firstFrame;
    short characterMask;
    short eventType;
    short frame;
    short index;
    short region;
    short result;
    int lastChalkboardTick;
    int rosterOffset;
    int personality;

    result = 0;
    lastChalkboardTick = 0;
    firstFrame = 0;
    characterMask = 0;
    g_apszRecRoomMenuLabels_004704f8_WC1_UNMAPPED[0] =
        g_apszRecRoomBaseLabels_004704e8_WC1_UNMAPPED[0];
    g_apszRecRoomMenuLabels_004704f8_WC1_UNMAPPED[1] =
        g_apszRecRoomBaseLabels_004704e8_WC1_UNMAPPED[1];
    g_apszRecRoomMenuLabels_004704f8_WC1_UNMAPPED[2] =
        g_apszRecRoomBaseLabels_004704e8_WC1_UNMAPPED[2];

    PreloadMusicTrackHook(30);
    spacetrack(30, 2, 1);
    FlushInputEvents();
    LoadBriefingData((short)g_stCampaignState_0059ca50.currentSeries,
                     (short)g_stCampaignState_0059ca50.currentMission);
    g_pRecRoomRoster_005988b8 = LoadPacketAllocated(
        g_asCampaignPilotFiles_00469450_WC1_UNMAPPED[g_nCampaignDataSet_005a8118], 2);

    animationIds[0] = (signed char)(RandomInRange(0, 3) + 9);
    g_apRecRoomCharacterShapes_005988c0[0] = 0;
    g_apRecRoomCharacterShapes_005988c0[1] = 0;
    g_apRecRoomCharacterShapes_005988c0[2] = 0;
    g_aRecRoomMenuRegions_004704a0_WC1_UNMAPPED[0].left = 400;
    g_aRecRoomMenuRegions_004704a0_WC1_UNMAPPED[0].top = 400;
    g_aRecRoomMenuRegions_004704a0_WC1_UNMAPPED[0].right = 401;
    g_aRecRoomMenuRegions_004704a0_WC1_UNMAPPED[0].bottom = 401;
    *(ShortRect *)&g_aRecRoomMenuRegions_004704a0_WC1_UNMAPPED[1].left =
        *(ShortRect *)&g_aRecRoomMenuRegions_004704a0_WC1_UNMAPPED[0].left;
    *(ShortRect *)&g_aRecRoomMenuRegions_004704a0_WC1_UNMAPPED[2].left =
        *(ShortRect *)&g_aRecRoomMenuRegions_004704a0_WC1_UNMAPPED[0].left;

    g_apRecRoomCharacterShapes_005988c0[0] =
        FetchDiskPacketRetrying(5, 11, 0);
    GetShapeFrameBounds(&g_aRecRoomMenuRegions_004704a0_WC1_UNMAPPED[0].left,
                        g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[0].x,
                        g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[0].y,
                        g_apRecRoomCharacterShapes_005988c0[0], 0);

    rosterOffset = ((int)g_stCampaignState_0059ca50.currentMission +
                    (int)g_stCampaignState_0059ca50.currentSeries * 4) *
                       2 - 8;
    animationIds[1] =
        (signed char)g_pRecRoomRoster_005988b8[rosterOffset];
    if (animationIds[1] != -1) {
        personality = (int)animationIds[1];
        if (g_stCampaignState_0059ca50
                .personalityDeathMission[personality] == 0) {
            g_apRecRoomCharacterShapes_005988c0[1] =
                FetchDiskPacketRetrying(
                    5, (short)(personality + 3), 0);
            GetShapeFrameBounds(
                &g_aRecRoomMenuRegions_004704a0_WC1_UNMAPPED[1].left,
                g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[1].x,
                g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[1].y,
                g_apRecRoomCharacterShapes_005988c0[1], 0);
            sprintf(g_szTalkToFirstPilot_00470570_WC1_UNMAPPED,
                    g_szTalkToPilotFormat1_004705ec_WC1_UNMAPPED,
                    g_apWingmanPilots_00598a30[personality]->callsign);
        } else {
            sprintf(g_szTalkToFirstPilot_00470570_WC1_UNMAPPED,
                    g_szTalkToPilotFormat1_004705ec_WC1_UNMAPPED + 12);
        }
    }

    animationIds[2] =
        (signed char)g_pRecRoomRoster_005988b8[rosterOffset + 1];
    if (animationIds[1] != -1) {
        personality = (int)animationIds[2];
        if (g_stCampaignState_0059ca50
                .personalityDeathMission[personality] == 0) {
            g_apRecRoomCharacterShapes_005988c0[2] =
                FetchDiskPacketRetrying(
                    5, (short)(personality + 3), 0);
            GetShapeFrameBounds(
                &g_aRecRoomMenuRegions_004704a0_WC1_UNMAPPED[2].left,
                g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[2].x,
                g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[2].y,
                g_apRecRoomCharacterShapes_005988c0[2], 0);
            sprintf(g_szTalkToSecondPilot_00470588_WC1_UNMAPPED,
                    g_szTalkToPilotFormat2_004705fc_WC1_UNMAPPED,
                    g_apWingmanPilots_00598a30[personality]->callsign);
        } else {
            sprintf(g_szTalkToSecondPilot_00470588_WC1_UNMAPPED,
                    g_szTalkToPilotFormat2_004705fc_WC1_UNMAPPED + 12);
        }
    }

    ReleasePacketHandle(g_pRecRoomRoster_005988b8);
    if (g_apRecRoomCharacterShapes_005988c0[2] != 0)
        characterMask = 1;
    if (g_apRecRoomCharacterShapes_005988c0[1] != 0) {
        characterMask = 2;
        if (g_apRecRoomCharacterShapes_005988c0[2] != 0)
            characterMask = 3;
    }

    SetViewportRect(&g_stSecondaryViewBuffer_005d2c90, 0, 0, 319, 199);
    init_constellation(0);
    g_stConstellationViewport_005a6b40 = g_stSecondaryViewBuffer_005d2c90;
    SetViewportRect(&g_stConstellationViewport_005a6b40,
                    54, 35, 146, 72);
    InitializeConstellationField(&g_stConstellationViewport_005a6b40,
                                 -1, 6);

    animations[0] = g_abShotglassIdleAnimation_004703b8_WC1_UNMAPPED;
    animations[1] =
        g_apRecRoomAnimations_00470458_WC1_UNMAPPED[(int)animationIds[1]];
    animations[2] =
        g_apRecRoomAnimations_00470458_WC1_UNMAPPED[(int)animationIds[2]];
    InitializeRoomMenu(g_aRecRoomMenuRegions_004704a0_WC1_UNMAPPED,
                       g_apszRecRoomMenuLabels_004704f8_WC1_UNMAPPED,
                       &g_stRoomScreenViewport_005988a0,
                       g_szDefaultTextBuffer_005d2b80, 2);

    bottomSource = g_stSecondaryViewBuffer_005d2c90;
    SetViewportRect(&bottomSource, 0, 187, 319, 199);
    bottomDestination = g_stRoomScreenViewport_005988a0;
    SetViewportRect(&bottomDestination, 0, 187, 319, 199);
    g_pRecRoomBackgroundShape_00598a50 =
        FetchDiskPacketRetrying(5, 0, 0);
    g_nMenuPointerSpeed_00493054 = 1;
    g_bInputMode_0059a848 = 1;
    g_stMouseCursorState_0059ab10.viewport = &g_stRoomDisplayViewport_00598a60;

    pilotWork = g_stSecondaryViewBuffer_005d2c90;
    shotglassWork = g_stSecondaryViewBuffer_005d2c90;
    pilotDestination = g_stRoomScreenViewport_005988a0;
    shotglassDestination = g_stRoomScreenViewport_005988a0;

    if (g_apRecRoomCharacterShapes_005988c0[2] != 0) {
        GetShapeFrameBounds(
            &secondPilotBounds.left,
            g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[2].x,
            g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[2].y,
            g_apRecRoomCharacterShapes_005988c0[2],
            (short)*animations[2]);
    }
    if (g_apRecRoomCharacterShapes_005988c0[1] != 0) {
        GetShapeFrameBounds(
            &firstPilotBounds.left,
            g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[1].x,
            g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[1].y,
            g_apRecRoomCharacterShapes_005988c0[1],
            (short)*animations[1]);
        *(ShortRect *)&pilotWork.left = firstPilotBounds;
        if (g_apRecRoomCharacterShapes_005988c0[2] != 0) {
            UnionRectBounds((ShortRect *)&pilotWork.left,
                            &firstPilotBounds, &secondPilotBounds);
        }
    } else if (g_apRecRoomCharacterShapes_005988c0[2] != 0) {
        *(ShortRect *)&pilotWork.left = secondPilotBounds;
    }
    if (g_apRecRoomCharacterShapes_005988c0[1] != 0 ||
        g_apRecRoomCharacterShapes_005988c0[2] != 0) {
        *(ShortRect *)&pilotDestination.left =
            *(ShortRect *)&pilotWork.left;
    }

    WarpWc1MouseTo(160, 100);
    GetShapeFrameBounds(
        &shotglassWork.left,
        g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[0].x,
        g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[0].y,
        g_apRecRoomCharacterShapes_005988c0[0],
        (short)*animations[0]);

    while (result == 0) {
        if (firstFrame == 0) {
            DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 0, 0,
                              g_pRecRoomBackgroundShape_00598a50, 0);
            if (characterMask != 0) {
                DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 158, 128,
                                  g_pRecRoomBackgroundShape_00598a50,
                                  characterMask);
            }
            SetFrameTimerPeriodDirect(0);
        }

        if (IsFrameTickElapsed() != 0) {
            DrawSpriteDefault(&pilotWork, 0, 0,
                              g_pRecRoomBackgroundShape_00598a50, 0);
            for (index = 0; index < 3; index++) {
                if (g_apRecRoomCharacterShapes_005988c0[index] != 0) {
                    if (*animations[index] == -1) {
                        if (index == 0) {
                            animationIds[0] =
                                (signed char)(RandomInRange(0, 3) + 9);
                            if (animationIds[0] == 11 &&
                                RandomInRange(0, 3) != 0)
                                animationIds[0]--;
                        }
                        animations[index] =
                            g_apRecRoomAnimations_00470458_WC1_UNMAPPED[
                                (int)animationIds[index]];
                    }

                    if (index > 0) {
                        DrawSpriteDefault(
                            &pilotWork,
                            g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[index].x,
                            g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[index].y,
                            g_apRecRoomCharacterShapes_005988c0[index], 0);
                        frame = (short)*animations[index]++;
                        DrawSpriteDefault(
                            &pilotWork,
                            g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[index].x,
                            g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[index].y,
                            g_apRecRoomCharacterShapes_005988c0[index],
                            frame);
                    } else {
                        UnionRectBounds(
                            (ShortRect *)&shotglassWork.left,
                            (ShortRect *)&shotglassWork.left,
                            (ShortRect *)&g_stConstellationViewport_005a6b40.left);
                        DrawConstellationField();
                        DrawSpriteDefault(
                            &shotglassWork, 0, 0,
                            g_pRecRoomBackgroundShape_00598a50, 0);
                        frame = (short)*animations[index]++;
                        DrawSpriteDefault(
                            &shotglassWork,
                            g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[index].x,
                            g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[index].y,
                            g_apRecRoomCharacterShapes_005988c0[index],
                            frame);
                        GetShapeFrameBounds(
                            &nextFrameBounds.left,
                            g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[index].x,
                            g_aRecRoomCharacterOrigins_00470490_WC1_UNMAPPED[index].y,
                            g_apRecRoomCharacterShapes_005988c0[index],
                            (short)*animations[index]);
                        if (firstFrame != 0) {
                            *(ShortRect *)&shotglassDestination.left =
                                *(ShortRect *)&shotglassWork.left;
                            if (ShouldSuspendCursorForRect(
                                    &nextFrameBounds) != 0) {
                                SuspendWc1MouseCursor();
                                CopyViewportContents(
                                    &shotglassWork,
                                    &shotglassDestination);
                                ResumeMouseCursorHook();
                            } else {
                                CopyViewportContents(
                                    &shotglassWork,
                                    &shotglassDestination);
                            }
                        }
                    }
                }
            }

            if (firstFrame == 0) {
                firstFrame = 1;
                if (DAT_00470510_WC1_UNMAPPED != 0) {
                    PanToScreen(&g_stSecondaryViewBuffer_005d2c90,
                                &g_stRoomScreenViewport_005988a0);
                    DAT_00470510_WC1_UNMAPPED = 0;
                } else {
                    CopyViewportContents(
                        &g_stSecondaryViewBuffer_005d2c90,
                        &g_stRoomScreenViewport_005988a0);
                }
                ResumeMouseCursorHook();
            } else if (g_apRecRoomCharacterShapes_005988c0[1] != 0 ||
                       g_apRecRoomCharacterShapes_005988c0[2] != 0) {
                if (ShouldSuspendCursorForRect(
                        (ShortRect *)&pilotWork.left) != 0) {
                    SuspendWc1MouseCursor();
                    CopyViewportContents(&pilotWork,
                                         &pilotDestination);
                    ResumeMouseCursorHook();
                } else {
                    CopyViewportContents(&pilotWork,
                                         &pilotDestination);
                }
            }

            SuspendWc1MouseCursor();
            CopyViewportContents(&bottomSource, &bottomDestination);
            RefreshRoomMenuLabel();
            ResumeMouseCursorHook();
            SetFrameTimerPeriodDirect(9);
        }

        clicked = 0;
        eventType = PollInputEvent(&event);
        if (eventType == 3 || eventType == 5) {
            ClearInputKeyStatePreservingModifiers();
            if ((short)event.value == 0x1c ||
                (short)event.value == 0x39) {
                clicked = 1;
            } else {
                MoveMenuPointerFromKeyboard(&event);
            }
        } else if (eventType == 2 || eventType == 10) {
            clicked = 1;
        } else if (eventType == 13) {
            UpdateRoomMenuCursor();
        }

        if (clicked != 0) {
            region = FindMenuRegionAtPoint(
                event.x, event.y, g_aRecRoomMenuRegions_004704a0_WC1_UNMAPPED);
            SuspendWc1MouseCursor();
            if (region >= 0 && region <= 2) {
                if (g_apRecRoomCharacterShapes_005988c0[region] != 0) {
                    free_constellation();
                    ReleasePacketHandle(g_pRecRoomBackgroundShape_00598a50);
                    g_stSecondaryViewBuffer_005d2c90.bottom = 127;
                    g_stScreenViewport_005d21a0.top = 24;
                    g_stScreenViewport_005d21a0.bottom = 151;
                    InitializeConversationText();
                    ClearViewport(&g_stRoomScreenViewport_005988a0,
                                  g_cSecondaryViewBufferColour_0049cb4c);
                    g_pConversationBackdropShape_00598c04 =
                        FetchDiskPacketRetrying(
                            5, 1, 0);
                    SceneDirector(
                        2,
                        g_apRecRoomSceneData_00598ae0[region],
                        g_apRecRoomTextData_00598aa0[region]);
                    g_bSceneEscapeRequested_0049d4b0 = 0;
                    SetEventManagerPump(PollMenuInputDevices);
                    FreePacketAndClear(
                        &g_pConversationBackdropShape_00598c04, 0);
                    SetFrameTimerPeriodDirect(1);
                    g_stScreenViewport_005d21a0.top = 0;
                    g_stScreenViewport_005d21a0.bottom = 199;
                    g_stSecondaryViewBuffer_005d2c90.bottom = 199;
                    g_stConstellationViewport_005a6b40 =
                        g_stSecondaryViewBuffer_005d2c90;
                    g_stConstellationViewport_005a6b40.left = 54;
                    g_stConstellationViewport_005a6b40.top = 35;
                    g_stConstellationViewport_005a6b40.right = 146;
                    g_stConstellationViewport_005a6b40.bottom = 72;
                    init_constellation(0);
                    InitializeConstellationField(
                        &g_stConstellationViewport_005a6b40, -1, 6);
                    g_pRecRoomBackgroundShape_00598a50 =
                        FetchDiskPacketRetrying(
                            5, 0, 0);
                    ClearViewport(&g_stRoomScreenViewport_005988a0,
                                  g_cSecondaryViewBufferColour_0049cb4c);
                }
            } else if (region == 3) {
                FlushInputEvents();
                if ((int)(DAT_0059ab54 - lastChalkboardTick) >
                    g_nInputTickScale_005c8d24) {
                    ShowWc1ChalkBoard();
                    ClearViewport(&g_stSecondaryViewBuffer_005d2c90, g_cSecondaryViewBufferColour_0049cb4c);
                    lastChalkboardTick = (int)DAT_0059ab54;
                }
            } else if (region == 4 || region == 5) {
                result = region;
            } else {
                clicked = 0;
                ResumeMouseCursorHook();
            }

            g_stRoomMouseViewport_00598a80 = g_stScreenViewport_005d21a0;
            g_stMouseCursorState_0059ab10.viewport = &g_stRoomMouseViewport_00598a80;
            g_bInputMode_0059a848 = 1;
            if (clicked != 0)
                firstFrame = 0;
        }

        ShowMemoryStatusDebug();
        MarkDibDirty();
        DIBslamReal();
    }

    g_nMenuPointerSpeed_00493054 = 2;
    g_nMenuInputRepeatDelay_005a8208 =
        g_nSavedRoomControllerX_005988b4;
    EventManagerHook(0);
    ReleasePacketHandle(g_apRecRoomCharacterShapes_005988c0[0]);
    ReleasePacketHandle(g_apRecRoomCharacterShapes_005988c0[1]);
    ReleasePacketHandle(g_apRecRoomCharacterShapes_005988c0[2]);
    free_constellation();
    ReleasePacketHandle(g_pRecRoomBackgroundShape_00598a50);
    ReleaseTextFont(0);
    ReleasePacketHandle(g_pBriefingPacket_00598aec);
    ClearViewport(&g_stScreenViewport_005d21a0, g_cSecondaryViewBufferColour_0049cb4c);
    free_viewport(&g_stSecondaryViewBuffer_005d2c90);
    g_bSceneEscapeRequested_0049d4b0 = 0;
    StopMusicUnlessSuppressed();
    ReleaseMusicTrackHook(30);
    return result;
}

/* Function start: WC2_UNMAPPED */
void ShowWc1ChalkBoard(void)
{
    InputEventState event;
    TextContext context;
    TextContext *previousContext;
    unsigned char *background;
    unsigned char savedInputMode;
    char pilotName[20];
    short done;
    short index;
    short other;
    short pilot;
    short rank;
    short rowY;
    short swap;
    int score;
    int otherScore;

    for (index = 0; index < 9; index++) {
        for (other = index; other < 9; other++) {
            pilot = g_asChalkBoardPilotOrder_00470518_WC1_UNMAPPED[index];
            swap = g_asChalkBoardPilotOrder_00470518_WC1_UNMAPPED[other];
            score = g_aPilotRecords_005988d0[pilot].kills * 1000 -
                    g_aPilotRecords_005988d0[pilot].missions + 1;
            otherScore = g_aPilotRecords_005988d0[swap].kills * 1000 -
                         g_aPilotRecords_005988d0[swap].missions + 1;
            if (score < otherScore) {
                g_asChalkBoardPilotOrder_00470518_WC1_UNMAPPED[index] = swap;
                g_asChalkBoardPilotOrder_00470518_WC1_UNMAPPED[other] = pilot;
            }
        }
    }

    memset(&context, 0, sizeof(context));
    g_stChalkBoardDate_00470514_WC1_UNMAPPED = *g_pCurrentCampaignDate_005a86a8;
    background = FetchDiskPacketRetrying(5, 2, 0);
    previousContext = g_pCurrentTextContext_005c8d1c;
    g_stModalSourceViewport_005d2c50 = g_stScreenViewport_005d21a0;
    context.viewport = &g_stModalSourceViewport_005d2c50;
    context.text = g_szDefaultTextBuffer_005d2b80;
    context.textCursor = g_szDefaultTextBuffer_005d2b80;
    InitializeTextContextFromFont(&context, 3,
                                  g_bPrimaryViewBufferColour_0049cb50, -1);
    savedInputMode = g_bInputMode_0059a848;
    g_bInputMode_0059a848 = 1;
    done = 0;
    do {
        PumpWindowMessages(0);
        if (PeekInputEvent(&event, 10) != 0 ||
            PeekInputEvent(&event, 2) != 0 ||
            PeekInputEvent(&event, 3) != 0)
            done = 1;

        DrawSpriteDefault(&g_stModalSourceViewport_005d2c50,
                          0, 0, background, 0);
        context.alignment = 2;
        SetTextContext(&context);
        FormatTextBufferFromStart(g_szChalkBoardHeading_0047060c_WC1_UNMAPPED,
                                  0, 10);
        DrawFormattedText(g_szChalkBoardPilotHeading_0047062c_WC1_UNMAPPED,
                          60, 24);
        DrawFormattedText(g_szChalkBoardScoreHeading_00470638_WC1_UNMAPPED,
                          185, 24);
        rowY = 46;
        index = 0;
        for (; index < 9; index++, rowY = (short)(rowY + 16)) {
            pilot = g_asChalkBoardPilotOrder_00470518_WC1_UNMAPPED[index];
            DosStrcpy(pilotName, g_aPilotRecords_005988d0[pilot].name);
            _strupr(pilotName);
            rank = g_aPilotRecords_005988d0[pilot].rank;
            if (rank < 0)
                rank = 0;
            if (rank > 4)
                rank = 4;
            DrawFormattedText(g_szChalkBoardPilotFormat_0047064c_WC1_UNMAPPED,
                              10, rowY,
                              g_apszPilotRankNames_0049a608[rank],
                              pilotName);
            if (pilot == 8 ||
                g_stCampaignState_0059ca50
                    .personalityDeathMission[pilot] == 0) {
                DrawFormattedText(g_szChalkBoardScoreFormat_00470658_WC1_UNMAPPED,
                                  230, rowY,
                                  g_aPilotRecords_005988d0[pilot].missions,
                                  280,
                                  g_aPilotRecords_005988d0[pilot].kills);
            } else {
                DrawFormattedText(g_szChalkBoardKiaFormat_00470668_WC1_UNMAPPED,
                                  230, rowY,
                                  g_szKilledInAction_00470664_WC1_UNMAPPED);
            }
        }
        MarkDibDirty();
        DIBslamReal();
    } while (done == 0);

    ReleaseTextFont(3);
    ReleasePacketHandle(background);
    g_pCurrentTextContext_005c8d1c = previousContext;
    ClearInputKeyStatePreservingModifiers();
    g_bInputMode_0059a848 = savedInputMode;
    FlushInputEvents();
}

#pragma function(strcmp)

/* Function start: 0x459C4D */
short PollCampaignChalkboardMenu(unsigned char *scene)
{
    short selection;

    selection = 0;
    SetMenuInputPump();
    FlushInputEvents();
    for (;;) {
        selection = PollSceneHotspotInput(scene, 0, 0, 0, 0);
        if (selection != 0)
            break;
        DisableMouseCursorDrawing();
        DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                          58, 175,
                          g_pCampaignChalkboardShape_0049ca54, 3);
        g_stDefaultTextContext_005d2d20.viewport =
            &g_stSecondaryViewBuffer_005d2c90;
        InitializeTextContextFromFont(
            &g_stDefaultTextContext_005d2d20, 0,
            g_bPrimaryViewBufferColour_0049cb50, -1);
        if (g_pszPersonnelFooter_00492658 != 0) {
            SetTextCursor(
                (unsigned short)((320 -
                    MeasureTextPixelWidthClamped(
                        g_pszPersonnelFooter_00492658)) >> 1),
                180);
            if (strcmp(g_pszPersonnelFooter_00492658,
                       "Exit to DOS") != 0) {
                DrawFormattedText("%S",
                                  g_pszPersonnelFooter_00492658);
            } else {
                DrawFormattedText("%S", "Exit the Game");
            }
        }
        g_stDefaultTextContext_005d2d20.viewport =
            &g_stScreenViewport_005d21a0;
        EnableMouseCursorDrawing();
        RefreshMemoryStatusOverlay();
    }
    return selection;
}

/* Function start: 0x459D74 */
short RunCampaignChalkboardMenu(short campaignSlot)
{
    short replay;
    short result;
    short selection;

    selection = 0;
    result = 1;
    replay = 1;
    PreloadMusicTrack(0x35);
    spacetrack(0x35, 2, 1);
    SetInputViewport(&g_stSecondaryViewBuffer_005d2c90);
    while (selection == 0) {
        g_bDisableChalkboardReplay_0049ca58 =
            (short)(g_pCampaignGlobals_00499c94->field_0a == 0);
        selection = PollCampaignChalkboardMenu(
            g_pCampaignChalkboardShape_0049ca54);
        switch (selection) {
        case 1:
            g_bDisableChalkboardReplay_0049ca58 = 0;
            DisableMouseCursorDrawing();
            FreePacketAndClear(&g_pCampaignChalkboardShape_0049ca54, 0);
            free_viewport(&g_stSecondaryViewBuffer_005d2c90);
            StopMusicUnlessSuppressed();
            ReleaseMusicTrack(0x35);
            g_pCampaignGlobals_00499c94->field_08 =
                g_pCampaignGlobals_00499c94->field_0a;
            RunCampaignScript(campaignSlot);
            g_pCampaignGlobals_00499c94->field_08 = 0;
            result = 0;
            break;
        case 4:
            exit_squadron(0);
            break;
        case 3:
            /* The WC2 save/load menu at 0x4353D4 remains unreconstructed. */
            g_bDisableChalkboardReplay_0049ca58 = 0;
            DisableMouseCursorDrawing();
            if (g_nOriginDevUnlock_0049d774 != 0)
                strcpy(g_stCurrentPilotProfile_00493408.callsign,
                       "CHEATER");
            DisableMouseCursorDrawing();
            RefreshCampaignChalkboardScreen(
                (short)(g_pCampaignGlobals_00499c94->field_0e & 0xff));
            result = 1;
            break;
        case 5:
            g_bDisableChalkboardReplay_0049ca58 = 0;
            result = 1;
            break;
        case 2:
            g_bDisableChalkboardReplay_0049ca58 = 0;
            DisableMouseCursorDrawing();
            FreePacketAndClear(&g_pCampaignChalkboardShape_0049ca54, 0);
            free_viewport(&g_stSecondaryViewBuffer_005d2c90);
            StopMusicUnlessSuppressed();
            ReleaseMusicTrack(0x35);
            ClearViewport(&g_stModalSourceViewport_005d2c50, 0);
            while (replay != 0) {
                RunCampaignScript(campaignSlot);
                replay = 0;
                if (g_pCampaignGlobals_00499c94->field_16 != 0) {
                    replay++;
                    g_pCampaignGlobals_00499c94->field_08++;
                }
                g_pCampaignGlobals_00499c94->field_16 = 0;
            }
            g_pCampaignGlobals_00499c94->field_08 =
                g_pCampaignGlobals_00499c94->field_0a;
            InitializeCampaignChalkboardScreen(
                (short)(g_pCampaignGlobals_00499c94->field_0e & 0xff));
            result = 1;
            break;
        }
    }
    return result;
}

#pragma intrinsic(strcmp)

/* Function start: WC2_UNMAPPED */
void ResetCampaignData(void)
{
    memcpy(&g_stCampaignState_0059ca50,
           &g_stInitialCampaignState_004700b0_WC1_UNMAPPED,
           sizeof(g_stCampaignState_0059ca50));
    memcpy(g_aPilotRecords_005988d0,
           g_aInitialPilotRecords_00470108_WC1_UNMAPPED,
           sizeof(g_aPilotRecords_005988d0));
    InitializeTrainSimHighScores();
    CorrectPointers();
}

/* Function start: 0x469BE0 */
short ReadPacketSectionData(PacketSectionHandle *handle,
                            void *destination,
                            unsigned int length)
{
    int offset;
    int end;

    offset = (int)(handle->dataOffset + handle->position);
    end = (int)(handle->dataOffset + handle->dataSize);
    if (destination == 0)
        return 0;
    if (end < (int)(offset + length) || length == (unsigned int)-1)
        length = (unsigned int)(end - offset);
    if (ReadDataFileAtOffset((unsigned short)handle->file, offset,
                             length, destination) == 0)
        return 0;
    handle->position += length;
    return 1;
}

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
#ifdef WC1_SDL
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
    int rightExtent;
#ifndef WC1_SDL
    short *frameHeader;
#endif
    int frameOffset;
    int left;
    int top;
    int bottom;
#ifdef WC1_SDL
    short frameExtents[4];
#endif

    frameOffset = (int)(short)(frame * 4 + 4);
    if (frameOffset < (int)*(unsigned short *)(shape + 4)) {
#ifdef WC1_SDL
        memcpy(frameExtents, shape + *(int *)(shape + frameOffset),
               sizeof(frameExtents));
        rightExtent = frameExtents[0];
        left = frameExtents[1];
        top = frameExtents[2];
        bottom = frameExtents[3];
#else
        frameHeader = (short *)(shape + *(int *)(shape + frameOffset));
        rightExtent = *frameHeader++;
        left = *frameHeader++;
        top = *frameHeader++;
        bottom = *frameHeader;
#endif
        *width = (short)(left + rightExtent + 1);
        *height = (short)(top + bottom + 1);
        *leftExtent = (short)left;
        *topExtent = (short)top;
    }
}

/* Function start: 0x425618 */
void DecodeShapeFrame(unsigned char *shape, short frame,
                      unsigned char *bitmap, int width, short height,
                      int leftExtent, int topExtent)
{
    unsigned char *commands;
    unsigned char *runData;
    unsigned char *destination;
    unsigned char code;
    unsigned char colour;
    volatile unsigned short rowCode;
    unsigned short runLength;
    unsigned short copyLength;
    short maximumX;
    short maximumY;
    int frameOffset;
    int x;
    int y;
    int runRight;
    int skip;
#ifdef WC1_SDL
    short coordinate;
#endif

    if (shape == 0 || frame < 0)
        return;
    frameOffset = (int)(short)(frame * 4 + 4);
    if (frameOffset >= (int)*(unsigned short *)(shape + 4))
        return;

    maximumX = (short)(width - 1);
    commands = shape + *(int *)(shape + frameOffset) + 8;
    maximumY = (short)(height - 1);
#ifdef WC1_SDL
    memcpy((void *)&rowCode, commands, sizeof(rowCode));
#else
    rowCode = *(unsigned short *)commands;
#endif
    commands += 2;
    while (rowCode != 0) {
#ifdef WC1_SDL
        memcpy(&coordinate, commands, sizeof(coordinate));
        x = leftExtent + coordinate;
        memcpy(&coordinate, commands + 2, sizeof(coordinate));
        y = topExtent + coordinate;
#else
        x = leftExtent + *(short *)commands;
        y = topExtent + *(short *)(commands + 2);
#endif
        destination = bitmap + y * width + x;
        commands += 4;
        if ((rowCode & 1) != 0) {
            rowCode >>= 1;
            while (rowCode != 0) {
                code = *commands;
                commands++;
                if ((code & 1) != 0) {
                    code >>= 1;
                    colour = *commands;
                    commands++;
                    runLength = code;
                    rowCode = (unsigned short)(rowCode - runLength);
                    if (y >= 0 && y <= maximumY) {
                        runRight = x + runLength - 1;
                        if (x <= maximumX && runRight >= 0) {
                            skip = 0;
                            copyLength = runLength;
                            if (x < 0) {
                                skip = -x;
                                copyLength =
                                    (unsigned short)(copyLength + x);
                            }
                            if (maximumX < runRight)
                                copyLength = (unsigned short)(copyLength -
                                                       runRight + maximumX);
                            memset(destination + skip, colour,
                                   (short)copyLength);
                        }
                    }
                } else {
                    code >>= 1;
                    runLength = code;
                    rowCode = (unsigned short)(rowCode - runLength);
                    runData = commands;
                    if (y >= 0 && y <= maximumY) {
                        runRight = x + runLength - 1;
                        if (x <= maximumX && runRight >= 0) {
                            skip = 0;
                            copyLength = runLength;
                            if (x < 0) {
                                skip = -x;
                                copyLength =
                                    (unsigned short)(copyLength + x);
                            }
                            if (maximumX < runRight)
                                copyLength = (unsigned short)(copyLength -
                                                       runRight + maximumX);
                            memcpy(destination + skip, runData + skip,
                                   (short)copyLength);
                        }
                    }
                    commands = runData + runLength;
                }
                x += runLength;
                destination += runLength;
            }
        } else {
            rowCode >>= 1;
            if (y >= 0 && y <= maximumY) {
                runRight = x + rowCode - 1;
                if (x <= maximumX && runRight >= 0) {
                    skip = 0;
                    copyLength = rowCode;
                    if (x < 0) {
                        skip = -x;
                        copyLength = (unsigned short)(copyLength + x);
                    }
                    if (maximumX < runRight)
                        copyLength = (unsigned short)(copyLength -
                                                     runRight + maximumX);
                    memcpy(destination + skip, commands + skip,
                           (short)copyLength);
                }
            }
            commands += rowCode;
        }
#ifdef WC1_SDL
        memcpy((void *)&rowCode, commands, sizeof(rowCode));
#else
        rowCode = *(unsigned short *)commands;
#endif
        commands += 2;
    }
}

/* Function start: 0x4259E2 */
int SignExtendClipCoord(volatile short v)
{
    if ((unsigned short)v < 0xfdc0)
        return (unsigned short)v;
    return (int)(short)v;
}
