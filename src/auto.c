/*
 *  Autopilot travel and interstitial flight sequences.
 *
 *  Retail object 0x4228B0-0x42347F (link order; see docs/ORDER.md).
 */
#include "game.h"

short g_nAutopilotFormationShipCount_00493060 = 0;
short g_bAutopilotSequenceActive_00493064 = 0;

/* WC2's auto.c declares the autopilot debug flag two bytes wide where every
 * other translation unit declares it as a char.  In the retail image the byte
 * above it is padding, so the value is the same and only the load width
 * differs; on the port the byte above it is the sanitizer's redzone, so read
 * the flag at its own width there. */
#ifdef SDL_PORT
#define g_nAutopilotDebugEnabled g_bAutopilotDebugEnabled_00499bfc
#else
#define g_nAutopilotDebugEnabled \
    (*(const short *)&g_bAutopilotDebugEnabled_00499bfc)
#endif


/* Function start: 0x4228B0 */
void visit_the_cinema(int view, int obj, short frames)
{
    int originalUnlock;
    int savedInputFlag;
    int savedKeyboardFlag;

    originalUnlock = g_nOriginDevUnlock_0049d774;
    savedInputFlag = g_bPlayerDamageEnabled_0049d77c;
    savedKeyboardFlag = g_bPlayerCollisionEnabled_0049d780;
    g_nOriginDevUnlock_0049d774 = 1;
    g_bPlayerDamageEnabled_0049d77c = 0;
    g_bPlayerCollisionEnabled_0049d780 = 0;
    /* The retail call converts both int parameters implicitly.  Making the
     * narrowing explicit changes the register setup and loses an exact match. */
    force_view(view, obj);
    while (frames-- > 0) {
        Update_3Space();
        RenderSpaceViewFrame();
        MarkDibDirty();
        DIBslamReal();
    }
    g_nOriginDevUnlock_0049d774 = originalUnlock;
    g_bPlayerDamageEnabled_0049d77c = (short)savedInputFlag;
    g_bPlayerCollisionEnabled_0049d780 = (short)savedKeyboardFlag;
}

/* Function start: 0x422953 */
short player_wingman(short obj)
{
    return obj != -1 && g_asShipWingLeader_00495dd0[obj] == 0;
}

/* Function start: 0x422990 */
void set_speed(short obj, short speed)
{
    g_anShipSpeed_00494e20[obj] = (int)speed << 8;
    fix_velocity(obj);
}

#pragma function(abs)
void set_speed(short obj, short speed);

/* Function start: 0x4229B9 */
void auto_position(short obj, short *formationSlot)
{
    short sideways;
    short vertical;
    short forward;

    if (player_wingman(obj) != 0) {
        position_relative_ijk(&g_aShipPosition_00494550[obj], 0,
                              g_aShipFormationOffset_00495468[obj].x,
                              g_aShipFormationOffset_00495468[obj].y,
                              g_aShipFormationOffset_00495468[obj].z);
        return;
    }

    (*formationSlot)++;
    if ((*formationSlot & 1) != 0)
        sideways = 650;
    else
        sideways = -650;
    if (g_nAutopilotFormationShipCount_00493060 == *formationSlot)
        sideways = 0;
    vertical = 0;
    forward = (short)(MaxShort(1, (short)(*formationSlot >> 1)) * -1800);
    if (g_nYourWingman_0049346c != -1) {
        if (abs(
            g_aShipFormationOffset_00495468[g_nYourWingman_0049346c].z -
            forward) <
            g_asObjectCollisionRadius_004950e8[obj] +
            g_asObjectCollisionRadius_004950e8[g_nYourWingman_0049346c])
            vertical = 500;
    }
    position_relative_ijk(&g_aShipPosition_00494550[obj], 0,
                          sideways, vertical, forward);
}

/* Function start: 0x422B1C */
void auto_pilot_sequence(void)
{
    short cruiseSpeed;
    short ordnance;
    FixedVector destination;
    short ownCruise;
    signed char inFormation[10];
    FixedVector advance;
    short savedMode;
    short slot;
    short obj;
    short destinationRange;
    short nearest;
    short startRange;
    short moveOn;
    FILE *autoLog;

    savedMode = g_nTrainSimActive_0049d758;
    slot = 0;
    moveOn = 1;
    autoLog = 0;
    g_bAutopilotSequenceActive_00493064 = 1;
    destination = g_aMissionObjectives_004932a8[
        g_abFlightPath_004932a0[g_cCurrentNavPointIndex_00493298]].position;

    if (auto_pilot_valid(1) == 0)
        return;
    if (g_nAutopilotDebugEnabled != 0)
        autoLog = fopen("auto.$$$", "w+");
    if (g_nAutopilotDebugEnabled != 0)
        LogMemoryStateToFile(autoLog);
    if (g_ucPendingEjectionTransition_0049b8ac != 0xff &&
        g_nPendingEjectionSequenceCount_0049b8b8 != 0) {
        g_nEjectionSequenceState_0049b8c0 = 0;
        g_nPendingEjectionSequenceCount_0049b8b8 = 0;
        ejection_sequence(
            g_ucPendingEjectionTransition_0049b8ac, 1);
        g_ucPendingEjectionTransition_0049b8ac = 0xff;
        if (g_nArcadeState_0049d75c == 1)
            return;
    }
    if (distance_between_points(
            &g_aMissionObjectives_004932a8[
                g_cCurrentObjective_004931cc].position,
            &g_aMissionNavPoints_00491e98[
                g_nCurrentNavPoint_004931bc].position) <
        g_aMissionNavPoints_00491e98[
            g_nCurrentNavPoint_004931bc].proximityRadius + 25)
        moveOn = 0;

    for (ordnance = 0; ordnance < 10; ordnance++) {
        if (g_aeObjectClass_00495328[ordnance] == OBJECT_CLASS_MISSILE)
            remove_object(ordnance);
    }
    clean_up_cockpit();
    ResetSoundState();
    g_nAutopilotFormationShipCount_00493060 = 0;

    for (obj = 0; obj < 10; obj++) {
        inFormation[obj] = 0;
        g_anShipSpeed_00494e20[obj] = 0;
        zero_vector(&g_aShipVelocity_00494898[obj]);
        if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver_00495600[obj] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide_004955d0[obj] == SIDE_IMPERIAL) {
            if (is_team_member(
                    g_asShipMissionIndex_00495d00[obj]) != 0) {
                if (kilrathi_near(obj, 10000) == 0) {
                    inFormation[obj] = -1;
                    g_anObjectYawRotation_00494fc8[obj] =
                        g_anYawGoal_004954c0[obj] =
                        g_anObjectPitchRotation_00494f38[obj] =
                        g_anPitchGoal_004954a8[obj] =
                        g_anObjectRollRotation_00495058[obj] =
                        g_anRollGoal_004954d8[obj] = 0;
                    g_aeSpecialManeuver_00495600[obj] =
                        SPECIAL_MANEUVER_NONE;
                    if (obj != 0 &&
                        obj != g_nYourWingman_0049346c)
                        g_nAutopilotFormationShipCount_00493060++;
                }
            } else if (moveOn != 0) {
                remove_object(obj);
            }
        }
    }

    startRange = distance_from_point(0, &destination);
    point_at(0, destination);
    set_speed(0, 60);
    inFormation[0] = 1;
    slot = 0;

    for (obj = 0; obj < 10; obj++) {
        if (inFormation[obj] == -1 && obj != 0) {
            if (distance_from_object(obj, 0) > 20000) {
                if (player_wingman(obj) != 0)
                    inFormation[obj] = 1;
                else if (equ_vector(
                             &g_aShipDestination_004953f0[obj],
                             &destination) != 0 &&
                         g_asShipSide_004955d0[obj] ==
                             g_asShipSide_004955d0[0])
                    inFormation[obj] = 2;
                else
                    inFormation[obj] = 3;
            } else {
                if (player_wingman(obj) != 0 ||
                    equ_vector(&g_aShipDestination_004953f0[obj],
                               &destination) != 0)
                    inFormation[obj] = 1;
                else
                    inFormation[obj] = 3;
            }
            if (inFormation[obj] == 1 &&
                g_asShipSide_004955d0[obj] ==
                    g_asShipSide_004955d0[0]) {
                auto_position(obj, &slot);
                ScaleFixedVector(&g_aShipForwardVector_00494208[0],
                                 0x3c00,
                                 &g_aShipVelocity_00494898[obj]);
                copy_frame(0, obj);
                g_anShipSpeed_00494e20[obj] =
                    g_anShipSpeed_00494e20[0];
                g_aShipVelocity_00494898[obj] =
                    g_aShipVelocity_00494898[0];
            }
        }
    }

    g_nTrainSimActive_0049d758 = 4;
    visit_the_cinema(13, 0, 90);
    while (g_nTrainSimActive_0049d758 == 4) {
        ComputeVectorDelta(&g_aShipPosition_00494550[0],
                           &destination, &advance);
        NormalizeFixedVector(&advance);
        ScaleFixedVector(&advance, 0x19000, &advance);
        AddFixedVectors(&g_aShipPosition_00494550[0], &advance,
                        &g_aShipPosition_00494550[0]);
        ReleaseStaleNavTarget();
        check_hazards();

        destinationRange = distance_from_point(0, &destination);
        nearest = 0x7fff;
        for (obj = 0; obj < 10; obj++) {
            if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_SHIP &&
                inFormation[obj] == 0)
                nearest = MinShort(
                    nearest, distance_from_object(0, obj));
        }

        if ((unsigned short)destinationRange < 1000 ||
            g_pActiveHazardField_00493278 != 0 ||
            nearest < 4000 ||
            report_kilrathi_rout(1) != 0)
            g_nTrainSimActive_0049d758 = savedMode;
    }

    SubtractFixedVectors(&g_aShipPosition_00494550[0], &advance,
                         &g_aShipPosition_00494550[0]);
    cruiseSpeed = g_asShipMaximumVelocity_00495f70[0];
    for (obj = 0; obj < 10; obj++) {
        if (inFormation[obj] != 0) {
            ownCruise = g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[obj]].cruiseVelocity;
            if (ownCruise < cruiseSpeed)
                cruiseSpeed = ownCruise;
        }
    }

    slot = 0;
    for (obj = 0; obj < 10; obj++) {
        if (inFormation[obj] != 0 &&
            g_asShipSide_004955d0[obj] == g_asShipSide_004955d0[0]) {
            set_speed(obj, cruiseSpeed);
            if (obj != 0) {
                switch (inFormation[obj]) {
                case 1:
                case 2:
                    auto_position(obj, &slot);
                    break;
                case 3:
                    if (distance_from_point(
                            obj,
                            &g_aShipDestination_004953f0[obj]) <
                        startRange)
                        g_aShipPosition_00494550[obj] =
                            g_aShipDestination_004953f0[obj];
                    break;
                }
            }
        }
    }

    Update_3Space();
    if (g_nCockpitDisplayMode_0049d71c == 0) {
        force_view(0, 0);
    } else {
        free_view_buffer();
        SetViewportRect(&g_stViewScratchInfo_005d2b00, 0, 0,
                        (unsigned short)(g_nScreenWidth_0049d4d8 - 1),
                        (unsigned short)(g_nScreenHeight_0049d4dc - 1));
        initialize_view_buffer();
        g_nCockpitDisplayMode_0049d71c = 1;
        force_view(0, 0);
        g_nCockpitDisplayMode_0049d71c = 1;
        free_view_buffer();
        SetViewportRect(&g_stViewScratchInfo_005d2b00, 0, 0, 319, 199);
        initialize_view_buffer();
    }
    if (g_bHighMemoryBuffersSetState_005d2ad8 != 0)
        ResetCannedSceneRecording();
    if (g_nAutopilotDebugEnabled != 0)
        LogMemoryStateToFile(autoLog);
    g_bAutopilotSequenceActive_00493064 = 0;
    if (autoLog != 0)
        fclose(autoLog);
}
