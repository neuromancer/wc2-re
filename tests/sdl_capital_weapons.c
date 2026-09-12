#include "game.h"

int main(void)
{
    const FixedVector targetOffsets[] = {
        {5000, 0, 0}, {-5000, 0, 0},
        {0, 5000, 0}, {0, -5000, 0},
        {0, 0, 5000}, {0, 0, -5000},
        {3000, -4000, 5000}, {-3000, 4000, -5000},
        {6000, 0, 0}, {-6000, 0, 0}
    };
    short ship;
    short target;
    ShipWeaponSlot *weapon;
    FixedVector aim;
    FixedVector targetPosition;
    FixedVector *position;
    FixedVector *velocity;
    double dx;
    double dy;
    double dz;
    double distance;
    double speed;
    double facing;
    short projectile;
    short object;
    unsigned int scenario;

    g_nAudioEnabled_0049c244 = 0;
    g_bHighMemoryBuffersReady_005d2ad8 = 0;
    g_aObjectTypeData_00496d30[0].collisionRadius = 100;
    for (object = 1; object <= 2; object++) {
        g_aeObjectClass_00495328[object] = OBJECT_CLASS_CAPITAL_SHIP;
        g_acObjectType_00493980[object] = 0;
        g_asObjectCollisionRadius_004950e8[object] = 100;
        g_asShipMaximumVelocity_00495f70[object] = 100;
        init_ijk(object);
    }
    g_asShipSide_004955d0[1] = SIDE_KILRATHI;
    g_asShipSide_004955d0[2] = SIDE_IMPERIAL;

    for (scenario = 0;
         scenario < sizeof(targetOffsets) / sizeof(targetOffsets[0]);
         scenario++) {
        ship = scenario == 9 ? 2 : 1;
        target = ship == 1 ? 2 : 1;
        g_acShipTarget_00495f20[ship] = (signed char)target;
        for (object = 10; object <= SPACE_LAST_MOVING_OBJECT; object++)
            g_aeObjectClass_00495328[object] = OBJECT_CLASS_NULL;
        g_aShipPosition_00494550[ship].x = 700 * 256;
        g_aShipPosition_00494550[ship].y = -900 * 256;
        g_aShipPosition_00494550[ship].z = 1200 * 256;
        g_aShipPosition_00494550[target].x =
            g_aShipPosition_00494550[ship].x + targetOffsets[scenario].x * 256;
        g_aShipPosition_00494550[target].y =
            g_aShipPosition_00494550[ship].y + targetOffsets[scenario].y * 256;
        g_aShipPosition_00494550[target].z =
            g_aShipPosition_00494550[ship].z + targetOffsets[scenario].z * 256;
        targetPosition = g_aShipPosition_00494550[target];
        aim = targetPosition;
        /* A led shot must use the supplied point, not the target's current
         * position. Keep each component distinct from the shooter's too. */
        if (scenario == 7) {
            aim.x += 1300 * 256;
            aim.y -= 1700 * 256;
            aim.z += 2100 * 256;
        }

        if (scenario >= 8) {
            /* Exercise the strike AI for both factions, including its target
             * lead: (6000 - 100 - 100) / 250 gives 23 travel ticks. */
            g_aShipWeapons_004956b0[ship][0] = 1;
            weapon = (ShipWeaponSlot *)&g_aShipWeapons_004956b0[ship][1];
            weapon->type = 0x0d;
            DAT_005d1bd0[ship] = 0;
            DAT_005d1c10[ship] = 0;
            zero_vector(&g_aShipVelocity_00494898[target]);
            g_aShipVelocity_00494898[target].y = 20 * 256;
            aim.y += 23 * 20 * 256;
            capital_ship_strike_mission(ship);
        } else {
            SpawnCapitalFlakProjectile(0, ship, aim.x, aim.y, aim.z);
        }
        projectile = find_child_object(ship, OBJECT_CLASS_PROJECTILE);
        if (projectile == -1 || CountShipProjectiles(ship) != 1) {
            fprintf(stderr, "capital shot %u: projectile was not created\n", scenario);
            return 1;
        }
        position = &g_aShipPosition_00494550[projectile];
        velocity = &g_aShipVelocity_00494898[projectile];
        dx = (double)aim.x - position->x;
        dy = (double)aim.y - position->y;
        dz = (double)aim.z - position->z;
        distance = sqrt(dx * dx + dy * dy + dz * dz);
        speed = sqrt((double)velocity->x * velocity->x +
                     (double)velocity->y * velocity->y +
                     (double)velocity->z * velocity->z);
        facing = dx * velocity->x + dy * velocity->y + dz * velocity->z;
        if (speed < 247 * 256 || speed > 251 * 256 ||
            facing < 0.9999 * distance * speed) {
            fprintf(stderr, "capital shot %u: projectile misses its aim point\n",
                    scenario);
            return 1;
        }
        if (g_asObjectType_00495298[projectile] != 0x0d ||
            g_asObjectDamage_00495178[projectile] != 300 ||
            g_asObjectCounter_00494be0[projectile] != 100 ||
            memcmp(&targetPosition, &g_aShipPosition_00494550[target],
                   sizeof(targetPosition)) != 0) {
            fprintf(stderr, "capital shot %u: projectile or target state corrupted\n",
                    scenario);
            return 1;
        }
    }

    /* A full object pool must leave the ships alone and emit no projectile. */
    for (object = 10; object <= SPACE_LAST_MOVING_OBJECT; object++)
        g_aeObjectClass_00495328[object] = OBJECT_CLASS_MINE;
    SpawnCapitalFlakProjectile(0, ship, aim.x, aim.y, aim.z);
    if (CountShipProjectiles(ship) != 0 ||
        g_acShipTarget_00495f20[ship] != target) {
        fprintf(stderr, "capital shot: exhausted object pool changed ship state\n");
        return 1;
    }
    return 0;
}
