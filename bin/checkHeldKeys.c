/* Held flight keys in WC2, through the port's SDL event path and
 * HandleSpaceFlightControls (issue #6). The game acts on one key a frame, the
 * one its latest key event named, while the system repeats only the key
 * pressed last. A held afterburner, gun or throttle key therefore acted on its
 * press, waited out the system's repeat delay, and stopped for good once
 * another key went down, such as an arrow key to steer. Each now acts every
 * frame its key is held; the throttle waits a quarter of a second first, so a
 * tap is still one step, and Ctrl with + or - (the frame skip) steps only on
 * key events. The guns' calls are counted from the port's input trace. No
 * game assets are needed. */
#include "game.h"

static Uint8 keys[SDL_NUM_SCANCODES];
static Uint32 now = 1000;
static FILE *messages;
static char *trace;
static size_t traceSize;
static size_t traceRead;
static int burning;

static void Require(int condition, const char *message)
{
    if (!condition) {
        fprintf(messages, "FAIL: %s (key 0x%02x, throttle %d, frame skip %d)\n", message,
                (unsigned char)g_cCurrentKey, g_anShipSpeed[0] / 256, g_nFrameSkip);
        exit(1);
    }
}

/* The game's clock, which the throttle's delay reads: 50 ms a frame. */
Uint32 SDL_GetTicks(void)
{
    return now;
}

/* Pushed events leave SDL's own mouse state alone; the mouse stays still. */
Uint32 SDL_GetMouseState(int *x, int *y)
{
    if (x != NULL)
        *x = 160;
    if (y != NULL)
        *y = 100;
    return 0;
}

/* A key event as SDL reports it; held keys are what GetAsyncKeyState reads. */
static void Key(SDL_Scancode code, int pressed, int repeat)
{
    SDL_Event event = {0};

    keys[code] = (Uint8)(pressed != 0);
    event.type = pressed ? SDL_KEYDOWN : SDL_KEYUP;
    event.key.state = pressed ? SDL_PRESSED : SDL_RELEASED;
    event.key.repeat = (Uint8)repeat;
    event.key.keysym.scancode = code;
    event.key.keysym.sym = SDL_GetKeyFromScancode(code);
    event.key.keysym.mod = keys[SDL_SCANCODE_LCTRL] ? KMOD_LCTRL : 0;
    Require(SDL_PushEvent(&event) == 1, "queue a key");
}

/* One flight frame, with the system repeating a key if one is given. The
 * afterburner burns down as update_objects_in_space has it do. */
static void Frame(SDL_Scancode repeating)
{
    if (repeating != SDL_SCANCODE_UNKNOWN)
        Key(repeating, 1, 1);
    HandleSpaceFlightControls();
    burning = g_aeSpecialManeuver[0] == SPECIAL_MANEUVER_AFTERBURNER;
    if (burning && --g_asShipAfterburnerTimer[0] <= 0) {
        g_asShipAfterburnerTimer[0] = 0;
        g_aeSpecialManeuver[0] = SPECIAL_MANEUVER_NONE;
    }
    now += 50;
}

/* fire_players_lasers traces each call; count those since the last count. */
static int GunCalls(void)
{
    const char *line;
    int calls = 0;

    fflush(stderr);
    for (line = trace + traceRead; (line = strstr(line, "[fire]")) != NULL; line++)
        calls++;
    traceRead = traceSize;
    return calls;
}

static int Throttle(void)
{
    return g_anShipSpeed[0] / 256;
}

int main(int argc, char **argv)
{
    int frame;
    int steps;

    (void)argc;
    (void)argv;
    /* The input trace goes to a memory stream; the sanitizers write to the
     * descriptor itself, so their reports still show. */
    SDL_setenv("INPUT_TRACE", "1", 1);
    messages = stderr;
    stderr = open_memstream(&trace, &traceSize);
    Require(stderr != NULL, "trace into memory");
    SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
    Require(SDL_Init(SDL_INIT_VIDEO) == 0, "initialize headless SDL video");
    g_nInputEventPoolInitialized = 1;
    g_176_pfnCurrentInputPump = get_player_input;
    g_8_nActiveInputDeviceWord = -1;
    g_cCurrentKey = g_cPreviousKey = (signed char)0x80;
    /* ServiceInputDevices discards what it pumped while the clock has not
     * reached its poll deadline, which a running game soon leaves behind. */
    g_nNextInputPollTick = 0x7fffffff;
    SdlSetKeyboardSnapshot(keys);
    /* Fuel for the afterburner and room for the throttle; the guns' calls
     * stop short of firing, with no gun ready. */
    g_anShipFuel[0] = 1000;
    g_asShipMaximumVelocity[0] = 1000;
    g_aObjectTypeData[(signed char)g_acObjectType[0]].maximumVelocity = 40;
    g_asObjectCounter[0] = 0;
    /* The data starts the engine's damage at -1, which malfunctions at random. */
    memset(g_acPlayerComponentDamage, 0, sizeof(g_acPlayerComponentDamage));
    /* There is no sound data to start the afterburner's effect from. */
    g_6_nPendingAfterburnerSfxFrame = 0x7fffffff;
    g_nFrameSkip = 1;

    /* The guns fire every frame Space is held, before the system repeats it,
     * while an arrow key steers, and after that key comes up. */
    Key(SDL_SCANCODE_SPACE, 1, 0);
    for (frame = 0; frame < 8; frame++) {
        Frame(SDL_SCANCODE_UNKNOWN);
        Require(GunCalls() == 1, "held Space fires the guns every frame");
    }
    Key(SDL_SCANCODE_RIGHT, 1, 0);
    for (frame = 0; frame < 8; frame++) {
        Frame(frame > 0 ? SDL_SCANCODE_RIGHT : SDL_SCANCODE_UNKNOWN);
        Require(GunCalls() == 1, "steering with Right keeps the guns firing");
    }
    Key(SDL_SCANCODE_RIGHT, 0, 0);
    for (frame = 0; frame < 4; frame++) {
        Frame(SDL_SCANCODE_UNKNOWN);
        Require(GunCalls() == 1, "letting go of Right keeps the guns firing");
    }
    for (frame = 0; frame < 4; frame++) {
        Frame(SDL_SCANCODE_SPACE);
        Require(GunCalls() == 1, "a repeat of Space is no second call in a frame");
    }
    Key(SDL_SCANCODE_SPACE, 0, 0);
    Frame(SDL_SCANCODE_UNKNOWN);
    Frame(SDL_SCANCODE_UNKNOWN);
    Require(GunCalls() == 0, "releasing Space stops the guns");

    /* The afterburner burns while Tab is held, steering or firing too. */
    Key(SDL_SCANCODE_TAB, 1, 0);
    for (frame = 0; frame < 30; frame++) {
        if (frame == 10)
            Key(SDL_SCANCODE_LEFT, 1, 0);
        if (frame == 20) {
            Key(SDL_SCANCODE_LEFT, 0, 0);
            Key(SDL_SCANCODE_SPACE, 1, 0);
        }
        Frame(frame > 10 && frame < 20 ? SDL_SCANCODE_LEFT : SDL_SCANCODE_UNKNOWN);
        Require(burning, "held Tab keeps the afterburner lit, steering or firing");
        Require(GunCalls() == (frame >= 20), "and Space fires the guns meanwhile");
    }
    Key(SDL_SCANCODE_SPACE, 0, 0);
    Key(SDL_SCANCODE_TAB, 0, 0);
    for (frame = 0; frame < 12; frame++)
        Frame(SDL_SCANCODE_UNKNOWN);
    Require(!burning && GunCalls() == 0, "releasing Tab puts the afterburner out");

    /* A tap of + is one step of the throttle; held, + waits a quarter of a
     * second, then adds a step every frame, steering or not. */
    Key(SDL_SCANCODE_EQUALS, 1, 0);
    Frame(SDL_SCANCODE_UNKNOWN);
    Frame(SDL_SCANCODE_UNKNOWN);
    Key(SDL_SCANCODE_EQUALS, 0, 0);
    Frame(SDL_SCANCODE_UNKNOWN);
    Require(Throttle() == 1, "a tap of + is one step");
    g_anShipSpeed[0] = 0;
    Key(SDL_SCANCODE_EQUALS, 1, 0);
    for (frame = 0; frame < 5; frame++) {
        Frame(SDL_SCANCODE_UNKNOWN);
        Require(Throttle() == 1, "held +, the throttle waits a quarter of a second");
    }
    for (frame = 0; frame < 5; frame++) {
        Frame(SDL_SCANCODE_UNKNOWN);
        Require(Throttle() == frame + 2, "then + adds a step every frame");
    }
    Key(SDL_SCANCODE_DOWN, 1, 0);
    for (frame = 0; frame < 5; frame++) {
        Frame(frame > 0 ? SDL_SCANCODE_DOWN : SDL_SCANCODE_UNKNOWN);
        Require(Throttle() == frame + 7, "and keeps doing so while Down steers");
    }
    Key(SDL_SCANCODE_DOWN, 0, 0);
    Key(SDL_SCANCODE_EQUALS, 0, 0);
    Frame(SDL_SCANCODE_UNKNOWN);
    steps = Throttle();
    Frame(SDL_SCANCODE_UNKNOWN);
    Require(Throttle() == steps, "releasing + stops the throttle");

    /* The system's repeats still take a step each, never two in a frame. */
    g_anShipSpeed[0] = 0;
    Key(SDL_SCANCODE_EQUALS, 1, 0);
    Frame(SDL_SCANCODE_UNKNOWN);
    for (frame = 0; frame < 9; frame++) {
        Frame(SDL_SCANCODE_EQUALS);
        Require(Throttle() == frame + 2, "a repeat of + is one step a frame");
    }
    Key(SDL_SCANCODE_EQUALS, 0, 0);
    Frame(SDL_SCANCODE_UNKNOWN);

    /* - takes the throttle down the same way. */
    Key(SDL_SCANCODE_MINUS, 1, 0);
    for (frame = 0; frame < 10; frame++)
        Frame(SDL_SCANCODE_UNKNOWN);
    Key(SDL_SCANCODE_MINUS, 0, 0);
    Frame(SDL_SCANCODE_UNKNOWN);
    Require(Throttle() == 10 - 6, "held -, the throttle comes down the same way");

    /* Ctrl with + steps the frame skip on its key events alone. */
    g_anShipSpeed[0] = 0;
    Key(SDL_SCANCODE_LCTRL, 1, 0);
    Frame(SDL_SCANCODE_UNKNOWN);
    Key(SDL_SCANCODE_EQUALS, 1, 0);
    for (frame = 0; frame < 10; frame++)
        Frame(SDL_SCANCODE_UNKNOWN);
    Require(g_nFrameSkip == 2 && Throttle() == 0, "held Ctrl and + step the frame skip once");
    Frame(SDL_SCANCODE_EQUALS);
    Require(g_nFrameSkip == 3, "and once more on the system's repeat");
    Key(SDL_SCANCODE_EQUALS, 0, 0);
    Key(SDL_SCANCODE_LCTRL, 0, 0);
    Frame(SDL_SCANCODE_UNKNOWN);

    SDL_Quit();
    fclose(stderr);
    stderr = messages;
    free(trace);
    fputs("PASS: held afterburner, gun and throttle keys act every frame, steering or "
          "firing too; a tap of + or - is one step, and Ctrl's frame skip steps per key "
          "event.\n", stdout);
    return 0;
}
