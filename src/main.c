/*
 *  Main application loop and spaceflight frame.
 *
 *  Retail object 0x465780-0x46979F (link order; see docs/ORDER.md).
 */
#include "game.h"

unsigned char g_abPauseInputState_005c85d0[0x100];

unsigned char g_abPreviousPauseInputState_005c86e0[0x100];

#pragma function(strcpy)
/* Function start: 0x465780 */
void SetPersonnelCursorPosition(int x, int y, short unused1,
                                short unused2)
{
    g_nPersonnelCursorX_005c8470 = x;
    g_nPersonnelCursorY_005c8472 = y;
}

/* Function start: 0x46579D */
short LogMemoryUsage(void)
{
    /* WC2's system.c declares the canned-scene tape file as a pointer where
     * it is really the string itself, so the unlink is handed the first four
     * bytes of "tape.tmp" and always fails.  The port cannot dereference a
     * 32-bit value as a pointer, so it removes the file as intended. */
#ifdef SDL_PORT
    _unlink(g_szCannedSceneTapeFile_00490208);
#else
    _unlink(*(char *const *)g_szCannedSceneTapeFile_00490208);
#endif
    StopMusicIfDriverActive();
    EMShutDown();
    if (g_bSpeechEnabled_005c8de8 != 0)
        ShutdownSoundBlaster();
    if (g_bParentDirectorySelected_0049d78c == 0) {
        _chdir("..");
        g_bParentDirectorySelected_0049d78c++;
    }
    ConfigureDefaultSpacePalette(g_nDetectedGraphicsMode_005c80d2);
    if (g_nOriginDevUnlock_0049d774 != 0) {
        printf("Series %d, Mission %d (%c)\n",
               (int)g_nCurrentSeries_005c5870,
               (int)g_nCurrentMission_005c5878,
               (int)g_nCurrentMission_005c5878 + 'A');
    }
    if (g_nShowMemoryStatus_0049d784 != 0) {
        printf(
            "initial far: %lu.  current far: %lu.  current near: %u.\n",
            g_dwInitialFreeMemory_005c8dd0,
            GetLargestFreeMemoryBlockByType(0), 0);
    }
    FinalizeInputManagerHook();
    MessagePumpHook(8);
    if (g_pMemoryAdjustment_0049d788 != 0)
        free(g_pMemoryAdjustment_0049d788);
    return 0;
}

/* Function start: 0x46589D */
void AdjustSpaceFramePeriod(short adjustment)
{
    g_nSpaceFramePeriod_0049d768 = MinShort(
        MaxShort((short)(g_nSpaceFramePeriod_0049d768 + adjustment), 1),
        0x13);
    ShowOnScreenMessage(0, "%d/60th SEC FRAME DURATION.",
                        g_nSpaceFramePeriod_0049d768 + 1);
}

/* Function start: 0x4658E9 */
/* Nothing in the shipped executable calls this.  The DOS near-heap check;
 * the Win32 port always reports a sound heap. */
short CheckHeap(void)
{
    short status;

    status = 1;
    return status;
}

/* Function start: 0x465906 */
/* Nothing in the shipped executable calls this.  The DOS Ctrl-Break
 * handler: returning 1 lets the program continue. */
short CtrlBreakHandler(void)
{
    return 1;
}

/* Function start: 0x46591A */
void RunGameApplication(short argc, char **argv)
{
    short argumentCount;
    int memoryBytes;
    short selectionMade;

    selectionMade = 0;
    argumentCount = LoadWingCmdrCfgFile(argc, argv);
    _chdir("gamedat");
    printf("Loading WING COMMANDER 2.  Please wait...\n");
    g_bHighMemoryResourcesEnabled_005c80e4 = 1;
    g_stDefaultPilotProfile_00492660.field_5e = 5;
    memcpy(&g_stCurrentPilotProfile_00493408,
           &g_stDefaultPilotProfile_00492660,
           sizeof(g_stCurrentPilotProfile_00493408));
#ifdef SDL_PORT
    GameMain(argumentCount, g_pStartupArguments_005c57f0);
#else
    main(argumentCount, g_pStartupArguments_005c57f0);
#endif

    if (g_bMemoryAdjustmentEnabled_0049cc84 != 0) {
        memoryBytes = (unsigned short)g_nMemoryAdjustmentKb_005c8dda << 10;
        g_pMemoryAdjustment_0049d788 =
            AllocateTaggedMemory((unsigned int)memoryBytes, 0x40);
        if (g_pMemoryAdjustment_0049d788 != 0)
#ifdef SDL_PORT
            printf("Adjusting Mem size by %dK\n",
                   g_nMemoryAdjustmentKb_005c8dda);
#else
            /* This unit passed the short as a full dword (MOV EAX,[g]),
             * so the next global's bytes ride along in the high half. */
            printf("Adjusting Mem size by %dK\n",
                   *(int *)&g_nMemoryAdjustmentKb_005c8dda);
#endif
    }

    g_stCurrentPilotProfile_00493408.series = g_nDirectSeries_0049d79c;
    g_stDefaultPilotProfile_00492660.series =
        g_stCurrentPilotProfile_00493408.series;
    g_stCurrentPilotProfile_00493408.mission = g_stDirectMissionSelection_0049d798.mission;
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

    if (g_stDirectMissionSelection_0049d798.launch == 0) {
#ifdef SDL_PORT
        if (SdlOriginalTitleMusicReady() &&
            SdlOriginalTitleSequenceAvailable())
            Title_Sequence();
#endif
        g_bSceneEscapeRequested_0049d4b0 = 0;
        for (;;) {
            if (g_nOriginDevUnlock_0049d774 != 0 &&
                g_bDeveloperCampaignReady_004926c4 != 0 &&
                g_bDirectCampaignSelection_0049cc74 != 0) {
                g_nSelectedCampaignSlot_005d3bf2 = 0;
                while (RunSelectedCampaign() != 0) {
                }
            } else {
                while (selectionMade == 0) {
                    switch (RunTitleScreen()) {
                    case 0:
                        g_nSelectedCampaignSlot_005d3bf2 =
                            BarracksScreen();
                        InitializeNewPilotCampaign(
                            g_nSelectedCampaignSlot_005d3bf2);
                        selectionMade = RunPilotDatabaseMenu();
                        break;
                    case 1:
                        g_nSelectedCampaignSlot_005d3bf2 =
                            LoadSelectedPilotCampaign();
                        selectionMade = 1;
                        break;
                    }
                }
                selectionMade = 0;
                if (g_nOriginDevUnlock_0049d774 != 0) {
                    strcpy(g_stCurrentPilotProfile_00493408.callsign,
                           "CHEATER");
                    strcpy(g_stPilotNameVariables_00499ef8.callsign,
                           "CHEATER");
                }
                while (RunSelectedCampaign() != 0) {
                }
            }
        }
    } else {
        g_nCurrentSeries_005c5870 = g_nDirectSeries_0049d79c;
        g_nCurrentMission_005c5878 = g_stDirectMissionSelection_0049d798.mission;
        LoadTemporaryCampaignGlobals();
        g_pCampaignGlobals_00499c94->field_08 = 0;
        InitializeCampaignConstellationState(
            g_pCampaignGlobals_00499c94, 1);
        SaveAndFreeTemporaryCampaignGlobals();
        init_mission(g_nDirectSeries_0049d79c,
                     g_stDirectMissionSelection_0049d798.mission);
        RunSpaceFlight(g_nMissionEntryNavOverride_0049d790);
        LogMemoryUsage();
        printf("You ");
        switch (g_nArcadeState_0049d75c) {
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
    }
    if (g_pMemoryAdjustment_0049d788 != 0) {
        free(g_pMemoryAdjustment_0049d788);
    }
}

#pragma intrinsic(strcpy)
/* Function start: 0x465CBC */
void free_view_buffer(void)
{
    if (message_showing())
        EndCommMenu();
    if (g_stViewBuffer_005d2b00.pixels != 0)
        free_viewport(&g_stViewBuffer_005d2b00);
}

/* Function start: 0x465CF6 */
void initialize_view_buffer(void)
{
    short allocated;

    if (g_bSpaceViewBufferEnabled_0049d7a4 != 0 &&
        g_stViewBuffer_005d2b00.pixels == 0) {
        allocated = (short)AllocateViewport(
            &g_stViewBuffer_005d2b00,
            (short)g_cPrimaryViewBufferColour_0049cb88, 0x20);
        if (allocated == 0)
            ReportOutOfMemoryAndExit("SPACE BUFFER");
    }
}

/* Function start: 0x465D55 */
void dump_buffer_to_screen(void)
{
#ifdef SDL_PORT
    SdlCompleteSpaceFrame();
#endif
    if (g_nCockpitDisplayMode_0049d71c > 0) {
        CopyViewportContents(
            &g_stViewBuffer_005d2b00, &g_stScreenViewport_005d21a0);
    } else {
        switch (g_cScreenViewportMode_005c82a6) {
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
            CopyViewportRuns(
                &g_stViewBuffer_005d2b00, &g_stScreenViewport_005d21a0,
                (const short *)g_pScreenViewportGeometry_005c82b0);
            break;
        }
    }
    if (g_nShowMemoryStatus_0049d784 != 0)
        ShowMemoryStatusDebug();
}

/* Function start: 0x465E25 */
void SelectNextExternalViewObject(void)
{
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
}

/* Function start: 0x465E88 */
unsigned int clear_view_buffer(void)
{
    ClearViewport(&g_stViewBuffer_005d2b00, (unsigned char)g_cPrimaryViewBufferColour_0049cb88);
#ifdef SDL_PORT
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
                          g_cRedColour_0049cb5c);
            g_bViewportDirty_0049d76c = 1;
            g_asSpacePaletteFade_005d2d60[0] = 0;
            break;
        }
    }
}

/* Function start: 0x46604F */
void house_keep(void)
{
    unsigned char palette;
    short sequenceState;

    if (g_nTrainSimActive_0049d758 == 0 &&
        g_nCannedSceneMode_0049021c == 0) {
        if (g_ucPendingEjectionTransition_0049b8ac != 0xff &&
            g_nPendingEjectionSequenceCount_0049b8b8 != 0) {
            sequenceState = g_nEjectionSequenceState_0049b8c0;
            g_nEjectionSequenceState_0049b8c0++;
            if (sequenceState == 0x28) {
                g_nEjectionSequenceState_0049b8c0 = 0;
                g_nPendingEjectionSequenceCount_0049b8b8 = 0;
                ejection_sequence(g_ucPendingEjectionTransition_0049b8ac, 1);
                g_ucPendingEjectionTransition_0049b8ac = 0xff;
            }
        }
        if ((g_nSpaceFrame_00493134 & 0x1f) == 0)
            ReleaseStaleNavTarget();
        if (g_nHazardFieldCount_004931d0 != 0 &&
            (g_nSpaceFrame_00493134 & 0xf) == 0)
            check_hazards();
    }
    if (g_nCurrentView_00492fa8 == 0 || g_nCurrentView_00492fa8 == 4) {
        for (palette = 0; palette < 6; palette++) {
            FadeFlightPaletteEntry(
                g_aasCockpitHitPaletteFades_005d2cb0[palette]);
            SetPaletteEntry((short)(palette + 0xf9),
                            g_aasCockpitHitPaletteFades_005d2cb0[palette]);
        }
    } else if (g_nCriticalDamageWarningSfxHandle_005d1ec0 != 0) {
        ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(
            g_nCriticalDamageWarningSfxHandle_005d1ec0, 1);
        g_nCriticalDamageWarningSfxHandle_005d1ec0 = 0;
        g_abCockpitLightGoal_005d1eb8[3] = 0;
    }
}

/* Function start: 0x4661C2 */
void get_player_input(void)
{
    /* t and ev held the DOS double-click logic; the port left them unused. */
    long t;
    short buttons;
    short ev;

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
            g_anFlightInput_004931a8[1] =
                g_anFlightInput_004931a8[0] =
                g_anFlightInput_004931a8[2] = 0;
            handled = 1;
            break;
        case 0x48:
            g_nCockpitControlState_0049d7ac = 0;
            if (g_anFlightInput_004931a8[0] < 0) {
                g_anFlightInput_004931a8[0] = 0;
            } else {
                if (shift != 0)
                    g_anFlightInput_004931a8[0] = 9;
                if (g_anFlightInput_004931a8[0] < 9)
                    g_anFlightInput_004931a8[0]++;
                else if (g_cPreviousKey_0049312c < 0)
                    g_anFlightInput_004931a8[0]++;
                else
                    g_anFlightInput_004931a8[0]--;
            }
            handled = 1;
            break;
        case 0x50:
            g_nCockpitControlState_0049d7ac = 0;
            if (g_anFlightInput_004931a8[0] > 0) {
                g_anFlightInput_004931a8[0] = 0;
            } else {
                if (shift != 0)
                    g_anFlightInput_004931a8[0] = -9;
                if (g_anFlightInput_004931a8[0] > -9) {
                    g_anFlightInput_004931a8[0]--;
                } else {
                    if (g_cPreviousKey_0049312c < 0)
                        g_anFlightInput_004931a8[0]--;
                    else
                        g_anFlightInput_004931a8[0]++;
                }
            }
            handled = 1;
            break;
        case 0x34:
        case 0x53:
            g_nCockpitControlState_0049d7ac = 0;
            if (g_anFlightInput_004931a8[2] < 0) {
                g_anFlightInput_004931a8[2] = 0;
            } else {
                if (shift != 0)
                    g_anFlightInput_004931a8[2] = 9;
                if (g_anFlightInput_004931a8[2] < 9)
                    g_anFlightInput_004931a8[2]++;
                else if (g_cPreviousKey_0049312c < 0)
                    g_anFlightInput_004931a8[2]++;
                else
                    g_anFlightInput_004931a8[2]--;
            }
            handled = 1;
            break;
        case 0x33:
        case 0x52:
            g_nCockpitControlState_0049d7ac = 0;
            if (g_anFlightInput_004931a8[2] > 0) {
                g_anFlightInput_004931a8[2] = 0;
            } else {
                if (shift != 0)
                    g_anFlightInput_004931a8[2] = -9;
                if (g_anFlightInput_004931a8[2] > -9) {
                    g_anFlightInput_004931a8[2]--;
                } else {
                    if (g_cPreviousKey_0049312c < 0)
                        g_anFlightInput_004931a8[2]--;
                    else
                        g_anFlightInput_004931a8[2]++;
                }
            }
            handled = 1;
            break;
        case 0x4d:
            g_nCockpitControlState_0049d7ac = 0;
            if (g_anFlightInput_004931a8[1] < 0) {
                g_anFlightInput_004931a8[1] = 0;
            } else {
                if (shift != 0)
                    g_anFlightInput_004931a8[1] = 9;
                if (g_anFlightInput_004931a8[1] < 9)
                    g_anFlightInput_004931a8[1]++;
                else if (g_cPreviousKey_0049312c < 0)
                    g_anFlightInput_004931a8[1]++;
                else
                    g_anFlightInput_004931a8[1]--;
            }
            handled = 1;
            break;
        case 0x4b:
            g_nCockpitControlState_0049d7ac = 0;
            if (g_anFlightInput_004931a8[1] > 0) {
                g_anFlightInput_004931a8[1] = 0;
            } else {
                if (shift != 0)
                    g_anFlightInput_004931a8[1] = -9;
                if (g_anFlightInput_004931a8[1] > -9) {
                    g_anFlightInput_004931a8[1]--;
                } else {
                    if (g_cPreviousKey_0049312c < 0)
                        g_anFlightInput_004931a8[1]--;
                    else
                        g_anFlightInput_004931a8[1]++;
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
#ifdef SDL_PORT
    SdlTracef("[fire] counter=%d energy=%d gun=0x%x releaseIdx=%d\n",
                 (int)g_asObjectCounter_00494be0[0],
                 (int)g_asShipWeaponEnergy_00495590[0],
                 (unsigned)g_nSelectedGunType_004934dc,
                 (int)g_nSelectedReleaseWeaponIndex_004934e0);
#endif
    if (g_asObjectCounter_00494be0[0] == -1 &&
        g_asShipWeaponEnergy_00495590[0] > 0) {
        fire_fixed_projectile_weapon(0);
        if (g_acShipTarget_00495f20[0] != -1 &&
            get_mode(1) == 5)
            SelectCockpitVduMode(1, 3);
    }
#ifdef SDL_PORT
    return 0;
#endif
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
                g_anObjectYawRotation_00494fc8[0] -= g_anFlightInput_004931a8[1];
                g_anObjectPitchRotation_00494f38[0] -= g_anFlightInput_004931a8[0];
            }
        }
    } else {
        g_anObjectPitchRotation_00494f38[0] = (short)(
            (g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[0]].yawRate *
             g_anFlightInput_004931a8[0]) / 8);
        g_anObjectYawRotation_00494fc8[0] = (short)-(
            (g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[0]].pitchRate *
             g_anFlightInput_004931a8[1]) / 8);
        g_anObjectRollRotation_00495058[0] = (short)-(
            (g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[0]].rollRate *
             g_anFlightInput_004931a8[2]) / 8);
    }
}

/* Function start: 0x466AD2 */
short IsInputEventQueued(int type)
{
    return FindQueuedInputEvent(type) != 0;
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
#ifdef SDL_PORT
    signed char trailingKeyRelease;

    trailingKeyRelease = 0;
#endif

    result = 0;
    mouse = &g_stHostMouseMessage_005d10d0;
    buttons = 0;
    polledInput = 0;
    g_cPreviousKey_0049312c = g_cCurrentKey_00493128;
    g_anPreviousFlightInput_004931b0[1] = g_anFlightInput_004931a8[1];
    g_anPreviousFlightInput_004931b0[0] = g_anFlightInput_004931a8[0];
    g_anPreviousFlightInput_004931b0[2] = g_anFlightInput_004931a8[2];
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
            g_anFlightInput_004931a8[1] =
                g_anFlightInput_004931a8[0] =
                g_anFlightInput_004931a8[2] =
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
                g_anFlightInput_004931a8[2] =
                    g_nMouseYawInput_0049d800 = horizontal;
                g_nMousePitchInput_0049d804 = (short)-vertical;
                accelerate((short)(g_nMousePitchInput_0049d804 / 2));
            } else if (g_nCockpitControlGoal_0049d7d0 == 1) {
                g_nCockpitControlGoal_0049d7d0 = 0;
                g_anFlightInput_004931a8[2] = 0;
                g_anFlightInput_004931a8[1] = g_nMouseYawInput_0049d800 = 0;
                g_anFlightInput_004931a8[0] = g_nMousePitchInput_0049d804 = 0;
                SetPersonnelMousePosition(
                    (short)((g_stViewBuffer_005d2b00.left +
                             g_stViewBuffer_005d2b00.right) / 2),
                    (short)((g_stViewBuffer_005d2b00.top +
                             g_stViewBuffer_005d2b00.bottom) / 2));
            } else {
                g_anFlightInput_004931a8[2] = 0;
                g_anFlightInput_004931a8[1] =
                    g_nMouseYawInput_0049d800 = horizontal;
                g_anFlightInput_004931a8[0] =
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
                g_anFlightInput_004931a8[2] =
                    (short)g_stCurrentFlightInput_0049d7b0.x;
                accelerate((short)-(
                    g_stCurrentFlightInput_0049d7b0.y / 2));
            } else {
                if (g_anFlightInput_004931a8[2] != 0 &&
                    g_cCurrentKey_00493128 == 0) {
                    g_anFlightInput_004931a8[2] = 0;
                    g_stPreviousFlightInput_005c57d0.x = -1;
                }
                if (g_stPreviousFlightInput_005c57d0.x !=
                        g_stCurrentFlightInput_0049d7b0.x ||
                    g_stPreviousFlightInput_005c57d0.y !=
                        g_stCurrentFlightInput_0049d7b0.y) {
                    g_anFlightInput_004931a8[1] =
                        (short)g_stCurrentFlightInput_0049d7b0.x;
                    g_anFlightInput_004931a8[0] =
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
#ifdef SDL_PORT
            trailingKeyRelease = 0;
#endif
            g_nCockpitControlGoal_0049d7d0 = 0;
            g_cCurrentKey_00493128 = (signed char)event.status;
            process_player_input();
            break;
#ifdef SDL_PORT
        case 5:
            trailingKeyRelease = 1;
            break;
#endif
        }
    }
#ifdef SDL_PORT
    /* Apply a final key release after draining earlier repeat events. */
    if (trailingKeyRelease != 0 &&
        g_bPolledFlightInputQueued_005c587a == 0 &&
        g_nCockpitControlState_0049d7ac == 0) {
        g_anFlightInput_004931a8[1] =
            g_anFlightInput_004931a8[0] =
            g_anFlightInput_004931a8[2] =
            g_bMouseMoveEventQueued_0049d7fc = 0;
    }
#endif

    if (g_bSecondaryMouseButtonHeld_0049d7f4 == 1)
        g_cCurrentKey_00493128 = 0xf;
    if ((g_cCurrentKey_00493128 & (signed char)0x80) != 0 &&
        (g_stCurrentFlightInput_0049d7b0.buttons & 1) != 0)
        g_cCurrentKey_00493128 = 0x39;
    if ((g_cCurrentKey_00493128 & (signed char)0x80) != 0 &&
        (g_wPendingInputButtons_005c80d4 & 1) != 0)
        g_cCurrentKey_00493128 = 0x39;
#ifdef SDL_PORT
    SdlApplyJoystickFlightControls();
#endif
    g_stPreviousFlightInput_005c57d0 =
        g_stCurrentFlightInput_0049d7b0;
    g_bPlayerInputActive_005c8090 = savedPlayerInputActive;
    return result;
}

#pragma function(strlen)

/* Function start: 0x4672C5 */
/* Message dwell time: grows with text length, scaled by the speed setting. */
short MeasureMessageWidth(const char *text)
{
    return (short)((MinShort(5, (short)(strlen(text) >> 1)) + 5) *
                   ((char)g_cMessageSpeed_0049b778 + 1));
}

/* Function start: 0x467300 */
void AcknowledgeModalMessage(void)
{
#ifdef SDL_PORT
    SdlSuspendMouseGrab();
#endif
    ConfigureInputPump(1, PollJoystickButtonEvents);
    SetFrameTimerAndWait(20);
    FlushPendingInputEvents();
    getch();
    ConfigureInputPump(1, get_player_input);
#ifdef SDL_PORT
    SdlResumeMouseGrab();
#endif
}

/* Function start: 0x46733D */
short HandleSpaceFlightControls(void)
{
    short inputHandled;
    short notRepeated;
    short control;
    short modifiers;
    short keyState;
    short object;

    g_bInputCursorEnabled_005c80e6 = 1;
    g_nInputRepeatDelay_005c80d6 = g_bInputCursorEnabled_005c80e6;
    if (g_nCurrentView_00492fa8 == 4) {
        inputHandled = HandleFleetOverviewInput();
    } else {
        inputHandled = player_input();
        players_flight_dynamics();
    }
    notRepeated =
        g_cPreviousKey_0049312c != g_cCurrentKey_00493128 &&
        inputHandled == 0;
    control = (short)GetControlKeyState();
    modifiers = (short)GetKeyboardModifiers();

    if (g_bAltBHotkey_005d1290 != 0 &&
        g_nLastAltCommandScanCode_005d1274 != 0x30 &&
        g_nYourWingman_0049346c != -1 &&
        g_aeShipObjective_00495f08[g_nYourWingman_0049346c] ==
            OBJECTIVE_HOLD_FORMATION &&
        any_enemy(0, 14000) != 0) {
        IssueQuickCommCommand(g_nYourWingman_0049346c, 4);
        g_nLastAltCommandScanCode_005d1274 = 0x30;
    }
    if (g_bJoystickCalibrationHotkey_005d1284 != 0 &&
        g_nLastAltCommandScanCode_005d1274 != 0x2e) {
        CalibrateJoystickInteractive();
        g_nLastAltCommandScanCode_005d1274 = 0x2e;
    }
    if (g_bAltFHotkey_005d127c != 0 &&
        g_nLastAltCommandScanCode_005d1274 != 0x21 &&
        g_nYourWingman_0049346c != -1) {
        IssueQuickCommCommand(g_nYourWingman_0049346c, 6);
        g_nLastAltCommandScanCode_005d1274 = 0x21;
    }
    if (g_bAltAHotkey_005d1294 != 0 &&
        g_nLastAltCommandScanCode_005d1274 != 0x1e &&
        g_nYourWingman_0049346c != -1 &&
        g_acShipTarget_00495f20[0] != -1) {
        IssueQuickCommCommand(g_nYourWingman_0049346c, 1);
        g_nLastAltCommandScanCode_005d1274 = 0x1e;
    }
    if (g_bAltHHotkey_005d128c != 0 &&
        g_nLastAltCommandScanCode_005d1274 != 0x23 &&
        g_nYourWingman_0049346c != -1 &&
        any_enemy(0, 14000) != 0) {
        IssueQuickCommCommand(g_nYourWingman_0049346c, 2);
        g_nLastAltCommandScanCode_005d1274 = 0x23;
    }
    if (g_bAltDHotkey_005d1280 != 0 &&
        g_nLastAltCommandScanCode_005d1274 != 0x20 &&
        g_nYourWingman_0049346c != -1) {
        IssueQuickCommCommand(g_nYourWingman_0049346c, 13);
        g_nLastAltCommandScanCode_005d1274 = 0x20;
    }
    if (g_bAltTHotkey_005d1298 != 0 &&
        g_nLastAltCommandScanCode_005d1274 != 0x14 &&
        g_acShipTarget_00495f20[0] != -1 &&
        g_asShipSide_004955d0[g_acShipTarget_00495f20[0]] ==
            SIDE_KILRATHI) {
        IssueQuickCommCommand(
            g_acShipTarget_00495f20[0],
            (short)((unsigned short)RandomInRange(0, 2) +
                    g_nEnemyTauntCommandBase_0049b76c));
        g_nLastAltCommandScanCode_005d1274 = 0x14;
    }
#ifdef SDL_PORT
    /* SDL latches these command presses until one flight-control pass. */
    g_bAltBHotkey_005d1290 = 0;
    g_bAltFHotkey_005d127c = 0;
    g_bAltAHotkey_005d1294 = 0;
    g_bAltHHotkey_005d128c = 0;
    g_bAltDHotkey_005d1280 = 0;
    g_bAltTHotkey_005d1298 = 0;
#endif
    if (g_bAltNumpadAddHotkey_005d1270 != 0 &&
        g_nLastAltCommandScanCode_005d1274 != 0x4e) {
        g_nLastAltCommandScanCode_005d1274 = 0x4e;
        AdjustSpaceFramePeriod(1);
    }
    if (g_bAltNumpadSubtractHotkey_005d12a8 != 0 &&
        g_nLastAltCommandScanCode_005d1274 != 0x4a) {
        g_nLastAltCommandScanCode_005d1274 = 0x4a;
        AdjustSpaceFramePeriod(-1);
    }

    switch ((signed char)g_cCurrentKey_00493128) {
        case 0x13:
            if (notRepeated && g_bHighMemoryBuffersReady_005d2ad8 != 0) {
                g_bSceneEscapeRequested_0049d4b0 = 0;
                BeginInflightReplayHook();
            }
            break;
        case 0x1e:
            if (notRepeated && control == 0) {
                g_nCockpitControlState_0049d7ac = 0;
                if (get_mode(1) != 5)
                    SelectCockpitVduMode(1, 5);
                if (g_nCockpitDisplayMode_0049d71c == 0) {
                    auto_pilot_sequence();
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_stViewBuffer_005d2b00, 0, 0,
                        (short)(g_nScreenWidth_0049d4d8 - 1),
                        (short)(g_nScreenHeight_0049d4dc - 1));
                    initialize_view_buffer();
                    g_nCockpitDisplayMode_0049d71c = -2;
                    auto_pilot_sequence();
                    g_nCockpitDisplayMode_0049d71c = 1;
                    free_view_buffer();
                    SetViewportRect(&g_stViewBuffer_005d2b00,
                                    0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
            break;
        case 0x2e:
            if (notRepeated) {
                g_bDisplayWingmanTargetData_0049347c = 0;
                if (message_showing() != 0) {
                    EndCommMenu();
                } else if (get_mode(1) != 4) {
                    SelectCockpitVduMode(1, 4);
                } else {
                    CloseCommChoiceMenu();
                }
            }
            break;
        case 0x12:
            if (notRepeated && control != 0 &&
                g_acPlayerComponentDamage_00493470[7] != 4) {
                if (RandomInRange(
                        0, g_acPlayerComponentDamage_00493470[7]) == 0)
                    g_nArcadeState_0049d75c = 2;
                else
                    malf_sound();
            }
            break;
        case 0x31:
            if (notRepeated) {
                SelectCockpitVduMode(1, 5);
                g_bDisplayWingmanTargetData_0049347c = 0;
            }
            break;
        case 0x32:
            if (notRepeated && control == 0)
                SetMessageDisplaySpeed();
            break;
        case 0x2f:
            if (notRepeated && control == 0) {
                g_bVideoImagesSuppressed_0049b784 =
                    g_bVideoImagesSuppressed_0049b784 == 0;
                if (g_bVideoImagesSuppressed_0049b784 != 0)
                    SetHudMessageText(
                        "VIDEO IMAGES SUPRESSED",
                        g_cRedColour_0049cb5c, 20);
                else
                    SetHudMessageText("VIDEO IMAGES ENABLED",
                                      g_ucPrimaryTextColour_0049cb64, 20);
            }
            break;
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            if (notRepeated && get_mode(1) == 4 &&
                g_nCurrentView_00492fa8 == 0 && (signed char)g_cCurrentKey_00493128 >= 2 &&
                (signed char)g_cCurrentKey_00493128 <=
                    g_nCommMenuChoiceCount_0049b770 + 2 &&
                get_mode(1) == 4) {
                Chosen_communicate_option(
                    (short)((signed char)g_cCurrentKey_00493128 - 2));
            }
            break;
        case 0x1f:
            if (control != 0 && notRepeated) {
                if (g_nSfxVolumeSetting_0049d74c == 0)
                    g_nSfxVolumeSetting_0049d74c = 20;
                else
                    g_nSfxVolumeSetting_0049d74c = 0;
                SetSoundEffectsVolume(
                    g_anVolumeLevels_0049d720[
                        g_nSfxVolumeSetting_0049d74c / 2]);
                ShowOnScreenMessage(0, "SFX VOLUME: %d.",
                                    g_nSfxVolumeSetting_0049d74c / 2);
            }
            break;
        case 0x3b:
            if (g_nTargetCameraFrame_0049d3e8 == 1)
                ClearTargetCameraView();
            g_nCockpitControlState_0049d7ac = 0;
            if (GetF1KeyLatch() != 0) {
                if (g_cScreenViewportMode_005c82a6 == 0) {
                    g_nCockpitDisplayMode_0049d71c =
                        g_nCockpitDisplayMode_0049d71c == 0;
                    free_view_buffer();
                    if (g_nCockpitDisplayMode_0049d71c != 0) {
                        g_bFullScreenSpaceView_0049d718 = 0;
                        SetViewportRect(&g_stViewBuffer_005d2b00,
                                        0, 0, 319, 199);
                        initialize_cockpit(
                            g_cScreenViewportMode_005c82a6++);
                    } else {
                        g_bFullScreenSpaceView_0049d718 = 1;
                        SetViewportRect(
                            &g_stViewBuffer_005d2b00, 0, 0,
                            (short)(g_nScreenWidth_0049d4d8 - 1),
                            (short)(g_nScreenHeight_0049d4dc - 1));
                        initialize_cockpit(
                            g_cScreenViewportMode_005c82a6++);
                    }
                } else {
                    g_nCockpitControlState_0049d7ac = 0;
                    if (g_nCockpitDisplayMode_0049d71c == 0) {
                        new_view(0, 0);
                    } else {
                        free_view_buffer();
                        SetViewportRect(
                            &g_stViewBuffer_005d2b00, 0, 0,
                            (short)(g_nScreenWidth_0049d4d8 - 1),
                            (short)(g_nScreenHeight_0049d4dc - 1));
                        initialize_view_buffer();
                        new_view(0, 0);
                        free_view_buffer();
                        SetViewportRect(&g_stViewBuffer_005d2b00, 0, 0, 319, 199);
                        initialize_view_buffer();
                    }
                }
                FlushInputEvents();
            }
            break;
        case 0x3c:
            if (g_nTargetCameraFrame_0049d3e8 == 1)
                ClearTargetCameraView();
            if (g_cScreenViewportMode_005c82a6 == 2) {
                g_nCockpitDisplayMode_0049d71c =
                    g_nCockpitDisplayMode_0049d71c == 0;
                free_view_buffer();
                if (g_nCockpitDisplayMode_0049d71c != 0) {
                    g_bFullScreenSpaceView_0049d718 = 0;
                    SetViewportRect(&g_stViewBuffer_005d2b00,
                                    0, 0, 319, 199);
                    initialize_cockpit(g_cScreenViewportMode_005c82a6++);
                } else {
                    g_bFullScreenSpaceView_0049d718 = 1;
                    SetViewportRect(
                        &g_stViewBuffer_005d2b00, 0, 0,
                        (short)(g_nScreenWidth_0049d4d8 - 1),
                        (short)(g_nScreenHeight_0049d4dc - 1));
                    initialize_cockpit(g_cScreenViewportMode_005c82a6++);
                }
            } else {
                g_nCockpitControlState_0049d7ac = 0;
                if (g_nCockpitDisplayMode_0049d71c == 0) {
                    new_view(2, 0);
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_stViewBuffer_005d2b00, 0, 0,
                        (short)(g_nScreenWidth_0049d4d8 - 1),
                        (short)(g_nScreenHeight_0049d4dc - 1));
                    initialize_view_buffer();
                    new_view(2, 0);
                    free_view_buffer();
                    SetViewportRect(&g_stViewBuffer_005d2b00,
                                    0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
            break;
        case 0x3d:
            if (g_nTargetCameraFrame_0049d3e8 == 1)
                ClearTargetCameraView();
            if (g_cScreenViewportMode_005c82a6 == 1) {
                g_nCockpitDisplayMode_0049d71c =
                    g_nCockpitDisplayMode_0049d71c == 0;
                free_view_buffer();
                if (g_nCockpitDisplayMode_0049d71c != 0) {
                    g_bFullScreenSpaceView_0049d718 = 0;
                    SetViewportRect(&g_stViewBuffer_005d2b00,
                                    0, 0, 319, 199);
                    initialize_cockpit(g_cScreenViewportMode_005c82a6++);
                } else {
                    g_bFullScreenSpaceView_0049d718 = 1;
                    SetViewportRect(
                        &g_stViewBuffer_005d2b00, 0, 0,
                        (short)(g_nScreenWidth_0049d4d8 - 1),
                        (short)(g_nScreenHeight_0049d4dc - 1));
                    initialize_cockpit(g_cScreenViewportMode_005c82a6++);
                }
            } else {
                g_nCockpitControlState_0049d7ac = 0;
                if (g_nCockpitDisplayMode_0049d71c == 0) {
                    new_view(1, 0);
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_stViewBuffer_005d2b00, 0, 0,
                        (short)(g_nScreenWidth_0049d4d8 - 1),
                        (short)(g_nScreenHeight_0049d4dc - 1));
                    initialize_view_buffer();
                    new_view(1, 0);
                    free_view_buffer();
                    SetViewportRect(&g_stViewBuffer_005d2b00,
                                    0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
            break;
        case 0x3e:
            if (g_nTargetCameraFrame_0049d3e8 == 1)
                ClearTargetCameraView();
            if (g_cScreenViewportMode_005c82a6 == 3) {
                g_nCockpitDisplayMode_0049d71c =
                    g_nCockpitDisplayMode_0049d71c == 0;
                free_view_buffer();
                if (g_nCockpitDisplayMode_0049d71c != 0) {
                    g_bFullScreenSpaceView_0049d718 = 0;
                    SetViewportRect(&g_stViewBuffer_005d2b00,
                                    0, 0, 319, 199);
                    initialize_cockpit(g_cScreenViewportMode_005c82a6++);
                } else {
                    g_bFullScreenSpaceView_0049d718 = 1;
                    SetViewportRect(
                        &g_stViewBuffer_005d2b00, 0, 0,
                        (short)(g_nScreenWidth_0049d4d8 - 1),
                        (short)(g_nScreenHeight_0049d4dc - 1));
                    initialize_cockpit(g_cScreenViewportMode_005c82a6++);
                }
            } else {
                g_nCockpitControlState_0049d7ac = 0;
                if (g_nCockpitDisplayMode_0049d71c == 0) {
                    new_view(3, 0);
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_stViewBuffer_005d2b00, 0, 0,
                        (short)(g_nScreenWidth_0049d4d8 - 1),
                        (short)(g_nScreenHeight_0049d4dc - 1));
                    initialize_view_buffer();
                    new_view(3, 0);
                    free_view_buffer();
                    SetViewportRect(&g_stViewBuffer_005d2b00,
                                    0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
            break;
        case 0x3f:
            if (g_nTargetCameraFrame_0049d3e8 == 1)
                ClearTargetCameraView();
            if (g_cScreenViewportMode_005c82a6 == 5) {
                g_nCockpitDisplayMode_0049d71c =
                    g_nCockpitDisplayMode_0049d71c == 0;
                free_view_buffer();
                if (g_nCockpitDisplayMode_0049d71c != 0) {
                    g_bFullScreenSpaceView_0049d718 = 0;
                    SetViewportRect(&g_stViewBuffer_005d2b00,
                                    0, 0, 319, 199);
                    initialize_cockpit(g_cScreenViewportMode_005c82a6++);
                } else {
                    g_bFullScreenSpaceView_0049d718 = 1;
                    SetViewportRect(
                        &g_stViewBuffer_005d2b00, 0, 0,
                        (short)(g_nScreenWidth_0049d4d8 - 1),
                        (short)(g_nScreenHeight_0049d4dc - 1));
                    initialize_cockpit(g_cScreenViewportMode_005c82a6++);
                }
            } else {
                g_nCockpitControlState_0049d7ac = 0;
                if (g_nCockpitDisplayMode_0049d71c == 0) {
                    new_view(5, 0);
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_stViewBuffer_005d2b00, 0, 0,
                        (short)(g_nScreenWidth_0049d4d8 - 1),
                        (short)(g_nScreenHeight_0049d4dc - 1));
                    initialize_view_buffer();
                    new_view(5, 0);
                    free_view_buffer();
                    SetViewportRect(&g_stViewBuffer_005d2b00,
                                    0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
            break;
        case 0x40:
            if (g_nTargetCameraFrame_0049d3e8 == 1)
                ClearTargetCameraView();
            if (g_cScreenViewportMode_005c82a6 == 15) {
                g_nCockpitDisplayMode_0049d71c =
                    g_nCockpitDisplayMode_0049d71c == 0;
                free_view_buffer();
                if (g_nCockpitDisplayMode_0049d71c != 0) {
                    g_bFullScreenSpaceView_0049d718 = 0;
                    SetViewportRect(&g_stViewBuffer_005d2b00,
                                    0, 0, 319, 199);
                    initialize_cockpit(g_cScreenViewportMode_005c82a6++);
                } else {
                    g_bFullScreenSpaceView_0049d718 = 1;
                    SetViewportRect(
                        &g_stViewBuffer_005d2b00, 0, 0,
                        (short)(g_nScreenWidth_0049d4d8 - 1),
                        (short)(g_nScreenHeight_0049d4dc - 1));
                    initialize_cockpit(g_cScreenViewportMode_005c82a6++);
                }
            } else {
                g_nCockpitControlState_0049d7ac = 0;
                if (g_nCockpitDisplayMode_0049d71c == 0) {
                    new_view(15, 0);
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_stViewBuffer_005d2b00, 0, 0,
                        (short)(g_nScreenWidth_0049d4d8 - 1),
                        (short)(g_nScreenHeight_0049d4dc - 1));
                    initialize_view_buffer();
                    new_view(15, 0);
                    free_view_buffer();
                    SetViewportRect(&g_stViewBuffer_005d2b00,
                                    0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
            break;
        case 0x41:
            if (g_nTargetCameraFrame_0049d3e8 == 1)
                ClearTargetCameraView();
            if (g_cScreenViewportMode_005c82a6 == 8) {
                g_nCockpitDisplayMode_0049d71c =
                    g_nCockpitDisplayMode_0049d71c == 0;
                free_view_buffer();
                if (g_nCockpitDisplayMode_0049d71c != 0) {
                    g_bFullScreenSpaceView_0049d718 = 0;
                    SetViewportRect(&g_stViewBuffer_005d2b00,
                                    0, 0, 319, 199);
                    initialize_cockpit(g_cScreenViewportMode_005c82a6++);
                } else {
                    g_bFullScreenSpaceView_0049d718 = 1;
                    SetViewportRect(
                        &g_stViewBuffer_005d2b00, 0, 0,
                        (short)(g_nScreenWidth_0049d4d8 - 1),
                        (short)(g_nScreenHeight_0049d4dc - 1));
                    initialize_cockpit(g_cScreenViewportMode_005c82a6++);
                }
            } else if (g_acShipTarget_00495f20[0] != -1) {
                g_nCockpitControlState_0049d7ac = 0;
                if (g_nCockpitDisplayMode_0049d71c == 0) {
                    new_view(8, 0);
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_stViewBuffer_005d2b00, 0, 0,
                        (short)(g_nScreenWidth_0049d4d8 - 1),
                        (short)(g_nScreenHeight_0049d4dc - 1));
                    initialize_view_buffer();
                    new_view(8, 0);
                    free_view_buffer();
                    SetViewportRect(&g_stViewBuffer_005d2b00,
                                    0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
            break;
        case 0x42:
            if (g_nTargetCameraFrame_0049d3e8 == 1)
                ClearTargetCameraView();
            if (notRepeated) {
                g_nCockpitControlState_0049d7ac = 0;
                g_bMissileCameraEnabled_00493504 ^= 1;
                if (g_bMissileCameraEnabled_00493504 != 0)
                    SetHudMessageText("MISSILE CAMERA ON",
                                      g_cRedColour_0049cb5c, 20);
                else
                    SetHudMessageText("MISSILE CAMERA OFF",
                                      g_ucPrimaryTextColour_0049cb64, 20);
            }
            break;
        case 0x43:
            if (notRepeated) {
                g_nCockpitControlState_0049d7ac = 0;
                SelectNextExternalViewObject();
                if (g_nCockpitDisplayMode_0049d71c == 0) {
                    new_view(5, 0);
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_stViewBuffer_005d2b00, 0, 0,
                        (short)(g_nScreenWidth_0049d4d8 - 1),
                        (short)(g_nScreenHeight_0049d4dc - 1));
                    initialize_view_buffer();
                    force_view(5, 0);
                    free_view_buffer();
                    SetViewportRect(&g_stViewBuffer_005d2b00,
                                    0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
            break;
    }

    switch ((signed char)g_cCurrentKey_00493128) {
    case 0x0d:
    case 0x4e:
        if (control != 0) {
            ReportFramesSkipped(1);
        } else {
            accelerate(1);
        }
        break;
    case 0x2b:
        accelerate(9000);
        break;
    case 0x0c:
    case 0x4a:
        if (control != 0) {
            ReportFramesSkipped(-1);
        } else {
            accelerate(-1);
        }
        break;
    case 0x0e:
        g_anShipSpeed_00494e20[0] = 0;
        break;
    case 0x1c:
        if (notRepeated && g_nSelectedReleaseWeaponIndex_004934e0 != -1 &&
            g_nExternalViewShip_00493468 == -1) {
            g_nExternalViewShip_00493468 = fire_missile(0);
            if (g_nExternalViewShip_00493468 != -1 &&
                g_asObjectType_00495298[g_nExternalViewShip_00493468] ==
                    0x14) {
                g_nExternalViewShip_00493468 = -1;
            } else if (g_bMissileCameraEnabled_00493504 == 1 &&
                       g_nExternalViewShip_00493468 != -1) {
                new_view(7, g_nExternalViewShip_00493468);
            }
        }
        break;
    case 0x39:
        if (g_nCurrentView_00492fa8 == 4)
            FireTargetCameraGuns();
        else
            fire_players_lasers();
        break;
    case 0x0f:
    case 0x37:
        if (g_asObjectType_00495298[0] != 0x33)
            your_afterburner();
        break;
    case 1:
        g_bSceneEscapeRequested_0049d4b0 = 0;
        if (get_mode(1) == 4) {
            CloseCommChoiceMenu();
#ifdef SDL_PORT
        } else {
            /* Retail leaves Escape inert here.  The port makes it another
             * pause key while preserving its communication-menu action. */
            goto pause_spaceflight;
#endif
        }
        break;
    case 0x14:
        if (notRepeated && control == 0) {
            SelectCockpitVduMode(1, 3);
            g_bDisplayWingmanTargetData_0049347c = 0;
        }
        break;
    case 0x11:
        if (notRepeated) {
            if (g_nCurrentView_00492fa8 == 4) {
                ToggleTargetCameraOverlay();
            } else if (IsCockpitWeaponShapeLoaded() != 0) {
                SelectCockpitVduMode(0, 1);
            }
        }
        break;
    case 0x2f:
        if (notRepeated && control != 0)
            ShowVersionBanner();
        break;
    case 0x24:
        if (notRepeated) {
            if (control != 0)
                CalibrateJoystickInteractive();
            else
                TryBeginStarSystemJump();
        }
        break;
    case 0x19:
#ifdef SDL_PORT
pause_spaceflight:
#endif
        g_bPauseInputActive_0049ac9c = 1;
        ShowGamePausedBanner((short)(control < 1));
        g_bPauseInputActive_0049ac9c = 0;
        SetFrameTimerPeriodDirect(1);
        for (keyState = 0; keyState < 0x100; keyState++) {
            g_abPauseInputState_005c85d0[keyState] = 0;
            g_abPreviousPauseInputState_005c86e0[keyState] =
                g_abPauseInputState_005c85d0[keyState];
        }
        ServiceInputDevices(-1);
        g_nCockpitControlGoal_0049d7d0 = 0;
        g_nCockpitControlState_0049d7ac =
            g_nCockpitControlGoal_0049d7d0;
        break;
    case 0x20:
        if (notRepeated && control == 0 &&
            IsCockpitWeaponShapeLoaded() != 0)
            SelectCockpitVduMode(0, 2);
        break;
    case 0x22:
        if (notRepeated) {
            if (g_nCurrentView_00492fa8 == 4) {
                ToggleTargetCameraOverlay();
            } else if (IsCockpitWeaponShapeLoaded() != 0) {
                SelectCockpitVduMode(0, 1);
            }
        }
        break;
    case 0x26:
        if (notRepeated) {
            g_bTargetLockMode_00493500 =
                (short)(g_bTargetLockMode_00493500 == 0);
            PlaySfxWaveFileByNumber(0x19, -1, 0);
            if (get_mode(1) == 3)
                InvalidateVduMode(1);
        }
        break;
    case 0x32:
        if (notRepeated && control != 0) {
            if (g_nMusicVolumeSetting_0049d750 == 0)
                g_nMusicVolumeSetting_0049d750 = 20;
            else
                g_nMusicVolumeSetting_0049d750 = 0;
            SetMusicStreamVolume(
                (unsigned short)g_anVolumeLevels_0049d720[
                    g_nMusicVolumeSetting_0049d750 / 2]);
            ShowOnScreenMessage(0, "MUSIC VOLUME: %d.",
                                g_nMusicVolumeSetting_0049d750 / 2);
        }
        break;
    }

    switch ((signed char)g_cCurrentKey_00493128) {
    case 0x48:
        if (control != 0) {
            g_nSfxVolumeSetting_0049d74c++;
            if (g_nSfxVolumeSetting_0049d74c > 20)
                g_nSfxVolumeSetting_0049d74c = 20;
            SaveVolumeSettingsToRegistry();
            SetSoundEffectsVolume(
                g_anVolumeLevels_0049d720[
                    g_nSfxVolumeSetting_0049d74c / 2]);
            ShowOnScreenMessage(0, "SFX VOLUME: %d.",
                                g_nSfxVolumeSetting_0049d74c / 2);
            return 0;
        }
        break;
    case 0x50:
        if (control != 0) {
            g_nSfxVolumeSetting_0049d74c--;
            if (g_nSfxVolumeSetting_0049d74c < 0)
                g_nSfxVolumeSetting_0049d74c = 0;
            SetSoundEffectsVolume(
                g_anVolumeLevels_0049d720[
                    g_nSfxVolumeSetting_0049d74c / 2]);
            SaveVolumeSettingsToRegistry();
            ShowOnScreenMessage(0, "SFX VOLUME: %d.",
                                g_nSfxVolumeSetting_0049d74c / 2);
            return 0;
        }
        break;
    case 0x4d:
        if (control != 0) {
            g_nMusicVolumeSetting_0049d750++;
            if (g_nMusicVolumeSetting_0049d750 > 20)
                g_nMusicVolumeSetting_0049d750 = 20;
            SaveVolumeSettingsToRegistry();
            SetMusicStreamVolume(
                (unsigned short)g_anVolumeLevels_0049d720[
                    g_nMusicVolumeSetting_0049d750 / 2]);
            ShowOnScreenMessage(0, "MUSIC VOLUME: %d.",
                                g_nMusicVolumeSetting_0049d750 / 2);
            return 0;
        }
        break;
    case 0x4b:
        if (control != 0) {
            g_nMusicVolumeSetting_0049d750--;
            if (g_nMusicVolumeSetting_0049d750 < 0)
                g_nMusicVolumeSetting_0049d750 = 0;
            SaveVolumeSettingsToRegistry();
            SetMusicStreamVolume(
                (unsigned short)g_anVolumeLevels_0049d720[
                    g_nMusicVolumeSetting_0049d750 / 2]);
            ShowOnScreenMessage(0, "MUSIC VOLUME: %d.",
                                g_nMusicVolumeSetting_0049d750 / 2);
            return 0;
        }
        break;
    }

    if (g_nOriginDevUnlock_0049d774 != 0) {
        switch ((signed char)g_cCurrentKey_00493128) {
        case 0x52:
            if (notRepeated && control != 0) {
                for (object = 1; object < 10; object++) {
                    if (g_asShipSide_004955d0[object] !=
                            g_asShipSide_004955d0[0] &&
                        g_aeObjectClass_00495328[object] ==
                            OBJECT_CLASS_SHIP) {
                        while (explode(0, object) == 0) {
                        }
                    }
                }
            }
            break;
        case 0x53:
            if (notRepeated && control != 0 &&
                g_bDebugBreakEnabled_0049c238 != 0) {
                object = g_acShipTarget_00495f20[0];
                if (object != -1) {
                    while (explode(0, object) == 0) {
                    }
                }
            }
            break;
        }
    }
    return 0;
}

/* Function start: 0x468E7A */
unsigned int DrawSpaceSceneFrame(void)
{
    short paletteCycleActive;
    short hidePlayer;

    paletteCycleActive = 0;
    hidePlayer = 0;
    if (g_nSpaceExplosionFlashActive_00492fb4 != 0) {
        if ((short)g_nSpacePaletteFadeMode_004901e8 == 0x13) {
            g_nSpaceExplosionFlashStep_005c5872++;
            if (g_nSpaceExplosionFlashStep_005c5872 < 4)
                paletteCycleActive++;
            if (g_aSpaceExplosionFlashPalette_0049d808[
                    g_nSpaceExplosionFlashStep_005c5872][0] != 0) {
                g_asSpacePaletteFade_005d2d60[0] =
                    g_aSpaceExplosionFlashPalette_0049d808[
                        g_nSpaceExplosionFlashStep_005c5872][0];
                g_asSpacePaletteFade_005d2d60[1] =
                    g_aSpaceExplosionFlashPalette_0049d808[
                        g_nSpaceExplosionFlashStep_005c5872][1];
                g_asSpacePaletteFade_005d2d60[2] =
                    g_aSpaceExplosionFlashPalette_0049d808[
                        g_nSpaceExplosionFlashStep_005c5872][2];
            } else {
                g_asSpacePaletteFade_005d2d60[0] = 0;
                g_asSpacePaletteFade_005d2d60[1] = 0;
                g_asSpacePaletteFade_005d2d60[2] = 0x20;
                g_nSpaceExplosionFlashActive_00492fb4 = 0;
            }
            SetPaletteEntry((short)g_cPrimaryViewBufferColour_0049cb88,
                            g_asSpacePaletteFade_005d2d60);
        }
    } else {
        UpdateSpacePaletteFade();
    }
    g_nFrameSkipCountdown_0049d760--;
    if (g_nFrameSkipCountdown_0049d760 > 0)
        return 0;
    g_nFrameSkipCountdown_0049d760 = g_nFrameSkip_0049d764;
    g_nRenderedSpaceFrame_00493138++;
    transform_objects_to_your_view();
    update_star_field();
    place_exhaust_on_ships();
    reposition_fixed_child_objects();
    BuildObjectDepthOrder();
#ifdef SDL_PORT
    SdlBeginSpaceFrame(
        (const struct ScreenViewportGeometry *)
            g_pScreenViewportGeometry_005c82b0,
        (int)g_cScreenViewportMode_005c82a6,
        /* Match dump_buffer_to_screen's active copy mode.  The stored
           preference has the opposite sense and omits cockpit offsets. */
        g_nCockpitDisplayMode_0049d71c > 0,
        (unsigned char)g_cPrimaryViewBufferColour_0049cb88);
#endif
    if (g_bJumpSequenceActive_004962f0 != 0 &&
        g_asShipManeuver_00495f48[0] == MANEUVER_WARPING_OUT) {
        hidePlayer = 1;
        g_aeObjectClass_00495328[0] = OBJECT_CLASS_NULL;
    }
    if (paletteCycleActive == 0)
        draw_sorted_objects_to_buffer();
    if (hidePlayer != 0)
        g_aeObjectClass_00495328[0] = OBJECT_CLASS_SHIP;
    if (g_nCurrentView_00492fa8 == 0 || g_nCurrentView_00492fa8 == 4)
        overlay_head_up_display();
    return 1;
}

/* Function start: 0x46903F */
void RenderSpaceViewFrame(void)
{
    int displayMode;

    if (DrawSpaceSceneFrame() == 0)
        return;
    check_message();
    RestoreCockpitExplosionIfVisible();
    if (g_pszPendingHudMessage_0049afec !=
            g_pszDisplayedHudMessage_0049aff0 &&
        g_pszDisplayedHudMessage_0049aff0 != 0) {
        ClearHudMessageDisplay(0);
    }
    if (g_nCockpitDisplayMode_0049d71c > 0) {
        displayMode = g_nCockpitDisplayMode_0049d71c;
        g_nCockpitDisplayMode_0049d71c = 9;
    }
    dump_buffer_to_screen();
    if (g_nCockpitDisplayMode_0049d71c == 9)
        g_nCockpitDisplayMode_0049d71c = displayMode;
    if (g_nCurrentView_00492fa8 == 0 || g_nCurrentView_00492fa8 == 4)
        RestoreTransientCockpitGraphics();
    ClearViewport(&g_stViewBuffer_005d2b00,
                  g_cPrimaryViewBufferColour_0049cb88);
}

/* Function start: 0x4690FF */
short Draw_3Space_Frame(void)
{
    Update_3Space();
    if (g_nArcadeState_0049d75c != 0)
        return 0;
    if (g_nFrameSkipCountdown_0049d760 <= 1)
        clear_view_buffer();
    return (unsigned short)DrawSpaceSceneFrame();
}

/* Function start: 0x469143 */
short GetShipDistanceToNavPoint(short ship, MissionNavPoint *navPoint)
{
    FixedVector delta;

    ComputeVectorDelta(&g_aShipPosition_00494550[ship],
                       &navPoint->position, &delta);
    return FixedToShortSaturating(Vector_magnitude(&delta));
}

/* Function start: 0x46918D */
short FindNearestNavPoint(short ship)
{
    MissionNavPoint *entry = g_aMissionNavPoints_00491e98;
    short navPointIndex = 0;

    for (; navPointIndex < 10; navPointIndex++, entry++) {
        if (entry->type == 1 &&
            entry->systemIndex == g_nCurrentStarSystem_005d169c &&
            GetShipDistanceToNavPoint(ship, entry) <
                entry->proximityRadius)
            return navPointIndex;
    }
    return g_nCurrentNavPoint_004931bc;
}

/* Function start: 0x469223 */
unsigned int ReleaseStaleNavTarget(void)
{
    short v = FindNearestNavPoint(0);

    if (g_nCurrentNavPoint_004931bc != v)
        set_up_action_sphere(v);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x46925E */
int RunSpaceFlight(short entryNavPoint)
{
    InputPumpContext *savedInputContext;
    unsigned char savedInputDelay;
    short savedUiInputMode;
    short savedCursor;
    short result;
    short i;

    savedUiInputMode = g_nUiInputMode_005c8d3c;
    g_bSpaceFlightActive_005c586c = 1;
    g_nCockpitDisplayMode_0049d71c = 0;
    if (g_bFullScreenSpaceView_0049d718 == 0)
        g_nCockpitDisplayMode_0049d71c = 1;
    for (i = 0; i < 7; i++)
        g_acMissionShipStatusFlags_005d2fb0[i] = 0;
    g_bMissionDeathSequencePending_0049b720 = 0;
    g_bMissionWingmanFlag_005c8dbe = 0;
    g_nFrameSkipCountdown_0049d760 = 1;
    InitializeSpaceFlightInput();
    g_nSpaceExplosionFlashActive_00492fb4 = 0;
    savedInputDelay = g_nInputRepeatDelay_005c80d6;
    savedInputContext = g_pActiveInputContext_005c8487;
    ReleaseSceneMusicPacket();
    init_inflight_music();

    if (entryNavPoint == -1)
        entryNavPoint = g_aMissionShips_00492290[
            g_stMissionHeader_005d3e70.playerMissionShip].navPoint;
    set_up_action_sphere(entryNavPoint);
    force_view(0, 0);
    SetSpaceFlightFrameTiming();

    if (g_nCockpitDisplayMode_0049d71c != 0) {
        free_view_buffer();
        SetViewportRect(&g_stViewBuffer_005d2b00, 0, 0,
                        (unsigned short)(g_nScreenWidth_0049d4d8 - 1),
                        (unsigned short)(g_nScreenHeight_0049d4dc - 1));
        initialize_view_buffer();
        new_view(0, 0);
        free_view_buffer();
        SetViewportRect(&g_stViewBuffer_005d2b00, 0, 0, 319, 199);
        g_cScreenViewportMode_005c82a6++;
        initialize_cockpit(g_cScreenViewportMode_005c82a6 - 1);
        g_nCockpitControlState_0049d7ac =
            g_nCockpitControlGoal_0049d7d0 = 0;
        initialize_view_buffer();
        FlushInputEvents();
    }

    copy_frame(0, 0x44);
    SetPersonnelMousePosition(
        (short)((g_stViewBuffer_005d2b00.left +
                 g_stViewBuffer_005d2b00.right) / 2),
        (short)((g_stViewBuffer_005d2b00.top +
                 g_stViewBuffer_005d2b00.bottom) / 2));
    g_nCockpitControlState_0049d7ac = 0;
    g_nArcadeState_0049d75c = 0;
#ifdef SDL_PORT
    SdlSetMouseGrab(1);
#endif

    while (g_nArcadeState_0049d75c == 0) {
        SetFrameTimerPeriodDirect(g_nSpaceFramePeriod_0049d768);
        if (g_nCannedSceneMode_0049021c != 0) {
            ProcessCannedSceneInput();
            result = 0;
            if (g_nCannedSceneRecordedFrameCount_005d3faa ==
                g_nCannedSceneFrameCount_005d404c) {
                FinishCannedScenePlayback();
                ResetCannedSceneRecording();
            }
        }
        if (g_nCannedSceneMode_0049021c == 0 &&
            g_bJumpSequenceActive_004962f0 == 0)
            result = (short)HandleSpaceFlightControls();
        if (result == -1) {
            g_nArcadeState_0049d75c = 5;
        } else {
            Update_3Space();
            if (g_nArcadeState_0049d75c != 0)
                break;
            g_bRenderingSpaceFrame_0049d858 = 1;
            RenderSpaceViewFrame();
            g_bRenderingSpaceFrame_0049d858 = 0;
            update_cockpit();
            if (g_pszPendingHudMessage_0049afec != 0)
                SetHudTextColour(g_pszPendingHudMessage_0049afec,
                                  g_nHudMessageColour_005d1ef0);
        }
        MarkDibDirty();
        DIBslamReal();
        WaitForFrameTick();
    }

#ifdef SDL_PORT
    SdlSetMouseGrab(0);
    SdlCancelSpaceFrame();
#endif
    SetViewportRect(&g_stViewBuffer_005d2b00, 0, 0,
                    (unsigned short)(g_nScreenWidth_0049d4d8 - 1),
                    (unsigned short)(g_nScreenHeight_0049d4dc - 1));
    g_nCockpitDisplayMode_0049d71c = 0;
    if (g_nArcadeState_0049d75c == 1)
        flag_objective(find_objective(1, -1), 2);
    ResetCockpitPaletteEntries();
    SetDefaultFrameTiming();
    if (savedInputContext != 0)
        SetInputViewport(savedInputContext->viewport);
    free_inflight_music();
    ConfigureInputPump(1, 0);
    g_nCockpitControlState_0049d7ac = 0;
    g_nUiInputMode_005c8d3c = savedUiInputMode;
    g_nInputRepeatDelay_005c80d6 = savedInputDelay;
    g_bSpaceFlightActive_005c586c = 0;
    return g_nArcadeState_0049d75c;
}

/* Function start: 0x4695FD */
short calculate_damage_level(void)
{
    ObjectTypeData *typeData;
    int damage;
    int armorDamage;

    armorDamage = 0;
    typeData = &g_aObjectTypeData_00496d30[g_acObjectType_00493980[0]];
    damage = g_asObjectDamage_00495178[0] * 5;
    damage += (g_acShipDamage_00495690[0] * 30) /
              typeData->damageCapacity;
    if (typeData->armorFront < g_aasShipArmor_00495540[0][0])
        armorDamage += (typeData->armorFront * 100) /
                       g_aasShipArmor_00495540[0][0];
    if (typeData->armorRear < g_aasShipArmor_00495540[0][1])
        armorDamage += (typeData->armorRear * 100) /
                       g_aasShipArmor_00495540[0][1];
    if (typeData->armorLeft < g_aasShipArmor_00495540[0][2])
        armorDamage += (typeData->armorLeft * 100) /
                       g_aasShipArmor_00495540[0][2];
    if (typeData->armorRight < g_aasShipArmor_00495540[0][3])
        armorDamage += (typeData->armorRight * 100) /
                       g_aasShipArmor_00495540[0][3];
    armorDamage /= 4;
    damage += armorDamage;

    if (damage < 5)
        return 0;
    else if (damage < 40)
        return 1;
    else if (damage < 70)
        return 2;
    else
        return 3;
}

/* This unit's initialized data, after its functions so that it does not
 * move the functions' symbol keys. */
int g_bFullScreenSpaceView_0049d718 = 1;
int g_nCockpitDisplayMode_0049d71c = 0;
int g_anVolumeLevels_0049d720[11] = {
    0, 500, 1000, 2500, 5000, 11000,
    22500, 34000, 44000, 54000, 64000
};
int g_nSfxVolumeSetting_0049d74c = 0x14;
int g_nMusicVolumeSetting_0049d750 = 0x14;
int g_nUnusedMainWord_0049d754 = 0x1370;
int g_nTrainSimActive_0049d758 = 0;
int g_nArcadeState_0049d75c = 0;
short g_nFrameSkipCountdown_0049d760 = 1;
short g_nFrameSkip_0049d764 = 0;
short g_nSpaceFramePeriod_0049d768 = 4;
short g_bViewportDirty_0049d76c = 0;
short g_nCockpitCameraObject_0049d770 = 0;
short g_nOriginDevUnlock_0049d774 = 0;
short g_bCarrierLandingEnabled_0049d778 = 1;
short g_bPlayerDamageEnabled_0049d77c = 1;
short g_bPlayerCollisionEnabled_0049d780 = 1;
short g_nShowMemoryStatus_0049d784 = 0;
void *g_pMemoryAdjustment_0049d788 = 0;
signed char g_bParentDirectorySelected_0049d78c = 0;
short g_nMissionEntryNavOverride_0049d790 = -1;
int g_nUnusedMainWord_0049d794 = 0;
struct DirectMissionSelection g_stDirectMissionSelection_0049d798 = { 0, 0 };
short g_nDirectSeries_0049d79c = 1;
int g_nUnusedMainWord_0049d7a0 = 0;
short g_bSpaceViewBufferEnabled_0049d7a4 = 0;
int g_nUnusedMainWord_0049d7a8 = 0;
short g_nCockpitControlState_0049d7ac = 0;
InputDeviceSample g_stCurrentFlightInput_0049d7b0 = { 0 };
int g_nUnusedMainWord_0049d7bc = 0;
short g_wFlightInputButtonState_0049d7c0 = 0;
int g_nUnusedMainWord_0049d7c4 = 0;
int g_bFlightButtonTwoHeld_0049d7c8 = 0;
int g_bFlightButtonOneHeld_0049d7cc = 0;
short g_nCockpitControlGoal_0049d7d0 = 0;
short g_asMouseYawThresholds_0049d7d8[6] = {
    10, 37, 52, 57, 62, 1070
};
short g_asMousePitchThresholds_0049d7e8[6] = {
    5, 18, 27, 35, 38, 1040
};
signed char g_bSecondaryMouseButtonHeld_0049d7f4 = 0;
int g_nUnusedMainWord_0049d7f8 = 0;
signed char g_bMouseMoveEventQueued_0049d7fc = 0;
short g_nMouseYawInput_0049d800 = 0;
short g_nMousePitchInput_0049d804 = 0;
short g_aSpaceExplosionFlashPalette_0049d808[13][3] = {
    {63, 63, 35},
    {63, 63, 35},
    {63, 63, 63},
    {63, 63, 63},
    {63, 63, 63},
    {58, 58, 58},
    {50, 50, 50},
    {42, 42, 42},
    {33, 33, 33},
    {25, 25, 25},
    {17, 17, 17},
    {8, 8, 8},
    {0, 0, 0}
};
int g_bRenderingSpaceFrame_0049d858 = 0;
