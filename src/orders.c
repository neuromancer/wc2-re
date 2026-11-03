/*
 *  Wingman orders and landing requests.
 *
 *  Retail object 0x421910-0x42259F (link order; see docs/ORDER.md).
 */
#include "game.h"

short g_bEjectionSequencePending_00493058 = 0;

/* Function start: 0x421910 */
void cleanup_objectives(void)
{
    short proximityRadius;
    short type;
    short missionIndex;
    short shipObj;
    short home;
    short navObjective;

    navObjective = 0;
    home = find_ship_index(g_stMissionHeader_005d3e70.wingmanMissionShip);
    proximityRadius = g_aMissionNavPoints_00491e98[
        g_stMissionHeader_005d3e70.entryNavPoint].proximityRadius;
    while ((type =
                g_aMissionObjectives_004932a8[navObjective].type) != -1 &&
           navObjective < 8) {
        if (achieved(navObjective) == 0) {
            missionIndex = g_aMissionObjectives_004932a8[navObjective].index;
            switch (type) {
            case 0:
            case 1:
            case 3:
                if (visited(navObjective) != 0)
                    flag_objective(navObjective, 2);
                break;
            case 2:
                switch (g_aMissionShips_00492290[missionIndex].missionType) {
                case MISSION_TYPE_WARP_ARRIVE:
                case MISSION_TYPE_COME_HOME:
                    shipObj = find_ship_index(missionIndex);
                    if (shipObj != -1 && home != -1 &&
                        distance_from_object(shipObj, home) < proximityRadius)
                        g_aMissionShips_00492290[missionIndex].state = 1;
                    if (g_aMissionShips_00492290[missionIndex].state == 1 &&
                        sighted(navObjective) != 0) {
                        flag_objective(navObjective, 2);
                        affect_mission_score(0, 8, -1);
                    }
                    break;
                case MISSION_TYPE_GOTO_WARP:
                    if (g_aMissionShips_00492290[missionIndex].state == 2) {
                        flag_objective(navObjective, 2);
                        affect_mission_score(0, 8, -1);
                    }
                    break;
                default:
                    if (g_aMissionShips_00492290[missionIndex].state == 0 &&
                        sighted(navObjective) != 0) {
                        flag_objective(navObjective, 2);
                        affect_mission_score(0, 8, -1);
                    }
                }
                break;
            case 4:
                if (g_aMissionShips_00492290[missionIndex].state == 3)
                    flag_objective(navObjective, 2);
                break;
            }
        }
        navObjective++;
    }
}

/* Function start: 0x421BC6 */
short IsShipRouting(short ship)
{
    if (g_asShipMissionType_00495de8[ship] == MISSION_TYPE_ROUT)
        return 1;
    else
        return 0;
}

/* Function start: 0x421BFC */
void SendWingmanCommandAcknowledgement(short ship, short accepted)
{
    if (accepted == 1) {
        send_message(ship, 0);
        return;
    }
    send_message(ship, 1);
}

short report_kilrathi_rout(int mode);
short detect_enemy_tail(short obj);
short any_enemy(short obj, short range);
void engage(short obj, short target, short objective);
unsigned int reset_objective(short ship, short objective);
unsigned int alter_objective(short ship, short objective);
short IsShipRouting(short ship);

/* Function start: 0x421C35 */
short disobey_formation(short ship)
{
    if (ship != g_nYourWingman_0049346c)
        return 0;
    switch (g_nWingmanFormationDisobeyMode_0049613a) {
    case 1:
        return report_kilrathi_rout(0);
    case 2:
        return detect_enemy_tail(0) != -1;
    case 3:
        return 1;
    case 4:
        return any_enemy(ship, 5000);
    default:
        return 0;
    }
}

/* Function start: 0x421CF6 */
short IsEngagementTargetDisallowed(short ship, short target)
{
    return target == ship ||
           g_asShipSide_004955d0[target] == g_asShipSide_004955d0[ship] ||
           IsShipRouting(ship) != 0;
}

/* Function start: 0x421D59 */
short CanPlayerLand(void)
{
    short index;
    short ok;

    ok = 0;
    for (index = 0; index < 10; index++) {
        if (g_aeObjectClass_00495328[index] >= OBJECT_CLASS_SHIP &&
            g_asShipSide_004955d0[index] == SIDE_KILRATHI)
            return 0;
    }
    if (g_cPlayerKillCount_005d2fa8 > 0)
        ok = 1;
    for (index = 0; index < g_cMissionObjectiveCount_00493294; index++) {
        if (g_aMissionObjectives_004932a8[index].type != 1 &&
            (achieved(index) != 0 ||
             (visited(index) != 0 &&
              g_aMissionObjectives_004932a8[index].type != 2)))
            ok = 1;
    }
    return ok;
}

short any_enemy(short obj, short range);
short triumph(short obj);
void request(short requester, short ship, short command);

/* Function start: 0x421E6D */
short ShouldWingmanAcceptRoutCommand(short ship, short pilot)
{
    if (pilot <= 4 || ship != g_nYourWingman_0049346c)
        return 1;
    else
        switch (g_nWingmanRoutDecisionMode_00496138) {
        case 1:
            return 0;
        case 2:
            return any_enemy(0, 10000) == 0;
        case 3:
            return any_enemy(ship, 5000) == 0;
        case 4:
            return triumph(0);
        case 5:
            return g_asShipMissionType_00495de8[ship] ==
                   MISSION_TYPE_CANNED_SEQUENCE;
        default:
            return 1;
        }
}

void SendWingmanCommandAcknowledgement(short ship, short accepted);

/* Function start: 0x421F86 */
void request(short requester, short ship, short command)
{
    short target;
    short object;

    target = g_acShipTarget_00495f20[requester];
    switch (command) {
    case 0:
        break;
    case 3:
        if (ShouldWingmanAcceptRoutCommand(ship,
                                           g_asPilotLevel_00495d60[ship]) != 0 &&
            try2rout(ship) != 0) {
            g_bEngageAllowed_0049612c = 0;
            SendWingmanCommandAcknowledgement(ship, 1);
        } else {
            SendWingmanCommandAcknowledgement(ship, 0);
        }
        break;
    case 1:
        allow_engage();
        if (IsEngagementTargetDisallowed(ship, target) != 0 ||
            (ShipHasTorpedo(ship) == 0 &&
             CanShipWeaponDamageTarget(ship, target) == 0)) {
            SendWingmanCommandAcknowledgement(ship, 0);
        } else {
            engage(ship, target, OBJECTIVE_ENGAGE_ENEMY);
            SendWingmanCommandAcknowledgement(ship, 1);
        }
        break;
    case 2:
        allow_engage();
        target = -1;
        for (object = 0; object < 10; object++) {
            if (g_aeObjectClass_00495328[object] >= OBJECT_CLASS_SHIP &&
                g_aeSpecialManeuver_00495600[object] !=
                    SPECIAL_MANEUVER_UNKNOWN_9 &&
                g_asShipSide_004955d0[ship] !=
                    g_asShipSide_004955d0[object] &&
                g_acShipTarget_00495f20[object] == requester) {
                target = object;
                break;
            }
        }
        if (target == -1) {
            request(requester, ship, 6);
        } else {
            engage(ship, target, OBJECTIVE_ENGAGE_ENEMY);
            SendWingmanCommandAcknowledgement(ship, 1);
        }
        break;
    case 4:
        allow_engage();
        if (g_aeShipObjective_00495f08[ship] == OBJECTIVE_HOLD_FORMATION) {
            reset_objective(ship, OBJECTIVE_BREAK_FORMATION);
            SendWingmanCommandAcknowledgement(ship, 1);
        } else {
            SendWingmanCommandAcknowledgement(ship, 0);
        }
        break;
    case 6:
        disallow_engage();
        if (disobey_formation(ship) == 0) {
            SendWingmanCommandAcknowledgement(ship, 0);
        } else {
            reset_objective(ship, OBJECTIVE_HOLD_FORMATION);
            g_nAutoEngageTimer_00496130 = -150;
            SendWingmanCommandAcknowledgement(ship, 1);
        }
        break;
    case 5:
        disallow_engage();
        if (disobey_formation(ship) == 0) {
            alter_objective(ship, OBJECTIVE_BREAK_FORMATION);
            SendWingmanCommandAcknowledgement(ship, 0);
        } else {
            g_nAutoEngageTimer_00496130 = -150;
            SendWingmanCommandAcknowledgement(ship, 1);
        }
        break;
    case 7:
    case 8:
        g_bRadioSilence_0049b780 = 0;
        SendWingmanCommandAcknowledgement(ship, 1);
        if (command == 7)
            g_bRadioSilence_0049b780 = 1;
        else
            g_bRadioSilence_0049b780 = 0;
        break;
    case 11:
        cleanup_objectives();
        if (CanPlayerLand() != 0) {
            g_bLandingCommRequestPending_00492fa0 = 1;
            send_message(ship, 0x12);
        } else {
            send_message(ship, 0x13);
        }
        break;
    case 12:
        cleanup_objectives();
        if (CanPlayerLand() != 0) {
            g_bLandingCommRequestPending_00492fa0 = 1;
            send_message(ship, 0x14);
            g_bEjectionSequencePending_00493058 = 1;
        } else {
            send_message(ship, 0x15);
        }
        break;
    case 9:
        if (g_ucPendingEjectionTransition_0049b8ac != 0xff &&
            g_bEjectionAwaitingCommCommand_0049b8b4 != 0) {
            ejection_sequence(g_ucPendingEjectionTransition_0049b8ac, 1);
            g_ucPendingEjectionTransition_0049b8ac = 0xff;
            g_bEjectionAwaitingCommCommand_0049b8b4 = 0;
        } else if (g_asShipMissionIndex_00495d00[ship] !=
                   g_nHomeMissionShipIndex_005d1e22) {
            send_message(ship, 0x11);
        } else {
            cleanup_objectives();
            if (CanPlayerLand() != 0) {
                g_bLandingCommRequestPending_00492fa0 = 1;
                send_message(ship, 0x10);
            } else {
                send_message(ship, 0x11);
            }
        }
        break;
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
        if (RandomBelow(100) < 70 ||
            (g_asPilotLevel_00495d60[ship] == 5 &&
             g_asShipSide_004955d0[ship] == SIDE_KILRATHI)) {
            send_message(ship,
                         (signed char)(command + 0x19 -
                                       g_nEnemyTauntCommandBase_0049b76c));
            g_nEnemyTauntCommandBase_0049b76c =
                (short)(g_nEnemyTauntCommandBase_0049b76c | 1);
        }
        if (g_acShipTarget_00495f20[ship] != requester &&
            IsShipRouting(ship) == 0)
            engage(ship, requester, OBJECTIVE_ENGAGE_ENEMY);
        break;
    case 10:
        if (g_asShipSide_004955d0[target] != SIDE_KILRATHI) {
            send_message(ship, 0xf);
            g_asShipIdentified_00496078[ship] = 1;
        }
        break;
    case 13:
        if (g_nYourWingman_0049346c != -1) {
            send_message(ship, 0xf);
            g_bDisplayWingmanTargetData_0049347c = 1;
        }
        break;
    }
}
