/*
 *  Joystick sampling.
 *
 *  Retail object 0x44F1F0-0x44F2FF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x44F1F0 */
short SampleBothJoysticks(InputDeviceSample *samples,
                          unsigned int fallback)
{
    GetJoystickPosition((unsigned int *)&samples[0].x,
                        (unsigned int *)&samples[0].y,
                        (int *)&samples[0].buttons, 0, fallback);
    GetJoystickPosition((unsigned int *)&samples[1].x,
                        (unsigned int *)&samples[1].y,
                        (int *)&samples[1].buttons, 1, fallback);
    return 1;
}

/* Function start: 0x44F247 */
void SampleJoystickDevice(InputDeviceSample *samples, short joystick,
                          unsigned int fallback)
{
    short sampleIndex;

    sampleIndex = 0;
    if (joystick != 0)
        sampleIndex = 1;
    else
        sampleIndex = 0;
    GetJoystickPosition(
        (unsigned int *)&samples[sampleIndex].x,
        (unsigned int *)&samples[sampleIndex].y,
        (int *)&samples[sampleIndex].buttons, joystick, fallback);
}

/* Function start: 0x44F2B8 */
/* Nothing in the shipped executable calls this. */
short ReadJoystickButtons(void)
{
    return GetJoystickButtons();
}

/* Function start: 0x44F2CD */
/* Nothing in the shipped executable calls this.  Samples the active input
 * device into its own sample slot. */
void SampleActiveJoystickDevice(void)
{
    SampleJoystickDevice(&g_aInputDeviceSamplesList[
                             g_8_nActiveInputDeviceWord],
                         g_8_nActiveInputDeviceWord, 0);
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short g_8_nActiveInputDeviceWord; /* 0x005d1726 */
int g_nControllerVerticalDeadZoneWord; /* 0x005d1728 */
int g_nJoystickHorizDeadZone; /* 0x005d172c */
int g_26_nUnusedControllerNumber[3]; /* 0x005d1730 */ /* never referenced */
int g_59_nJoystickUpScaleWord; /* 0x005d173c */
int g_12_nControllerLeftScaleValue; /* 0x005d1740 */
int g_4_nJoystickMaximumXValue; /* 0x005d1744 */
int g_nJoystickMaximumY; /* 0x005d1748 */
int g_12_nJoystickMinimumXNumber; /* 0x005d174c */
int g_2_nJsMinimumY; /* 0x005d1750 */
int g_nJoystickVerticalRangeNumber; /* 0x005d1754 */
int g_3_nControllerHorizontalRange; /* 0x005d1758 */
int g_1_nJsRightScaleWord; /* 0x005d175c */
int g_12_nJoystickDownScaleWord; /* 0x005d1760 */
int g_2_nJoystickCentreY; /* 0x005d1764 */
int g_1_nJoystickCentreXNumber; /* 0x005d1768 */
int g_4_nControllerFailureValue; /* 0x005d176c */
char g_nUnusedJsWord; /* 0x005d1770 */ /* never referenced */
InputDeviceSample g_aInputDeviceSamplesList[2]; /* 0x005d1780 */
short g_3_nFlightControlCommandValue; /* 0x005d1798 */
char g_16_nUnusedJoystickWord; /* 0x005d179a */ /* never referenced */
