/*
 *  Autopilot travel and interstitial flight sequences.
 *
 *  Address range 0x403e50-0x40460f (provisional -- see docs/ORDER.md).
 *  Boundary evidence: the Mac auto unit preserves visit_the_cinema,
 *  player_wingman, set_speed, auto_position, and auto_pilot_sequence in this
 *  exact order; the next Win32 function is a palette-file loader in another
 *  compilation unit.
 */
#include "wc1.h"

/* Function start: 0x4228B0 */
void visit_the_cinema(int view, int obj, short frames)
{
    int savedOriginUnlock;
    int savedInputFlag;
    int savedKeyboardFlag;

    savedOriginUnlock = g_nOriginDevUnlock_0049d774;
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
    g_nOriginDevUnlock_0049d774 = savedOriginUnlock;
    g_bPlayerDamageEnabled_0049d77c = (short)savedInputFlag;
    g_bPlayerCollisionEnabled_0049d780 = (short)savedKeyboardFlag;
}

/* Function start: 0x422953 */
unsigned int player_wingman(short obj)
{
    if (obj != -1 && g_asShipWingLeader_00495dd0[obj] == 0)
        return 1;
    return 0;
}

/* Function start: 0x422990 */
void set_speed(short obj, short speed)
{
    g_anShipSpeed_0059b320[obj] = (int)speed << 8;
    fix_velocity(obj);
}

/* Function start: 0x4229B9 */
#pragma function(abs)
void auto_position(short obj, short *formationSlot)
{
    short lateral;
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
        lateral = 650;
    else
        lateral = -650;
    if (g_nAutopilotFormationShipCount_00493060 == *formationSlot)
        lateral = 0;
    vertical = 0;
    forward = MaxShort(1, (short)(*formationSlot >> 1));
    forward = (short)(forward * -1800);
    if (g_nYourWingman_0049346c != -1) {
        if (abs(
            g_aShipFormationOffset_00495468[g_nYourWingman_0049346c].z -
            forward) <
            g_asObjectCollisionRadius_004950e8[obj] +
            g_asObjectCollisionRadius_004950e8[g_nYourWingman_0049346c])
            vertical = 500;
    }
    position_relative_ijk(&g_aShipPosition_00494550[obj], 0,
                          lateral, vertical, forward);
}

/* Function start: 0x422B1C */
void auto_pilot_sequence(void)
{
    FILE *debugFile;
    short savedCannedSceneMode;
    short leaveCurrentNavPoint;
    short formationSlot;
    FixedVector destination;
    signed char travelMode[10];
    FixedVector travelStep;
    short ship;
    short initialDistance;
    short destinationRange;
    short nearestShipRange;
    short cruiseSpeed;
    short other;
    short missile;

    savedCannedSceneMode = g_nCannedSceneMode_0049021c;
    formationSlot = 0;
    leaveCurrentNavPoint = 1;
    debugFile = 0;
    g_bAutopilotSequenceActive_00493064 = 1;
    destination = g_aMissionObjectives_004932a8[
        g_abFlightPath_004932a0[g_cCurrentNavPointIndex_00493298]].position;

    if (auto_pilot_valid(1) != 0) {
        if (g_bAutopilotDebugEnabled_00499bfc != 0)
            debugFile = fopen("auto.$$$", "w+");
        if (g_ucPendingEjectionTransition_0049b8ac != 0xff &&
            g_nPendingEjectionSequenceCount_0049b8b8 != 0) {
            g_nEjectionSequenceState_0049b8c0 = 0;
            g_nPendingEjectionSequenceCount_0049b8b8 = 0;
            ejection_sequence(
                g_ucPendingEjectionTransition_0049b8ac, 1);
            g_ucPendingEjectionTransition_0049b8ac = 0xff;
            if (g_nArcadeState_0049d75c == 1) {
                g_ucPendingEjectionTransition_0049b8ac = 0xff;
                return;
            }
        }
        if (distance_between_points(
                &g_aMissionObjectives_004932a8[
                    g_cCurrentObjective_004931cc].position,
                &g_aMissionNavPoints_00491e98[
                    g_nCurrentNavPoint_004931bc].position) <
            g_aMissionNavPoints_00491e98[
                g_nCurrentNavPoint_004931bc].proximityRadius + 25)
            leaveCurrentNavPoint = 0;

        for (missile = 0; missile < 10; missile++) {
            if (g_aeObjectClass_00495328[missile] == OBJECT_CLASS_MISSILE)
                remove_object(missile);
        }
        clean_up_cockpit();
        ResetSoundState();
        g_nAutopilotFormationShipCount_00493060 = 0;

        for (ship = 0; ship < 10; ship++) {
            travelMode[ship] = 0;
            g_anShipSpeed_0059b320[ship] = 0;
            zero_vector(&g_aShipVelocity_00494898[ship]);
            if (g_aeObjectClass_00495328[ship] >= OBJECT_CLASS_SHIP &&
                g_aeSpecialManeuver_00495600[ship] !=
                    SPECIAL_MANEUVER_UNKNOWN_9 &&
                g_asShipSide_004955d0[ship] == SIDE_IMPERIAL) {
                if (is_team_member(
                        g_nShipMissionIndices_0059c830[ship]) != 0) {
                    if (kilrathi_near(ship, 10000) == 0) {
                        travelMode[ship] = -1;
                        g_aeSpecialManeuver_00495600[ship] =
                            SPECIAL_MANEUVER_NONE;
                        g_anRollGoal_004954d8[ship] = 0;
                        g_anObjectRollRotation_00495058[ship] = 0;
                        g_anPitchGoal_004954a8[ship] = 0;
                        g_anObjectPitchRotation_00494f38[ship] = 0;
                        g_anYawGoal_004954c0[ship] = 0;
                        g_anObjectYawRotation_00494fc8[ship] = 0;
                        if (ship != 0 &&
                            ship != g_nYourWingman_0049346c)
                            g_nAutopilotFormationShipCount_00493060++;
                    }
                } else if (leaveCurrentNavPoint != 0) {
                    remove_object(ship);
                }
            }
        }

        initialDistance = distance_from_point(0, &destination);
        point_at(0, destination);
        set_speed(0, 60);
        travelMode[0] = 1;
        formationSlot = 0;

        for (ship = 0; ship < 10; ship++) {
            if (travelMode[ship] == -1 && ship != 0) {
                if (distance_from_object(ship, 0) > 20000) {
                    if (player_wingman(ship) != 0)
                        travelMode[ship] = 1;
                    else if (equ_vector(
                                 &g_aShipDestination_004953f0[ship],
                                 &destination) != 0)
                        travelMode[ship] = 2;
                    else
                        travelMode[ship] = 3;
                } else {
                    if (player_wingman(ship) != 0 ||
                        equ_vector(&g_aShipDestination_004953f0[ship],
                                   &destination) != 0)
                        travelMode[ship] = 1;
                    else
                        travelMode[ship] = 3;
                }
                if (travelMode[ship] == 1) {
                    auto_position(ship, &formationSlot);
                    ScaleFixedVector(&g_aShipForwardVector_00494208[0],
                                     0x3c00,
                                     &g_aShipVelocity_00494898[ship]);
                    copy_frame(0, ship);
                    g_anShipSpeed_0059b320[ship] =
                        g_anShipSpeed_0059b320[0];
                    g_aShipVelocity_00494898[ship] =
                        g_aShipVelocity_00494898[0];
                }
            }
        }

        g_nCannedSceneMode_0049021c = 4;
        visit_the_cinema(12, 0, 120);
        while (g_nCannedSceneMode_0049021c == 4) {
            ComputeVectorDelta(&g_aShipPosition_00494550[0],
                               &destination, &travelStep);
            NormalizeFixedVector(&travelStep);
            ScaleFixedVector(&travelStep, 0x19000, &travelStep);
            AddFixedVectors(&g_aShipPosition_00494550[0], &travelStep,
                            &g_aShipPosition_00494550[0]);
            ReleaseStaleNavTarget();
            check_hazards();

            nearestShipRange = 0x7fff;
            destinationRange = distance_from_point(0, &destination);
            for (other = 0; other < 10; other++) {
                if (g_aeObjectClass_00495328[other] >= OBJECT_CLASS_SHIP &&
                    travelMode[other] == 0)
                    nearestShipRange = MinShort(
                        nearestShipRange, distance_from_object(0, other));
            }

            if ((unsigned short)destinationRange < 1000 ||
                g_pActiveHazardField_00493278 != 0 ||
                nearestShipRange < 4000 ||
                report_kilrathi_rout(1) != 0)
                g_nCannedSceneMode_0049021c = savedCannedSceneMode;
        }

        SubtractFixedVectors(&g_aShipPosition_00494550[0], &travelStep,
                             &g_aShipPosition_00494550[0]);
        cruiseSpeed = g_asShipMaximumSpeed_0059c440[0];
        for (ship = 0; ship < 10; ship++) {
            if (travelMode[ship] != 0 &&
                g_aObjectTypeData_00496d30[
                    g_acObjectType_00493980[ship]].cruiseVelocity <
                    cruiseSpeed)
                cruiseSpeed = g_aObjectTypeData_00496d30[
                    g_acObjectType_00493980[ship]].cruiseVelocity;
        }

        formationSlot = 0;
        for (ship = 0; ship < 10; ship++) {
            if (travelMode[ship] != 0) {
                set_speed(ship, cruiseSpeed);
                if (ship != 0 && travelMode[ship] >= 1) {
                    switch (travelMode[ship]) {
                    case 1:
                    case 2:
                        auto_position(ship, &formationSlot);
                        break;
                    case 3:
                        if (distance_from_point(
                                ship,
                                &g_aShipDestination_004953f0[ship]) <
                            initialDistance)
                            g_aShipPosition_00494550[ship] =
                                g_aShipDestination_004953f0[ship];
                        break;
                    }
                }
            }
        }

        Update_3Space();
        if (g_nCockpitDisplayMode_0049d71c == 0) {
            force_view(0, 0);
            SetMousePosition(
                (g_stViewBuffer_005d2b00.right - g_stViewBuffer_005d2b00.left) / 2 + 1,
                (g_stViewBuffer_005d2b00.bottom - g_stViewBuffer_005d2b00.top) / 2);
        } else {
            free_view_buffer();
            SetViewportRect(&g_stViewBuffer_005d2b00, 0, 0,
                            (unsigned short)(g_nScreenWidth_0049d4d8 - 1),
                            (unsigned short)(g_nScreenHeight_0049d4dc - 1));
            initialize_view_buffer();
            g_nCockpitDisplayMode_0049d71c = 1;
            force_view(0, 0);
            g_nCockpitDisplayMode_0049d71c = 1;
            free_view_buffer();
            SetViewportRect(&g_stViewBuffer_005d2b00, 0, 0, 319, 199);
            initialize_view_buffer();
            SetMousePosition(
                (g_stViewBuffer_005d2b00.right - g_stViewBuffer_005d2b00.left) / 2,
                g_nViewCenterY_005c80da);
        }
        if (debugFile != 0)
            fclose(debugFile);
    }
    g_bAutopilotSequenceActive_00493064 = 0;
}
