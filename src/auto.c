/*
 *  Autopilot travel and interstitial flight sequences.
 *
 *  Retail object 0x4228B0-0x42347F (link order; see docs/ORDER.md).
 */
#include "game.h"

short g_nAutopilotFormationShipCount = 0; /* 0x00493060 */
short g_bAutopilotSequenceActive = 0; /* 0x00493064 */

/* WC2's auto.c declares the autopilot debug flag two bytes wide where every
 * other translation unit declares it as a char.  In the retail image the byte
 * above it is padding, so the value is the same and only the load width
 * differs; on the port the byte above it is the sanitizer's redzone, so read
 * the flag at its own width there. */
#ifdef SDL_PORT
#define g_nAutopilotDebugEnabled g_bAutopilotDebugEnabled
#else
#define g_nAutopilotDebugEnabled \
    (*(const short *)&g_bAutopilotDebugEnabled)
#endif


/* Function start: 0x4228B0 */
void visit_the_cinema(int view, int obj, short frames)
{
    int originalUnlock;
    int savedInputFlag;
    int savedKeyboardFlag;

    originalUnlock = g_nOriginDevUnlock;
    savedInputFlag = g_bPlayerDamageEnabled;
    savedKeyboardFlag = g_bPlayerCollisionEnabled;
    g_nOriginDevUnlock = 1;
    g_bPlayerDamageEnabled = 0;
    g_bPlayerCollisionEnabled = 0;
    /* The retail call converts both int parameters implicitly.  Making the
     * narrowing explicit changes the register setup and loses an exact match. */
    force_view(view, obj);
    while (frames-- > 0) {
        Update_3Space();
        RenderSpaceViewFrame();
        MarkDibDirty();
        DIBslamReal();
    }
    g_nOriginDevUnlock = originalUnlock;
    g_bPlayerDamageEnabled = (short)savedInputFlag;
    g_bPlayerCollisionEnabled = (short)savedKeyboardFlag;
}

/* Function start: 0x422953 */
short player_wingman(short obj)
{
    return obj != -1 && g_asShipWingLeader[obj] == 0;
}

/* Function start: 0x422990 */
void set_speed(short obj, short speed)
{
    g_anShipSpeed[obj] = (int)speed << 8;
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
        position_relative_ijk(&g_aShipPosition[obj], 0,
                              g_aShipFormationOffset[obj].x,
                              g_aShipFormationOffset[obj].y,
                              g_aShipFormationOffset[obj].z);
        return;
    }

    (*formationSlot)++;
    if ((*formationSlot & 1) != 0)
        sideways = 650;
    else
        sideways = -650;
    if (g_nAutopilotFormationShipCount == *formationSlot)
        sideways = 0;
    vertical = 0;
    forward = (short)(MaxShort(1, (short)(*formationSlot >> 1)) * -1800);
    if (g_nYourWingman != -1) {
        if (abs(
            g_aShipFormationOffset[g_nYourWingman].z -
            forward) <
            g_asObjectCollisionRadius[obj] +
            g_asObjectCollisionRadius[g_nYourWingman])
            vertical = 500;
    }
    position_relative_ijk(&g_aShipPosition[obj], 0,
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

    savedMode = g_nTrainSimActive;
    slot = 0;
    moveOn = 1;
    autoLog = 0;
    g_bAutopilotSequenceActive = 1;
    destination = g_aMissionObjectives[
        g_abFlightPath[g_cCurrentNavPointIndex]].position;

    if (auto_pilot_valid(1) == 0)
        return;
    if (g_nAutopilotDebugEnabled != 0)
        autoLog = fopen("auto.$$$", "w+");
    if (g_nAutopilotDebugEnabled != 0)
        LogMemoryStateToFile(autoLog);
    if (g_ucPendingEjectionTransition != 0xff &&
        g_nPendingEjectionSequenceCount != 0) {
        g_nEjectionSequenceState = 0;
        g_nPendingEjectionSequenceCount = 0;
        ejection_sequence(
            g_ucPendingEjectionTransition, 1);
        g_ucPendingEjectionTransition = 0xff;
        if (g_nArcadeState == 1)
            return;
    }
    if (distance_between_points(
            &g_aMissionObjectives[
                g_cCurrentObjective].position,
            &g_aMissionNavPoints[
                g_nCurrentNavPoint].position) <
        g_aMissionNavPoints[
            g_nCurrentNavPoint].proximityRadius + 25)
        moveOn = 0;

    for (ordnance = 0; ordnance < 10; ordnance++) {
        if (g_aeObjectClass[ordnance] == OBJECT_CLASS_MISSILE)
            remove_object(ordnance);
    }
    clean_up_cockpit();
    ResetSoundState();
    g_nAutopilotFormationShipCount = 0;

    for (obj = 0; obj < 10; obj++) {
        inFormation[obj] = 0;
        g_anShipSpeed[obj] = 0;
        zero_vector(&g_aShipVelocity[obj]);
        if (g_aeObjectClass[obj] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver[obj] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide[obj] == SIDE_IMPERIAL) {
            if (is_team_member(
                    g_asShipMissionIndex[obj]) != 0) {
                if (kilrathi_near(obj, 10000) == 0) {
                    inFormation[obj] = -1;
                    g_anObjectYawRotation[obj] =
                        g_anYawGoal[obj] =
                        g_anObjectPitchRotation[obj] =
                        g_anPitchGoal[obj] =
                        g_anObjectRollRotation[obj] =
                        g_anRollGoal[obj] = 0;
                    g_aeSpecialManeuver[obj] =
                        SPECIAL_MANEUVER_NONE;
                    if (obj != 0 &&
                        obj != g_nYourWingman)
                        g_nAutopilotFormationShipCount++;
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
                             &g_aShipDestination[obj],
                             &destination) != 0 &&
                         g_asShipSide[obj] ==
                             g_asShipSide[0])
                    inFormation[obj] = 2;
                else
                    inFormation[obj] = 3;
            } else {
                if (player_wingman(obj) != 0 ||
                    equ_vector(&g_aShipDestination[obj],
                               &destination) != 0)
                    inFormation[obj] = 1;
                else
                    inFormation[obj] = 3;
            }
            if (inFormation[obj] == 1 &&
                g_asShipSide[obj] ==
                    g_asShipSide[0]) {
                auto_position(obj, &slot);
                ScaleFixedVector(&g_aShipForwardVector[0],
                                 0x3c00,
                                 &g_aShipVelocity[obj]);
                copy_frame(0, obj);
                g_anShipSpeed[obj] =
                    g_anShipSpeed[0];
                g_aShipVelocity[obj] =
                    g_aShipVelocity[0];
            }
        }
    }

    g_nTrainSimActive = 4;
    visit_the_cinema(13, 0, 90);
    while (g_nTrainSimActive == 4) {
        ComputeVectorDelta(&g_aShipPosition[0],
                           &destination, &advance);
        NormalizeFixedVector(&advance);
        ScaleFixedVector(&advance, 0x19000, &advance);
        AddFixedVectors(&g_aShipPosition[0], &advance,
                        &g_aShipPosition[0]);
        ReleaseStaleNavTarget();
        check_hazards();

        destinationRange = distance_from_point(0, &destination);
        nearest = 0x7fff;
        for (obj = 0; obj < 10; obj++) {
            if (g_aeObjectClass[obj] >= OBJECT_CLASS_SHIP &&
                inFormation[obj] == 0)
                nearest = MinShort(
                    nearest, distance_from_object(0, obj));
        }

        if ((unsigned short)destinationRange < 1000 ||
            g_pActiveHazardField != 0 ||
            nearest < 4000 ||
            report_kilrathi_rout(1) != 0)
            g_nTrainSimActive = savedMode;
    }

    SubtractFixedVectors(&g_aShipPosition[0], &advance,
                         &g_aShipPosition[0]);
    cruiseSpeed = g_asShipMaximumVelocity[0];
    for (obj = 0; obj < 10; obj++) {
        if (inFormation[obj] != 0) {
            ownCruise = g_aObjectTypeData[
                g_acObjectType[obj]].cruiseVelocity;
            if (ownCruise < cruiseSpeed)
                cruiseSpeed = ownCruise;
        }
    }

    slot = 0;
    for (obj = 0; obj < 10; obj++) {
        if (inFormation[obj] != 0 &&
            g_asShipSide[obj] == g_asShipSide[0]) {
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
                            &g_aShipDestination[obj]) <
                        startRange)
                        g_aShipPosition[obj] =
                            g_aShipDestination[obj];
                    break;
                }
            }
        }
    }

    Update_3Space();
    if (g_nCockpitDisplayMode == 0) {
        force_view(0, 0);
    } else {
        free_view_buffer();
        SetViewportRect(&g_27_stViewScratchInfo, 0, 0,
                        (unsigned short)(g_nScreenWidth - 1),
                        (unsigned short)(g_nScreenHeight - 1));
        initialize_view_buffer();
        g_nCockpitDisplayMode = 1;
        force_view(0, 0);
        g_nCockpitDisplayMode = 1;
        free_view_buffer();
        SetViewportRect(&g_27_stViewScratchInfo, 0, 0, 319, 199);
        initialize_view_buffer();
    }
    if (g_78_bHighMemoryBuffersSetState != 0)
        ResetCannedSceneRecording();
    if (g_nAutopilotDebugEnabled != 0)
        LogMemoryStateToFile(autoLog);
    g_bAutopilotSequenceActive = 0;
    if (autoLog != 0)
        fclose(autoLog);
}
