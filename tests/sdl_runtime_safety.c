#include "wc1.h"

int main(int argumentCount, char **arguments)
{
    float thrusterScreenX;
    float thrusterScreenY;
    short collisionPartner = 15;
    short object;
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

    /* A fifth VDU mode cannot fit in the four-entry stack.  The native port
     * drops that push and leaves the current mode in place. */
    g_acVduModeStackDepth_004934c8[1] = 3;
    g_aaiVduModeStack_00493498[1][3] = 6;
    push_mode(1, 4);
    if (get_mode(1) != 6 || g_acVduModeStackDepth_004934c8[1] != 3)
        return 1;

    /* A transmission VDU can be revisited after its speaker has been cleared.
     * WC2 then reads the zero word immediately before the ship-side table. */
    g_nCommDeathSequenceFrame_0049ae84 = 0;
    g_bCommSpeechPlaying_0049b7a0 = 0;
    g_nCommSpeakerObject_0049b794 = -1;
    g_nCommPortraitIndex_0049b79c = -1;
    g_acShipLastAttacker_004955c0[14] = 0;
    g_acShipLastAttacker_004955c0[15] = 0;
    vid_transmit();
    if (get_mode(1) != 6)
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

    /* With no target, WC2 reads the zero-filled word immediately before the
     * collision-radius table.  Preserve that result without crossing a
     * native global's sanitizer redzone. */
    g_acShipTarget_00495f20[ship] = -1;
    g_asShipManeuver_00495f48[ship] = MANEUVER_THINKING;
    g_asObjectCollisionRadius_004950e8[ship] = 25;
    g_nTargetFacing_00493198 = -1;
    DAT_005b30f0 = 0;
    perform_maneuver(ship);
    if (DAT_005b30f0 != 100 ||
        g_asShipManeuver_00495f48[ship] != MANEUVER_NONE)
        return 1;

    /* An unattributed destruction reads the zero-filled byte immediately
     * before the owner table in WC2, turning attacker -1 into object 0. */
    g_asPilotLevel_00495d60[target] = 0;
    g_aeSpecialManeuver_00495600[target] = SPECIAL_MANEUVER_NONE;
    g_aeObjectClass_00495328[target] = OBJECT_CLASS_CAPITAL_SHIP;
    g_abShipEjectionSequenceEnabled_00496120[target] = 1;
    g_asShipSide_004955d0[0] = SIDE_KILRATHI;
    g_asShipSide_004955d0[target] = SIDE_IMPERIAL;
    g_nInFlightMusicActive_0049bf08 = 0;
    g_cPlayerKillCount_005d2fa8 = 0;
    if (ResolveObjectDestruction(-1, target) != 0 ||
        g_cPlayerKillCount_005d2fa8 != 1)
        return 1;
    g_asShipSide_004955d0[0] = SIDE_IMPERIAL;

    /* A base colliding with an asteroid in slot 15 produces a negative mass
     * response and reaches beyond WC2's twelve ship-velocity entries. */
    for (object = 0; object <= WC2_SPACE_LAST_MOVING_OBJECT; object++) {
        g_aeObjectClass_00495328[object] = OBJECT_CLASS_NULL;
        g_asObjectScreenX_00493598[object] = (short)0x8001;
    }
    g_aeObjectClass_00495328[0] = OBJECT_CLASS_BASE;
    g_aeObjectClass_00495328[collisionPartner] = OBJECT_CLASS_ASTEROID;
    g_acObjectType_00493980[0] = 0;
    g_acObjectType_00493980[collisionPartner] = 1;
    g_aObjectTypeData_00496d30[0].radarRadius = 25;
    g_aObjectTypeData_00496d30[1].radarRadius = 200;
    g_asObjectCollisionRadius_004950e8[0] = 10;
    g_asObjectCollisionRadius_004950e8[collisionPartner] = 10;
    zero_vector(&g_aShipPosition_00494550[0]);
    zero_vector(&g_aShipPosition_00494550[collisionPartner]);
    zero_vector(&g_aShipVelocity_00494898[0]);
    zero_vector(&g_aShipVelocity_00494898[collisionPartner]);
    g_aShipPosition_00494550[collisionPartner].x = 0x100;
    g_acObjectOwner_00495208[collisionPartner] = -1;
    g_acLastCollisionObject_00495250[0] = -1;
    g_acLastCollisionObject_00495250[collisionPartner] = -1;
    g_asShipMaximumVelocity_00495f70[0] = 1;
    g_bPlayerCollisionEnabled_0049d780 = 1;
    g_nCurrentView_00492fa8 = 1;
    object_collision(0);
    if (g_acLastCollisionObject_00495250[0] != collisionPartner ||
        g_acLastCollisionObject_00495250[collisionPartner] != 0)
        return 1;

    /* Autopilot's view-13 camera draws the running engines as fixed children.
     * Their enhanced positions need the parent's view-space coordinates. */
    for (object = 0; object <= WC2_SPACE_LAST_MOVING_OBJECT; object++) {
        g_aeObjectClass_00495328[object] = OBJECT_CLASS_NULL;
        g_asObjectScreenX_00493598[object] = (short)0x8001;
    }
    init_ijk(WC2_EYE_OBJECT);
    zero_vector(&g_aShipPosition_00494550[WC2_EYE_OBJECT]);
    g_asObjectCollisionRadius_004950e8[WC2_EYE_OBJECT] = 0;
    g_nScreenWidth_0049d4d8 = 320;
    g_nViewCenterX_005c80d8 = 160;
    g_nViewCenterY_005c80da = 100;
    g_aeObjectClass_00495328[ship] = OBJECT_CLASS_PROJECTILE;
    g_asObjectCollisionRadius_004950e8[ship] = 0;
    g_asObjectScale_00494d90[ship] = 0x100;
    g_aShipPosition_00494550[ship].x = 0;
    g_aShipPosition_00494550[ship].y = 0;
    g_aShipPosition_00494550[ship].z = 0x20000;
    transform_objects_to_your_view();
    if (g_aObjectViewPosition_0059afa0[ship].z != 0x20000)
        return 1;

    object = 10;
    g_aeObjectClass_00495328[object] = OBJECT_CLASS_FIXED_OBJECT;
    g_asObjectType_00495298[object] = WC2_OBJECT_TYPE_THRUSTERS;
    g_acObjectOwner_00495208[object] = (signed char)ship;
    g_asObjectScreenX_00493598[object] = 0;
    g_asObjectScreenY_00493628[object] = 0;
    g_asObjectDistance_00493ae8[object] = 0;
    g_asObjectScale_00494d90[object] = 0x100;
    g_asObjectScreenAngle_004936b8[ship] = 0;
    g_asObjectFlip_004939c8[ship] = 0;
    reposition_fixed_child_objects();
    Wc1SdlGetThrusterScreenPosition(
        object, &thrusterScreenX, &thrusterScreenY);
    if (thrusterScreenX != 160.0f || thrusterScreenY != 100.0f)
        return 1;

    g_bAfterburnerSfxActive_005d3864 = 1;
    Wc1SdlStopDosSoundEffects();
    if (g_bAfterburnerSfxActive_005d3864 != 0)
        return 1;

    return 0;
}
