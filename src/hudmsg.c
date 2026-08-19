/*
 *  On-screen message banners and the debug cheat keys.
 *
 *  Address range 0x428000-0x42afff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: ShowOnScreenMessage and its six callers; string band 0x46A24C-0x46A378.
 *  The Mac CODE 4 `targ` symbols prove the nested 0x42A8F0-0x42ACFF unit.
 *  The Mac CODE 15 `select` symbols prove the nested 0x42AD00-0x42AF9F unit.
 */
#include "wc1.h"

static unsigned char g_abPauseInputState_005c85d0[0x100];
static unsigned char g_abPreviousPauseInputState_005c86e0[0x100];

/* Function start: 0x4672C5 */
/* Message dwell time: grows with text length, scaled by the speed setting. */
short MeasureMessageWidth(const char *text)
{
    return (short)((MinShort(5, (short)(DosStrlen(text) >> 1)) + 5) *
                   ((char)g_cMessageSpeed_0049b778 + 1));
}

/* Function start: 0x467300 */
void AcknowledgeModalMessage(void)
{
    ConfigureInputPump(1, PollJoystickButtonEvents);
    SetFrameTimerAndWait(20);
    FlushPendingInputEvents();
    _getch();
    ConfigureInputPump(1, get_player_input);
}

/* Function start: 0x437C2E */
void ShowModalMessage(const char *format, ...)
{
    va_list arguments;
    char text[52];

    va_start(arguments, format);
    vsprintf(text, format, arguments);
    va_end(arguments);
    if (ShowModalTextPanel(1, text) != 0) {
        AcknowledgeModalMessage();
        ReleaseModalTextPanel();
        return;
    }
    SystemDebugPrintf(text);
    AcknowledgeModalMessage();
}

/* Function start: 0x437C96 */
void ReportOutOfMemoryAndExit(const char *resource)
{
    ShowModalMessage("ERROR: Out of memory for %s", resource);
    FatalErrorAndExit("You do not have enough memory to run Wing Commander.");
}

/* Function start: 0x437DFA */
void ShowOnScreenMessage(short duration, const char *format, ...)
{
    char text[52];
    short modalShown;
    va_list arguments;

    va_start(arguments, format);
    vsprintf(text, format, arguments);
    va_end(arguments);
    modalShown = 0;
    if (duration == 9999)
        modalShown = (short)ShowModalTextPanel(1, text);
    if (modalShown == 0) {
        if (duration == 0)
            duration = MeasureMessageWidth(text);
        ClearHudMessageDisplay(1);
        DosStrcpy(g_szOnScreenMessageBuffer_005d1890, text);
        SetHudMessageText(g_szOnScreenMessageBuffer_005d1890,
                          g_abGamePaletteReservedColours_0049cb54[8], duration);
    }
    if (g_bPauseInputActive_0049ac9c != 0) {
        FlushPendingInputPresses();
        FlushInputEvents();
        ClearDebugPauseFlags();
    }
    if (duration == 9999) {
        while (WaitForInputKey() == 0)
            ServiceSoundSystem();
    }
    if (modalShown == 0) {
        if (duration == 9999)
            SetHudMessageText("", g_abGamePaletteReservedColours_0049cb54[8], 2);
    } else {
        ReleaseModalTextPanel();
    }
}

/* Function start: 0x437F2F */
void ShowGamePausedBanner(short showBanner)
{
    if (showBanner != 0) {
        ShowOnScreenMessage(9999, "GAME PAUSED");
    } else {
        while (WaitForInputKey() == 0)
            ServiceSoundSystem();
    }
}

/* Function start: 0x437F77 */
void ShowVersionBanner(void)
{
    ShowOnScreenMessage(9999, "WING COMMANDER VER. %s",
                        g_pszGameVersion_0049b528);
}

/* Function start: 0x437F9A */
void SetMessageDisplaySpeed(void)
{
    g_cMessageSpeed_0049b778 =
        (unsigned char)(((signed char)g_cMessageSpeed_0049b778 + 1) % 5);
    ShowOnScreenMessage(0, "MESSAGES SPEED IS NOW %d.",
                        (signed char)g_cMessageSpeed_0049b778 + 1);
}

/* Function start: 0x437FD3 */
void ReportFramesSkipped(short adjustment)
{
    g_nFrameSkip_0049d764 = MinShort(
        MaxShort((short)(g_nFrameSkip_0049d764 + adjustment), 1), 5);
    ShowOnScreenMessage(0, "%d FRAMES SKIPPED.",
                        g_nFrameSkip_0049d764 - 1);
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
                        g_abGamePaletteReservedColours_0049cb54[8], 20);
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
                                      g_abGamePaletteReservedColours_0049cb54[8], 20);
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
        if (get_mode(1) == 4)
            CloseCommChoiceMenu();
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
        if (notRepeated && control != 0)
            CalibrateJoystickInteractive();
        break;
    case 0x19:
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
    short navPointIndex = 0;
    MissionNavPoint *navPoint = g_aMissionNavPoints_00491e98;

    for (; navPointIndex < 10; navPointIndex++, navPoint++) {
        if (navPoint->type == 1 &&
            navPoint->systemIndex == g_nCurrentStarSystem_005d169c &&
            GetShipDistanceToNavPoint(ship, navPoint) <
                navPoint->proximityRadius)
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
    return 0;
}

/* Function start: 0x4379F0 */
void InitializeSpaceFlightInput(void)
{
    ClearInputPump();
    InitializeInputDriverHook();
    g_bInputCursorEnabled_005c80e6 = 1;
    g_nInputRepeatDelay_005c80d6 = g_bInputCursorEnabled_005c80e6;
    g_nFlightInputCommand_005d1798 = 0;
    ConfigureInputPump(1, get_player_input);
    SetInputViewport(&g_stViewBuffer_005d2b00);
    g_nUiInputMode_005c8d3c = 0;
}

/* Function start: 0x42D4C1 */
void ReleaseSceneMusicPacket(void)
{
    if (g_pSceneMusicPacket_00499c08 != 0) {
        StopMusic(0);
        g_dwRestoredSceneMusicState_005d14e8 =
            g_dwSceneMusicState_005d2dc4;
        ReleasePacketSlot(&g_pSceneMusicPacket_00499c08);
    }
}

/* Function start: 0x446950 */
void BeginInflightReplayHook(void)
{
}

/* Function start: 0x446960 */
void ResetCannedScenePlaybackBuffer(void)
{
    g_nCannedSceneWriteIndex_005d3fa8 = 0;
    if (g_nCannedSceneRecordedFrameCount_005d3faa == 0)
        new_view(5, 0);
}

/* Function start: 0x446FC8 */
void ProcessCannedSceneInput(void)
{
    InputEventState event;
    short changed;
    short eventType;

    g_cPreviousKey_0049312c = g_cCurrentKey_00493128;
    ServiceInputDevices(15);
    eventType = GetNextInputEvent(&event);
    if (eventType == 4) {
        g_cCurrentKey_00493128 = (signed char)event.status;
        if (g_cPreviousKey_0049312c !=
            g_cCurrentKey_00493128)
            changed = 1;
        else
            changed = 0;
        switch (g_cCurrentKey_00493128) {
        case 0x33:
        case 0x34:
            break;
        case 0x3b:
            new_view(0, 0);
            break;
        case 0x3c:
            new_view(2, 0);
            break;
        case 0x3d:
            new_view(1, 0);
            break;
        case 0x3e:
            new_view(3, 0);
            break;
        case 0x3f:
            new_view(5, 0);
            break;
        case 0x40:
            new_view(15, 0);
            break;
        case 0x41:
            if (g_acShipTarget_00495f20[0] != -1)
                new_view(8, 0);
            break;
        case 0x43:
            if (changed != 0) {
                SelectNextExternalViewObject();
                force_view(5, (short)g_cViewObject_0049313c);
            }
            break;
        }
    }
}

#pragma function(memcpy)
/* Function start: 0x44698F */
void FinishCannedScenePlayback(void)
{
    if (g_bHighMemoryBuffersReady_005d2ad8 == 0)
        return;
    g_pCannedSceneStateBlock_005d3fb0 =
        (void *)IdentityDword((Wc2DwordPtr)g_pHighMemoryBlockB_00490200);
#ifndef WC1_SDL
    memcpy(&g_dwCannedSceneSnapshotStart_00493130,
           g_pCannedSceneStateBlock_005d3fb0,
           WC2_CANNED_SCENE_SNAPSHOT_BYTES);
#endif
    if (g_nArcadeState_0049d75c != 4) {
        force_view(0, 0);
        SetHudMessageText(g_szEndInflightReplay_0049b738,
                          (unsigned short)g_abGamePaletteReservedColours_0049cb54[8],
                          20);
    }
    _unlink(g_szCannedSceneTapeFile_00490208);
}
#pragma intrinsic(memcpy)

/* Function start: 0x446A1D */
unsigned short RestoreCannedSceneObjectState(
    CannedSceneObjectStateRecord *record)
{
    short obj;
    int mode;

    obj = record->object;
    g_aShipVelocity_00494898[obj] = record->velocity;
    g_aShipUpVector_00493ec0[obj] = record->up;
    g_aShipForwardVector_00494208[obj] = record->forward;
    mode = record->mode;
    switch (mode) {
    case 0:
        g_aeSpecialManeuver_00495600[obj] = 0;
        break;
    case 1:
        g_aeSpecialManeuver_00495600[obj] = 1;
        break;
    case 2:
        g_asShipManeuver_00495f48[obj] = 1;
        break;
    case 3:
        g_asShipManeuver_00495f48[obj] = 0;
        break;
    case 7:
        BeginShipCloak(obj);
        break;
    case 8:
        ResetShipCloakState(obj);
        break;
    }
    return 0x2f;
}

/* Function start: 0x446B6A */
unsigned short ApplyCannedSceneObjectEventRecord(
    CannedSceneObjectEventRecord *record)
{
    short obj;
    short objectType;
    signed char owner;
    int event;

    obj = record->object;
    objectType = record->objectType;
    owner = record->owner;
    event = record->event;
    switch (event) {
    case 0:
        if (objectType == WC2_OBJECT_TYPE_STAR) {
            ClearViewport(&g_stViewBuffer_005d2b00,
                          g_bPrimaryViewBufferColour_0049cb50);
            g_bViewportDirty_0049d76c++;
        }
        set_objects_data(obj, objectType, owner, 0);
        g_aShipVelocity_00494898[obj] = record->velocity;
        g_aShipPosition_00494550[obj] = record->position;
        g_asObjectCounter_00494be0[obj] = record->counter;
        g_asObjectScale_00494d90[obj] = record->scale;
        break;
    case 1:
        remove_object(obj);
        break;
    case 2:
        break;
    case 3:
        approach(obj);
        break;
    case 4:
        auto_pilot_sequence();
        break;
    }
    return 0x2a;
}

/* Function start: 0x446CC2 */
unsigned short ApplyCannedSceneSoundRecord(
    CannedSceneBriefingCharacterRecord *record)
{
    signed char sound;
    short sourceObject;
    short animationFrame;

    sound = record->character;
    sourceObject = record->pose;
    animationFrame = record->animationFrame;
    PlaySfxWaveFileByNumber(sound, sourceObject, 0);
    return 0xa;
}

/* Function start: 0x446D0D */
unsigned short ApplyCannedSceneMusicRecord(
    CannedSceneMusicCommandRecord *record)
{
    int track;
    int command;
    short enabled;

    track = record->track;
    command = record->command;
    enabled = record->enabled;
    ProcessMusicScriptCommand(track, command, enabled);
    return 0x10;
}

/* Function start: 0x446D55 */
void ApplyCannedSceneFrameEvents(void)
{
    signed char done;
    CannedSceneRecordHeader *record;
    unsigned int opcode;

    done = 0;
    if (g_bHighMemoryBuffersReady_005d2ad8 != 0) {
        g_dwHighMemoryParagraph_005d3fb4 =
            IdentityDword((Wc2DwordPtr)g_pHighMemoryBlockA_004901f8);
        while (done == 0) {
            record = (CannedSceneRecordHeader *)(
                (unsigned int)(unsigned short)
                    g_nCannedSceneWriteIndex_005d3fa8 +
                g_dwHighMemoryParagraph_005d3fb4);
            if (record->frame == g_nSpaceFrame_00493134) {
                opcode = record->opcode;
                switch (opcode) {
                case 0:
                    g_nCannedSceneWriteIndex_005d3fa8 = (short)(
                        (unsigned short)g_nCannedSceneWriteIndex_005d3fa8 +
                        RestoreCannedSceneObjectState(
                            (CannedSceneObjectStateRecord *)record));
                    break;
                case 1:
                    g_nCannedSceneWriteIndex_005d3fa8 = (short)(
                        (unsigned short)g_nCannedSceneWriteIndex_005d3fa8 +
                        ApplyCannedSceneSoundRecord(
                            (CannedSceneBriefingCharacterRecord *)record));
                    break;
                case 2:
                    g_nCannedSceneWriteIndex_005d3fa8 = (short)(
                        (unsigned short)g_nCannedSceneWriteIndex_005d3fa8 +
                        ApplyCannedSceneMusicRecord(
                            (CannedSceneMusicCommandRecord *)record));
                    break;
                case 3:
                    g_nCannedSceneWriteIndex_005d3fa8 = (short)(
                        (unsigned short)g_nCannedSceneWriteIndex_005d3fa8 +
                        ApplyCannedSceneObjectEventRecord(
                            (CannedSceneObjectEventRecord *)record));
                    break;
                }
            } else {
                done++;
            }
        }
    }
}

/* Function start: 0x446F14 */
void LoadCannedScenePlaybackBuffer(void)
{
    short file;

    file = OpenDataFileOrDie(g_szCannedSceneTapeFile_00490208);
    if (file == -1)
        ReportFatalErrorCode(g_szCannedSceneReadOpenError_0049b750);
    ReadDataFileAtOffset((unsigned short)file,
                         g_nCannedSceneFileOffset_005d3fac,
                         2, &g_nCannedSceneSegmentEndFrame_00490218);
    g_nCannedSceneFileOffset_005d3fac += 2;
    ReadDataFileAtOffset((unsigned short)file,
                         g_nCannedSceneFileOffset_005d3fac,
                         2, &g_nCannedSceneWriteIndex_005d3fa8);
    g_nCannedSceneFileOffset_005d3fac += 2;
    ReadDataFileAtOffset(
        (unsigned short)file, g_nCannedSceneFileOffset_005d3fac,
        (unsigned int)(unsigned short)g_nCannedSceneWriteIndex_005d3fa8,
        g_pHighMemoryBlockA_004901f8);
    g_nCannedSceneFileOffset_005d3fac +=
        (unsigned int)(unsigned short)g_nCannedSceneWriteIndex_005d3fa8;
    CloseDataFile((unsigned short)file);
}

/* Function start: 0x40230E */
void WriteTapeInitialState(void)
{
    short file;

    file = CreateDataFile(g_szCannedSceneTapeFile_00490208);
    if (file == -1)
        ReportFatalErrorCode(g_szCannedSceneCreateError_00490270);
#ifndef WC1_SDL
    WriteDataFileAtOffset((unsigned short)file,
                          g_nCannedSceneFileOffset_005d3fac,
                          WC2_CANNED_SCENE_SNAPSHOT_BYTES,
                          &g_dwCannedSceneSnapshotStart_00493130);
#endif
    g_nCannedSceneFileOffset_005d3fac += WC2_CANNED_SCENE_SNAPSHOT_BYTES;
    CloseDataFile((unsigned short)file);
}

/* Function start: 0x402385 */
void FlushCannedSceneRecordingBuffer(void)
{
    short file;

    file = OpenDataFileOrDie(g_szCannedSceneTapeFile_00490208);
    if (file == -1)
        ReportFatalErrorCode(g_szCannedSceneWriteOpenError_00490274);
    WriteDataFileAtOffset((unsigned short)file,
                          g_nCannedSceneFileOffset_005d3fac,
                          2, &g_nSpaceFrame_00493134);
    g_nCannedSceneFileOffset_005d3fac += 2;
    WriteDataFileAtOffset((unsigned short)file,
                          g_nCannedSceneFileOffset_005d3fac,
                          2, &g_nCannedSceneWriteIndex_005d3fa8);
    g_nCannedSceneFileOffset_005d3fac += 2;
    WriteDataFileAtOffset(
        (unsigned short)file, g_nCannedSceneFileOffset_005d3fac,
        (unsigned int)(unsigned short)g_nCannedSceneWriteIndex_005d3fa8,
        g_pHighMemoryBlockA_004901f8);
    g_nCannedSceneFileOffset_005d3fac +=
        (unsigned int)(unsigned short)g_nCannedSceneWriteIndex_005d3fa8;
    CloseDataFile((unsigned short)file);
}

/* Function start: 0x401A62 */
void InitializeCannedSceneFrameIndex(void)
{
    CannedSceneBufferHeader *header;
    short index;

    for (index = 0; index < 70; index++)
        g_asCannedSceneFrameOffsets_005d3fc0[index] = -1;
    g_nCannedSceneWriteIndex_005d3fa8 = 0;
    header = (CannedSceneBufferHeader *)IdentityDword(
        (Wc2DwordPtr)g_pHighMemoryBlockA_004901f8);
    header->nextFrame = -1;
    header->byteCount = 0x29a;
}

/* Function start: 0x401ACB */
void HandleCannedSceneBufferBoundary(void)
{
    FixedVector savedUp;
    FixedVector savedForward;
    FixedVector savedPosition;
    int savedView;
    FixedVector savedVelocity;

    if (g_bHighMemoryBuffersReady_005d2ad8 != 0) {
        if (g_nCannedSceneMode_0049021c != 0) {
            savedPosition = g_aShipPosition_00494550[WC2_EYE_OBJECT];
            savedVelocity = g_aShipVelocity_00494898[WC2_EYE_OBJECT];
            savedUp = g_aShipUpVector_00493ec0[WC2_EYE_OBJECT];
            savedForward = g_aShipForwardVector_00494208[WC2_EYE_OBJECT];
            savedView = g_nCurrentView_00492fa8;
            LoadCannedScenePlaybackBuffer();
            ResetCannedScenePlaybackBuffer();
            g_aShipPosition_00494550[WC2_EYE_OBJECT] = savedPosition;
            g_aShipVelocity_00494898[WC2_EYE_OBJECT] = savedVelocity;
            g_aShipUpVector_00493ec0[WC2_EYE_OBJECT] = savedUp;
            g_aShipForwardVector_00494208[WC2_EYE_OBJECT] = savedForward;
            g_nCurrentView_00492fa8 = savedView;
        } else {
            FlushCannedSceneRecordingBuffer();
            InitializeCannedSceneFrameIndex();
        }
        g_nCannedSceneBufferNearCapacityFlag_00490214 = 0;
    }
}

/* Function start: 0x401C1A */
void CheckCannedSceneBufferCapacity(void)
{
    if ((int)(unsigned short)g_wHighMemoryBlockBytes_004901fc - 1000 <=
            (int)(unsigned short)g_nCannedSceneWriteIndex_005d3fa8) {
        if ((int)(unsigned short)g_wHighMemoryBlockBytes_004901fc <=
                (int)(unsigned short)g_nCannedSceneWriteIndex_005d3fa8) {
            ReportFatalErrorCode(g_abCannedSceneBufferOverflowCode_00490224);
        }
        g_nCannedSceneBufferNearCapacityFlag_00490214 = 1;
    }
}

/* Function start: 0x401C72 */
int GetCannedSceneObjectMode(short obj)
{
    if (g_aeObjectClass_00495328[obj] < OBJECT_CLASS_SHIP)
        return 0;
    if (g_anShipCloakState_00496020[obj] == 1 &&
        g_asShipCloakElapsedFrames_00496060[obj] == 0)
        return 7;
    if (g_anShipCloakState_00496020[obj] == 0 &&
        g_asShipCloakCooldown_00496048[obj] == 40)
        return 8;
    if (g_asShipTactic_00495f30[obj] == TACTIC_WARP_OUT)
        return 2;
    if (g_asShipTactic_00495f30[obj] == TACTIC_WARP_IN)
        return 3;
    switch (g_aeSpecialManeuver_00495600[obj]) {
    case 1:
        return 1;
    default:
        return 0;
    }
}

/* Function start: 0x401D79 */
void RecordCannedSceneObjectState(short obj)
{
    FixedVector previousForward;
    FixedVector previousVelocity;
    int mode;
    CannedSceneObjectStateRecord *record;
    FixedVector previousUp;

    if (g_bHighMemoryBuffersReady_005d2ad8 != 0 &&
        g_nCannedSceneMode_0049021c == 0) {
        g_dwHighMemoryParagraph_005d3fb4 =
            IdentityDword((Wc2DwordPtr)g_pHighMemoryBlockA_004901f8);
        mode = GetCannedSceneObjectMode(obj);
        if ((unsigned short)g_asCannedSceneFrameOffsets_005d3fc0[obj] !=
            0xffff) {
            record = (CannedSceneObjectStateRecord *)(
                (unsigned int)(unsigned short)
                    g_asCannedSceneFrameOffsets_005d3fc0[obj] +
                g_dwHighMemoryParagraph_005d3fb4);
            previousVelocity = record->velocity;
            previousUp = record->up;
            previousForward = record->forward;
            if (equ_vector(&g_aShipVelocity_00494898[obj],
                           &previousVelocity) != 0 &&
                equ_vector(&g_aShipUpVector_00493ec0[obj],
                           &previousUp) != 0 &&
                equ_vector(&g_aShipForwardVector_00494208[obj],
                           &previousForward) != 0 &&
                record->mode == mode)
                return;
        }
        g_nCannedSceneStateRecordMarker_00490220 = 0;
        record = (CannedSceneObjectStateRecord *)(
            (unsigned int)(unsigned short)g_nCannedSceneWriteIndex_005d3fa8 +
            g_dwHighMemoryParagraph_005d3fb4);
        g_asCannedSceneFrameOffsets_005d3fc0[obj] =
            g_nCannedSceneWriteIndex_005d3fa8;
        record->opcode = 0;
        record->frame = g_nSpaceFrame_00493134;
        record->object = (signed char)obj;
        record->velocity = g_aShipVelocity_00494898[obj];
        record->forward = g_aShipForwardVector_00494208[obj];
        record->up = g_aShipUpVector_00493ec0[obj];
        record->mode = mode;
        record->endMarker = -1;
        record->nextOffset = 0x29b;
        g_nCannedSceneWriteIndex_005d3fa8 = (short)(
            (unsigned short)g_nCannedSceneWriteIndex_005d3fa8 + 0x2f);
        CheckCannedSceneBufferCapacity();
    }
}

/* Function start: 0x401FDD */
void RecordCannedSceneObjectEvent(short obj, int event)
{
    CannedSceneObjectEventRecord *record;

    if (g_bHighMemoryBuffersReady_005d2ad8 != 0) {
        if (g_nCannedSceneMode_0049021c == 0) {
            g_dwHighMemoryParagraph_005d3fb4 =
                IdentityDword((Wc2DwordPtr)g_pHighMemoryBlockA_004901f8);
            record = (CannedSceneObjectEventRecord *)(
                (unsigned int)(unsigned short)
                    g_nCannedSceneWriteIndex_005d3fa8 +
                g_dwHighMemoryParagraph_005d3fb4);
            record->opcode = 3;
            record->event = event;
            record->frame = g_nSpaceFrame_00493134;
            record->object = (signed char)obj;
            switch (event) {
            case 0:
                record->position = g_aShipPosition_00494550[obj];
                record->velocity = g_aShipVelocity_00494898[obj];
                record->objectType = g_asObjectType_00495298[obj];
                record->owner = g_acObjectOwner_00495208[obj];
                record->counter = g_asObjectCounter_00494be0[obj];
                record->scale = g_asObjectScale_00494d90[obj];
                record->endMarker = -1;
                record->nextOffset = 0x29c;
                break;
            case 1:
                record->endMarker = -1;
                record->nextOffset = 0x29f;
                break;
            case 3:
                record->endMarker = -1;
                record->nextOffset = 0x2a0;
                break;
            }
            g_nCannedSceneWriteIndex_005d3fa8 = (short)(
                (unsigned short)g_nCannedSceneWriteIndex_005d3fa8 + 0x2a);
            CheckCannedSceneBufferCapacity();
        }
    }
}

/* Function start: 0x401A10 */
void ResetCannedSceneRecording(void)
{
    if (g_bHighMemoryBuffersReady_005d2ad8 != 0) {
        _unlink(g_szCannedSceneTapeFile_00490208);
        g_nCannedSceneFrameCount_005d404c = 0;
        g_nCannedSceneFileOffset_005d3fac = 0;
        g_nCannedSceneMode_0049021c = 0;
        WriteTapeInitialState();
        InitializeCannedSceneFrameIndex();
    }
}

/* Function start: 0x46925E */
int RunSpaceFlight(short entryNavPoint)
{
    InputPumpContext *savedInputContext;
    unsigned char savedInputDelay;
    short savedUiInputMode;
    short inputResult;
    short index;

    savedUiInputMode = g_nUiInputMode_005c8d3c;
    g_bSpaceFlightActive_005c586c = 1;
    g_nCockpitDisplayMode_0049d71c = 0;
    if (g_bFullScreenSpaceView_0049d718 == 0)
        g_nCockpitDisplayMode_0049d71c = 1;
    for (index = 0; index < 7; index++)
        g_acMissionShipStatusFlags_005d2fb0[index] = 0;
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
        initialize_cockpit(g_cScreenViewportMode_005c82a6++);
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

    while (g_nArcadeState_0049d75c == 0) {
        SetFrameTimerPeriodDirect(g_nSpaceFramePeriod_0049d768);
        if (g_nCannedSceneMode_0049021c != 0) {
            ProcessCannedSceneInput();
            inputResult = 0;
            if (g_nCannedSceneRecordedFrameCount_005d3faa ==
                g_nCannedSceneFrameCount_005d404c) {
                FinishCannedScenePlayback();
                ResetCannedSceneRecording();
            }
        }
        if (g_nCannedSceneMode_0049021c == 0 &&
            g_bJumpSequenceActive_004962f0 == 0)
            inputResult = (short)HandleSpaceFlightControls();
        if (inputResult == -1) {
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
    if (damage < 40)
        return 1;
    if (damage < 70)
        return 2;
    return 3;
}

/* Function start: 0x419A40 */
void CopyHugeMemoryOverlapSafe(void *destination, void *source, int count)
{
    if (count != 0) {
        if (DosFarPtrToNear(source) < DosFarPtrToNear(destination)) {
            destination = DosNearPtrToFar(
                DosFarPtrToNear(destination) + count);
            source = DosNearPtrToFar(DosFarPtrToNear(source) + count);
            while ((count -= 0xffff) > -1) {
                source = DosNearPtrToFar(
                    DosFarPtrToNear(source) - 0xffff);
                destination = DosNearPtrToFar(
                    DosFarPtrToNear(destination) - 0xffff);
                DosMemcpy(destination, source, 0xffff);
            }
            count += 0xffff;
            source = DosNearPtrToFar(DosFarPtrToNear(source) - count);
            destination = DosNearPtrToFar(
                DosFarPtrToNear(destination) - count);
            DosMemcpy(destination, source, (unsigned int)count);
        } else {
            while ((count -= 0xffff) > -1) {
                DosMemcpy(destination, source, 0xffff);
                source = DosNearPtrToFar(
                    DosFarPtrToNear(source) + 0xffff);
                destination = DosNearPtrToFar(
                    DosFarPtrToNear(destination) + 0xffff);
            }
            count += 0xffff;
            DosMemcpy(destination, source, (unsigned int)count);
        }
    }
}

#pragma function(memset)

/* Function start: 0x453820 */
void FadeViewportPaletteToColour(Viewport *viewport,
                                 unsigned short colour,
                                 short enabled)
{
    unsigned char *indices;
    short activeCount;
    short index;
    unsigned short target[3];
    short *targetPalette;
    short *currentPalette;

    if (g_nSpacePaletteFadeMode_004901e8 == 0x13) {
        indices = AllocateTaggedMemory(256, 0);
        if (indices == 0)
            return;
        memset(indices, 0, 256);
        activeCount = CollectActivePaletteIndices(viewport, indices, 256);
        currentPalette = AllocateTaggedMemory(
            (unsigned int)(activeCount * 6), 0);
        if (currentPalette == 0) {
            ReleasePacketHandle(indices);
            return;
        }
        memset(currentPalette, 0,
               (unsigned int)(activeCount * 6));
        targetPalette = AllocateTaggedMemory(
            (unsigned int)(activeCount * 6), 0);
        if (targetPalette == 0) {
            ReleasePacketHandle(indices);
            ReleasePacketHandle(currentPalette);
            return;
        }
        memset(targetPalette, 0,
               (unsigned int)(activeCount * 6));
        GetPaletteEntry((short)colour, target);
        for (index = 0; index < activeCount; index++) {
            GetPaletteEntry(
                (short)indices[index],
                (unsigned short *)&currentPalette[index * 3]);
            memcpy(&targetPalette[index * 3], target, 6);
        }
        while (StepPaletteTransition(
                   currentPalette, targetPalette,
                   (short)(activeCount * 3)) != 0) {
            if (enabled != 0) {
                index = enabled;
                while (index-- != 0)
                    WaitForVerticalBlankThunk();
            }
            for (index = 0; index < activeCount; index++) {
                SetPaletteEntry((short)indices[index],
                                &currentPalette[index * 3]);
            }
        }
        ReleasePacketHandle(targetPalette);
        ReleasePacketHandle(currentPalette);
        ReleasePacketHandle(indices);
    }
}

#pragma intrinsic(memset)

/* Function start: 0x424A60 */
short find_objective(int type, short index)
{
    short objective;

    objective = 0;
    while (objective < (short)g_cMissionObjectiveCount_00493294) {
        if (g_aMissionObjectives_004932a8[objective].type == type) {
            if (index == -1)
                return objective;
            if (g_aMissionObjectives_004932a8[objective].index == index)
                return objective;
        }
        objective++;
    }
    return -1;
}

/* Function start: 0x424AEE */
void arrive_from_warp(short obj)
{
    short objective = find_objective(0, g_nCurrentNavPoint_004931bc);

    if (objective != -1) {
        if (g_aMissionObjectives_004932a8[
                g_abFlightPath_004932a0[objective]].type != 1)
            flag_objective(objective, 1);
        if (g_cCurrentObjective_004931cc == objective)
            set_next_destination();
    }
    place_ship_near_player_until_valid(obj, 2000, 5000);
    unwarp(obj);
    g_anShipSpeed_00494e20[obj] =
        (int)g_asShipMaximumVelocity_00495f70[obj] << 8;
    fix_velocity(obj);
    if (g_asShipSide_004955d0[obj] == SIDE_IMPERIAL)
        reset_mission_type(obj, MISSION_TYPE_COME_HOME);
    else
        reset_mission_type(obj, MISSION_TYPE_PATROL);
}

/* Function start: 0x424C05 */
void unwarp(short obj)
{
    short effect;

    ClearViewport(&g_stViewBuffer_005d2b00,
                  g_bPrimaryViewBufferColour_0049cb50);
    g_bViewportDirty_0049d76c = 1;
    PlaySfxWaveFileByNumber(0x34, obj, 0);
    effect = find_vacant_3d_object();
    if (effect != -1) {
        set_objects_data(effect, OBJECT_TYPE_HYPERSPACE_JUMP_FLASH,
                         obj, 0);
        g_aShipPosition_00494550[effect] = g_aShipPosition_00494550[obj];
        g_aShipVelocity_00494898[effect] = g_aShipVelocity_00494898[obj];
        g_asShipManeuver_00495f48[obj] = MANEUVER_NONE;
        g_asObjectCounter_00494be0[obj] = 6;
        RecordCannedSceneObjectEvent(effect, 0);
    } else {
        g_abShipNavPointIndex_00495f60[obj] = (signed char)
            g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[obj]].resourceType;
        set_objects_data(obj, OBJECT_TYPE_HYPERSPACE_JUMP_FLASH,
                         obj, 0);
        RecordCannedSceneObjectEvent(obj, 0);
    }
    if (obj == 0)
        g_asShipManeuver_00495f48[0] = MANEUVER_WARPING_IN;
}

/* Function start: 0x424D4D */
void warp(short obj)
{
    short effect;

    ClearViewport(&g_stViewBuffer_005d2b00,
                  g_bPrimaryViewBufferColour_0049cb50);
    g_bViewportDirty_0049d76c = 1;
    PlaySfxWaveFileByNumber(0x34, obj, 0);
    effect = find_vacant_3d_object();
    if (effect != -1) {
        set_objects_data(effect, OBJECT_TYPE_HYPERSPACE_JUMP_FLASH,
                         (short)g_acObjectOwner_00495208[obj], 0);
        g_aShipPosition_00494550[effect] = g_aShipPosition_00494550[obj];
        g_aShipVelocity_00494898[effect] = g_aShipVelocity_00494898[obj];
        g_asShipManeuver_00495f48[obj] = MANEUVER_WARPING_OUT;
        g_asObjectCounter_00494be0[obj] = 6;
        RecordCannedSceneObjectEvent(effect, 0);
    } else {
        set_objects_data(obj, OBJECT_TYPE_HYPERSPACE_JUMP_FLASH,
                         (short)g_acObjectOwner_00495208[obj], 0);
        RecordCannedSceneObjectEvent(obj, 0);
    }
    if (g_asShipSide_004955d0[obj] == SIDE_KILRATHI)
        g_nEscapedEnemyCount_004962e8++;
}

/* Function start: 0x461480 */
short find_next_gun(short obj, short currentGun)
{
    unsigned char *loadout;
    short foundCurrent;
    short firstGun;
    short weaponCount;
    ShipWeaponSlot *weaponSlot;
    short weapon;

    loadout = g_aShipWeapons_004956b0[obj];
    foundCurrent = 0;
    firstGun = -1;
    weaponCount = (signed char)loadout[0];
    weaponSlot = (ShipWeaponSlot *)&loadout[1];
    for (weapon = 0; weapon < weaponCount; weapon++, weaponSlot++) {
        if (g_aObjectTypeData_00496d30[weaponSlot->type].objectClass ==
                OBJECT_CLASS_PROJECTILE) {
            if (weaponSlot->type != WC2_OBJECT_TYPE_TURRET_GUN) {
                if (firstGun == -1)
                    firstGun = weaponSlot->type;
                if (foundCurrent == 0) {
                    if (weaponSlot->type == currentGun)
                        foundCurrent = 1;
                } else if (weaponSlot->type != currentGun) {
                    return weaponSlot->type;
                }
            }
        }
    }
    if (foundCurrent != 0 && firstGun != currentGun)
        return 0x80;
    return firstGun;
}

/* Function start: 0x4615A2 */
int select_guns(short selectedGun)
{
    unsigned char *loadout;
    short found;
    short weaponCount;
    short weapon;
    ShipWeaponSlot *weaponSlot;

    loadout = g_aShipWeapons_004956b0[0];
    found = 0;
    weaponCount = (signed char)loadout[0];
    weapon = 0;
    weaponSlot = (ShipWeaponSlot *)&loadout[1];
    for (; weapon < weaponCount; weapon++, weaponSlot++) {
        if (g_aObjectTypeData_00496d30[weaponSlot->type].objectClass ==
                OBJECT_CLASS_PROJECTILE) {
            if (weaponSlot->type == selectedGun || selectedGun == 0x80) {
                weaponSlot->disabled = 0;
                found = 1;
            } else {
                weaponSlot->disabled = 1;
            }
        }
    }
    if (found != 0)
        return selectedGun;
    return -1;
}

/* Function start: 0x46166D */
void select_new_gun(void)
{
    g_nSelectedGunType_004934dc =
        select_guns(find_next_gun(0, g_nSelectedGunType_004934dc));
    if (get_mode(0) == 1)
        InvalidateVduMode(0);
}

/* Function start: 0x4616B8 */
void select_new_release_weapon(short preferredType)
{
    signed char weaponCount;
    short firstWeapon;
    signed char weapon;
    short weaponType;

    weaponCount = g_aShipWeapons_004956b0[0][0];
    weapon = (signed char)g_nSelectedReleaseWeaponIndex_004934e0;
    weapon++;
    if (weaponCount <= weapon)
        weapon = 0;
    if (g_nSelectedReleaseWeaponIndex_004934e0 == -1) {
        if (preferredType != -1) {
            for (weapon = 0; weaponCount > weapon; weapon++) {
                if (((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[
                        weapon].type == preferredType) {
                    ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[
                        weapon].disabled = 0;
                    g_nSelectedReleaseWeaponIndex_004934e0 = weapon;
                    break;
                }
            }
        }
        if (g_nSelectedReleaseWeaponIndex_004934e0 == -1) {
            for (weapon = 0; weaponCount > weapon; weapon++) {
                weaponType =
                    ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[
                        weapon].weaponType;
                if (g_aObjectTypeData_00496d30[weaponType].objectClass !=
                    OBJECT_CLASS_PROJECTILE) {
                    ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[
                        weapon].disabled = 0;
                    g_nSelectedReleaseWeaponIndex_004934e0 = weapon;
                    break;
                }
            }
        }
    } else {
        firstWeapon = weapon;
        while (weapon != g_nSelectedReleaseWeaponIndex_004934e0) {
            weaponType =
                ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[
                    weapon].weaponType;
            if (g_aObjectTypeData_00496d30[weaponType].objectClass !=
                    OBJECT_CLASS_PROJECTILE &&
                ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[
                    weapon].type !=
                    ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[
                        g_nSelectedReleaseWeaponIndex_004934e0].type) {
                ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[
                    g_nSelectedReleaseWeaponIndex_004934e0].disabled = 1;
                ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[
                    weapon].disabled = 0;
                g_nSelectedReleaseWeaponIndex_004934e0 = weapon;
                break;
            }
            weapon++;
            if (weaponCount <= weapon)
                weapon = 0;
            if (weapon == firstWeapon)
                break;
        }
    }
    if (get_mode(0) == 1)
        InvalidateVduMode(0);
}

/* Function start: 0x40A2D0 */
void CALLBACK FrameTimerCallback(UINT timerId, UINT message,
                                 DWORD user, DWORD first, DWORD second)
{
    (void)timerId;
    (void)message;
    (void)user;
    (void)first;
    (void)second;
    g_nFrameTimerPending_005c844c = 0;
}

/* Function start: 0x40A2E7 */
void SetMultimediaTimerCallback(int period)
{
    int milliseconds = period * 1000 / 60;

    if (period == 0) {
        g_nFrameTimerPending_005c844c = 0;
        if (g_uFrameTimerEventId_005d3bec != 0) {
            timeKillEvent(g_uFrameTimerEventId_005d3bec);
            g_uFrameTimerEventId_005d3bec = 0;
        }
    } else {
        if (g_uFrameTimerEventId_005d3bec != 0) {
            timeKillEvent(g_uFrameTimerEventId_005d3bec);
            g_uFrameTimerEventId_005d3bec = 0;
        }
        g_nFrameTimerPending_005c844c = 1;
        g_uFrameTimerEventId_005d3bec = timeSetEvent(milliseconds, milliseconds,
                                    FrameTimerCallback, 0, 0);
    }
}
