/*
 *  Ship AI bookkeeping: alerts, collisions, goals and squads.
 *
 *  Retail object 0x429550-0x42BABF (link order; see docs/ORDER.md).
 */
#define LOGIC_C
#include "game.h"

/* Function start: 0x429550 */
void SendKilrathiAceGreetingOnce(short obj)
{
    if (g_bKilrathiAceGreetingSent == 0) {
        send_message(obj, 0x17);
        g_bKilrathiAceGreetingSent = 1;
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
    return (g_anShipAlertFlags[ship] & bits) != 0;
}

/* Function start: 0x4295D9 */
void set_alert(short i, unsigned int bits)
{
    g_anShipAlertFlags[i] |= bits;
}

/* Function start: 0x4295F2 */
void clear_alert(short i)
{
    g_anShipAlertFlags[i] = 0;
    g_acShipAlertSource[i] = -1;
    g_asShipAlertCountdown[i] = 0;
}

/* Function start: 0x429625 */
void start_collision_alert(short obj, short other)
{
    if ((short)g_acShipAlertSource[obj] != other) {
        g_acShipAlertSource[obj] = (signed char)other;
        steady_object(obj);
    }
    g_asShipAlertCountdown[obj] = 3;
    set_alert(obj, 1);
    g_asShipAfterburnerTimer[obj] = 0;
    set_special(obj, SPECIAL_MANEUVER_NONE);
}

/* Function start: 0x429698 */
void try2end_collision_alert(short obj)
{
    g_asShipAlertCountdown[obj]--;
    if (g_asShipAlertCountdown[obj] <= 0)
        clear_alert(obj);
    else
        set_alert(obj, 2);
}

/* Function start: 0x4296E2 */
short normal_speed(short obj)
{
    return g_aeSpecialManeuver[obj] !=
               SPECIAL_MANEUVER_AFTERBURNER &&
           real_velocity(obj) <= g_asShipMaximumVelocity[obj];
}

/* Function start: 0x429736 */
short real_crash_time(short obj, short other)
{
    short crash;
    short gap;
    short contactRange;
    short time;
    long speed;
    FixedVector future;
    long startDistance;
    FixedVector relativeVelocity;
    FixedVector currentOffset;
    FixedVector travelled;
    short checkTime;

    crash = 0;
#ifdef SDL_PORT
    contactRange = (short)(SdlGetBalancedCollisionRadius(obj) +
                           SdlGetBalancedCollisionRadius(other) + 30);
#else
    contactRange = (short)(g_asObjectCollisionRadius[obj] +
                           g_asObjectCollisionRadius[other] + 30);
#endif
    ComputeVectorDelta(&g_aShipPosition[obj],
                       &g_aShipPosition[other],
                       &currentOffset);
    startDistance = Vector_magnitude(&currentOffset);
    ComputeVectorDelta(&g_aShipVelocity[obj],
                       &g_aShipVelocity[other],
                       &relativeVelocity);
    speed = Vector_magnitude(&relativeVelocity);
    if (speed == 0)
        return 0x7fbc;

    time = FixedToShortSaturating(
        DivideFixed((int)startDistance, (int)speed));
    if (time >= 30)
        return time;

    ScaleFixedVector(&relativeVelocity, (int)time << 8, &travelled);
    AddFixedVectors(&currentOffset, &travelled, &future);
    gap = FixedToShortSaturating(Vector_magnitude(&future));
    if (contactRange < gap) {
        if ((contactRange + contactRange) >> 4 > gap)
            return 25;
        else
            return 32000;
    }

    checkTime = 0;
    while (crash == 0 && checkTime < time) {
        checkTime += MaxShort(1, (short)((time - checkTime) >> 1));
        ScaleFixedVector(&relativeVelocity, (int)checkTime << 8, &travelled);
        AddFixedVectors(&currentOffset, &travelled, &future);
        gap = FixedToShortSaturating(Vector_magnitude(&future));
        if (contactRange >= gap)
            crash = 1;
    }
    return checkTime;
}

/* Function start: 0x429958 */
short crash_time(short obj, short other)
{
    if (g_aeObjectClass[other] == OBJECT_CLASS_ASTEROID &&
        (g_asObjectScreenX[other] == -0x7fff ||
         g_asObjectScreenX[obj] == -0x7fff))
        return 0x7fff;
    else
        return real_crash_time(obj, other);
}

/* Function start: 0x4299C9 */
short detect_collisions(short obj)
{
    short object;
    short time;
    short soonest;
    short candidate;

    candidate = -1;
    soonest = 30;
    g_5_nCollisionCountdownNumber = 1000;
    candidate = -1;
    if (g_pActiveHazardField != 0) {
        for (object = 0; object <= 0x42; object++) {
            if (g_aeObjectClass[object] != OBJECT_CLASS_NULL &&
                object != obj &&
                g_aeObjectClass[object] >= OBJECT_CLASS_ASTEROID &&
                g_aeObjectClass[object] != OBJECT_CLASS_MISSILE) {
                time = crash_time(obj, object);
                if (time < soonest) {
                    soonest = time;
                    candidate = object;
                }
            }
        }
    } else {
        for (object = 0; object <= 9; object++) {
            if (g_aeObjectClass[object] != OBJECT_CLASS_NULL &&
                object != obj &&
                g_aeObjectClass[object] != OBJECT_CLASS_MISSILE) {
                time = crash_time(obj, object);
                if (time < soonest) {
                    soonest = time;
                    candidate = object;
                }
            }
        }
    }
    if (candidate != -1)
        g_5_nCollisionCountdownNumber = soonest;
    return candidate;
}

/* Function start: 0x429B55 */
short unactive(short ship)
{
    return ship == -1 ||
           g_aeObjectClass[ship] < OBJECT_CLASS_SHIP ||
           g_asShipManeuver[ship] == MANEUVER_HARD_BRAKE;
}

/* Function start: 0x429BA8 */
short are_alive(short obj)
{
    return unactive(obj) == 0 &&
           g_aeShipObjective[obj] != SHIP_OBJECTIVE_NOT_ALIVE;
}

/* Function start: 0x429BF0 */
void trim_goals(short obj, short amount)
{
    if (g_anYawGoal[obj] > amount)
        g_anYawGoal[obj] = amount;
    else if (g_anYawGoal[obj] < -amount)
        g_anYawGoal[obj] = -amount;
    if (g_anPitchGoal[obj] > amount)
        g_anPitchGoal[obj] = amount;
    else if (g_anPitchGoal[obj] < -amount)
        g_anPitchGoal[obj] = -amount;
}

/* Function start: 0x429CAD */
short report_kilrathi_rout(int mode)
{
    short obj;

    for (obj = 0; obj < 10; obj++) {
        if (g_aeObjectClass[obj] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver[obj] !=
                SPECIAL_MANEUVER_UNKNOWN_9) {
            switch (mode) {
            case 0:
                if (g_asShipSide[obj] == SIDE_KILRATHI)
                    return 1;
                break;
            case 1:
                if (g_asShipSide[obj] == SIDE_KILRATHI &&
                    g_asShipMissionType[obj] !=
                        MISSION_TYPE_ROUT &&
                    distance_from_object(0, obj) < 16000)
                    return 1;
                break;
            case 2:
                if (g_asShipSide[obj] == SIDE_KILRATHI &&
                    (g_aeShipObjective[obj] ==
                         OBJECTIVE_ENGAGE_ENEMY ||
                     g_aeShipObjective[obj] ==
                         OBJECTIVE_DESTROY_SHIP))
                    return 1;
                break;
            }
        }
    }
    if (mode == 1 && g_nCurrentWave != -1)
        ProcessEnemyWaveCompletion();
    return 0;
}

/* Function start: 0x429E24 */
short find_ship_index(short missionShip)
{
    short obj;

    g_nLastFoundShip = -1;
    if (missionShip != -1) {
        for (obj = 0; obj < 10; obj++) {
            if (((g_aeObjectClass[obj] >= OBJECT_CLASS_SHIP &&
                  g_aeSpecialManeuver[obj] !=
                      SPECIAL_MANEUVER_UNKNOWN_9) ||
                 g_aeObjectClass[obj] == OBJECT_CLASS_FUTURION) &&
                g_asShipMissionIndex[obj] == missionShip) {
                g_nLastFoundShip = obj;
                return g_nLastFoundShip;
            }
        }
    }
    return g_nLastFoundShip;
}

/* Function start: 0x429EE0 */
short try2rout(short obj)
{
    short other;
    short hasBase;

    hasBase = 0;
    if (any_enemy(obj, 10000) != 0) {
        for (other = 0; other < 10; other++) {
            if (g_aeObjectClass[other] >=
                    OBJECT_CLASS_CAPITAL_SHIP &&
                g_aeSpecialManeuver[other] !=
                    SPECIAL_MANEUVER_UNKNOWN_9 &&
                g_asShipSide[other] ==
                    g_asShipSide[obj])
                hasBase = 1;
        }
    }
    if (hasBase != 0) {
        g_acShipStress[obj] = 0;
        maneuver_complete(obj);
    } else {
        reset_mission_type(obj, MISSION_TYPE_ROUT);
        if (obj == g_nYourWingman) {
            g_acMissionShipStatusFlagsBytes[0] |= 4;
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
    return (g_anYawGoal[ship] |
            g_anPitchGoal[ship] |
            g_anRollGoal[ship]) == 0;
}

/* Function start: 0x42A062 */
signed char no_goal(short ship)
{
    return (g_anYawGoal[ship] |
            g_anPitchGoal[ship] |
            g_anRollGoal[ship]) == 0;
}

/* Function start: 0x42A0A9 */
short being_tailed(short obj, short other)
{
    get_facing_range_from_object(obj, other);
    return g_nFacingToTarget < -70 &&
           g_nTargetFacing > 90 &&
           g_nTargetRange < 4000;
}

/* Function start: 0x42A108 */
short detect_enemy_tail(short obj)
{
    short other;

    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass[other] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver[other] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide[obj] != g_asShipSide[other] &&
            g_acShipTarget[other] == obj &&
            being_tailed(obj, other) != 0)
            return other;
    }
    return -1;
}

/* Function start: 0x42A1C5 */
short is_ship_tailing_player_target(short obj)
{
    short target = g_acShipTarget[0];

    if (unactive(target) == 0 && being_tailed(target, obj) != 0)
        return 1;
    return 0;
}

/* Function start: 0x42A222 */
short FindMissileTargetingObject(short obj)
{
    short other;

    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass[other] == OBJECT_CLASS_MISSILE &&
            g_acShipTarget[other] == obj)
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
    g_acShipList[index++] = (signed char)leader;
    g_acShipList[index] = -1;
    for (obj = 0; obj < 10; obj++) {
        if (g_asShipWingLeader[obj] == leader) {
            g_acShipList[index++] = (signed char)obj;
            g_acShipList[index] = -1;
        }
    }
}

/* Function start: 0x42A313 */
unsigned int find_squad_center(FixedVector *center)
{
    short count;

    zero_vector(center);
    count = -1;
    while (g_acShipList[++count] != -1) {
        AddFixedVectors(center, &g_aShipPosition[
                            (short)g_acShipList[count]],
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
    while ((member = g_acShipList[++i]) != -1) {
        ComputeVectorDelta(&center, &g_aShipPosition[member],
                           &g_aShipDestination[member]);
        ScaleFixedVector(&g_aShipDestination[member], 0xa00,
                         &g_aShipDestination[member]);
        AddFixedVectors(&g_aShipDestination[member],
                        &g_aShipPosition[member],
                        &g_aShipDestination[member]);
        steady_object(member);
        reset_objective(member, OBJECTIVE_BREAK_FORMATION);
    }
}

/* Function start: 0x42A490 */
void reset_mission_type(short obj, short missionType)
{
    if (missionType == MISSION_TYPE_ROUT &&
        g_asShipSide[obj] == SIDE_KILRATHI)
        report_kilrathi_rout(1);
    reset_objective(obj, OBJECTIVE_NONE);
    g_asShipMissionType[obj] = missionType;
}

/* Function start: 0x42A4E5 */
unsigned int change_mission_type(short obj, short missionType)
{
    if (g_aeShipObjective[obj] == OBJECTIVE_ENGAGE_ENEMY)
        g_asShipMissionType[obj] = missionType;
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
    g_aeShipObjective[ship] = objective;
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42A55F */
unsigned int alter_objective(short ship, short objective)
{
    steady_object(ship);
    alter_tactic(ship, TACTIC_NONE);
    g_aeShipObjective[ship] = objective;
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42A594 */
unsigned int reset_tactic(short ship, short tactic)
{
    reset_maneuver(ship, MANEUVER_NONE);
    g_asShipTactic[ship] = tactic;
    g_acShipTarget[ship] = -1;
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42A5C8 */
unsigned int alter_tactic(short ship, short tactic)
{
    reset_maneuver(ship, MANEUVER_NONE);
    g_asShipTactic[ship] = tactic;
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42A5F1 */
void reset_maneuver(short ship, short maneuver)
{
    g_asShipManeuver[ship] = (enum ShipManeuver)maneuver;
    g_acShipSequence[ship] = 0;
    g_asShipCount[ship] = 0;
}

/* Function start: 0x42A625 */
void try2reset_maneuver(short obj, short maneuver)
{
    if (g_asShipManeuver[obj] != maneuver) {
        reset_maneuver(obj, maneuver);
        steady_object(obj);
    }
}

/* Function start: 0x42A664 */
void set_special(short ship, short special)
{

    if (g_aeSpecialManeuver[ship] <
            SPECIAL_MANEUVER_LOST_CONTROL ||
        special > g_aeSpecialManeuver[ship])
        g_aeSpecialManeuver[ship] = special;

    if (g_aeSpecialManeuver[ship] ==
            SPECIAL_MANEUVER_BLOWING_UP &&
        (short)alert_flag(ship, 1) != 0)
        g_aeSpecialManeuver[ship] = SPECIAL_MANEUVER_NONE;
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
        (g_aObjectTypeData[
             g_acObjectType[obj]].cruiseVelocity & ~1) << 7);
}

/* Function start: 0x42A754 */
unsigned int approach_cruise_speed(short ship)
{
    approach_speed(ship,
        (int)g_aObjectTypeData[
            g_acObjectType[ship]].cruiseVelocity << 8);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42A788 */
unsigned int approach_full_speed(short ship)
{
    approach_speed(ship,
                      (int)g_asShipMaximumVelocity[ship] << 8);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42A7AF */
unsigned int approach_ship_speed(short obj, short other)
{
    approach_speed(obj, g_anShipSpeed[other]);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42A7D2 */
void ComputePointAheadOfObject(short obj, unsigned short distance,
                               FixedVector *point)
{
    ScaleFixedVector(&g_aShipForwardVector[obj],
                     (int)distance << 8, point);
    AddFixedVectors(&g_aShipPosition[obj], point, point);
}

/* Function start: 0x42A823 */
void ComputePointBehindObject(short obj, unsigned short distance,
                              FixedVector *point)
{
    ScaleFixedVector(&g_aShipForwardVector[obj],
                     -((int)distance << 8), point);
    AddFixedVectors(&g_aShipPosition[obj], point, point);
}

/* Function start: 0x42A876 */
short close_behind(short range)
{
    return g_nTargetRange < range && g_nTargetFacing < -0x32;
}

void trim_goals(short obj, short amount);

/* Function start: 0x42A8B5 */
short scan_for_enemy(short obj, unsigned short range)
{
    short other;
    short distance;

    g_nTargetRange = 0;
    g_nTargetShip = -1;
    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass[other] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver[other] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide[other] != g_asShipSide[obj] &&
            target_valid(obj, other) != 0) {
            distance =
                distance_from_point(other, &g_aShipPosition[obj]);
            if (distance < range &&
                (g_nTargetShip == -1 ||
                 distance < g_nTargetRange)) {
                g_nTargetShip = other;
                g_nTargetRange = distance;
            }
        }
    }
    if (g_nTargetShip != -1) {
        get_facing_range_from_object(obj, g_nTargetShip);
        g_nTargetRange =
            distance_from_object(obj, g_nTargetShip);
    }
    return g_nTargetShip;
}

short distance_from_object(short obj, short other);

/* Function start: 0x42AA0D */
short any_enemy(short obj, short range)
{
    short other;

    g_nTargetShip = -1;
    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass[other] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver[other] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide[obj] != g_asShipSide[other]) {
            g_nTargetRange = distance_from_object(obj, other);
            if (g_nTargetRange < range) {
                g_nTargetShip = other;
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
        if (g_aeObjectClass[other] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver[other] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide[obj] != g_asShipSide[other]) {
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

    ComputeVectorDelta(&g_aShipPosition[obj],
                       &g_aShipPosition[other], &delta);
    return IsVectorWithinRange(&delta, range);
}

/* Function start: 0x42ABCE */
short attacker_in_range(short obj, unsigned short range)
{
    short other;

    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass[other] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver[other] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide[other] != g_asShipSide[obj] &&
            g_acShipTarget[other] == obj) {
            g_nTargetRange = distance_from_object(other, obj);
            if (g_nTargetRange < range) {
                g_nTargetShip = other;
                return 1;
            }
        }
    }
    g_nTargetShip = -1;
    return 0;
}

short distance_from_object(short obj, short other);

/* Function start: 0x42ACAF */
short in_danger(short obj)
{
    unsigned short range;
    short attacker;

    g_nTargetShip = -1;
    for (attacker = 0; attacker < 10; attacker++) {
        if (g_aeObjectClass[attacker] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver[attacker] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide[attacker] != g_asShipSide[obj] &&
            g_acShipTarget[attacker] == obj) {
            range = (unsigned short)distance_from_object(attacker, obj);
            if (g_nTargetShip == -1 ||
                range < g_nTargetRange) {
                g_nTargetShip = attacker;
                g_nTargetRange = (short)range;
            }
        }
    }
    return g_nTargetShip != -1;
}

/* Function start: 0x42ADB8 */
short target_within_range(short obj)
{
    short target = g_acShipTarget[obj];

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
        if ((g_aeObjectClass[other] >= OBJECT_CLASS_SHIP &&
             g_aeSpecialManeuver[other] !=
                 SPECIAL_MANEUVER_UNKNOWN_9 &&
             g_asShipSide[other] != g_asShipSide[obj] &&
             g_aeObjectClass[other] == OBJECT_CLASS_SHIP) ||
            (g_aeObjectClass[other] == OBJECT_CLASS_MISSILE &&
             g_asObjectType[other] == 0x13 &&
             g_acShipTarget[other] == obj)) {
            distance = distance_from_object(obj, other);
            if (distance < range) {
                g_acShipList[count] =
                    (signed char)other;
                g_asShipListRange[count] = distance;
                count++;
            }
        }
    }
    g_acShipList[count] = -1;
    return count;
}

/* Function start: 0x42AF60 */
void inherit_leader_mission(short obj)
{
    short leader = g_asShipWingLeader[obj];

    if (leader != -1 &&
        g_aeObjectClass[obj] >= OBJECT_CLASS_SHIP) {
        g_asShipMissionType[obj] =
            g_asShipMissionType[leader];
        g_asShipMissionParameter[obj] =
            g_asShipMissionParameter[leader];
        g_aShipMissionSpot[obj] =
            g_aShipMissionSpot[leader];
        g_asShipWingLeader[obj] = -1;
    }
}

/* Function start: 0x42B00A */
void inherit_leader(short obj)
{
    short leader = g_asShipWingLeader[obj];
    short other;

    if (leader != -1 && g_aeObjectClass[obj] >= OBJECT_CLASS_SHIP) {
        inherit_leader_mission(obj);
        g_asShipWingLeader[obj] =
            g_asShipWingLeader[leader];
        for (other = 0; other < 10; other++) {
            if (g_asShipWingLeader[other] == leader)
                g_asShipWingLeader[other] = obj;
        }
    }
}

/* Function start: 0x42B0B7 */
short dead_ship(short i)
{
    return i == -1 || g_aMissionShips[i].state == 3;
}

/* Function start: 0x42B0FB */
short gone_ship(short missionShip)
{
    return missionShip == -1 ||
           g_aMissionShips[missionShip].state == 3 ||
           g_aMissionShips[missionShip].state == 2;
}

/* Function start: 0x42B15A */
/* WC2 passes a margin every caller ignores; the body never reads it. */
short skill_check(short obj, short margin)
{
    if (g_asPilotLevel[obj] < 5 &&
        RandomBelowOrEqual((short)(g_asPilotLevel[obj] + 4)) == 0)
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
    navPoint = g_aMissionNavPoints;
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
        while (g_aMissionNavPoints[navIndex].type > 1)
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
        *point = g_aShipPosition[obj];
        return 1;
    }
    if (dead_ship(missionShip) != 0)
        return 0;
    navPoint = find_ships_sphere(missionShip);
    if (navPoint != -1) {
        AddFixedVectors(&g_aMissionNavPoints[navPoint].position,
                        &g_aMissionShips[missionShip].position,
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

    if (g_asShipSide[obj] == SIDE_KILRATHI) {
        *point = g_aMissionNavPoints[
            g_asShipMissionParameter[obj]].position;
        return;
    }
    pathStep = (short)g_abShipNavPointIndex[obj];
    while (++pathStep < MISSION_OBJECTIVE_COUNT) {
        navObjective = (short)g_abFlightPath[pathStep];
#ifdef SDL_PORT
        /* Skipping a vanished mobile objective can reach the path sentinel. */
        if (navObjective < 0 || navObjective >= g_cMissionObjectiveCount)
            return;
#endif
        switch (g_aMissionObjectives[navObjective].type) {
        case 0:
            *point = g_aMissionObjectives[navObjective].position;
            g_abShipNavPointIndex[obj] = (signed char)pathStep;
            return;
        case 1:
            missionIndex =
                (short)g_aMissionObjectives[navObjective].index;
            shipObj = find_ship_index(missionIndex);
            if (shipObj == -1)
#ifdef SDL_PORT
            {
                /* A mobile objective names a mission ship, not a nav point.
                 * SO1 uses indices up to 15, including ships in other systems. */
                if (!locate_ship(missionIndex, point))
                    continue;
            }
#else
                *point = g_aMissionNavPoints[missionIndex].position;
#endif
            else
                *point = g_aShipPosition[shipObj];
            g_abShipNavPointIndex[obj] = (signed char)pathStep;
            return;
        }
    }
}

/* Function start: 0x42B524 */
unsigned int get_first_follow_point(short obj, FixedVector *point)
{
    if (g_asShipSide[obj] == SIDE_IMPERIAL)
        g_abShipNavPointIndex[obj] =
            g_cCurrentNavPointIndex - 1;
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
            g_aMissionNavPoints[navPoint].missionShips[slot];
        if (missionIndex != -1 &&
            g_aMissionShips[missionIndex].side !=
                g_asShipSide[obj])
            return 1;
    }
    return 0;
}

/* Function start: 0x42B5F7 */
short abandoned(short obj, short other)
{
    short navPoint;

    if ((g_abShipTurn[obj] & 7) != 0 ||
        g_asShipSide[obj] == SIDE_KILRATHI ||
        RandomBelowOrEqual(8) != 0)
        return 0;
    navPoint = FindNearestNavPoint(obj);
    if (g_nCurrentNavPoint != navPoint &&
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
    if (g_aeShipObjective[obj] != objective) {
        reset_objective(obj, objective);
        if (g_asPilotLevel[obj] == 5 &&
            g_asShipSide[obj] == SIDE_KILRATHI)
            SendKilrathiAceGreetingOnce(obj);
    }
    g_acShipTarget[obj] = target;
}

short ShipHasTorpedo(short ship);
short CanShipWeaponDamageTarget(short ship, short target);
short triumph(short obj);

/* Function start: 0x42B73B */
short target_valid(short obj, short target)
{
    if (target == -1)
        return 0;
    if (g_anShipCloakState[target] == 1 &&
        g_asShipCloakElapsedFrames[target] >= 40)
        return 0;
    if (ShipHasTorpedo(obj) == 0 &&
        CanShipWeaponDamageTarget(obj, target) == 0)
        return 0;
    return unactive(target) == 0 &&
           g_asShipSide[obj] != g_asShipSide[target];
}

/* Function start: 0x42B80C */
short triumph(short obj)
{
    short result;
    short objective;

    result = 0;
    switch (g_asShipMissionType[obj]) {
    case MISSION_TYPE_PATROL:
        objective = 0;
        while (g_abFlightPath[objective] != -1) {
            if (visited((short)g_abFlightPath[objective]) != 0)
                objective++;
            else
                break;
        }
        result = 1;
        break;
    case MISSION_TYPE_ESCORT:
    case MISSION_TYPE_DEFEND:
    case MISSION_TYPE_WINGMAN:
        result = dead_ship(g_asShipMissionParameter[obj]) == 0;
        break;
    case MISSION_TYPE_STRIKE:
        result = dead_ship(g_asShipMissionParameter[obj]);
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
    typeData = &g_aObjectTypeData[g_acObjectType[obj]];
    if (g_aeObjectClass[obj] < OBJECT_CLASS_SHIP)
        return 100;
    total = (short)(g_aasShipArmor[obj][0] * 23 /
                        typeData->armorFront + total);
    total = (short)(g_aasShipArmor[obj][1] * 27 /
                        typeData->armorRear + total);
    total = (short)(g_aasShipArmor[obj][2] * 12 /
                        typeData->armorLeft + total);
    total = (short)(g_aasShipArmor[obj][3] * 12 /
                        typeData->armorRight + total);
    total = (short)(26 - g_acShipDamage[obj] * 26 /
                             typeData->damageCapacity + total);
    return total;
}

short g_bKilrathiAceGreetingSent = 0; /* 0x00499bf4 */

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short g_5_nCollisionCountdownNumber; /* 0x005d2faa */
signed char g_acMissionShipStatusFlagsBytes[8]; /* 0x005d2fb0 */
short g_bWingmanKilledThisMission; /* 0x005d2fb8 */
signed char g_4_acInitialShipKillNumber[8]; /* 0x005d2fc0 */
short g_nLastFoundShip; /* 0x005d2fc8 */
