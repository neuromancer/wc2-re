/* libFuzzer-driven WC2/SO1/SO2 flight controls, after wc1-re/bin/checkFlight.c;
 * run through runGameTests.py, which prepares a disposable installation.
 * Input: campaign (0 WC2, 1 SO1, 2 SO2; one per process), series (1-based),
 * mission (0-based), start, little-endian PRNG seed (4 bytes), then pairs of
 * action and hold bytes. Start bits 0-3 give the flight-path legs travelled
 * before control begins; with bit 7 set they name a nav point (0-9) entered
 * directly instead, which reaches navs off the flight path. Bit 6 starts in
 * the cockpit rather than the full-screen view. Action bits 0-5 select a
 * control; bit 6 keeps the controls already held. Hold bits 0-5 give 1..64
 * ticks and bits 6-7 multiply them by 1, 2, 4 or 8. WC2_FLIGHT_TRACE prints
 * where each input started and ended. WC2_FLIGHT_KEYS rebinds the flight keys
 * and mouse buttons, in the launcher's [keys] form. */
#include "game.h"
#include <sanitizer/common_interface_defs.h>
#include <sys/stat.h>

enum { ACTION_NONE, ACTION_KEY, ACTION_POINTER, ACTION_BUTTONS, ACTION_WHEEL };

typedef struct FlightAction {
    unsigned char kind;
    SDL_Scancode key;
    short x; /* pointer x, button mask or wheel step */
    short y;
} FlightAction;

/* Keys and mouse events reach player_input and HandleSpaceFlightControls
 * through the port's SDL translation; Shift, Ctrl and Alt combine with held
 * keys (Alt gives the radio commands A, B, D, F, H and T). Pause (P, Esc),
 * quit (Cmd+Q), joystick calibration (Ctrl+J) and the in-flight computer
 * (N on the navigation VDU) wait for other input and stay out. Ctrl+S and M only change sound or text settings, and pressing
 * both buttons comes from keeping one held. */
static const FlightAction actions[] = {
    {ACTION_NONE, SDL_SCANCODE_UNKNOWN, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_UP, 0, 0}, {ACTION_KEY, SDL_SCANCODE_DOWN, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_LEFT, 0, 0}, {ACTION_KEY, SDL_SCANCODE_RIGHT, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_HOME, 0, 0}, {ACTION_KEY, SDL_SCANCODE_PAGEUP, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_END, 0, 0}, {ACTION_KEY, SDL_SCANCODE_PAGEDOWN, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_COMMA, 0, 0}, {ACTION_KEY, SDL_SCANCODE_PERIOD, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_EQUALS, 0, 0}, {ACTION_KEY, SDL_SCANCODE_MINUS, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_BACKSPACE, 0, 0}, {ACTION_KEY, SDL_SCANCODE_BACKSLASH, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_TAB, 0, 0}, {ACTION_KEY, SDL_SCANCODE_SPACE, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_RETURN, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_G, 0, 0}, {ACTION_KEY, SDL_SCANCODE_W, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_L, 0, 0}, {ACTION_KEY, SDL_SCANCODE_T, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_D, 0, 0}, {ACTION_KEY, SDL_SCANCODE_C, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_N, 0, 0}, {ACTION_KEY, SDL_SCANCODE_V, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_J, 0, 0}, {ACTION_KEY, SDL_SCANCODE_A, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_E, 0, 0}, {ACTION_KEY, SDL_SCANCODE_B, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_F, 0, 0}, {ACTION_KEY, SDL_SCANCODE_H, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_LSHIFT, 0, 0}, {ACTION_KEY, SDL_SCANCODE_LCTRL, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_LALT, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_1, 0, 0}, {ACTION_KEY, SDL_SCANCODE_2, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_3, 0, 0}, {ACTION_KEY, SDL_SCANCODE_4, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_5, 0, 0}, {ACTION_KEY, SDL_SCANCODE_6, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_7, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_F1, 0, 0}, {ACTION_KEY, SDL_SCANCODE_F2, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_F3, 0, 0}, {ACTION_KEY, SDL_SCANCODE_F4, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_F5, 0, 0}, {ACTION_KEY, SDL_SCANCODE_F6, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_F7, 0, 0}, {ACTION_KEY, SDL_SCANCODE_F8, 0, 0},
    {ACTION_KEY, SDL_SCANCODE_F9, 0, 0},
    {ACTION_POINTER, SDL_SCANCODE_UNKNOWN, 40, 25},
    {ACTION_POINTER, SDL_SCANCODE_UNKNOWN, 160, 25},
    {ACTION_POINTER, SDL_SCANCODE_UNKNOWN, 280, 25},
    {ACTION_POINTER, SDL_SCANCODE_UNKNOWN, 40, 100},
    {ACTION_POINTER, SDL_SCANCODE_UNKNOWN, 160, 100},
    {ACTION_POINTER, SDL_SCANCODE_UNKNOWN, 280, 100},
    {ACTION_POINTER, SDL_SCANCODE_UNKNOWN, 40, 175},
    {ACTION_POINTER, SDL_SCANCODE_UNKNOWN, 160, 175},
    {ACTION_POINTER, SDL_SCANCODE_UNKNOWN, 280, 175},
    {ACTION_BUTTONS, SDL_SCANCODE_UNKNOWN, SDL_BUTTON_LMASK, 0},
    {ACTION_BUTTONS, SDL_SCANCODE_UNKNOWN, SDL_BUTTON_RMASK, 0},
    {ACTION_WHEEL, SDL_SCANCODE_UNKNOWN, 1, 0}, {ACTION_WHEEL, SDL_SCANCODE_UNKNOWN, -1, 0}
};

typedef char FlightActionsFitSixBits[SDL_arraysize(actions) <= 64 ? 1 : -1];

/* MODULE.000 also holds a 13th series, but no campaign flight reaches it:
 * SERIES.S00 sends every final branch there and GAMEFLOW.S00 plays the
 * ending. Its nav records are uninitialized editor bytes whose wave commands
 * request ALTINT files no release includes. */
static const int seriesCounts[] = {12, 5, 5};
static const char *const campaignNames[] = {"wc2", "so1", "so2"};
static Uint8 keys[SDL_NUM_SCANCODES];
static Uint32 buttons;
static int pointerX = 160, pointerY = 100;
static SDL_Scancode repeatedKey = SDL_SCANCODE_UNKNOWN;
static unsigned short activeNavs[12][4];
static int fuzzCampaign;
static int seriesFilter = -1, missionFilter = -1;
static int tickLimit = 600;
static int inputActive;
static int traceInputs;
static char prefPath[PATH_MAX];

/* Settings that flight keys toggle and no mission resets. */
static signed char initialMessageSpeed;
static signed char initialVideoImagesSuppressed;
static int initialSfxVolume;
static int initialMusicVolume;
static short initialSpaceFramePeriod;
static short initialTargetLockMode;
static short initialFrameSkip;
static signed char initialViewDisplayMode;

/* Simulated time, so inputs repeat regardless of host speed. The flight loop
 * steps four 1/60-second game ticks at a time. Scene pumps step it too, and
 * game-thread sleeps, waits that keep pumping and waits that keep reading the
 * clock within one step add time of their own, so timed waits finish without
 * host time passing. Each input restarts the clock. */
static Uint32 probeTicks;
static Uint32 probeExtraMilliseconds;
static Uint32 probeStepMilliseconds;
static Uint32 probeLongestStep;
static unsigned int probePumpsThisStep;
static unsigned int probeReadsThisStep;
static SDL_threadID gameThread;
void SdlPumpFlightProbeHostEvents(void);

static void Require(int condition, const char *message)
{
    char summary[256];

    if (!condition) {
        fprintf(stderr, "FAIL: %s (%s)\n", message, SDL_GetError());
        /* Fork mode keeps only the sanitizer's report; name the check there. */
        snprintf(summary, sizeof(summary), "flight fuzzer: %s", message);
        __sanitizer_report_error_summary(summary);
        abort();
    }
}

static void CheckFuzzExit(void)
{
    /* Original fatal errors may call exit(); make libFuzzer retain the input. */
    Require(!inputActive, "game exited during flight input or initialization");
}

/* A wait that never ends fails here, inside the wait so that the crash
 * stack shows it, well before the game's own millisecond arithmetic
 * overflows after about ten hours. */
static Uint32 CheckProbeClock(Uint32 milliseconds)
{
    static Uint32 reportedStep = (Uint32)-1;

    Require(milliseconds < 2 * 60 * 60 * 1000,
            "game returns to flight within two simulated hours");
    /* Traces show where a step first waits a simulated minute. */
    if (traceInputs && milliseconds - probeStepMilliseconds > 60 * 1000 &&
        reportedStep != probeStepMilliseconds) {
        reportedStep = probeStepMilliseconds;
        fprintf(stderr, "TRACE one step has waited %u ms:\n",
                milliseconds - probeStepMilliseconds);
        __sanitizer_print_stack_trace();
    }
    return milliseconds;
}

/* The probe's copy of src/sdl/input.c reads this instead of SDL_GetTicks.
 * PumpWindowMessages turns the ticks part back into probeTicks. */
Uint32 SdlFlightProbeGetTicks(void)
{
    if (SDL_ThreadID() != gameThread)
        return (probeTicks * 1000 + 59) / 60 + probeExtraMilliseconds;
    if (++probeReadsThisStep > 256)
        probeExtraMilliseconds++;
    return CheckProbeClock((probeTicks * 1000 + 59) / 60 + probeExtraMilliseconds);
}

/* ...and this instead of SDL_Delay. The audio and streaming threads still
 * sleep in host time. */
void SdlFlightProbeDelay(Uint32 milliseconds)
{
    if (SDL_ThreadID() != gameThread) {
        SDL_Delay(milliseconds);
        return;
    }
    probeExtraMilliseconds += milliseconds != 0 ? milliseconds : 1;
    CheckProbeClock((probeTicks * 1000 + 59) / 60 + probeExtraMilliseconds);
}

static void StepProbeClock(void)
{
    Uint32 milliseconds;

    probeTicks += 4;
    probePumpsThisStep = 0;
    probeReadsThisStep = 0;
    milliseconds = SdlFlightProbeGetTicks();
    /* For traces: the longest step shows a wait that burns simulated time. */
    if (milliseconds - probeStepMilliseconds > probeLongestStep)
        probeLongestStep = milliseconds - probeStepMilliseconds;
    probeStepMilliseconds = milliseconds;
    g_196_nInputTimerTicks = (int)(milliseconds - g_13_lGameClockStartData) * 60 / 1000;
}

/* Mid-flight rescues and dialogue still run through the real scene engine.
 * Advance their text, like the cutscene checks, without injecting flight keys.
 * Each scene pump, and every 64th pump of another wait, is a clock step. */
void SdlPumpEvents(void)
{
    static unsigned int scenePolls;
    static int wasInScene;
    int inScene = g_bSceneDisplayUpdateActive != 0;

    if (inScene != wasInScene) {
        if (traceInputs)
            fprintf(stderr, "TRACE scene %s at %u ticks\n", inScene ? "begins" : "ends",
                    probeTicks);
        wasInScene = inScene;
    }
    if (inScene && ++scenePolls % 47 == 0) {
        SDL_Event event = {0};
        event.type = SDL_KEYDOWN;
        event.key.keysym.scancode = SDL_SCANCODE_SPACE;
        event.key.keysym.sym = SDLK_SPACE;
        SDL_PushEvent(&event);
        event.type = SDL_KEYUP;
        SDL_PushEvent(&event);
    }
    if (inScene || ++probePumpsThisStep >= 64)
        StepProbeClock();
    SdlPumpFlightProbeHostEvents();
}

int SdlFlightProbeMessageBox(Uint32 flags, const char *title, const char *message,
                             SDL_Window *window)
{
    char summary[256];

    (void)flags;
    (void)window;
    fprintf(stderr, "MESSAGE: %s: %s\n", title, message);
    /* A fatal game error shows one before it exits; keep it with the report. */
    snprintf(summary, sizeof(summary), "message box: %s: %s", title, message);
    __sanitizer_report_error_summary(summary);
    return 0;
}

void SdlFinishCutsceneOnly(void)
{
    Require(0, "flight fuzzer must not enter a campaign cutscene");
}

/* The port's button events add or remove one button from SDL's own mouse
 * state, which pushed events never change; report the driver's buttons. */
Uint32 SDL_GetMouseState(int *x, int *y)
{
    if (x != NULL)
        *x = pointerX;
    if (y != NULL)
        *y = pointerY;
    return buttons;
}

/* Volume keys save settings through the registry emulation; keep that in the
 * disposable installation rather than the player's preferences. */
char *SDL_GetPrefPath(const char *organization, const char *application)
{
    (void)organization;
    (void)application;
    return SDL_strdup(prefPath);
}

static void QueueFlightKey(SDL_Scancode code, int pressed)
{
    SDL_Event event = {0};

    if (code == SDL_SCANCODE_UNKNOWN)
        return;
    keys[code] = pressed != 0;
    event.type = pressed ? SDL_KEYDOWN : SDL_KEYUP;
    event.key.state = pressed ? SDL_PRESSED : SDL_RELEASED;
    event.key.repeat = pressed == 2;
    event.key.keysym.scancode = code;
    event.key.keysym.sym = SDL_GetKeyFromScancode(code);
    /* The port reads Alt radio commands from each event's modifiers. */
    event.key.keysym.mod = (keys[SDL_SCANCODE_LSHIFT] ? KMOD_LSHIFT : 0) |
        (keys[SDL_SCANCODE_LCTRL] ? KMOD_LCTRL : 0) |
        (keys[SDL_SCANCODE_LALT] ? KMOD_LALT : 0);
    Require(SDL_PushEvent(&event) == 1, "queue flight key through SDL");
}

/* Window ID 0 has no window to map from, so the port takes these positions
 * as its 320x200 coordinates. */
static void QueueMouseEvent(Uint32 type, Uint8 button, int wheel)
{
    SDL_Event event = {0};

    event.type = type;
    if (type == SDL_MOUSEMOTION) {
        event.motion.state = buttons;
        event.motion.x = pointerX;
        event.motion.y = pointerY;
    } else if (type == SDL_MOUSEWHEEL) {
        event.wheel.y = wheel;
        event.wheel.direction = SDL_MOUSEWHEEL_NORMAL;
    } else {
        event.button.button = button;
        event.button.state = type == SDL_MOUSEBUTTONDOWN ? SDL_PRESSED : SDL_RELEASED;
        event.button.clicks = 1;
        event.button.x = pointerX;
        event.button.y = pointerY;
    }
    Require(SDL_PushEvent(&event) == 1, "queue mouse event through SDL");
}

/* Ctrl+J opens the interactive joystick calibration, and N with the
 * navigation VDU already up opens the in-flight computer, as in WC1. Both
 * wait for input that the fuzzer cannot give while they run. */
static int OpensModalScreen(SDL_Scancode code)
{
    return (code == SDL_SCANCODE_J && keys[SDL_SCANCODE_LCTRL]) ||
        (code == SDL_SCANCODE_LCTRL && keys[SDL_SCANCODE_J]) ||
        (code == SDL_SCANCODE_N && get_mode(1) == 5);
}

static void ApplyFlightAction(const FlightAction *action)
{
    Uint32 pressed;

    switch (action->kind) {
    case ACTION_KEY:
        if (!keys[action->key] && !OpensModalScreen(action->key)) {
            QueueFlightKey(action->key, 1);
            repeatedKey = action->key;
        }
        break;
    case ACTION_POINTER:
        pointerX = action->x;
        pointerY = action->y;
        QueueMouseEvent(SDL_MOUSEMOTION, 0, 0);
        break;
    case ACTION_BUTTONS:
        pressed = (Uint32)action->x & ~buttons;
        buttons |= (Uint32)action->x;
        if (pressed & SDL_BUTTON_LMASK)
            QueueMouseEvent(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 0);
        if (pressed & SDL_BUTTON_RMASK)
            QueueMouseEvent(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT, 0);
        break;
    case ACTION_WHEEL:
        QueueMouseEvent(SDL_MOUSEWHEEL, 0, action->x);
        break;
    default:
        break;
    }
}

static void ReleaseFlightControls(void)
{
    int code;

    for (code = 0; code < SDL_NUM_SCANCODES; code++) {
        if (keys[code])
            QueueFlightKey((SDL_Scancode)code, 0);
    }
    if (buttons & SDL_BUTTON_LMASK) {
        buttons &= ~SDL_BUTTON_LMASK;
        QueueMouseEvent(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 0);
    }
    if (buttons & SDL_BUTTON_RMASK) {
        buttons &= ~SDL_BUTTON_RMASK;
        QueueMouseEvent(SDL_MOUSEBUTTONUP, SDL_BUTTON_RIGHT, 0);
    }
    repeatedKey = SDL_SCANCODE_UNKNOWN;
}

/* One RunSpaceFlight frame, without its frame-timer wait. */
static void FlightStep(void)
{
    StepProbeClock();
    SdlPumpEvents();
    if (!g_bJumpSequenceActive)
        Require(HandleSpaceFlightControls() != -1, "flight controls continue");
    Update_3Space();
    if (g_nArcadeState != 0)
        return;
    g_bRenderingSpaceFrame = 1;
    RenderSpaceViewFrame();
    g_bRenderingSpaceFrame = 0;
    update_cockpit();
    if (g_pszPendingHudMessage != NULL)
        SetHudTextColour(g_pszPendingHudMessage, g_18_nHudMessageColour);
    MarkDibDirty();
    DIBslamReal();
}

/* Escort waits per input: three minutes at the default 20 frames per second. */
#define ESCORT_WAIT_TICKS 3600

/* auto_pilot_valid's "Wait for" rule: a capital ship heading for this nav
 * point's warp point keeps autopilot off until it has jumped. Leaving
 * earlier would take ships on the player's wing along, which play cannot. */
static int WaitingForWarpingShip(void)
{
    short ship;

    for (ship = 1; ship < 10; ship++) {
        if (g_aeObjectClass[ship] >= OBJECT_CLASS_CAPITAL_SHIP &&
            g_asShipMissionType[ship] == MISSION_TYPE_GOTO_WARP &&
            g_asShipMissionParameter[ship] == g_nCurrentNavPoint &&
            !equ_vector(&g_aMissionObjectives[(signed char)g_cCurrentObjective].position,
                        &g_aShipDestination[ship]))
            return 1;
    }
    return 0;
}

/* Reach a later part of the mission as autopilot arrival would: a scene the
 * mission queued for autopilot plays first, and a capital ship leaving by
 * this nav point's warp point is seen off; then the player stops short of
 * the next flight-path objective with the formation that auto_pilot_sequence
 * takes along and enters that nav point's sphere, and the game's own
 * objective checks register the arrival, advancing the flight path and
 * applying the nav triggers. No controls are held meanwhile. */
static void TravelFlightPath(int legs)
{
    FixedVector destination;
    signed char travelling[10];
    short formationSlot;
    short startRange;
    short ship;
    int previousLeg;
    int tick;
    int waited = 0;

    while (legs-- > 0 && !g_nArcadeState && g_cCurrentNavPointIndex >= 0 &&
           g_cCurrentNavPointIndex < (int)SDL_arraysize(g_abFlightPath) &&
           g_abFlightPath[g_cCurrentNavPointIndex] != -1) {
        previousLeg = g_cCurrentNavPointIndex;
        if (g_ucPendingEjectionTransition != 0xff && g_nPendingEjectionSequenceCount != 0) {
            g_nEjectionSequenceState = 0;
            g_nPendingEjectionSequenceCount = 0;
            ejection_sequence(g_ucPendingEjectionTransition, 1);
            g_ucPendingEjectionTransition = 0xff;
            if (g_nArcadeState)
                break;
        }
        while (waited < ESCORT_WAIT_TICKS && WaitingForWarpingShip() && !g_nArcadeState) {
            FlightStep();
            waited++;
        }
        if (g_nArcadeState || WaitingForWarpingShip())
            break;
        destination = g_aMissionObjectives[g_abFlightPath[g_cCurrentNavPointIndex]].position;
        if ((unsigned short)distance_from_point(0, &destination) >= 1500) {
            /* auto_pilot_sequence's formation, of team members clear of
               Kilrathi: wingmen, and ships bound for the same point, end in
               formation; the rest reach their own destination if it is
               nearer than the player's and otherwise stay behind. */
            startRange = distance_from_point(0, &destination);
            for (ship = 1; ship < 10; ship++) {
                travelling[ship] = 0;
                if (g_aeObjectClass[ship] < OBJECT_CLASS_SHIP ||
                    g_aeSpecialManeuver[ship] == SPECIAL_MANEUVER_UNKNOWN_9 ||
                    g_asShipSide[ship] != SIDE_IMPERIAL ||
                    !is_team_member(g_asShipMissionIndex[ship]) || kilrathi_near(ship, 10000))
                    continue;
                if (player_wingman(ship) || equ_vector(&g_aShipDestination[ship], &destination))
                    travelling[ship] = 1;
                else
                    travelling[ship] = 3;
            }
            point_at(0, destination);
            g_aShipPosition[0] = destination;
            position_relative_ijk(&g_aShipPosition[0], 0, 0, 0, -1000);
            formationSlot = 0;
            for (ship = 1; ship < 10; ship++) {
                if (travelling[ship] == 1) {
                    auto_position(ship, &formationSlot);
                    copy_frame(0, ship);
                    g_anShipSpeed[ship] = g_anShipSpeed[0];
                    g_aShipVelocity[ship] = g_aShipVelocity[0];
                } else if (travelling[ship] == 3 &&
                           distance_from_point(ship, &g_aShipDestination[ship]) < startRange) {
                    g_aShipPosition[ship] = g_aShipDestination[ship];
                }
                if (traceInputs && travelling[ship])
                    fprintf(stderr, "TRACE leg %d: ship %d %s, %d from the player\n",
                            previousLeg, ship, travelling[ship] == 1 ? "in formation" :
                            "toward its own destination", distance_from_object(ship, 0));
            }
            /* Enter the sphere from the arrival view, as house_keep does after
               manual flight: set_up_action_sphere explodes the ships it removes
               while they are on screen, so stale departure positions would
               destroy ships left far behind. */
            set_eye_direction_and_position();
            Draw_3Space_Frame();
            ReleaseStaleNavTarget();
        }
        /* update_objective_location checks one objective per frame. */
        for (tick = 0; tick <= g_cMissionObjectiveCount && !g_nArcadeState; tick++)
            FlightStep();
        /* Wait there like the player while an escorted or returning ship is
           still on its way; it registers the arrival itself. */
        for (tick = 0; waited < ESCORT_WAIT_TICKS && g_cCurrentNavPointIndex == previousLeg &&
             escorting_a_ship() && !g_nArcadeState; tick++, waited++)
            FlightStep();
        if (g_cCurrentNavPointIndex == previousLeg) {
            /* An inactive nav point, objectives still hidden, or a late escort. */
            if (traceInputs)
                fprintf(stderr, "TRACE leg %d stops at objective %d (type %d, range %d) "
                        "after waiting %d ticks\n", previousLeg, g_abFlightPath[previousLeg],
                        g_aMissionObjectives[g_abFlightPath[previousLeg]].type,
                        distance_from_point(0, &g_aMissionObjectives[
                            g_abFlightPath[previousLeg]].position), tick);
            break;
        }
    }
}

/* As the campaign loop: reset the text contexts, which reload the font the
 * previous flight's teardown released, init_mission, then RunSpaceFlight's
 * setup without frame pacing, window grabs or the canned-scene recorder. */
static void BeginFlight(int series, int mission, int entryNav, int cockpit)
{
    g_nArcadeState = 0;
    g_nTrainSimActive = 0;
    g_1_nResourcePaletteState = 0;
    g_nMissionEntryNavOverride = entryNav;
    g_stCurrentPilotProfile = g_stDefaultPilotProfile;
    g_stCurrentPilotProfile.series = g_nCurSeries = series;
    g_stCurrentPilotProfile.mission = g_nCurrentLevel = mission;
    LoadStartingCampaignGlobals(0);
    g_pCampaignGlobals->series = series;
    g_pCampaignGlobals->mission = mission;
    InitializeCampaignConstellationState(g_pCampaignGlobals, 1);
    SaveAndFreeTemporaryCampaignGlobals();
    ResetGameTextContexts();
    init_mission(series, mission);
    g_1_bSpaceFlightActive = 1;
    g_bFullScreenSpaceView = !cockpit;
    g_nCockpitDisplayMode = cockpit;
    memset(g_acMissionShipStatusFlagsBytes, 0, 7);
    g_bMissionDeathSequencePending = 0;
    g_6_bMissionWingmanState = 0;
    g_nFrameSkipCountdown = 1;
    InitializeSpaceFlightInput();
    g_nSpaceExplosionFlashActive = 0;
    ReleaseSceneMusicPacket();
    init_inflight_music();
    if (entryNav == -1)
        entryNav = g_aMissionShips[g_13_stMissionHeaderBlock.playerMissionShip].navPoint;
    set_up_action_sphere(entryNav);
    force_view(0, 0);
    if (cockpit) {
        free_view_buffer();
        SetViewportRect(&g_27_stViewScratchInfo, 0, 0, (unsigned short)(g_nScreenWidth - 1),
                        (unsigned short)(g_nScreenHeight - 1));
        initialize_view_buffer();
        new_view(0, 0);
        free_view_buffer();
        SetViewportRect(&g_27_stViewScratchInfo, 0, 0, 319, 199);
        g_1_cStoredViewDisplayModeValue++;
        initialize_cockpit(g_1_cStoredViewDisplayModeValue - 1);
        g_nCockpitControlState = g_nCockpitControlGoal = 0;
        initialize_view_buffer();
        FlushInputEvents();
    }
    copy_frame(0, 0x44);
    SetPersonnelMousePosition(160, 100);
    g_nCockpitControlState = g_nCockpitControlGoal = 0;
    FlushInputEvents();
    g_cCurrentKey = g_cPreviousKey = (char)0x80;
    SdlSetKeyboardSnapshot(keys);
    g_17_nFramePeriodMillis = 0;
    g_dwNextFrameDeadline = 0;
    g_8_nActiveInputDeviceWord = -1;
}

/* RunSpaceFlight's own cleanup, then the campaign loop's teardown after each
 * flight. Anything it leaves would pile up over the inputs of one process. */
static void EndFlight(void)
{
    ReleaseFlightControls();
    SdlPumpEvents();
    SdlSetKeyboardSnapshot(NULL);
    FlushInputEvents();
    SetViewportRect(&g_27_stViewScratchInfo, 0, 0, (unsigned short)(g_nScreenWidth - 1),
                    (unsigned short)(g_nScreenHeight - 1));
    g_nCockpitDisplayMode = 0;
    if (g_nArcadeState == 1)
        flag_objective(find_objective(1, -1), 2);
    ResetCockpitPaletteEntries();
    free_inflight_music();
    ConfigureInputPump(1, 0);
    g_nCockpitControlState = 0;
    g_1_bSpaceFlightActive = 0;
    ReleaseSpaceflightResources();
    if (traceInputs)
        fprintf(stderr, "TRACE %d packet handles after the flight\n", g_nPacketHandleCount);
}

static int ActionIndex(unsigned char kind, SDL_Scancode key, short x)
{
    int index;

    for (index = 0; index < (int)SDL_arraysize(actions); index++) {
        if (actions[index].kind == kind && actions[index].key == key &&
            ((kind != ACTION_POINTER && kind != ACTION_BUTTONS) || actions[index].x == x))
            return index;
    }
    Require(0, "seed action");
    return 0;
}

static void WriteSeed(const char *directory, int series, int mission, int start)
{
    char path[PATH_MAX];
    FILE *file;
    unsigned char seed[32] = {
        (unsigned char)fuzzCampaign, (unsigned char)series, (unsigned char)mission,
        (unsigned char)start, 0, 0, 0, 0
    };
    size_t size = 8;

    /* Throttle, steer while firing, aim and click, cycle guns and missiles,
     * fire a missile, open the radio and pick its first line, light the
     * afterburner, then try autopilot. */
    seed[size++] = ActionIndex(ACTION_KEY, SDL_SCANCODE_EQUALS, 0); seed[size++] = 31;
    seed[size++] = ActionIndex(ACTION_KEY, SDL_SCANCODE_UP, 0); seed[size++] = 15;
    seed[size++] = ActionIndex(ACTION_KEY, SDL_SCANCODE_SPACE, 0) | 0x40; seed[size++] = 31;
    seed[size++] = ActionIndex(ACTION_POINTER, SDL_SCANCODE_UNKNOWN, 160); seed[size++] = 7;
    seed[size++] = ActionIndex(ACTION_BUTTONS, SDL_SCANCODE_UNKNOWN, SDL_BUTTON_LMASK);
    seed[size++] = 15;
    seed[size++] = ActionIndex(ACTION_KEY, SDL_SCANCODE_G, 0); seed[size++] = 0;
    seed[size++] = ActionIndex(ACTION_KEY, SDL_SCANCODE_W, 0); seed[size++] = 0;
    seed[size++] = ActionIndex(ACTION_KEY, SDL_SCANCODE_RETURN, 0); seed[size++] = 7;
    seed[size++] = ActionIndex(ACTION_KEY, SDL_SCANCODE_C, 0); seed[size++] = 3;
    seed[size++] = ActionIndex(ACTION_KEY, SDL_SCANCODE_1, 0); seed[size++] = 15;
    seed[size++] = ActionIndex(ACTION_KEY, SDL_SCANCODE_TAB, 0); seed[size++] = 15;
    seed[size++] = ActionIndex(ACTION_KEY, SDL_SCANCODE_A, 0); seed[size++] = 31;
    if (start & 0x80)
        snprintf(path, sizeof(path), "%s/%s-%d-%d-nav%d", directory, campaignNames[fuzzCampaign],
                 series, mission, start & 15);
    else
        snprintf(path, sizeof(path), "%s/%s-%d-%d-leg%d%s", directory,
                 campaignNames[fuzzCampaign], series, mission, start & 15,
                 (start & 0x40) ? "-cockpit" : "");
    file = fopen(path, "rb");
    if (file != NULL) {
        fclose(file);
        return;
    }
    file = fopen(path, "wb");
    Require(file != NULL && fwrite(seed, size, 1, file) == 1 && fclose(file) == 0,
            "write flight seed");
}

int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    SDL_Window *window;
    const char *scratch = getenv("WC2_TEST_ROOT");
    const char *filter = getenv("WC2_FLIGHT_CASE");
    const char *campaignName = getenv("WC2_FLIGHT_CAMPAIGN");
    const char *ticks = getenv("WC2_FLIGHT_TICKS");
    const char *seeds = getenv("WC2_FLIGHT_SEEDS");
    /* Rebound keys and mouse buttons, as the launcher's [keys] lines. */
    const char *keyBindings = getenv("WC2_FLIGHT_KEYS");
    int filterCampaign = -1;
    int series, mission, nav, legs, cases = 0;
    int list = getenv("WC2_FLIGHT_LIST") != NULL;
    (void)argc;
    (void)argv;

    Require(atexit(CheckFuzzExit) == 0, "register early-exit check");
    inputActive = 1;
    gameThread = SDL_ThreadID();
    traceInputs = getenv("WC2_FLIGHT_TRACE") != NULL;
    if (keyBindings != NULL)
        SdlSetKeyBindings(keyBindings);
    Require(scratch != NULL, "use runGameTests.py to prepare disposable data");
    if (campaignName)
        fuzzCampaign = atoi(campaignName);
    if (filter)
        Require(sscanf(filter, "%d:%d:%d", &filterCampaign, &seriesFilter, &missionFilter) == 3 &&
                filterCampaign == fuzzCampaign, "campaign:series:mission filter");
    if (ticks)
        tickLimit = atoi(ticks);
    Require(fuzzCampaign >= 0 && fuzzCampaign < 3 && tickLimit > 0 && tickLimit <= 3600,
            "valid campaign and tick budget");
    snprintf(prefPath, sizeof(prefPath), "%s/PREFS/", scratch);
    Require(mkdir(prefPath, 0700) == 0 || errno == EEXIST, "create disposable preferences");
    Require(chdir(scratch) == 0, "enter scratch installation");
    Require(SdlSelectCampaign(fuzzCampaign), "campaign data installed");
    SDL_SetMainReady();
    Require(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) == 0, "SDL init");
    window = SDL_CreateWindow("WC2 flight fuzzer", 0, 0, 640, 400, SDL_WINDOW_HIDDEN);
    Require(window != NULL, "headless window");
    DIBinstall((HWND)window);
    g_10_hMainWindow = (HWND)window;
    SdlStartEventPump();
    Require(SdlInitializeOriginFxAudio(SdlUsingDosData()), "initialize original audio");
    InitializeAudioSystem((HWND)window);
    InitializeAudioStreamer((HWND)window);
    AllocateApplicationScratchBuffer();
    g_bApplicationControllerActive = 1;
    g_19_bStoredHighMemoryEnabled = 1;
    g_stDefaultPilotProfile.field_5e = 5;
    g_stCurrentPilotProfile = g_stDefaultPilotProfile;
    Require(chdir("GAMEDAT") == 0, "enter assets");
    GameMain(0, NULL);
    LoadOriginFxDrivers();
    SetMenuInputPump();
    g_10_nMenuDarkColour = 2;
    g_nMenuSelectedCol = 10;
    g_3_nOptionMessageColour = 0x5e;
    g_bPlayerDamageEnabled = g_bPlayerCollisionEnabled = 1;
    LoadStartingCampaignGlobals(0);
    InitializeCampaignConstellationState(g_pCampaignGlobals, 1);
    SaveAndFreeTemporaryCampaignGlobals();
    initialMessageSpeed = g_cMessageSpeed;
    initialVideoImagesSuppressed = g_bVideoImagesSuppressed;
    initialSfxVolume = g_nSfxVolumeSetting;
    initialMusicVolume = g_nMusicVolumeSetting;
    initialSpaceFramePeriod = g_nSpaceFramePeriod;
    initialTargetLockMode = g_bTargetLockMode;
    initialFrameSkip = g_nFrameSkip;
    initialViewDisplayMode = g_1_cStoredViewDisplayModeValue;
    SDL_ClearError(); /* Dummy-driver setup errors are not failures. */
    if (list)
        printf("INSTALLED %d\n", fuzzCampaign);
    for (series = 1; series <= seriesCounts[fuzzCampaign]; series++) {
        if (seriesFilter >= 0 && seriesFilter != series)
            continue;
        for (mission = 0; mission < 4; mission++) {
            /* The base game's fourth slot in series 7 is unused. */
            if ((missionFilter >= 0 && missionFilter != mission) ||
                (fuzzCampaign == 0 && series == 7 && mission == 3))
                continue;
            LoadMissionData(series, mission);
            for (nav = 0; nav < 10; nav++) {
                if (g_aMissionNavPoints[nav].type == 1)
                    activeNavs[series - 1][mission] |= 1 << nav;
            }
            if (!activeNavs[series - 1][mission])
                continue;
            if (list)
                printf("CASE %d %d %d\n", fuzzCampaign, series, mission);
            for (legs = 0; seeds != NULL && legs < 4; legs++)
                WriteSeed(seeds, series, mission, legs);
            if (seeds != NULL)
                WriteSeed(seeds, series, mission, 0x40);
            for (nav = 0; seeds != NULL && nav < 10; nav++) {
                if (activeNavs[series - 1][mission] & (1 << nav))
                    WriteSeed(seeds, series, mission, 0x80 | nav);
            }
            cases++;
        }
    }
    inputActive = 0;
    if (list) {
        FILE *file = fopen("probe-complete", "w");
        Require(file != NULL && fclose(file) == 0, "record completed inventory");
        exit(0);
    }
    Require(cases > 0, "matching missions installed");
    return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    const FlightAction *action;
    int series, mission, start, entryNav = -1, tick = 0, hold, frame;
    int startLeg, reachedLeg;
    size_t position;
    unsigned int seed;

    if (size < 10)
        return -1;
    series = data[1];
    mission = data[2];
    start = data[3];
    if (data[0] != fuzzCampaign || series < 1 || series > seriesCounts[fuzzCampaign] ||
        mission > 3 || !activeNavs[series - 1][mission])
        return -1;
    if (start & 0x80) {
        entryNav = start & 15;
        if (entryNav >= 10 || !(activeNavs[series - 1][mission] & (1 << entryNav)))
            return -1;
    }
    inputActive = 1;
    seed = (unsigned int)data[4] | ((unsigned int)data[5] << 8) |
           ((unsigned int)data[6] << 16) | ((unsigned int)data[7] << 24);
    srand(seed);
    /* Restart the clock and everything that remembers it. */
    probeTicks = 0;
    probeExtraMilliseconds = 0;
    probeStepMilliseconds = 0;
    probeLongestStep = 0;
    probePumpsThisStep = probeReadsThisStep = 0;
    InitGameClockRandomEpoch();
    g_196_nInputTimerTicks = 0;
    g_nNextInputPollTick = 0;
    g_nNextInputQueueFlushTick = 0;
    g_nNextCutsceneFrameClock = 0;
    /* Setup draws frames before BeginFlight resets the frame timer; the
     * last input's deadline would make the throttle wait for its clock. */
    g_dwNextFrameDeadline = 0;
    g_lPreviousFrameTick = 0;
    memset(keys, 0, sizeof(keys));
    buttons = 0;
    pointerX = 160;
    pointerY = 100;
    repeatedKey = SDL_SCANCODE_UNKNOWN;
    g_cMessageSpeed = initialMessageSpeed;
    g_bVideoImagesSuppressed = initialVideoImagesSuppressed;
    g_nSfxVolumeSetting = initialSfxVolume;
    g_nMusicVolumeSetting = initialMusicVolume;
    g_nSpaceFramePeriod = initialSpaceFramePeriod;
    g_bTargetLockMode = initialTargetLockMode;
    g_nFrameSkip = initialFrameSkip;
    g_1_cStoredViewDisplayModeValue = initialViewDisplayMode;
    SdlPumpEvents();
    FlushInputEvents();
    ClearInputKeyState();
    BeginFlight(series, mission, entryNav, (start & 0x40) != 0);
    startLeg = g_cCurrentNavPointIndex;
    if (entryNav == -1)
        TravelFlightPath(start & 15);
    reachedLeg = g_cCurrentNavPointIndex;
    for (position = 8; position + 1 < size && tick < tickLimit && !g_nArcadeState; position += 2) {
        if (!(data[position] & 0x40))
            ReleaseFlightControls();
        action = &actions[(data[position] & 63) % SDL_arraysize(actions)];
        hold = (1 + (data[position + 1] & 63)) << (data[position + 1] >> 6);
        ApplyFlightAction(action);
        for (frame = 0; frame < hold && tick < tickLimit && !g_nArcadeState; frame++, tick++) {
            /* Keyboard repeat applies to the most recent key. */
            if (frame && frame % 4 == 0 && repeatedKey != SDL_SCANCODE_UNKNOWN &&
                keys[repeatedKey] && !OpensModalScreen(repeatedKey))
                QueueFlightKey(repeatedKey, 2);
            FlightStep();
        }
    }
    if (traceInputs)
        fprintf(stderr, "TRACE %s series %d mission %d%s: %s %d, leg %d -> %d -> %d, %d ticks, "
                "arcade state %d, speed %d at %d %d %d; clock %u ms, longest step %u ms\n",
                campaignNames[fuzzCampaign], series,
                mission, (start & 0x40) ? " cockpit" : "",
                entryNav == -1 ? "legs asked" : "entered nav", entryNav == -1 ? start & 15 : entryNav,
                startLeg, reachedLeg, g_cCurrentNavPointIndex, tick, g_nArcadeState,
                g_anShipSpeed[0], g_aShipPosition[0].x, g_aShipPosition[0].y, g_aShipPosition[0].z,
                (probeTicks * 1000 + 59) / 60 + probeExtraMilliseconds, probeLongestStep);
    EndFlight();
    inputActive = 0;
    return 0;
}
