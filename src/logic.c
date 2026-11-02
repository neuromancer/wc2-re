/*
 *  Ship AI bookkeeping: alerts, collisions, goals and squads.
 *
 *  Retail object 0x429550-0x42BABF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x429550 */
void SendKilrathiAceGreetingOnce(short obj)
{
    if (g_bKilrathiAceGreetingSent_00499bf4 == 0) {
        send_message(obj, 0x17);
        g_bKilrathiAceGreetingSent_00499bf4 = 1;
    }
}

/* Function start: 0x429581 */
short signed_random(short range)
{
    return RandomBelowOrEqual((short)(range * 2)) - range;
}

/* Function start: 0x4295A9 */
short alert_flag(short ship, unsigned int bits)
{
    return (g_anShipAlertFlags_00495f88[ship] & bits) != 0;
}

/* Function start: 0x4295D9 */
void set_alert(short i, unsigned int bits)
{
    g_anShipAlertFlags_00495f88[i] |= bits;
}

/* Function start: 0x4295F2 */
void clear_alert(short i)
{
    g_anShipAlertFlags_00495f88[i] = 0;
    g_acShipAlertSource_00495fb0[i] = -1;
    g_asShipAlertCountdown_00495fc0[i] = 0;
}

/* Function start: 0x429625 */
void start_collision_alert(short obj, short other)
{
    if ((short)g_acShipAlertSource_00495fb0[obj] != other) {
        g_acShipAlertSource_00495fb0[obj] = (signed char)other;
        steady_object(obj);
    }
    g_asShipAlertCountdown_00495fc0[obj] = 3;
    set_alert(obj, 1);
    g_asShipAfterburnerTimer_004955a8[obj] = 0;
    set_special(obj, SPECIAL_MANEUVER_NONE);
}

/* Function start: 0x429698 */
void try2end_collision_alert(short obj)
{
    g_asShipAlertCountdown_00495fc0[obj]--;
    if (g_asShipAlertCountdown_00495fc0[obj] <= 0)
        clear_alert(obj);
    else
        set_alert(obj, 2);
}

/* Function start: 0x4296E2 */
short normal_speed(short obj)
{
    return g_aeSpecialManeuver_00495600[obj] !=
               SPECIAL_MANEUVER_AFTERBURNER &&
           real_velocity(obj) <= g_asShipMaximumVelocity_00495f70[obj];
}

/* Function start: 0x429736 */
short real_crash_time(short obj, short other)
{
    short collisionRadius;
    short time;
    short range;
    short elapsed;
    short step;
    int collisionFound;
    long distance;
    long relativeSpeed;
    FixedVector relativePosition;
    FixedVector relativeVelocity;
    FixedVector travel;
    FixedVector separation;

#ifdef SDL_PORT
    collisionRadius = SdlGetBalancedCollisionRadius(obj);
    collisionRadius += SdlGetBalancedCollisionRadius(other);
#else
    collisionRadius = g_asObjectCollisionRadius_004950e8[obj];
    collisionRadius += g_asObjectCollisionRadius_004950e8[other];
#endif
    collisionRadius += 30;
    collisionFound = 0;
    ComputeVectorDelta(&g_aShipPosition_00494550[obj],
                       &g_aShipPosition_00494550[other],
                       &relativePosition);
    distance = Vector_magnitude(&relativePosition);
    if (((long)collisionRadius + 1500) * 0x100 < distance)
        return 0x7fff;

    ComputeVectorDelta(&g_aShipVelocity_00494898[obj],
                       &g_aShipVelocity_00494898[other],
                       &relativeVelocity);
    relativeSpeed = Vector_magnitude(&relativeVelocity);
    if (relativeSpeed == 0)
        return 0x7fbc;

    time = FixedToShortSaturating(
        DivideFixed((int)distance, (int)relativeSpeed));
    if (time >= 30)
        return time;

    ScaleFixedVector(&relativeVelocity, (int)time << 8, &travel);
    AddFixedVectors(&relativePosition, &travel, &separation);
    range = FixedToShortSaturating(
        Vector_magnitude(&separation));
    if (range > collisionRadius) {
        if ((collisionRadius * 2 >> 4) > range)
            return 25;
        return 32000;
    }

    elapsed = 0;
    do {
        if (elapsed >= time)
            break;
        step = MaxShort(1, (short)((time - elapsed) >> 1));
        elapsed += step;
        ScaleFixedVector(&relativeVelocity, (int)elapsed << 8, &travel);
        AddFixedVectors(&relativePosition, &travel, &separation);
        range = FixedToShortSaturating(
            Vector_magnitude(&separation));
        if (collisionRadius >= range)
            collisionFound = 1;
    } while (collisionFound == 0);
    return elapsed;
}

/* Function start: 0x429958 */
short crash_time(short obj, short other)
{
    if (g_aeObjectClass_00495328[other] == OBJECT_CLASS_ASTEROID &&
        (g_asObjectScreenX_00493598[other] == -0x7fff ||
         g_asObjectScreenX_00493598[obj] == -0x7fff))
        return 0x7fff;
    else
        return real_crash_time(obj, other);
}

/* Function start: 0x4299C9 */
short detect_collisions(short obj)
{
    short time;
    short soonest;
    short candidate;
    short object;

    candidate = -1;
    soonest = 30;
    g_nCollisionCountdown_005d2faa = 1000;
    candidate = -1;
    if (g_pActiveHazardField_00493278 != 0) {
        for (object = 0; object <= 0x42; object++) {
            if (g_aeObjectClass_00495328[object] != OBJECT_CLASS_NULL &&
                object != obj &&
                g_aeObjectClass_00495328[object] >= OBJECT_CLASS_ASTEROID &&
                g_aeObjectClass_00495328[object] != OBJECT_CLASS_MISSILE) {
                time = crash_time(obj, object);
                if (time < soonest) {
                    soonest = time;
                    candidate = object;
                }
            }
        }
    } else {
        for (object = 0; object <= 9; object++) {
            if (g_aeObjectClass_00495328[object] != OBJECT_CLASS_NULL &&
                object != obj &&
                g_aeObjectClass_00495328[object] != OBJECT_CLASS_MISSILE) {
                time = crash_time(obj, object);
                if (time < soonest) {
                    soonest = time;
                    candidate = object;
                }
            }
        }
    }
    if (candidate != -1)
        g_nCollisionCountdown_005d2faa = soonest;
    return candidate;
}

/* Function start: 0x429B55 */
short unactive(short ship)
{
    return ship == -1 ||
           g_aeObjectClass_00495328[ship] < OBJECT_CLASS_SHIP ||
           g_asShipManeuver_00495f48[ship] == MANEUVER_HARD_BRAKE;
}

/* Function start: 0x429BA8 */
short are_alive(short obj)
{
    return unactive(obj) == 0 &&
           g_aeShipObjective_00495f08[obj] != SHIP_OBJECTIVE_NOT_ALIVE;
}

/* Function start: 0x429BF0 */
void trim_goals(short obj, short amount)
{
    if (g_anYawGoal_004954c0[obj] > amount)
        g_anYawGoal_004954c0[obj] = amount;
    else if (g_anYawGoal_004954c0[obj] < -amount)
        g_anYawGoal_004954c0[obj] = -amount;
    if (g_anPitchGoal_004954a8[obj] > amount)
        g_anPitchGoal_004954a8[obj] = amount;
    else if (g_anPitchGoal_004954a8[obj] < -amount)
        g_anPitchGoal_004954a8[obj] = -amount;
}

/* Function start: 0x429CAD */
short report_kilrathi_rout(int mode)
{
    short obj;

    for (obj = 0; obj < 10; obj++) {
        if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver_00495600[obj] !=
                SPECIAL_MANEUVER_UNKNOWN_9) {
            switch (mode) {
            case 0:
                if (g_asShipSide_004955d0[obj] == SIDE_KILRATHI)
                    return 1;
                break;
            case 1:
                if (g_asShipSide_004955d0[obj] == SIDE_KILRATHI &&
                    g_asShipMissionType_00495de8[obj] !=
                        MISSION_TYPE_ROUT &&
                    distance_from_object(0, obj) < 16000)
                    return 1;
                break;
            case 2:
                if (g_asShipSide_004955d0[obj] == SIDE_KILRATHI &&
                    (g_aeShipObjective_00495f08[obj] ==
                         OBJECTIVE_ENGAGE_ENEMY ||
                     g_aeShipObjective_00495f08[obj] ==
                         OBJECTIVE_DESTROY_SHIP))
                    return 1;
                break;
            }
        }
    }
    if (mode == 1 && g_nCurrentWave_004931c0 != -1)
        ProcessEnemyWaveCompletion();
    return 0;
}

/* Function start: 0x429E24 */
short find_ship_index(short missionShip)
{
    short obj;

    g_nLastFoundShip_005d2fc8 = -1;
    if (missionShip != -1) {
        for (obj = 0; obj < 10; obj++) {
            if (((g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_SHIP &&
                  g_aeSpecialManeuver_00495600[obj] !=
                      SPECIAL_MANEUVER_UNKNOWN_9) ||
                 g_aeObjectClass_00495328[obj] == OBJECT_CLASS_FUTURION) &&
                g_asShipMissionIndex_00495d00[obj] == missionShip) {
                g_nLastFoundShip_005d2fc8 = obj;
                return g_nLastFoundShip_005d2fc8;
            }
        }
    }
    return g_nLastFoundShip_005d2fc8;
}

/* Function start: 0x429EE0 */
short try2rout(short obj)
{
    short other;
    short hasBase;

    hasBase = 0;
    if (any_enemy(obj, 10000) != 0) {
        for (other = 0; other < 10; other++) {
            if (g_aeObjectClass_00495328[other] >=
                    OBJECT_CLASS_CAPITAL_SHIP &&
                g_aeSpecialManeuver_00495600[other] !=
                    SPECIAL_MANEUVER_UNKNOWN_9 &&
                g_asShipSide_004955d0[other] ==
                    g_asShipSide_004955d0[obj])
                hasBase = 1;
        }
    }
    if (hasBase != 0) {
        g_acShipStress_00496100[obj] = 0;
        maneuver_complete(obj);
    } else {
        reset_mission_type(obj, MISSION_TYPE_ROUT);
        if (obj == g_nYourWingman_0049346c) {
            g_acMissionShipStatusFlags_005d2fb0[0] |= 4;
            send_message(obj, 0xb);
        }
    }
    return (short)(hasBase == 0);
}

/* Function start: 0x42A003 */
signed char CanSetNewShipTurnGoal(short ship)
{
    if (GetAdaptiveTurnRate() > 120)
        return 1;
    return (g_anYawGoal_004954c0[ship] |
            g_anPitchGoal_004954a8[ship] |
            g_anRollGoal_004954d8[ship]) == 0;
}

/* Function start: 0x42A062 */
signed char no_goal(short ship)
{
    return (g_anYawGoal_004954c0[ship] |
            g_anPitchGoal_004954a8[ship] |
            g_anRollGoal_004954d8[ship]) == 0;
}

/* Function start: 0x42A0A9 */
short being_tailed(short obj, short other)
{
    get_facing_range_from_object(obj, other);
    return g_nFacingToTarget_00493194 < -70 &&
           g_nTargetFacing_00493198 > 90 &&
           g_nTargetRange_0049319c < 4000;
}

/* Function start: 0x42A108 */
short detect_enemy_tail(short obj)
{
    short other;

    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass_00495328[other] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver_00495600[other] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[other] &&
            g_acShipTarget_00495f20[other] == obj &&
            being_tailed(obj, other) != 0)
            return other;
    }
    return -1;
}

/* Function start: 0x42A1C5 */
short is_ship_tailing_player_target(short obj)
{
    short target = g_acShipTarget_00495f20[0];

    if (unactive(target) == 0 && being_tailed(target, obj) != 0)
        return 1;
    return 0;
}

/* Function start: 0x42A222 */
short FindMissileTargetingObject(short obj)
{
    short other;

    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass_00495328[other] == OBJECT_CLASS_MISSILE &&
            g_acShipTarget_00495f20[other] == obj)
            return other;
    }
    return 0;
}

/* Function start: 0x42A28E */
void build_squad_list(short leader)
{
    short index;
    short obj;

    index = 0;
    g_acShipList_00496148[index++] = (signed char)leader;
    g_acShipList_00496148[index] = -1;
    for (obj = 0; obj < 10; obj++) {
        if (g_asShipWingLeader_00495dd0[obj] == leader) {
            g_acShipList_00496148[index++] = (signed char)obj;
            g_acShipList_00496148[index] = -1;
        }
    }
}

/* Function start: 0x42A313 */
unsigned int find_squad_center(FixedVector *center)
{
    short count;

    zero_vector(center);
    count = -1;
    while (g_acShipList_00496148[++count] != -1) {
        AddFixedVectors(center, &g_aShipPosition_00494550[
                            (short)g_acShipList_00496148[count]],
                        center);
    }
    if (count != 0)
        divide_vector(center, (int)count << 8, center);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42A39A */
void init_formation_burst(short obj)
{
    FixedVector center;
    short member;
    short i;

    i = -1;
    build_squad_list(obj);
    find_squad_center(&center);
    while ((member = g_acShipList_00496148[++i]) != -1) {
        ComputeVectorDelta(&center, &g_aShipPosition_00494550[member],
                           &g_aShipDestination_004953f0[member]);
        ScaleFixedVector(&g_aShipDestination_004953f0[member], 0xa00,
                         &g_aShipDestination_004953f0[member]);
        AddFixedVectors(&g_aShipDestination_004953f0[member],
                        &g_aShipPosition_00494550[member],
                        &g_aShipDestination_004953f0[member]);
        steady_object(member);
        reset_objective(member, OBJECTIVE_BREAK_FORMATION);
    }
}

/* Function start: 0x42A490 */
void reset_mission_type(short obj, short missionType)
{
    if (missionType == MISSION_TYPE_ROUT &&
        g_asShipSide_004955d0[obj] == SIDE_KILRATHI)
        report_kilrathi_rout(1);
    reset_objective(obj, OBJECTIVE_NONE);
    g_asShipMissionType_00495de8[obj] = missionType;
}

/* Function start: 0x42A4E5 */
unsigned int change_mission_type(short obj, short missionType)
{
    if (g_aeShipObjective_00495f08[obj] == OBJECTIVE_ENGAGE_ENEMY)
        g_asShipMissionType_00495de8[obj] = missionType;
    else
        reset_mission_type(obj, missionType);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42A52A */
unsigned int reset_objective(short ship, short objective)
{
    steady_object(ship);
    reset_tactic(ship, TACTIC_NONE);
    g_aeShipObjective_00495f08[ship] = objective;
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42A55F */
unsigned int alter_objective(short ship, short objective)
{
    steady_object(ship);
    alter_tactic(ship, TACTIC_NONE);
    g_aeShipObjective_00495f08[ship] = objective;
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42A594 */
unsigned int reset_tactic(short ship, short tactic)
{
    reset_maneuver(ship, MANEUVER_NONE);
    g_asShipTactic_00495f30[ship] = tactic;
    g_acShipTarget_00495f20[ship] = -1;
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42A5C8 */
unsigned int alter_tactic(short ship, short tactic)
{
    reset_maneuver(ship, MANEUVER_NONE);
    g_asShipTactic_00495f30[ship] = tactic;
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42A5F1 */
void reset_maneuver(short ship, short maneuver)
{
    g_asShipManeuver_00495f48[ship] = (enum ShipManeuver)maneuver;
    g_acShipSequence_00495fe8[ship] = 0;
    g_asShipCount_00495ff8[ship] = 0;
}

/* Function start: 0x42A625 */
void try2reset_maneuver(short obj, short maneuver)
{
    if (g_asShipManeuver_00495f48[obj] != maneuver) {
        reset_maneuver(obj, maneuver);
        steady_object(obj);
    }
}

/* Function start: 0x42A664 */
void set_special(short ship, short special)
{

    if (g_aeSpecialManeuver_00495600[ship] <
            SPECIAL_MANEUVER_LOST_CONTROL ||
        special > g_aeSpecialManeuver_00495600[ship])
        g_aeSpecialManeuver_00495600[ship] = special;

    if (g_aeSpecialManeuver_00495600[ship] ==
            SPECIAL_MANEUVER_BLOWING_UP &&
        (short)alert_flag(ship, 1) != 0)
        g_aeSpecialManeuver_00495600[ship] = SPECIAL_MANEUVER_NONE;
}

/* Function start: 0x42A6E8 */
unsigned int approach_zero_speed(short ship)
{
    approach_speed(ship, 0);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42A701 */
unsigned int approach_min_speed(short obj)
{
    approach_speed(obj, 0x500);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42A71D */
void approach_half_speed(short obj)
{
    approach_speed(
        obj,
        (g_aObjectTypeData_00496d30[
             g_acObjectType_00493980[obj]].cruiseVelocity & ~1) << 7);
}

/* Function start: 0x42A754 */
unsigned int approach_cruise_speed(short ship)
{
    approach_speed(ship,
        (int)g_aObjectTypeData_00496d30[
            g_acObjectType_00493980[ship]].cruiseVelocity << 8);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42A788 */
unsigned int approach_full_speed(short ship)
{
    approach_speed(ship,
                      (int)g_asShipMaximumVelocity_00495f70[ship] << 8);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42A7AF */
unsigned int approach_ship_speed(short obj, short other)
{
    approach_speed(obj, g_anShipSpeed_00494e20[other]);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42A7D2 */
void ComputePointAheadOfObject(short obj, unsigned short distance,
                               FixedVector *point)
{
    ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                     (int)distance << 8, point);
    AddFixedVectors(&g_aShipPosition_00494550[obj], point, point);
}

/* Function start: 0x42A823 */
void ComputePointBehindObject(short obj, unsigned short distance,
                              FixedVector *point)
{
    ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                     -((int)distance << 8), point);
    AddFixedVectors(&g_aShipPosition_00494550[obj], point, point);
}

/* Function start: 0x42A876 */
short close_behind(short range)
{
    return g_nTargetRange_0049319c < range && g_nTargetFacing_00493198 < -0x32;
}

/* Function start: 0x42A8B5 */
short scan_for_enemy(short obj, unsigned short range)
{
    short other;
    short distance;

    g_nTargetRange_0049319c = 0;
    g_nTargetShip_004931a0 = -1;
    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass_00495328[other] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver_00495600[other] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide_004955d0[other] != g_asShipSide_004955d0[obj] &&
            target_valid(obj, other) != 0) {
            distance =
                distance_from_point(other, &g_aShipPosition_00494550[obj]);
            if (distance < range &&
                (g_nTargetShip_004931a0 == -1 ||
                 distance < g_nTargetRange_0049319c)) {
                g_nTargetShip_004931a0 = other;
                g_nTargetRange_0049319c = distance;
            }
        }
    }
    if (g_nTargetShip_004931a0 != -1) {
        get_facing_range_from_object(obj, g_nTargetShip_004931a0);
        g_nTargetRange_0049319c =
            distance_from_object(obj, g_nTargetShip_004931a0);
    }
    return g_nTargetShip_004931a0;
}

short distance_from_object(short obj, short other);

/* Function start: 0x42AA0D */
short any_enemy(short obj, short range)
{
    short other;

    g_nTargetShip_004931a0 = -1;
    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass_00495328[other] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver_00495600[other] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[other]) {
            g_nTargetRange_0049319c = distance_from_object(obj, other);
            if (g_nTargetRange_0049319c < range) {
                g_nTargetShip_004931a0 = other;
                return 1;
            }
        }
    }
    return 0;
}

short ships_within_range(short obj, short other, short range);

/* Function start: 0x42AAD3 */
/* Nothing in the shipped executable calls this.  Like any_enemy, but only
 * reports whether a live enemy ship is closer than range. */
short enemy_within_range(short obj, short range)
{
    short other;
    short enemyRange;

    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass_00495328[other] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver_00495600[other] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[other]) {
            enemyRange = distance_from_object(obj, other);
            if (range > enemyRange)
                return 1;
        }
    }
    return 0;
}

/* Function start: 0x42AB81 */
short ships_within_range(short obj, short other, short range)
{
    FixedVector delta;

    ComputeVectorDelta(&g_aShipPosition_00494550[obj],
                       &g_aShipPosition_00494550[other], &delta);
    return IsVectorWithinRange(&delta, range);
}

/* Function start: 0x42ABCE */
short attacker_in_range(short obj, unsigned short range)
{
    short other;

    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass_00495328[other] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver_00495600[other] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide_004955d0[other] != g_asShipSide_004955d0[obj] &&
            g_acShipTarget_00495f20[other] == obj) {
            g_nTargetRange_0049319c = distance_from_object(other, obj);
            if (g_nTargetRange_0049319c < range) {
                g_nTargetShip_004931a0 = other;
                return 1;
            }
        }
    }
    g_nTargetShip_004931a0 = -1;
    return 0;
}

short distance_from_object(short obj, short other);

/* Function start: 0x42ACAF */
short in_danger(short obj)
{
    unsigned short range;
    short attacker;

    g_nTargetShip_004931a0 = -1;
    for (attacker = 0; attacker < 10; attacker++) {
        if (g_aeObjectClass_00495328[attacker] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver_00495600[attacker] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide_004955d0[attacker] != g_asShipSide_004955d0[obj] &&
            g_acShipTarget_00495f20[attacker] == obj) {
            range = (unsigned short)distance_from_object(attacker, obj);
            if (g_nTargetShip_004931a0 == -1 ||
                range < g_nTargetRange_0049319c) {
                g_nTargetShip_004931a0 = attacker;
                g_nTargetRange_0049319c = (short)range;
            }
        }
    }
    return g_nTargetShip_004931a0 != -1;
}

/* Function start: 0x42ADB8 */
short target_within_range(short obj)
{
    short target = g_acShipTarget_00495f20[obj];

    if (unactive(target) != 0)
        return 0;
    if (target_valid(obj, target) == 0)
        return 0;
    return ships_within_range(obj, target, 7000);
}

short target_within_range(short obj);
short unactive(short ship);
short target_valid(short obj, short target);

/* Function start: 0x42AE32 */
short build_target_list(short obj, short range)
{
    short count;
    short other;
    short distance;

    count = 0;
    for (other = 0; other < 10; other++) {
        if ((g_aeObjectClass_00495328[other] >= OBJECT_CLASS_SHIP &&
             g_aeSpecialManeuver_00495600[other] !=
                 SPECIAL_MANEUVER_UNKNOWN_9 &&
             g_asShipSide_004955d0[other] != g_asShipSide_004955d0[obj] &&
             g_aeObjectClass_00495328[other] == OBJECT_CLASS_SHIP) ||
            (g_aeObjectClass_00495328[other] == OBJECT_CLASS_MISSILE &&
             g_asObjectType_00495298[other] == 0x13 &&
             g_acShipTarget_00495f20[other] == obj)) {
            distance = distance_from_object(obj, other);
            if (distance < range) {
                g_acShipList_00496148[count] =
                    (signed char)other;
                g_asShipListRange_00496158[count] = distance;
                count++;
            }
        }
    }
    g_acShipList_00496148[count] = -1;
    return count;
}

/* Function start: 0x42AF60 */
void inherit_leader_mission(short obj)
{
    short leader = g_asShipWingLeader_00495dd0[obj];

    if (leader != -1 &&
        g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_SHIP) {
        g_asShipMissionType_00495de8[obj] =
            g_asShipMissionType_00495de8[leader];
        g_asShipMissionParameter_00495e00[obj] =
            g_asShipMissionParameter_00495e00[leader];
        g_aShipMissionSpot_00495e18[obj] =
            g_aShipMissionSpot_00495e18[leader];
        g_asShipWingLeader_00495dd0[obj] = -1;
    }
}

/* Function start: 0x42B00A */
void inherit_leader(short obj)
{
    short leader = g_asShipWingLeader_00495dd0[obj];
    short other;

    if (leader != -1 && g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_SHIP) {
        inherit_leader_mission(obj);
        g_asShipWingLeader_00495dd0[obj] =
            g_asShipWingLeader_00495dd0[leader];
        for (other = 0; other < 10; other++) {
            if (g_asShipWingLeader_00495dd0[other] == leader)
                g_asShipWingLeader_00495dd0[other] = obj;
        }
    }
}

/* Function start: 0x42B0B7 */
short dead_ship(short i)
{
    return i == -1 || g_aMissionShips_00492290[i].state == 3;
}

/* Function start: 0x42B0FB */
short gone_ship(short missionShip)
{
    return missionShip == -1 ||
           g_aMissionShips_00492290[missionShip].state == 3 ||
           g_aMissionShips_00492290[missionShip].state == 2;
}

/* Function start: 0x42B15A */
/* WC2 passes a margin every caller ignores; the body never reads it. */
short skill_check(short obj, short margin)
{
    if (g_asPilotLevel_00495d60[obj] < 5 &&
        RandomBelowOrEqual((short)(g_asPilotLevel_00495d60[obj] + 4)) == 0)
        return 0;
    return 1;
}

/* Function start: 0x42B1AE */
short find_ships_sphere(short missionShip)
{
    short navIndex;
    MissionNavPoint *navPoint;
    short fallback;
    short shipIndex;

    fallback = -1;
    navPoint = g_aMissionNavPoints_00491e98;
    navIndex = 0;
    for (; navIndex < MISSION_NAV_POINT_COUNT;
         navIndex++, navPoint++) {
        for (shipIndex = 0; shipIndex < MISSION_NAV_POINT_COUNT;
             shipIndex++) {
            if (navPoint->missionShips[shipIndex] == missionShip) {
                if (navPoint->type == 1)
                    return navIndex;
                else if (fallback == -1)
                    fallback = navIndex;
            }
        }
    }
    if (fallback != -1) {
        navIndex = fallback;
        while (g_aMissionNavPoints_00491e98[navIndex].type > 1)
            navIndex--;
        return navIndex;
    }
    return -1;
}

/* Function start: 0x42B2A7 */
short locate_ship(short missionShip, FixedVector *point)
{
    short navPoint;
    short obj;

    obj = find_ship_index(missionShip);
    if (obj != -1) {
        *point = g_aShipPosition_00494550[obj];
        return 1;
    }
    if (dead_ship(missionShip) != 0)
        return 0;
    navPoint = find_ships_sphere(missionShip);
    if (navPoint != -1) {
        AddFixedVectors(&g_aMissionNavPoints_00491e98[navPoint].position,
                        &g_aMissionShips_00492290[missionShip].position,
                        point);
        return 1;
    }
    zero_vector(point);
    return 0;
}

/* Function start: 0x42B38F */
void get_follow_point(short obj, FixedVector *point)
{
    short pathStep;
    short missionIndex;
    short shipObj;
    short navObjective;

    if (g_asShipSide_004955d0[obj] == SIDE_KILRATHI) {
        *point = g_aMissionNavPoints_00491e98[
            g_asShipMissionParameter_00495e00[obj]].position;
        return;
    }
    pathStep = (short)g_abShipNavPointIndex_00495f60[obj];
    while (++pathStep < MISSION_OBJECTIVE_COUNT) {
        navObjective = (short)g_abFlightPath_004932a0[pathStep];
        switch (g_aMissionObjectives_004932a8[navObjective].type) {
        case 0:
            *point = g_aMissionObjectives_004932a8[navObjective].position;
            g_abShipNavPointIndex_00495f60[obj] = (signed char)pathStep;
            return;
        case 1:
            missionIndex =
                (short)g_aMissionObjectives_004932a8[navObjective].index;
            shipObj = find_ship_index(missionIndex);
            if (shipObj == -1)
                *point = g_aMissionNavPoints_00491e98[missionIndex].position;
            else
                *point = g_aShipPosition_00494550[shipObj];
            g_abShipNavPointIndex_00495f60[obj] = (signed char)pathStep;
            return;
        }
    }
}

/* Function start: 0x42B524 */
unsigned int get_first_follow_point(short obj, FixedVector *point)
{
    if (g_asShipSide_004955d0[obj] == SIDE_IMPERIAL)
        g_abShipNavPointIndex_00495f60[obj] =
            g_cCurrentNavPointIndex_00493298 - 1;
    get_follow_point(obj, point);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42B565 */
short hostile_sphere(short obj, short navPoint)
{
    short slot;
    short missionIndex;

    for (slot = 0; slot < 10; slot++) {
        missionIndex =
            g_aMissionNavPoints_00491e98[navPoint].missionShips[slot];
        if (missionIndex != -1 &&
            g_aMissionShips_00492290[missionIndex].side !=
                g_asShipSide_004955d0[obj])
            return 1;
    }
    return 0;
}

/* Function start: 0x42B5F7 */
short abandoned(short obj, short other)
{
    short navPoint;

    if ((g_abShipTurn_00495fd8[obj] & 7) != 0 ||
        g_asShipSide_004955d0[obj] == SIDE_KILRATHI ||
        RandomBelowOrEqual(8) != 0)
        return 0;
    navPoint = FindNearestNavPoint(obj);
    if (g_nCurrentNavPoint_004931bc != navPoint &&
        hostile_sphere(obj, navPoint) != 0 &&
        distance_from_object(obj, other) > 10000) {
        explode(-1, obj);
        return 1;
    }
    return 0;
}

/* Function start: 0x42B6C5 */
void engage(short obj, short target, short objective)
{
    if (g_aeShipObjective_00495f08[obj] != objective) {
        reset_objective(obj, objective);
        if (g_asPilotLevel_00495d60[obj] == 5 &&
            g_asShipSide_004955d0[obj] == SIDE_KILRATHI)
            SendKilrathiAceGreetingOnce(obj);
    }
    g_acShipTarget_00495f20[obj] = target;
}

short ShipHasTorpedo(short ship);
short CanShipWeaponDamageTarget(short ship, short target);
short triumph(short obj);

/* Function start: 0x42B73B */
short target_valid(short obj, short target)
{
    if (target == -1)
        return 0;
    if (g_anShipCloakState_00496020[target] == 1 &&
        g_asShipCloakElapsedFrames_00496060[target] >= 40)
        return 0;
    if (ShipHasTorpedo(obj) == 0 &&
        CanShipWeaponDamageTarget(obj, target) == 0)
        return 0;
    return unactive(target) == 0 &&
           g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[target];
}

/* Function start: 0x42B80C */
short triumph(short obj)
{
    short result;
    short objective;

    result = 0;
    switch (g_asShipMissionType_00495de8[obj]) {
    case MISSION_TYPE_PATROL:
        objective = 0;
        while (g_abFlightPath_004932a0[objective] != -1) {
            if (visited((short)g_abFlightPath_004932a0[objective]) != 0)
                objective++;
            else
                break;
        }
        result = 1;
        break;
    case MISSION_TYPE_ESCORT:
    case MISSION_TYPE_DEFEND:
    case MISSION_TYPE_WINGMAN:
        result = dead_ship(g_asShipMissionParameter_00495e00[obj]) == 0;
        break;
    case MISSION_TYPE_STRIKE:
        result = dead_ship(g_asShipMissionParameter_00495e00[obj]);
        break;
    }
    return result;
}

/* Function start: 0x42B916 */
short find_ratio(short inputMinimum, short inputMaximum,
                         short input, short outputMinimum,
                         short outputMaximum)
{
    if (input < inputMinimum)
        return outputMinimum;
    if (input > inputMaximum)
        return outputMaximum;
    return outputMinimum + ((int)(outputMaximum - outputMinimum) *
                            (int)(input - inputMinimum)) /
                               (int)(inputMaximum - inputMinimum);
}

/* Function start: 0x42B985 */
short evaluate_damage(short obj)
{
    short total;
    ObjectTypeData *typeData;

    total = 0;
    typeData = &g_aObjectTypeData_00496d30[g_acObjectType_00493980[obj]];
    if (g_aeObjectClass_00495328[obj] < OBJECT_CLASS_SHIP)
        return 100;
    total = (short)(g_aasShipArmor_00495540[obj][0] * 23 /
                        typeData->armorFront + total);
    total = (short)(g_aasShipArmor_00495540[obj][1] * 27 /
                        typeData->armorRear + total);
    total = (short)(g_aasShipArmor_00495540[obj][2] * 12 /
                        typeData->armorLeft + total);
    total = (short)(g_aasShipArmor_00495540[obj][3] * 12 /
                        typeData->armorRight + total);
    total = (short)(26 - g_acShipDamage_00495690[obj] * 26 /
                             typeData->damageCapacity + total);
    return total;
}
