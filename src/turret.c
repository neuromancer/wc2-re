/*
 *  Fleet overview, target camera and turret guns.
 *
 *  Retail object 0x45E9F0-0x46147F (link order; see docs/ORDER.md).
 */
#include "game.h"

static const ShortVector g_aTargetCameraEyeOffsets_0049d430[3] = {
    {0, 0, -80},
    {-50, 0, 0},
    {50, 0, 0}
};

static const ShortVector g_aaTurretProjectileOffsets_0049d448[2][2] = {
    {{-25, 0, -80}, {25, 0, -80}},
    {{-50, 0, 25}, {-50, 0, -25}}
};

static short g_nTargetCameraGunCooldown_0049d33c;

/* The two target-camera gun offsets per display sit inside the gun display
 * configuration record at +0x88, not in a table of their own. */
#define g_aaTargetCameraGunOffsets_0049d3c8 \
    ((ShortVector (*)[2])(g_abGunDisplayConfiguration_0049d340 + 0x88))

static const short g_asFleetMouseYawThresholds_0049d400[6] = {
    10, 37, 52, 57, 62, 1070
};

static const short g_asFleetMousePitchThresholds_0049d410[6] = {
    5, 18, 27, 35, 38, 1040
};

static signed char g_bFleetPolledInputActive_0049d420;

static signed char g_bFleetButtonInputActive_0049d424;

static short g_nFleetMouseYawVelocity_0049d428;

static short g_nFleetMousePitchVelocity_0049d42c;

static short g_bFleetPrimaryInputQueued_005c57dc;

static short g_bFleetMouseMoveQueued_005c57ea;

short g_bSavedTargetLockAcquired_0049d488;

/* Function start: 0x45E9F0 */
short HandleFleetOverviewInput(void)
{
    InputEventState event;
    short result;
    int fired;
    signed char discreteInput;
    short eventType;
    short horizontal;
    short vertical;
    short horizontalMagnitude;
    short verticalMagnitude;
    short threshold;
    short notRepeated;
    short pauseModifiers;
    short unusedModifiers;

    result = 0;
    fired = 0;
    g_cPreviousKey_0049312c = g_cCurrentKey_00493128;
    ServiceInputDevices(15);
    g_bPolledFlightInputQueued_005c587a = IsInputEventQueued(7);
    g_bFleetMouseMoveQueued_005c57ea = IsInputEventQueued(3);
    g_bDiscreteFlightInputQueued_005c57e4 =
        (short)(IsInputEventQueued(4) | IsInputEventQueued(6));
    g_bFleetPrimaryInputQueued_005c57dc = IsInputEventQueued(1);
    discreteInput = 0;
    if (g_bFleetMouseMoveQueued_005c57ea == 0)
        g_cCurrentKey_00493128 |= (signed char)0x80;
    if (g_bDiscreteFlightInputQueued_005c57e4 == 0) {
        g_cCurrentKey_00493128 |= (signed char)0x80;
        if (g_bPolledFlightInputQueued_005c587a == 0 &&
            g_nCockpitControlState_0049d7ac == 0)
            g_bFleetPolledInputActive_0049d420 = 0;
    } else {
        discreteInput = 1;
    }
    if (g_bFleetPrimaryInputQueued_005c57dc == 0 &&
        g_stHostMouseMessage_005d10d0.primaryButton != 0) {
        g_cCurrentKey_00493128 = 0x39;
        fired = 1;
    }
    if (g_bFleetPrimaryInputQueued_005c57dc == 0)
        g_bFleetButtonInputActive_0049d424 = 0;

    while ((eventType = GetNextInputEvent(&event)) != 0) {
        switch (eventType) {
        case 3:
            if (g_nCockpitControlState_0049d7ac == 0) {
                SetPersonnelMousePosition(
                    (short)((g_stViewBuffer_005d2b00.left +
                             g_stViewBuffer_005d2b00.right) / 2),
                    (short)((g_stViewBuffer_005d2b00.top +
                             g_stViewBuffer_005d2b00.bottom) / 2));
                g_nCockpitControlState_0049d7ac = 1;
                g_nUiCursorFrame_005c8481 = 2;
            }
            horizontal = (short)(
                event.x -
                (g_stViewBuffer_005d2b00.right -
                 g_stViewBuffer_005d2b00.left) / 2 + 1);
            vertical = (short)(
                event.y -
                (g_stViewBuffer_005d2b00.bottom -
                 g_stViewBuffer_005d2b00.top) / 2);
            g_nPersonnelCursorX_005c8470 = event.x;
            g_nPersonnelCursorY_005c8472 = event.y;
            horizontalMagnitude = (short)labs((long)horizontal);
            verticalMagnitude = (short)labs((long)vertical);
            threshold = 0;
            while (g_asFleetMouseYawThresholds_0049d400[threshold] <=
                   horizontalMagnitude)
                threshold++;
            if (horizontal < 0)
                horizontal = (short)-threshold;
            else
                horizontal = threshold;
            threshold = 0;
            while (g_asFleetMousePitchThresholds_0049d410[threshold] <=
                   verticalMagnitude)
                threshold++;
            if (vertical < 0)
                vertical = (short)-threshold;
            else
                vertical = threshold;
            if ((int)event.x - 4 <=
                (int)g_stViewBuffer_005d2b00.left)
                horizontal = -8;
            if ((int)event.x + 4 >=
                (int)g_stViewBuffer_005d2b00.right)
                horizontal = 8;
            if ((int)event.y - 4 <=
                (int)g_stViewBuffer_005d2b00.top)
                vertical = -8;
            if ((int)event.y + 4 >=
                (int)g_stViewBuffer_005d2b00.bottom)
                vertical = 8;
            if (horizontal >= 9)
                horizontal = 8;
            if (horizontal <= -9)
                horizontal = -8;
            if (vertical >= 9)
                vertical = 8;
            if (vertical <= -9)
                vertical = -8;
            g_nFleetMouseYawVelocity_0049d428 =
                (short)-horizontal;
            g_nFleetOverviewYawVelocity_0049d3ec =
                g_nFleetMouseYawVelocity_0049d428;
            g_nFleetMousePitchVelocity_0049d42c = vertical;
            g_nFleetOverviewPitchVelocity_0049d3f0 =
                g_nFleetMousePitchVelocity_0049d42c;
            break;
        case 7:
            g_nCockpitControlState_0049d7ac = 0;
            if (g_nTargetCameraFrame_0049d3e8 == 0) {
                g_nFleetOverviewYawVelocity_0049d3ec =
                    (short)-g_stCurrentFlightInput_0049d7b0.x;
                g_nFleetOverviewPitchVelocity_0049d3f0 =
                    (short)-g_stCurrentFlightInput_0049d7b0.y;
            }
            break;
        case 1:
            g_nCockpitControlState_0049d7ac = 0;
            g_cCurrentKey_00493128 = 0x39;
            fired = 1;
            break;
        case 5:
            g_nCockpitControlState_0049d7ac = 0;
            g_nFleetOverviewYawVelocity_0049d3ec = 0;
            g_nFleetOverviewPitchVelocity_0049d3f0 =
                g_nFleetOverviewYawVelocity_0049d3ec;
            break;
        case 6:
            result = 1;
        case 4:
            g_nCockpitControlState_0049d7ac = 0;
            g_nCockpitControlGoal_0049d7d0 = 0;
            g_cCurrentKey_00493128 = (signed char)event.status;
            notRepeated =
                g_cPreviousKey_0049312c != g_cCurrentKey_00493128 &&
                result == 0;
            pauseModifiers =
                (short)(g_wInputDeviceButtonState_005c8432 & 0x1800);
            unusedModifiers =
                (short)(g_wInputDeviceButtonState_005c8432 & 0x600);
            switch ((signed char)g_cCurrentKey_00493128) {
            case 0x1c:
            case 0x39:
                g_cCurrentKey_00493128 = 0x39;
                fired = 1;
                break;
            case 0x50:
                if (g_nFleetOverviewPitchVelocity_0049d3f0 > -8)
                    g_nFleetOverviewPitchVelocity_0049d3f0--;
                break;
            case 0x48:
                if (g_nFleetOverviewPitchVelocity_0049d3f0 < 8)
                    g_nFleetOverviewPitchVelocity_0049d3f0++;
                break;
            case 0x4b:
                if (g_nFleetOverviewYawVelocity_0049d3ec < 8)
                    g_nFleetOverviewYawVelocity_0049d3ec++;
                break;
            case 0x4d:
                if (g_nFleetOverviewYawVelocity_0049d3ec > -8)
                    g_nFleetOverviewYawVelocity_0049d3ec--;
                break;
            case 0x26:
                if (g_cPreviousKey_0049312c !=
                    g_cCurrentKey_00493128) {
                    g_bTargetLockMode_00493500 =
                        (short)(g_bTargetLockMode_00493500 == 0);
                    PlaySfxWaveFileByNumber(0x19, -1, 0);
                }
                break;
            case 0x14:
                if (g_cPreviousKey_0049312c !=
                    g_cCurrentKey_00493128)
                    cycle_onscreen_targets();
                break;
            case 0x22:
                ClearTargetCameraView();
            case 0x3b:
            case 0x3c:
            case 0x3d:
            case 0x3e:
            case 0x3f:
            case 0x40:
            case 0x41:
                break;
            case 0x19:
                ShowGamePausedBanner(pauseModifiers < 1);
            default:
                g_cCurrentKey_00493128 = (signed char)0x80;
                break;
            }
            break;
        }
    }

    if (g_nTargetCameraMode_005c8d50 == 0) {
        if ((g_cCurrentKey_00493128 & 0x80) != 0 &&
            (g_stCurrentFlightInput_0049d7b0.buttons & 1) != 0)
            g_cCurrentKey_00493128 = 0x39;
    } else if ((g_cCurrentKey_00493128 & 0x80) != 0 &&
               (g_stCurrentFlightInput_0049d7b0.buttons & 1) != 0 &&
               (g_stPreviousFlightInput_005c57d0.buttons & 1) == 0) {
        g_cCurrentKey_00493128 = 0x39;
    }
    if (g_nTargetCameraMode_005c8d50 == 0 &&
        (g_cCurrentKey_00493128 & 0x80) != 0 &&
        (g_wPendingInputButtons_005c80d4 & 1) != 0)
        g_cCurrentKey_00493128 = 0x39;
    if (g_cPreviousKey_0049312c != g_cCurrentKey_00493128 &&
        g_cCurrentKey_00493128 == 0x39) {
        if (g_nTargetCameraMode_005c8d50 == 0) {
            FireTargetCameraGuns();
        } else if (g_nTargetCameraZoom_0049d3e4 >= 0x21 &&
                   g_nTargetCameraFrame_0049d3e8 == 0) {
            ToggleTargetCameraTracking();
        }
        g_cCurrentKey_00493128 = (signed char)0x80;
    }
    if (g_nTargetCameraFrame_0049d3e8 == 1 &&
        (g_nCurrentView_00492fa8 != 4 ||
         g_nTargetCameraMode_005c8d50 != 1 || fired == 0))
        ClearTargetCameraView();
    g_stPreviousFlightInput_005c57d0 =
        g_stCurrentFlightInput_0049d7b0;
    if (g_nTargetCameraGunCooldown_0049d33c > 0)
        g_nTargetCameraGunCooldown_0049d33c--;
    else
        g_nTargetCameraGunCooldown_0049d33c = 0;
    return result;
}

/* Function start: 0x45F200 */
void UpdateFleetOverviewCameraRotation(void)
{
    char yawRotated;

    yawRotated = 0;
    if (g_nFleetOverviewYawVelocity_0049d3ec != 0 &&
        ((g_nFleetOverviewYaw_0049d3f4 < 65 &&
          g_nFleetOverviewYaw_0049d3f4 > -65) ||
         (g_nFleetOverviewYaw_0049d3f4 == 65 &&
          g_nFleetOverviewYawVelocity_0049d3ec < 0) ||
         (g_nFleetOverviewYaw_0049d3f4 == -65 &&
          g_nFleetOverviewYawVelocity_0049d3ec > 0))) {
        if (g_nFleetOverviewYaw_0049d3f4 +
                g_nFleetOverviewYawVelocity_0049d3ec > 65)
            g_nFleetOverviewYawVelocity_0049d3ec =
                (short)(65 - g_nFleetOverviewYaw_0049d3f4);
        else if (g_nFleetOverviewYaw_0049d3f4 +
                     g_nFleetOverviewYawVelocity_0049d3ec < -65)
            g_nFleetOverviewYawVelocity_0049d3ec =
                (short)(-65 - g_nFleetOverviewYaw_0049d3f4);
        rotate_about_j(
            g_nFleetOverviewYawVelocity_0049d3ec,
            &g_aShipRightVector_00493b78[EYE_OBJECT],
            &g_aShipForwardVector_00494208[EYE_OBJECT]);
        g_nFleetOverviewYaw_0049d3f4 =
            (short)(g_nFleetOverviewYaw_0049d3f4 +
                    g_nFleetOverviewYawVelocity_0049d3ec);
        yawRotated++;
    }
    if (g_nFleetOverviewPitchVelocity_0049d3f0 != 0 &&
        yawRotated == 0 &&
        ((g_nFleetOverviewPitch_0049d3f8 < 65 &&
          g_nFleetOverviewPitch_0049d3f8 > -65) ||
         (g_nFleetOverviewPitch_0049d3f8 >= 65 &&
          g_nFleetOverviewPitchVelocity_0049d3f0 < 0) ||
         (g_nFleetOverviewPitch_0049d3f8 <= -65 &&
          g_nFleetOverviewPitchVelocity_0049d3f0 > 0))) {
        if (g_nFleetOverviewPitch_0049d3f8 +
                g_nFleetOverviewPitchVelocity_0049d3f0 > 65)
            g_nFleetOverviewPitchVelocity_0049d3f0 =
                (short)(65 - g_nFleetOverviewPitch_0049d3f8);
        else if (g_nFleetOverviewPitch_0049d3f8 +
                     g_nFleetOverviewPitchVelocity_0049d3f0 < -65)
            g_nFleetOverviewPitchVelocity_0049d3f0 =
                (short)(-65 - g_nFleetOverviewPitch_0049d3f8);
        rotate_about_i(
            g_nFleetOverviewPitchVelocity_0049d3f0,
            &g_aShipUpVector_00493ec0[EYE_OBJECT],
            &g_aShipForwardVector_00494208[EYE_OBJECT]);
        g_nFleetOverviewPitch_0049d3f8 =
            (short)(g_nFleetOverviewPitch_0049d3f8 +
                    g_nFleetOverviewPitchVelocity_0049d3f0);
    }
    fix_objects_ijk(EYE_OBJECT);
}

/* Function start: 0x45F439 */
void FireTargetCameraGuns(void)
{
    short projectileSpeed;
    short projectileType;
    short shot;
    short projectile;
    ObjectTypeData *projectileData;
    FixedVector direction;
    short leadDistance;

    projectileSpeed = 10;
    projectileType = 8;
    if (g_nGunDisplayEnergyPercent_005c8d4e <= 0 ||
        g_nTargetCameraGunCooldown_0049d33c > 0)
        return;
    for (shot = 0; shot < 2; shot++) {
            projectile = new_object(projectileType, 0);
            if (projectile != -1) {
                g_abProjectileCollisionBonus_004960a8[projectile] = 1;
                projectileData =
                    &g_aObjectTypeData_00496d30[projectileType];
                copy_frame(EYE_OBJECT, projectile);
                g_asObjectDamage_00495178[projectile] =
                    projectileData->damageCapacity;
                projectileSpeed = (short)(
                    g_aObjectTypeData_00496d30[
                        g_acObjectType_00493980[projectile]]
                        .maximumVelocity * 2);
                g_nGunDisplayEnergyPercent_005c8d4e =
                    (short)(g_nGunDisplayEnergyPercent_005c8d4e -
                            projectileData->animationDelay / 2);
                g_aShipPosition_00494550[projectile].x =
                    g_aaTargetCameraGunOffsets_0049d3c8[
                        g_nGunDisplayIndex_005c8dc0][shot].y *
                        g_aShipUpVector_00493ec0[EYE_OBJECT].x +
                    g_aaTargetCameraGunOffsets_0049d3c8[
                        g_nGunDisplayIndex_005c8dc0][shot].z *
                        g_aShipForwardVector_00494208[
                            EYE_OBJECT].x +
                    g_aaTargetCameraGunOffsets_0049d3c8[
                        g_nGunDisplayIndex_005c8dc0][shot].x *
                        g_aShipRightVector_00493b78[
                            EYE_OBJECT].x +
                    g_aShipPosition_00494550[EYE_OBJECT].x;
                g_aShipPosition_00494550[projectile].y =
                    g_aaTargetCameraGunOffsets_0049d3c8[
                        g_nGunDisplayIndex_005c8dc0][shot].y *
                        g_aShipUpVector_00493ec0[EYE_OBJECT].y +
                    g_aaTargetCameraGunOffsets_0049d3c8[
                        g_nGunDisplayIndex_005c8dc0][shot].z *
                        g_aShipForwardVector_00494208[
                            EYE_OBJECT].y +
                    g_aaTargetCameraGunOffsets_0049d3c8[
                        g_nGunDisplayIndex_005c8dc0][shot].x *
                        g_aShipRightVector_00493b78[
                            EYE_OBJECT].y +
                    g_aShipPosition_00494550[EYE_OBJECT].y;
                g_aShipPosition_00494550[projectile].z =
                    g_aaTargetCameraGunOffsets_0049d3c8[
                        g_nGunDisplayIndex_005c8dc0][shot].y *
                        g_aShipUpVector_00493ec0[EYE_OBJECT].z +
                    g_aaTargetCameraGunOffsets_0049d3c8[
                        g_nGunDisplayIndex_005c8dc0][shot].z *
                        g_aShipForwardVector_00494208[
                            EYE_OBJECT].z +
                    g_aaTargetCameraGunOffsets_0049d3c8[
                        g_nGunDisplayIndex_005c8dc0][shot].x *
                        g_aShipRightVector_00493b78[
                            EYE_OBJECT].z +
                    g_aShipPosition_00494550[EYE_OBJECT].z;
                g_asObjectCounter_00494be0[projectile] =
                    (short)g_aObjectTypeData_00496d30[
                        projectileType].lifetime;
                zero_vector(&g_aShipVelocity_00494898[projectile]);
                direction =
                    g_aShipForwardVector_00494208[EYE_OBJECT];
                NormalizeFixedVector(&direction);
                leadDistance = (short)(
                    (projectileData->lifetime + 5) *
                    projectileData->maximumVelocity);
                ScaleFixedVector(
                    &direction, (int)leadDistance << 8, &direction);
                AddFixedVectors(
                    &g_aShipPosition_00494550[EYE_OBJECT],
                    &direction, &direction);
                point_at(projectile, direction);
                ScaleFixedVector(
                    &g_aShipForwardVector_00494208[projectile],
                    (int)projectileSpeed << 8, &direction);
                g_aShipVelocity_00494898[projectile] = direction;
                RecordCannedSceneObjectEvent(projectile, 0);
                g_nTargetCameraGunCooldown_0049d33c =
                    (short)(g_acGunRefireDelay_00492e1c[
                                projectileType - 7] >> 1);
                PlaySfxWaveFileByNumber(8, projectile, 0);
            }
    }
}

/* Function start: 0x45F7EC */
void UpdateShipTurretGuns(short ship)
{
    short projectileSpeed;
    short target;
    short gunSide;
    short turretCount;
    int turret;
    FixedVector gunRight;
    FixedVector gunUp;
    FixedVector gunForward;
    short candidate;
    int facing;
    FixedVector facingRight;
    ShortVector gunOffset;
    FixedVector gunPosition;
    short projectileType;
    FixedVector aimJitter;
    short shot;
    short projectile;
    ObjectTypeData *projectileData;
    FixedVector aimPoint;
    short leadDistance;

    projectileSpeed = 10;
    target = -1;
    gunSide = 0;
    turretCount = CountShipCockpitGunDisplays(ship);
    if (turretCount == 0)
        return;

    for (turret = 0; turret < turretCount; turret++) {
#ifdef SDL_PORT
        /* These tests choose a firing chance by the firing ship's side.
         * Reset every turret: new_object replaces this local with a
         * projectile index (or -1) after the preceding turret fires. */
        projectile = ship;
#endif
        if (ship == 0 && g_nCurrentView_00492fa8 == 4 &&
            turret == g_nTargetCameraOverlayMode_005c8db8)
            continue;
        if (g_asShipSide_004955d0[projectile] == SIDE_IMPERIAL &&
            (unsigned short)RandomInRange(0, 10) < 8)
            continue;
        if (g_asShipSide_004955d0[projectile] == SIDE_KILRATHI &&
            (unsigned short)RandomInRange(0, 10) < 5)
            continue;
        if (g_aasGunCooldown_005c8d70[ship][turret] > 0)
            continue;

        switch (turret) {
        case 0:
            gunRight = g_aShipRightVector_00493b78[ship];
            negate_vector(&gunRight);
            gunUp = g_aShipUpVector_00493ec0[ship];
            gunForward = g_aShipForwardVector_00494208[ship];
            negate_vector(&gunForward);
            gunSide = 0;
            break;
        case 1:
            gunRight = g_aShipRightVector_00493b78[ship];
            gunUp = g_aShipUpVector_00493ec0[ship];
            gunForward = g_aShipForwardVector_00494208[ship];
            negate_vector(&gunForward);
            gunSide = 1;
            break;
        case 2:
            gunRight = g_aShipRightVector_00493b78[ship];
            negate_vector(&gunRight);
            gunUp = g_aShipUpVector_00493ec0[ship];
            gunForward = g_aShipForwardVector_00494208[ship];
            gunSide = 1;
            break;
        }

        NormalizeFixedVector(&gunRight);
        NormalizeFixedVector(&gunUp);
        NormalizeFixedVector(&gunForward);
        for (candidate = 0; candidate < 10; candidate++) {
            if (g_aeObjectClass_00495328[candidate] ==
                    OBJECT_CLASS_SHIP &&
                g_asShipSide_004955d0[candidate] !=
                    g_asShipSide_004955d0[ship] &&
                g_anShipCloakState_00496020[candidate] != 1) {
                g_nTargetRange_0049319c = (short)(
                    distance_from_point(
                        ship, &g_aShipPosition_00494550[candidate]) -
                    g_asObjectCollisionRadius_004950e8[ship]);
                g_vNormalizedToTarget_005d3bd0 =
                    g_vToTarget_00493188;
                NormalizeFixedVector(&g_vNormalizedToTarget_005d3bd0);
                if (turret == 2) {
                    facing = dot_product(
                        &g_vNormalizedToTarget_005d3bd0,
                        &g_aShipRightVector_00493b78[ship]);
                } else if (turret == 1) {
                    facingRight = g_aShipRightVector_00493b78[ship];
                    negate_vector(&facingRight);
                    facing = dot_product(
                        &g_vNormalizedToTarget_005d3bd0, &facingRight);
                } else {
                    facing = dot_product(
                        &g_vNormalizedToTarget_005d3bd0, &gunForward);
                }
                g_nFacingToTarget_00493194 =
                    (short)(((short)facing * 100) >> 8);
                if (g_nFacingToTarget_00493194 > 85 &&
                    g_nTargetRange_0049319c < 4000)
                    target = candidate;
            }
        }
        if (target == -1)
            continue;

        gunOffset = g_aTargetCameraEyeOffsets_0049d430[turret];
        gunPosition.x =
            g_aShipUpVector_00493ec0[ship].x * gunOffset.y +
            g_aShipForwardVector_00494208[ship].x * gunOffset.z +
            g_aShipRightVector_00493b78[ship].x * gunOffset.x +
            g_aShipPosition_00494550[ship].x;
        gunPosition.y =
            g_aShipUpVector_00493ec0[ship].y * gunOffset.y +
            g_aShipForwardVector_00494208[ship].y * gunOffset.z +
            g_aShipRightVector_00493b78[ship].y * gunOffset.x +
            g_aShipPosition_00494550[ship].y;
        gunPosition.z =
            g_aShipUpVector_00493ec0[ship].z * gunOffset.y +
            g_aShipRightVector_00493b78[ship].z * gunOffset.x +
            g_aShipForwardVector_00494208[ship].z * gunOffset.z +
            g_aShipPosition_00494550[ship].z;
        projectileType = 8;
#ifdef SDL_PORT
        /* Retail's SHL converts signed random offsets to fixed point. */
        aimJitter.x =
            ((unsigned short)RandomInRange(0, 500) - 250) * 256;
        aimJitter.y =
            ((unsigned short)RandomInRange(0, 500) - 250) * 256;
        aimJitter.z =
            ((unsigned short)RandomInRange(0, 500) - 250) * 256;
#else
        aimJitter.x =
            ((unsigned short)RandomInRange(0, 500) - 250) << 8;
        aimJitter.y =
            ((unsigned short)RandomInRange(0, 500) - 250) << 8;
        aimJitter.z =
            ((unsigned short)RandomInRange(0, 500) - 250) << 8;
#endif

        for (shot = 0; shot < 2; shot++) {
            projectile = new_object(projectileType, ship);
            if (projectile != -1) {
                projectileData =
                    &g_aObjectTypeData_00496d30[projectileType];
                g_aShipRightVector_00493b78[projectile] = gunRight;
                g_aShipUpVector_00493ec0[projectile] = gunUp;
                g_aShipForwardVector_00494208[projectile] = gunForward;
                g_asObjectDamage_00495178[projectile] =
                    projectileData->damageCapacity;
                projectileSpeed =
                    g_aObjectTypeData_00496d30[
                        g_acObjectType_00493980[projectile]]
                        .maximumVelocity;
                g_aShipPosition_00494550[projectile].x =
                    g_aaTurretProjectileOffsets_0049d448[gunSide][shot].y *
                        gunUp.x +
                    g_aaTurretProjectileOffsets_0049d448[gunSide][shot].z *
                        gunForward.x +
                    g_aaTurretProjectileOffsets_0049d448[gunSide][shot].x *
                        gunRight.x +
                    gunPosition.x;
                g_aShipPosition_00494550[projectile].y =
                    g_aaTurretProjectileOffsets_0049d448[gunSide][shot].y *
                        gunUp.y +
                    g_aaTurretProjectileOffsets_0049d448[gunSide][shot].z *
                        gunForward.y +
                    g_aaTurretProjectileOffsets_0049d448[gunSide][shot].x *
                        gunRight.y +
                    gunPosition.y;
                g_aShipPosition_00494550[projectile].z =
                    g_aaTurretProjectileOffsets_0049d448[gunSide][shot].z *
                        gunForward.z +
                    g_aaTurretProjectileOffsets_0049d448[gunSide][shot].y *
                        gunUp.z +
                    g_aaTurretProjectileOffsets_0049d448[gunSide][shot].x *
                        gunRight.z +
                    gunPosition.z;
                g_asObjectCounter_00494be0[projectile] =
                    projectileData->lifetime;
                zero_vector(&g_aShipVelocity_00494898[projectile]);
                aimPoint = gunForward;
                NormalizeFixedVector(&aimPoint);
                leadDistance = (short)(
                    (projectileData->lifetime + 5) *
                    projectileData->maximumVelocity);
                ScaleFixedVector(
                    &aimPoint, (int)leadDistance << 8, &aimPoint);
                aimPoint = g_aShipPosition_00494550[target];
                AddFixedVectors(&aimJitter, &aimPoint, &aimPoint);
                point_at(projectile, aimPoint);
                ScaleFixedVector(
                    &g_aShipForwardVector_00494208[projectile],
                    (int)projectileSpeed << 8, &aimPoint);
                g_aShipVelocity_00494898[projectile] = aimPoint;
                RecordCannedSceneObjectEvent(projectile, 0);
                PlaySfxWaveFileByNumber(8, projectile, 0);
                g_aasGunCooldown_005c8d70[ship][turret] = 12;
                if (ship != 0 &&
                    g_asShipSide_004955d0[ship] == SIDE_IMPERIAL)
                    g_aasGunCooldown_005c8d70[ship][turret] =
                        (short)(
                            g_aasGunCooldown_005c8d70[ship][turret] +
                            6);
            }
        }
        if (ship != 0)
            return;
    }
}

/* Function start: 0x4601F1 */
void ResetTargetCameraView(void)
{
    g_nFleetOverviewYawVelocity_0049d3ec =
        g_nFleetOverviewPitchVelocity_0049d3f0 =
            g_nFleetOverviewYaw_0049d3f4 =
                g_nFleetOverviewPitch_0049d3f8 = 0;
    ResetCockpitPaletteEntries();
    ResetScannerContacts();
    g_nTargetCameraMode_005c8d50 = 1;
    ToggleTargetCameraOverlay();
}

/* Function start: 0x460242 */
short CountShipCockpitGunDisplays(short ship)
{
    short gunCount;
    short weapon;

    gunCount = 0;
    for (weapon = 0;
         weapon < (short)(signed char)g_aShipWeapons_004956b0[ship][0];
         weapon++) {
        if (((ShipWeaponSlot *)&g_aShipWeapons_004956b0[ship][1])[
                weapon].type == 0x0b)
            gunCount++;
    }
    return gunCount;
}

/* Function start: 0x4602BE */
void ToggleTargetCameraOverlay(void)
{
    if (g_nTargetCameraMode_005c8d50 == 0) {
        g_nTargetCameraZoom_0049d3e4 = 0x21;
        g_nTargetCameraFrame_0049d3e8 = 0;
        g_nTargetCameraMode_005c8d50 = 1;
        g_acShipTarget_00495f20[0] = -1;
        DrawSpriteDefault(
            &g_stScreenViewport_005d21a0,
            ((CockpitTargetCameraOverlay *)
                 g_abGunDisplayConfiguration_0049d340)
                [g_nGunDisplayIndex_005c8dc0].enabledX,
            ((CockpitTargetCameraOverlay *)
                 g_abGunDisplayConfiguration_0049d340)
                [g_nGunDisplayIndex_005c8dc0].enabledY,
            g_apCockpitVduOverlayShapes_0049a5f8[
                g_nGunDisplayIndex_005c8dc0],
            ((CockpitTargetCameraOverlay *)
                 g_abGunDisplayConfiguration_0049d340)
                [g_nGunDisplayIndex_005c8dc0].enabledFrame);
    } else {
        g_acShipTarget_00495f20[0] = -1;
        g_nTargetCameraZoom_0049d3e4 = 0;
        g_nTargetCameraFrame_0049d3e8 = 0;
        g_nTargetCameraMode_005c8d50 = 0;
        DrawSpriteDefault(
            &g_stScreenViewport_005d21a0,
            ((CockpitTargetCameraOverlay *)
                 g_abGunDisplayConfiguration_0049d340)
                [g_nGunDisplayIndex_005c8dc0].disabledX,
            ((CockpitTargetCameraOverlay *)
                 g_abGunDisplayConfiguration_0049d340)
                [g_nGunDisplayIndex_005c8dc0].disabledY,
            g_apCockpitVduOverlayShapes_0049a5f8[
                g_nGunDisplayIndex_005c8dc0],
            ((CockpitTargetCameraOverlay *)
                 g_abGunDisplayConfiguration_0049d340)
                [g_nGunDisplayIndex_005c8dc0].disabledFrame);
    }
}

/* Function start: 0x4603C5 */
void ClearTargetCameraView(void)
{
    FlushSoundEffectsAndLog(g_nTargetCameraSoundHandle_0049d3fc, 0);
    if (g_nTargetCameraObject_0049d338 != -1)
        zero_vector(&g_aShipVelocity_00494898[
            g_nTargetCameraObject_0049d338]);
    g_nTargetCameraSoundHandle_0049d3fc = 0;
    if (g_nTargetCameraEffectObjectA_0049d468 != -1)
        remove_object(g_nTargetCameraEffectObjectA_0049d468);
    if (g_nTargetCameraEffectObjectB_0049d46c != -1)
        remove_object(g_nTargetCameraEffectObjectB_0049d46c);
    g_nTargetCameraEffectObjectA_0049d468 = -1;
    g_nTargetCameraEffectObjectB_0049d46c = -1;
    g_nTargetCameraFrame_0049d3e8 = 0;
    g_nTargetCameraZoom_0049d3e4 = 0;
    g_nTargetCameraObject_0049d338 = -1;
    g_acShipTarget_00495f20[0] = -1;
    g_nFleetOverviewYawVelocity_0049d3ec = 0;
    g_nFleetOverviewPitchVelocity_0049d3f0 = 0;
}

/* Function start: 0x460498 */
void ToggleTargetCameraTracking(void)
{
    if (g_nTargetCameraFrame_0049d3e8 == 1) {
        ClearTargetCameraView();
    } else if (g_acShipTarget_00495f20[0] != -1) {
        g_nTargetCameraObject_0049d338 =
            (short)g_acShipTarget_00495f20[0];
        if (g_bHighMemoryResourcesEnabled_005c80e4 != 0 &&
            g_aObjectTypeData_00496d30[
                g_nObjectType62Index_00492d64].shapeSet != 0 &&
            g_aObjectTypeData_00496d30[
                g_nObjectType63Index_00492d68].shapeSet != 0) {
            g_nTargetCameraEffectObjectA_0049d468 =
                find_vacant_3d_object();
            set_objects_data(
                g_nTargetCameraEffectObjectA_0049d468, 0x3e,
                g_nTargetCameraObject_0049d338, 0);
            g_aShipPosition_00494550[
                g_nTargetCameraEffectObjectA_0049d468] =
                g_aShipPosition_00494550[
                    g_nTargetCameraObject_0049d338];
            g_aShipPosition_00494550[
                g_nTargetCameraEffectObjectA_0049d468].z -= 0x6400;
            g_asObjectViewFrame_00493508[
                g_nTargetCameraEffectObjectA_0049d468] = 0;

            g_nTargetCameraEffectObjectB_0049d46c =
                find_vacant_3d_object();
            set_objects_data(
                g_nTargetCameraEffectObjectB_0049d46c, 0x3f,
                g_nTargetCameraObject_0049d338, 0);
            g_aShipPosition_00494550[
                g_nTargetCameraEffectObjectB_0049d46c] =
                g_aShipPosition_00494550[
                    g_nTargetCameraObject_0049d338];
            g_aShipPosition_00494550[
                g_nTargetCameraEffectObjectB_0049d46c].z += 0x6400;
            g_asObjectViewFrame_00493508[
                g_nTargetCameraEffectObjectB_0049d46c] = 0;
        }
        /* The original's precedence slip: (!frame) & 7, not !(frame & 7). */
        if (!g_nSpaceFrame_00493134 & 7)
            PlaySfxWaveFileByNumber(0x26, -1, 0);
        g_nTargetCameraFrame_0049d3e8 = 1;
        g_nTargetCameraZoom_0049d3e4 = 100;
    }
}

/* Function start: 0x460650 */
void UpdateTargetCameraObject(short obj)
{
    long speed;
    short objective;

    if ((g_nSpaceFrame_00493134 & 7) == 0) {
        FlushSoundEffectsAndLog(g_nTargetCameraSoundHandle_0049d3fc, 0);
        g_nTargetCameraSoundHandle_0049d3fc = 0;
        PlaySfxWaveFileByNumber(0x26, -1, 0);
    }
    zero_vector(&g_aShipVelocity_00494898[obj]);
    point_ship_at_object(obj, 0);
    trim_goals(obj, 10);
    g_vTargetCameraForce_005c8d60.x = g_aShipForwardVector_00494208[EYE_OBJECT].x;
    g_vTargetCameraForce_005c8d60.y = g_aShipForwardVector_00494208[EYE_OBJECT].y;
    g_vTargetCameraForce_005c8d60.z = g_aShipForwardVector_00494208[EYE_OBJECT].z;
    negate_vector(&g_vTargetCameraForce_005c8d60);
    ScaleFixedVector(&g_vTargetCameraForce_005c8d60, 0x6400,
                     &g_vTargetCameraForce_005c8d60);
    apply_force_to_objects_center(&g_vTargetCameraForce_005c8d60, obj);
    g_vTargetCameraForce_005c8d60.x = g_aShipVelocity_00494898[EYE_OBJECT].x;
    g_vTargetCameraForce_005c8d60.y = g_aShipVelocity_00494898[EYE_OBJECT].y;
    g_vTargetCameraForce_005c8d60.z = g_aShipVelocity_00494898[EYE_OBJECT].z;
    apply_force_to_objects_center(&g_vTargetCameraForce_005c8d60, obj);
    speed = Vector_magnitude(&g_aShipVelocity_00494898[obj]);
    if (speed > 0x6400)
        NormalizeAndScaleVector(&g_aShipVelocity_00494898[obj], 0x6400);
    if (distance_between_points(&g_aShipPosition_00494550[0],
                                &g_aShipPosition_00494550[obj]) < 500) {
        FlushSoundEffectsAndLog(g_nTargetCameraSoundHandle_0049d3fc, 0);
        g_nTargetCameraSoundHandle_0049d3fc = 0;
        if (g_pCampaignGlobals_00499c94 == 0)
            LoadTemporaryCampaignGlobals();
        if (g_asObjectType_00495298[obj] == 0x3d) {
            g_pCampaignGlobals_00499c94->field_0e = 0;
        } else {
            g_pCampaignGlobals_00499c94->field_0e = 1;
            if (g_bMissionDeathSequencePending_0049b720 != 0) {
                g_bMissionDeathSequencePending_0049b720 = 0;
                g_bMissionWingmanFlag_005c8dbe = 1;
            }
        }
        for (objective = 0;
             objective < (short)g_cMissionObjectiveCount_00493294;
             objective++) {
            if (g_aMissionObjectives_004932a8[objective].type == 6 &&
                g_aMissionObjectives_004932a8[objective].index ==
                    g_asShipMissionIndex_00495d00[obj])
                flag_reached(objective, 0);
        }
        remove_object(obj);
        g_aMissionShips_00492290[
            g_asShipMissionIndex_00495d00[obj]].state = 2;
        ClearTargetCameraView();
        ejection_sequence((short)(g_nPlayerShipType_00493464 + 0x32), 1);
    }
}

#pragma function(abs)
/* Function start: 0x4608E8 */
void UpdateTargetCameraTracking(void)
{
    short yawDeviation;
    short pitchDeviation;
    short yawOverflow;
    short excessPitch;
    short distanceScale;
    unsigned short distance;

    distanceScale = 0;
    excessPitch = distanceScale;
    yawOverflow = excessPitch;
    g_nFleetOverviewYawVelocity_0049d3ec = 0;
    if (g_asObjectScreenX_00493598[
            g_nTargetCameraObject_0049d338] > 1)
        g_nFleetOverviewYawVelocity_0049d3ec = -1;
    if (g_asObjectScreenX_00493598[
            g_nTargetCameraObject_0049d338] < -1)
        g_nFleetOverviewYawVelocity_0049d3ec = 1;
    g_nFleetOverviewPitchVelocity_0049d3f0 = 0;
    if (g_asObjectScreenY_00493628[
            g_nTargetCameraObject_0049d338] > 1)
        g_nFleetOverviewPitchVelocity_0049d3f0 = 1;
    if (g_asObjectScreenY_00493628[
            g_nTargetCameraObject_0049d338] < -1)
        g_nFleetOverviewPitchVelocity_0049d3f0 = -1;
    distance = (unsigned short)distance_from_object(
        0, g_nTargetCameraObject_0049d338);

    if (g_nFleetOverviewYawVelocity_0049d3ec > 65 ||
        g_nFleetOverviewYawVelocity_0049d3ec < -65) {
        ClearTargetCameraView();
        zero_vector(&g_aShipVelocity_00494898[
            g_nTargetCameraObject_0049d338]);
        return;
    }
    if (g_nFleetOverviewPitchVelocity_0049d3f0 > 65 ||
        g_nFleetOverviewPitchVelocity_0049d3f0 < -65) {
        ClearTargetCameraView();
        zero_vector(&g_aShipVelocity_00494898[
            g_nTargetCameraObject_0049d338]);
        return;
    }
    if (distance > 4000) {
        ClearTargetCameraView();
        zero_vector(&g_aShipVelocity_00494898[
            g_nTargetCameraObject_0049d338]);
        return;
    }

    yawDeviation = (short)(abs((int)g_nFleetOverviewYaw_0049d3f4) - 17);
    pitchDeviation = (short)(abs((int)g_nFleetOverviewPitch_0049d3f8) - 17);
    if (yawDeviation > 0)
        yawOverflow = yawDeviation;
    if (pitchDeviation > 0)
        excessPitch = pitchDeviation;
    distanceScale = (short)(distance >> 6);
    g_nTargetCameraZoom_0049d3e4 = 100;
    if (excessPitch < yawOverflow && distanceScale < yawOverflow)
        g_nTargetCameraZoom_0049d3e4 =
            (short)(g_nTargetCameraZoom_0049d3e4 - yawOverflow);
    if (excessPitch > yawOverflow && distanceScale < excessPitch)
        g_nTargetCameraZoom_0049d3e4 =
            (short)(g_nTargetCameraZoom_0049d3e4 - excessPitch);
    if (distanceScale > yawOverflow && distanceScale > excessPitch)
        g_nTargetCameraZoom_0049d3e4 =
            (short)(g_nTargetCameraZoom_0049d3e4 - distanceScale);
    if (g_nTargetCameraZoom_0049d3e4 > 100)
        g_nTargetCameraZoom_0049d3e4 = 100;
    if (g_nTargetCameraZoom_0049d3e4 < 67)
        g_nTargetCameraZoom_0049d3e4 = 67;
}

#pragma intrinsic(abs)
/* Function start: 0x460BAF */
void ClearSavedTorpedoTargetLock(void)
{
    g_bSavedPlayerTarget_0049d464 = 0;
    g_bSavedTargetLockAcquired_0049d488 = 0;
}

/* Function start: 0x460BCC */
void UpdateSavedTorpedoTargetLock(void)
{
    if (g_bSavedPlayerTarget_0049d464 != 0) {
        get_facing_range_from_object(0, g_nSavedPlayerTarget_0049d460);
        if (g_nFacingToTarget_00493194 < 100)
            point_ship_at_object(0, g_nSavedPlayerTarget_0049d460);
        if (g_nTargetRange_0049319c < 3000)
            approach_zero_speed(0);
        if ((g_nTargetLockMusicCooldown_005d1e78 -
                     g_nFacingToTarget_00493194 > 5 &&
                 g_nFacingToTarget_00493194 < 100) ||
            unactive(g_nSavedPlayerTarget_0049d460) != 0 ||
            g_nTargetRange_0049319c > 10000 ||
            g_acPlayerComponentDamage_00493470[5] >= 4) {
            ClearSavedTorpedoTargetLock();
            SetHudMessageText(
                "Torpedo Lock Lost",
                (unsigned short)g_abGamePaletteReservedColours_0049cb54[4],
                20);
            handle_collisions(0);
            return;
        } else {
            g_nTargetLockMusicCooldown_005d1e78 =
                g_nFacingToTarget_00493194;
            if (g_nTargetLockMusicCooldown_005d1e78 > 100)
                g_nTargetLockMusicCooldown_005d1e78 = 100;
            g_nSavedTargetLockCountdown_0049d484--;
            g_bSavedTargetLockAcquired_0049d488 = 0;
        }
        if (g_nSavedTargetLockCountdown_0049d484 == 0) {
            SetHudMessageText(
                "Torpedo Locked!",
                (unsigned short)g_abGamePaletteReservedColours_0049cb54[4],
                20);
            g_bSavedTargetLockAcquired_0049d488 = 1;
        }
    } else
        handle_collisions(0);
}

/* Function start: 0x460D3E */
void SetTargetCameraEyePosition(short cameraMode)
{
    ShortVector offset;

    offset = g_aTargetCameraEyeOffsets_0049d430[cameraMode];
    g_aShipPosition_00494550[EYE_OBJECT].x =
        offset.y * g_aShipUpVector_00493ec0[0].x +
        offset.z * g_aShipForwardVector_00494208[0].x +
        offset.x * g_aShipRightVector_00493b78[0].x +
        g_aShipPosition_00494550[0].x;
    g_aShipPosition_00494550[EYE_OBJECT].y =
        offset.y * g_aShipUpVector_00493ec0[0].y +
        offset.z * g_aShipForwardVector_00494208[0].y +
        offset.x * g_aShipRightVector_00493b78[0].y +
        g_aShipPosition_00494550[0].y;
    g_aShipPosition_00494550[EYE_OBJECT].z =
        offset.y * g_aShipUpVector_00493ec0[0].z +
        offset.z * g_aShipForwardVector_00494208[0].z +
        offset.x * g_aShipRightVector_00493b78[0].z +
        g_aShipPosition_00494550[0].z;
}

/* Function start: 0x460DFE */
void DrawTargetCameraAttitudeIndicators(unsigned short colour)
{
    CockpitScannerGeometry scanner;

    scanner = ((CockpitScannerGeometry *)(
        g_abGunDisplayConfiguration_0049d340 + 0x40))[
            g_nGunDisplayIndex_005c8dc0];
    if (g_nFleetOverviewYaw_0049d3f4 != 0) {
        if (g_nFleetOverviewYaw_0049d3f4 < 0)
            g_nTargetCameraYawIndicatorX_005c8d3e =
                (short)(scanner.centerX +
                        g_nFleetOverviewYaw_0049d3f4 * 14 / 65 + 14);
        else
            g_nTargetCameraYawIndicatorX_005c8d3e =
                (short)(scanner.centerX -
                        (g_nFleetOverviewYaw_0049d3f4 * -14 / 65 + 14));
        g_nTargetCameraYawIndicatorY_005c8d40 = scanner.centerY;
        g_asTargetCameraYawIndicatorBackground_005c8d48[0] =
            (short)GetViewportPixel(
                &g_stScreenViewport_005d21a0,
                g_nTargetCameraYawIndicatorX_005c8d3e,
                (short)(g_nTargetCameraYawIndicatorY_005c8d40 - 1));
        DrawViewportPixel(
            &g_stScreenViewport_005d21a0,
            g_nTargetCameraYawIndicatorX_005c8d3e,
            (short)(g_nTargetCameraYawIndicatorY_005c8d40 - 1),
            (short)colour);
        g_asTargetCameraYawIndicatorBackground_005c8d48[1] =
            (short)GetViewportPixel(
                &g_stScreenViewport_005d21a0,
                g_nTargetCameraYawIndicatorX_005c8d3e,
                g_nTargetCameraYawIndicatorY_005c8d40);
        DrawViewportPixel(
            &g_stScreenViewport_005d21a0,
            g_nTargetCameraYawIndicatorX_005c8d3e,
            g_nTargetCameraYawIndicatorY_005c8d40,
            (short)colour);
        g_asTargetCameraYawIndicatorBackground_005c8d48[2] =
            (short)GetViewportPixel(
                &g_stScreenViewport_005d21a0,
                g_nTargetCameraYawIndicatorX_005c8d3e,
                (short)(g_nTargetCameraYawIndicatorY_005c8d40 + 1));
        DrawViewportPixel(
            &g_stScreenViewport_005d21a0,
            g_nTargetCameraYawIndicatorX_005c8d3e,
            (short)(g_nTargetCameraYawIndicatorY_005c8d40 + 1),
            (short)colour);
    }
    if (g_nFleetOverviewPitch_0049d3f8 != 0) {
        if (g_nFleetOverviewPitch_0049d3f8 > 0)
            g_nTargetCameraPitchIndicatorY_005c8d6e =
                (short)(scanner.centerY +
                        g_nFleetOverviewPitch_0049d3f8 * -14 / 65 + 14);
        else
            g_nTargetCameraPitchIndicatorY_005c8d6e =
                (short)(scanner.centerY -
                        (g_nFleetOverviewPitch_0049d3f8 * 14 / 65 + 14));
        g_nTargetCameraPitchIndicatorX_005c8d6c = scanner.centerX;
        g_asTargetCameraPitchIndicatorBackground_005c8db0[0] =
            (short)GetViewportPixel(
                &g_stScreenViewport_005d21a0,
                (short)(g_nTargetCameraPitchIndicatorX_005c8d6c - 1),
                g_nTargetCameraPitchIndicatorY_005c8d6e);
        DrawViewportPixel(
            &g_stScreenViewport_005d21a0,
            (short)(g_nTargetCameraPitchIndicatorX_005c8d6c - 1),
            g_nTargetCameraPitchIndicatorY_005c8d6e,
            (short)colour);
        g_asTargetCameraPitchIndicatorBackground_005c8db0[1] =
            (short)GetViewportPixel(
                &g_stScreenViewport_005d21a0,
                g_nTargetCameraPitchIndicatorX_005c8d6c,
                g_nTargetCameraPitchIndicatorY_005c8d6e);
        DrawViewportPixel(
            &g_stScreenViewport_005d21a0,
            g_nTargetCameraPitchIndicatorX_005c8d6c,
            g_nTargetCameraPitchIndicatorY_005c8d6e,
            (short)colour);
        g_asTargetCameraPitchIndicatorBackground_005c8db0[2] =
            (short)GetViewportPixel(
                &g_stScreenViewport_005d21a0,
                (short)(g_nTargetCameraPitchIndicatorX_005c8d6c + 1),
                g_nTargetCameraPitchIndicatorY_005c8d6e);
        DrawViewportPixel(
            &g_stScreenViewport_005d21a0,
            (short)(g_nTargetCameraPitchIndicatorX_005c8d6c + 1),
            g_nTargetCameraPitchIndicatorY_005c8d6e,
            (short)colour);
    }
}

/* Function start: 0x4610BC */
void RestoreTargetCameraAttitudeIndicators(void)
{
    CockpitScannerGeometry scanner;
    short colour;

    colour = 0xfa;
    scanner = ((CockpitScannerGeometry *)(
        g_abGunDisplayConfiguration_0049d340 + 0x40))[
            g_nGunDisplayIndex_005c8dc0];
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        g_nTargetCameraYawIndicatorX_005c8d3e,
        (short)(g_nTargetCameraYawIndicatorY_005c8d40 - 1),
        g_asTargetCameraYawIndicatorBackground_005c8d48[0]);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        g_nTargetCameraYawIndicatorX_005c8d3e,
        g_nTargetCameraYawIndicatorY_005c8d40,
        g_asTargetCameraYawIndicatorBackground_005c8d48[1]);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        g_nTargetCameraYawIndicatorX_005c8d3e,
        (short)(g_nTargetCameraYawIndicatorY_005c8d40 + 1),
        g_asTargetCameraYawIndicatorBackground_005c8d48[2]);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        (short)(g_nTargetCameraPitchIndicatorX_005c8d6c - 1),
        g_nTargetCameraPitchIndicatorY_005c8d6e,
        g_asTargetCameraPitchIndicatorBackground_005c8db0[0]);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        g_nTargetCameraPitchIndicatorX_005c8d6c,
        g_nTargetCameraPitchIndicatorY_005c8d6e,
        g_asTargetCameraPitchIndicatorBackground_005c8db0[1]);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        (short)(g_nTargetCameraPitchIndicatorX_005c8d6c + 1),
        g_nTargetCameraPitchIndicatorY_005c8d6e,
        g_asTargetCameraPitchIndicatorBackground_005c8db0[2]);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        scanner.centerX, scanner.centerY, colour);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        (short)(scanner.centerX + 1), scanner.centerY, colour);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        (short)(scanner.centerX - 1), scanner.centerY, colour);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        scanner.centerX, (short)(scanner.centerY + 1), colour);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        (short)(scanner.centerX + 1),
        (short)(scanner.centerY + 1), colour);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        (short)(scanner.centerX - 1),
        (short)(scanner.centerY + 1), colour);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        scanner.centerX, (short)(scanner.centerY - 1), colour);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        (short)(scanner.centerX + 1),
        (short)(scanner.centerY - 1), colour);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        (short)(scanner.centerX - 1),
        (short)(scanner.centerY - 1), colour);
}

/* Function start: 0x4612C1 */
/* Nothing in the shipped executable calls this.  The fleet-overview copy of
 * determine_pilot_hand: picks the pilot-hand frame from the overview's yaw
 * and pitch velocities. */
void DetermineFleetOverviewPilotHand(void)
{
    short yaw;
    short pitch;

    yaw = g_nFleetOverviewYawVelocity_0049d3ec;
    pitch = g_nFleetOverviewPitchVelocity_0049d3f0;
    if (yaw > 0) {
        g_cPilotHandFrame_005d1c30 =
            (signed char)MinShort((short)(yaw + 8), 12);
        return;
    }
    if (yaw < 0) {
        g_cPilotHandFrame_005d1c30 =
            (signed char)MinShort((short)(4 - yaw), 8);
        return;
    }
    if (pitch > 0) {
        g_cPilotHandFrame_005d1c30 =
            (signed char)MinShort((short)(pitch + 12), 16);
        return;
    }
    if (pitch < 0) {
        g_cPilotHandFrame_005d1c30 =
            (signed char)MinShort((short)-pitch, 4);
        return;
    }
    g_cPilotHandFrame_005d1c30 = 0;
}

/* Function start: 0x46138D */
void DrawUnreferencedPilotHandFrame(Viewport *viewport, unsigned char *shape,
                       BarracksAnimationState *state)
{
    short x;
    short y;

    x = (short)(g_stPilotHandOrigin_0049af90.x -
                g_stCockpitViewport_005d2160.left);
    y = (short)(g_stPilotHandOrigin_0049af90.y -
                g_stCockpitViewport_005d2160.top);
    CopyViewportContents(&g_stPilotHandBackgroundViewport_005d2b40,
                         &g_stPilotHandViewport_005d2c70);
    DrawSpriteDefault(&g_stPilotHandViewport_005d2c70, x, y,
                      g_pPilotHandAnimationShape_005d2c64,
                      (short)(signed char)g_cPilotHandFrame_005d1c30);
    DrawSpriteDefault(
        &g_stPilotHandViewport_005d2c70,
        (short)(x + g_asPilotHandOffsets_0049aff8[
            (signed char)g_cPilotHandFrame_005d1c30 * 2]),
        (short)(y + g_asPilotHandOffsets_0049aff8[
            (signed char)g_cPilotHandFrame_005d1c30 * 2 + 1]),
        g_pPilotHandAnimationShape_005d2c64, 0x11);
    CopyViewportContents(&g_stPilotHandViewport_005d2c70,
                         &g_stCockpitViewport_005d2160);
    g_cLastPilotHandFrame_0049aff4 = g_cPilotHandFrame_005d1c30;
}

/* Function start: 0x461456 */
void UpdateTargetCameraCockpitHook(void)
{
    return;
}

/* Function start: 0x461466 */
void FinalizeTargetCameraViewHook(void)
{
    return;
}
