/*
 *  Kill board, conversation scenes and save-slot flags.
 *
 *  Address range 0x43c000-0x440bff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: ShowTigersClawKillBoard/RunConversationScene; string band 0x4705DC-0x470668.
 */
#include "wc1.h"

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
