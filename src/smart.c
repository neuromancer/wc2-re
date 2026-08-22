/*
 *  Ship collision avoidance, formation following, and maneuver selection.
 *
 *  Address range 0x433ac0-0x434ccf.  The boundary is corroborated by the
 *  ordered MacsBug symbols in the Super Wing Commander `smart` unit: its
 *  final routine is chase_speed, immediately followed by the random unit.
 */
#include "game.h"

/* Function start: 0x41E4E0 */
#pragma function(abs)
void steer_away_from_object(short obj, short other, short amount)
{
    FixedVector predicted;
    FixedVector difference;
    FixedVector relative;

    g_anRollGoal_004954d8[obj] = 0;
    AddFixedVectors(&g_aShipPosition_00494550[other],
                    &g_aShipVelocity_00494898[other], &predicted);
    ComputeVectorDelta(&g_aShipPosition_00494550[obj],
                       &predicted, &difference);
    transform_to_objects_frame(&difference, &relative, obj);
    if (abs(relative.x) < abs(relative.y)) {
        if (relative.x > 0)
            amount = -amount;
        g_anYawGoal_004954c0[obj] = amount;
    } else {
        if (relative.y > 0)
            amount = -amount;
        g_anPitchGoal_004954a8[obj] = amount;
    }
}

/* Function start: 0x41E5C8 */
void steer_away_from_predicted_object(short obj, short other,
                                      short predictionTicks, short amount)
{
    FixedVector predicted;
    FixedVector difference;
    FixedVector relative;

    g_anRollGoal_004954d8[obj] = 0;
    ScaleFixedVector(&g_aShipVelocity_00494898[other],
                     (int)predictionTicks << 8, &predicted);
    AddFixedVectors(&g_aShipPosition_00494550[other],
                    &predicted, &predicted);
    ComputeVectorDelta(&g_aShipPosition_00494550[obj],
                       &predicted, &difference);
    transform_to_objects_frame(&difference, &relative, obj);
    if (abs(relative.x) < abs(relative.y)) {
        if (relative.x < 0)
            amount = -amount;
        g_anYawGoal_004954c0[obj] = amount;
    } else {
        if (relative.y > 0)
            amount = -amount;
        g_anPitchGoal_004954a8[obj] = amount;
    }
}

/* Function start: 0x41E6C8 */
void prevent_collision(short obj)
{
    short collisionTime;
    short other;

    other = g_acShipAlertSource_00495fb0[obj];
    collisionTime = 0;
    if (other == -1) {
        clear_alert(obj);
        return;
    }
    collisionTime = g_nCollisionCountdown_005d2faa;
    if (collisionTime >= 30) {
        approach_full_speed(obj);
        try2end_collision_alert(obj);
        return;
    }
    get_facing_range_from_point(obj, &g_aShipPosition_00494550[other]);
    if (g_nFacingToTarget_00493194 > 75) {
        if (g_nTargetFacing_00493198 < -70)
            approach_zero_speed(obj);
        else
            approach_full_speed(obj);
    } else if (g_nFacingToTarget_00493194 < -70 &&
               normal_speed(obj) != 0) {
        fire_afterburner(obj, 8);
    } else {
        approach_full_speed(obj);
    }
    if (CanSetNewShipTurnGoal(obj) != 0) {
        if (g_nFacingToTarget_00493194 < -60 &&
            g_nTargetFacing_00493198 > 60) {
            veer_random(obj, 14);
        } else {
            steer_away_from_predicted_object(
                obj, other, (short)(collisionTime >> 1),
                MaxShort(0, MinShort(g_nFacingToTarget_00493194, 25)));
        }
    }
}

/* Function start: 0x41E860 */
short handle_collisions(short obj)
{
    short other;

    other = detect_collisions(obj);
    if (other != -1)
        start_collision_alert(obj, other);
    if (alert_flag(obj, 1) != 0)
        prevent_collision(obj);
    return alert_flag(obj, 1);
}

/* Function start: 0x41E8D3 */
short CountEnemyFighters(void)
{
    short count;
    short obj;

    count = 0;
    for (obj = 0; obj < 10; obj++) {
        if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP &&
            g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[0])
            count++;
    }
    return count;
}

/* Function start: 0x41E945 */
short regulate_turn(short obj)
{
    if (g_aeSpecialManeuver_00495600[obj] ==
        SPECIAL_MANEUVER_UNKNOWN_9)
        return 1;
    g_acTurnRegulator_00495618[obj]--;
    if (g_aeObjectClass_00495328[obj] < OBJECT_CLASS_CAPITAL_SHIP &&
        handle_collisions(obj) != 0)
        return 1;
    if ((g_nSpaceFrame_00493134 & 1) != 0 &&
        obj == g_nYourWingman_0049346c &&
        g_bWingmanTurnRateUnlocked_00493040 == 0)
        return 1;
    if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP &&
        GetAdaptiveTurnRate() >= 120 &&
        CountEnemyFighters() < 3 &&
        g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[0])
        g_acTurnRegulator_00495618[obj] = 0;
    if (g_acTurnRegulator_00495618[obj] > 0)
        return 1;
    g_abShipTurn_00495fd8[obj]++;
    g_acTurnRegulator_00495618[obj] = g_acTurnInterval_00495628[obj];
    return 0;
}

/* Function start: 0x41EA86 */
unsigned int select_target(short obj)
{
    if (g_asShipSide_004955d0[obj] == SIDE_KILRATHI) {
        if (RandomBelow(100) < 50) {
            if (attacker_in_range(0, 16000) == 0) {
                if (distance_from_object(obj, 0) < 5000) {
                    g_acShipTarget_00495f20[obj] = 0;
                    return 0;
                }
            }
        }
    }
    g_acShipTarget_00495f20[obj] =
        (signed char)scan_for_enemy(obj, 16000);
    return 0;
}

/* Function start: 0x41EB1D */
unsigned int veer_random(short obj, short amount)
{
    switch (RandomBelowOrEqual(3)) {
    case 0:
        g_anYawGoal_004954c0[obj] = amount;
        return 0;
    case 1:
        g_anYawGoal_004954c0[obj] = -amount;
        return 0;
    case 2:
        g_anPitchGoal_004954a8[obj] = amount;
        return 0;
    case 3:
        g_anPitchGoal_004954a8[obj] = -amount;
        break;
    }
    return 0;
}

/* Function start: 0x41EBC1 */
unsigned int offset_location(short obj, const ShortVector *offset,
                             FixedVector *location)
{
    *location = g_aShipPosition_00494550[obj];
    position_relative(location, g_aShipForwardVector_00494208[obj],
                      offset->z);
    position_relative(location, g_aShipUpVector_00493ec0[obj],
                      offset->y);
    position_relative(location, g_aShipRightVector_00493b78[obj],
                      offset->x);
    return 0;
}

/* Function start: 0x41EC73 */
unsigned int compute_formation_destination(short leader,
                                           const ShortVector *offset,
                                           FixedVector *destination)
{
    offset_location(leader, offset, destination);
    AddFixedVectors(destination, &g_aShipVelocity_00494898[leader],
                    destination);
    AddFixedVectors(destination, &g_aShipVelocity_00494898[leader],
                    destination);
    AddFixedVectors(destination, &g_aShipVelocity_00494898[leader],
                    destination);
    return 0;
}

/* Function start: 0x41ECEF */
unsigned int control_speed(short obj, unsigned short range,
                           int desiredSpeed)
{
    unsigned short travelTime;
    int brakingMargin;

    travelTime = range /
        MaxShort(FixedToShortSaturating(g_anShipSpeed_00494e20[obj]), 1);
    brakingMargin = GetShipAccelerationRate(obj) * travelTime;
    brakingMargin -=
        AbsInt(desiredSpeed - g_anShipSpeed_00494e20[obj]);
    if (brakingMargin <= 0) {
        approach_speed(obj, desiredSpeed);
        return 0;
    }
    if (brakingMargin < 12800) {
        celerate(obj, 0x100);
        return 0;
    }
    celerate(obj, GetShipAccelerationRate(obj));
    return 0;
}

/* Function start: 0x41EDCC */
unsigned int chase_location(short obj, const FixedVector *destination,
                            short reference)
{
    int pointAtDestination;
    int desiredSpeed;
    short forwardFacing;
    short speed;
    FixedVector forwardTravel;
    FixedVector projectedDestination;

    pointAtDestination = 0;
    desiredSpeed = g_anShipSpeed_00494e20[reference] - 0x200;
    if (desiredSpeed < 0)
        desiredSpeed = 0;
    ScaleFixedVector(&g_aShipForwardVector_00494208[reference],
                     g_anShipSpeed_00494e20[reference] * 15,
                     &forwardTravel);
    AddFixedVectors(destination, &forwardTravel, &projectedDestination);
    get_facing_range_from_point(obj, &projectedDestination);
    forwardFacing = g_nFacingToTarget_00493194;
    get_facing_range_from_point(obj, destination);
    if (g_nFacingToTarget_00493194 < 0) {
        approach_speed(obj, 0);
        speed = FixedToShortSaturating(g_anShipSpeed_00494e20[reference]);
        speed = MaxShort(speed, 1);
        if (g_nTargetRange_0049319c / speed > 49)
            pointAtDestination = 1;
    } else if (g_nTargetRange_0049319c > 175 && CanSetNewShipTurnGoal(obj) != 0) {
        pointAtDestination = 1;
    }
    control_speed(obj, (unsigned short)g_nTargetRange_0049319c,
                  desiredSpeed);
    if (g_nFacingToTarget_00493194 > 85 &&
        g_nTargetRange_0049319c > 175)
        g_anRollGoal_004954d8[obj] =
            match_roll_orientation(obj, reference);
    if (pointAtDestination != 0)
        point_ship(obj, 0, &g_vToTarget_00493188);
    if (g_nTargetRange_0049319c < 175) {
        if (forwardFacing > 90) {
            copy_frame(reference, obj);
            steady_object(obj);
            approach_speed(obj, g_anShipSpeed_00494e20[reference]);
            return 0;
        }
        point_ship(obj, 0, &forwardTravel);
    }
    if (g_nTargetRange_0049319c < 600)
        trim_goals(obj, 10);
    return 0;
}

/* Function start: 0x41EFFE */
void goto_location(short obj, const FixedVector *destination)
{
    get_facing_range_from_point(obj, destination);
    if (g_nFacingToTarget_00493194 < 51) {
        approach_half_speed(obj);
        if (CanSetNewShipTurnGoal(obj) != 0)
            point_ship_at_point(obj, destination);
    } else if (g_nTargetRange_0049319c > 3000 ||
               g_nTargetRange_0049319c /
                   MaxShort(FixedToShortSaturating(
                                g_anShipSpeed_00494e20[obj]),
                            1) > 50) {
        approach_full_speed(obj);
    } else {
        approach_cruise_speed(obj);
    }
}

/* Function start: 0x41F0C5 */
unsigned int goto_formation(short obj, const FixedVector *destination,
                            short leader)
{
    short forwardFacing;
    FixedVector projected;

    AddFixedVectors(&g_aShipPosition_00494550[obj],
                    &g_aShipForwardVector_00494208[leader], &projected);
    get_facing_range_from_point(obj, &projected);
    forwardFacing = g_nFacingToTarget_00493194;
    get_facing_range_from_point(obj, destination);
    if (g_nFacingToTarget_00493194 < 40) {
        approach_min_speed(obj);
    } else if (g_nTargetRange_0049319c > 2000) {
        approach_full_speed(obj);
        if (g_nTargetRange_0049319c > 3000 &&
            g_nFacingToTarget_00493194 > 70 && normal_speed(obj) != 0)
            fire_afterburner(obj, 5);
    } else if (g_nTargetRange_0049319c > 200) {
        control_speed(obj, (unsigned short)g_nTargetRange_0049319c,
                      g_anShipSpeed_00494e20[leader]);
    } else {
        approach_ship_speed(obj, leader);
    }
    if (g_nTargetRange_0049319c < 200) {
        steady_object(obj);
        if (forwardFacing > 90) {
            copy_frame(leader, obj);
            return 0;
        }
        point_parallel(obj, leader);
    } else if (CanSetNewShipTurnGoal(obj) != 0) {
        point_ship(obj, 0, &g_vToTarget_00493188);
    }
    if (g_nTargetRange_0049319c < 700) {
        trim_goals(obj, 10);
        if (forwardFacing > 90)
            g_anRollGoal_004954d8[obj] =
                match_roll_orientation(obj, leader);
    }
    return 0;
}

/* Function start: 0x41F2C5 */
unsigned int maintain_formation(short obj)
{
    short leader;
    FixedVector destination;

    leader = g_asShipWingLeader_00495dd0[obj];
    compute_formation_destination(leader,
                                  &g_aShipFormationOffset_00495468[obj],
                                  &destination);
    g_aShipDestination_004953f0[obj] = destination;
    goto_formation(obj, &destination, leader);
    return 0;
}

/* Function start: 0x41F337 */
unsigned int reset_stress(short obj)
{
    short damage;

    if (obj >= 12) {
        if (g_acPilotAggression_00493030[g_asPilotLevel_00495d60[obj]] == 0)
            damage = 100;
        else
            damage = evaluate_damage(obj);
        if (damage < 30) {
            damage = 30;
        } else if (damage < 75) {
            damage = find_ratio(30, 74, damage, 29, 15);
        } else {
            damage = find_ratio(75, 100, damage, 14, 0);
        }
        g_acShipStress_00496100[obj] = (signed char)damage;
    }
    return 0;
}

/* Function start: 0x41F3EF */
short stress_morale(short obj)
{
    if (g_acShipStress_00496100[obj] < 15)
        return 0;
    if (g_acShipStress_00496100[obj] < 30)
        return 1;
    return 2;
}

/* Function start: 0x41F446 */
enum ShipManeuver any_defense(short obj)
{
    short count;
    short pilotLevel;
    signed char *maneuvers;

    count = 0;
    pilotLevel = g_asPilotLevel_00495d60[obj];
    if (pilotLevel > 4)
        pilotLevel = 4;
    maneuvers = g_apDefenseManeuvers_00493018[pilotLevel];
    while (maneuvers[count] != -1)
        count++;
    return (enum ShipManeuver)
        maneuvers[RandomBelowOrEqual((short)(count - 1))];
}

/* Function start: 0x41F4C6 */
short pick_regular_maneuver(short obj, short event)
{
    short reroll;

    reroll = RandomBelowOrEqual(100) < 3 ||
             g_asShipManeuver_00495f48[obj] == MANEUVER_NONE;
    if (stress_morale(obj) == 2)
        return MANEUVER_OUTA_HERE;
    if ((g_asIntelligenceEvent_00492fc0[obj] == event ||
         RandomBelowOrEqual(100) < 20) &&
        (event == 0 || event == 3 || event == 4 || event == 7) &&
        reroll == 0)
        return g_asShipManeuver_00495f48[obj];

    switch (event) {
    case 0:
    case 1:
        if (g_aeObjectClass_00495328[
                g_acShipTarget_00495f20[obj]] >=
            OBJECT_CLASS_CAPITAL_SHIP)
            return MANEUVER_STRAFE_ENEMY;
        if (g_asPilotLevel_00495d60[obj] * 5 + 60 > RandomBelow(100))
            return MANEUVER_ZIP_PAST;
        return any_defense(obj);
    case 3:
        return any_defense(obj);
    case 2:
        return MANEUVER_TRY2TAIL;
    case 4:
        if (g_asPilotLevel_00495d60[obj] * 20 + 30 <=
            RandomBelowOrEqual(100))
            return any_defense(obj);
        return MANEUVER_STRAFE_ENEMY;
    case 5:
        return MANEUVER_TAIL_FIRE;
    case 6:
        if (g_asPilotLevel_00495d60[obj] >= 2)
            return MANEUVER_HARD_TURN;
        else
            return MANEUVER_WABBLE;
    case 7:
        return any_defense(obj);
    case 8:
        return MANEUVER_LINE_UP_DROP;
    }
    return MANEUVER_ROLL_OVER;
}

/* Function start: 0x41F714 */
enum ShipManeuver pick_from_list(const ManeuverChoice *choice, short obj,
                                 short event)
{
    enum ShipManeuver maneuver;
    short chooseAgain;

    maneuver = g_asShipManeuver_00495f48[obj];
    if (maneuver == MANEUVER_NONE)
        chooseAgain = 1;
    else
        chooseAgain = 0;
    if (((choice->primary != maneuver &&
          choice->secondary != maneuver &&
          RandomBelow(100) < 10) ||
         g_asIntelligenceEvent_00492fc0[obj] != event) &&
        choice->primary < MANEUVER_UNKNOWN_48 &&
        choice->secondary < MANEUVER_UNKNOWN_48)
        chooseAgain = 1;
    if (chooseAgain == 0) {
        if (RandomBelowOrEqual(100) < 5)
            chooseAgain = 1;
    }
    if (chooseAgain != 0) {
        if (RandomBelowOrEqual(99) >= choice->threshold)
            maneuver = (enum ShipManeuver)choice->secondary;
        else
            maneuver = (enum ShipManeuver)choice->primary;
    }
    return maneuver;
}

/* Function start: 0x41F83F */
enum ShipManeuver pick_kilrathi_maneuver(short obj, int event)
{
    const ManeuverChoice *choice;
    enum ShipManeuver maneuver;

    choice = &((const ManeuverChoice (*)[9][3])
                   g_aShipIntelligenceData_005d3060)[
        g_asShipIntelSlot_00495d30[obj]][event][stress_morale(obj)];
    maneuver = pick_from_list(choice, obj, (short)event);
    switch (maneuver) {
    case MANEUVER_UNKNOWN_48:
        maneuver = MANEUVER_STRAFE_ENEMY;
        if (event == 5)
            maneuver = MANEUVER_ZIP_PAST;
        break;
    case MANEUVER_UNKNOWN_49:
        maneuver = any_defense(obj);
        break;
    default:
        break;
    }
    return maneuver;
}

/* Function start: 0x41F902 */
/* How many earlier ships on this ship's side are already pressing the same
 * target while its intelligence event is 5. */
short CountAlliesOnSameTarget(short obj)
{
    short count;
    short other;

    count = 0;
    for (other = 1; other < obj; other++) {
        if (g_aeObjectClass_00495328[other] == OBJECT_CLASS_SHIP &&
            g_asShipSide_004955d0[other] == g_asShipSide_004955d0[obj] &&
            g_acShipTarget_00495f20[other] == g_acShipTarget_00495f20[obj] &&
            g_asIntelligenceEvent_00492fc0[obj] == 5)
            count++;
    }
    return count;
}

/* Function start: 0x41F9AF */
void process_maneuver_node(short obj, short event)
{
    const ManeuverChoice *choice;
    short maneuver;
    short index;

    maneuver = (short)g_asShipManeuver_00495f48[obj];
    g_bWingmanTurnRateUnlocked_00493040 = 0;
    if ((g_asPilotLevel_00495d60[obj] >= 5 &&
         g_asShipSide_004955d0[obj] != 0) ||
        g_nYourWingman_0049346c == obj) {
        choice = &((const ManeuverChoice (*)[9][3])
                       g_aShipIntelligenceData_005d3060)[
            g_asShipIntelSlot_00495d30[obj]][event][stress_morale(obj)];
        maneuver = (short)pick_from_list(choice, obj, (short)event);
    } else if (g_asShipSide_004955d0[obj] == 1) {
        maneuver = (short)pick_kilrathi_maneuver(obj, event);
    } else {
        maneuver = pick_regular_maneuver(obj, event);
    }

    if (g_asShipSide_004955d0[obj] == 1 &&
        g_nFacingToTarget_00493194 > 0x61 &&
        g_nTargetRange_0049319c < 4000)
        fire(obj, g_acShipTarget_00495f20[obj]);

    if (g_nYourWingman_0049346c == obj) {
        for (index = 0; index < 8; index++) {
            if (g_stMissionHeader_005d3e70.initialMissionShips[index] ==
                g_asShipMissionIndex_00495d00[obj])
                break;
        }
        if (g_acInitialShipKillCount_005d2fc0[index] <
            g_cPlayerKillCount_005d2fa8)
            g_bWingmanTurnRateUnlocked_00493040 = 1;
        else
            g_bWingmanTurnRateUnlocked_00493040 = 0;
    }

    if ((event == 3 || event == 7) &&
        g_nTargetRange_0049319c < 2000 &&
        g_nTargetFacing_00493198 > 0x5e)
        fire_afterburner(obj, 10);

    if (g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[0] &&
        GetAdaptiveTurnRate() > 0x78)
        goto pressAttack;
    if (g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[0] ||
        g_bWingmanTurnRateUnlocked_00493040 == 0)
        goto chooseEvasion;

pressAttack:
    if (g_acShipTarget_00495f20[obj] != -1 && event != 4 &&
        g_aeSpecialManeuver_00495600[
            g_acShipTarget_00495f20[obj]] == 1)
        fire_afterburner(obj, 10);

    if (event != 5 &&
        g_asShipCloakCooldown_00496048[obj] == 0 &&
        g_anShipCloakState_00496020[obj] == 0)
        maneuver = 0x2f;

chooseEvasion:
    if (DAT_004960f0[obj] > 0)
        maneuver = 0x2e;

    if (maneuver == 0x28 && CountAlliesOnSameTarget(obj) > 1) {
        if (event == 5)
            maneuver = 0x2e;
        else
            maneuver = 0x12;
    }
    if (g_asShipManeuver_00495f48[obj] != maneuver)
        reset_maneuver(obj, maneuver);
}

/* Function start: 0x41FD2B */
void handle_stress(short obj, short event)
{
    short aggression;
    short damage;

    aggression =
        g_acPilotAggression_00493030[g_asPilotLevel_00495d60[obj]];
    switch (event) {
    case 3:
    case 4:
    case 7:
        g_acShipStress_00496100[obj] += aggression;
        break;
    case 5:
        g_acShipStress_00496100[obj] -= aggression;
        break;
    case 6:
        g_acShipStress_00496100[obj] += aggression * 2;
        break;
    case 8:
        g_acShipStress_00496100[obj] /= 2;
        break;
    case -1:
    case 2:
        g_acShipStress_00496100[obj] -=
            g_acPilotRecovery_00493038[g_asPilotLevel_00495d60[obj]];
        break;
    }
    damage = evaluate_damage(obj);
    if (damage < 40) {
        g_acShipStress_00496100[obj] += aggression * 2;
    } else if (damage < 75) {
        g_acShipStress_00496100[obj] =
            (signed char)MinShort(
                (short)((short)g_acShipStress_00496100[obj] + aggression),
                28);
    } else {
        g_acShipStress_00496100[obj] =
            (signed char)MinShort(g_acShipStress_00496100[obj], 7);
    }
    if (event == 6 && g_acShipStress_00496100[obj] >= 30)
        g_acShipStress_00496100[obj] = 29;
    if (g_acShipStress_00496100[obj] < 0)
        g_acShipStress_00496100[obj] = 0;
}

/* Function start: 0x41FF37 */
void intelligence_events(short obj)
{
    short event;
    short targetGone;
    short target;
    short previousStress;
    short playerDamage;

    event = -1;
    target = g_acShipTarget_00495f20[obj];
    previousStress = (short)g_acShipStress_00496100[obj];
    targetGone = 0;
    if (unactive(target) == 0) {
        get_facing_range_from_object(obj, g_acShipTarget_00495f20[obj]);
        if (g_aeSpecialManeuver_00495600[
                g_acShipTarget_00495f20[obj]] ==
            SPECIAL_MANEUVER_UNKNOWN_9) {
            event = 8;
        } else {
            event = 0;
            if (g_nTargetRange_0049319c > 8000) {
                event = 2;
            } else if (g_nFacingToTarget_00493194 > 55 &&
                       g_nTargetFacing_00493198 < -55) {
                event = 5;
            } else if (g_nFacingToTarget_00493194 > 75 &&
                       g_nTargetFacing_00493198 > 75 &&
                       g_nTargetRange_0049319c < 5000) {
                event = 4;
                if (g_acShipCollisionCooldown_00496010[obj] > 0 &&
                    g_asObjectDamage_00495178[obj] > 0)
                    event = 7;
            } else if (FindMissileTargetingObject(obj) != 0) {
                event = 6;
            } else if (g_acShipCollisionCooldown_00496010[obj] > 0) {
                event = 7;
            } else if (g_nFacingToTarget_00493194 < -70 &&
                       g_nTargetFacing_00493198 > 90 &&
                       g_nTargetRange_0049319c < 4000 &&
                       g_nTargetRange_0049319c < 5000) {
                event = 3;
                if (g_asShipSide_004955d0[obj] == SIDE_KILRATHI &&
                    GetAdaptiveTurnRate() <= 30)
                    event = 0;
            } else if (g_anShipSpeed_00494e20[target] < 5120) {
                event = 1;
            }
        }
    } else if (FindMissileTargetingObject(obj) != 0) {
        event = 6;
    } else if (g_acShipCollisionCooldown_00496010[obj] > 0) {
        event = 7;
    } else {
        targetGone = 1;
    }

    handle_stress(obj, event);
    if (event != -1)
        process_maneuver_node(obj, event);
    if (event == -1 && targetGone != 0) {
        if (any_enemy(obj, 16000) == 0)
            reset_objective(obj, OBJECTIVE_NONE);
        else
            select_target(obj);
        reset_stress(obj);
    }

    if (g_nYourWingman_0049346c == obj &&
        g_aeObjectClass_00495328[0] == OBJECT_CLASS_SHIP &&
        g_acShipPendingMessage_00495d98[g_nYourWingman_0049346c] == -1) {
        if (previousStress < 15 && g_acShipStress_00496100[obj] >= 15 &&
            RandomBelowOrEqual(3) == 0) {
            send_message(obj, 4);
        } else {
            playerDamage = evaluate_damage(0);
            if (RandomBelow(1000) < 4 && playerDamage < 35) {
                if (evaluate_damage(obj) > playerDamage)
                    send_message(obj, 10);
                else
                    send_message(obj, 4);
            }
        }
    }
    g_asIntelligenceEvent_00492fc0[obj] = event;
}

/* Function start: 0x440571 */
void chase_speed(short obj, short range)
{
    if (range < g_nTargetRange_0049319c) {
        approach_full_speed(obj);
        if ((short)(range * 2) < g_nTargetRange_0049319c)
            fire_afterburner(obj, 2);
    } else if (range > g_nTargetRange_0049319c) {
        approach_zero_speed(obj);
    } else {
        approach_speed(obj,
                       g_anShipSpeed_00494e20[
                           g_acShipTarget_00495f20[obj]]);
    }
}
