/* The frame rate keys in WC2 flight, through the port's SDL event path: [ and
 * ] (the two keys right of P) lower and raise spaceflight's frame rate cap,
 * which the game's speed follows, as the Kilrathi Saga's Alt+N and Alt+M do,
 * and keep it for the launcher on release. RunSpaceFlight's frame timer
 * follows the cap, so that the cap alone sets the pace. No game assets are
 * needed. */
#include "game.h"

static int failures;
static int savedHalves;
static int saves;

static void Expect(int condition, const char *what)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s (cap %.1f fps, %ld ms; timer %d/60 s)\n", what,
                (double)g_fSpaceFlightFrameRate, (long)g_17_nFramePeriodMillis,
                g_nSpaceFramePeriod);
        failures++;
    }
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

/* The key presses and releases the game was given, of one key or of any. */
static int QueuedKeys(int virtualKey)
{
    InputEvent *event;
    int count = 0;

    for (event = g_pInputEventHead; event != 0; event = event->next) {
        if ((event->type == INPUT_EVENT_KEY_DOWN || event->type == INPUT_EVENT_KEY_UP) &&
            (virtualKey == 0 || event->value == virtualKey))
            count++;
    }
    return count;
}

static void Key(SDL_Scancode code, Uint16 modifiers, int pressed, int repeat)
{
    SDL_Event event;

    memset(&event, 0, sizeof(event));
    event.type = pressed ? SDL_KEYDOWN : SDL_KEYUP;
    event.key.state = pressed ? SDL_PRESSED : SDL_RELEASED;
    event.key.repeat = (Uint8)repeat;
    event.key.keysym.scancode = code;
    event.key.keysym.sym = SDL_GetKeyFromScancode(code);
    event.key.keysym.mod = modifiers;
    if (SDL_PushEvent(&event) != 1) {
        fprintf(stderr, "FAIL: queue a key\n");
        exit(1);
    }
    SdlPumpEvents();
}

static void Tap(SDL_Scancode code, Uint16 modifiers)
{
    Key(code, modifiers, 1, 0);
    Key(code, modifiers, 0, 0);
}

/* The GUI launcher's saver, which rewrites flight_fps in its settings. */
static void SaveRate(int halves)
{
    savedHalves = halves;
    saves++;
}

int main(void)
{
    float rate;
    int index;

    SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "FAIL: SDL: %s\n", SDL_GetError());
        return 1;
    }
    g_nInputEventPoolInitialized = 1;
    g_176_pfnCurrentInputPump = get_player_input;
    g_8_nActiveInputDeviceWord = -1;
    g_1_bSpaceFlightActive = 1;
    g_nArcadeState = 0;
    SdlSetFlightFrameRate(40);
    SdlSetFlightFpsSaver(SaveRate);
    SetSpaceFlightFrameTiming();
    SdlApplyJoystickFlightControls();
    Expect(SdlSpaceflightInputActive(), "flight input is active");
    Expect(g_fSpaceFlightFrameRate == 20.0f && g_17_nFramePeriodMillis == 50 &&
               g_nSpaceFramePeriod == 3,
           "Normal is the 20 fps cap with the port's 3/60 s frame timer");

    Key(SDL_SCANCODE_LEFTBRACKET, 0, 1, 0);
    Expect(g_fSpaceFlightFrameRate == 19.5f && g_17_nFramePeriodMillis == 51 &&
               g_nSpaceFramePeriod == 3,
           "[ lowers the cap by half a frame");
    Expect(g_pszPendingHudMessage != 0 &&
               strcmp(g_pszPendingHudMessage, "Space Flight Max FPS : 19.5") == 0,
           "the HUD shows the new cap");
    Expect(saves == 0, "the rate is not kept while the key is down");
    Key(SDL_SCANCODE_LEFTBRACKET, 0, 0, 0);
    Expect(saves == 1 && savedHalves == 39, "releasing [ keeps 19.5 fps");
    Expect(QueuedKeys(0) == 0, "[ never reaches the game");

    Key(SDL_SCANCODE_RIGHTBRACKET, 0, 1, 0);
    for (index = 0; index < 3; index++)
        Key(SDL_SCANCODE_RIGHTBRACKET, 0, 1, 1);
    Expect(saves == 1, "holding ] keeps nothing yet");
    Key(SDL_SCANCODE_RIGHTBRACKET, 0, 0, 0);
    Expect(g_fSpaceFlightFrameRate == 21.5f && g_17_nFramePeriodMillis == 46,
           "holding ] repeats");
    Expect(g_nSpaceFramePeriod == 1,
           "above 20 fps the frame timer runs at its shortest, under the cap");
    Expect(saves == 2 && savedHalves == 43, "releasing ] keeps 21.5 fps");
    for (index = 0; index < 40; index++)
        Tap(SDL_SCANCODE_RIGHTBRACKET, 0);
    Expect(g_fSpaceFlightFrameRate == 32.0f && g_17_nFramePeriodMillis == 31 &&
               savedHalves == 64,
           "the cap stops at 32 fps");
    for (index = 0; index < 80; index++)
        Tap(SDL_SCANCODE_LEFTBRACKET, 0);
    Expect(g_fSpaceFlightFrameRate == 8.0f && g_17_nFramePeriodMillis == 125 &&
               g_nSpaceFramePeriod == 3 && savedHalves == 16,
           "the cap stops at 8 fps, with the port's frame timer again");
    Expect(QueuedKeys(0) == 0, "neither key reaches the game");

    /* With Ctrl, Alt or Command the keys are the game's, and the Kilrathi
     * Saga's Alt+N and Alt+M no longer move the cap. */
    rate = g_fSpaceFlightFrameRate;
    Tap(SDL_SCANCODE_LEFTBRACKET, KMOD_LCTRL);
    Expect(QueuedKeys(0) == 2 && g_fSpaceFlightFrameRate == rate, "Ctrl+[ reaches the game");
    FlushInputEvents();
    Tap(SDL_SCANCODE_RIGHTBRACKET, KMOD_LALT);
    Expect(QueuedKeys(0) == 2 && g_fSpaceFlightFrameRate == rate, "Alt+] reaches the game");
    FlushInputEvents();
    Tap(SDL_SCANCODE_RIGHTBRACKET, KMOD_LGUI);
    Expect(QueuedKeys(0) == 2 && g_fSpaceFlightFrameRate == rate,
           "Command (or Windows) with ] reaches the game");
    FlushInputEvents();
    Tap(SDL_SCANCODE_M, KMOD_LALT);
    Expect(g_fSpaceFlightFrameRate == rate, "Alt+M leaves the cap alone");
    FlushInputEvents();

    /* Once the flight loop has ended the keys are the game's. */
    g_nArcadeState = 1;
    Tap(SDL_SCANCODE_RIGHTBRACKET, 0);
    Expect(g_fSpaceFlightFrameRate == rate && QueuedKeys(0) == 2,
           "after the flight ] reaches the game");
    FlushInputEvents();
    g_nArcadeState = 0;

    /* A key the player bound to an action sends the action instead. */
    SdlSetKeyBindings("lock-target=%5b\n");
    Tap(SDL_SCANCODE_LEFTBRACKET, 0);
    Expect(g_fSpaceFlightFrameRate == rate && QueuedKeys('L') == 2,
           "a bound [ locks the target");
    FlushInputEvents();
    Tap(SDL_SCANCODE_RIGHTBRACKET, 0);
    Expect(g_fSpaceFlightFrameRate == rate + 0.5f && QueuedKeys(0) == 0,
           "and ] still raises the cap");
    SdlSetKeyBindings("");
    Tap(SDL_SCANCODE_LEFTBRACKET, 0);
    Expect(g_fSpaceFlightFrameRate == rate, "unbinding gives [ back");

    /* The launcher's choices set the cap and the frame timer alike. */
    SdlSetFlightFrameRate(30);
    Expect(g_fSpaceFlightFrameRate == 15.0f && g_nSpaceFramePeriod == 3,
           "Slow, the Kilrathi Saga's own 15 fps");
    SdlSetFlightFrameRate(41);
    Expect(g_fSpaceFlightFrameRate == 20.5f && g_nSpaceFramePeriod == 1, "a Custom 20.5 fps");
    SdlSetFlightFrameRate(40);
    Expect(g_fSpaceFlightFrameRate == 20.0f && g_nSpaceFramePeriod == 3, "Normal again");

    SDL_Quit();
    if (failures != 0)
        return 1;
    puts("PASS: [ and ] move the flight frame rate cap, which the frame timer follows, "
         "and keep it on release.");
    return 0;
}
