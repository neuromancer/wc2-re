/*
 *  Ship destruction and mission ejection pods.
 *
 *  Retail object 0x446710-0x44690F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x446710 */
void BeginShipDestructionSequence(short obj)
{
    send_message(obj, 9);
    g_nPendingEjectionShip_005d1bc4 = obj;
    set_special(obj, SPECIAL_MANEUVER_UNKNOWN_9);
    if (g_aObjectTypeData_00496d30[OBJECT_DATA_INDEX(OBJECT_TYPE_EXPLOSION2)].shapeSet != 0)
        g_asObjectCounter_00494be0[obj] = 8;
    else
        g_asObjectCounter_00494be0[obj] = 4;
    if (g_aObjectTypeData_00496d30[OBJECT_DATA_INDEX(OBJECT_TYPE_EXPLOSION2)].shapeSet != 0)
        g_asShipExplosionStageTimer_005d3850[obj] = 8;
    else
        g_asShipExplosionStageTimer_005d3850[obj] = 4;
    g_aMissionShips_00492290[g_asShipMissionIndex_00495d00[obj]].state = 3;
    ShipExplosion(obj);
    explosion_shock_wave(
        obj,
        g_aObjectTypeData_00496d30[
            (signed char)g_acObjectType_00493980[obj]].explosionDamage);
    if (g_asObjectScreenX_00493598[obj] != -32767)
        PlaySfxWaveFileByNumber(4, obj, 0);
    g_bMissionDeathSequencePending_0049b720 = 1;
    g_bMissionEjectionPodSpawned_0049b724 = 0;
}

void set_objects_data(short obj, short type, short owner, short matchObjectClass);
void PlaySfxWaveFileByNumber(int soundNumber, int sourceObject, int looping);
void set_special(short ship, short special);
void zero_vector(FixedVector *vector);

/* Function start: 0x446823 */
void SpawnMissionEjectionPod(short obj)
{
    short initialShip;

    if (g_aObjectTypeData_00496d30[
            OBJECT_DATA_EJECTION_POD].shapeSet == 0) {
        g_aObjectTypeData_00496d30[
            OBJECT_DATA_EJECTION_POD].shapeSet =
            FetchDiskPacketRetrying("pilotanm.vga", 2, 0);
    }
    set_objects_data(obj, OBJECT_DATA_EJECTION_POD, -1, 0);
    zero_vector(&g_aShipVelocity_00494898[obj]);
    RecordCannedSceneObjectEvent(obj, 0);
    g_bMissionEjectionPodSpawned_0049b724 = 1;
    for (initialShip = 0; initialShip < 8; initialShip++) {
        if (g_stMissionHeader_005d3e70.initialMissionShips[initialShip] ==
                g_asShipMissionIndex_00495d00[obj]) {
            g_acMissionShipStatusFlags_005d2fb0[initialShip] |= 2;
            if (obj == g_nYourWingman_0049346c)
                g_nYourWingman_0049346c = -1;
        }
    }
}
