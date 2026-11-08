/*
 *  Fleet overview, target camera and turret guns.
 *
 *  Retail object 0x45E9F0-0x46147F (link order; see docs/ORDER.md).
 */
#define TURRET_C
#include "game.h"

static ShortVector g_aTargetCameraEyeOffsets[3]; /* 0x0049d430 */

static ShortVector g_aaTurretProjectileOffsets[2][2]; /* 0x0049d448 */

static short g_nTargetCameraGunCooldown; /* 0x0049d33c */

static short g_asFleetMouseYawThresholds[6]; /* 0x0049d400 */

static short g_asFleetMousePitchThresholds[6]; /* 0x0049d410 */

static signed char g_bFleetPolledInputActive; /* 0x0049d420 */

static signed char g_bFleetButtonInputActive; /* 0x0049d424 */

static short g_nFleetMouseYawVelocity; /* 0x0049d428 */

static short g_nFleetMousePitchVelocity; /* 0x0049d42c */

extern short g_bMapPrimaryInputQueued; /* 0x005c57dc */ /* main.c common */

extern short g_2_bFleetMouseMovePosted; /* 0x005c57ea */ /* main.c common */

#pragma function(abs)
/* Function start: 0x45E9F0 */
short HandleFleetOverviewInput(void)
{
    short absX;
    short scrollY;
    signed char arrowKeys;
    short otherKeys;
    InputEventState state;
    short eventType;
    short action;
    short yAbs;
    short deadZone;
    int firePressed;
    short pauseModifiers;
    short justPressed;
    short scrollX;

    action = 0;
    firePressed = 0;
    g_cPreviousKey = g_cCurrentKey;
    ServiceInputDevices(15);
    g_19_bPolledFlightInputQueued = IsInputEventQueued(INPUT_EVENT_JOYSTICK_MOVE);
    g_2_bFleetMouseMovePosted = IsInputEventQueued(INPUT_EVENT_MOUSE_MOVE);
    g_1_bDiscreteFlightInputWaiting =
        (short)(IsInputEventQueued(INPUT_EVENT_KEY_DOWN) |
                IsInputEventQueued(INPUT_EVENT_LEGACY_6));
    g_bMapPrimaryInputQueued = IsInputEventQueued(INPUT_EVENT_BUTTON_DOWN);
    arrowKeys = 0;
    if (g_2_bFleetMouseMovePosted == 0)
        g_cCurrentKey |= (signed char)0x80;
    if (g_1_bDiscreteFlightInputWaiting == 0) {
        g_cCurrentKey |= (signed char)0x80;
        if (g_19_bPolledFlightInputQueued == 0 &&
            g_nCockpitControlState == 0)
            g_bFleetPolledInputActive = 0;
    } else {
        arrowKeys = 1;
    }
    if (g_bMapPrimaryInputQueued == 0 &&
        g_6_stHostMouseTextBlock.primaryButton != 0) {
        g_cCurrentKey = 0x39;
        firePressed = 1;
    }
    if (g_bMapPrimaryInputQueued == 0)
        g_bFleetButtonInputActive = 0;

    while ((eventType = GetNextInputEvent(&state)) != INPUT_EVENT_NONE) {
        switch (eventType) {
        case INPUT_EVENT_MOUSE_MOVE:
            if (g_nCockpitControlState == 0) {
                SetPersonnelMousePosition(
                    (short)((g_27_stViewScratchInfo.left +
                             g_27_stViewScratchInfo.right) / 2),
                    (short)((g_27_stViewScratchInfo.top +
                             g_27_stViewScratchInfo.bottom) / 2));
                g_nCockpitControlState = 1;
                g_37_stPersonnelMousePointerState.frame = 2;
            }
            scrollX = (short)(
                state.x -
                (g_27_stViewScratchInfo.right -
                 g_27_stViewScratchInfo.left) / 2 + 1);
            scrollY = (short)(
                state.y -
                (g_27_stViewScratchInfo.bottom -
                 g_27_stViewScratchInfo.top) / 2);
            g_37_stPersonnelMousePointerState.x = state.x;
            g_37_stPersonnelMousePointerState.y = state.y;
            absX = (short)abs(scrollX);
            yAbs = (short)abs(scrollY);
            deadZone = 0;
            while (g_asFleetMouseYawThresholds[deadZone] <=
                   absX)
                deadZone++;
            if (scrollX < 0)
                scrollX = (short)-deadZone;
            else
                scrollX = deadZone;
            deadZone = 0;
            while (g_asFleetMousePitchThresholds[deadZone] <=
                   yAbs)
                deadZone++;
            if (scrollY < 0)
                scrollY = (short)-deadZone;
            else
                scrollY = deadZone;
            if ((int)state.x - 4 <=
                (int)g_27_stViewScratchInfo.left)
                scrollX = -8;
            if ((int)state.x + 4 >=
                (int)g_27_stViewScratchInfo.right)
                scrollX = 8;
            if ((int)state.y - 4 <=
                (int)g_27_stViewScratchInfo.top)
                scrollY = -8;
            if ((int)state.y + 4 >=
                (int)g_27_stViewScratchInfo.bottom)
                scrollY = 8;
            if (scrollX >= 9)
                scrollX = 8;
            if (scrollX <= -9)
                scrollX = -8;
            if (scrollY >= 9)
                scrollY = 8;
            if (scrollY <= -9)
                scrollY = -8;
            g_nFleetMouseYawVelocity =
                (short)-scrollX;
            g_nFleetOverviewYawVelocity =
                g_nFleetMouseYawVelocity;
            g_nFleetMousePitchVelocity = scrollY;
            g_nFleetOverviewPitchVelocity =
                g_nFleetMousePitchVelocity;
            break;
        case INPUT_EVENT_JOYSTICK_MOVE:
            g_nCockpitControlState = 0;
            if (g_nTargetCameraFrame == 0) {
                g_nFleetOverviewYawVelocity =
                    (short)-g_stCurrentFlightInput.x;
                g_nFleetOverviewPitchVelocity =
                    (short)-g_stCurrentFlightInput.y;
            }
            break;
        case INPUT_EVENT_BUTTON_DOWN:
            g_nCockpitControlState = 0;
            g_cCurrentKey = 0x39;
            firePressed = 1;
            break;
        case INPUT_EVENT_KEY_UP:
            g_nCockpitControlState = 0;
            g_nFleetOverviewYawVelocity = 0;
            g_nFleetOverviewPitchVelocity =
                g_nFleetOverviewYawVelocity;
            break;
        case INPUT_EVENT_LEGACY_6:
            action = 1;
        case INPUT_EVENT_KEY_DOWN:
            g_nCockpitControlState = 0;
            g_nCockpitControlGoal = 0;
            g_cCurrentKey = (signed char)state.status;
            justPressed =
                g_cPreviousKey != g_cCurrentKey &&
                action == 0;
            pauseModifiers =
                (short)((unsigned short)g_12_wInputDeviceButtonStatus & 0x1800);
            otherKeys =
                (short)((unsigned short)g_12_wInputDeviceButtonStatus & 0x600);
            switch ((signed char)g_cCurrentKey) {
            case 0x1c:
            case 0x39:
                g_cCurrentKey = 0x39;
                firePressed = 1;
                break;
            case 0x50:
                if (g_nFleetOverviewPitchVelocity > -8)
                    g_nFleetOverviewPitchVelocity--;
                break;
            case 0x48:
                if (g_nFleetOverviewPitchVelocity < 8)
                    g_nFleetOverviewPitchVelocity++;
                break;
            case 0x4b:
                if (g_nFleetOverviewYawVelocity < 8)
                    g_nFleetOverviewYawVelocity++;
                break;
            case 0x4d:
                if (g_nFleetOverviewYawVelocity > -8)
                    g_nFleetOverviewYawVelocity--;
                break;
            case 0x26:
                if (g_cPreviousKey !=
                    g_cCurrentKey) {
                    g_bTargetLockMode =
                        (short)(g_bTargetLockMode == 0);
                    PlaySfxWaveFileByNumber(0x19, -1, 0);
                }
                break;
            case 0x14:
                if (g_cPreviousKey !=
                    g_cCurrentKey)
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
                ShowGamePausedBanner(pauseModifiers == 0);
            default:
                g_cCurrentKey = (signed char)0x80;
                break;
            }
            break;
        }
    }

    if (g_nAimCameraMode == 0) {
        if ((g_cCurrentKey & 0x80) != 0 &&
            (g_stCurrentFlightInput.buttons & 1) != 0)
            g_cCurrentKey = 0x39;
    } else if ((g_cCurrentKey & 0x80) != 0 &&
               (g_stCurrentFlightInput.buttons & 1) != 0 &&
               (g_10_stPrevFlightControls.buttons & 1) == 0) {
        g_cCurrentKey = 0x39;
    }
    if (g_nAimCameraMode == 0 &&
        (g_cCurrentKey & 0x80) != 0 &&
        (g_6_wQueuedButtonsState & 1) != 0)
        g_cCurrentKey = 0x39;
    if (g_cPreviousKey != g_cCurrentKey &&
        g_cCurrentKey == 0x39) {
        if (g_nAimCameraMode == 0) {
            FireTargetCameraGuns();
        } else if (g_nTargetCameraZoom >= 0x21 &&
                   g_nTargetCameraFrame == 0) {
            ToggleTargetCameraTracking();
        }
        g_cCurrentKey = (signed char)0x80;
    }
    if (g_nTargetCameraFrame == 1 &&
        (g_nCurrentView != 4 ||
         g_nAimCameraMode != 1 || firePressed == 0))
        ClearTargetCameraView();
    g_10_stPrevFlightControls =
        g_stCurrentFlightInput;
    if (g_nTargetCameraGunCooldown > 0)
        g_nTargetCameraGunCooldown--;
    else
        g_nTargetCameraGunCooldown = 0;
    return action;
}

/* Function start: 0x45F200 */
void UpdateFleetOverviewCameraRotation(void)
{
    char yawRotated;

    yawRotated = 0;
    if (g_nFleetOverviewYawVelocity != 0 &&
        ((g_nFleetOverviewYaw < 65 &&
          g_nFleetOverviewYaw > -65) ||
         (g_nFleetOverviewYaw == 65 &&
          g_nFleetOverviewYawVelocity < 0) ||
         (g_nFleetOverviewYaw == -65 &&
          g_nFleetOverviewYawVelocity > 0))) {
        if (g_nFleetOverviewYaw +
                g_nFleetOverviewYawVelocity > 65)
            g_nFleetOverviewYawVelocity =
                (short)(65 - g_nFleetOverviewYaw);
        else if (g_nFleetOverviewYaw +
                     g_nFleetOverviewYawVelocity < -65)
            g_nFleetOverviewYawVelocity =
                (short)(-65 - g_nFleetOverviewYaw);
        rotate_about_j(
            g_nFleetOverviewYawVelocity,
            &g_aShipRightVector[EYE_OBJECT],
            &g_aShipForwardVector[EYE_OBJECT]);
        g_nFleetOverviewYaw =
            (short)(g_nFleetOverviewYaw +
                    g_nFleetOverviewYawVelocity);
        yawRotated++;
    }
    if (g_nFleetOverviewPitchVelocity != 0 &&
        yawRotated == 0 &&
        ((g_nFleetOverviewPitch < 65 &&
          g_nFleetOverviewPitch > -65) ||
         (g_nFleetOverviewPitch >= 65 &&
          g_nFleetOverviewPitchVelocity < 0) ||
         (g_nFleetOverviewPitch <= -65 &&
          g_nFleetOverviewPitchVelocity > 0))) {
        if (g_nFleetOverviewPitch +
                g_nFleetOverviewPitchVelocity > 65)
            g_nFleetOverviewPitchVelocity =
                (short)(65 - g_nFleetOverviewPitch);
        else if (g_nFleetOverviewPitch +
                     g_nFleetOverviewPitchVelocity < -65)
            g_nFleetOverviewPitchVelocity =
                (short)(-65 - g_nFleetOverviewPitch);
        rotate_about_i(
            g_nFleetOverviewPitchVelocity,
            &g_aShipUpVector[EYE_OBJECT],
            &g_aShipForwardVector[EYE_OBJECT]);
        g_nFleetOverviewPitch =
            (short)(g_nFleetOverviewPitch +
                    g_nFleetOverviewPitchVelocity);
    }
    fix_objects_ijk(EYE_OBJECT);
}

/* Function start: 0x45F439 */
void FireTargetCameraGuns(void)
{
    short type;
    short shot;
    short projectile;
    ObjectTypeData *projectileData;
    short projectileVelocity;
    FixedVector direction;
    short travel;

    projectileVelocity = 10;
    type = 8;
    if (g_1_nGunShowEnergyPercent <= 0 ||
        g_nTargetCameraGunCooldown > 0)
        return;
    for (shot = 0; shot < 2; shot++) {
            projectile = new_object(type, 0);
            if (projectile != -1) {
                g_abProjectileCollisionBonus[projectile] = 1;
                projectileData =
                    &g_aObjectTypeData[type];
                copy_frame(EYE_OBJECT, projectile);
                g_asObjectDamage[projectile] =
                    projectileData->damageCapacity;
                projectileVelocity = (short)(
                    g_aObjectTypeData[
                        g_acObjectType[projectile]]
                        .maximumVelocity * 2);
                g_1_nGunShowEnergyPercent =
                    (short)(g_1_nGunShowEnergyPercent -
                            projectileData->animationDelay / 2);
                /* The two gun offsets per display sit inside the gun
                 * display configuration record at +0x88. The ~~ identities
                 * make CL 4.1 emit the up term first, matching retail;
                 * neither complement survives instruction emission. */
                g_aShipPosition[projectile].x =
                    ~~((*(ShortVector (*)[2][2])(
                        g_abGunDisplayConfiguration + 0x88))[
                        g_8_nGunDisplaySlot][shot].y *
                        g_aShipUpVector[EYE_OBJECT].x) +
                    (*(ShortVector (*)[2][2])(
                        g_abGunDisplayConfiguration + 0x88))[
                        g_8_nGunDisplaySlot][shot].z *
                        g_aShipForwardVector[
                            EYE_OBJECT].x +
                    (*(ShortVector (*)[2][2])(
                        g_abGunDisplayConfiguration + 0x88))[
                        g_8_nGunDisplaySlot][shot].x *
                        g_aShipRightVector[
                            EYE_OBJECT].x +
                    g_aShipPosition[EYE_OBJECT].x;
                g_aShipPosition[projectile].y =
                    ~~((*(ShortVector (*)[2][2])(
                        g_abGunDisplayConfiguration + 0x88))[
                        g_8_nGunDisplaySlot][shot].y *
                        g_aShipUpVector[EYE_OBJECT].y) +
                    (*(ShortVector (*)[2][2])(
                        g_abGunDisplayConfiguration + 0x88))[
                        g_8_nGunDisplaySlot][shot].z *
                        g_aShipForwardVector[
                            EYE_OBJECT].y +
                    (*(ShortVector (*)[2][2])(
                        g_abGunDisplayConfiguration + 0x88))[
                        g_8_nGunDisplaySlot][shot].x *
                        g_aShipRightVector[
                            EYE_OBJECT].y +
                    g_aShipPosition[EYE_OBJECT].y;
                g_aShipPosition[projectile].z =
                    (*(ShortVector (*)[2][2])(
                        g_abGunDisplayConfiguration + 0x88))[
                        g_8_nGunDisplaySlot][shot].y *
                        g_aShipUpVector[EYE_OBJECT].z +
                    (*(ShortVector (*)[2][2])(
                        g_abGunDisplayConfiguration + 0x88))[
                        g_8_nGunDisplaySlot][shot].z *
                        g_aShipForwardVector[
                            EYE_OBJECT].z +
                    (*(ShortVector (*)[2][2])(
                        g_abGunDisplayConfiguration + 0x88))[
                        g_8_nGunDisplaySlot][shot].x *
                        g_aShipRightVector[
                            EYE_OBJECT].z +
                    g_aShipPosition[EYE_OBJECT].z;
                g_asObjectCounter[projectile] =
                    (short)g_aObjectTypeData[
                        type].lifetime;
                zero_vector(&g_aShipVelocity[projectile]);
                direction =
                    g_aShipForwardVector[EYE_OBJECT];
                NormalizeFixedVector(&direction);
                travel = (short)(
                    (projectileData->lifetime + 5) *
                    projectileData->maximumVelocity);
                ScaleFixedVector(
                    &direction, (int)travel << 8, &direction);
                AddFixedVectors(
                    &g_aShipPosition[EYE_OBJECT],
                    &direction, &direction);
                point_at(projectile, direction);
                ScaleFixedVector(
                    &g_aShipForwardVector[projectile],
                    (int)projectileVelocity << 8, &direction);
                g_aShipVelocity[projectile] = direction;
                RecordCannedSceneObjectEvent(projectile, 0);
                g_nTargetCameraGunCooldown =
                    (short)(g_acGunRefireDelay[
                                type - 7] >> 1);
                PlaySfxWaveFileByNumber(8, projectile, 0);
            }
    }
}

/* Function start: 0x45F7EC */
void UpdateShipTurretGuns(short ship)
{
    short victim;
    short whichSide;
    short aimLead;
    ObjectTypeData *projectileData;
    FixedVector front;
    short projectile;
    FixedVector spread;
    FixedVector aimPoint;
    short boltSpeed;
    int t;
    short round;
    int angle;
    ShortVector gunOffset;
    FixedVector gunLateral;
    short ammo;
    FixedVector gunUp;
    short candidate;
    short turrets;
    FixedVector gunOrigin;

    boltSpeed = 10;
    victim = -1;
    whichSide = 0;
    turrets = CountShipCockpitGunDisplays(ship);
    if (turrets == 0)
        return;

    for (t = 0; t < turrets; t++) {
#ifdef SDL_PORT
        /* These tests choose a firing chance by the firing ship's side.
         * Reset every t: new_object replaces this local with a
         * projectile index (or -1) after the preceding t fires. */
        projectile = ship;
#endif
        if (ship == 0 && g_nCurrentView == 4 &&
            t == g_9_nTargetCameraOverlayMode)
            continue;
        if (g_asShipSide[projectile] == SIDE_IMPERIAL &&
            (unsigned short)RandomInRange(0, 10) < 8)
            continue;
        if (g_asShipSide[projectile] == SIDE_KILRATHI &&
            (unsigned short)RandomInRange(0, 10) < 5)
            continue;
        if (g_27_aasGunCooldown[ship][t] > 0)
            continue;

        switch (t) {
        case 0:
            gunLateral = g_aShipRightVector[ship];
            negate_vector(&gunLateral);
            gunUp = g_aShipUpVector[ship];
            front = g_aShipForwardVector[ship];
            negate_vector(&front);
            whichSide = 0;
            break;
        case 1:
            gunLateral = g_aShipRightVector[ship];
            gunUp = g_aShipUpVector[ship];
            front = g_aShipForwardVector[ship];
            negate_vector(&front);
            whichSide = 1;
            break;
        case 2:
            gunLateral = g_aShipRightVector[ship];
            negate_vector(&gunLateral);
            gunUp = g_aShipUpVector[ship];
            front = g_aShipForwardVector[ship];
            whichSide = 1;
            break;
        }

        NormalizeFixedVector(&gunLateral);
        NormalizeFixedVector(&gunUp);
        NormalizeFixedVector(&front);
        for (candidate = 0; candidate < 10; candidate++) {
            if (g_aeObjectClass[candidate] ==
                    OBJECT_CLASS_SHIP &&
                g_asShipSide[candidate] !=
                    g_asShipSide[ship] &&
                g_anShipCloakState[candidate] != 1) {
                g_nTargetRange = (short)(
                    distance_from_point(
                        ship, &g_aShipPosition[candidate]) -
                    g_asObjectCollisionRadius[ship]);
                g_2_vNormalizedToTgt =
                    g_vToTarget;
                NormalizeFixedVector(&g_2_vNormalizedToTgt);
                if (t == 2) {
                    angle = dot_product(
                        &g_2_vNormalizedToTgt,
                        &g_aShipRightVector[ship]);
                } else if (t == 1) {
                    aimPoint = g_aShipRightVector[ship];
                    negate_vector(&aimPoint);
                    angle = dot_product(
                        &g_2_vNormalizedToTgt, &aimPoint);
                } else {
                    angle = dot_product(
                        &g_2_vNormalizedToTgt, &front);
                }
                g_nFacingToTarget =
                    (short)(((short)angle * 100) >> 8);
                if (g_nFacingToTarget > 85 &&
                    g_nTargetRange < 4000)
                    victim = candidate;
            }
        }
        if (victim == -1)
            continue;

        gunOffset = g_aTargetCameraEyeOffsets[t];
        gunOrigin.x =
            g_aShipUpVector[ship].x * gunOffset.y +
            g_aShipForwardVector[ship].x * gunOffset.z +
            g_aShipRightVector[ship].x * gunOffset.x +
            g_aShipPosition[ship].x;
        gunOrigin.y =
            g_aShipUpVector[ship].y * gunOffset.y +
            g_aShipForwardVector[ship].y * gunOffset.z +
            g_aShipRightVector[ship].y * gunOffset.x +
            g_aShipPosition[ship].y;
        gunOrigin.z =
            g_aShipUpVector[ship].z * gunOffset.y +
            g_aShipRightVector[ship].z * gunOffset.x +
            g_aShipForwardVector[ship].z * gunOffset.z +
            g_aShipPosition[ship].z;
        ammo = 8;
#ifdef SDL_PORT
        /* Retail's SHL converts signed random offsets to fixed point. */
        spread.x =
            ((unsigned short)RandomInRange(0, 500) - 250) * 256;
        spread.y =
            ((unsigned short)RandomInRange(0, 500) - 250) * 256;
        spread.z =
            ((unsigned short)RandomInRange(0, 500) - 250) * 256;
#else
        spread.x =
            ((unsigned short)RandomInRange(0, 500) - 250) << 8;
        spread.y =
            ((unsigned short)RandomInRange(0, 500) - 250) << 8;
        spread.z =
            ((unsigned short)RandomInRange(0, 500) - 250) << 8;
#endif

        for (round = 0; round < 2; round++) {
            projectile = new_object(ammo, ship);
            if (projectile != -1) {
                projectileData =
                    &g_aObjectTypeData[ammo];
                g_aShipRightVector[projectile] = gunLateral;
                g_aShipUpVector[projectile] = gunUp;
                g_aShipForwardVector[projectile] = front;
                g_asObjectDamage[projectile] =
                    projectileData->damageCapacity;
                boltSpeed =
                    g_aObjectTypeData[
                        g_acObjectType[projectile]]
                        .maximumVelocity;
                g_aShipPosition[projectile].x =
                    g_aaTurretProjectileOffsets[whichSide][round].y *
                        gunUp.x +
                    g_aaTurretProjectileOffsets[whichSide][round].z *
                        front.x +
                    g_aaTurretProjectileOffsets[whichSide][round].x *
                        gunLateral.x +
                    gunOrigin.x;
                g_aShipPosition[projectile].y =
                    g_aaTurretProjectileOffsets[whichSide][round].y *
                        gunUp.y +
                    g_aaTurretProjectileOffsets[whichSide][round].z *
                        front.y +
                    g_aaTurretProjectileOffsets[whichSide][round].x *
                        gunLateral.y +
                    gunOrigin.y;
                g_aShipPosition[projectile].z =
                    g_aaTurretProjectileOffsets[whichSide][round].z *
                        front.z +
                    g_aaTurretProjectileOffsets[whichSide][round].y *
                        gunUp.z +
                    g_aaTurretProjectileOffsets[whichSide][round].x *
                        gunLateral.z +
                    gunOrigin.z;
                g_asObjectCounter[projectile] =
                    (short)g_aObjectTypeData[ammo].lifetime;
                zero_vector(&g_aShipVelocity[projectile]);
                aimPoint = front;
                NormalizeFixedVector(&aimPoint);
                aimLead = (short)(
                    (projectileData->lifetime + 5) *
                    projectileData->maximumVelocity);
                ScaleFixedVector(
                    &aimPoint, (int)aimLead << 8, &aimPoint);
                aimPoint.x = g_aShipPosition[victim].x;
                aimPoint.y = g_aShipPosition[victim].y;
                aimPoint.z = g_aShipPosition[victim].z;
                AddFixedVectors(&spread, &aimPoint, &aimPoint);
                point_at(projectile, aimPoint);
                ScaleFixedVector(
                    &g_aShipForwardVector[projectile],
                    (int)boltSpeed << 8, &aimPoint);
                g_aShipVelocity[projectile].x = aimPoint.x;
                g_aShipVelocity[projectile].y = aimPoint.y;
                g_aShipVelocity[projectile].z = aimPoint.z;
                RecordCannedSceneObjectEvent(projectile, 0);
                PlaySfxWaveFileByNumber(8, projectile, 0);
                g_27_aasGunCooldown[ship][t] = 12;
                if (ship != 0 &&
                    g_asShipSide[ship] == SIDE_IMPERIAL)
                    g_27_aasGunCooldown[ship][t] =
                        (short)(
                            g_27_aasGunCooldown[ship][t] +
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
    g_nFleetOverviewYawVelocity =
        g_nFleetOverviewPitchVelocity =
            g_nFleetOverviewYaw =
                g_nFleetOverviewPitch = 0;
    ResetCockpitPaletteEntries();
    ResetScannerContacts();
    g_nAimCameraMode = 1;
    ToggleTargetCameraOverlay();
}

/* Function start: 0x460242 */
short CountShipCockpitGunDisplays(short ship)
{
    short gunCount;
    short weapon;

    gunCount = 0;
    for (weapon = 0;
         weapon < (short)(signed char)g_aShipWeapons[ship][0];
         weapon++) {
        if (((ShipWeaponSlot *)&g_aShipWeapons[ship][1])[
                weapon].type == 0x0b)
            gunCount++;
    }
    return gunCount;
}

/* Function start: 0x4602BE */
void ToggleTargetCameraOverlay(void)
{
    if (g_nAimCameraMode == 0) {
        g_nTargetCameraZoom = 0x21;
        g_nTargetCameraFrame = 0;
        g_nAimCameraMode = 1;
        g_acShipTarget[0] = -1;
        DrawSpriteDefault(
            &g_101_stScrViewInfo,
            ((CockpitTargetCameraOverlay *)
                 g_abGunDisplayConfiguration)
                [g_8_nGunDisplaySlot].enabledX,
            ((CockpitTargetCameraOverlay *)
                 g_abGunDisplayConfiguration)
                [g_8_nGunDisplaySlot].enabledY,
            g_apCockpitVduOverlayShapes[
                g_8_nGunDisplaySlot],
            ((CockpitTargetCameraOverlay *)
                 g_abGunDisplayConfiguration)
                [g_8_nGunDisplaySlot].enabledFrame);
    } else {
        g_acShipTarget[0] = -1;
        g_nTargetCameraZoom = 0;
        g_nTargetCameraFrame = 0;
        g_nAimCameraMode = 0;
        DrawSpriteDefault(
            &g_101_stScrViewInfo,
            ((CockpitTargetCameraOverlay *)
                 g_abGunDisplayConfiguration)
                [g_8_nGunDisplaySlot].disabledX,
            ((CockpitTargetCameraOverlay *)
                 g_abGunDisplayConfiguration)
                [g_8_nGunDisplaySlot].disabledY,
            g_apCockpitVduOverlayShapes[
                g_8_nGunDisplaySlot],
            ((CockpitTargetCameraOverlay *)
                 g_abGunDisplayConfiguration)
                [g_8_nGunDisplaySlot].disabledFrame);
    }
}

/* Function start: 0x4603C5 */
void ClearTargetCameraView(void)
{
    FlushSoundEffectsAndLog(g_nTargetCameraSoundHandle, 0);
    if (g_nTargetCameraObject != -1)
        zero_vector(&g_aShipVelocity[
            g_nTargetCameraObject]);
    g_nTargetCameraSoundHandle = 0;
    if (g_nTargetCameraEffectObjectA != -1)
        remove_object(g_nTargetCameraEffectObjectA);
    if (g_nTargetCameraEffectObjectB != -1)
        remove_object(g_nTargetCameraEffectObjectB);
    g_nTargetCameraEffectObjectA = -1;
    g_nTargetCameraEffectObjectB = -1;
    g_nTargetCameraFrame = 0;
    g_nTargetCameraZoom = 0;
    g_nTargetCameraObject = -1;
    g_acShipTarget[0] = -1;
    g_nFleetOverviewYawVelocity = 0;
    g_nFleetOverviewPitchVelocity = 0;
}

/* Function start: 0x460498 */
void ToggleTargetCameraTracking(void)
{
    if (g_nTargetCameraFrame == 1) {
        ClearTargetCameraView();
    } else if (g_acShipTarget[0] != -1) {
        g_nTargetCameraObject =
            (short)g_acShipTarget[0];
        if (g_19_bStoredHighMemoryEnabled != 0 &&
            g_aObjectTypeData[
                g_nObjectType62Index].shapeSet != 0 &&
            g_aObjectTypeData[
                g_nObjectType63Index].shapeSet != 0) {
            g_nTargetCameraEffectObjectA =
                find_vacant_3d_object();
            set_objects_data(
                g_nTargetCameraEffectObjectA, 0x3e,
                g_nTargetCameraObject, 0);
            g_aShipPosition[
                g_nTargetCameraEffectObjectA] =
                g_aShipPosition[
                    g_nTargetCameraObject];
            g_aShipPosition[
                g_nTargetCameraEffectObjectA].z -= 0x6400;
            g_asObjectViewFrame[
                g_nTargetCameraEffectObjectA] = 0;

            g_nTargetCameraEffectObjectB =
                find_vacant_3d_object();
            set_objects_data(
                g_nTargetCameraEffectObjectB, 0x3f,
                g_nTargetCameraObject, 0);
            g_aShipPosition[
                g_nTargetCameraEffectObjectB] =
                g_aShipPosition[
                    g_nTargetCameraObject];
            g_aShipPosition[
                g_nTargetCameraEffectObjectB].z += 0x6400;
            g_asObjectViewFrame[
                g_nTargetCameraEffectObjectB] = 0;
        }
        /* The original's precedence slip: (!frame) & 7, not !(frame & 7). */
        if (!g_nSpaceFrame & 7)
            PlaySfxWaveFileByNumber(0x26, -1, 0);
        g_nTargetCameraFrame = 1;
        g_nTargetCameraZoom = 100;
    }
}

/* Function start: 0x460650 */
void UpdateTargetCameraObject(short obj)
{
    long speed;
    short objective;

    if ((g_nSpaceFrame & 7) == 0) {
        FlushSoundEffectsAndLog(g_nTargetCameraSoundHandle, 0);
        g_nTargetCameraSoundHandle = 0;
        PlaySfxWaveFileByNumber(0x26, -1, 0);
    }
    zero_vector(&g_aShipVelocity[obj]);
    point_ship_at_object(obj, 0);
    trim_goals(obj, 10);
    g_6_vTargetCameraForce.x = g_aShipForwardVector[EYE_OBJECT].x;
    g_6_vTargetCameraForce.y = g_aShipForwardVector[EYE_OBJECT].y;
    g_6_vTargetCameraForce.z = g_aShipForwardVector[EYE_OBJECT].z;
    negate_vector(&g_6_vTargetCameraForce);
    ScaleFixedVector(&g_6_vTargetCameraForce, 0x6400,
                     &g_6_vTargetCameraForce);
    apply_force_to_objects_center(&g_6_vTargetCameraForce, obj);
    g_6_vTargetCameraForce.x = g_aShipVelocity[EYE_OBJECT].x;
    g_6_vTargetCameraForce.y = g_aShipVelocity[EYE_OBJECT].y;
    g_6_vTargetCameraForce.z = g_aShipVelocity[EYE_OBJECT].z;
    apply_force_to_objects_center(&g_6_vTargetCameraForce, obj);
    speed = Vector_magnitude(&g_aShipVelocity[obj]);
    if (speed > 0x6400)
        NormalizeAndScaleVector(&g_aShipVelocity[obj], 0x6400);
    if (distance_between_points(&g_aShipPosition[0],
                                &g_aShipPosition[obj]) < 500) {
        FlushSoundEffectsAndLog(g_nTargetCameraSoundHandle, 0);
        g_nTargetCameraSoundHandle = 0;
        if (g_pCampaignGlobals == 0)
            LoadTemporaryCampaignGlobals();
        if (g_asObjectType[obj] == 0x3d) {
            g_pCampaignGlobals->field_0e = 0;
        } else {
            g_pCampaignGlobals->field_0e = 1;
            if (g_bMissionDeathSequencePending != 0) {
                g_bMissionDeathSequencePending = 0;
                g_6_bMissionWingmanState = 1;
            }
        }
        for (objective = 0;
             objective < (short)g_cMissionObjectiveCount;
             objective++) {
            if (g_aMissionObjectives[objective].type == 6 &&
                g_aMissionObjectives[objective].index ==
                    g_asShipMissionIndex[obj])
                flag_reached(objective, 0);
        }
        remove_object(obj);
        g_aMissionShips[
            g_asShipMissionIndex[obj]].state = 2;
        ClearTargetCameraView();
        ejection_sequence((short)(g_stCurrentPilotProfile.playerShipType + 0x32), 1);
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
    g_nFleetOverviewYawVelocity = 0;
    if (g_asObjectScreenX[
            g_nTargetCameraObject] > 1)
        g_nFleetOverviewYawVelocity = -1;
    if (g_asObjectScreenX[
            g_nTargetCameraObject] < -1)
        g_nFleetOverviewYawVelocity = 1;
    g_nFleetOverviewPitchVelocity = 0;
    if (g_asObjectScreenY[
            g_nTargetCameraObject] > 1)
        g_nFleetOverviewPitchVelocity = 1;
    if (g_asObjectScreenY[
            g_nTargetCameraObject] < -1)
        g_nFleetOverviewPitchVelocity = -1;
    distance = (unsigned short)distance_from_object(
        0, g_nTargetCameraObject);

    if (g_nFleetOverviewYawVelocity > 65 ||
        g_nFleetOverviewYawVelocity < -65) {
        ClearTargetCameraView();
        zero_vector(&g_aShipVelocity[
            g_nTargetCameraObject]);
        return;
    }
    if (g_nFleetOverviewPitchVelocity > 65 ||
        g_nFleetOverviewPitchVelocity < -65) {
        ClearTargetCameraView();
        zero_vector(&g_aShipVelocity[
            g_nTargetCameraObject]);
        return;
    }
    if (distance > 4000) {
        ClearTargetCameraView();
        zero_vector(&g_aShipVelocity[
            g_nTargetCameraObject]);
        return;
    }

    yawDeviation = (short)(abs((int)g_nFleetOverviewYaw) - 17);
    pitchDeviation = (short)(abs((int)g_nFleetOverviewPitch) - 17);
    if (yawDeviation > 0)
        yawOverflow = yawDeviation;
    if (pitchDeviation > 0)
        excessPitch = pitchDeviation;
    distanceScale = (short)(distance >> 6);
    g_nTargetCameraZoom = 100;
    if (excessPitch < yawOverflow && distanceScale < yawOverflow)
        g_nTargetCameraZoom =
            (short)(g_nTargetCameraZoom - yawOverflow);
    if (excessPitch > yawOverflow && distanceScale < excessPitch)
        g_nTargetCameraZoom =
            (short)(g_nTargetCameraZoom - excessPitch);
    if (distanceScale > yawOverflow && distanceScale > excessPitch)
        g_nTargetCameraZoom =
            (short)(g_nTargetCameraZoom - distanceScale);
    if (g_nTargetCameraZoom > 100)
        g_nTargetCameraZoom = 100;
    if (g_nTargetCameraZoom < 67)
        g_nTargetCameraZoom = 67;
}

#pragma intrinsic(abs)
/* Function start: 0x460BAF */
void ClearSavedTorpedoTargetLock(void)
{
    g_bSavedPlayerTarget = 0;
    g_bSavedTargetLockAcquired = 0;
}

/* Function start: 0x460BCC */
void UpdateSavedTorpedoTargetLock(void)
{
    if (g_bSavedPlayerTarget != 0) {
        get_facing_range_from_object(0, g_nSavedPlayerTarget);
        if (g_nFacingToTarget < 100)
            point_ship_at_object(0, g_nSavedPlayerTarget);
        if (g_nTargetRange < 3000)
            approach_zero_speed(0);
        if ((g_nTargetLockMusicCooldown -
                     g_nFacingToTarget > 5 &&
                 g_nFacingToTarget < 100) ||
            unactive(g_nSavedPlayerTarget) != 0 ||
            g_nTargetRange > 10000 ||
            g_acPlayerComponentDamage[5] >= 4) {
            ClearSavedTorpedoTargetLock();
            SetHudMessageText(
                _("Torpedo Lock Lost"),
                (unsigned short)g_cYellowColour,
                20);
            handle_collisions(0);
            return;
        } else {
            g_nTargetLockMusicCooldown =
                g_nFacingToTarget;
            if (g_nTargetLockMusicCooldown > 100)
                g_nTargetLockMusicCooldown = 100;
            g_nSavedTargetLockCountdown--;
            g_bSavedTargetLockAcquired = 0;
        }
        if (g_nSavedTargetLockCountdown == 0) {
            SetHudMessageText(
                _("Torpedo Locked!"),
                (unsigned short)g_cYellowColour,
                20);
            g_bSavedTargetLockAcquired = 1;
        }
    } else
        handle_collisions(0);
}

/* Function start: 0x460D3E */
void SetTargetCameraEyePosition(short cameraMode)
{
    ShortVector offset;

    offset = g_aTargetCameraEyeOffsets[cameraMode];
    g_aShipPosition[EYE_OBJECT].x =
        offset.y * g_aShipUpVector[0].x +
        offset.z * g_aShipForwardVector[0].x +
        offset.x * g_aShipRightVector[0].x +
        g_aShipPosition[0].x;
    g_aShipPosition[EYE_OBJECT].y =
        offset.y * g_aShipUpVector[0].y +
        offset.z * g_aShipForwardVector[0].y +
        offset.x * g_aShipRightVector[0].y +
        g_aShipPosition[0].y;
    g_aShipPosition[EYE_OBJECT].z =
        offset.y * g_aShipUpVector[0].z +
        offset.z * g_aShipForwardVector[0].z +
        offset.x * g_aShipRightVector[0].z +
        g_aShipPosition[0].z;
}

/* Function start: 0x460DFE */
void DrawTargetCameraAttitudeIndicators(unsigned short colour)
{
    CockpitScannerGeometry scanner;

    scanner = ((CockpitScannerGeometry *)(
        g_abGunDisplayConfiguration + 0x40))[
            g_8_nGunDisplaySlot];
    if (g_nFleetOverviewYaw != 0) {
        if (g_nFleetOverviewYaw < 0)
            g_1_nTargetCameraYawIndicatorX =
                (short)(scanner.centerX +
                        g_nFleetOverviewYaw * 14 / 65 + 14);
        else
            g_1_nTargetCameraYawIndicatorX =
                (short)(scanner.centerX -
                        (g_nFleetOverviewYaw * -14 / 65 + 14));
        g_1_nTargetLensYawIndicatorYWord = scanner.centerY;
        g_asAimCameraYawIndicatorBackground[0] =
            (short)GetViewportPixel(
                &g_101_stScrViewInfo,
                g_1_nTargetCameraYawIndicatorX,
                (short)(g_1_nTargetLensYawIndicatorYWord - 1));
        DrawViewportPixel(
            &g_101_stScrViewInfo,
            g_1_nTargetCameraYawIndicatorX,
            (short)(g_1_nTargetLensYawIndicatorYWord - 1),
            (short)colour);
        g_asAimCameraYawIndicatorBackground[1] =
            (short)GetViewportPixel(
                &g_101_stScrViewInfo,
                g_1_nTargetCameraYawIndicatorX,
                g_1_nTargetLensYawIndicatorYWord);
        DrawViewportPixel(
            &g_101_stScrViewInfo,
            g_1_nTargetCameraYawIndicatorX,
            g_1_nTargetLensYawIndicatorYWord,
            (short)colour);
        g_asAimCameraYawIndicatorBackground[2] =
            (short)GetViewportPixel(
                &g_101_stScrViewInfo,
                g_1_nTargetCameraYawIndicatorX,
                (short)(g_1_nTargetLensYawIndicatorYWord + 1));
        DrawViewportPixel(
            &g_101_stScrViewInfo,
            g_1_nTargetCameraYawIndicatorX,
            (short)(g_1_nTargetLensYawIndicatorYWord + 1),
            (short)colour);
    }
    if (g_nFleetOverviewPitch != 0) {
        if (g_nFleetOverviewPitch > 0)
            g_nTargetLensPitchIndicatorY =
                (short)(scanner.centerY +
                        g_nFleetOverviewPitch * -14 / 65 + 14);
        else
            g_nTargetLensPitchIndicatorY =
                (short)(scanner.centerY -
                        (g_nFleetOverviewPitch * 14 / 65 + 14));
        g_4_nTargetCameraPitchPipX = scanner.centerX;
        g_asTargetCameraPitchIndicatorBackgroundTable[0] =
            (short)GetViewportPixel(
                &g_101_stScrViewInfo,
                (short)(g_4_nTargetCameraPitchPipX - 1),
                g_nTargetLensPitchIndicatorY);
        DrawViewportPixel(
            &g_101_stScrViewInfo,
            (short)(g_4_nTargetCameraPitchPipX - 1),
            g_nTargetLensPitchIndicatorY,
            (short)colour);
        g_asTargetCameraPitchIndicatorBackgroundTable[1] =
            (short)GetViewportPixel(
                &g_101_stScrViewInfo,
                g_4_nTargetCameraPitchPipX,
                g_nTargetLensPitchIndicatorY);
        DrawViewportPixel(
            &g_101_stScrViewInfo,
            g_4_nTargetCameraPitchPipX,
            g_nTargetLensPitchIndicatorY,
            (short)colour);
        g_asTargetCameraPitchIndicatorBackgroundTable[2] =
            (short)GetViewportPixel(
                &g_101_stScrViewInfo,
                (short)(g_4_nTargetCameraPitchPipX + 1),
                g_nTargetLensPitchIndicatorY);
        DrawViewportPixel(
            &g_101_stScrViewInfo,
            (short)(g_4_nTargetCameraPitchPipX + 1),
            g_nTargetLensPitchIndicatorY,
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
        g_abGunDisplayConfiguration + 0x40))[
            g_8_nGunDisplaySlot];
    DrawViewportPixel(
        &g_101_stScrViewInfo,
        g_1_nTargetCameraYawIndicatorX,
        (short)(g_1_nTargetLensYawIndicatorYWord - 1),
        g_asAimCameraYawIndicatorBackground[0]);
    DrawViewportPixel(
        &g_101_stScrViewInfo,
        g_1_nTargetCameraYawIndicatorX,
        g_1_nTargetLensYawIndicatorYWord,
        g_asAimCameraYawIndicatorBackground[1]);
    DrawViewportPixel(
        &g_101_stScrViewInfo,
        g_1_nTargetCameraYawIndicatorX,
        (short)(g_1_nTargetLensYawIndicatorYWord + 1),
        g_asAimCameraYawIndicatorBackground[2]);
    DrawViewportPixel(
        &g_101_stScrViewInfo,
        (short)(g_4_nTargetCameraPitchPipX - 1),
        g_nTargetLensPitchIndicatorY,
        g_asTargetCameraPitchIndicatorBackgroundTable[0]);
    DrawViewportPixel(
        &g_101_stScrViewInfo,
        g_4_nTargetCameraPitchPipX,
        g_nTargetLensPitchIndicatorY,
        g_asTargetCameraPitchIndicatorBackgroundTable[1]);
    DrawViewportPixel(
        &g_101_stScrViewInfo,
        (short)(g_4_nTargetCameraPitchPipX + 1),
        g_nTargetLensPitchIndicatorY,
        g_asTargetCameraPitchIndicatorBackgroundTable[2]);
    DrawViewportPixel(
        &g_101_stScrViewInfo,
        scanner.centerX, scanner.centerY, colour);
    DrawViewportPixel(
        &g_101_stScrViewInfo,
        (short)(scanner.centerX + 1), scanner.centerY, colour);
    DrawViewportPixel(
        &g_101_stScrViewInfo,
        (short)(scanner.centerX - 1), scanner.centerY, colour);
    DrawViewportPixel(
        &g_101_stScrViewInfo,
        scanner.centerX, (short)(scanner.centerY + 1), colour);
    DrawViewportPixel(
        &g_101_stScrViewInfo,
        (short)(scanner.centerX + 1),
        (short)(scanner.centerY + 1), colour);
    DrawViewportPixel(
        &g_101_stScrViewInfo,
        (short)(scanner.centerX - 1),
        (short)(scanner.centerY + 1), colour);
    DrawViewportPixel(
        &g_101_stScrViewInfo,
        scanner.centerX, (short)(scanner.centerY - 1), colour);
    DrawViewportPixel(
        &g_101_stScrViewInfo,
        (short)(scanner.centerX + 1),
        (short)(scanner.centerY - 1), colour);
    DrawViewportPixel(
        &g_101_stScrViewInfo,
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

    yaw = g_nFleetOverviewYawVelocity;
    pitch = g_nFleetOverviewPitchVelocity;
    if (yaw > 0) {
        g_14_cPilotHandFrameCode =
            (signed char)MinShort((short)(yaw + 8), 12);
        return;
    }
    if (yaw < 0) {
        g_14_cPilotHandFrameCode =
            (signed char)MinShort((short)(4 - yaw), 8);
        return;
    }
    if (pitch > 0) {
        g_14_cPilotHandFrameCode =
            (signed char)MinShort((short)(pitch + 12), 16);
        return;
    }
    if (pitch < 0) {
        g_14_cPilotHandFrameCode =
            (signed char)MinShort((short)-pitch, 4);
        return;
    }
    g_14_cPilotHandFrameCode = 0;
}

/* Function start: 0x46138D */
void DrawUnreferencedPilotHandFrame(Viewport *viewport, unsigned char *shape,
                       BarracksAnimationState *state)
{
    short x;
    short y;

    x = (short)(g_stPilotHandOrigin.x -
                g_3_stDashViewport.left);
    y = (short)(g_stPilotHandOrigin.y -
                g_3_stDashViewport.top);
    CopyViewportContents(&g_stPilotHandBackgroundViewportBlock,
                         &g_stPilotHandViewportRecord);
    DrawSpriteDefault(&g_stPilotHandViewportRecord, x, y,
                      g_pPilotHandAnimationShapePtr,
                      (short)(signed char)g_14_cPilotHandFrameCode);
    DrawSpriteDefault(
        &g_stPilotHandViewportRecord,
        (short)(x + g_asPilotHandOffsets[
            (signed char)g_14_cPilotHandFrameCode * 2]),
        (short)(y + g_asPilotHandOffsets[
            (signed char)g_14_cPilotHandFrameCode * 2 + 1]),
        g_pPilotHandAnimationShapePtr, 0x11);
    CopyViewportContents(&g_stPilotHandViewportRecord,
                         &g_3_stDashViewport);
    g_cLastPilotHandFrame = g_14_cPilotHandFrameCode;
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

/* This unit's initialized data, in retail order; the statics are declared
 * at the top so that the functions keep their symbol keys. */
short g_nTargetCameraObject = -1; /* 0x0049d338 */
static short g_nTargetCameraGunCooldown = 4; /* 0x0049d33c */
unsigned char g_abGunDisplayConfiguration[0xa4] = { 0 }; /* 0x0049d340 */
short g_nTargetCameraZoom = 0; /* 0x0049d3e4 */
short g_nTargetCameraFrame = 0; /* 0x0049d3e8 */
short g_nFleetOverviewYawVelocity = 0; /* 0x0049d3ec */
short g_nFleetOverviewPitchVelocity = 0; /* 0x0049d3f0 */
short g_nFleetOverviewYaw = 0; /* 0x0049d3f4 */
short g_nFleetOverviewPitch = 0; /* 0x0049d3f8 */
int g_nTargetCameraSoundHandle = 0; /* 0x0049d3fc */
static short g_asFleetMouseYawThresholds[6] = {
    10, 37, 52, 57, 62, 1070
}; /* 0x0049d400 */
static short g_asFleetMousePitchThresholds[6] = {
    5, 18, 27, 35, 38, 1040
}; /* 0x0049d410 */
int g_nUnusedTurretWord = 0; /* 0x0049d41c */
static signed char g_bFleetPolledInputActive = 0; /* 0x0049d420 */
static signed char g_bFleetButtonInputActive = 0; /* 0x0049d424 */
static short g_nFleetMouseYawVelocity = 0; /* 0x0049d428 */
static short g_nFleetMousePitchVelocity = 0; /* 0x0049d42c */
static ShortVector g_aTargetCameraEyeOffsets[3] = {
    {0, 0, -80},
    {-50, 0, 0},
    {50, 0, 0}
}; /* 0x0049d430 */
static ShortVector g_aaTurretProjectileOffsets[2][2] = {
    {{-25, 0, -80}, {25, 0, -80}},
    {{-50, 0, 25}, {-50, 0, -25}}
}; /* 0x0049d448 */
short g_nSavedPlayerTarget = -1; /* 0x0049d460 */
short g_bSavedPlayerTarget = 0; /* 0x0049d464 */
short g_nTargetCameraEffectObjectA = -1; /* 0x0049d468 */
short g_nTargetCameraEffectObjectB = -1; /* 0x0049d46c */
short g_asShipTurretFireEnabled[10] = { 0 }; /* 0x0049d470 */
short g_nSavedTargetLockCountdown = 1000; /* 0x0049d484 */
short g_bSavedTargetLockAcquired = 0; /* 0x0049d488 */

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short g_1_nTargetCameraYawIndicatorX; /* 0x005c8d3e */
short g_1_nTargetLensYawIndicatorYWord; /* 0x005c8d40 */
short g_asAimCameraYawIndicatorBackground[3]; /* 0x005c8d48 */
short g_1_nGunShowEnergyPercent; /* 0x005c8d4e */
int g_nAimCameraMode; /* 0x005c8d50 */
FixedVector g_6_vTargetCameraForce; /* 0x005c8d60 */
short g_4_nTargetCameraPitchPipX; /* 0x005c8d6c */
short g_nTargetLensPitchIndicatorY; /* 0x005c8d6e */
short g_27_aasGunCooldown[10][3]; /* 0x005c8d70 */
short g_asTargetCameraPitchIndicatorBackgroundTable[3]; /* 0x005c8db0 */
int g_9_nTargetCameraOverlayMode; /* 0x005c8db8 */
short g_12_nGunDisplayCount; /* 0x005c8dbc */
short g_6_bMissionWingmanState; /* 0x005c8dbe */
short g_8_nGunDisplaySlot; /* 0x005c8dc0 */
