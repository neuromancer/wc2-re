/*
 *  WINGLEADER main module.
 *
 *  Address range 0x4274e0-0x427fff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: main() at 0x004274E0, confirmed against the leaked DOS source screenshot.
 */
#include "wc1.h"

static short g_nTargetCameraGunCooldown_0049d33c;
static ShortVector g_aaTargetCameraGunOffsets_0049d3c8[2][2];
static const short g_asFleetMouseYawThresholds_0049d400[6] = {
    10, 37, 52, 57, 62, 1070
};
static const short g_asFleetMousePitchThresholds_0049d410[6] = {
    5, 18, 27, 35, 38, 1040
};
static signed char g_bFleetPolledInputActive_0049d420;
static signed char g_bFleetButtonInputActive_0049d424;
static short g_nFleetMouseYawVelocity_0049d428;
static short g_nFleetMousePitchVelocity_0049d42c;
static short g_bFleetPrimaryInputQueued_005c57dc;
static short g_bFleetMouseMoveQueued_005c57ea;

/* Function start: WC2_UNMAPPED */
#ifdef WC1_SDL
int RunWc1GameMain(short argc, char **argv)
{
    char *argument;
    int animationDemo;
    int selection;
    int launchMission;
    short argumentCount;
    short argumentIndex;
    short mission;
    short series;
    short gameFlowResult;

    mission = 0;
    launchMission = 0;
    series = 1;
    animationDemo = 0;

    GetShutdownErrorCode(g_abDiskPromptDriveState_005a7d20);
    ExitCleanupHook();
    StartupHook(GetJoystickButtonEdge);
    DAT_0059ab4c = GetStartupErrorCode(0x21);
    Wc1ShutdownHook(0x21, MouseIdleHook);

    argumentCount = LoadWingCmdrCfgFile(argc, argv);
    _chdir("gamedat");
#if 0
    LoadInstallDat();
    _chdir("..");
#endif
#ifdef WC1_SDL
    if (Wc1SdlUsingDosData())
        DAT_0059ab34 = 1;
#endif
    if (GetCurrentDiskDriveHook() > 'B')
        DAT_0059ab34 = 1;

    SystemDebugPrintf("Loading WING COMMANDER.  Please wait...\n");
    DAT_005a7d9c = 1;
    g_nMusicDriverMode_0049be8c = 4;
    DAT_0059a856 = 1;
    ResetCampaignData();

    for (argumentIndex = 0;
         argumentIndex < argumentCount;
         argumentIndex++) {
        argument = g_pStartupArguments_005c57f0[argumentIndex];
        if (strcmp("Origin", argument) == 0)
            g_nOriginDevUnlock_0049d774 = 1;

        switch (argument[0]) {
        case '?':
            SystemDebugPrintf("Version %s.\n", g_pszGameVersion_0049b528);
        case '-':
            if (argument[1] == 'm')
                g_nShowMemoryStatus_0049d784 = 1;
            if (g_nOriginDevUnlock_0049d774 != 0) {
                switch (argument[1]) {
                case 'b':
                    g_bPlayerCollisionEnabled_0049d780 = 0;
                    break;
                case 'f':
                    g_bShowFrameRate_0049c260 = 1;
                    break;
                case 'k':
                    g_bPlayerDamageEnabled_0049d77c = 0;
                    break;
                case 'q':
                    g_bConfigQuickModeEnabled_0049c264 = 0;
                    break;
                }
            }
            break;
        case 'A':
        case 'a':
            if (argument[1] == 's' || argument[1] == 'S') {
                g_nMissionEntryNavOverride_0049d790 = (short)atoi(argument + 2);
            } else {
                g_nMusicDriverMode_0049be8c = 2;
                g_nArcadeStartupParameter_005a7b8a =
                    (short)atoi(argument + 1);
            }
            break;
        case 'E':
        case 'e':
            g_bSlowSceneAnimation_00469998_WC1_UNMAPPED = 1;
            break;
        case 'P':
        case 'p':
            g_nMusicDriverMode_0049be8c = 3;
            break;
        case 'R':
        case 'r':
            g_nMusicDriverMode_0049be8c = 1;
            break;
        case 'T':
        case 't':
            g_bSlowSceneAnimation_00469998_WC1_UNMAPPED = 3;
            break;
        case 'V':
        case 'v':
            g_bSlowSceneAnimation_00469998_WC1_UNMAPPED = 0;
            break;
        case 'Z':
        case 'z':
            DAT_005a7d9c = 1;
            break;
        case 'l':
            if (g_nOriginDevUnlock_0049d774 != 0)
                launchMission = 1;
            break;
        case 'm':
            if (g_nOriginDevUnlock_0049d774 != 0)
                mission = (short)atoi(argument + 1);
            break;
        case 's':
            if (g_nOriginDevUnlock_0049d774 != 0) {
                DAT_004688f0 = 1;
                series = (short)atoi(argument + 1);
            }
            break;
        case 'w':
            if (g_nOriginDevUnlock_0049d774 != 0) {
                animationDemo = 1;
                mission = (short)atoi(argument + 1);
            }
            break;
        }
    }

    SetCinematicFrameTiming(20.0f);
    g_stCampaignState_0059ca50.currentSeries = (signed char)series;
    g_nSpacePaletteFadeMode_004901e8 = 0x13;
    g_stInitialCampaignState_004700b0_WC1_UNMAPPED.currentSeries =
        (signed char)series;
    g_stCampaignState_0059ca50.currentMission = (signed char)mission;
    g_stInitialCampaignState_004700b0_WC1_UNMAPPED.currentMission =
        (signed char)mission;
    LoadOriginFxDrivers();
    g_nSpacePaletteFadeMode_004901e8 = 0x13;

    if (animationDemo == 1) {
        SystemDebugPrintf("Pre animation: %lu.\n",
                          GetLargestFreeMemoryBlockByType(0));
        WaitForKeyAcknowledge(0);
        RunCampaignGameLoop((short)mission);
        SystemDebugPrintf("Post animation: %lu.\n",
                          GetLargestFreeMemoryBlockByType(0));
        WaitForKeyAcknowledge(0);
    }

    LoadVolumeSettingsFromRegistry();
    SetSoundEffectsVolume(
        g_anVolumeLevels_0049d720[g_nSfxVolumeSetting_0049d74c / 2]);
    SetMusicStreamVolume((unsigned short)g_anVolumeLevels_0049d720[
        g_nMusicVolumeSetting_0049d750 / 2]);

    if (launchMission != 0) {
#ifdef WC1_SDL
        if (InitWc1Mission(series, mission) != 0) {
            fprintf(stderr,
                    "Mission SERIES=%d MISSION=%d is absent or invalid.\n",
                    (int)series, (int)mission);
            return 1;
        }
#else
        init_mission(series, mission);
#endif
#ifdef WC1_SDL
        /* Use the normal campaign setup unless cockpitless mode was requested. */
        if (g_bCockpitEnabled_0049c26c != 0)
            LaunchPlayerShip();
#endif
        RunSpaceFlight(g_nMissionEntryNavOverride_0049d790);
        exit_squadron("Bye!");
        return 0;
    }

#ifdef WC1_SDL
    Wc1SdlPlayDosStartupIntro();
#endif
    g_bSceneEscapeRequested_0049d4b0 = 0;
    for (;;) {
        FrameStartHook(0);
        DAT_004688e0 = 1;
        selection = RunWc1TitleSequence();
        switch (selection) {
        case 0:
            StartNewCampaign(0);
            break;
        case 2:
            StartNewCampaign(1);
            break;
        case 3:
            StartNewCampaign(2);
            break;
        default:
            break;
        }
        gameFlowResult = RunWc1GameFlow();
        while (gameFlowResult != 0)
            gameFlowResult = RunWc1GameFlow();
    }
}
#endif

#ifndef WC1_SDL

#pragma function(strcpy)

/* Function start: 0x46591A */
void RunGameApplication(short argc, char **argv)
{
    short argumentCount;
    int memoryBytes;
    short campaignSelected;
    int roomSelection;
    int flightResult;

    campaignSelected = 0;
    argumentCount = LoadWingCmdrCfgFile(argc, argv);
    _chdir("gamedat");
    printf("Loading WING COMMANDER 2.  Please wait...\n");
    g_bHighMemoryResourcesEnabled_005c80e4 = 1;
    g_stDefaultPilotProfile_00492660.field_5e = 5;
    memcpy(&g_stCurrentPilotProfile_00493408,
           &g_stDefaultPilotProfile_00492660,
           sizeof(g_stCurrentPilotProfile_00493408));
    main(argumentCount, g_pStartupArguments_005c57f0);

    if (g_bMemoryAdjustmentEnabled_0049cc84 != 0) {
        memoryBytes = (unsigned short)g_nMemoryAdjustmentKb_005c8dda << 10;
        g_pMemoryAdjustment_0049d788 =
            AllocateTaggedMemory((unsigned int)memoryBytes, 0x40);
        if (g_pMemoryAdjustment_0049d788 != 0)
            printf("Adjusting Mem size by %dK\n",
                   g_nMemoryAdjustmentKb_005c8dda);
    }

    g_stCurrentPilotProfile_00493408.series = g_nDirectSeries_0049d79c;
    g_stDefaultPilotProfile_00492660.series =
        g_stCurrentPilotProfile_00493408.series;
    g_stCurrentPilotProfile_00493408.mission = g_nDirectMission_0049d79a;
    g_stDefaultPilotProfile_00492660.mission =
        g_stCurrentPilotProfile_00493408.mission;
    g_nResourcePaletteMode_005c57e6 = 0;
    LoadOriginFxDrivers();
    SetMenuInputPump();
    if (g_bRewritePacketExtensions_0049cb48 == 0) {
        g_nMenuShadowColour_005c5876 = 2;
        g_nMenuHighlightColour_005c5874 = 10;
        g_nMenuTextColour_005c57e8 = 0x5e;
    } else {
        g_nMenuShadowColour_005c5876 = 0;
        g_nMenuHighlightColour_005c5874 = 10;
        g_nMenuTextColour_005c57e8 = 2;
    }

    if (g_bDirectMissionLaunch_0049d798 == 0) {
        g_bSceneEscapeRequested_0049d4b0 = 0;
        for (;;) {
            if (g_nOriginDevUnlock_0049d774 != 0 &&
                g_bDeveloperCampaignReady_004926c4 != 0 &&
                g_bDirectCampaignSelection_0049cc74 != 0) {
                g_nSelectedCampaignSlot_005d3bf2 = 0;
                while (RunSelectedCampaign() != 0) {
                }
            } else {
                while (campaignSelected == 0) {
                    roomSelection = RunTitleScreen();
                    switch (roomSelection) {
                    case 0:
                        g_nSelectedCampaignSlot_005d3bf2 =
                            BarracksScreen();
                        InitializeNewPilotCampaign(
                            g_nSelectedCampaignSlot_005d3bf2);
                        campaignSelected = RunPilotDatabaseMenu();
                        break;
                    case 1:
                        g_nSelectedCampaignSlot_005d3bf2 =
                            LoadSelectedPilotCampaign();
                        campaignSelected = 1;
                        break;
                    }
                }
                campaignSelected = 0;
                if (g_nOriginDevUnlock_0049d774 != 0) {
                    strcpy(g_stCurrentPilotProfile_00493408.callsign,
                           "CHEATER");
                    strcpy(g_szPilotCallsign_00499ef8, "CHEATER");
                }
                while (RunSelectedCampaign() != 0) {
                }
            }
        }
    } else {
        g_nCurrentSeries_005c5870 = g_nDirectSeries_0049d79c;
        g_nCurrentMission_005c5878 = g_nDirectMission_0049d79a;
        LoadTemporaryCampaignGlobals();
        g_pCampaignGlobals_00499c94->field_08 = 0;
        InitializeCampaignConstellationState(
            g_pCampaignGlobals_00499c94, 1);
        SaveAndFreeTemporaryCampaignGlobals();
        init_mission(g_nDirectSeries_0049d79c,
                     g_nDirectMission_0049d79a);
        RunSpaceFlight(g_nMissionEntryNavOverride_0049d790);
        LogMemoryUsage();
        printf("You ");
        switch (flightResult = g_nArcadeState_0049d75c) {
        case 1:
            printf("have landed.\n");
            break;
        case 3:
            printf("are stranded.\n");
            break;
        case 4:
            printf("have died.\n");
            break;
        case 2:
            printf("have ejected.\n");
            break;
        }
        if (g_pMemoryAdjustment_0049d788 != 0) {
            free(g_pMemoryAdjustment_0049d788);
        }
    }
}

#pragma intrinsic(strcpy)

#endif

/* Function start: 0x465CBC */
void free_view_buffer(void)
{
    if (message_showing())
        EndCommMenu();
    if (g_stViewBuffer_005d2b00.pixels != 0)
        free_viewport(&g_stViewBuffer_005d2b00);
#if 0
    return 0;
#endif
}

/* Function start: 0x465CF6 */
void initialize_view_buffer(void)
{
#if 0
    if (g_bSpaceViewBufferEnabled_0049d7a4 != 0 && g_stViewBuffer_005d2b00.pixels == 0) {
        if (AllocateViewport(&g_stViewBuffer_005d2b00, (short)g_cPrimaryViewBufferColour_0049cb88,
                             0x20) == 0)
            ReportOutOfMemoryAndExit(g_szSpaceBuffer_0049d978);
    }
    return 0;
#else
    short allocated;

    if (g_bSpaceViewBufferEnabled_0049d7a4 != 0 &&
        g_stViewBuffer_005d2b00.pixels == 0) {
        allocated = (short)AllocateViewport(
            &g_stViewBuffer_005d2b00,
            (short)g_cPrimaryViewBufferColour_0049cb88, 0x20);
        if (allocated == 0)
            ReportOutOfMemoryAndExit(g_szSpaceBuffer_0049d978);
    }
#endif
}

/* Function start: 0x465D55 */
void dump_buffer_to_screen(void)
{
    int mode;

#ifdef WC1_SDL
    Wc1SdlCompleteSpaceFrame();
#endif
    if (g_nCockpitDisplayMode_0049d71c > 0) {
        CopyViewportContents(
            &g_stViewBuffer_005d2b00, &g_stScreenViewport_005d21a0);
    } else {
        mode = (int)g_cScreenViewportMode_005c82a6;
        switch (mode) {
        case 4:
            g_stScreenViewport_005d21a0.top = 24;
            CopyViewportContents(
                &g_stViewBuffer_005d2b00, &g_stScreenViewport_005d21a0);
            g_stScreenViewport_005d21a0.top = 0;
            break;
        case 5:
            CopyViewportContents(
                &g_stViewBuffer_005d2b00, &g_stScreenViewport_005d21a0);
            break;
        default:
            fizzle_fade(
                &g_stViewBuffer_005d2b00, &g_stScreenViewport_005d21a0,
                g_pScreenViewportGeometry_005c82b0);
            break;
        }
    }
    if (g_nShowMemoryStatus_0049d784 != 0)
        ShowMemoryStatusDebug();
}

/* Function start: 0x465E88 */
unsigned int clear_view_buffer(void)
{
    ClearViewport(&g_stViewBuffer_005d2b00, (unsigned char)g_cPrimaryViewBufferColour_0049cb88);
    return 0;
}

/* Function start: 0x437CBF */
void InitializeConversationViewport(void)
{
    ClearViewport(&g_stModalSourceViewport_005d2c50, g_cSecondaryViewBufferColour_0049cb4c);
    if (g_stSecondaryViewBuffer_005d2c90.pixels != 0)
        free_viewport(&g_stSecondaryViewBuffer_005d2c90);
    g_stScreenViewport_005d21a0.top = 24;
    g_stScreenViewport_005d21a0.bottom = 151;
    g_stSecondaryViewBuffer_005d2c90.left = 0;
    g_stSecondaryViewBuffer_005d2c90.right = 319;
    g_stSecondaryViewBuffer_005d2c90.top = 0;
    g_stSecondaryViewBuffer_005d2c90.bottom = 127;
    if (AllocateViewport(&g_stSecondaryViewBuffer_005d2c90, (short)g_cSecondaryViewBufferColour_0049cb4c, 0) == 0)
        ReportPacketLoadError(0, 0, 0, 0,
                              g_szAllocateBufferTag_0049ae20);
#if 0
    return 0;
#endif
}

/* Function start: 0x437D68 */
void ResetScreenClipToFullHeight(void)
{
    free_viewport(&g_stSecondaryViewBuffer_005d2c90);
    g_stScreenViewport_005d21a0.top = 0;
    g_stScreenViewport_005d21a0.bottom = 199;
#if 0
    return 0;
#endif
}

/* Function start: 0x437D92 */
void InitializeConversationText(void)
{
    g_stConversationTextViewport_005a7570 =
        g_stModalSourceViewport_005d2c50;
    g_stConversationTextViewport_005a7570.top = 152;
    g_stConversationTextContext_005d2d40.viewport =
        &g_stConversationTextViewport_005a7570;
    g_stConversationTextContext_005d2d40.text =
        g_szDefaultTextBuffer_005d2b80;
    g_stConversationTextContext_005d2d40.alignment = 2;
    InitializeTextContextFromFont(&g_stConversationTextContext_005d2d40,
                                  0,
                                  g_bPrimaryViewBufferColour_0049cb50,
                                  g_cSecondaryViewBufferColour_0049cb4c);
    SetTextContext(&g_stConversationTextContext_005d2d40);
#if 0
    return 0;
#endif
}

/* Function start: 0x465EA9 */
void RefreshMemoryStatusOverlay(void)
{
    WaitForVerticalBlankThunk();
    CopyViewportContents(&g_stSecondaryViewBuffer_005d2c90, &g_stScreenViewport_005d21a0);
    MarkDibDirty();
    DIBslamReal();
    if (g_nShowMemoryStatus_0049d784 != 0)
        ShowMemoryStatusDebug();
}

/* Function start: 0x465EE9 */
void Update_3Space(void)
{
#if 0
    house_keep();
    house_keep_objects();
    update_objects_in_space();
    set_eye_direction_and_position();
    servicetrack();
    g_nSpaceFrame_00493134++;
    return 0;
#else
    house_keep();
    CheckAllGuardedAllocations();
    house_keep_objects();
    if (g_nArcadeState_0049d75c != 0)
        return;
    update_objects_in_space();
    set_eye_direction_and_position();
    if (g_nCannedSceneMode_0049021c == 0)
        servicetrack();
    g_nSpaceFrame_00493134++;
#endif
}

/* Function start: 0x465F3A */
void TriggerPlayerHitPaletteFlash(void)
{
    if (g_nCurrentView_00492fa8 <= 4)
        g_asSpacePaletteFade_005d2d60[0] = 0x30;
}

/* Function start: 0x465F5B */
void FadeFlightPaletteEntry(short *entry)
{
    if (entry[0] != 0) {
        entry[0] = (short)(entry[0] - 4);
        entry[1] = 0;
        entry[2] = 0;
    } else
        entry[1] = 0;
}

/* Function start: 0x465FA3 */
void UpdateSpacePaletteFade(void)
{
#if 0
    if (g_asSpacePaletteFade_005d2d60[0] != 0) {
        switch ((int)(short)g_nSpacePaletteFadeMode_004901e8) {
        case 9:
        case 13:
            ClearViewport(&g_stViewBuffer_005d2b00,
                          g_abGamePaletteReservedColours_0049cb54[8]);
            g_bViewportDirty_0049d76c = 1;
            g_asSpacePaletteFade_005d2d60[0] = 0;
            break;
        case 0x13:
            g_asSpacePaletteFade_005d2d60[0] = (short)(g_asSpacePaletteFade_005d2d60[0] - 4);
            SetPaletteEntry((short)g_cPrimaryViewBufferColour_0049cb88,
                            g_asSpacePaletteFade_005d2d60);
            break;
        }
    }
    return 0;
#else
    if (g_asSpacePaletteFade_005d2d60[0] != 0) {
        switch ((int)(short)g_nSpacePaletteFadeMode_004901e8) {
        case 0x13:
            g_asSpacePaletteFade_005d2d60[0] =
                (short)(g_asSpacePaletteFade_005d2d60[0] - 0x10);
            SetPaletteEntry((short)g_cPrimaryViewBufferColour_0049cb88,
                            g_asSpacePaletteFade_005d2d60);
            break;
        case 9:
        case 13:
            ClearViewport(&g_stViewBuffer_005d2b00,
                          g_abGamePaletteReservedColours_0049cb54[8]);
            g_bViewportDirty_0049d76c = 1;
            g_asSpacePaletteFade_005d2d60[0] = 0;
            break;
        }
    }
#endif
}

/* Function start: 0x46604F */
unsigned int house_keep(void)
{
    short palette;

    if (g_nCannedSceneMode_0049021c == 0 &&
        g_nTrainSimActive_0049d758 == 0) {
        if ((g_nSpaceFrame_00493134 & 0x1f) == 0)
            ReleaseStaleNavTarget();
        if (g_nHazardFieldCount_004931d0 != 0 &&
            (g_nSpaceFrame_00493134 & 0xf) == 0)
            check_hazards();
    }
    if (g_nCurrentView_00492fa8 == 0) {
        palette = 0;
        do {
            FadeFlightPaletteEntry(
                g_aasCockpitHitPaletteFades_005d2cb0[palette]);
            SetPaletteEntry((short)(palette + 0xb9),
                            g_aasCockpitHitPaletteFades_005d2cb0[palette]);
            palette++;
        } while (palette < 6);
        return 0;
    }
    if (g_nCriticalDamageWarningSfxHandle_005d1ec0 != 0) {
        ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(
            g_nCriticalDamageWarningSfxHandle_005d1ec0, 1);
        g_nCriticalDamageWarningSfxHandle_005d1ec0 = 0;
        g_abCockpitLightGoal_005d1eb8[3] = 0;
    }
    return 0;
}

/* Function start: WC2_UNMAPPED */
void init_player_input(void)
{
    SetMousePosition(
        (g_stViewBuffer_005d2b00.right - g_stViewBuffer_005d2b00.left) / 2 + 1,
        g_nViewCenterY_005c80da);
    ClearDebugPauseFlags();
    g_bInputCursorEnabled_005c80e6 = 0;
    g_bSuppressNextMouseMove_005c843c = 1;
}

/* Function start: 0x4661C2 */
void get_player_input(void)
{
    short buttons;

    if (g_nActiveInputDevice_005d1726 == -1)
        return;
    ReadCalibratedJoystick();
    buttons = (short)(g_aInputDeviceSamples_005d1780[
        g_nActiveInputDevice_005d1726].buttons & 3);
    g_wSavedInputDeviceButtonState_005c4b14 =
        g_wFlightInputButtonState_0049d7c0 =
        g_wInputDeviceButtonState_005c8432;
    g_wFlightInputButtonState_0049d7c0 &= (short)~0x1f;
    g_wFlightInputButtonState_0049d7c0 |= buttons;
    g_wInputDeviceButtonState_005c8432 =
        g_wFlightInputButtonState_0049d7c0;
    if ((buttons & 1) != 0) {
        TranslatePolledInputEvent(10, (unsigned int)buttons);
        g_bFlightButtonOneHeld_0049d7cc = 1;
    } else if (g_bFlightButtonOneHeld_0049d7cc != 0) {
        TranslatePolledInputEvent(0x45, 1);
        g_bFlightButtonOneHeld_0049d7cc = 0;
    }
    if ((buttons & 2) != 0) {
        TranslatePolledInputEvent(10, (unsigned int)buttons);
        g_bFlightButtonTwoHeld_0049d7c8 = 1;
    } else if (g_bFlightButtonTwoHeld_0049d7c8 != 0) {
        TranslatePolledInputEvent(0x45, 2);
        g_bFlightButtonTwoHeld_0049d7c8 = 0;
    }
    if (g_aInputDeviceSamples_005d1780[
            g_nActiveInputDevice_005d1726].x != 0 ||
        g_aInputDeviceSamples_005d1780[
            g_nActiveInputDevice_005d1726].y != 0) {
        TranslatePolledInputEvent(7, (unsigned int)buttons);
    } else if (g_aInputDeviceSamples_005d1780[
                   g_nActiveInputDevice_005d1726].x !=
                   g_stCurrentFlightInput_0049d7b0.x ||
               g_aInputDeviceSamples_005d1780[
                   g_nActiveInputDevice_005d1726].y !=
                   g_stCurrentFlightInput_0049d7b0.y) {
        TranslatePolledInputEvent(7, (unsigned int)buttons);
    }
    g_stCurrentFlightInput_0049d7b0 =
        g_aInputDeviceSamples_005d1780[g_nActiveInputDevice_005d1726];
    g_wInputDeviceButtonState_005c8432 =
        g_wSavedInputDeviceButtonState_005c4b14;
}

/* Function start: 0x4663A2 */
void process_player_input(void)
{
    short finished;
    short shift;

    {
    short keys[4];

    {
    short *key;
    short handled;

    handled = 0;
    finished = 0;
    shift = (short)GetShiftKeyState();
    key = keys;
    switch (g_cCurrentKey_00493128) {
    case 0x47:
        *key++ = 0x48;
        *key++ = 0x4b;
        break;
    case 0x49:
        *key++ = 0x48;
        *key++ = 0x4d;
        break;
    case 0x4f:
        *key++ = 0x50;
        *key++ = 0x4b;
        break;
    case 0x51:
        *key++ = 0x50;
        *key++ = 0x4d;
        break;
    default:
        *key++ = (short)g_cCurrentKey_00493128;
        break;
    }
    *key = -1;

    key = keys;
    while (finished == 0) {
        switch (*key++) {
        case -1:
            finished++;
            break;
        case 0x4c:
            SetPersonnelMousePosition(
                (short)((g_stViewBuffer_005d2b00.left + g_stViewBuffer_005d2b00.right) / 2),
                (short)((g_stViewBuffer_005d2b00.top + g_stViewBuffer_005d2b00.bottom) / 2));
            g_nYawInput_004931aa =
                g_nPitchInput_004931a8 =
                g_nRollInput_004931ac = 0;
            handled = 1;
            break;
        case 0x48:
            g_nCockpitControlState_0049d7ac = 0;
            if (g_nPitchInput_004931a8 < 0) {
                g_nPitchInput_004931a8 = 0;
            } else {
                if (shift != 0)
                    g_nPitchInput_004931a8 = 9;
                if (g_nPitchInput_004931a8 < 9)
                    g_nPitchInput_004931a8++;
                else if (g_cPreviousKey_0049312c < 0)
                    g_nPitchInput_004931a8++;
                else
                    g_nPitchInput_004931a8--;
            }
            handled = 1;
            break;
        case 0x50:
            g_nCockpitControlState_0049d7ac = 0;
            if (g_nPitchInput_004931a8 > 0) {
                g_nPitchInput_004931a8 = 0;
            } else {
                if (shift != 0)
                    g_nPitchInput_004931a8 = -9;
                if (g_nPitchInput_004931a8 > -9) {
                    g_nPitchInput_004931a8--;
                } else {
                    if (g_cPreviousKey_0049312c < 0)
                        g_nPitchInput_004931a8--;
                    else
                        g_nPitchInput_004931a8++;
                }
            }
            handled = 1;
            break;
        case 0x34:
        case 0x53:
            g_nCockpitControlState_0049d7ac = 0;
            if (g_nRollInput_004931ac < 0) {
                g_nRollInput_004931ac = 0;
            } else {
                if (shift != 0)
                    g_nRollInput_004931ac = 9;
                if (g_nRollInput_004931ac < 9)
                    g_nRollInput_004931ac++;
                else if (g_cPreviousKey_0049312c < 0)
                    g_nRollInput_004931ac++;
                else
                    g_nRollInput_004931ac--;
            }
            handled = 1;
            break;
        case 0x33:
        case 0x52:
            g_nCockpitControlState_0049d7ac = 0;
            if (g_nRollInput_004931ac > 0) {
                g_nRollInput_004931ac = 0;
            } else {
                if (shift != 0)
                    g_nRollInput_004931ac = -9;
                if (g_nRollInput_004931ac > -9) {
                    g_nRollInput_004931ac--;
                } else {
                    if (g_cPreviousKey_0049312c < 0)
                        g_nRollInput_004931ac--;
                    else
                        g_nRollInput_004931ac++;
                }
            }
            handled = 1;
            break;
        case 0x4d:
            g_nCockpitControlState_0049d7ac = 0;
            if (g_nYawInput_004931aa < 0) {
                g_nYawInput_004931aa = 0;
            } else {
                if (shift != 0)
                    g_nYawInput_004931aa = 9;
                if (g_nYawInput_004931aa < 9)
                    g_nYawInput_004931aa++;
                else if (g_cPreviousKey_0049312c < 0)
                    g_nYawInput_004931aa++;
                else
                    g_nYawInput_004931aa--;
            }
            handled = 1;
            break;
        case 0x4b:
            g_nCockpitControlState_0049d7ac = 0;
            if (g_nYawInput_004931aa > 0) {
                g_nYawInput_004931aa = 0;
            } else {
                if (shift != 0)
                    g_nYawInput_004931aa = -9;
                if (g_nYawInput_004931aa > -9) {
                    g_nYawInput_004931aa--;
                } else {
                    if (g_cPreviousKey_0049312c < 0)
                        g_nYawInput_004931aa--;
                    else
                        g_nYawInput_004931aa++;
                }
            }
            handled = 1;
            break;
        default:
            break;
        }
    }
    }
    }
}

/* Function start: 0x466908 */
unsigned int fire_players_lasers(void)
{
    if (g_asObjectCounter_00494be0[0] == -1 &&
        g_asShipWeaponEnergy_00495590[0] > 0) {
        fire_fixed_projectile_weapon(0);
        if (g_acShipTarget_00495f20[0] != -1 &&
            get_mode(1) == 5)
            SelectCockpitVduMode(1, 3);
    }
    return 0;
}

/* Function start: 0x46696E */
void players_flight_dynamics(void)
{
    if (g_aeSpecialManeuver_00495600[0] ==
            SPECIAL_MANEUVER_BLOWING_UP) {
        if (g_asObjectCounter_00494be0[0] == -1) {
            if (g_aObjectTypeData_00496d30[
                    g_acObjectType_00493980[0]].pitchRate >
                    g_anObjectYawRotation_00494fc8[0] &&
                g_aObjectTypeData_00496d30[
                    g_acObjectType_00493980[0]].yawRate >
                    g_anObjectPitchRotation_00494f38[0] &&
                g_aObjectTypeData_00496d30[
                    g_acObjectType_00493980[0]].rollRate >
                    g_anObjectRollRotation_00495058[0]) {
                g_aeSpecialManeuver_00495600[0] = SPECIAL_MANEUVER_NONE;
            } else {
                g_anObjectYawRotation_00494fc8[0] -= g_nYawInput_004931aa;
                g_anObjectPitchRotation_00494f38[0] -= g_nPitchInput_004931a8;
            }
        }
    } else {
        g_anObjectPitchRotation_00494f38[0] = (short)(
            (g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[0]].yawRate *
             g_nPitchInput_004931a8) / 8);
        g_anObjectYawRotation_00494fc8[0] = (short)-(
            (g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[0]].pitchRate *
             g_nYawInput_004931aa) / 8);
        g_anObjectRollRotation_00495058[0] = (short)-(
            (g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[0]].rollRate *
             g_nRollInput_004931ac) / 8);
    }
}

/* Function start: 0x466AD2 */
short IsInputEventQueued(int type)
{
    if (FindQueuedInputEvent(type) != 0) {
        return 1;
    }
    return 0;
}

/* Function start: 0x466B02 */
short player_input(void)
{
    InputEventState event;
    short result;
    unsigned int buttons;
    int polledInput;
    short eventType;
    short afterburnerControl;
    short horizontalMagnitude;
    short verticalMagnitude;
    short threshold;
    short horizontal;
    short vertical;
    int savedPlayerInputActive;
    HostMouseMessage *mouse;

    result = 0;
    mouse = &g_stHostMouseMessage_005d10d0;
    buttons = 0;
    polledInput = 0;
    g_cPreviousKey_0049312c = g_cCurrentKey_00493128;
    g_nPreviousYawInput_004931b2 = g_nYawInput_004931aa;
    g_nPreviousPitchInput_004931b0 = g_nPitchInput_004931a8;
    g_nPreviousRollInput_004931b4 = g_nRollInput_004931ac;
    savedPlayerInputActive = g_bPlayerInputActive_005c8090;
    g_bPlayerInputActive_005c8090 = 1;
    ServiceInputDevices(15);
    g_bPolledFlightInputQueued_005c587a = IsInputEventQueued(7);
    g_bMouseMoveEventQueued_0049d7fc =
        (signed char)IsInputEventQueued(3);
    g_bDiscreteFlightInputQueued_005c57e4 =
        (short)(IsInputEventQueued(4) | IsInputEventQueued(6));
    if (g_bMouseMoveEventQueued_0049d7fc == 0)
        g_cCurrentKey_00493128 |= (signed char)0x80;
    if (IsInputEventQueued(5) != 0) {
        g_cCurrentKey_00493128 |= (signed char)0x80;
        if (g_bPolledFlightInputQueued_005c587a == 0 &&
            g_nCockpitControlState_0049d7ac == 0) {
            g_nYawInput_004931aa =
                g_nPitchInput_004931a8 =
                g_nRollInput_004931ac =
                g_bMouseMoveEventQueued_0049d7fc = 0;
        }
    }

    buttons = mouse->secondaryButton * 2 | mouse->primaryButton;
    if (buttons == 0) {
        g_bMouseFireButtonLatched_005c4b10 = 0;
    } else {
        if ((mouse->primaryButton & 1) != 0) {
            if ((buttons & 2) == 0)
                fire_players_lasers();
            else if (g_bSecondaryMouseButtonHeld_0049d7f4 == 1)
                fire_players_lasers();
            else
                g_cCurrentKey_00493128 = 0x1c;
        }
        if (buttons == 3)
            g_cCurrentKey_00493128 = 0x1c;
    }

    while ((eventType = GetNextInputEvent(&event)) != 0) {
        switch (eventType) {
        case 3:
            if (polledInput != 0)
                break;
            afterburnerControl = (short)(event.modifiers & 2);
            if (afterburnerControl != 0 &&
                g_aeSpecialManeuver_00495600[0] ==
                    SPECIAL_MANEUVER_AFTERBURNER)
                afterburnerControl = 0;
            if (g_nCockpitControlState_0049d7ac == 0) {
                SetPersonnelMousePosition(
                    (short)((g_stViewBuffer_005d2b00.left +
                             g_stViewBuffer_005d2b00.right) / 2),
                    (short)((g_stViewBuffer_005d2b00.top +
                             g_stViewBuffer_005d2b00.bottom) / 2));
                g_nCockpitControlState_0049d7ac = 1;
                g_nUiCursorFrame_005c8481 = 2;
            }
            horizontal = (short)(event.x -
                (g_stViewBuffer_005d2b00.right -
                 g_stViewBuffer_005d2b00.left) / 2 + 1);
            vertical = (short)(event.y -
                (g_stViewBuffer_005d2b00.bottom -
                 g_stViewBuffer_005d2b00.top) / 2);
            g_nPersonnelCursorX_005c8470 = event.x;
            g_nPersonnelCursorY_005c8472 = event.y;
            horizontalMagnitude = (short)abs((int)horizontal);
            verticalMagnitude = (short)abs((int)vertical);
            threshold = 0;
            while (g_asMouseYawThresholds_0049d7d8[threshold] <=
                   horizontalMagnitude)
                threshold++;
            if (horizontal < 0)
                horizontal = (short)-threshold;
            else
                horizontal = threshold;
            threshold = 0;
            while (g_asMousePitchThresholds_0049d7e8[threshold] <=
                   verticalMagnitude)
                threshold++;
            if (vertical < 0)
                vertical = (short)-threshold;
            else
                vertical = threshold;
            if ((int)event.x - 4 <= (int)g_stViewBuffer_005d2b00.left)
                horizontal = -8;
            if ((int)event.x + 4 >= (int)g_stViewBuffer_005d2b00.right)
                horizontal = 8;
            if ((int)event.y - 4 <= (int)g_stViewBuffer_005d2b00.top)
                vertical = -8;
            if ((int)event.y + 4 >= (int)g_stViewBuffer_005d2b00.bottom)
                vertical = 8;
            if (horizontal > 8)
                horizontal = 8;
            if (horizontal < -8)
                horizontal = -8;
            if (vertical > 8)
                vertical = 8;
            if (vertical < -8)
                vertical = -8;
            if (afterburnerControl != 0) {
                g_nCockpitControlGoal_0049d7d0 = 1;
                g_nRollInput_004931ac =
                    g_nMouseYawInput_0049d800 = horizontal;
                g_nMousePitchInput_0049d804 = (short)-vertical;
                accelerate((short)(g_nMousePitchInput_0049d804 / 2));
            } else if (g_nCockpitControlGoal_0049d7d0 == 1) {
                g_nCockpitControlGoal_0049d7d0 = 0;
                g_nRollInput_004931ac = 0;
                g_nYawInput_004931aa = g_nMouseYawInput_0049d800 = 0;
                g_nPitchInput_004931a8 = g_nMousePitchInput_0049d804 = 0;
                SetPersonnelMousePosition(
                    (short)((g_stViewBuffer_005d2b00.left +
                             g_stViewBuffer_005d2b00.right) / 2),
                    (short)((g_stViewBuffer_005d2b00.top +
                             g_stViewBuffer_005d2b00.bottom) / 2));
            } else {
                g_nRollInput_004931ac = 0;
                g_nYawInput_004931aa =
                    g_nMouseYawInput_0049d800 = horizontal;
                g_nPitchInput_004931a8 =
                    g_nMousePitchInput_0049d804 = vertical;
            }
            break;
        case 7:
            polledInput = 1;
            g_cCurrentKey_00493128 = (signed char)PollKeyboardState();
            if (g_cCurrentKey_00493128 != 0)
                process_player_input();
            g_nCockpitControlGoal_0049d7d0 = 0;
            g_nCockpitControlState_0049d7ac =
                g_nCockpitControlGoal_0049d7d0;
            afterburnerControl = (short)(event.modifiers & 2);
            if (afterburnerControl != 0 &&
                g_aeSpecialManeuver_00495600[0] ==
                    SPECIAL_MANEUVER_AFTERBURNER)
                afterburnerControl = 0;
            if (afterburnerControl != 0) {
                g_nRollInput_004931ac =
                    (short)g_stCurrentFlightInput_0049d7b0.x;
                accelerate((short)-(
                    g_stCurrentFlightInput_0049d7b0.y / 2));
            } else {
                if (g_nRollInput_004931ac != 0 &&
                    g_cCurrentKey_00493128 == 0) {
                    g_nRollInput_004931ac = 0;
                    g_stPreviousFlightInput_005c57d0.x = -1;
                }
                if (g_stPreviousFlightInput_005c57d0.x !=
                        g_stCurrentFlightInput_0049d7b0.x ||
                    g_stPreviousFlightInput_005c57d0.y !=
                        g_stCurrentFlightInput_0049d7b0.y) {
                    g_nYawInput_004931aa =
                        (short)g_stCurrentFlightInput_0049d7b0.x;
                    g_nPitchInput_004931a8 =
                        (short)-g_stCurrentFlightInput_0049d7b0.y;
                }
            }
            break;
        case 2:
            if (event.value == 2)
                g_bSecondaryMouseButtonHeld_0049d7f4 = 0;
            break;
        case 1:
            if ((event.modifiers & 1) != 0) {
                g_cCurrentKey_00493128 = 0x39;
                if ((event.modifiers & 2) == 0)
                    fire_players_lasers();
                else if (g_bSecondaryMouseButtonHeld_0049d7f4 == 1)
                    fire_players_lasers();
                else
                    g_cCurrentKey_00493128 = 0x1c;
            }
            if (event.value == 2 && event.status > 1)
                g_bSecondaryMouseButtonHeld_0049d7f4 = 1;
            break;
        case 6:
            result = 1;
        case 4:
            g_nCockpitControlGoal_0049d7d0 = 0;
            g_cCurrentKey_00493128 = (signed char)event.status;
            process_player_input();
            break;
        }
    }

    if (g_bSecondaryMouseButtonHeld_0049d7f4 == 1)
        g_cCurrentKey_00493128 = 0xf;
    if ((g_cCurrentKey_00493128 & (signed char)0x80) != 0 &&
        (g_stCurrentFlightInput_0049d7b0.buttons & 1) != 0)
        g_cCurrentKey_00493128 = 0x39;
    if ((g_cCurrentKey_00493128 & (signed char)0x80) != 0 &&
        (g_wPendingInputButtons_005c80d4 & 1) != 0)
        g_cCurrentKey_00493128 = 0x39;
    g_stPreviousFlightInput_005c57d0 =
        g_stCurrentFlightInput_0049d7b0;
    g_bPlayerInputActive_005c8090 = savedPlayerInputActive;
    return result;
}

/* Function start: 0x465E25 */
void SelectNextExternalViewObject(void)
{
#if 0
    short object;

    object = (short)g_cViewObject_0049313c;
    g_cViewObject_0049313c = -1;
    do {
        object++;
        if (object > 9)
            object = 0;
        if (g_aeObjectClass_00495328[object] >= OBJECT_CLASS_SHIP)
            g_cViewObject_0049313c = (signed char)object;
    } while (g_cViewObject_0049313c == -1);
    return 0;
#else
    signed char object;

    object = g_cViewObject_0049313c;
    g_cViewObject_0049313c = -1;
    while (g_cViewObject_0049313c == -1) {
        object++;
        if (object > 9)
            object = 0;
        if (g_aeObjectClass_00495328[object] >= OBJECT_CLASS_SHIP)
            g_cViewObject_0049313c = object;
    }
#endif
}

/* Function start: WC2_UNMAPPED */
unsigned int SelectPreviousExternalViewObject(void)
{
    short object;

    object = (short)g_cViewObject_0049313c;
    g_cViewObject_0049313c = -1;
    do {
        object--;
        if (object < 0)
            object = 9;
        if (g_aeObjectClass_00495328[object] >= OBJECT_CLASS_SHIP)
            g_cViewObject_0049313c = (signed char)object;
    } while (g_cViewObject_0049313c == -1);
    return 0;
}

/* Function start: 0x45E9F0 */
short HandleFleetOverviewInput(void)
{
    InputEventState event;
    short result;
    int fired;
    signed char discreteInput;
    short eventType;
    short horizontal;
    short vertical;
    short horizontalMagnitude;
    short verticalMagnitude;
    short threshold;
    short notRepeated;
    short pauseModifiers;
    short unusedModifiers;

    result = 0;
    fired = 0;
    g_cPreviousKey_0049312c = g_cCurrentKey_00493128;
    ServiceInputDevices(15);
    g_bPolledFlightInputQueued_005c587a = IsInputEventQueued(7);
    g_bFleetMouseMoveQueued_005c57ea = IsInputEventQueued(3);
    g_bDiscreteFlightInputQueued_005c57e4 =
        (short)(IsInputEventQueued(4) | IsInputEventQueued(6));
    g_bFleetPrimaryInputQueued_005c57dc = IsInputEventQueued(1);
    discreteInput = 0;
    if (g_bFleetMouseMoveQueued_005c57ea == 0)
        g_cCurrentKey_00493128 |= (signed char)0x80;
    if (g_bDiscreteFlightInputQueued_005c57e4 == 0) {
        g_cCurrentKey_00493128 |= (signed char)0x80;
        if (g_bPolledFlightInputQueued_005c587a == 0 &&
            g_nCockpitControlState_0049d7ac == 0)
            g_bFleetPolledInputActive_0049d420 = 0;
    } else {
        discreteInput = 1;
    }
    if (g_bFleetPrimaryInputQueued_005c57dc == 0 &&
        g_stHostMouseMessage_005d10d0.primaryButton != 0) {
        g_cCurrentKey_00493128 = 0x39;
        fired = 1;
    }
    if (g_bFleetPrimaryInputQueued_005c57dc == 0)
        g_bFleetButtonInputActive_0049d424 = 0;

    while ((eventType = GetNextInputEvent(&event)) != 0) {
        switch (eventType) {
        case 3:
            if (g_nCockpitControlState_0049d7ac == 0) {
                SetPersonnelMousePosition(
                    (short)((g_stViewBuffer_005d2b00.left +
                             g_stViewBuffer_005d2b00.right) / 2),
                    (short)((g_stViewBuffer_005d2b00.top +
                             g_stViewBuffer_005d2b00.bottom) / 2));
                g_nCockpitControlState_0049d7ac = 1;
                g_nUiCursorFrame_005c8481 = 2;
            }
            horizontal = (short)(
                event.x -
                (g_stViewBuffer_005d2b00.right -
                 g_stViewBuffer_005d2b00.left) / 2 + 1);
            vertical = (short)(
                event.y -
                (g_stViewBuffer_005d2b00.bottom -
                 g_stViewBuffer_005d2b00.top) / 2);
            g_nPersonnelCursorX_005c8470 = event.x;
            g_nPersonnelCursorY_005c8472 = event.y;
            horizontalMagnitude = (short)labs((long)horizontal);
            verticalMagnitude = (short)labs((long)vertical);
            threshold = 0;
            while (g_asFleetMouseYawThresholds_0049d400[threshold] <=
                   horizontalMagnitude)
                threshold++;
            if (horizontal < 0)
                horizontal = (short)-threshold;
            else
                horizontal = threshold;
            threshold = 0;
            while (g_asFleetMousePitchThresholds_0049d410[threshold] <=
                   verticalMagnitude)
                threshold++;
            if (vertical < 0)
                vertical = (short)-threshold;
            else
                vertical = threshold;
            if ((int)event.x - 4 <=
                (int)g_stViewBuffer_005d2b00.left)
                horizontal = -8;
            if ((int)event.x + 4 >=
                (int)g_stViewBuffer_005d2b00.right)
                horizontal = 8;
            if ((int)event.y - 4 <=
                (int)g_stViewBuffer_005d2b00.top)
                vertical = -8;
            if ((int)event.y + 4 >=
                (int)g_stViewBuffer_005d2b00.bottom)
                vertical = 8;
            if (horizontal >= 9)
                horizontal = 8;
            if (horizontal <= -9)
                horizontal = -8;
            if (vertical >= 9)
                vertical = 8;
            if (vertical <= -9)
                vertical = -8;
            g_nFleetMouseYawVelocity_0049d428 =
                (short)-horizontal;
            g_nFleetOverviewYawVelocity_0049d3ec =
                g_nFleetMouseYawVelocity_0049d428;
            g_nFleetMousePitchVelocity_0049d42c = vertical;
            g_nFleetOverviewPitchVelocity_0049d3f0 =
                g_nFleetMousePitchVelocity_0049d42c;
            break;
        case 7:
            g_nCockpitControlState_0049d7ac = 0;
            if (g_nTargetCameraFrame_0049d3e8 == 0) {
                g_nFleetOverviewYawVelocity_0049d3ec =
                    (short)-g_stCurrentFlightInput_0049d7b0.x;
                g_nFleetOverviewPitchVelocity_0049d3f0 =
                    (short)-g_stCurrentFlightInput_0049d7b0.y;
            }
            break;
        case 1:
            g_nCockpitControlState_0049d7ac = 0;
            g_cCurrentKey_00493128 = 0x39;
            fired = 1;
            break;
        case 5:
            g_nCockpitControlState_0049d7ac = 0;
            g_nFleetOverviewYawVelocity_0049d3ec = 0;
            g_nFleetOverviewPitchVelocity_0049d3f0 =
                g_nFleetOverviewYawVelocity_0049d3ec;
            break;
        case 6:
            result = 1;
        case 4:
            g_nCockpitControlState_0049d7ac = 0;
            g_nCockpitControlGoal_0049d7d0 = 0;
            g_cCurrentKey_00493128 = (signed char)event.status;
            notRepeated =
                g_cPreviousKey_0049312c != g_cCurrentKey_00493128 &&
                result == 0;
            pauseModifiers =
                (short)(g_wInputDeviceButtonState_005c8432 & 0x1800);
            unusedModifiers =
                (short)(g_wInputDeviceButtonState_005c8432 & 0x600);
            switch ((signed char)g_cCurrentKey_00493128) {
            case 0x1c:
            case 0x39:
                g_cCurrentKey_00493128 = 0x39;
                fired = 1;
                break;
            case 0x50:
                if (g_nFleetOverviewPitchVelocity_0049d3f0 > -8)
                    g_nFleetOverviewPitchVelocity_0049d3f0--;
                break;
            case 0x48:
                if (g_nFleetOverviewPitchVelocity_0049d3f0 < 8)
                    g_nFleetOverviewPitchVelocity_0049d3f0++;
                break;
            case 0x4b:
                if (g_nFleetOverviewYawVelocity_0049d3ec < 8)
                    g_nFleetOverviewYawVelocity_0049d3ec++;
                break;
            case 0x4d:
                if (g_nFleetOverviewYawVelocity_0049d3ec > -8)
                    g_nFleetOverviewYawVelocity_0049d3ec--;
                break;
            case 0x26:
                if (g_cPreviousKey_0049312c !=
                    g_cCurrentKey_00493128) {
                    g_bTargetLockMode_00493500 =
                        (short)(g_bTargetLockMode_00493500 == 0);
                    PlaySfxWaveFileByNumber(0x19, -1, 0);
                }
                break;
            case 0x14:
                if (g_cPreviousKey_0049312c !=
                    g_cCurrentKey_00493128)
                    cycle_onscreen_targets();
                break;
            case 0x22:
                ClearTargetCameraView();
            case 0x3b:
            case 0x3c:
            case 0x3d:
            case 0x3e:
            case 0x3f:
            case 0x40:
            case 0x41:
                break;
            case 0x19:
                ShowGamePausedBanner(pauseModifiers < 1);
            default:
                g_cCurrentKey_00493128 = (signed char)0x80;
                break;
            }
            break;
        }
    }

    if (g_nTargetCameraMode_005c8d50 == 0) {
        if ((g_cCurrentKey_00493128 & 0x80) != 0 &&
            (g_stCurrentFlightInput_0049d7b0.buttons & 1) != 0)
            g_cCurrentKey_00493128 = 0x39;
    } else if ((g_cCurrentKey_00493128 & 0x80) != 0 &&
               (g_stCurrentFlightInput_0049d7b0.buttons & 1) != 0 &&
               (g_stPreviousFlightInput_005c57d0.buttons & 1) == 0) {
        g_cCurrentKey_00493128 = 0x39;
    }
    if (g_nTargetCameraMode_005c8d50 == 0 &&
        (g_cCurrentKey_00493128 & 0x80) != 0 &&
        (g_wPendingInputButtons_005c80d4 & 1) != 0)
        g_cCurrentKey_00493128 = 0x39;
    if (g_cPreviousKey_0049312c != g_cCurrentKey_00493128 &&
        g_cCurrentKey_00493128 == 0x39) {
        if (g_nTargetCameraMode_005c8d50 == 0) {
            FireTargetCameraGuns();
        } else if (g_nTargetCameraZoom_0049d3e4 >= 0x21 &&
                   g_nTargetCameraFrame_0049d3e8 == 0) {
            ToggleTargetCameraTracking();
        }
        g_cCurrentKey_00493128 = (signed char)0x80;
    }
    if (g_nTargetCameraFrame_0049d3e8 == 1 &&
        (g_nCurrentView_00492fa8 != 4 ||
         g_nTargetCameraMode_005c8d50 != 1 || fired == 0))
        ClearTargetCameraView();
    g_stPreviousFlightInput_005c57d0 =
        g_stCurrentFlightInput_0049d7b0;
    if (g_nTargetCameraGunCooldown_0049d33c > 0)
        g_nTargetCameraGunCooldown_0049d33c--;
    else
        g_nTargetCameraGunCooldown_0049d33c = 0;
    return result;
}

/* Function start: 0x45F200 */
void UpdateFleetOverviewCameraRotation(void)
{
    char yawRotated;

    yawRotated = 0;
    if (g_nFleetOverviewYawVelocity_0049d3ec != 0 &&
        ((g_nFleetOverviewYaw_0049d3f4 < 65 &&
          g_nFleetOverviewYaw_0049d3f4 > -65) ||
         (g_nFleetOverviewYaw_0049d3f4 == 65 &&
          g_nFleetOverviewYawVelocity_0049d3ec < 0) ||
         (g_nFleetOverviewYaw_0049d3f4 == -65 &&
          g_nFleetOverviewYawVelocity_0049d3ec > 0))) {
        if (g_nFleetOverviewYaw_0049d3f4 +
                g_nFleetOverviewYawVelocity_0049d3ec > 65)
            g_nFleetOverviewYawVelocity_0049d3ec =
                (short)(65 - g_nFleetOverviewYaw_0049d3f4);
        else if (g_nFleetOverviewYaw_0049d3f4 +
                     g_nFleetOverviewYawVelocity_0049d3ec < -65)
            g_nFleetOverviewYawVelocity_0049d3ec =
                (short)(-65 - g_nFleetOverviewYaw_0049d3f4);
        rotate_about_j(
            g_nFleetOverviewYawVelocity_0049d3ec,
            &g_aShipRightVector_00493b78[WC2_EYE_OBJECT],
            &g_aShipForwardVector_00494208[WC2_EYE_OBJECT]);
        g_nFleetOverviewYaw_0049d3f4 =
            (short)(g_nFleetOverviewYaw_0049d3f4 +
                    g_nFleetOverviewYawVelocity_0049d3ec);
        yawRotated++;
    }
    if (g_nFleetOverviewPitchVelocity_0049d3f0 != 0 &&
        yawRotated == 0 &&
        ((g_nFleetOverviewPitch_0049d3f8 < 65 &&
          g_nFleetOverviewPitch_0049d3f8 > -65) ||
         (g_nFleetOverviewPitch_0049d3f8 >= 65 &&
          g_nFleetOverviewPitchVelocity_0049d3f0 < 0) ||
         (g_nFleetOverviewPitch_0049d3f8 <= -65 &&
          g_nFleetOverviewPitchVelocity_0049d3f0 > 0))) {
        if (g_nFleetOverviewPitch_0049d3f8 +
                g_nFleetOverviewPitchVelocity_0049d3f0 > 65)
            g_nFleetOverviewPitchVelocity_0049d3f0 =
                (short)(65 - g_nFleetOverviewPitch_0049d3f8);
        else if (g_nFleetOverviewPitch_0049d3f8 +
                     g_nFleetOverviewPitchVelocity_0049d3f0 < -65)
            g_nFleetOverviewPitchVelocity_0049d3f0 =
                (short)(-65 - g_nFleetOverviewPitch_0049d3f8);
        rotate_about_i(
            g_nFleetOverviewPitchVelocity_0049d3f0,
            &g_aShipUpVector_00493ec0[WC2_EYE_OBJECT],
            &g_aShipForwardVector_00494208[WC2_EYE_OBJECT]);
        g_nFleetOverviewPitch_0049d3f8 =
            (short)(g_nFleetOverviewPitch_0049d3f8 +
                    g_nFleetOverviewPitchVelocity_0049d3f0);
    }
    fix_objects_ijk(WC2_EYE_OBJECT);
}

/* Function start: 0x45F439 */
void FireTargetCameraGuns(void)
{
    short projectileSpeed;
    short projectileType;
    short shot;
    short projectile;
    ObjectTypeData *projectileData;
    FixedVector direction;
    short leadDistance;

    projectileSpeed = 10;
    projectileType = 8;
    if (g_nGunDisplayEnergyPercent_005c8d4e > 0 &&
        g_nTargetCameraGunCooldown_0049d33c < 1) {
        shot = 0;
        while (shot < 2) {
            projectile = new_object(projectileType, 0);
            if (projectile != -1) {
                g_abProjectileCollisionBonus_004960a8[projectile] = 1;
                projectileData =
                    &g_aObjectTypeData_00496d30[projectileType];
                copy_frame(WC2_EYE_OBJECT, projectile);
                g_asObjectDamage_00495178[projectile] =
                    projectileData->damageCapacity;
                projectileSpeed = (short)(
                    g_aObjectTypeData_00496d30[
                        g_acObjectType_00493980[projectile]]
                        .maximumVelocity * 2);
                g_nGunDisplayEnergyPercent_005c8d4e =
                    (short)(g_nGunDisplayEnergyPercent_005c8d4e -
                            projectileData->animationDelay / 2);
                g_aShipPosition_00494550[projectile].x =
                    g_aaTargetCameraGunOffsets_0049d3c8[
                        g_nGunDisplayIndex_005c8dc0][shot].y *
                        g_aShipUpVector_00493ec0[WC2_EYE_OBJECT].x +
                    g_aaTargetCameraGunOffsets_0049d3c8[
                        g_nGunDisplayIndex_005c8dc0][shot].z *
                        g_aShipForwardVector_00494208[
                            WC2_EYE_OBJECT].x +
                    g_aaTargetCameraGunOffsets_0049d3c8[
                        g_nGunDisplayIndex_005c8dc0][shot].x *
                        g_aShipRightVector_00493b78[
                            WC2_EYE_OBJECT].x +
                    g_aShipPosition_00494550[WC2_EYE_OBJECT].x;
                g_aShipPosition_00494550[projectile].y =
                    g_aaTargetCameraGunOffsets_0049d3c8[
                        g_nGunDisplayIndex_005c8dc0][shot].y *
                        g_aShipUpVector_00493ec0[WC2_EYE_OBJECT].y +
                    g_aaTargetCameraGunOffsets_0049d3c8[
                        g_nGunDisplayIndex_005c8dc0][shot].z *
                        g_aShipForwardVector_00494208[
                            WC2_EYE_OBJECT].y +
                    g_aaTargetCameraGunOffsets_0049d3c8[
                        g_nGunDisplayIndex_005c8dc0][shot].x *
                        g_aShipRightVector_00493b78[
                            WC2_EYE_OBJECT].y +
                    g_aShipPosition_00494550[WC2_EYE_OBJECT].y;
                g_aShipPosition_00494550[projectile].z =
                    g_aaTargetCameraGunOffsets_0049d3c8[
                        g_nGunDisplayIndex_005c8dc0][shot].y *
                        g_aShipUpVector_00493ec0[WC2_EYE_OBJECT].z +
                    g_aaTargetCameraGunOffsets_0049d3c8[
                        g_nGunDisplayIndex_005c8dc0][shot].z *
                        g_aShipForwardVector_00494208[
                            WC2_EYE_OBJECT].z +
                    g_aaTargetCameraGunOffsets_0049d3c8[
                        g_nGunDisplayIndex_005c8dc0][shot].x *
                        g_aShipRightVector_00493b78[
                            WC2_EYE_OBJECT].z +
                    g_aShipPosition_00494550[WC2_EYE_OBJECT].z;
                g_asObjectCounter_00494be0[projectile] =
                    projectileData->lifetime;
                zero_vector(&g_aShipVelocity_00494898[projectile]);
                direction =
                    g_aShipForwardVector_00494208[WC2_EYE_OBJECT];
                NormalizeFixedVector(&direction);
                leadDistance = (short)(
                    (projectileData->lifetime + 5) *
                    projectileData->maximumVelocity);
                ScaleFixedVector(
                    &direction, (int)leadDistance << 8, &direction);
                AddFixedVectors(
                    &g_aShipPosition_00494550[WC2_EYE_OBJECT],
                    &direction, &direction);
                point_at(projectile, direction);
                ScaleFixedVector(
                    &g_aShipForwardVector_00494208[projectile],
                    (int)projectileSpeed << 8, &direction);
                g_aShipVelocity_00494898[projectile] = direction;
                RecordCannedSceneObjectEvent(projectile, 0);
                g_nTargetCameraGunCooldown_0049d33c =
                    (short)(g_acGunRefireDelay_00492e1c[
                                projectileType - 7] >> 1);
                PlaySfxWaveFileByNumber(8, projectile, 0);
            }
            shot++;
        }
    }
}
