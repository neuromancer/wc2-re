#include "wc1.h"

#include "video_internal.h"

#include <stdio.h>
#include <string.h>

static int Wc1SdlParsePortArguments(int *argumentCount, char **arguments,
                                    int *useEnhancedRenderer)
{
    char *argument;
    int argumentIndex;
    int outputArgumentIndex;

    outputArgumentIndex = 1;
    *useEnhancedRenderer = 0;
    argumentIndex = 1;
    while (argumentIndex < *argumentCount) {
        argument = arguments[argumentIndex];
        if (strcmp(argument, "--enhanced") == 0) {
            *useEnhancedRenderer = 1;
        } else if (strcmp(argument, "--joystick-debug") == 0) {
            Wc1SdlEnableJoystickDebug();
        } else if (strcmp(argument, "--joystick-rumble") == 0) {
            Wc1SdlEnableJoystickRumble();
        } else if (strncmp(argument, "--joystick-mode=", 16) == 0) {
            if (!Wc1SdlSetJoystickMode(argument + 16)) {
                fprintf(stderr, "Unknown joystick mode: %s\n",
                        argument + 16);
                return 0;
            }
        } else if (strncmp(argument, "--joystick-axes=", 16) == 0) {
            if (!Wc1SdlSetJoystickAxesMode(argument + 16)) {
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

static void Wc1SdlApplyLegacyArguments(int argumentCount, char **arguments)
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

static int Wc1SdlRunRuntimeChecks(void)
{
    g_aShipWeapons_004956b0[1][0] = 2;
    remove_weapon(1, 0);
    if (g_aShipWeapons_004956b0[1][0] != 1)
        return 1;

    /* WC2's readout indexes the special-maneuver table with the target, so
     * the check has to name a real object; the game never reaches it with the
     * -1 "no target" sentinel. */
    g_acShipTarget_00495f20[0] = 1;
    g_cTargetDisplayObject_004934f4 = 1;
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
    g_cCockpitView_0059dab0 = 4;
    vdu_polygon(2, 50);

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
    int gameResult;
    int useEnhancedRenderer;
    int usingDosData;

    if (!Wc1SdlParsePortArguments(&argumentCount, arguments,
                                   &useEnhancedRenderer))
        return 1;
    if (useEnhancedRenderer) {
        Wc1SdlSetVideoBackend(
            WC1_SDL_VIDEO_BACKEND_GL_SHARP_BILINEAR);
    }
    checkOnly = argumentCount == 2 && strcmp(arguments[1], "--check") == 0;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER |
                 SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    windowFlags = SDL_WINDOW_RESIZABLE;
    if (!Wc1SdlConfigureVideoWindow(&windowFlags)) {
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
    Wc1SdlStartEventPump();
    if (checkOnly) {
        gameResult = Wc1SdlRunRuntimeChecks();
    } else {
        CheckLauncherAndConfig();
        usingDosData = Wc1SdlUsingDosData();
        if (usingDosData) {
            /* DOS audio drivers cannot be used by the native SDL2 host. */
            g_nAudioEnabled_0049c244 = 0;
        }
        if (usingDosData || useEnhancedRenderer) {
            if (!Wc1SdlInitializeOriginFxAudio(usingDosData)) {
                if (usingDosData) {
                    fprintf(stderr,
                            "DOS audio is unavailable.\n");
                } else {
                    fprintf(stderr,
                            "OriginFX intro music is unavailable.\n");
                }
            }
        }
        Wc1SdlApplyLegacyArguments(argumentCount, arguments);
        MonoDebug_install();
        InitializeAudioSystem((HWND)window);
        InitializeAudioStreamer((HWND)window);
        srand((unsigned int)time(0));
        InitGameClockRandomEpoch();
        CreateDebugOverlayConsole(0, (HWND)window, 60, 20);
        g_dwGameStartTime_005d12b4 = (unsigned int)time(0);
        g_pfnInputPump_005c840c = 0;
        SDL_SetWindowMouseGrab(window, SDL_TRUE);
        SDL_ShowCursor(SDL_DISABLE);
        /* The same sequence WinMain runs once the window exists. */
        g_nInputClock_005c84a8 = 0;
        AllocateApplicationScratchBuffer();
        g_bApplicationControllerActive_0049c25c = 1;
        RunGameApplication(0, &g_pEmptyStartupArgumentVector_0049c470);
        g_bApplicationShutdownStarted_0049c23c = 1;
        ReleaseApplicationScratchBuffer();
        gameResult = 0;
        SDL_SetWindowMouseGrab(window, SDL_FALSE);
        SDL_ShowCursor(SDL_ENABLE);
        DestroyGlobalDebugOverlayConsole();
        if ((g_dwStreamerState_005c4c38 & 1) != 0)
            ix_streamer_destroy();
        ServiceAudioStream();
        Wc1SdlShutdownOriginFxAudio();
    }

    DIBunInstall();
    Wc1SdlShutdownJoysticks();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return gameResult;
}
