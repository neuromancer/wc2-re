#ifndef WC2_SDL_SLINT_LAUNCHER_API_H
#define WC2_SDL_SLINT_LAUNCHER_API_H

#include "../joystick_calibration.h"

#include <stddef.h>

#define SDL_LAUNCHER_DIRECTORY_CAPACITY 4096
#define SDL_LAUNCHER_KEY_BINDINGS_CAPACITY 4096

enum SdlLauncherResult {
    SDL_LAUNCHER_UNAVAILABLE = -2,
    SDL_LAUNCHER_ERROR = -1,
    SDL_LAUNCHER_CANCELLED = 0,
    SDL_LAUNCHER_ACCEPTED = 1
};

/* Game catalogs: none, translations/es.dat and translations/pl.dat. */
enum SdlLauncherLanguage {
    SDL_LAUNCHER_LANGUAGE_ENGLISH = 0,
    SDL_LAUNCHER_LANGUAGE_SPANISH = 1,
    SDL_LAUNCHER_LANGUAGE_POLISH = 2
};

enum SdlLauncherJoystickMode {
    SDL_LAUNCHER_JOYSTICK_ORIGINAL = 0,
    SDL_LAUNCHER_JOYSTICK_FOUR_BUTTON_TWO_AXIS = 1,
    SDL_LAUNCHER_JOYSTICK_FOUR_BUTTON_FOUR_AXIS = 2
};

enum SdlLauncherJoystickAxes {
    SDL_LAUNCHER_AXES_TWIN_STICK_ROLL = 0,
    SDL_LAUNCHER_AXES_TWIN_STICK_YAW = 1,
    SDL_LAUNCHER_AXES_HOTAS_YAW = 2,
    SDL_LAUNCHER_AXES_HOTAS_ROLL = 3,
    SDL_LAUNCHER_AXES_LINEAR_THROTTLE = 4,
    SDL_LAUNCHER_AXES_RUDDER_YAW = 5,
    SDL_LAUNCHER_AXES_RUDDER_ROLL = 6
};

enum SdlLauncherExplicitOption {
    SDL_LAUNCHER_EXPLICIT_ENHANCED = 1 << 0,
    SDL_LAUNCHER_EXPLICIT_EGA = 1 << 1,
    SDL_LAUNCHER_EXPLICIT_BALANCED = 1 << 2,
    SDL_LAUNCHER_EXPLICIT_RUMBLE = 1 << 3,
    SDL_LAUNCHER_EXPLICIT_JOYSTICK_MODE = 1 << 4,
    SDL_LAUNCHER_EXPLICIT_JOYSTICK_AXES = 1 << 5,
    SDL_LAUNCHER_EXPLICIT_CHEATS = 1 << 6,
    SDL_LAUNCHER_EXPLICIT_FULLSCREEN = 1 << 7,
    SDL_LAUNCHER_EXPLICIT_LANGUAGE = 1 << 8,
    SDL_LAUNCHER_EXPLICIT_CAMPAIGN = 1 << 9,
    SDL_LAUNCHER_EXPLICIT_JOYSTICK_DEAD_ZONE = 1 << 10,
    SDL_LAUNCHER_EXPLICIT_MUSIC = 1 << 11,
    SDL_LAUNCHER_EXPLICIT_MT32_ROMS = 1 << 12,
    SDL_LAUNCHER_EXPLICIT_MT32_MODEL = 1 << 13,
    SDL_LAUNCHER_EXPLICIT_HIRES_HUD = 1 << 14
};

/* The music the game plays (SDL_PORT_MUSIC_* in include/sdl_port.h). */
enum SdlLauncherMusic {
    SDL_LAUNCHER_MUSIC_DEFAULT = 0, /* Kilrathi Saga streams, DOS AdLib */
    SDL_LAUNCHER_MUSIC_ADLIB = 1,
    SDL_LAUNCHER_MUSIC_MT32 = 2
};

#define SDL_LAUNCHER_MT32_MODEL_CAPACITY 32

typedef struct SdlLauncherOptions {
    char gameDirectory[SDL_LAUNCHER_DIRECTORY_CAPACITY];
    int fullscreen;
    int language; /* SdlLauncherLanguage */
    int campaign; /* 0: WC2, 1: Special Operations 1, 2: Special Operations 2 */
    int enhancedRenderer;
    int egaDither;
    /* Cockpit text in a finer game font, with the enhanced renderer. */
    int hiResHudFonts;
    int balancedDifficulty;
    int cheatMode;
    int joystickRumble;
    int joystickMode;
    int joystickAxes;
    /* Percent of travel, 0 to SDL_JOYSTICK_DEAD_ZONE_MAXIMUM; the original's
     * is SDL_JOYSTICK_DEAD_ZONE_ORIGINAL. */
    int joystickDeadZone;
    int musicDevice;
    /* The player's MT-32 or CM-32L ROMs, and optionally the libmt32emu
     * machine to use (empty: the best one the folder has). */
    char mt32Roms[SDL_LAUNCHER_DIRECTORY_CAPACITY];
    char mt32Model[SDL_LAUNCHER_MT32_MODEL_CAPACITY];
    /* Instance IDs survive enumeration changes while the launcher is open.
     * GUID/path identify the device again if it is unplugged and reconnected. */
    int joystickInstanceId;
    int joystickGuidUnique;
    char joystickGuid[33];
    char joystickPath[SDL_LAUNCHER_DIRECTORY_CAPACITY];
    /* Flight keys and mouse buttons changed in the controls window, one
     * "action=bindings" line each (see src/sdl/keymap.c). */
    char keyBindings[SDL_LAUNCHER_KEY_BINDINGS_CAPACITY];
    /* Command-line values take precedence over saved GUI settings. */
    unsigned int explicitOptions;
} SdlLauncherOptions;

#ifdef __cplusplus
extern "C" {
#endif

/* Accepting also sets the working directory to the selected installation. */
int SdlRunLauncherGui(SdlLauncherOptions *options);
/* German UI strings in a player's German DOS WC2.EXE (0 for any other). */
int SdlCountGermanText(const char *path);
/* 1 when the folder has a playable MT-32 or CM-32L ROM pair, describing it;
 * otherwise 0 and the reason (src/sdl/mt32.c). */
int SdlDescribeMt32Roms(const char *directory, const char *machine,
                        char *text, size_t textSize);

#ifdef __cplusplus
}
#endif

#endif
