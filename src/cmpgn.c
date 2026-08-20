/*
 *  Campaign mission packet decoding (`cmpgn` in the Mac segment names).
 *
 *  Address range 0x404610-0x40609f (provisional -- see docs/ORDER.md).
 *  Boundary evidence: auto.c ends before 0x00404610 and brains.c begins at
 *  0x004060A0; LoadMissionData is the recovered function at 0x004059B0.
 */
#include "wc1.h"

#pragma function(memcmp)
#pragma function(memcpy)
#pragma function(strcat)

#pragma pack(push, 1)
typedef struct MissionHeaderDisk {
    short entryNavPoint;
    short homeMissionShip;
    short playerMissionShip;
    short initialMissionShips[8];
    short field_16;
} MissionHeaderDisk;

typedef struct MissionNavPointDisk {
    char name[30];
    signed char type;
    FixedVector position;
    unsigned short proximityRadius;
    signed char triggers[4][2];
    short preloadObjectTypes[2];
    short missionShips[10];
} MissionNavPointDisk;

typedef struct MissionObjectiveDisk {
    short type;
    short index;
    char description[60];
} MissionObjectiveDisk;

typedef struct MissionShipDisk {
    short type;
    short side;
    signed char leader;
    signed char field_5;
    short missionType;
    signed char navPoint;
    FixedVector position;
    short pitch;
    short yaw;
    short roll;
    signed char formationSpot;
    short speed;
    short rating;
    short pilot;
    short field_2c;
    short field_2e;
    signed char state;
    signed char leaderMissionIndex;
    signed char formationIndex;
    signed char targetMissionIndex;
} MissionShipDisk;
#pragma pack(pop)

typedef char MissionHeaderDisk_size_must_be_0x18[
    sizeof(MissionHeaderDisk) == 0x18 ? 1 : -1];
typedef char MissionNavPointDisk_size_must_be_0x4d[
    sizeof(MissionNavPointDisk) == 0x4d ? 1 : -1];
typedef char MissionObjectiveDisk_size_must_be_0x40[
    sizeof(MissionObjectiveDisk) == 0x40 ? 1 : -1];
typedef char MissionShipDisk_size_must_be_0x2a[
    sizeof(MissionShipDisk) == 0x2a ? 1 : -1];

/* Function start: WC2_UNMAPPED */
unsigned short __stdcall LoadWc1PaletteTripletsFile(const char *path)
{
    unsigned char *palette;
    FILE *file;
#ifdef WC1_SDL
    char resolvedPath[PATH_MAX];
#endif

    palette = AllocateTaggedMemory(0x300, 0);
    if (palette == 0)
        return 0;
#ifdef WC1_SDL
    if (Wc1SdlResolvePath(path, resolvedPath, sizeof(resolvedPath)))
        file = fopen(resolvedPath, "rb");
    else
        file = 0;
#else
    file = fopen(path, "rb");
#endif
    if (file != 0) {
        fseek(file, 0x30, SEEK_SET);
        fread(palette, 0x300, 1, file);
        SetWholePaletteFromTriplets(palette);
        fclose(file);
        ReleasePacketHandle(palette);
        return 1;
    }
    ReleasePacketHandle(palette);
    return 0;
}

/* Function start: 0x44EA50 */
/* Clear what an ejection leaves in space: pods take the pilot animation, the
 * 0x3D marker survives, every other transient object is removed. */
void ClearTransientObjectsForEjection(void)
{
    short obj;

    for (obj = 0; obj < WC2_SPACE_OBJECT_COUNT; obj++) {
        switch (g_aeObjectClass_00495328[obj]) {
        case OBJECT_CLASS_FUTURION:
        case OBJECT_CLASS_EXPLOSION:
        case OBJECT_CLASS_DEBRIS:
        case OBJECT_CLASS_PROJECTILE:
        case OBJECT_CLASS_MISSILE:
            if (g_asObjectType_00495298[obj] ==
                WC2_OBJECT_TYPE_EJECTION_POD) {
                if (g_aObjectTypeData_00496d30[
                        WC2_OBJECT_TYPE_EJECTION_POD].shapeSet == 0) {
                    g_aObjectTypeData_00496d30[
                        WC2_OBJECT_TYPE_EJECTION_POD].shapeSet =
                        FetchDiskPacketRetrying("pilotanm.vga", 2, 0);
                }
                g_apObjectShape_00493868[obj] =
                    g_aObjectTypeData_00496d30[
                        WC2_OBJECT_TYPE_EJECTION_POD].shapeSet;
            } else if (g_asObjectType_00495298[obj] != 0x3d) {
                remove_object(obj);
            }
            break;
        default:
            break;
        }
    }
}

/* Function start: 0x44EB4E */
/* Flag every nav-point objective at the current nav point as reached. */
void FlagCurrentNavObjectivesReached(void)
{
    short objective;

    for (objective = 0;
         objective < g_cMissionObjectiveCount_00493294;
         objective++) {
        if (g_aMissionObjectives_004932a8[objective].type == 0 &&
            g_aMissionObjectives_004932a8[objective].index ==
                g_nCurrentNavPoint_004931bc)
            flag_reached(objective, 0);
    }
}

/* Function start: 0x44EBCA */
/* End of mission: fold the results into the campaign globals, run the
 * campaign script and, unless the arcade took over, bring the cockpit and the
 * 3-Space objects back for the next leg. */
void ejection_sequence(short outcome, signed char restoreCockpit)
{
    FILE *log;
    long mainBefore;
    long farBefore;
    long mainLargestBefore;
    long farLargestBefore;
    long mainMid;
    long farMid;
    long mainLargestMid;
    long farLargestMid;
    long mainAfter;
    long farAfter;
    long mainLargestAfter;
    long farLargestAfter;
    long mainEnd;
    long farEnd;
    long mainLargestEnd;
    long farLargestEnd;
    short savedSeries;
    short ship;

    if (g_bMissionEndPending_0049da4c == 0)
        return;

    mainBefore = GetAvailableMainMemory();
    farBefore = GetAvailableFarMemory();
    mainLargestBefore = GetLargestMainMemoryBlock();
    farLargestBefore = GetLargestFreeMemoryBlock();
    if ((unsigned short)outcome == 0x75) {
        g_nArcadeState_0049d75c = 1;
        return;
    }

    g_nResourcePaletteMode_005c57e6 = 1;
    if (g_bAutopilotDebugEnabled_00499bfc != 0)
        log = fopen("mid.$$$", "w+");
    if (g_bAutopilotDebugEnabled_00499bfc != 0)
        LogMemoryStateToFile(log);
    ReleaseSpaceflightResources();
    if (g_bAutopilotDebugEnabled_00499bfc != 0)
        LogMemoryStateToFile(log);
    if (g_pCampaignGlobals_00499c94 == 0)
        LoadTemporaryCampaignGlobals();
    if (g_pCampaignGlobals_00499c94 == 0)
        ReportFatalErrorCode("021");

    savedSeries = g_pCampaignGlobals_00499c94->series;
    g_pCampaignGlobals_00499c94->series = 1000;
    g_pCampaignGlobals_00499c94->mission = outcome;
    g_pCampaignGlobals_00499c94->previousSeries =
        g_stCurrentPilotProfile_00493408.series;
    g_pCampaignGlobals_00499c94->previousMission =
        g_stCurrentPilotProfile_00493408.mission;
    g_pCampaignGlobals_00499c94->damageLevel =
        (short)calculate_damage_level();

    mainMid = GetAvailableMainMemory();
    farMid = GetAvailableFarMemory();
    mainLargestMid = GetLargestMainMemoryBlock();
    farLargestMid = GetLargestFreeMemoryBlock();
    if (g_bAutopilotDebugEnabled_00499bfc != 0)
        LogMemoryStateToFile(log);
    StoreMissionResultsInCampaignGlobals(g_pCampaignGlobals_00499c94);
    RunCampaignScript(g_nSelectedCampaignSlot_005d3bf2);
    g_cHudMessageView_005d1c37 = -1;
    if (g_bAutopilotDebugEnabled_00499bfc != 0)
        LogMemoryStateToFile(log);
    g_nArcadeState_0049d75c = g_pCampaignGlobals_00499c94->arcadeState;
    InitializeCampaignConstellationState(g_pCampaignGlobals_00499c94, 0);

    mainAfter = GetAvailableMainMemory();
    farAfter = GetAvailableFarMemory();
    mainLargestAfter = GetLargestMainMemoryBlock();
    farLargestAfter = GetLargestFreeMemoryBlock();
    ReleasePacketSlot(&g_pCampaignGlobals_00499c94);
    FadeViewportPaletteToColour(&g_stModalSourceViewport_005d2c50, 0, 1);
    ClearViewport(&g_stModalSourceViewport_005d2c50, 0);
    RestoreGamePalette();
    ResetGameTextContexts();
    for (ship = 0; ship < 10; ship++)
        g_asShipIntelState_0049b678[ship] = 0;

    if (restoreCockpit != 0 && g_nArcadeState_0049d75c == 0) {
        if (g_bAutopilotDebugEnabled_00499bfc != 0)
            LogMemoryStateToFile(log);
        /* The original passes the ship type; the callee ignores it. */
        ((void (__cdecl *)(short))InitializeCockpitResources)(
            g_nPlayerShipType_00493464);
        load_common_3Space_objects();
        init_constellation(savedSeries);
        g_b3SpaceObjectsActive_0049c8ec = 1;
        load_all_slots();
        g_bSpaceViewBufferEnabled_0049d7a4 = 1;
        initialize_view_buffer();
        ClearTransientObjectsForEjection();
        if (g_pPilotStatus_005d2fcc[
                g_acShipPortrait_00495d88[g_nYourWingman_0049346c]] == 0)
            remove_object(g_nYourWingman_0049346c);
        g_cScreenViewportMode_005c82a6 = -1;
        force_view(0, 0);
        if (g_bAutopilotDebugEnabled_00499bfc != 0)
            LogMemoryStateToFile(log);
    }

    g_nResourcePaletteMode_005c57e6 = 0;
    InitializeSpaceFlightInput();
    ReleaseSceneMusicPacket();

    if (g_nShowMemoryStatus_0049d784 != 0) {
        mainEnd = GetAvailableMainMemory();
        farEnd = GetAvailableFarMemory();
        mainLargestEnd = GetLargestMainMemoryBlock();
        farLargestEnd = GetLargestFreeMemoryBlock();
        printf("      before  large    after   large\n");
        printf("Main: %6ld  %6ld   %6ld  %6ld\n", mainBefore,
               mainLargestBefore, mainEnd, mainLargestEnd);
        printf("EMS:  %7ld %6ld   %7ld %6ld\n", farBefore,
               farLargestBefore, farEnd, farLargestEnd);
        printf("CMem: %6ld  %6ld   %6ld  %6ld\n", mainMid,
               mainLargestMid, mainAfter, mainLargestAfter);
        printf("EEMS: %7ld %6ld   %7ld %6ld\n", farAfter,
               farLargestAfter, farAfter, farLargestAfter);
        WaitForAnyInputPress();
    }
    FlagCurrentNavObjectivesReached();
    if (g_bHighMemoryBuffersReady_005d2ad8 != 0)
        ResetCannedSceneRecording();
    if (log != 0 && g_bAutopilotDebugEnabled_00499bfc != 0)
        fclose(log);
}

/* Function start: WC2_UNMAPPED */
unsigned int ParseFaceAnimation(char *text, short *commands)
{
    char duration[8];
    char *durationCursor;
    short frame;
    short sequenceIndex;

    sequenceIndex = 0;
    frame = (short)*text++;
    while (frame != 0) {
        if (frame == 'R') {
            *commands = -2;
            commands += 2;
            commands[-1] = sequenceIndex;
        } else {
            if (frame >= 'A' && frame <= 'F')
                frame = (short)(frame - 'A' + 10);
            else
                frame = (short)(frame - '0');
            *commands = frame;
            durationCursor = duration;
            frame = (short)*text++;
            while (frame != ',') {
                *durationCursor++ = (char)frame;
                frame = (short)*text++;
            }
            *durationCursor = '\0';
            commands[1] = (short)atoi(duration);
            commands += 2;
            sequenceIndex++;
        }
        frame = (short)*text++;
    }
    *commands = -1;
    return 0;
}

/* Function start: WC2_UNMAPPED */
unsigned int ParseMouthAnimation(char *text, short *commands)
{
    char character;
    char duration[5] = "";
    short durationLength;
    short ticks;

    while ((character = *text++) != '\0') {
        if (character == '$') {
            *commands = 9;
        } else if (character <= 'z' && character >= 'a') {
            *commands = g_asMouthFramesByPhoneme_004655f0_WC1_UNMAPPED[
                character - 'a'];
        } else {
            continue;
        }
        ticks = 1;
        durationLength = 0;
        while (*text <= '9') {
            character = *text;
            if (character < '0')
                break;
            duration[durationLength] = character;
            durationLength++;
            text++;
            duration[durationLength] = '\0';
            ticks = (short)atoi(duration);
            character = *text;
        }
        commands[1] = ticks;
        commands += 2;
    }
    *commands = -1;
    return 0;
}

/* Function start: WC2_UNMAPPED */
char *AddPCName(const char *text)
{
    char formatted[12];
    const char *marker;
    char *output;
    short length;

    g_szTextScratchBuffer_005d1c40[0] = '\0';
    for (;;) {
        marker = DosStrchr(text, '$');
        if (marker == 0) {
            DosStrcat(g_szTextScratchBuffer_005d1c40, text);
            return g_szTextScratchBuffer_005d1c40;
        }
        output = DosStrchr(g_szTextScratchBuffer_005d1c40, 0);
        while (text != marker) {
            *output++ = *text++;
        }
        *output = '\0';
        marker++;
        switch (*marker++) {
        case 'A':
            DosStrcat(g_szTextScratchBuffer_005d1c40,
                      g_apszMedalNames_0046e2e0_WC1_UNMAPPED[
                          g_nConversationMedalIndex_00598c08]);
            break;
        case 'C':
            DosStrcat(g_szTextScratchBuffer_005d1c40,
                      g_stCampaignState_0059ca50.currentPilot->callsign);
            break;
        case 'D':
            sprintf(formatted, g_szCampaignDateFormat_00465630_WC1_UNMAPPED,
                    g_pCurrentCampaignDate_005a86a8->year,
                    g_pCurrentCampaignDate_005a86a8->day);
            DosStrcat(g_szTextScratchBuffer_005d1c40, formatted);
            break;
        case 'E':
            sprintf(formatted, g_szSavedCampaignDateFormat_0046563c_WC1_UNMAPPED,
                    g_stSavedCampaignDate_0046e188_WC1_UNMAPPED.year,
                    g_stSavedCampaignDate_0046e188_WC1_UNMAPPED.day);
            DosStrcat(g_szTextScratchBuffer_005d1c40, formatted);
            break;
        case 'K':
            sprintf(formatted, g_szConversationIntegerFormat_00465628_WC1_UNMAPPED,
                    g_cPlayerKillCount_005d2fa8);
            DosStrcat(g_szTextScratchBuffer_005d1c40, formatted);
            break;
        case 'L':
            sprintf(formatted,
                    g_szConversationIntegerFormatAlt_0046562c_WC1_UNMAPPED,
                    g_nWingmanKillCount_005a7cb8);
            DosStrcat(g_szTextScratchBuffer_005d1c40, formatted);
            break;
        case 'N':
        case 'P':
            DosStrcat(g_szTextScratchBuffer_005d1c40,
                      g_stCampaignState_0059ca50.currentPilot->name);
            break;
        case 'R':
            DosStrcat(g_szTextScratchBuffer_005d1c40,
                      g_apszPilotRankNames_0049a608[
                          g_stCampaignState_0059ca50.currentPilot->rank]);
            length = DosStrlen(g_szTextScratchBuffer_005d1c40);
            if (g_szTextScratchBuffer_005d1c40[length - 1] == '.' &&
                *marker == '.')
                g_szTextScratchBuffer_005d1c40[
                    DosStrlen(g_szTextScratchBuffer_005d1c40) - 1] = '\0';
            break;
        case 'S':
            DosStrcat(g_szTextScratchBuffer_005d1c40,
                      (char *)g_abSeriesAuxData_005d3c40);
            break;
        case 'T':
            sprintf(formatted, g_szCampaignTimeFormat_00465648_WC1_UNMAPPED,
                    (int)((signed char *)g_pElapsedCampaignDate_005d3e8c)[0],
                    (int)((signed char *)g_pElapsedCampaignDate_005d3e8c)[1]);
            DosStrcat(g_szTextScratchBuffer_005d1c40, formatted);
            break;
        case 'W':
            DosStrcat(g_szTextScratchBuffer_005d1c40,
                      g_apWingmanPilots_00598a30[*marker++ - '0']->name);
            break;
        }
        text = marker;
    }
}

/* Function start: 0x4098F2 */
short RunCampaignContinuePromptLoop(unsigned char *promptShapes,
                                    unsigned char *fieldShape,
                                    short promptFrame)
{
    short selection;

    selection = 0;
    while (selection == 0) {
        selection = PollSceneHotspotInput(promptShapes, 0, 0, 0, 0);
        if (selection == 0) {
            DisableMouseCursorDrawing();
            DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                              0, 0, fieldShape, 0);
            DrawConstellationField();
            DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                              160, 66, promptShapes, promptFrame);
            DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                              0, 126, promptShapes, 2);
            EnableMouseCursorDrawing();
            WaitForVerticalBlankThunk();
            CopyViewportContents(&g_stSecondaryViewBuffer_005d2c90,
                                 &g_stScreenViewport_005d21a0);
        }
    }
    return selection;
}

/* Function start: 0x4099A8 */
short PromptToContinueCampaign(short promptFrame)
{
    unsigned char *promptShapes;
    unsigned char *fieldShape;
    short selection;

    promptShapes = 0;
    fieldShape = 0;
    selection = 0;
    StopMusicUnlessSuppressed();
    free_view_buffer();
    g_stSecondaryViewBuffer_005d2c90.left = 0;
    g_stSecondaryViewBuffer_005d2c90.right = 319;
    g_stSecondaryViewBuffer_005d2c90.top = 0;
    g_stSecondaryViewBuffer_005d2c90.bottom = 199;
    if ((short)AllocateViewport(
            &g_stSecondaryViewBuffer_005d2c90,
            g_cSecondaryViewBufferColour_0049cb4c, 0) == 0) {
        ReportFatalErrorCode("016");
    }
    ClearViewport(&g_stSecondaryViewBuffer_005d2c90,
                  g_cSecondaryViewBufferColour_0049cb4c);
    init_constellation(0);
    InitializeConstellationField(&g_stSecondaryViewBuffer_005d2c90,
                                 0, 16);
    SetMenuInputPump();
    SetInputViewport(&g_stSecondaryViewBuffer_005d2c90);
    EnableMouseCursorDrawing();
    promptShapes = FetchDiskPacketRetrying("continue.vga", 0, 0);
    fieldShape = FetchDiskPacketRetrying("field.v00", 1, 0);
    while (selection == 0) {
        switch (selection = RunCampaignContinuePromptLoop(
                    promptShapes, fieldShape, promptFrame)) {
        case 1:
            FreePacketAndClear(&promptShapes, 0);
            FreePacketAndClear(&fieldShape, 0);
            free_viewport(&g_stSecondaryViewBuffer_005d2c90);
            FreePacketAndClear(&g_pConstellationShape_005d2c4c, 0);
            ClearInputPump();
            DisableMouseCursorDrawing();
            return 1;
        case 2:
            FreePacketAndClear(&promptShapes, 0);
            FreePacketAndClear(&fieldShape, 0);
            free_viewport(&g_stSecondaryViewBuffer_005d2c90);
            FreePacketAndClear(&g_pConstellationShape_005d2c4c, 0);
            ClearInputPump();
            DisableMouseCursorDrawing();
            return 0;
        }
    }
    return selection;
}

/* Function start: WC2_UNMAPPED */
unsigned int LoadFace(short face)
{
    switch (g_nConversationCharacter_0046e580_WC1_UNMAPPED) {
    case 0:
        g_nConversationBackdropFrame_0046e588_WC1_UNMAPPED = 4;
        break;
    case 1:
        g_nConversationBackdropFrame_0046e588_WC1_UNMAPPED = 5;
        break;
    case 2:
        g_nConversationBackdropFrame_0046e588_WC1_UNMAPPED = 0;
        init_constellation(0);
        g_stConstellationViewport_005a6b40 = g_stSecondaryViewBuffer_005d2c90;
        g_stConstellationViewport_005a6b40.bottom = 76;
        InitializeConstellationField(&g_stConstellationViewport_005a6b40,
                                     -1, 16);
        g_bConversationConstellation_0046e58c_WC1_UNMAPPED = 1;
        break;
    case 4:
        g_nConversationBackdropFrame_0046e588_WC1_UNMAPPED = 2;
        break;
    case 8:
        g_nConversationBackdropFrame_0046e588_WC1_UNMAPPED = 1;
        init_constellation(0);
        InitializeConstellationField(&g_stSecondaryViewBuffer_005d2c90, -1, 16);
        g_bConversationConstellation_0046e58c_WC1_UNMAPPED = 1;
        break;
    case 9:
        g_nConversationBackdropFrame_0046e588_WC1_UNMAPPED = 2;
        break;
    case 10:
    case 12:
        g_nConversationBackdropFrame_0046e588_WC1_UNMAPPED = 0;
        break;
    case 3:
    case 11:
    case 13:
        g_nConversationBackdropFrame_0046e588_WC1_UNMAPPED = 1;
        break;
    default:
        g_nConversationBackdropFrame_0046e588_WC1_UNMAPPED = -1;
        break;
    }
    if (face != g_nTalkingHeadFace_0046e584_WC1_UNMAPPED &&
        g_pTalkingHeadShape_00598c0c != 0)
        FreePacketAndClear(&g_pTalkingHeadShape_00598c0c, 0);
    if (g_pTalkingHeadShape_00598c0c == 0)
        g_pTalkingHeadShape_00598c0c =
            FetchDiskPacketRetrying(6, face, 0);
    g_nTalkingHeadFace_0046e584_WC1_UNMAPPED = face;
    if (g_pConversationOverlayShape_00598c30 == 0)
        g_pConversationOverlayShape_00598c30 =
            FetchDiskPacketRetrying(6, 11, 0);
    g_nTalkingHeadFaceX_005a8754 =
        g_aTalkingHeadOrigins_0046e190_WC1_UNMAPPED[face].faceX;
    g_nTalkingHeadFaceY_005a8756 =
        g_aTalkingHeadOrigins_0046e190_WC1_UNMAPPED[face].faceY;
    g_nTalkingHeadMouthX_005a875a =
        g_aTalkingHeadOrigins_0046e190_WC1_UNMAPPED[face].mouthX;
    g_nTalkingHeadMouthY_005a8758 =
        g_aTalkingHeadOrigins_0046e190_WC1_UNMAPPED[face].mouthY;
    CloseTalk(g_pTalkingHeadShape_00598c0c, -1, -1);
    return 0;
}

/* Function start: WC2_UNMAPPED */
unsigned int LongTalk(unsigned char *talker, char *text,
                      short *mouthCommands, short *faceCommands,
                      short duration)
{
    short *faceStart;
    short *mouthStart;
    short faceCountdown;
    short faceFrame;
    short mouthCountdown;
    short mouthFrame;
    short waiting;

    waiting = 0;
    AddPCName(text);
    faceStart = faceCommands;
    mouthStart = mouthCommands;
    faceFrame = 0;
    ClearViewport(&g_stConversationTextViewport_005d2b60,
                  g_cSecondaryViewBufferColour_0049cb4c);
    mouthFrame = 0;
    faceCountdown = 0;
    mouthCountdown = 0;
    FormatTextBufferFromStart(g_szConversationTextFormat_00465654_WC1_UNMAPPED,
                              0, 160,
                              g_nConversationTextColour_00598c10,
                              g_szTextScratchBuffer_005d1c40);
    g_nFrameSkipCountdown_0049d760 = 1;
    for (;;) {
        if (*mouthCommands == -1 && *faceCommands == -1) {
            if (waiting == 0) {
                CloseTalk(talker, -1, -1);
                MarkDibDirty();
                DIBslamReal();
                WaitForWc1SceneAdvance(duration, 0);
                return 0;
            }
            IsFrameTickElapsed();
            return 0;
        }
        if (mouthCountdown-- == 0) {
            if (*mouthCommands != -1)
                mouthCommands += 2;
            switch (*mouthCommands) {
            case -2:
                mouthCommands = mouthStart;
            default:
                mouthFrame = *mouthCommands;
                mouthCountdown = (short)(mouthCommands[1] * 2);
                break;
            case -1:
                mouthFrame = -1;
                if (waiting == 0) {
                    waiting++;
                    SetFrameTimerPeriodDirect(duration);
                }
                break;
            }
        }
        if (faceCountdown-- == 0) {
            if (*faceCommands != -1)
                faceCommands += 2;
            switch (*faceCommands) {
            case -2:
                faceCommands = faceStart;
            default:
                faceFrame = *faceCommands;
                if (faceFrame == 10)
                    faceFrame = -1;
                faceCountdown = (short)(faceCommands[1] * 2);
                break;
            case -1:
                faceFrame = -1;
                break;
            }
        }
        g_nFrameSkipCountdown_0049d760--;
        if (g_nFrameSkipCountdown_0049d760 < 1) {
            g_nFrameSkipCountdown_0049d760 = g_nFrameSkip_0049d764;
            CloseTalk(talker, mouthFrame, faceFrame);
            MarkDibDirty();
            DIBslamReal();
        }
        if (CheckEscaped() != 0)
            break;
        if (waiting != 0 && (short)IsFrameTickElapsed() != 0)
            return 0;
    }
    do {
    } while (CheckEscaped() != 0);
    return 0;
}

/* Function start: WC2_UNMAPPED */
unsigned int CloseTalk(unsigned char *talker, short mouthFrame,
                       short faceFrame)
{
    if (g_bConversationConstellation_0046e58c_WC1_UNMAPPED == 1)
        DrawConstellationField();
    switch (g_nConversationSceneType_00598c0a) {
    case 0:
    case 1:
    case 2:
    case 4:
    case 5:
        if (g_nConversationBackdropFrame_0046e588_WC1_UNMAPPED != -1) {
            DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 0, 0,
                              g_pConversationBackdropShape_00598c04,
                              g_nConversationBackdropFrame_0046e588_WC1_UNMAPPED);
            break;
        }
        ClearViewport(&g_stSecondaryViewBuffer_005d2c90, g_cSecondaryViewBufferColour_0049cb4c);
        break;
    case 3:
        ClearViewport(&g_stSecondaryViewBuffer_005d2c90, g_cPrimaryViewBufferColour_0049cb88);
        break;
    default:
        ClearViewport(&g_stSecondaryViewBuffer_005d2c90, g_cSecondaryViewBufferColour_0049cb4c);
        break;
    }
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 0, 0, talker, 0);
    if (faceFrame > -1)
        DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                          g_nTalkingHeadFaceX_005a8754,
                          g_nTalkingHeadFaceY_005a8756,
                          talker, (short)(faceFrame + 11));
    if (mouthFrame > -1)
        DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                          g_nTalkingHeadMouthX_005a875a,
                          g_nTalkingHeadMouthY_005a8758,
                          talker, (short)(mouthFrame + 1));
    if (g_bConversationOverlay_0046e590_WC1_UNMAPPED != 0)
        DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 0, 0,
                          g_pConversationOverlayShape_00598c30,
                          MinShort(g_nTalkingHeadFace_0046e584_WC1_UNMAPPED, 1));
    switch (g_nConversationCharacter_0046e580_WC1_UNMAPPED) {
    case 5:
        DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 0, 0,
                          g_pConversationSpecialShape_005a86ec, 10);
        RefreshMemoryStatusOverlay();
        return 0;
    case 6:
        DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 0, 0,
                          g_pConversationSpecialShape_005a86ec, 10);
        DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 0, 0,
                          g_pConversationSpecialShape_005a86ec, 11);
        RefreshMemoryStatusOverlay();
        return 0;
    }
    RefreshMemoryStatusOverlay();
    return 0;
}

/* Function start: WC2_UNMAPPED */
unsigned int Briefing(short series, short mission)
{
    g_bSceneEscapeRequested_0049d4b0 = 0;
    PreloadMusicTrackHook(0x18);
    PreloadMusicTrackHook(0x19);
    PreloadMusicTrackHook(0x1a);
    LoadMissionData(series, mission);
    LoadBriefingData(series, mission);
    if (g_bSceneEscapeRequested_0049d4b0 == 0) {
        Build_objective_list();
        LoadBriefingRoom();
    }
    g_bSceneEscapeRequested_0049d4b0 = 0;
    ReleasePacketHandle(g_pBriefingPacket_00598aec);
    ReleaseMusicTrackHook(0x18);
    ReleaseMusicTrackHook(0x19);
    ReleaseMusicTrackHook(0x1a);
    return 0;
}

/* Function start: 0x424E8C */
void MarkPilotDead(short pilot)
{
    g_pPilotStatus_005d2fcc[pilot] = 0;
}

/* Function start: 0x424EA7 */
void clean_up_cockpit(void)
{
    g_bTargetLockMode_00493500 = 0;
    g_acShipTarget_00495f20[0] = -1;
    if (g_nYourWingman_0049346c != -1) {
        g_acShipTarget_00495f20[g_nYourWingman_0049346c] = -1;
        g_nAutoEngageTimer_00496130 = -1;
        reset_objective(g_nYourWingman_0049346c,
                        OBJECTIVE_HOLD_FORMATION);
    }
    ClearHudGunReadouts();
}

/* Function start: 0x4251F2 */
void CompleteStarSystemJump(void)
{
    short destinationNavPoint;
    short previousDelay;

    previousDelay = g_nStarSystemJumpDelay_004962f4;
    g_nStarSystemJumpDelay_004962f4++;
    if (previousDelay < 75) {
        g_asObjectCounter_00494be0[0] = 1;
    } else {
        FadeViewportPaletteToColour(
            &g_stScreenViewport_005d21a0,
            g_cSecondaryViewBufferColour_0049cb4c, 1);
        ClearViewport(&g_stScreenViewport_005d21a0,
                      g_cSecondaryViewBufferColour_0049cb4c);
        RestoreGamePalette();

        for (destinationNavPoint = 0; destinationNavPoint < 10;
             destinationNavPoint++) {
            if (g_aMissionNavPoints_00491e98[destinationNavPoint].type != 0 &&
                g_aMissionNavPoints_00491e98[destinationNavPoint]
                        .systemIndex != g_nCurrentStarSystem_005d169c &&
                g_aMissionNavPoints_00491e98[destinationNavPoint].field_2e !=
                    0) {
                break;
            }
        }

        g_anShipFuel_00495638[0] -= g_anShipFuel_00495638[0] / 2;
        g_aeObjectClass_00495328[0] = OBJECT_CLASS_SHIP;
        new_view(0, 0);
        g_bAutopilotSequenceActive_00493064 = 1;
        set_up_action_sphere(destinationNavPoint);
        g_bAutopilotSequenceActive_00493064 = 0;
        g_aShipPosition_00494550[0] =
            g_aMissionNavPoints_00491e98[destinationNavPoint].position;
        g_aShipPosition_00494550[0].x +=
            g_aMissionNavPoints_00491e98[destinationNavPoint]
                .proximityRadius;
        g_asObjectScale_00494d90[0] =
            g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[0]].scale;
        g_bStarSystemJumpTransition_004962ec = 0;
        g_asShipManeuver_00495f48[0] = MANEUVER_NONE;
        g_asObjectCounter_00494be0[0] =
            (short)g_nSavedPlayerObjectCounter_005d2fe0;

        if (g_nYourWingman_0049346c != -1) {
            g_anShipFuel_00495638[g_nYourWingman_0049346c] -=
                g_anShipFuel_00495638[g_nYourWingman_0049346c] / 2;
            g_asObjectCounter_00494be0[g_nYourWingman_0049346c] =
                (short)g_nSavedWingmanObjectCounter_005d2fdc;
            g_asShipMissionType_00495de8[g_nYourWingman_0049346c] =
                MISSION_TYPE_WINGMAN;
            g_aeShipObjective_00495f08[g_nYourWingman_0049346c] =
                OBJECTIVE_HOLD_FORMATION;
            g_aShipPosition_00494550[g_nYourWingman_0049346c] =
                g_aShipPosition_00494550[0];
            g_anShipSpeed_00494e20[g_nYourWingman_0049346c] =
                g_anShipSpeed_00494e20[0];
            g_asObjectScale_00494d90[g_nYourWingman_0049346c] =
                g_aObjectTypeData_00496d30[
                    g_acObjectType_00493980[
                        g_nYourWingman_0049346c]].scale;
        }

        ClearViewport(&g_stViewBuffer_005d2b00,
                      g_bPrimaryViewBufferColour_0049cb50);
        g_bViewportDirty_0049d76c = 1;
        g_bJumpSequenceActive_004962f0 = 0;
        if (g_bHighMemoryBuffersReady_005d2ad8 != 0)
            ResetCannedSceneRecording();
    }
}

/* Function start: 0x409B80 */
void ReleaseSpaceflightResources(void)
{
    short font;

    remove_nav_pointer();
    g_bCockpitResourcesActive_0049c8e8 = 1;
    free_cockpit();
    free_all_slots();
    free_3Space();
    for (font = 0; font < 4; font++) {
        if (g_apTextFonts_005d2200[font] != 0) {
            ReleasePacketHandle(g_apTextFonts_005d2200[font]);
            g_apTextFonts_005d2200[font] = 0;
        }
    }
    if (g_nShowMemoryStatus_0049d784 != 0) {
        ShowMemoryStatusDebug();
        WaitForAnyInputPress();
    }
}

/* Function start: WC2_UNMAPPED */
unsigned int LoadBriefingData(short series, short mission)
{
    BriefingPacketHeader *header;

    g_pBriefingPacket_00598aec = FetchDiskPacketRetrying(
        g_asCampaignBriefingFiles_00469458_WC1_UNMAPPED[g_nCampaignDataSet_005a8118],
        (short)(mission + series * 4), 0);
    header = (BriefingPacketHeader *)g_pBriefingPacket_00598aec;
    g_pBriefingSceneData_00598c00 = g_pBriefingPacket_00598aec +
        header->briefingScene;
    g_pBriefingTextData_00598af0 = g_pBriefingPacket_00598aec +
        header->briefingText;
    g_pDebriefingSceneData_00598afc = g_pBriefingPacket_00598aec +
        header->debriefingScene;
    g_pDebriefingTextData_00598c28 = g_pBriefingPacket_00598aec +
        header->debriefingText;
    g_apRecRoomSceneData_00598ae0[0] = g_pBriefingPacket_00598aec +
        header->recRoomScene0;
    g_apRecRoomTextData_00598aa0[0] = g_pBriefingPacket_00598aec +
        header->recRoomText0;
    g_apRecRoomSceneData_00598ae0[2] = g_pBriefingPacket_00598aec +
        header->recRoomScene2;
    g_apRecRoomTextData_00598aa0[2] = g_pBriefingPacket_00598aec +
        header->recRoomText2;
    g_apRecRoomSceneData_00598ae0[1] = g_pBriefingPacket_00598aec +
        header->recRoomScene1;
    g_apRecRoomTextData_00598aa0[1] = g_pBriefingPacket_00598aec +
        header->recRoomText1;
    return 0;
}

/* Function start: 0x4401C0 */
void LoadMissionData(short series, short mission)
{
    unsigned char *packet;
    unsigned char *source;
    int offset;
    char campaignSlot[4];
    char moduleName[13] = "module.00";

    strcat(moduleName,
           _itoa((int)g_nSelectedCampaignSlot_005d3bf2,
                 campaignSlot, 10));

    packet = FetchDiskPacketRetrying(moduleName, 0, 0);
    offset = series * 0x60;
    offset += mission * 0x18;
    source = packet + offset;
    memcpy(&g_stMissionHeader_005d3e70, source, 0x18);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(moduleName, 1, 0);
    offset = series * 0xfc8;
    offset += mission * 0x3f2;
    source = packet + offset;
    memcpy(g_aMissionNavPoints_00491e98, source, 0x3f2);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(moduleName, 2, 0);
    offset = series * 0x800;
    offset += mission * 0x200;
    source = packet + offset;
    memcpy(g_aMissionObjectiveSources_005d3c70, source, 0x200);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(moduleName, 3, 0);
    offset = series * 0xf00;
    offset += mission * 0x3c0;
    source = packet + offset;
    memcpy(g_aMissionShips_00492290, source, 0x3c0);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(moduleName, 4, 0);
    offset = series * 0xa0;
    offset += mission * 0x28;
    source = packet + offset;
    memcpy(g_abMissionAuxData_005d3c10, source, 0x28);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(moduleName, 5, 0);
    offset = series * 0x28;
    source = packet + offset;
    memcpy(g_abSeriesAuxData_005d3c40, source, 0x28);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(moduleName, 6, 0);
    offset = series * 0x378;
    offset += mission * 0xde;
    source = packet + offset;
    memcpy(g_abMissionBriefingData_005d3e90, source, 0xde);
    ReleasePacketHandle(packet);
}

/* Function start: 0x42ECCB */
void UpdateMap(char *text, short objective)
{
    int deadline;

    ClearCutsceneViewport(&g_stModalSourceViewport_005d2c50, 0);
    g_bBriefingMapActive_0049bcb0 = 1;
    LoadMissionData(g_pCampaignGlobals_00499c94->series,
                    g_pCampaignGlobals_00499c94->mission);
    Build_objective_list();
    g_cCurrentObjective_004931cc = (signed char)objective;
    g_pActiveCutscenePixels_005c83dc = 0;
    BriefingMap_DisplayMap();
    g_bBriefingMapActive_0049bcb0 = 0;
    RouteCutsceneViewportToDisplay();
    SetTextContext(&g_stCutsceneTextContext_005d2f40);
    ClearCutsceneTextViewport();
    ExpandCutsceneText((const unsigned char *)text,
                       g_pszCutsceneWorkBuffer_005d2ecc);
    if (IsCutsceneSpeechLoaded() == 0 || g_nAudioEnabled_0049c244 == 0) {
        if (memcmp(g_pszCutsceneWorkBuffer_005d2ecc, "50", 2) != 0)
            FormatTextBufferFromStart("%s%P", g_pszCutsceneWorkBuffer_005d2ecc);
    }
    g_bCutsceneTextRestorePending_00499da0 = 1;
    RestoreCutsceneTextBacking();
    ServiceInputDevices(-1);
    deadline = g_nInputClock_005c84a8 + 600;
    while ((int)g_nInputClock_005c84a8 < deadline &&
           g_bCutsceneSkipFrame_00499c54 == 0 &&
           g_bCutsceneViewportPreallocated_00499c4c == 0) {
        if (ServiceInputDevices(-1) != 0) {
            ServiceCutsceneRuntimeHook();
            if (FindQueuedInputEvent(2) != 0 || FindQueuedInputEvent(5) != 0)
                break;
        }
    }
    ClearCutsceneViewport(&g_stModalSourceViewport_005d2c50, 0);
}

/* Function start: WC2_UNMAPPED */
unsigned int CloseLook(unsigned char *shape, short shot,
                       short *animation, char *text, short duration,
                       short unused)
{
    short character;
    short countdown;
    short escaped;
    short finished;
    short frame;
    short sceneFrame;
    short *cursor;
    short *start;

    finished = 0;
    sceneFrame = 0;
    AddPCName(text);
    ClearViewport(&g_stConversationTextViewport_005d2b60,
                  g_cSecondaryViewBufferColour_0049cb4c);
    FormatTextBufferFromStart(g_szCloseLookTextFormat_0046566c_WC1_UNMAPPED,
                              0, 160,
                              g_nConversationTextColour_00598c10,
                              g_szTextScratchBuffer_005d1c40);
    g_nFrameSkipCountdown_0049d760 = 1;
    cursor = animation;
    start = animation;
    if (shot == 2 || shot == 11) {
        if (*cursor != -1) {
            countdown = 0;
            do {
                if (countdown-- == 0) {
                    if (*cursor != -1)
                        cursor += 2;
                    if (*cursor == -2)
                        cursor = start;
                    else if (*cursor == -1) {
                        frame = -1;
                        if (finished == 0) {
                            finished = 1;
                            SetFrameTimerPeriodDirect(duration);
                        }
                    } else {
                        frame = *cursor;
                        countdown = (short)(cursor[1] * 2);
                    }
                }
                g_nFrameSkipCountdown_0049d760--;
                if (g_nFrameSkipCountdown_0049d760 < 1) {
                    g_nFrameSkipCountdown_0049d760 = g_nFrameSkip_0049d764;
                    if (shot == 11) {
                        DrawDebriefingLongShot();
                        if (frame > -1)
                            DrawSpriteDefault(
                                &g_stSecondaryViewBuffer_005d2c90,
                                g_nDebriefingPodiumX_0046e57c_WC1_UNMAPPED, 53,
                                g_pConversationBackdropShape_00598c04,
                                (short)(frame + 17));
                    } else if (frame > -1) {
                        DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 225, 34,
                                          shape, frame);
                    }
                    RefreshMemoryStatusOverlay();
                    MarkDibDirty();
                    DIBslamReal();
                }
                escaped = CheckEscaped();
                if (escaped != 0) {
                    do {
                        escaped = CheckEscaped();
                    } while (escaped != 0);
                    return 0;
                }
                if (finished != 0 && IsFrameTickElapsed() != 0)
                    return 0;
                MarkDibDirty();
                DIBslamReal();
            } while (*cursor != -1);
        }
    } else if (shot == 0) {
        for (; sceneFrame < 22; sceneFrame++) {
            g_nFrameSkipCountdown_0049d760--;
            if (g_nFrameSkipCountdown_0049d760 < 1) {
                g_nFrameSkipCountdown_0049d760 = g_nFrameSkip_0049d764;
                DrawSpriteDefault(
                    &g_stSecondaryViewBuffer_005d2c90, 0, 0,
                    g_pConversationBackdropShape_00598c04, 0);
                DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 241, 60,
                                  g_pBriefingAnimationShape_00598c14,
                                  sceneFrame);
                DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 241, 64,
                                  g_pBriefingAnimationShape_00598c14, 22);
                for (character = 0; character < 14; character++) {
                    RecordCannedSceneBriefingCharacter(
                        character, 0,
                        g_aBriefingCharacters_0046e218_WC1_UNMAPPED[character]
                            .animation[sceneFrame]);
                }
                RefreshMemoryStatusOverlay();
                MarkDibDirty();
                DIBslamReal();
            }
            if (CheckEscaped() != 0)
                sceneFrame = 20;
            if (sceneFrame == 20)
                g_nFrameSkipCountdown_0049d760 = 1;
            MarkDibDirty();
            DIBslamReal();
        }
    }
    MarkDibDirty();
    DIBslamReal();
    WaitForWc1SceneAdvance(duration, unused);
    return 0;
}
