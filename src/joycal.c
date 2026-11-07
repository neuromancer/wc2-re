/*
 *  Joystick calibration file and calibrated reads.
 *
 *  Retail object 0x423480-0x423C5F (link order; see docs/ORDER.md).
 */
#include "game.h"

const int g_nJoystickUnavailableSample = -1; /* 0x0048e054 */

signed char g_acExpectedGraphicsModes[5] = {
    0x13, 0x0d, 4, 9, 7
}; /* 0x00493078 */

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
    g_4_nControllerFailureValue =
        g_nJoystickUnavailableSample;
    SampleBothJoysticks(stickSamples, g_nJoystickUnavailableSample);
    if (stickSamples[0].x != g_nJoystickUnavailableSample &&
        stickSamples[0].y != g_nJoystickUnavailableSample)
        activeDevice = 0;
    else if (stickSamples[1].x != g_nJoystickUnavailableSample &&
             stickSamples[1].y != g_nJoystickUnavailableSample)
        activeDevice = 1;

    if (activeDevice != -1) {
#ifdef SDL_PORT
        /* SDL already normalizes axes. A legacy calibration belongs to its
         * old slot/device and must not override an explicit launcher choice. */
        file = SdlHasPreferredJoystick() ? -1 : _open("j.cal", 0x8000);
#else
        file = _open("j.cal", 0x8000);
#endif
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
            g_1_nJoystickCentreXNumber = (unsigned int)restX;
            g_2_nJoystickCentreY = (unsigned int)restY;
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
            g_1_nJoystickCentreXNumber =
                ((int)minX + (int)maxX) / 2;
            g_2_nJoystickCentreY =
                ((int)minY + (int)maxY) / 2;
        }

        if (g_1_nJoystickCentreXNumber > 10) {
            g_2_nJoystickCalibrationMinimumXNumber =
                g_1_nJoystickCentreXNumber - 10;
            g_2_nControllerCalibrationMaximumX =
                g_1_nJoystickCentreXNumber + 10;
        } else {
            g_2_nControllerCalibrationMaximumX =
                g_1_nJoystickCentreXNumber;
            g_2_nJoystickCalibrationMinimumXNumber =
                g_2_nControllerCalibrationMaximumX;
        }
        if (g_2_nJoystickCentreY > 10) {
            g_nJoystickCalibrationMinimumY =
                g_2_nJoystickCentreY - 10;
            g_7_nJoystickTuningTopY =
                g_2_nJoystickCentreY + 10;
        } else {
            g_7_nJoystickTuningTopY =
                g_2_nJoystickCentreY;
            g_nJoystickCalibrationMinimumY =
                g_7_nJoystickTuningTopY;
        }

        g_1_nJsRightScaleWord =
            g_2_nJoystickCalibrationMinimumXNumber /
            (int)horizontalRange;
        g_12_nControllerLeftScaleValue =
            g_1_nJsRightScaleWord;
        g_12_nJoystickDownScaleWord =
            g_nJoystickCalibrationMinimumY /
            (int)verticalRange;
        g_59_nJoystickUpScaleWord =
            g_12_nJoystickDownScaleWord;
        if (g_12_nControllerLeftScaleValue == 0)
            g_12_nControllerLeftScaleValue++;
        if (g_59_nJoystickUpScaleWord == 0)
            g_59_nJoystickUpScaleWord++;
        if (g_1_nJsRightScaleWord == 0)
            g_1_nJsRightScaleWord++;
        if (g_12_nJoystickDownScaleWord == 0)
            g_12_nJoystickDownScaleWord++;

        g_12_nJoystickMinimumXNumber =
            g_1_nJoystickCentreXNumber -
            horizontalRange * g_12_nControllerLeftScaleValue;
        g_2_nJsMinimumY =
            g_2_nJoystickCentreY -
            verticalRange * g_59_nJoystickUpScaleWord;
        g_4_nJoystickMaximumXValue =
            horizontalRange * g_12_nControllerLeftScaleValue +
            g_1_nJoystickCentreXNumber;
        g_nJoystickMaximumY =
            verticalRange * g_59_nJoystickUpScaleWord +
            g_2_nJoystickCentreY;
        g_nJoystickHorizDeadZone = horizontalDeadZone;
        g_nControllerVerticalDeadZoneWord = verticalDeadZone;
    }
    g_8_nActiveInputDeviceWord = activeDevice;
    return activeDevice;
}

/* Function start: 0x4238E9 */
short ReadCalibratedJoystick(void)
{
    short normalizedX;
    short normalizedY;
    short inputDevice;

    inputDevice = g_8_nActiveInputDeviceWord;
    if (g_8_nActiveInputDeviceWord == -1)
        return 0;
    SampleJoystickDevice(
        &g_aInputDeviceSamplesList[inputDevice], inputDevice,
        g_4_nControllerFailureValue);
    if (g_aInputDeviceSamplesList[inputDevice].x ==
            g_4_nControllerFailureValue ||
        g_aInputDeviceSamplesList[inputDevice].y ==
            g_4_nControllerFailureValue) {
        g_aInputDeviceSamplesList[inputDevice].x =
            g_aInputDeviceSamplesList[inputDevice].y =
            g_aInputDeviceSamplesList[inputDevice].buttons = 0;
        g_8_nActiveInputDeviceWord = -1;
        return 0;
    }

    g_3_nJoystickRawX =
        g_aInputDeviceSamplesList[inputDevice].x;
    g_nStickRawY =
        g_aInputDeviceSamplesList[inputDevice].y;
    if (g_aInputDeviceSamplesList[inputDevice].x <
        g_12_nJoystickMinimumXNumber)
        g_aInputDeviceSamplesList[inputDevice].x =
            g_12_nJoystickMinimumXNumber;
    if (g_aInputDeviceSamplesList[inputDevice].x >
        g_4_nJoystickMaximumXValue)
        g_aInputDeviceSamplesList[inputDevice].x =
            g_4_nJoystickMaximumXValue;
    if (g_aInputDeviceSamplesList[inputDevice].y <
        g_2_nJsMinimumY)
        g_aInputDeviceSamplesList[inputDevice].y =
            g_2_nJsMinimumY;
    if (g_aInputDeviceSamplesList[inputDevice].y >
        g_nJoystickMaximumY)
        g_aInputDeviceSamplesList[inputDevice].y =
            g_nJoystickMaximumY;

    normalizedY = 0;
    normalizedX = normalizedY;
    if (g_aInputDeviceSamplesList[inputDevice].x <
        g_1_nJoystickCentreXNumber) {
        normalizedX = (short)((g_1_nJoystickCentreXNumber -
            g_aInputDeviceSamplesList[inputDevice].x) /
            g_12_nControllerLeftScaleValue);
        if (normalizedX <= g_nJoystickHorizDeadZone)
            normalizedX = 0;
        else if (normalizedX != 0)
            normalizedX = (short)-normalizedX;
    } else if (g_aInputDeviceSamplesList[inputDevice].x >
               g_1_nJoystickCentreXNumber) {
        normalizedX = (short)((
            g_aInputDeviceSamplesList[inputDevice].x -
            g_1_nJoystickCentreXNumber) /
            g_1_nJsRightScaleWord);
        if (normalizedX <= g_nJoystickHorizDeadZone)
            normalizedX = 0;
    }

    if (g_aInputDeviceSamplesList[inputDevice].y <
        g_2_nJoystickCentreY) {
        normalizedY = (short)((g_2_nJoystickCentreY -
            g_aInputDeviceSamplesList[inputDevice].y) /
            g_59_nJoystickUpScaleWord);
        if (normalizedY <= g_nControllerVerticalDeadZoneWord)
            normalizedY = 0;
        else if (normalizedY != 0)
            normalizedY = (short)-normalizedY;
    } else if (g_aInputDeviceSamplesList[inputDevice].y >
               g_2_nJoystickCentreY) {
        normalizedY = (short)((
            g_aInputDeviceSamplesList[inputDevice].y -
            g_2_nJoystickCentreY) /
            g_12_nJoystickDownScaleWord);
        if (normalizedY <= g_nControllerVerticalDeadZoneWord)
            normalizedY = 0;
    }
    g_aInputDeviceSamplesList[inputDevice].x = (int)normalizedX;
    g_aInputDeviceSamplesList[inputDevice].y = (int)normalizedY;
    return 1;
}

/* Unreferenced driver names; WC1's joystick unit has the same pair. */
static char g_szTMDriverName[] = "TM.DRV"; /* 0x00493080 */
static char g_szSuperTMDriverName[] = "SUPERTM.DRV"; /* 0x00493088 */

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
int g_nJoystickCalibrationMinimumY; /* 0x005d2ff0 */
int g_2_nJoystickCalibrationMinimumXNumber; /* 0x005d2ff4 */
int g_3_nJoystickRawX; /* 0x005d2ff8 */
int g_nStickRawY; /* 0x005d2ffc */
int g_2_nControllerCalibrationMaximumX; /* 0x005d3000 */
int g_7_nJoystickTuningTopY; /* 0x005d3004 */
char g_nUnusedJoycalValue; /* 0x005d3008 */ /* never referenced */
unsigned char *g_1_pCampaignDataBase; /* 0x005d300c */
