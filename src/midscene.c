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
        switch (g_aeObjectClass[obj]) {
        case OBJECT_CLASS_FUTURION:
        case OBJECT_CLASS_EXPLOSION:
        case OBJECT_CLASS_DEBRIS:
        case OBJECT_CLASS_PROJECTILE:
        case OBJECT_CLASS_MISSILE:
            if (g_asObjectType[obj] ==
                OBJECT_DATA_EJECTION_POD) {
                if (g_aObjectTypeData[
                        OBJECT_DATA_EJECTION_POD].shapeSet == 0) {
                    g_aObjectTypeData[
                        OBJECT_DATA_EJECTION_POD].shapeSet =
                        FetchDiskPacketRetrying("pilotanm.vga", 2, 0);
                    g_apObjectShape[obj] =
                        g_aObjectTypeData[
                            OBJECT_DATA_EJECTION_POD].shapeSet;
                }
            } else if (g_asObjectType[obj] != 0x3d) {
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
         objective < g_cMissionObjectiveCount;
         objective++) {
        if (g_aMissionObjectives[objective].type == 0 &&
            g_aMissionObjectives[objective].index ==
                g_nCurrentNavPoint)
            flag_reached(objective, 0);
    }
}

/* Function start: 0x44EBCA */
/* End of mission: fold the results into the campaign globals, run the
 * campaign script and, unless the arcade took over, bring the cockpit and the
 * 3-Space objects back for the next leg. */
void ejection_sequence(short outcome, signed char restoreCockpit)
{
    long mainMiddle;
    long lowBefore;
    long mainBlockEnd;
    long farEnd;
    long farLargestBefore;
    FILE *stream;
    long extMid;
    long mainLargestBefore;
    long baseAfter;
    long farLargestDuring;
    long mainBlockMiddle;
    long farBegin;
    long farLargestPost;
    long baseFinal;
    long farBlockEnd;
    long nearBlockPost;
    long farLater;
    short series;
    short ship;

    if (g_bMissionEndPending == 0)
        return;

    lowBefore = GetAvailableMainMemory();
    farBegin = GetAvailableFarMemory();
    mainLargestBefore = GetLargestMainMemoryBlock();
    farLargestBefore = GetLargestFreeMemoryBlock();
    if ((unsigned short)outcome == 0x75) {
        g_nArcadeState = 1;
        return;
    }

    g_1_nResourcePaletteState = 1;
    if (g_bAutopilotDebugEnabled != 0)
        stream = fopen("mid.$$$", "w+");
    if (g_bAutopilotDebugEnabled != 0)
        LogMemoryStateToFile(stream);
    ReleaseSpaceflightResources();
    if (g_bAutopilotDebugEnabled != 0)
        LogMemoryStateToFile(stream);
    if (g_pCampaignGlobals == 0)
        LoadTemporaryCampaignGlobals();
    if (g_pCampaignGlobals == 0)
        ReportFatalErrorCode("021");

    series = g_pCampaignGlobals->series;
    g_pCampaignGlobals->series = 1000;
    g_pCampaignGlobals->mission = outcome;
    g_pCampaignGlobals->previousSeries =
        g_stCurrentPilotProfile.series;
    g_pCampaignGlobals->previousMission =
        g_stCurrentPilotProfile.mission;
    g_pCampaignGlobals->damageLevel =
        (short)calculate_damage_level();

    mainMiddle = GetAvailableMainMemory();
    extMid = GetAvailableFarMemory();
    mainBlockMiddle = GetLargestMainMemoryBlock();
    farLargestDuring = GetLargestFreeMemoryBlock();
    if (g_bAutopilotDebugEnabled != 0)
        LogMemoryStateToFile(stream);
    StoreMissionResultsInCampaignGlobals(g_pCampaignGlobals);
    RunCampaignScript(g_nSelectedCampaignSlot);
    g_3_cOverlayMessageView = -1;
    if (g_bAutopilotDebugEnabled != 0)
        LogMemoryStateToFile(stream);
    g_nArcadeState = g_pCampaignGlobals->arcadeState;
    InitializeCampaignConstellationState(g_pCampaignGlobals, 0);

    baseAfter = GetAvailableMainMemory();
    farLater = GetAvailableFarMemory();
    nearBlockPost = GetLargestMainMemoryBlock();
    farLargestPost = GetLargestFreeMemoryBlock();
    ReleasePacketSlot(&g_pCampaignGlobals);
    FadeViewportPaletteToColour(&g_2_stModalSourceWindow, 0, 1);
    ClearViewport(&g_2_stModalSourceWindow, 0);
    RestoreGamePalette();
    ResetGameTextContexts();
    for (ship = 0; ship < 10; ship++)
        g_asShipIntelState[ship] = 0;

    if (restoreCockpit != 0 && g_nArcadeState == 0) {
        if (g_bAutopilotDebugEnabled != 0)
            LogMemoryStateToFile(stream);
        InitializeCockpitResources(g_stCurrentPilotProfile.playerShipType);
        load_common_3Space_objects();
        init_constellation(series);
        g_b3SpaceObjectsActive = 1;
        load_all_slots();
        g_bSpaceViewBufferEnabled = 1;
        initialize_view_buffer();
        ClearTransientObjectsForEjection();
        if (
#ifdef SDL_PORT
            /* SO1 can resume a mid-flight scene after the wingman has
             * already been removed. There is no portrait at index -1. */
            g_nYourWingman >= 0 && g_nYourWingman < 10 &&
            g_acShipPortrait[g_nYourWingman] >= 0 &&
            g_acShipPortrait[g_nYourWingman] < 32 &&
#endif
            g_pPilotStatus[
                g_acShipPortrait[g_nYourWingman]] == 0)
            remove_object(g_nYourWingman);
        g_1_cStoredViewDisplayModeValue = -1;
        force_view(0, 0);
    }
    if (g_bAutopilotDebugEnabled != 0)
        LogMemoryStateToFile(stream);

    g_1_nResourcePaletteState = 0;
    InitializeSpaceFlightInput();
    ReleaseSceneMusicPacket();

    if (g_nShowMemoryStatus != 0) {
        baseFinal = GetAvailableMainMemory();
        farEnd = GetAvailableFarMemory();
        mainBlockEnd = GetLargestMainMemoryBlock();
        farBlockEnd = GetLargestFreeMemoryBlock();
        printf("      before  large    after   large\n");
        printf("Main: %6ld  %6ld   %6ld  %6ld\n", lowBefore,
               mainLargestBefore, baseFinal, mainBlockEnd);
        printf("EMS:  %7ld %6ld   %7ld %6ld\n", farBegin,
               farLargestBefore, farEnd, farBlockEnd);
        printf("CMem: %6ld  %6ld   %6ld  %6ld\n", mainMiddle,
               mainBlockMiddle, baseAfter, nearBlockPost);
        printf("EEMS: %7ld %6ld   %7ld %6ld\n", farLater,
               farLargestPost, farLater, farLargestPost);
        WaitForAnyInputPress();
    }
    FlagCurrentNavObjectivesReached();
    if (g_78_bHighMemoryBuffersSetState != 0)
        ResetCannedSceneRecording();
    if (stream != 0 && g_bAutopilotDebugEnabled != 0)
        fclose(stream);
}
