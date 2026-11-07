/*
 *  Flight maneuvers.
 *
 *  Retail object 0x440490-0x44276F (link order; see docs/ORDER.md).
 */
/* This unit owns the fl storage declared in globals.h (as statics). */
#define FL_C
#include "game.h"

unsigned char g_abManeuverRerollChance[50] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 3, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0,
    0, 0, 0
}; /* 0x0049b538 */
/* perform_maneuver (0x4426AF) dispatches through this table; its guard admits
 * maneuvers 0 through 0x31, and "Maneuver error %d" follows at 0x49B638. */
void (*g_apManeuverHandlers[50])(short, short) = {
    /* 00 */ (void (*)(short, short))Mnone,
    /* 01 */ (void (*)(short, short))Mnone,
    /* 02 */ (void (*)(short, short))Mveer_away,
    /* 03 */ (void (*)(short, short))ShipAiState3,
    /* 04 */ (void (*)(short, short))Mfull_ahead,
    /* 05 */ (void (*)(short, short))Mthink,
    /* 06 */ (void (*)(short, short))Mram_missile,
    /* 07 */ (void (*)(short, short))Mturn_n_kick,
    /* 08 */ (void (*)(short, short))Mtight_loop,
    /* 09 */ (void (*)(short, short))Mhard_break,
    /* 10 */ (void (*)(short, short))Msit_n_spin,
    /* 11 */ (void (*)(short, short))Mturn_n_spin,
    /* 12 */ (void (*)(short, short))Mburnout,
    /* 13 */ (void (*)(short, short))Mwabble,
    /* 14 */ (void (*)(short, short))Mroll_over,
    /* 15 */ (void (*)(short, short))Mhard_turn,
    /* 16 */ (void (*)(short, short))Mfish_hook,
    /* 17 */ (void (*)(short, short))Msplit_left,
    /* 18 */ (void (*)(short, short))Msit_n_fire,
    /* 19 */ (void (*)(short, short))Mkickit,
    /* 20 */ (void (*)(short, short))Mturn_n_kick,
    /* 21 */ (void (*)(short, short))Mrout_me,
    /* 22 */ (void (*)(short, short))Mdrop_a_mine,
    /* 23 */ (void (*)(short, short))Msplit_right,
    /* 24 */ (void (*)(short, short))Mzig_zag,
    /* 25 */ (void (*)(short, short))Mgloat,
    /* 26 */ (void (*)(short, short))Mtail_fire,
    /* 27 */ (void (*)(short, short))Mtarget_laser,
    /* 28 */ (void (*)(short, short))ShipAiState28,
    /* 29 */ (void (*)(short, short))Mstrafe_enemy,
    /* 30 */ (void (*)(short, short))Mbest_strafe,
    /* 31 */ (void (*)(short, short))Mstrafe_n_roll,
    /* 32 */ (void (*)(short, short))Mkill_missile,
    /* 33 */ (void (*)(short, short))Msuicide_run,
    /* 34 */ (void (*)(short, short))Mzig_zag_pitch,
    /* 35 */ (void (*)(short, short))ShipAiState35,
    /* 36 */ (void (*)(short, short))Mtarget_missile,
    /* 37 */ (void (*)(short, short))Mget_distance,
    /* 38 */ (void (*)(short, short))Mcorkscrew,
    /* 39 */ (void (*)(short, short))Mtry2tail,
    /* 40 */ (void (*)(short, short))Mzip_past,
    /* 41 */ (void (*)(short, short))Mbuzz_debris,
    /* 42 */ (void (*)(short, short))Mline_up_drop,
    /* 43 */ (void (*)(short, short))Mchill,
    /* 44 */ (void (*)(short, short))ShipAiState44,
    /* 45 */ (void (*)(short, short))ShipAiState45,
    /* 46 */ (void (*)(short, short))ShipAiState46,
    /* 47 */ (void (*)(short, short))ShipAiState47,
    /* 48 */ (void (*)(short, short))Mbest_strafe,
    /* 49 */ (void (*)(short, short))Mreset
};

/* WC2 indexes the reroll table with the ship's current manoeuvre without
 * checking it, and MANEUVER_NONE is -1.  The byte in front of the table is
 * zero in the retail image, so the port answers zero rather than reproduce
 * the overrun. */
#ifdef SDL_PORT
#define MANEUVER_REROLL_CHANCE(m) \
    ((m) != MANEUVER_NONE ? g_abManeuverRerollChance[m] : 0)
#else
#define MANEUVER_REROLL_CHANCE(m) g_abManeuverRerollChance[m]
#endif

/* WC2 adds the target's collision radius before it has checked the target, so
 * with nothing targeted it reads the word at 0x4950e6, in the zero-filled
 * alignment gap before the radius table.  Make that result explicit on the
 * native port rather than cross a global's sanitizer redzone. */
#ifdef SDL_PORT
#define COLLISION_RADIUS_OF(obj) \
    ((obj) != -1 ? g_asObjectCollisionRadius[obj] : 0)
#else
#define COLLISION_RADIUS_OF(obj) g_asObjectCollisionRadius[obj]
#endif

/* Function start: 0x440490 */
void HandleUnsupportedManeuver(short maneuver, short ship, short target)
{
    if (g_nOriginDevUnlock != 0)
        printf("Maneuver error %d", maneuver);
    Mzip_past(ship, target);
}

/* Function start: 0x4404CC */
void BeginShipCloak(short ship)
{
    if (g_anShipCloakState[ship] != 1) {
        g_anShipCloakState[ship] = 1;
        recalc_max_velocity(ship);
        g_asShipCloakElapsedFrames[ship] = 0;
        if (g_asObjectScreenX[ship] != -0x7fff &&
            g_nCannedSceneMode == 0)
            PlaySfxWaveFileByNumber(44, ship, 0);
    }
}

/* Function start: 0x440549 */
void ResetShipCloakState(short ship)
{
    g_anShipCloakState[ship] = 0;
    g_asShipCloakCooldown[ship] = 40;
}

/* Function start: 0x440571 */
void chase_speed(short obj, short range)
{
    if (range < g_nTargetRange) {
        approach_full_speed(obj);
        if (range * 2 < g_nTargetRange)
            fire_afterburner(obj, 2);
    } else if (range > g_nTargetRange) {
        approach_zero_speed(obj);
    } else {
        approach_speed(obj,
                       g_anShipSpeed[
                           g_acShipTarget[obj]]);
    }
}

/* Function start: 0x440606 */
void SetShipAiScratchWord(unsigned short v)
{
    g_nShipAiScratchValue = v;
}

/* Function start: 0x44061B */
void maneuver_complete(short ship)
{
    set_special(ship, SPECIAL_MANEUVER_NONE);
    reset_maneuver(ship, MANEUVER_NONE);
}

/* Function start: 0x440642 */
void Mline_up_drop(short ship, short target)
{
    /* The shipped comparison normalizes this test to 0/1, then compares it
       with 9.  We therefore believe this completion path is unreachable. */
    if ((g_aeSpecialManeuver[target] ==
         SPECIAL_MANEUVER_NORMAL) == 9) {
        maneuver_complete(ship);
        return;
    }
    if (CanSetNewShipTurnGoal(ship) != 0) {
        point_ship_at_point(ship, &g_aShipPosition[target]);
        g_anRollGoal[ship] = 360;
    }
}

/* Function start: 0x4406BA */
void Mwabble(short ship)
{
    g_asShipCount[ship]++;
    if (g_asShipCount[ship] > 20) {
        maneuver_complete(ship);
        return;
    }

    fire_afterburner(ship, 20);
    if (CanSetNewShipTurnGoal(ship) != 0) {
        switch (RandomBelowOrEqual(2)) {
        case 0:
            g_anYawGoal[ship] = signed_random(6) * 5;
            break;
        case 1:
            g_anPitchGoal[ship] = signed_random(6) * 5;
            break;
        default:
            g_anRollGoal[ship] = signed_random(10) * 5;
        }
    }
}

/* Function start: 0x4407B0 */
void advance(short ship)
{
    if (g_asShipManeuver[ship] != MANEUVER_NONE)
        g_acShipSequence[ship]++;
}

/* Function start: 0x4407DA */
void ShipAiState35(short ship, short target)
{
    HandleUnsupportedManeuver(1, ship, target);

}

/* Function start: 0x4407F7 */
void Mfull_ahead(short ship, short target)
{
    HandleUnsupportedManeuver(2, ship, target);

}

/* Function start: 0x440814 */
void Mchill(short ship, short target)
{
    HandleUnsupportedManeuver(3, ship, target);

}

/* Function start: 0x440831 */
void Mdrop_a_mine(short ship, short target)
{
    HandleUnsupportedManeuver(4, ship, target);

}

/* Function start: 0x44084E */
void Mthink(short ship, short target)
{
    HandleUnsupportedManeuver(5, ship, target);

}

/* Function start: 0x44086B */
void Mtight_loop(short ship)
{
    approach_cruise_speed(ship);
    switch (g_acShipSequence[ship]) {
    case 0:
        g_anPitchGoal[ship] = 180;
        advance(ship);
        return;
    case 1:
        if (no_goal(ship) != 0)
            advance(ship);
        approach_cruise_speed(ship);
        break;
    case 2:
        g_anPitchGoal[ship] = 180;
        advance(ship);
        return;
    case 3:
        if (no_goal(ship) != 0)
            maneuver_complete(ship);
        break;
    default:
        maneuver_complete(ship);
    }
}

/* Function start: 0x440967 */
void Mhard_break(short ship)
{
    short advanceSequence;

    advanceSequence = 1;
    switch (g_acShipSequence[ship]) {
    case 0:
        set_special(ship, SPECIAL_MANEUVER_KILL_ENGINES);
        break;
    case 1:
        advanceSequence = ++g_asShipCount[ship] > 3;
        if (advanceSequence != 0)
            g_asShipCount[ship] = 0;
        break;
    case 2:
        fire_super_brake(ship);
        break;
    case 3:
        advanceSequence = ++g_asShipCount[ship] > 3;
        if (advanceSequence != 0)
            maneuver_complete(ship);
        break;
    }
    if (advanceSequence != 0)
        advance(ship);
}

#pragma function(labs)
/* Function start: 0x440A93 */
void Msit_n_spin(short ship, short target)
{
    short advanceSequence;
    FixedVector aimPoint;

    advanceSequence = 1;
    switch (g_acShipSequence[ship]) {
    case 0:
        if (++g_asShipCount[ship] < 4) {
            approach_speed(ship, g_anShipSpeed[target]);
            ScaleFixedVector(&g_aShipForwardVector[target],
                             g_nTargetRange * 2, &aimPoint);
            AddFixedVectors(&g_aShipPosition[target],
                            &aimPoint, &aimPoint);
            point_ship_at_point(ship, &aimPoint);
            advanceSequence = 0;
        } else {
            g_asShipCount[ship] = 0;
        }
        break;
    case 1:
        steady_object(ship);
        break;
    case 2:
        break;
    case 3:
        if (labs(g_anShipSpeed[ship] -
                g_anShipSpeed[target]) < 0x200) {
            approach_speed(ship, g_anShipSpeed[target]);
            advanceSequence = 0;
        }
        break;
    case 4:
        set_special(ship, SPECIAL_MANEUVER_KILL_ENGINES);
        break;
    case 5:
        steady_object(ship);
        point_ship_at_object(ship, target);
        break;
    case 6:
        advanceSequence = no_goal(ship);
        break;
    case 7:
        if (g_nFacingToTarget > 85)
            fire(ship, target);
        if (++g_asShipCount[ship] <= 5)
            advanceSequence = 0;
        break;
    case 8:
        veer_random(ship, 35);
        break;
    case 9:
        advanceSequence = no_goal(ship);
        break;
    case 10:
        set_special(ship, SPECIAL_MANEUVER_NONE);
        reset_maneuver(ship, MANEUVER_ROLL_OVER);
        break;
    }
    if (advanceSequence != 0)
        advance(ship);
}

/* Function start: 0x440D0B */
void Mturn_n_spin(short ship, short target)
{
    short advanceSequence;

    advanceSequence = 1;
    switch (g_acShipSequence[ship]) {
    case 0:
        veer_random(ship, 90);
        break;
    case 1:
        ++g_asShipCount[ship];
        advanceSequence = g_asShipCount[ship] > 2;
        break;
    case 2:
        advanceSequence = g_nTargetFacing <= 80;
        if (advanceSequence == 0) {
            maneuver_complete(ship);
        } else {
            set_special(ship, SPECIAL_MANEUVER_KILL_ENGINES);
            point_ship_at_object(ship, target);
        }
        break;
    case 3:
        if (no_goal(ship) != 0)
            maneuver_complete(ship);
        break;
    }
    if (advanceSequence != 0)
        advance(ship);
}

/* Function start: 0x440E46 */
void Mburnout(short ship, short target)
{
    (void)target;

    switch (g_acShipSequence[ship]) {
    case 0:
        fire_afterburner(ship, 10);
        advance(ship);
        break;
    case 1:
        g_anYawGoal[ship] =
            (short)((RandomBelowOrEqual(2) * 2 - 1) * 90);
        advance(ship);
        break;
    default:
        if (no_goal(ship) != 0)
            maneuver_complete(ship);
        break;
    }
}

/* Function start: 0x440F01 */
void Mkickit(short ship)
{
    switch (g_acShipSequence[ship]) {
    case 0:
        fire_afterburner(ship, 10);
        advance(ship);
        break;
    default:
        if (g_aeSpecialManeuver[ship] ==
            SPECIAL_MANEUVER_NONE)
            maneuver_complete(ship);
    }
}

/* Function start: 0x440F76 */
void Mturn_n_kick(short ship)
{
    short pointed;
    short obj;

    pointed = 0;
    switch (g_acShipSequence[ship]) {
    case 0:
        for (obj = 0; obj < 10; obj++) {
            if (g_aeObjectClass[obj] == OBJECT_CLASS_MISSILE &&
                g_acShipTarget[obj] == ship) {
                point_perpendicular(ship, obj);
                pointed = 1;
                break;
            }
        }
        if (pointed == 0)
            veer_random(ship, 90);
        advance(ship);
        break;
    case 1:
        if (no_goal(ship) != 0) {
            fire_afterburner(ship, 10);
            advance(ship);
        } else {
            approach_full_speed(ship);
        }
        break;
    case 2:
        if (g_aeSpecialManeuver[ship] == SPECIAL_MANEUVER_NONE)
            maneuver_complete(ship);
    }
}

/* Function start: 0x4410C0 */
void Mroll_over(short ship)
{
    if (g_acShipSequence[ship] == 0) {
        advance(ship);
        g_anRollGoal[ship] =
            (RandomBelowOrEqual(2) * 2 - 1) * 180;
    } else if (g_anRollGoal[ship] == 0) {
        maneuver_complete(ship);
    } else {
        approach_full_speed(ship);
    }
}

/* Function start: 0x441146 */
void Mhard_turn(short ship)
{
    approach_full_speed(ship);
    switch (g_acShipSequence[ship]) {
    case 0:
        g_anYawGoal[ship] =
            (RandomBelowOrEqual(2) * 2 - 1) * 180;
        advance(ship);
        break;
    default:
        if (no_goal(ship) != 0)
            maneuver_complete(ship);
    }
}

/* Function start: 0x4411E1 */
void Mfish_hook(short ship, short target)
{
    short advanceSequence = 1;

    (void)target;
    switch (g_acShipSequence[ship]) {
    case 0:
        g_anYawGoal[ship] =
            (RandomBelowOrEqual(2) * 2 - 1) * 120;
        fire_afterburner(ship, 5);
        break;
    case 1:
        advanceSequence = no_goal(ship);
        approach_full_speed(ship);
        break;
    case 2:
        fire_super_brake(ship);
        g_anYawGoal[ship] =
            (RandomBelowOrEqual(2) * 2 - 1) * 45;
        break;
    case 3:
        advanceSequence =
            g_aObjectTypeData[
                g_acObjectType[ship]].cruiseVelocity >=
            real_velocity(ship);
        break;
    case 4:
        advanceSequence = no_goal(ship);
        break;
    case 5:
        fire_afterburner(ship, 10);
        break;
    default:
        if (normal_speed(ship) != 0)
            maneuver_complete(ship);
    }
    if (advanceSequence != 0)
        advance(ship);
}

/* Function start: 0x441379 */
void Mtry2tail(short ship, short target)
{
    FixedVector position;

    if (unactive(target) == 0) {
        position = g_aShipPosition[target];
        position_relative(&position, g_aShipRightVector[target],
                          (short)-(g_asObjectCollisionRadius[target] +
                                   1000));
        point_ship_at_point(ship, &position);
        fire_afterburner(ship, 10);
        if (g_aeSpecialManeuver[ship] == SPECIAL_MANEUVER_NONE ||
            g_nTargetRange < 0x1f40)
            maneuver_complete(ship);
    } else {
        maneuver_complete(ship);
    }
}

/* Function start: 0x44145A */
void Msplit_left(short ship)
{
    approach_full_speed(ship);
    switch (g_acShipSequence[ship]) {
    case 0:
        g_anYawGoal[ship] = 90;
        advance(ship);
        break;
    case 1:
        fire_afterburner(ship, 10);
        advance(ship);
        /* The retail switch falls through into the completion test. */
    default:
        if (no_goal(ship) != 0 &&
            g_aeSpecialManeuver[ship] == SPECIAL_MANEUVER_NONE)
            maneuver_complete(ship);
        break;
    }
}

/* Function start: 0x44151B */
void Msplit_right(short ship)
{
    approach_full_speed(ship);
    switch (g_acShipSequence[ship]) {
    case 0:
        g_anYawGoal[ship] = -90;
        advance(ship);
        break;
    case 1:
        fire_afterburner(ship, 10);
        advance(ship);
        /* The retail switch falls through into the completion test. */
    default:
        if (no_goal(ship) != 0 &&
            g_aeSpecialManeuver[ship] == SPECIAL_MANEUVER_NONE)
            maneuver_complete(ship);
        break;
    }
}

/* Function start: 0x4415DC */
void Mgloat(short ship)
{
    switch (g_acShipSequence[ship]) {
    case 0:
        set_special(ship, SPECIAL_MANEUVER_KILL_ENGINES);
        advance(ship);
        break;
    case 1:
        g_anPitchGoal[ship] = 15;
        if (RandomBelow(100) < 50)
            advance(ship);
        break;
    default:
        g_anPitchGoal[ship] = -30;
        if (RandomBelow(100) < 50) {
            if (++g_asShipCount[ship] < 10)
                g_acShipSequence[ship] = 1;
            else
                maneuver_complete(ship);
        }
        break;
    }
}

/* Function start: 0x4416D5 */
void Mtail_fire(short ship, short target)
{
    if (CanSetNewShipTurnGoal(ship) != 0)
        point_ship_at_object(ship, target);
    chase_speed(ship,
        (short)(g_asObjectCollisionRadius[ship] * 6 +
                g_asObjectCollisionRadius[target]));
    fire(ship, target);
}

/* Function start: 0x441742 */
void Mzip_past(short ship, short target)
{
    if (unactive(target) == 0) {
        if (close_behind(
                g_asObjectCollisionRadius[target] + 2000) != 0) {
            Mtail_fire(ship, target);
        } else {
            approach_full_speed(ship);
            if (g_nTargetRange > 3000 &&
                g_nFacingToTarget > 75)
                fire_afterburner(ship, 3);
            if (CanSetNewShipTurnGoal(ship) != 0) {
                if (g_nTargetFacing > 80)
                    point_ship_below_object(ship, target);
                else
                    point_ship_behind_object(ship, target);
            }
        }
    } else {
        maneuver_complete(ship);
    }
}

/* Function start: 0x441837 */
/* Dispatch slot 47: cloak and break off, or zip past when the cloak is
 * unavailable. */
void ShipAiState47(short ship, short target)
{
    if (g_asShipCloakCooldown[ship] == 0 &&
        g_anShipCloakState[ship] == 0) {
        maneuver_complete(ship);
        BeginShipCloak(ship);
    } else {
        Mzip_past(ship, target);
    }
}

/* Function start: 0x441895 */
/* Dispatch slot 28: unimplemented in WC2. */
void ShipAiState28(short ship, short target)
{
    HandleUnsupportedManeuver(6, ship, target);

}

/* Function start: 0x4418B2 */
/* Dispatch slot 46: pick a random roll and yaw, burn out of the fight, then
 * wait for the turn to finish. */
void ShipAiState46(short ship)
{
    switch (g_acShipSequence[ship]) {
    case 0:
        g_anRollGoal[ship] =
            (short)((2 * RandomBelowOrEqual(2) - 1) * 180);
        g_anYawGoal[ship] =
            (short)((2 * RandomBelowOrEqual(2) - 1) * 60);
        approach_full_speed(ship);
        fire_afterburner(ship, 10);
        advance(ship);
        break;
    default:
        approach_full_speed(ship);
        if (no_goal(ship) != 0) {
            g_asProjectileEvasionTicks[ship] = 0;
            maneuver_complete(ship);
        }
    }
}

/* Function start: 0x44199B */
void Mram_missile(short ship, short target)
{
    approach_full_speed(ship);
    if (CanSetNewShipTurnGoal(ship) != 0)
        point_ship_at_object(ship, target);
    if (g_nFacingToTarget > 75 &&
        g_nTargetRange < 6000) {
        fire_missile(ship);
        maneuver_complete(ship);
    }
}

/* Function start: 0x441A13 */
void Mbuzz_debris(short ship)
{
    switch (g_acShipSequence[ship]) {
    case 0:
        veer_random(ship, 10);
        advance(ship);
        break;
    case 1:
        if (CanSetNewShipTurnGoal(ship) != 0) {
            fire_afterburner(ship, 10);
            advance(ship);
        } else {
            approach_full_speed(ship);
        }
        break;
    default:
        if (g_aeSpecialManeuver[ship] ==
            SPECIAL_MANEUVER_NONE) {
            maneuver_complete(ship);
        } else if (g_nFacingToTarget > 95) {
            veer_random(ship, 10);
        }
    }
}

/* Function start: 0x441AFC */
void Mstrafe_enemy(short ship, short target)
{
    point_ship_at_object(ship, target);
    if (g_asObjectType[ship] == 0x33 &&
        (g_asShipMissionType[ship] == 2 ||
         g_aeObjectClass[target] >= 0xd)) {
        if (g_nTargetRange < 3000)
            approach_zero_speed(ship);
        else
            approach_cruise_speed(ship);
    }
    if (g_nTargetRange > 5000)
        fire_afterburner(ship, 10);
    fire(ship, target);
    maneuver_complete(ship);
}

/* Function start: 0x441BC1 */
void Mbest_strafe(short ship, short target)
{
    if (g_nTargetFacing < 0x50) {
        Mstrafe_enemy(ship, target);
        return;
    }
    Mzip_past(ship, target);
}

/* Function start: 0x441C01 */
void Mtarget_missile(short ship, short target)
{

    switch (g_acShipSequence[ship]) {
    case 0:
        point_ship_at_object(ship, target);
        advance(ship);
        break;
    case 1:
        if (CanSetNewShipTurnGoal(ship) != 0) {
            fire_afterburner(ship, 10);
            advance(ship);
        }
        break;
    case 2:
        Mstrafe_enemy(ship, target);
        advance(ship);
        break;
    case 3:
        if (no_goal(ship) != 0 &&
            g_aeSpecialManeuver[ship] != -1)
            maneuver_complete(ship);
        break;
    }

}

/* Function start: 0x441D00 */
void Msit_n_fire(short ship, short target)
{
    if (CanSetNewShipTurnGoal(ship) != 0)
        point_ship_at_object(ship, target);
    if (g_nTargetRange > 3000)
        approach_cruise_speed(ship);
    else
        approach_zero_speed(ship);
    fire(ship, target);
}

/* Function start: 0x441D71 */
void Mstrafe_n_roll(short ship, short target)
{
    if (0 < g_asObjectCounter[ship]) {
        g_anRollGoal[ship] = 0x2d;
        return;
    }
    Mstrafe_enemy(ship, target);
}

/* Function start: 0x441DB3 */
void Mkill_missile(short ship, short target)
{
    if (FindMissileTargetingObject(ship) == 0) {
        maneuver_complete(ship);
        return;
    }
    get_facing_range_from_object(ship, g_nTargetShip);
    if (g_nFacingToTarget < 0) {
        reset_maneuver(ship, MANEUVER_FISH_HOOK);
        Mfish_hook(ship, target);
        return;
    }
    if (g_nFacingToTarget < 80) {
        reset_maneuver(ship, MANEUVER_BURNOUT);
        Mburnout(ship, target);
        return;
    }
    point_ship_at_object(ship, g_nTargetShip);
    if (g_nTargetRange < 8000)
        fire(ship, g_acShipTarget[ship]);
}

/* Function start: 0x441E9C */
void Msuicide_run(short ship, short target)
{
    point_ship_at_object(ship, target);
    fire_afterburner(ship, 160);
}

/* Function start: 0x441EC8 */
void Mget_distance(short ship, short target)
{
    if (g_nTargetRange > 2000) {
        maneuver_complete(ship);
        return;
    }
    if (g_nTargetRange < 700 && normal_speed(ship) != 0)
        fire_afterburner(ship, 10);
    else
        approach_full_speed(ship);
    if (g_nFacingToTarget > 0 && CanSetNewShipTurnGoal(ship) != 0)
        steer_away_from_object(ship, target,
                               MinShort(20, g_nFacingToTarget));
}

/* Function start: 0x441F86 */
void general_zig(short ship, unsigned int target, short pitch)
{
    short complete;

    (void)target;
    complete = 1;
    switch (g_acShipSequence[ship] % 6) {
    case 0:
        fire_afterburner(ship, 10);
        g_anYawGoal[ship] = -60;
        g_anPitchGoal[ship] = pitch;
        break;
    case 3:
        fire_afterburner(ship, 10);
        g_anYawGoal[ship] = 60;
        g_anPitchGoal[ship] = (short)-pitch;
        break;
    case 1:
    case 4:
        complete = no_goal(ship);
        g_asShipCount[ship] = 0;
        break;
    case 2:
    case 5:
        g_asShipCount[ship]++;
        complete = (short)(g_asShipCount[ship] >= 2);
        break;
    }
    if (g_acShipSequence[ship] >= 12 &&
        g_aeSpecialManeuver[ship] == SPECIAL_MANEUVER_NONE &&
        no_goal(ship) != 0)
        maneuver_complete(ship);
    if (complete != 0)
        advance(ship);
}

/* Function start: 0x44210C */
void Mzig_zag(short ship, unsigned int target)
{
    general_zig(ship, target, 0);
}

/* Function start: 0x442129 */
void Mzig_zag_pitch(short ship, unsigned int target)
{
    general_zig(ship, target, 0x23);
}

/* Function start: 0x442146 */
void Mcorkscrew(short ship)
{
    approach_full_speed(ship);
    if (no_goal(ship) != 0) {
        if (--g_asShipCount[ship] <= 0) {
            switch (g_acShipSequence[ship] % 4) {
            case 0:
                g_anYawGoal[ship] = -20;
                break;
            case 2:
                g_anYawGoal[ship] = 20;
                break;
            case 1:
            case 3:
                g_anRollGoal[ship] = 20;
                break;
            }
            g_asShipCount[ship] = 4;
            advance(ship);
        }
    }
    if (g_acShipSequence[ship] > 8)
        maneuver_complete(ship);
}

/* Function start: 0x442252 */
void Mveer_away(short ship, short target)
{
    switch (g_acShipSequence[ship]) {
    case 0:
        if (g_nFacingToTarget > 80)
            steer_away_from_object(ship, target, 40);
        else
            steer_away_from_object(ship, target, 10);
        advance(ship);
        break;
    default:
        if (g_asObjectCollisionRadius[target] * 3 <
            g_nTargetRange) {
            veer_random(ship, 8);
            maneuver_complete(ship);
        } else {
            if (no_goal(ship) != 0) {
                if (g_nFacingToTarget > 80) {
                    steer_away_from_object(ship, target, 40);
                } else if (g_nFacingToTarget < -65 ||
                           RandomBelowOrEqual(100) < 4) {
                    veer_random(ship, 16);
                }
            }
            if ((g_nShipAiScratchValue >= g_nTargetRange ||
                 RandomBelowOrEqual(100) < 10) &&
                normal_speed(ship) != 0)
                fire_afterburner(ship, 10);
            else
                approach_full_speed(ship);
        }
    }
}

/* Function start: 0x4423E2 */
void ShipAiState44(short ship)
{
    g_acShipStress[ship] = 0;
    maneuver_complete(ship);
}

/* Function start: 0x442404 */
/* Dispatch slot 8: unimplemented in WC2. */
void Mtarget_laser(short ship, short target)
{
    HandleUnsupportedManeuver(8, ship, target);
}

/* Function start: 0x442421 */
void Mrout_me(short ship)
{
    try2rout(ship);
}

/* Function start: 0x442438 */
/* Dispatch slot 3: unimplemented in WC2. */
void ShipAiState3(short ship, short target)
{
    HandleUnsupportedManeuver(9, ship, target);

}

/* Function start: 0x442455 */
/* Dispatch slot 45: close on the target at full speed and afterburn once it
 * is roughly ahead. */
void ShipAiState45(short ship, short target)
{
    if (unactive(target) == 0) {
        approach_full_speed(ship);
        point_ship_at_object(ship, target);
        if (g_nFacingToTarget > 80)
            fire_afterburner(ship, 10);
    } else {
        maneuver_complete(ship);
    }
}

/* Function start: 0x4424C2 */
/* Empty in the original: dispatch-table slots 0 and 1 (no-op / invalid state). */
void Mnone(void)
{
}

/* Function start: 0x4424CD */
void Mreset(short ship)
{
    maneuver_complete(ship);
}

/* Function start: 0x4424E4 */
void perform_maneuver(short obj)
{
    short previous;
    short range;
    short weaponSlot;
    short target;

    target = g_acShipTarget[obj];
    if (g_asIntelligenceEvent[obj] == 6) {
        weaponSlot = find_weapon(obj, OBJECT_DATA_CHAFF_POD);
        if (weaponSlot != -1)
            fire_weapon(obj, weaponSlot);
    }
    g_bAiMissileFiringEnabled = 1;

    previous = g_asShipManeuver[obj];
    g_bCurrentManeuverReroll =
        MANEUVER_REROLL_CHANCE(g_asShipManeuver[obj]);
    range = g_nTargetRange;
    if (g_nTargetFacing < 0)
        SetShipAiScratchWord(
            (unsigned short)(g_asObjectCollisionRadius[obj] * 4 +
                             COLLISION_RADIUS_OF(target)));
    else
        SetShipAiScratchWord(
            (unsigned short)(g_asObjectCollisionRadius[obj] * 6 +
                             COLLISION_RADIUS_OF(target)));

    if (unactive(target) != 0) {
        switch (g_asShipManeuver[obj]) {
        case MANEUVER_GLOAT:
            ((void (__cdecl *)(short, short))Mgloat)(obj, target);
            break;
        case MANEUVER_LINE_UP_DROP:
            Mline_up_drop(obj, target);
            break;
        case MANEUVER_VEER_AWAY:
            Mveer_away(obj, target);
            break;
        default:
            maneuver_complete(obj);
        }
    } else if (g_asShipManeuver[obj] >= 0 &&
               g_asShipManeuver[obj] <
                   MANEUVER_HANDLER_COUNT) {
        g_apManeuverHandlers[
            g_asShipManeuver[obj]](obj, target);
    } else {
        if (g_nOriginDevUnlock != 0) {
            printf("bigtime_maneuver_error %d %d %d %d", obj,
                   g_asShipManeuver[obj],
                   g_asShipIntelSlot[3],
                   g_asShipIntelSlot[obj]);
            getch();
        }
        maneuver_complete(obj);
    }

    if (g_asShipManeuver[obj] == previous &&
        RandomBelowOrEqual(100) < g_bCurrentManeuverReroll)
        maneuver_complete(obj);
}
