/*
 *  Campaign start, continue prompt and game loop.
 *
 *  Retail object 0x409850-0x40A2CF (link order; see docs/ORDER.md).
 */
#include "game.h"

PilotProfile g_stDefaultPilotProfile_00492660 = { 0 };
short g_bNewPilotCampaignInitialized_004926c0 = 0;
short g_bDeveloperCampaignReady_004926c4 = 1;
static char g_szStartingCampaignGlobalsFile_004926c8[] = "startglb.000";

/* Function start: 0x409850 */
void InitializeNewPilotCampaign(short campaignSlot)
{
    g_nSelectedCampaignSlot_005d3bf2 = campaignSlot;
    g_nSelectedStartingCampaign_005d3bf0 = -1;
    LoadStartingCampaignGlobals(0);
    SaveAndFreeTemporaryCampaignGlobals();
    LoadTemporaryCampaignGlobals();
    if (g_pCampaignGlobals_00499c94 == 0)
        ReportFatalErrorCode("015");
    g_pCampaignGlobals_00499c94->series = 1000;
    g_pCampaignGlobals_00499c94->mission = 2000;
    FadeViewportPaletteToColour(&g_stModalSourceWindow_005d2c50, 0, 1);
    RunCampaignScript(g_nSelectedCampaignSlot_005d3bf2);
    ReleasePacketSlot((void **)&g_pCampaignGlobals_00499c94);
    RestoreGamePalette();
    ResetGameTextContexts();
    g_bNewPilotCampaignInitialized_004926c0 = 1;
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
        DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                          0, 0, fieldShape, 0);
        DrawConstellationField();
        DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                          160, 66, promptShapes, promptFrame);
        DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                          0, 126, promptShapes, 2);
        EnableMouseCursorDrawing();
        WaitForVerticalBlankThunk();
        CopyViewportContents(&g_stSecondaryViewBufferBlock_005d2c90,
                             &g_stScrViewInfo_005d21a0);
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
    g_stSecondaryViewBufferBlock_005d2c90.left = 0;
    g_stSecondaryViewBufferBlock_005d2c90.right = 319;
    g_stSecondaryViewBufferBlock_005d2c90.top = 0;
    g_stSecondaryViewBufferBlock_005d2c90.bottom = 199;
    if ((short)AllocateViewport(
            &g_stSecondaryViewBufferBlock_005d2c90,
            g_cSecondaryViewBufferColour_0049cb4c, 0) == 0) {
        ReportFatalErrorCode("016");
    }
    ClearViewport(&g_stSecondaryViewBufferBlock_005d2c90,
                  g_cSecondaryViewBufferColour_0049cb4c);
    init_constellation(0);
    InitializeConstellationField(&g_stSecondaryViewBufferBlock_005d2c90,
                                 0, 16);
    SetMenuInputPump();
    SetInputViewport(&g_stSecondaryViewBufferBlock_005d2c90);
    EnableMouseCursorDrawing();
    promptShapes = FetchDiskPacketRetrying("continue.vga", 0, 0);
    starField = FetchDiskPacketRetrying("field.v00", 1, 0);
    while (choice == 0) {
        switch (choice = RunCampaignContinuePromptLoop(
                    promptShapes, starField, promptFrame)) {
        case 1:
            FreePacketAndClear(&promptShapes, 0);
            FreePacketAndClear(&starField, 0);
            free_viewport(&g_stSecondaryViewBufferBlock_005d2c90);
            FreePacketAndClear(&g_pConstellationSprite_005d2c4c, 0);
            ClearInputPump();
            DisableMouseCursorDrawing();
            return 1;
        case 2:
            FreePacketAndClear(&promptShapes, 0);
            FreePacketAndClear(&starField, 0);
            free_viewport(&g_stSecondaryViewBufferBlock_005d2c90);
            FreePacketAndClear(&g_pConstellationSprite_005d2c4c, 0);
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
    g_bCockpitResourcesActive_0049c8e8 = 1;
    free_cockpit();
    free_all_slots();
    free_3Space();
    for (font = 0; font < 4; font++) {
        if (g_apTextGlyphs_005d2200[font] != 0) {
            ReleasePacketHandle(g_apTextGlyphs_005d2200[font]);
            g_apTextGlyphs_005d2200[font] = 0;
        }
    }
    if (g_nShowMemoryStatus_0049d784 != 0) {
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
    DAT_005c80c0 = (unsigned char *)((uintptr_t)DAT_005d300c + 0x22);
#else
    DAT_005c80c0 = DAT_005d300c + 0x22;
#endif
    ReleaseSpaceflightResources();
    LoadTemporaryCampaignGlobals();
    if (g_nOriginDevUnlock_0049d774 != 0 &&
        g_bDeveloperCampaignReady_004926c4 != 0 &&
        g_bDirectCampaignSelection_0049cc74 != 0) {
        g_pCampaignGlobals_00499c94->series =
            g_nDirectSeries_0049d79c;
        g_pCampaignGlobals_00499c94->mission =
            g_stDirectMissionSelection_0049d798.mission;
    }
    g_bDeveloperCampaignReady_004926c4 = 0;
    g_pCampaignGlobals_00499c94->field_08 = 0;
    g_pCampaignGlobals_00499c94->arcadeState = 0;

    while (campaignDone == 0) {
        g_pCutsceneCockpitPalette_00499c0c =
            AllocateScenePointerTable(1, 0x3420, 2, "HB1");
        g_pActiveCutscenePixels_005c83dc =
            g_pCutsceneCockpitPalette_00499c0c;
        g_bCutsceneViewportPreallocated_00499c4c = 1;
        RunCampaignScript(campaignSlot);
        g_bCutsceneViewportPreallocated_00499c4c = 0;
        InitializeCampaignChalkboardScreen(
            (short)(g_pCampaignGlobals_00499c94->field_0e & 0xff));
        g_pCampaignGlobals_00499c94->damageLevel = 0;
        clear_cockpit_damage();
        while (RunCampaignChalkboardMenu(campaignSlot) != 0) {
            if (g_pCampaignGlobals_00499c94->arcadeState >= 4)
                break;
        }
        g_bNewPilotCampaignInitialized_004926c0 = 0;
        ReleasePacketSlot(&g_pCutsceneCockpitPalette_00499c0c);
        g_pActiveCutscenePixels_005c83dc = 0;
        ClearInputPump();
        if (g_nShowMemoryStatus_0049d784 != 0) {
            ShowMemoryStatusDebug();
            WaitForAnyInputPress();
        }
        if (g_pCampaignGlobals_00499c94->arcadeState >= 4) {
            g_pCampaignGlobals_00499c94->series = 1;
            g_pCampaignGlobals_00499c94->mission = 0;
            campaignDone++;
        }
        InitializeCampaignConstellationState(
            g_pCampaignGlobals_00499c94, 1);
        g_pCampaignGlobals_00499c94->arcadeState = 0;
        SaveAndFreeTemporaryCampaignGlobals();
        for (slot = 0; slot < 4; slot++) {
            if (g_apTextGlyphs_005d2200[slot] != 0) {
                ReleasePacketHandle(g_apTextGlyphs_005d2200[slot]);
                g_apTextGlyphs_005d2200[slot] = 0;
            }
        }
        if (campaignDone != 0)
            return 0;
        if (g_bPumpMessagesDuringLoad_0049cc7c != 0)
            DAT_005c8430 = 1;

        flightComplete = 0;
        while (flightComplete == 0) {
            if (g_nShowMemoryStatus_0049d784 != 0) {
                ShowMemoryStatusDebug();
                WaitForAnyInputPress();
            }
            g_nTrainSimActive_0049d758 = 0;
            series = g_stCurrentPilotProfile_00493408.series;
            g_nCurSeries_005c5870 = series;
            mission = g_stCurrentPilotProfile_00493408.mission;
            g_nCurrentLevel_005c5878 = mission;
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
                g_nArcadeState_0049d75c = 3;
                break;
            case 4:
                death_sequence();
                if (PromptToContinueCampaign(0) != 0) {
                    g_asShipSide_004955d0[0] = 0;
                    g_bFriendlyFireWarningIssued_00492d5c = 0;
                    flightComplete = 0;
                    LoadSelectedPilotCampaign();
                } else {
                    flightComplete = 1;
                }
                break;
            case 2:
                ShowPlayerEjectionSequence();
                check_stranded();
                if (g_nArcadeState_0049d75c == 3)
                    ShowPlayerStrandingSequence();
                if (PromptToContinueCampaign(1) != 0) {
                    g_asShipSide_004955d0[0] = 0;
                    g_bFriendlyFireWarningIssued_00492d5c = 0;
                    flightComplete = 0;
                    LoadSelectedPilotCampaign();
                } else {
                    if (g_nArcadeState_0049d75c == 5) {
                        ejection_sequence(1000, 0);
                        g_nArcadeState_0049d75c = 5;
                    }
                    flightComplete = 1;
                }
                campaignDone = 0;
                break;
            default:
                flightComplete = 1;
                break;
            }
            if (g_nShowMemoryStatus_0049d784 != 0) {
                ShowMemoryStatusDebug();
                WaitForAnyInputPress();
            }
            ReleaseSpaceflightResources();
        }

        g_stCurrentPilotProfile_00493408.field_43 +=
            g_cPilotKillCountValue_005d2fa8;
        g_stCurrentPilotProfile_00493408.field_41++;
        DAT_005c8430 = 0;
        LoadTemporaryCampaignGlobals();
        StoreMissionResultsInCampaignGlobals(
            g_pCampaignGlobals_00499c94);
        FreePacketAndClear(&g_pPilotStatus_005d2fcc, 0);
        if (g_nShowMemoryStatus_0049d784 != 0) {
            ShowMemoryStatusDebug();
            WaitForAnyInputPress();
        }
        if (campaignDone == 0) {
            g_pCutsceneCockpitPalette_00499c0c =
                AllocateScenePointerTable(1, 0x3420, 2, "HB2");
            g_pActiveCutscenePixels_005c83dc =
                g_pCutsceneCockpitPalette_00499c0c;
            if (g_nArcadeState_0049d75c == 4) {
                g_pCampaignGlobals_00499c94->series = 1000;
                g_pCampaignGlobals_00499c94->mission = 5000;
                g_pCampaignGlobals_00499c94->previousSeries =
                    g_stCurrentPilotProfile_00493408.series;
                g_pCampaignGlobals_00499c94->previousMission =
                    g_stCurrentPilotProfile_00493408.mission;
                g_pCampaignGlobals_00499c94->arcadeState = 4;
                if (g_asShipSide_004955d0[0] == 1 ||
                    g_bFriendlyFireWarningIssued_00492d5c != 0) {
                    g_pCampaignGlobals_00499c94->shipMissionFlags[0] = 4;
                }
            } else if (g_nArcadeState_0049d75c == 5) {
                g_pCampaignGlobals_00499c94->series = 1;
                g_pCampaignGlobals_00499c94->mission = 0;
                g_pCampaignGlobals_00499c94->arcadeState = 5;
                g_pCampaignGlobals_00499c94->shipMissionFlags[0] = 4;
            } else {
                g_pCampaignGlobals_00499c94->field_08 =
                    g_pCampaignGlobals_00499c94->field_0c;
            }
            g_pCampaignGlobals_00499c94->damageLevel =
                (short)calculate_damage_level();
            if (g_nArcadeState_0049d75c != 3 &&
                g_nArcadeState_0049d75c != 5) {
                RunCampaignScript(campaignSlot);
            }
            g_pCampaignGlobals_00499c94->field_08 = 0;
            ReleasePacketSlot(&g_pCutsceneCockpitPalette_00499c0c);
            g_pActiveCutscenePixels_005c83dc = 0;
            g_nArcadeState_0049d75c =
                g_pCampaignGlobals_00499c94->arcadeState;
            g_pCampaignGlobals_00499c94->arcadeState = 0;
            g_pCampaignGlobals_00499c94->damageLevel = 0;
            if (g_nArcadeState_0049d75c > 2) {
                ReleasePacketSlot((void **)&g_pCampaignGlobals_00499c94);
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
    g_szStartingCampaignGlobalsFile_004926c8[11] =
        (char)(campaign + '0');
    g_pCampaignGlobals_00499c94 =
        LoadWholeDataFile(g_szStartingCampaignGlobalsFile_004926c8);
}

/* Function start: 0x40A2A3 */
short RunSelectedCampaign(void)
{
    return (short)RunCampaignGameLoop(g_nSelectedCampaignSlot_005d3bf2);
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short g_nSelectedStartingCampaign_005d3bf0;
short g_nSelectedCampaignSlot_005d3bf2;
