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

int g_nCapitalShipViewDistance_00468ff4 = 0x7d000;
unsigned char g_bLandingAuthorized_00468ff8 = 0;
short g_bAlternateChaseView_00492fac = 0;
int g_nChaseCameraMaximumVelocity_00492fb0 = 0x8c00;
short g_nSavedPlayerTarget_0049d460 = -1;
short g_bSavedPlayerTarget_0049d464;
short g_nSavedTargetLockCountdown_0049d484 = 1000;
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
    if (g_nActiveInputDevice_005a819c == -1)
        return;
    if (g_apTextFonts_005d2200[1] != 0)
        fontLoaded = 1;

    g_stDefaultTextContext_005d2d20.alignment = 2;
    InitializeTextContextFromFont(&g_stDefaultTextContext_005d2d20, 1,
        g_cViewportClearColour_004699a0, (signed char)g_cSecondaryViewBufferColour_0049cb4c);
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
        SampleJoystickDevice(samples, g_nActiveInputDevice_005a819c, 0x7fff);
        g_nJoystickMinimumX_005a81b8 = samples[g_nActiveInputDevice_005a819c].x;
        g_nJoystickMinimumY_005a81bc = samples[g_nActiveInputDevice_005a819c].y;
        calibration[0] = (short)samples[g_nActiveInputDevice_005a819c].x;
        calibration[1] = (short)samples[g_nActiveInputDevice_005a819c].y;
        WaitForJoystickButtonRelease();
        ReleaseModalTextPanel();
    }

    shown = ShowModalTextPanel(1,
        "Move stick to the LOWER RIGHT, press a button");
    if (shown != 0) {
        DIBslamReal();
        WaitForJoystickButtonPress();
        SampleJoystickDevice(samples, g_nActiveInputDevice_005a819c, 0x7fff);
        g_nJoystickMaximumX_005a81b0 = samples[g_nActiveInputDevice_005a819c].x;
        g_nJoystickMaximumY_005a81b4 = samples[g_nActiveInputDevice_005a819c].y;
        calibration[2] = (short)samples[g_nActiveInputDevice_005a819c].x;
        calibration[3] = (short)samples[g_nActiveInputDevice_005a819c].y;
        WaitForJoystickButtonRelease();
        ReleaseModalTextPanel();
    }

    shown = ShowModalTextPanel(1,
        "Center Joystick, press a button");
    if (shown != 0) {
        DIBslamReal();
        WaitForJoystickButtonPress();
        SampleJoystickDevice(samples, g_nActiveInputDevice_005a819c, 0x7fff);
        g_nJoystickCentreX_005a81dc = samples[g_nActiveInputDevice_005a819c].x;
        g_nJoystickCentreY_005a81d8 = samples[g_nActiveInputDevice_005a819c].y;
        calibration[4] = (short)samples[g_nActiveInputDevice_005a819c].x;
        calibration[5] = (short)samples[g_nActiveInputDevice_005a819c].y;
        WaitForJoystickButtonRelease();
        ReleaseModalTextPanel();
    }

    if (fontLoaded == 0)
        ReleaseTextFont(1);

    if (g_nJoystickHorizontalRange_005a81cc == 0)
        g_nJoystickHorizontalRange_005a81cc = 9;
    if (g_nJoystickVerticalRange_005a81c8 == 0)
        g_nJoystickVerticalRange_005a81c8 = 9;
    g_nJoystickLeftScale_005a81ac =
        (g_nJoystickCentreX_005a81dc - g_nJoystickMinimumX_005a81b8) /
        g_nJoystickHorizontalRange_005a81cc;
    g_nJoystickRightScale_005a81d0 =
        (g_nJoystickMaximumX_005a81b0 - g_nJoystickCentreX_005a81dc) /
        g_nJoystickHorizontalRange_005a81cc;
    g_nJoystickUpScale_005a81a8 =
        (g_nJoystickCentreY_005a81d8 - g_nJoystickMinimumY_005a81bc) /
        g_nJoystickVerticalRange_005a81c8;
    g_nJoystickDownScale_005a81d4 =
        (g_nJoystickMaximumY_005a81b4 - g_nJoystickCentreY_005a81d8) /
        g_nJoystickVerticalRange_005a81c8;
    if (g_nJoystickLeftScale_005a81ac == 0)
        g_nJoystickLeftScale_005a81ac = 1;
    if (g_nJoystickRightScale_005a81d0 == 0)
        g_nJoystickRightScale_005a81d0 = 1;
    if (g_nJoystickUpScale_005a81a8 == 0)
        g_nJoystickUpScale_005a81a8 = 1;
    if (g_nJoystickDownScale_005a81d4 == 0)
        g_nJoystickDownScale_005a81d4 = 1;

    g_nJoystickMinimumX_005a81b8 = g_nJoystickCentreX_005a81dc -
        g_nJoystickLeftScale_005a81ac *
            g_nJoystickHorizontalRange_005a81cc;
    g_nJoystickMinimumY_005a81bc = g_nJoystickCentreY_005a81d8 -
        g_nJoystickUpScale_005a81a8 * g_nJoystickVerticalRange_005a81c8;
    g_nJoystickMaximumX_005a81b0 = g_nJoystickCentreX_005a81dc +
        g_nJoystickRightScale_005a81d0 *
            g_nJoystickHorizontalRange_005a81cc;
    g_nJoystickMaximumY_005a81b4 = g_nJoystickCentreY_005a81d8 +
        g_nJoystickDownScale_005a81d4 * g_nJoystickVerticalRange_005a81c8;
    g_nJoystickFailureValue_005a81e0 = g_nJoystickMaximumX_005a81b0 * 2;

    if (g_nJoystickMaximumX_005a81b0 <= g_nJoystickMinimumX_005a81b8 ||
        g_nJoystickMaximumY_005a81b4 <= g_nJoystickMinimumY_005a81bc) {
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
    failed = _write(file, &g_nActiveInputDevice_005a819c, 2) == -1;
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
            &g_aInputDeviceSamples_005a81f0[
                g_nActiveInputDevice_005a819c],
            g_nActiveInputDevice_005a819c, 0);
    } while (g_aInputDeviceSamples_005a81f0[
                 g_nActiveInputDevice_005a819c].buttons != 0);
}

/* Function start: 0x418D5F */
void WaitForJoystickButtonPress(void)
{
    do {
        SampleJoystickDevice(
            &g_aInputDeviceSamples_005a81f0[
                g_nActiveInputDevice_005a819c],
            g_nActiveInputDevice_005a819c, 0);
    } while (g_aInputDeviceSamples_005a81f0[
                 g_nActiveInputDevice_005a819c].buttons == 0);
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
    g_aShipPosition_00494550[63] = centre;
    g_aShipRightVector_00493b78[63] = orientation;
    g_aShipUpVector_00493ec0[63] = orientation;
    point_at(63, g_aShipPosition_00494550[0]);

    ScaleFixedVector(&g_aShipRightVector_00493b78[63],
                     cameraDistance >> 2, &offset);
    AddFixedVectors(&g_aShipPosition_00494550[63], &offset,
                    &g_aShipPosition_00494550[63]);
    ScaleFixedVector(&g_aShipUpVector_00493ec0[63], 0x9600, &offset);
    AddFixedVectors(&g_aShipPosition_00494550[63], &offset,
                    &g_aShipPosition_00494550[63]);
    ScaleFixedVector(&g_aShipForwardVector_00494208[63],
                     cameraDistance, &offset);
    AddFixedVectors(&g_aShipPosition_00494550[63], &offset,
                    &g_aShipPosition_00494550[63]);

    g_aShipPosition_00494550[WC2_EYE_OBJECT] =
        g_aShipPosition_00494550[63];
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
        g_bRestorePlayerTarget_00493500 != 0 &&
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
            g_bRestorePlayerTarget_00493500 = 1;
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
unsigned int house_keep_objects(void)
{
    short obj;

    for (obj = 0; obj <= WC1_SPACE_LAST_MOVING_OBJECT; obj++) {
        switch (g_aeObjectClass_00495328[obj]) {
        case OBJECT_CLASS_DUST:
            if (g_acObjectType_00493980[obj] == OBJECT_TYPE_DEBRIS_DUST &&
                count_down(obj) == -1 &&
                g_asObjectScreenX_00493598[obj] == (short)0x8001)
                remove_object(obj);
            break;
        case OBJECT_CLASS_DEBRIS:
            if (count_down(obj) == -1)
                remove_object(obj);
            break;
        case OBJECT_CLASS_FIXED_OBJECT:
            if (g_acObjectType_00493980[obj] == OBJECT_TYPE_TURRET ||
                g_acObjectType_00493980[obj] == OBJECT_TYPE_THRUSTERS)
                remove_object(obj);
            break;
        case OBJECT_CLASS_PROJECTILE:
            if (count_down(obj) == 0) {
                if (g_acObjectType_00493980[obj] == OBJECT_TYPE_TURRET)
                    explode((short)g_acObjectOwner_00495208[obj], obj);
                else
                    remove_object(obj);
            }
            break;
        case OBJECT_CLASS_MINE:
            if (g_acObjectCollisionGraceTicks_0059ddb0[obj] > 0)
                g_acObjectCollisionGraceTicks_0059ddb0[obj]--;
            if (count_down(obj) == 0)
                explode(obj, obj);
            break;
        case OBJECT_CLASS_MISSILE:
            g_abShipExhaustHeat_0059d610[obj] = 0;
            if (g_acObjectCollisionGraceTicks_0059ddb0[obj] > 0)
                g_acObjectCollisionGraceTicks_0059ddb0[obj]--;
            if (g_aeShipTactic_0059d5e0[obj] == TACTIC_SIT_STILL) {
                if (count_down(obj) <= 0) {
                    g_aeShipTactic_0059d5e0[obj] = TACTIC_RAM;
                    g_asObjectCounter_00494be0[obj] =
                        g_aObjectTypeData_00496d30[
                            g_acObjectType_00493980[obj]].lifetime;
                    if (g_acObjectType_00493980[obj] ==
                            OBJECT_TYPE_DUMB_FIRE_MISSILE) {
                        vector_component_in_dir(
                            &g_aShipVelocity_00494898[obj],
                            &g_aShipForwardVector_00494208[obj],
                            &g_aShipVelocity_00494898[obj]);
                    }
                }
            } else if (count_down(obj) <= 0) {
                explode(obj, obj);
            }
            break;
        case OBJECT_CLASS_SHIP:
        case OBJECT_CLASS_CAPITAL_SHIP:
            g_abShipExhaustHeat_0059d610[obj] = 0;
            if (count_down(obj) > 0) {
                if (g_asShipManeuver_00495f48[obj] ==
                        MANEUVER_WARPING_OUT)
                    g_asObjectScale_00494d90[obj] >>= 1;
                if (g_aeSpecialManeuver_00495600[obj] ==
                        SPECIAL_MANEUVER_UNKNOWN_9 &&
                    g_aeObjectClass_00495328[obj] ==
                        OBJECT_CLASS_CAPITAL_SHIP) {
                    if (g_asObjectCounter_00494be0[obj] == 7) {
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
            } else if (g_asObjectCounter_00494be0[obj] == 0 &&
                       g_aeSpecialManeuver_00495600[obj] ==
                           SPECIAL_MANEUVER_UNKNOWN_9) {
                if (g_nYourWingman_0049346c != -1 &&
                    DAT_0059c910[obj] == 0 &&
                    g_asShipSide_004955d0[obj] == SIDE_KILRATHI &&
                    RandomBelowOrEqual(100) < 10) {
                    send_message(g_nYourWingman_0049346c, 6);
                }
                Create_explosion_debris(obj);
                break;
            } else if (g_asObjectCounter_00494be0[obj] == 0) {
                if (g_asShipManeuver_00495f48[obj] ==
                        MANEUVER_WARPING_IN) {
                    if (g_aeShipTactic_0059d5e0[obj] != TACTIC_WARP_IN) {
                        if ((short)g_acObjectOwner_00495208[obj] == obj) {
                            set_objects_data(
                                obj,
                                (enum ObjectType)
                                    g_abShipNavPointIndex_00495f60[obj],
                                -1, 0);
                            reset_maneuver(obj, -1);
                        } else {
                            remove_object(obj);
                        }
                    }
                } else if (g_asShipManeuver_00495f48[obj] ==
                               MANEUVER_WARPING_OUT &&
                           g_asShipSide_004955d0[obj] != SIDE_NEUTRAL) {
                    if (obj == 0 || obj == g_nYourWingman_0049346c) {
                        if (obj == 0)
                            CompleteStarSystemJump();
                    } else {
                        g_aMissionShips_00492290[
                            g_nShipMissionIndices_0059c830[obj]].state = 2;
                        remove_object(obj);
                    }
                }
            }
            if (g_acObjectType_00493980[obj] ==
                    OBJECT_TYPE_TIGERS_CLAW &&
                g_bPlayerCollisionsEnabled_00469ff8 != 0 &&
                g_bLandingAuthorized_00468ff8 != 0 &&
                normal_speed(0) != 0) {
                get_facing_range_from_object(0, obj);
                if (g_nTargetRange_0049319c < 700 &&
                    g_nFacingToTarget_00493194 > 75 &&
                    g_nTargetFacing_00493198 > 70) {
                    g_nArcadeState_0049d75c = 1;
                    g_nPlayerCollisionObject_00493480 = obj;
                }
            }
            break;
        }
    }
    return 0;
}

/* Function start: 0x41C7ED */
unsigned int update_objects_in_space(void)
{
    short obj;

    clear_crash_cache();
    obj = 0;
    do {
        if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_FUTURION) {
            futurion_intelligence(obj);
        } else if (g_aeObjectClass_00495328[obj] > OBJECT_CLASS_PLANET) {
            animate_object(obj);
            if (g_aeObjectClass_00495328[obj] != OBJECT_CLASS_NULL &&
                g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_PROJECTILE) {
                object_collision(obj);
                rotate_object(obj);
                if (obj >= 10 ||
                    g_aeSpecialManeuver_00495600[obj] !=
                        SPECIAL_MANEUVER_UNKNOWN_9) {
                    if (obj != 0)
                        object_intelligence(obj);
                    if (obj < 10 &&
                        g_aeObjectClass_00495328[obj] >=
                            OBJECT_CLASS_MISSILE) {
                        if (obj != 0)
                            rotate_object_to_goal(obj);
                        if (g_aeObjectClass_00495328[obj] ==
                                OBJECT_CLASS_SHIP)
                            replenish_weapon_energy_bank(obj);
                    }
                }
            }
        }
        obj++;
    } while (obj <= WC1_SPACE_LAST_MOVING_OBJECT);

    obj = 0;
    do {
        if (g_aeObjectClass_00495328[obj] > OBJECT_CLASS_PLANET) {
            accelerate_and_move_object(obj);
            if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_SHIP) {
                replenish_shields(obj);
                housekeep_power_plant_and_fuel(obj);
            }
        }
        obj++;
    } while (obj <= WC1_SPACE_LAST_MOVING_OBJECT);
    return 0;
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
unsigned int accelerate_and_move_object(short obj)
{
    FixedVector delta;
    FixedVector accelerationVector;
    int acceleration;
    int magnitude;

    if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_MISSILE) {
        if (g_aeSpecialManeuver_00495600[obj] ==
                SPECIAL_MANEUVER_KILL_ENGINES) {
            g_abShipExhaustHeat_0059d610[obj] = 0;
            if (RandomBelowOrEqual(100) < 10)
                set_special(obj, SPECIAL_MANEUVER_NONE);
        } else if (g_aeSpecialManeuver_00495600[obj] ==
                       SPECIAL_MANEUVER_STOP_DRIFT) {
            approach_zero_speed(obj);
            NormalizeAndScaleVector(&g_aShipVelocity_00494898[obj],
                                    g_anShipSpeed_0059b320[obj]);
            if (g_anShipSpeed_0059b320[obj] == 0)
                set_special(obj, SPECIAL_MANEUVER_NONE);
        }
        if (g_aeSpecialManeuver_00495600[obj] <
                SPECIAL_MANEUVER_KILL_ENGINES &&
            g_aeShipTactic_0059d5e0[obj] != TACTIC_SIT_STILL) {
            zero_vector(&accelerationVector);
            switch (g_aeSpecialManeuver_00495600[obj]) {
            case SPECIAL_MANEUVER_AFTERBURNER:
                g_asShipAfterburnerTimer_0059c810[obj]--;
                if (g_asShipAfterburnerTimer_0059c810[obj] == 0) {
                    g_aeSpecialManeuver_00495600[obj] =
                        SPECIAL_MANEUVER_NONE;
                    if (DAT_005a7cec != 0) {
                        FlushSoundEffectsAndLog();
                        DAT_005a7cec = 0;
                    }
                    ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                                     g_anShipSpeed_0059b320[obj], &delta);
                } else {
                    ScaleFixedVector(
                        &g_aShipForwardVector_00494208[obj],
                        (g_aObjectTypeData_00496d30[
                            g_acObjectType_00493980[obj]].maximumVelocity +
                         20) * 0x200,
                        &delta);
                    drain_fuel(obj, 200);
                    g_abShipExhaustHeat_0059d610[obj] = 3;
                }
                break;
            case SPECIAL_MANEUVER_SUPER_BRAKE:
                g_asShipAfterburnerTimer_0059c810[obj]--;
                if (g_asShipAfterburnerTimer_0059c810[obj] == 0) {
                    g_aeSpecialManeuver_00495600[obj] =
                        SPECIAL_MANEUVER_NONE;
                    ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                                     g_anShipSpeed_0059b320[obj], &delta);
                } else {
                    zero_vector(&delta);
                    drain_fuel(obj, 140);
                }
                break;
            default:
                ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                                 g_anShipSpeed_0059b320[obj], &delta);
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
                    g_abShipExhaustHeat_0059d610[obj] = 2;
                }
            }
            AddFixedVectors(&accelerationVector,
                            &g_aShipVelocity_00494898[obj],
                            &g_aShipVelocity_00494898[obj]);
            if (obj == 0)
                g_vPlayerAcceleration_0059b460 = accelerationVector;
        }
    }
    AddFixedVectors(&g_aShipPosition_00494550[obj],
                    &g_aShipVelocity_00494898[obj],
                    &g_aShipPosition_00494550[obj]);
    return 0;
}

/* Function start: 0x41D07B */
unsigned int animate_shape(short obj)
{
    unsigned char *animation;
    enum ObjectType type;
    short command;

    type = g_acObjectType_00493980[obj];
    animation = g_aObjectTypeData_00496d30[type].animation;
    if (animation == 0)
        return 0;
    if (--g_asObjectAnimationDelay_0059b660[obj] > 0)
        return 0;
    g_asObjectAnimationDelay_0059b660[obj] =
        g_aObjectTypeData_00496d30[type].yawRate;
    command = *(unsigned short *)(animation +
              g_asObjectAnimationIndex_0059da30[obj] * 4);
    switch (command & 0xf000) {
    case 0x9000:
        command &= 0x0fff;
        g_asObjectAnimationIndex_0059da30[obj] = command;
        command = *(unsigned short *)(animation + command * 4);
        if (g_asObjectScreenX_00493598[obj] != (short)0x8001 &&
            (type == OBJECT_TYPE_DEBRIS_WING ||
             type == OBJECT_TYPE_DEBRIS_METAL_SHEET)) {
            PlaySfxWaveFileByNumber(13, obj, 0);
        }
        break;
    case 0xa000:
        remove_object(obj);
        return 0;
    }

    if ((command & 0x0c00) == 0x0400) {
        g_asObjectScale_00494d90[obj] +=
            (command & 0x3f) * (g_asObjectScale_00494d90[obj] >> 6);
    } else if ((command & 0x0c00) == 0x0800) {
        g_asObjectScale_00494d90[obj] -=
            (command & 0x3f) * (g_asObjectScale_00494d90[obj] >> 6);
    } else {
        command &= 0x3f;
        g_asObjectViewFrame_00493508[obj] = command;
    }
    g_asObjectFlip_004939c8[obj] = (command & 0xc0) >> 2;
    g_asObjectAnimationIndex_0059da30[obj]++;
    return 0;
}

/* Function start: 0x41D2DA */
unsigned int animate_object(short obj)
{
    FixedVector offset;
    short effect;

    switch (g_aeObjectClass_00495328[obj]) {
    case OBJECT_CLASS_EXPLOSION:
        animate_shape(obj);
        break;
    case OBJECT_CLASS_DEBRIS:
        animate_shape(obj);
        break;
    case OBJECT_CLASS_FIXED_OBJECT:
        animate_shape(obj);
        break;
    case OBJECT_CLASS_ASTEROID:
        animate_shape(obj);
        break;
    case OBJECT_CLASS_MINE:
        animate_shape(obj);
        break;
    case OBJECT_CLASS_SHIP:
        if (g_asObjectScreenX_00493598[obj] == (short)0x8001 ||
            (g_nRenderedSpaceFrame_00493138 & 3) != 0 ||
            (g_aObjectTypeData_00496d30[
                 g_acObjectType_00493980[obj]].damageCapacity >> 1) - 1 >
                g_asShipAccumulatedDamage_0059dee0[obj]) {
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
                              OBJECT_TYPE_RED_SPARK),
            obj, 0);
        g_asObjectScale_00494d90[effect] =
            g_asObjectScale_00494d90[obj];
        if (RandomInRange(0, 3) == 0)
            PlaySfxWaveFileByNumber(7, obj, 0, 0);
        break;
    }
    return 0;
}

/* Function start: 0x41D4C7 */
unsigned int hit_asteroid(short asteroid, short destructionChance)
{
    short fragments;

    if (RandomBelowOrEqual((short)(destructionChance - 1)) == 0) {
        fragments = (short)(RandomBelowOrEqual(1) + 2);
        while (fragments > 0) {
            make_shard(asteroid, g_aShipVelocity_00494898[asteroid]);
            fragments--;
        }
        explode(-1, asteroid);
    } else if (RandomBelowOrEqual(7) == 0) {
        make_shard(asteroid, g_vCollisionDelta_0059d690);
    }
    return 0;
}

/* Function start: 0x41D591 */
int object_collision(short obj)
{
    SphericalVector impact;
    FixedVector relativeVelocity;
    FixedVector separation;
    FixedVector objectComponent;
    FixedVector partnerComponent;
    FixedVector componentDelta;
    FixedVector tangent;
    FixedVector partnerTangent;
    FixedVector impulse;
    FixedVector force;
    enum ObjectClass partnerClass;
    int separationScale;
    int responseScale;
    int forceMagnitude;
    int totalMass;
    int objectMass;
    int partnerMass;
    int collisionSpeed;
    short partner;
    short damage;
    short savedScale;
    signed char owner;

    partner = check_for_collision(obj);
    if (partner == -1) {
        g_acLastCollisionObject_0059d6a0[obj] = -1;
        return 0;
    }
    if (DAT_0046a000 == 0 && (obj == 0 || partner == 0))
        return 0;

    NormalizeFixedVector(&g_vCollisionDelta_0059d690);
    ComputeVectorDelta(&g_aShipVelocity_00494898[partner],
                       &g_aShipVelocity_00494898[obj],
                       &relativeVelocity);
    NormalizeFixedVector(&relativeVelocity);
    owner = g_acObjectOwner_00495208[obj];
    switch (g_aeObjectClass_00495328[obj]) {
    case OBJECT_CLASS_PROJECTILE:
        if (owner == partner)
            break;
        partnerClass = g_aeObjectClass_00495328[partner];
        if (partnerClass > OBJECT_CLASS_MINE) {
            if (partner == 0) {
                rectangular_to_spherical(
                    &g_aObjectViewPosition_0059afa0[obj], &impact);
                if (abs(impact.pitch) < 45) {
                    if (abs(impact.yaw) < 45)
                        g_aPaletteFadeEntries_005a76d0[1][0] = 0x38;
                    else if (abs(impact.yaw) < 136) {
                        if (impact.yaw < 0)
                            g_aPaletteFadeEntries_005a76d0[3][0] = 0x38;
                        else
                            g_aPaletteFadeEntries_005a76d0[5][0] = 0x38;
                    }
                    else
                        g_aPaletteFadeEntries_005a76d0[0][0] = 0x38;
                } else if (impact.pitch < 0)
                    g_aPaletteFadeEntries_005a76d0[2][0] = 0x38;
                else
                    g_aPaletteFadeEntries_005a76d0[4][0] = 0x38;
            }
            DAT_0059c910[partner] = owner;
            g_acShipAiCooldown_0059d680[partner] += 4;
            damage = (short)(g_asShipAccumulatedDamage_0059dee0[obj] -
                             g_asObjectCounter_00494be0[obj] / 2);
            force = g_aShipVelocity_00494898[obj];
            NormalizeFixedVector(&force);
            ScaleFixedVector(&force, (int)damage << 8, &force);
            negate_vector(&g_vCollisionDelta_0059d690);
            apply_force_to_object(&g_vCollisionDelta_0059d690,
                                  &force, partner);
            inflict_damage(obj, partner, damage, &relativeVelocity);
        }
        savedScale = g_asObjectScale_00494d90[obj];
        set_objects_data(obj, OBJECT_TYPE_LASER_SPARK, owner, 0);
        g_asObjectScale_00494d90[obj] = (short)(savedScale * 2);
        g_aShipVelocity_00494898[obj] =
            g_aShipVelocity_00494898[partner];
        if (partnerClass == OBJECT_CLASS_ASTEROID)
            hit_asteroid(partner, 3);
        return 0;

    case OBJECT_CLASS_ASTEROID:
        if (g_aeObjectClass_00495328[partner] ==
                OBJECT_CLASS_ASTEROID) {
            if (g_asObjectScreenX_00493598[obj] == (short)0x8001) {
                remove_object(obj);
                return 0;
            }
            hit_asteroid(obj, 0);
            return 0;
        }
        break;

    case OBJECT_CLASS_MINE:
        if (owner == partner ||
            g_acObjectCollisionGraceTicks_0059ddb0[obj] > 0)
            return 0;
        if (easy2see(obj) == 0 &&
            (g_nCurrentView_00492fa8 == 0 || partner != 0)) {
            remove_object(obj);
            return 0;
        }
        explode(obj, obj);
        return 0;

    case OBJECT_CLASS_MISSILE:
        if (owner != partner ||
            g_acObjectCollisionGraceTicks_0059ddb0[obj] < 1) {
            ScaleFixedVector(&g_aShipVelocity_00494898[obj],
                (unsigned short)g_asObjectRadarRadius_0059c790[obj]
                    << 8, &force);
            negate_vector(&g_vCollisionDelta_0059d690);
            apply_force_to_object(&g_vCollisionDelta_0059d690,
                                  &force, partner);
            explode(obj, obj);
            zero_vector(&g_aShipVelocity_00494898[obj]);
            return 0;
        }
        break;

    case OBJECT_CLASS_SHIP:
    case OBJECT_CLASS_CAPITAL_SHIP:
        partnerClass = g_aeObjectClass_00495328[partner];
        if ((partnerClass == OBJECT_CLASS_ASTEROID ||
             (partnerClass == OBJECT_CLASS_MINE &&
              g_acObjectOwner_00495208[partner] == -1)) &&
            easy2see(partner) == 0 &&
            (g_nCurrentView_00492fa8 == 0 || obj != 0)) {
            remove_object(partner);
        }
        partnerClass = g_aeObjectClass_00495328[partner];
        if ((partnerClass == OBJECT_CLASS_ASTEROID ||
             partnerClass == OBJECT_CLASS_SHIP ||
             partnerClass == OBJECT_CLASS_CAPITAL_SHIP) &&
            g_acLastCollisionObject_0059d6a0[obj] != partner) {
            PlaySfxWaveFileByNumber(0x1c, obj, 0);
            g_acLastCollisionObject_0059d6a0[obj] =
                (signed char)partner;
            g_acLastCollisionObject_0059d6a0[partner] =
                (signed char)obj;

            separationScale = DivideFixed(
                (g_asObjectCollisionRadius_004950e8[obj] +
                 g_asObjectCollisionRadius_004950e8[partner]) << 8,
                Vector_magnitude(&g_vCollisionDelta_0059d690));
            separationScale = MinInt(separationScale, 0x7d000);
            ScaleFixedVector(&g_vCollisionDelta_0059d690,
                             separationScale, &separation);
            AddFixedVectors(&g_aShipPosition_00494550[obj], &separation,
                            &g_aShipPosition_00494550[partner]);

            vector_component_in_dir(
                &g_aShipVelocity_00494898[obj],
                &g_vCollisionDelta_0059d690, &objectComponent);
            SubtractFixedVectors(&g_aShipVelocity_00494898[obj],
                                 &objectComponent, &tangent);
            vector_component_in_dir(
                &g_aShipVelocity_00494898[partner],
                &g_vCollisionDelta_0059d690, &partnerComponent);
            ComputeVectorDelta(&partnerComponent, &objectComponent,
                               &componentDelta);
            collisionSpeed =
                (short)((unsigned int)Vector_magnitude(&componentDelta) >> 8);
            damage = (short)((collisionSpeed * collisionSpeed) >> 1);
#ifdef WC1_SDL
            if (obj == 0 || partner == 0)
                Wc1SdlQueueJoystickCollisionRumble(collisionSpeed);
#endif

            objectMass = (unsigned short)
                g_asObjectRadarRadius_0059c790[obj];
            partnerMass = (unsigned short)
                g_asObjectRadarRadius_0059c790[partner];
            totalMass = objectMass + partnerMass;
            responseScale = ((objectMass - partnerMass) * 256) /
                            totalMass;
            responseScale = MaxInt(0x40, responseScale);
            responseScale = MinInt(responseScale, 0x400);
            ScaleFixedVector(&componentDelta, responseScale, &impulse);
            AddFixedVectors(&impulse, &partnerComponent, &impulse);
            forceMagnitude = MultiplyFixed(
                objectMass * 0x600,
                Vector_magnitude(&impulse)) + 0xa00;
            AddFixedVectors(&impulse, &g_aShipVelocity_00494898[obj],
                            &g_aShipVelocity_00494898[obj]);

            if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP) {
                NormalizeFixedVector(&tangent);
                negate_vector(&tangent);
                ScaleFixedVector(&tangent, forceMagnitude, &tangent);
                rotational_acceleration(&g_vCollisionDelta_0059d690,
                                        &tangent, obj);
                negate_vector(&relativeVelocity);
                inflict_damage(partner, obj, damage, &relativeVelocity);
                negate_vector(&relativeVelocity);
            }

            SubtractFixedVectors(&g_aShipVelocity_00494898[partner],
                                 &partnerComponent, &partnerTangent);
            responseScale = (objectMass << 9) / totalMass;
            responseScale = MaxInt(0x40, responseScale);
            responseScale = MinInt(responseScale, 0x400);
            ScaleFixedVector(&componentDelta, responseScale, &impulse);
            AddFixedVectors(&impulse, &partnerComponent, &impulse);
            AddFixedVectors(&impulse,
                            &g_aShipVelocity_00494898[partner],
                            &g_aShipVelocity_00494898[partner]);
            if (partnerClass == OBJECT_CLASS_SHIP) {
                NormalizeFixedVector(&partnerTangent);
                negate_vector(&partnerTangent);
                ScaleFixedVector(&partnerTangent, forceMagnitude,
                                 &partnerTangent);
                negate_vector(&g_vCollisionDelta_0059d690);
                rotational_acceleration(&g_vCollisionDelta_0059d690,
                                        &partnerTangent, partner);
                inflict_damage(obj, partner, damage, &relativeVelocity);
            }
            if (partnerClass == OBJECT_CLASS_CAPITAL_SHIP) {
                SubtractFixedVectors(&g_aShipPosition_00494550[obj],
                                     &g_aShipVelocity_00494898[obj],
                                     &g_aShipPosition_00494550[obj]);
                g_anShipSpeed_0059b320[obj] = 0;
                g_aShipVelocity_00494898[obj] =
                    g_aShipVelocity_00494898[partner];
            }
        }
        break;
    }
    return 0;
}

/* Function start: 0x41E276 */
unsigned int object_intelligence(short obj)
{
    if (g_nCannedSceneMode_0049021c == 4)
        return 0;
    if (g_nCannedSceneMode_0049021c == 2 &&
        g_aeObjectClass_00495328[obj] > OBJECT_CLASS_MISSILE) {
        update_canned_sequence(obj);
        return 0;
    }
    switch (g_aeObjectClass_00495328[obj]) {
    case OBJECT_CLASS_FUTURION:
        futurion_intelligence(obj);
        break;
    case OBJECT_CLASS_MINE:
        mine_intelligence(obj);
        break;
    case OBJECT_CLASS_MISSILE:
        if ((abs((int)g_nSpaceFrame_00493134) & 3) != 0 &&
            g_nExternalViewShip_00493468 != obj) {
            break;
        }
        if (g_acShipTarget_00495f20[obj] != -1) {
            get_facing_range_from_object(
                obj, g_acShipTarget_00495f20[obj]);
        }
        switch (g_acObjectType_00493980[obj]) {
        case OBJECT_TYPE_DUMB_FIRE_MISSILE:
            g_anShipSpeed_0059b320[obj] =
                (get_ship_max_velocity(obj) + 10) * 0x100;
            break;
        case OBJECT_TYPE_HEAT_SEEKING_MISSILE:
            heat_seeking_missile_intelligence(obj);
            break;
        case OBJECT_TYPE_FF_MISSILE:
            FF_missile_intelligence(obj);
            break;
        case OBJECT_TYPE_IMAGE_RECOGNITION_MISSILE:
            point_ship(obj, 0, &g_vToTarget_00493188);
            g_anShipSpeed_0059b320[obj] =
                (get_ship_max_velocity(obj) + 10) * 0x100;
            break;
        }
        break;
    case OBJECT_CLASS_SHIP:
        ship_intelligence(obj);
        break;
    case OBJECT_CLASS_CAPITAL_SHIP:
        capital_ship_intelligence(obj);
        break;
    }
    return 0;
}
