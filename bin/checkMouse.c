/* Mouse buttons in WC2 flight, through the port's SDL event path and
 * player_input. A press chooses the key its buttons stand for -- Space fires
 * the guns, Enter a missile -- and retail released that key only on a frame
 * without mouse motion, so a click made while steering kept the guns firing
 * until the mouse stopped (issue #61). No game assets are needed. */
#include "game.h"

static Uint32 buttons;
static int pointerX = 160;
static int pointerY = 100;

static void Require(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s (key 0x%02x, previous 0x%02x)\n", message,
                (unsigned char)g_cCurrentKey, (unsigned char)g_cPreviousKey);
        exit(1);
    }
}

/* Pushed events leave SDL's own mouse state alone, and the port reads it for
 * the buttons that a button event does not name; report the probe's. */
Uint32 SDL_GetMouseState(int *x, int *y)
{
    if (x != NULL)
        *x = pointerX;
    if (y != NULL)
        *y = pointerY;
    return buttons;
}

/* With no window to map from, the port takes these as 320x200 positions. */
static void Steer(void)
{
    SDL_Event event = {0};

    pointerX = pointerX == 160 ? 200 : 160;
    event.type = SDL_MOUSEMOTION;
    event.motion.state = buttons;
    event.motion.x = pointerX;
    event.motion.y = pointerY;
    Require(SDL_PushEvent(&event) == 1, "queue a mouse motion");
}

static void Button(Uint8 button, int pressed)
{
    SDL_Event event = {0};

    if (pressed)
        buttons |= SDL_BUTTON(button);
    else
        buttons &= ~(Uint32)SDL_BUTTON(button);
    event.type = pressed ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
    event.button.button = button;
    event.button.state = pressed ? SDL_PRESSED : SDL_RELEASED;
    event.button.clicks = 1;
    event.button.x = pointerX;
    event.button.y = pointerY;
    Require(SDL_PushEvent(&event) == 1, "queue a mouse button");
}

static void Key(SDL_Scancode code, int pressed)
{
    SDL_Event event = {0};

    event.type = pressed ? SDL_KEYDOWN : SDL_KEYUP;
    event.key.state = pressed ? SDL_PRESSED : SDL_RELEASED;
    event.key.keysym.scancode = code;
    event.key.keysym.sym = SDL_GetKeyFromScancode(code);
    Require(SDL_PushEvent(&event) == 1, "queue a key");
}

/* One flight frame's input, then the key HandleSpaceFlightControls acts on. */
static int Frame(void)
{
    player_input();
    return (signed char)g_cCurrentKey;
}

/* HandleSpaceFlightControls fires a missile only on a new Enter. */
static int NewMissile(void)
{
    return Frame() == 0x1c && g_cPreviousKey != g_cCurrentKey;
}

int main(int argc, char **argv)
{
    int frame;

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

    /* The guns: fire while the button is down, stop when it comes up. */
    Button(SDL_BUTTON_LEFT, 1);
    Steer();
    Require(Frame() == 0x39, "a click while steering fires the guns");
    Steer();
    Require(Frame() == 0x39, "holding the button keeps them firing");
    Button(SDL_BUTTON_LEFT, 0);
    Steer();
    Require(Frame() != 0x39, "releasing it while steering stops them");
    for (frame = 0; frame < 4; frame++) {
        Steer();
        Require(Frame() != 0x39, "and they stay stopped while the mouse moves");
    }
    Button(SDL_BUTTON_LEFT, 1);
    Button(SDL_BUTTON_LEFT, 0);
    Steer();
    Require(Frame() == 0x39, "a click within one frame fires once");
    Steer();
    Require(Frame() != 0x39, "and stops on the next frame while steering");

    /* A missile: each press of both buttons fires one. */
    Button(SDL_BUTTON_LEFT, 1);
    Button(SDL_BUTTON_RIGHT, 1);
    Steer();
    Require(NewMissile(), "both buttons while steering fire a missile");
    Steer();
    Require(Frame() == 0x1c && g_cPreviousKey == 0x1c, "holding them fires no more");
    Button(SDL_BUTTON_RIGHT, 0);
    Steer();
    Require(Frame() != 0x1c, "releasing one while steering releases Enter");
    Button(SDL_BUTTON_RIGHT, 1);
    Steer();
    Require(NewMissile(), "pressing it again fires the next missile");
    Button(SDL_BUTTON_LEFT, 0);
    Button(SDL_BUTTON_RIGHT, 0);
    Steer();
    Frame();

    /* Keys are released by their own release, as before. */
    Key(SDL_SCANCODE_SPACE, 1);
    Steer();
    Require(Frame() == 0x39, "Space fires the guns");
    for (frame = 0; frame < 4; frame++) {
        Steer();
        Require(Frame() == 0x39, "held Space keeps firing while the mouse moves");
    }
    Key(SDL_SCANCODE_SPACE, 0);
    Steer();
    Require(Frame() != 0x39, "releasing Space stops the guns");

    SDL_Quit();
    puts("PASS: mouse buttons release the guns and missiles they fire, "
         "even while steering.");
    return 0;
}
