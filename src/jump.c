/*
 *  Objectives, warp arrival and star-system jumps.
 *
 *  Retail object 0x424A60-0x4254BF (link order; see docs/ORDER.md).
 */
#include "game.h"

short g_nEscapedEnemyCount_004962e8 = 0;
short g_bStarSystemJumpTransition_004962ec = 0;
short g_bJumpSequenceActive_004962f0 = 0;
short g_nStarSystemJumpDelay_004962f4 = 0;

/* Function start: 0x424A60 */
short find_objective(short type, short index)
{
    short objective;

    for (objective = 0;
         objective < (short)g_cMissionObjectiveCount_00493294;
         objective++) {
        if (g_aMissionObjectives_004932a8[objective].type == type &&
            (index == -1 ||
             g_aMissionObjectives_004932a8[objective].index == index))
            return objective;
    }
    return -1;
}

/* Function start: 0x424AEE */
void arrive_from_warp(short obj)
{
    short objective = find_objective(0, g_nCurrentNavPoint_004931bc);

    if (obj != 0) {
        if (objective != -1) {
            if (g_aMissionObjectives_004932a8[
                    g_abFlightPath_004932a0[objective]].type != 1)
                flag_objective(objective, 1);
            if (g_cCurrentObjective_004931cc == objective)
                set_next_destination();
        }
        place_ship_near_player_until_valid(obj, 2000, 5000);
        if (g_asShipSide_004955d0[obj] == SIDE_IMPERIAL)
            reset_mission_type(obj, MISSION_TYPE_COME_HOME);
        else
            reset_mission_type(obj, MISSION_TYPE_PATROL);
    } else {
        g_asShipMissionType_00495de8[0] = MISSION_TYPE_NONE;
        g_asShipTactic_00495f30[0] = TACTIC_NONE;
    }
    unwarp(obj);
    g_anShipSpeed_00494e20[obj] =
        (int)g_asShipMaximumVelocity_00495f70[obj] << 8;
    fix_velocity(obj);
}

/* Function start: 0x424C05 */
void unwarp(short obj)
{
    short effect;

    ClearViewport(&g_stViewBuffer_005d2b00,
                  g_bPrimaryViewBufferColour_0049cb50);
    g_bViewportDirty_0049d76c = 1;
    PlaySfxWaveFileByNumber(0x34, obj, 0);
    effect = find_vacant_3d_object();
    if (effect != -1) {
        set_objects_data(effect, OBJECT_TYPE_HYPERSPACE_JUMP_FLASH,
                         obj, 0);
        g_aShipPosition_00494550[effect] = g_aShipPosition_00494550[obj];
        g_aShipVelocity_00494898[effect] = g_aShipVelocity_00494898[obj];
        g_asShipManeuver_00495f48[obj] = MANEUVER_NONE;
        g_asObjectCounter_00494be0[obj] = 6;
        RecordCannedSceneObjectEvent(effect, 0);
    } else {
        g_abShipNavPointIndex_00495f60[obj] = (signed char)
            g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[obj]].resourceType;
        set_objects_data(obj, OBJECT_TYPE_HYPERSPACE_JUMP_FLASH,
                         obj, 0);
        RecordCannedSceneObjectEvent(obj, 0);
    }
    if (obj == 0)
        g_asShipManeuver_00495f48[0] = MANEUVER_WARPING_IN;
}

/* Function start: 0x424D4D */
void warp(short obj)
{
    short effect;

    ClearViewport(&g_stViewBuffer_005d2b00,
                  g_bPrimaryViewBufferColour_0049cb50);
    g_bViewportDirty_0049d76c = 1;
    PlaySfxWaveFileByNumber(0x34, obj, 0);
    effect = find_vacant_3d_object();
    if (effect != -1) {
        set_objects_data(effect, OBJECT_TYPE_HYPERSPACE_JUMP_FLASH,
                         (short)g_acObjectOwner_00495208[obj], 0);
        g_aShipPosition_00494550[effect] = g_aShipPosition_00494550[obj];
        g_aShipVelocity_00494898[effect] = g_aShipVelocity_00494898[obj];
        g_asShipManeuver_00495f48[obj] = MANEUVER_WARPING_OUT;
        g_asObjectCounter_00494be0[obj] = 6;
        RecordCannedSceneObjectEvent(effect, 0);
    } else {
        set_objects_data(obj, OBJECT_TYPE_HYPERSPACE_JUMP_FLASH,
                         (short)g_acObjectOwner_00495208[obj], 0);
        RecordCannedSceneObjectEvent(obj, 0);
    }
    if (g_asShipSide_004955d0[obj] == SIDE_KILRATHI)
        g_nEscapedEnemyCount_004962e8++;
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

    if (g_asObjectType_00495298[0] != 0x33 ||
        g_bJumpSequenceActive_004962f0 != 0)
        return;
    for (object = 0; object < 10; object++) {
        if (g_aeObjectClass_00495328[object] == OBJECT_CLASS_SHIP &&
            g_asShipSide_004955d0[object] == SIDE_KILRATHI) {
            set_global_message(
                "Enemy near",
                g_abGamePaletteReservedColours_0049cb54[4], 3);
            return;
        }
    }
    if (g_acJumpDriveUsedBySystem_005d2fe8[
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
    g_acJumpDriveUsedBySystem_005d2fe8[
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

/* Function start: 0x4251F2 */
void CompleteStarSystemJump(void)
{
    short previousDelay;
    short destinationNavPoint;

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
