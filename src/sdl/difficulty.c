#include "game.h"

int g_bSdlBalancedDifficulty;

short SdlGetBalancedCollisionRadius(short object)
{
    short radius;

    radius = g_asObjectCollisionRadius[object];
    if (g_bSdlBalancedDifficulty != 0 &&
        g_aeObjectClass[object] == OBJECT_CLASS_ASTEROID)
        radius = (short)(radius / 2);
    return radius;
}
