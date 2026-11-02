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
        if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_SHIP &&
            g_asShipSide_004955d0[obj] != side)
            return 1;
    }
    return 0;
}

/* Function start: 0x45655D */
void launch_carrier_fighter(short carrier)
{
    short source;
    short fighter;

    if (has_opposing_ship(g_asShipSide_004955d0[carrier]) == 0)
        return;

    switch (g_asObjectType_00495298[carrier]) {
    case 0x35:
        g_nCarrierLaunchInterval_005d108c = 300;
        break;
    case 0x38:
        g_nCarrierLaunchInterval_005d108c = 250;
        break;
    case 0x39:
        g_nCarrierLaunchInterval_005d108c = 150;
        break;
    case 0x3A:
        g_nCarrierLaunchInterval_005d108c = 100;
        break;
    case 0x3B:
        g_nCarrierLaunchInterval_005d108c = 75;
        break;
    default:
        return;
    }

    if (g_asCarrierLaunchTimer_005d1090[carrier]++ <
        g_nCarrierLaunchInterval_005d108c)
        return;
    g_asCarrierLaunchTimer_005d1090[carrier] = 0;

    for (source = 0; source < 10; source++) {
        if (g_aeObjectClass_00495328[source] == OBJECT_CLASS_SHIP &&
            g_asShipSide_004955d0[source] ==
                g_asShipSide_004955d0[carrier])
            break;
    }
    if (source == 10)
        return;
    if (g_aObjectTypeData_00496d30[
            g_acObjectType_00493980[source]].resourceType == 10)
        return;

    fighter = initialize_ship(
        g_aObjectTypeData_00496d30[
            g_acObjectType_00493980[source]].resourceType,
        -1, 1);
    if (fighter == -1)
        return;

    Set_up_ship_info(fighter, g_asShipMissionIndex_00495d00[source],
                     (signed char)g_nCurrentNavPoint_004931bc);
    g_asShipMissionType_00495de8[fighter] = MISSION_TYPE_PATROL;
    g_acShipTarget_00495f20[fighter] = 0;
    g_aeShipObjective_00495f08[fighter] = OBJECTIVE_ENGAGE_ENEMY;
    g_asShipWingLeader_00495dd0[fighter] = -1;
    g_abShipEjectionSequenceEnabled_00496120[fighter] = 0;
    g_apShipMissionRecord_00495da8[fighter] =
        (MissionShipRecord *)&g_aObjectTypeData_00496d30[
            g_acObjectType_00493980[fighter]];
    if (g_asPilotLevel_00495d60[fighter] >= 5)
        g_asPilotLevel_00495d60[fighter] = 4;

    if (g_asShipSide_004955d0[fighter] == SIDE_KILRATHI) {
        g_acShipPortrait_00495d88[fighter] = 0x1A;
    } else if (RandomBelowOrEqual(1) != 0) {
        g_acShipPortrait_00495d88[fighter] = 0x14;
    } else {
        g_acShipPortrait_00495d88[fighter] = 0x15;
    }

    position_relative(
        &g_aShipPosition_00494550[fighter],
        g_aShipForwardVector_00494208[carrier],
        (short)(g_asObjectCollisionRadius_004950e8[carrier] + 500));
    g_anShipSpeed_00494e20[fighter] =
        (int)g_aObjectTypeData_00496d30[
            g_acObjectType_00493980[source]].maximumVelocity << 8;
    ScaleFixedVector(&g_aShipForwardVector_00494208[carrier],
                     g_anShipSpeed_00494e20[fighter],
                     &g_aShipVelocity_00494898[fighter]);
}
