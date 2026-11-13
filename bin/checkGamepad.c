/* Mapped controllers' analog triggers and View button through WC2's real SDL
 * event and polling paths, on an SDL virtual controller, after wc1-re's
 * check. No game assets are needed. */
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

    SdlShutdownJoysticks();
    SDL_JoystickClose(pad);
    Require(SDL_JoystickDetachVirtual(device) == 0, "detach the virtual controller");
    SDL_Quit();
    puts("PASS: controller triggers fire in WC2 flight, and View cycles the views.");
    return 0;
}
