/*
 *  Wingman orders and landing requests.
 *
 *  Retail object 0x421910-0x42259F (link order; see docs/ORDER.md).
 */
#include "game.h"

short g_bEjectionSequencePending = 0; /* 0x00493058 */

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
    home = find_ship_index(g_13_stMissionHeaderBlock.wingmanMissionShip);
    proximityRadius = g_aMissionNavPoints[
        g_13_stMissionHeaderBlock.entryNavPoint].proximityRadius;
    while ((type =
                g_aMissionObjectives[navObjective].type) != -1 &&
           navObjective < 8) {
        if (achieved(navObjective) == 0) {
            missionIndex = g_aMissionObjectives[navObjective].index;
            switch (type) {
            case 0:
            case 1:
            case 3:
                if (visited(navObjective) != 0)
                    flag_objective(navObjective, 2);
                break;
            case 2:
                switch (g_aMissionShips[missionIndex].missionType) {
                case MISSION_TYPE_WARP_ARRIVE:
                case MISSION_TYPE_COME_HOME:
                    shipObj = find_ship_index(missionIndex);
                    if (shipObj != -1 && home != -1 &&
                        distance_from_object(shipObj, home) < proximityRadius)
                        g_aMissionShips[missionIndex].state = 1;
                    if (g_aMissionShips[missionIndex].state == 1 &&
                        sighted(navObjective) != 0) {
                        flag_objective(navObjective, 2);
                        affect_mission_score(0, 8, -1);
                    }
                    break;
                case MISSION_TYPE_GOTO_WARP:
                    if (g_aMissionShips[missionIndex].state == 2) {
                        flag_objective(navObjective, 2);
                        affect_mission_score(0, 8, -1);
                    }
                    break;
                default:
                    if (g_aMissionShips[missionIndex].state == 0 &&
                        sighted(navObjective) != 0) {
                        flag_objective(navObjective, 2);
                        affect_mission_score(0, 8, -1);
                    }
                }
                break;
            case 4:
                if (g_aMissionShips[missionIndex].state == 3)
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
    if (g_asShipMissionType[ship] == MISSION_TYPE_ROUT)
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
    if (ship != g_nYourWingman)
        return 0;
    switch (g_stWingmanIntelligence.formationDisobeyMode) {
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
           g_asShipSide[target] == g_asShipSide[ship] ||
           IsShipRouting(ship) != 0;
}

/* Function start: 0x421D59 */
short CanPlayerLand(void)
{
    short index;
    short ok;

    ok = 0;
    for (index = 0; index < 10; index++) {
        if (g_aeObjectClass[index] >= OBJECT_CLASS_SHIP &&
            g_asShipSide[index] == SIDE_KILRATHI)
            return 0;
    }
    if (g_36_cPilotKillCountValue > 0)
        ok = 1;
    for (index = 0; index < g_cMissionObjectiveCount; index++) {
        if (g_aMissionObjectives[index].type != 1 &&
            (achieved(index) != 0 ||
             (visited(index) != 0 &&
              g_aMissionObjectives[index].type != 2)))
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
    if (pilot <= 4 || ship != g_nYourWingman)
        return 1;
    else
        switch (g_stWingmanIntelligence.routDecisionMode) {
        case 1:
            return 0;
        case 2:
            return any_enemy(0, 10000) == 0;
        case 3:
            return any_enemy(ship, 5000) == 0;
        case 4:
            return triumph(0);
        case 5:
            return g_asShipMissionType[ship] ==
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

    target = g_acShipTarget[requester];
    switch (command) {
    case 0:
        break;
    case 3:
        if (ShouldWingmanAcceptRoutCommand(ship,
                                           g_asPilotLevel[ship]) != 0 &&
            try2rout(ship) != 0) {
            g_bEngageAllowed = 0;
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
            if (g_aeObjectClass[object] >= OBJECT_CLASS_SHIP &&
                g_aeSpecialManeuver[object] !=
                    SPECIAL_MANEUVER_UNKNOWN_9 &&
                g_asShipSide[ship] !=
                    g_asShipSide[object] &&
                g_acShipTarget[object] == requester) {
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
        if (g_aeShipObjective[ship] == OBJECTIVE_HOLD_FORMATION) {
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
            g_nAutoEngageTimer = -150;
            SendWingmanCommandAcknowledgement(ship, 1);
        }
        break;
    case 5:
        disallow_engage();
        if (disobey_formation(ship) == 0) {
            alter_objective(ship, OBJECTIVE_BREAK_FORMATION);
            SendWingmanCommandAcknowledgement(ship, 0);
        } else {
            g_nAutoEngageTimer = -150;
            SendWingmanCommandAcknowledgement(ship, 1);
        }
        break;
    case 7:
    case 8:
        g_bRadioSilence = 0;
        SendWingmanCommandAcknowledgement(ship, 1);
        if (command == 7)
            g_bRadioSilence = 1;
        else
            g_bRadioSilence = 0;
        break;
    case 11:
        cleanup_objectives();
        if (CanPlayerLand() != 0) {
            g_bLandingCommRequestPending = 1;
            send_message(ship, 0x12);
        } else {
            send_message(ship, 0x13);
        }
        break;
    case 12:
        cleanup_objectives();
        if (CanPlayerLand() != 0) {
            g_bLandingCommRequestPending = 1;
            send_message(ship, 0x14);
            g_bEjectionSequencePending = 1;
        } else {
            send_message(ship, 0x15);
        }
        break;
    case 9:
        if (g_ucPendingEjectionTransition != 0xff &&
            g_bEjectionAwaitingCommCommand != 0) {
            ejection_sequence(g_ucPendingEjectionTransition, 1);
            g_ucPendingEjectionTransition = 0xff;
            g_bEjectionAwaitingCommCommand = 0;
        } else if (g_asShipMissionIndex[ship] !=
                   g_6_nHomeOperationShipIndexValue) {
            send_message(ship, 0x11);
        } else {
            cleanup_objectives();
            if (CanPlayerLand() != 0) {
                g_bLandingCommRequestPending = 1;
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
            (g_asPilotLevel[ship] == 5 &&
             g_asShipSide[ship] == SIDE_KILRATHI)) {
            send_message(ship,
                         (signed char)(command + 0x19 -
                                       g_nEnemyTauntCommandBase));
            g_nEnemyTauntCommandBase =
                (short)(g_nEnemyTauntCommandBase | 1);
        }
        if (g_acShipTarget[ship] != requester &&
            IsShipRouting(ship) == 0)
            engage(ship, requester, OBJECTIVE_ENGAGE_ENEMY);
        break;
    case 10:
        if (g_asShipSide[target] != SIDE_KILRATHI) {
            send_message(ship, 0xf);
            g_asShipIdentified[ship] = 1;
        }
        break;
    case 13:
        if (g_nYourWingman != -1) {
            send_message(ship, 0xf);
            g_bDisplayWingmanTargetData = 1;
        }
        break;
    }
}
