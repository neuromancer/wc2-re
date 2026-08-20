/*
 *  Space-object simulation (Mac `spc` compilation unit), preceded by the
 *  Win32 joystick calibration/input helpers in this provisional range.
 *
 *  Address range 0x410000-0x412fff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: the Mac `spc` symbol run maps rotate_eye_to_goal through
 *  object_intelligence to 0x410a30-0x4139ff.
 */
#include "wc1.h"

#pragma function(abs)

int g_nCapitalShipViewDistance_00492fa4 = 0x7d000;
short g_bAlternateChaseView_00492fac = 0;
int g_nChaseCameraMaximumVelocity_00492fb0 = 0x8c00;
short g_nSavedPlayerTarget_0049d460 = -1;
short g_bSavedPlayerTarget_0049d464;
short g_asShipTurretFireEnabled_0049d470[10];
short g_nSavedTargetLockCountdown_0049d484 = 1000;
short g_bSavedTargetLockAcquired_0049d488;
short g_nCockpitCameraObject_0049d770;
FixedVector g_vExternalViewOffset_005d3200;

/* Function start: 0x418770 */
void CalibrateJoystickInteractive()
{
    InputDeviceSample samples[2];
    short calibration[6];
    short shown;
    int file;
    int failed;
    int fontLoaded;

    fontLoaded = 0;
    if (g_nActiveInputDevice_005d1726 == -1)
        return;
    if (g_apTextFonts_005d2200[1] != 0)
        fontLoaded = 1;

    g_stDefaultTextContext_005d2d20.alignment = 2;
    InitializeTextContextFromFont(&g_stDefaultTextContext_005d2d20, 1,
        g_bPrimaryViewBufferColour_0049cb50, (signed char)g_cSecondaryViewBufferColour_0049cb4c);
    shown = ShowModalTextPanel(1,
        "Turn AUTO FIRE off if present, press a button");
    if (shown != 0) {
        DIBslamReal();
        WaitForJoystickButtonPress();
        WaitForJoystickButtonRelease();
        ReleaseModalTextPanel();
    }

    shown = ShowModalTextPanel(1,
        "Move stick to the UPPER LEFT, press a button");
    if (shown != 0) {
        DIBslamReal();
        WaitForJoystickButtonPress();
        SampleJoystickDevice(samples, g_nActiveInputDevice_005d1726, 0x7fff);
        g_nJoystickMinimumX_005d174c = samples[g_nActiveInputDevice_005d1726].x;
        g_nJoystickMinimumY_005d1750 = samples[g_nActiveInputDevice_005d1726].y;
        calibration[0] = (short)samples[g_nActiveInputDevice_005d1726].x;
        calibration[1] = (short)samples[g_nActiveInputDevice_005d1726].y;
        WaitForJoystickButtonRelease();
        ReleaseModalTextPanel();
    }

    shown = ShowModalTextPanel(1,
        "Move stick to the LOWER RIGHT, press a button");
    if (shown != 0) {
        DIBslamReal();
        WaitForJoystickButtonPress();
        SampleJoystickDevice(samples, g_nActiveInputDevice_005d1726, 0x7fff);
        g_nJoystickMaximumX_005d1744 = samples[g_nActiveInputDevice_005d1726].x;
        g_nJoystickMaximumY_005d1748 = samples[g_nActiveInputDevice_005d1726].y;
        calibration[2] = (short)samples[g_nActiveInputDevice_005d1726].x;
        calibration[3] = (short)samples[g_nActiveInputDevice_005d1726].y;
        WaitForJoystickButtonRelease();
        ReleaseModalTextPanel();
    }

    shown = ShowModalTextPanel(1,
        "Center Joystick, press a button");
    if (shown != 0) {
        DIBslamReal();
        WaitForJoystickButtonPress();
        SampleJoystickDevice(samples, g_nActiveInputDevice_005d1726, 0x7fff);
        g_nJoystickCentreX_005d1768 = samples[g_nActiveInputDevice_005d1726].x;
        g_nJoystickCentreY_005d1764 = samples[g_nActiveInputDevice_005d1726].y;
        calibration[4] = (short)samples[g_nActiveInputDevice_005d1726].x;
        calibration[5] = (short)samples[g_nActiveInputDevice_005d1726].y;
        WaitForJoystickButtonRelease();
        ReleaseModalTextPanel();
    }

    if (fontLoaded == 0)
        ReleaseTextFont(1);

    if (g_nJoystickHorizontalRange_005d1758 == 0)
        g_nJoystickHorizontalRange_005d1758 = 9;
    if (g_nJoystickVerticalRange_005d1754 == 0)
        g_nJoystickVerticalRange_005d1754 = 9;
    g_nJoystickLeftScale_005d1740 =
        (g_nJoystickCentreX_005d1768 - g_nJoystickMinimumX_005d174c) /
        g_nJoystickHorizontalRange_005d1758;
    g_nJoystickRightScale_005d175c =
        (g_nJoystickMaximumX_005d1744 - g_nJoystickCentreX_005d1768) /
        g_nJoystickHorizontalRange_005d1758;
    g_nJoystickUpScale_005d173c =
        (g_nJoystickCentreY_005d1764 - g_nJoystickMinimumY_005d1750) /
        g_nJoystickVerticalRange_005d1754;
    g_nJoystickDownScale_005d1760 =
        (g_nJoystickMaximumY_005d1748 - g_nJoystickCentreY_005d1764) /
        g_nJoystickVerticalRange_005d1754;
    if (g_nJoystickLeftScale_005d1740 == 0)
        g_nJoystickLeftScale_005d1740 = 1;
    if (g_nJoystickRightScale_005d175c == 0)
        g_nJoystickRightScale_005d175c = 1;
    if (g_nJoystickUpScale_005d173c == 0)
        g_nJoystickUpScale_005d173c = 1;
    if (g_nJoystickDownScale_005d1760 == 0)
        g_nJoystickDownScale_005d1760 = 1;

    g_nJoystickMinimumX_005d174c = g_nJoystickCentreX_005d1768 -
        g_nJoystickLeftScale_005d1740 *
            g_nJoystickHorizontalRange_005d1758;
    g_nJoystickMinimumY_005d1750 = g_nJoystickCentreY_005d1764 -
        g_nJoystickUpScale_005d173c * g_nJoystickVerticalRange_005d1754;
    g_nJoystickMaximumX_005d1744 = g_nJoystickCentreX_005d1768 +
        g_nJoystickRightScale_005d175c *
            g_nJoystickHorizontalRange_005d1758;
    g_nJoystickMaximumY_005d1748 = g_nJoystickCentreY_005d1764 +
        g_nJoystickDownScale_005d1760 * g_nJoystickVerticalRange_005d1754;
    g_nJoystickFailureValue_005d176c = g_nJoystickMaximumX_005d1744 * 2;

    if (g_nJoystickMaximumX_005d1744 <= g_nJoystickMinimumX_005d174c ||
        g_nJoystickMaximumY_005d1748 <= g_nJoystickMinimumY_005d1750) {
        shown = ShowModalTextPanel(1,
            "FAILED! Center Joystick, press a button");
        if (shown != 0) {
            DIBslamReal();
            WaitForJoystickButtonPress();
            WaitForJoystickButtonRelease();
            LoadJoystickCalibrationFile(8, 8, 1, 1);
            ReleaseModalTextPanel();
        }
        _unlink("j.cal");
        return;
    }

    file = _open("j.cal", 0x8301, 0x180);
    if (file == -1)
        return;
    failed = _write(file, &g_nActiveInputDevice_005d1726, 2) == -1;
    if (failed == 0)
        failed = _write(file, &calibration[0], 2) == -1;
    if (failed == 0)
        failed = _write(file, &calibration[1], 2) == -1;
    if (failed == 0)
        failed = _write(file, &calibration[2], 2) == -1;
    if (failed == 0)
        failed = _write(file, &calibration[3], 2) == -1;
    if (failed == 0)
        failed = _write(file, &calibration[4], 2) == -1;
    if (failed == 0)
        failed = _write(file, &calibration[5], 2) == -1;
    _close(file);
    if (failed != 0)
        _unlink("j.cal");
}

/* Function start: 0x418D14 */
void WaitForJoystickButtonRelease(void)
{
    do {
        SampleJoystickDevice(
            &g_aInputDeviceSamples_005d1780[
                g_nActiveInputDevice_005d1726],
            g_nActiveInputDevice_005d1726, 0);
        ServiceSoundSystem();
    } while (g_aInputDeviceSamples_005d1780[
                 g_nActiveInputDevice_005d1726].buttons != 0);
}

/* Function start: 0x418D5F */
void WaitForJoystickButtonPress(void)
{
    do {
        SampleJoystickDevice(
            &g_aInputDeviceSamples_005d1780[
                g_nActiveInputDevice_005d1726],
            g_nActiveInputDevice_005d1726, 0);
        ServiceSoundSystem();
    } while (g_aInputDeviceSamples_005d1780[
                 g_nActiveInputDevice_005d1726].buttons == 0);
}

/* Function start: 0x44ADE0 */
void SetFleetOverviewView(int initializeCockpit)
{
    FixedVector centre;
    FixedVector orientation;
    FixedVector offset;
    int shipCount;
    int maximumRange;
    int playerRange;
    int cameraDistance;
    int range;
    int object;

    orientation.x = 0xff;
    orientation.y = 0xff;
    orientation.z = 0xff;
    shipCount = 0;
    if (initializeCockpit != 0) {
        free_view_buffer();
        if ((int)GetLargestFreeMemoryBlockByType(0) < 66000)
            initialize_cockpit(4);
        else
            initialize_cockpit(6);
        zero_vector(&g_aShipVelocity_00494898[WC2_EYE_OBJECT]);
    }

    object = 0;
    do {
        if (g_aeObjectClass_00495328[object] >= OBJECT_CLASS_SHIP)
            shipCount++;
        object++;
    } while (object < 10);

    if (shipCount > 1) {
        zero_vector(&centre);
        object = 0;
        do {
            if (g_aeObjectClass_00495328[object] >= OBJECT_CLASS_SHIP) {
                centre.x += g_aShipPosition_00494550[object].x / shipCount;
                centre.y += g_aShipPosition_00494550[object].y / shipCount;
                centre.z += g_aShipPosition_00494550[object].z / shipCount;
            }
            object++;
        } while (object < 10);

        maximumRange = 0x4b000;
        playerRange = 0xff;
        object = 9;
        do {
            if (g_aeObjectClass_00495328[object] >= OBJECT_CLASS_SHIP) {
                ComputeVectorDelta(&centre,
                                   &g_aShipPosition_00494550[object],
                                   &offset);
                range = Vector_magnitude(&offset);
                if (maximumRange < range)
                    maximumRange = range;
                if (object == 0)
                    playerRange = range;
            }
            object--;
        } while (object >= 0);
        if (maximumRange <= 0x1f4000)
            playerRange = maximumRange;
    } else {
        maximumRange = 0x4b000;
        playerRange = 0x4b000;
        position_relative_ijk(&centre, 0, 400, 400, 400);
    }

    cameraDistance = (playerRange >> 3) * 9 + 0x2bc00;
    g_aShipPosition_00494550[WC2_SCRATCH_VIEW_OBJECT] = centre;
    g_aShipRightVector_00493b78[WC2_SCRATCH_VIEW_OBJECT] = orientation;
    g_aShipUpVector_00493ec0[WC2_SCRATCH_VIEW_OBJECT] = orientation;
    point_at(WC2_SCRATCH_VIEW_OBJECT, g_aShipPosition_00494550[0]);

    ScaleFixedVector(
        &g_aShipRightVector_00493b78[WC2_SCRATCH_VIEW_OBJECT],
        cameraDistance >> 2, &offset);
    AddFixedVectors(&g_aShipPosition_00494550[WC2_SCRATCH_VIEW_OBJECT],
                    &offset,
                    &g_aShipPosition_00494550[WC2_SCRATCH_VIEW_OBJECT]);
    ScaleFixedVector(&g_aShipUpVector_00493ec0[WC2_SCRATCH_VIEW_OBJECT],
                     0x9600, &offset);
    AddFixedVectors(&g_aShipPosition_00494550[WC2_SCRATCH_VIEW_OBJECT],
                    &offset,
                    &g_aShipPosition_00494550[WC2_SCRATCH_VIEW_OBJECT]);
    ScaleFixedVector(
        &g_aShipForwardVector_00494208[WC2_SCRATCH_VIEW_OBJECT],
        cameraDistance, &offset);
    AddFixedVectors(&g_aShipPosition_00494550[WC2_SCRATCH_VIEW_OBJECT],
                    &offset,
                    &g_aShipPosition_00494550[WC2_SCRATCH_VIEW_OBJECT]);

    g_aShipPosition_00494550[WC2_EYE_OBJECT] =
        g_aShipPosition_00494550[WC2_SCRATCH_VIEW_OBJECT];
    g_aShipRightVector_00493b78[WC2_EYE_OBJECT] = orientation;
    g_aShipUpVector_00493ec0[WC2_EYE_OBJECT] = orientation;
    if (maximumRange < 0x271000)
        point_at(WC2_EYE_OBJECT, centre);
    else
        point_at(WC2_EYE_OBJECT, g_aShipPosition_00494550[0]);
}

/* Function start: 0x419C10 */
unsigned int rotate_eye_to_goal(void)
{
    short totalError;

    totalError = (short)(abs(g_anObjectPitchRotation_00494f38[
                                WC2_EYE_OBJECT] -
                            g_nEyePitchGoal_00493140) +
                         abs(g_anObjectYawRotation_00494fc8[
                                 WC2_EYE_OBJECT] -
                            g_nEyeYawGoal_00493144) +
                         abs(g_anObjectRollRotation_00495058[
                                 WC2_EYE_OBJECT] -
                            g_nEyeRollGoal_00493148));
    match_rotation_goal(&g_anObjectPitchRotation_00494f38[
                            WC2_EYE_OBJECT],
                        &g_nEyePitchGoal_00493140, totalError,
                        g_nEyePitchRate_0049314c);
    match_rotation_goal(&g_anObjectYawRotation_00494fc8[
                            WC2_EYE_OBJECT],
                        &g_nEyeYawGoal_00493144, totalError,
                        g_nEyeYawRate_00493150);
    match_rotation_goal(&g_anObjectRollRotation_00495058[
                            WC2_EYE_OBJECT],
                        &g_nEyeRollGoal_00493148, totalError,
                        g_nEyeRollRate_00493154);
    return 0;
}

/* Function start: 0x419CDC */
short GetVectorMagnitude(const FixedVector *vector)
{
    return FixedToShortSaturating(Vector_magnitude(vector));
}

/* Function start: 0x419D01 */
unsigned int set_eye_direction_and_position(void)
{
    FixedVector positionDelta;
    FixedVector desiredPosition;
    FixedVector adjustment;
    FixedVector vector;
    short desiredDistance;
    FixedVector direction;

    if (g_bStarSystemJumpTransition_004962ec != 0)
        goto finished;

    switch (g_nCurrentView_00492fa8) {
    case 5:
        if (g_aeObjectClass_00495328[g_cViewObject_0049313c] !=
            OBJECT_CLASS_NULL) {
            ComputeVectorDelta(
                &g_aShipPosition_00494550[WC2_EYE_OBJECT],
                &g_aShipPosition_00494550[g_cViewObject_0049313c],
                &direction);
            desiredPosition = direction;
            SetVectorFixedPoint(
                (unsigned int *)&desiredPosition,
                (short)(g_bAlternateChaseView_00492fac == 0 ? -700 : -500));
            AddFixedVectors(
                &desiredPosition,
                &g_aShipPosition_00494550[g_cViewObject_0049313c],
                &desiredPosition);
            ComputeVectorDelta(
                &g_aShipPosition_00494550[WC2_EYE_OBJECT],
                &desiredPosition, &positionDelta);
            vector = positionDelta;
            divide_vector(
                &vector,
                (g_bAlternateChaseView_00492fac == 0 ? 25 : 7) << 8,
                &vector);
            if (Vector_magnitude(&vector) >
                g_nChaseCameraMaximumVelocity_00492fb0)
                NormalizeAndScaleVector(
                    &vector, g_nChaseCameraMaximumVelocity_00492fb0);
            g_aShipVelocity_00494898[WC2_EYE_OBJECT] = vector;
            g_aShipForwardVector_00494208[WC2_EYE_OBJECT] = direction;
            shrink_vector(&g_aShipForwardVector_00494208[WC2_EYE_OBJECT]);
            fix_objects_ijk(WC2_EYE_OBJECT);
            g_nEyeRollGoal_00493148 = match_roll_orientation(
                WC2_EYE_OBJECT, (short)g_cViewObject_0049313c);
            if (g_nEyeRollGoal_00493148 != 0) {
                if (abs((int)g_nEyeRollGoal_00493148) > 4) {
                    g_nEyeRollRate_00493154 = 4;
                    rotate_eye_to_goal();
                    rotate_object(WC2_EYE_OBJECT);
                } else {
                    g_nEyeRollGoal_00493148 = 0;
                    g_aShipUpVector_00493ec0[WC2_EYE_OBJECT] =
                        g_aShipUpVector_00493ec0[0];
                }
            }
            AddFixedVectors(&g_aShipPosition_00494550[WC2_EYE_OBJECT],
                            &g_aShipVelocity_00494898[WC2_EYE_OBJECT],
                            &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
        }
        break;
    case 6:
        zero_vector(&g_aShipVelocity_00494898[WC2_EYE_OBJECT]);
        ComputeVectorDelta(
            &g_aShipPosition_00494550[WC2_EYE_OBJECT],
            &g_aShipPosition_00494550[g_cViewObject_0049313c], &vector);
        if (Vector_magnitude(&vector) > 0x7d000) {
            vector = g_aShipVelocity_00494898[g_cViewObject_0049313c];
            NormalizeFixedVector(&vector);
            g_aShipUpVector_00493ec0[WC2_EYE_OBJECT] =
                g_aShipUpVector_00493ec0[g_cViewObject_0049313c];
            if (vector.x == g_aShipUpVector_00493ec0[WC2_EYE_OBJECT].x &&
                vector.y == g_aShipUpVector_00493ec0[WC2_EYE_OBJECT].y &&
                vector.z == g_aShipUpVector_00493ec0[WC2_EYE_OBJECT].z) {
                g_aShipUpVector_00493ec0[WC2_EYE_OBJECT] =
                    g_aShipRightVector_00493b78[g_cViewObject_0049313c];
            }
            ScaleFixedVector(&vector, -0x12430, &vector);
            AddFixedVectors(
                &g_aShipPosition_00494550[g_cViewObject_0049313c], &vector,
                &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
            g_aShipForwardVector_00494208[WC2_EYE_OBJECT] = vector;
            fix_objects_ijk(WC2_EYE_OBJECT);
            ScaleFixedVector(&g_aShipRightVector_00493b78[WC2_EYE_OBJECT],
                             0x19000, &vector);
            AddFixedVectors(&g_aShipPosition_00494550[WC2_EYE_OBJECT],
                            &vector,
                            &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
            ScaleFixedVector(&g_aShipUpVector_00493ec0[WC2_EYE_OBJECT],
                             0x6400, &vector);
            AddFixedVectors(&g_aShipPosition_00494550[WC2_EYE_OBJECT],
                            &vector,
                            &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
            ComputeVectorDelta(
                &g_aShipPosition_00494550[WC2_EYE_OBJECT],
                &g_aShipPosition_00494550[g_cViewObject_0049313c], &vector);
            g_aShipForwardVector_00494208[WC2_EYE_OBJECT] = vector;
            fix_objects_ijk(WC2_EYE_OBJECT);
            generate_stars();
        } else {
            g_aShipForwardVector_00494208[WC2_EYE_OBJECT] = vector;
            fix_objects_ijk(WC2_EYE_OBJECT);
        }
        break;
    case 14:
        AddFixedVectors(&g_aShipPosition_00494550[WC2_EYE_OBJECT],
                        &g_aShipVelocity_00494898[WC2_EYE_OBJECT],
                        &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
        look_at(0);
        break;
    case 13:
        ScaleFixedVector(&g_aShipRightVector_00493b78[WC2_EYE_OBJECT],
                         -0xa00, &vector);
        AddFixedVectors(&g_aShipPosition_00494550[WC2_EYE_OBJECT], &vector,
                        &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
        AddFixedVectors(&g_aShipPosition_00494550[WC2_EYE_OBJECT],
                        &g_aShipVelocity_00494898[WC2_EYE_OBJECT],
                        &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
        look_at(0);
        break;
    case 7:
        if (g_nExternalViewShip_00493468 == -1) {
            zero_vector(&g_aShipVelocity_00494898[WC2_EYE_OBJECT]);
            if (g_nExternalViewTimeout_005d3212++ > 20) {
                if (g_nCockpitDisplayMode_0049d71c == 0) {
                    new_view(0, 0);
                } else {
                    free_view_buffer();
                    SetViewportRect(
                        &g_stViewBuffer_005d2b00, 0, 0,
                        (unsigned short)(g_nScreenWidth_0049d4d8 - 1),
                        (unsigned short)(g_nScreenHeight_0049d4dc - 1));
                    initialize_view_buffer();
                    new_view(0, 0);
                    free_view_buffer();
                    SetViewportRect(&g_stViewBuffer_005d2b00,
                                    0, 0, 319, 199);
                    initialize_view_buffer();
                }
            }
        } else {
            desiredDistance = 250;
            if (g_asObjectType_00495298[g_nExternalViewShip_00493468] ==
                0x13)
                desiredDistance = 800;
            ComputeVectorDelta(
                &g_aShipPosition_00494550[WC2_EYE_OBJECT],
                &g_aShipPosition_00494550[g_nExternalViewShip_00493468],
                &vector);
            if (g_asShipTactic_00495f30[g_nExternalViewShip_00493468] != 0) {
                if (Vector_magnitude(&vector) >
                    ((int)desiredDistance + 50) << 8) {
                    divide_vector(
                        &vector,
                        ((int)g_nExternalViewDistance_005d320e & ~1) << 7,
                        &g_aShipVelocity_00494898[WC2_EYE_OBJECT]);
                    g_nExternalViewDistance_005d320e = MaxShort(
                        (short)(g_nExternalViewDistance_005d320e - 1), 8);
                } else {
                    adjustment = vector;
                    NormalizeFixedVector(&adjustment);
                    ScaleFixedVector(&adjustment,
                                     -((int)desiredDistance << 8),
                                     &adjustment);
                    AddFixedVectors(
                        &vector, &adjustment,
                        &g_aShipVelocity_00494898[WC2_EYE_OBJECT]);
                }
            }
            g_aShipForwardVector_00494208[WC2_EYE_OBJECT] = vector;
            fix_objects_ijk(WC2_EYE_OBJECT);
        }
        AddFixedVectors(&g_aShipPosition_00494550[WC2_EYE_OBJECT],
                        &g_aShipVelocity_00494898[WC2_EYE_OBJECT],
                        &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
        break;
    case 15:
        SetFleetOverviewView(0);
        break;
    case 8:
        if (g_acShipTarget_00495f20[0] != -1) {
            ScaleFixedVector(&g_aShipRightVector_00493b78[0],
                             0x12c00, &vector);
            AddFixedVectors(&g_aShipPosition_00494550[0], &vector,
                            &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
            ComputeVectorDelta(
                &g_aShipPosition_00494550[WC2_EYE_OBJECT],
                &g_aShipPosition_00494550[g_acShipTarget_00495f20[0]],
                &vector);
            NormalizeFixedVector(&vector);
            g_aShipForwardVector_00494208[WC2_EYE_OBJECT] = vector;
            ScaleFixedVector(&vector, -0x25800, &vector);
            AddFixedVectors(&g_aShipPosition_00494550[WC2_EYE_OBJECT],
                            &vector,
                            &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
            fix_objects_ijk(WC2_EYE_OBJECT);
        } else {
            new_view(0, 0);
        }
        break;
    case 0:
        if (g_nCockpitCameraObject_0049d770 != 0) {
            copy_frame(g_nCockpitCameraObject_0049d770, WC2_EYE_OBJECT);
            g_aShipVelocity_00494898[WC2_EYE_OBJECT] =
                g_aShipVelocity_00494898[g_nCockpitCameraObject_0049d770];
            g_aShipPosition_00494550[WC2_EYE_OBJECT] =
                g_aShipPosition_00494550[g_nCockpitCameraObject_0049d770];
        } else {
            copy_frame(0, WC2_EYE_OBJECT);
            g_aShipVelocity_00494898[WC2_EYE_OBJECT] =
                g_aShipVelocity_00494898[0];
            g_aShipPosition_00494550[WC2_EYE_OBJECT] =
                g_aShipPosition_00494550[0];
        }
        break;
    case 1:
        g_aShipRightVector_00493b78[WC2_EYE_OBJECT] =
            g_aShipForwardVector_00494208[0];
        negate_vector(&g_aShipRightVector_00493b78[WC2_EYE_OBJECT]);
        g_aShipUpVector_00493ec0[WC2_EYE_OBJECT] =
            g_aShipUpVector_00493ec0[0];
        g_aShipForwardVector_00494208[WC2_EYE_OBJECT] =
            g_aShipRightVector_00493b78[0];
        g_aShipVelocity_00494898[WC2_EYE_OBJECT] =
            g_aShipVelocity_00494898[0];
        g_aShipPosition_00494550[WC2_EYE_OBJECT] =
            g_aShipPosition_00494550[0];
        break;
    case 2:
        g_aShipRightVector_00493b78[WC2_EYE_OBJECT] =
            g_aShipForwardVector_00494208[0];
        g_aShipUpVector_00493ec0[WC2_EYE_OBJECT] =
            g_aShipUpVector_00493ec0[0];
        g_aShipForwardVector_00494208[WC2_EYE_OBJECT] =
            g_aShipRightVector_00493b78[0];
        negate_vector(&g_aShipForwardVector_00494208[WC2_EYE_OBJECT]);
        g_aShipVelocity_00494898[WC2_EYE_OBJECT] =
            g_aShipVelocity_00494898[0];
        g_aShipPosition_00494550[WC2_EYE_OBJECT] =
            g_aShipPosition_00494550[0];
        break;
    case 3:
        copy_frame(0, WC2_EYE_OBJECT);
        negate_vector(&g_aShipRightVector_00493b78[WC2_EYE_OBJECT]);
        negate_vector(&g_aShipForwardVector_00494208[WC2_EYE_OBJECT]);
        g_aShipVelocity_00494898[WC2_EYE_OBJECT] =
            g_aShipVelocity_00494898[0];
        g_aShipPosition_00494550[WC2_EYE_OBJECT] =
            g_aShipPosition_00494550[0];
        break;
    case 4:
        switch (g_nTargetCameraOverlayMode_005c8db8) {
        case 0:
            copy_frame(0, WC2_EYE_OBJECT);
            negate_vector(&g_aShipRightVector_00493b78[WC2_EYE_OBJECT]);
            negate_vector(&g_aShipForwardVector_00494208[WC2_EYE_OBJECT]);
            break;
        case 1:
            g_aShipRightVector_00493b78[WC2_EYE_OBJECT] =
                g_aShipForwardVector_00494208[0];
            g_aShipUpVector_00493ec0[WC2_EYE_OBJECT] =
                g_aShipUpVector_00493ec0[0];
            g_aShipForwardVector_00494208[WC2_EYE_OBJECT] =
                g_aShipRightVector_00493b78[0];
            negate_vector(&g_aShipForwardVector_00494208[WC2_EYE_OBJECT]);
            break;
        case 2:
            g_aShipRightVector_00493b78[WC2_EYE_OBJECT] =
                g_aShipForwardVector_00494208[0];
            negate_vector(&g_aShipRightVector_00493b78[WC2_EYE_OBJECT]);
            g_aShipUpVector_00493ec0[WC2_EYE_OBJECT] =
                g_aShipUpVector_00493ec0[0];
            g_aShipForwardVector_00494208[WC2_EYE_OBJECT] =
                g_aShipRightVector_00493b78[0];
            break;
        }
        rotate_about_j(g_nFleetOverviewYaw_0049d3f4,
                       &g_aShipRightVector_00493b78[WC2_EYE_OBJECT],
                       &g_aShipForwardVector_00494208[WC2_EYE_OBJECT]);
        rotate_about_i(g_nFleetOverviewPitch_0049d3f8,
                       &g_aShipUpVector_00493ec0[WC2_EYE_OBJECT],
                       &g_aShipForwardVector_00494208[WC2_EYE_OBJECT]);
        g_aShipVelocity_00494898[WC2_EYE_OBJECT] =
            g_aShipVelocity_00494898[0];
        SetTargetCameraEyePosition(
            (short)g_nTargetCameraOverlayMode_005c8db8);
        UpdateFleetOverviewCameraRotation();
        break;
    case 10:
        copy_frame(0, WC2_EYE_OBJECT);
        negate_vector(&g_aShipRightVector_00493b78[WC2_EYE_OBJECT]);
        negate_vector(&g_aShipForwardVector_00494208[WC2_EYE_OBJECT]);
        g_aShipVelocity_00494898[WC2_EYE_OBJECT] =
            g_aShipVelocity_00494898[0];
        g_aShipPosition_00494550[WC2_EYE_OBJECT] =
            g_aShipPosition_00494550[0];
        break;
    case 11:
        copy_frame((short)g_cViewObject_0049313c, WC2_EYE_OBJECT);
        g_aShipVelocity_00494898[WC2_EYE_OBJECT] =
            g_aShipVelocity_00494898[g_cViewObject_0049313c];
        g_aShipPosition_00494550[WC2_EYE_OBJECT] =
            g_aShipPosition_00494550[g_cViewObject_0049313c];
        break;
    case 12:
        ComputeVectorDelta(
            &g_aShipPosition_00494550[WC2_EYE_OBJECT],
            &g_aShipPosition_00494550[g_cViewObject_0049313c], &vector);
        if (Vector_magnitude(&vector) < 0x25800) {
            adjustment = vector;
            NormalizeFixedVector(&adjustment);
            ScaleFixedVector(&adjustment, -0x25800, &adjustment);
            AddFixedVectors(&vector, &adjustment, &adjustment);
            AddFixedVectors(&g_aShipPosition_00494550[WC2_EYE_OBJECT],
                            &adjustment,
                            &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
        }
        g_aShipForwardVector_00494208[WC2_EYE_OBJECT] = vector;
        fix_objects_ijk(WC2_EYE_OBJECT);
        AddFixedVectors(&g_aShipPosition_00494550[WC2_EYE_OBJECT],
                        &g_aShipVelocity_00494898[WC2_EYE_OBJECT],
                        &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
        break;
    case 18:
        AddFixedVectors(&g_aShipPosition_00494550[WC2_EYE_OBJECT],
                        &g_aShipVelocity_00494898[WC2_EYE_OBJECT],
                        &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
        rotate_object(WC2_EYE_OBJECT);
        rotate_eye_to_goal();
        break;
    case 16:
        AddFixedVectors(&g_aShipPosition_00494550[WC2_EYE_OBJECT],
                        &g_aShipVelocity_00494898[WC2_EYE_OBJECT],
                        &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
        break;
    case 17:
        AddFixedVectors(
            &g_aShipPosition_00494550[g_cViewObject_0049313c],
            &g_vExternalViewOffset_005d3200,
            &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
        g_aShipVelocity_00494898[WC2_EYE_OBJECT] =
            g_aShipVelocity_00494898[g_cViewObject_0049313c];
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
    g_nCurrentView_00492fa8 = -1;
    new_view(view, obj);
}

/* Function start: 0x41AD8B */
void SaveTorpedoTargetLockForViewChange(int previousView)
{
    if (g_bSavedPlayerTarget_0049d464 == 0 &&
        previousView == 0 &&
        g_nTargetLockCountdown_004934ec >= 0 &&
        unactive(g_acShipTarget_00495f20[0]) == 0 &&
        g_bTargetLockMode_00493500 != 0 &&
        g_nSelectedReleaseWeaponIndex_004934e0 != -1 &&
        ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[
            g_nSelectedReleaseWeaponIndex_004934e0].type == 0x13) {
        g_nSavedPlayerTarget_0049d460 = g_acShipTarget_00495f20[0];
        g_bSavedPlayerTarget_0049d464 = 1;
        g_nTargetLockMusicCooldown_005d1e78 = -100;
        g_nSavedTargetLockCountdown_0049d484 =
            g_nTargetLockCountdown_004934ec;
    }
}

/* Function start: 0x41AE40 */
unsigned int new_view(int view, short obj)
{
    int previousView;
    FixedVector vector;

    if (IsCockpitExplosionActive() != 0)
        goto finished;
    if (g_nCurrentView_00492fa8 == view) {
        if (view == 5)
            g_bAlternateChaseView_00492fac =
                (short)(1 - g_bAlternateChaseView_00492fac);
        goto finished;
    }
    if (view < 4 && g_cScreenViewportMode_005c82a6 == view) {
        if (g_nCurrentView_00492fa8 == -1)
            g_nCurrentView_00492fa8 = view;
        goto finished;
    }
    if (view == 7 && g_nExternalViewShip_00493468 == -1)
        goto finished;

    if (g_nTargetCameraFrame_0049d3e8 == 1)
        ClearTargetCameraView();
    g_cViewObject_0049313c = (signed char)obj;
    previousView = g_nCurrentView_00492fa8;
    g_bTargetLockActive_0049ae80 = 0;
    if (view == 0 && previousView == 4) {
        if (g_bSavedPlayerTarget_0049d464 != 0) {
            g_nTargetLockCountdown_004934ec =
                (short)(g_nSavedTargetLockCountdown_0049d484 + 1);
            if (g_nTargetLockCountdown_004934ec < 0)
                g_nTargetLockCountdown_004934ec = 0;
            g_bTargetLockMode_00493500 = 1;
            g_acShipTarget_00495f20[0] =
                (signed char)g_nSavedPlayerTarget_0049d460;
        }
        g_bSavedPlayerTarget_0049d464 = 0;
    }
    if (view == 0)
        g_bSavedPlayerTarget_0049d464 = 0;
    g_nCurrentView_00492fa8 = view;
    if (obj != -1) {
        g_asObjectCollisionRadius_004950e8[WC2_EYE_OBJECT] =
            MaxShort(10, g_asObjectCollisionRadius_004950e8[obj]);
    } else {
        g_asObjectCollisionRadius_004950e8[WC2_EYE_OBJECT] = 10;
    }

    switch (view) {
    case 0:
        initialize_cockpit(0);
        break;
    case 1:
        if (CountShipCockpitGunDisplays(0) > 1) {
            g_nCurrentView_00492fa8 = 4;
            SaveTorpedoTargetLockForViewChange(previousView);
        }
        initialize_cockpit(1);
        break;
    case 2:
        if (CountShipCockpitGunDisplays(0) > 1) {
            g_nCurrentView_00492fa8 = 4;
            SaveTorpedoTargetLockForViewChange(previousView);
        }
        initialize_cockpit(2);
        break;
    case 3:
        if (HasShipCockpitGunDisplay(0) != 0) {
            g_nCurrentView_00492fa8 = 4;
            SaveTorpedoTargetLockForViewChange(previousView);
        }
        initialize_cockpit(3);
        break;
    case 6:
        initialize_cockpit(4);
        vector = g_aShipVelocity_00494898[g_cViewObject_0049313c];
        if (NormalizeFixedVector(&vector) == 0)
            vector =
                g_aShipForwardVector_00494208[g_cViewObject_0049313c];
        g_aShipUpVector_00493ec0[WC2_EYE_OBJECT] =
            g_aShipUpVector_00493ec0[g_cViewObject_0049313c];
        if (vector.x == g_aShipUpVector_00493ec0[WC2_EYE_OBJECT].x &&
            vector.y == g_aShipUpVector_00493ec0[WC2_EYE_OBJECT].y &&
            vector.z == g_aShipUpVector_00493ec0[WC2_EYE_OBJECT].z) {
            g_aShipUpVector_00493ec0[WC2_EYE_OBJECT] =
                g_aShipRightVector_00493b78[g_cViewObject_0049313c];
        }
        ScaleFixedVector(&vector, -0x12430, &vector);
        AddFixedVectors(
            &g_aShipPosition_00494550[g_cViewObject_0049313c],
            &vector, &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
        g_aShipForwardVector_00494208[WC2_EYE_OBJECT] = vector;
        fix_objects_ijk(WC2_EYE_OBJECT);
        zero_vector(&g_aShipVelocity_00494898[WC2_EYE_OBJECT]);
        ScaleFixedVector(&g_aShipRightVector_00493b78[WC2_EYE_OBJECT],
                         0x19000, &vector);
        AddFixedVectors(&g_aShipPosition_00494550[WC2_EYE_OBJECT],
                        &vector,
                        &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
        ScaleFixedVector(&g_aShipUpVector_00493ec0[WC2_EYE_OBJECT],
                         0x6400, &vector);
        AddFixedVectors(&g_aShipPosition_00494550[WC2_EYE_OBJECT],
                        &vector,
                        &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
        ComputeVectorDelta(
            &g_aShipPosition_00494550[WC2_EYE_OBJECT],
            &g_aShipPosition_00494550[g_cViewObject_0049313c],
            &vector);
        g_aShipForwardVector_00494208[WC2_EYE_OBJECT] = vector;
        fix_objects_ijk(WC2_EYE_OBJECT);
        break;
    case 13:
        initialize_cockpit(4);
        copy_frame(0, WC2_EYE_OBJECT);
        position_relative_ijk(&g_aShipPosition_00494550[WC2_EYE_OBJECT],
                              0, 0, 0, 2000);
        zero_vector(&g_aShipVelocity_00494898[WC2_EYE_OBJECT]);
        look_at(0);
        break;
    case 14:
        break;
    case 5:
        initialize_cockpit(4);
        ScaleFixedVector(
            &g_aShipForwardVector_00494208[g_cViewObject_0049313c],
            -0x4b000, &vector);
        AddFixedVectors(
            &g_aShipPosition_00494550[g_cViewObject_0049313c],
            &vector, &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
        g_aShipUpVector_00493ec0[WC2_EYE_OBJECT] =
            g_aShipUpVector_00493ec0[g_cViewObject_0049313c];
        g_aShipForwardVector_00494208[WC2_EYE_OBJECT] = vector;
        fix_objects_ijk(WC2_EYE_OBJECT);
        zero_vector(&g_aShipVelocity_00494898[WC2_EYE_OBJECT]);
        break;
    case 7:
        initialize_cockpit(4);
        ScaleFixedVector(
            &g_aShipRightVector_00493b78[g_nExternalViewShip_00493468],
            0x25800, &vector);
        AddFixedVectors(
            &g_aShipPosition_00494550[g_nExternalViewShip_00493468],
            &vector, &g_aShipPosition_00494550[WC2_EYE_OBJECT]);
        ComputeVectorDelta(
            &g_aShipPosition_00494550[WC2_EYE_OBJECT],
            &g_aShipPosition_00494550[g_nExternalViewShip_00493468],
            &vector);
        g_aShipUpVector_00493ec0[WC2_EYE_OBJECT] =
            g_aShipUpVector_00493ec0[0];
        g_aShipForwardVector_00494208[WC2_EYE_OBJECT] = vector;
        fix_objects_ijk(WC2_EYE_OBJECT);
        zero_vector(&g_aShipVelocity_00494898[WC2_EYE_OBJECT]);
        g_nExternalViewDistance_005d320e = 0x20;
        g_nExternalViewTimeout_005d3212 = 0;
        break;
    case 8:
        initialize_cockpit(4);
        copy_frame(0, WC2_EYE_OBJECT);
        zero_vector(&g_aShipVelocity_00494898[WC2_EYE_OBJECT]);
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
        g_nEyePitchGoal_00493140 =
            g_nEyeYawGoal_00493144 =
                g_nEyeRollGoal_00493148 = 0;
        g_nEyePitchRate_0049314c =
            g_nEyeYawRate_00493150 =
                g_nEyeRollRate_00493154 = 1;
        break;
    case 17:
        ComputeVectorDelta(
            &g_aShipPosition_00494550[g_cViewObject_0049313c],
            &g_aShipPosition_00494550[WC2_EYE_OBJECT],
            &g_vExternalViewOffset_005d3200);
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
    ScaleFixedVector(&g_aShipForwardVector_00494208[WC2_EYE_OBJECT],
                     (int)forwardDistance << 8, &offset);
    AddFixedVectors(&origin, &offset, &origin);
    ScaleFixedVector(&g_aShipRightVector_00493b78[WC2_EYE_OBJECT],
                     rightOffset, &offset);
    AddFixedVectors(&offset, &origin, &origin);
    ScaleFixedVector(&g_aShipUpVector_00493ec0[WC2_EYE_OBJECT],
                     upOffset, &offset);
    AddFixedVectors(&offset, &origin, &origin);
    AddFixedVectors(&origin,
                    &g_aShipPosition_00494550[WC2_EYE_OBJECT],
                    &g_aShipPosition_00494550[obj]);
    FillFixedVectorWithRandomComponents(2, &g_aShipVelocity_00494898[obj]);
    g_asObjectScreenAngle_004936b8[obj] =
        (short)(MaxShort((short)(1 - RandomInRange(0, 3)), 0) * 0x10 +
                RandomInRange(0, 3));
}

/* Function start: 0x41B762 */
unsigned int generate_stars(void)
{
    FixedVector origin;
    short distance;
    short obj;

    origin.x = 0;
    origin.y = 0;
    origin.z = 0;
    for (obj = 40; obj < 55; obj++) {
        if (obj < 48) {
            distance = RandomInRange(0, 1400);
#ifdef WC1_SDL
            start_dust(obj, origin, distance,
                       signed_random(distance) * 0x100,
                       signed_random(distance) * 0x100);
#else
            start_dust(obj, origin, distance,
                       signed_random(distance) << 8,
                       signed_random(distance) << 8);
#endif
        } else {
            g_aeObjectClass_00495328[obj] = OBJECT_CLASS_STAR;
            g_nStarFieldIRotation_005d3210 = signed_random(45);
            g_nStarFieldJRotation_005d320c = signed_random(45);
            copy_frame(WC2_EYE_OBJECT, 69);
            rotate_about_j(g_nStarFieldJRotation_005d320c,
                           &g_aShipRightVector_00493b78[69],
                           &g_aShipForwardVector_00494208[69]);
            rotate_about_i(g_nStarFieldIRotation_005d3210,
                           &g_aShipUpVector_00493ec0[69],
                           &g_aShipForwardVector_00494208[69]);
            ScaleFixedVector(&g_aShipForwardVector_00494208[69],
                             15000 << 8,
                             &g_aShipPosition_00494550[obj]);
            g_asObjectViewFrame_00493508[obj] =
                (short)(RandomInRange(0, 5) + 32);
        }
    }
}

/* Function start: 0x41B8D6 */
unsigned int update_star_field(void)
{
    FixedVector cameraMotion;
    FixedVector viewMotion;
    FixedVector origin;
    int distance;
    short randomChoice;
    short rightRandom;
    short upRandom;
    unsigned char shift;
    int rightOffset;
    int upOffset;
    int objectIndex;
    int objectOffset;
    short obj;
    unsigned int hazardActive;

    hazardActive = g_pActiveHazardField_00493278 != 0;
    g_vPreviousStarFieldMotion_00493168 = g_vStarFieldMotion_00493158;
    ScaleFixedVector(&g_aShipForwardVector_00494208[WC2_EYE_OBJECT],
                     200 << 8, &cameraMotion);
    ScaleFixedVector(&g_aShipVelocity_00494898[WC2_EYE_OBJECT],
                     20 << 8, &g_vStarFieldMotion_00493158);
    AddFixedVectors(&cameraMotion, &g_vStarFieldMotion_00493158,
                    &g_vStarFieldMotion_00493158);
    ComputeVectorDelta(&g_vPreviousStarFieldMotion_00493168,
                       &g_vStarFieldMotion_00493158, &origin);
    for (obj = 40; obj < 55; obj++) {
        objectIndex = (int)obj;
        objectOffset = objectIndex * sizeof(short);
        if (*(short *)((unsigned char *)g_asObjectScreenX_00493598 +
                       objectOffset) == (short)0x8001) {
            randomChoice = RandomInRange(0, 7);
            if (hazardActive == 0) {
                if (g_aeObjectClass_00495328[objectIndex] ==
                        OBJECT_CLASS_ASTEROID ||
                    (int)g_aeObjectClass_00495328[objectIndex] == 0x15 ||
                    g_aeObjectClass_00495328[objectIndex] ==
                        OBJECT_CLASS_NULL) {
                    set_objects_data(obj, 0x2e, -1, 0);
                    randomChoice = 0;
                }
            } else if (obj < 48) {
                extra_hazard(obj);
            }
            if ((g_aeObjectClass_00495328[objectIndex] == OBJECT_CLASS_STAR ||
                 g_aeObjectClass_00495328[objectIndex] ==
                     OBJECT_CLASS_PLANET) &&
                randomChoice == 0 &&
                (g_anObjectYawRotation_00494fc8[0] |
                 g_anObjectPitchRotation_00494f38[0]) != 0) {
                copy_frame(WC2_EYE_OBJECT, 69);
                if (g_anObjectPitchRotation_00494f38[0] != 0) {
                    g_nStarFieldIRotation_005d3210 =
                        g_anObjectPitchRotation_00494f38[0] < 0 ?
                            -45 : 45;
                    g_nStarFieldJRotation_005d320c = signed_random(45);
                }
                if (g_anObjectYawRotation_00494fc8[0] != 0 &&
                    (g_anObjectPitchRotation_00494f38[0] == 0 ||
                     RandomInRange(0, 1) != 0)) {
                    g_nStarFieldJRotation_005d320c =
                        g_anObjectYawRotation_00494fc8[0] < 0 ? -45 : 45;
                    g_nStarFieldIRotation_005d3210 = signed_random(45);
                }
                rotate_about_i(g_nStarFieldIRotation_005d3210,
                               &g_aShipUpVector_00493ec0[69],
                               &g_aShipForwardVector_00494208[69]);
                rotate_about_j(g_nStarFieldJRotation_005d320c,
                               &g_aShipRightVector_00493b78[69],
                               &g_aShipForwardVector_00494208[69]);
                ScaleFixedVector(&g_aShipForwardVector_00494208[69],
                                 15000 << 8,
                                 &g_aShipPosition_00494550[objectIndex]);
                *(short *)((unsigned char *)g_asObjectViewFrame_00493508 +
                           objectOffset) =
                    (short)(RandomInRange(0, 5) + 32);
                break;
            }
            if (g_aeObjectClass_00495328[objectIndex] == OBJECT_CLASS_DUST &&
                randomChoice < 2) {
                transform_to_objects_frame(
                    &g_aShipVelocity_00494898[WC2_EYE_OBJECT],
                    &viewMotion, WC2_EYE_OBJECT);
                ScaleFixedVector(&viewMotion, 10 << 8, &viewMotion);
                if (viewMotion.z >= 0) {
                    distance =
                        (unsigned short)RandomInRange(
                            0, (short)(viewMotion.z >> 8)) +
                        g_asObjectCollisionRadius_004950e8[WC2_EYE_OBJECT];
                    distance = distance * 2 +
                        (unsigned short)RandomInRange(0, 350);
                } else {
                    distance = (unsigned short)RandomInRange(0, 40) +
                        g_asObjectCollisionRadius_004950e8[WC2_EYE_OBJECT];
                }
                rightRandom = signed_random((short)(distance >> 1));
                upRandom = signed_random((short)(distance >> 1));
                shift = (unsigned char)(viewMotion.z <= 0 ? 9 : 8);
#ifdef WC1_SDL
                rightOffset =
                    viewMotion.x + (int)rightRandom * (1 << shift);
                upOffset =
                    viewMotion.y + (int)upRandom * (1 << shift);
#else
                rightOffset =
                    viewMotion.x + ((int)rightRandom << shift);
                upOffset = viewMotion.y + ((int)upRandom << shift);
#endif
                start_dust(obj, origin, distance, rightOffset, upOffset);
                break;
            }
        }
    }
    if (g_pActiveHazardField_00493278 != 0 &&
        g_nCannedSceneMode_0049021c == 0)
        update_hazards();
    return 0;
}

/* Function start: 0x41BD44 */
short count_down(short obj)
{
    if (g_asObjectCounter_00494be0[obj] != -1)
        g_asObjectCounter_00494be0[obj]--;
    return g_asObjectCounter_00494be0[obj];
}

/* Function start: 0x41BD81 */
void house_keep_objects(void)
{
    short other;
    short obj;
    short gun;
    short projectileDetonated;

    for (obj = 0; obj <= WC2_SPACE_LAST_MOVING_OBJECT; obj++) {
        switch (g_aeObjectClass_00495328[obj]) {
        case OBJECT_CLASS_FIXED_OBJECT:
            switch (g_asObjectType_00495298[obj]) {
            case 0x0c:
            case 0x2b:
                remove_object(obj);
                break;
            }
            break;
        case OBJECT_CLASS_PROJECTILE:
            projectileDetonated = 0;
            if (g_asObjectType_00495298[obj] == 0x0c) {
                for (other = 0; other < 10; other++) {
                    if (g_aeObjectClass_00495328[other] ==
                            OBJECT_CLASS_MISSILE &&
                        g_asObjectType_00495298[other] ==
                            WC2_OBJECT_TYPE_TORPEDO &&
                        g_asMissileProximityFuse_005d38e0[other] != 0 &&
                        distance_from_object(obj, other) < 200) {
                        explode((short)g_acObjectOwner_00495208[obj], obj);
                        projectileDetonated = 1;
                    }
                }
            }
            if (projectileDetonated == 0 && count_down(obj) == 0) {
                if (g_asObjectType_00495298[obj] == 0x0c)
                    explode((short)g_acObjectOwner_00495208[obj], obj);
                else
                    remove_object(obj);
            }
            break;
        case OBJECT_CLASS_DUST:
            if (g_asObjectType_00495298[obj] == 0x2f &&
                count_down(obj) == -1 &&
                g_asObjectScreenX_00493598[obj] == (short)0x8001)
                remove_object(obj);
            break;
        case OBJECT_CLASS_EXPLOSION:
            if (g_asObjectType_00495298[obj] == 0x25 &&
                g_asObjectAnimationIndex_00494c70[obj] == 0x0c &&
                g_asObjectScreenX_00493598[obj] != (short)0x8001) {
                if (g_nSpacePaletteFadeMode_004901e8 == 0x13) {
                    g_nSpaceExplosionFlashStep_005c5872 = 0;
                    g_asSpacePaletteFade_005d2d60[0] =
                        (short)(g_aSpaceExplosionFlashPalette_0049d808[0][0]
                                << 2);
                    g_asSpacePaletteFade_005d2d60[1] =
                        (short)(g_aSpaceExplosionFlashPalette_0049d808[0][0]
                                << 2);
                    g_asSpacePaletteFade_005d2d60[2] = 0;
                    SetPaletteEntry(
                        (short)g_cPrimaryViewBufferColour_0049cb88,
                        g_asSpacePaletteFade_005d2d60);
                    g_nSpaceExplosionFlashActive_00492fb4++;
                } else {
                    ClearViewport(
                        &g_stViewBuffer_005d2b00,
                        (short)g_bPrimaryViewBufferColour_0049cb50);
                    g_bViewportDirty_0049d76c++;
                }
            }
            break;
        case OBJECT_CLASS_DEBRIS:
            if (g_asObjectType_00495298[obj] == 0x3d ||
                g_asObjectType_00495298[obj] ==
                    WC2_OBJECT_TYPE_EJECTION_POD)
                break;
            if (count_down(obj) == -1 &&
                g_asObjectScreenX_00493598[obj] == (short)0x8001)
                remove_object(obj);
            break;
        case OBJECT_CLASS_MINE:
            if (g_acObjectCollisionGraceTicks_00494d48[obj] > 0)
                g_acObjectCollisionGraceTicks_00494d48[obj]--;
            if (count_down(obj) == 0)
                explode(obj, obj);
            break;
        case OBJECT_CLASS_MISSILE:
            g_acShipExhaustHeat_00495660[obj] = 0;
            if (g_asObjectType_00495298[obj] ==
                    WC2_OBJECT_TYPE_CHAFF_POD) {
                if (count_down(obj) <= 0)
                    explode(obj, obj);
            } else {
                if (g_acObjectCollisionGraceTicks_00494d48[obj] > 0)
                    g_acObjectCollisionGraceTicks_00494d48[obj]--;
                if (g_asObjectType_00495298[obj] !=
                        WC2_OBJECT_TYPE_DART_DUMB_FIRE_MISSILE &&
                    g_asShipTactic_00495f30[obj] == TACTIC_RAM &&
                    g_anShipCloakState_00496020[
                        g_acShipTarget_00495f20[obj]] == 1) {
                    explode(obj, obj);
                } else if (g_asShipTactic_00495f30[obj] ==
                               TACTIC_SIT_STILL &&
                           g_nCannedSceneMode_0049021c == 0) {
                    if (count_down(obj) <= 0) {
                        g_asShipTactic_00495f30[obj] = TACTIC_RAM;
                        g_asObjectCounter_00494be0[obj] = (short)(
                            *(int *)&g_aObjectTypeData_00496d30[
                                g_acObjectType_00493980[obj]].lifetime);
                        if (g_asObjectType_00495298[obj] ==
                                WC2_OBJECT_TYPE_DART_DUMB_FIRE_MISSILE) {
                            vector_component_in_dir(
                                &g_aShipVelocity_00494898[obj],
                                &g_aShipForwardVector_00494208[obj],
                                &g_aShipVelocity_00494898[obj]);
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
            if (DAT_005d1bd0[obj] > 0)
                DAT_005d1bd0[obj]--;
            if (DAT_004960f0[obj] > 0)
                DAT_004960f0[obj]--;
            if (DAT_005d1bd0[obj] > 0 &&
                g_asObjectType_00495298[obj] < 0x3a)
                DAT_005d1bd0[obj]--;
            if (DAT_005d1bf0[obj] > 0)
                DAT_005d1bf0[obj]--;
            if (g_asShipExplosionStageTimer_005d3850[obj] > 0)
                g_asShipExplosionStageTimer_005d3850[obj]--;
            for (gun = 0; gun < 3; gun++) {
                if (g_asGunCooldown_005c8d70[obj * 3 + gun] > 0)
                    g_asGunCooldown_005c8d70[obj * 3 + gun]--;
            }
            if (g_asShipCloakCooldown_00496048[obj] > 0)
                g_asShipCloakCooldown_00496048[obj]--;
            g_asShipCloakElapsedFrames_00496060[obj]++;
            if (g_asShipFriendlyFireCooldown_00496090[obj] > 0)
                g_asShipFriendlyFireCooldown_00496090[obj]--;
            g_acShipExhaustHeat_00495660[obj] = 0;
            if (count_down(obj) > 0) {
                if (g_asShipManeuver_00495f48[obj] ==
                        MANEUVER_WARPING_OUT)
                    g_asObjectScale_00494d90[obj] >>= 1;
                if (g_aeSpecialManeuver_00495600[obj] ==
                        SPECIAL_MANEUVER_UNKNOWN_9 &&
                    g_aeObjectClass_00495328[obj] >=
                        OBJECT_CLASS_CAPITAL_SHIP) {
                    if ((g_aObjectTypeData_00496d30[WC2_OBJECT_TYPE(OBJECT_TYPE_EXPLOSION2)].shapeSet !=
                          0 ? 8 : 4) - 1 ==
                            g_asObjectCounter_00494be0[obj]) {
                        ShipExplosion(obj);
                        explosion_shock_wave(
                            obj, g_aObjectTypeData_00496d30[
                                g_acObjectType_00493980[obj]].
                                    explosionDamage);
                    } else {
                        while ((unsigned short)RandomInRange(0, 100) < 50)
                            onboard_explosion(obj);
                    }
                }
            } else if (g_asObjectCounter_00494be0[obj] == 0) {
                if (g_aeSpecialManeuver_00495600[obj] ==
                        SPECIAL_MANEUVER_UNKNOWN_9) {
                    if (g_nYourWingman_0049346c != -1 &&
                        g_acShipLastAttacker_004955c0[obj] == 0 &&
                        g_asShipSide_004955d0[obj] == SIDE_KILRATHI &&
                        RandomBelowOrEqual(100) < 20) {
                        send_message(g_nYourWingman_0049346c, 7);
                    }
                    Create_explosion_debris(obj);
                    if (g_nArcadeState_0049d75c != 0)
                        return;
                    if (g_bMissionDeathSequencePending_0049b720 != 0 &&
                        g_nPendingEjectionShip_005d1bc4 == obj)
                        SpawnMissionEjectionPod(obj);
                    break;
                }
                switch (g_asShipManeuver_00495f48[obj]) {
                case MANEUVER_WARPING_OUT:
                    if (g_asShipSide_004955d0[obj] != SIDE_NEUTRAL) {
                        if (obj != 0 && obj != g_nYourWingman_0049346c) {
                            g_aMissionShips_00492290[
                                g_asShipMissionIndex_00495d00[obj]].state = 2;
                            remove_object(obj);
                            if (g_nCannedSceneMode_0049021c == 0)
                                RecordCannedSceneObjectEvent(obj, 1);
                        } else if (obj == 0) {
                            CompleteStarSystemJump();
                        }
                    }
                    break;
                case MANEUVER_WARPING_IN:
                    if (g_asShipTactic_00495f30[obj] != TACTIC_WARP_IN) {
                        if ((short)g_acObjectOwner_00495208[obj] != obj) {
                            remove_object(obj);
                        } else {
                            set_objects_data(
                                obj,
                                (short)g_abShipNavPointIndex_00495f60[obj],
                                -1, 1);
                            reset_maneuver(obj, -1);
                        }
                    }
                    break;
                }
            }
            if (g_asShipMissionIndex_00495d00[obj] ==
                    g_nHomeMissionShipIndex_005d1e22 &&
                g_bCarrierLandingEnabled_0049d778 != 0 &&
                g_bLandingCommRequestPending_00492fa0 != 0 &&
                normal_speed(0) != 0 &&
                distance_from_object(0, obj) < 700) {
                g_nPlayerCollisionObject_00493480 = obj;
                g_nArcadeState_0049d75c = 1;
            }
            break;
        }
    }
}

/* Function start: 0x41C7ED */
void update_objects_in_space(void)
{
    short obj;

    if (g_nCannedSceneMode_0049021c != 0) {
        if (g_bSceneEscapeRequested_0049d4b0 != 0) {
            g_nCannedSceneRecordedFrameCount_005d3faa =
                g_nCannedSceneFrameCount_005d404c;
        } else {
            g_nCannedSceneRecordedFrameCount_005d3faa++;
            if (g_nCannedSceneSegmentEndFrame_00490218 ==
                g_nSpaceFrame_00493134)
                HandleCannedSceneBufferBoundary();
            ApplyCannedSceneFrameEvents();
        }
    } else {
        g_nCannedSceneFrameCount_005d404c++;
        if (g_nCannedSceneBufferNearCapacityFlag_00490214 != 0)
            HandleCannedSceneBufferBoundary();
    }

    for (obj = 0; obj <= WC2_SPACE_LAST_MOVING_OBJECT; obj++) {
        if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_NULL) {
            if (g_nCannedSceneMode_0049021c == 0 &&
                g_aeSpecialManeuver_00495600[obj] ==
                    SPECIAL_MANEUVER_UNKNOWN_9)
                RecordCannedSceneObjectState(obj);
        } else if (g_aeObjectClass_00495328[obj] ==
                   OBJECT_CLASS_FUTURION) {
            futurion_intelligence(obj);
        } else if (g_aeObjectClass_00495328[obj] > OBJECT_CLASS_PLANET) {
            animate_object(obj);
            if (g_nCannedSceneMode_0049021c != 0) {
                AddFixedVectors(&g_aShipPosition_00494550[obj],
                                &g_aShipVelocity_00494898[obj],
                                &g_aShipPosition_00494550[obj]);
                if (obj < 10)
                    fix_objects_ijk(obj);
            } else {
                if (g_aeObjectClass_00495328[obj] >=
                    OBJECT_CLASS_PROJECTILE) {
                    object_collision(obj);
                    rotate_object(obj);
                    if (obj >= 10 ||
                        g_aeSpecialManeuver_00495600[obj] !=
                            SPECIAL_MANEUVER_UNKNOWN_9) {
                        if (obj != 0 ||
                            g_bJumpSequenceActive_004962f0 != 0) {
                            object_intelligence(obj);
                        } else if (g_nCurrentView_00492fa8 == 4) {
                            UpdateSavedTorpedoTargetLock();
                        }
                        if (g_aeObjectClass_00495328[obj] ==
                                OBJECT_CLASS_SHIP &&
                            g_asShipTurretFireEnabled_0049d470[obj] != 0)
                            UpdateShipTurretGuns(obj);
                        if (obj < 10 &&
                            g_aeObjectClass_00495328[obj] >=
                                OBJECT_CLASS_MISSILE) {
                            if (g_nCannedSceneMode_0049021c == 0)
                                RecordCannedSceneObjectState(obj);
                            if (obj != 0 || g_nCurrentView_00492fa8 == 4)
                                rotate_object_to_goal(obj);
                        }
                    }
                }
                if (g_nCurrentView_00492fa8 == 4 &&
                    g_nTargetCameraMode_005c8d50 == 1 &&
                    g_nTargetCameraObject_0049d338 == obj &&
                    g_nTargetCameraFrame_0049d3e8 == 1)
                    UpdateTargetCameraObject(obj);
                accelerate_and_move_object(obj);
                if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_SHIP) {
                    replenish_shields(obj);
                    housekeep_power_plant_and_fuel(obj);
                    replenish_weapon_energy_bank(obj);
                }
            }
        }
    }
}

/* Function start: 0x41CB3C */
unsigned int rotate_object(short obj)
{
    if (g_anObjectPitchRotation_00494f38[obj] != 0) {
        alter_pitch(g_anObjectPitchRotation_00494f38[obj], obj);
        ClampVectorTo30(&g_anObjectPitchRotation_00494f38[obj]);
    }
    if (g_anObjectYawRotation_00494fc8[obj] != 0) {
        alter_yaw(g_anObjectYawRotation_00494fc8[obj], obj);
        ClampVectorTo30(&g_anObjectYawRotation_00494fc8[obj]);
    }
    if (g_anObjectRollRotation_00495058[obj] != 0) {
        alter_roll(g_anObjectRollRotation_00495058[obj], obj);
        ClampVectorTo30(&g_anObjectRollRotation_00495058[obj]);
    }
    return 0;
}

/* Function start: 0x41CC0A */
void accelerate_and_move_object(short obj)
{
    FixedVector delta;
    FixedVector accelerationVector;
    int acceleration;
    int magnitude;

    if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_MISSILE) {
        switch (g_aeSpecialManeuver_00495600[obj]) {
        case SPECIAL_MANEUVER_KILL_ENGINES:
            g_acShipExhaustHeat_00495660[obj] = 0;
            if (RandomBelowOrEqual(100) < 10)
                set_special(obj, SPECIAL_MANEUVER_NONE);
            break;
        case SPECIAL_MANEUVER_STOP_DRIFT:
            approach_zero_speed(obj);
            NormalizeAndScaleVector(&g_aShipVelocity_00494898[obj],
                                    g_anShipSpeed_00494e20[obj]);
            if (g_anShipSpeed_00494e20[obj] == 0)
                set_special(obj, SPECIAL_MANEUVER_NONE);
            break;
        }
        if (g_aeSpecialManeuver_00495600[obj] <
                SPECIAL_MANEUVER_KILL_ENGINES &&
            g_asShipTactic_00495f30[obj] != TACTIC_SIT_STILL) {
            zero_vector(&accelerationVector);
            switch (g_aeSpecialManeuver_00495600[obj]) {
            case SPECIAL_MANEUVER_AFTERBURNER:
                g_asShipAfterburnerTimer_004955a8[obj]--;
                if (g_asShipAfterburnerTimer_004955a8[obj] == 0) {
                    g_aeSpecialManeuver_00495600[obj] =
                        SPECIAL_MANEUVER_NONE;
                    ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                                     g_anShipSpeed_00494e20[obj], &delta);
                } else {
                    ScaleFixedVector(
                        &g_aShipForwardVector_00494208[obj],
                        (g_aObjectTypeData_00496d30[
                             g_acObjectType_00493980[obj]].maximumVelocity *
                             2 +
                         40) << 8,
                        &delta);
                    drain_fuel(obj, 200);
                    g_acShipExhaustHeat_00495660[obj] = 3;
                }
                break;
            case SPECIAL_MANEUVER_SUPER_BRAKE:
                g_asShipAfterburnerTimer_004955a8[obj]--;
                if (g_asShipAfterburnerTimer_004955a8[obj] == 0) {
                    g_aeSpecialManeuver_00495600[obj] =
                        SPECIAL_MANEUVER_NONE;
                    ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                                     g_anShipSpeed_00494e20[obj], &delta);
                } else {
                    zero_vector(&delta);
                    drain_fuel(obj, 140);
                }
                break;
            default:
                ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                                 g_anShipSpeed_00494e20[obj], &delta);
                break;
            }
            ComputeVectorDelta(&g_aShipVelocity_00494898[obj], &delta,
                               &delta);
            magnitude = Vector_magnitude(&delta);
            if (magnitude > 0) {
                acceleration = GetShipAccelerationRate(obj);
                if (alert_flag(obj, 1) != 0 && acceleration < 0x500)
                    acceleration = 0x500;
                if (g_aeSpecialManeuver_00495600[obj] ==
                        SPECIAL_MANEUVER_AFTERBURNER ||
                    g_aeSpecialManeuver_00495600[obj] ==
                        SPECIAL_MANEUVER_SUPER_BRAKE) {
                    acceleration *= 2;
                }
                acceleration = MultiplyFixed(
                    acceleration,
                    DivideFixed(
                        dot_product(
                            &delta,
                            &g_aShipForwardVector_00494208[obj]),
                        magnitude) + 0x200);
                ScaleFixedVector(
                    &delta,
                    MinInt(DivideFixed(acceleration >> 1, magnitude),
                           0x100),
                    &accelerationVector);
                if (g_aeSpecialManeuver_00495600[obj] !=
                        SPECIAL_MANEUVER_AFTERBURNER) {
                    g_acShipExhaustHeat_00495660[obj] = 2;
                }
            }
            AddFixedVectors(&accelerationVector,
                            &g_aShipVelocity_00494898[obj],
                            &g_aShipVelocity_00494898[obj]);
            if (obj == 0)
                g_vPlayerAcceleration_00493488 = accelerationVector;
        }
    }
    AddFixedVectors(&g_aShipPosition_00494550[obj],
                    &g_aShipVelocity_00494898[obj],
                    &g_aShipPosition_00494550[obj]);
}

/* Function start: 0x41D07B */
void animate_shape(short obj)
{
    short type;
    unsigned char *animation;
    short command;

    type = g_acObjectType_00493980[obj];
    animation = g_aObjectTypeData_00496d30[type].animation;
    if (animation == 0)
        return;
    if (--g_acObjectAnimationDelay_00494d00[obj] > 0)
        return;
    g_acObjectAnimationDelay_00494d00[obj] =
        g_aObjectTypeData_00496d30[type].yawRate;
    command = *(unsigned short *)(animation +
              g_asObjectAnimationIndex_00494c70[obj] * 2);
    switch (command & 0xf000) {
    case 0x9000:
        g_asObjectAnimationIndex_00494c70[obj] = command & 0x0fff;
        command = *(unsigned short *)(animation +
                  g_asObjectAnimationIndex_00494c70[obj] * 2);
        if (g_asObjectScreenX_00493598[obj] != (short)0x8001 &&
            (g_asObjectType_00495298[obj] == 0x20 ||
             g_asObjectType_00495298[obj] == 0x1f ||
             g_asObjectType_00495298[obj] == 0x3d)) {
            PlaySfxWaveFileByNumber(13, obj, 0);
        }
        break;
    case 0xa000:
        remove_object(obj);
        return;
    }

    switch (command & 0x0c00) {
    case 0x0400:
        g_asObjectScale_00494d90[obj] +=
            (command & 0x3f) *
            ((unsigned short)g_asObjectScale_00494d90[obj] >> 6);
        break;
    case 0x0800:
        g_asObjectScale_00494d90[obj] -=
            (command & 0x3f) *
            ((unsigned short)g_asObjectScale_00494d90[obj] >> 6);
        break;
    default:
        command &= 0x3f;
        g_asObjectViewFrame_00493508[obj] = command;
        break;
    }
    g_asObjectFlip_004939c8[obj] = (command & 0xc0) >> 2;
    g_asObjectAnimationIndex_00494c70[obj]++;
}

/* Function start: 0x41D2DA */
void animate_object(short obj)
{
    FixedVector offset;
    short effect;

    switch (g_aeObjectClass_00495328[obj]) {
    case OBJECT_CLASS_EXPLOSION:
    case OBJECT_CLASS_DEBRIS:
    case OBJECT_CLASS_FIXED_OBJECT:
    case OBJECT_CLASS_ASTEROID:
    case OBJECT_CLASS_MINE:
        animate_shape(obj);
        break;
    case OBJECT_CLASS_SHIP:
        if (g_asObjectScreenX_00493598[obj] == (short)0x8001 ||
            (g_nRenderedSpaceFrame_00493138 & 3) != 0 ||
            (g_aObjectTypeData_00496d30[
                 g_acObjectType_00493980[obj]].damageCapacity >> 1) - 1 >
                g_asObjectDamage_00495178[obj]) {
            break;
        }
        effect = find_vacant_3d_object();
        if (effect == -1)
            break;
        ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                         -((int)g_asObjectCollisionRadius_004950e8[obj]
                             << 8),
                         &offset);
        AddFixedVectors(&g_aShipPosition_00494550[obj], &offset,
                        &g_aShipPosition_00494550[effect]);
        FillFixedVectorWithRandomComponents(20, &offset);
        AddFixedVectors(&g_aShipPosition_00494550[effect], &offset,
                        &g_aShipPosition_00494550[effect]);
        zero_vector(&g_aShipVelocity_00494898[effect]);
        set_objects_data(
            effect,
            (enum ObjectType)((unsigned short)RandomInRange(0, 2) +
                              WC2_OBJECT_TYPE_SHIP_DAMAGE_EFFECT_FIRST),
            obj, 0);
        g_asObjectScale_00494d90[effect] =
            g_asObjectScale_00494d90[obj];
        if (RandomInRange(0, 3) == 0)
            PlaySfxWaveFileByNumber(7, obj, 0);
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
            make_shard(asteroid, g_aShipVelocity_00494898[asteroid]);
        }
        explode(-1, asteroid);
    } else if (RandomBelowOrEqual(7) == 0) {
        make_shard(asteroid, g_vCollisionDelta_00493178);
    }
}

/* Function start: 0x41D591 */
void object_collision(short obj)
{
    SphericalVector impact;
    FixedVector relativeVelocity;
    FixedVector viewDelta;
    FixedVector viewRelative;
    FixedVector componentDelta;
    FixedVector partnerComponent;
    FixedVector tangent;
    FixedVector force;
    short objectMass;
    short partnerMass;
    int totalMass;
    int responseScale;
    int forceMagnitude;
    short speed;
    short damage;
    short savedScale;
    short adaptiveDamageScale;
    short lifetime;
    short partner;

    objectMass = g_aObjectTypeData_00496d30[
        g_acObjectType_00493980[obj]].radarRadius;
    partner = check_for_collision(obj);
    if (partner == -1) {
        g_acLastCollisionObject_00495250[obj] = -1;
    } else if (g_bPlayerCollisionEnabled_0049d780 != 0 ||
               (partner != 0 && obj != 0)) {
        partnerMass = g_aObjectTypeData_00496d30[
            g_acObjectType_00493980[partner]].radarRadius;
        NormalizeFixedVector(&g_vCollisionDelta_00493178);
        ComputeVectorDelta(&g_aShipVelocity_00494898[partner],
                           &g_aShipVelocity_00494898[obj],
                           &relativeVelocity);
        NormalizeFixedVector(&relativeVelocity);

        switch (g_aeObjectClass_00495328[obj]) {
        case OBJECT_CLASS_PROJECTILE:
            if (g_acObjectOwner_00495208[obj] != partner &&
                g_aeObjectClass_00495328[partner] !=
                    OBJECT_CLASS_PROJECTILE) {
                if (g_aeObjectClass_00495328[partner] >=
                        OBJECT_CLASS_MISSILE) {
                    if (partner == 0) {
                        ComputeVectorDelta(
                            &g_aShipPosition_00494550[WC2_EYE_OBJECT],
                            &g_aShipPosition_00494550[obj], &viewDelta);
                        transform_to_objects_frame(
                            &viewDelta, &viewRelative, WC2_EYE_OBJECT);
                        rectangular_to_spherical(&viewRelative, &impact);
                        if (abs(impact.pitch) < 45) {
                            if (abs(impact.yaw) < 45) {
                                g_aasCockpitHitPaletteFades_005d2cb0[1][0] =
                                    0x38;
                            } else if (abs(impact.yaw) < 136) {
                                if (impact.yaw < 0) {
                                    g_aasCockpitHitPaletteFades_005d2cb0[3][0] =
                                        0x38;
                                } else {
                                    g_aasCockpitHitPaletteFades_005d2cb0[5][0] =
                                        0x38;
                                }
                            } else {
                                g_aasCockpitHitPaletteFades_005d2cb0[0][0] =
                                    0x38;
                            }
                        } else if (impact.pitch < 0) {
                            g_aasCockpitHitPaletteFades_005d2cb0[2][0] =
                                0x38;
                        } else {
                            g_aasCockpitHitPaletteFades_005d2cb0[4][0] =
                                0x38;
                        }
                    }
                    g_acShipLastAttacker_004955c0[partner] =
                        g_acObjectOwner_00495208[obj];
                    g_acShipCollisionCooldown_00496010[partner] += 4;
                    if (g_abProjectileCollisionBonus_004960a8[obj] != 0)
                        DAT_004960f0[partner] += 4;
                    if (g_asObjectType_00495298[obj] == 9 ||
                        g_asObjectType_00495298[obj] == 0x0e) {
                        damage = g_asObjectDamage_00495178[obj];
                    } else {
                        lifetime = *(int *)&g_aObjectTypeData_00496d30[
                            g_acObjectType_00493980[obj]].lifetime;
                        damage = (short)(
                            (g_asObjectCounter_00494be0[obj] / lifetime + 1) *
                            (g_asObjectDamage_00495178[obj] >> 1));
                    }
                    adaptiveDamageScale = MinShort(
                        150, MaxShort(70, GetAdaptiveTurnRate()));
                    if (g_asShipSide_004955d0[partner] == 0) {
                        damage = (short)(adaptiveDamageScale * damage / 100);
                    }
                    force = g_aShipVelocity_00494898[obj];
                    NormalizeFixedVector(&force);
                    ScaleFixedVector(&force, (int)damage << 8, &force);
                    negate_vector(&g_vCollisionDelta_00493178);
                    apply_force_to_object(&g_vCollisionDelta_00493178,
                                          &force, partner);
                    g_bApplyingCollisionDamage_00492fb8 = 1;
                    inflict_damage(obj, partner, damage, &relativeVelocity);
                    g_bApplyingCollisionDamage_00492fb8 = 0;
                }
                savedScale = g_asObjectScale_00494d90[obj];
                set_objects_data(
                    obj, WC2_OBJECT_TYPE_PROJECTILE_IMPACT_EFFECT,
                    g_acObjectOwner_00495208[obj], 0);
                g_asObjectScale_00494d90[obj] =
                    (unsigned short)(savedScale + savedScale);
                g_aShipVelocity_00494898[obj] =
                    g_aShipVelocity_00494898[partner];
                RecordCannedSceneObjectEvent(obj, 0);
                if (g_aeObjectClass_00495328[partner] ==
                        OBJECT_CLASS_ASTEROID) {
                    hit_asteroid(partner, 3);
                }
            }
            break;

        case OBJECT_CLASS_MINE:
            if (g_acObjectOwner_00495208[obj] != partner &&
                g_acObjectCollisionGraceTicks_00494d48[obj] < 1) {
                if (easy2see(obj) == 0 &&
                    (g_nCurrentView_00492fa8 == 0 || partner != 0)) {
                    remove_object(obj);
                } else {
                    explode(obj, obj);
                }
            }
            break;

        case OBJECT_CLASS_MISSILE:
            if (g_acObjectOwner_00495208[obj] != partner ||
                g_acObjectCollisionGraceTicks_00494d48[obj] < 1) {
                ScaleFixedVector(&g_aShipVelocity_00494898[obj],
                                 (int)objectMass << 8, &force);
                negate_vector(&g_vCollisionDelta_00493178);
                apply_force_to_object(&g_vCollisionDelta_00493178,
                                      &force, partner);
                explode(obj, obj);
                zero_vector(&g_aShipVelocity_00494898[obj]);
            }
            break;

        case OBJECT_CLASS_ASTEROID:
            if (g_aeObjectClass_00495328[partner] ==
                    OBJECT_CLASS_ASTEROID) {
                if (g_asObjectScreenX_00493598[obj] == (short)0x8001)
                    remove_object(obj);
                else
                    hit_asteroid(obj, 0);
            }
            break;

        case OBJECT_CLASS_SHIP:
        case OBJECT_CLASS_CAPITAL_SHIP:
        case OBJECT_CLASS_BASE:
            if ((g_aeObjectClass_00495328[partner] ==
                     OBJECT_CLASS_ASTEROID ||
                 (g_aeObjectClass_00495328[partner] ==
                      OBJECT_CLASS_MINE &&
                  g_acObjectOwner_00495208[partner] == -1)) &&
                easy2see(partner) == 0 &&
                (g_nCurrentView_00492fa8 == 0 || obj != 0)) {
                remove_object(partner);
            }
            if ((g_aeObjectClass_00495328[partner] ==
                     OBJECT_CLASS_ASTEROID ||
                 g_aeObjectClass_00495328[partner] ==
                     OBJECT_CLASS_SHIP ||
                 g_aeObjectClass_00495328[partner] >=
                     OBJECT_CLASS_CAPITAL_SHIP) &&
                g_acLastCollisionObject_00495250[obj] != partner) {
                PlaySfxWaveFileByNumber(0x1c, obj, 0);
                g_acLastCollisionObject_00495250[partner] =
                    (signed char)obj;
                g_acLastCollisionObject_00495250[obj] =
                    (signed char)partner;

                forceMagnitude =
                    Vector_magnitude(&g_vCollisionDelta_00493178);
                forceMagnitude = DivideFixed(
                    (g_asObjectCollisionRadius_004950e8[obj] +
                     g_asObjectCollisionRadius_004950e8[partner]) << 8,
                    forceMagnitude);
                forceMagnitude = MinInt(forceMagnitude, 0x7d000);
                ScaleFixedVector(&g_vCollisionDelta_00493178,
                                 forceMagnitude, &force);
                AddFixedVectors(&g_aShipPosition_00494550[obj], &force,
                                &g_aShipPosition_00494550[partner]);

                vector_component_in_dir(
                    &g_aShipVelocity_00494898[obj],
                    &g_vCollisionDelta_00493178, &force);
                SubtractFixedVectors(&g_aShipVelocity_00494898[obj],
                                     &force, &tangent);
                vector_component_in_dir(
                    &g_aShipVelocity_00494898[partner],
                    &g_vCollisionDelta_00493178, &partnerComponent);
                ComputeVectorDelta(&partnerComponent, &force,
                                   &componentDelta);
                damage = (short)(Vector_magnitude(&componentDelta) >> 8);
#ifdef WC1_SDL
                if (obj == 0 || partner == 0)
                    Wc1SdlQueueJoystickCollisionRumble(damage);
#endif
                damage = (short)((damage * damage) >> 1);
                totalMass = partnerMass + objectMass;
                responseScale = ((objectMass - partnerMass) << 8) /
                                totalMass;
                responseScale = MinInt(
                    MaxInt(0x40, responseScale), 0x400);
                ScaleFixedVector(&componentDelta, responseScale, &force);
                AddFixedVectors(&force, &partnerComponent, &force);
                forceMagnitude = MultiplyFixed(
                    (int)objectMass * 0x600,
                    Vector_magnitude(&force)) + 0xa00;
                speed = (short)(Vector_magnitude(
                    &g_aShipVelocity_00494898[obj]) >> 8);
                if (speed < g_asShipMaximumVelocity_00495f70[obj]) {
                    AddFixedVectors(&force,
                                    &g_aShipVelocity_00494898[obj],
                                    &g_aShipVelocity_00494898[obj]);
                }

                if (g_aeObjectClass_00495328[obj] ==
                        OBJECT_CLASS_SHIP) {
                    NormalizeFixedVector(&tangent);
                    negate_vector(&tangent);
                    ScaleFixedVector(&tangent, forceMagnitude, &tangent);
                    rotational_acceleration(&g_vCollisionDelta_00493178,
                                            &tangent, obj);
                    negate_vector(&relativeVelocity);
                    if (g_asObjectType_00495298[obj] == 0x33 &&
                        g_aeObjectClass_00495328[partner] ==
                            OBJECT_CLASS_ASTEROID) {
                        damage /= 2;
                    }
                    inflict_damage(partner, obj, damage, &relativeVelocity);
                    negate_vector(&relativeVelocity);
                }

                SubtractFixedVectors(&g_aShipVelocity_00494898[partner],
                                     &partnerComponent, &tangent);
                responseScale = ((objectMass + objectMass) << 8) /
                                totalMass;
                responseScale = MinInt(
                    MaxInt(0x40, responseScale), 0x400);
                ScaleFixedVector(&componentDelta, responseScale, &force);
                AddFixedVectors(&force, &partnerComponent, &force);
                speed = (short)(Vector_magnitude(
                    &g_aShipVelocity_00494898[obj]) >> 8);
                if (speed < g_asShipMaximumVelocity_00495f70[partner]) {
                    AddFixedVectors(&force,
                                    &g_aShipVelocity_00494898[partner],
                                    &g_aShipVelocity_00494898[partner]);
                }
                if (g_aeObjectClass_00495328[partner] ==
                        OBJECT_CLASS_SHIP) {
                    NormalizeFixedVector(&tangent);
                    negate_vector(&tangent);
                    ScaleFixedVector(&tangent, forceMagnitude, &tangent);
                    negate_vector(&g_vCollisionDelta_00493178);
                    rotational_acceleration(&g_vCollisionDelta_00493178,
                                            &tangent, partner);
                    inflict_damage(obj, partner, damage, &relativeVelocity);
                }
                if (g_aeObjectClass_00495328[partner] >=
                        OBJECT_CLASS_CAPITAL_SHIP) {
                    SubtractFixedVectors(&g_aShipPosition_00494550[obj],
                                         &g_aShipVelocity_00494898[obj],
                                         &g_aShipPosition_00494550[obj]);
                    g_anShipSpeed_00494e20[obj] = 0;
                    g_aShipVelocity_00494898[obj] =
                        g_aShipVelocity_00494898[partner];
                }
            }
            break;
        }
    }
}

/* Function start: 0x41E276 */
void object_intelligence(short obj)
{
    short target;

    if (g_nTrainSimActive_0049d758 == 4)
        return;
    switch (g_aeObjectClass_00495328[obj]) {
    case OBJECT_CLASS_SHIP:
        if (g_asObjectType_00495298[obj] == 0x33 &&
            (obj == 0 ||
             (g_bJumpSequenceActive_004962f0 != 0 &&
              g_nYourWingman_0049346c == obj))) {
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
        if (g_nSpaceFrame_00493134 % 4 != 0 &&
            g_nExternalViewShip_00493468 != obj) {
            break;
        }
        target = g_acShipTarget_00495f20[obj];
        if (target != -1)
            get_facing_range_from_object(obj, target);
        switch (g_asObjectType_00495298[obj]) {
        case WC2_OBJECT_TYPE_DART_DUMB_FIRE_MISSILE:
            g_anShipSpeed_00494e20[obj] =
                (get_ship_max_velocity(obj) + 10) * 0x100;
            break;
        case WC2_OBJECT_TYPE_JAVELIN_HEAT_SEEKING_MISSILE:
            heat_seeking_missile_intelligence(obj);
            break;
        case WC2_OBJECT_TYPE_PILUM_FRIEND_OR_FOE_MISSILE:
            FF_missile_intelligence(obj);
            break;
        case WC2_OBJECT_TYPE_SPICULUM_IMAGE_RECOGNITION_MISSILE:
        case WC2_OBJECT_TYPE_TORPEDO:
            point_ship(obj, 0, &g_vToTarget_00493188);
            g_anShipSpeed_00494e20[obj] =
                (get_ship_max_velocity(obj) + 10) * 0x100;
            break;
        case WC2_OBJECT_TYPE_CHAFF_POD:
            chaff_intelligence(obj);
            break;
        }
        break;
    }
}
