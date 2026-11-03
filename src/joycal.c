/*
 *  Joystick calibration file and calibrated reads.
 *
 *  Retail object 0x423480-0x423C5F (link order; see docs/ORDER.md).
 */
#include "game.h"

const int g_nJoystickUnavailableSample_0048e054 = -1;

signed char g_acExpectedGraphicsModes_00493078[5] = {
    0x13, 0x0d, 4, 9, 7
};

/* Function start: 0x423480 */
short LoadJoystickCalibrationFile(short horizontalRange,
                                  short verticalRange,
                                  short horizontalDeadZone,
                                  short verticalDeadZone)
{
    InputDeviceSample stickSamples[2];
    unsigned short restX;
    unsigned short restY;
    unsigned short maxY;
    unsigned short minY;
    unsigned short minX;
    int fileError;
    unsigned short maxX;
    int file;
    short activeDevice;
    unsigned short passes;

    activeDevice = -1;
    /* The DOS polling calibration's starting pass count; unused here. */
    passes = 0x40;
    fileError = 1;
    if (horizontalRange == 0)
        horizontalRange++;
    if (verticalRange == 0)
        verticalRange++;
    g_nControllerFailureValue_005d176c =
        g_nJoystickUnavailableSample_0048e054;
    SampleBothJoysticks(stickSamples, g_nJoystickUnavailableSample_0048e054);
    if (stickSamples[0].x != g_nJoystickUnavailableSample_0048e054 &&
        stickSamples[0].y != g_nJoystickUnavailableSample_0048e054)
        activeDevice = 0;
    else if (stickSamples[1].x != g_nJoystickUnavailableSample_0048e054 &&
             stickSamples[1].y != g_nJoystickUnavailableSample_0048e054)
        activeDevice = 1;

    if (activeDevice != -1) {
        file = _open("j.cal", 0x8000);
        if (file != -1) {
            fileError = _read(file, &activeDevice, 2) <= 0;
            if (fileError == 0)
                fileError = _read(file, &minX, 2) <= 0;
            if (fileError == 0)
                fileError = _read(file, &minY, 2) <= 0;
            if (fileError == 0)
                fileError = _read(file, &maxX, 2) <= 0;
            if (fileError == 0)
                fileError = _read(file, &maxY, 2) <= 0;
            if (fileError == 0)
                fileError = _read(file, &restX, 2) <= 0;
            if (fileError == 0)
                fileError = _read(file, &restY, 2) <= 0;
            g_nJoystickCentreXNumber_005d1768 = (unsigned int)restX;
            g_nJoystickCentreY_005d1764 = (unsigned int)restY;
            _close(file);
            if (fileError != 0)
                _unlink("j.cal");
        }

        if (fileError != 0) {
            GetJoystickDevCaps(activeDevice,
                               (short *)&minX,
                               (short *)&maxX,
                               (short *)&minY,
                               (short *)&maxY);
            g_nJoystickCentreXNumber_005d1768 =
                ((int)minX + (int)maxX) / 2;
            g_nJoystickCentreY_005d1764 =
                ((int)minY + (int)maxY) / 2;
        }

        if (g_nJoystickCentreXNumber_005d1768 > 10) {
            g_nJoystickCalibrationMinimumXNumber_005d2ff4 =
                g_nJoystickCentreXNumber_005d1768 - 10;
            g_nControllerCalibrationMaximumX_005d3000 =
                g_nJoystickCentreXNumber_005d1768 + 10;
        } else {
            g_nControllerCalibrationMaximumX_005d3000 =
                g_nJoystickCentreXNumber_005d1768;
            g_nJoystickCalibrationMinimumXNumber_005d2ff4 =
                g_nControllerCalibrationMaximumX_005d3000;
        }
        if (g_nJoystickCentreY_005d1764 > 10) {
            g_nJoystickCalibrationMinimumY_005d2ff0 =
                g_nJoystickCentreY_005d1764 - 10;
            g_nJoystickTuningTopY_005d3004 =
                g_nJoystickCentreY_005d1764 + 10;
        } else {
            g_nJoystickTuningTopY_005d3004 =
                g_nJoystickCentreY_005d1764;
            g_nJoystickCalibrationMinimumY_005d2ff0 =
                g_nJoystickTuningTopY_005d3004;
        }

        g_nJsRightScaleWord_005d175c =
            g_nJoystickCalibrationMinimumXNumber_005d2ff4 /
            (int)horizontalRange;
        g_nControllerLeftScaleValue_005d1740 =
            g_nJsRightScaleWord_005d175c;
        g_nJoystickDownScaleWord_005d1760 =
            g_nJoystickCalibrationMinimumY_005d2ff0 /
            (int)verticalRange;
        g_nJoystickUpScaleWord_005d173c =
            g_nJoystickDownScaleWord_005d1760;
        if (g_nControllerLeftScaleValue_005d1740 == 0)
            g_nControllerLeftScaleValue_005d1740++;
        if (g_nJoystickUpScaleWord_005d173c == 0)
            g_nJoystickUpScaleWord_005d173c++;
        if (g_nJsRightScaleWord_005d175c == 0)
            g_nJsRightScaleWord_005d175c++;
        if (g_nJoystickDownScaleWord_005d1760 == 0)
            g_nJoystickDownScaleWord_005d1760++;

        g_nJoystickMinimumXNumber_005d174c =
            g_nJoystickCentreXNumber_005d1768 -
            horizontalRange * g_nControllerLeftScaleValue_005d1740;
        g_nJsMinimumY_005d1750 =
            g_nJoystickCentreY_005d1764 -
            verticalRange * g_nJoystickUpScaleWord_005d173c;
        g_nJoystickMaximumXValue_005d1744 =
            horizontalRange * g_nControllerLeftScaleValue_005d1740 +
            g_nJoystickCentreXNumber_005d1768;
        g_nJoystickMaximumY_005d1748 =
            verticalRange * g_nJoystickUpScaleWord_005d173c +
            g_nJoystickCentreY_005d1764;
        g_nJoystickHorizDeadZone_005d172c = horizontalDeadZone;
        g_nControllerVerticalDeadZoneWord_005d1728 = verticalDeadZone;
    }
    g_nActiveInputDeviceWord_005d1726 = activeDevice;
    return activeDevice;
}

/* Function start: 0x4238E9 */
short ReadCalibratedJoystick(void)
{
    short normalizedX;
    short normalizedY;
    short inputDevice;

    inputDevice = g_nActiveInputDeviceWord_005d1726;
    if (g_nActiveInputDeviceWord_005d1726 == -1)
        return 0;
    SampleJoystickDevice(
        &g_aInputDeviceSamplesList_005d1780[inputDevice], inputDevice,
        g_nControllerFailureValue_005d176c);
    if (g_aInputDeviceSamplesList_005d1780[inputDevice].x ==
            g_nControllerFailureValue_005d176c ||
        g_aInputDeviceSamplesList_005d1780[inputDevice].y ==
            g_nControllerFailureValue_005d176c) {
        g_aInputDeviceSamplesList_005d1780[inputDevice].x =
            g_aInputDeviceSamplesList_005d1780[inputDevice].y =
            g_aInputDeviceSamplesList_005d1780[inputDevice].buttons = 0;
        g_nActiveInputDeviceWord_005d1726 = -1;
        return 0;
    }

    g_nJoystickRawX_005d2ff8 =
        g_aInputDeviceSamplesList_005d1780[inputDevice].x;
    g_nStickRawY_005d2ffc =
        g_aInputDeviceSamplesList_005d1780[inputDevice].y;
    if (g_aInputDeviceSamplesList_005d1780[inputDevice].x <
        g_nJoystickMinimumXNumber_005d174c)
        g_aInputDeviceSamplesList_005d1780[inputDevice].x =
            g_nJoystickMinimumXNumber_005d174c;
    if (g_aInputDeviceSamplesList_005d1780[inputDevice].x >
        g_nJoystickMaximumXValue_005d1744)
        g_aInputDeviceSamplesList_005d1780[inputDevice].x =
            g_nJoystickMaximumXValue_005d1744;
    if (g_aInputDeviceSamplesList_005d1780[inputDevice].y <
        g_nJsMinimumY_005d1750)
        g_aInputDeviceSamplesList_005d1780[inputDevice].y =
            g_nJsMinimumY_005d1750;
    if (g_aInputDeviceSamplesList_005d1780[inputDevice].y >
        g_nJoystickMaximumY_005d1748)
        g_aInputDeviceSamplesList_005d1780[inputDevice].y =
            g_nJoystickMaximumY_005d1748;

    normalizedY = 0;
    normalizedX = normalizedY;
    if (g_aInputDeviceSamplesList_005d1780[inputDevice].x <
        g_nJoystickCentreXNumber_005d1768) {
        normalizedX = (short)((g_nJoystickCentreXNumber_005d1768 -
            g_aInputDeviceSamplesList_005d1780[inputDevice].x) /
            g_nControllerLeftScaleValue_005d1740);
        if (normalizedX <= g_nJoystickHorizDeadZone_005d172c)
            normalizedX = 0;
        else if (normalizedX != 0)
            normalizedX = (short)-normalizedX;
    } else if (g_aInputDeviceSamplesList_005d1780[inputDevice].x >
               g_nJoystickCentreXNumber_005d1768) {
        normalizedX = (short)((
            g_aInputDeviceSamplesList_005d1780[inputDevice].x -
            g_nJoystickCentreXNumber_005d1768) /
            g_nJsRightScaleWord_005d175c);
        if (normalizedX <= g_nJoystickHorizDeadZone_005d172c)
            normalizedX = 0;
    }

    if (g_aInputDeviceSamplesList_005d1780[inputDevice].y <
        g_nJoystickCentreY_005d1764) {
        normalizedY = (short)((g_nJoystickCentreY_005d1764 -
            g_aInputDeviceSamplesList_005d1780[inputDevice].y) /
            g_nJoystickUpScaleWord_005d173c);
        if (normalizedY <= g_nControllerVerticalDeadZoneWord_005d1728)
            normalizedY = 0;
        else if (normalizedY != 0)
            normalizedY = (short)-normalizedY;
    } else if (g_aInputDeviceSamplesList_005d1780[inputDevice].y >
               g_nJoystickCentreY_005d1764) {
        normalizedY = (short)((
            g_aInputDeviceSamplesList_005d1780[inputDevice].y -
            g_nJoystickCentreY_005d1764) /
            g_nJoystickDownScaleWord_005d1760);
        if (normalizedY <= g_nControllerVerticalDeadZoneWord_005d1728)
            normalizedY = 0;
    }
    g_aInputDeviceSamplesList_005d1780[inputDevice].x = (int)normalizedX;
    g_aInputDeviceSamplesList_005d1780[inputDevice].y = (int)normalizedY;
    return 1;
}

/* Unreferenced driver names; WC1's joystick unit has the same pair. */
static char g_szTMDriverName_00493080[] = "TM.DRV";
static char g_szSuperTMDriverName_00493088[] = "SUPERTM.DRV";

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
int g_nJoystickCalibrationMinimumY_005d2ff0;
int g_nJoystickCalibrationMinimumXNumber_005d2ff4;
int g_nJoystickRawX_005d2ff8;
int g_nStickRawY_005d2ffc;
int g_nControllerCalibrationMaximumX_005d3000;
int g_nJoystickTuningTopY_005d3004;
char g_nUnusedJoycalValue_005d3008; /* never referenced */
unsigned char *DAT_005d300c;
