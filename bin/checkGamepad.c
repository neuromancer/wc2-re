/* WC2's SDL event and polling paths on virtual controllers: legacy triggers
 * and views, device priority, and Custom gamepad and raw joystick bindings.
 * No game assets are needed. */
#include "game.h"

static SDL_Joystick *pad;

static void Require(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

/* Virtual controller buttons and axes; the triggers are half axes, 0 when
   released. */
enum { PAD_A = 0, PAD_BACK = 4 };
enum { PAD_LEFT_TRIGGER = 4, PAD_RIGHT_TRIGGER = 5 };

static void SetAxis(int axis, Sint16 value)
{
    Require(SDL_JoystickSetVirtualAxis(pad, axis, value) == 0, "move a virtual axis");
    SDL_JoystickUpdate();
    SdlPumpEvents();
}

static void SetButton(int button, int pressed)
{
    Require(SDL_JoystickSetVirtualButton(pad, button, pressed) == 0, "press a virtual button");
    SDL_JoystickUpdate();
    SdlPumpEvents();
}

static unsigned int PolledButtons(void)
{
    JOYINFO information;

    Require(SdlReadJoystick(0, &information), "poll the controller as WC2's joystick");
    return information.wButtons;
}

/* Whether a key with this scan code was pressed since the last call. */
static int QueuedPress(unsigned int scanCode)
{
    InputEvent *event;
    int found = 0;

    for (event = g_pInputEventHead; event != NULL; event = event->next) {
        if (event->type == INPUT_EVENT_KEY_DOWN && event->field_14 == scanCode)
            found = 1;
    }
    FlushInputEvents();
    return found;
}

static int AnyQueuedPress(void)
{
    InputEvent *event;
    int found = 0;

    for (event = g_pInputEventHead; event != NULL; event = event->next)
        found |= event->type == INPUT_EVENT_KEY_DOWN;
    FlushInputEvents();
    return found;
}

/* Exercise slot choice with SDL enumerating pedals, a throttle, then a stick. */
static void CheckDevicePriority(void)
{
    int devices[3];
    SDL_Joystick *handles[3];
    JOYINFO information;
    char guid[33];
    int index;
    static const SDL_JoystickType types[3] = {
        SDL_JOYSTICK_TYPE_UNKNOWN, SDL_JOYSTICK_TYPE_THROTTLE, SDL_JOYSTICK_TYPE_FLIGHT_STICK
    };

    for (index = 0; index < 3; index++) {
        devices[index] = SDL_JoystickAttachVirtual(types[index], 2, 2, 0);
        Require(devices[index] >= 0, "attach pedals, throttle and flight stick");
        handles[index] = SDL_JoystickOpen(devices[index]);
        Require(handles[index] != NULL, "open the priority fixture");
        SDL_JoystickSetVirtualAxis(handles[index], 0, index == 2 ? 0 : 32767);
    }
    SDL_JoystickUpdate();
    Require(SdlReadJoystick(0, &information) && information.wXpos == 32768,
            "the centred flight stick takes slot 1 before the pedals and throttle");
    Require(SdlReadJoystick(1, &information) && information.wXpos == 65535,
            "an unknown device precedes the throttle in the remaining slot");
    SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(handles[1]), guid, sizeof(guid));
    SdlSetPreferredJoystick(SDL_JoystickInstanceID(handles[1]), guid, "", 1);
    Require(SdlReadJoystick(0, &information) && information.wXpos == 65535,
            "an explicit throttle choice overrides automatic device priority");
    SdlShutdownJoysticks();
    for (index = 2; index >= 0; index--) {
        SDL_JoystickClose(handles[index]);
        Require(SDL_JoystickDetachVirtual(devices[index]) == 0, "detach the priority fixture");
    }
    SDL_FlushEvents(SDL_JOYAXISMOTION, SDL_FINGERDOWN - 1);
}

static void CalibrateCustomStick(void)
{
    g_8_nActiveInputDeviceWord = 0;
    g_4_nControllerFailureValue = (unsigned int)-1;
    g_1_nJoystickCentreXNumber = 32767;
    g_2_nJoystickCentreY = 32767;
    g_nJoystickHorizDeadZone = 2;
    g_nControllerVerticalDeadZoneWord = 2;
    SdlRestoreJoystickCalibration(10, 10, 65524, 65524, 9, 9);
    g_asShipMaximumVelocity[0] = 400;
    g_176_pfnCurrentInputPump = get_player_input;
    g_nNextInputPollTick = 0x7fffffff;
    g_nArcadeState = 0;
    SdlSetJoystickMode("custom");
    SdlSetPadBindings("");
    SdlApplyJoystickFlightControls();
}

static void CustomFrame(void)
{
    get_player_input();
    Require(g_aInputDeviceSamplesList[0].x == 0 && g_aInputDeviceSamplesList[0].y == 0,
            "Custom takes the calibrated axes before the original stick code sees them");
    player_input();
}

static void CheckCustomGamepad(void)
{
    CalibrateCustomStick();
    SdlSetPadBindings("left-stick-x=roll\nleft-stick-y=throttle\nright-stick-x=yaw\nright-stick-y=-pitch\n"
                      "a=\nb=roll-and-speed\nx=eject\ny=attack-my-target\ndpad-up=pitch-up\n");
    SetAxis(0, 32767);
    SetAxis(1, -32768);
    SetAxis(3, -32768);
    CustomFrame();
    Require(g_anFlightInput[2] == 9 && g_anFlightInput[1] == 0 && g_anFlightInput[0] == -9 &&
            g_anShipSpeed[0] == (400 << 8), "Custom roll, full throttle and reversed pitch use the chosen axes");
    SetAxis(0, 0);
    SetAxis(1, 32767);
    SetAxis(3, 0);
    CustomFrame();
    Require(g_anFlightInput[2] == 0 && g_anFlightInput[0] == 0 && g_anShipSpeed[0] == 0,
            "centred Custom axes stop turning and the throttle can stop the ship");
    SetButton(PAD_A, 1);
    Require(PolledButtons() == 0, "an unbound A does not fire");
    SetButton(PAD_A, 0);
    SetAxis(PAD_RIGHT_TRIGGER, 32767);
    Require(PolledButtons() == 1, "the independently bound right trigger fires the guns");
    SetAxis(PAD_RIGHT_TRIGGER, 0);
    SdlSetPadBindings("left-trigger=roll\n");
    SetAxis(1, 0);
    SetAxis(PAD_LEFT_TRIGGER, 32767);
    CustomFrame();
    Require(g_anFlightInput[2] == 9, "a trigger can be an analog roll axis");
    SetAxis(PAD_LEFT_TRIGGER, 0);
    CustomFrame();
    Require(g_anFlightInput[2] == 0, "an analog trigger releases the turn");

    SdlSetPadBindings("b=roll-and-speed\nx=eject\ny=attack-my-target\ndpad-up=pitch-up\n");
    SetAxis(0, 32767);
    SetAxis(1, -32768);
    SetButton(1, 1);
    CustomFrame();
    Require(g_anFlightInput[2] == 9 && g_anFlightInput[1] == 0 && g_anFlightInput[0] == 0 &&
            g_anShipSpeed[0] > 0, "the held modifier turns yaw into roll and pitch into speed");
    SetButton(1, 0);
    SetAxis(0, 0);
    SetAxis(1, 0);
    CustomFrame();
    FlushInputEvents();
    SetButton(2, 1);
    SetButton(2, 0);
    Require(QueuedPress(0x12) && SdlGetAsyncKeyState(VK_CONTROL),
            "a quick eject press keeps Ctrl until flight consumes the action");
    CustomFrame();
    CustomFrame();
    Require(!SdlGetAsyncKeyState(VK_CONTROL), "the controller's synthetic Ctrl releases");
    g_1_bSpaceFlightActive = 1;
    SetButton(3, 1);
    SetButton(3, 0);
    Require(g_bMenuAHotkey && !AnyQueuedPress(), "WC2's attack command does not also press autopilot");
    g_bMenuAHotkey = 0;
    SetButton(11, 1);
    Require(SdlGetAsyncKeyState(VK_DOWN), "a rebound D-pad direction holds a steering key");
    SetButton(11, 0);
    CustomFrame();
    CustomFrame();
    Require(!SdlGetAsyncKeyState(VK_DOWN), "the steering key releases with its D-pad direction");
    SdlSetPadBindings("dpad-up=fire-guns\n");
    g_acVduModeStackDepth[1] = 0;
    g_aaiVduModeStack[1][0] = 4;
    SetButton(11, 1);
    Require(PolledButtons() == 0, "the communications display consumes a D-pad bound to held guns");
    SetButton(11, 0);
    g_aaiVduModeStack[1][0] = 0;
    FlushInputEvents();
}

static void SetHat(int hat, Uint8 value)
{
    Require(SDL_JoystickSetVirtualHat(pad, hat, value) == 0, "move a virtual hat");
    SDL_JoystickUpdate();
    SdlPumpEvents();
}

static void CheckCustomFlightStick(void)
{
    int device = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_FLIGHT_STICK, 8, 32, 2);

    Require(device >= 0, "attach a flight stick with eight axes, 32 buttons and two hats");
    pad = SDL_JoystickOpen(device);
    Require(pad != NULL, "open the raw flight stick");
    g_176_pfnCurrentInputPump = PollMenuInputDevices;
    SdlPumpEvents();
    CalibrateCustomStick();
    SdlSetPadBindings("axis-1=roll\naxis-2=-pitch\naxis-3=\naxis-4=\naxis-7=yaw\naxis-8=throttle\n"
                      "button-1=\nbutton-32=fire-guns\nhat-2-up=eject\nhat-2-right=jump\n");
    SetAxis(0, 32767);
    SetAxis(1, -32768);
    SetAxis(6, -32768);
    SetAxis(7, -32768);
    CustomFrame();
    Require(g_anFlightInput[2] == 9 && g_anFlightInput[0] == -9 && g_anFlightInput[1] == -9 &&
            g_anShipSpeed[0] == (400 << 8), "raw axes 1, 2, 7 and 8 obey their own functions and reversal");
    SetButton(0, 1);
    Require(PolledButtons() == 0, "raw button 1 can be unbound");
    SetButton(0, 0);
    SetButton(31, 1);
    Require(PolledButtons() == 1, "raw button 32 can fire the guns");
    SetButton(31, 0);
    FlushInputEvents();
    SetHat(1, SDL_HAT_UP);
    Require(QueuedPress(0x12) && SdlGetAsyncKeyState(VK_CONTROL), "the second hat can eject with Ctrl");
    SetHat(1, SDL_HAT_RIGHT);
    Require(QueuedPress(0x24), "the second hat can use WC2's jump action");
    SetHat(1, SDL_HAT_CENTERED);
    FlushInputEvents();
    g_bAutopilotSequenceActive = 1;
    g_bSceneEscapeRequested = 0;
    SetButton(8, 1);
    SetButton(8, 0);
    Require(QueuedPress(0x01) && g_bSceneEscapeRequested, "raw button 9 still skips the autopilot fly-by");
    g_bAutopilotSequenceActive = 0;
    g_bSceneEscapeRequested = 0;
    g_176_pfnCurrentInputPump = PollMenuInputDevices;
    SetButton(0, 1);
    Require(PolledButtons() == 1, "raw menu buttons keep their fixed meaning");
    SetButton(0, 0);
    SdlShutdownJoysticks();
    SDL_JoystickClose(pad);
    Require(SDL_JoystickDetachVirtual(device) == 0, "detach the raw flight stick");
    FlushInputEvents();
}

int main(int argc, char **argv)
{
    /* F2 shows view 2, F3 view 1, F4 view 3 and F5 view 5. Every turret is
       view 4, with the cockpit of the view whose key chose it. */
    static const struct {
        int view;
        int cockpit;
        unsigned int key;
    } views[] = {
        {0, 0, 0x3c}, {2, 2, 0x3d}, {1, 1, 0x3e}, {3, 3, 0x3f}, {5, 3, 0x3b},
        {8, 0, 0x3b}, {4, 2, 0x3d}, {4, 1, 0x3e}, {4, 3, 0x3f}
    };
    char guid[33];
    char mapping[512];
    size_t index;
    int device;

    (void)argc;
    (void)argv;
    SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    Require(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) == 0,
            "initialize headless SDL input");
    g_nInputEventPoolInitialized = 1;
    CheckDevicePriority();
    device = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER, 6, 15, 0);
    Require(device >= 0, "attach a virtual controller");
    SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(device), guid, sizeof(guid));
    SDL_snprintf(mapping, sizeof(mapping),
        "%s,Gamepad probe,a:b0,b:b1,x:b2,y:b3,back:b4,guide:b5,start:b6,leftstick:b7,"
        "rightstick:b8,leftshoulder:b9,rightshoulder:b10,dpup:b11,dpdown:b12,dpleft:b13,"
        "dpright:b14,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:+a4,righttrigger:+a5,",
        guid);
    Require(SDL_GameControllerAddMapping(mapping) >= 0 && SDL_IsGameController(device),
            "map the virtual controller with analog triggers");
    pad = SDL_JoystickOpen(device);
    Require(pad != NULL, "open the virtual controller");
    SdlPumpEvents();
    g_8_nActiveInputDeviceWord = 0;
    Require(SdlSetJoystickMode("4button-4axis"), "four-button, four-axis layout");

    /* Polling: in flight RT is button 1, with hysteresis; not elsewhere. */
    g_176_pfnCurrentInputPump = get_player_input;
    Require(PolledButtons() == 0, "released triggers press nothing");
    SetAxis(PAD_RIGHT_TRIGGER, 32767);
    Require(PolledButtons() == 1, "RT fires the guns in flight");
    SetAxis(PAD_RIGHT_TRIGGER, 12000);
    Require(PolledButtons() == 1, "a trigger stays pressed until below a quarter");
    SetAxis(PAD_RIGHT_TRIGGER, 4000);
    Require(PolledButtons() == 0, "a trigger releases below a quarter");
    SetAxis(PAD_RIGHT_TRIGGER, 12000);
    Require(PolledButtons() == 0, "and presses again only past half travel");
    g_176_pfnCurrentInputPump = PollMenuInputDevices;
    SetAxis(PAD_RIGHT_TRIGGER, 32767);
    Require(PolledButtons() == 0, "outside flight the triggers do nothing");
    SetButton(PAD_A, 1);
    Require(PolledButtons() == 1, "A still clicks in menus");
    SetButton(PAD_A, 0);
    SetAxis(PAD_RIGHT_TRIGGER, 0);
    Require(SdlSetJoystickMode("original"), "original layout");
    g_176_pfnCurrentInputPump = get_player_input;
    SetAxis(PAD_LEFT_TRIGGER, 32767);
    Require(PolledButtons() == 2, "in the original layout LT is button 2");
    SetAxis(PAD_LEFT_TRIGGER, 0);
    Require(SdlSetJoystickMode("4button-4axis"), "four-button, four-axis layout again");

    /* Events in flight: LT fires a missile (Enter); RT queues nothing, as A
       does; View steps through F1-F5 without toggling the cockpit. */
    SdlApplyJoystickFlightControls();
    g_nArcadeState = 0;
    FlushInputEvents();
    SetAxis(PAD_LEFT_TRIGGER, 32767);
    Require(QueuedPress(0x1c), "LT fires a missile in flight");
    SetAxis(PAD_LEFT_TRIGGER, 0);
    Require(!AnyQueuedPress(), "releasing LT presses nothing");
    SetAxis(PAD_RIGHT_TRIGGER, 32767);
    SetAxis(PAD_RIGHT_TRIGGER, 0);
    Require(!AnyQueuedPress(), "RT's guns are polled, like A's");
    for (index = 0; index < SDL_arraysize(views); index++) {
        g_nCurrentView = views[index].view;
        g_1_cStoredViewDisplayModeValue = (signed char)views[index].cockpit;
        g_bF1KeyDown = 0;
        SetButton(PAD_BACK, 1);
        SetButton(PAD_BACK, 0);
        Require(QueuedPress(views[index].key) &&
                g_bF1KeyDown == (views[index].key == 0x3b),
                "View steps front, left, right, rear, chase, then front, past turrets too");
    }
    g_nCurrentView = 0;
    g_1_cStoredViewDisplayModeValue = 0;
    Require(!g_bSceneEscapeRequested, "View is not Escape in flight");
    g_bAutopilotSequenceActive = 1;
    SetButton(PAD_BACK, 1);
    SetButton(PAD_BACK, 0);
    Require(QueuedPress(0x01) && g_bSceneEscapeRequested, "View is Escape during the autopilot");
    g_bAutopilotSequenceActive = 0;
    g_bSceneEscapeRequested = 0;
    g_176_pfnCurrentInputPump = PollMenuInputDevices;
    SetButton(PAD_BACK, 1);
    SetButton(PAD_BACK, 0);
    Require(QueuedPress(0x01) && g_bSceneEscapeRequested, "View is Escape outside flight");
    g_bSceneEscapeRequested = 0;
    SetAxis(PAD_LEFT_TRIGGER, 32767);
    Require(!AnyQueuedPress(), "menus take no triggers");
    SetAxis(PAD_LEFT_TRIGGER, 0);
    FlushInputEvents();

    CheckCustomGamepad();
    SdlShutdownJoysticks();
    SDL_JoystickClose(pad);
    Require(SDL_JoystickDetachVirtual(device) == 0, "detach the virtual controller");
    CheckCustomFlightStick();
    SDL_Quit();
    puts("PASS: WC2 controller layouts, automatic device priority, Custom gamepad and raw flight-stick bindings.");
    return 0;
}
