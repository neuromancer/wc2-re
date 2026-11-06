#include "game.h"

int main(void)
{
    const short freeSlotCounts[] = {6, 1, 0};
    ShipWeaponSlot *weapons;
    short object;
    short turret;
    short freeSlots;
    short expectedCooldown;
    unsigned int scenario;

    g_nAudioEnabled = 0;
    g_78_bHighMemoryBuffersSetState = 0;
    g_nCurrentView = 0;
    memset(g_aShipWeapons, 0, sizeof(g_aShipWeapons));
    g_aShipWeapons[0][0] = 3;
    weapons = (ShipWeaponSlot *)&g_aShipWeapons[0][1];
    for (turret = 0; turret < 3; turret++)
        weapons[turret].type = 0x0b;

    /* A neutral firing ship avoids random faction-specific firing delays.
     * Put one hostile ship in each turret's firing arc. */
    for (object = 0; object < 4; object++) {
        g_aeObjectClass[object] = OBJECT_CLASS_SHIP;
        g_asShipSide[object] = SIDE_KILRATHI;
    }
    g_asShipSide[0] = SIDE_NEUTRAL;
    init_ijk(0);
    g_aShipPosition[1].z = -1000 * 256;
    g_aShipPosition[2].x = -1000 * 256;
    g_aShipPosition[3].x = 1000 * 256;
    memset(&g_aObjectTypeData[8], 0,
           sizeof(g_aObjectTypeData[8]));
    g_aObjectTypeData[8].objectClass = OBJECT_CLASS_PROJECTILE;
    g_aObjectTypeData[8].field_18 = 8;
    g_aObjectTypeData[8].maximumVelocity = 100;
    g_aObjectTypeData[8].lifetime = 10;
    g_aObjectTypeData[8].damageCapacity = 7;

    for (scenario = 0;
         scenario < sizeof(freeSlotCounts) / sizeof(freeSlotCounts[0]);
         scenario++) {
        freeSlots = freeSlotCounts[scenario];
        memset(g_27_aasGunCooldown, 0, sizeof(g_27_aasGunCooldown));
        for (object = 10; object <= SPACE_LAST_MOVING_OBJECT; object++) {
            /* Visible mines cannot be borrowed when the object pool is full. */
            g_aeObjectClass[object] = OBJECT_CLASS_MINE;
            g_asObjectScreenX[object] = 0;
        }
        for (object = 12; object < 12 + freeSlots; object++)
            g_aeObjectClass[object] = OBJECT_CLASS_NULL;

        /* The first turret leaves projectile == 13, or -1 on allocation
         * failure. Neither value can index the ship-side table on the next
         * turret iteration. Exercise the actual object allocator here. */
        UpdateShipTurretGuns(0);
        if (CountShipProjectiles(0) != freeSlots) {
            fprintf(stderr, "%d free slots: incorrect turret projectile count\n",
                    freeSlots);
            return 1;
        }
        for (turret = 0; turret < 3; turret++) {
            expectedCooldown = freeSlots > turret * 2 ? 12 : 0;
            if (g_27_aasGunCooldown[0][turret] != expectedCooldown) {
                fprintf(stderr, "%d free slots: incorrect turret %d cooldown\n",
                        freeSlots, turret);
                return 1;
            }
        }
        for (object = 12; object < 12 + freeSlots; object++) {
            if (g_asObjectDamage[object] != 7 ||
                g_asObjectCounter[object] != 10) {
                fprintf(stderr, "turret projectile %d was not initialized\n", object);
                return 1;
            }
        }
        UpdateShipTurretGuns(0);
        if (CountShipProjectiles(0) != freeSlots) {
            fprintf(stderr, "turret cooldown did not prevent additional shots\n");
            return 1;
        }
    }
    return 0;
}
