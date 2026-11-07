/*
 *  Campaign start, continue prompt and game loop.
 *
 *  Retail object 0x409850-0x40A2CF (link order; see docs/ORDER.md).
 */
#include "game.h"

PilotProfile g_stDefaultPilotProfile = { 0 }; /* 0x00492660 */
short g_bNewPilotCampaignInitialized = 0; /* 0x004926c0 */
short g_bDeveloperCampaignReady = 1; /* 0x004926c4 */
static char g_szStartingCampaignGlobalsFile[] = "startglb.000"; /* 0x004926c8 */

/* Function start: 0x409850 */
void InitializeNewPilotCampaign(short campaignSlot)
{
    g_nSelectedCampaignSlot = campaignSlot;
    g_nSelectedStartingCampaign = -1;
    LoadStartingCampaignGlobals(0);
    SaveAndFreeTemporaryCampaignGlobals();
    LoadTemporaryCampaignGlobals();
    if (g_pCampaignGlobals == 0)
        ReportFatalErrorCode("015");
    g_pCampaignGlobals->series = 1000;
    g_pCampaignGlobals->mission = 2000;
    FadeViewportPaletteToColour(&g_2_stModalSourceWindow, 0, 1);
    RunCampaignScript(g_nSelectedCampaignSlot);
    ReleasePacketSlot((void **)&g_pCampaignGlobals);
    RestoreGamePalette();
    ResetGameTextContexts();
    g_bNewPilotCampaignInitialized = 1;
}

/* Function start: 0x4098F2 */
short RunCampaignContinuePromptLoop(unsigned char *promptShapes,
                                    unsigned char *fieldShape,
                                    short promptFrame)
{
    short selection;

    selection = 0;
    while ((selection = PollSceneHotspotInput(promptShapes, 0, 0, 0, 0)) ==
           0) {
        DisableMouseCursorDrawing();
        DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                          0, 0, fieldShape, 0);
        DrawConstellationField();
        DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                          160, 66, promptShapes, promptFrame);
        DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                          0, 126, promptShapes, 2);
        EnableMouseCursorDrawing();
        WaitForVerticalBlankThunk();
        CopyViewportContents(&g_stSecondaryViewBufferBlock,
                             &g_101_stScrViewInfo);
    }
    return selection;
}

/* Function start: 0x4099A8 */
short PromptToContinueCampaign(short promptFrame)
{
    unsigned char *promptShapes;
    unsigned char *starField;
    short choice;

    promptShapes = 0;
    starField = 0;
    choice = 0;
    StopMusicUnlessSuppressed();
    free_view_buffer();
    g_stSecondaryViewBufferBlock.left = 0;
    g_stSecondaryViewBufferBlock.right = 319;
    g_stSecondaryViewBufferBlock.top = 0;
    g_stSecondaryViewBufferBlock.bottom = 199;
    if ((short)AllocateViewport(
            &g_stSecondaryViewBufferBlock,
            g_cSecondaryViewBufferColour, 0) == 0) {
        ReportFatalErrorCode("016");
    }
    ClearViewport(&g_stSecondaryViewBufferBlock,
                  g_cSecondaryViewBufferColour);
    init_constellation(0);
    InitializeConstellationField(&g_stSecondaryViewBufferBlock,
                                 0, 16);
    SetMenuInputPump();
    SetInputViewport(&g_stSecondaryViewBufferBlock);
    EnableMouseCursorDrawing();
    promptShapes = FetchDiskPacketRetrying("continue.vga", 0, 0);
    starField = FetchDiskPacketRetrying("field.v00", 1, 0);
    while (choice == 0) {
        switch (choice = RunCampaignContinuePromptLoop(
                    promptShapes, starField, promptFrame)) {
        case 1:
            FreePacketAndClear(&promptShapes, 0);
            FreePacketAndClear(&starField, 0);
            free_viewport(&g_stSecondaryViewBufferBlock);
            FreePacketAndClear(&g_pConstellationSprite, 0);
            ClearInputPump();
            DisableMouseCursorDrawing();
            return 1;
        case 2:
            FreePacketAndClear(&promptShapes, 0);
            FreePacketAndClear(&starField, 0);
            free_viewport(&g_stSecondaryViewBufferBlock);
            FreePacketAndClear(&g_pConstellationSprite, 0);
            ClearInputPump();
            DisableMouseCursorDrawing();
            return 0;
        }
    }
#ifdef SDL_PORT
    return choice;
#endif
}

/* Function start: 0x409B80 */
void ReleaseSpaceflightResources(void)
{
    short font;

    remove_nav_pointer();
    g_bCockpitResourcesActive = 1;
    free_cockpit();
    free_all_slots();
    free_3Space();
    for (font = 0; font < 4; font++) {
        if (g_36_apTextGlyphs[font] != 0) {
            ReleasePacketHandle(g_36_apTextGlyphs[font]);
            g_36_apTextGlyphs[font] = 0;
        }
    }
    if (g_nShowMemoryStatus != 0) {
        ShowMemoryStatusDebug();
        WaitForAnyInputPress();
    }
}

/* Function start: 0x409C1A */
short RunCampaignGameLoop(short campaignSlot)
{
    short campaignDone;
    short slot;
    unsigned int availableMemory;
    short flightComplete;
    short series;
    short mission;

    campaignDone = 0;
    flightComplete = 0;
#ifdef SDL_PORT
    /* This base is zero-initialized in the retail image.  MSVC formed address
     * 0x22 here and nothing ever dereferences it; express the same integer
     * address without applying an offset to a null C pointer. */
    g_39_pCurrentCampaignDataOffset = (unsigned char *)((uintptr_t)g_1_pCampaignDataBase + 0x22);
#else
    g_39_pCurrentCampaignDataOffset = g_1_pCampaignDataBase + 0x22;
#endif
    ReleaseSpaceflightResources();
    LoadTemporaryCampaignGlobals();
    if (g_nOriginDevUnlock != 0 &&
        g_bDeveloperCampaignReady != 0 &&
        g_bDirectCampaignSelection != 0) {
        g_pCampaignGlobals->series =
            g_nDirectSeries;
        g_pCampaignGlobals->mission =
            g_stDirectMissionSelection.mission;
    }
    g_bDeveloperCampaignReady = 0;
    g_pCampaignGlobals->field_08 = 0;
    g_pCampaignGlobals->arcadeState = 0;

    while (campaignDone == 0) {
        g_pCutsceneCockpitPalette =
            AllocateScenePointerTable(1, 0x3420, 2, "HB1");
        g_13_pActiveCutscenePixelsAddress =
            g_pCutsceneCockpitPalette;
        g_bCutsceneViewportPreallocated = 1;
        RunCampaignScript(campaignSlot);
        g_bCutsceneViewportPreallocated = 0;
        InitializeCampaignChalkboardScreen(
            (short)(g_pCampaignGlobals->field_0e & 0xff));
        g_pCampaignGlobals->damageLevel = 0;
        clear_cockpit_damage();
        while (RunCampaignChalkboardMenu(campaignSlot) != 0) {
            if (g_pCampaignGlobals->arcadeState >= 4)
                break;
        }
        g_bNewPilotCampaignInitialized = 0;
        ReleasePacketSlot(&g_pCutsceneCockpitPalette);
        g_13_pActiveCutscenePixelsAddress = 0;
        ClearInputPump();
        if (g_nShowMemoryStatus != 0) {
            ShowMemoryStatusDebug();
            WaitForAnyInputPress();
        }
        if (g_pCampaignGlobals->arcadeState >= 4) {
            g_pCampaignGlobals->series = 1;
            g_pCampaignGlobals->mission = 0;
            campaignDone++;
        }
        InitializeCampaignConstellationState(
            g_pCampaignGlobals, 1);
        g_pCampaignGlobals->arcadeState = 0;
        SaveAndFreeTemporaryCampaignGlobals();
        for (slot = 0; slot < 4; slot++) {
            if (g_36_apTextGlyphs[slot] != 0) {
                ReleasePacketHandle(g_36_apTextGlyphs[slot]);
                g_36_apTextGlyphs[slot] = 0;
            }
        }
        if (campaignDone != 0)
            return 0;
        if (g_bPumpMessagesDuringLoad != 0)
            g_6_bCampaignFlightRunning = 1;

        flightComplete = 0;
        while (flightComplete == 0) {
            if (g_nShowMemoryStatus != 0) {
                ShowMemoryStatusDebug();
                WaitForAnyInputPress();
            }
            g_nTrainSimActive = 0;
            series = g_stCurrentPilotProfile.series;
            g_nCurSeries = series;
            mission = g_stCurrentPilotProfile.mission;
            g_nCurrentLevel = mission;
            ResetGameTextContexts();
            OpenDiskDataFile(1);
            init_mission(series, mission);
            ReleaseSceneMusicPacket();
            switch (RunSpaceFlight(-1)) {
            case 1:
                flightComplete = 1;
                break;
            case 3:
                ShowPlayerStrandingSequence();
                flightComplete = 1;
                g_nArcadeState = 3;
                break;
            case 4:
                death_sequence();
                if (PromptToContinueCampaign(0) != 0) {
                    g_asShipSide[0] = 0;
                    g_bFriendlyFireWarningIssued = 0;
                    flightComplete = 0;
                    LoadSelectedPilotCampaign();
                } else {
                    flightComplete = 1;
                }
                break;
            case 2:
                ShowPlayerEjectionSequence();
                check_stranded();
                if (g_nArcadeState == 3)
                    ShowPlayerStrandingSequence();
                if (PromptToContinueCampaign(1) != 0) {
                    g_asShipSide[0] = 0;
                    g_bFriendlyFireWarningIssued = 0;
                    flightComplete = 0;
                    LoadSelectedPilotCampaign();
                } else {
                    if (g_nArcadeState == 5) {
                        ejection_sequence(1000, 0);
                        g_nArcadeState = 5;
                    }
                    flightComplete = 1;
                }
                campaignDone = 0;
                break;
            default:
                flightComplete = 1;
                break;
            }
            if (g_nShowMemoryStatus != 0) {
                ShowMemoryStatusDebug();
                WaitForAnyInputPress();
            }
            ReleaseSpaceflightResources();
        }

        g_stCurrentPilotProfile.field_43 +=
            g_36_cPilotKillCountValue;
        g_stCurrentPilotProfile.field_41++;
        g_6_bCampaignFlightRunning = 0;
        LoadTemporaryCampaignGlobals();
        StoreMissionResultsInCampaignGlobals(
            g_pCampaignGlobals);
        FreePacketAndClear(&g_pPilotStatus, 0);
        if (g_nShowMemoryStatus != 0) {
            ShowMemoryStatusDebug();
            WaitForAnyInputPress();
        }
        if (campaignDone == 0) {
            g_pCutsceneCockpitPalette =
                AllocateScenePointerTable(1, 0x3420, 2, "HB2");
            g_13_pActiveCutscenePixelsAddress =
                g_pCutsceneCockpitPalette;
            if (g_nArcadeState == 4) {
                g_pCampaignGlobals->series = 1000;
                g_pCampaignGlobals->mission = 5000;
                g_pCampaignGlobals->previousSeries =
                    g_stCurrentPilotProfile.series;
                g_pCampaignGlobals->previousMission =
                    g_stCurrentPilotProfile.mission;
                g_pCampaignGlobals->arcadeState = 4;
                if (g_asShipSide[0] == 1 ||
                    g_bFriendlyFireWarningIssued != 0) {
                    g_pCampaignGlobals->shipMissionFlags[0] = 4;
                }
            } else if (g_nArcadeState == 5) {
                g_pCampaignGlobals->series = 1;
                g_pCampaignGlobals->mission = 0;
                g_pCampaignGlobals->arcadeState = 5;
                g_pCampaignGlobals->shipMissionFlags[0] = 4;
            } else {
                g_pCampaignGlobals->field_08 =
                    g_pCampaignGlobals->field_0c;
            }
            g_pCampaignGlobals->damageLevel =
                (short)calculate_damage_level();
            if (g_nArcadeState != 3 &&
                g_nArcadeState != 5) {
                RunCampaignScript(campaignSlot);
            }
            g_pCampaignGlobals->field_08 = 0;
            ReleasePacketSlot(&g_pCutsceneCockpitPalette);
            g_13_pActiveCutscenePixelsAddress = 0;
            g_nArcadeState =
                g_pCampaignGlobals->arcadeState;
            g_pCampaignGlobals->arcadeState = 0;
            g_pCampaignGlobals->damageLevel = 0;
            if (g_nArcadeState > 2) {
                ReleasePacketSlot((void **)&g_pCampaignGlobals);
                LoadStartingCampaignGlobals(campaignSlot);
                SaveAndFreeTemporaryCampaignGlobals();
                return 0;
            } else {
                OpenDiskDataFile(0);
            }
        }
        availableMemory = GetAvailableMainMemory();
    }
    return 1;
}

/* Function start: 0x40A27A */
void LoadStartingCampaignGlobals(short campaign)
{
    g_szStartingCampaignGlobalsFile[11] =
        (char)(campaign + '0');
    g_pCampaignGlobals =
        LoadWholeDataFile(g_szStartingCampaignGlobalsFile);
}

/* Function start: 0x40A2A3 */
short RunSelectedCampaign(void)
{
    return (short)RunCampaignGameLoop(g_nSelectedCampaignSlot);
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short g_nSelectedStartingCampaign; /* 0x005d3bf0 */
short g_nSelectedCampaignSlot; /* 0x005d3bf2 */
