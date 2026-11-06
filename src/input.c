/*
 *  Menu input devices and joystick button events.
 *
 *  Retail object 0x421210-0x42190F (link order; see docs/ORDER.md).
 */
#include "game.h"

short g_nInputDoubleClickInterval = 1; /* 0x00493050 */
short g_nMenuPointerSpeed = 2; /* 0x00493054 */

/* Function start: 0x421210 */
short UpdateInputDeviceTransitions(short raw)
{
    short changed;
    short buttons;
    short activeDevice;

    activeDevice = g_8_nActiveInputDeviceWord;
    if (raw == 0)
        ReadCalibratedJoystick();
    else
        SampleJoystickDevice(&g_aInputDeviceSamplesList[activeDevice],
                             activeDevice, 0);

    g_7_asInputButton2DoubleClickValues[activeDevice] = 0;
    g_asControlsButton1DoubleClickTable[activeDevice] =
        g_7_asInputButton2DoubleClickValues[activeDevice];
    g_9_asInputButton2ChangedList[activeDevice] = 0;
    g_3_asControlsButton1ChangedValues[activeDevice] =
        g_9_asInputButton2ChangedList[activeDevice];
    buttons = (short)g_aInputDeviceSamplesList[activeDevice].buttons;
    if (g_2_asPreviousInputButtonsList[activeDevice] != buttons) {
        changed = buttons & 1;
        changed ^= g_2_asPreviousInputButtonsList[activeDevice] & 1;
        if (changed != 0) {
            g_3_asControlsButton1ChangedValues[activeDevice]++;
            if ((buttons & 1) != 0) {
                if (g_196_nInputTimerTicks -
                    g_anControlsButton1PressTime[activeDevice] <=
                        g_nInputDoubleClickInterval *
                        g_3_nEventClockScaleValue)
                    g_asControlsButton1DoubleClickTable[activeDevice]++;
                g_anControlsButton1PressTime[activeDevice] =
                    g_196_nInputTimerTicks;
            }
        }
        changed = buttons & 2;
        changed ^= g_2_asPreviousInputButtonsList[activeDevice] & 2;
        if (changed != 0) {
            g_9_asInputButton2ChangedList[activeDevice]++;
            if ((buttons & 2) != 0) {
                if (g_196_nInputTimerTicks -
                    g_9_anInputButton2PushTime[activeDevice] <=
                        g_nInputDoubleClickInterval *
                        g_3_nEventClockScaleValue)
                    g_7_asInputButton2DoubleClickValues[activeDevice]++;
                g_9_anInputButton2PushTime[activeDevice] =
                    g_196_nInputTimerTicks;
            }
        }
        g_2_asPreviousInputButtonsList[activeDevice] = buttons;
    }
    return 1;
}

/* Function start: 0x4213FE */
/* Nothing in the shipped executable calls this; the DOS joystick pointer
 * module's null procedure. */
void JoystickInputHook(void)
{
}

/* Function start: 0x421409 */
void PollJoystickButtonEvents(void)
{
    short doubleClick;

    if (g_8_nActiveInputDeviceWord == -1)
        return;
    if (g_3_bInputPollingGuard == 0) {
        g_3_bInputPollingGuard++;
        UpdateInputDeviceTransitions(1);
        if (g_3_asControlsButton1ChangedValues
                [g_8_nActiveInputDeviceWord] != 0) {
            doubleClick = 0;
            if (g_asControlsButton1DoubleClickTable
                    [g_8_nActiveInputDeviceWord] != 0)
                doubleClick = 3;
            QueueInputEventAtCursor(
                (g_aInputDeviceSamplesList
                    [g_8_nActiveInputDeviceWord].buttons & 1) + 1,
                0, doubleClick);
        }
        if (g_9_asInputButton2ChangedList
                [g_8_nActiveInputDeviceWord] != 0) {
            doubleClick = 0;
            if (g_7_asInputButton2DoubleClickValues
                    [g_8_nActiveInputDeviceWord] != 0)
                doubleClick = 3;
            QueueInputEventAtCursor(
                ((g_aInputDeviceSamplesList
                    [g_8_nActiveInputDeviceWord].buttons & 2) >> 1) + 1,
                1, doubleClick);
        }
        g_6_stPersonnelMouseData.secondaryButton =
            (unsigned char)g_aInputDeviceSamplesList
                [g_8_nActiveInputDeviceWord].buttons;
        g_6_stPersonnelMouseData.primaryButton =
            g_6_stPersonnelMouseData.secondaryButton;
        g_3_bInputPollingGuard--;
    }
}

/* Function start: 0x421530 */
void PollMenuInputDevices(void)
{
    signed char clickKind;
    signed char changes;

    changes = 0;
    if (g_8_nActiveInputDeviceWord == -1)
        return;
    g_6_stPersonnelMouseData.x = g_16_stEventQueuePointerData.x;
    g_6_stPersonnelMouseData.y = g_16_stEventQueuePointerData.y;
    if (g_3_bInputPollingGuard == 0) {
        g_3_bInputPollingGuard++;
        UpdateInputDeviceTransitions(0);

        if (g_3_asControlsButton1ChangedValues
                [g_8_nActiveInputDeviceWord] != 0) {
            g_6_stPersonnelMouseData.x +=
                g_aInputDeviceSamplesList
                    [g_8_nActiveInputDeviceWord].x *
                g_nMenuPointerSpeed;
            g_6_stPersonnelMouseData.y +=
                g_aInputDeviceSamplesList
                    [g_8_nActiveInputDeviceWord].y *
                g_nMenuPointerSpeed;
            clickKind = 0;
            if (g_asControlsButton1DoubleClickTable
                    [g_8_nActiveInputDeviceWord] != 0)
                clickKind = 3;
            QueueInputEventAtCursor(
                (g_aInputDeviceSamplesList
                    [g_8_nActiveInputDeviceWord].buttons & 1) + 1,
                0, (short)clickKind);
            changes++;
        }
        if (g_9_asInputButton2ChangedList
                [g_8_nActiveInputDeviceWord] != 0) {
            g_6_stPersonnelMouseData.x +=
                g_aInputDeviceSamplesList
                    [g_8_nActiveInputDeviceWord].x *
                g_nMenuPointerSpeed;
            g_6_stPersonnelMouseData.y +=
                g_aInputDeviceSamplesList
                    [g_8_nActiveInputDeviceWord].y *
                g_nMenuPointerSpeed;
            clickKind = 0;
            if (g_asControlsButton1DoubleClickTable
                    [g_8_nActiveInputDeviceWord] != 0)
                clickKind = 3;
            QueueInputEventAtCursor(
                ((g_aInputDeviceSamplesList
                    [g_8_nActiveInputDeviceWord].buttons & 2) >> 1) + 1,
                1, (short)clickKind);
            changes++;
        }
        if (changes == 0) {
            g_6_stPersonnelMouseData.x +=
                g_aInputDeviceSamplesList
                    [g_8_nActiveInputDeviceWord].x *
                g_nMenuPointerSpeed;
            g_6_stPersonnelMouseData.y +=
                g_aInputDeviceSamplesList
                    [g_8_nActiveInputDeviceWord].y *
                g_nMenuPointerSpeed;
            changes = (unsigned char)(
                g_aInputDeviceSamplesList
                    [g_8_nActiveInputDeviceWord].y |
                g_aInputDeviceSamplesList
                    [g_8_nActiveInputDeviceWord].x);
            if (changes != 0) {
                FlushInputEvents();
                g_6_stPersonnelMouseData.x =
                    g_6_stPersonnelMouseData.x > 0
                        ? g_6_stPersonnelMouseData.x : 0;
                g_6_stPersonnelMouseData.x =
                    g_6_stPersonnelMouseData.x < 319
                        ? g_6_stPersonnelMouseData.x : 319;
                g_6_stPersonnelMouseData.y =
                    g_6_stPersonnelMouseData.y > 0
                        ? g_6_stPersonnelMouseData.y : 0;
                g_6_stPersonnelMouseData.y =
                    g_6_stPersonnelMouseData.y < 199
                        ? g_6_stPersonnelMouseData.y : 199;
                SetMousePosition(g_6_stPersonnelMouseData.x,
                                 g_6_stPersonnelMouseData.y);
            }
        }

        g_6_stPersonnelMouseData.secondaryButton =
            (unsigned char)g_aInputDeviceSamplesList
                [g_8_nActiveInputDeviceWord].buttons;
        g_6_stPersonnelMouseData.primaryButton =
            g_6_stPersonnelMouseData.secondaryButton;
        g_6_stPersonnelMouseData.x =
            g_6_stPersonnelMouseData.x > 0
                ? g_6_stPersonnelMouseData.x : 0;
        g_6_stPersonnelMouseData.x =
            g_6_stPersonnelMouseData.x < 319
                ? g_6_stPersonnelMouseData.x : 319;
        g_6_stPersonnelMouseData.y =
            g_6_stPersonnelMouseData.y > 0
                ? g_6_stPersonnelMouseData.y : 0;
        g_6_stPersonnelMouseData.y =
            g_6_stPersonnelMouseData.y < 199
                ? g_6_stPersonnelMouseData.y : 199;
        if (changes != 0) {
            if (g_bSceneDisplayUpdateActive == 0)
                DisableMouseCursorDrawing();
            g_16_stEventQueuePointerData.x = g_6_stPersonnelMouseData.x;
            g_16_stEventQueuePointerData.y = g_6_stPersonnelMouseData.y;
            g_16_stEventQueuePointerData.primaryButton =
                g_6_stPersonnelMouseData.primaryButton;
            g_16_stEventQueuePointerData.secondaryButton =
                g_6_stPersonnelMouseData.secondaryButton;
            g_16_stEventQueuePointerData.flags = g_6_stPersonnelMouseData.flags;
            if (g_bSceneDisplayUpdateActive == 0)
                EnableMouseCursorDrawing();
        }
        g_3_bInputPollingGuard--;
    }
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
int g_9_anInputButton2PushTime[2]; /* 0x005d3028 */
short g_asControlsButton1DoubleClickTable[2]; /* 0x005d3030 */
short g_3_asControlsButton1ChangedValues[2]; /* 0x005d3034 */
short g_9_asInputButton2ChangedList[2]; /* 0x005d3038 */
short g_2_asPreviousInputButtonsList[2]; /* 0x005d303c */
int g_anControlsButton1PressTime[2]; /* 0x005d3040 */
short g_7_asInputButton2DoubleClickValues[2]; /* 0x005d3048 */
signed char g_3_bInputPollingGuard; /* 0x005d304c */
