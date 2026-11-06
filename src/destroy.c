/*
 *  Ship destruction and mission ejection pods.
 *
 *  Retail object 0x446710-0x44690F (link order; see docs/ORDER.md).
 */
#include "game.h"

short g_bMissionDeathSequencePending = 0; /* 0x0049b720 */
short g_bMissionEjectionPodSpawned = 0; /* 0x0049b724 */

/* Function start: 0x446710 */
void BeginShipDestructionSequence(short obj)
{
    send_message(obj, 9);
    g_nPendingEjectionShip = obj;
    set_special(obj, SPECIAL_MANEUVER_UNKNOWN_9);
    if (g_aObjectTypeData[OBJECT_DATA_INDEX(OBJECT_TYPE_EXPLOSION2)].shapeSet != 0)
        g_asObjectCounter[obj] = 8;
    else
        g_asObjectCounter[obj] = 4;
    if (g_aObjectTypeData[OBJECT_DATA_INDEX(OBJECT_TYPE_EXPLOSION2)].shapeSet != 0)
        g_7_asShipBurstStageCountdown[obj] = 8;
    else
        g_7_asShipBurstStageCountdown[obj] = 4;
    g_aMissionShips[g_asShipMissionIndex[obj]].state = 3;
    ShipExplosion(obj);
    explosion_shock_wave(
        obj,
        g_aObjectTypeData[
            (signed char)g_acObjectType[obj]].explosionDamage);
    if (g_asObjectScreenX[obj] != -32767)
        PlaySfxWaveFileByNumber(4, obj, 0);
    g_bMissionDeathSequencePending = 1;
    g_bMissionEjectionPodSpawned = 0;
}

void set_objects_data(short obj, short type, short owner, short matchObjectClass);
void PlaySfxWaveFileByNumber(int soundNumber, int sourceObject, int looping);
void set_special(short ship, short special);
void zero_vector(FixedVector *vector);

/* Function start: 0x446823 */
void SpawnMissionEjectionPod(short obj)
{
    short initialShip;

    if (g_aObjectTypeData[
            OBJECT_DATA_EJECTION_POD].shapeSet == 0) {
        g_aObjectTypeData[
            OBJECT_DATA_EJECTION_POD].shapeSet =
            FetchDiskPacketRetrying("pilotanm.vga", 2, 0);
    }
    set_objects_data(obj, OBJECT_DATA_EJECTION_POD, -1, 0);
    zero_vector(&g_aShipVelocity[obj]);
    RecordCannedSceneObjectEvent(obj, 0);
    g_bMissionEjectionPodSpawned = 1;
    for (initialShip = 0; initialShip < 8; initialShip++) {
        if (g_13_stMissionHeaderBlock.initialMissionShips[initialShip] ==
                g_asShipMissionIndex[obj]) {
            g_acMissionShipStatusFlagsBytes[initialShip] |= 2;
            if (obj == g_nYourWingman)
                g_nYourWingman = -1;
        }
    }
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short g_nPendingEjectionShip; /* 0x005d1bc4 */
