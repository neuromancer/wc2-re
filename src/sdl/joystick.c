#include "game.h"
#include "joystick_calibration.h"
#include "keymap_actions.h"
#include "pad_bindings.h"

typedef struct SdlJoystickDevice {
    SDL_GameController *controller;
    SDL_Joystick *joystick;
    SDL_JoystickID instanceId;
    Uint8 hatState;
    /* A raw joystick's second hat, which only the Custom mode reads. */
    Uint8 secondHatState;
    /* Mapped controllers' analog triggers, as pressed buttons: bit 0 LT,
       bit 1 RT. Polling and events latch them separately. */
    Uint8 triggersHeld;
    Uint8 triggersPressed;
    int rumbleSupport; /* -1 unavailable, 0 untested, 1 available */
} SdlJoystickDevice;

typedef enum SdlJoystickMode {
    SDL_JOYSTICK_ORIGINAL,
    SDL_JOYSTICK_FOUR_BUTTON_TWO_AXIS,
    SDL_JOYSTICK_FOUR_BUTTON_FOUR_AXIS,
    SDL_JOYSTICK_CUSTOM
} SdlJoystickMode;

typedef enum SdlJoystickAxesMode {
    SDL_JOYSTICK_AXES_TWIN_STICK_ROLL,
    SDL_JOYSTICK_AXES_TWIN_STICK_YAW,
    SDL_JOYSTICK_AXES_HOTAS_YAW,
    SDL_JOYSTICK_AXES_HOTAS_ROLL,
    SDL_JOYSTICK_AXES_LINEAR_THROTTLE,
    SDL_JOYSTICK_AXES_RUDDER_YAW,
    SDL_JOYSTICK_AXES_RUDDER_ROLL
} SdlJoystickAxesMode;

typedef enum SdlJoystickButton {
    SDL_JOYSTICK_BUTTON_A,
    SDL_JOYSTICK_BUTTON_B,
    SDL_JOYSTICK_BUTTON_X,
    SDL_JOYSTICK_BUTTON_Y,
    SDL_JOYSTICK_BUTTON_LEFT_SHOULDER,
    SDL_JOYSTICK_BUTTON_RIGHT_SHOULDER,
    SDL_JOYSTICK_BUTTON_LEFT_TRIGGER,
    SDL_JOYSTICK_BUTTON_RIGHT_TRIGGER,
    SDL_JOYSTICK_BUTTON_BACK = 8,
    SDL_JOYSTICK_BUTTON_START,
    SDL_JOYSTICK_BUTTON_LEFT_STICK,
    SDL_JOYSTICK_BUTTON_RIGHT_STICK,
    SDL_JOYSTICK_BUTTON_DPAD_UP,
    SDL_JOYSTICK_BUTTON_DPAD_DOWN,
    SDL_JOYSTICK_BUTTON_DPAD_LEFT,
    SDL_JOYSTICK_BUTTON_DPAD_RIGHT
} SdlJoystickButton;

static SdlJoystickDevice g_aSdlJoystickDevices[2];
static int g_bSdlJoystickDebug;
static int g_bSdlJoystickInputStarted;
static int g_bSdlJoystickRumbleEnabled;
static int g_bSdlJoystickSpaceflightActive;
static int g_bSdlTwoAxisModifierActive;
static int g_nSdlCommunicationMenuSelection;
static int g_nSdlRumblingJoystick = -1;
static SDL_JoystickID g_nSdlPreferredJoystickInstance = -1;
static SDL_JoystickGUID g_stSdlPreferredJoystickGuid;
static char *g_pszSdlPreferredJoystickPath;
static int g_bSdlPreferredJoystickGuidUnique;
static Uint16 g_wSdlRumbleLow;
static Uint16 g_wSdlRumbleHigh;
static Uint32 g_dwSdlRumbleDeadline;
static SdlJoystickMode g_eSdlJoystickMode;
static int g_bSdlSteamDeckPreset;
static int g_nSdlJoystickSource;
static SdlJoystickAxesMode g_eSdlJoystickAxesMode;

static int g_nSdlJoystickDeadZone = SDL_JOYSTICK_DEAD_ZONE_ORIGINAL;

/* The Custom mode's bindings: a flight key's action, a special function or
 * an axis function for each control of pad_bindings.h. */
typedef enum SdlPadFunctionKind {
    SDL_PAD_UNBOUND,
    SDL_PAD_KEY,
    SDL_PAD_CYCLE_VIEWS,
    SDL_PAD_ROLL_AND_SPEED,
    SDL_PAD_AXIS
} SdlPadFunctionKind;

/* g_apszSdlPadAxisFunctions' order. */
typedef enum SdlPadAxisFunction {
    SDL_PAD_PITCH,
    SDL_PAD_YAW,
    SDL_PAD_ROLL,
    SDL_PAD_SPEED,
    SDL_PAD_THROTTLE
} SdlPadAxisFunction;

typedef struct SdlPadBinding {
    SdlPadFunctionKind kind;
    /* The action's index in g_aSdlPadActions, or the SdlPadAxisFunction. */
    int value;
    /* 1, or -1 for a reversed axis. */
    int sign;
} SdlPadBinding;

/* The flight keys' actions (keymap_actions.h), with the key and modifier
 * that the game's own handler reads. */
typedef struct SdlPadAction {
    const char *id;
    SDL_Scancode key;
    Uint16 modifier;
} SdlPadAction;

#define SDL_PAD_ACTION(id, key, modifier, ...) {id, key, modifier},
static const SdlPadAction g_aSdlPadActions[] = {
    SDL_KEYMAP_ACTION_LIST(SDL_PAD_ACTION)
};
#undef SDL_PAD_ACTION

#define SDL_PAD_ACTIONS \
    ((int)(sizeof(g_aSdlPadActions) / sizeof(g_aSdlPadActions[0])))

/* A button held on a speed key repeats it, as a held key repeats, with the
 * delay and rate of the mouse buttons' keys (src/sdl/events.c). */
#define SDL_PAD_KEY_DELAY 250
#define SDL_PAD_KEY_REPEAT 33

static SdlPadBinding g_aSdlPadBindings[SDL_PAD_CONTROLS];
/* The guns fire and the afterburner burns while their buttons are held, as
 * the joystick's first button and the WCAT layouts' X do. */
static int g_nSdlPadFireGuns = -1;
static int g_nSdlPadAfterburner = -1;
/* Frames for which a pressed button's key stays held even if it was let go
 * at once, so that the game reads the key's modifier with the press. */
static Uint8 g_abSdlPadLatches[SDL_PAD_CONTROLS];
static Uint32 g_adwSdlPadRepeats[SDL_PAD_CONTROLS];
/* The left stick's steps, which the bindings take over from the game. */
static int g_nSdlPadStickX;
static int g_nSdlPadStickY;
/* The pitch, yaw and roll that the axes gave in the last frame. */
static int g_anSdlPadTurns[3];

static void SdlCloseJoystick(SdlJoystickDevice *device);
static void SdlRefreshJoysticks(void);

int SdlSetJoystickDeadZone(int percent)
{
    if (percent < 0 || percent > SDL_JOYSTICK_DEAD_ZONE_MAXIMUM)
        return 0;
    g_nSdlJoystickDeadZone = percent;
    return 1;
}

/* LoadJoystickCalibrationFile's dead zone arguments. */
short SdlGetJoystickDeadZoneSteps(void)
{
    return (short)SdlJoystickDeadZoneSteps(g_nSdlJoystickDeadZone);
}

static void SdlRefreshJoysticks(void);
static int SdlPadBindingsActive(void);

/* A missing joystick samples as 0xffff, a position, not as -1. A joystick
 * chosen in the launcher takes slot 0 (SdlRefreshJoysticks); a calibration
 * for the other slot must not make that one the stick instead. */
int SdlAcceptJoystickCalibration(short device,
                                 unsigned short minimumX, unsigned short minimumY,
                                 unsigned short maximumX, unsigned short maximumY,
                                 unsigned short centreX, unsigned short centreY)
{
    if (device < 0 || device >= 2 ||
        (SdlHasPreferredJoystick() && device != 0))
        return 0;
    SdlRefreshJoysticks();
    return g_aSdlJoystickDevices[device].joystick != 0 &&
        SdlJoystickAxisCalibrationValid(minimumX, centreX, maximumX) &&
        SdlJoystickAxisCalibrationValid(minimumY, centreY, maximumY);
}

void SdlRestoreJoystickCalibration(unsigned short minimumX, unsigned short minimumY,
                                   unsigned short maximumX, unsigned short maximumY,
                                   short horizontalRange, short verticalRange)
{
    SdlJoystickAxis axis;

    SdlCalibrateJoystickAxis(&axis, minimumX, g_1_nJoystickCentreXNumber,
                             maximumX, horizontalRange);
    g_12_nControllerLeftScaleValue = axis.lowScale;
    g_1_nJsRightScaleWord = axis.highScale;
    g_12_nJoystickMinimumXNumber = axis.minimum;
    g_4_nJoystickMaximumXValue = axis.maximum;
    SdlCalibrateJoystickAxis(&axis, minimumY, g_2_nJoystickCentreY,
                             maximumY, verticalRange);
    g_59_nJoystickUpScaleWord = axis.lowScale;
    g_12_nJoystickDownScaleWord = axis.highScale;
    g_2_nJsMinimumY = axis.minimum;
    g_nJoystickMaximumY = axis.maximum;
}

void SdlShapeJoystickDeadZone(InputDeviceSample *sample)
{
    SdlJoystickAxis axis;
    int custom = SdlPadBindingsActive();

    axis.centre = g_1_nJoystickCentreXNumber;
    axis.lowScale = g_12_nControllerLeftScaleValue;
    axis.highScale = g_1_nJsRightScaleWord;
    axis.minimum = g_12_nJoystickMinimumXNumber;
    axis.maximum = g_4_nJoystickMaximumXValue;
    sample->x = SdlShapeJoystickPosition(&axis, sample->x,
                                         g_nSdlJoystickDeadZone);
    if (custom) {
        /* Use the existing calibration hook to take the normalized stick
         * for Custom bindings. Retail's subsequent normalization sees the
         * centre, so it cannot also steer with these axes. */
        g_nSdlPadStickX = SdlJoystickAxisStep(&axis, sample->x, g_nJoystickHorizDeadZone);
        sample->x = axis.centre;
    }
    axis.centre = g_2_nJoystickCentreY;
    axis.lowScale = g_59_nJoystickUpScaleWord;
    axis.highScale = g_12_nJoystickDownScaleWord;
    axis.minimum = g_2_nJsMinimumY;
    axis.maximum = g_nJoystickMaximumY;
    sample->y = SdlShapeJoystickPosition(&axis, sample->y,
                                         g_nSdlJoystickDeadZone);
    if (custom) {
        g_nSdlPadStickY = SdlJoystickAxisStep(&axis, sample->y, g_nControllerVerticalDeadZoneWord);
        sample->y = axis.centre;
    }
}

int SdlSetJoystickMode(const char *name)
{
    int wasRaw = !g_bSdlSteamDeckPreset && g_eSdlJoystickMode == SDL_JOYSTICK_CUSTOM &&
        g_nSdlJoystickSource == SDL_PAD_SOURCE_JOYSTICK;
    int slot;

    if (strcmp(name, "original") == 0)
        g_eSdlJoystickMode = SDL_JOYSTICK_ORIGINAL;
    else if (strcmp(name, "4button-2axis") == 0)
        g_eSdlJoystickMode = SDL_JOYSTICK_FOUR_BUTTON_TWO_AXIS;
    else if (strcmp(name, "4button-4axis") == 0)
        g_eSdlJoystickMode = SDL_JOYSTICK_FOUR_BUTTON_FOUR_AXIS;
    else if (strcmp(name, "custom") == 0 || strcmp(name, "steam-deck") == 0)
        g_eSdlJoystickMode = SDL_JOYSTICK_CUSTOM;
    else
        return 0;
    g_bSdlSteamDeckPreset = strcmp(name, "steam-deck") == 0;
    if (wasRaw != (!g_bSdlSteamDeckPreset && g_eSdlJoystickMode == SDL_JOYSTICK_CUSTOM &&
                    g_nSdlJoystickSource == SDL_PAD_SOURCE_JOYSTICK)) {
        for (slot = 0; slot < 2; slot++)
            SdlCloseJoystick(&g_aSdlJoystickDevices[slot]);
        if (g_bSdlJoystickInputStarted)
            SdlRefreshJoysticks();
    }
    return 1;
}

int SdlSetJoystickSource(int source)
{
    int wasRaw = g_nSdlJoystickSource == SDL_PAD_SOURCE_JOYSTICK;
    int slot;

    if (source < SDL_PAD_SOURCE_AUTO || source > SDL_PAD_SOURCE_JOYSTICK)
        return 0;
    g_nSdlJoystickSource = source;
    if (!g_bSdlSteamDeckPreset && g_eSdlJoystickMode == SDL_JOYSTICK_CUSTOM &&
        wasRaw != (source == SDL_PAD_SOURCE_JOYSTICK)) {
        for (slot = 0; slot < 2; slot++)
            SdlCloseJoystick(&g_aSdlJoystickDevices[slot]);
        if (g_bSdlJoystickInputStarted)
            SdlRefreshJoysticks();
    }
    return 1;
}

int SdlSetJoystickAxesMode(const char *name)
{
    if (strcmp(name, "twin-stick-roll") == 0)
        g_eSdlJoystickAxesMode =
            SDL_JOYSTICK_AXES_TWIN_STICK_ROLL;
    else if (strcmp(name, "twin-stick-yaw") == 0)
        g_eSdlJoystickAxesMode =
            SDL_JOYSTICK_AXES_TWIN_STICK_YAW;
    else if (strcmp(name, "hotas-yaw") == 0)
        g_eSdlJoystickAxesMode = SDL_JOYSTICK_AXES_HOTAS_YAW;
    else if (strcmp(name, "hotas-roll") == 0)
        g_eSdlJoystickAxesMode = SDL_JOYSTICK_AXES_HOTAS_ROLL;
    else if (strcmp(name, "linear-throttle") == 0)
        g_eSdlJoystickAxesMode =
            SDL_JOYSTICK_AXES_LINEAR_THROTTLE;
    else if (strcmp(name, "rudder-yaw") == 0)
        g_eSdlJoystickAxesMode = SDL_JOYSTICK_AXES_RUDDER_YAW;
    else if (strcmp(name, "rudder-roll") == 0)
        g_eSdlJoystickAxesMode = SDL_JOYSTICK_AXES_RUDDER_ROLL;
    else
        return 0;
    return 1;
}

static int SdlPadNameIs(const char *name, size_t length, const char *expected)
{
    return strlen(expected) == length && memcmp(name, expected, length) == 0;
}

static int SdlFindPadAction(const char *name, size_t length)
{
    int action;

    for (action = 0; action < SDL_PAD_ACTIONS; action++) {
        if (SdlPadNameIs(name, length, g_aSdlPadActions[action].id))
            return action;
    }
    return -1;
}

/* Reads one function for a control; 0 when the control cannot take it. */
static int SdlParsePadFunction(int control, const char *name, size_t length,
                               SdlPadBinding *binding)
{
    int index;

    binding->kind = SDL_PAD_UNBOUND;
    binding->value = 0;
    binding->sign = 1;
    if (length == 0)
        return 1;
    if (SdlPadControlTakesAxis(control)) {
        const char *axis = name;
        size_t axisLength = length;
        int sign = 1;

        if (*axis == '-') {
            sign = -1;
            axis++;
            axisLength--;
        }
        for (index = 0; index < SDL_PAD_AXIS_FUNCTIONS; index++) {
            if (SdlPadNameIs(axis, axisLength, g_apszSdlPadAxisFunctions[index])) {
                binding->kind = SDL_PAD_AXIS;
                binding->value = index;
                binding->sign = sign;
                return 1;
            }
        }
    }
    if (!SdlPadControlTakesButton(control))
        return 0;
    if (SdlPadNameIs(name, length, "cycle-views")) {
        binding->kind = SDL_PAD_CYCLE_VIEWS;
        return 1;
    }
    if (SdlPadNameIs(name, length, "roll-and-speed")) {
        binding->kind = SDL_PAD_ROLL_AND_SPEED;
        return 1;
    }
    index = SdlFindPadAction(name, length);
    if (index < 0)
        return 0;
    binding->kind = SDL_PAD_KEY;
    binding->value = index;
    return 1;
}

/* The Custom mode's "control=function" lines over its defaults for the axis
 * layout, so the axis layout must be set first. */
void SdlSetPadBindings(const char *text)
{
    const char *line = g_bSdlSteamDeckPreset ? NULL : text;
    int control;

    for (control = 0; control < SDL_PAD_CONTROLS; control++) {
        const char *name = (g_bSdlSteamDeckPreset ||
            (g_eSdlJoystickMode == SDL_JOYSTICK_CUSTOM && g_nSdlJoystickSource == SDL_PAD_SOURCE_STEAM_DECK &&
             control < SDL_PAD_GAMEPAD_CONTROLS))
            ? SdlSteamDeckPadFunction(control)
            : SdlDefaultPadFunction(control, g_eSdlJoystickAxesMode);

        SdlParsePadFunction(control, name, strlen(name), &g_aSdlPadBindings[control]);
    }
    while (line != NULL && *line != '\0') {
        const char *end = strchr(line, '\n');
        size_t length = end != NULL ? (size_t)(end - line) : strlen(line);
        const char *separator = memchr(line, '=', length);

        if (separator != NULL) {
            size_t nameLength = (size_t)(separator - line);
            size_t valueLength = length - nameLength - 1;
            SdlPadBinding binding;

            for (control = 0; control < SDL_PAD_CONTROLS; control++) {
                if (SdlPadNameIs(line, nameLength, g_apszSdlPadControls[control]))
                    break;
            }
            if (control < SDL_PAD_CONTROLS &&
                SdlParsePadFunction(control, separator + 1, valueLength, &binding))
                g_aSdlPadBindings[control] = binding;
            else
                fprintf(stderr, "Ignoring the game controller binding \"%.*s\".\n",
                        (int)length, line);
        }
        line = end != NULL ? end + 1 : NULL;
    }
    g_nSdlPadFireGuns = SdlFindPadAction("fire-guns", strlen("fire-guns"));
    g_nSdlPadAfterburner = SdlFindPadAction("afterburner", strlen("afterburner"));
}

void SdlEnableJoystickDebug(void)
{
    g_bSdlJoystickDebug = 1;
}

void SdlSetPreferredJoystick(SDL_JoystickID instanceId,
                                 const char *guid, const char *path,
                                 int guidUnique)
{
    g_nSdlPreferredJoystickInstance = instanceId;
    g_stSdlPreferredJoystickGuid = SDL_JoystickGetGUIDFromString(guid);
    SDL_free(g_pszSdlPreferredJoystickPath);
    g_pszSdlPreferredJoystickPath = path[0] != '\0' ? SDL_strdup(path) : 0;
    g_bSdlPreferredJoystickGuidUnique = guidUnique;
}

int SdlHasPreferredJoystick(void)
{
    return g_nSdlPreferredJoystickInstance >= 0;
}

void SdlSetJoystickRumbleEnabled(int enabled)
{
    g_bSdlJoystickRumbleEnabled = enabled;
}

static void SdlStopJoystickRumble(void)
{
    SdlJoystickDevice *device;

    if (g_nSdlRumblingJoystick < 0 ||
        g_nSdlRumblingJoystick >= 2)
        return;
    device = &g_aSdlJoystickDevices[g_nSdlRumblingJoystick];
    if (device->joystick != 0 && device->rumbleSupport > 0)
        SDL_JoystickRumble(device->joystick, 0, 0, 0);
    g_nSdlRumblingJoystick = -1;
}

static void SdlSendJoystickRumble(int slot, Uint16 low, Uint16 high)
{
    SdlJoystickDevice *device;

    device = &g_aSdlJoystickDevices[slot];
    if (device->joystick == 0 || device->rumbleSupport < 0)
        return;
    /* Brief renewals expire safely if a pause or modal dialog stops updates. */
    if (SDL_JoystickRumble(device->joystick, low, high, 100) != 0) {
        device->rumbleSupport = -1;
        if (g_bSdlJoystickDebug) {
            printf("SDL joystick slot %d does not support rumble: %s\n",
                   slot, SDL_GetError());
            fflush(stdout);
        }
        SDL_ClearError();
        return;
    }
    if (device->rumbleSupport == 0 && g_bSdlJoystickDebug) {
        printf("SDL joystick slot %d rumble enabled\n", slot);
        fflush(stdout);
    }
    device->rumbleSupport = 1;
    g_nSdlRumblingJoystick = slot;
}

static void SdlUpdateJoystickRumble(void)
{
    Uint16 low;
    Uint16 high;
    Uint32 now;
    int activeSlot;

    if (!g_bSdlJoystickRumbleEnabled)
        return;
    activeSlot = (int)g_8_nActiveInputDeviceWord;
    if (!g_bSdlJoystickSpaceflightActive || activeSlot < 0 ||
        activeSlot >= 2 ||
        g_aSdlJoystickDevices[activeSlot].joystick == 0) {
        SdlStopJoystickRumble();
        return;
    }
    if (g_nSdlRumblingJoystick != -1 &&
        g_nSdlRumblingJoystick != activeSlot)
        SdlStopJoystickRumble();

    low = 0;
    high = 0;
    if (g_anShipFuel[0] > 0 &&
        g_aeSpecialManeuver[0] ==
            SPECIAL_MANEUVER_AFTERBURNER &&
        g_asShipAfterburnerTimer[0] > 0) {
        low = 0x6000;
        high = 0x3000;
    }

    now = SDL_GetTicks();
    if (g_dwSdlRumbleDeadline != 0 &&
        !SDL_TICKS_PASSED(now, g_dwSdlRumbleDeadline)) {
        if (low < g_wSdlRumbleLow)
            low = g_wSdlRumbleLow;
        if (high < g_wSdlRumbleHigh)
            high = g_wSdlRumbleHigh;
    } else {
        g_wSdlRumbleLow = 0;
        g_wSdlRumbleHigh = 0;
        g_dwSdlRumbleDeadline = 0;
    }

    if (low == 0 && high == 0) {
        SdlStopJoystickRumble();
        return;
    }
    SdlSendJoystickRumble(activeSlot, low, high);
}

static void SdlQueueJoystickRumble(Uint16 low, Uint16 high,
                                      Uint32 duration)
{
    Uint32 deadline;
    Uint32 now;

    if (!g_bSdlJoystickRumbleEnabled)
        return;

    now = SDL_GetTicks();
    deadline = now + duration;
    if (deadline == 0)
        deadline = 1;
    if (g_dwSdlRumbleDeadline == 0 ||
        SDL_TICKS_PASSED(now, g_dwSdlRumbleDeadline)) {
        g_wSdlRumbleLow = low;
        g_wSdlRumbleHigh = high;
        g_dwSdlRumbleDeadline = deadline;
    } else {
        if (g_wSdlRumbleLow < low)
            g_wSdlRumbleLow = low;
        if (g_wSdlRumbleHigh < high)
            g_wSdlRumbleHigh = high;
        if (SDL_TICKS_PASSED(deadline, g_dwSdlRumbleDeadline))
            g_dwSdlRumbleDeadline = deadline;
    }
    SdlUpdateJoystickRumble();
}

void SdlQueueJoystickWeaponRumble(int weaponType)
{
    Uint16 low;
    Uint16 high;
    Uint32 duration;

    /* Lasers fire continuously, so feedback here quickly becomes noise. */
    switch (weaponType) {
    case OBJECT_DATA_LASER_CANNON:
        return;
    case OBJECT_DATA_NEUTRON_GUN:
        low = 0x6000;
        high = 0x7800;
        duration = 75;
        break;
    case OBJECT_DATA_MASS_DRIVER:
    case OBJECT_DATA_TURRET_GUN:
        low = 0x7800;
        high = 0x6000;
        duration = 90;
        break;
    case OBJECT_DATA_PARTICLE_CANNON:
        low = 0x6800;
        high = 0x8800;
        duration = 90;
        break;
    case OBJECT_DATA_DART_DF:
        low = 0x9000;
        high = 0x6000;
        duration = 150;
        break;
    case OBJECT_DATA_JAVELIN_HS:
        low = 0x8800;
        high = 0x5800;
        duration = 170;
        break;
    case OBJECT_DATA_PILUM_FF:
        low = 0x8000;
        high = 0x6800;
        duration = 180;
        break;
    case OBJECT_DATA_SPICULUM_IR:
        low = 0x9000;
        high = 0x5800;
        duration = 170;
        break;
    case OBJECT_DATA_TORPEDO:
        low = 0xa000;
        high = 0x6800;
        duration = 200;
        break;
    default:
        return;
    }
    SdlQueueJoystickRumble(low, high, duration);
}

void SdlQueueJoystickDamageRumble(int damage)
{
    if (damage <= 0)
        return;
    if (damage < 16) {
        SdlQueueJoystickRumble(0x5800, 0x7000, 90);
    } else if (damage < 64) {
        SdlQueueJoystickRumble(0x7800, 0x8000, 130);
    } else {
        SdlQueueJoystickRumble(0xa000, 0x9000, 180);
    }
}

void SdlQueueJoystickCollisionRumble(int collisionSpeed)
{
    if (collisionSpeed <= 0)
        return;
    if (collisionSpeed < 10) {
        SdlQueueJoystickRumble(0x6800, 0x5000, 110);
    } else if (collisionSpeed < 30) {
        SdlQueueJoystickRumble(0x8800, 0x6800, 160);
    } else {
        SdlQueueJoystickRumble(0xb000, 0x7800, 220);
    }
}

void SdlLogJoystickEvent(const SDL_Event *event)
{
    const char *name;

    if (!g_bSdlJoystickDebug)
        return;

    switch (event->type) {
    case SDL_JOYAXISMOTION:
        printf("SDL joystick %d axis %u = %d\n",
               (int)event->jaxis.which, (unsigned int)event->jaxis.axis,
               (int)event->jaxis.value);
        break;
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
        printf("SDL joystick %d button %u %s\n",
               (int)event->jbutton.which,
               (unsigned int)event->jbutton.button,
               event->type == SDL_JOYBUTTONDOWN ? "down" : "up");
        break;
    case SDL_JOYHATMOTION:
        printf("SDL joystick %d hat %u = 0x%02x\n",
               (int)event->jhat.which, (unsigned int)event->jhat.hat,
               (unsigned int)event->jhat.value);
        break;
    case SDL_CONTROLLERAXISMOTION:
        name = SDL_GameControllerGetStringForAxis(
            (SDL_GameControllerAxis)event->caxis.axis);
        printf("SDL controller %d axis %s (%u) = %d\n",
               (int)event->caxis.which, name != 0 ? name : "unknown",
               (unsigned int)event->caxis.axis,
               (int)event->caxis.value);
        break;
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
        name = SDL_GameControllerGetStringForButton(
            (SDL_GameControllerButton)event->cbutton.button);
        printf("SDL controller %d button %s (%u) %s\n",
               (int)event->cbutton.which,
               name != 0 ? name : "unknown",
               (unsigned int)event->cbutton.button,
               event->type == SDL_CONTROLLERBUTTONDOWN ? "down" : "up");
        break;
    default:
        return;
    }
    fflush(stdout);
}

static void SdlCloseJoystick(SdlJoystickDevice *device)
{
    if (device->joystick != 0 && device->rumbleSupport > 0)
        SDL_JoystickRumble(device->joystick, 0, 0, 0);
    if (g_nSdlRumblingJoystick >= 0 &&
        device == &g_aSdlJoystickDevices[
                      g_nSdlRumblingJoystick])
        g_nSdlRumblingJoystick = -1;
    if (device->controller != 0)
        SDL_GameControllerClose(device->controller);
    else if (device->joystick != 0)
        SDL_JoystickClose(device->joystick);
    device->controller = 0;
    device->joystick = 0;
    device->instanceId = -1;
    device->hatState = SDL_HAT_CENTERED;
    device->secondHatState = SDL_HAT_CENTERED;
    device->triggersHeld = 0;
    device->triggersPressed = 0;
    device->rumbleSupport = 0;
}

static int SdlFindJoystick(SDL_JoystickID instanceId)
{
    int slot;

    slot = 0;
    while (slot < 2) {
        if (g_aSdlJoystickDevices[slot].joystick != 0 &&
            g_aSdlJoystickDevices[slot].instanceId == instanceId)
            return slot;
        slot++;
    }
    return -1;
}

static int SdlFindFreeJoystickSlot(void)
{
    int slot;

    slot = 0;
    while (slot < 2) {
        if (g_aSdlJoystickDevices[slot].joystick == 0)
            return slot;
        slot++;
    }
    return -1;
}

static void SdlOpenJoystick(int deviceIndex)
{
    SDL_GameController *controller;
    SDL_Joystick *joystick;
    SDL_JoystickID instanceId;
    SdlJoystickDevice *device;
    int slot;

    instanceId = SDL_JoystickGetDeviceInstanceID(deviceIndex);
    if (instanceId < 0 || SdlFindJoystick(instanceId) != -1)
        return;
    slot = SdlFindFreeJoystickSlot();
    if (slot == -1)
        return;

    controller = 0;
    joystick = 0;
    if (SDL_IsGameController(deviceIndex) &&
        !(!g_bSdlSteamDeckPreset && g_eSdlJoystickMode == SDL_JOYSTICK_CUSTOM &&
          g_nSdlJoystickSource == SDL_PAD_SOURCE_JOYSTICK)) {
        controller = SDL_GameControllerOpen(deviceIndex);
        if (controller != 0)
            joystick = SDL_GameControllerGetJoystick(controller);
    }
    if (joystick == 0) {
        if (controller != 0)
            SDL_GameControllerClose(controller);
        controller = 0;
        joystick = SDL_JoystickOpen(deviceIndex);
    }
    if (joystick == 0)
        return;

    device = &g_aSdlJoystickDevices[slot];
    device->controller = controller;
    device->joystick = joystick;
    device->instanceId = SDL_JoystickInstanceID(joystick);
    device->hatState = SDL_HAT_CENTERED;
    device->secondHatState = SDL_HAT_CENTERED;
    device->triggersHeld = 0;
    device->triggersPressed = 0;
    device->rumbleSupport = 0;
#if SDL_VERSION_ATLEAST(2, 0, 18)
    if (g_bSdlJoystickRumbleEnabled) {
        if (SDL_JoystickHasRumble(joystick) == SDL_FALSE)
            device->rumbleSupport = -1;
        printf("SDL joystick slot %d %s rumble support\n", slot,
               device->rumbleSupport < 0 ? "does not expose" : "exposes");
        fflush(stdout);
    }
#endif
    if (g_bSdlJoystickDebug) {
        const char *name = SDL_JoystickName(joystick);

        printf("SDL joystick slot %d opened: %s, instance %d, %s, "
               "SDL type %d (preference %d), %d axes, %d buttons, %d hats\n",
               slot, name != 0 ? name : "unknown",
               (int)device->instanceId,
               controller != 0 ? "mapped controller" : "raw joystick",
               (int)SDL_JoystickGetType(joystick), SdlJoystickPreference(deviceIndex),
               SDL_JoystickNumAxes(joystick),
               SDL_JoystickNumButtons(joystick),
               SDL_JoystickNumHats(joystick));
        fflush(stdout);
    }
}

static int SdlFindPreferredJoystick(int deviceCount)
{
    SDL_JoystickGUID guid;
    const char *path;
    int deviceIndex;
    int candidate;
    int matches;

    if (!SdlHasPreferredJoystick())
        return -1;
    /* An attached device keeps its instance ID even if other devices leave. */
    for (deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++) {
        if (SDL_JoystickGetDeviceInstanceID(deviceIndex) ==
            g_nSdlPreferredJoystickInstance)
            return deviceIndex;
    }

    candidate = -1;
    matches = 0;
    for (deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++) {
        guid = SDL_JoystickGetDeviceGUID(deviceIndex);
        if (memcmp(&guid, &g_stSdlPreferredJoystickGuid, sizeof(guid)) != 0)
            continue;
        path = 0;
#if SDL_VERSION_ATLEAST(2, 24, 0)
        path = SDL_JoystickPathForIndex(deviceIndex);
#endif
        if (g_pszSdlPreferredJoystickPath != 0 && path != 0 &&
            strcmp(path, g_pszSdlPreferredJoystickPath) == 0)
            return deviceIndex;
        candidate = deviceIndex;
        matches++;
    }
    /* A GUID identifies a model, not an individual unit. Only use it alone
     * if that model was unique in the launcher and is still unambiguous. */
    return g_bSdlPreferredJoystickGuidUnique && matches == 1 ? candidate : -1;
}

static void SdlRefreshJoysticks(void)
{
    SdlJoystickDevice *device;
    int deviceCount;
    int deviceIndex;
    int preferredIndex;
    int preference;
    int slot;

    SDL_LockJoysticks();
    slot = 0;
    while (slot < 2) {
        device = &g_aSdlJoystickDevices[slot];
        if (device->joystick != 0 &&
            SDL_JoystickGetAttached(device->joystick) == SDL_FALSE)
            SdlCloseJoystick(device);
        slot++;
    }

    deviceCount = SDL_NumJoysticks();
    preferredIndex = SdlFindPreferredJoystick(deviceCount);
    if (preferredIndex >= 0) {
        g_nSdlPreferredJoystickInstance =
            SDL_JoystickGetDeviceInstanceID(preferredIndex);
        if (g_aSdlJoystickDevices[0].joystick == 0 ||
            g_aSdlJoystickDevices[0].instanceId != g_nSdlPreferredJoystickInstance) {
            /* Reclaim the primary slot even when fallback devices filled both
             * slots during a disconnection. Rebuild the secondary in preference order. */
            for (slot = 0; slot < 2; slot++)
                SdlCloseJoystick(&g_aSdlJoystickDevices[slot]);
            g_bSdlTwoAxisModifierActive = 0;
            SdlOpenJoystick(preferredIndex);
        }
    }
    /* Sticks and gamepads before throttles, wheels and pedals. */
    for (preference = 0; preference < SDL_JOYSTICK_PREFERENCES; preference++) {
        deviceIndex = 0;
        while (deviceIndex < deviceCount &&
               SdlFindFreeJoystickSlot() != -1) {
            if (SdlJoystickPreference(deviceIndex) == preference)
                SdlOpenJoystick(deviceIndex);
            deviceIndex++;
        }
    }
    SDL_UnlockJoysticks();
}

static Sint16 SdlReadJoystickAxis(
    const SdlJoystickDevice *device, int axis)
{
    SDL_GameControllerAxis controllerAxis;

    if (device->joystick == 0)
        return 0;
    if (device->controller != 0) {
        switch (axis) {
        case 0:
            controllerAxis = SDL_CONTROLLER_AXIS_LEFTX;
            break;
        case 1:
            controllerAxis = SDL_CONTROLLER_AXIS_LEFTY;
            break;
        case 2:
            controllerAxis = SDL_CONTROLLER_AXIS_RIGHTX;
            break;
        case 3:
            controllerAxis = SDL_CONTROLLER_AXIS_RIGHTY;
            break;
        default:
            return 0;
        }
        return SDL_GameControllerGetAxis(device->controller,
                                         controllerAxis);
    }
    if (SDL_JoystickNumAxes(device->joystick) <= axis)
        return 0;
    return SDL_JoystickGetAxis(device->joystick, axis);
}

static int SdlReadJoystickButton(
    const SdlJoystickDevice *device, int button)
{
    SDL_GameControllerButton controllerButton;

    if (device->joystick == 0)
        return 0;
    if (device->controller != 0) {
        switch (button) {
        case 0:
            controllerButton = SDL_CONTROLLER_BUTTON_A;
            break;
        case 1:
            controllerButton = SDL_CONTROLLER_BUTTON_B;
            break;
        case 2:
            controllerButton = SDL_CONTROLLER_BUTTON_X;
            break;
        case 3:
            controllerButton = SDL_CONTROLLER_BUTTON_Y;
            break;
        default:
            return 0;
        }
        return SDL_GameControllerGetButton(device->controller,
                                           controllerButton) != 0;
    }
    if (SDL_JoystickNumButtons(device->joystick) <= button)
        return 0;
    return SDL_JoystickGetButton(device->joystick, button) != 0;
}

/* Mapped controllers report LT and RT as axes, 0 to 32767: past half travel
 * a trigger presses, below a quarter it releases. 1 when the latch changed. */
#define SDL_PORT_TRIGGER_PRESS 16384
#define SDL_PORT_TRIGGER_RELEASE 8192

static int SdlLatchTrigger(Uint8 *latch, Uint8 bit, Sint16 value)
{
    if (!(*latch & bit) && value >= SDL_PORT_TRIGGER_PRESS) {
        *latch |= bit;
        return 1;
    }
    if ((*latch & bit) && value < SDL_PORT_TRIGGER_RELEASE) {
        *latch &= (Uint8)~bit;
        return 1;
    }
    return 0;
}

/* In flight RT is a second joystick button 0 (guns, as A) and LT a second
 * button 1 (B: missile, or the original roll/throttle modifier). */
static int SdlReadJoystickTrigger(SdlJoystickDevice *device, int button)
{
    Uint8 bit = button == 0 ? 2 : button == 1 ? 1 : 0;

    if (device->controller == 0 || bit == 0)
        return 0;
    SdlLatchTrigger(&device->triggersHeld, bit,
        SDL_GameControllerGetAxis(device->controller, bit == 2 ?
            SDL_CONTROLLER_AXIS_TRIGGERRIGHT : SDL_CONTROLLER_AXIS_TRIGGERLEFT));
    return (device->triggersHeld & bit) != 0;
}

/* The Custom mode's controls a device takes (pad_bindings.h): a mapped
 * controller the gamepad's, any other joystick its own buttons, hats and
 * axes. */
static int SdlPadDeviceControl(const SdlJoystickDevice *device, int control)
{
    if (!g_bSdlSteamDeckPreset && device->controller == 0 &&
        (g_nSdlJoystickSource == SDL_PAD_SOURCE_GAMEPAD ||
         g_nSdlJoystickSource == SDL_PAD_SOURCE_STEAM_DECK))
        return 0;
    return device->controller != 0 ? control < SDL_PAD_GAMEPAD_CONTROLS
                                    : control >= SDL_PAD_GAMEPAD_CONTROLS;
}

/* Whether one of the Custom mode's buttons is held: a mapped controller's
 * button or trigger, or another joystick's button or hat direction. */
static int SdlPadControlHeld(SdlJoystickDevice *device, int control)
{
    static const SDL_GameControllerButton buttons[SDL_PAD_BUTTONS] = {
        SDL_CONTROLLER_BUTTON_A, SDL_CONTROLLER_BUTTON_B,
        SDL_CONTROLLER_BUTTON_X, SDL_CONTROLLER_BUTTON_Y,
        SDL_CONTROLLER_BUTTON_LEFTSHOULDER, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
        SDL_CONTROLLER_BUTTON_INVALID, SDL_CONTROLLER_BUTTON_INVALID,
        SDL_CONTROLLER_BUTTON_BACK, SDL_CONTROLLER_BUTTON_START,
        SDL_CONTROLLER_BUTTON_LEFTSTICK, SDL_CONTROLLER_BUTTON_RIGHTSTICK,
        SDL_CONTROLLER_BUTTON_DPAD_UP, SDL_CONTROLLER_BUTTON_DPAD_DOWN,
        SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT
    };
    static const Uint8 hatDirections[4] = {
        SDL_HAT_UP, SDL_HAT_DOWN, SDL_HAT_LEFT, SDL_HAT_RIGHT
    };

    int index;

    if (device->joystick == 0 || !SdlPadControlTakesButton(control) ||
        !SdlPadDeviceControl(device, control))
        return 0;
    if (device->controller != 0) {
        if (control == SDL_PAD_LEFT_TRIGGER || control == SDL_PAD_RIGHT_TRIGGER)
            return SdlReadJoystickTrigger(device, control == SDL_PAD_RIGHT_TRIGGER ? 0 : 1);
        if ((g_bSdlSteamDeckPreset || g_nSdlJoystickSource == SDL_PAD_SOURCE_STEAM_DECK) &&
            control >= SDL_PAD_DPAD_UP) {
            for (index = 0; index < 4; index++) {
                if (SdlSteamDeckPaddleControl(index) == control &&
                    SDL_GameControllerGetButton(device->controller,
                        (SDL_GameControllerButton)(SDL_CONTROLLER_BUTTON_PADDLE1 + index)))
                    return 1;
            }
        }
        return SDL_GameControllerGetButton(device->controller, buttons[control]) != 0;
    }
    if (control >= SDL_PAD_STICK_HAT) {
        index = (control - SDL_PAD_STICK_HAT) / 4;
        return index < SDL_JoystickNumHats(device->joystick) &&
            (SDL_JoystickGetHat(device->joystick, index) &
             hatDirections[(control - SDL_PAD_STICK_HAT) % 4]) != 0;
    }
    index = control - SDL_PAD_STICK_BUTTON;
    return index < SDL_JoystickNumButtons(device->joystick) &&
        SDL_JoystickGetButton(device->joystick, index) != 0;
}

/* The gamepad's D-pad, or another joystick's first hat. */
static int SdlPadControlIsDpad(int control)
{
    return (control >= SDL_PAD_DPAD_UP && control < SDL_PAD_BUTTONS) ||
        (control >= SDL_PAD_STICK_HAT && control < SDL_PAD_STICK_HAT + 4);
}

/* Whether a button bound to a function (for a key, the action) is held. */
static int SdlPadFunctionHeld(SdlJoystickDevice *device, SdlPadFunctionKind kind,
                              int action)
{
    int control;

    for (control = 0; control < SDL_PAD_CONTROLS; control++) {
        if (g_aSdlPadBindings[control].kind == kind &&
            !(SdlPadControlIsDpad(control) && get_mode(1) == 4) &&
            (kind != SDL_PAD_KEY || g_aSdlPadBindings[control].value == action) &&
            SdlPadControlHeld(device, control))
            return 1;
    }
    return 0;
}

/* WC2's flight in the Custom mode. */
static int SdlPadBindingsActive(void)
{
    return g_eSdlJoystickMode == SDL_JOYSTICK_CUSTOM &&
        g_176_pfnCurrentInputPump == get_player_input;
}

BOOL SdlReadJoystick(unsigned int deviceIndex, JOYINFO *information)
{
    SdlJoystickDevice *device;
    unsigned int buttonState;
    int buttonIndex;

    if (deviceIndex >= 2)
        return FALSE;
    g_bSdlJoystickInputStarted = 1;
    SDL_JoystickUpdate();
    SdlRefreshJoysticks();
    device = &g_aSdlJoystickDevices[deviceIndex];
    if (device->joystick == 0)
        return FALSE;

    information->wXpos =
        (unsigned int)((int)SdlReadJoystickAxis(device, 0) + 32768);
    information->wYpos =
        (unsigned int)((int)SdlReadJoystickAxis(device, 1) + 32768);
    information->wZpos = 0;
    buttonState = 0;
    if (g_176_pfnCurrentInputPump == get_player_input &&
        g_eSdlJoystickMode != SDL_JOYSTICK_ORIGINAL) {
        if (SdlPadBindingsActive()) {
            if (SdlPadFunctionHeld(device, SDL_PAD_KEY, g_nSdlPadFireGuns))
                buttonState = 1;
        } else if (SdlReadJoystickButton(device, 0) || SdlReadJoystickTrigger(device, 0))
            buttonState = 1;
    } else {
        buttonIndex = 0;
        while (buttonIndex < 2) {
            if (SdlReadJoystickButton(device, buttonIndex) ||
                (g_176_pfnCurrentInputPump == get_player_input &&
                 SdlReadJoystickTrigger(device, buttonIndex)))
                buttonState |= 1U << buttonIndex;
            buttonIndex++;
        }
    }
    if (deviceIndex != 0)
        buttonState <<= 2;
    information->wButtons = buttonState;
    return TRUE;
}

static int SdlControllerHasAxis(
    const SdlJoystickDevice *device, SDL_GameControllerAxis axis)
{
    SDL_GameControllerButtonBind binding;

    binding = SDL_GameControllerGetBindForAxis(device->controller, axis);
    return binding.bindType != SDL_CONTROLLER_BINDTYPE_NONE;
}

static int SdlReadExtraJoystickAxes(Sint16 *first, Sint16 *second)
{
    SdlJoystickDevice *device;
    SdlJoystickDevice *other;
    int activeSlot;

    activeSlot = (int)g_8_nActiveInputDeviceWord;
    if (activeSlot < 0 || activeSlot >= 2)
        return 0;
    device = &g_aSdlJoystickDevices[activeSlot];
    if (device->joystick == 0)
        return 0;

    if (device->controller != 0) {
        if (SdlControllerHasAxis(
                device, SDL_CONTROLLER_AXIS_RIGHTX)) {
            *first = SdlReadJoystickAxis(device, 2);
            if (SdlControllerHasAxis(
                    device, SDL_CONTROLLER_AXIS_RIGHTY)) {
                *second = SdlReadJoystickAxis(device, 3);
                return 2;
            }
            return 1;
        }
    } else if (SDL_JoystickNumAxes(device->joystick) >= 3) {
        *first = SdlReadJoystickAxis(device, 2);
        if (SDL_JoystickNumAxes(device->joystick) >= 4) {
            *second = SdlReadJoystickAxis(device, 3);
            return 2;
        }
        return 1;
    }

    other = &g_aSdlJoystickDevices[activeSlot == 0 ? 1 : 0];
    if (other->joystick == 0)
        return 0;
    *first = SdlReadJoystickAxis(other, 0);
    *second = SdlReadJoystickAxis(other, 1);
    return 2;
}

/* The extra axes read like WC2's stick without j.cal. */
static short SdlNormalizeFlightAxis(Sint16 raw)
{
    SdlJoystickAxis axis;
    int position;

    SdlDefaultJoystickAxis(&axis);
    position = SdlShapeJoystickPosition(&axis, raw + 32768,
                                        g_nSdlJoystickDeadZone);
    return (short)SdlJoystickAxisStep(&axis, position,
                                      SdlJoystickDeadZoneSteps(g_nSdlJoystickDeadZone));
}

/* The speed requested by a throttle, 0 stopped to 65535 full speed. */
static void SdlApplyThrottlePosition(unsigned int position)
{
    int maximumSpeed;
    int targetSpeed;

    maximumSpeed = g_asShipMaximumVelocity[0] << 8;
    targetSpeed = (int)(((int64_t)position * maximumSpeed) / 65535);
    celerate(0, targetSpeed - g_anShipSpeed[0]);
}

/* A throttle axis gives full speed at its forward end, SDL's negative one. */
static void SdlApplyLinearThrottle(Sint16 raw)
{
    SdlApplyThrottlePosition((unsigned int)(32767 - raw));
}

static void SdlQueueScanCodePress(unsigned short scanCode);

/* One of the Custom mode's axes: its step, -9 to 9 (0 to 9 for a trigger),
 * and its position, 0 to 65535, both growing to the right, upwards or as a
 * trigger is pressed. The right stick is whatever SdlReadExtraJoystickAxes
 * reads: a raw joystick's third and fourth axes, or the other joystick's
 * first two. 0 when the device has no such axis. */
static int SdlReadPadAxis(const SdlJoystickDevice *device, int control,
                          const Sint16 *extra, int extraCount,
                          int *step, int *position)
{
    Sint16 raw;
    int axis;

    if (!SdlPadDeviceControl(device, control))
        return 0;
    /* Another joystick's axes by number, in pairs read as a stick's; its
       first two are the stick the game calibrates. */
    if (control >= SDL_PAD_STICK_AXIS) {
        axis = control - SDL_PAD_STICK_AXIS;
        if (axis >= SDL_JoystickNumAxes(device->joystick))
            return 0;
        raw = SdlReadJoystickAxis(device, axis);
        *step = axis == 0 ? g_nSdlPadStickX : axis == 1 ? g_nSdlPadStickY
                                                        : SdlNormalizeFlightAxis(raw);
        *position = raw + 32768;
        if ((axis & 1) != 0) {
            *step = -*step;
            *position = 65535 - *position;
        }
        return 1;
    }
    switch (control) {
    case SDL_PAD_LEFT_STICK_X:
        *step = g_nSdlPadStickX;
        *position = SdlReadJoystickAxis(device, 0) + 32768;
        return 1;
    case SDL_PAD_LEFT_STICK_Y:
        *step = -g_nSdlPadStickY;
        *position = 32767 - SdlReadJoystickAxis(device, 1);
        return 1;
    case SDL_PAD_RIGHT_STICK_X:
        if (extraCount < 1)
            return 0;
        *step = SdlNormalizeFlightAxis(extra[0]);
        *position = extra[0] + 32768;
        return 1;
    case SDL_PAD_RIGHT_STICK_Y:
        if (extraCount < 2)
            return 0;
        *step = -SdlNormalizeFlightAxis(extra[1]);
        *position = 32767 - extra[1];
        return 1;
    case SDL_PAD_LEFT_TRIGGER:
    case SDL_PAD_RIGHT_TRIGGER:
        if (device->controller == 0)
            return 0;
        raw = SDL_GameControllerGetAxis(device->controller,
            control == SDL_PAD_RIGHT_TRIGGER ? SDL_CONTROLLER_AXIS_TRIGGERRIGHT
                                             : SDL_CONTROLLER_AXIS_TRIGGERLEFT);
        *step = SdlNormalizeFlightAxis(raw);
        *position = raw > 0 ? raw * 2 : 0;
        return 1;
    default:
        return 0;
    }
}

static int SdlClampTurn(int turn)
{
    return turn < -9 ? -9 : turn > 9 ? 9 : turn;
}

/* A frame of WC2's flight in the Custom mode. The axes add up per function;
 * while a roll-and-speed button is held, yaw rolls and pitch changes speed,
 * as the original joystick's second button makes them. An axis steers while
 * it is off centre and once more as it returns, so that the keys and the
 * mouse steer while the sticks rest. A throttle sets the speed every frame,
 * as the HOTAS layouts do. */
static void SdlApplyPadFlightControls(SdlJoystickDevice *device)
{
    Sint16 extra[2] = {0, 0};
    int turns[3] = {0, 0, 0};
    int speed = 0;
    int throttle = -1;
    int extraCount;
    int modifier;
    int control;
    int axis;
    Uint32 now;

    if (SdlPadFunctionHeld(device, SDL_PAD_KEY, g_nSdlPadAfterburner))
        your_afterburner();
    modifier = SdlPadFunctionHeld(device, SDL_PAD_ROLL_AND_SPEED, 0);
    extraCount = SdlReadExtraJoystickAxes(&extra[0], &extra[1]);
    for (control = 0; control < SDL_PAD_CONTROLS; control++) {
        const SdlPadBinding *binding = &g_aSdlPadBindings[control];
        int function = binding->value;
        int step;
        int position;

        if (binding->kind != SDL_PAD_AXIS ||
            !SdlReadPadAxis(device, control, extra, extraCount, &step, &position))
            continue;
        if (modifier && function == SDL_PAD_YAW)
            function = SDL_PAD_ROLL;
        else if (modifier && function == SDL_PAD_PITCH)
            function = SDL_PAD_SPEED;
        if (function == SDL_PAD_THROTTLE) {
            if (throttle < 0)
                throttle = binding->sign < 0 ? 65535 - position : position;
        } else if (function == SDL_PAD_SPEED) {
            speed += binding->sign * step;
        } else {
            turns[function] += binding->sign * step;
        }
    }
    for (axis = 0; axis < 3; axis++) {
        int turn = SdlClampTurn(turns[axis]);

        if (turn != 0 || g_anSdlPadTurns[axis] != 0) {
            if (axis == SDL_PAD_PITCH)
                g_anFlightInput[0] = turn;
            else if (axis == SDL_PAD_YAW)
                g_anFlightInput[1] = turn;
            else
                g_anFlightInput[2] = turn;
        }
        g_anSdlPadTurns[axis] = turn;
    }
    speed = SdlClampTurn(speed);
    if (speed != 0)
        accelerate(speed / 2);
    if (throttle >= 0)
        SdlApplyThrottlePosition((unsigned int)throttle);

    now = SDL_GetTicks();
    for (control = 0; control < SDL_PAD_CONTROLS; control++) {
        const SdlPadBinding *binding = &g_aSdlPadBindings[control];

        if (binding->kind == SDL_PAD_KEY &&
            (g_aSdlPadActions[binding->value].key == SDL_SCANCODE_EQUALS ||
             g_aSdlPadActions[binding->value].key == SDL_SCANCODE_MINUS) &&
            SdlPadControlHeld(device, control) &&
            SDL_TICKS_PASSED(now, g_adwSdlPadRepeats[control])) {
            g_adwSdlPadRepeats[control] = now + SDL_PAD_KEY_REPEAT;
            SdlQueueScanCodePress((unsigned short)SdlTranslateScanCode(
                g_aSdlPadActions[binding->value].key));
        }
        if (g_abSdlPadLatches[control] != 0)
            g_abSdlPadLatches[control]--;
    }
    g_nSdlPadStickX = 0;
    g_nSdlPadStickY = 0;
}

/* The keys that the Custom mode's held buttons hold in flight, for the
 * game's GetAsyncKeyState: steering, the full turn rate and a key's
 * modifier. The D-pad leaves the communications display's choices alone. */
const Uint8 *SdlPadKeyboardState(const Uint8 *keys)
{
    static Uint8 heldKeys[SDL_NUM_SCANCODES];
    SdlJoystickDevice *device;
    int copied = 0;
    int control;
    int slot = g_8_nActiveInputDeviceWord;

    if (g_eSdlJoystickMode != SDL_JOYSTICK_CUSTOM || !SdlSpaceflightInputActive() ||
        slot < 0 || slot >= 2 || g_aSdlJoystickDevices[slot].joystick == 0)
        return keys;
    device = &g_aSdlJoystickDevices[slot];
    for (control = 0; control < SDL_PAD_CONTROLS; control++) {
        const SdlPadBinding *binding = &g_aSdlPadBindings[control];
        const SdlPadAction *action;

        if (binding->kind != SDL_PAD_KEY || binding->value == g_nSdlPadFireGuns ||
            binding->value == g_nSdlPadAfterburner ||
            (SdlPadControlIsDpad(control) && get_mode(1) == 4) ||
            (g_abSdlPadLatches[control] == 0 && !SdlPadControlHeld(device, control)))
            continue;
        if (!copied) {
            memcpy(heldKeys, keys, sizeof(heldKeys));
            copied = 1;
        }
        action = &g_aSdlPadActions[binding->value];
        heldKeys[action->key] = 1;
        if ((action->modifier & KMOD_CTRL) != 0)
            heldKeys[SDL_SCANCODE_LCTRL] = 1;
    }
    return copied ? heldKeys : keys;
}

/* Issue #67: the Kilrathi Saga's WC2 steers from key events. Each press or
 * system repeat of a steering key turns one step harder, and any key's
 * release levels every axis, so a held key turned only as fast as the
 * system repeated it, and letting go of one of two directions stopped the
 * other. WC1's Kilrathi Saga build reads the held keys every frame through
 * the same PollKeyboardState instead, as the reporter found DOS WC2 to
 * behave; WC2 itself does so only while a joystick moves. player_input calls
 * this on the frames it reads no joystick: while the keyboard holds the axes
 * it releases each one none of whose keys is held, then turns the held keys
 * one step, as a repeat of them would. */
void SdlSteerWithHeldKeys(void)
{
    static signed char lastSteeringKey = (signed char)0x80;
    signed char currentKey;
    signed char previousKey;
    signed char key;
    int diagonal;

    key = (signed char)PollKeyboardState();
    if (g_nCockpitControlState == 0) {
        diagonal = GetAsyncKeyState(VK_HOME) || GetAsyncKeyState(VK_PRIOR) ||
            GetAsyncKeyState(VK_END) || GetAsyncKeyState(VK_NEXT);
        if (!diagonal && !GetAsyncKeyState(VK_UP) && !GetAsyncKeyState(VK_DOWN))
            g_anFlightInput[0] = 0;
        if (!diagonal && !GetAsyncKeyState(VK_LEFT) && !GetAsyncKeyState(VK_RIGHT))
            g_anFlightInput[1] = 0;
        if (!GetAsyncKeyState(VK_INSERT) && !GetAsyncKeyState(0xbc) &&
            !GetAsyncKeyState(VK_DELETE) && !GetAsyncKeyState(0xbe))
            g_anFlightInput[2] = 0;
    }
    if (key == 0) {
        lastSteeringKey = (signed char)0x80;
        return;
    }
    currentKey = g_cCurrentKey;
    previousKey = g_cPreviousKey;
    g_cCurrentKey = key;
    g_cPreviousKey = lastSteeringKey;
    process_player_input();
    g_cCurrentKey = currentKey;
    g_cPreviousKey = previousKey;
    lastSteeringKey = key;
}

void SdlApplyJoystickFlightControls(void)
{
    InputDeviceSample *sample;
    SdlJoystickMode mode;
    Sint16 firstRaw;
    Sint16 secondRaw;
    short first;
    short second;
    int activeSlot;
    int axisCount;

    g_bSdlJoystickSpaceflightActive = 1;
    mode = g_eSdlJoystickMode;
    if (g_176_pfnCurrentInputPump != get_player_input ||
        g_eSdlJoystickMode == SDL_JOYSTICK_ORIGINAL) {
        SdlUpdateJoystickRumble();
        return;
    }
    activeSlot = (int)g_8_nActiveInputDeviceWord;
    if (activeSlot < 0 || activeSlot >= 2) {
        SdlUpdateJoystickRumble();
        return;
    }
    if (mode == SDL_JOYSTICK_CUSTOM) {
        SdlApplyPadFlightControls(&g_aSdlJoystickDevices[activeSlot]);
        SdlUpdateJoystickRumble();
        return;
    }
    sample = &g_aInputDeviceSamplesList[activeSlot];
    if (SdlReadJoystickButton(
            &g_aSdlJoystickDevices[activeSlot], 2))
        your_afterburner();
    SdlUpdateJoystickRumble();

    if (g_eSdlJoystickMode ==
            SDL_JOYSTICK_FOUR_BUTTON_TWO_AXIS) {
        if (SdlReadJoystickButton(
                &g_aSdlJoystickDevices[activeSlot], 3)) {
            g_bSdlTwoAxisModifierActive = 1;
            g_anFlightInput[1] = 0;
            g_anFlightInput[0] = 0;
            g_anFlightInput[2] = (short)sample->x;
            accelerate((short)-(sample->y / 2));
        } else if (g_bSdlTwoAxisModifierActive) {
            g_bSdlTwoAxisModifierActive = 0;
            g_anFlightInput[2] = 0;
            g_anFlightInput[1] = (short)sample->x;
            g_anFlightInput[0] = (short)-sample->y;
        }
        return;
    }

    axisCount = SdlReadExtraJoystickAxes(&firstRaw, &secondRaw);
    if (axisCount == 0)
        return;
    first = SdlNormalizeFlightAxis(firstRaw);
    second = axisCount > 1 ? SdlNormalizeFlightAxis(secondRaw) : 0;

    switch (g_eSdlJoystickAxesMode) {
    case SDL_JOYSTICK_AXES_TWIN_STICK_ROLL:
        g_anFlightInput[2] = first;
        if (axisCount > 1)
            accelerate((short)-(second / 2));
        break;
    case SDL_JOYSTICK_AXES_TWIN_STICK_YAW:
        g_anFlightInput[2] = (short)sample->x;
        g_anFlightInput[1] = first;
        if (axisCount > 1)
            accelerate((short)-(second / 2));
        break;
    case SDL_JOYSTICK_AXES_HOTAS_YAW:
        g_anFlightInput[2] = (short)sample->x;
        g_anFlightInput[1] = first;
        if (axisCount > 1)
            SdlApplyLinearThrottle(secondRaw);
        break;
    case SDL_JOYSTICK_AXES_HOTAS_ROLL:
        g_anFlightInput[2] = first;
        if (axisCount > 1)
            SdlApplyLinearThrottle(secondRaw);
        break;
    case SDL_JOYSTICK_AXES_LINEAR_THROTTLE:
        if (axisCount > 1 &&
            g_aSdlJoystickDevices[activeSlot].controller != 0)
            SdlApplyLinearThrottle(secondRaw);
        else
            SdlApplyLinearThrottle(firstRaw);
        break;
    case SDL_JOYSTICK_AXES_RUDDER_YAW:
        g_anFlightInput[2] = (short)sample->x;
        g_anFlightInput[1] = first;
        break;
    case SDL_JOYSTICK_AXES_RUDDER_ROLL:
        g_anFlightInput[2] = first;
        break;
    }
}

int SdlSpaceflightInputActive(void)
{
    return g_bSdlJoystickSpaceflightActive &&
        g_176_pfnCurrentInputPump == get_player_input &&
        g_nArcadeState == 0;
}

void SdlEndJoystickSpaceflight(void)
{
    g_bSdlJoystickSpaceflightActive = 0;
    g_bSdlTwoAxisModifierActive = 0;
    memset(g_abSdlPadLatches, 0, sizeof(g_abSdlPadLatches));
    memset(g_anSdlPadTurns, 0, sizeof(g_anSdlPadTurns));
    g_nSdlPadStickX = 0;
    g_nSdlPadStickY = 0;
    g_wSdlRumbleLow = 0;
    g_wSdlRumbleHigh = 0;
    g_dwSdlRumbleDeadline = 0;
    SdlStopJoystickRumble();
}

static int SdlControllerButtonIndex(int button)
{
    if ((g_bSdlSteamDeckPreset || (g_eSdlJoystickMode == SDL_JOYSTICK_CUSTOM &&
         g_nSdlJoystickSource == SDL_PAD_SOURCE_STEAM_DECK)) && button >= SDL_CONTROLLER_BUTTON_PADDLE1 &&
        button <= SDL_CONTROLLER_BUTTON_PADDLE4)
        return SdlSteamDeckPaddleControl(button - SDL_CONTROLLER_BUTTON_PADDLE1);
    switch ((SDL_GameControllerButton)button) {
    case SDL_CONTROLLER_BUTTON_A:
        return SDL_JOYSTICK_BUTTON_A;
    case SDL_CONTROLLER_BUTTON_B:
        return SDL_JOYSTICK_BUTTON_B;
    case SDL_CONTROLLER_BUTTON_X:
        return SDL_JOYSTICK_BUTTON_X;
    case SDL_CONTROLLER_BUTTON_Y:
        return SDL_JOYSTICK_BUTTON_Y;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
        return SDL_JOYSTICK_BUTTON_LEFT_SHOULDER;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        return SDL_JOYSTICK_BUTTON_RIGHT_SHOULDER;
    case SDL_CONTROLLER_BUTTON_BACK:
        return SDL_JOYSTICK_BUTTON_BACK;
    case SDL_CONTROLLER_BUTTON_START:
        return SDL_JOYSTICK_BUTTON_START;
    case SDL_CONTROLLER_BUTTON_LEFTSTICK:
        return SDL_JOYSTICK_BUTTON_LEFT_STICK;
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
        return SDL_JOYSTICK_BUTTON_RIGHT_STICK;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        return SDL_JOYSTICK_BUTTON_DPAD_UP;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        return SDL_JOYSTICK_BUTTON_DPAD_DOWN;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        return SDL_JOYSTICK_BUTTON_DPAD_LEFT;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        return SDL_JOYSTICK_BUTTON_DPAD_RIGHT;
    default:
        return -1;
    }
}

static void SdlQueueScanCodePress(unsigned short scanCode)
{
    /* player_input samples one transition before consuming the remaining
       queue, so lead with the release for this impulse.  WC2 key events are
       type 4 for a press and type 5 for a release. */
    QueueInputEvent(INPUT_EVENT_KEY_UP, 0, 0, scanCode, 0, 0, 0, scanCode, 0);
    QueueInputEvent(INPUT_EVENT_KEY_DOWN, 0, 0, scanCode, 0, 0, 0, scanCode, 0);
}

/* In flight the View (Back) button steps through the views of F1-F5:
 * front, left, right, rear and chase, then front again. F2 shows view 2,
 * F3 view 1, F4 view 3 and F5 view 5. On a ship with turrets the side and
 * rear keys may show a turret instead, which is view 4 for all of them; the
 * cockpit new_view loaded for it (initialize_cockpit) still names the key's
 * view. In the front view F1 would turn the cockpit off, so it is only sent
 * from another view, with the latch its handler checks (as a key press sets
 * it). */
static void SdlCycleFlightView(void)
{
    unsigned short scanCode;
    int view;

    view = g_nCurrentView == 4 ? g_1_cStoredViewDisplayModeValue : g_nCurrentView;
    switch (view) {
    case 0:
        scanCode = 0x3c;
        break;
    case 2:
        scanCode = 0x3d;
        break;
    case 1:
        scanCode = 0x3e;
        break;
    case 3:
        scanCode = 0x3f;
        break;
    default:
        scanCode = 0x3b;
        break;
    }
    if (scanCode == 0x3b)
        g_bF1KeyDown = 1;
    SdlQueueScanCodePress(scanCode);
}

int SdlGetCommunicationMenuSelection(void)
{
    if (g_nCommMenuChoiceCount <= 0) {
        g_nSdlCommunicationMenuSelection = 0;
        return -1;
    }
    if (g_nCommMenuReuseMode == 0 ||
        g_nSdlCommunicationMenuSelection >=
            g_nCommMenuChoiceCount)
        g_nSdlCommunicationMenuSelection = 0;
    return g_nSdlCommunicationMenuSelection;
}

static int SdlHandleCommunicationDpad(int button)
{
    int selection;

    if (get_mode(1) != 4)
        return 0;
    selection = SdlGetCommunicationMenuSelection();
    switch ((SdlJoystickButton)button) {
    case SDL_JOYSTICK_BUTTON_DPAD_UP:
        if (selection != -1) {
            if (selection == 0)
                selection = g_nCommMenuChoiceCount;
            g_nSdlCommunicationMenuSelection = selection - 1;
            InvalidateVduMode(1);
        }
        return 1;
    case SDL_JOYSTICK_BUTTON_DPAD_DOWN:
        if (selection != -1) {
            selection++;
            if (selection >= g_nCommMenuChoiceCount)
                selection = 0;
            g_nSdlCommunicationMenuSelection = selection;
            InvalidateVduMode(1);
        }
        return 1;
    case SDL_JOYSTICK_BUTTON_DPAD_LEFT:
        SdlQueueScanCodePress(0x01);
        return 1;
    case SDL_JOYSTICK_BUTTON_DPAD_RIGHT:
        if (selection != -1)
            SdlQueueScanCodePress(
                (unsigned short)(selection + 2));
        return 1;
    default:
        return 0;
    }
}

static unsigned short SdlFlightScanCodeForButton(int button)
{
    switch ((SdlJoystickButton)button) {
    case SDL_JOYSTICK_BUTTON_B:
        return 0x1c;
    case SDL_JOYSTICK_BUTTON_Y:
        if (g_eSdlJoystickMode ==
                SDL_JOYSTICK_FOUR_BUTTON_FOUR_AXIS)
            return 0x14;
        break;
    case SDL_JOYSTICK_BUTTON_LEFT_SHOULDER:
        return 0x22;
    case SDL_JOYSTICK_BUTTON_RIGHT_SHOULDER:
        return 0x11;
    case SDL_JOYSTICK_BUTTON_LEFT_TRIGGER:
    case SDL_JOYSTICK_BUTTON_LEFT_STICK:
        return 0x31;
    case SDL_JOYSTICK_BUTTON_RIGHT_TRIGGER:
    case SDL_JOYSTICK_BUTTON_RIGHT_STICK:
        return 0x1e;
    case SDL_JOYSTICK_BUTTON_DPAD_UP:
        return 0x2b;
    case SDL_JOYSTICK_BUTTON_DPAD_DOWN:
        return 0x0e;
    case SDL_JOYSTICK_BUTTON_DPAD_LEFT:
        return 0x2e;
    case SDL_JOYSTICK_BUTTON_DPAD_RIGHT:
        return 0x26;
    default:
        break;
    }
    return 0;
}

/* A Custom button pressed in flight presses its action's key once, as the
 * WCAT layouts' buttons do, and SdlPadKeyboardState holds the key while the
 * button is down. The guns, the afterburner and the roll-and-speed modifier
 * work while held instead (SdlReadJoystick, SdlApplyPadFlightControls). */
static void SdlPressPadControl(int button)
{
    const SdlPadBinding *binding;
    int scanCode;

    if (!SdlPadControlTakesButton(button))
        return;
    binding = &g_aSdlPadBindings[button];
    if (binding->kind == SDL_PAD_CYCLE_VIEWS) {
        SdlCycleFlightView();
        return;
    }
    if (binding->kind != SDL_PAD_KEY || binding->value == g_nSdlPadFireGuns ||
        binding->value == g_nSdlPadAfterburner)
        return;
    scanCode = SdlTranslateScanCode(g_aSdlPadActions[binding->value].key);
    if (scanCode == 0)
        return;
    g_abSdlPadLatches[button] = 2;
    g_adwSdlPadRepeats[button] = SDL_GetTicks() + SDL_PAD_KEY_DELAY;
    if (g_bSdlJoystickDebug) {
        printf("SDL joystick button %d -> %s, scan 0x%02x\n", button,
               g_aSdlPadActions[binding->value].id, (unsigned int)scanCode);
        fflush(stdout);
    }
    SdlQueuePadKeyPress(g_aSdlPadActions[binding->value].key,
                         g_aSdlPadActions[binding->value].modifier);
}

static int SdlControllerMapsRawButton(
    const SdlJoystickDevice *device, int rawButton)
{
    SDL_GameControllerButtonBind binding;
    int controllerButton;

    controllerButton = 0;
    while (controllerButton < SDL_CONTROLLER_BUTTON_MAX) {
        binding = SDL_GameControllerGetBindForButton(
            device->controller,
            (SDL_GameControllerButton)controllerButton);
        if (binding.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON &&
            binding.value.button == rawButton)
            return 1;
        controllerButton++;
    }
    return 0;
}

void SdlHandleJoystickButtonEvent(SDL_JoystickID instanceId,
                                      int button, int pressed,
                                      int controllerEvent)
{
    SdlJoystickDevice *device;
    unsigned short eventType;
    unsigned short scanCode;
    int spaceflightActive;
    int custom;
    int slot;

    slot = SdlFindJoystick(instanceId);
    if (slot == -1 || slot != (int)g_8_nActiveInputDeviceWord)
        return;
    device = &g_aSdlJoystickDevices[slot];
    if (controllerEvent) {
        if (device->controller == 0)
            return;
        button = SdlControllerButtonIndex(button);
    } else if (device->controller != 0 &&
               SdlControllerMapsRawButton(device, button)) {
        return;
    }

    spaceflightActive = SdlSpaceflightInputActive();
    custom = spaceflightActive && g_eSdlJoystickMode == SDL_JOYSTICK_CUSTOM;
    if (custom && !SdlPadDeviceControl(device,
            device->controller != 0 ? 0 : SDL_PAD_STICK_BUTTON))
        return;
    /* There another joystick's buttons are its own, by number, and
       SdlHandleJoystickHatEvent binds its hats. */
    if (custom && device->controller == 0 && !controllerEvent &&
        !(button == SDL_JOYSTICK_BUTTON_BACK && SdlAutopilotSequenceRunning())) {
        if (pressed && button < SDL_PAD_STICK_BUTTONS)
            SdlPressPadControl(SDL_PAD_STICK_BUTTON + button);
        return;
    }
    eventType = pressed ? INPUT_EVENT_KEY_DOWN : INPUT_EVENT_KEY_UP;
    if (button == SDL_JOYSTICK_BUTTON_BACK) {
        /* View is Escape outside flight and during the autopilot fly-by. */
        if (spaceflightActive && !SdlAutopilotSequenceRunning()) {
            if (pressed && custom)
                SdlPressPadControl(button);
            else if (pressed)
                SdlCycleFlightView();
            return;
        }
        if (pressed)
            g_bSceneEscapeRequested = 1;
        QueueInputEvent(eventType, 0, 0, 0x1b, 0, 0, 0, 0x01, 0);
        return;
    }
    if (button == SDL_JOYSTICK_BUTTON_START) {
        if (pressed && custom)
            SdlPressPadControl(button);
        else if ((spaceflightActive || g_bPauseInputActive != 0) && pressed)
            SdlQueueScanCodePress(0x19);
        return;
    }
    if (!pressed)
        return;
    if (button == SDL_JOYSTICK_BUTTON_Y &&
        !spaceflightActive && g_bInputEventQueueEnabled != 0) {
        QueueInputEvent(INPUT_EVENT_KEY_UP, 0, 0, 'Y', 0, 0, 0, 0x15, 'y');
        QueueInputEvent(INPUT_EVENT_KEY_DOWN, 0, 0, 'Y', 0, 0, 0, 0x15, 'y');
        return;
    }
    if (!spaceflightActive ||
        g_eSdlJoystickMode == SDL_JOYSTICK_ORIGINAL)
        return;

    if (SdlHandleCommunicationDpad(button))
        return;
    if (custom) {
        SdlPressPadControl(button);
        return;
    }
    scanCode = SdlFlightScanCodeForButton(button);
    if (scanCode == 0)
        return;
    if (g_bSdlJoystickDebug) {
        printf("SDL joystick action button %d -> scan 0x%02x\n",
               button, (unsigned int)scanCode);
        fflush(stdout);
    }
    SdlQueueScanCodePress(scanCode);
}

/* Mapped controllers' trigger axes: in flight RT presses A and LT presses
 * B, as SdlReadJoystick also polls them. Elsewhere A and B events do
 * nothing. */
void SdlHandleJoystickTriggerEvent(SDL_JoystickID instanceId, Uint8 axis,
                                   Sint16 value)
{
    SdlJoystickDevice *device;
    Uint8 bit;
    int slot;

    bit = axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT ? 2 :
        axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT ? 1 : 0;
    slot = SdlFindJoystick(instanceId);
    if (bit == 0 || slot == -1)
        return;
    device = &g_aSdlJoystickDevices[slot];
    if (device->controller == 0 ||
        !SdlLatchTrigger(&device->triggersPressed, bit, value))
        return;
    /* The Custom mode's triggers are buttons of their own in flight, or
       axes that SdlApplyPadFlightControls reads. */
    if (g_eSdlJoystickMode == SDL_JOYSTICK_CUSTOM && SdlSpaceflightInputActive()) {
        if ((device->triggersPressed & bit) != 0 && slot == g_8_nActiveInputDeviceWord)
            SdlPressPadControl(bit == 2 ? SDL_PAD_RIGHT_TRIGGER : SDL_PAD_LEFT_TRIGGER);
        return;
    }
    SdlHandleJoystickButtonEvent(instanceId,
        bit == 2 ? SDL_CONTROLLER_BUTTON_A : SDL_CONTROLLER_BUTTON_B,
        (device->triggersPressed & bit) != 0, 1);
}

void SdlHandleJoystickHatEvent(SDL_JoystickID instanceId,
                                  Uint8 hat, Uint8 value)
{
    static const Uint8 directions[4] = {
        SDL_HAT_UP, SDL_HAT_DOWN, SDL_HAT_LEFT, SDL_HAT_RIGHT
    };
    SdlJoystickDevice *device;
    Uint8 *state;
    Uint8 pressed;
    int direction;
    int slot;

    slot = SdlFindJoystick(instanceId);
    if (slot == -1 || slot != g_8_nActiveInputDeviceWord ||
        hat >= SDL_PAD_STICK_HATS)
        return;
    device = &g_aSdlJoystickDevices[slot];
    if (device->controller != 0)
        return;

    state = hat == 0 ? &device->hatState : &device->secondHatState;
    pressed = (value & ~*state);
    *state = value;
    /* The Custom mode's flight binds both hats' directions; the first hat
       still chooses in the communications display. */
    if (g_eSdlJoystickMode == SDL_JOYSTICK_CUSTOM && SdlSpaceflightInputActive()) {
        if (!SdlPadDeviceControl(device, SDL_PAD_STICK_HAT))
            return;
        for (direction = 0; direction < 4; direction++) {
            if ((pressed & directions[direction]) == 0 ||
                (hat == 0 && SdlHandleCommunicationDpad(SDL_JOYSTICK_BUTTON_DPAD_UP + direction)))
                continue;
            SdlPressPadControl(SDL_PAD_STICK_HAT + hat * 4 + direction);
        }
        return;
    }
    if (hat != 0)
        return;
    if ((pressed & SDL_HAT_UP) != 0)
        SdlHandleJoystickButtonEvent(
            instanceId, SDL_JOYSTICK_BUTTON_DPAD_UP, 1, 0);
    else if ((pressed & SDL_HAT_DOWN) != 0)
        SdlHandleJoystickButtonEvent(
            instanceId, SDL_JOYSTICK_BUTTON_DPAD_DOWN, 1, 0);
    else if ((pressed & SDL_HAT_LEFT) != 0)
        SdlHandleJoystickButtonEvent(
            instanceId, SDL_JOYSTICK_BUTTON_DPAD_LEFT, 1, 0);
    else if ((pressed & SDL_HAT_RIGHT) != 0)
        SdlHandleJoystickButtonEvent(
            instanceId, SDL_JOYSTICK_BUTTON_DPAD_RIGHT, 1, 0);
}

BOOL SdlReadJoystickAxisRange(unsigned int deviceIndex,
                                 unsigned int *xMinimum,
                                 unsigned int *xMaximum,
                                 unsigned int *yMinimum,
                                 unsigned int *yMaximum)
{
    if (deviceIndex >= 2)
        return FALSE;
    SdlRefreshJoysticks();
    if (g_aSdlJoystickDevices[deviceIndex].joystick == 0)
        return FALSE;
    *xMinimum = 0;
    *xMaximum = 0xffff;
    *yMinimum = 0;
    *yMaximum = 0xffff;
    return TRUE;
}

void SdlHandleJoystickDeviceEvent(Uint32 type, Sint32 which)
{
    int slot;

    if (type == SDL_JOYDEVICEREMOVED) {
        slot = SdlFindJoystick((SDL_JoystickID)which);
        if (slot != -1)
            SdlCloseJoystick(&g_aSdlJoystickDevices[slot]);
    }
    SdlRefreshJoysticks();
    if (type == SDL_JOYDEVICEADDED &&
        g_bSdlJoystickInputStarted &&
        g_8_nActiveInputDeviceWord == -1)
        LoadJoystickCalibrationFile(9, 9, SdlGetJoystickDeadZoneSteps(),
                                    SdlGetJoystickDeadZoneSteps());
}

void SdlShutdownJoysticks(void)
{
    int slot;

    slot = 0;
    while (slot < 2) {
        SdlCloseJoystick(&g_aSdlJoystickDevices[slot]);
        slot++;
    }
    g_bSdlJoystickInputStarted = 0;
    g_bSdlJoystickSpaceflightActive = 0;
    g_bSdlTwoAxisModifierActive = 0;
    g_nSdlCommunicationMenuSelection = 0;
    SDL_free(g_pszSdlPreferredJoystickPath);
    g_pszSdlPreferredJoystickPath = 0;
    g_nSdlPreferredJoystickInstance = -1;
    g_bSdlPreferredJoystickGuidUnique = 0;
}
