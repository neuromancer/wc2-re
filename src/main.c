/*
 *  Main application loop and spaceflight frame.
 *
 *  Retail object 0x465780-0x46979F (link order; see docs/ORDER.md).
 */
/* This unit owns storage declared in globals.h as statics. */
#define MAIN_C
#include "game.h"

extern unsigned char g_128_abPauseInputState[0x100]; /* 0x005c85d0 */

extern unsigned char g_13_abPreviousPauseKeyStatuses[0x100]; /* 0x005c86e0 */

#pragma function(strcpy)
/* Function start: 0x465780 */
void SetPersonnelCursorPosition(int x, int y, short unused1,
                                short unused2)
{
    g_37_stPersonnelMousePointerState.x = x;
    g_37_stPersonnelMousePointerState.y = y;
}

/* Function start: 0x46579D */
short LogMemoryUsage(void)
{
    /* WC2's system.c declares the canned-scene tape file as a pointer where
     * it is really the string itself, so the unlink is handed the first four
     * bytes of "tape.tmp" and always fails.  The port cannot dereference a
     * 32-bit value as a pointer, so it removes the file as intended. */
#ifdef SDL_PORT
    _unlink(g_szCannedSceneTapeFile);
#else
    _unlink(*(char *const *)g_szCannedSceneTapeFile);
#endif
    StopMusicIfDriverActive();
    EMShutDown();
    if (g_bSpeechEnabledFlag != 0)
        ShutdownSoundBlaster();
    if (g_bParentDirectorySelected == 0) {
        _chdir("..");
        g_bParentDirectorySelected++;
    }
    ConfigureDefaultSpacePalette(g_14_nCurrentGraphicsModeDetectedValue);
    if (g_nOriginDevUnlock != 0) {
        printf("Series %d, Mission %d (%c)\n",
               (int)g_nCurSeries,
               (int)g_nCurrentLevel,
               (int)g_nCurrentLevel + 'A');
    }
    if (g_nShowMemoryStatus != 0) {
        printf(
            "initial far: %lu.  current far: %lu.  current near: %u.\n",
            g_dwInitialSpareMemory,
            GetLargestFreeMemoryBlockByType(0), 0);
    }
    FinalizeInputManagerHook();
    MessagePumpHook(8);
    if (g_pMemoryAdjustment != 0)
        free(g_pMemoryAdjustment);
    return 0;
}

/* Function start: 0x46589D */
void AdjustSpaceFramePeriod(short adjustment)
{
    g_nSpaceFramePeriod = MinShort(
        MaxShort((short)(g_nSpaceFramePeriod + adjustment), 1),
        0x13);
    ShowOnScreenMessage(0, "%d/60th SEC FRAME DURATION.",
                        g_nSpaceFramePeriod + 1);
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
    g_19_bStoredHighMemoryEnabled = 1;
    g_stDefaultPilotProfile.field_5e = 5;
    memcpy(&g_stCurrentPilotProfile,
           &g_stDefaultPilotProfile,
           sizeof(g_stCurrentPilotProfile));
#ifdef SDL_PORT
    GameMain(argumentCount, g_pInitialArguments);
#else
    main(argumentCount, g_pInitialArguments);
#endif

    if (g_bMemoryAdjustmentEnabled != 0) {
        memoryBytes = (unsigned short)g_5_nStorageAdjustmentKbValue << 10;
        g_pMemoryAdjustment =
            AllocateTaggedMemory((unsigned int)memoryBytes, 0x40);
        if (g_pMemoryAdjustment != 0)
#ifdef SDL_PORT
            printf("Adjusting Mem size by %dK\n",
                   g_5_nStorageAdjustmentKbValue);
#else
            /* This unit passed the short as a full dword (MOV EAX,[g]),
             * so the next global's bytes ride along in the high half. */
            printf("Adjusting Mem size by %dK\n",
                   *(int *)&g_5_nStorageAdjustmentKbValue);
#endif
    }

    g_stCurrentPilotProfile.series = g_nDirectSeries;
    g_stDefaultPilotProfile.series =
        g_stCurrentPilotProfile.series;
    g_stCurrentPilotProfile.mission = g_stDirectMissionSelection.mission;
    g_stDefaultPilotProfile.mission =
        g_stCurrentPilotProfile.mission;
    g_1_nResourcePaletteState = 0;
    LoadOriginFxDrivers();
    SetMenuInputPump();
    if (g_bRewritePacketExtensions == 0) {
        g_10_nMenuDarkColour = 2;
        g_nMenuSelectedCol = 10;
        g_3_nOptionMessageColour = 0x5e;
    } else {
        g_10_nMenuDarkColour = 0;
        g_nMenuSelectedCol = 10;
        g_3_nOptionMessageColour = 2;
    }

    if (g_stDirectMissionSelection.launch == 0) {
#ifdef SDL_PORT
        if (SdlOriginalTitleMusicReady() &&
            SdlOriginalTitleSequenceAvailable())
            Title_Sequence();
#endif
        g_bSceneEscapeRequested = 0;
        for (;;) {
            if (g_nOriginDevUnlock != 0 &&
                g_bDeveloperCampaignReady != 0 &&
                g_bDirectCampaignSelection != 0) {
                g_nSelectedCampaignSlot = 0;
                while (RunSelectedCampaign() != 0) {
                }
            } else {
                while (selectionMade == 0) {
                    switch (RunTitleScreen()) {
                    case 0:
                        g_nSelectedCampaignSlot =
                            BarracksScreen();
                        InitializeNewPilotCampaign(
                            g_nSelectedCampaignSlot);
#ifdef SDL_PORT
                        if (SdlTryImportSo1Pilot())
                            selectionMade = 1;
                        else
#endif
                        selectionMade = RunPilotDatabaseMenu();
                        break;
                    case 1:
                        g_nSelectedCampaignSlot =
                            LoadSelectedPilotCampaign();
                        selectionMade = 1;
                        break;
                    }
                }
                selectionMade = 0;
                if (g_nOriginDevUnlock != 0) {
                    strcpy(g_stCurrentPilotProfile.callsign,
                           "CHEATER");
                    strcpy(g_stPilotNameVariables.callsign,
                           "CHEATER");
                }
                while (RunSelectedCampaign() != 0) {
                }
            }
        }
    } else {
        g_nCurSeries = g_nDirectSeries;
        g_nCurrentLevel = g_stDirectMissionSelection.mission;
        LoadTemporaryCampaignGlobals();
        g_pCampaignGlobals->field_08 = 0;
        InitializeCampaignConstellationState(
            g_pCampaignGlobals, 1);
        SaveAndFreeTemporaryCampaignGlobals();
        init_mission(g_nDirectSeries,
                     g_stDirectMissionSelection.mission);
        RunSpaceFlight(g_nMissionEntryNavOverride);
        LogMemoryUsage();
        printf("You ");
        switch (g_nArcadeState) {
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
    if (g_pMemoryAdjustment != 0) {
        free(g_pMemoryAdjustment);
    }
}

#pragma intrinsic(strcpy)
/* Function start: 0x465CBC */
void free_view_buffer(void)
{
    if (message_showing())
        EndCommMenu();
    if (g_27_stViewScratchInfo.pixels != 0)
        free_viewport(&g_27_stViewScratchInfo);
}

/* Function start: 0x465CF6 */
void initialize_view_buffer(void)
{
    short allocated;

    if (g_bSpaceViewBufferEnabled != 0 &&
        g_27_stViewScratchInfo.pixels == 0) {
        allocated = (short)AllocateViewport(
            &g_27_stViewScratchInfo,
            (short)g_cPrimaryViewBufferColour, 0x20);
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
    if (g_nCockpitDisplayMode > 0) {
        CopyViewportContents(
            &g_27_stViewScratchInfo, &g_101_stScrViewInfo);
    } else {
        switch (g_1_cStoredViewDisplayModeValue) {
        case 4:
            g_101_stScrViewInfo.top = 24;
            CopyViewportContents(
                &g_27_stViewScratchInfo, &g_101_stScrViewInfo);
            g_101_stScrViewInfo.top = 0;
            break;
        case 5:
            CopyViewportContents(
                &g_27_stViewScratchInfo, &g_101_stScrViewInfo);
            break;
        default:
            CopyViewportRuns(
                &g_27_stViewScratchInfo, &g_101_stScrViewInfo,
                (const short *)g_15_pScreenViewportGeometry);
            break;
        }
    }
    if (g_nShowMemoryStatus != 0)
        ShowMemoryStatusDebug();
}

/* Function start: 0x465E25 */
void SelectNextExternalViewObject(void)
{
    signed char object;

    object = g_cViewObject;
    g_cViewObject = -1;
    while (g_cViewObject == -1) {
        object++;
        if (object > 9)
            object = 0;
        if (g_aeObjectClass[object] >= OBJECT_CLASS_SHIP)
            g_cViewObject = object;
    }
}

/* Function start: 0x465E88 */
unsigned int clear_view_buffer(void)
{
    ClearViewport(&g_27_stViewScratchInfo, (unsigned char)g_cPrimaryViewBufferColour);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x465EA9 */
void RefreshMemoryStatusOverlay(void)
{
    WaitForVerticalBlankThunk();
    CopyViewportContents(&g_stSecondaryViewBufferBlock, &g_101_stScrViewInfo);
    MarkDibDirty();
    DIBslamReal();
    if (g_nShowMemoryStatus != 0)
        ShowMemoryStatusDebug();
}

/* Function start: 0x465EE9 */
void Update_3Space(void)
{
    house_keep();
    CheckAllGuardedAllocations();
    house_keep_objects();
    if (g_nArcadeState != 0)
        return;
    update_objects_in_space();
    set_eye_direction_and_position();
    if (g_nCannedSceneMode == 0)
        servicetrack();
    g_nSpaceFrame++;
}

/* Function start: 0x465F3A */
void TriggerPlayerHitPaletteFlash(void)
{
    if (g_nCurrentView <= 4)
        g_1_asSpacePaletteFade[0] = 0x30;
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
    if (g_1_asSpacePaletteFade[0] != 0) {
        switch ((int)(short)g_nSpacePaletteFadeMode) {
        case 0x13:
            g_1_asSpacePaletteFade[0] =
                (short)(g_1_asSpacePaletteFade[0] - 0x10);
            SetPaletteEntry((short)g_cPrimaryViewBufferColour,
                            g_1_asSpacePaletteFade);
            break;
        case 9:
        case 13:
            ClearViewport(&g_27_stViewScratchInfo,
                          g_cRedColour);
            g_bViewportDirty = 1;
            g_1_asSpacePaletteFade[0] = 0;
            break;
        }
    }
}

/* Function start: 0x46604F */
void house_keep(void)
{
    unsigned char palette;
    short sequenceState;

    if (g_nTrainSimActive == 0 &&
        g_nCannedSceneMode == 0) {
        if (g_ucPendingEjectionTransition != 0xff &&
            g_nPendingEjectionSequenceCount != 0) {
            sequenceState = g_nEjectionSequenceState;
            g_nEjectionSequenceState++;
            if (sequenceState == 0x28) {
                g_nEjectionSequenceState = 0;
                g_nPendingEjectionSequenceCount = 0;
                ejection_sequence(g_ucPendingEjectionTransition, 1);
                g_ucPendingEjectionTransition = 0xff;
            }
        }
        if ((g_nSpaceFrame & 0x1f) == 0)
            ReleaseStaleNavTarget();
        if (g_nHazardFieldCount != 0 &&
            (g_nSpaceFrame & 0xf) == 0)
            check_hazards();
    }
    if (g_nCurrentView == 0 || g_nCurrentView == 4) {
        for (palette = 0; palette < 6; palette++) {
            FadeFlightPaletteEntry(
                g_9_aasCockpitHitPaletteFades[palette]);
            SetPaletteEntry((short)(palette + 0xf9),
                            g_9_aasCockpitHitPaletteFades[palette]);
        }
    } else if (g_4_nSevereDmgWarningSfxHandle != 0) {
        ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(
            g_4_nSevereDmgWarningSfxHandle, 1);
        g_4_nSevereDmgWarningSfxHandle = 0;
        g_12_abCockpitLightGoalTable[3] = 0;
    }
}

/* Function start: 0x4661C2 */
void get_player_input(void)
{
    /* t and ev held the DOS double-click logic; the port left them unused. */
    long t;
    short buttons;
    short ev;

    if (g_8_nActiveInputDeviceWord == -1)
        return;
    ReadCalibratedJoystick();
    buttons = (short)(g_aInputDeviceSamplesList[
        g_8_nActiveInputDeviceWord].buttons & INPUT_BUTTON_BOTH);
    g_wSavedInputDeviceButtonState =
        g_wFlightInputButtonState =
        g_12_wInputDeviceButtonStatus;
    g_wFlightInputButtonState &= (short)~0x1f;
    g_wFlightInputButtonState |= buttons;
    g_12_wInputDeviceButtonStatus =
        g_wFlightInputButtonState;
    if ((buttons & INPUT_BUTTON_PRIMARY) != 0) {
        TranslatePolledInputEvent(POLLED_INPUT_BUTTON_DOWN, (unsigned int)buttons);
        g_bFlightButtonOneHeld = 1;
    } else if (g_bFlightButtonOneHeld != 0) {
        TranslatePolledInputEvent(POLLED_INPUT_BUTTON_UP, INPUT_BUTTON_PRIMARY);
        g_bFlightButtonOneHeld = 0;
    }
    if ((buttons & INPUT_BUTTON_SECONDARY) != 0) {
        TranslatePolledInputEvent(POLLED_INPUT_BUTTON_DOWN, (unsigned int)buttons);
        g_bFlightButtonTwoHeld = 1;
    } else if (g_bFlightButtonTwoHeld != 0) {
        TranslatePolledInputEvent(POLLED_INPUT_BUTTON_UP, INPUT_BUTTON_SECONDARY);
        g_bFlightButtonTwoHeld = 0;
    }
    if (g_aInputDeviceSamplesList[
            g_8_nActiveInputDeviceWord].x != 0 ||
        g_aInputDeviceSamplesList[
            g_8_nActiveInputDeviceWord].y != 0) {
        TranslatePolledInputEvent(INPUT_EVENT_JOYSTICK_MOVE, (unsigned int)buttons);
    } else if (g_aInputDeviceSamplesList[
                   g_8_nActiveInputDeviceWord].x !=
                   g_stCurrentFlightInput.x ||
               g_aInputDeviceSamplesList[
                   g_8_nActiveInputDeviceWord].y !=
                   g_stCurrentFlightInput.y) {
        TranslatePolledInputEvent(INPUT_EVENT_JOYSTICK_MOVE, (unsigned int)buttons);
    }
    g_stCurrentFlightInput =
        g_aInputDeviceSamplesList[g_8_nActiveInputDeviceWord];
    g_12_wInputDeviceButtonStatus =
        g_wSavedInputDeviceButtonState;
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
    switch (g_cCurrentKey) {
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
        *key++ = (short)g_cCurrentKey;
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
                (short)((g_27_stViewScratchInfo.left + g_27_stViewScratchInfo.right) / 2),
                (short)((g_27_stViewScratchInfo.top + g_27_stViewScratchInfo.bottom) / 2));
            g_anFlightInput[1] =
                g_anFlightInput[0] =
                g_anFlightInput[2] = 0;
            handled = 1;
            break;
        case 0x48:
            g_nCockpitControlState = 0;
            if (g_anFlightInput[0] < 0) {
                g_anFlightInput[0] = 0;
            } else {
                if (shift != 0)
                    g_anFlightInput[0] = 9;
                if (g_anFlightInput[0] < 9)
                    g_anFlightInput[0]++;
                else if (g_cPreviousKey < 0)
                    g_anFlightInput[0]++;
                else
                    g_anFlightInput[0]--;
            }
            handled = 1;
            break;
        case 0x50:
            g_nCockpitControlState = 0;
            if (g_anFlightInput[0] > 0) {
                g_anFlightInput[0] = 0;
            } else {
                if (shift != 0)
                    g_anFlightInput[0] = -9;
                if (g_anFlightInput[0] > -9) {
                    g_anFlightInput[0]--;
                } else {
                    if (g_cPreviousKey < 0)
                        g_anFlightInput[0]--;
                    else
                        g_anFlightInput[0]++;
                }
            }
            handled = 1;
            break;
        case 0x34:
        case 0x53:
            g_nCockpitControlState = 0;
            if (g_anFlightInput[2] < 0) {
                g_anFlightInput[2] = 0;
            } else {
                if (shift != 0)
                    g_anFlightInput[2] = 9;
                if (g_anFlightInput[2] < 9)
                    g_anFlightInput[2]++;
                else if (g_cPreviousKey < 0)
                    g_anFlightInput[2]++;
                else
                    g_anFlightInput[2]--;
            }
            handled = 1;
            break;
        case 0x33:
        case 0x52:
            g_nCockpitControlState = 0;
            if (g_anFlightInput[2] > 0) {
                g_anFlightInput[2] = 0;
            } else {
                if (shift != 0)
                    g_anFlightInput[2] = -9;
                if (g_anFlightInput[2] > -9) {
                    g_anFlightInput[2]--;
                } else {
                    if (g_cPreviousKey < 0)
                        g_anFlightInput[2]--;
                    else
                        g_anFlightInput[2]++;
                }
            }
            handled = 1;
            break;
        case 0x4d:
            g_nCockpitControlState = 0;
            if (g_anFlightInput[1] < 0) {
                g_anFlightInput[1] = 0;
            } else {
                if (shift != 0)
                    g_anFlightInput[1] = 9;
                if (g_anFlightInput[1] < 9)
                    g_anFlightInput[1]++;
                else if (g_cPreviousKey < 0)
                    g_anFlightInput[1]++;
                else
                    g_anFlightInput[1]--;
            }
            handled = 1;
            break;
        case 0x4b:
            g_nCockpitControlState = 0;
            if (g_anFlightInput[1] > 0) {
                g_anFlightInput[1] = 0;
            } else {
                if (shift != 0)
                    g_anFlightInput[1] = -9;
                if (g_anFlightInput[1] > -9) {
                    g_anFlightInput[1]--;
                } else {
                    if (g_cPreviousKey < 0)
                        g_anFlightInput[1]--;
                    else
                        g_anFlightInput[1]++;
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
                 (int)g_asObjectCounter[0],
                 (int)g_asShipWeaponEnergy[0],
                 (unsigned)g_nSelectedGunType,
                 (int)g_nSelectedReleaseWeaponIndex);
#endif
    if (g_asObjectCounter[0] == -1 &&
        g_asShipWeaponEnergy[0] > 0) {
        fire_fixed_projectile_weapon(0);
        if (g_acShipTarget[0] != -1 &&
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
    if (g_aeSpecialManeuver[0] ==
            SPECIAL_MANEUVER_BLOWING_UP) {
        if (g_asObjectCounter[0] == -1) {
            if (g_aObjectTypeData[
                    g_acObjectType[0]].pitchRate >
                    g_anObjectYawRotation[0] &&
                g_aObjectTypeData[
                    g_acObjectType[0]].yawRate >
                    g_anObjectPitchRotation[0] &&
                g_aObjectTypeData[
                    g_acObjectType[0]].rollRate >
                    g_anObjectRollRotation[0]) {
                g_aeSpecialManeuver[0] = SPECIAL_MANEUVER_NONE;
            } else {
                g_anObjectYawRotation[0] -= g_anFlightInput[1];
                g_anObjectPitchRotation[0] -= g_anFlightInput[0];
            }
        }
    } else {
        g_anObjectPitchRotation[0] = (short)(
            (g_aObjectTypeData[
                g_acObjectType[0]].yawRate *
             g_anFlightInput[0]) / 8);
        g_anObjectYawRotation[0] = (short)-(
            (g_aObjectTypeData[
                g_acObjectType[0]].pitchRate *
             g_anFlightInput[1]) / 8);
        g_anObjectRollRotation[0] = (short)-(
            (g_aObjectTypeData[
                g_acObjectType[0]].rollRate *
             g_anFlightInput[2]) / 8);
    }
}

/* Function start: 0x466AD2 */
short IsInputEventQueued(int type)
{
    return FindQueuedInputEvent(type) != 0;
}

#pragma function(abs)

/* Function start: 0x466B02 */
short player_input(void)
{
    InputEventState message;
    short keyHandled;
    unsigned int buttons;
    int inputPolled;
    short inputCode;
    short boostControl;
    short absX;
    short verticalDistance;
    short threshold;
    short dx;
    short mouseY;
    int savedPlayerInputActive;
    HostMouseMessage *mouse;
#ifdef SDL_PORT
    signed char trailingKeyRelease;

    trailingKeyRelease = 0;
#endif

    keyHandled = 0;
    mouse = &g_6_stHostMouseTextBlock;
    buttons = 0;
    inputPolled = 0;
    g_cPreviousKey = g_cCurrentKey;
    g_anPreviousFlightInput[1] = g_anFlightInput[1];
    g_anPreviousFlightInput[0] = g_anFlightInput[0];
    g_anPreviousFlightInput[2] = g_anFlightInput[2];
    savedPlayerInputActive = g_10_bPlayerInputActive;
    g_10_bPlayerInputActive = 1;
    ServiceInputDevices(15);
    g_19_bPolledFlightInputQueued = IsInputEventQueued(INPUT_EVENT_JOYSTICK_MOVE);
    g_bMouseMoveEventQueued =
        (signed char)IsInputEventQueued(INPUT_EVENT_MOUSE_MOVE);
    g_1_bDiscreteFlightInputWaiting =
        (short)(IsInputEventQueued(INPUT_EVENT_KEY_DOWN) |
                IsInputEventQueued(INPUT_EVENT_LEGACY_6));
    if (g_bMouseMoveEventQueued == 0)
        g_cCurrentKey |= (signed char)0x80;
    if (IsInputEventQueued(INPUT_EVENT_KEY_UP) != 0) {
        g_cCurrentKey |= (signed char)0x80;
        if (g_19_bPolledFlightInputQueued == 0 &&
            g_nCockpitControlState == 0) {
            g_anFlightInput[1] =
                g_anFlightInput[0] =
                g_anFlightInput[2] =
                g_bMouseMoveEventQueued = 0;
        }
    }

    buttons = mouse->secondaryButton * 2 | mouse->primaryButton;
    if (buttons != 0) {
        if ((buttons & INPUT_BUTTON_PRIMARY) != 0) {
            if ((buttons & INPUT_BUTTON_SECONDARY) != 0) {
                if (g_bSecondaryMouseButtonHeld == 1)
                    fire_players_lasers();
                else
                    g_cCurrentKey = 0x1c;
            } else {
                fire_players_lasers();
            }
        }
        if (buttons == INPUT_BUTTON_BOTH)
            g_cCurrentKey = 0x1c;
    } else {
        g_bMouseFireButtonLatched = 0;
    }

    while ((inputCode = GetNextInputEvent(&message)) != INPUT_EVENT_NONE) {
        switch (inputCode) {
        case INPUT_EVENT_MOUSE_MOVE:
            if (inputPolled != 0)
                break;
            boostControl = (short)(message.modifiers & INPUT_MOD_SECONDARY_BUTTON);
            if (boostControl != 0 &&
                g_aeSpecialManeuver[0] ==
                    SPECIAL_MANEUVER_AFTERBURNER)
                boostControl = 0;
            if (g_nCockpitControlState == 0) {
                SetPersonnelMousePosition(
                    (short)((g_27_stViewScratchInfo.left +
                             g_27_stViewScratchInfo.right) / 2),
                    (short)((g_27_stViewScratchInfo.top +
                             g_27_stViewScratchInfo.bottom) / 2));
                g_nCockpitControlState = 1;
                g_37_stPersonnelMousePointerState.frame = 2;
            }
            dx = (short)(message.x -
                (g_27_stViewScratchInfo.right -
                 g_27_stViewScratchInfo.left) / 2 + 1);
            mouseY = (short)(message.y -
                (g_27_stViewScratchInfo.bottom -
                 g_27_stViewScratchInfo.top) / 2);
            g_37_stPersonnelMousePointerState.x = message.x;
            g_37_stPersonnelMousePointerState.y = message.y;
            absX = (short)abs((int)dx);
            verticalDistance = (short)abs((int)mouseY);
            threshold = 0;
            while (g_asMouseYawThresholds[threshold] <=
                   absX)
                threshold++;
            if (dx < 0)
                dx = (short)-threshold;
            else
                dx = threshold;
            threshold = 0;
            while (g_asMousePitchThresholds[threshold] <=
                   verticalDistance)
                threshold++;
            if (mouseY < 0)
                mouseY = (short)-threshold;
            else
                mouseY = threshold;
            if ((int)message.x - 4 <= (int)g_27_stViewScratchInfo.left)
                dx = -8;
            if ((int)message.x + 4 >= (int)g_27_stViewScratchInfo.right)
                dx = 8;
            if ((int)message.y - 4 <= (int)g_27_stViewScratchInfo.top)
                mouseY = -8;
            if ((int)message.y + 4 >= (int)g_27_stViewScratchInfo.bottom)
                mouseY = 8;
            if (dx >= 9)
                dx = 8;
            if (dx <= -9)
                dx = -8;
            if (mouseY >= 9)
                mouseY = 8;
            if (mouseY <= -9)
                mouseY = -8;
            if (boostControl != 0) {
                g_nCockpitControlGoal = 1;
                g_anFlightInput[2] =
                    g_nMouseYawInput = dx;
                g_nMousePitchInput = (short)-mouseY;
                accelerate((short)(g_nMousePitchInput / 2));
            } else if (g_nCockpitControlGoal == 1) {
                g_nCockpitControlGoal = 0;
                g_anFlightInput[2] = 0;
                g_anFlightInput[1] = g_nMouseYawInput = 0;
                g_anFlightInput[0] = g_nMousePitchInput = 0;
                SetPersonnelMousePosition(
                    (short)((g_27_stViewScratchInfo.left +
                             g_27_stViewScratchInfo.right) / 2),
                    (short)((g_27_stViewScratchInfo.top +
                             g_27_stViewScratchInfo.bottom) / 2));
            } else {
                g_anFlightInput[2] = 0;
                g_anFlightInput[1] =
                    g_nMouseYawInput = dx;
                g_anFlightInput[0] =
                    g_nMousePitchInput = mouseY;
            }
            break;
        case INPUT_EVENT_JOYSTICK_MOVE:
            inputPolled = 1;
            g_cCurrentKey = (signed char)PollKeyboardState();
            if (g_cCurrentKey != 0)
                process_player_input();
            g_nCockpitControlGoal = 0;
            g_nCockpitControlState =
                g_nCockpitControlGoal;
            boostControl = (short)(message.modifiers & INPUT_MOD_SECONDARY_BUTTON);
            if (boostControl != 0 &&
                g_aeSpecialManeuver[0] ==
                    SPECIAL_MANEUVER_AFTERBURNER)
                boostControl = 0;
            if (boostControl != 0) {
                g_anFlightInput[2] =
                    (short)g_stCurrentFlightInput.x;
                accelerate((short)-(
                    g_stCurrentFlightInput.y / 2));
            } else {
                if (g_anFlightInput[2] != 0 &&
                    g_cCurrentKey == 0) {
                    g_anFlightInput[2] = 0;
                    g_10_stPrevFlightControls.x = -1;
                }
                if (g_10_stPrevFlightControls.x !=
                        g_stCurrentFlightInput.x ||
                    g_10_stPrevFlightControls.y !=
                        g_stCurrentFlightInput.y) {
                    g_anFlightInput[1] =
                        (short)g_stCurrentFlightInput.x;
                    g_anFlightInput[0] =
                        (short)-g_stCurrentFlightInput.y;
                }
            }
            break;
        case INPUT_EVENT_BUTTON_UP:
            if (message.value == INPUT_BUTTON_SECONDARY)
                g_bSecondaryMouseButtonHeld = 0;
            break;
        case INPUT_EVENT_BUTTON_DOWN:
            if ((message.modifiers & INPUT_MOD_PRIMARY_BUTTON) != 0) {
                g_cCurrentKey = 0x39;
                if ((message.modifiers & INPUT_MOD_SECONDARY_BUTTON) != 0) {
                    if (g_bSecondaryMouseButtonHeld == 1)
                        fire_players_lasers();
                    else
                        g_cCurrentKey = 0x1c;
                } else {
                    fire_players_lasers();
                }
            }
            if (message.value == INPUT_BUTTON_SECONDARY && message.status > 1)
                g_bSecondaryMouseButtonHeld = 1;
            break;
        case INPUT_EVENT_LEGACY_6:
            keyHandled = 1;
        case INPUT_EVENT_KEY_DOWN:
#ifdef SDL_PORT
            trailingKeyRelease = 0;
#endif
            g_nCockpitControlGoal = 0;
            g_cCurrentKey = (signed char)message.status;
            process_player_input();
            break;
#ifdef SDL_PORT
        case INPUT_EVENT_KEY_UP:
            trailingKeyRelease = 1;
            break;
#endif
        }
    }
#ifdef SDL_PORT
    /* Apply a final key release after draining earlier repeat events. */
    if (trailingKeyRelease != 0 &&
        g_19_bPolledFlightInputQueued == 0 &&
        g_nCockpitControlState == 0) {
        g_anFlightInput[1] =
            g_anFlightInput[0] =
            g_anFlightInput[2] =
            g_bMouseMoveEventQueued = 0;
    }
#endif

    if (g_bSecondaryMouseButtonHeld == 1)
        g_cCurrentKey = 0xf;
    if ((g_cCurrentKey & 0x80) != 0 &&
        (g_stCurrentFlightInput.buttons & INPUT_BUTTON_PRIMARY) != 0)
        g_cCurrentKey = 0x39;
    if ((g_cCurrentKey & 0x80) != 0 &&
        (g_6_wQueuedButtonsState & 1) != 0)
        g_cCurrentKey = 0x39;
#ifdef SDL_PORT
    SdlApplyJoystickFlightControls();
#endif
    g_10_stPrevFlightControls =
        g_stCurrentFlightInput;
    g_10_bPlayerInputActive = savedPlayerInputActive;
    return keyHandled;
}

#pragma function(strlen)

/* Function start: 0x4672C5 */
/* Message dwell time: grows with text length, scaled by the speed setting. */
short MeasureMessageWidth(const char *text)
{
    return (short)((MinShort(5, (short)(strlen(text) >> 1)) + 5) *
                   ((char)g_cMessageSpeed + 1));
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
    /* The DOS dynamic-intelligence display used these two buffers.  Its
     * Win32 command was removed, but the retail frame retains their slots. */
    char debugText[60];
    char decimalText[30];
    short inputConsumed;
    short notRepeated;
    short controlHeld;
    short modifierMask;
    short pauseScan;
    short object;

    g_26_bMouseCursorEnabled = 1;
    g_88_nEventRepeatDelayState = g_26_bMouseCursorEnabled;
    if (g_nCurrentView == 4) {
        inputConsumed = HandleFleetOverviewInput();
    } else {
        inputConsumed = player_input();
        players_flight_dynamics();
    }
    notRepeated =
        g_cPreviousKey != g_cCurrentKey &&
        inputConsumed == 0;
    controlHeld = (short)GetControlKeyState();
    modifierMask = (short)GetKeyboardModifiers();

    if (g_4_bMenuBHotkey != 0 &&
        g_27_nLastAltCommandScanNumberWord != 0x30 &&
        g_nYourWingman != -1 &&
        g_aeShipObjective[g_nYourWingman] ==
            OBJECTIVE_HOLD_FORMATION &&
        any_enemy(0, 14000) != 0) {
        IssueQuickCommCommand(g_nYourWingman, 4);
        g_27_nLastAltCommandScanNumberWord = 0x30;
    }
    if (g_bJoystickCalibrationHotkey != 0 &&
        g_27_nLastAltCommandScanNumberWord != 0x2e) {
        CalibrateJoystickInteractive();
        g_27_nLastAltCommandScanNumberWord = 0x2e;
    }
    if (g_2_bAltFHotkeyFlag != 0 &&
        g_27_nLastAltCommandScanNumberWord != 0x21 &&
        g_nYourWingman != -1) {
        IssueQuickCommCommand(g_nYourWingman, 6);
        g_27_nLastAltCommandScanNumberWord = 0x21;
    }
    if (g_bMenuAHotkey != 0 &&
        g_27_nLastAltCommandScanNumberWord != 0x1e &&
        g_nYourWingman != -1 &&
        g_acShipTarget[0] != -1) {
        IssueQuickCommCommand(g_nYourWingman, 1);
        g_27_nLastAltCommandScanNumberWord = 0x1e;
    }
    if (g_2_bAltHHotkey != 0 &&
        g_27_nLastAltCommandScanNumberWord != 0x23 &&
        g_nYourWingman != -1 &&
        any_enemy(0, 14000) != 0) {
        IssueQuickCommCommand(g_nYourWingman, 2);
        g_27_nLastAltCommandScanNumberWord = 0x23;
    }
    if (g_6_bAltDChordState != 0 &&
        g_27_nLastAltCommandScanNumberWord != 0x20 &&
        g_nYourWingman != -1) {
        IssueQuickCommCommand(g_nYourWingman, 13);
        g_27_nLastAltCommandScanNumberWord = 0x20;
    }
    if (g_6_bMenuTHotkeySwitch != 0 &&
        g_27_nLastAltCommandScanNumberWord != 0x14 &&
        g_acShipTarget[0] != -1 &&
        g_asShipSide[g_acShipTarget[0]] ==
            SIDE_KILRATHI) {
        IssueQuickCommCommand(
            g_acShipTarget[0],
            (short)((unsigned short)RandomInRange(0, 2) +
                    g_nEnemyTauntCommandBase));
        g_27_nLastAltCommandScanNumberWord = 0x14;
    }
#ifdef SDL_PORT
    /* SDL latches these command presses until one flight-control pass. */
    g_4_bMenuBHotkey = 0;
    g_2_bAltFHotkeyFlag = 0;
    g_bMenuAHotkey = 0;
    g_2_bAltHHotkey = 0;
    g_6_bAltDChordState = 0;
    g_6_bMenuTHotkeySwitch = 0;
#endif
    if (g_39_bAltNumpadAddHotkeySwitch != 0 &&
        g_27_nLastAltCommandScanNumberWord != 0x4e) {
        g_27_nLastAltCommandScanNumberWord = 0x4e;
        AdjustSpaceFramePeriod(1);
    }
    if (g_bAltNumpadSubtractShortcut != 0 &&
        g_27_nLastAltCommandScanNumberWord != 0x4a) {
        g_27_nLastAltCommandScanNumberWord = 0x4a;
        AdjustSpaceFramePeriod(-1);
    }

    switch ((signed char)g_cCurrentKey) {
        case 0x13:
            if (notRepeated && g_78_bHighMemoryBuffersSetState != 0) {
                g_bSceneEscapeRequested = 0;
                BeginInflightReplayHook();
            }
            break;
        case 0x1e:
            if (notRepeated && controlHeld == 0) {
                g_nCockpitControlState = 0;
                if (get_mode(1) != 5)
                    SelectCockpitVduMode(1, 5);
                if (g_nCockpitDisplayMode == 0) {
                    auto_pilot_sequence();
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_27_stViewScratchInfo, 0, 0,
                        (short)(g_nScreenWidth - 1),
                        (short)(g_nScreenHeight - 1));
                    initialize_view_buffer();
                    g_nCockpitDisplayMode = -2;
                    auto_pilot_sequence();
                    g_nCockpitDisplayMode = 1;
                    free_view_buffer();
                    SetViewportRect(&g_27_stViewScratchInfo,
                                    0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
            break;
        case 0x2e:
            if (notRepeated) {
                g_bDisplayWingmanTargetData = 0;
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
            if (notRepeated && controlHeld != 0 &&
                g_acPlayerComponentDamage[7] != 4) {
                if (RandomInRange(
                        0, g_acPlayerComponentDamage[7]) == 0)
                    g_nArcadeState = 2;
                else
                    malf_sound();
            }
            break;
        case 0x31:
            if (notRepeated) {
                SelectCockpitVduMode(1, 5);
                g_bDisplayWingmanTargetData = 0;
            }
            break;
        case 0x32:
            if (notRepeated && controlHeld == 0)
                SetMessageDisplaySpeed();
            break;
        case 0x2f:
            if (notRepeated && controlHeld == 0) {
                g_bVideoImagesSuppressed =
                    g_bVideoImagesSuppressed == 0;
                if (g_bVideoImagesSuppressed != 0)
                    SetHudMessageText(
                        _("VIDEO IMAGES SUPRESSED"),
                        g_cRedColour, 20);
                else
                    SetHudMessageText(_("VIDEO IMAGES ENABLED"),
                                      g_ucPrimaryTextColour, 20);
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
                g_nCurrentView == 0 && (signed char)g_cCurrentKey >= 2 &&
                (signed char)g_cCurrentKey <=
                    g_nCommMenuChoiceCount + 2 &&
                get_mode(1) == 4) {
                Chosen_communicate_option(
                    (short)((signed char)g_cCurrentKey - 2));
            }
            break;
        case 0x1f:
            if (controlHeld != 0 && notRepeated) {
                if (g_nSfxVolumeSetting == 0)
                    g_nSfxVolumeSetting = 20;
                else
                    g_nSfxVolumeSetting = 0;
                SetSoundEffectsVolume(
                    g_anVolumeLevels[
                        g_nSfxVolumeSetting / 2]);
                ShowOnScreenMessage(0, _("SFX VOLUME: %d."),
                                    g_nSfxVolumeSetting / 2);
            }
            break;
        case 0x3b:
            if (g_nTargetCameraFrame == 1)
                ClearTargetCameraView();
            g_nCockpitControlState = 0;
            if (GetF1KeyLatch() == 0)
                break;
            if (g_1_cStoredViewDisplayModeValue == 0) {
                g_nCockpitDisplayMode =
                    g_nCockpitDisplayMode == 0;
                free_view_buffer();
                if (g_nCockpitDisplayMode != 0) {
                    g_bFullScreenSpaceView = 0;
                    SetViewportRect(&g_27_stViewScratchInfo,
                                    0, 0, 319, 199);
                    initialize_cockpit(
                        ++g_1_cStoredViewDisplayModeValue - 1);
                } else {
                    g_bFullScreenSpaceView = 1;
                    SetViewportRect(
                        &g_27_stViewScratchInfo, 0, 0,
                        (short)(g_nScreenWidth - 1),
                        (short)(g_nScreenHeight - 1));
                    initialize_cockpit(
                        ++g_1_cStoredViewDisplayModeValue - 1);
                }
            } else {
                g_nCockpitControlState = 0;
                if (g_nCockpitDisplayMode == 0) {
                    new_view(0, 0);
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_27_stViewScratchInfo, 0, 0,
                        (short)(g_nScreenWidth - 1),
                        (short)(g_nScreenHeight - 1));
                    initialize_view_buffer();
                    new_view(0, 0);
                    free_view_buffer();
                    SetViewportRect(&g_27_stViewScratchInfo, 0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
            FlushInputEvents();
            break;
        case 0x3c:
            if (g_nTargetCameraFrame == 1)
                ClearTargetCameraView();
            if (g_1_cStoredViewDisplayModeValue == 2) {
                g_nCockpitDisplayMode =
                    g_nCockpitDisplayMode == 0;
                free_view_buffer();
                if (g_nCockpitDisplayMode != 0) {
                    g_bFullScreenSpaceView = 0;
                    SetViewportRect(&g_27_stViewScratchInfo,
                                    0, 0, 319, 199);
                    initialize_cockpit(++g_1_cStoredViewDisplayModeValue - 1);
                } else {
                    g_bFullScreenSpaceView = 1;
                    SetViewportRect(
                        &g_27_stViewScratchInfo, 0, 0,
                        (short)(g_nScreenWidth - 1),
                        (short)(g_nScreenHeight - 1));
                    initialize_cockpit(++g_1_cStoredViewDisplayModeValue - 1);
                }
            } else {
                g_nCockpitControlState = 0;
                if (g_nCockpitDisplayMode == 0) {
                    new_view(2, 0);
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_27_stViewScratchInfo, 0, 0,
                        (short)(g_nScreenWidth - 1),
                        (short)(g_nScreenHeight - 1));
                    initialize_view_buffer();
                    new_view(2, 0);
                    free_view_buffer();
                    SetViewportRect(&g_27_stViewScratchInfo,
                                    0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
            break;
        case 0x3d:
            if (g_nTargetCameraFrame == 1)
                ClearTargetCameraView();
            if (g_1_cStoredViewDisplayModeValue == 1) {
                g_nCockpitDisplayMode =
                    g_nCockpitDisplayMode == 0;
                free_view_buffer();
                if (g_nCockpitDisplayMode != 0) {
                    g_bFullScreenSpaceView = 0;
                    SetViewportRect(&g_27_stViewScratchInfo,
                                    0, 0, 319, 199);
                    initialize_cockpit(++g_1_cStoredViewDisplayModeValue - 1);
                } else {
                    g_bFullScreenSpaceView = 1;
                    SetViewportRect(
                        &g_27_stViewScratchInfo, 0, 0,
                        (short)(g_nScreenWidth - 1),
                        (short)(g_nScreenHeight - 1));
                    initialize_cockpit(++g_1_cStoredViewDisplayModeValue - 1);
                }
            } else {
                g_nCockpitControlState = 0;
                if (g_nCockpitDisplayMode == 0) {
                    new_view(1, 0);
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_27_stViewScratchInfo, 0, 0,
                        (short)(g_nScreenWidth - 1),
                        (short)(g_nScreenHeight - 1));
                    initialize_view_buffer();
                    new_view(1, 0);
                    free_view_buffer();
                    SetViewportRect(&g_27_stViewScratchInfo,
                                    0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
            break;
        case 0x3e:
            if (g_nTargetCameraFrame == 1)
                ClearTargetCameraView();
            if (g_1_cStoredViewDisplayModeValue == 3) {
                g_nCockpitDisplayMode =
                    g_nCockpitDisplayMode == 0;
                free_view_buffer();
                if (g_nCockpitDisplayMode != 0) {
                    g_bFullScreenSpaceView = 0;
                    SetViewportRect(&g_27_stViewScratchInfo,
                                    0, 0, 319, 199);
                    initialize_cockpit(++g_1_cStoredViewDisplayModeValue - 1);
                } else {
                    g_bFullScreenSpaceView = 1;
                    SetViewportRect(
                        &g_27_stViewScratchInfo, 0, 0,
                        (short)(g_nScreenWidth - 1),
                        (short)(g_nScreenHeight - 1));
                    initialize_cockpit(++g_1_cStoredViewDisplayModeValue - 1);
                }
            } else {
                g_nCockpitControlState = 0;
                if (g_nCockpitDisplayMode == 0) {
                    new_view(3, 0);
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_27_stViewScratchInfo, 0, 0,
                        (short)(g_nScreenWidth - 1),
                        (short)(g_nScreenHeight - 1));
                    initialize_view_buffer();
                    new_view(3, 0);
                    free_view_buffer();
                    SetViewportRect(&g_27_stViewScratchInfo,
                                    0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
            break;
        case 0x3f:
            if (g_nTargetCameraFrame == 1)
                ClearTargetCameraView();
            if (g_1_cStoredViewDisplayModeValue == 5) {
                g_nCockpitDisplayMode =
                    g_nCockpitDisplayMode == 0;
                free_view_buffer();
                if (g_nCockpitDisplayMode != 0) {
                    g_bFullScreenSpaceView = 0;
                    SetViewportRect(&g_27_stViewScratchInfo,
                                    0, 0, 319, 199);
                    initialize_cockpit(++g_1_cStoredViewDisplayModeValue - 1);
                } else {
                    g_bFullScreenSpaceView = 1;
                    SetViewportRect(
                        &g_27_stViewScratchInfo, 0, 0,
                        (short)(g_nScreenWidth - 1),
                        (short)(g_nScreenHeight - 1));
                    initialize_cockpit(++g_1_cStoredViewDisplayModeValue - 1);
                }
            } else {
                g_nCockpitControlState = 0;
                if (g_nCockpitDisplayMode == 0) {
                    new_view(5, 0);
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_27_stViewScratchInfo, 0, 0,
                        (short)(g_nScreenWidth - 1),
                        (short)(g_nScreenHeight - 1));
                    initialize_view_buffer();
                    new_view(5, 0);
                    free_view_buffer();
                    SetViewportRect(&g_27_stViewScratchInfo,
                                    0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
            break;
        case 0x40:
            if (g_nTargetCameraFrame == 1)
                ClearTargetCameraView();
            if (g_1_cStoredViewDisplayModeValue == 15) {
                g_nCockpitDisplayMode =
                    g_nCockpitDisplayMode == 0;
                free_view_buffer();
                if (g_nCockpitDisplayMode != 0) {
                    g_bFullScreenSpaceView = 0;
                    SetViewportRect(&g_27_stViewScratchInfo,
                                    0, 0, 319, 199);
                    initialize_cockpit(++g_1_cStoredViewDisplayModeValue - 1);
                } else {
                    g_bFullScreenSpaceView = 1;
                    SetViewportRect(
                        &g_27_stViewScratchInfo, 0, 0,
                        (short)(g_nScreenWidth - 1),
                        (short)(g_nScreenHeight - 1));
                    initialize_cockpit(++g_1_cStoredViewDisplayModeValue - 1);
                }
            } else {
                g_nCockpitControlState = 0;
                if (g_nCockpitDisplayMode == 0) {
                    new_view(15, 0);
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_27_stViewScratchInfo, 0, 0,
                        (short)(g_nScreenWidth - 1),
                        (short)(g_nScreenHeight - 1));
                    initialize_view_buffer();
                    new_view(15, 0);
                    free_view_buffer();
                    SetViewportRect(&g_27_stViewScratchInfo,
                                    0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
            break;
        case 0x41:
            if (g_nTargetCameraFrame == 1)
                ClearTargetCameraView();
            if (g_1_cStoredViewDisplayModeValue == 8) {
                g_nCockpitDisplayMode =
                    g_nCockpitDisplayMode == 0;
                free_view_buffer();
                if (g_nCockpitDisplayMode != 0) {
                    g_bFullScreenSpaceView = 0;
                    SetViewportRect(&g_27_stViewScratchInfo,
                                    0, 0, 319, 199);
                    initialize_cockpit(++g_1_cStoredViewDisplayModeValue - 1);
                } else {
                    g_bFullScreenSpaceView = 1;
                    SetViewportRect(
                        &g_27_stViewScratchInfo, 0, 0,
                        (short)(g_nScreenWidth - 1),
                        (short)(g_nScreenHeight - 1));
                    initialize_cockpit(++g_1_cStoredViewDisplayModeValue - 1);
                }
            } else if (g_acShipTarget[0] != -1) {
                g_nCockpitControlState = 0;
                if (g_nCockpitDisplayMode == 0) {
                    new_view(8, 0);
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_27_stViewScratchInfo, 0, 0,
                        (short)(g_nScreenWidth - 1),
                        (short)(g_nScreenHeight - 1));
                    initialize_view_buffer();
                    new_view(8, 0);
                    free_view_buffer();
                    SetViewportRect(&g_27_stViewScratchInfo,
                                    0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
            break;
        case 0x42:
            if (g_nTargetCameraFrame == 1)
                ClearTargetCameraView();
            if (notRepeated) {
                g_nCockpitControlState = 0;
                g_bMissileCameraEnabled ^= 1;
                if (g_bMissileCameraEnabled != 0)
                    SetHudMessageText(_("MISSILE CAMERA ON"),
                                      g_cRedColour, 20);
                else
                    SetHudMessageText(_("MISSILE CAMERA OFF"),
                                      g_ucPrimaryTextColour, 20);
            }
            break;
        case 0x43:
            if (notRepeated) {
                g_nCockpitControlState = 0;
                SelectNextExternalViewObject();
                if (g_nCockpitDisplayMode == 0) {
                    new_view(5, 0);
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_27_stViewScratchInfo, 0, 0,
                        (short)(g_nScreenWidth - 1),
                        (short)(g_nScreenHeight - 1));
                    initialize_view_buffer();
                    force_view(5, 0);
                    free_view_buffer();
                    SetViewportRect(&g_27_stViewScratchInfo,
                                    0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
            break;
    }

    switch ((signed char)g_cCurrentKey) {
    case 0x0d:
    case 0x4e:
        if (controlHeld != 0) {
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
        if (controlHeld != 0) {
            ReportFramesSkipped(-1);
        } else {
            accelerate(-1);
        }
        break;
    case 0x0e:
        g_anShipSpeed[0] = 0;
        break;
    case 0x1c:
        if (notRepeated && g_nSelectedReleaseWeaponIndex != -1 &&
            g_nExternalViewShip == -1) {
            g_nExternalViewShip = fire_missile(0);
            if (g_nExternalViewShip != -1 &&
                g_asObjectType[g_nExternalViewShip] ==
                    0x14) {
                g_nExternalViewShip = -1;
            } else if (g_bMissileCameraEnabled == 1 &&
                       g_nExternalViewShip != -1) {
                new_view(7, g_nExternalViewShip);
            }
        }
        break;
    case 0x39:
        if (g_nCurrentView == 4)
            FireTargetCameraGuns();
        else
            fire_players_lasers();
        break;
    case 0x0f:
    case 0x37:
        if (g_asObjectType[0] != 0x33)
            your_afterburner();
        break;
    case 1:
        g_bSceneEscapeRequested = 0;
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
        if (notRepeated && controlHeld == 0) {
            SelectCockpitVduMode(1, 3);
            g_bDisplayWingmanTargetData = 0;
        }
        break;
    case 0x11:
        if (notRepeated) {
            if (g_nCurrentView == 4) {
                ToggleTargetCameraOverlay();
            } else if (IsCockpitWeaponShapeLoaded() != 0) {
                SelectCockpitVduMode(0, 1);
            }
        }
        break;
    case 0x2f:
        if (notRepeated && controlHeld != 0)
            ShowVersionBanner();
        break;
    case 0x24:
        if (notRepeated && controlHeld != 0)
            CalibrateJoystickInteractive();
        else if (notRepeated)
            TryBeginStarSystemJump();
        break;
    case 0x19:
#ifdef SDL_PORT
pause_spaceflight:
#endif
        g_bPauseInputActive = 1;
        ShowGamePausedBanner((short)(controlHeld == 0));
        g_bPauseInputActive = 0;
        SetFrameTimerPeriodDirect(1);
        for (pauseScan = 0; pauseScan < 0x100; pauseScan++) {
            g_128_abPauseInputState[pauseScan] = 0;
            g_13_abPreviousPauseKeyStatuses[pauseScan] =
                g_128_abPauseInputState[pauseScan];
        }
        ServiceInputDevices(-1);
        g_nCockpitControlGoal = 0;
        g_nCockpitControlState =
            g_nCockpitControlGoal;
        break;
    case 0x20:
        if (notRepeated && controlHeld == 0 &&
            IsCockpitWeaponShapeLoaded() != 0)
            SelectCockpitVduMode(0, 2);
        break;
    case 0x22:
        if (notRepeated) {
            if (g_nCurrentView == 4) {
                ToggleTargetCameraOverlay();
            } else if (IsCockpitWeaponShapeLoaded() != 0) {
                SelectCockpitVduMode(0, 1);
            }
        }
        break;
    case 0x26:
        if (notRepeated) {
            g_bTargetLockMode =
                (short)(g_bTargetLockMode == 0);
            PlaySfxWaveFileByNumber(0x19, -1, 0);
            if (get_mode(1) == 3)
                InvalidateVduMode(1);
        }
        break;
    case 0x32:
        if (notRepeated && controlHeld != 0) {
            if (g_nMusicVolumeSetting == 0)
                g_nMusicVolumeSetting = 20;
            else
                g_nMusicVolumeSetting = 0;
            SetMusicStreamVolume(
                (unsigned short)g_anVolumeLevels[
                    g_nMusicVolumeSetting / 2]);
            ShowOnScreenMessage(0, _("MUSIC VOLUME: %d."),
                                g_nMusicVolumeSetting / 2);
        }
        break;
    }

    switch ((signed char)g_cCurrentKey) {
    case 0x48:
        if (controlHeld != 0) {
            g_nSfxVolumeSetting++;
            if (g_nSfxVolumeSetting > 20)
                g_nSfxVolumeSetting = 20;
            SaveVolumeSettingsToRegistry();
            SetSoundEffectsVolume(
                g_anVolumeLevels[
                    g_nSfxVolumeSetting / 2]);
            ShowOnScreenMessage(0, _("SFX VOLUME: %d."),
                                g_nSfxVolumeSetting / 2);
            return 0;
        }
        break;
    case 0x50:
        if (controlHeld != 0) {
            g_nSfxVolumeSetting--;
            if (g_nSfxVolumeSetting < 0)
                g_nSfxVolumeSetting = 0;
            SetSoundEffectsVolume(
                g_anVolumeLevels[
                    g_nSfxVolumeSetting / 2]);
            SaveVolumeSettingsToRegistry();
            ShowOnScreenMessage(0, _("SFX VOLUME: %d."),
                                g_nSfxVolumeSetting / 2);
            return 0;
        }
        break;
    case 0x4d:
        if (controlHeld != 0) {
            g_nMusicVolumeSetting++;
            if (g_nMusicVolumeSetting > 20)
                g_nMusicVolumeSetting = 20;
            SaveVolumeSettingsToRegistry();
            SetMusicStreamVolume(
                (unsigned short)g_anVolumeLevels[
                    g_nMusicVolumeSetting / 2]);
            ShowOnScreenMessage(0, _("MUSIC VOLUME: %d."),
                                g_nMusicVolumeSetting / 2);
            return 0;
        }
        break;
    case 0x4b:
        if (controlHeld != 0) {
            g_nMusicVolumeSetting--;
            if (g_nMusicVolumeSetting < 0)
                g_nMusicVolumeSetting = 0;
            SaveVolumeSettingsToRegistry();
            SetMusicStreamVolume(
                (unsigned short)g_anVolumeLevels[
                    g_nMusicVolumeSetting / 2]);
            ShowOnScreenMessage(0, _("MUSIC VOLUME: %d."),
                                g_nMusicVolumeSetting / 2);
            return 0;
        }
        break;
    }

    if (g_nOriginDevUnlock != 0) {
        switch ((signed char)g_cCurrentKey) {
        case 0x52:
            if (notRepeated && controlHeld != 0) {
                for (object = 1; object < 10; object++) {
                    if (g_asShipSide[object] !=
                            g_asShipSide[0] &&
                        g_aeObjectClass[object] ==
                            OBJECT_CLASS_SHIP) {
                        while (explode(0, object) == 0) {
                        }
                    }
                }
            }
            break;
        case 0x53:
            if (notRepeated && controlHeld != 0 &&
                g_bDebugBreakEnabled != 0) {
                object = g_acShipTarget[0];
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
    if (g_nSpaceExplosionFlashActive != 0) {
        if ((short)g_nSpacePaletteFadeMode == 0x13) {
            g_nSpaceBlastFlashIncrement++;
            if (g_nSpaceBlastFlashIncrement < 4)
                paletteCycleActive++;
            if (g_aSpaceExplosionFlashPalette[
                    g_nSpaceBlastFlashIncrement][0] != 0) {
                g_1_asSpacePaletteFade[0] =
                    g_aSpaceExplosionFlashPalette[
                        g_nSpaceBlastFlashIncrement][0];
                g_1_asSpacePaletteFade[1] =
                    g_aSpaceExplosionFlashPalette[
                        g_nSpaceBlastFlashIncrement][1];
                g_1_asSpacePaletteFade[2] =
                    g_aSpaceExplosionFlashPalette[
                        g_nSpaceBlastFlashIncrement][2];
            } else {
                g_1_asSpacePaletteFade[0] = 0;
                g_1_asSpacePaletteFade[1] = 0;
                g_1_asSpacePaletteFade[2] = 0x20;
                g_nSpaceExplosionFlashActive = 0;
            }
            SetPaletteEntry((short)g_cPrimaryViewBufferColour,
                            g_1_asSpacePaletteFade);
        }
    } else {
        UpdateSpacePaletteFade();
    }
    g_nFrameSkipCountdown--;
    if (g_nFrameSkipCountdown > 0)
        return 0;
    g_nFrameSkipCountdown = g_nFrameSkip;
    g_nRenderedSpaceFrame++;
    transform_objects_to_your_view();
    update_star_field();
    place_exhaust_on_ships();
    reposition_fixed_child_objects();
    BuildObjectDepthOrder();
#ifdef SDL_PORT
    SdlBeginSpaceFrame(
        (const struct ScreenViewportGeometry *)
            g_15_pScreenViewportGeometry,
        (int)g_1_cStoredViewDisplayModeValue,
        /* Match dump_buffer_to_screen's active copy mode.  The stored
           preference has the opposite sense and omits cockpit offsets. */
        g_nCockpitDisplayMode > 0,
        (unsigned char)g_cPrimaryViewBufferColour);
#endif
    if (g_bJumpSequenceActive != 0 &&
        g_asShipManeuver[0] == MANEUVER_WARPING_OUT) {
        hidePlayer = 1;
        g_aeObjectClass[0] = OBJECT_CLASS_NULL;
    }
    if (paletteCycleActive == 0)
        draw_sorted_objects_to_buffer();
    if (hidePlayer != 0)
        g_aeObjectClass[0] = OBJECT_CLASS_SHIP;
    if (g_nCurrentView == 0 || g_nCurrentView == 4)
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
    if (g_pszPendingHudMessage !=
            g_pszDisplayedHudMessage &&
        g_pszDisplayedHudMessage != 0) {
        ClearHudMessageDisplay(0);
    }
    if (g_nCockpitDisplayMode > 0) {
        displayMode = g_nCockpitDisplayMode;
        g_nCockpitDisplayMode = 9;
    }
    dump_buffer_to_screen();
    if (g_nCockpitDisplayMode == 9)
        g_nCockpitDisplayMode = displayMode;
    if (g_nCurrentView == 0 || g_nCurrentView == 4)
        RestoreTransientCockpitGraphics();
    ClearViewport(&g_27_stViewScratchInfo,
                  g_cPrimaryViewBufferColour);
}

/* Function start: 0x4690FF */
short Draw_3Space_Frame(void)
{
    Update_3Space();
    if (g_nArcadeState != 0)
        return 0;
    if (g_nFrameSkipCountdown <= 1)
        clear_view_buffer();
    return (unsigned short)DrawSpaceSceneFrame();
}

/* Function start: 0x469143 */
short GetShipDistanceToNavPoint(short ship, MissionNavPoint *navPoint)
{
    FixedVector delta;

    ComputeVectorDelta(&g_aShipPosition[ship],
                       &navPoint->position, &delta);
    return FixedToShortSaturating(Vector_magnitude(&delta));
}

/* Function start: 0x46918D */
short FindNearestNavPoint(short ship)
{
    MissionNavPoint *entry = g_aMissionNavPoints;
    short navPointIndex = 0;

    for (; navPointIndex < 10; navPointIndex++, entry++) {
        if (entry->type == 1 &&
            entry->systemIndex == g_nCurStellarSystem &&
            GetShipDistanceToNavPoint(ship, entry) <
                entry->proximityRadius)
            return navPointIndex;
    }
    return g_nCurrentNavPoint;
}

/* Function start: 0x469223 */
unsigned int ReleaseStaleNavTarget(void)
{
    short v = FindNearestNavPoint(0);

    if (g_nCurrentNavPoint != v)
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

    savedUiInputMode = g_18_nInterfaceEventMode;
    g_1_bSpaceFlightActive = 1;
    g_nCockpitDisplayMode = 0;
    if (g_bFullScreenSpaceView == 0)
        g_nCockpitDisplayMode = 1;
    for (i = 0; i < 7; i++)
        g_acMissionShipStatusFlagsBytes[i] = 0;
    g_bMissionDeathSequencePending = 0;
    g_6_bMissionWingmanState = 0;
    g_nFrameSkipCountdown = 1;
    InitializeSpaceFlightInput();
    g_nSpaceExplosionFlashActive = 0;
    savedInputDelay = g_88_nEventRepeatDelayState;
    savedInputContext = g_37_stPersonnelMousePointerState.context;
    ReleaseSceneMusicPacket();
    init_inflight_music();

    if (entryNavPoint == -1)
        entryNavPoint = g_aMissionShips[
            g_13_stMissionHeaderBlock.playerMissionShip].navPoint;
    set_up_action_sphere(entryNavPoint);
    force_view(0, 0);
    SetSpaceFlightFrameTiming();

    if (g_nCockpitDisplayMode != 0) {
        free_view_buffer();
        SetViewportRect(&g_27_stViewScratchInfo, 0, 0,
                        (unsigned short)(g_nScreenWidth - 1),
                        (unsigned short)(g_nScreenHeight - 1));
        initialize_view_buffer();
        new_view(0, 0);
        free_view_buffer();
        SetViewportRect(&g_27_stViewScratchInfo, 0, 0, 319, 199);
        g_1_cStoredViewDisplayModeValue++;
        initialize_cockpit(g_1_cStoredViewDisplayModeValue - 1);
        g_nCockpitControlState =
            g_nCockpitControlGoal = 0;
        initialize_view_buffer();
        FlushInputEvents();
    }

    copy_frame(0, 0x44);
    SetPersonnelMousePosition(
        (short)((g_27_stViewScratchInfo.left +
                 g_27_stViewScratchInfo.right) / 2),
        (short)((g_27_stViewScratchInfo.top +
                 g_27_stViewScratchInfo.bottom) / 2));
    g_nCockpitControlState = 0;
    g_nArcadeState = 0;
#ifdef SDL_PORT
    SdlSetMouseGrab(1);
#endif

    while (g_nArcadeState == 0) {
        SetFrameTimerPeriodDirect(g_nSpaceFramePeriod);
        if (g_nCannedSceneMode != 0) {
            ProcessCannedSceneInput();
            result = 0;
            if (g_nCannedSceneRecordedFrameCountNumber ==
                g_5_nCannedSceneFrameCount) {
                FinishCannedScenePlayback();
                ResetCannedSceneRecording();
            }
        }
        if (g_nCannedSceneMode == 0 &&
            g_bJumpSequenceActive == 0)
            result = (short)HandleSpaceFlightControls();
        if (result == -1) {
            g_nArcadeState = 5;
        } else {
            Update_3Space();
            if (g_nArcadeState != 0)
                break;
            g_bRenderingSpaceFrame = 1;
            RenderSpaceViewFrame();
            g_bRenderingSpaceFrame = 0;
            update_cockpit();
            if (g_pszPendingHudMessage != 0)
                SetHudTextColour(g_pszPendingHudMessage,
                                  g_18_nHudMessageColour);
        }
        MarkDibDirty();
        DIBslamReal();
        WaitForFrameTick();
    }

#ifdef SDL_PORT
    SdlSetMouseGrab(0);
    SdlCancelSpaceFrame();
#endif
    SetViewportRect(&g_27_stViewScratchInfo, 0, 0,
                    (unsigned short)(g_nScreenWidth - 1),
                    (unsigned short)(g_nScreenHeight - 1));
    g_nCockpitDisplayMode = 0;
    if (g_nArcadeState == 1)
        flag_objective(find_objective(1, -1), 2);
    ResetCockpitPaletteEntries();
    SetDefaultFrameTiming();
    if (savedInputContext != 0)
        SetInputViewport(savedInputContext->viewport);
    free_inflight_music();
    ConfigureInputPump(1, 0);
    g_nCockpitControlState = 0;
    g_18_nInterfaceEventMode = savedUiInputMode;
    g_88_nEventRepeatDelayState = savedInputDelay;
    g_1_bSpaceFlightActive = 0;
    return g_nArcadeState;
}

/* Function start: 0x4695FD */
short calculate_damage_level(void)
{
    ObjectTypeData *typeData;
    int damage;
    int armorDamage;

    armorDamage = 0;
    typeData = &g_aObjectTypeData[g_acObjectType[0]];
    damage = g_asObjectDamage[0] * 5;
    damage += (g_acShipDamage[0] * 30) /
              typeData->damageCapacity;
    if (typeData->armorFront < g_aasShipArmor[0][0])
        armorDamage += (typeData->armorFront * 100) /
                       g_aasShipArmor[0][0];
    if (typeData->armorRear < g_aasShipArmor[0][1])
        armorDamage += (typeData->armorRear * 100) /
                       g_aasShipArmor[0][1];
    if (typeData->armorLeft < g_aasShipArmor[0][2])
        armorDamage += (typeData->armorLeft * 100) /
                       g_aasShipArmor[0][2];
    if (typeData->armorRight < g_aasShipArmor[0][3])
        armorDamage += (typeData->armorRight * 100) /
                       g_aasShipArmor[0][3];
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
int g_bFullScreenSpaceView = 1; /* 0x0049d718 */
int g_nCockpitDisplayMode = 0; /* 0x0049d71c */
int g_anVolumeLevels[11] = {
    0, 500, 1000, 2500, 5000, 11000,
    22500, 34000, 44000, 54000, 64000
}; /* 0x0049d720 */
int g_nSfxVolumeSetting = 0x14; /* 0x0049d74c */
int g_nMusicVolumeSetting = 0x14; /* 0x0049d750 */
int g_nUnusedMainWord1 = 0x1370; /* 0x0049d754 */
int g_nTrainSimActive = 0; /* 0x0049d758 */
int g_nArcadeState = 0; /* 0x0049d75c */
short g_nFrameSkipCountdown = 1; /* 0x0049d760 */
short g_nFrameSkip = 0; /* 0x0049d764 */
short g_nSpaceFramePeriod = 4; /* 0x0049d768 */
short g_bViewportDirty = 0; /* 0x0049d76c */
short g_nCockpitCameraObject = 0; /* 0x0049d770 */
short g_nOriginDevUnlock = 0; /* 0x0049d774 */
short g_bCarrierLandingEnabled = 1; /* 0x0049d778 */
short g_bPlayerDamageEnabled = 1; /* 0x0049d77c */
short g_bPlayerCollisionEnabled = 1; /* 0x0049d780 */
short g_nShowMemoryStatus = 0; /* 0x0049d784 */
void *g_pMemoryAdjustment = 0; /* 0x0049d788 */
signed char g_bParentDirectorySelected = 0; /* 0x0049d78c */
short g_nMissionEntryNavOverride = -1; /* 0x0049d790 */
int g_nUnusedMainWord2 = 0; /* 0x0049d794 */
struct DirectMissionSelection g_stDirectMissionSelection = { 0, 0 }; /* 0x0049d798 */
short g_nDirectSeries = 1; /* 0x0049d79c */
int g_nUnusedMainWord3 = 0; /* 0x0049d7a0 */
short g_bSpaceViewBufferEnabled = 0; /* 0x0049d7a4 */
int g_nUnusedMainWord4 = 0; /* 0x0049d7a8 */
short g_nCockpitControlState = 0; /* 0x0049d7ac */
InputDeviceSample g_stCurrentFlightInput = { 0 }; /* 0x0049d7b0 */
int g_nUnusedMainWord5 = 0; /* 0x0049d7bc */
short g_wFlightInputButtonState = 0; /* 0x0049d7c0 */
int g_nUnusedMainWord6 = 0; /* 0x0049d7c4 */
int g_bFlightButtonTwoHeld = 0; /* 0x0049d7c8 */
int g_bFlightButtonOneHeld = 0; /* 0x0049d7cc */
short g_nCockpitControlGoal = 0; /* 0x0049d7d0 */
short g_asMouseYawThresholds[6] = {
    10, 37, 52, 57, 62, 1070
}; /* 0x0049d7d8 */
short g_asMousePitchThresholds[6] = {
    5, 18, 27, 35, 38, 1040
}; /* 0x0049d7e8 */
signed char g_bSecondaryMouseButtonHeld = 0; /* 0x0049d7f4 */
int g_nUnusedMainWord7 = 0; /* 0x0049d7f8 */
signed char g_bMouseMoveEventQueued = 0; /* 0x0049d7fc */
short g_nMouseYawInput = 0; /* 0x0049d800 */
short g_nMousePitchInput = 0; /* 0x0049d804 */
short g_aSpaceExplosionFlashPalette[13][3] = {
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
}; /* 0x0049d808 */
int g_bRenderingSpaceFrame = 0; /* 0x0049d858 */

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
InputDeviceSample g_10_stPrevFlightControls; /* 0x005c57d0 */
short g_bMapPrimaryInputQueued; /* 0x005c57dc */
int g_6_nUnusedMainNumber1; /* 0x005c57e0 */ /* never referenced */
short g_1_bDiscreteFlightInputWaiting; /* 0x005c57e4 */
short g_1_nResourcePaletteState; /* 0x005c57e6 */
short g_3_nOptionMessageColour; /* 0x005c57e8 */
short g_2_bFleetMouseMovePosted; /* 0x005c57ea */
char *g_pInitialArguments[31]; /* 0x005c57f0 */
int g_1_bSpaceFlightActive; /* 0x005c586c */
short g_nCurSeries; /* 0x005c5870 */
short g_nSpaceBlastFlashIncrement; /* 0x005c5872 */
short g_nMenuSelectedCol; /* 0x005c5874 */
short g_10_nMenuDarkColour; /* 0x005c5876 */
short g_nCurrentLevel; /* 0x005c5878 */
short g_19_bPolledFlightInputQueued; /* 0x005c587a */
int g_39_nIdleMainNumber[4]; /* 0x005c5880 */ /* never referenced */
InputEvent g_3_aInputEventArray[0x100]; /* 0x005c5890 */
int g_10_bPlayerInputActive; /* 0x005c8090 */
int g_47_nUnusedMainValue1; /* 0x005c8094 */ /* never referenced */
int g_47_nUnusedMainValue2[2]; /* 0x005c8098 */ /* never referenced */
int g_129_nUnusedMainNumber2[2]; /* 0x005c80a0 */ /* never referenced */
