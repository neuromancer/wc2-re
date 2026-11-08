/*
 *  Objectives, warp arrival and star-system jumps.
 *
 *  Retail object 0x424A60-0x4254BF (link order; see docs/ORDER.md).
 */
#include "game.h"

short g_nEscapedEnemyCount = 0; /* 0x004962e8 */
short g_bStarSystemJumpTransition = 0; /* 0x004962ec */
short g_bJumpSequenceActive = 0; /* 0x004962f0 */
short g_nStarSystemJumpDelay = 0; /* 0x004962f4 */

/* Function start: 0x424A60 */
short find_objective(short type, short index)
{
    short objective;

    for (objective = 0;
         objective < (short)g_cMissionObjectiveCount;
         objective++) {
        if (g_aMissionObjectives[objective].type == type &&
            (index == -1 ||
             g_aMissionObjectives[objective].index == index))
            return objective;
    }
    return -1;
}

/* Function start: 0x424AEE */
void arrive_from_warp(short obj)
{
    short objective = find_objective(0, g_nCurrentNavPoint);

    if (obj != 0) {
        if (objective != -1) {
            if (g_aMissionObjectives[
                    g_abFlightPath[objective]].type != 1)
                flag_objective(objective, 1);
            if (g_cCurrentObjective == objective)
                set_next_destination();
        }
        place_ship_near_player_until_valid(obj, 2000, 5000);
        if (g_asShipSide[obj] == SIDE_IMPERIAL)
            reset_mission_type(obj, MISSION_TYPE_COME_HOME);
        else
            reset_mission_type(obj, MISSION_TYPE_PATROL);
    } else {
        g_asShipMissionType[0] = MISSION_TYPE_NONE;
        g_asShipTactic[0] = TACTIC_NONE;
    }
    unwarp(obj);
    g_anShipSpeed[obj] =
        (int)g_asShipMaximumVelocity[obj] << 8;
    fix_velocity(obj);
}

/* Function start: 0x424C05 */
void unwarp(short obj)
{
    short effect;

    ClearViewport(&g_27_stViewScratchInfo,
                  g_bPrimaryViewBufferColour);
    g_bViewportDirty = 1;
    PlaySfxWaveFileByNumber(0x34, obj, 0);
    effect = find_vacant_3d_object();
    if (effect != -1) {
        set_objects_data(effect, OBJECT_TYPE_HYPERSPACE_JUMP_FLASH,
                         obj, 0);
        g_aShipPosition[effect] = g_aShipPosition[obj];
        g_aShipVelocity[effect] = g_aShipVelocity[obj];
        g_asShipManeuver[obj] = MANEUVER_NONE;
        g_asObjectCounter[obj] = 6;
        RecordCannedSceneObjectEvent(effect, 0);
    } else {
        g_abShipNavPointIndex[obj] = (signed char)
            g_aObjectTypeData[
                g_acObjectType[obj]].resourceType;
        set_objects_data(obj, OBJECT_TYPE_HYPERSPACE_JUMP_FLASH,
                         obj, 0);
        RecordCannedSceneObjectEvent(obj, 0);
    }
    if (obj == 0)
        g_asShipManeuver[0] = MANEUVER_WARPING_IN;
}

/* Function start: 0x424D4D */
void warp(short obj)
{
    short effect;

    ClearViewport(&g_27_stViewScratchInfo,
                  g_bPrimaryViewBufferColour);
    g_bViewportDirty = 1;
    PlaySfxWaveFileByNumber(0x34, obj, 0);
    effect = find_vacant_3d_object();
    if (effect != -1) {
        set_objects_data(effect, OBJECT_TYPE_HYPERSPACE_JUMP_FLASH,
                         (short)g_acObjectOwner[obj], 0);
        g_aShipPosition[effect] = g_aShipPosition[obj];
        g_aShipVelocity[effect] = g_aShipVelocity[obj];
        g_asShipManeuver[obj] = MANEUVER_WARPING_OUT;
        g_asObjectCounter[obj] = 6;
        RecordCannedSceneObjectEvent(effect, 0);
    } else {
        set_objects_data(obj, OBJECT_TYPE_HYPERSPACE_JUMP_FLASH,
                         (short)g_acObjectOwner[obj], 0);
        RecordCannedSceneObjectEvent(obj, 0);
    }
    if (g_asShipSide[obj] == SIDE_KILRATHI)
        g_nEscapedEnemyCount++;
}

/* Function start: 0x424E8C */
void MarkPilotDead(short pilot)
{
    g_pPilotStatus[pilot] = 0;
}

/* Function start: 0x424EA7 */
void clean_up_cockpit(void)
{
    g_bTargetLockMode = 0;
    g_acShipTarget[0] = -1;
    if (g_nYourWingman != -1) {
        g_acShipTarget[g_nYourWingman] = -1;
        g_nAutoEngageTimer = -1;
        reset_objective(g_nYourWingman,
                        OBJECTIVE_HOLD_FORMATION);
    }
    ClearHudGunReadouts();
}

/* Function start: 0x424EFF */
void TryBeginStarSystemJump(void)
{
    short object;

    if (g_asObjectType[0] != 0x33 ||
        g_bJumpSequenceActive != 0)
        return;
    for (object = 0; object < 10; object++) {
        if (g_aeObjectClass[object] == OBJECT_CLASS_SHIP &&
            g_asShipSide[object] == SIDE_KILRATHI) {
            set_global_message(
                _("Enemy near"),
                g_cYellowColour, 3);
            return;
        }
    }
    if (g_acJumpDriveUsedBySystem[
            g_nCurStellarSystem] != 0) {
        set_global_message(
            _("Capacitors Empty"),
            g_cYellowColour, 3);
        return;
    }
    for (object = 1; object < 10; object++) {
        if (g_aeObjectClass[object] >=
                OBJECT_CLASS_CAPITAL_SHIP &&
            g_asShipMissionType[object] ==
                MISSION_TYPE_GOTO_WARP &&
            g_asShipMissionParameter[object] ==
                g_nCurrentNavPoint &&
            equ_vector(
                &g_aMissionObjectives[
                    g_cCurrentObjective].position,
                &g_aShipDestination[object]) == 0) {
            sprintf(g_pszAutopilotWaitReason,
                    _("Wait for %s"),
                    g_apShipMissionRecord[object]->name);
            set_global_message(
                g_pszAutopilotWaitReason,
                g_cYellowColour, 3);
            return;
        }
    }
    if (g_aMissionNavPoints[
            g_nCurrentNavPoint].field_2e == 0) {
        set_global_message(
            _("No jump point"),
            g_cYellowColour, 3);
        return;
    }
    if (g_ucPendingEjectionTransition != 0xff &&
        g_nPendingEjectionSequenceCount != 0) {
        g_nEjectionSequenceState = 0;
        g_nPendingEjectionSequenceCount = 0;
        ejection_sequence(g_ucPendingEjectionTransition, 1);
        g_ucPendingEjectionTransition = 0xff;
    }
    spacetrack(0x2f, 1, 0);
    force_view(0x0d, 0);
    g_bStarSystemJumpTransition = 1;
    g_acJumpDriveUsedBySystem[
        g_nCurStellarSystem]++;
    g_bJumpSequenceActive = 1;
    g_nStarSystemJumpDelay = 0;
    g_1_nSavedPlayerObjectCounter =
        g_asObjectCounter[0];
    g_asShipMissionType[0] = MISSION_TYPE_GOTO_WARP;
    g_asShipCount[0] = 0;
    g_asShipTactic[0] = TACTIC_SIT_STILL;
    if (g_nYourWingman != -1) {
        g_nSavedWingmanObjectCounterValue =
            g_asObjectCounter[g_nYourWingman];
        g_asShipMissionType[g_nYourWingman] =
            MISSION_TYPE_GOTO_WARP;
        g_asShipCount[g_nYourWingman] = 0;
        g_asShipTactic[g_nYourWingman] =
            TACTIC_SIT_STILL;
    }
}

/* Function start: 0x4251F2 */
void CompleteStarSystemJump(void)
{
    short previousDelay;
    short destinationNavPoint;

    previousDelay = g_nStarSystemJumpDelay;
    g_nStarSystemJumpDelay++;
    if (previousDelay < 75) {
        g_asObjectCounter[0] = 1;
    } else {
        FadeViewportPaletteToColour(
            &g_101_stScrViewInfo,
            g_cSecondaryViewBufferColour, 1);
        ClearViewport(&g_101_stScrViewInfo,
                      g_cSecondaryViewBufferColour);
        RestoreGamePalette();

        for (destinationNavPoint = 0; destinationNavPoint < 10;
             destinationNavPoint++) {
            if (g_aMissionNavPoints[destinationNavPoint].type != 0 &&
                g_aMissionNavPoints[destinationNavPoint]
                        .systemIndex != g_nCurStellarSystem &&
                g_aMissionNavPoints[destinationNavPoint].field_2e !=
                    0) {
                break;
            }
        }

        g_anShipFuel[0] -= g_anShipFuel[0] / 2;
        g_aeObjectClass[0] = OBJECT_CLASS_SHIP;
        new_view(0, 0);
        g_bAutopilotSequenceActive = 1;
        set_up_action_sphere(destinationNavPoint);
        g_bAutopilotSequenceActive = 0;
        g_aShipPosition[0] =
            g_aMissionNavPoints[destinationNavPoint].position;
        g_aShipPosition[0].x +=
            g_aMissionNavPoints[destinationNavPoint]
                .proximityRadius;
        g_asObjectScale[0] =
            g_aObjectTypeData[
                g_acObjectType[0]].scale;
        g_bStarSystemJumpTransition = 0;
        g_asShipManeuver[0] = MANEUVER_NONE;
        g_asObjectCounter[0] =
            (short)g_1_nSavedPlayerObjectCounter;

        if (g_nYourWingman != -1) {
            g_anShipFuel[g_nYourWingman] -=
                g_anShipFuel[g_nYourWingman] / 2;
            g_asObjectCounter[g_nYourWingman] =
                (short)g_nSavedWingmanObjectCounterValue;
            g_asShipMissionType[g_nYourWingman] =
                MISSION_TYPE_WINGMAN;
            g_aeShipObjective[g_nYourWingman] =
                OBJECTIVE_HOLD_FORMATION;
            g_aShipPosition[g_nYourWingman] =
                g_aShipPosition[0];
            g_anShipSpeed[g_nYourWingman] =
                g_anShipSpeed[0];
            g_asObjectScale[g_nYourWingman] =
                g_aObjectTypeData[
                    g_acObjectType[
                        g_nYourWingman]].scale;
        }

        ClearViewport(&g_27_stViewScratchInfo,
                      g_bPrimaryViewBufferColour);
        g_bViewportDirty = 1;
        g_bJumpSequenceActive = 0;
        if (g_78_bHighMemoryBuffersSetState != 0)
            ResetCannedSceneRecording();
    }
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
int g_nSavedWingmanObjectCounterValue; /* 0x005d2fdc */
int g_1_nSavedPlayerObjectCounter; /* 0x005d2fe0 */
signed char g_acJumpDriveUsedBySystem[5]; /* 0x005d2fe8 */
