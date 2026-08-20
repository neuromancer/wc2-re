#include "wc1.h"

int main(int argumentCount, char **arguments)
{
    short ship = 1;
    short target = 2;

    g_asShipSide_004955d0[0] = SIDE_IMPERIAL;
    g_asShipSide_004955d0[1] = SIDE_KILRATHI;
    g_acPlayerComponentDamage_00493470[5] = 0;
    g_asObjectScreenX_00493598[1] = 0;
    g_asObjectScreenY_00493628[1] = 0;
    g_nTargetLockCountdown_004934ec = 0;
    g_nSelectedReleaseWeaponIndex_004934e0 = -1;
    target_locking(1);
    if (g_nTargetLockCountdown_004934ec != -1)
        return 1;

    g_acShipTarget_00495f20[ship] = (signed char)target;
    g_asShipManeuver_00495f48[ship] = MANEUVER_BUZZ_DEBRIS;
    g_acShipSequence_00495fe8[ship] = 2;
    g_aeSpecialManeuver_00495600[ship] = SPECIAL_MANEUVER_NONE;
    g_aeObjectClass_00495328[target] = OBJECT_CLASS_SHIP;
    g_asShipManeuver_00495f48[target] = MANEUVER_NONE;
    g_aShipPosition_00494550[ship].x = 0;
    g_aShipPosition_00494550[ship].y = 0;
    g_aShipPosition_00494550[ship].z = 0;
    g_aShipPosition_00494550[target].x = 0;
    g_aShipPosition_00494550[target].y = 0;
    g_aShipPosition_00494550[target].z = 0x10000;
    g_aShipForwardVector_00494208[ship].z = 0x100;
    g_aShipForwardVector_00494208[target].z = 0x100;
    perform_maneuver(ship);
    if (g_asShipManeuver_00495f48[ship] != MANEUVER_NONE)
        return 1;

    g_bAfterburnerSfxActive_005d3864 = 1;
    Wc1SdlStopDosSoundEffects();
    if (g_bAfterburnerSfxActive_005d3864 != 0)
        return 1;

    return 0;
}
