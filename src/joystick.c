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
    SampleJoystickDevice(&g_aInputDeviceSamplesList_005d1780[
                             g_nActiveInputDeviceWord_005d1726],
                         g_nActiveInputDeviceWord_005d1726, 0);
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short g_nActiveInputDeviceWord_005d1726;
int g_nControllerVerticalDeadZoneWord_005d1728;
int g_nJoystickHorizDeadZone_005d172c;
int g_nUnusedControllerNumber_005d1730[3]; /* never referenced */
int g_nJoystickUpScaleWord_005d173c;
int g_nControllerLeftScaleValue_005d1740;
int g_nJoystickMaximumXValue_005d1744;
int g_nJoystickMaximumY_005d1748;
int g_nJoystickMinimumXNumber_005d174c;
int g_nJsMinimumY_005d1750;
int g_nJoystickVerticalRangeNumber_005d1754;
int g_nControllerHorizontalRange_005d1758;
int g_nJsRightScaleWord_005d175c;
int g_nJoystickDownScaleWord_005d1760;
int g_nJoystickCentreY_005d1764;
int g_nJoystickCentreXNumber_005d1768;
int g_nControllerFailureValue_005d176c;
char g_nUnusedJsWord_005d1770; /* never referenced */
InputDeviceSample g_aInputDeviceSamplesList_005d1780[2];
short g_nFlightControlCommandValue_005d1798;
char g_nUnusedJoystickWord_005d179a; /* never referenced */
