#include "game.h"

#include "video_internal.h"

#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
extern __declspec(dllimport) BOOL __stdcall ImmDisableIME(DWORD threadId);
#endif

static jmp_buf g_stSdlCutsceneExit;
static int g_bSdlCutsceneExitArmed;

void SdlFinishCutsceneOnly(void)
{
    if (g_bSdlCutsceneExitArmed != 0)
        longjmp(g_stSdlCutsceneExit, 1);
}

static void SdlRunGameApplication(int argumentCount, char **arguments)
{
    if (g_bSdlCutsceneOnly != 0 &&
        setjmp(g_stSdlCutsceneExit) != 0) {
        g_bSdlCutsceneExitArmed = 0;
        return;
    }
    g_bSdlCutsceneExitArmed = g_bSdlCutsceneOnly;
    RunGameApplication((short)(argumentCount - 1), arguments);
    g_bSdlCutsceneExitArmed = 0;
}

static int SdlParsePortArguments(int *argumentCount, char **arguments,
                                    int *useEnhancedRenderer,
                                    int *cutsceneOnly)
{
    char *argument;
    int argumentIndex;
    int outputArgumentIndex;

    outputArgumentIndex = 1;
    *useEnhancedRenderer = 0;
    *cutsceneOnly = 0;
    argumentIndex = 1;
    while (argumentIndex < *argumentCount) {
        argument = arguments[argumentIndex];
        if (strcmp(argument, "--enhanced") == 0) {
            *useEnhancedRenderer = 1;
        } else if (strcmp(argument, "--cutscene-only") == 0) {
            *cutsceneOnly = 1;
        } else if (strcmp(argument, "--joystick-debug") == 0) {
            SdlEnableJoystickDebug();
        } else if (strcmp(argument, "--joystick-rumble") == 0) {
            SdlEnableJoystickRumble();
        } else if (strncmp(argument, "--joystick-mode=", 16) == 0) {
            if (!SdlSetJoystickMode(argument + 16)) {
                fprintf(stderr, "Unknown joystick mode: %s\n",
                        argument + 16);
                return 0;
            }
        } else if (strncmp(argument, "--joystick-axes=", 16) == 0) {
            if (!SdlSetJoystickAxesMode(argument + 16)) {
                fprintf(stderr, "Unknown joystick axes mode: %s\n",
                        argument + 16);
                return 0;
            }
        } else {
            arguments[outputArgumentIndex++] = argument;
        }
        argumentIndex++;
    }
    *argumentCount = outputArgumentIndex;
    arguments[outputArgumentIndex] = 0;
    return 1;
}

static void SdlApplyLegacyArguments(int argumentCount, char **arguments)
{
    const char *argument;
    char command;
    int argumentIndex;

    argumentIndex = 1;
    while (argumentIndex < argumentCount) {
        argument = arguments[argumentIndex];
        if (strcmp(argument, "$#SAGA.EXE") == 0)
            g_bApplicationControllerActive_0049c25c = 1;
        command = argument[0] == '-' ? argument[1] : argument[0];
        switch (command) {
        case 'b':
            *(unsigned char *)&g_bPlayerCollisionEnabled_0049d780 = 0;
            break;
        case 'c':
            g_bCockpitEnabled_0049c26c = 0;
            break;
        case 'f':
            g_bShowFrameRate_0049c260 = 1;
            break;
        case 'k':
            *(unsigned char *)&g_bPlayerDamageEnabled_0049d77c = 0;
            break;
        case 'q':
            g_bConfigQuickModeEnabled_0049c264 = 0;
            break;
        default:
            break;
        }
        argumentIndex++;
    }
}

static int SdlRunRuntimeChecks(void)
{
    g_aShipWeapons_004956b0[1][0] = 2;
    remove_weapon(1, 0);
    if (g_aShipWeapons_004956b0[1][0] != 1)
        return 1;

    /* Nothing targeted, the readout already showing that, and a frame the
     * periodic redraw does not fall on: the walk stays out of show_target_disp
     * and its text, which needs a font the check has no data to load. */
    g_acShipTarget_00495f20[0] = -1;
    g_cTargetDisplayObject_004934f4 = -1;
    g_nRenderedSpaceFrame_00493138 = 1;
    DrawTargetRangeReadout();

    g_aeObjectClass_00495328[1] = OBJECT_CLASS_SHIP;
    g_acObjectOwner_00495208[1] = -1;
    g_nYourWingman_0049346c = -1;
    send_appropriate_message(1, 0);

    /* Likewise the reroll-chance table is indexed by the maneuver, so drive
     * it with one the ship can actually be flying. */
    g_acShipTarget_00495f20[0] = 1;
    g_asShipManeuver_00495f48[0] = MANEUVER_THINKING;
    perform_maneuver(0);

    g_pSnowStaticSound_004a2664 = ix_system_new_sound(0);
    if (g_pSnowStaticSound_004a2664 == 0)
        return 1;
    stop_all_sounds();
    if (g_pSnowStaticSound_004a2664 != 0)
        return 1;

    /* WC2's vdu_polygon draws straight into the panel shape instead of
     * leaving a rectangle behind, so the call is the check. */
    vdu_polygon(2, 50);

    /* A strike whose target is gone routs, but only for a ship the mission
     * header does not list: strike_mission reads the mission-ship record of a
     * team member, and this parameter names none. */
    g_asShipMissionIndex_00495d00[1] = 1;
    g_asShipMissionParameter_00495e00[1] = -1;
    g_aeShipObjective_00495f08[1] = OBJECTIVE_HOME_BASE;
    strike_mission(1);
    if (g_asShipMissionType_00495de8[1] != MISSION_TYPE_ROUT)
        return 1;

    g_nCommPortraitIndex_0049b79c = -1;
    FreeCommDisplayResources();
    return 0;
}

int main(int argumentCount, char **arguments)
{
    SDL_Window *window;
    Uint32 windowFlags;
    int checkOnly;
    int cutsceneOnly;
    int gameResult;
    int useEnhancedRenderer;
    int usingDosData;

#ifdef _WIN32
    ImmDisableIME((DWORD)-1);
#endif
    if (!SdlParsePortArguments(&argumentCount, arguments,
                                   &useEnhancedRenderer,
                                   &cutsceneOnly))
        return 1;
    g_bSdlCutsceneOnly = cutsceneOnly;
    if (useEnhancedRenderer) {
        SdlSetVideoBackend(
            SDL_VIDEO_BACKEND_GL_SHARP_BILINEAR);
    }
    checkOnly = argumentCount == 2 && strcmp(arguments[1], "--check") == 0;
    /* The build defines SDL_MAIN_HANDLED, so tell SDL the process is already
     * up rather than letting it rename main() out from under us. */
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER |
                 SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    windowFlags = SDL_WINDOW_RESIZABLE;
    if (!SdlConfigureVideoWindow(&windowFlags)) {
        fprintf(stderr, "SDL video configuration failed: %s\n",
                SDL_GetError());
        SDL_Quit();
        return 1;
    }
    if (checkOnly)
        windowFlags |= SDL_WINDOW_HIDDEN;
    window =
        SDL_CreateWindow("Wing Commander SDL2 port", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, 960, 600, windowFlags);
    if (window == 0) {
        fprintf(stderr, "SDL window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    if (useEnhancedRenderer)
        fprintf(stderr,
                "Experimental enhanced rendering enabled "
                "(GL sharp bilinear).\n");

    DIBinstall((HWND)window);
    g_hMainWindow_005d10e0 = (HWND)window;
    SdlStartEventPump();
    if (checkOnly) {
        gameResult = SdlRunRuntimeChecks();
    } else {
        CheckLauncherAndConfig();
        usingDosData = SdlUsingDosData();
        if (!SdlInitializeOriginFxAudio(usingDosData)) {
            if (usingDosData) {
                fprintf(stderr, "DOS audio is unavailable.\n");
            } else {
                fprintf(stderr,
                        "Original orchestral intro music is unavailable.\n");
            }
        }
        /* The flight loop waits one timer period per frame, so this is the
         * frame duration in 60ths of a second less one -- the readout in
         * AdjustSpaceFramePeriod prints it as period + 1.  Retail ships 4,
         * which is 5/60s or about 12fps; the port takes a tick off for 4/60s,
         * about 15fps, which visibly smooths flight.  Set here rather than in
         * globals.c so that file stays a faithful mirror of the image's data,
         * and because the in-flight keys can still adjust it from here. */
        g_nSpaceFramePeriod_0049d768 = 3;
        SdlApplyLegacyArguments(argumentCount, arguments);
        MonoDebug_install();
        InitializeAudioSystem((HWND)window);
        InitializeAudioStreamer((HWND)window);
        srand((unsigned int)time(0));
        InitGameClockRandomEpoch();
        CreateDebugOverlayConsole(0, (HWND)window, 60, 20);
        g_dwGameStartTime_005d12b4 = (unsigned int)time(0);
        g_pfnInputPump_005c840c = 0;
        SDL_ShowCursor(SDL_DISABLE);
        /* The same sequence WinMain runs once the window exists. */
        g_nInputClock_005c84a8 = 0;
        AllocateApplicationScratchBuffer();
        g_bApplicationControllerActive_0049c25c = 1;
        /* RunGameApplication receives the number of arguments after argv[0].
         * Its recovered option loader starts copying at argv[1], then preserves
         * WC2's original one-token lookahead by exposing one fewer argument to
         * the game's parser. */
        SdlRunGameApplication(argumentCount, arguments);
        g_bApplicationShutdownStarted_0049c23c = 1;
        ReleaseApplicationScratchBuffer();
        gameResult = 0;
        SdlSetMouseGrab(0);
        SDL_ShowCursor(SDL_ENABLE);
        DestroyGlobalDebugOverlayConsole();
        if ((g_dwStreamerState_005c4c38 & 1) != 0)
            ix_streamer_destroy();
        ServiceAudioStream();
        SdlShutdownOriginFxAudio();
    }

    DIBunInstall();
    SdlShutdownJoysticks();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return gameResult;
}
