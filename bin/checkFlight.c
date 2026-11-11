/* Seeded WC2/SO1/SO2 flight checks, adapted from wc1-re/bin/checkFlight.c.
 * Run through runGameTests.py to keep assets, saves and logs disposable. */
#include "game.h"

static Uint8 g_abFlightProbeKeys[SDL_NUM_SCANCODES];
static uint32_t g_uFlightProbeInputRandom;
/* Simulated time, so runs repeat regardless of host speed.  The flight loop
 * steps four 1/60-second game ticks at a time.  Scene pumps step it too, and
 * game-thread sleeps, waits that keep pumping and waits that keep reading
 * the clock within one step add time of their own, so timed waits finish
 * without host time passing. */
static Uint32 g_uFlightProbeTicks;
static Uint32 g_uFlightProbeExtraMilliseconds;
static unsigned int g_uFlightProbePumpsThisStep;
static unsigned int g_uFlightProbeReadsThisStep;
static SDL_threadID g_uFlightProbeGameThread;
void SdlPumpFlightProbeHostEvents(void);
static void RequireFlightCheck(int condition, const char *message);

/* The probe's copy of src/sdl/input.c reads this instead of SDL_GetTicks.
 * PumpWindowMessages turns the ticks part back into g_uFlightProbeTicks. */
Uint32 SdlFlightProbeGetTicks(void)
{
    if (SDL_ThreadID() == g_uFlightProbeGameThread &&
        ++g_uFlightProbeReadsThisStep > 256)
        g_uFlightProbeExtraMilliseconds++;
    return (g_uFlightProbeTicks * 1000 + 59) / 60 +
           g_uFlightProbeExtraMilliseconds;
}

/* ...and this instead of SDL_Delay.  The audio and streaming threads still
 * sleep in host time. */
void SdlFlightProbeDelay(Uint32 milliseconds)
{
    if (SDL_ThreadID() != g_uFlightProbeGameThread) {
        SDL_Delay(milliseconds);
        return;
    }
    g_uFlightProbeExtraMilliseconds += milliseconds != 0 ? milliseconds : 1;
}

static void StepFlightProbeClock(void)
{
    Uint32 milliseconds;

    g_uFlightProbeTicks += 4;
    g_uFlightProbePumpsThisStep = 0;
    g_uFlightProbeReadsThisStep = 0;
    milliseconds = SdlFlightProbeGetTicks();
    /* A wait that never ends shows up here, well before the game's own
     * millisecond arithmetic overflows after about ten hours. */
    RequireFlightCheck(milliseconds < 2 * 60 * 60 * 1000,
                       "game returns to flight within two simulated hours");
    g_196_nInputTimerTicks = (int)(milliseconds - g_13_lGameClockStartData) *
                             60 / 1000;
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
        printf("SCENE %s\n", inScene ? "begin" : "end");
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
    if (inScene || ++g_uFlightProbePumpsThisStep >= 64)
        StepFlightProbeClock();
    SdlPumpFlightProbeHostEvents();
}

int SdlFlightProbeMessageBox(Uint32 flags, const char *title,
                            const char *message, SDL_Window *window)
{
    (void)flags;
    (void)window;
    fprintf(stderr, "MESSAGE: %s: %s\n", title, message);
    return 0;
}

static void RequireFlightCheck(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s (%s)\n", message, SDL_GetError());
        exit(1);
    }
}

void SdlFinishCutsceneOnly(void)
{
    RequireFlightCheck(0, "flight probe must not enter a campaign cutscene");
}

static void QueueFlightKey(SDL_Scancode code, int pressed)
{
    SDL_Event event = {0};

    if (code == SDL_SCANCODE_UNKNOWN)
        return;
    g_abFlightProbeKeys[code] = pressed != 0;
    event.type = pressed ? SDL_KEYDOWN : SDL_KEYUP;
    event.key.state = pressed ? SDL_PRESSED : SDL_RELEASED;
    event.key.repeat = pressed == 2;
    event.key.keysym.scancode = code;
    event.key.keysym.sym = SDL_GetKeyFromScancode(code);
    RequireFlightCheck(SDL_PushEvent(&event) == 1, "queue flight key through SDL");
}

static SDL_Scancode ChooseFlightKey(void)
{
    /* Pause, quit, autopilot and modal menus (including N/navigation)
     * require another input driver.
     * Keep damage, collisions, enemy AI, weapons and rendering enabled. */
    static const SDL_Scancode choices[] = {
        SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN,
        SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_COMMA,
        SDL_SCANCODE_PERIOD, SDL_SCANCODE_SPACE, SDL_SCANCODE_SPACE,
        SDL_SCANCODE_SPACE, SDL_SCANCODE_EQUALS, SDL_SCANCODE_EQUALS,
        SDL_SCANCODE_MINUS, SDL_SCANCODE_TAB, SDL_SCANCODE_RETURN,
        SDL_SCANCODE_G, SDL_SCANCODE_W, SDL_SCANCODE_T, SDL_SCANCODE_L,
        SDL_SCANCODE_D, SDL_SCANCODE_C, SDL_SCANCODE_G,
        SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
        SDL_SCANCODE_F1, SDL_SCANCODE_F2, SDL_SCANCODE_F3,
        SDL_SCANCODE_F4, SDL_SCANCODE_F5, SDL_SCANCODE_F6,
        SDL_SCANCODE_F7, SDL_SCANCODE_F8, SDL_SCANCODE_F9
    };

    g_uFlightProbeInputRandom = g_uFlightProbeInputRandom * 1664525u + 1013904223u;
    return choices[(g_uFlightProbeInputRandom >> 16) % SDL_arraysize(choices)];
}

static void RunFlightEncounter(int series, int mission, int nav, int ticks)
{
    SDL_Scancode held = SDL_SCANCODE_UNKNOWN;
    int tick;

    g_nArcadeState = 0;
    g_nTrainSimActive = 0;
    g_1_nResourcePaletteState = 0;
    g_nMissionEntryNavOverride = nav;
    g_stCurrentPilotProfile = g_stDefaultPilotProfile;
    g_stCurrentPilotProfile.series = g_nCurSeries = series;
    g_stCurrentPilotProfile.mission = g_nCurrentLevel = mission;
    LoadStartingCampaignGlobals(0);
    g_pCampaignGlobals->series = series;
    g_pCampaignGlobals->mission = mission;
    InitializeCampaignConstellationState(g_pCampaignGlobals, 1);
    SaveAndFreeTemporaryCampaignGlobals();
    init_mission(series, mission);
    /* Follow RunSpaceFlight's setup, then step its controls/physics/drawing
     * without waiting for wall-clock frame pacing, as the WC1 probe does. */
    g_1_bSpaceFlightActive = 1;
    g_bFullScreenSpaceView = 1;
    g_nCockpitDisplayMode = 0;
    memset(g_acMissionShipStatusFlagsBytes, 0, 7);
    g_bMissionDeathSequencePending = 0;
    g_6_bMissionWingmanState = 0;
    g_nFrameSkipCountdown = 1;
    InitializeSpaceFlightInput();
    g_nSpaceExplosionFlashActive = 0;
    ReleaseSceneMusicPacket();
    init_inflight_music();
    set_up_action_sphere(nav);
    force_view(0, 0);
    copy_frame(0, 0x44);
    SetPersonnelMousePosition(160, 100);
    g_nCockpitControlState = g_nCockpitControlGoal = 0;
    FlushInputEvents();
    g_cCurrentKey = g_cPreviousKey = (char)0x80;
    SdlSetKeyboardSnapshot(g_abFlightProbeKeys);
    g_17_nFramePeriodMillis = 0;
    g_dwNextFrameDeadline = 0;
    g_8_nActiveInputDeviceWord = -1;

    for (tick = 0; tick < ticks && g_nArcadeState == 0; tick++) {
        StepFlightProbeClock();
        if (tick % 16 == 0) {
            QueueFlightKey(held, 0);
            held = ChooseFlightKey();
            printf("INPUT tick=%d key=%d (%s)\n", tick, held,
                   SDL_GetScancodeName(held));
            QueueFlightKey(held, 1);
        } else if (tick % 4 == 0) {
            QueueFlightKey(held, 2);
        }
        SdlPumpEvents();
        if (!g_bJumpSequenceActive)
            RequireFlightCheck(HandleSpaceFlightControls() != -1, "flight controls continue");
        Update_3Space();
        if (g_nArcadeState != 0)
            break;
        g_bRenderingSpaceFrame = 1;
        RenderSpaceViewFrame();
        g_bRenderingSpaceFrame = 0;
        update_cockpit();
        if (g_pszPendingHudMessage != NULL)
            SetHudTextColour(g_pszPendingHudMessage, g_18_nHudMessageColour);
        MarkDibDirty();
        DIBslamReal();
    }
    printf("END ticks=%d outcome=%d position=%d,%d,%d\n", tick,
           g_nArcadeState, g_aShipPosition[0].x, g_aShipPosition[0].y,
           g_aShipPosition[0].z);
    QueueFlightKey(held, 0);
    SdlPumpEvents();
    SdlSetKeyboardSnapshot(NULL);
    FlushInputEvents();
    if (g_nArcadeState == 1)
        flag_objective(find_objective(1, -1), 2);
    free_inflight_music();
    ConfigureInputPump(1, 0);
    g_1_bSpaceFlightActive = 0;
    free_cockpit();
    free_all_slots();
    free_3Space();
}

int main(int argc, char **argv)
{
    SDL_Window *window;
    FILE *complete;
    int campaign, series, mission, nav, ticks = 0, selectedNav = -1;
    int completed = 0, list;
    unsigned int seed = 0;
    /* MODULE.000 also holds a 13th series, but no campaign flight reaches
     * it: SERIES.S00 sends every final branch to series 13 and GAMEFLOW.S00
     * then plays the ending.  Its nav records are uninitialized editor bytes
     * whose wave commands request ALTINT files no release includes, so the
     * original exits with a disk error as each of those flights starts. */
    const int seriesCounts[] = {12, 5, 5};

    setvbuf(stdout, NULL, _IONBF, 0);
    g_uFlightProbeGameThread = SDL_ThreadID();
    RequireFlightCheck(argc == 4 || argc == 7 || argc == 8,
            "scratch campaign --list OR scratch campaign series mission seed ticks [nav]");
    campaign = atoi(argv[2]);
    RequireFlightCheck(campaign >= 0 && campaign < 3, "campaign in 0..2");
    list = argc == 4;
    if (list) {
        RequireFlightCheck(!strcmp(argv[3], "--list"), "list mode");
    } else {
        series = atoi(argv[3]);
        mission = atoi(argv[4]);
        seed = (unsigned int)strtoul(argv[5], NULL, 10);
        ticks = atoi(argv[6]);
        if (argc == 8)
            selectedNav = atoi(argv[7]);
        RequireFlightCheck(series >= 1 && series <= seriesCounts[campaign] &&
                mission >= 0 && mission < 4 && ticks > 0 && ticks <= 3600 &&
                selectedNav >= -1 && selectedNav < 10, "valid mission and tick budget");
    }
    RequireFlightCheck(_chdir(argv[1]) == 0, "enter scratch installation");
    RequireFlightCheck(SdlSelectCampaign(campaign), "campaign data installed");
    SDL_SetMainReady();
    RequireFlightCheck(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) == 0, "SDL init");
    window = SDL_CreateWindow("WC2 flight test", 0, 0, 640, 400, 0);
    RequireFlightCheck(window != NULL, "headless window");
    DIBinstall((HWND)window);
    g_10_hMainWindow = (HWND)window;
    SdlStartEventPump();
    RequireFlightCheck(SdlInitializeOriginFxAudio(SdlUsingDosData()), "initialize original audio");
    InitializeAudioSystem((HWND)window);
    InitializeAudioStreamer((HWND)window);
    AllocateApplicationScratchBuffer();
    g_bApplicationControllerActive = 1;
    g_19_bStoredHighMemoryEnabled = 1;
    g_stDefaultPilotProfile.field_5e = 5;
    g_stCurrentPilotProfile = g_stDefaultPilotProfile;
    RequireFlightCheck(_chdir("GAMEDAT") == 0, "enter assets");
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
    srand(seed);
    g_uFlightProbeInputRandom = seed;

    if (list) {
        for (series = 1; series <= seriesCounts[campaign]; series++) {
            for (mission = 0; mission < 4; mission++) {
                /* The base game's fourth slot in series 7 is unused. */
                if (campaign == 0 && mission == 3 && series == 7)
                    continue;
                LoadMissionData(series, mission);
                for (nav = 0; nav < 10; nav++) {
                    if (g_aMissionNavPoints[nav].type == 1) {
                        printf("CASE %d %d %d\n", campaign, series, mission);
                        break;
                    }
                }
            }
        }
    } else {
        for (nav = 0; nav < 10; nav++) {
            LoadMissionData(series, mission);
            if (g_aMissionNavPoints[nav].type != 1 ||
                (selectedNav != -1 && nav != selectedNav))
                continue;
            printf("NAV campaign=%d series=%d mission=%d nav=%d seed=%u\n",
                   campaign, series, mission, nav, seed);
            RunFlightEncounter(series, mission, nav, ticks);
            completed++;
        }
        RequireFlightCheck(completed > 0, "mission contains selected flight encounters");
        printf("PASS: %d nav encounters\n", completed);
    }
    g_bApplicationShutdownStarted = 1;
    ReleaseApplicationScratchBuffer();
    if ((g_dwStreamerMode & 1) != 0)
        ix_streamer_destroy();
    ServiceAudioStream();
    SdlShutdownOriginFxAudio();
    DIBunInstall();
    SDL_DestroyWindow(window);
    SDL_Quit();
    complete = fopen("probe-complete", "w");
    RequireFlightCheck(complete != NULL && fclose(complete) == 0, "record completed probe");
    return 0;
}
