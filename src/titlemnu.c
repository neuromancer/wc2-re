/*
 *  Joystick calibration, title menu and barracks screen.
 *
 *  Retail object 0x418770-0x4199EF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x418770 */
void CalibrateJoystickInteractive()
{
    InputDeviceSample samples[2];
    short calibration[6];
    short savedFailureValue;
    int file;
    int failed;
    short fontLoaded;

    fontLoaded = 0;
    savedFailureValue = (short)g_nJoystickFailureValue_005d176c;
    failed = 1;
    if (g_nActiveInputDevice_005d1726 == -1)
        return;
    g_stDefaultTextContext_005d2d20.alignment = 2;
    if (g_apTextFonts_005d2200[1] != 0)
        fontLoaded++;

    InitializeTextContextFromFont(&g_stDefaultTextContext_005d2d20, 1,
        g_bPrimaryViewBufferColour_0049cb50, (signed char)g_cSecondaryViewBufferColour_0049cb4c);
    if (ShowModalTextPanel(1,
        "Turn AUTO FIRE off if present, press a button") != 0) {
        DIBslamReal();
        WaitForJoystickButtonPress();
        WaitForJoystickButtonRelease();
        ReleaseModalTextPanel();
    }

    if (ShowModalTextPanel(1,
        "Move stick to the UPPER LEFT, press a button") != 0) {
        DIBslamReal();
        WaitForJoystickButtonPress();
        SampleJoystickDevice(samples, g_nActiveInputDevice_005d1726, 0x7fff);
        g_nJoystickMinimumX_005d174c = samples[g_nActiveInputDevice_005d1726].x;
        calibration[0] = (short)g_nJoystickMinimumX_005d174c;
        g_nJoystickMinimumY_005d1750 = samples[g_nActiveInputDevice_005d1726].y;
        calibration[1] = (short)g_nJoystickMinimumY_005d1750;
        WaitForJoystickButtonRelease();
        ReleaseModalTextPanel();
    }

    if (ShowModalTextPanel(1,
        "Move stick to the LOWER RIGHT, press a button") != 0) {
        DIBslamReal();
        WaitForJoystickButtonPress();
        SampleJoystickDevice(samples, g_nActiveInputDevice_005d1726, 0x7fff);
        g_nJoystickMaximumX_005d1744 = samples[g_nActiveInputDevice_005d1726].x;
        calibration[2] = (short)g_nJoystickMaximumX_005d1744;
        g_nJoystickMaximumY_005d1748 = samples[g_nActiveInputDevice_005d1726].y;
        calibration[3] = (short)g_nJoystickMaximumY_005d1748;
        WaitForJoystickButtonRelease();
        ReleaseModalTextPanel();
    }

    if (ShowModalTextPanel(1,
        "Center Joystick, press a button") != 0) {
        DIBslamReal();
        WaitForJoystickButtonPress();
        SampleJoystickDevice(samples, g_nActiveInputDevice_005d1726, 0x7fff);
        g_nJoystickCentreX_005d1768 = samples[g_nActiveInputDevice_005d1726].x;
        calibration[4] = (short)g_nJoystickCentreX_005d1768;
        g_nJoystickCentreY_005d1764 = samples[g_nActiveInputDevice_005d1726].y;
        calibration[5] = (short)g_nJoystickCentreY_005d1764;
        WaitForJoystickButtonRelease();
        ReleaseModalTextPanel();
    }

    if (fontLoaded == 0)
        ReleaseTextFont(1);

    if (g_nJoystickHorizontalRange_005d1758 == 0)
        g_nJoystickHorizontalRange_005d1758 = 9;
    if (g_nJoystickVerticalRange_005d1754 == 0)
        g_nJoystickVerticalRange_005d1754 = 9;
    g_nJoystickLeftScale_005d1740 =
        (g_nJoystickCentreX_005d1768 - g_nJoystickMinimumX_005d174c) /
        g_nJoystickHorizontalRange_005d1758;
    g_nJoystickRightScale_005d175c =
        (g_nJoystickMaximumX_005d1744 - g_nJoystickCentreX_005d1768) /
        g_nJoystickHorizontalRange_005d1758;
    g_nJoystickUpScale_005d173c =
        (g_nJoystickCentreY_005d1764 - g_nJoystickMinimumY_005d1750) /
        g_nJoystickVerticalRange_005d1754;
    g_nJoystickDownScale_005d1760 =
        (g_nJoystickMaximumY_005d1748 - g_nJoystickCentreY_005d1764) /
        g_nJoystickVerticalRange_005d1754;
    if (g_nJoystickLeftScale_005d1740 == 0)
        g_nJoystickLeftScale_005d1740++;
    if (g_nJoystickRightScale_005d175c == 0)
        g_nJoystickRightScale_005d175c++;
    if (g_nJoystickUpScale_005d173c == 0)
        g_nJoystickUpScale_005d173c++;
    if (g_nJoystickDownScale_005d1760 == 0)
        g_nJoystickDownScale_005d1760++;

    g_nJoystickMinimumX_005d174c = g_nJoystickCentreX_005d1768 -
        g_nJoystickLeftScale_005d1740 *
            g_nJoystickHorizontalRange_005d1758;
    g_nJoystickMinimumY_005d1750 = g_nJoystickCentreY_005d1764 -
        g_nJoystickUpScale_005d173c * g_nJoystickVerticalRange_005d1754;
    g_nJoystickMaximumX_005d1744 = g_nJoystickCentreX_005d1768 +
        g_nJoystickRightScale_005d175c *
            g_nJoystickHorizontalRange_005d1758;
    g_nJoystickMaximumY_005d1748 = g_nJoystickCentreY_005d1764 +
        g_nJoystickDownScale_005d1760 * g_nJoystickVerticalRange_005d1754;
    g_nJoystickFailureValue_005d176c = g_nJoystickMaximumX_005d1744 * 2;

    if (g_nJoystickMaximumX_005d1744 <= g_nJoystickMinimumX_005d174c ||
        g_nJoystickMaximumY_005d1748 <= g_nJoystickMinimumY_005d1750) {
        if (ShowModalTextPanel(1,
            "FAILED! Center Joystick, press a button") != 0) {
            DIBslamReal();
            WaitForJoystickButtonPress();
            WaitForJoystickButtonRelease();
            LoadJoystickCalibrationFile(8, 8, 1, 1);
            ReleaseModalTextPanel();
        }
        _unlink("j.cal");
        return;
    }

    file = _open("j.cal", 0x8301, 0x180);
    if (file != -1) {
        failed = _write(file, &g_nActiveInputDevice_005d1726, 2) == -1;
        if (failed == 0)
            failed = _write(file, &calibration[0], 2) == -1;
        if (failed == 0)
            failed = _write(file, &calibration[1], 2) == -1;
        if (failed == 0)
            failed = _write(file, &calibration[2], 2) == -1;
        if (failed == 0)
            failed = _write(file, &calibration[3], 2) == -1;
        if (failed == 0)
            failed = _write(file, &calibration[4], 2) == -1;
        if (failed == 0)
            failed = _write(file, &calibration[5], 2) == -1;
        _close(file);
        if (failed != 0)
            _unlink("j.cal");
    }
}

/* Function start: 0x418D14 */
void WaitForJoystickButtonRelease(void)
{
    do {
        SampleJoystickDevice(
            &g_aInputDeviceSamples_005d1780[
                g_nActiveInputDevice_005d1726],
            g_nActiveInputDevice_005d1726, 0);
        ServiceSoundSystem();
    } while (g_aInputDeviceSamples_005d1780[
                 g_nActiveInputDevice_005d1726].buttons != 0);
}

/* Function start: 0x418D5F */
void WaitForJoystickButtonPress(void)
{
    do {
        SampleJoystickDevice(
            &g_aInputDeviceSamples_005d1780[
                g_nActiveInputDevice_005d1726],
            g_nActiveInputDevice_005d1726, 0);
        ServiceSoundSystem();
    } while (g_aInputDeviceSamples_005d1780[
                 g_nActiveInputDevice_005d1726].buttons == 0);
}

/* Function start: 0x418DAA */
short RunCampaignSelectionFrame(void *scene, void *field)
{
    short selection;

    selection = 0;
    while ((selection = PollSceneHotspotInput(scene, 0, 0, 0, 0)) == 0) {
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
    if (g_bSceneEscapeRequested_0049d4b0 != 0)
        selection = 0;
    return selection;
}

/* Function start: 0x418ECD */
short RunTitleMenuInputLoop(unsigned char *buttons,
                            unsigned char *logo,
                            unsigned char *background,
                            short showSecondButton)
{
    int savedKeyboardMouse;
    short choice;
    short maxSelection;

    choice = 0;
    maxSelection = 0;
    if (showSecondButton == 0)
        maxSelection = 1;
    g_bKeyboardMouseEnabled_0049be68 = 1;
    ClearDebugPauseFlags();
    ReleaseInputEventQueue();
    while ((choice = PollSceneHotspotInput(buttons, 0, 0, 0,
                                           maxSelection)) == 0) {
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
    g_bKeyboardMouseEnabled_0049be68 = savedKeyboardMouse;
    return choice;
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
    int wipeType;
    void *wipe;
    signed char wipeDone;

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
    wipeType = 8;
    g_bRoomTransitionAnimationEnabled_00499c00 = 1;
    if (g_bRoomTransitionAnimationEnabled_00499c00 != 0) {
        wipe = AllocateTaggedMemory(0xce, 0);
        if (wipe != 0) {
            InitializeViewportWipe(&g_stSecondaryViewBuffer_005d2c90,
                                   &screenBuffer, wipeType, 0x3c, 0,
                                   wipe);
            wipeDone = 0;
            while (wipeDone == 0) {
                wipeDone = AdvanceViewportWipe(wipe);
                if (WaitForInputKey() != 0 ||
                    g_bSceneEscapeRequested_0049d4b0 != 0) {
                    g_bSceneEscapeRequested_0049d4b0 = 1;
                    break;
                }
                CopyViewportContents(&screenBuffer,
                                     &g_stScreenViewport_005d21a0);
                DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                                  0, 0, background, 0);
                DrawConstellationField();
                DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                                  160, 100, logo, 0x19);
                DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                                  160, 100, logo, 0x1f);
                DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                                  160, 100, logo, 0x20);
            }
            FinishViewportWipe(wipe);
            ReleasePacketHandle(wipe);
        }
    }
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

/* Function start: 0x419831 */
short BarracksScreen(void)
{
    signed char choice;
    void *campaignScene;
    void *field;

    choice = 0;
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
        while (choice == 0) {
            choice = (signed char)RunCampaignSelectionFrame(
                campaignScene, field);
        }
        FreePacketAndClear(&g_pConstellationShape_005d2c4c, 0);
        FreePacketAndClear(&campaignScene, 0);
        FreePacketAndClear(&field, 0);
        free_viewport(&g_stSecondaryViewBuffer_005d2c90);
        return (short)(choice - 1);
    }
    FreePacketAndClear(&campaignScene, 0);
    free_viewport(&g_stSecondaryViewBuffer_005d2c90);
    return 0;
}
