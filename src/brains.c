/*
 *  Ship intelligence and missions.
 *
 *  Retail object 0x442770-0x44655F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x442770 */
void cruise_home(short obj)
{
    short range;

    if (abandoned(obj, 0) != 0)
        return;
    if ((g_abShipTurn_00495fd8[obj] & 7) == 5) {
        if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP &&
            g_asShipSide_004955d0[obj] == g_asShipSide_004955d0[0] &&
            distance_from_object(obj, 0) > 16000) {
            remove_object(obj);
            return;
        }
        if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_CAPITAL_SHIP ||
            g_asObjectType_00495298[obj] == 0x33)
            approach_cruise_speed(obj);
        else if (normal_speed(obj) != 0)
            fire_afterburner(obj, 10);
        if (CanSetNewShipTurnGoal(obj) != 0)
            point_ship_at_point(obj, &g_aShipDestination_004953f0[obj]);
        range = distance_from_point(obj, &g_aShipDestination_004953f0[obj]);
        if (equ_vector(&g_aShipDestination_004953f0[obj],
                       &g_aShipMissionSpot_00495e18[obj]) != 0) {
            if (range < 5000) {
                reset_tactic(obj, TACTIC_HEAD_HOME);
                set_special(obj, SPECIAL_MANEUVER_KILL_ENGINES);
                zero_vector(&g_aShipVelocity_00494898[obj]);
            }
            return;
        }
        if (range < 1500) {
            if (g_aMissionObjectives_004932a8[
                    g_abFlightPath_004932a0[
                        g_abShipNavPointIndex_00495f60[obj]]].type != 1)
                flag_objective(
                    g_abFlightPath_004932a0[
                        g_abShipNavPointIndex_00495f60[obj]], 1);
            get_follow_point(obj, &g_aShipDestination_004953f0[obj]);
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

    switch (g_asShipTactic_00495f30[obj]) {
    case TACTIC_CRUISE:
        cruise_home(obj);
        break;
    case TACTIC_HEAD_HOME:
        home = find_ship_index(g_stMissionHeader_005d3e70.homeMissionShip);
        range = distance_from_object(obj, home);
        if (g_asObjectCollisionRadius_004950e8[obj] + 800 < range)
            approach_cruise_speed(obj);
        else
            approach_ship_speed(obj, home);
        point_ship_behind_object(obj, home);
        if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP &&
            g_asObjectCollisionRadius_004950e8[obj] + 800 > range)
            remove_object(obj);
        break;
    case TACTIC_NONE:
        reset_tactic(obj, TACTIC_CRUISE);
        if (g_asShipSide_004955d0[obj] == SIDE_IMPERIAL &&
            g_asShipMissionType_00495de8[obj] == MISSION_TYPE_ROUT) {
            g_aShipDestination_004953f0[obj] =
                g_aMissionObjectives_004932a8[
                    find_objective(1, -1)].position;
        } else {
            get_first_follow_point(obj, &g_aShipDestination_004953f0[obj]);
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

    if (unactive(g_asShipWingLeader_00495dd0[obj]) == 0 &&
        g_asShipMissionType_00495de8[
            g_asShipWingLeader_00495dd0[obj]] == MISSION_TYPE_ROUT) {
        maintain_formation(obj);
        return;
    }
    if (g_asShipSide_004955d0[obj] == SIDE_IMPERIAL) {
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

    if (newTarget != -1 && newTarget != g_acShipTarget_00495f20[obj])
        g_acShipTarget_00495f20[obj] = newTarget;
    else if (target_valid(obj, g_acShipTarget_00495f20[obj]) == 0)
        select_target(obj);
    return g_acShipTarget_00495f20[obj];
}

/* Function start: 0x442D58 */
short check_destroy_target(short obj)
{
    short destroyTarget = find_ship_index(g_asShipMissionParameter_00495e00[obj]);

    if (destroyTarget == -1) {
        g_acShipTarget_00495f20[obj] = check_engage_target(obj);
    } else if (g_aeObjectClass_00495328[destroyTarget] ==
                   OBJECT_CLASS_FUTURION ||
               gone_ship(g_asShipMissionParameter_00495e00[obj]) != 0) {
        check_engage_target(obj);
    } else {
        if (70 - MaxShort(0, MinShort(4,
                (short)g_asPilotLevel_00495d60[obj])) * 15 <
            evaluate_damage(obj)) {
            g_acShipTarget_00495f20[obj] = destroyTarget;
            if (g_asShipSide_004955d0[destroyTarget] ==
                g_asShipSide_004955d0[obj])
                g_acShipTarget_00495f20[obj] = -1;
        } else if (target_valid(obj, g_acShipTarget_00495f20[obj]) != 0 &&
                   RandomBelow(100) > 3) {
            check_engage_target(obj);
        } else {
            g_acShipTarget_00495f20[obj] = destroyTarget;
        }
    }
    if (g_acShipTarget_00495f20[obj] == -1)
        check_engage_target(obj);
    return g_acShipTarget_00495f20[obj];
}

/* Function start: 0x442EFC */
void maneuvering(short obj, short newTarget)
{
    g_acShipTarget_00495f20[obj] = newTarget;
    intelligence_events(obj);
    perform_maneuver(obj);
}

/* Function start: 0x442F2D */
void formation_burst(short obj)
{
    approach_full_speed(obj);
    if (CanSetNewShipTurnGoal(obj) != 0)
        point_ship(obj, 0, &g_aShipDestination_004953f0[obj]);
    g_asShipCount_00495ff8[obj]++;
    if (g_asShipCount_00495ff8[obj] > 9) {
        if (g_asShipMissionType_00495de8[obj] == MISSION_TYPE_STRIKE)
            engage(obj, g_acShipTarget_00495f20[obj],
                   OBJECTIVE_DESTROY_SHIP);
        else
            engage(obj, g_acShipTarget_00495f20[obj],
                   OBJECTIVE_ENGAGE_ENEMY);
    }
}

/* Function start: 0x442FE9 */
void disallow_engage(void)
{
    g_bEngageAllowed_0049612c = 0;
}

/* Function start: 0x442FFD */
void allow_engage(void)
{
    g_bEngageAllowed_0049612c = 1;
    g_nAutoEngageTimer_00496130 = -1;
}

void engage(short obj, short target, short objective);
void send_message(short obj, signed char message);
short attacker_in_range(short obj, unsigned short range);
void fire_afterburner(short obj, short time);
short real_velocity(short obj);

/* Function start: 0x44301A */
void try2allow_engage(short obj)
{
    if (obj != g_nYourWingman_0049346c) {
        allow_engage();
        return;
    }
    if (g_nWingmanEngagementMode_0049613e == 1) {
        allow_engage();
        return;
    }
    if (g_nWingmanEngagementMode_0049613e == 2 &&
        RandomBelowOrEqual(100) < 50) {
        allow_engage();
        return;
    }
    g_nAutoEngageTimer_00496130 = -40;
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
    short leader = g_asShipWingLeader_00495dd0[obj];

    if (leader == -1)
        leader = obj;
    maintain_formation(obj);
    if (attacker_in_range(leader, 12000) != 0) {
        if (obj == g_nYourWingman_0049346c ||
            g_nYourWingman_0049346c == -1) {
            if (g_nAutoEngageTimer_00496130 < -1)
                g_nAutoEngageTimer_00496130++;
            else if (g_nAutoEngageTimer_00496130 != -1 &&
                     --g_nAutoEngageTimer_00496130 == 0)
                try2allow_engage(obj);
        }
        if (g_bEngageAllowed_0049612c != 0) {
            engage(obj, g_nTargetShip_004931a0,
                   OBJECTIVE_ENGAGE_ENEMY);
        } else if (obj == g_nYourWingman_0049346c &&
                   g_nAutoEngageTimer_00496130 == -1) {
            send_message(obj, 3);
            g_nAutoEngageTimer_00496130 = 40;
        }
    } else if (obj == g_nYourWingman_0049346c &&
               g_nEnemySighting_0049b670 != g_nCurrentWave_004931c0 &&
               any_enemy(obj, 16000) != 0 &&
               message_showing() == 0 && g_nCurrentView_00492fa8 == 0) {
        send_message(obj, 2);
        g_nEnemySighting_0049b670 = g_nCurrentWave_004931c0;
    }

    if (g_aeSpecialManeuver_00495600[obj] == SPECIAL_MANEUVER_NONE &&
        distance_from_object(obj, leader) > 9000) {
        if (facing_to_object(obj, &g_aShipPosition_00494550[leader]) > 85 &&
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
    switch (g_acShipSequence_00495fe8[obj]) {
    case 0:
        steady_object(obj);
        g_anYawGoal_004954c0[obj] = -30;
        g_anRollGoal_004954d8[obj] = -45;
        g_anPitchGoal_004954a8[obj] = -20;
        g_acShipSequence_00495fe8[obj]++;
        break;
    case 1:
        if (CanSetNewShipTurnGoal(obj) != 0)
            engage(obj, g_acShipTarget_00495f20[obj],
                   OBJECTIVE_ENGAGE_ENEMY);
        break;
    default:
        g_acShipSequence_00495fe8[obj] = 0;
    }
}

/* Function start: 0x4433B1 */
void imperial_wingman(short obj)
{
    switch (g_aeShipObjective_00495f08[obj]) {
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
    short leader = g_asShipWingLeader_00495dd0[obj];

    if (leader == -1) {
        change_mission_type(obj, MISSION_TYPE_PATROL);
        return;
    } else if (unactive(leader) != 0) {
        inherit_leader(obj);
        return;
    }
    if ((g_aeShipObjective_00495f08[leader] == OBJECTIVE_ENGAGE_ENEMY ||
         g_aeShipObjective_00495f08[leader] == OBJECTIVE_DESTROY_SHIP) &&
        g_aeShipObjective_00495f08[obj] != g_aeShipObjective_00495f08[leader])
        engage(obj, g_acShipTarget_00495f20[obj],
               g_aeShipObjective_00495f08[leader]);

    switch (g_aeShipObjective_00495f08[obj]) {
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
    if (g_asShipSide_004955d0[obj] == SIDE_IMPERIAL) {
        imperial_wingman(obj);
        return;
    }
    kilrathi_wingman(obj);
}

/* Function start: 0x44366D */
short dist_from_home(short obj)
{
    return distance_from_point(obj, &g_aShipMissionSpot_00495e18[obj]);
}

/* Function start: 0x443698 */
short scan_and_lock(short obj, short newTactic)
{
    g_acShipTarget_00495f20[obj] = scan_for_enemy(obj, 14000);
    if (g_acShipTarget_00495f20[obj] != -1)
        g_asShipTactic_00495f30[obj] = newTactic;
    return g_acShipTarget_00495f20[obj] != -1;
}

/* Function start: 0x443707 */
void patrol_area(short obj)
{
    short target = g_acShipTarget_00495f20[obj];

    switch (g_asShipTactic_00495f30[obj]) {
    case TACTIC_LOOK_OUT:
        approach_cruise_speed(obj);
        if (scan_and_lock(obj, TACTIC_APPROACH_TARGET) == 0 &&
            dist_from_home(obj) > 8000)
            reset_tactic(obj, TACTIC_HEAD_HOME);
        break;
    case TACTIC_HEAD_HOME:
        approach_cruise_speed(obj);
        if (scan_and_lock(obj, TACTIC_APPROACH_TARGET) == 0) {
            get_facing_range_from_point(obj, &g_aShipMissionSpot_00495e18[obj]);
            if (g_nTargetRange_0049319c < 3000) {
                reset_tactic(obj, TACTIC_LOOK_OUT);
            } else {
                point_ship_at_point(obj, &g_aShipMissionSpot_00495e18[obj]);
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
            if (g_nTargetRange_0049319c < 10000)
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
    switch (g_aeShipObjective_00495f08[obj]) {
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
        g_aeShipObjective_00495f08[obj] = OBJECTIVE_WANDER;
        g_asShipTactic_00495f30[obj] = TACTIC_APPROACH_TARGET;
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
    if ((g_abShipTurn_00495fd8[obj] & 7) == 6)
        g_acShipTarget_00495f20[obj] = scan_for_enemy(obj, 15000);

    if (g_acShipTarget_00495f20[obj] == -1) {
        approach_cruise_speed(obj);
    } else {
        fire_turrets(obj);
        get_facing_range_from_object(obj, g_acShipTarget_00495f20[obj]);
        if (g_nFacingToTarget_00493194 > 65)
            approach_half_speed(obj);
        else
            approach_full_speed(obj);
    }

    if ((g_abShipTurn_00495fd8[obj] & 7) == 2) {
        if (g_asShipMissionParameter_00495e00[obj] ==
            g_nCurrentNavPoint_004931bc) {
            g_acShipNavPoint_004953e0[obj] =
                (signed char)g_nCurrentNavPoint_004931bc;
            g_aShipDestination_004953f0[obj] =
                g_aShipMissionSpot_00495e18[obj];
        }
        if (CanSetNewShipTurnGoal(obj) != 0)
            point_ship_at_point(obj, &g_aShipDestination_004953f0[obj]);
        range = distance_from_point(obj, &g_aShipDestination_004953f0[obj]);
        if (range < 1500) {
            if (g_asShipSide_004955d0[obj] == SIDE_IMPERIAL)
                flag_reached((short)g_abFlightPath_004932a0[
                    g_abShipNavPointIndex_00495f60[obj]], 1);
            if (equ_vector(&g_aShipDestination_004953f0[obj],
                    &g_aShipMissionSpot_00495e18[obj]) != 0) {
                reset_tactic(obj, TACTIC_SIT_STILL);
                set_special(obj, SPECIAL_MANEUVER_KILL_ENGINES);
            } else {
                get_follow_point(obj, &g_aShipDestination_004953f0[obj]);
            }
        }
    }
}

/* Function start: 0x443C45 */
void prepare_for_jump(short obj)
{
    short count;
    short delay;

    if (g_anShipSpeed_00494e20[obj] != 0) {
        set_special(obj, SPECIAL_MANEUVER_STOP_DRIFT);
        return;
    }
    count = ++g_asShipCount_00495ff8[obj];
    delay = g_asShipSide_004955d0[obj] == SIDE_KILRATHI ? 250 : 25;
    if (count <= delay)
        return;

    get_facing_range_from_object(obj, 0);
    if (g_nFacingToTarget_00493194 > 90 && CanSetNewShipTurnGoal(obj) != 0) {
        g_anYawGoal_004954c0[obj] = signed_random(30);
        return;
    }
    delay = g_asShipSide_004955d0[obj] == SIDE_KILRATHI ? 270 : 45;
    if (g_asShipCount_00495ff8[obj] > delay ||
        (g_nTargetFacing_00493198 > 80 && g_nTargetRange_0049319c < 6000)) {
        reset_tactic(obj, TACTIC_WARP_OUT);
        fire_afterburner(obj, 10);
    }
}

/* Function start: 0x443D6E */
void accelerate_and_jump(short obj)
{
    approach_full_speed(obj);
    if (g_asShipCount_00495ff8[obj]++ == 4)
        warp(obj);
}

/* Function start: 0x443DBD */
void reach_warp(short obj)
{
    switch (g_asShipTactic_00495f30[obj]) {
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
        get_first_follow_point(obj, &g_aShipDestination_004953f0[obj]);
        break;
    default:
        fail(obj);
    }
}

/* Function start: 0x443E8C */
void split_patrol_mission(short obj)
{
    if (g_asShipMissionParameter_00495e00[obj] ==
        g_nCurrentNavPoint_004931bc) {
        g_asShipWingLeader_00495dd0[obj] = -1;
        g_acShipNavPoint_004953e0[obj] =
            (signed char)g_nCurrentNavPoint_004931bc;
    }
    g_aeShipObjective_00495f08[obj] = OBJECTIVE_HOLD_FORMATION;
    imperial_wingman(obj);
}

/* Function start: 0x443EE9 */
void warp_arrival(short obj)
{
    if (g_asShipTactic_00495f30[obj] == TACTIC_WARP_IN)
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
    short buddy = find_ship_index(g_asShipMissionParameter_00495e00[obj]);

    if (unactive(buddy) != 0) {
        change_mission_type(obj, MISSION_TYPE_PATROL);
        return;
    }
    if ((g_abShipTurn_00495fd8[obj] & 3) == 0 && in_danger(buddy) != 0 &&
        g_nTargetRange_0049319c < 3000)
        engage(obj, g_nTargetShip_004931a0,
               OBJECTIVE_ENGAGE_ENEMY);
    if (g_aeShipObjective_00495f08[obj] != OBJECTIVE_HOME_BASE &&
        (g_abShipTurn_00495fd8[obj] & 7) == 4 &&
        distance_from_object(obj, buddy) > 5000)
        reset_objective(obj, OBJECTIVE_HOME_BASE);

    switch (g_aeShipObjective_00495f08[obj]) {
    case OBJECTIVE_HOME_BASE:
        return_to_buddy(obj, buddy);
        break;
    case OBJECTIVE_WANDER:
        escort_buddy(obj, buddy);
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
    if (gone_ship(g_asShipMissionParameter_00495e00[obj]) != 0)
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

    if (g_asObjectType_00495298[obj] == 0x33)
        nearRange = 8000;
    else
        nearRange = 5000;
    dist = (unsigned short)distance_from_object(obj, goal);
    if (g_aeObjectClass_00495328[goal] != OBJECT_CLASS_FUTURION &&
        70 - MaxShort(0, MinShort(4,
                (short)g_asPilotLevel_00495d60[obj])) * 15 <
            evaluate_damage(obj) &&
        nearRange < dist) {
        streak_toward(obj, goal, (short)dist);
        return;
    }
    possibleTarget = scan_for_enemy(obj, 10000);
    possibleRange = (unsigned short)g_nTargetRange_0049319c;
    if (possibleTarget != -1 &&
        (possibleRange * 3 < dist ||
         g_aeObjectClass_00495328[goal] == OBJECT_CLASS_FUTURION)) {
        init_formation_burst(obj);
        g_acShipTarget_00495f20[obj] = (signed char)possibleTarget;
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

    goal = find_ship_index(g_asShipMissionParameter_00495e00[obj]);
    if (goal == -1 ||
        (goal != -1 &&
         g_aeObjectClass_00495328[goal] == OBJECT_CLASS_FUTURION)) {
        if (is_team_member(g_asShipMissionIndex_00495d00[obj]) == 0) {
            check_goal(obj);
        } else if (g_aMissionShips_00492290[
                       g_asShipMissionParameter_00495e00[obj]].state == 3) {
            change_mission_type(obj, 9);
            g_asShipTactic_00495f30[obj] = TACTIC_CRUISE;
            if (g_bHomeObjectiveFlagged_0049b674 == 0) {
                flag_objective(g_cCurrentObjective_004931cc, 1);
                set_next_destination();
                g_bHomeObjectiveFlagged_0049b674++;
            }
            locate_ship(g_stMissionHeader_005d3e70.homeMissionShip,
                        &g_aShipMissionSpot_00495e18[obj]);
            g_aShipDestination_004953f0[obj] =
                g_aShipMissionSpot_00495e18[obj];
            return;
        } else {
            g_bHomeObjectiveFlagged_0049b674 = 0;
            switch (g_asShipTactic_00495f30[obj]) {
            case TACTIC_NONE:
                reset_tactic(obj, TACTIC_CRUISE);
                get_first_follow_point(obj,
                                       &g_aShipDestination_004953f0[obj]);
                break;
            case TACTIC_CRUISE:
                cruise_to_destination(obj);
                break;
            }
            return;
        }
    }
    switch (g_aeShipObjective_00495f08[obj]) {
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
    short master = find_ship_index(g_asShipMissionParameter_00495e00[obj]);

    if (master == -1) {
        change_mission_type(obj, MISSION_TYPE_PATROL);
        return;
    }
    if (g_abShipTurn_00495fd8[obj] % 10 == 0 &&
        in_danger(master) != 0 && g_nTargetRange_0049319c < 6000 &&
        g_aeShipObjective_00495f08[obj] != OBJECTIVE_ENGAGE_ENEMY)
        engage(obj, g_nTargetShip_004931a0,
               OBJECTIVE_ENGAGE_ENEMY);
    if (g_aeShipObjective_00495f08[obj] != OBJECTIVE_HOME_BASE &&
        (g_abShipTurn_00495fd8[obj] & 7) == 4 &&
        distance_from_object(obj, master) > 10000)
        reset_objective(obj, OBJECTIVE_HOME_BASE);

    switch (g_aeShipObjective_00495f08[obj]) {
    case OBJECTIVE_WANDER:
        g_acShipTarget_00495f20[obj] =
            (signed char)scan_for_enemy(obj, 7000);
        if (g_acShipTarget_00495f20[obj] != -1) {
            engage(obj, (short)g_acShipTarget_00495f20[obj],
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
    short goal = find_ship_index(g_asShipMissionParameter_00495e00[obj]);

    if (unactive(goal) != 0) {
        change_mission_type(obj, MISSION_TYPE_PATROL);
        return;
    }
    switch (g_aeShipObjective_00495f08[obj]) {
    case OBJECTIVE_REACH_SHIP:
        if (attacker_in_range(obj, 3500) != 0)
            engage(obj, g_nTargetShip_004931a0,
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
    if (g_acShipCollisionCooldown_00496010[obj] > 0)
        g_acShipCollisionCooldown_00496010[obj]--;
    if (regulate_turn(obj) != 0)
        return;
    switch (g_asShipMissionType_00495de8[obj]) {
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
    short radius = g_aMissionNavPoints_00491e98[
        g_nCurrentNavPoint_004931bc].proximityRadius >> 1;
    FixedVector center =
        g_aMissionNavPoints_00491e98[g_nCurrentNavPoint_004931bc].position;
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
        get_facing_range_from_object(obj, g_nTargetShip_004931a0);
        g_acShipTarget_00495f20[obj] = g_nTargetShip_004931a0;
        fire(obj, g_nTargetShip_004931a0);
        if (CanSetNewShipTurnGoal(obj) != 0) {
            if (RandomBelowOrEqual(4) == 0) {
                g_anYawGoal_004954c0[obj] = signed_random(90);
                g_anRollGoal_004954d8[obj] = signed_random(90);
            } else {
                point_capital_ship_at_object(obj,
                                             g_nTargetShip_004931a0);
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
        g_acShipTarget_00495f20[obj] = -1;
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

    if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_BASE)
        g_anObjectYawRotation_00494fc8[obj] = 2;
    fire_turrets(obj);
    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass_00495328[other] >= OBJECT_CLASS_CAPITAL_SHIP &&
            g_asShipSide_004955d0[other] != g_asShipSide_004955d0[obj]) {
            g_asShipMissionType_00495de8[obj] = MISSION_TYPE_STRIKE;
            g_acShipTarget_00495f20[obj] = -1;
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
        g_acObjectOwner_00495208[projectile] = (signed char)obj;
        ComputeVectorDelta(&g_aShipPosition_00494550[obj],
                           &g_aShipPosition_00494550[
                               g_acShipTarget_00495f20[obj]],
                           &direction);
        NormalizeFixedVector(&direction);
        ScaleFixedVector(
            &direction,
            (int)g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[obj]].collisionRadius << 8,
            &direction);
        zero_vector(&g_aShipPosition_00494550[projectile]);
        AddFixedVectors(&g_aShipPosition_00494550[obj], &direction,
                        &g_aShipPosition_00494550[projectile]);
        speed = g_aObjectTypeData_00496d30[0x0d].maximumVelocity;
        g_asObjectDamage_00495178[projectile] =
            g_aObjectTypeData_00496d30[0x0d].damageCapacity;
        g_asObjectCounter_00494be0[projectile] =
            g_aObjectTypeData_00496d30[0x0d].lifetime;
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

/* Function start: 0x445087 */
void capital_ship_strike_mission(short obj)
{
    short projectileVelocity;
    short enemyFighterCount;
    short effect;
    int aimOffset;
    short travelTicks;
    short target;
    short range;
    short found;
    short flakBurstIndex;
    FixedVector direction;
    FixedVector lead;
    short clearance;
    short firedFlak;
    short evasive;
    short friendlyFighterCount;
    FixedVector adjustedAim;
    FixedVector aimPoint;
    short weapon;
    short other;
    ShipWeaponSlot *slot;
    short lockedTarget;

    flakBurstIndex = 0;
    firedFlak = 0;
    target = g_acShipTarget_00495f20[obj];
    if (target != -1 &&
        g_aeObjectClass_00495328[target] < OBJECT_CLASS_CAPITAL_SHIP)
        destroyer_intelligence(obj);
    if (target == -1) {
        g_acShipTarget_00495f20[obj] =
            (signed char)find_ship_index(
                g_asShipMissionParameter_00495e00[obj]);
        g_acShipSequence_00495fe8[obj] = 0;
    }
    target = g_acShipTarget_00495f20[obj];
    if (target == -1 || unactive(target) != 0) {
        found = 0;
        for (other = 0; other < 10; other++) {
            if (g_aeObjectClass_00495328[other] >=
                    OBJECT_CLASS_CAPITAL_SHIP &&
                g_asShipSide_004955d0[other] !=
                    g_asShipSide_004955d0[obj]) {
                g_acShipTarget_00495f20[obj] = (signed char)other;
                target = other;
                found = 1;
            }
        }
        if (found == 0) {
            g_acShipTarget_00495f20[obj] = -1;
            approach_zero_speed(obj);
            fire_turrets(obj);
            return;
        }
    }
    if (g_aeObjectClass_00495328[target] < OBJECT_CLASS_CAPITAL_SHIP)
        destroyer_intelligence(obj);
    fire_turrets(obj);
    g_acShipTarget_00495f20[obj] = (signed char)target;
    get_facing_range_from_object(obj, target);

    if (DAT_005d1c10[obj] != 0) {
        DAT_005d1c10[obj]--;
        if (DAT_005d1c10[obj] == 0) {
            effect = new_object(0x0e, obj);
            if (effect != -1) {
                DAT_005d1bf0[obj] =
                    (short)((unsigned short)RandomInRange(0, 200) +
                            400);
                g_acObjectOwner_00495208[effect] = (signed char)obj;
                ComputeVectorDelta(
                    &g_aShipPosition_00494550[obj],
                    &g_aShipPosition_00494550[
                        g_acShipTarget_00495f20[obj]],
                    &direction);
                NormalizeFixedVector(&direction);
                ScaleFixedVector(
                    &direction,
                    (int)g_aObjectTypeData_00496d30[
                        g_acObjectType_00493980[obj]].collisionRadius << 8,
                    &direction);
                zero_vector(&g_aShipPosition_00494550[effect]);
                AddFixedVectors(&g_aShipPosition_00494550[obj],
                                &direction,
                                &g_aShipPosition_00494550[effect]);
                projectileVelocity =
                    g_aObjectTypeData_00496d30[0x0e].maximumVelocity;
                g_asObjectDamage_00495178[effect] =
                    g_aObjectTypeData_00496d30[0x0e].damageCapacity;
                g_asObjectCounter_00494be0[effect] =
                    g_aObjectTypeData_00496d30[0x0e].lifetime;
                send_at_point(
                    effect,
                    &g_aShipPosition_00494550[
                        g_acShipTarget_00495f20[obj]],
                    projectileVelocity);
                RecordCannedSceneObjectEvent(effect, 0);
                PlaySfxWaveFileByNumber(0x29, obj, 0);
            }
        }
    } else {
        range = distance_from_object(obj, target);
        lockedTarget = target;
        if (range < 5000) {
            g_acShipSequence_00495fe8[obj] = 1;
            point_capital_ship_at_object(obj, lockedTarget);
            approach_speed(
                obj,
                (unsigned short)RandomInRange(
                    50, (unsigned short)(
                        g_asShipMaximumVelocity_00495f70[obj] << 8)));
        } else if (range > 10000 ||
                   g_acShipSequence_00495fe8[obj] != 1) {
            g_acShipSequence_00495fe8[obj] = 2;
            point_ship_at_object(obj, lockedTarget);
            approach_speed(
                obj,
                (unsigned short)RandomInRange(
                    50, (unsigned short)(
                        g_asShipMaximumVelocity_00495f70[obj] << 8)));
        }
        if (range < 10000) {
            slot = (ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1];
            for (weapon = 0;
                 weapon <
                     (signed char)g_aShipWeapons_004956b0[obj][0];
                 weapon++, slot++) {
                if (slot->type == 0x0e &&
                    DAT_005d1bf0[obj] == 0 &&
                    DAT_005d1c10[obj] == 0 &&
                    g_nFacingToTarget_00493194 > 20) {
                    PlaySfxWaveFileByNumber(0x28, obj, 0);
                    DAT_005d1c10[obj] = 20;
                }
                if (slot->type == 0x0d &&
                    DAT_005d1bd0[obj] == 0) {
                    enemyFighterCount = 0;
                    friendlyFighterCount = 0;
                    evasive = 0;
                    for (other = 0; other < 10; other++) {
                        if (g_aeObjectClass_00495328[other] ==
                            OBJECT_CLASS_SHIP) {
                            if (g_asShipSide_004955d0[other] !=
                                g_asShipSide_004955d0[obj])
                                enemyFighterCount =
                                    (short)(enemyFighterCount + 1);
                            else
                                friendlyFighterCount =
                                    (short)(friendlyFighterCount + 1);
                        }
                    }
                    if (enemyFighterCount > friendlyFighterCount &&
                        RandomBelowOrEqual(2) == 0)
                        evasive = 1;
                    if (g_asObjectDamage_00495178[obj] > 0 &&
                        RandomBelowOrEqual(1) == 0)
                        evasive = 1;
                    aimOffset =
                        ((int)g_asObjectCollisionRadius_004950e8[
                            lockedTarget] << 8) * 3;
                    firedFlak = 1;
                    switch (flakBurstIndex % 8) {
                    case 0:
                        ComputeVectorDelta(
                            &g_aShipPosition_00494550[obj],
                            &g_aShipPosition_00494550[lockedTarget],
                            &g_vToTarget_00493188);
                        clearance = (short)(
                            FixedToShortSaturating(
                                Vector_magnitude(
                                    &g_vToTarget_00493188)) -
                            g_asObjectCollisionRadius_004950e8[obj] -
                            g_asObjectCollisionRadius_004950e8[
                                lockedTarget]);
                        travelTicks = (short)(clearance / 250);
                        ScaleFixedVector(
                            &g_aShipVelocity_00494898[lockedTarget],
                            (int)travelTicks << 8, &lead);
                        AddFixedVectors(
                            &g_aShipPosition_00494550[lockedTarget],
                            &lead, &aimPoint);
                        if (evasive == 1) {
                            if (RandomBelowOrEqual(1) == 0)
                                aimPoint.x +=
                                    (int)(clearance / 3) << 8;
                            if (RandomBelowOrEqual(1) == 0)
                                aimPoint.y +=
                                    (int)(clearance / 3) << 8;
                            if (RandomBelowOrEqual(1) == 0)
                                aimPoint.z +=
                                    (int)(clearance / 3) << 8;
                        }
                        SpawnCapitalFlakProjectile(
                            weapon, obj, aimPoint.x,
                            aimPoint.y, aimPoint.z);
                        break;
                    case 1:
                        adjustedAim = aimPoint;
                        adjustedAim.x += aimOffset;
                        SpawnCapitalFlakProjectile(
                            weapon, obj, adjustedAim.x,
                            adjustedAim.y, adjustedAim.z);
                        break;
                    case 2:
                        adjustedAim = aimPoint;
                        adjustedAim.x -= aimOffset;
                        SpawnCapitalFlakProjectile(
                            weapon, obj, adjustedAim.x,
                            adjustedAim.y, adjustedAim.z);
                        break;
                    case 3:
                        adjustedAim = aimPoint;
                        adjustedAim.y += aimOffset;
                        SpawnCapitalFlakProjectile(
                            weapon, obj, adjustedAim.x,
                            adjustedAim.y, adjustedAim.z);
                        break;
                    case 4:
                        adjustedAim = aimPoint;
                        adjustedAim.y -= aimOffset;
                        SpawnCapitalFlakProjectile(
                            weapon, obj, adjustedAim.x,
                            adjustedAim.y, adjustedAim.z);
                        break;
                    case 5:
                        adjustedAim = aimPoint;
                        adjustedAim.y -= aimOffset;
                        SpawnCapitalFlakProjectile(
                            weapon, obj, adjustedAim.x,
                            adjustedAim.y, adjustedAim.z);
                        break;
                    case 6:
                        adjustedAim = aimPoint;
                        adjustedAim.y += aimOffset;
                        adjustedAim.x -= aimOffset;
                        SpawnCapitalFlakProjectile(
                            weapon, obj, adjustedAim.x,
                            adjustedAim.y, adjustedAim.z);
                        break;
                    case 7:
                        adjustedAim = aimPoint;
                        adjustedAim.x -= aimOffset;
                        adjustedAim.y -= aimOffset;
                        SpawnCapitalFlakProjectile(
                            weapon, obj, adjustedAim.x,
                            adjustedAim.y, adjustedAim.z);
                        break;
                    }
                    flakBurstIndex = (short)(flakBurstIndex + 1);
                }
            }
        }
        if (firedFlak != 0) {
            found = 0;
            for (other = 0; other < 10; other++) {
                if (DAT_005d1c10[other] != 0)
                    found = 1;
            }
            if (found == 0)
                PlaySfxWaveFileByNumber(0x27, obj, 0);
            DAT_005d1bd0[obj] =
                (short)((unsigned short)RandomInRange(0, 200) + 300);
        }
    }
}

/* Function start: 0x445AAC */
void capital_ship_intelligence(short obj)
{
    if (regulate_turn(obj) != 0)
        return;
    if (g_asObjectType_00495298[obj] >= 0x39)
        launch_carrier_fighter(obj);

    switch (g_asShipMissionType_00495de8[obj]) {
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
        } else if (g_asShipSide_004955d0[obj] == SIDE_KILRATHI) {
            switch (g_asObjectType_00495298[obj]) {
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
        } else {
            g_nTargetShip_004931a0 = g_acShipTarget_00495f20[obj];
            if (unactive(g_nTargetShip_004931a0) != 0)
                scan_for_enemy(obj, 15000);

            switch (g_asShipTactic_00495f30[obj]) {
            case TACTIC_SELF_DEFENSE:
                approach_full_speed(obj);
                if (unactive(g_acShipTarget_00495f20[obj]) != 0) {
                    select_target(obj);
                    if (unactive(g_acShipTarget_00495f20[obj]) != 0)
                        reset_tactic(obj, TACTIC_NONE);
                } else {
                    fire_turrets(obj);
                }
                break;
            default:
                if (g_nTargetShip_004931a0 != -1) {
                    approach_full_speed(obj);
                    g_asShipTactic_00495f30[obj] =
                        TACTIC_SELF_DEFENSE;
                    g_acShipTarget_00495f20[obj] =
                        (signed char)g_nTargetShip_004931a0;
                    fire_turrets(obj);
                } else {
                    approach_cruise_speed(obj);
                }
                break;
            }
        }
        break;
    }
}

/* Function start: 0x445D9A */
void futurion_intelligence(short obj)
{
    get_facing_range_from_object(0, obj);
    g_asActionCount_004955e8[obj]++;
    if (g_nTargetRange_0049319c > 1000 &&
        g_asActionCount_004955e8[obj] > 1000) {
        g_aeObjectClass_00495328[obj] =
            (enum ObjectClass)g_asObjectCounter_00494be0[obj];
        return;
    }
    if (g_asActionCount_004955e8[obj] > 200 &&
        g_nTargetRange_0049319c < 4000 &&
        g_nTargetRange_0049319c > 1000 &&
        g_nFacingToTarget_00493194 > 80)
        g_aeObjectClass_00495328[obj] =
            (enum ObjectClass)g_asObjectCounter_00494be0[obj];
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
        if (g_aeObjectClass_00495328[other] != OBJECT_CLASS_NULL &&
            g_asObjectType_00495298[other] == OBJECT_DATA_CHAFF_POD &&
            distance_from_object(obj, other) < 3000) {
            g_acShipTarget_00495f20[obj] = (signed char)other;
            chaffFound = 1;
            break;
        }
    }
    if (chaffFound != 0) {
        point_ship_at_object(obj, other);
        g_anShipSpeed_00494e20[obj] =
            (get_ship_max_velocity(obj) + 10) << 8;
    } else if (g_nFacingToTarget_00493194 < 0 ||
               g_acShipTarget_00495f20[obj] == -1) {
        g_acShipTarget_00495f20[obj] = -1;
        g_cViableTargetCount_00496178 = 0;
        for (other = 0; other <= 9; other++) {
            if (other != obj &&
                g_aeObjectClass_00495328[other] >= OBJECT_CLASS_SHIP) {
                get_facing_range_from_object(obj, other);
                if (g_nTargetRange_0049319c < 9000 &&
                    g_nFacingToTarget_00493194 > 0 &&
                    g_nTargetFacing_00493198 < 0) {
                    g_acViableTarget_00496180[
                        g_cViableTargetCount_00496178] = (signed char)other;
                    g_asViableTargetDistance_00496190[
                        g_cViableTargetCount_00496178] =
                            g_nTargetRange_0049319c;
                    g_cViableTargetCount_00496178++;
                }
            }
        }
        SortViableTargetsByDistance();
        if (g_cViableTargetCount_00496178 > 0) {
            for (hottestHeat = 3; hottestHeat > 0; hottestHeat--) {
                for (hottest = 0;
                     hottest < g_cViableTargetCount_00496178;
                     hottest++) {
                    if (g_aeObjectClass_00495328[
                            g_acViableTarget_00496180[hottest]] ==
                            OBJECT_CLASS_CAPITAL_SHIP ||
                        g_acShipExhaustHeat_00495660[
                            g_acViableTarget_00496180[hottest]] == hottestHeat) {
                        g_acShipTarget_00495f20[obj] =
                            g_acViableTarget_00496180[hottest];
                        hottestHeat = 0;
                        break;
                    }
                }
            }
        }
        if (g_acShipTarget_00495f20[obj] == -1)
            explode(obj, obj);
    } else {
        point_ship(obj, 0, &g_vToTarget_00493188);
        g_anShipSpeed_00494e20[obj] =
            (get_ship_max_velocity(obj) + 10) << 8;
    }
}

void point_ship(short obj, short turnRate, const FixedVector *direction);
void point_ship_at_object(short obj, short other);
short explode(short attacker, short victim);
short distance_from_object(short obj, short other);
void send_message(short obj, signed char message);
short get_ship_max_velocity(short obj);
short RandomBelowOrEqual(short n);
void mine_intelligence(short obj);

/* Function start: 0x446133 */
void FF_missile_intelligence(short obj)
{
    short chaffFound;
    short other;

    chaffFound = 0;
    if (g_asShipTactic_00495f30[obj] == TACTIC_RAM) {
        for (other = 0; other < 10; other++) {
            if (g_aeObjectClass_00495328[other] != OBJECT_CLASS_NULL &&
                g_asObjectType_00495298[other] ==
                    OBJECT_DATA_CHAFF_POD &&
                g_asShipSide_004955d0[
                    g_acObjectOwner_00495208[other]] !=
                    g_asShipSide_004955d0[
                        g_acObjectOwner_00495208[obj]] &&
                distance_from_object(obj, other) < 3000) {
                g_acShipTarget_00495f20[obj] = (signed char)other;
                chaffFound = 1;
                break;
            }
        }
        if (chaffFound != 0) {
            point_ship_at_object(obj, other);
            g_anShipSpeed_00494e20[obj] =
                (get_ship_max_velocity(obj) + 10) << 8;
        } else if (g_acShipTarget_00495f20[obj] == -1) {
            g_cViableTargetCount_00496178 = 0;
            for (other = 0; other <= 9; other++) {
                if (obj != other &&
                    g_aeObjectClass_00495328[other] >= OBJECT_CLASS_SHIP &&
                    (g_asShipSide_004955d0[
                         g_acObjectOwner_00495208[obj]] !=
                         g_asShipSide_004955d0[other] ||
                     g_acShipDestroyedWeaponCount_00495680[other] == -1)) {
                    g_nTargetRange_0049319c =
                        distance_from_object(obj, other);
                    if (g_nTargetRange_0049319c < 9000) {
                        g_acViableTarget_00496180[
                            g_cViableTargetCount_00496178] =
                                (signed char)other;
                        g_asViableTargetDistance_00496190[
                            g_cViableTargetCount_00496178] =
                                g_nTargetRange_0049319c;
                        g_cViableTargetCount_00496178++;
                    }
                }
            }
            SortViableTargetsByDistance();
            if (g_cViableTargetCount_00496178 > 0)
                g_acShipTarget_00495f20[obj] =
                    g_acViableTarget_00496180[0];
        } else {
            point_ship(obj, 0, &g_vToTarget_00493188);
            g_anShipSpeed_00494e20[obj] =
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
        if (g_aeObjectClass_00495328[other] == OBJECT_CLASS_MISSILE &&
            g_asObjectType_00495298[other] != OBJECT_DATA_CHAFF_POD) {
            distance = distance_from_object(obj, other);
            if (distance < 3000) {
                switch (g_asObjectType_00495298[other]) {
                case OBJECT_DATA_SPICULUM_IMAGE_RECOGNITION_MISSILE:
                    explode(obj, other);
                    break;
                case OBJECT_DATA_JAVELIN_HEAT_SEEKING_MISSILE:
                case OBJECT_DATA_PILUM_FRIEND_OR_FOE_MISSILE:
                    g_acShipTarget_00495f20[other] = (signed char)obj;
                    break;
                }
            }
        }
    }
}

/* Function start: 0x446474 */
void mine_intelligence(short obj)
{
    short other;
    short range;

    if (g_asObjectCounter_00494be0[obj] != -1)
        return;
    for (other = 0; other <= 9; other++) {
        if (other != obj &&
            g_aeObjectClass_00495328[other] >= OBJECT_CLASS_SHIP) {
            range = distance_from_object(obj, other);
            if (range < g_aObjectTypeData_00496d30[
                            g_acObjectType_00493980[obj]].collisionRadius ||
                (range < 50 && RandomBelowOrEqual(7) == 0)) {
                explode(obj, obj);
                return;
            }
        }
    }
}
