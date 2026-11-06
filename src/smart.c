/*
 *  Pilot steering, formation flying and maneuver selection.
 *
 *  Retail object 0x41E4E0-0x42033F (link order; see docs/ORDER.md).
 */
#include "game.h"

signed char g_aDefenseManeuversNovice[8] = {
    24, 24, 13, 14, -1, 0, 0, 0
}; /* 0x00492fd8 */
signed char g_aDefenseManeuversVeteran[8] = {
    8, 13, 15, 14, 19, 24, -1, 0
}; /* 0x00492fe0 */
signed char g_aDefenseManeuversElite[16] = {
    8, 15, 17, 23, 19, 9, 20, 24, 14, -1, 0, 0, 0, 0, 0, 0
}; /* 0x00492fe8 */
signed char g_aDefenseManeuversAce[16] = {
    17, 23, 15, 19, 9, 14, 20, 12, -1, 0, 0, 0, 0, 0, 0, 0
}; /* 0x00492ff8 */
signed char g_aDefenseManeuversBoss[16] = {
    15, 19, 12, 11, 17, 23, 7, 9, -1, 0, 0, 0, 0, 0, 0, 0
}; /* 0x00493008 */
signed char *g_apDefenseManeuvers[5] = {
    g_aDefenseManeuversNovice,
    g_aDefenseManeuversVeteran,
    g_aDefenseManeuversElite,
    g_aDefenseManeuversAce,
    g_aDefenseManeuversBoss
}; /* 0x00493018 */
signed char g_acPilotAggression[8] = {
    3, 3, 3, 2, 2, 3, 0, 0
}; /* 0x00493030 */
signed char g_acPilotRecovery[8] = {
    6, 7, 8, 8, 9, 8, 0, 0
}; /* 0x00493038 */
short g_bWingmanTurnRateUnlocked = 0; /* 0x00493040 */

#pragma function(labs)
/* Function start: 0x41E4E0 */
void steer_away_from_object(short obj, short other, short amount)
{
    FixedVector predicted;
    FixedVector gap;
    FixedVector relative;

    g_anRollGoal[obj] = 0;
    AddFixedVectors(&g_aShipPosition[other],
                    &g_aShipVelocity[other], &predicted);
    ComputeVectorDelta(&g_aShipPosition[obj],
                       &predicted, &gap);
    transform_to_objects_frame(&gap, &relative, obj);
    if (labs(relative.x) < labs(relative.y)) {
        if (relative.x > 0)
            amount = -amount;
        g_anYawGoal[obj] = amount;
    } else {
        if (relative.y > 0)
            amount = -amount;
        g_anPitchGoal[obj] = amount;
    }
}

void transform_to_objects_frame(const FixedVector *source, FixedVector *destination, short obj);
void ComputeVectorDelta(FixedVector *from, FixedVector *to, FixedVector *delta);
void try2end_collision_alert(short obj);

unsigned int offset_location(short obj, const ShortVector *offset, FixedVector *location);
unsigned int compute_formation_destination(short leader, const ShortVector *offset, FixedVector *destination);
unsigned int select_target(short obj);

/* Function start: 0x41E5C8 */
void steer_away_from_predicted_object(short obj, short other,
                                      short predictionTicks, short amount)
{
    FixedVector predicted;
    FixedVector delta;
    FixedVector relative;

    g_anRollGoal[obj] = 0;
    ScaleFixedVector(&g_aShipVelocity[other],
                     (int)predictionTicks << 8, &predicted);
    AddFixedVectors(&g_aShipPosition[other],
                    &predicted, &predicted);
    ComputeVectorDelta(&g_aShipPosition[obj],
                       &predicted, &delta);
    transform_to_objects_frame(&delta, &relative, obj);
    if (labs(relative.x) < labs(relative.y)) {
        if (relative.x < 0)
            amount = -amount;
        g_anYawGoal[obj] = amount;
    } else {
        if (relative.y > 0)
            amount = -amount;
        g_anPitchGoal[obj] = amount;
    }
}

/* Function start: 0x41E6C8 */
void prevent_collision(short obj)
{
    short timeToImpact;
    short other;

    other = g_acShipAlertSource[obj];
    timeToImpact = 0;
    if (other == -1) {
        clear_alert(obj);
        return;
    }
    timeToImpact = g_5_nCollisionCountdownNumber;
    if (timeToImpact >= 30) {
        approach_full_speed(obj);
        try2end_collision_alert(obj);
        return;
    }
    get_facing_range_from_point(obj, &g_aShipPosition[other]);
    if (g_nFacingToTarget > 75) {
        if (g_nTargetFacing < -70)
            approach_zero_speed(obj);
        else
            approach_full_speed(obj);
    } else if (g_nFacingToTarget < -70 &&
               normal_speed(obj) != 0) {
        fire_afterburner(obj, 8);
    } else {
        approach_full_speed(obj);
    }
    if (CanSetNewShipTurnGoal(obj) != 0) {
        if (g_nFacingToTarget < -60 &&
            g_nTargetFacing > 60) {
            veer_random(obj, 14);
        } else {
            steer_away_from_predicted_object(
                obj, other, (short)(timeToImpact >> 1),
                MaxShort(0, MinShort(g_nFacingToTarget, 25)));
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
        if (g_aeObjectClass[obj] == OBJECT_CLASS_SHIP &&
            g_asShipSide[obj] != g_asShipSide[0])
            count++;
    }
    return count;
}

void position_relative(FixedVector *position, FixedVector direction, short distance);
short scan_for_enemy(short obj, unsigned short range);
short attacker_in_range(short obj, unsigned short range);

/* Function start: 0x41E945 */
short regulate_turn(short obj)
{
    if (g_aeSpecialManeuver[obj] ==
        SPECIAL_MANEUVER_UNKNOWN_9)
        return 1;
    g_acTurnRegulator[obj]--;
    if (g_aeObjectClass[obj] < OBJECT_CLASS_CAPITAL_SHIP &&
        handle_collisions(obj) != 0)
        return 1;
    if ((g_nSpaceFrame & 1) != 0 &&
        obj == g_nYourWingman &&
        g_bWingmanTurnRateUnlocked == 0)
        return 1;
    if (g_aeObjectClass[obj] == OBJECT_CLASS_SHIP &&
        GetAdaptiveTurnRate() >= 120 &&
        CountEnemyFighters() < 3 &&
        g_asShipSide[obj] != g_asShipSide[0])
        g_acTurnRegulator[obj] = 0;
    if (g_acTurnRegulator[obj] > 0)
        return 1;
    g_abShipTurn[obj]++;
    g_acTurnRegulator[obj] = g_acTurnInterval[obj];
    return 0;
}

/* Function start: 0x41EA86 */
unsigned int select_target(short obj)
{
    if (g_asShipSide[obj] == SIDE_KILRATHI &&
        RandomBelow(100) < 50 && attacker_in_range(0, 16000) == 0 &&
        distance_from_object(obj, 0) < 5000)
        g_acShipTarget[obj] = 0;
    else
        g_acShipTarget[obj] =
            (signed char)scan_for_enemy(obj, 16000);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x41EB1D */
unsigned int veer_random(short obj, short amount)
{
    switch (RandomBelowOrEqual(3)) {
    case 0:
        g_anYawGoal[obj] = amount;
        break;
    case 1:
        g_anYawGoal[obj] = -amount;
        break;
    case 2:
        g_anPitchGoal[obj] = amount;
        break;
    case 3:
        g_anPitchGoal[obj] = -amount;
        break;
    }
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x41EBC1 */
unsigned int offset_location(short obj, const ShortVector *offset,
                             FixedVector *location)
{
    *location = g_aShipPosition[obj];
    position_relative(location, g_aShipForwardVector[obj],
                      offset->z);
    position_relative(location, g_aShipUpVector[obj],
                      offset->y);
    position_relative(location, g_aShipRightVector[obj],
                      offset->x);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x41EC73 */
unsigned int compute_formation_destination(short leader,
                                           const ShortVector *offset,
                                           FixedVector *destination)
{
    offset_location(leader, offset, destination);
    AddFixedVectors(destination, &g_aShipVelocity[leader],
                    destination);
    AddFixedVectors(destination, &g_aShipVelocity[leader],
                    destination);
    AddFixedVectors(destination, &g_aShipVelocity[leader],
                    destination);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x41ECEF */
void control_speed(short obj, unsigned short range, int desiredSpeed)
{
    unsigned short ticks;
    int speedError;
    int brakingMargin;
    int reachableSpeed;

    ticks = range /
        MaxShort(FixedToShortSaturating(g_anShipSpeed[obj]), 1);
    reachableSpeed = GetShipAccelerationRate(obj) * ticks;
    speedError = desiredSpeed - g_anShipSpeed[obj];
    brakingMargin = reachableSpeed - AbsInt(speedError);
    if (brakingMargin <= 0)
        approach_speed(obj, desiredSpeed);
    else if (brakingMargin < 12800)
        celerate(obj, 0x100);
    else
        celerate(obj, GetShipAccelerationRate(obj));
}

/* Function start: 0x41EDCC */
void chase_location(short obj, const FixedVector *destination,
                            short reference)
{
    short pointAtDestination;
    int speedGoal;
    short facing;
    FixedVector travel;
    FixedVector projected;

    pointAtDestination = 0;
    speedGoal = g_anShipSpeed[reference] - 0x200;
    if (speedGoal < 0)
        speedGoal = 0;
    ScaleFixedVector(&g_aShipForwardVector[reference],
                     g_anShipSpeed[reference] * 15,
                     &travel);
    AddFixedVectors(destination, &travel, &projected);
    get_facing_range_from_point(obj, &projected);
    facing = g_nFacingToTarget;
    get_facing_range_from_point(obj, destination);
    if (g_nFacingToTarget < 0) {
        approach_speed(obj, 0);
        if (g_nTargetRange /
                MaxShort(FixedToShortSaturating(
                             g_anShipSpeed[reference]), 1) > 49)
            pointAtDestination = 1;
    } else if (g_nTargetRange > 175 && CanSetNewShipTurnGoal(obj) != 0) {
        pointAtDestination = 1;
    }
    control_speed(obj, (unsigned short)g_nTargetRange,
                  speedGoal);
    if (g_nFacingToTarget > 85 &&
        g_nTargetRange > 175)
        g_anRollGoal[obj] =
            match_roll_orientation(obj, reference);
    if (pointAtDestination != 0)
        point_ship(obj, 0, &g_vToTarget);
    if (g_nTargetRange < 175) {
        if (facing > 90) {
            copy_frame(reference, obj);
            steady_object(obj);
            approach_speed(obj, g_anShipSpeed[reference]);
            return;
        } else
            point_ship(obj, 0, &travel);
    }
    if (g_nTargetRange < 600)
        trim_goals(obj, 10);
}

/* Function start: 0x41EFFE */
void goto_location(short obj, const FixedVector *destination)
{
    get_facing_range_from_point(obj, destination);
    if (g_nFacingToTarget < 51) {
        approach_half_speed(obj);
        if (CanSetNewShipTurnGoal(obj) != 0)
            point_ship_at_point(obj, destination);
    } else if (g_nTargetRange > 3000 ||
               g_nTargetRange /
                   MaxShort(FixedToShortSaturating(
                                g_anShipSpeed[obj]),
                            1) > 50) {
        approach_full_speed(obj);
    } else {
        approach_cruise_speed(obj);
    }
}

/* Function start: 0x41F0C5 */
void goto_formation(short obj, const FixedVector *destination,
                    short leader)
{
    short facingAhead;
    FixedVector projected;

    AddFixedVectors(&g_aShipPosition[obj],
                    &g_aShipForwardVector[leader], &projected);
    get_facing_range_from_point(obj, &projected);
    facingAhead = g_nFacingToTarget;
    get_facing_range_from_point(obj, destination);
    if (g_nFacingToTarget < 40) {
        approach_min_speed(obj);
    } else if (g_nTargetRange > 2000) {
        approach_full_speed(obj);
        if (g_nTargetRange > 3000 &&
            g_nFacingToTarget > 70 && normal_speed(obj) != 0)
            fire_afterburner(obj, 5);
    } else if (g_nTargetRange > 200) {
        control_speed(obj, (unsigned short)g_nTargetRange,
                      g_anShipSpeed[leader]);
    } else {
        approach_ship_speed(obj, leader);
    }
    if (g_nTargetRange < 200) {
        steady_object(obj);
        if (facingAhead > 90) {
            copy_frame(leader, obj);
            return;
        } else
            point_parallel(obj, leader);
    } else if (CanSetNewShipTurnGoal(obj) != 0) {
        point_ship(obj, 0, &g_vToTarget);
    }
    if (g_nTargetRange < 700) {
        trim_goals(obj, 10);
        if (facingAhead > 90)
            g_anRollGoal[obj] =
                match_roll_orientation(obj, leader);
    }
}

/* Function start: 0x41F2C5 */
unsigned int maintain_formation(short obj)
{
    short leader;
    FixedVector destination;

    leader = g_asShipWingLeader[obj];
    compute_formation_destination(leader,
                                  &g_aShipFormationOffset[obj],
                                  &destination);
    g_aShipDestination[obj] = destination;
    goto_formation(obj, &destination, leader);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x41F337 */
unsigned int reset_stress(short obj)
{
    short stress;
    short damage;

    if (obj >= 12) {
        if (g_acPilotAggression[g_asPilotLevel[obj]] == 0)
            damage = 100;
        else
            damage = evaluate_damage(obj);
        if (damage < 30) {
            stress = 30;
        } else if (damage < 75) {
            stress = find_ratio(30, 74, damage, 29, 15);
        } else {
            stress = find_ratio(75, 100, damage, 14, 0);
        }
        g_acShipStress[obj] = (signed char)stress;
    }
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x41F3EF */
short stress_morale(short obj)
{
    if (g_acShipStress[obj] < 15)
        return 0;
    else if (g_acShipStress[obj] < 30)
        return 1;
    else
        return 2;
}

/* Function start: 0x41F446 */
short any_defense(short obj)
{
    short count;
    short skillLevel;
    signed char *maneuvers;

    count = 0;
    skillLevel = g_asPilotLevel[obj];
    if (skillLevel > 4)
        skillLevel = 4;
    maneuvers = g_apDefenseManeuvers[skillLevel];
    while (maneuvers[count] != -1)
        count++;
    return maneuvers[RandomBelowOrEqual((short)(count - 1))];
}

/* Function start: 0x41F4C6 */
short pick_regular_maneuver(short obj, short event)
{
    short reroll;

    reroll = RandomBelowOrEqual(100) < 3 ||
             g_asShipManeuver[obj] == MANEUVER_NONE;
    if (stress_morale(obj) == 2)
        return MANEUVER_OUTA_HERE;
    if ((g_asIntelligenceEvent[obj] == event ||
         RandomBelowOrEqual(100) < 20) &&
        (event == 0 || event == 3 || event == 4 || event == 7) &&
        reroll == 0)
        return g_asShipManeuver[obj];

    switch (event) {
    case 0:
    case 1:
        if (g_aeObjectClass[
                g_acShipTarget[obj]] >=
            OBJECT_CLASS_CAPITAL_SHIP)
            return MANEUVER_STRAFE_ENEMY;
        if (g_asPilotLevel[obj] * 5 + 60 > RandomBelow(100))
            return MANEUVER_ZIP_PAST;
        return any_defense(obj);
    case 3:
        return any_defense(obj);
    case 2:
        return MANEUVER_TRY2TAIL;
    case 4:
        return RandomBelowOrEqual(100) >=
                       g_asPilotLevel[obj] * 20 + 30
                   ? any_defense(obj)
                   : MANEUVER_STRAFE_ENEMY;
    case 5:
        return MANEUVER_TAIL_FIRE;
    case 6:
        if (g_asPilotLevel[obj] >= 2)
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

short RandomBelow(short n);

/* Function start: 0x41F714 */
short pick_from_list(const ManeuverChoice *choice, short obj,
                                 short event)
{
    short maneuver;
    short chooseAgain;

    maneuver = g_asShipManeuver[obj];
    if (maneuver == MANEUVER_NONE)
        chooseAgain = 1;
    else
        chooseAgain = 0;
    if (((choice->primary != maneuver &&
          choice->secondary != maneuver &&
          RandomBelow(100) < 10) ||
         g_asIntelligenceEvent[obj] != event) &&
        choice->primary < MANEUVER_UNKNOWN_48 &&
        choice->secondary < MANEUVER_UNKNOWN_48)
        chooseAgain = 1;
    if (chooseAgain == 0) {
        if (RandomBelowOrEqual(100) < 5)
            chooseAgain = 1;
    }
    if (chooseAgain != 0) {
        if (RandomBelowOrEqual(99) >= choice->threshold)
            maneuver = choice->secondary;
        else
            maneuver = choice->primary;
    }
    return maneuver;
}

/* Function start: 0x41F83F */
short pick_kilrathi_maneuver(short obj, short event)
{
    const ManeuverChoice *choice;
    short maneuver;

    choice = &((const ManeuverChoice (*)[9][3])
                   g_aShipIntelligenceData)[
        g_asShipIntelSlot[obj]][event][stress_morale(obj)];
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
        if (g_aeObjectClass[other] == OBJECT_CLASS_SHIP &&
            g_asShipSide[other] == g_asShipSide[obj] &&
            g_acShipTarget[other] == g_acShipTarget[obj] &&
            g_asIntelligenceEvent[obj] == 5)
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

    maneuver = (short)g_asShipManeuver[obj];
    g_bWingmanTurnRateUnlocked = 0;
    if (g_asPilotLevel[obj] < 5 ||
        (g_asShipSide[obj] == 0 &&
         g_nYourWingman != obj)) {
        if (g_asShipSide[obj] == 1)
            maneuver = (short)pick_kilrathi_maneuver(obj, event);
        else
            maneuver = pick_regular_maneuver(obj, event);
    } else {
        choice = &((const ManeuverChoice (*)[9][3])
                       g_aShipIntelligenceData)[
            g_asShipIntelSlot[obj]][event][stress_morale(obj)];
        maneuver = (short)pick_from_list(choice, obj, (short)event);
    }

    if (g_asShipSide[obj] == 1 &&
        g_nFacingToTarget > 0x61 &&
        g_nTargetRange < 4000)
        fire(obj, g_acShipTarget[obj]);

    if (g_nYourWingman == obj) {
        for (index = 0; index < 8; index++) {
            if (g_13_stMissionHeaderBlock.initialMissionShips[index] ==
                g_asShipMissionIndex[obj])
                break;
        }
        if (g_4_acInitialShipKillNumber[index] <
            g_36_cPilotKillCountValue)
            g_bWingmanTurnRateUnlocked = 1;
        else
            g_bWingmanTurnRateUnlocked = 0;
    }

    if ((event == 3 || event == 7) &&
        g_nTargetRange < 2000 &&
        g_nTargetFacing > 0x5e)
        fire_afterburner(obj, 10);

    if ((g_asShipSide[obj] != g_asShipSide[0] &&
         GetAdaptiveTurnRate() > 0x78) ||
        (g_asShipSide[obj] == g_asShipSide[0] &&
         g_bWingmanTurnRateUnlocked != 0)) {
        if (g_acShipTarget[obj] != -1 && event != 4 &&
            g_aeSpecialManeuver[
                g_acShipTarget[obj]] == 1)
            fire_afterburner(obj, 10);

        if (event != 5 &&
            g_asShipCloakCooldown[obj] == 0 &&
            g_anShipCloakState[obj] == 0)
            maneuver = 0x2f;
    }

    if (g_asProjectileEvasionTicks[obj] > 0)
        maneuver = 0x2e;

    if (maneuver == 0x28 && CountAlliesOnSameTarget(obj) > 1) {
        if (event == 5)
            maneuver = 0x2e;
        else
            maneuver = 0x12;
    }
    if (g_asShipManeuver[obj] != maneuver)
        reset_maneuver(obj, maneuver);
}

/* Function start: 0x41FD2B */
void handle_stress(short obj, short event)
{
    short aggressiveness;
    short damage;

    aggressiveness =
        g_acPilotAggression[g_asPilotLevel[obj]];
    switch (event) {
    case -1:
    case 2:
        g_acShipStress[obj] -=
            g_acPilotRecovery[g_asPilotLevel[obj]];
        break;
    case 6:
        g_acShipStress[obj] += aggressiveness * 2;
        break;
    case 8:
        g_acShipStress[obj] /= 2;
        break;
    case 5:
        g_acShipStress[obj] -= aggressiveness;
        break;
    case 3:
    case 4:
    case 7:
        g_acShipStress[obj] += aggressiveness;
        break;
    case 0:
        break;
    }
    damage = evaluate_damage(obj);
    if (damage < 40) {
        g_acShipStress[obj] += aggressiveness * 2;
    } else if (damage < 75) {
        g_acShipStress[obj] =
            (signed char)MinShort(
                (short)((short)g_acShipStress[obj] + aggressiveness),
                28);
    } else {
        g_acShipStress[obj] =
            (signed char)MinShort(g_acShipStress[obj], 7);
    }
    if (event == 6 && g_acShipStress[obj] >= 30)
        g_acShipStress[obj] = 29;
    if (g_acShipStress[obj] < 0)
        g_acShipStress[obj] = 0;
}

/* The unnamed parameter preserves later C1 symbol keys after case 0. */
void handle_stress(short obj, short);
unsigned int reset_objective(short ship, short objective);
unsigned int reset_stress(short obj);

/* Function start: 0x41FF37 */
void intelligence_events(short obj)
{
    short event;
    short targetGone;
    short target;
    short previousStress;
    short damageLevel;

    event = -1;
    target = g_acShipTarget[obj];
    previousStress = (short)g_acShipStress[obj];
    targetGone = 0;
    if (unactive(target) == 0) {
        get_facing_range_from_object(obj, g_acShipTarget[obj]);
        if (g_aeSpecialManeuver[
                g_acShipTarget[obj]] ==
            SPECIAL_MANEUVER_UNKNOWN_9) {
            event = 8;
        } else {
            event = 0;
            if (g_nTargetRange > 8000) {
                event = 2;
            } else if (g_nFacingToTarget > 55 &&
                       g_nTargetFacing < -55) {
                event = 5;
            } else if (g_nFacingToTarget > 75 &&
                       g_nTargetFacing > 75 &&
                       g_nTargetRange < 5000) {
                event = 4;
                if (g_acShipCollisionCooldown[obj] > 0 &&
                    g_asObjectDamage[obj] > 0)
                    event = 7;
            } else if (FindMissileTargetingObject(obj) != 0) {
                event = 6;
            } else if (g_acShipCollisionCooldown[obj] > 0) {
                event = 7;
            } else if (g_nFacingToTarget < -70 &&
                       g_nTargetFacing > 90 &&
                       g_nTargetRange < 4000 &&
                       g_nTargetRange < 5000) {
                event = 3;
                if (g_asShipSide[obj] == SIDE_KILRATHI &&
                    GetAdaptiveTurnRate() <= 30)
                    event = 0;
            } else if (g_anShipSpeed[target] < 5120) {
                event = 1;
            }
        }
    } else if (FindMissileTargetingObject(obj) != 0) {
        event = 6;
    } else if (g_acShipCollisionCooldown[obj] > 0) {
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

    if (g_nYourWingman == obj &&
        g_aeObjectClass[0] == OBJECT_CLASS_SHIP &&
        g_acShipPendingMessage[g_nYourWingman] == -1) {
        if (previousStress < 15 && g_acShipStress[obj] >= 15 &&
            RandomBelowOrEqual(3) == 0) {
            send_message(obj, 4);
        } else {
            damageLevel = evaluate_damage(0);
            if (RandomBelow(1000) < 4 && damageLevel < 35) {
                if (evaluate_damage(obj) > damageLevel)
                    send_message(obj, 10);
                else
                    send_message(obj, 4);
            }
        }
    }
    g_asIntelligenceEvent[obj] = event;
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
unsigned char g_aShipIntelligenceData[5][0x51]; /* 0x005d3060 */
