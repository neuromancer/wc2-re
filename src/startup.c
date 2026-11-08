/*
 *  Game startup and main.
 *
 *  Retail object 0x45B810-0x45CA4F (link order; see docs/ORDER.md).
 */
#include "game.h"

extern unsigned int g_dwCannedSceneSnapshotEnd; /* 0x004961a4 */

/* Function start: 0x45B810 */
void LoadGamePaletteFile(void)
{
    short black[3] = { 0, 0, 0 };
    short entry;
    unsigned char *colours;
    unsigned char *paletteData;

    /* As in WC1, the legacy modes treat the colour list from blue onwards as
     * a byte array; the Win32 globals are four bytes apart. */
    colours = &g_cBlueColour;
    switch (g_nSpacePaletteFadeMode) {
    case 9:
    case 13:
        for (entry = 0; entry < 14u; entry++)
            colours[entry] = g_abLegacyVideoModeColours[entry];
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
    if (g_18_bEventManagerInitializationState != 0)
        ShutdownEventManager();
}

/* Function start: 0x45B924 */
void EMStartUp(void)
{
    if (InitializeInputManagerMemory(20) == 0)
        ReportFatalErrorCode("020");
    g_59_pfnEventShutdownCallback = (void (*)(void))LogMemoryUsage;
    InitializeInputDriverHook();
    if (g_bRewritePacketExtensions == 0) {
        LoadInputCursorShape(g_szInputCursorPacketName,
                             0, 0x13);
    } else {
        RewritePacketFilenameForInstalledData(
            g_szInputCursorPacketName);
        LoadInputCursorShape(g_szInputCursorPacketName,
                             0, 0x0d);
    }
    FinalizeInputDriverHook();
    SetInputViewport(&g_101_stScrViewInfo);
    g_86_nCurrentInputTickInterval = 20;
    g_88_nEventRepeatDelayState = 20;
    g_26_bMouseCursorEnabled = 1;
    g_cKeypadMouseSpeedValue = 3;
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

    g_14_nCurrentGraphicsModeDetectedValue = QueryCurrentGraphicsMode();
    g_nNearHeapMaxDescriptors = 0x20c;
    IsSoundHardwarePresent(8);
    if (g_2_bPumpMessagesWhileLoadingState != 0)
        MessagePumpHook(4);
    if (g_19_bStoredHighMemoryEnabled == 0)
        printf("No ");
    printf("Expanded Memory Detected.\n");
    g_bOriginFxDriverActive = 0;
    if (LoadGraphicsDriver(g_bRewritePacketExtensions) == 0)
        exit_squadron("Failed to load Origin-FX drivers");
    InitializeMouseCursorDepth(0);
    g_nInputDoubleClickInterval = 2;
    EMStartUp();
    g_dwInitialSpareMemory =
        GetLargestFreeMemoryBlockByType(0);
    availableMemory = (int)GetAvailableFarMemoryByType(4);
    g_nAvailableGameMemoryNumber = 0xc63e0;
    availableMemory = 0x7c0600;
    if (g_bSpeechResourcesEnabled != 0 &&
        g_19_bStoredHighMemoryEnabled != 0) {
        g_bSpeechEnabledFlag = 0;
        if (g_nMinimumMemoryNoMusic + 0x1000 <
            (int)g_dwInitialSpareMemory) {
            if (InitializeSoundBlaster(
                    g_3_nAudioBlasterPortSettingValue,
                    g_3_nVoiceBlasterIrqSetting) == 0) {
                g_bSpeechEnabledFlag = 1;
                printf("Speech Enabled.\n");
            }
        }
    }
#ifndef SDL_PORT
    if (g_19_bStoredHighMemoryEnabled != 0)
        InitializeHighMemoryGraphicsBuffers();
#endif
    memoryLogFile = fopen("mem.$$$", "w+");
    LogMemoryStateToFile(memoryLogFile);
    if (memoryLogFile != 0)
        fclose(memoryLogFile);
    if (g_19_bStoredHighMemoryEnabled == 0) {
        g_bExpandedShipGraphicsEnabled = 0;
        g_nMinimumMemoryNoMusic = 0x42680;
    }
    if (g_bRewritePacketExtensions == 1) {
        g_nMinimumMemoryNoMusic -= 20000;
        g_nMinimumMemoryFullMusic -= 20000;
        g_nMinimumMemoryAlternate -= 20000;
    }
    shipMemoryBytes =
        (int)g_nExpandedShipMemoryKb << 10;
    if (g_nMusicDriverMode != 0 &&
        g_nMusicDriverMode != 3) {
        if (g_nMusicDriverMode == 1)
            g_8_nMusicMemoryNeed = 0x1b800;
        else if (g_nMusicDriverMode == 2)
            g_8_nMusicMemoryNeed = 0x13800;
        if (g_19_bStoredHighMemoryEnabled != 0)
            requiredBytes += g_8_nMusicMemoryNeed;
    }
    SetFrameTimerPeriodDirect(0x78);
    g_nAvailableGameMemoryNumber = 0xc63e0;
    availableMemory = 0x7c0600;
    if (availableMemory > requiredBytes) {
        g_nAvailableGameMemoryNumber = 0xc63e0;
        g_98_nMemConfiguration = 2;
        printf("Expanded Memory fully used.\n");
    } else {
        g_nAvailableGameMemoryNumber =
            (int)g_dwInitialSpareMemory -
            g_nMinimumMemoryNoMusic;
        if (g_nAvailableGameMemoryNumber < 0)
            exit_squadron(g_szNotEnoughMemoryMessage);
        g_98_nMemConfiguration = 0;
        if (g_nMusicDriverMode == 1 ||
            g_nMusicDriverMode == 2) {
            if (g_nMinimumMemoryFullMusic <
                    (int)g_dwInitialSpareMemory &&
                (int)GetAvailableFarMemoryByType(4) >
                    g_8_nMusicMemoryNeed) {
                g_nAvailableGameMemoryNumber =
                    (int)g_dwInitialSpareMemory -
                    g_nMinimumMemoryFullMusic;
                g_98_nMemConfiguration = 1;
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
        g_acExpectedGraphicsModes[
            g_bRewritePacketExtensions]);
    if (g_acExpectedGraphicsModes[
            g_bRewritePacketExtensions] !=
        QueryCurrentGraphicsMode())
        exit_squadron(
            "Requested graphics display mode not available.");
    LoadGamePaletteFile();
    InitializeGameTextContexts();
    paletteResult = InitializePostPaletteState();
    if (g_nMusicDriverMode == 3)
        SetMusicTickRateHook(0x1e00);
    else
        SetMusicTickRateHook(0x3c);
    g_nFrameSkip = 1;
    if (g_nMusicDriverMode != 0 &&
        g_nMusicDriverMode != 3)
        InitializeMusicResources();
    initialize_direction_view_frames();
    strcat(packetName,
           itoa((int)g_nSelectedCampaignSlot,
                 campaignDigits, 10));
    g_13_pStartupStellarPacket =
        LoadPacketAllocated(packetName, 0);
    if (g_nOriginDevUnlock != 0)
        _unlink("logfile.txt");
    if (g_nShowMemoryStatus != 0) {
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

/* The memory check above reports through exit_squadron with the shared
 * message; its named parameter also keeps this unit's symbol-key numbering. */
void exit_squadron(const char *msg, ...);

/* Function start: 0x45BF30 */
void InitializeHighMemoryGraphicsBuffers(void)
{
    unsigned int highMemoryEnd;

    g_pHighMemoryBlockA =
        AllocateDefaultMemory((unsigned int)g_wHighMemoryBlockBytes);
    g_dwHighMemoryParagraphValue =
        IdentityDword((DwordPtr)g_pHighMemoryBlockA);
    highMemoryEnd = (unsigned int)g_wHighMemoryBlockBytes +
        g_dwHighMemoryParagraphValue;
    /* The DOS build shrank and reallocated block A until it ended inside the
     * addressable window.  The Win32 port reduced the test to a constant, so
     * the retry body is compiled but never runs. */
    while (0) {
        g_wHighMemoryBlockBytes =
            g_wHighMemoryBlockBytes - 1;
        FreePacketAndClear(&g_pHighMemoryBlockA, 4);
        g_pHighMemoryBlockA = 0;
        g_pHighMemoryBlockA = AllocateDefaultMemory(
            (unsigned int)g_wHighMemoryBlockBytes);
        g_dwHighMemoryParagraphValue =
            IdentityDword((DwordPtr)g_pHighMemoryBlockA);
        highMemoryEnd = (unsigned int)g_wHighMemoryBlockBytes +
            g_dwHighMemoryParagraphValue;
    }
    g_pHighMemoryBlockB =
        AllocateDefaultMemory(CANNED_SCENE_SNAPSHOT_BYTES);
    if (g_pHighMemoryBlockB == 0 ||
        g_pHighMemoryBlockA == 0) {
        FreePacketAndClear(&g_pHighMemoryBlockA, 4);
        FreePacketAndClear(&g_pHighMemoryBlockB, 4);
        g_78_bHighMemoryBuffersSetState = 0;
    } else {
        g_78_bHighMemoryBuffersSetState = 1;
    }
}

/* Function start: 0x45C04B */
/* Nothing in the shipped executable calls this.  Releases the two blocks
 * InitializeHighMemoryGraphicsBuffers reserves. */
void ReleaseHighMemoryGraphicsBuffers(void)
{
    if (g_78_bHighMemoryBuffersSetState == 0)
        return;
    FreePacketAndClear(&g_pHighMemoryBlockA, 4);
    FreePacketAndClear(&g_pHighMemoryBlockB, 4);
}

/* Function start: 0x45C088 */
void InitializeGameTextContexts(void)
{
    InitializeDIBScreenViewport(&g_101_stScrViewInfo,
                                (unsigned char)g_cSecondaryViewBufferColour);
    g_2_stModalSourceWindow = g_101_stScrViewInfo;
    g_3_stDefaultStringContext.viewport =
        &g_2_stModalSourceWindow;
    g_3_stDefaultStringContext.text =
        g_6_szDefaultTextBufferText;
    InitializeTextContextFromFont(&g_3_stDefaultStringContext, 1,
                                  (unsigned char)g_ucPrimaryTextColour,
                                  g_cSecondaryViewBufferColour);
    SetTextContext(&g_3_stDefaultStringContext);

    g_168_stFlightTextContext.viewport = &g_27_stViewScratchInfo;
    g_168_stFlightTextContext.text = g_6_szDefaultTextBufferText;
    g_168_stFlightTextContext.alignment = 2;
    InitializeTextContextFromFont(&g_168_stFlightTextContext, 1,
                                  g_cRedColour, -1);
}

/* Function start: 0x45C128 */
void ResetGameTextContexts(void)
{
    g_3_stDefaultStringContext.viewport =
        &g_2_stModalSourceWindow;
    g_3_stDefaultStringContext.text =
        g_6_szDefaultTextBufferText;
    InitializeTextContextFromFont(&g_3_stDefaultStringContext, 1,
                                  g_ucPrimaryTextColour,
                                  g_cSecondaryViewBufferColour);
    SetTextContext(&g_3_stDefaultStringContext);
    g_168_stFlightTextContext.viewport = &g_27_stViewScratchInfo;
    g_168_stFlightTextContext.text = g_6_szDefaultTextBufferText;
    g_168_stFlightTextContext.alignment = 2;
    InitializeTextContextFromFont(&g_168_stFlightTextContext, 1,
                                  g_cRedColour, -1);
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
    g_9_aDirectionLookRightVector[frame] = right;
    g_137_aDirectionViewUpVectorArray[frame] = up;
    g_2_aDirectionViewForwardAxisTable[frame] = forwardAxis;
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
    if (g_19_bStoredHighMemoryEnabled == 0)
        return;
    if (LoadShapeSet(g_aCommon3SpaceResources,
                     4, "objects.vga") == 0)
        return;
    LoadShapeSet(g_aMissionResourceDescriptors,
                 4, "objects.vga");
    g_aObjectTypeData[34].shapeSet =
        g_aObjectTypeData[35].shapeSet;
    g_aObjectTypeData[33].shapeSet =
        g_aObjectTypeData[34].shapeSet;
    g_aObjectTypeData[30].shapeSet =
        g_aObjectTypeData[33].shapeSet;
    g_aObjectTypeData[29].shapeSet =
        g_aObjectTypeData[30].shapeSet;
    LoadShapeSet(g_aHighMemoryCockpitResources,
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
    if (g_nMusicDriverMode != 0)
        StopMusicStream();
}

/* Function start: 0x45C419 */
void InitializeMusicResources(void)
{
    int resource;

    FadeMusic(200);
    if (g_19_bStoredHighMemoryEnabled != 0) {
        for (resource = 0; resource < 0x42; resource++) {
            if (g_acMusicResourcePresent[resource] != 0) {
                if (g_nMusicDriverMode == 1) {
                    g_aTuneResources[resource].packet =
                        FetchDiskPacketRetrying(
                            "music.r00", (short)resource, 4);
                } else {
                    g_aTuneResources[resource].packet =
                        FetchDiskPacketRetrying(
                            "music.a00", (short)resource, 4);
                }
            } else {
                g_aTuneResources[resource].packet = 0;
            }
            if (g_aTuneResources[resource].packet == 0)
                continue;
            else
                g_aTuneResources[resource].loaded = 1;
        }
    } else if (g_98_nMemConfiguration == 1) {
        g_aLimitedMusicBuffers[0].buffer =
            AllocateTaggedMemory(0x33d0, 0x10);
        g_aLimitedMusicBuffers[0].state = -1;
        g_aLimitedMusicBuffers[1].buffer =
            AllocateTaggedMemory(0x33d0, 0x10);
        g_aLimitedMusicBuffers[1].state = -1;
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

    g_bRewritePacketExtensions = 0;
    g_bSpeechResourcesEnabled = 1;
    g_3_nAudioBlasterPortSettingValue = 2;
    g_3_nVoiceBlasterIrqSetting = 5;
    for (argumentIndex = 0; argumentIndex < argc; argumentIndex++) {
        if (strcmp("Origin", argv[argumentIndex]) == 0)
            g_nOriginDevUnlock = 1;

        switch (argv[argumentIndex][0]) {
        case '?':
            printf("Version %s.\n", g_pszGameVersion);
        case '+':
            if (argv[argumentIndex][1] == 'w')
                g_bRoomTransitionAnimationEnabled = 1;
            if (argv[argumentIndex][1] == 'l')
                g_bAutopilotDebugEnabled = 1;
            if (argv[argumentIndex][1] == 'e') {
                g_bMemoryAdjustmentEnabled = 1;
                g_5_nStorageAdjustmentKbValue =
                    (short)atoi(argv[argumentIndex] + 2);
            }
            break;
        case '-':
            if (argv[argumentIndex][1] == 'm')
                g_nShowMemoryStatus = 1;
            if (argv[argumentIndex][1] == 'x')
                g_2_bPumpMessagesWhileLoadingState = 1;
            if (argv[argumentIndex][1] == 't')
                g_bPumpMessagesDuringLoad = 1;
            if (argv[argumentIndex][1] == 'f')
                g_bSkipCampaignScenes = 1;
            if (g_nOriginDevUnlock != 0) {
                switch (argv[argumentIndex][1]) {
                case 'k':
                    g_bPlayerDamageEnabled = 0;
                    break;
                case 'b':
                    g_bPlayerCollisionEnabled = 0;
                    break;
                }
            }
            break;
        case 'E':
        case 'e':
            if (g_nOriginDevUnlock != 0)
                g_stDirectMissionSelection.launch = 1;
            break;
        case 'T':
        case 't':
            if (g_nOriginDevUnlock != 0) {
                missionIsDigit = isdigit(argv[argumentIndex][1]);
                if (missionIsDigit != 0) {
                    g_stDirectMissionSelection.mission =
                        (short)atoi(argv[argumentIndex] + 1);
                } else {
                    g_stDirectMissionSelection.mission = (short)(
                        toupper(argv[argumentIndex][1]) - 'A');
                }
                if (g_stDirectMissionSelection.mission > 3)
                    g_stDirectMissionSelection.mission = 0;
                g_bDirectCampaignSelection = 1;
            }
            break;
        case 'V':
        case 'v':
#ifdef SDL_PORT
            /* DOS wc2.cfg uses a bare V for VGA. With cheats enabled,
             * the Win32 developer parser otherwise treats atoi("") as
             * series 0 and skips New Game/Continue. Explicit V<number>
             * arguments still select a developer campaign. */
            if (argv[argumentIndex][1] == 0)
                break;
#endif
            if (g_nOriginDevUnlock != 0) {
                g_nDirectSeries =
                    (short)atoi(argv[argumentIndex] + 1);
                g_bDirectCampaignSelection = 1;
            }
            break;
        case 'Z':
        case 'z':
            campaignIsDigit = isdigit(argv[argumentIndex][1]);
            if (campaignIsDigit != 0) {
                g_nSelectedStartingCampaign =
                    (short)(argv[argumentIndex][1] - '0');
            } else {
                g_nSelectedStartingCampaign = 0;
            }
            break;
        case 'R':
        case 'r':
            break;
        case 'X':
        case 'x':
            g_bExpandedShipGraphicsEnabled = 1;
            g_nExpandedShipMemoryKb =
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
            g_bSpeechResourcesEnabled = 1;
            g_3_nAudioBlasterPortSettingValue =
                (short)(argv[argumentIndex][1] - '0');
            g_3_nVoiceBlasterIrqSetting =
                (short)(argv[argumentIndex][2] - '0');
            break;
        }
    }
}

/* This unit's initialized data, after its functions so that it does not
 * move the functions' symbol keys.  The colour list matches WC1's (blue,
 * yellow, red, dark green, primary text, orange, dark blue, ...); WC2 never
 * reads its dark blue or brown entries. */
short g_bRewritePacketExtensions = 0; /* 0x0049cb48 */
unsigned char g_cSecondaryViewBufferColour = 0; /* 0x0049cb4c */
unsigned char g_bPrimaryViewBufferColour = 15; /* 0x0049cb50 */
unsigned char g_cBlueColour = 0x25; /* 0x0049cb54 */
unsigned char g_cYellowColour = 0x42; /* 0x0049cb58 */
unsigned char g_cRedColour = 0x47; /* 0x0049cb5c */
unsigned char g_cDarkGreenColour = 0x5d; /* 0x0049cb60 */
unsigned char g_ucPrimaryTextColour = 0x5b; /* 0x0049cb64 */
unsigned char g_ucHostileCapitalScannerColour = 0x46; /* 0x0049cb68 */
unsigned char g_cDarkBlueColour = 0x27; /* 0x0049cb6c */
unsigned char g_ucHomeCarrierScannerColour = 0xa9; /* 0x0049cb70 */
unsigned char g_ucPlayerNavLabelColour = 0xa5; /* 0x0049cb74 */
unsigned char g_ucNavObjectiveMarkerColour = 0xb6; /* 0x0049cb78 */
unsigned char g_ucDefaultTextColour = 0x5c; /* 0x0049cb7c */
unsigned char g_cBrownColour = 0xe8; /* 0x0049cb80 */
unsigned char g_ucAsteroidNavMarkerColour = 0xf5; /* 0x0049cb84 */
unsigned char g_cPrimaryViewBufferColour = 0xf8; /* 0x0049cb88 */
unsigned char g_abLegacyVideoModeColours[14] = {
    9, 14, 4, 2, 10, 12, 15, 8, 7, 5, 2, 6, 7, 0
}; /* 0x0049cb90 */
char g_szInputCursorPacketName[12] = "arrow.vga"; /* 0x0049cba0 */
short g_nExpandedShipMemoryKb = 0; /* 0x0049cbac */
short g_bOriginFxDriverActive = 0; /* 0x0049cbb0 */
char g_szNotEnoughMemoryMessage[] =
    "You do not have enough memory to play Wing Commander 2.\n"
    "Refer to your reference guide for assistance."; /* 0x0049cbb8 */
int g_nMinimumMemoryNoMusic = 0x43238; /* 0x0049cc20 */
int g_nMinimumMemoryFullMusic = 0x49bb0; /* 0x0049cc24 */
int g_nMinimumMemoryAlternate = 0x43238; /* 0x0049cc28 */
short g_bSpeechResourcesEnabled = 0; /* 0x0049cc2c */
signed char g_acMusicResourcePresent[66] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0
}; /* 0x0049cc30 */
short g_bDirectCampaignSelection = 0; /* 0x0049cc74 */
short g_bSkipCampaignScenes = 0; /* 0x0049cc78 */
short g_bPumpMessagesDuringLoad = 0; /* 0x0049cc7c */
int g_nUnusedStartupWord = 0; /* 0x0049cc80 */
short g_bMemoryAdjustmentEnabled = 0; /* 0x0049cc84 */

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
int g_98_nMemConfiguration; /* 0x005c8dc8 */
int g_8_nMusicMemoryNeed; /* 0x005c8dcc */
unsigned int g_dwInitialSpareMemory; /* 0x005c8dd0 */
int g_3_nUnusedStartupValue; /* 0x005c8dd4 */ /* never referenced */
short g_3_nVoiceBlasterIrqSetting; /* 0x005c8dd8 */
short g_5_nStorageAdjustmentKbValue; /* 0x005c8dda */
short g_2_bPumpMessagesWhileLoadingState; /* 0x005c8ddc */
short g_3_nAudioBlasterPortSettingValue; /* 0x005c8dde */
int g_nAvailableGameMemoryNumber; /* 0x005c8de0 */
int g_11_nMissionResourceBudget; /* 0x005c8de4 */
short g_bSpeechEnabledFlag; /* 0x005c8de8 */
