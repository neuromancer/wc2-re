/*
 *  Menu input devices and joystick button events.
 *
 *  Retail object 0x421210-0x42190F (link order; see docs/ORDER.md).
 */
#include "game.h"

short g_nInputDoubleClickInterval_00493050 = 1;
short g_nMenuPointerSpeed_00493054 = 2;

/* Function start: 0x421210 */
short UpdateInputDeviceTransitions(short raw)
{
    short changed;
    short buttons;
    short activeDevice;

    activeDevice = g_nActiveInputDeviceWord_005d1726;
    if (raw == 0)
        ReadCalibratedJoystick();
    else
        SampleJoystickDevice(&g_aInputDeviceSamplesList_005d1780[activeDevice],
                             activeDevice, 0);

    g_asInputButton2DoubleClickValues_005d3048[activeDevice] = 0;
    g_asControlsButton1DoubleClickTable_005d3030[activeDevice] =
        g_asInputButton2DoubleClickValues_005d3048[activeDevice];
    g_asInputButton2ChangedList_005d3038[activeDevice] = 0;
    g_asControlsButton1ChangedValues_005d3034[activeDevice] =
        g_asInputButton2ChangedList_005d3038[activeDevice];
    buttons = (short)g_aInputDeviceSamplesList_005d1780[activeDevice].buttons;
    if (g_asPreviousInputButtonsList_005d303c[activeDevice] != buttons) {
        changed = buttons & 1;
        changed ^= g_asPreviousInputButtonsList_005d303c[activeDevice] & 1;
        if (changed != 0) {
            g_asControlsButton1ChangedValues_005d3034[activeDevice]++;
            if ((buttons & 1) != 0) {
                if (g_nInputClock_005c84a8 -
                    g_anControlsButton1PressTime_005d3040[activeDevice] <=
                        g_nInputDoubleClickInterval_00493050 *
                        g_nInputTickScale_005c8d24)
                    g_asControlsButton1DoubleClickTable_005d3030[activeDevice]++;
                g_anControlsButton1PressTime_005d3040[activeDevice] =
                    g_nInputClock_005c84a8;
            }
        }
        changed = buttons & 2;
        changed ^= g_asPreviousInputButtonsList_005d303c[activeDevice] & 2;
        if (changed != 0) {
            g_asInputButton2ChangedList_005d3038[activeDevice]++;
            if ((buttons & 2) != 0) {
                if (g_nInputClock_005c84a8 -
                    g_anInputButton2PushTime_005d3028[activeDevice] <=
                        g_nInputDoubleClickInterval_00493050 *
                        g_nInputTickScale_005c8d24)
                    g_asInputButton2DoubleClickValues_005d3048[activeDevice]++;
                g_anInputButton2PushTime_005d3028[activeDevice] =
                    g_nInputClock_005c84a8;
            }
        }
        g_asPreviousInputButtonsList_005d303c[activeDevice] = buttons;
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

    if (g_nActiveInputDeviceWord_005d1726 == -1)
        return;
    if (g_bInputPollingGuard_005d304c == 0) {
        g_bInputPollingGuard_005d304c++;
        UpdateInputDeviceTransitions(1);
        if (g_asControlsButton1ChangedValues_005d3034
                [g_nActiveInputDeviceWord_005d1726] != 0) {
            doubleClick = 0;
            if (g_asControlsButton1DoubleClickTable_005d3030
                    [g_nActiveInputDeviceWord_005d1726] != 0)
                doubleClick = 3;
            QueueInputEventAtCursor(
                (g_aInputDeviceSamplesList_005d1780
                    [g_nActiveInputDeviceWord_005d1726].buttons & 1) + 1,
                0, doubleClick);
        }
        if (g_asInputButton2ChangedList_005d3038
                [g_nActiveInputDeviceWord_005d1726] != 0) {
            doubleClick = 0;
            if (g_asInputButton2DoubleClickValues_005d3048
                    [g_nActiveInputDeviceWord_005d1726] != 0)
                doubleClick = 3;
            QueueInputEventAtCursor(
                ((g_aInputDeviceSamplesList_005d1780
                    [g_nActiveInputDeviceWord_005d1726].buttons & 2) >> 1) + 1,
                1, doubleClick);
        }
        g_bPersonnelSecondaryButton_005c8d05 =
            (unsigned char)g_aInputDeviceSamplesList_005d1780
                [g_nActiveInputDeviceWord_005d1726].buttons;
        g_bPersonnelPrimaryButton_005c8d04 =
            g_bPersonnelSecondaryButton_005c8d05;
        g_bInputPollingGuard_005d304c--;
    }
}

/* Function start: 0x421530 */
void PollMenuInputDevices(void)
{
    signed char clickKind;
    signed char changes;

    changes = 0;
    if (g_nActiveInputDeviceWord_005d1726 == -1)
        return;
    g_nPersonnelMouseX_005c8d00 = g_stQueuedMouse_005c83f0.x;
    g_nPersonnelMouseY_005c8d02 = g_stQueuedMouse_005c83f0.y;
    if (g_bInputPollingGuard_005d304c == 0) {
        g_bInputPollingGuard_005d304c++;
        UpdateInputDeviceTransitions(0);

        if (g_asControlsButton1ChangedValues_005d3034
                [g_nActiveInputDeviceWord_005d1726] != 0) {
            g_nPersonnelMouseX_005c8d00 +=
                g_aInputDeviceSamplesList_005d1780
                    [g_nActiveInputDeviceWord_005d1726].x *
                g_nMenuPointerSpeed_00493054;
            g_nPersonnelMouseY_005c8d02 +=
                g_aInputDeviceSamplesList_005d1780
                    [g_nActiveInputDeviceWord_005d1726].y *
                g_nMenuPointerSpeed_00493054;
            clickKind = 0;
            if (g_asControlsButton1DoubleClickTable_005d3030
                    [g_nActiveInputDeviceWord_005d1726] != 0)
                clickKind = 3;
            QueueInputEventAtCursor(
                (g_aInputDeviceSamplesList_005d1780
                    [g_nActiveInputDeviceWord_005d1726].buttons & 1) + 1,
                0, (short)clickKind);
            changes++;
        }
        if (g_asInputButton2ChangedList_005d3038
                [g_nActiveInputDeviceWord_005d1726] != 0) {
            g_nPersonnelMouseX_005c8d00 +=
                g_aInputDeviceSamplesList_005d1780
                    [g_nActiveInputDeviceWord_005d1726].x *
                g_nMenuPointerSpeed_00493054;
            g_nPersonnelMouseY_005c8d02 +=
                g_aInputDeviceSamplesList_005d1780
                    [g_nActiveInputDeviceWord_005d1726].y *
                g_nMenuPointerSpeed_00493054;
            clickKind = 0;
            if (g_asControlsButton1DoubleClickTable_005d3030
                    [g_nActiveInputDeviceWord_005d1726] != 0)
                clickKind = 3;
            QueueInputEventAtCursor(
                ((g_aInputDeviceSamplesList_005d1780
                    [g_nActiveInputDeviceWord_005d1726].buttons & 2) >> 1) + 1,
                1, (short)clickKind);
            changes++;
        }
        if (changes == 0) {
            g_nPersonnelMouseX_005c8d00 +=
                g_aInputDeviceSamplesList_005d1780
                    [g_nActiveInputDeviceWord_005d1726].x *
                g_nMenuPointerSpeed_00493054;
            g_nPersonnelMouseY_005c8d02 +=
                g_aInputDeviceSamplesList_005d1780
                    [g_nActiveInputDeviceWord_005d1726].y *
                g_nMenuPointerSpeed_00493054;
            changes = (unsigned char)(
                g_aInputDeviceSamplesList_005d1780
                    [g_nActiveInputDeviceWord_005d1726].y |
                g_aInputDeviceSamplesList_005d1780
                    [g_nActiveInputDeviceWord_005d1726].x);
            if (changes != 0) {
                FlushInputEvents();
                g_nPersonnelMouseX_005c8d00 =
                    g_nPersonnelMouseX_005c8d00 > 0
                        ? g_nPersonnelMouseX_005c8d00 : 0;
                g_nPersonnelMouseX_005c8d00 =
                    g_nPersonnelMouseX_005c8d00 < 319
                        ? g_nPersonnelMouseX_005c8d00 : 319;
                g_nPersonnelMouseY_005c8d02 =
                    g_nPersonnelMouseY_005c8d02 > 0
                        ? g_nPersonnelMouseY_005c8d02 : 0;
                g_nPersonnelMouseY_005c8d02 =
                    g_nPersonnelMouseY_005c8d02 < 199
                        ? g_nPersonnelMouseY_005c8d02 : 199;
                SetMousePosition(g_nPersonnelMouseX_005c8d00,
                                 g_nPersonnelMouseY_005c8d02);
            }
        }

        g_bPersonnelSecondaryButton_005c8d05 =
            (unsigned char)g_aInputDeviceSamplesList_005d1780
                [g_nActiveInputDeviceWord_005d1726].buttons;
        g_bPersonnelPrimaryButton_005c8d04 =
            g_bPersonnelSecondaryButton_005c8d05;
        g_nPersonnelMouseX_005c8d00 =
            g_nPersonnelMouseX_005c8d00 > 0
                ? g_nPersonnelMouseX_005c8d00 : 0;
        g_nPersonnelMouseX_005c8d00 =
            g_nPersonnelMouseX_005c8d00 < 319
                ? g_nPersonnelMouseX_005c8d00 : 319;
        g_nPersonnelMouseY_005c8d02 =
            g_nPersonnelMouseY_005c8d02 > 0
                ? g_nPersonnelMouseY_005c8d02 : 0;
        g_nPersonnelMouseY_005c8d02 =
            g_nPersonnelMouseY_005c8d02 < 199
                ? g_nPersonnelMouseY_005c8d02 : 199;
        if (changes != 0) {
            if (g_bSceneDisplayUpdateActive_00499bb8 == 0)
                DisableMouseCursorDrawing();
            g_stQueuedMouse_005c83f0.x = g_nPersonnelMouseX_005c8d00;
            g_stQueuedMouse_005c83f0.y = g_nPersonnelMouseY_005c8d02;
            g_stQueuedMouse_005c83f0.primaryButton =
                g_bPersonnelPrimaryButton_005c8d04;
            g_stQueuedMouse_005c83f0.secondaryButton =
                g_bPersonnelSecondaryButton_005c8d05;
            g_stQueuedMouse_005c83f0.flags = g_wPersonnelInputFlags_005c8d07;
            if (g_bSceneDisplayUpdateActive_00499bb8 == 0)
                EnableMouseCursorDrawing();
        }
        g_bInputPollingGuard_005d304c--;
    }
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
int g_anInputButton2PushTime_005d3028[2];
short g_asControlsButton1DoubleClickTable_005d3030[2];
short g_asControlsButton1ChangedValues_005d3034[2];
short g_asInputButton2ChangedList_005d3038[2];
short g_asPreviousInputButtonsList_005d303c[2];
int g_anControlsButton1PressTime_005d3040[2];
short g_asInputButton2DoubleClickValues_005d3048[2];
signed char g_bInputPollingGuard_005d304c;
