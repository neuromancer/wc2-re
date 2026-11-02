/*
 *  Mid-mission ejection scene.
 *
 *  Retail object 0x44EA50-0x44F00F (link order; see docs/ORDER.md).
 */
#include "game.h"

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
                    g_apObjectShape_00493868[obj] =
                        g_aObjectTypeData_00496d30[
                            OBJECT_DATA_EJECTION_POD].shapeSet;
                }
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
