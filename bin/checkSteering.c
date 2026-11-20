/* Keyboard steering in WC2 flight, through the port's SDL event path and
 * player_input (issue #67). The Kilrathi Saga build turns the ship one step
 * per key event, so a held key turned only as fast as the system repeated it,
 * and any key's release stopped every axis, even one whose key was still
 * held. WC1's Kilrathi Saga build reads the held keys every frame instead, as
 * the reporter found DOS WC2 to behave. No game assets are needed. */
#include "game.h"

static Uint8 keys[SDL_NUM_SCANCODES];

static void Require(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s (pitch %d, yaw %d, roll %d)\n", message,
                g_anFlightInput[0], g_anFlightInput[1], g_anFlightInput[2]);
        exit(1);
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
    event.key.keysym.mod = keys[SDL_SCANCODE_LSHIFT] ? KMOD_LSHIFT : 0;
    Require(SDL_PushEvent(&event) == 1, "queue a key");
}

static void Frames(int count)
{
    while (count-- > 0)
        player_input();
}

int main(int argc, char **argv)
{
    int frame;
    short yaw;

    (void)argc;
    (void)argv;
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

    /* A held key turns harder every frame, before the system repeats it. */
    Key(SDL_SCANCODE_RIGHT, 1, 0);
    Frames(1);
    Require(g_anFlightInput[1] == 1, "pressing Right starts a turn");
    for (frame = 2; frame <= 9; frame++) {
        Frames(1);
        Require(g_anFlightInput[1] == frame, "holding Right turns harder each frame");
    }
    for (frame = 0; frame < 6; frame++) {
        Key(SDL_SCANCODE_RIGHT, 1, 1);
        Frames(1);
        Require(g_anFlightInput[1] >= 8 && g_anFlightInput[1] <= 9,
                "a held key stays at a full turn as the system repeats it");
    }

    /* Letting go of one of two directions keeps turning the other way. */
    Key(SDL_SCANCODE_DOWN, 1, 0);
    Frames(3);
    Require(g_anFlightInput[0] == -3 && g_anFlightInput[1] >= 8,
            "Down with Right turns down and right");
    Key(SDL_SCANCODE_DOWN, 0, 0);
    Frames(1);
    Require(g_anFlightInput[0] == 0, "releasing Down levels the pitch");
    Require(g_anFlightInput[1] >= 8, "and Right keeps turning without a repeat");
    yaw = g_anFlightInput[1];
    Frames(4);
    Require(g_anFlightInput[1] >= 8 && g_anFlightInput[0] == 0,
            "Right keeps turning while it is held");
    (void)yaw;
    Key(SDL_SCANCODE_RIGHT, 0, 0);
    Frames(1);
    Require(g_anFlightInput[0] == 0 && g_anFlightInput[1] == 0 &&
            g_anFlightInput[2] == 0, "releasing Right stops the turn");
    Frames(3);
    Require(g_anFlightInput[1] == 0, "and the ship stays level");

    /* The other way round, and the roll keys keep their own axis. */
    Key(SDL_SCANCODE_UP, 1, 0);
    Key(SDL_SCANCODE_LEFT, 1, 0);
    Frames(2);
    Require(g_anFlightInput[0] == 2 && g_anFlightInput[1] == -2, "Up with Left");
    Key(SDL_SCANCODE_LEFT, 0, 0);
    Frames(1);
    Require(g_anFlightInput[1] == 0 && g_anFlightInput[0] == 3,
            "releasing Left keeps pitching up");
    Key(SDL_SCANCODE_UP, 0, 0);
    Key(SDL_SCANCODE_PERIOD, 1, 0);
    Frames(2);
    Require(g_anFlightInput[0] == 0 && g_anFlightInput[2] == 2,
            "releasing Up for a roll key rolls and levels the pitch");
    Key(SDL_SCANCODE_PERIOD, 0, 0);
    Frames(1);
    Require(g_anFlightInput[2] == 0, "releasing the roll key stops the roll");

    /* The keypad steers as the arrows do, diagonals included. */
    Key(SDL_SCANCODE_KP_6, 1, 0);
    Frames(3);
    Require(g_anFlightInput[1] == 3, "holding keypad 6 turns right harder each frame");
    Key(SDL_SCANCODE_KP_6, 0, 0);
    Frames(1);
    Require(g_anFlightInput[1] == 0, "releasing keypad 6 stops the turn");
    Key(SDL_SCANCODE_KP_7, 1, 0);
    Frames(2);
    Require(g_anFlightInput[0] == 2 && g_anFlightInput[1] == -2,
            "keypad 7 turns up and left");
    Key(SDL_SCANCODE_KP_7, 0, 0);
    Frames(1);
    Require(g_anFlightInput[0] == 0 && g_anFlightInput[1] == 0,
            "releasing it stops the turn");

    /* Shift still gives the full turn at once. */
    Key(SDL_SCANCODE_LSHIFT, 1, 0);
    Key(SDL_SCANCODE_LEFT, 1, 0);
    Frames(1);
    Require(g_anFlightInput[1] <= -9, "Shift with Left turns fully at once");
    Key(SDL_SCANCODE_LEFT, 0, 0);
    Key(SDL_SCANCODE_LSHIFT, 0, 0);
    Frames(1);
    Require(g_anFlightInput[1] == 0, "and stops with the key");

    SDL_Quit();
    puts("PASS: held keys turn a step every frame, and each axis stops only "
         "when its own keys are up.");
    return 0;
}
