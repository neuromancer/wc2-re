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
    savedFailure = (short)g_4_nControllerFailureValue;
    error = 1;
    if (g_8_nActiveInputDeviceWord == -1)
        return;
    g_3_stDefaultStringContext.alignment = 2;
    if (g_36_apTextGlyphs[1] != 0)
        fontReady++;

    InitializeTextContextFromFont(&g_3_stDefaultStringContext, 1,
        g_bPrimaryViewBufferColour, (signed char)g_cSecondaryViewBufferColour);
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
        SampleJoystickDevice(sample, g_8_nActiveInputDeviceWord, 0x7fff);
        g_12_nJoystickMinimumXNumber = sample[g_8_nActiveInputDeviceWord].x;
        leftX = (short)g_12_nJoystickMinimumXNumber;
        g_2_nJsMinimumY = sample[g_8_nActiveInputDeviceWord].y;
        yTop = (short)g_2_nJsMinimumY;
        WaitForJoystickButtonRelease();
        ReleaseModalTextPanel();
    }

    if (ShowModalTextPanel(1,
        "Move stick to the LOWER RIGHT, press a button") != 0) {
        DIBslamReal();
        WaitForJoystickButtonPress();
        SampleJoystickDevice(sample, g_8_nActiveInputDeviceWord, 0x7fff);
        g_4_nJoystickMaximumXValue = sample[g_8_nActiveInputDeviceWord].x;
        maxX = (short)g_4_nJoystickMaximumXValue;
        g_nJoystickMaximumY = sample[g_8_nActiveInputDeviceWord].y;
        yMax = (short)g_nJoystickMaximumY;
        WaitForJoystickButtonRelease();
        ReleaseModalTextPanel();
    }

    if (ShowModalTextPanel(1,
        "Center Joystick, press a button") != 0) {
        DIBslamReal();
        WaitForJoystickButtonPress();
        SampleJoystickDevice(sample, g_8_nActiveInputDeviceWord, 0x7fff);
        g_1_nJoystickCentreXNumber = sample[g_8_nActiveInputDeviceWord].x;
        centreX = (short)g_1_nJoystickCentreXNumber;
        g_2_nJoystickCentreY = sample[g_8_nActiveInputDeviceWord].y;
        midY = (short)g_2_nJoystickCentreY;
        WaitForJoystickButtonRelease();
        ReleaseModalTextPanel();
    }

    if (fontReady == 0)
        ReleaseTextFont(1);

    if (g_3_nControllerHorizontalRange == 0)
        g_3_nControllerHorizontalRange = 9;
    if (g_nJoystickVerticalRangeNumber == 0)
        g_nJoystickVerticalRangeNumber = 9;
    g_12_nControllerLeftScaleValue =
        (g_1_nJoystickCentreXNumber - g_12_nJoystickMinimumXNumber) /
        g_3_nControllerHorizontalRange;
    g_1_nJsRightScaleWord =
        (g_4_nJoystickMaximumXValue - g_1_nJoystickCentreXNumber) /
        g_3_nControllerHorizontalRange;
    g_59_nJoystickUpScaleWord =
        (g_2_nJoystickCentreY - g_2_nJsMinimumY) /
        g_nJoystickVerticalRangeNumber;
    g_12_nJoystickDownScaleWord =
        (g_nJoystickMaximumY - g_2_nJoystickCentreY) /
        g_nJoystickVerticalRangeNumber;
    if (g_12_nControllerLeftScaleValue == 0)
        g_12_nControllerLeftScaleValue++;
    if (g_59_nJoystickUpScaleWord == 0)
        g_59_nJoystickUpScaleWord++;
    if (g_1_nJsRightScaleWord == 0)
        g_1_nJsRightScaleWord++;
    if (g_12_nJoystickDownScaleWord == 0)
        g_12_nJoystickDownScaleWord++;

    g_12_nJoystickMinimumXNumber = g_1_nJoystickCentreXNumber -
        g_12_nControllerLeftScaleValue *
            g_3_nControllerHorizontalRange;
    g_2_nJsMinimumY = g_2_nJoystickCentreY -
        g_59_nJoystickUpScaleWord * g_nJoystickVerticalRangeNumber;
    g_4_nJoystickMaximumXValue = g_1_nJoystickCentreXNumber +
        g_1_nJsRightScaleWord *
            g_3_nControllerHorizontalRange;
    g_nJoystickMaximumY = g_2_nJoystickCentreY +
        g_12_nJoystickDownScaleWord * g_nJoystickVerticalRangeNumber;
    g_4_nControllerFailureValue = g_4_nJoystickMaximumXValue * 2;

    if (g_4_nJoystickMaximumXValue <= g_12_nJoystickMinimumXNumber ||
        g_nJoystickMaximumY <= g_2_nJsMinimumY) {
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
        error = _write(file, &g_8_nActiveInputDeviceWord, 2) == -1;
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
            &g_aInputDeviceSamplesList[
                g_8_nActiveInputDeviceWord],
            g_8_nActiveInputDeviceWord, 0);
        ServiceSoundSystem();
    } while (g_aInputDeviceSamplesList[
                 g_8_nActiveInputDeviceWord].buttons != 0);
}

/* Function start: 0x418D5F */
void WaitForJoystickButtonPress(void)
{
    do {
        SampleJoystickDevice(
            &g_aInputDeviceSamplesList[
                g_8_nActiveInputDeviceWord],
            g_8_nActiveInputDeviceWord, 0);
        ServiceSoundSystem();
    } while (g_aInputDeviceSamplesList[
                 g_8_nActiveInputDeviceWord].buttons == 0);
}

/* Function start: 0x418DAA */
short RunCampaignSelectionFrame(void *scene, void *field)
{
    short selection;

    selection = 0;
    while ((selection = PollSceneHotspotInput(scene, 0, 0, 0, 0)) == 0) {
        DisableMouseCursorDrawing();
        DrawSpriteDefault(
            &g_stSecondaryViewBufferBlock, 0, 0, field, 0);
        DrawConstellationField();
        DrawSpriteDefault(
            &g_stSecondaryViewBufferBlock, 0, 0, scene, 0);
        if (g_pszPersonnelFooter != 0) {
            g_3_stDefaultStringContext.viewport =
                &g_stSecondaryViewBufferBlock;
            InitializeTextContextFromFont(
                &g_3_stDefaultStringContext, 1,
                g_bPrimaryViewBufferColour, -1);
            SetTextCursor(
                (unsigned short)((320 -
                    MeasureTextPixelWidthClamped(
                        g_pszPersonnelFooter)) >> 1),
                180);
            DrawFormattedText("%S", g_pszPersonnelFooter);
            g_3_stDefaultStringContext.viewport =
                &g_101_stScrViewInfo;
        }
        EnableMouseCursorDrawing();
        WaitForVerticalBlankThunk();
        CopyViewportContents(&g_stSecondaryViewBufferBlock,
                             &g_101_stScrViewInfo);
    }
    if (g_bSceneEscapeRequested != 0)
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
    g_bKeyboardMouseEnabled = 1;
    ClearDebugPauseFlags();
    ReleaseInputEventQueue();
    while ((choice = PollSceneHotspotInput(buttons, 0, 0, 0,
                                           maxSelection)) == 0) {
        PumpWindowMessages(0);
        DrawSpriteDefault(&g_stSecondaryViewBufferBlock, 0, 0,
                          background, 0);
        DrawConstellationField();
        DrawSpriteDefault(&g_stSecondaryViewBufferBlock, 160, 100,
                          logo, 0x19);
        DrawSpriteDefault(&g_stSecondaryViewBufferBlock, 160, 100,
                          logo, 0x1f);
        DrawSpriteDefault(&g_stSecondaryViewBufferBlock, 160, 100,
                          logo, 0x20);
        DrawSpriteDefault(&g_stSecondaryViewBufferBlock, 0, 0,
                          buttons, 0);
        if (showSecondButton != 0)
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock, 0, 0,
                              buttons, 1);
        RefreshMemoryStatusOverlay();
    }
    g_bKeyboardMouseEnabled = savedKeyboardMouse;
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
    g_18_nInterfaceEventMode = 0;
    SetMenuInputPump();
    DisableMouseCursorDrawing();
    g_18_nInterfaceEventMode = 0;
    ClearViewport(&g_2_stModalSourceWindow,
                  g_cSecondaryViewBufferColour);
    g_stSecondaryViewBufferBlock.left = 0;
    g_stSecondaryViewBufferBlock.right = 319;
    g_stSecondaryViewBufferBlock.top = 0;
    g_stSecondaryViewBufferBlock.bottom = 199;
    if (AllocateViewport(&g_stSecondaryViewBufferBlock,
                         g_cSecondaryViewBufferColour, 0) == 0)
        ReportFatalErrorCode("031");
    screenCopy.left = 0;
    screenCopy.right = 319;
    screenCopy.top = 0;
    screenCopy.bottom = 199;
    if (AllocateViewport(&screenCopy,
                         g_cSecondaryViewBufferColour, 0) == 0)
        ReportFatalErrorCode("032");
    PreloadMusicTrack(0x36);
    spacetrack(0x36, 1, 1);
    init_constellation(0);
    InitializeConstellationField(
        &g_stSecondaryViewBufferBlock, 0, 0x10);
    buttons = FetchDiskPacketRetrying("buttons.v00", 0, 0);
    logo = FetchDiskPacketRetrying("wc2logo.vga", 0, 0);
    background = FetchDiskPacketRetrying("field.v00", 1, 0);
    FlushInputEvents();
    ClearDebugPauseFlags();
    if (WaitForInputKey() != 0)
        g_bSceneEscapeRequested = 1;
    if (g_bSceneEscapeRequested == 0)
        PlaySfxWaveFileByNumber(0x28, -1, 0);
    if (g_bSceneEscapeRequested == 0) {
        for (logoFrame = 0; logoFrame < 0x1a; logoFrame++) {
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                              0, 0, background, 0);
            DrawConstellationField();
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                              160, 100, logo, logoFrame);
            RefreshMemoryStatusOverlay();
            if (WaitForInputKey() != 0) {
                g_bSceneEscapeRequested = 1;
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
    if (g_bSceneEscapeRequested == 0) {
        ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(0, 0);
        PlaySfxWaveFileByNumber(99, -1, 0);
    }
    if (g_bSceneEscapeRequested == 0) {
        for (logoFrame = 0; logoFrame < 10; logoFrame++) {
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                              0, 0, background, 0);
            DrawConstellationField();
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                              160, 100, logo, 0x19);
            RefreshMemoryStatusOverlay();
            if (WaitForInputKey() != 0) {
                g_bSceneEscapeRequested = 1;
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
    if (g_bSceneEscapeRequested == 0) {
        for (logoFrame = 0x1a; logoFrame < 0x20; logoFrame++) {
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                              0, 0, background, 0);
            DrawConstellationField();
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                              160, 100, logo, 0x19);
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                              160, 100, logo, logoFrame);
            RefreshMemoryStatusOverlay();
            if (WaitForInputKey() != 0) {
                g_bSceneEscapeRequested = 1;
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
    if (g_bSceneEscapeRequested == 0)
        PlaySfxWaveFileByNumber(99, -1, 0);
    if (g_bSceneEscapeRequested == 0) {
        for (logoFrame = 0; logoFrame < 10; logoFrame++) {
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                              0, 0, background, 0);
            DrawConstellationField();
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                              160, 100, logo, 0x19);
            DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                              160, 100, logo, 0x1f);
            RefreshMemoryStatusOverlay();
            if (WaitForInputKey() != 0) {
                g_bSceneEscapeRequested = 1;
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
    if (g_bSceneEscapeRequested == 0)
        PlaySfxWaveFileByNumber(100, -1, 0);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                      0, 0, background, 0);
    DrawConstellationField();
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                      160, 100, logo, 0x19);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                      160, 100, logo, 0x1f);
    RefreshMemoryStatusOverlay();
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                      160, 100, logo, 0x20);
    CopyViewportContents(&g_101_stScrViewInfo,
                         &screenCopy);
    MarkDibDirty();
    DIBslamReal();
    wipeType = 8;
    g_bRoomTransitionAnimationEnabled = 1;
    if (g_bRoomTransitionAnimationEnabled != 0) {
        wipeState = AllocateTaggedMemory(0xce, 0);
        if (wipeState != 0) {
            InitializeViewportWipe(&g_stSecondaryViewBufferBlock,
                                   &screenCopy, wipeType, 0x3c, 0,
                                   wipeState);
            done = 0;
            while (done == 0) {
                done = AdvanceViewportWipe(wipeState);
                if (WaitForInputKey() != 0 ||
                    g_bSceneEscapeRequested != 0) {
                    g_bSceneEscapeRequested = 1;
                    break;
                }
                CopyViewportContents(&screenCopy,
                                     &g_101_stScrViewInfo);
                DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                                  0, 0, background, 0);
                DrawConstellationField();
                DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                                  160, 100, logo, 0x19);
                DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                                  160, 100, logo, 0x1f);
                DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
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
    g_18_nInterfaceEventMode = 1;
    SetInputViewport(&g_stSecondaryViewBufferBlock);
    SetPersonnelMousePosition(159, 159);
    EnableMouseCursorDrawing();
    ((void (__cdecl *)(int, int))FlushSoundEffectsAndLog)(
        sfxHandle, 0);
    g_bNewPilotCampaignInitialized = 0;
    free_viewport(&screenCopy);
    if (g_bSceneEscapeRequested != 0)
        g_bSceneEscapeRequested = 0;
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
    g_bSceneEscapeRequested = 0;
    menuChoice = 0;
    while (menuChoice == 0) {
        menuChoice = (signed char)RunTitleMenuInputLoop(
            buttons, logo, background, canContinueCampaign);
    }
    DisableMouseCursorDrawing();
    FreePacketAndClear(&buttons, 0);
    FreePacketAndClear(&logo, 0);
    FreePacketAndClear(&background, 0);
    free_viewport(&g_stSecondaryViewBufferBlock);
    FreePacketAndClear(&g_pConstellationSprite, 0);
    StopMusicUnlessSuppressed();
    ReleaseMusicTrack(0x36);
    g_bSceneEscapeRequested = 0;
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
    ClearViewport(&g_2_stModalSourceWindow,
                  g_cSecondaryViewBufferColour);
    g_stSecondaryViewBufferBlock.left = 0;
    g_stSecondaryViewBufferBlock.right = 319;
    g_stSecondaryViewBufferBlock.top = 0;
    g_stSecondaryViewBufferBlock.bottom = 199;
    if ((short)AllocateViewport(
            &g_stSecondaryViewBufferBlock,
            g_cSecondaryViewBufferColour, 0) == 0) {
        ReportFatalErrorCode("033");
    }
    ClearViewport(&g_stSecondaryViewBufferBlock, 0);
    campaignScene = LoadNamedPacket(
        "campaign.vga", 0, 0, 0, 0, 1);
    if (campaignScene != 0 && WaitForSceneAdvance(campaignScene) != 0) {
        field = FetchDiskPacketRetrying("field.v00", 1, 0);
        init_constellation(0);
        InitializeConstellationField(
            &g_stSecondaryViewBufferBlock, 0, 16);
        SetMenuInputPump();
        SetInputViewport(&g_stSecondaryViewBufferBlock);
        SetPersonnelMousePosition(159, 99);
        EnableMouseCursorDrawing();
        while (choice == 0) {
            choice = (signed char)RunCampaignSelectionFrame(
                campaignScene, field);
        }
        FreePacketAndClear(&g_pConstellationSprite, 0);
        FreePacketAndClear(&campaignScene, 0);
        FreePacketAndClear(&field, 0);
        free_viewport(&g_stSecondaryViewBufferBlock);
        return (short)(choice - 1);
    }
    FreePacketAndClear(&campaignScene, 0);
    free_viewport(&g_stSecondaryViewBufferBlock);
    return 0;
}
