#include "game.h"

#include "video_internal.h"
#include "slint/launcher_api.h"

#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
extern __declspec(dllimport) BOOL __stdcall ImmDisableIME(DWORD threadId);
#endif

static jmp_buf g_stSdlCutsceneExit;
static int g_bSdlCutsceneExitArmed;

static const char *const g_apszSdlLauncherJoystickModes[] = {
    "original", "4button-2axis", "4button-4axis"
};

static const char *const g_apszSdlLauncherJoystickAxes[] = {
    "twin-stick-roll", "twin-stick-yaw", "hotas-yaw", "hotas-roll",
    "linear-throttle", "rudder-yaw", "rudder-roll"
};

static int SdlFindLauncherChoice(const char *name,
                                    const char *const *choices,
                                    size_t choiceCount)
{
    size_t choice;

    choice = 0;
    while (choice < choiceCount) {
        if (strcmp(name, choices[choice]) == 0)
            return (int)choice;
        choice++;
    }
    return 0;
}

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
                                    int *cutsceneOnly,
                                    int *useLauncherGui,
                                    SdlLauncherOptions *launcherOptions)
{
    char *argument;
    int argumentIndex;
    int outputArgumentIndex;

    outputArgumentIndex = 1;
    *useEnhancedRenderer = 0;
    *cutsceneOnly = 0;
    *useLauncherGui = *argumentCount == 1;
    g_bSdlBalancedDifficulty = 0;
    argumentIndex = 1;
    while (argumentIndex < *argumentCount) {
        argument = arguments[argumentIndex];
        if (strcmp(argument, "--gui") == 0) {
            *useLauncherGui = 1;
        } else if (strcmp(argument, "--fullscreen") == 0 ||
                   strcmp(argument, "--windowed") == 0) {
            launcherOptions->fullscreen = strcmp(argument, "--fullscreen") == 0;
            launcherOptions->explicitOptions |= SDL_LAUNCHER_EXPLICIT_FULLSCREEN;
        } else if (strcmp(argument, "--enhanced") == 0) {
            *useEnhancedRenderer = 1;
            launcherOptions->enhancedRenderer = 1;
            launcherOptions->explicitOptions |= SDL_LAUNCHER_EXPLICIT_ENHANCED;
        } else if (strcmp(argument, "--balanced-difficulty") == 0) {
            g_bSdlBalancedDifficulty = 1;
            launcherOptions->balancedDifficulty = 1;
            launcherOptions->explicitOptions |= SDL_LAUNCHER_EXPLICIT_BALANCED;
        } else if (strcmp(argument, "--cheats") == 0) {
            launcherOptions->cheatMode = 1;
            launcherOptions->explicitOptions |= SDL_LAUNCHER_EXPLICIT_CHEATS;
        } else if (strcmp(argument, "--cutscene-only") == 0) {
            *cutsceneOnly = 1;
        } else if (strcmp(argument, "--joystick-debug") == 0) {
            SdlEnableJoystickDebug();
        } else if (strcmp(argument, "--joystick-rumble") == 0) {
            SdlSetJoystickRumbleEnabled(1);
            launcherOptions->joystickRumble = 1;
            launcherOptions->explicitOptions |= SDL_LAUNCHER_EXPLICIT_RUMBLE;
        } else if (strncmp(argument, "--joystick-mode=", 16) == 0) {
            if (!SdlSetJoystickMode(argument + 16)) {
                fprintf(stderr, "Unknown joystick mode: %s\n",
                        argument + 16);
                return 0;
            }
            launcherOptions->joystickMode = SdlFindLauncherChoice(
                argument + 16, g_apszSdlLauncherJoystickModes,
                SDL_arraysize(g_apszSdlLauncherJoystickModes));
            launcherOptions->explicitOptions |= SDL_LAUNCHER_EXPLICIT_JOYSTICK_MODE;
        } else if (strncmp(argument, "--joystick-axes=", 16) == 0) {
            if (!SdlSetJoystickAxesMode(argument + 16)) {
                fprintf(stderr, "Unknown joystick axes mode: %s\n",
                        argument + 16);
                return 0;
            }
            launcherOptions->joystickAxes = SdlFindLauncherChoice(
                argument + 16, g_apszSdlLauncherJoystickAxes,
                SDL_arraysize(g_apszSdlLauncherJoystickAxes));
            launcherOptions->explicitOptions |= SDL_LAUNCHER_EXPLICIT_JOYSTICK_AXES;
        } else if (strcmp(argument, "--ega") == 0) {
            launcherOptions->egaDither = 1;
            launcherOptions->explicitOptions |= SDL_LAUNCHER_EXPLICIT_EGA;
        } else {
            arguments[outputArgumentIndex++] = argument;
        }
        argumentIndex++;
    }
    *argumentCount = outputArgumentIndex;
    arguments[outputArgumentIndex] = 0;
    return 1;
}

static void SdlInitializeLauncherOptions(SdlLauncherOptions *options)
{
    memset(options, 0, sizeof(*options));
    options->joystickInstanceId = -1;
    if (GetCurrentDirectoryA(sizeof(options->gameDirectory),
                             options->gameDirectory) == 0)
        strcpy(options->gameDirectory, ".");
}

static int SdlOpenLauncherGui(SdlLauncherOptions *options)
{
#ifdef WC2_STATIC_GUI
    return SdlRunLauncherGui(options);
#else
    (void)options;
    fprintf(stderr,
            "The graphical launcher is not included in this build.\n"
            "Build it with 'make modern-gui'.\n");
    return SDL_LAUNCHER_UNAVAILABLE;
#endif
}

static int SdlApplyLauncherOptions(const SdlLauncherOptions *options,
                                      int *useEnhancedRenderer)
{
    if (options->joystickMode < SDL_LAUNCHER_JOYSTICK_ORIGINAL ||
        options->joystickMode >
            SDL_LAUNCHER_JOYSTICK_FOUR_BUTTON_FOUR_AXIS ||
        options->joystickAxes < SDL_LAUNCHER_AXES_TWIN_STICK_ROLL ||
        options->joystickAxes > SDL_LAUNCHER_AXES_RUDDER_ROLL) {
        fprintf(stderr, "The GUI returned invalid joystick options.\n");
        return 0;
    }
    *useEnhancedRenderer = options->enhancedRenderer;
    g_bSdlBalancedDifficulty = options->balancedDifficulty;
    SdlSetJoystickRumbleEnabled(options->joystickRumble);
    SdlSetPreferredJoystick(options->joystickInstanceId,
                             options->joystickGuid, options->joystickPath,
                             options->joystickGuidUnique);
    SdlSetJoystickMode(
        g_apszSdlLauncherJoystickModes[options->joystickMode]);
    SdlSetJoystickAxesMode(
        g_apszSdlLauncherJoystickAxes[options->joystickAxes]);
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
            g_bApplicationControllerActive = 1;
        command = argument[0] == '-' ? argument[1] : argument[0];
        switch (command) {
        case 'b':
            *(unsigned char *)&g_bPlayerCollisionEnabled = 0;
            break;
        case 'c':
            g_bCockpitEnabled = 0;
            break;
        case 'f':
            g_bShowFrameRate = 1;
            break;
        case 'k':
            *(unsigned char *)&g_bPlayerDamageEnabled = 0;
            break;
        case 'q':
            g_bConfigQuickModeEnabled = 0;
            break;
        default:
            break;
        }
        argumentIndex++;
    }
}

int main(int argumentCount, char **arguments)
{
    SDL_Window *window;
    Uint32 windowFlags;
    int cutsceneOnly;
    int launcherGuiImplicit;
    int launcherResult;
    int useEnhancedRenderer;
    int useLauncherGui;
    int usingDosData;
    SdlLauncherOptions launcherOptions;

    launcherGuiImplicit = argumentCount == 1;
    SdlInitializeLauncherOptions(&launcherOptions);
    if (!SdlParsePortArguments(&argumentCount, arguments,
                                   &useEnhancedRenderer,
                                   &cutsceneOnly,
                                   &useLauncherGui,
                                   &launcherOptions))
        return 1;
    if (useLauncherGui) {
        launcherResult = SdlOpenLauncherGui(&launcherOptions);
        if (launcherResult == SDL_LAUNCHER_UNAVAILABLE &&
            launcherGuiImplicit)
            useLauncherGui = 0;
        else if (launcherResult == SDL_LAUNCHER_CANCELLED)
            return 0;
        else if (launcherResult != SDL_LAUNCHER_ACCEPTED)
            return 1;
        if (useLauncherGui &&
            !SetCurrentDirectoryA(launcherOptions.gameDirectory)) {
            fprintf(stderr, "Unable to enter game directory: %s\n",
                    launcherOptions.gameDirectory);
            return 1;
        }
        if (useLauncherGui &&
            !SdlApplyLauncherOptions(&launcherOptions,
                                     &useEnhancedRenderer))
            return 1;
    }
    SdlSetEgaDitherEnabled(launcherOptions.egaDither);
    if (launcherOptions.cheatMode) {
        /* CheckLauncherAndConfig (0x453C95) uses the same combination for
         * Saga's Cheater setting. Both DOS and Saga data share these flags. */
        g_nOriginDevUnlock = 1;
        g_bPlayerDamageEnabled = 0;
        g_bPlayerCollisionEnabled = 0;
    }
#ifdef _WIN32
    ImmDisableIME((DWORD)-1);
#endif
    g_bSdlCutsceneOnly = cutsceneOnly;
    if (useEnhancedRenderer) {
        SdlSetVideoBackend(
            SDL_VIDEO_BACKEND_GL_SHARP_BILINEAR);
    }
    /* The build defines SDL_MAIN_HANDLED, so tell SDL the process is already
     * up rather than letting it rename main() out from under us. */
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER |
                 SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    windowFlags = SDL_WINDOW_RESIZABLE;
    if (launcherOptions.fullscreen)
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    if (!SdlConfigureVideoWindow(&windowFlags)) {
        fprintf(stderr, "SDL video configuration failed: %s\n",
                SDL_GetError());
        SDL_Quit();
        return 1;
    }
    window =
        SDL_CreateWindow("Wing Commander SDL2 port", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, 960, 600, windowFlags);
    if (window == 0) {
        fprintf(stderr, "SDL window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    if (useLauncherGui) {
        /* Slint may create the macOS application before SDL. Cocoa's normal
         * show path does not reactivate an existing application, so transfer
         * focus explicitly when replacing the launcher with the game window. */
        SDL_ShowWindow(window);
        SDL_RaiseWindow(window);
        SDL_PumpEvents();
    }
    if (useEnhancedRenderer)
        fprintf(stderr,
                "Experimental enhanced rendering enabled "
                "(GL sharp bilinear).\n");

    DIBinstall((HWND)window);
    g_10_hMainWindow = (HWND)window;
    SdlStartEventPump();
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
    g_nSpaceFramePeriod = 3;
    SdlApplyLegacyArguments(argumentCount, arguments);
    MonoDebug_install();
    InitializeAudioSystem((HWND)window);
    InitializeAudioStreamer((HWND)window);
    srand((unsigned int)time(0));
    InitGameClockRandomEpoch();
    CreateDebugOverlayConsole(0, (HWND)window, 60, 20);
    g_11_dwGameZeroTime = (unsigned int)time(0);
    g_176_pfnCurrentInputPump = 0;
    SDL_ShowCursor(SDL_DISABLE);
    /* The same sequence WinMain runs once the window exists. */
    g_196_nInputTimerTicks = 0;
    AllocateApplicationScratchBuffer();
    g_bApplicationControllerActive = 1;
    /* RunGameApplication receives the number of arguments after argv[0].
     * Its recovered option loader starts copying at argv[1], then preserves
     * WC2's original one-token lookahead by exposing one fewer argument to
     * the game's parser. */
    SdlRunGameApplication(argumentCount, arguments);
    g_bApplicationShutdownStarted = 1;
    ReleaseApplicationScratchBuffer();
    SdlSetMouseGrab(0);
    SDL_ShowCursor(SDL_ENABLE);
    DestroyGlobalDebugOverlayConsole();
    if ((g_dwStreamerMode & 1) != 0)
        ix_streamer_destroy();
    ServiceAudioStream();
    SdlShutdownOriginFxAudio();

    DIBunInstall();
    SdlShutdownJoysticks();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
