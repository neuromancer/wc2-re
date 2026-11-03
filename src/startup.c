/*
 *  Game startup and main.
 *
 *  Retail object 0x45B810-0x45CA4F (link order; see docs/ORDER.md).
 */
#include "game.h"

extern unsigned int g_dwCannedSceneSnapshotEnd_004961a4;

/* Function start: 0x45B810 */
void LoadGamePaletteFile(void)
{
    short black[3] = { 0, 0, 0 };
    short entry;
    unsigned char *colours;
    unsigned char *paletteData;

    colours = g_abGamePaletteReservedColours_0049cb54;
    switch (g_nSpacePaletteFadeMode_004901e8) {
    case 9:
    case 13:
        for (entry = 0; entry < 14u; entry++)
            colours[entry] = g_abLegacyVideoModeColours_0049cb90[entry];
        break;
    case 0x13:
        paletteData = FetchDiskPacketRetrying("brief.pal", 0, 0);
        for (entry = 0; entry < 0x100; entry++)
            SetPaletteEntryFromTriplet(paletteData + entry * 3, entry);
        ResetCockpitPaletteEntries();
        SaveGamePalette();
        break;
    }
}

/* Function start: 0x45B905 */
void EMShutDown(void)
{
    if (g_bInputManagerInitialized_005c80ca != 0)
        ShutdownEventManager();
}

/* Function start: 0x45B924 */
void EMStartUp(void)
{
    if (InitializeInputManagerMemory(20) == 0)
        ReportFatalErrorCode("020");
    g_pfnEventManagerShutdown_005c8be0 = (void (*)(void))LogMemoryUsage;
    InitializeInputDriverHook();
    if (g_bRewritePacketExtensions_0049cb48 == 0) {
        LoadInputCursorShape(g_szInputCursorPacketName_0049cba0,
                             0, 0x13);
    } else {
        RewritePacketFilenameForInstalledData(
            g_szInputCursorPacketName_0049cba0);
        LoadInputCursorShape(g_szInputCursorPacketName_0049cba0,
                             0, 0x0d);
    }
    FinalizeInputDriverHook();
    SetInputViewport(&g_stScreenViewport_005d21a0);
    g_nInputTickInterval_005c8448 = 20;
    g_nInputRepeatDelay_005c80d6 = 20;
    g_bInputCursorEnabled_005c80e6 = 1;
    g_cKeypadMouseSpeed_005c83e6 = 3;
}

#pragma function(strcat)
/* Function start: 0x45B9D3 */
void LoadOriginFxDrivers(void)
{
    int requiredBytes = 100000;
    char campaignDigits[4];
    FILE *memoryLogFile;
    char packetName[13] = "stars.00";
    int availableMemory;
    int shipMemoryBytes;
    short paletteResult;

    g_nDetectedGraphicsMode_005c80d2 = QueryCurrentGraphicsMode();
    g_nGraphicsInitializationFlags_00493048 = 0x20c;
    IsSoundHardwarePresent(8);
    if (g_bPumpMessagesWhileLoading_005c8ddc != 0)
        MessagePumpHook(4);
    if (g_bHighMemoryResourcesEnabled_005c80e4 == 0)
        printf("No ");
    printf("Expanded Memory Detected.\n");
    g_bOriginFxDriverActive_0049cbb0 = 0;
    if (LoadGraphicsDriver(g_bRewritePacketExtensions_0049cb48) == 0)
        exit_squadron("Failed to load Origin-FX drivers");
    InitializeMouseCursorDepth(0);
    g_nInputDoubleClickInterval_00493050 = 2;
    EMStartUp();
    g_dwInitialFreeMemory_005c8dd0 =
        GetLargestFreeMemoryBlockByType(0);
    availableMemory = (int)GetAvailableFarMemoryByType(4);
    g_nAvailableGameMemory_005c8de0 = 0xc63e0;
    availableMemory = 0x7c0600;
    if (g_bSpeechResourcesEnabled_0049cc2c != 0 &&
        g_bHighMemoryResourcesEnabled_005c80e4 != 0) {
        g_bSpeechEnabled_005c8de8 = 0;
        if (g_nMinimumMemoryNoMusic_0049cc20 + 0x1000 <
            (int)g_dwInitialFreeMemory_005c8dd0) {
            if (InitializeSoundBlaster(
                    g_nSoundBlasterPortSetting_005c8dde,
                    g_nSoundBlasterIrqSetting_005c8dd8) == 0) {
                g_bSpeechEnabled_005c8de8 = 1;
                printf("Speech Enabled.\n");
            }
        }
    }
#ifndef SDL_PORT
    if (g_bHighMemoryResourcesEnabled_005c80e4 != 0)
        InitializeHighMemoryGraphicsBuffers();
#endif
    memoryLogFile = fopen("mem.$$$", "w+");
    LogMemoryStateToFile(memoryLogFile);
    if (memoryLogFile != 0)
        fclose(memoryLogFile);
    if (g_bHighMemoryResourcesEnabled_005c80e4 == 0) {
        g_bExpandedShipGraphicsEnabled_004931a4 = 0;
        g_nMinimumMemoryNoMusic_0049cc20 = 0x42680;
    }
    if (g_bRewritePacketExtensions_0049cb48 == 1) {
        g_nMinimumMemoryNoMusic_0049cc20 -= 20000;
        g_nMinimumMemoryFullMusic_0049cc24 -= 20000;
        g_nMinimumMemoryAlternate_0049cc28 -= 20000;
    }
    shipMemoryBytes =
        (int)g_nExpandedShipMemoryKb_0049cbac << 10;
    if (g_nMusicDriverMode_0049be8c != 0 &&
        g_nMusicDriverMode_0049be8c != 3) {
        if (g_nMusicDriverMode_0049be8c == 1)
            g_nMusicMemoryRequirement_005c8dcc = 0x1b800;
        else if (g_nMusicDriverMode_0049be8c == 2)
            g_nMusicMemoryRequirement_005c8dcc = 0x13800;
        if (g_bHighMemoryResourcesEnabled_005c80e4 != 0)
            requiredBytes += g_nMusicMemoryRequirement_005c8dcc;
    }
    SetFrameTimerPeriodDirect(0x78);
    g_nAvailableGameMemory_005c8de0 = 0xc63e0;
    availableMemory = 0x7c0600;
    if (availableMemory > requiredBytes) {
        g_nAvailableGameMemory_005c8de0 = 0xc63e0;
        g_nMemoryConfiguration_005c8dc8 = 2;
        printf("Expanded Memory fully used.\n");
    } else {
        g_nAvailableGameMemory_005c8de0 =
            (int)g_dwInitialFreeMemory_005c8dd0 -
            g_nMinimumMemoryNoMusic_0049cc20;
        if (g_nAvailableGameMemory_005c8de0 < 0)
            exit_squadron(
                "You do not have enough memory to play Wing Commander 2.\n"
                "Refer to your reference guide for assistance.");
        g_nMemoryConfiguration_005c8dc8 = 0;
        if (g_nMusicDriverMode_0049be8c == 1 ||
            g_nMusicDriverMode_0049be8c == 2) {
            if (g_nMinimumMemoryFullMusic_0049cc24 <
                    (int)g_dwInitialFreeMemory_005c8dd0 &&
                (int)GetAvailableFarMemoryByType(4) >
                    g_nMusicMemoryRequirement_005c8dcc) {
                g_nAvailableGameMemory_005c8de0 =
                    (int)g_dwInitialFreeMemory_005c8dd0 -
                    g_nMinimumMemoryFullMusic_0049cc24;
                g_nMemoryConfiguration_005c8dc8 = 1;
                printf("Full");
            } else {
                printf("Limited");
            }
            printf(" music/FX will play.");
        }
    }
    LoadSpaceflightResources();
    LoadJoystickCalibrationFile(9, 9, 1, 1);
    ConfigureInputPump(1, PollJoystickButtonEvents);
    ConfigureDefaultSpacePalette(
        g_acExpectedGraphicsModes_00493078[
            g_bRewritePacketExtensions_0049cb48]);
    if (g_acExpectedGraphicsModes_00493078[
            g_bRewritePacketExtensions_0049cb48] !=
        QueryCurrentGraphicsMode())
        exit_squadron(
            "Requested graphics display mode not available.");
    LoadGamePaletteFile();
    InitializeGameTextContexts();
    paletteResult = InitializePostPaletteState();
    if (g_nMusicDriverMode_0049be8c == 3)
        SetMusicTickRateHook(0x1e00);
    else
        SetMusicTickRateHook(0x3c);
    g_nFrameSkip_0049d764 = 1;
    if (g_nMusicDriverMode_0049be8c != 0 &&
        g_nMusicDriverMode_0049be8c != 3)
        InitializeMusicResources();
    initialize_direction_view_frames();
    strcat(packetName,
           itoa((int)g_nSelectedCampaignSlot_005d3bf2,
                 campaignDigits, 10));
    g_pStartupStarPacket_005d212c =
        LoadPacketAllocated(packetName, 0);
    if (g_nOriginDevUnlock_0049d774 != 0)
        _unlink("logfile.txt");
    if (g_nShowMemoryStatus_0049d784 != 0) {
        _unlink("track0.txt");
        _unlink("track1.txt");
        _unlink("track2.txt");
        _unlink("track3.txt");
        _unlink("track4.txt");
        _unlink("track5.txt");
        _unlink("track6.txt");
        _unlink("track7.txt");
        _unlink("track8.txt");
        _unlink("track9.txt");
    }
    (void)shipMemoryBytes;
    (void)paletteResult;
}

/* Function start: 0x45BF30 */
void InitializeHighMemoryGraphicsBuffers(void)
{
    unsigned int highMemoryEnd;

    g_pHighMemoryBlockA_004901f8 =
        AllocateDefaultMemory((unsigned int)g_wHighMemoryBlockBytes_004901fc);
    g_dwHighMemoryParagraph_005d3fb4 =
        IdentityDword((DwordPtr)g_pHighMemoryBlockA_004901f8);
    highMemoryEnd = (unsigned int)g_wHighMemoryBlockBytes_004901fc +
        g_dwHighMemoryParagraph_005d3fb4;
    /* The DOS build shrank and reallocated block A until it ended inside the
     * addressable window.  The Win32 port reduced the test to a constant, so
     * the retry body is compiled but never runs. */
    while (0) {
        g_wHighMemoryBlockBytes_004901fc =
            g_wHighMemoryBlockBytes_004901fc - 1;
        FreePacketAndClear(&g_pHighMemoryBlockA_004901f8, 4);
        g_pHighMemoryBlockA_004901f8 = 0;
        g_pHighMemoryBlockA_004901f8 = AllocateDefaultMemory(
            (unsigned int)g_wHighMemoryBlockBytes_004901fc);
        g_dwHighMemoryParagraph_005d3fb4 =
            IdentityDword((DwordPtr)g_pHighMemoryBlockA_004901f8);
        highMemoryEnd = (unsigned int)g_wHighMemoryBlockBytes_004901fc +
            g_dwHighMemoryParagraph_005d3fb4;
    }
    g_pHighMemoryBlockB_00490200 =
        AllocateDefaultMemory(CANNED_SCENE_SNAPSHOT_BYTES);
    if (g_pHighMemoryBlockB_00490200 == 0 ||
        g_pHighMemoryBlockA_004901f8 == 0) {
        FreePacketAndClear(&g_pHighMemoryBlockA_004901f8, 4);
        FreePacketAndClear(&g_pHighMemoryBlockB_00490200, 4);
        g_bHighMemoryBuffersReady_005d2ad8 = 0;
    } else {
        g_bHighMemoryBuffersReady_005d2ad8 = 1;
    }
}

/* Function start: 0x45C04B */
/* Nothing in the shipped executable calls this.  Releases the two blocks
 * InitializeHighMemoryGraphicsBuffers reserves. */
void ReleaseHighMemoryGraphicsBuffers(void)
{
    if (g_bHighMemoryBuffersReady_005d2ad8 == 0)
        return;
    FreePacketAndClear(&g_pHighMemoryBlockA_004901f8, 4);
    FreePacketAndClear(&g_pHighMemoryBlockB_00490200, 4);
}

/* Function start: 0x45C088 */
void InitializeGameTextContexts(void)
{
    InitializeDIBScreenViewport(&g_stScreenViewport_005d21a0,
                                (unsigned char)g_cSecondaryViewBufferColour_0049cb4c);
    g_stModalSourceViewport_005d2c50 = g_stScreenViewport_005d21a0;
    g_stDefaultTextContext_005d2d20.viewport =
        &g_stModalSourceViewport_005d2c50;
    g_stDefaultTextContext_005d2d20.text =
        g_szDefaultTextBuffer_005d2b80;
    InitializeTextContextFromFont(&g_stDefaultTextContext_005d2d20, 1,
                                  (unsigned char)g_ucPrimaryTextColour_0049cb64,
                                  g_cSecondaryViewBufferColour_0049cb4c);
    SetTextContext(&g_stDefaultTextContext_005d2d20);

    g_stSpaceTextContext_005d21c0.viewport = &g_stViewBuffer_005d2b00;
    g_stSpaceTextContext_005d21c0.text = g_szDefaultTextBuffer_005d2b80;
    g_stSpaceTextContext_005d21c0.alignment = 2;
    InitializeTextContextFromFont(&g_stSpaceTextContext_005d21c0, 1,
                                  g_abGamePaletteReservedColours_0049cb54[8], -1);
}

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

/* Function start: 0x45C1A1 */
void initialize_direction_view_frame(short yaw, short pitch,
                                     signed char frame)
{
    FixedVector right = { 0x100, 0, 0 };
    FixedVector up = { 0, 0x100, 0 };
    FixedVector forwardAxis = { 0, 0, 0x100 };

    rotate_about_j(yaw, &right, &forwardAxis);
    rotate_about_i(pitch, &up, &forwardAxis);
    g_aDirectionViewRightVector_005d2210[frame] = right;
    g_aDirectionViewUpVector_005d2500[frame] = up;
    g_aDirectionViewForwardVector_005d27f0[frame] = forwardAxis;
}

/* Function start: 0x45C279 */
void initialize_direction_view_frames(void)
{
    signed char frameIndex;
    signed char yawIndex;
    SphericalVector direction = { 0x100, 0, 0 };
    signed char pitchRing;

    frameIndex = 0;
    direction.yaw = 0;
    direction.pitch = 90;
    initialize_direction_view_frame(direction.yaw, direction.pitch,
                                    frameIndex++);
    for (pitchRing = 0; pitchRing < 5; pitchRing++) {
        direction.pitch -= 30;
        direction.yaw = 0;
        for (yawIndex = 0; yawIndex < 12; yawIndex++) {
            initialize_direction_view_frame(direction.yaw, direction.pitch,
                                            frameIndex++);
            direction.yaw += 30;
        }
    }
    direction.pitch = -90;
    direction.yaw = 0;
    initialize_direction_view_frame(direction.yaw, direction.pitch,
                                    frameIndex);
}

/* Function start: 0x45C35C */
void LoadSpaceflightResources(void)
{
    if (g_bHighMemoryResourcesEnabled_005c80e4 == 0)
        return;
    if (LoadShapeSet(g_aCommon3SpaceResources_0049c728,
                     4, "objects.vga") == 0)
        return;
    LoadShapeSet(g_aMissionResourceDescriptors_0049c798,
                 4, "objects.vga");
    g_aObjectTypeData_00496d30[34].shapeSet =
        g_aObjectTypeData_00496d30[35].shapeSet;
    g_aObjectTypeData_00496d30[33].shapeSet =
        g_aObjectTypeData_00496d30[34].shapeSet;
    g_aObjectTypeData_00496d30[30].shapeSet =
        g_aObjectTypeData_00496d30[33].shapeSet;
    g_aObjectTypeData_00496d30[29].shapeSet =
        g_aObjectTypeData_00496d30[30].shapeSet;
    LoadShapeSet(g_aHighMemoryCockpitResources_0049c7e8,
                 4, "cockpit.vga");
}

/* Function start: 0x45C3EF */
/* Nothing in the shipped executable calls this; an empty sound-effects
 * driver hook between the resource loader and music start-up. */
void FxDriverInitHook(void)
{
}

/* Function start: 0x45C3FA */
void StopMusicIfDriverActive(void)
{
    if (g_nMusicDriverMode_0049be8c != 0)
        StopMusicStream();
}

/* Function start: 0x45C419 */
void InitializeMusicResources(void)
{
    int resource;

    FadeMusic(200);
    if (g_bHighMemoryResourcesEnabled_005c80e4 != 0) {
        for (resource = 0; resource < 0x42; resource++) {
            if (g_acMusicResourcePresent_0049cc30[resource] != 0) {
                if (g_nMusicDriverMode_0049be8c == 1) {
                    g_aMusicResources_005d13e0[resource].packet =
                        FetchDiskPacketRetrying(
                            "music.r00", (short)resource, 4);
                } else {
                    g_aMusicResources_005d13e0[resource].packet =
                        FetchDiskPacketRetrying(
                            "music.a00", (short)resource, 4);
                }
            } else {
                g_aMusicResources_005d13e0[resource].packet = 0;
            }
            if (g_aMusicResources_005d13e0[resource].packet == 0)
                continue;
            else
                g_aMusicResources_005d13e0[resource].loaded = 1;
        }
    } else if (g_nMemoryConfiguration_005c8dc8 == 1) {
        g_pLimitedMusicBufferA_0049bea0 =
            AllocateTaggedMemory(0x33d0, 0x10);
        g_nLimitedMusicBufferAState_0049bea4 = -1;
        g_pLimitedMusicBufferB_0049bea6 =
            AllocateTaggedMemory(0x33d0, 0x10);
        g_nLimitedMusicBufferBState_0049beaa = -1;
    }
}

#pragma function(strcmp)
/* Function start: 0x45C558 */
#ifdef SDL_PORT
/* The port's entry point is the SDL launcher, so the game's own main() is
 * compiled under a name that does not collide with it. */
void GameMain(short argc, char **argv)
#else
void main(short argc, char **argv)
#endif
{
    short argumentIndex;
    int campaignIsDigit;
    int missionIsDigit;

    g_bRewritePacketExtensions_0049cb48 = 0;
    g_bSpeechResourcesEnabled_0049cc2c = 1;
    g_nSoundBlasterPortSetting_005c8dde = 2;
    g_nSoundBlasterIrqSetting_005c8dd8 = 5;
    for (argumentIndex = 0; argumentIndex < argc; argumentIndex++) {
        if (strcmp("Origin", argv[argumentIndex]) == 0)
            g_nOriginDevUnlock_0049d774 = 1;

        switch (argv[argumentIndex][0]) {
        case '?':
            printf("Version %s.\n", g_pszGameVersion_0049b528);
        case '+':
            if (argv[argumentIndex][1] == 'w')
                g_bRoomTransitionAnimationEnabled_00499c00 = 1;
            if (argv[argumentIndex][1] == 'l')
                g_bAutopilotDebugEnabled_00499bfc = 1;
            if (argv[argumentIndex][1] == 'e') {
                g_bMemoryAdjustmentEnabled_0049cc84 = 1;
                g_nMemoryAdjustmentKb_005c8dda =
                    (short)atoi(argv[argumentIndex] + 2);
            }
            break;
        case '-':
            if (argv[argumentIndex][1] == 'm')
                g_nShowMemoryStatus_0049d784 = 1;
            if (argv[argumentIndex][1] == 'x')
                g_bPumpMessagesWhileLoading_005c8ddc = 1;
            if (argv[argumentIndex][1] == 't')
                g_bPumpMessagesDuringLoad_0049cc7c = 1;
            if (argv[argumentIndex][1] == 'f')
                g_bSkipCampaignScenes_0049cc78 = 1;
            if (g_nOriginDevUnlock_0049d774 != 0) {
                switch (argv[argumentIndex][1]) {
                case 'k':
                    g_bPlayerDamageEnabled_0049d77c = 0;
                    break;
                case 'b':
                    g_bPlayerCollisionEnabled_0049d780 = 0;
                    break;
                }
            }
            break;
        case 'E':
        case 'e':
            if (g_nOriginDevUnlock_0049d774 != 0)
                g_bDirectMissionLaunch_0049d798 = 1;
            break;
        case 'T':
        case 't':
            if (g_nOriginDevUnlock_0049d774 != 0) {
                missionIsDigit = isdigit(argv[argumentIndex][1]);
                if (missionIsDigit != 0) {
                    g_nDirectMission_0049d79a =
                        (short)atoi(argv[argumentIndex] + 1);
                } else {
                    g_nDirectMission_0049d79a = (short)(
                        toupper(argv[argumentIndex][1]) - 'A');
                }
                if (g_nDirectMission_0049d79a > 3)
                    g_nDirectMission_0049d79a = 0;
                g_bDirectCampaignSelection_0049cc74 = 1;
            }
            break;
        case 'V':
        case 'v':
            if (g_nOriginDevUnlock_0049d774 != 0) {
                g_nDirectSeries_0049d79c =
                    (short)atoi(argv[argumentIndex] + 1);
                g_bDirectCampaignSelection_0049cc74 = 1;
            }
            break;
        case 'Z':
        case 'z':
            campaignIsDigit = isdigit(argv[argumentIndex][1]);
            if (campaignIsDigit != 0) {
                g_nSelectedStartingCampaign_005d3bf0 =
                    (short)(argv[argumentIndex][1] - '0');
            } else {
                g_nSelectedStartingCampaign_005d3bf0 = 0;
            }
            break;
        case 'R':
        case 'r':
            break;
        case 'X':
        case 'x':
            g_bExpandedShipGraphicsEnabled_004931a4 = 1;
            g_nExpandedShipMemoryKb_0049cbac =
                (short)atoi(argv[argumentIndex] + 1);
            break;
        case 'A':
        case 'a':
            break;
        case 'P':
        case 'p':
            break;
        case 'C':
        case 'c':
            g_bSpeechResourcesEnabled_0049cc2c = 1;
            g_nSoundBlasterPortSetting_005c8dde =
                (short)(argv[argumentIndex][1] - '0');
            g_nSoundBlasterIrqSetting_005c8dd8 =
                (short)(argv[argumentIndex][2] - '0');
            break;
        }
    }
}
