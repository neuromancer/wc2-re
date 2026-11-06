/*
 *  Joystick calibration, title menu and barracks screen.
 *
 *  Retail object 0x418770-0x4199EF (link order; see docs/ORDER.md).
 */
#define TITLEMNU_C
#include "game.h"

/* Function start: 0x418770 */
void CalibrateJoystickInteractive()
{
    short yTop;
    short leftX;
    short savedFailure;
    InputDeviceSample sample[2];
    short centreX;
    short midY;
    short fontReady;
    short maxX;
    short yMax;
    int error;
    int file;

    fontReady = 0;
    savedFailure = (short)g_nControllerFailureValue_005d176c;
    error = 1;
    if (g_nActiveInputDeviceWord_005d1726 == -1)
        return;
    g_stDefaultStringContext_005d2d20.alignment = 2;
    if (g_apTextGlyphs_005d2200[1] != 0)
        fontReady++;

    InitializeTextContextFromFont(&g_stDefaultStringContext_005d2d20, 1,
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
        SampleJoystickDevice(sample, g_nActiveInputDeviceWord_005d1726, 0x7fff);
        g_nJoystickMinimumXNumber_005d174c = sample[g_nActiveInputDeviceWord_005d1726].x;
        leftX = (short)g_nJoystickMinimumXNumber_005d174c;
        g_nJsMinimumY_005d1750 = sample[g_nActiveInputDeviceWord_005d1726].y;
        yTop = (short)g_nJsMinimumY_005d1750;
        WaitForJoystickButtonRelease();
        ReleaseModalTextPanel();
    }

    if (ShowModalTextPanel(1,
        "Move stick to the LOWER RIGHT, press a button") != 0) {
        DIBslamReal();
        WaitForJoystickButtonPress();
        SampleJoystickDevice(sample, g_nActiveInputDeviceWord_005d1726, 0x7fff);
        g_nJoystickMaximumXValue_005d1744 = sample[g_nActiveInputDeviceWord_005d1726].x;
        maxX = (short)g_nJoystickMaximumXValue_005d1744;
        g_nJoystickMaximumY_005d1748 = sample[g_nActiveInputDeviceWord_005d1726].y;
        yMax = (short)g_nJoystickMaximumY_005d1748;
        WaitForJoystickButtonRelease();
        ReleaseModalTextPanel();
    }

    if (ShowModalTextPanel(1,
        "Center Joystick, press a button") != 0) {
        DIBslamReal();
        WaitForJoystickButtonPress();
        SampleJoystickDevice(sample, g_nActiveInputDeviceWord_005d1726, 0x7fff);
        g_nJoystickCentreXNumber_005d1768 = sample[g_nActiveInputDeviceWord_005d1726].x;
        centreX = (short)g_nJoystickCentreXNumber_005d1768;
        g_nJoystickCentreY_005d1764 = sample[g_nActiveInputDeviceWord_005d1726].y;
        midY = (short)g_nJoystickCentreY_005d1764;
        WaitForJoystickButtonRelease();
        ReleaseModalTextPanel();
    }

    if (fontReady == 0)
        ReleaseTextFont(1);

    if (g_nControllerHorizontalRange_005d1758 == 0)
        g_nControllerHorizontalRange_005d1758 = 9;
    if (g_nJoystickVerticalRangeNumber_005d1754 == 0)
        g_nJoystickVerticalRangeNumber_005d1754 = 9;
    g_nControllerLeftScaleValue_005d1740 =
        (g_nJoystickCentreXNumber_005d1768 - g_nJoystickMinimumXNumber_005d174c) /
        g_nControllerHorizontalRange_005d1758;
    g_nJsRightScaleWord_005d175c =
        (g_nJoystickMaximumXValue_005d1744 - g_nJoystickCentreXNumber_005d1768) /
        g_nControllerHorizontalRange_005d1758;
    g_nJoystickUpScaleWord_005d173c =
        (g_nJoystickCentreY_005d1764 - g_nJsMinimumY_005d1750) /
        g_nJoystickVerticalRangeNumber_005d1754;
    g_nJoystickDownScaleWord_005d1760 =
        (g_nJoystickMaximumY_005d1748 - g_nJoystickCentreY_005d1764) /
        g_nJoystickVerticalRangeNumber_005d1754;
    if (g_nControllerLeftScaleValue_005d1740 == 0)
        g_nControllerLeftScaleValue_005d1740++;
    if (g_nJoystickUpScaleWord_005d173c == 0)
        g_nJoystickUpScaleWord_005d173c++;
    if (g_nJsRightScaleWord_005d175c == 0)
        g_nJsRightScaleWord_005d175c++;
    if (g_nJoystickDownScaleWord_005d1760 == 0)
        g_nJoystickDownScaleWord_005d1760++;

    g_nJoystickMinimumXNumber_005d174c = g_nJoystickCentreXNumber_005d1768 -
        g_nControllerLeftScaleValue_005d1740 *
            g_nControllerHorizontalRange_005d1758;
    g_nJsMinimumY_005d1750 = g_nJoystickCentreY_005d1764 -
        g_nJoystickUpScaleWord_005d173c * g_nJoystickVerticalRangeNumber_005d1754;
    g_nJoystickMaximumXValue_005d1744 = g_nJoystickCentreXNumber_005d1768 +
        g_nJsRightScaleWord_005d175c *
            g_nControllerHorizontalRange_005d1758;
    g_nJoystickMaximumY_005d1748 = g_nJoystickCentreY_005d1764 +
        g_nJoystickDownScaleWord_005d1760 * g_nJoystickVerticalRangeNumber_005d1754;
    g_nControllerFailureValue_005d176c = g_nJoystickMaximumXValue_005d1744 * 2;

    if (g_nJoystickMaximumXValue_005d1744 <= g_nJoystickMinimumXNumber_005d174c ||
        g_nJoystickMaximumY_005d1748 <= g_nJsMinimumY_005d1750) {
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
        error = _write(file, &g_nActiveInputDeviceWord_005d1726, 2) == -1;
        if (error == 0)
            error = _write(file, &leftX, 2) == -1;
        if (error == 0)
            error = _write(file, &yTop, 2) == -1;
        if (error == 0)
            error = _write(file, &maxX, 2) == -1;
        if (error == 0)
            error = _write(file, &yMax, 2) == -1;
        if (error == 0)
            error = _write(file, &centreX, 2) == -1;
        if (error == 0)
            error = _write(file, &midY, 2) == -1;
        _close(file);
        if (error != 0)
            _unlink("j.cal");
    }
}

/* Function start: 0x418D14 */
void WaitForJoystickButtonRelease(void)
{
    do {
        SampleJoystickDevice(
            &g_aInputDeviceSamplesList_005d1780[
                g_nActiveInputDeviceWord_005d1726],
            g_nActiveInputDeviceWord_005d1726, 0);
        ServiceSoundSystem();
    } while (g_aInputDeviceSamplesList_005d1780[
                 g_nActiveInputDeviceWord_005d1726].buttons != 0);
}

/* Function start: 0x418D5F */
void WaitForJoystickButtonPress(void)
{
    do {
        SampleJoystickDevice(
            &g_aInputDeviceSamplesList_005d1780[
                g_nActiveInputDeviceWord_005d1726],
            g_nActiveInputDeviceWord_005d1726, 0);
        ServiceSoundSystem();
    } while (g_aInputDeviceSamplesList_005d1780[
                 g_nActiveInputDeviceWord_005d1726].buttons == 0);
}

/* Function start: 0x418DAA */
short RunCampaignSelectionFrame(void *scene, void *field)
{
    short selection;

    selection = 0;
    while ((selection = PollSceneHotspotInput(scene, 0, 0, 0, 0)) == 0) {
        DisableMouseCursorDrawing();
        DrawSpriteDefault(
            &g_stSecondaryViewBufferBlock_005d2c90, 0, 0, field, 0);
        DrawConstellationField();
        DrawSpriteDefault(
            &g_stSecondaryViewBufferBlock_005d2c90, 0, 0, scene, 0);
        if (g_pszPersonnelFooter_00492658 != 0) {
            g_stDefaultStringContext_005d2d20.viewport =
                &g_stSecondaryViewBufferBlock_005d2c90;
            InitializeTextContextFromFont(
                &g_stDefaultStringContext_005d2d20, 1,
                g_bPrimaryViewBufferColour_0049cb50, -1);
            SetTextCursor(
                (unsigned short)((320 -
                    MeasureTextPixelWidthClamped(
                        g_pszPersonnelFooter_00492658)) >> 1),
                180);
            DrawFormattedText("%S", g_pszPersonnelFooter_00492658);
            g_stDefaultStringContext_005d2d20.viewport =
                &g_stScrViewInfo_005d21a0;
        }
        EnableMouseCursorDrawing();
        WaitForVerticalBlankThunk();
        CopyViewportContents(&g_stSecondaryViewBufferBlock_005d2c90,
                             &g_stScrViewInfo_005d21a0);
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
        DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90, 0, 0,
                          background, 0);
        DrawConstellationField();
        DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90, 160, 100,
                          logo, 0x19);
        DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90, 160, 100,
                          logo, 0x1f);
        DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90, 160, 100,
                          logo, 0x20);
        DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90, 0, 0,
                          buttons, 0);
        if (showSecondButton != 0)
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90, 0, 0,
                              buttons, 1);
        RefreshMemoryStatusOverlay();
    }
    g_bKeyboardMouseEnabled_0049be68 = savedKeyboardMouse;
    return choice;
}

/* Function start: 0x418FFC */
short RunTitleScreen(void)
{
    unsigned char *background;
    int sfxHandle;
    unsigned char *buttons;
    unsigned char *logo;
    signed char done;
    short logoFrame;
    short canContinueCampaign;
    signed char menuChoice;
    int wipeType;
    Viewport screenCopy;
    void *wipeState;

    menuChoice = 0;
    done = 0;
    buttons = 0;
    logo = 0;
    background = 0;
    sfxHandle = 0;
    g_nInterfaceEventMode_005c8d3c = 0;
    SetMenuInputPump();
    DisableMouseCursorDrawing();
    g_nInterfaceEventMode_005c8d3c = 0;
    ClearViewport(&g_stModalSourceWindow_005d2c50,
                  g_cSecondaryViewBufferColour_0049cb4c);
    g_stSecondaryViewBufferBlock_005d2c90.left = 0;
    g_stSecondaryViewBufferBlock_005d2c90.right = 319;
    g_stSecondaryViewBufferBlock_005d2c90.top = 0;
    g_stSecondaryViewBufferBlock_005d2c90.bottom = 199;
    if (AllocateViewport(&g_stSecondaryViewBufferBlock_005d2c90,
                         g_cSecondaryViewBufferColour_0049cb4c, 0) == 0)
        ReportFatalErrorCode("031");
    screenCopy.left = 0;
    screenCopy.right = 319;
    screenCopy.top = 0;
    screenCopy.bottom = 199;
    if (AllocateViewport(&screenCopy,
                         g_cSecondaryViewBufferColour_0049cb4c, 0) == 0)
        ReportFatalErrorCode("032");
    PreloadMusicTrack(0x36);
    spacetrack(0x36, 1, 1);
    init_constellation(0);
    InitializeConstellationField(
        &g_stSecondaryViewBufferBlock_005d2c90, 0, 0x10);
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
        for (logoFrame = 0; logoFrame < 0x1a; logoFrame++) {
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                              0, 0, background, 0);
            DrawConstellationField();
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                              160, 100, logo, logoFrame);
            RefreshMemoryStatusOverlay();
            if (WaitForInputKey() != 0) {
                g_bSceneEscapeRequested_0049d4b0 = 1;
                break;
            }
            SetFrameTimerAndWait(4);
        }
    }
    if (sfxHandle != 0) {
        ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(
            sfxHandle, 0);
    }
    sfxHandle = 0;
    if (g_bSceneEscapeRequested_0049d4b0 == 0) {
        ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(0, 0);
        PlaySfxWaveFileByNumber(99, -1, 0);
    }
    if (g_bSceneEscapeRequested_0049d4b0 == 0) {
        for (logoFrame = 0; logoFrame < 10; logoFrame++) {
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                              0, 0, background, 0);
            DrawConstellationField();
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                              160, 100, logo, 0x19);
            RefreshMemoryStatusOverlay();
            if (WaitForInputKey() != 0) {
                g_bSceneEscapeRequested_0049d4b0 = 1;
                break;
            }
            SetFrameTimerAndWait(4);
        }
    }
    if (sfxHandle != 0) {
        ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(
            sfxHandle, 0);
    }
    sfxHandle = 0;
    PlaySfxWaveFileByNumber(6, -1, 0);
    if (g_bSceneEscapeRequested_0049d4b0 == 0) {
        for (logoFrame = 0x1a; logoFrame < 0x20; logoFrame++) {
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                              0, 0, background, 0);
            DrawConstellationField();
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                              160, 100, logo, 0x19);
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                              160, 100, logo, logoFrame);
            RefreshMemoryStatusOverlay();
            if (WaitForInputKey() != 0) {
                g_bSceneEscapeRequested_0049d4b0 = 1;
                break;
            }
            SetFrameTimerAndWait(4);
        }
    }
    if (sfxHandle != 0) {
        ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(
            sfxHandle, 0);
    }
    sfxHandle = 0;
    if (g_bSceneEscapeRequested_0049d4b0 == 0)
        PlaySfxWaveFileByNumber(99, -1, 0);
    if (g_bSceneEscapeRequested_0049d4b0 == 0) {
        for (logoFrame = 0; logoFrame < 10; logoFrame++) {
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                              0, 0, background, 0);
            DrawConstellationField();
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                              160, 100, logo, 0x19);
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                              160, 100, logo, 0x1f);
            RefreshMemoryStatusOverlay();
            if (WaitForInputKey() != 0) {
                g_bSceneEscapeRequested_0049d4b0 = 1;
                break;
            }
            SetFrameTimerAndWait(4);
        }
    }
    if (sfxHandle != 0) {
        ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(
            sfxHandle, 0);
    }
    sfxHandle = 0;
    if (g_bSceneEscapeRequested_0049d4b0 == 0)
        PlaySfxWaveFileByNumber(100, -1, 0);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                      0, 0, background, 0);
    DrawConstellationField();
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                      160, 100, logo, 0x19);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                      160, 100, logo, 0x1f);
    RefreshMemoryStatusOverlay();
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                      160, 100, logo, 0x20);
    CopyViewportContents(&g_stScrViewInfo_005d21a0,
                         &screenCopy);
    MarkDibDirty();
    DIBslamReal();
    wipeType = 8;
    g_bRoomTransitionAnimationEnabled_00499c00 = 1;
    if (g_bRoomTransitionAnimationEnabled_00499c00 != 0) {
        wipeState = AllocateTaggedMemory(0xce, 0);
        if (wipeState != 0) {
            InitializeViewportWipe(&g_stSecondaryViewBufferBlock_005d2c90,
                                   &screenCopy, wipeType, 0x3c, 0,
                                   wipeState);
            done = 0;
            while (done == 0) {
                done = AdvanceViewportWipe(wipeState);
                if (WaitForInputKey() != 0 ||
                    g_bSceneEscapeRequested_0049d4b0 != 0) {
                    g_bSceneEscapeRequested_0049d4b0 = 1;
                    break;
                }
                CopyViewportContents(&screenCopy,
                                     &g_stScrViewInfo_005d21a0);
                DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                                  0, 0, background, 0);
                DrawConstellationField();
                DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                                  160, 100, logo, 0x19);
                DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                                  160, 100, logo, 0x1f);
                DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                                  160, 100, logo, 0x20);
            }
            FinishViewportWipe(wipeState);
            ReleasePacketHandle(wipeState);
        }
    }
    if (sfxHandle != 0) {
        ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(
            sfxHandle, 0);
    }
    sfxHandle = 0;
    canContinueCampaign = (short)HasSavedPilotCampaign();
    SetMenuInputPump();
    g_nInterfaceEventMode_005c8d3c = 1;
    SetInputViewport(&g_stSecondaryViewBufferBlock_005d2c90);
    SetPersonnelMousePosition(159, 159);
    EnableMouseCursorDrawing();
    ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(
        sfxHandle, 0);
    g_bNewPilotCampaignInitialized_004926c0 = 0;
    free_viewport(&screenCopy);
    if (g_bSceneEscapeRequested_0049d4b0 != 0)
        g_bSceneEscapeRequested_0049d4b0 = 0;
    /* Retail calls the pump here with no argument at all (no prototype
     * was in scope); the SDL build passes the zero the other callers use. */
#ifdef SDL_PORT
    PumpWindowMessages(0);
    while (TakeInputPressCount() != 0) {
        PumpWindowMessages(0);
        FlushInputEvents();
    }
#else
    ((unsigned int (__cdecl *)(void))PumpWindowMessages)();
    while (TakeInputPressCount() != 0) {
        ((unsigned int (__cdecl *)(void))PumpWindowMessages)();
        FlushInputEvents();
    }
#endif
    FlushInputEvents();
    g_bSceneEscapeRequested_0049d4b0 = 0;
    menuChoice = 0;
    while (menuChoice == 0) {
        menuChoice = (signed char)RunTitleMenuInputLoop(
            buttons, logo, background, canContinueCampaign);
    }
    DisableMouseCursorDrawing();
    FreePacketAndClear(&buttons, 0);
    FreePacketAndClear(&logo, 0);
    FreePacketAndClear(&background, 0);
    free_viewport(&g_stSecondaryViewBufferBlock_005d2c90);
    FreePacketAndClear(&g_pConstellationSprite_005d2c4c, 0);
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
    ClearViewport(&g_stModalSourceWindow_005d2c50,
                  g_cSecondaryViewBufferColour_0049cb4c);
    g_stSecondaryViewBufferBlock_005d2c90.left = 0;
    g_stSecondaryViewBufferBlock_005d2c90.right = 319;
    g_stSecondaryViewBufferBlock_005d2c90.top = 0;
    g_stSecondaryViewBufferBlock_005d2c90.bottom = 199;
    if ((short)AllocateViewport(
            &g_stSecondaryViewBufferBlock_005d2c90,
            g_cSecondaryViewBufferColour_0049cb4c, 0) == 0) {
        ReportFatalErrorCode("033");
    }
    ClearViewport(&g_stSecondaryViewBufferBlock_005d2c90, 0);
    campaignScene = LoadNamedPacket(
        "campaign.vga", 0, 0, 0, 0, 1);
    if (campaignScene != 0 && WaitForSceneAdvance(campaignScene) != 0) {
        field = FetchDiskPacketRetrying("field.v00", 1, 0);
        init_constellation(0);
        InitializeConstellationField(
            &g_stSecondaryViewBufferBlock_005d2c90, 0, 16);
        SetMenuInputPump();
        SetInputViewport(&g_stSecondaryViewBufferBlock_005d2c90);
        SetPersonnelMousePosition(159, 99);
        EnableMouseCursorDrawing();
        while (choice == 0) {
            choice = (signed char)RunCampaignSelectionFrame(
                campaignScene, field);
        }
        FreePacketAndClear(&g_pConstellationSprite_005d2c4c, 0);
        FreePacketAndClear(&campaignScene, 0);
        FreePacketAndClear(&field, 0);
        free_viewport(&g_stSecondaryViewBufferBlock_005d2c90);
        return (short)(choice - 1);
    }
    FreePacketAndClear(&campaignScene, 0);
    free_viewport(&g_stSecondaryViewBufferBlock_005d2c90);
    return 0;
}
