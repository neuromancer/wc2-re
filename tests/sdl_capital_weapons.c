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

    g_nAudioEnabled = 0;
    g_78_bHighMemoryBuffersSetState = 0;
    g_aObjectTypeData[0].collisionRadius = 100;
    for (object = 1; object <= 2; object++) {
        g_aeObjectClass[object] = OBJECT_CLASS_CAPITAL_SHIP;
        g_acObjectType[object] = 0;
        g_asObjectCollisionRadius[object] = 100;
        g_asShipMaximumVelocity[object] = 100;
        init_ijk(object);
    }
    g_asShipSide[1] = SIDE_KILRATHI;
    g_asShipSide[2] = SIDE_IMPERIAL;

    for (scenario = 0;
         scenario < sizeof(targetOffsets) / sizeof(targetOffsets[0]);
         scenario++) {
        ship = scenario == 9 ? 2 : 1;
        target = ship == 1 ? 2 : 1;
        g_acShipTarget[ship] = (signed char)target;
        for (object = 10; object <= SPACE_LAST_MOVING_OBJECT; object++)
            g_aeObjectClass[object] = OBJECT_CLASS_NULL;
        g_aShipPosition[ship].x = 700 * 256;
        g_aShipPosition[ship].y = -900 * 256;
        g_aShipPosition[ship].z = 1200 * 256;
        g_aShipPosition[target].x =
            g_aShipPosition[ship].x + targetOffsets[scenario].x * 256;
        g_aShipPosition[target].y =
            g_aShipPosition[ship].y + targetOffsets[scenario].y * 256;
        g_aShipPosition[target].z =
            g_aShipPosition[ship].z + targetOffsets[scenario].z * 256;
        targetPosition = g_aShipPosition[target];
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
            g_aShipWeapons[ship][0] = 1;
            weapon = (ShipWeaponSlot *)&g_aShipWeapons[ship][1];
            weapon->type = 0x0d;
            g_2_asFlakCooldownTable[ship] = 0;
            g_asSpikeriLaunchCountdown[ship] = 0;
            zero_vector(&g_aShipVelocity[target]);
            g_aShipVelocity[target].y = 20 * 256;
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
        position = &g_aShipPosition[projectile];
        velocity = &g_aShipVelocity[projectile];
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
        if (g_asObjectType[projectile] != 0x0d ||
            g_asObjectDamage[projectile] != 300 ||
            g_asObjectCounter[projectile] != 100 ||
            memcmp(&targetPosition, &g_aShipPosition[target],
                   sizeof(targetPosition)) != 0) {
            fprintf(stderr, "capital shot %u: projectile or target state corrupted\n",
                    scenario);
            return 1;
        }
    }

    /* A full object pool must leave the ships alone and emit no projectile. */
    for (object = 10; object <= SPACE_LAST_MOVING_OBJECT; object++)
        g_aeObjectClass[object] = OBJECT_CLASS_MINE;
    SpawnCapitalFlakProjectile(0, ship, aim.x, aim.y, aim.z);
    if (CountShipProjectiles(ship) != 0 ||
        g_acShipTarget[ship] != target) {
        fprintf(stderr, "capital shot: exhausted object pool changed ship state\n");
        return 1;
    }
    return 0;
}
