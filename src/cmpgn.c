/*
 *  Campaign mission packet decoding (`cmpgn` in the Mac segment names).
 *
 *  Address range 0x404610-0x40609f (provisional -- see docs/ORDER.md).
 *  Boundary evidence: auto.c ends before 0x00404610 and brains.c begins at
 *  0x004060A0; LoadMissionData is the recovered function at 0x004059B0.
 */
#include "game.h"

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

/* Function start: 0x44EA50 */
/* Clear what an ejection leaves in space: pods take the pilot animation, the
 * 0x3D marker survives, every other transient object is removed. */
void ClearTransientObjectsForEjection(void)
{
    short obj;

    for (obj = 0; obj < SPACE_OBJECT_COUNT; obj++) {
        switch (g_aeObjectClass_00495328[obj]) {
        case OBJECT_CLASS_FUTURION:
        case OBJECT_CLASS_EXPLOSION:
        case OBJECT_CLASS_DEBRIS:
        case OBJECT_CLASS_PROJECTILE:
        case OBJECT_CLASS_MISSILE:
            if (g_asObjectType_00495298[obj] ==
                OBJECT_DATA_EJECTION_POD) {
                if (g_aObjectTypeData_00496d30[
                        OBJECT_DATA_EJECTION_POD].shapeSet == 0) {
                    g_aObjectTypeData_00496d30[
                        OBJECT_DATA_EJECTION_POD].shapeSet =
                        FetchDiskPacketRetrying("pilotanm.vga", 2, 0);
                }
                g_apObjectShape_00493868[obj] =
                    g_aObjectTypeData_00496d30[
                        OBJECT_DATA_EJECTION_POD].shapeSet;
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
        InitializeCockpitResources(g_nPlayerShipType_00493464);
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

/* Function start: 0x424EFF */
void TryBeginStarSystemJump(void)
{
    short object;

    if (g_asObjectType_00495298[0] == 0x33 &&
        g_bJumpSequenceActive_004962f0 == 0) {
        for (object = 0; object < 10; object++) {
            if (g_aeObjectClass_00495328[object] == OBJECT_CLASS_SHIP &&
                g_asShipSide_004955d0[object] == SIDE_KILRATHI) {
                set_global_message(
                    "Enemy near",
                    g_abGamePaletteReservedColours_0049cb54[4], 3);
                return;
            }
        }
        if (g_abJumpDriveUsedBySystem_005d2fe8[
                g_nCurrentStarSystem_005d169c] != 0) {
            set_global_message(
                "Capacitors Empty",
                g_abGamePaletteReservedColours_0049cb54[4], 3);
            return;
        }
        for (object = 1; object < 10; object++) {
            if (g_aeObjectClass_00495328[object] >=
                    OBJECT_CLASS_CAPITAL_SHIP &&
                g_asShipMissionType_00495de8[object] ==
                    MISSION_TYPE_GOTO_WARP &&
                g_asShipMissionParameter_00495e00[object] ==
                    g_nCurrentNavPoint_004931bc &&
                equ_vector(
                    &g_aMissionObjectives_004932a8[
                        g_cCurrentObjective_004931cc].position,
                    &g_aShipDestination_004953f0[object]) == 0) {
                sprintf(g_pszAutopilotWaitReason_0049b050,
                        "Wait for %s",
                        g_apShipMissionRecord_00495da8[object]->name);
                set_global_message(
                    g_pszAutopilotWaitReason_0049b050,
                    g_abGamePaletteReservedColours_0049cb54[4], 3);
                return;
            }
        }
        if (g_aMissionNavPoints_00491e98[
                g_nCurrentNavPoint_004931bc].field_2e == 0) {
            set_global_message(
                "No jump point",
                g_abGamePaletteReservedColours_0049cb54[4], 3);
            return;
        }
        if (g_ucPendingEjectionTransition_0049b8ac != 0xff &&
            g_nPendingEjectionSequenceCount_0049b8b8 != 0) {
            g_nEjectionSequenceState_0049b8c0 = 0;
            g_nPendingEjectionSequenceCount_0049b8b8 = 0;
            ejection_sequence(g_ucPendingEjectionTransition_0049b8ac, 1);
            g_ucPendingEjectionTransition_0049b8ac = 0xff;
        }
        spacetrack(0x2f, 1, 0);
        force_view(0x0d, 0);
        g_bStarSystemJumpTransition_004962ec = 1;
        g_abJumpDriveUsedBySystem_005d2fe8[
            g_nCurrentStarSystem_005d169c]++;
        g_bJumpSequenceActive_004962f0 = 1;
        g_nStarSystemJumpDelay_004962f4 = 0;
        g_nSavedPlayerObjectCounter_005d2fe0 =
            g_asObjectCounter_00494be0[0];
        g_asShipMissionType_00495de8[0] = MISSION_TYPE_GOTO_WARP;
        g_asShipCount_00495ff8[0] = 0;
        g_asShipTactic_00495f30[0] = TACTIC_SIT_STILL;
        if (g_nYourWingman_0049346c != -1) {
            g_nSavedWingmanObjectCounter_005d2fdc =
                g_asObjectCounter_00494be0[g_nYourWingman_0049346c];
            g_asShipMissionType_00495de8[g_nYourWingman_0049346c] =
                MISSION_TYPE_GOTO_WARP;
            g_asShipCount_00495ff8[g_nYourWingman_0049346c] = 0;
            g_asShipTactic_00495f30[g_nYourWingman_0049346c] =
                TACTIC_SIT_STILL;
        }
    }
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
    DosMemcpy(&g_stMissionHeader_005d3e70, source, 0x18);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(moduleName, 1, 0);
    offset = series * 0xfc8;
    offset += mission * 0x3f2;
    source = packet + offset;
    DosMemcpy(g_aMissionNavPoints_00491e98, source, 0x3f2);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(moduleName, 2, 0);
    offset = series * 0x800;
    offset += mission * 0x200;
    source = packet + offset;
    DosMemcpy(g_aMissionObjectiveSources_005d3c70, source, 0x200);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(moduleName, 3, 0);
    offset = series * 0xf00;
    offset += mission * 0x3c0;
    source = packet + offset;
    DosMemcpy(g_aMissionShips_00492290, source, 0x3c0);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(moduleName, 4, 0);
    offset = series * 0xa0;
    offset += mission * 0x28;
    source = packet + offset;
    DosMemcpy(g_abMissionAuxData_005d3c10, source, 0x28);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(moduleName, 5, 0);
    offset = series * 0x28;
    source = packet + offset;
    DosMemcpy(g_abSeriesAuxData_005d3c40, source, 0x28);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(moduleName, 6, 0);
    offset = series * 0x378;
    offset += mission * 0xde;
    source = packet + offset;
    DosMemcpy(g_abMissionBriefingData_005d3e90, source, 0xde);
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
#ifdef SDL_PORT
    if (memcmp(g_pszCutsceneWorkBuffer_005d2ecc, "50", 2) != 0)
        FormatTextBufferFromStart(
            "%s%P", g_pszCutsceneWorkBuffer_005d2ecc);
#else
    if (IsCutsceneSpeechLoaded() == 0 || g_nAudioEnabled_0049c244 == 0) {
        if (memcmp(g_pszCutsceneWorkBuffer_005d2ecc, "50", 2) != 0)
            FormatTextBufferFromStart("%s%P", g_pszCutsceneWorkBuffer_005d2ecc);
    }
#endif
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
