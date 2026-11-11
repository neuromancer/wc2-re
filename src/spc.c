/*
 *  Space views, star field and object simulation.
 *
 *  Retail object 0x419C10-0x41E4DF (link order; see docs/ORDER.md).
 */
#define SPC_C
#include "game.h"

signed char g_bLandingCommRequestPending = 0; /* 0x00492fa0 */
int g_nCapitalShipViewDistance = 0x7d000; /* 0x00492fa4 */
int g_nCurrentView = -1; /* 0x00492fa8 */
short g_bAlternateChaseView = 0; /* 0x00492fac */
int g_nChaseCameraMaximumVelocity = 0x8c00; /* 0x00492fb0 */
short g_nSpaceExplosionFlashActive = 0; /* 0x00492fb4 */
short g_bApplyingCollisionDamage = 0; /* 0x00492fb8 */
short g_asIntelligenceEvent[12] = { 0 }; /* 0x00492fc0 */

extern short g_nCockpitCameraObject; /* 0x0049d770 */

extern FixedVector g_vExternalViewOffset; /* 0x005d3200 */

/* WC2's collision response uses SHL for the signed mass difference.  Native C
 * cannot left-shift a negative value, so use the equivalent bounded multiply
 * in the port while retaining the original expression for comparison. */
#ifdef SDL_PORT
#define COLLISION_MASS_DIFFERENCE_SCALE(objectMass, partnerMass) \
    (((objectMass) - (partnerMass)) * 0x100)
#else
#define COLLISION_MASS_DIFFERENCE_SCALE(objectMass, partnerMass) \
    (((objectMass) - (partnerMass)) << 8)
#endif

/* Only slots 0-11 have ship maximum-velocity state, but WC2 indexes that
 * table with any colliding object.  Slot 15 reaches the zero high word of the
 * adjacent alert flags in the retail layout; make that zero result explicit
 * for every non-ship slot instead of crossing a native global's redzone. */
#ifdef SDL_PORT
#define COLLISION_MAXIMUM_VELOCITY(obj) \
    ((unsigned short)(obj) < 12 \
         ? g_asShipMaximumVelocity[obj] \
         : 0)
#else
#define COLLISION_MAXIMUM_VELOCITY(obj) \
    g_asShipMaximumVelocity[obj]
#endif

#pragma function(abs)
/* Function start: 0x419C10 */
unsigned int rotate_eye_to_goal(void)
{
    short totalError;

    /* CL 4.1 folds ~~ after selecting retail's roll-first call order. */
    totalError = (short)(abs(g_anObjectPitchRotation[
                                EYE_OBJECT] -
                            g_nEyePitchGoal) +
                         abs(g_anObjectYawRotation[
                                 EYE_OBJECT] -
                            g_nEyeYawGoal) +
                         ~~abs(g_anObjectRollRotation[
                                 EYE_OBJECT] -
                            g_nEyeRollGoal));
    match_rotation_goal(&g_anObjectPitchRotation[
                            EYE_OBJECT],
                        &g_nEyePitchGoal, totalError,
                        g_nEyePitchRate);
    match_rotation_goal(&g_anObjectYawRotation[
                            EYE_OBJECT],
                        &g_nEyeYawGoal, totalError,
                        g_nEyeYawRate);
    match_rotation_goal(&g_anObjectRollRotation[
                            EYE_OBJECT],
                        &g_nEyeRollGoal, totalError,
                        g_nEyeRollRate);
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x419CDC */
short GetVectorMagnitude(const FixedVector *vector)
{
    return FixedToShortSaturating(Vector_magnitude(vector));
}

/* Function start: 0x419D01 */
unsigned int set_eye_direction_and_position(void)
{
    FixedVector positionError;
    FixedVector desiredPosition;
    FixedVector adjustment;
    FixedVector vector;
    short desiredDistance;
    FixedVector direction;

    if (g_bStarSystemJumpTransition != 0)
        goto finished;

    switch (g_nCurrentView) {
    case 5:
        if (g_aeObjectClass[g_cViewObject] !=
            OBJECT_CLASS_NULL) {
            ComputeVectorDelta(
                &g_aShipPosition[EYE_OBJECT],
                &g_aShipPosition[g_cViewObject],
                &direction);
            desiredPosition = direction;
            SetVectorFixedPoint(
                (unsigned int *)&desiredPosition,
                (short)(g_bAlternateChaseView == 0 ? -700 : -500));
            AddFixedVectors(
                &desiredPosition,
                &g_aShipPosition[g_cViewObject],
                &desiredPosition);
            ComputeVectorDelta(
                &g_aShipPosition[EYE_OBJECT],
                &desiredPosition, &positionError);
            vector = positionError;
            divide_vector(
                &vector,
                (g_bAlternateChaseView == 0 ? 25 : 7) << 8,
                &vector);
            if (Vector_magnitude(&vector) >
                g_nChaseCameraMaximumVelocity)
                NormalizeAndScaleVector(
                    &vector, g_nChaseCameraMaximumVelocity);
            g_aShipVelocity[EYE_OBJECT] = vector;
            g_aShipForwardVector[EYE_OBJECT] = direction;
            shrink_vector(&g_aShipForwardVector[EYE_OBJECT]);
            fix_objects_ijk(EYE_OBJECT);
            g_nEyeRollGoal = match_roll_orientation(
                EYE_OBJECT, (short)g_cViewObject);
            if (g_nEyeRollGoal != 0) {
                if (abs((int)g_nEyeRollGoal) > 4) {
                    g_nEyeRollRate = 4;
                    rotate_eye_to_goal();
                    rotate_object(EYE_OBJECT);
                } else {
                    g_nEyeRollGoal = 0;
                    g_aShipUpVector[EYE_OBJECT] =
                        g_aShipUpVector[0];
                }
            }
            AddFixedVectors(&g_aShipPosition[EYE_OBJECT],
                            &g_aShipVelocity[EYE_OBJECT],
                            &g_aShipPosition[EYE_OBJECT]);
        }
        break;
    case 6:
        zero_vector(&g_aShipVelocity[EYE_OBJECT]);
        ComputeVectorDelta(
            &g_aShipPosition[EYE_OBJECT],
            &g_aShipPosition[g_cViewObject], &vector);
        if (Vector_magnitude(&vector) > 0x7d000) {
            vector = g_aShipVelocity[g_cViewObject];
            NormalizeFixedVector(&vector);
            g_aShipUpVector[EYE_OBJECT] =
                g_aShipUpVector[g_cViewObject];
            if (vector.x == g_aShipUpVector[EYE_OBJECT].x &&
                vector.y == g_aShipUpVector[EYE_OBJECT].y &&
                vector.z == g_aShipUpVector[EYE_OBJECT].z) {
                g_aShipUpVector[EYE_OBJECT] =
                    g_aShipRightVector[g_cViewObject];
            }
            ScaleFixedVector(&vector, -0x12430, &vector);
            AddFixedVectors(
                &g_aShipPosition[g_cViewObject], &vector,
                &g_aShipPosition[EYE_OBJECT]);
            g_aShipForwardVector[EYE_OBJECT] = vector;
            fix_objects_ijk(EYE_OBJECT);
            ScaleFixedVector(&g_aShipRightVector[EYE_OBJECT],
                             0x19000, &vector);
            AddFixedVectors(&g_aShipPosition[EYE_OBJECT],
                            &vector,
                            &g_aShipPosition[EYE_OBJECT]);
            ScaleFixedVector(&g_aShipUpVector[EYE_OBJECT],
                             0x6400, &vector);
            AddFixedVectors(&g_aShipPosition[EYE_OBJECT],
                            &vector,
                            &g_aShipPosition[EYE_OBJECT]);
            ComputeVectorDelta(
                &g_aShipPosition[EYE_OBJECT],
                &g_aShipPosition[g_cViewObject], &vector);
            g_aShipForwardVector[EYE_OBJECT] = vector;
            fix_objects_ijk(EYE_OBJECT);
            generate_stars();
        } else {
            g_aShipForwardVector[EYE_OBJECT] = vector;
            fix_objects_ijk(EYE_OBJECT);
        }
        break;
    case 14:
        AddFixedVectors(&g_aShipPosition[EYE_OBJECT],
                        &g_aShipVelocity[EYE_OBJECT],
                        &g_aShipPosition[EYE_OBJECT]);
        look_at(0);
        break;
    case 13:
        ScaleFixedVector(&g_aShipRightVector[EYE_OBJECT],
                         -0xa00, &vector);
        AddFixedVectors(&g_aShipPosition[EYE_OBJECT], &vector,
                        &g_aShipPosition[EYE_OBJECT]);
        AddFixedVectors(&g_aShipPosition[EYE_OBJECT],
                        &g_aShipVelocity[EYE_OBJECT],
                        &g_aShipPosition[EYE_OBJECT]);
        look_at(0);
        break;
    case 7:
        if (g_nExternalViewShip == -1) {
            zero_vector(&g_aShipVelocity[EYE_OBJECT]);
            if (g_7_nExternalViewTimeout++ > 20) {
                if (g_nCockpitDisplayMode == 0) {
                    new_view(0, 0);
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_27_stViewScratchInfo, 0, 0,
                        (unsigned short)(g_nScreenWidth - 1),
                        (unsigned short)(g_nScreenHeight - 1));
                    initialize_view_buffer();
                    new_view(0, 0);
                    free_view_buffer();
                    SetViewportRect(&g_27_stViewScratchInfo,
                                    0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
        } else {
            desiredDistance = 250;
            if (g_asObjectType[g_nExternalViewShip] ==
                0x13)
                desiredDistance = 800;
            ComputeVectorDelta(
                &g_aShipPosition[EYE_OBJECT],
                &g_aShipPosition[g_nExternalViewShip],
                &vector);
            if (g_asShipTactic[g_nExternalViewShip] != 0) {
                if (Vector_magnitude(&vector) >
                    ((int)desiredDistance + 50) << 8) {
                    divide_vector(
                        &vector,
                        ((int)g_2_nExternalViewDistanceValue & ~1) << 7,
                        &g_aShipVelocity[EYE_OBJECT]);
                    g_2_nExternalViewDistanceValue = MaxShort(
                        (short)(g_2_nExternalViewDistanceValue - 1), 8);
                } else {
                    adjustment = vector;
                    NormalizeFixedVector(&adjustment);
                    ScaleFixedVector(&adjustment,
                                     -((int)desiredDistance << 8),
                                     &adjustment);
                    AddFixedVectors(
                        &vector, &adjustment,
                        &g_aShipVelocity[EYE_OBJECT]);
                }
            }
            g_aShipForwardVector[EYE_OBJECT] = vector;
            fix_objects_ijk(EYE_OBJECT);
        }
        AddFixedVectors(&g_aShipPosition[EYE_OBJECT],
                        &g_aShipVelocity[EYE_OBJECT],
                        &g_aShipPosition[EYE_OBJECT]);
        break;
    case 15:
        SetFleetOverviewView(0);
        break;
    case 8:
        if (g_acShipTarget[0] != -1) {
            ScaleFixedVector(&g_aShipRightVector[0],
                             0x12c00, &vector);
            AddFixedVectors(&g_aShipPosition[0], &vector,
                            &g_aShipPosition[EYE_OBJECT]);
            ComputeVectorDelta(
                &g_aShipPosition[EYE_OBJECT],
                &g_aShipPosition[g_acShipTarget[0]],
                &vector);
            NormalizeFixedVector(&vector);
            g_aShipForwardVector[EYE_OBJECT] = vector;
            ScaleFixedVector(&vector, -0x25800, &vector);
            AddFixedVectors(&g_aShipPosition[EYE_OBJECT],
                            &vector,
                            &g_aShipPosition[EYE_OBJECT]);
            fix_objects_ijk(EYE_OBJECT);
        } else {
            new_view(0, 0);
        }
        break;
    case 0:
        if (g_nCockpitCameraObject != 0) {
            copy_frame(g_nCockpitCameraObject, EYE_OBJECT);
            g_aShipVelocity[EYE_OBJECT] =
                g_aShipVelocity[g_nCockpitCameraObject];
            g_aShipPosition[EYE_OBJECT] =
                g_aShipPosition[g_nCockpitCameraObject];
        } else {
            copy_frame(0, EYE_OBJECT);
            g_aShipVelocity[EYE_OBJECT] =
                g_aShipVelocity[0];
            g_aShipPosition[EYE_OBJECT] =
                g_aShipPosition[0];
        }
        break;
    case 1:
        g_aShipRightVector[EYE_OBJECT] =
            g_aShipForwardVector[0];
        negate_vector(&g_aShipRightVector[EYE_OBJECT]);
        g_aShipUpVector[EYE_OBJECT] =
            g_aShipUpVector[0];
        g_aShipForwardVector[EYE_OBJECT] =
            g_aShipRightVector[0];
        g_aShipVelocity[EYE_OBJECT] =
            g_aShipVelocity[0];
        g_aShipPosition[EYE_OBJECT] =
            g_aShipPosition[0];
        break;
    case 2:
        g_aShipRightVector[EYE_OBJECT] =
            g_aShipForwardVector[0];
        g_aShipUpVector[EYE_OBJECT] =
            g_aShipUpVector[0];
        g_aShipForwardVector[EYE_OBJECT] =
            g_aShipRightVector[0];
        negate_vector(&g_aShipForwardVector[EYE_OBJECT]);
        g_aShipVelocity[EYE_OBJECT] =
            g_aShipVelocity[0];
        g_aShipPosition[EYE_OBJECT] =
            g_aShipPosition[0];
        break;
    case 3:
        copy_frame(0, EYE_OBJECT);
        negate_vector(&g_aShipRightVector[EYE_OBJECT]);
        negate_vector(&g_aShipForwardVector[EYE_OBJECT]);
        g_aShipVelocity[EYE_OBJECT] =
            g_aShipVelocity[0];
        g_aShipPosition[EYE_OBJECT] =
            g_aShipPosition[0];
        break;
    case 4:
        switch (g_9_nTargetCameraOverlayMode) {
        case 0:
            copy_frame(0, EYE_OBJECT);
            negate_vector(&g_aShipRightVector[EYE_OBJECT]);
            negate_vector(&g_aShipForwardVector[EYE_OBJECT]);
            break;
        case 1:
            g_aShipRightVector[EYE_OBJECT] =
                g_aShipForwardVector[0];
            g_aShipUpVector[EYE_OBJECT] =
                g_aShipUpVector[0];
            g_aShipForwardVector[EYE_OBJECT] =
                g_aShipRightVector[0];
            negate_vector(&g_aShipForwardVector[EYE_OBJECT]);
            break;
        case 2:
            g_aShipRightVector[EYE_OBJECT] =
                g_aShipForwardVector[0];
            negate_vector(&g_aShipRightVector[EYE_OBJECT]);
            g_aShipUpVector[EYE_OBJECT] =
                g_aShipUpVector[0];
            g_aShipForwardVector[EYE_OBJECT] =
                g_aShipRightVector[0];
            break;
        }
        rotate_about_j(g_nFleetOverviewYaw,
                       &g_aShipRightVector[EYE_OBJECT],
                       &g_aShipForwardVector[EYE_OBJECT]);
        rotate_about_i(g_nFleetOverviewPitch,
                       &g_aShipUpVector[EYE_OBJECT],
                       &g_aShipForwardVector[EYE_OBJECT]);
        g_aShipVelocity[EYE_OBJECT] =
            g_aShipVelocity[0];
        SetTargetCameraEyePosition(
            (short)g_9_nTargetCameraOverlayMode);
        UpdateFleetOverviewCameraRotation();
        break;
    case 10:
        copy_frame(0, EYE_OBJECT);
        negate_vector(&g_aShipRightVector[EYE_OBJECT]);
        negate_vector(&g_aShipForwardVector[EYE_OBJECT]);
        g_aShipVelocity[EYE_OBJECT] =
            g_aShipVelocity[0];
        g_aShipPosition[EYE_OBJECT] =
            g_aShipPosition[0];
        break;
    case 11:
        copy_frame((short)g_cViewObject, EYE_OBJECT);
        g_aShipVelocity[EYE_OBJECT] =
            g_aShipVelocity[g_cViewObject];
        g_aShipPosition[EYE_OBJECT] =
            g_aShipPosition[g_cViewObject];
        break;
    case 12:
        ComputeVectorDelta(
            &g_aShipPosition[EYE_OBJECT],
            &g_aShipPosition[g_cViewObject], &vector);
        if (Vector_magnitude(&vector) < 0x25800) {
            adjustment = vector;
            NormalizeFixedVector(&adjustment);
            ScaleFixedVector(&adjustment, -0x25800, &adjustment);
            AddFixedVectors(&vector, &adjustment, &adjustment);
            AddFixedVectors(&g_aShipPosition[EYE_OBJECT],
                            &adjustment,
                            &g_aShipPosition[EYE_OBJECT]);
        }
        g_aShipForwardVector[EYE_OBJECT] = vector;
        fix_objects_ijk(EYE_OBJECT);
        AddFixedVectors(&g_aShipPosition[EYE_OBJECT],
                        &g_aShipVelocity[EYE_OBJECT],
                        &g_aShipPosition[EYE_OBJECT]);
        break;
    case 18:
        AddFixedVectors(&g_aShipPosition[EYE_OBJECT],
                        &g_aShipVelocity[EYE_OBJECT],
                        &g_aShipPosition[EYE_OBJECT]);
        rotate_object(EYE_OBJECT);
        rotate_eye_to_goal();
        break;
    case 16:
        AddFixedVectors(&g_aShipPosition[EYE_OBJECT],
                        &g_aShipVelocity[EYE_OBJECT],
                        &g_aShipPosition[EYE_OBJECT]);
        break;
    case 17:
        AddFixedVectors(
            &g_aShipPosition[g_cViewObject],
            &g_vExternalViewOffset,
            &g_aShipPosition[EYE_OBJECT]);
        g_aShipVelocity[EYE_OBJECT] =
            g_aShipVelocity[g_cViewObject];
        break;
    }

finished:
    ;
}

/* Function start: 0x41AD51 */
void force_view(int view, short obj)
{
    if (IsCockpitExplosionActive() != 0)
        return;
    g_nCurrentView = -1;
    new_view(view, obj);
}

/* Function start: 0x41AD8B */
void SaveTorpedoTargetLockForViewChange(int previousView)
{
    if (g_bSavedPlayerTarget == 0 &&
        previousView == 0 &&
        g_nTargetLockCountdown >= 0 &&
        unactive(g_acShipTarget[0]) == 0 &&
        g_bTargetLockMode != 0 &&
        g_nSelectedReleaseWeaponIndex != -1 &&
        ((ShipWeaponSlot *)&g_aShipWeapons[0][1])[
            g_nSelectedReleaseWeaponIndex].type == 0x13) {
        g_nSavedPlayerTarget = g_acShipTarget[0];
        g_bSavedPlayerTarget = 1;
        g_nTargetLockMusicCooldown = -100;
        g_nSavedTargetLockCountdown =
            g_nTargetLockCountdown;
    }
}

/* Function start: 0x41AE40 */
unsigned int new_view(int view, short obj)
{
    int previousView;
    FixedVector eyeOffset;

    if (IsCockpitExplosionActive() != 0)
        goto finished;
    if (g_nCurrentView == view) {
        if (view == 5)
            g_bAlternateChaseView =
                (short)(1 - g_bAlternateChaseView);
        goto finished;
    }
    if (view <= 3 && g_1_cStoredViewDisplayModeValue == view) {
        if (g_nCurrentView == -1)
            g_nCurrentView = view;
        goto finished;
    }
    if (view == 7 && g_nExternalViewShip == -1)
        goto finished;

    if (g_nTargetCameraFrame == 1)
        ClearTargetCameraView();
    g_cViewObject = (signed char)obj;
    previousView = g_nCurrentView;
    g_bTargetLockActive = 0;
    if (view == 0 && previousView == 4) {
        if (g_bSavedPlayerTarget != 0) {
            g_nTargetLockCountdown =
                (short)(g_nSavedTargetLockCountdown + 1);
            if (g_nTargetLockCountdown < 0)
                g_nTargetLockCountdown = 0;
            g_bTargetLockMode = 1;
            g_acShipTarget[0] =
                (signed char)g_nSavedPlayerTarget;
        }
        g_bSavedPlayerTarget = 0;
    }
    if (view == 0)
        g_bSavedPlayerTarget = 0;
    g_nCurrentView = view;
    if (obj != -1) {
        g_asObjectCollisionRadius[EYE_OBJECT] =
            MaxShort(10, g_asObjectCollisionRadius[obj]);
    } else {
        g_asObjectCollisionRadius[EYE_OBJECT] = 10;
    }

    switch (view) {
    case 0:
        initialize_cockpit(0);
        break;
    case 1:
        if (CountShipCockpitGunDisplays(0) > 1) {
            g_nCurrentView = 4;
            SaveTorpedoTargetLockForViewChange(previousView);
        }
        initialize_cockpit(1);
        break;
    case 2:
        if (CountShipCockpitGunDisplays(0) > 1) {
            g_nCurrentView = 4;
            SaveTorpedoTargetLockForViewChange(previousView);
        }
        initialize_cockpit(2);
        break;
    case 3:
        if (HasShipCockpitGunDisplay(0) != 0) {
            g_nCurrentView = 4;
            SaveTorpedoTargetLockForViewChange(previousView);
        }
        initialize_cockpit(3);
        break;
    case 6:
        initialize_cockpit(4);
        eyeOffset = g_aShipVelocity[g_cViewObject];
        if (NormalizeFixedVector(&eyeOffset) == 0)
            eyeOffset =
                g_aShipForwardVector[g_cViewObject];
        g_aShipUpVector[EYE_OBJECT] =
            g_aShipUpVector[g_cViewObject];
        if (eyeOffset.x == g_aShipUpVector[EYE_OBJECT].x &&
            eyeOffset.y == g_aShipUpVector[EYE_OBJECT].y &&
            eyeOffset.z == g_aShipUpVector[EYE_OBJECT].z) {
            g_aShipUpVector[EYE_OBJECT] =
                g_aShipRightVector[g_cViewObject];
        }
        ScaleFixedVector(&eyeOffset, -0x12430, &eyeOffset);
        AddFixedVectors(
            &g_aShipPosition[g_cViewObject],
            &eyeOffset, &g_aShipPosition[EYE_OBJECT]);
        g_aShipForwardVector[EYE_OBJECT] = eyeOffset;
        fix_objects_ijk(EYE_OBJECT);
        zero_vector(&g_aShipVelocity[EYE_OBJECT]);
        ScaleFixedVector(&g_aShipRightVector[EYE_OBJECT],
                         0x19000, &eyeOffset);
        AddFixedVectors(&g_aShipPosition[EYE_OBJECT],
                        &eyeOffset,
                        &g_aShipPosition[EYE_OBJECT]);
        ScaleFixedVector(&g_aShipUpVector[EYE_OBJECT],
                         0x6400, &eyeOffset);
        AddFixedVectors(&g_aShipPosition[EYE_OBJECT],
                        &eyeOffset,
                        &g_aShipPosition[EYE_OBJECT]);
        ComputeVectorDelta(
            &g_aShipPosition[EYE_OBJECT],
            &g_aShipPosition[g_cViewObject],
            &eyeOffset);
        g_aShipForwardVector[EYE_OBJECT] = eyeOffset;
        fix_objects_ijk(EYE_OBJECT);
        break;
    case 13:
        initialize_cockpit(4);
        copy_frame(0, EYE_OBJECT);
        position_relative_ijk(&g_aShipPosition[EYE_OBJECT],
                              0, 0, 0, 2000);
        zero_vector(&g_aShipVelocity[EYE_OBJECT]);
        look_at(0);
        break;
    case 14:
        break;
    case 5:
        initialize_cockpit(4);
        ScaleFixedVector(
            &g_aShipForwardVector[g_cViewObject],
            -0x4b000, &eyeOffset);
        AddFixedVectors(
            &g_aShipPosition[g_cViewObject],
            &eyeOffset, &g_aShipPosition[EYE_OBJECT]);
        g_aShipUpVector[EYE_OBJECT] =
            g_aShipUpVector[g_cViewObject];
        g_aShipForwardVector[EYE_OBJECT] = eyeOffset;
        fix_objects_ijk(EYE_OBJECT);
        zero_vector(&g_aShipVelocity[EYE_OBJECT]);
        break;
    case 7:
        initialize_cockpit(4);
        ScaleFixedVector(
            &g_aShipRightVector[g_nExternalViewShip],
            0x25800, &eyeOffset);
        AddFixedVectors(
            &g_aShipPosition[g_nExternalViewShip],
            &eyeOffset, &g_aShipPosition[EYE_OBJECT]);
        ComputeVectorDelta(
            &g_aShipPosition[EYE_OBJECT],
            &g_aShipPosition[g_nExternalViewShip],
            &eyeOffset);
        g_aShipUpVector[EYE_OBJECT] =
            g_aShipUpVector[0];
        g_aShipForwardVector[EYE_OBJECT] = eyeOffset;
        fix_objects_ijk(EYE_OBJECT);
        zero_vector(&g_aShipVelocity[EYE_OBJECT]);
        g_2_nExternalViewDistanceValue = 0x20;
        g_7_nExternalViewTimeout = 0;
        break;
    case 8:
        initialize_cockpit(4);
        copy_frame(0, EYE_OBJECT);
        zero_vector(&g_aShipVelocity[EYE_OBJECT]);
        break;
    case 15:
        SetFleetOverviewView(1);
        break;
    case 10:
        initialize_cockpit(6);
        break;
    case 11:
        break;
    case 12:
    case 18:
        initialize_cockpit(4);
        g_nEyePitchGoal =
            g_nEyeYawGoal =
                g_nEyeRollGoal = 0;
        g_nEyePitchRate =
            g_nEyeYawRate =
                g_nEyeRollRate = 1;
        break;
    case 17:
        ComputeVectorDelta(
            &g_aShipPosition[g_cViewObject],
            &g_aShipPosition[EYE_OBJECT],
            &g_vExternalViewOffset);
        break;
    case 16:
        initialize_cockpit(4);
        break;
    }

    set_eye_direction_and_position();
    generate_stars();

finished:
    ;
}

/* Function start: 0x41B62E */
unsigned int start_dust(short obj, FixedVector origin,
                        short forwardDistance, int rightOffset,
                        int upOffset)
{
    FixedVector offset;

    set_objects_data(obj, 0x2e, -1, 0);
    ScaleFixedVector(&g_aShipForwardVector[EYE_OBJECT],
                     (int)forwardDistance << 8, &offset);
    AddFixedVectors(&origin, &offset, &origin);
    ScaleFixedVector(&g_aShipRightVector[EYE_OBJECT],
                     rightOffset, &offset);
    AddFixedVectors(&offset, &origin, &origin);
    ScaleFixedVector(&g_aShipUpVector[EYE_OBJECT],
                     upOffset, &offset);
    AddFixedVectors(&offset, &origin, &origin);
    AddFixedVectors(&origin,
                    &g_aShipPosition[EYE_OBJECT],
                    &g_aShipPosition[obj]);
    FillFixedVectorWithRandomComponents(2, &g_aShipVelocity[obj]);
    g_asObjectScreenAngle[obj] =
        (short)(MaxShort((short)(1 - RandomInRange(0, 3)), 0) * 0x10 +
                RandomInRange(0, 3));
}

/* Function start: 0x41B762 */
unsigned int generate_stars(void)
{
    FixedVector origin;
    short distance;
    short star;

    origin.x = 0;
    origin.y = 0;
    origin.z = 0;
    for (star = 40; star < 55; star++) {
        if (star < 48) {
            distance = RandomInRange(0, 1400);
#ifdef SDL_PORT
            start_dust(star, origin, distance,
                       signed_random(distance) * 0x100,
                       signed_random(distance) * 0x100);
#else
            start_dust(star, origin, distance,
                       signed_random(distance) << 8,
                       signed_random(distance) << 8);
#endif
        } else {
            g_aeObjectClass[star] = OBJECT_CLASS_STAR;
            g_1_nStarFieldIRotation = signed_random(45);
            g_nStarFieldJRotationNumber = signed_random(45);
            copy_frame(EYE_OBJECT, 69);
            rotate_about_j(g_nStarFieldJRotationNumber,
                           &g_aShipRightVector[69],
                           &g_aShipForwardVector[69]);
            rotate_about_i(g_1_nStarFieldIRotation,
                           &g_aShipUpVector[69],
                           &g_aShipForwardVector[69]);
            ScaleFixedVector(&g_aShipForwardVector[69],
                             15000 << 8,
                             &g_aShipPosition[star]);
            g_asObjectViewFrame[star] =
                (short)(RandomInRange(0, 5) + 32);
        }
    }
}

/* Function start: 0x41B8D6 */
unsigned int update_star_field(void)
{
    unsigned char scaleShift;
    int rightOffset;
    short hazardFlag;
    short upSpread;
    short slot;
    FixedVector motion;
    int range;
    int dustUp;
    FixedVector delta;
    FixedVector cameraMotion;
    short outcome;
    short lateralSpread;

    hazardFlag = g_pActiveHazardField != 0;
    g_vPreviousStarFieldMotion = g_vStarFieldMotion;
    ScaleFixedVector(&g_aShipForwardVector[EYE_OBJECT],
                     200 << 8, &cameraMotion);
    ScaleFixedVector(&g_aShipVelocity[EYE_OBJECT],
                     20 << 8, &g_vStarFieldMotion);
    AddFixedVectors(&cameraMotion, &g_vStarFieldMotion,
                    &g_vStarFieldMotion);
    ComputeVectorDelta(&g_vPreviousStarFieldMotion,
                       &g_vStarFieldMotion, &delta);
    for (slot = 40; slot < 55; slot++) {
        if (g_asObjectScreenX[slot] == (short)0x8001) {
            outcome = RandomInRange(0, 7);
            if (hazardFlag == 0) {
                if (g_aeObjectClass[slot] ==
                        OBJECT_CLASS_ASTEROID ||
                    (int)g_aeObjectClass[slot] == 0x15 ||
                    g_aeObjectClass[slot] == OBJECT_CLASS_NULL) {
                    set_objects_data(slot, 0x2e, -1, 0);
                    outcome = 0;
                }
            } else if (slot < 48) {
                extra_hazard(slot);
            }
            if ((g_aeObjectClass[slot] == OBJECT_CLASS_STAR ||
                 g_aeObjectClass[slot] == OBJECT_CLASS_PLANET) &&
                outcome == 0) {
                if ((g_anObjectYawRotation[0] |
                     g_anObjectPitchRotation[0]) == 0)
                    continue;
                copy_frame(EYE_OBJECT, 69);
                if (g_anObjectPitchRotation[0] != 0) {
                    g_1_nStarFieldIRotation =
                        g_anObjectPitchRotation[0] < 0 ? -45 : 45;
                    g_nStarFieldJRotationNumber = signed_random(45);
                }
                if (g_anObjectYawRotation[0] != 0 &&
                    (g_anObjectPitchRotation[0] == 0 ||
                     RandomInRange(0, 1) != 0)) {
                    g_nStarFieldJRotationNumber =
                        g_anObjectYawRotation[0] < 0 ? -45 : 45;
                    g_1_nStarFieldIRotation = signed_random(45);
                }
                rotate_about_i(g_1_nStarFieldIRotation,
                               &g_aShipUpVector[69],
                               &g_aShipForwardVector[69]);
                rotate_about_j(g_nStarFieldJRotationNumber,
                               &g_aShipRightVector[69],
                               &g_aShipForwardVector[69]);
                ScaleFixedVector(&g_aShipForwardVector[69],
                                 15000 << 8,
                                 &g_aShipPosition[slot]);
                g_asObjectViewFrame[slot] =
                    RandomInRange(0, 5) + 32;
                break;
            } else if (g_aeObjectClass[slot] == OBJECT_CLASS_DUST &&
                       outcome < 2) {
                transform_to_objects_frame(
                    &g_aShipVelocity[EYE_OBJECT],
                    &motion, EYE_OBJECT);
                ScaleFixedVector(&motion, 10 << 8, &motion);
                if (motion.z >= 0) {
                    range = (unsigned short)RandomInRange(
                                0, (short)(motion.z >> 8)) * 2 +
                            (unsigned short)RandomInRange(0, 350) +
                            g_asObjectCollisionRadius[EYE_OBJECT] * 2;
                } else {
                    range = (unsigned short)RandomInRange(0, 40) +
                        g_asObjectCollisionRadius[EYE_OBJECT];
                }
                lateralSpread = signed_random((short)(range >> 1));
                upSpread = signed_random((short)(range >> 1));
                scaleShift = (unsigned char)(motion.z <= 0 ? 9 : 8);
#ifdef SDL_PORT
                rightOffset =
                    (int)lateralSpread * (1 << scaleShift) + motion.x;
                dustUp = (int)upSpread * (1 << scaleShift) + motion.y;
#else
                rightOffset = ((int)lateralSpread << scaleShift) + motion.x;
                dustUp = ((int)upSpread << scaleShift) + motion.y;
#endif
                start_dust(slot, delta, range, rightOffset, dustUp);
                break;
            }
        }
    }
    if (g_pActiveHazardField != 0 &&
        g_nCannedSceneMode == 0)
        update_hazards();
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x41BD44 */
short count_down(short obj)
{
    if (g_asObjectCounter[obj] != -1)
        g_asObjectCounter[obj]--;
    return g_asObjectCounter[obj];
}

/* Function start: 0x41BD81 */
void house_keep_objects(void)
{
    short missile;
    short obj;
    short gun;
    short projectileDetonated;

    for (obj = 0; obj <= SPACE_LAST_MOVING_OBJECT; obj++) {
        switch (g_aeObjectClass[obj]) {
        case OBJECT_CLASS_FIXED_OBJECT:
            switch (g_asObjectType[obj]) {
            case 0x0c:
            case 0x2b:
                remove_object(obj);
                break;
            }
            break;
        case OBJECT_CLASS_PROJECTILE:
            projectileDetonated = 0;
            if (g_asObjectType[obj] == 0x0c) {
                for (missile = 0; missile < 10; missile++) {
                    if (g_aeObjectClass[missile] ==
                            OBJECT_CLASS_MISSILE &&
                        g_asObjectType[missile] ==
                            OBJECT_DATA_TORPEDO &&
                        g_asMissileProximityFuseValues[missile] != 0 &&
                        distance_from_object(obj, missile) < 200) {
                        explode((short)g_acObjectOwner[obj], obj);
                        projectileDetonated = 1;
                    }
                }
            }
            if (projectileDetonated == 0 && count_down(obj) == 0) {
                if (g_asObjectType[obj] == 0x0c)
                    explode((short)g_acObjectOwner[obj], obj);
                else
                    remove_object(obj);
            }
            break;
        case OBJECT_CLASS_DUST:
            if (g_asObjectType[obj] == 0x2f &&
                count_down(obj) == -1 &&
                g_asObjectScreenX[obj] == (short)0x8001)
                remove_object(obj);
            break;
        case OBJECT_CLASS_EXPLOSION:
            if (((g_asObjectType[obj] == 0x25 &&
                  g_asObjectAnimationIndex[obj] == 0x0c)
#ifdef SDL_PORT
                 /* SO2.exe 0x4412D7 flashes the first six animation ticks. */
                 || (g_asObjectType[obj] == OBJECT_TYPE_NUCLEAR_EXPLOSION &&
                     g_asObjectAnimationIndex[obj] >= 0 &&
                     g_asObjectAnimationIndex[obj] <= 5)
#endif
                ) &&
                g_asObjectScreenX[obj] != (short)0x8001) {
                if (g_nSpacePaletteFadeMode == 0x13) {
                    g_nSpaceBlastFlashIncrement = 0;
                    g_1_asSpacePaletteFade[0] =
                        (short)(g_aSpaceExplosionFlashPalette[0][0]
                                << 2);
                    g_1_asSpacePaletteFade[1] =
                        (short)(g_aSpaceExplosionFlashPalette[0][0]
                                << 2);
                    g_1_asSpacePaletteFade[2] = 0;
                    SetPaletteEntry(
                        (short)g_cPrimaryViewBufferColour,
                        g_1_asSpacePaletteFade);
                    g_nSpaceExplosionFlashActive++;
                } else {
                    ClearViewport(
                        &g_27_stViewScratchInfo,
                        (short)g_bPrimaryViewBufferColour);
                    g_bViewportDirty++;
                }
            }
#ifdef SDL_PORT
            if (g_asObjectType[obj] == OBJECT_TYPE_NUCLEAR_EXPLOSION &&
                g_asObjectAnimationIndex[obj] == 6) {
                /* SO2.exe 0x441395 clears the current hazard field. */
                remove_all_hazards();
                g_nHazardFieldCount = 0;
            }
#endif
            break;
        case OBJECT_CLASS_DEBRIS:
            if (g_asObjectType[obj] == 0x3d ||
                g_asObjectType[obj] ==
                    OBJECT_DATA_EJECTION_POD)
                break;
            if (count_down(obj) == -1 &&
                g_asObjectScreenX[obj] == (short)0x8001)
                remove_object(obj);
            break;
        case OBJECT_CLASS_MINE:
            if (g_acObjectCollisionGraceTicks[obj] > 0)
                g_acObjectCollisionGraceTicks[obj]--;
            if (count_down(obj) == 0)
                explode(obj, obj);
            break;
        case OBJECT_CLASS_MISSILE:
            g_acShipExhaustHeat[obj] = 0;
            if (g_asObjectType[obj] ==
                    OBJECT_DATA_CHAFF_POD) {
                if (count_down(obj) <= 0)
                    explode(obj, obj);
            } else {
                if (g_acObjectCollisionGraceTicks[obj] > 0)
                    g_acObjectCollisionGraceTicks[obj]--;
                if (g_asObjectType[obj] !=
                        OBJECT_DATA_DART_DUMB_FIRE_MISSILE &&
                    g_asShipTactic[obj] == TACTIC_RAM &&
#ifdef SDL_PORT
                    g_acShipTarget[obj] != -1 &&
#endif
                    g_anShipCloakState[
                        g_acShipTarget[obj]] == 1) {
                    explode(obj, obj);
                } else if (g_asShipTactic[obj] ==
                               TACTIC_SIT_STILL &&
                           g_nCannedSceneMode == 0) {
                    if (count_down(obj) <= 0) {
                        g_asShipTactic[obj] = TACTIC_RAM;
                        g_asObjectCounter[obj] = (short)(
                            *(int *)&g_aObjectTypeData[
                                g_acObjectType[obj]].lifetime);
                        if (g_asObjectType[obj] ==
                                OBJECT_DATA_DART_DUMB_FIRE_MISSILE
#ifdef SDL_PORT
                            || g_asObjectType[obj] == OBJECT_TYPE_MACE
#endif
                            ) {
                            vector_component_in_dir(
                                &g_aShipVelocity[obj],
                                &g_aShipForwardVector[obj],
                                &g_aShipVelocity[obj]);
                        }
                    }
                } else if (count_down(obj) <= 0) {
                    explode(obj, obj);
                }
            }
            break;
        case OBJECT_CLASS_SHIP:
        case OBJECT_CLASS_CAPITAL_SHIP:
        case OBJECT_CLASS_BASE:
            if (g_2_asFlakCooldownTable[obj] > 0)
                g_2_asFlakCooldownTable[obj]--;
            if (g_asProjectileEvasionTicks[obj] > 0)
                g_asProjectileEvasionTicks[obj]--;
            if (g_2_asFlakCooldownTable[obj] > 0 &&
                g_asObjectType[obj] < 0x3a)
                g_2_asFlakCooldownTable[obj]--;
            if (g_asSpikeriCooldown[obj] > 0)
                g_asSpikeriCooldown[obj]--;
            if (g_7_asShipBurstStageCountdown[obj] > 0)
                g_7_asShipBurstStageCountdown[obj]--;
            /* Masked off with "& 0" in the original; the compiler keeps
             * the dead test behind a jump. */
            if ((g_nSpaceFrame & 0) &&
                g_asPlayerHitsCreditValues[obj] > 0)
                g_asPlayerHitsCreditValues[obj]--;
            for (gun = 0; gun < 3; gun++) {
                if (g_27_aasGunCooldown[obj][gun] > 0)
                    g_27_aasGunCooldown[obj][gun]--;
            }
            if (g_asShipCloakCooldown[obj] > 0)
                g_asShipCloakCooldown[obj]--;
            g_asShipCloakElapsedFrames[obj]++;
            if (g_asShipFriendlyFireCooldown[obj] > 0)
                g_asShipFriendlyFireCooldown[obj]--;
            g_acShipExhaustHeat[obj] = 0;
            if (count_down(obj) > 0) {
                if (g_asShipManeuver[obj] ==
                        MANEUVER_WARPING_OUT)
                    g_asObjectScale[obj] >>= 1;
                if (g_aeSpecialManeuver[obj] ==
                        SPECIAL_MANEUVER_UNKNOWN_9 &&
                    g_aeObjectClass[obj] >=
                        OBJECT_CLASS_CAPITAL_SHIP) {
                    if ((g_aObjectTypeData[OBJECT_DATA_INDEX(OBJECT_TYPE_EXPLOSION2)].shapeSet !=
                          0 ? 8 : 4) - 1 ==
                            g_asObjectCounter[obj]) {
                        ShipExplosion(obj);
                        explosion_shock_wave(
                            obj, g_aObjectTypeData[
                                g_acObjectType[obj]].
                                    explosionDamage);
                    } else {
                        while ((unsigned short)RandomInRange(0, 100) < 50)
                            onboard_explosion(obj);
                    }
                }
            } else if (g_asObjectCounter[obj] == 0) {
                if (g_aeSpecialManeuver[obj] ==
                        SPECIAL_MANEUVER_UNKNOWN_9) {
                    if (g_nYourWingman != -1 &&
                        g_acShipLastAttacker[obj] == 0 &&
                        g_asShipSide[obj] == SIDE_KILRATHI &&
                        RandomBelowOrEqual(100) < 20) {
                        send_message(g_nYourWingman, 7);
                    }
                    Create_explosion_debris(obj);
                    if (g_nArcadeState != 0)
                        return;
                    if (g_bMissionDeathSequencePending != 0 &&
                        g_nPendingEjectionShip == obj)
                        SpawnMissionEjectionPod(obj);
                    break;
                }
                switch (g_asShipManeuver[obj]) {
                case MANEUVER_WARPING_OUT:
                    if (g_asShipSide[obj] != SIDE_NEUTRAL) {
                        if (obj != 0 && obj != g_nYourWingman) {
                            g_aMissionShips[
                                g_asShipMissionIndex[obj]].state = 2;
                            remove_object(obj);
                            if (g_nCannedSceneMode == 0)
                                RecordCannedSceneObjectEvent(obj, 1);
                        } else if (obj == 0) {
                            CompleteStarSystemJump();
                        }
                    }
                    break;
                case MANEUVER_WARPING_IN:
                    if (g_asShipTactic[obj] != TACTIC_WARP_IN) {
                        if ((short)g_acObjectOwner[obj] != obj) {
                            remove_object(obj);
                        } else {
                            set_objects_data(
                                obj,
                                (short)g_abShipNavPointIndex[obj],
                                -1, 1);
                            reset_maneuver(obj, -1);
                        }
                    }
                    break;
                }
            }
            if (g_asShipMissionIndex[obj] ==
                    g_6_nHomeOperationShipIndexValue &&
                g_bCarrierLandingEnabled != 0 &&
                g_bLandingCommRequestPending != 0 &&
                normal_speed(0) != 0 &&
                distance_from_object(0, obj) < 700) {
                g_nPlayerCollisionObject = obj;
                g_nArcadeState = 1;
            }
            break;
        }
    }
}

/* Function start: 0x41C7ED */
void update_objects_in_space(void)
{
    short obj;

#ifndef SDL_PORT
    if (g_nCannedSceneMode != 0) {
        if (g_bSceneEscapeRequested != 0) {
            g_nCannedSceneRecordedFrameCountNumber =
                g_5_nCannedSceneFrameCount;
        } else {
            g_nCannedSceneRecordedFrameCountNumber++;
            if (g_nCannedSceneSegmentEndFrame ==
                g_nSpaceFrame)
                HandleCannedSceneBufferBoundary();
            ApplyCannedSceneFrameEvents();
        }
    } else {
        g_5_nCannedSceneFrameCount++;
        if (g_nCannedSceneBufferNearCapacityFlag != 0)
            HandleCannedSceneBufferBoundary();
    }
#endif

    for (obj = 0; obj <= SPACE_LAST_MOVING_OBJECT; obj++) {
        if (g_aeObjectClass[obj] == OBJECT_CLASS_NULL) {
            if (g_nCannedSceneMode == 0 &&
                g_aeSpecialManeuver[obj] ==
                    SPECIAL_MANEUVER_UNKNOWN_9)
                RecordCannedSceneObjectState(obj);
            continue;
        }
        if (g_aeObjectClass[obj] == OBJECT_CLASS_FUTURION) {
            futurion_intelligence(obj);
            continue;
        } else if (g_aeObjectClass[obj] > OBJECT_CLASS_PLANET) {
            animate_object(obj);
            if (g_nCannedSceneMode != 0) {
                AddFixedVectors(&g_aShipPosition[obj],
                                &g_aShipVelocity[obj],
                                &g_aShipPosition[obj]);
                if (obj < 10)
                    fix_objects_ijk(obj);
                continue;
            }
            if (g_aeObjectClass[obj] >=
                OBJECT_CLASS_PROJECTILE) {
                object_collision(obj);
                rotate_object(obj);
                if (obj >= 10 ||
                    g_aeSpecialManeuver[obj] !=
                        SPECIAL_MANEUVER_UNKNOWN_9) {
                    if (obj != 0 ||
                        g_bJumpSequenceActive != 0) {
                        object_intelligence(obj);
                    } else if (g_nCurrentView == 4) {
                        UpdateSavedTorpedoTargetLock();
                    }
                    if (g_aeObjectClass[obj] ==
                            OBJECT_CLASS_SHIP &&
                        g_asShipTurretFireEnabled[obj] != 0)
                        UpdateShipTurretGuns(obj);
                    if (obj < 10 &&
                        g_aeObjectClass[obj] >=
                            OBJECT_CLASS_MISSILE) {
                        if (g_nCannedSceneMode == 0)
                            RecordCannedSceneObjectState(obj);
                        if (obj != 0 || g_nCurrentView == 4)
                            rotate_object_to_goal(obj);
                    }
                }
            }
            if (g_nCurrentView == 4 &&
                g_nAimCameraMode == 1 &&
                g_nTargetCameraObject == obj &&
                g_nTargetCameraFrame == 1)
                UpdateTargetCameraObject(obj);
            accelerate_and_move_object(obj);
            if (g_aeObjectClass[obj] >= OBJECT_CLASS_SHIP) {
                replenish_shields(obj);
                housekeep_power_plant_and_fuel(obj);
                replenish_weapon_energy_bank(obj);
            }
        }
    }
}

/* Function start: 0x41CB3C */
unsigned int rotate_object(short obj)
{
    if (g_anObjectPitchRotation[obj] != 0) {
        alter_pitch(g_anObjectPitchRotation[obj], obj);
        ClampVectorTo30(&g_anObjectPitchRotation[obj]);
    }
    if (g_anObjectYawRotation[obj] != 0) {
        alter_yaw(g_anObjectYawRotation[obj], obj);
        ClampVectorTo30(&g_anObjectYawRotation[obj]);
    }
    if (g_anObjectRollRotation[obj] != 0) {
        alter_roll(g_anObjectRollRotation[obj], obj);
        ClampVectorTo30(&g_anObjectRollRotation[obj]);
    }
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x41CC0A */
void accelerate_and_move_object(short obj)
{
    int size;
    int thrust;
    int accel;
    FixedVector delta;
    FixedVector impulse;

    if (g_aeObjectClass[obj] >= OBJECT_CLASS_MISSILE) {
        switch (g_aeSpecialManeuver[obj]) {
        case SPECIAL_MANEUVER_KILL_ENGINES:
            g_acShipExhaustHeat[obj] = 0;
            if (RandomBelowOrEqual(100) < 10)
                set_special(obj, SPECIAL_MANEUVER_NONE);
            break;
        case SPECIAL_MANEUVER_STOP_DRIFT:
            approach_zero_speed(obj);
            NormalizeAndScaleVector(&g_aShipVelocity[obj],
                                    g_anShipSpeed[obj]);
            if (g_anShipSpeed[obj] == 0)
                set_special(obj, SPECIAL_MANEUVER_NONE);
            break;
        }
        if (g_aeSpecialManeuver[obj] <
                SPECIAL_MANEUVER_KILL_ENGINES &&
            g_asShipTactic[obj] != TACTIC_SIT_STILL) {
            zero_vector(&impulse);
            switch (g_aeSpecialManeuver[obj]) {
            case SPECIAL_MANEUVER_AFTERBURNER:
                g_asShipAfterburnerTimer[obj]--;
                if (g_asShipAfterburnerTimer[obj] == 0) {
                    if (obj == 0) {
                        /* The DOS build stopped the afterburner sound here;
                         * the Win32 port compiled the body out. */
#ifdef SDL_PORT
                        /* The DOS afterburner record is held rather than
                         * timed, so the burn ending is what silences it. */
                        if (g_3_bAfterburnerSfxActive != 0) {
                            FlushSoundEffectsAndLog(0, 0);
                            g_3_bAfterburnerSfxActive = 0;
                        }
#endif
                    }
                    g_aeSpecialManeuver[obj] =
                        SPECIAL_MANEUVER_NONE;
                    ScaleFixedVector(&g_aShipForwardVector[obj],
                                     g_anShipSpeed[obj], &delta);
                } else {
                    thrust = (g_aObjectTypeData[
                                     g_acObjectType[obj]]
                                         .maximumVelocity *
                                     2 +
                                 40) << 8;
                    ScaleFixedVector(&g_aShipForwardVector[obj],
                                     thrust, &delta);
                    drain_fuel(obj, 200);
                    g_acShipExhaustHeat[obj] = 3;
                }
                break;
            case SPECIAL_MANEUVER_SUPER_BRAKE:
                g_asShipAfterburnerTimer[obj]--;
                if (g_asShipAfterburnerTimer[obj] == 0) {
                    g_aeSpecialManeuver[obj] =
                        SPECIAL_MANEUVER_NONE;
                    ScaleFixedVector(&g_aShipForwardVector[obj],
                                     g_anShipSpeed[obj], &delta);
                } else {
                    zero_vector(&delta);
                    drain_fuel(obj, 140);
                }
                break;
            default:
                ScaleFixedVector(&g_aShipForwardVector[obj],
                                 g_anShipSpeed[obj], &delta);
                break;
            }
            ComputeVectorDelta(&g_aShipVelocity[obj], &delta,
                               &delta);
            size = Vector_magnitude(&delta);
            if (size > 0) {
                accel = GetShipAccelerationRate(obj);
                if (alert_flag(obj, 1) != 0 && accel < 0x500)
                    accel = 0x500;
                if (g_aeSpecialManeuver[obj] ==
                        SPECIAL_MANEUVER_AFTERBURNER ||
                    g_aeSpecialManeuver[obj] ==
                        SPECIAL_MANEUVER_SUPER_BRAKE) {
                    accel *= 2;
                }
                accel = MultiplyFixed(
                    accel,
                    DivideFixed(
                        dot_product(
                            &delta,
                            &g_aShipForwardVector[obj]),
                        size) + 0x200) >> 1;
                accel = MinInt(DivideFixed(accel, size), 0x100);
                ScaleFixedVector(&delta, accel, &impulse);
                if (g_aeSpecialManeuver[obj] !=
                        SPECIAL_MANEUVER_AFTERBURNER) {
                    g_acShipExhaustHeat[obj] = 2;
                }
            }
            AddFixedVectors(&impulse,
                            &g_aShipVelocity[obj],
                            &g_aShipVelocity[obj]);
            if (obj == 0)
                g_vPlayerAcceleration = impulse;
        }
    }
    AddFixedVectors(&g_aShipPosition[obj],
                    &g_aShipVelocity[obj],
                    &g_aShipPosition[obj]);
}

/* Function start: 0x41D07B */
void animate_shape(short obj)
{
    short type;
    unsigned char *sequence;
    short entry;

    type = g_acObjectType[obj];
    sequence = g_aObjectTypeData[type].animation;
    if (sequence == 0)
        return;
    if (--g_acObjectAnimationDelay[obj] > 0)
        return;
    g_acObjectAnimationDelay[obj] =
        g_aObjectTypeData[type].yawRate;
    entry = *(unsigned short *)(sequence +
              g_asObjectAnimationIndex[obj] * 2);
    switch (entry & 0xf000) {
    case 0x9000:
        g_asObjectAnimationIndex[obj] = entry & 0x0fff;
        entry = *(unsigned short *)(sequence +
                  g_asObjectAnimationIndex[obj] * 2);
        if (g_asObjectScreenX[obj] != (short)0x8001 &&
            (g_asObjectType[obj] == 0x20 ||
             g_asObjectType[obj] == 0x1f ||
             g_asObjectType[obj] == 0x3d)) {
            PlaySfxWaveFileByNumber(13, obj, 0);
        }
        break;
    case 0xa000:
        remove_object(obj);
        return;
    }

    switch (entry & 0x0c00) {
    case 0x0400:
        g_asObjectScale[obj] +=
            (entry & 0x3f) *
            ((unsigned short)g_asObjectScale[obj] >> 6);
        break;
    case 0x0800:
        g_asObjectScale[obj] -=
            (entry & 0x3f) *
            ((unsigned short)g_asObjectScale[obj] >> 6);
        break;
    default:
        entry &= 0x3f;
        g_asObjectViewFrame[obj] = entry;
    }
    g_asObjectFlip[obj] = (entry & 0xc0) >> 2;
    g_asObjectAnimationIndex[obj]++;
}

/* Function start: 0x41D2DA */
void animate_object(short obj)
{
    FixedVector offset;
    short effect;

    switch (g_aeObjectClass[obj]) {
    case OBJECT_CLASS_EXPLOSION:
    case OBJECT_CLASS_DEBRIS:
    case OBJECT_CLASS_FIXED_OBJECT:
    case OBJECT_CLASS_ASTEROID:
    case OBJECT_CLASS_MINE:
        animate_shape(obj);
        break;
    case OBJECT_CLASS_SHIP:
        if (g_asObjectScreenX[obj] != (short)0x8001 &&
            (g_nRenderedSpaceFrame & 3) == 0 &&
            (g_aObjectTypeData[
                 g_acObjectType[obj]].damageCapacity >> 1) - 1 <=
                g_asObjectDamage[obj]) {
            effect = find_vacant_3d_object();
            if (effect != -1) {
                ScaleFixedVector(&g_aShipForwardVector[obj],
                                 -((int)g_asObjectCollisionRadius[obj]
                                     << 8),
                                 &offset);
                AddFixedVectors(&g_aShipPosition[obj], &offset,
                                &g_aShipPosition[effect]);
                FillFixedVectorWithRandomComponents(20, &offset);
                AddFixedVectors(&g_aShipPosition[effect], &offset,
                                &g_aShipPosition[effect]);
                zero_vector(&g_aShipVelocity[effect]);
                set_objects_data(
                    effect,
                    (enum ObjectType)((unsigned short)RandomInRange(0, 2) +
                                      OBJECT_DATA_SHIP_DAMAGE_EFFECT_FIRST),
                    obj, 0);
                g_asObjectScale[effect] =
                    g_asObjectScale[obj];
                if (RandomInRange(0, 3) == 0)
                    PlaySfxWaveFileByNumber(7, obj, 0);
            }
        }
        break;
    }
}

/* Function start: 0x41D4C7 */
void hit_asteroid(short asteroid, short destructionChance)
{
    short fragments;

    if (RandomBelowOrEqual((short)(destructionChance - 1)) == 0) {
        destructionChance = (short)(RandomBelowOrEqual(1) + 2);
        for (fragments = 0; fragments < destructionChance; fragments++) {
            make_shard(asteroid, g_aShipVelocity[asteroid]);
        }
        explode(-1, asteroid);
    } else if (RandomBelowOrEqual(7) == 0) {
        make_shard(asteroid, g_vCollisionDelta);
    }
}

/* Function start: 0x41D591 */
void object_collision(short obj)
{
    short damage;
    short scale;
    SphericalVector impactBearing;
    short adaptiveDamageScale;
    FixedVector component;
    int elasticity;
    FixedVector offAxis;
    short objectMass;
    short otherMass;
    int forceMagnitude;
    FixedVector hitDirection;
    FixedVector otherComponent;
    FixedVector relativePosition;
    short impactDamage;
    short lifespan;
    FixedVector closingVelocity;
    int total;
    FixedVector cameraSpace;
    short travelSpeed;
    short collider;

    objectMass = g_aObjectTypeData[
        g_acObjectType[obj]].radarRadius;
    collider = check_for_collision(obj);
    if (collider != -1) {
        if (g_bPlayerCollisionEnabled == 0 &&
            (collider == 0 || obj == 0))
            return;
        otherMass = g_aObjectTypeData[
            g_acObjectType[collider]].radarRadius;
        NormalizeFixedVector(&g_vCollisionDelta);
        ComputeVectorDelta(&g_aShipVelocity[collider],
                           &g_aShipVelocity[obj],
                           &hitDirection);
        NormalizeFixedVector(&hitDirection);

        switch (g_aeObjectClass[obj]) {
        case OBJECT_CLASS_PROJECTILE:
            if (g_acObjectOwner[obj] != collider &&
                g_aeObjectClass[collider] !=
                    OBJECT_CLASS_PROJECTILE) {
                if (g_aeObjectClass[collider] >=
                        OBJECT_CLASS_MISSILE) {
                    if (collider == 0) {
                        ComputeVectorDelta(
                            &g_aShipPosition[EYE_OBJECT],
                            &g_aShipPosition[obj],
                            &relativePosition);
                        transform_to_objects_frame(
                            &relativePosition, &cameraSpace, EYE_OBJECT);
                        rectangular_to_spherical(&cameraSpace,
                                                 &impactBearing);
                        if (abs(impactBearing.pitch) < 45) {
                            if (abs(impactBearing.yaw) < 45)
                                g_9_aasCockpitHitPaletteFades[1][0] =
                                    0x38;
                            else if (abs(impactBearing.yaw) > 135)
                                g_9_aasCockpitHitPaletteFades[0][0] =
                                    0x38;
                            else if (impactBearing.yaw < 0)
                                g_9_aasCockpitHitPaletteFades[3][0] =
                                    0x38;
                            else
                                g_9_aasCockpitHitPaletteFades[5][0] =
                                    0x38;
                        } else if (impactBearing.pitch < 0) {
                            g_9_aasCockpitHitPaletteFades[2][0] =
                                0x38;
                        } else {
                            g_9_aasCockpitHitPaletteFades[4][0] =
                                0x38;
                        }
                    }
                    g_acShipLastAttacker[collider] =
                        g_acObjectOwner[obj];
                    g_acShipCollisionCooldown[collider] += 4;
                    if (g_abProjectileCollisionBonus[obj] != 0)
                        g_asProjectileEvasionTicks[collider] += 4;
                    if (g_asObjectType[obj] == 9 ||
                        g_asObjectType[obj] == 0x0e) {
                        damage = g_asObjectDamage[obj];
                    } else {
                        lifespan = *(int *)&g_aObjectTypeData[
                            g_acObjectType[obj]].lifetime;
                        damage = (short)(
                            (g_asObjectCounter[obj] / lifespan +
                             1) *
                            (g_asObjectDamage[obj] >> 1));
                    }
                    adaptiveDamageScale = MinShort(
                        150, MaxShort(70, GetAdaptiveTurnRate()));
                    if (g_asShipSide[collider] == 0) {
                        damage = (short)(adaptiveDamageScale * damage /
                                         100);
                    }
                    component = g_aShipVelocity[obj];
                    NormalizeFixedVector(&component);
                    ScaleFixedVector(&component, (int)damage << 8,
                                     &component);
                    negate_vector(&g_vCollisionDelta);
                    apply_force_to_object(&g_vCollisionDelta,
                                          &component, collider);
                    g_bApplyingCollisionDamage = 1;
                    inflict_damage(obj, collider, damage, &hitDirection);
                    g_bApplyingCollisionDamage = 0;
                }
                scale = g_asObjectScale[obj];
                set_objects_data(
                    obj, OBJECT_DATA_PROJECTILE_IMPACT_EFFECT,
                    g_acObjectOwner[obj], 0);
                g_asObjectScale[obj] =
                    (unsigned short)(scale * 2);
                g_aShipVelocity[obj] =
                    g_aShipVelocity[collider];
                RecordCannedSceneObjectEvent(obj, 0);
                if (g_aeObjectClass[collider] ==
                        OBJECT_CLASS_ASTEROID)
                    hit_asteroid(collider, 3);
            }
            break;

        case OBJECT_CLASS_MINE:
            if (g_acObjectOwner[obj] == collider ||
                g_acObjectCollisionGraceTicks[obj] > 0)
                return;
            if (easy2see(obj) == 0 &&
                (g_nCurrentView == 0 || collider != 0))
                remove_object(obj);
            else
                explode(obj, obj);
            break;

        case OBJECT_CLASS_MISSILE:
            if (g_acObjectOwner[obj] != collider ||
                g_acObjectCollisionGraceTicks[obj] <= 0) {
                ScaleFixedVector(&g_aShipVelocity[obj],
                                 (int)objectMass << 8, &component);
                negate_vector(&g_vCollisionDelta);
                apply_force_to_object(&g_vCollisionDelta,
                                      &component, collider);
                explode(obj, obj);
                zero_vector(&g_aShipVelocity[obj]);
            }
            break;

        case OBJECT_CLASS_ASTEROID:
            if (g_aeObjectClass[collider] ==
                    OBJECT_CLASS_ASTEROID) {
                if (g_asObjectScreenX[obj] == (short)0x8001)
                    remove_object(obj);
                else
                    hit_asteroid(obj, 0);
            }
            break;

        case OBJECT_CLASS_SHIP:
        case OBJECT_CLASS_CAPITAL_SHIP:
        case OBJECT_CLASS_BASE:
            if ((g_aeObjectClass[collider] ==
                     OBJECT_CLASS_ASTEROID ||
                 (g_aeObjectClass[collider] ==
                      OBJECT_CLASS_MINE &&
                  g_acObjectOwner[collider] == -1)) &&
                easy2see(collider) == 0 &&
                (g_nCurrentView == 0 || obj != 0))
                remove_object(collider);
            if ((g_aeObjectClass[collider] ==
                     OBJECT_CLASS_ASTEROID ||
                 g_aeObjectClass[collider] ==
                     OBJECT_CLASS_SHIP ||
                 g_aeObjectClass[collider] >=
                     OBJECT_CLASS_CAPITAL_SHIP) &&
                g_acLastCollisionObject[obj] != collider) {
                PlaySfxWaveFileByNumber(0x1c, obj, 0);
                g_acLastCollisionObject[collider] =
                    (signed char)obj;
                g_acLastCollisionObject[obj] =
                    (signed char)collider;

                forceMagnitude =
                    Vector_magnitude(&g_vCollisionDelta);
                forceMagnitude = DivideFixed(
#ifdef SDL_PORT
                    (SdlGetBalancedCollisionRadius(collider) +
                     SdlGetBalancedCollisionRadius(obj)) << 8,
#else
                    (g_asObjectCollisionRadius[collider] +
                     g_asObjectCollisionRadius[obj]) << 8,
#endif
                    forceMagnitude);
                forceMagnitude = MinInt(forceMagnitude, 0x7d000);
                ScaleFixedVector(&g_vCollisionDelta,
                                 forceMagnitude, &component);
                AddFixedVectors(&g_aShipPosition[obj], &component,
                                &g_aShipPosition[collider]);

                vector_component_in_dir(
                    &g_aShipVelocity[obj],
                    &g_vCollisionDelta, &component);
                SubtractFixedVectors(&g_aShipVelocity[obj],
                                     &component, &offAxis);
                vector_component_in_dir(
                    &g_aShipVelocity[collider],
                    &g_vCollisionDelta, &otherComponent);
                ComputeVectorDelta(&otherComponent, &component,
                                   &closingVelocity);
                impactDamage =
                    (short)(Vector_magnitude(&closingVelocity) >> 8);
#ifdef SDL_PORT
                if (obj == 0 || collider == 0)
                    SdlQueueJoystickCollisionRumble(impactDamage);
#endif
                impactDamage = (short)((impactDamage * impactDamage) >> 1);
                total = otherMass + objectMass;
                elasticity = COLLISION_MASS_DIFFERENCE_SCALE(
                                 objectMass, otherMass) /
                             total;
                elasticity = MinInt(MaxInt(0x40, elasticity), 0x400);
                ScaleFixedVector(&closingVelocity, elasticity, &component);
                AddFixedVectors(&component, &otherComponent, &component);
                forceMagnitude = MultiplyFixed(
                    (int)objectMass * 0x600,
                    Vector_magnitude(&component)) + 0xa00;
                travelSpeed = (short)(Vector_magnitude(
                    &g_aShipVelocity[obj]) >> 8);
                if (travelSpeed < COLLISION_MAXIMUM_VELOCITY(obj)) {
                    AddFixedVectors(&component,
                                    &g_aShipVelocity[obj],
                                    &g_aShipVelocity[obj]);
                }

                if (g_aeObjectClass[obj] == OBJECT_CLASS_SHIP) {
                    NormalizeFixedVector(&offAxis);
                    negate_vector(&offAxis);
                    ScaleFixedVector(&offAxis, forceMagnitude, &offAxis);
                    rotational_acceleration(&g_vCollisionDelta,
                                            &offAxis, obj);
                    negate_vector(&hitDirection);
                    if (g_asObjectType[obj] == 0x33 &&
                        g_aeObjectClass[collider] ==
                            OBJECT_CLASS_ASTEROID)
                        impactDamage /= 2;
                    inflict_damage(collider, obj, impactDamage,
                                   &hitDirection);
                    negate_vector(&hitDirection);
                }

                SubtractFixedVectors(&g_aShipVelocity[collider],
                                     &otherComponent, &offAxis);
                elasticity = ((objectMass + objectMass) << 8) / total;
                elasticity = MinInt(MaxInt(0x40, elasticity), 0x400);
                ScaleFixedVector(&closingVelocity, elasticity, &component);
                AddFixedVectors(&component, &otherComponent, &component);
                travelSpeed = (short)(Vector_magnitude(
                    &g_aShipVelocity[obj]) >> 8);
                if (travelSpeed < COLLISION_MAXIMUM_VELOCITY(collider)) {
                    AddFixedVectors(&component,
                                    &g_aShipVelocity[collider],
                                    &g_aShipVelocity[collider]);
                }
                if (g_aeObjectClass[collider] ==
                        OBJECT_CLASS_SHIP) {
                    NormalizeFixedVector(&offAxis);
                    negate_vector(&offAxis);
                    ScaleFixedVector(&offAxis, forceMagnitude, &offAxis);
                    negate_vector(&g_vCollisionDelta);
                    rotational_acceleration(&g_vCollisionDelta,
                                            &offAxis, collider);
                    inflict_damage(obj, collider, impactDamage,
                                   &hitDirection);
                }
                if (g_aeObjectClass[collider] >=
                        OBJECT_CLASS_CAPITAL_SHIP) {
                    SubtractFixedVectors(&g_aShipPosition[obj],
                                         &g_aShipVelocity[obj],
                                         &g_aShipPosition[obj]);
                    g_anShipSpeed[obj] = 0;
                    g_aShipVelocity[obj] =
                        g_aShipVelocity[collider];
                }
            }
            break;
        }
    } else {
        g_acLastCollisionObject[obj] = -1;
    }
}

/* Function start: 0x41E276 */
void object_intelligence(short obj)
{
    short target;

    if (g_nTrainSimActive == 4)
        return;
    switch (g_aeObjectClass[obj]) {
    case OBJECT_CLASS_SHIP:
        if (g_asObjectType[obj] == 0x33 &&
            (obj == 0 ||
             (g_bJumpSequenceActive != 0 &&
              g_nYourWingman == obj))) {
            capital_ship_intelligence(obj);
        } else {
            ship_intelligence(obj);
        }
        break;
    case OBJECT_CLASS_CAPITAL_SHIP:
        capital_ship_intelligence(obj);
        break;
    case OBJECT_CLASS_BASE:
        stationary_intelligence(obj);
        break;
    case OBJECT_CLASS_FUTURION:
        futurion_intelligence(obj);
        break;
    case OBJECT_CLASS_MINE:
        mine_intelligence(obj);
        break;
    case OBJECT_CLASS_MISSILE:
        if (g_nSpaceFrame % 4 == 0 ||
            g_nExternalViewShip == obj) {
            target = g_acShipTarget[obj];
            if (target != -1)
                get_facing_range_from_object(obj, target);
            switch (g_asObjectType[obj]) {
            case OBJECT_DATA_DART_DUMB_FIRE_MISSILE:
#ifdef SDL_PORT
            case OBJECT_TYPE_MACE:
#endif
                g_anShipSpeed[obj] =
                    (get_ship_max_velocity(obj) + 10) * 0x100;
                break;
            case OBJECT_DATA_JAVELIN_HEAT_SEEKING_MISSILE:
                heat_seeking_missile_intelligence(obj);
                break;
            case OBJECT_DATA_PILUM_FRIEND_OR_FOE_MISSILE:
                FF_missile_intelligence(obj);
                break;
            case OBJECT_DATA_SPICULUM_IMAGE_RECOGNITION_MISSILE:
            case OBJECT_DATA_TORPEDO:
                point_ship(obj, 0, &g_vToTarget);
                g_anShipSpeed[obj] =
                    (get_ship_max_velocity(obj) + 10) * 0x100;
                break;
            case OBJECT_DATA_CHAFF_POD:
                chaff_intelligence(obj);
                break;
            }
        }
        break;
    }
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
FixedVector g_vExternalViewOffset; /* 0x005d3200 */
short g_nStarFieldJRotationNumber; /* 0x005d320c */
short g_2_nExternalViewDistanceValue; /* 0x005d320e */
short g_1_nStarFieldIRotation; /* 0x005d3210 */
short g_7_nExternalViewTimeout; /* 0x005d3212 */
