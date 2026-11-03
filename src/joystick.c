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
    SampleJoystickDevice(&g_aInputDeviceSamples_005d1780[
                             g_nActiveInputDevice_005d1726],
                         g_nActiveInputDevice_005d1726, 0);
}
