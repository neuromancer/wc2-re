#include "game.h"

int main(void)
{
    const short initialTimers[10] = {-2, -1, 0, 1, 2, 3, 0, 2, 1, 3};
    const short expectedTimers[10] = {-2, -1, 0, 0, 0, 1, 0, 0, 0, 1};
    short object;

    /* Slots 8 and 9 are valid ships even though the retail evasion timer
     * table only holds eight words before overlapping the stress bytes. */
    for (object = 0; object <= SPACE_LAST_MOVING_OBJECT; object++) {
        g_aeObjectClass[object] = OBJECT_CLASS_NULL;
        g_asObjectCounter[object] = -1;
    }
    g_aeObjectClass[8] = OBJECT_CLASS_CAPITAL_SHIP;
    g_aeObjectClass[9] = OBJECT_CLASS_BASE;
    g_bCarrierLandingEnabled = 0;
    house_keep_objects();

    for (object = 0; object < 10; object++) {
        if (object < 8)
            g_aeObjectClass[object] = OBJECT_CLASS_SHIP;
        g_asProjectileEvasionTicks[object] = initialTimers[object];
        g_acShipStress[object] = (signed char)(object + 10);
    }
    house_keep_objects();
    house_keep_objects();
    for (object = 0; object < 10; object++) {
        if (g_asProjectileEvasionTicks[object] != expectedTimers[object] ||
            g_acShipStress[object] != object + 10) {
            fprintf(stderr, "ship %d: evasion countdown or stress corrupted\n",
                    object);
            return 1;
        }
    }

    /* Reusing the final two slots must reset their timers without clearing
     * stress belonging to ships 0-3, as the old SDL initialization did. */
    memset(&g_aMissionShips[0], 0,
           sizeof(g_aMissionShips[0]));
    g_aMissionShips[0].navPoint = -1;
    g_aMissionShips[0].formationIndex = -1;
    g_aMissionShips[0].leaderMissionIndex = -1;
    g_aMissionShips[0].missionType = MISSION_TYPE_NONE;
    g_aMissionShips[0].side = SIDE_IMPERIAL;
    g_nCurrentNavPoint = 0;
    for (object = 8; object < 10; object++) {
        g_asProjectileEvasionTicks[object] = 17;
        Set_up_ship_info(object, 0, -1);
        if (g_asProjectileEvasionTicks[object] != 0) {
            fprintf(stderr, "ship %d: evasion timer was not reset\n", object);
            return 1;
        }
    }
    for (object = 0; object < 8; object++) {
        if (g_acShipStress[object] != object + 10) {
            fprintf(stderr, "ship %d: initialization corrupted stress\n",
                    object);
            return 1;
        }
    }

    return 0;
}
