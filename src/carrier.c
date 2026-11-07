/*
 *  Carrier fighter launches.
 *
 *  Retail object 0x4564F0-0x4568DF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x4564F0 */
short has_opposing_ship(short side)
{
    short obj;

    for (obj = 0; obj < 10; obj++) {
        if (g_aeObjectClass[obj] >= OBJECT_CLASS_SHIP &&
            g_asShipSide[obj] != side)
            return 1;
    }
    return 0;
}

/* Function start: 0x45655D */
void launch_carrier_fighter(short carrier)
{
    short source;
    short fighter;

    if (has_opposing_ship(g_asShipSide[carrier]) == 0)
        return;

    switch (g_asObjectType[carrier]) {
    case 0x35:
        g_nCarrierLaunchIntervalValue = 300;
        break;
    case 0x38:
        g_nCarrierLaunchIntervalValue = 250;
        break;
    case 0x39:
        g_nCarrierLaunchIntervalValue = 150;
        break;
    case 0x3A:
        g_nCarrierLaunchIntervalValue = 100;
        break;
    case 0x3B:
        g_nCarrierLaunchIntervalValue = 75;
        break;
    default:
        return;
    }

    if (g_2_asCarrierLaunchTimer[carrier]++ <
        g_nCarrierLaunchIntervalValue)
        return;
    g_2_asCarrierLaunchTimer[carrier] = 0;

    for (source = 0; source < 10; source++) {
        if (g_aeObjectClass[source] == OBJECT_CLASS_SHIP &&
            g_asShipSide[source] ==
                g_asShipSide[carrier])
            break;
    }
    if (source == 10)
        return;
    if (g_aObjectTypeData[
            g_acObjectType[source]].resourceType == 10)
        return;

    fighter = initialize_ship(
        g_aObjectTypeData[
            g_acObjectType[source]].resourceType,
        -1, 1);
    if (fighter == -1)
        return;

    Set_up_ship_info(fighter, g_asShipMissionIndex[source],
                     (signed char)g_nCurrentNavPoint);
    g_asShipMissionType[fighter] = MISSION_TYPE_PATROL;
    g_acShipTarget[fighter] = 0;
    g_aeShipObjective[fighter] = OBJECTIVE_ENGAGE_ENEMY;
    g_asShipWingLeader[fighter] = -1;
    g_abShipEjectionSequenceEnabled[fighter] = 0;
    g_apShipMissionRecord[fighter] =
        (MissionShipRecord *)&g_aObjectTypeData[
            g_acObjectType[fighter]];
    if (g_asPilotLevel[fighter] >= 5)
        g_asPilotLevel[fighter] = 4;

    if (g_asShipSide[fighter] == SIDE_KILRATHI) {
        g_acShipPortrait[fighter] = 0x1A;
    } else if (RandomBelowOrEqual(1) != 0) {
        g_acShipPortrait[fighter] = 0x14;
    } else {
        g_acShipPortrait[fighter] = 0x15;
    }

    position_relative(
        &g_aShipPosition[fighter],
        g_aShipForwardVector[carrier],
        (short)(g_asObjectCollisionRadius[carrier] + 500));
    g_anShipSpeed[fighter] =
        (int)g_aObjectTypeData[
            g_acObjectType[source]].maximumVelocity << 8;
    ScaleFixedVector(&g_aShipForwardVector[carrier],
                     g_anShipSpeed[fighter],
                     &g_aShipVelocity[fighter]);
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short g_nCarrierLaunchIntervalValue; /* 0x005d108c */
short g_2_asCarrierLaunchTimer[10]; /* 0x005d1090 */
