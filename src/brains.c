/*
 *  Ship intelligence and missions.
 *
 *  Retail object 0x442770-0x44655F (link order; see docs/ORDER.md).
 */
#define BRAINS_C
#include "game.h"

short g_nEnemySighting = 0x7fff; /* 0x0049b670 */
short g_bHomeObjectiveFlagged = 0; /* 0x0049b674 */
short g_asShipIntelState[10] = { 0 }; /* 0x0049b678 */

/* Function start: 0x442770 */
void cruise_home(short obj)
{
    short range;

    if (abandoned(obj, 0) != 0)
        return;
    if ((g_abShipTurn[obj] & 7) == 5) {
        if (g_aeObjectClass[obj] == OBJECT_CLASS_SHIP &&
            g_asShipSide[obj] == g_asShipSide[0] &&
            distance_from_object(obj, 0) > 16000) {
#ifdef SDL_PORT
            /* set_up_action_sphere records a routed ship it removes as gone
             * (state 3).  A team member travels with the player and leaves
             * only here, so retail keeps its record at 0 and the next set-up
             * of its first nav point brings it back.  In SO1 S4M0, with
             * Hobbes dead, that makes Landis the player's wingman, and his
             * INTEL.021 is in no release: the game exits. */
            if (g_asShipMissionType[obj] == MISSION_TYPE_ROUT &&
                g_asShipMissionIndex[obj] >= 0 &&
                g_asShipMissionIndex[obj] < MISSION_SHIP_COUNT)
                g_aMissionShips[g_asShipMissionIndex[obj]].state = 3;
#endif
            remove_object(obj);
            return;
        }
        if (g_aeObjectClass[obj] == OBJECT_CLASS_CAPITAL_SHIP ||
            g_asObjectType[obj] == 0x33)
            approach_cruise_speed(obj);
        else if (normal_speed(obj) != 0)
            fire_afterburner(obj, 10);
        if (CanSetNewShipTurnGoal(obj) != 0)
            point_ship_at_point(obj, &g_aShipDestination[obj]);
        range = distance_from_point(obj, &g_aShipDestination[obj]);
        if (equ_vector(&g_aShipDestination[obj],
                       &g_aShipMissionSpot[obj]) != 0) {
            if (range < 5000) {
                reset_tactic(obj, TACTIC_HEAD_HOME);
                set_special(obj, SPECIAL_MANEUVER_KILL_ENGINES);
                zero_vector(&g_aShipVelocity[obj]);
            }
            return;
        }
        if (range < 1500) {
            if (g_aMissionObjectives[
                    g_abFlightPath[
                        g_abShipNavPointIndex[obj]]].type != 1)
                flag_objective(
                    g_abFlightPath[
                        g_abShipNavPointIndex[obj]], 1);
            get_follow_point(obj, &g_aShipDestination[obj]);
        }
    }
}

/* Function start: 0x4429A7 */
void fail(short obj)
{
    reset_objective(obj, OBJECTIVE_NONE);
}

/* Function start: 0x4429C0 */
void coming_home(short obj)
{
    short home;
    short range;

    switch (g_asShipTactic[obj]) {
    case TACTIC_CRUISE:
        cruise_home(obj);
        break;
    case TACTIC_HEAD_HOME:
        home = find_ship_index(g_13_stMissionHeaderBlock.homeMissionShip);
        range = distance_from_object(obj, home);
        if (g_asObjectCollisionRadius[obj] + 800 < range)
            approach_cruise_speed(obj);
        else
            approach_ship_speed(obj, home);
        point_ship_behind_object(obj, home);
        if (g_aeObjectClass[obj] == OBJECT_CLASS_SHIP &&
            g_asObjectCollisionRadius[obj] + 800 > range)
#ifdef SDL_PORT
        {
            /* Landing at home ends a routed ship too; see cruise_home. */
            if (g_asShipMissionType[obj] == MISSION_TYPE_ROUT &&
                g_asShipMissionIndex[obj] >= 0 &&
                g_asShipMissionIndex[obj] < MISSION_SHIP_COUNT)
                g_aMissionShips[g_asShipMissionIndex[obj]].state = 3;
            remove_object(obj);
        }
#else
            remove_object(obj);
#endif
        break;
    case TACTIC_NONE:
        reset_tactic(obj, TACTIC_CRUISE);
        if (g_asShipSide[obj] == SIDE_IMPERIAL &&
            g_asShipMissionType[obj] == MISSION_TYPE_ROUT) {
            g_aShipDestination[obj] =
                g_aMissionObjectives[
                    find_objective(1, -1)].position;
        } else {
            get_first_follow_point(obj, &g_aShipDestination[obj]);
        }
        break;
    default:
        fail(obj);
    }
}

/* Function start: 0x442B78 */
void run_away(short obj)
{
    FixedVector direction;

    if (unactive(g_asShipWingLeader[obj]) == 0 &&
        g_asShipMissionType[
            g_asShipWingLeader[obj]] == MISSION_TYPE_ROUT) {
        maintain_formation(obj);
        return;
    }
    if (g_asShipSide[obj] == SIDE_IMPERIAL) {
        coming_home(obj);
        return;
    }

    zero_vector(&direction);
    direction.y = 0x100;
    if ((obj & 1) != 0)
        direction.y = -direction.y;
    point_ship(obj, 0, &direction);
    if (normal_speed(obj) != 0 &&
        (RandomBelow(100) < 50 || any_enemy(obj, 16000) != 0))
        fire_afterburner(obj, 40);
    else
        approach_full_speed(obj);
    if (distance_from_object(obj, 0) > 16000)
        remove_object(obj);
}

/* Function start: 0x442CC3 */
short check_engage_target(short obj)
{
    short newTarget = detect_enemy_tail(obj);

    if (newTarget != -1 && newTarget != g_acShipTarget[obj])
        g_acShipTarget[obj] = newTarget;
    else if (target_valid(obj, g_acShipTarget[obj]) == 0)
        select_target(obj);
    return g_acShipTarget[obj];
}

/* Function start: 0x442D58 */
short check_destroy_target(short obj)
{
    short destroyTarget = find_ship_index(g_asShipMissionParameter[obj]);

    if (destroyTarget == -1) {
        g_acShipTarget[obj] = check_engage_target(obj);
    } else if (g_aeObjectClass[destroyTarget] ==
                   OBJECT_CLASS_FUTURION ||
               gone_ship(g_asShipMissionParameter[obj]) != 0) {
        check_engage_target(obj);
    } else {
        if (70 - MaxShort(0, MinShort(4,
                (short)g_asPilotLevel[obj])) * 15 <
            evaluate_damage(obj)) {
            g_acShipTarget[obj] = destroyTarget;
            if (g_asShipSide[destroyTarget] ==
                g_asShipSide[obj])
                g_acShipTarget[obj] = -1;
        } else if (target_valid(obj, g_acShipTarget[obj]) != 0 &&
                   RandomBelow(100) > 3) {
            check_engage_target(obj);
        } else {
            g_acShipTarget[obj] = destroyTarget;
        }
    }
    if (g_acShipTarget[obj] == -1)
        check_engage_target(obj);
    return g_acShipTarget[obj];
}

/* Function start: 0x442EFC */
void maneuvering(short obj, short newTarget)
{
    g_acShipTarget[obj] = newTarget;
    intelligence_events(obj);
    perform_maneuver(obj);
}

/* Function start: 0x442F2D */
void formation_burst(short obj)
{
    approach_full_speed(obj);
    if (CanSetNewShipTurnGoal(obj) != 0)
        point_ship(obj, 0, &g_aShipDestination[obj]);
    g_asShipCount[obj]++;
    if (g_asShipCount[obj] > 9) {
        if (g_asShipMissionType[obj] == MISSION_TYPE_STRIKE)
            engage(obj, g_acShipTarget[obj],
                   OBJECTIVE_DESTROY_SHIP);
        else
            engage(obj, g_acShipTarget[obj],
                   OBJECTIVE_ENGAGE_ENEMY);
    }
}

/* Function start: 0x442FE9 */
void disallow_engage(void)
{
    g_bEngageAllowed = 0;
}

/* Function start: 0x442FFD */
void allow_engage(void)
{
    g_bEngageAllowed = 1;
    g_nAutoEngageTimer = -1;
}

void engage(short obj, short target, short objective);
void send_message(short obj, signed char message);
short attacker_in_range(short obj, unsigned short range);
void fire_afterburner(short obj, short time);
short real_velocity(short obj);

/* Function start: 0x44301A */
void try2allow_engage(short obj)
{
    if (obj != g_nYourWingman) {
        allow_engage();
        return;
    }
    if (g_stWingmanIntelligence.engagementMode == 1) {
        allow_engage();
        return;
    }
    if (g_stWingmanIntelligence.engagementMode == 2 &&
        RandomBelowOrEqual(100) < 50) {
        allow_engage();
        return;
    }
    g_nAutoEngageTimer = -40;
}

void point_ship_at_object(short obj, short other);
unsigned int approach_ship_speed(short obj, short other);
unsigned int change_mission_type(short obj, short missionType);
short facing_to_object(short obj, FixedVector *point);
short distance_from_point(short obj, const FixedVector *point);
void wingman_mission(short obj);

/* Function start: 0x443095 */
void imperial_formation(short obj)
{
    short leader = g_asShipWingLeader[obj];

    if (leader == -1)
        leader = obj;
    maintain_formation(obj);
    if (attacker_in_range(leader, 12000) != 0) {
        if (obj == g_nYourWingman ||
            g_nYourWingman == -1) {
            if (g_nAutoEngageTimer < -1)
                g_nAutoEngageTimer++;
#ifdef SDL_PORT
            /* SO1.exe 0x442E65 and SO2.exe 0x404E85: with no wingman of
             * the player's, ships in formation ask to engage at once; the
             * timer below only ever starts for that wingman. */
            else if (g_nSdlCampaign != 0 && obj != g_nYourWingman)
                try2allow_engage(obj);
#endif
            else if (g_nAutoEngageTimer != -1 &&
                     --g_nAutoEngageTimer == 0)
                try2allow_engage(obj);
        }
        if (g_bEngageAllowed != 0) {
            engage(obj, g_nTargetShip,
                   OBJECTIVE_ENGAGE_ENEMY);
        } else if (obj == g_nYourWingman &&
                   g_nAutoEngageTimer == -1) {
            send_message(obj, 3);
            g_nAutoEngageTimer = 40;
        }
    } else if (obj == g_nYourWingman &&
               g_nEnemySighting != g_nCurrentWave &&
               any_enemy(obj, 16000) != 0 &&
               message_showing() == 0 && g_nCurrentView == 0) {
        send_message(obj, 2);
        g_nEnemySighting = g_nCurrentWave;
    }

    if (g_aeSpecialManeuver[obj] == SPECIAL_MANEUVER_NONE &&
        distance_from_object(obj, leader) > 9000) {
        if (facing_to_object(obj, &g_aShipPosition[leader]) > 85 &&
            real_velocity(obj) < 110) {
            fire_afterburner(obj, 10);
            return;
        }
        point_ship_at_object(obj, leader);
        approach_ship_speed(obj, leader);
    }
}

/* Function start: 0x4432EB */
void formation_break(short obj)
{
    switch (g_acShipSequence[obj]) {
    case 0:
        steady_object(obj);
        g_anYawGoal[obj] = -30;
        g_anRollGoal[obj] = -45;
        g_anPitchGoal[obj] = -20;
        g_acShipSequence[obj]++;
        break;
    case 1:
        if (CanSetNewShipTurnGoal(obj) != 0)
            engage(obj, g_acShipTarget[obj],
                   OBJECTIVE_ENGAGE_ENEMY);
        break;
    default:
        g_acShipSequence[obj] = 0;
    }
}

/* Function start: 0x4433B1 */
void imperial_wingman(short obj)
{
    switch (g_aeShipObjective[obj]) {
    case OBJECTIVE_HOLD_FORMATION:
        imperial_formation(obj);
        break;
    case OBJECTIVE_BREAK_FORMATION:
        formation_break(obj);
        break;
    case OBJECTIVE_ENGAGE_ENEMY:
        maneuvering(obj, check_engage_target(obj));
        break;
    case OBJECTIVE_DESTROY_SHIP:
        maneuvering(obj, check_destroy_target(obj));
        break;
    case OBJECTIVE_NONE:
        reset_objective(obj, OBJECTIVE_HOLD_FORMATION);
        break;
    default:
        fail(obj);
    }
}

/* Function start: 0x443499 */
void kilrathi_wingman(short obj)
{
    short leader = g_asShipWingLeader[obj];

    if (leader == -1) {
        change_mission_type(obj, MISSION_TYPE_PATROL);
        return;
    } else if (unactive(leader) != 0) {
        inherit_leader(obj);
        return;
    }
    if ((g_aeShipObjective[leader] == OBJECTIVE_ENGAGE_ENEMY ||
         g_aeShipObjective[leader] == OBJECTIVE_DESTROY_SHIP) &&
        g_aeShipObjective[obj] != g_aeShipObjective[leader])
        engage(obj, g_acShipTarget[obj],
               g_aeShipObjective[leader]);

    switch (g_aeShipObjective[obj]) {
    case OBJECTIVE_HOLD_FORMATION:
        maintain_formation(obj);
        break;
    case OBJECTIVE_BREAK_FORMATION:
        formation_burst(obj);
        break;
    case OBJECTIVE_DESTROY_SHIP:
    case OBJECTIVE_ENGAGE_ENEMY:
        maneuvering(obj, check_engage_target(obj));
        break;
    case OBJECTIVE_NONE:
        reset_objective(obj, OBJECTIVE_HOLD_FORMATION);
        break;
    default:
        fail(obj);
    }
}

/* Function start: 0x443631 */
void wingman_mission(short obj)
{
    if (g_asShipSide[obj] == SIDE_IMPERIAL) {
        imperial_wingman(obj);
        return;
    }
    kilrathi_wingman(obj);
}

/* Function start: 0x44366D */
short dist_from_home(short obj)
{
    return distance_from_point(obj, &g_aShipMissionSpot[obj]);
}

/* Function start: 0x443698 */
short scan_and_lock(short obj, short newTactic)
{
    g_acShipTarget[obj] = scan_for_enemy(obj, 14000);
    if (g_acShipTarget[obj] != -1)
        g_asShipTactic[obj] = newTactic;
    return g_acShipTarget[obj] != -1;
}

/* Function start: 0x443707 */
void patrol_area(short obj)
{
    short target = g_acShipTarget[obj];

    switch (g_asShipTactic[obj]) {
    case TACTIC_LOOK_OUT:
        approach_cruise_speed(obj);
        if (scan_and_lock(obj, TACTIC_APPROACH_TARGET) == 0 &&
            dist_from_home(obj) > 8000)
            reset_tactic(obj, TACTIC_HEAD_HOME);
        break;
    case TACTIC_HEAD_HOME:
        approach_cruise_speed(obj);
        if (scan_and_lock(obj, TACTIC_APPROACH_TARGET) == 0) {
            get_facing_range_from_point(obj, &g_aShipMissionSpot[obj]);
            if (g_nTargetRange < 3000) {
                reset_tactic(obj, TACTIC_LOOK_OUT);
            } else {
                point_ship_at_point(obj, &g_aShipMissionSpot[obj]);
                trim_goals(obj, 7);
            }
        }
        break;
    case TACTIC_APPROACH_TARGET:
        approach_full_speed(obj);
        if (unactive(target) != 0) {
            if (scan_and_lock(obj, TACTIC_APPROACH_TARGET) == 0)
                alter_tactic(obj, TACTIC_LOOK_OUT);
        } else {
            get_facing_range_from_object(obj, target);
            if (g_nTargetRange < 10000)
                init_formation_burst(obj);
            else if (CanSetNewShipTurnGoal(obj) != 0)
                point_ship_at_object(obj, target);
        }
        break;
    case TACTIC_NONE:
        reset_tactic(obj, TACTIC_APPROACH_TARGET);
        break;
    default:
        fail(obj);
    }
}

/* Function start: 0x443930 */
void kilrathi_patrol(short obj)
{
    switch (g_aeShipObjective[obj]) {
    case OBJECTIVE_BREAK_FORMATION:
        formation_burst(obj);
        break;
    case OBJECTIVE_WANDER:
    case OBJECTIVE_HOLD_FORMATION:
        patrol_area(obj);
        break;
    case OBJECTIVE_ENGAGE_ENEMY:
        maneuvering(obj, check_engage_target(obj));
        break;
    case OBJECTIVE_NONE:
        g_aeShipObjective[obj] = OBJECTIVE_WANDER;
        g_asShipTactic[obj] = TACTIC_APPROACH_TARGET;
        break;
    default:
        fail(obj);
    }
}

/* Function start: 0x443A08 */
void cruise_to_destination(short obj)
{
    short range;

    if (abandoned(obj, 0) != 0)
        return;
    if ((g_abShipTurn[obj] & 7) == 6)
        g_acShipTarget[obj] = scan_for_enemy(obj, 15000);

    if (g_acShipTarget[obj] == -1) {
        approach_cruise_speed(obj);
    } else {
        fire_turrets(obj);
        get_facing_range_from_object(obj, g_acShipTarget[obj]);
        if (g_nFacingToTarget > 65)
            approach_half_speed(obj);
        else
            approach_full_speed(obj);
    }

    if ((g_abShipTurn[obj] & 7) == 2) {
        if (g_asShipMissionParameter[obj] ==
            g_nCurrentNavPoint) {
            g_acShipNavPoint[obj] =
                (signed char)g_nCurrentNavPoint;
            g_aShipDestination[obj] =
                g_aShipMissionSpot[obj];
        }
        if (CanSetNewShipTurnGoal(obj) != 0)
            point_ship_at_point(obj, &g_aShipDestination[obj]);
        range = distance_from_point(obj, &g_aShipDestination[obj]);
        if (range < 1500) {
            if (g_asShipSide[obj] == SIDE_IMPERIAL)
                flag_reached((short)g_abFlightPath[
                    g_abShipNavPointIndex[obj]], 1);
            if (equ_vector(&g_aShipDestination[obj],
                    &g_aShipMissionSpot[obj]) != 0) {
                reset_tactic(obj, TACTIC_SIT_STILL);
                set_special(obj, SPECIAL_MANEUVER_KILL_ENGINES);
            } else {
                get_follow_point(obj, &g_aShipDestination[obj]);
            }
        }
    }
}

/* Function start: 0x443C45 */
void prepare_for_jump(short obj)
{
    if (g_anShipSpeed[obj] != 0) {
        set_special(obj, SPECIAL_MANEUVER_STOP_DRIFT);
        return;
    }
    g_asShipCount[obj]++;
    if ((g_asShipSide[obj] == SIDE_KILRATHI ? 250 : 25) <
        g_asShipCount[obj]) {
        get_facing_range_from_object(obj, 0);
        if (g_nFacingToTarget > 90 &&
            CanSetNewShipTurnGoal(obj) != 0) {
            g_anYawGoal[obj] = signed_random(30);
        } else if ((g_asShipSide[obj] == SIDE_KILRATHI ? 250
                                                                : 25) +
                           20 <
                       g_asShipCount[obj] ||
                   (g_nTargetFacing > 80 &&
                    g_nTargetRange < 6000)) {
            reset_tactic(obj, TACTIC_WARP_OUT);
            fire_afterburner(obj, 10);
        }
    }
}

unsigned int reset_tactic(short ship, short tactic);

/* Function start: 0x443D6E */
void accelerate_and_jump(short obj)
{
    approach_full_speed(obj);
    if (g_asShipCount[obj]++ == 4)
        warp(obj);
}

/* Function start: 0x443DBD */
void reach_warp(short obj)
{
    switch (g_asShipTactic[obj]) {
    case TACTIC_CRUISE:
        cruise_to_destination(obj);
        break;
    case TACTIC_SIT_STILL:
        prepare_for_jump(obj);
        break;
    case TACTIC_WARP_OUT:
        accelerate_and_jump(obj);
        break;
    case TACTIC_NONE:
        reset_tactic(obj, TACTIC_CRUISE);
        get_first_follow_point(obj, &g_aShipDestination[obj]);
        break;
    default:
        fail(obj);
    }
}

/* Function start: 0x443E8C */
void split_patrol_mission(short obj)
{
    if (g_asShipMissionParameter[obj] ==
        g_nCurrentNavPoint) {
        g_asShipWingLeader[obj] = -1;
        g_acShipNavPoint[obj] =
            (signed char)g_nCurrentNavPoint;
    }
    g_aeShipObjective[obj] = OBJECTIVE_HOLD_FORMATION;
    imperial_wingman(obj);
}

/* Function start: 0x443EE9 */
void warp_arrival(short obj)
{
    if (g_asShipTactic[obj] == TACTIC_WARP_IN)
        arrive_from_warp(obj);
    else
        reset_tactic(obj, TACTIC_WARP_IN);
}

/* Function start: 0x443F28 */
void return_to_buddy(short obj, short buddy)
{
    approach_cruise_speed(obj);
    if (CanSetNewShipTurnGoal(obj) != 0)
        point_ship_at_object(obj, buddy);
    if (distance_from_object(obj, buddy) < 1000) {
        reset_objective(obj, OBJECTIVE_WANDER);
        point_parallel(obj, buddy);
    }
}

/* Function start: 0x443FA2 */
void escort_buddy(short obj, short buddy)
{
    approach_ship_speed(obj, buddy);
    if (CanSetNewShipTurnGoal(obj) != 0)
        point_parallel(obj, buddy);
}

/* Function start: 0x443FE4 */
void escort_mission(short obj)
{
    short buddy = find_ship_index(g_asShipMissionParameter[obj]);

    if (unactive(buddy) != 0) {
        change_mission_type(obj, MISSION_TYPE_PATROL);
        return;
    }
    if ((g_abShipTurn[obj] & 3) == 0 && in_danger(buddy) != 0 &&
        g_nTargetRange < 3000)
        engage(obj, g_nTargetShip,
               OBJECTIVE_ENGAGE_ENEMY);
    if (g_aeShipObjective[obj] != OBJECTIVE_HOME_BASE &&
        (g_abShipTurn[obj] & 7) == 4 &&
        distance_from_object(obj, buddy) > 5000)
        reset_objective(obj, OBJECTIVE_HOME_BASE);

    switch (g_aeShipObjective[obj]) {
    case OBJECTIVE_WANDER:
        escort_buddy(obj, buddy);
        break;
    case OBJECTIVE_HOME_BASE:
        return_to_buddy(obj, buddy);
        break;
    case OBJECTIVE_ENGAGE_ENEMY:
        maneuvering(obj, check_engage_target(obj));
        break;
    case OBJECTIVE_NONE:
        reset_objective(obj, OBJECTIVE_WANDER);
        break;
    default:
        fail(obj);
    }
}

/* Function start: 0x444199 */
void check_goal(short obj)
{
    if (gone_ship(g_asShipMissionParameter[obj]) != 0)
        reset_mission_type(obj, MISSION_TYPE_ROUT);
    else
        reset_mission_type(obj, MISSION_TYPE_PATROL);
}

/* Function start: 0x4441E5 */
void streak_toward(short obj, short goal, short range)
{
    if (CanSetNewShipTurnGoal(obj) != 0) {
        if (RandomBelow(100) < 95)
            point_ship_at_object(obj, goal);
        else
            veer_random(obj, 20);
    }
    if (range > 2000 && normal_speed(obj) != 0) {
        approach_full_speed(obj);
        fire_afterburner(obj, 10);
    } else {
        approach_full_speed(obj);
    }
}

/* Function start: 0x444291 */
void approach_and_engage(short obj, short goal)
{
    unsigned short dist;
    unsigned short possibleRange;
    short possibleTarget;
    short nearRange;

    if (g_asObjectType[obj] == 0x33)
        nearRange = 8000;
    else
        nearRange = 5000;
    dist = (unsigned short)distance_from_object(obj, goal);
    if (g_aeObjectClass[goal] != OBJECT_CLASS_FUTURION &&
        70 - MaxShort(0, MinShort(4,
                (short)g_asPilotLevel[obj])) * 15 <
            evaluate_damage(obj) &&
        nearRange < dist) {
        streak_toward(obj, goal, (short)dist);
        return;
    }
    possibleTarget = scan_for_enemy(obj, 10000);
    possibleRange = (unsigned short)g_nTargetRange;
    if (possibleTarget != -1 &&
        (possibleRange * 3 < dist ||
         g_aeObjectClass[goal] == OBJECT_CLASS_FUTURION)) {
        init_formation_burst(obj);
        g_acShipTarget[obj] = (signed char)possibleTarget;
    } else if (nearRange > dist) {
        engage(obj, goal, OBJECTIVE_DESTROY_SHIP);
    } else {
        streak_toward(obj, goal, (short)dist);
    }
}

/* Function start: 0x444420 */
void strike_mission(short obj)
{
    short goal;

    goal = find_ship_index(g_asShipMissionParameter[obj]);
    if (goal == -1 ||
        (goal != -1 &&
         g_aeObjectClass[goal] == OBJECT_CLASS_FUTURION)) {
        if (is_team_member(g_asShipMissionIndex[obj]) == 0) {
            check_goal(obj);
        } else if (g_aMissionShips[
                       g_asShipMissionParameter[obj]].state == 3) {
            change_mission_type(obj, 9);
            g_asShipTactic[obj] = TACTIC_CRUISE;
            if (g_bHomeObjectiveFlagged == 0) {
                flag_objective(g_cCurrentObjective, 1);
                set_next_destination();
                g_bHomeObjectiveFlagged++;
            }
            locate_ship(g_13_stMissionHeaderBlock.homeMissionShip,
                        &g_aShipMissionSpot[obj]);
            g_aShipDestination[obj] =
                g_aShipMissionSpot[obj];
            return;
        } else {
            g_bHomeObjectiveFlagged = 0;
            switch (g_asShipTactic[obj]) {
            case TACTIC_NONE:
                reset_tactic(obj, TACTIC_CRUISE);
                get_first_follow_point(obj,
                                       &g_aShipDestination[obj]);
                break;
            case TACTIC_CRUISE:
                cruise_to_destination(obj);
                break;
            }
            return;
        }
    }
    switch (g_aeShipObjective[obj]) {
    case OBJECTIVE_BREAK_FORMATION:
        formation_burst(obj);
        break;
    case OBJECTIVE_HOME_BASE:
    case OBJECTIVE_HOLD_FORMATION:
        approach_and_engage(obj, goal);
        break;
    case OBJECTIVE_ENGAGE_ENEMY:
        maneuvering(obj, check_destroy_target(obj));
        break;
    case OBJECTIVE_DESTROY_SHIP:
        maneuvering(obj, check_destroy_target(obj));
        break;
    case OBJECTIVE_NONE:
        reset_objective(obj, OBJECTIVE_HOME_BASE);
        break;
    default:
        fail(obj);
    }
}

/* Function start: 0x4446C9 */
void return_to_master(short obj, short master)
{
    short range = distance_from_object(obj, master);

    streak_toward(obj, master, range);
    if (range < 5000) {
        reset_objective(obj, OBJECTIVE_WANDER);
        point_perpendicular(obj, master);
    }
}

/* Function start: 0x44472C */
void defend_mission(short obj)
{
    short master = find_ship_index(g_asShipMissionParameter[obj]);

    if (master == -1) {
        change_mission_type(obj, MISSION_TYPE_PATROL);
        return;
    }
    if (g_abShipTurn[obj] % 10 == 0 &&
        in_danger(master) != 0 && g_nTargetRange < 6000 &&
        g_aeShipObjective[obj] != OBJECTIVE_ENGAGE_ENEMY)
        engage(obj, g_nTargetShip,
               OBJECTIVE_ENGAGE_ENEMY);
    if (g_aeShipObjective[obj] != OBJECTIVE_HOME_BASE &&
        (g_abShipTurn[obj] & 7) == 4 &&
        distance_from_object(obj, master) > 10000)
        reset_objective(obj, OBJECTIVE_HOME_BASE);

    switch (g_aeShipObjective[obj]) {
    case OBJECTIVE_WANDER:
        g_acShipTarget[obj] =
            (signed char)scan_for_enemy(obj, 7000);
        if (g_acShipTarget[obj] != -1) {
            engage(obj, (short)g_acShipTarget[obj],
                   OBJECTIVE_ENGAGE_ENEMY);
        } else {
            approach_half_speed(obj);
            if (CanSetNewShipTurnGoal(obj) != 0)
                point_perpendicular(obj, master);
        }
        break;
    case OBJECTIVE_HOME_BASE:
        return_to_master(obj, master);
        break;
    case OBJECTIVE_ENGAGE_ENEMY:
        maneuvering(obj, check_engage_target(obj));
        break;
    case OBJECTIVE_NONE:
        reset_objective(obj, OBJECTIVE_WANDER);
        break;
    default:
        fail(obj);
    }
}

/* Function start: 0x444966 */
void rendezvous_mission(short obj)
{
    short goal = find_ship_index(g_asShipMissionParameter[obj]);

    if (unactive(goal) != 0) {
        change_mission_type(obj, MISSION_TYPE_PATROL);
        return;
    }
    switch (g_aeShipObjective[obj]) {
    case OBJECTIVE_REACH_SHIP:
        if (attacker_in_range(obj, 3500) != 0)
            engage(obj, g_nTargetShip,
                   OBJECTIVE_ENGAGE_ENEMY);
        if (distance_from_object(obj, goal) < 2500) {
            reset_mission_type(obj, MISSION_TYPE_DEFEND);
            return;
        }
        if (attacker_in_range(goal, 9000) != 0)
            approach_full_speed(obj);
        else
            approach_cruise_speed(obj);
        if (CanSetNewShipTurnGoal(obj) != 0)
            point_ship_at_object(obj, goal);
        break;
    case OBJECTIVE_ENGAGE_ENEMY:
        maneuvering(obj, check_engage_target(obj));
        break;
    default:
        reset_objective(obj, OBJECTIVE_REACH_SHIP);
    }
}

/* Function start: 0x444ADC */
void ship_intelligence(short obj)
{
    if (g_acShipCollisionCooldown[obj] > 0)
        g_acShipCollisionCooldown[obj]--;
    if (regulate_turn(obj) != 0)
        return;
    switch (g_asShipMissionType[obj]) {
    case MISSION_TYPE_PATROL:
        kilrathi_patrol(obj);
        break;
    case MISSION_TYPE_ESCORT:
        escort_mission(obj);
        break;
    case MISSION_TYPE_STRIKE:
        strike_mission(obj);
        break;
    case MISSION_TYPE_DEFEND:
        defend_mission(obj);
        break;
    case MISSION_TYPE_WINGMAN:
        wingman_mission(obj);
        break;
    case MISSION_TYPE_RENDEZVOUS:
        rendezvous_mission(obj);
        break;
    case MISSION_TYPE_GOTO_WARP:
        reach_warp(obj);
        break;
    case MISSION_TYPE_WARP_ARRIVE:
        warp_arrival(obj);
        break;
    case MISSION_TYPE_COME_HOME:
        coming_home(obj);
        break;
    case MISSION_TYPE_ROUT:
        run_away(obj);
        break;
    case MISSION_TYPE_SPLIT_PATROL:
        split_patrol_mission(obj);
        break;
    case MISSION_TYPE_NONE:
        if (obj == 0) {
            approach_cruise_speed(obj);
            break;
        }
        inherit_leader_mission(obj);
        break;
    default:
        fail(obj);
    }
}

/* Function start: 0x444C80 */
void orbit_sphere(short obj)
{
    short radius = g_aMissionNavPoints[
        g_nCurrentNavPoint].proximityRadius >> 1;
    FixedVector center =
        g_aMissionNavPoints[g_nCurrentNavPoint].position;
    short range = distance_from_point(obj, &center);

    if (CanSetNewShipTurnGoal(obj) != 0 && range > radius - 750) {
        if (range > radius)
            point_ship_at_point(obj, &center);
        else
            point_perpendicular_to_point(obj, &center);
    }
    trim_goals(obj, 10);
}

/* Function start: 0x444D5C */
void tanker_intelligence(short obj)
{
    if (attacker_in_range(obj, 3000) != 0) {
        approach_full_speed(obj);
        get_facing_range_from_object(obj, g_nTargetShip);
        g_acShipTarget[obj] = g_nTargetShip;
        fire(obj, g_nTargetShip);
        if (CanSetNewShipTurnGoal(obj) != 0) {
            if (RandomBelowOrEqual(4) == 0) {
                g_anYawGoal[obj] = signed_random(90);
                g_anRollGoal[obj] = signed_random(90);
            } else {
                point_capital_ship_at_object(obj,
                                             g_nTargetShip);
            }
        }
        return;
    }
    approach_cruise_speed(obj);
    orbit_sphere(obj);
}

/* Function start: 0x444E51 */
void destroyer_intelligence(short obj)
{
    if (fire_turrets(obj) != 0) {
        g_acShipTarget[obj] = -1;
        approach_half_speed(obj);
    } else {
        approach_cruise_speed(obj);
    }
    orbit_sphere(obj);
}

void point_capital_ship_at_object(short obj, short other);

/* Function start: 0x444EA7 */
void stationary_intelligence(short obj)
{
    short other;

    if (g_aeObjectClass[obj] == OBJECT_CLASS_BASE)
        g_anObjectYawRotation[obj] = 2;
    fire_turrets(obj);
    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass[other] >= OBJECT_CLASS_CAPITAL_SHIP &&
            g_asShipSide[other] != g_asShipSide[obj]) {
            g_asShipMissionType[obj] = MISSION_TYPE_STRIKE;
            g_acShipTarget[obj] = -1;
        }
    }
}

/* Function start: 0x444F53 */
void SpawnCapitalFlakProjectile(short weapon, short obj,
                                int x, int y, int z)
{
    FixedVector direction;
    short projectile;
    short speed;

    projectile = new_object(0x0d, obj);
    if (projectile != -1) {
        g_acObjectOwner[projectile] = (signed char)obj;
        ComputeVectorDelta(&g_aShipPosition[obj],
                           &g_aShipPosition[
                               g_acShipTarget[obj]],
                           &direction);
        NormalizeFixedVector(&direction);
        ScaleFixedVector(
            &direction,
            (int)g_aObjectTypeData[
                g_acObjectType[obj]].collisionRadius << 8,
            &direction);
        zero_vector(&g_aShipPosition[projectile]);
        AddFixedVectors(&g_aShipPosition[obj], &direction,
                        &g_aShipPosition[projectile]);
        speed = g_aObjectTypeData[0x0d].maximumVelocity;
        g_asObjectDamage[projectile] =
            g_aObjectTypeData[0x0d].damageCapacity;
        g_asObjectCounter[projectile] =
            g_aObjectTypeData[0x0d].lifetime;
#ifdef SDL_PORT
        /* Retail addresses three consecutive coordinate words on the stack.
         * Native scalar arguments need not be adjacent, so supply a vector. */
        direction.x = x;
        direction.y = y;
        direction.z = z;
        send_at_point(projectile, &direction, speed);
#else
        send_at_point(projectile, (FixedVector *)&x, speed);
#endif
        RecordCannedSceneObjectEvent(projectile, 0);
    }
}

int send_at_point(short obj, FixedVector *point, short speed);
void point_capital_ship_at_object(short obj, short other);
void get_facing_range_from_object(short obj, short other);
short unactive(short ship);

/* Function start: 0x445087 */
void capital_ship_strike_mission(short obj)
{
    short lockedTarget;
    int aimOffset;
    short cue;
    short victim;
    short mayFire;
    short enemyShips;
    FixedVector adjustedAim;
    ShipWeaponSlot *mountSlot;
    short allies;
    short frames;
    FixedVector spot;
    short gun;
    FixedVector leadVector;
    FixedVector direction;
    short candidate;
    short dodging;
    short seen;
    short shotSpeed;
    short flakShot;
    short flakIndex;
    short dist;

    flakIndex = 0;
    flakShot = 0;
    victim = g_acShipTarget[obj];
    if (victim != -1 &&
        g_aeObjectClass[victim] < OBJECT_CLASS_CAPITAL_SHIP)
        destroyer_intelligence(obj);
    if (victim == -1) {
        g_acShipTarget[obj] =
            (signed char)find_ship_index(
                g_asShipMissionParameter[obj]);
        g_acShipSequence[obj] = 0;
    }
    victim = g_acShipTarget[obj];
    if (victim == -1 || unactive(victim) != 0) {
        seen = 0;
        for (candidate = 0; candidate < 10; candidate++) {
            if (g_aeObjectClass[candidate] >=
                    OBJECT_CLASS_CAPITAL_SHIP &&
                g_asShipSide[obj] !=
                    g_asShipSide[candidate]) {
                g_acShipTarget[obj] = (signed char)candidate;
                victim = candidate;
                seen = 1;
            }
        }
        if (seen == 0) {
            g_acShipTarget[obj] = -1;
            approach_zero_speed(obj);
            fire_turrets(obj);
            return;
        }
    }
    if (g_aeObjectClass[victim] < OBJECT_CLASS_CAPITAL_SHIP)
        destroyer_intelligence(obj);
    fire_turrets(obj);
    g_acShipTarget[obj] = (signed char)victim;
    get_facing_range_from_object(obj, victim);

    /* The flak countdown shares its else with the idle case: retail
     * decrements inside the condition. */
    if (g_asSpikeriLaunchCountdown[obj] != 0 && --g_asSpikeriLaunchCountdown[obj] == 0) {
        cue = new_object(0x0e, obj);
        if (cue != -1) {
            g_asSpikeriCooldown[obj] =
                (short)((unsigned short)RandomInRange(0, 200) +
                        400);
            g_acObjectOwner[cue] = (signed char)obj;
            ComputeVectorDelta(
                &g_aShipPosition[obj],
                &g_aShipPosition[
                    g_acShipTarget[obj]],
                &direction);
            NormalizeFixedVector(&direction);
            ScaleFixedVector(
                &direction,
                (int)g_aObjectTypeData[
                    g_acObjectType[obj]].collisionRadius << 8,
                &direction);
            zero_vector(&g_aShipPosition[cue]);
            AddFixedVectors(&g_aShipPosition[obj],
                            &direction,
                            &g_aShipPosition[cue]);
            shotSpeed =
                g_aObjectTypeData[0x0e].maximumVelocity;
            g_asObjectDamage[cue] =
                g_aObjectTypeData[0x0e].damageCapacity;
            g_asObjectCounter[cue] =
                g_aObjectTypeData[0x0e].lifetime;
            send_at_point(
                cue,
                &g_aShipPosition[
                    g_acShipTarget[obj]],
                shotSpeed);
            RecordCannedSceneObjectEvent(cue, 0);
            PlaySfxWaveFileByNumber(0x29, obj, 0);
        }
    } else {
        dist = distance_from_object(obj, victim);
        lockedTarget = victim;
        if (dist < 5000) {
            g_acShipSequence[obj] = 1;
            point_capital_ship_at_object(obj, lockedTarget);
            approach_speed(
                obj,
                (unsigned short)RandomInRange(
                    50, (unsigned short)(
                        g_asShipMaximumVelocity[obj] << 8)));
        } else if (dist > 10000 ||
                   g_acShipSequence[obj] != 1) {
            g_acShipSequence[obj] = 2;
            point_ship_at_object(obj, lockedTarget);
            approach_speed(
                obj,
                (unsigned short)RandomInRange(
                    50, (unsigned short)(
                        g_asShipMaximumVelocity[obj] << 8)));
        }
        if (dist < 10000) {
            mountSlot = (ShipWeaponSlot *)&g_aShipWeapons[obj][1];
            for (gun = 0;
                 gun <
                     (signed char)g_aShipWeapons[obj][0];
                 gun++, mountSlot++) {
                if (mountSlot->type == 0x0e &&
                    g_asSpikeriCooldown[obj] == 0 &&
                    g_asSpikeriLaunchCountdown[obj] == 0 &&
                    g_nFacingToTarget > 20) {
                    PlaySfxWaveFileByNumber(0x28, obj, 0);
                    g_asSpikeriLaunchCountdown[obj] = 20;
                }
                if (mountSlot->type == 0x0d &&
                    g_2_asFlakCooldownTable[obj] == 0) {
                    enemyShips = 0;
                    allies = 0;
                    dodging = 0;
                    for (candidate = 0; candidate < 10; candidate++) {
                        if (g_aeObjectClass[candidate] ==
                            OBJECT_CLASS_SHIP) {
                            if (g_asShipSide[obj] !=
                                g_asShipSide[candidate])
                                enemyShips =
                                    (short)(enemyShips + 1);
                            else
                                allies =
                                    (short)(allies + 1);
                        }
                    }
                    if (enemyShips > allies &&
                        RandomBelowOrEqual(2) == 0)
                        dodging = 1;
                    if (g_asObjectDamage[obj] > 0 &&
                        RandomBelowOrEqual(1) == 0)
                        dodging = 1;
                    aimOffset =
                        ((int)g_asObjectCollisionRadius[
                            lockedTarget] << 8) * 3;
                    flakShot = 1;
                    switch (flakIndex % 8) {
                    case 0:
                        ComputeVectorDelta(
                            &g_aShipPosition[obj],
                            &g_aShipPosition[lockedTarget],
                            &g_vToTarget);
                        mayFire = (short)(
                            FixedToShortSaturating(
                                Vector_magnitude(
                                    &g_vToTarget)) -
                            g_asObjectCollisionRadius[obj] -
                            g_asObjectCollisionRadius[
                                lockedTarget]);
                        frames = (short)(mayFire / 250);
                        ScaleFixedVector(
                            &g_aShipVelocity[lockedTarget],
                            (int)frames << 8, &leadVector);
                        AddFixedVectors(
                            &g_aShipPosition[lockedTarget],
                            &leadVector, &spot);
                        if (dodging == 1) {
                            if (RandomBelowOrEqual(1) == 0)
                                spot.x +=
                                    (int)(mayFire / 3) << 8;
                            if (RandomBelowOrEqual(1) == 0)
                                spot.y +=
                                    (int)(mayFire / 3) << 8;
                            if (RandomBelowOrEqual(1) == 0)
                                spot.z +=
                                    (int)(mayFire / 3) << 8;
                        }
                        SpawnCapitalFlakProjectile(
                            gun, obj, spot.x,
                            spot.y, spot.z);
                        break;
                    case 1:
                        adjustedAim = spot;
                        adjustedAim.x += aimOffset;
                        SpawnCapitalFlakProjectile(
                            gun, obj, adjustedAim.x,
                            adjustedAim.y, adjustedAim.z);
                        break;
                    case 2:
                        adjustedAim = spot;
                        adjustedAim.x -= aimOffset;
                        SpawnCapitalFlakProjectile(
                            gun, obj, adjustedAim.x,
                            adjustedAim.y, adjustedAim.z);
                        break;
                    case 3:
                        adjustedAim = spot;
                        adjustedAim.y += aimOffset;
                        SpawnCapitalFlakProjectile(
                            gun, obj, adjustedAim.x,
                            adjustedAim.y, adjustedAim.z);
                        break;
                    case 4:
                        adjustedAim = spot;
                        adjustedAim.y -= aimOffset;
                        SpawnCapitalFlakProjectile(
                            gun, obj, adjustedAim.x,
                            adjustedAim.y, adjustedAim.z);
                        break;
                    case 5:
                        adjustedAim = spot;
                        adjustedAim.y -= aimOffset;
                        SpawnCapitalFlakProjectile(
                            gun, obj, adjustedAim.x,
                            adjustedAim.y, adjustedAim.z);
                        break;
                    case 6:
                        adjustedAim = spot;
                        adjustedAim.y += aimOffset;
                        adjustedAim.x -= aimOffset;
                        SpawnCapitalFlakProjectile(
                            gun, obj, adjustedAim.x,
                            adjustedAim.y, adjustedAim.z);
                        break;
                    case 7:
                        adjustedAim = spot;
                        adjustedAim.x -= aimOffset;
                        adjustedAim.y -= aimOffset;
                        SpawnCapitalFlakProjectile(
                            gun, obj, adjustedAim.x,
                            adjustedAim.y, adjustedAim.z);
                        break;
                    }
                    flakIndex = (short)(flakIndex + 1);
                }
            }
        }
        if (flakShot != 0) {
            seen = 0;
            for (candidate = 0; candidate < 10; candidate++) {
                if (g_asSpikeriLaunchCountdown[candidate] != 0)
                    seen = 1;
            }
            if (seen == 0)
                PlaySfxWaveFileByNumber(0x27, obj, 0);
            g_2_asFlakCooldownTable[obj] =
                (short)((unsigned short)RandomInRange(0, 200) + 300);
        }
    }
}

/* Function start: 0x445AAC */
void capital_ship_intelligence(short obj)
{
    if (regulate_turn(obj) != 0)
        return;
    if (g_asObjectType[obj] >= 0x39)
        launch_carrier_fighter(obj);

    switch (g_asShipMissionType[obj]) {
    case MISSION_TYPE_NONE:
        stationary_intelligence(obj);
        break;
    case MISSION_TYPE_GOTO_WARP:
        reach_warp(obj);
        break;
    case MISSION_TYPE_WARP_ARRIVE:
        warp_arrival(obj);
        break;
    case MISSION_TYPE_COME_HOME:
        coming_home(obj);
        break;
    case MISSION_TYPE_ROUT:
        run_away(obj);
        break;
    case MISSION_TYPE_STRIKE:
        capital_ship_strike_mission(obj);
        break;
    default:
        if (obj == 0) {
            approach_cruise_speed(0);
            break;
        }
        if (g_asShipSide[obj] == SIDE_KILRATHI) {
            switch (g_asObjectType[obj]) {
            case 0x36:
            case 0x37:
                tanker_intelligence(obj);
                break;
            case 0x34:
            case 0x35:
            case 0x38:
            case 0x39:
            case 0x3A:
                destroyer_intelligence(obj);
                break;
            }
            break;
        }
        g_nTargetShip = g_acShipTarget[obj];
        if (unactive(g_nTargetShip) != 0)
            scan_for_enemy(obj, 15000);

        switch (g_asShipTactic[obj]) {
        case TACTIC_SELF_DEFENSE:
            approach_full_speed(obj);
            if (unactive(g_acShipTarget[obj]) != 0) {
                select_target(obj);
                if (unactive(g_acShipTarget[obj]) != 0)
                    reset_tactic(obj, TACTIC_NONE);
            } else {
                fire_turrets(obj);
            }
            break;
        default:
            if (g_nTargetShip != -1) {
                approach_full_speed(obj);
                g_asShipTactic[obj] =
                    TACTIC_SELF_DEFENSE;
                g_acShipTarget[obj] =
                    (signed char)g_nTargetShip;
                fire_turrets(obj);
            } else {
                approach_cruise_speed(obj);
            }
        }
    }
}

/* Function start: 0x445D9A */
void futurion_intelligence(short obj)
{
    get_facing_range_from_object(0, obj);
    g_asActionCount[obj]++;
    if (g_nTargetRange > 1000 &&
        g_asActionCount[obj] > 1000) {
        g_aeObjectClass[obj] =
            (enum ObjectClass)g_asObjectCounter[obj];
        return;
    }
    if (g_asActionCount[obj] > 200 &&
        g_nTargetRange < 4000 &&
        g_nTargetRange > 1000 &&
        g_nFacingToTarget > 80)
        g_aeObjectClass[obj] =
            (enum ObjectClass)g_asObjectCounter[obj];
}

/* Function start: 0x445E68 */
void heat_seeking_missile_intelligence(short obj)
{
    short chaffFound;
    short hottestHeat;
    short hottest;
    short other;

    chaffFound = 0;
    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass[other] != OBJECT_CLASS_NULL &&
            g_asObjectType[other] == OBJECT_DATA_CHAFF_POD &&
            distance_from_object(obj, other) < 3000) {
            g_acShipTarget[obj] = (signed char)other;
            chaffFound = 1;
            break;
        }
    }
    if (chaffFound != 0) {
        point_ship_at_object(obj, other);
        g_anShipSpeed[obj] =
            (get_ship_max_velocity(obj) + 10) << 8;
    } else if (g_nFacingToTarget < 0 ||
               g_acShipTarget[obj] == -1) {
        g_acShipTarget[obj] = -1;
        g_cViableTargetCount = 0;
        for (other = 0; other <= 9; other++) {
            if (other != obj &&
                g_aeObjectClass[other] >= OBJECT_CLASS_SHIP) {
                get_facing_range_from_object(obj, other);
                if (g_nTargetRange < 9000 &&
                    g_nFacingToTarget > 0 &&
                    g_nTargetFacing < 0) {
                    g_acViableTarget[
                        g_cViableTargetCount] = (signed char)other;
                    g_asViableTargetDistance[
                        g_cViableTargetCount] =
                            g_nTargetRange;
                    g_cViableTargetCount++;
                }
            }
        }
        SortViableTargetsByDistance();
        if (g_cViableTargetCount > 0) {
            for (hottestHeat = 3; hottestHeat > 0; hottestHeat--) {
                for (hottest = 0;
                     hottest < g_cViableTargetCount;
                     hottest++) {
                    if (g_aeObjectClass[
                            g_acViableTarget[hottest]] ==
                            OBJECT_CLASS_CAPITAL_SHIP ||
                        g_acShipExhaustHeat[
                            g_acViableTarget[hottest]] == hottestHeat) {
                        g_acShipTarget[obj] =
                            g_acViableTarget[hottest];
                        hottestHeat = 0;
                        break;
                    }
                }
            }
        }
        if (g_acShipTarget[obj] == -1)
            explode(obj, obj);
    } else {
        point_ship(obj, 0, &g_vToTarget);
        g_anShipSpeed[obj] =
            (get_ship_max_velocity(obj) + 10) << 8;
    }
}

void point_ship(short obj, short turnRate, const FixedVector *direction);
void point_ship_at_object(short obj, short other);
short explode(short attacker, short victim);
short distance_from_object(short obj, short other);
/* Function start: 0x446133 */
void FF_missile_intelligence(short obj)
{
    short chaffFound;
    short other;

    chaffFound = 0;
    if (g_asShipTactic[obj] == TACTIC_RAM) {
        for (other = 0; other < 10; other++) {
            if (g_aeObjectClass[other] != OBJECT_CLASS_NULL &&
                g_asObjectType[other] ==
                    OBJECT_DATA_CHAFF_POD &&
                g_asShipSide[
                    g_acObjectOwner[other]] !=
                    g_asShipSide[
                        g_acObjectOwner[obj]] &&
                distance_from_object(obj, other) < 3000) {
                g_acShipTarget[obj] = (signed char)other;
                chaffFound = 1;
                break;
            }
        }
        if (chaffFound != 0) {
            point_ship_at_object(obj, other);
            g_anShipSpeed[obj] =
                (get_ship_max_velocity(obj) + 10) << 8;
        } else if (g_acShipTarget[obj] == -1) {
            g_cViableTargetCount = 0;
            for (other = 0; other <= 9; other++) {
                if (obj != other &&
                    g_aeObjectClass[other] >= OBJECT_CLASS_SHIP &&
                    (g_asShipSide[
                         g_acObjectOwner[obj]] !=
                         g_asShipSide[other] ||
                     g_acShipCommunicator[other] == -1)) {
                    g_nTargetRange =
                        distance_from_object(obj, other);
                    if (g_nTargetRange < 9000) {
                        g_acViableTarget[
                            g_cViableTargetCount] =
                                (signed char)other;
                        g_asViableTargetDistance[
                            g_cViableTargetCount] =
                                g_nTargetRange;
                        g_cViableTargetCount++;
                    }
                }
            }
            SortViableTargetsByDistance();
            if (g_cViableTargetCount > 0)
                g_acShipTarget[obj] =
                    g_acViableTarget[0];
        } else {
            point_ship(obj, 0, &g_vToTarget);
            g_anShipSpeed[obj] =
                (get_ship_max_velocity(obj) + 10) << 8;
        }
    }
}

/* Function start: 0x446395 */
void chaff_intelligence(short obj)
{
    short other;
    short distance;

    for (other = 1; other < 10; other++) {
        if (g_aeObjectClass[other] == OBJECT_CLASS_MISSILE &&
            g_asObjectType[other] != OBJECT_DATA_CHAFF_POD) {
            distance = distance_from_object(obj, other);
            if (distance < 3000) {
                switch (g_asObjectType[other]) {
                case OBJECT_DATA_SPICULUM_IMAGE_RECOGNITION_MISSILE:
                    explode(obj, other);
                    break;
                case OBJECT_DATA_JAVELIN_HEAT_SEEKING_MISSILE:
                case OBJECT_DATA_PILUM_FRIEND_OR_FOE_MISSILE:
                    g_acShipTarget[other] = (signed char)obj;
                    break;
                }
            }
        }
    }
}

short explode(short attacker, short victim);

/* Function start: 0x446474 */
void mine_intelligence(short obj)
{
    short other;
    short range;

    if (g_asObjectCounter[obj] != -1)
        return;
    for (other = 0; other <= 9; other++) {
        if (other != obj &&
            g_aeObjectClass[other] >= OBJECT_CLASS_SHIP) {
            range = distance_from_object(obj, other);
            if (range < g_aObjectTypeData[
                            g_acObjectType[obj]].collisionRadius ||
                (range < 50 && RandomBelowOrEqual(7) == 0)) {
                explode(obj, obj);
                return;
            }
        }
    }
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short g_2_asFlakCooldownTable[16]; /* 0x005d1bd0 */
short g_asSpikeriCooldown[16]; /* 0x005d1bf0 */
short g_asSpikeriLaunchCountdown[10]; /* 0x005d1c10 */
