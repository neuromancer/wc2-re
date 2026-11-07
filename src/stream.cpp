/*
 *  Game wrappers around the audio streamer.
 *
 *  Retail object 0x428900-0x428DFF (link order; see docs/ORDER.md).
 *
 *  A C++ unit: ix_log_printf, which the ix library calls with C++ linkage,
 *  sits between the streamer wrappers, and GetStreamerState follows the
 *  unit's code as a separately aligned COMDAT.
 */
#include "game.h"

/* Uninitialized C++ definitions: this unit's .bss, which LINK orders by the
 * identifiers' hash keys (the spellings follow the original addresses). */
int g_nStreamerAudioPlaying; /* 0x005b2810 */
#ifdef SDL_PORT
char g_szStreamerDirectory[4096]; /* 0x005b2818 */
#else
char g_szStreamerDirectory[128]; /* 0x005b2818 */
#endif
char g_15_szLegacyGameDataPath[0x400]; /* 0x005b2898 */
/* Formatting scratch buffer for ix_log_printf; it fills the 0x450 bytes up
 * to mono.c's storage. */
char g_3_szIxLogText[0x450]; /* 0x005b2c98 */

void ix_log_printf(const char *fmt, ...);

/* Called only by UpdateStreamerStoppedFlag; MSVC places the out-of-line copy
 * after the unit's code as a separately aligned COMDAT. */
/* Function start: 0x428DE0 */
inline unsigned int GetStreamerState(void)
{
    return g_dwStreamerMode;
}

/* Function start: 0x428900 */
void InitializeAudioStreamer(HWND window)
{
    if (g_nAudioEnabled == 0)
        return;
    ix_streamer_configure(3, (void *)1);
    ix_streamer_configure(0, window);
    ix_streamer_init();
    ix_streamer_set_intensity(0x19);
    return;
}

/* Function start: 0x42894B */
void SignalAudioMixerWakeEvent(void)
{
    if (g_nAudioEnabled == 0)
        return;
    ix_streamer_configure(2, 0);
    return;
}

/* Function start: 0x428979 */
void UpdateStreamerStoppedFlag(void)
{
#ifdef SDL_PORT
    if (SdlUsingOriginFxMusic())
        return;
#endif
    if (g_nAudioEnabled == 0)
        return;
    if ((GetStreamerState() & 4) == 0)
        g_nMusicTrackComplete = 1;
    else
        g_nMusicTrackComplete = 0;
    return;
}

/* Function start: 0x4289BF */
void Streamer_open(const char *streamName)
{
    char *streamsDirectory;
#ifdef SDL_PORT
    int length;
#endif

    if (g_nAudioEnabled == 0)
        return;
    streamsDirectory = LocateStreamsDirOnDisc();
#ifdef SDL_PORT
    if (streamsDirectory == 0) {
        fprintf(stderr, "Unable to locate STREAMS from '%s'.\n",
                SdlDescribeWorkingDirectory());
        g_nStreamerAudioPlaying = 0;
        return;
    }
    length = snprintf(g_szStreamerDirectory,
                      sizeof(g_szStreamerDirectory), "%s%s",
                      streamsDirectory, streamName);
    if (length < 0 || (size_t)length >= sizeof(g_szStreamerDirectory)) {
        fprintf(stderr, "Music stream path is too long: '%s%s'.\n",
                streamsDirectory, streamName);
        g_nStreamerAudioPlaying = 0;
        return;
    }
#else
    sprintf(g_szStreamerDirectory, "%s%s",
            streamsDirectory, streamName);
#endif
    SoundDebugPrintf("Streamer_open %s\n", g_szStreamerDirectory);
    ix_streamer_open_stream_file(g_szStreamerDirectory);
    g_nStreamerAudioPlaying = 0;
    return;
}

/* Function start: 0x428A2F */
void Streamer_play(void)
{
    if (g_nAudioEnabled == 0)
        return;
    if (g_nStreamerAudioPlaying == 0) {
        g_nStreamerAudioPlaying = 1;
        ix_streamer_audio_play();
    }
    return;
}

/* Function start: 0x428A6D */
void Streamer_stop(void)
{
    if (g_nAudioEnabled == 0)
        return;
    if (g_nStreamerAudioPlaying != 0) {
        ix_streamer_audio_stop();
        g_nStreamerAudioPlaying = 0;
    }
    return;
}

/* Function start: 0x428AAB */
void SetStreamerIntensity(int intensity)
{
    if (g_nAudioEnabled == 0)
        return;
    ix_streamer_set_intensity((unsigned char)intensity);
    if (g_nStreamerAudioPlaying == 0)
        Streamer_play();
    return;
}

/* Function start: 0x428AEB */
void SetStreamerTrigger(int trigger)
{
    if (g_nAudioEnabled == 0)
        return;
    ix_streamer_set_trigger((char)trigger);
    if (trigger >= 0 && g_nStreamerAudioPlaying == 0)
        ForceStreamerTrigger(trigger);
    return;
}

/* Function start: 0x428B3C */
void ForceStreamerTrigger(int trigger)
{
    if (g_nAudioEnabled == 0)
        return;
    if (trigger > 0) {
        ix_streamer_force_trigger((char)trigger);
        if (g_nStreamerAudioPlaying == 0)
            Streamer_play();
    }
    return;
}

/* Function start: 0x428B86 */
void Streamer_close(void)
{
    if (g_nAudioEnabled == 0)
        return;
    ix_streamer_close_stream_file();
    return;
}

/* Function start: 0x428BAD */
void DestroyAudioStreamer(void)
{
    if (g_nAudioEnabled == 0)
        return;
    ix_streamer_destroy();
    return;
}

/* The ix diagnostic printer.  Every ix diagnostic goes through here, 107
 * call sites, always in pairs:
 *     ix_log_printf("Fatal [%s - %d]:\n", __FILE__, __LINE__);
 *     ix_log_printf("human readable message");
 * The ix units call it with C++ linkage. */
#pragma function(strcpy)
/* Function start: 0x428BD4 */
void ix_log_printf(const char *fmt, ...)
{
    va_list arguments;

    if (fmt != 0) {
        va_start(arguments, fmt);
        vsprintf(g_3_szIxLogText, fmt, arguments);
        va_end(arguments);
    } else {
        strcpy(g_3_szIxLogText, "(null)");
    }
#ifdef SDL_PORT
    SdlOutputDebugString(g_3_szIxLogText);
#else
    OutputDebugStringA(g_3_szIxLogText);
#endif
}

/* The registry key and value names are constant arrays (the original keeps
 * one copy of each in .rdata, shared by both functions below). */
const char g_szOriginRegistryKey[] =
    "Software\\Origin Systems\\WC: Kilrathi Saga"; /* 0x0048e070 */
const char g_szInstalledToValueName[] = "Installed To:"; /* 0x0048e0a0 */
const char g_szWing1InstalledValueName[] = "Wing 1 Installed"; /* 0x0048e0b0 */
const char g_szCheaterValueName[] = "Cheater"; /* 0x0048e0c8 */

#pragma function(strcat)
/* Function start: 0x428C35 */
char *GetLegacyGameDataPath(void)
{
#ifdef SDL_PORT
    return 0;
#else
    HKEY hKey;
    DWORD installed;
    DWORD valueType;
    DWORD valueSize;
    char *path;

    path = 0;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      g_szOriginRegistryKey,
                      0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
        valueType = REG_SZ;
        valueSize = sizeof(installed);
        if (RegQueryValueExA(hKey, g_szWing1InstalledValueName, 0, &valueType,
                             (unsigned char *)&installed,
                             &valueSize) == ERROR_SUCCESS &&
            installed != 0) {
            valueType = REG_SZ;
            valueSize = sizeof(g_15_szLegacyGameDataPath);
            if (RegQueryValueExA(
                    hKey, g_szInstalledToValueName, 0, &valueType,
                    (unsigned char *)
                        g_15_szLegacyGameDataPath,
                    &valueSize) == ERROR_SUCCESS) {
                path = g_15_szLegacyGameDataPath;
                strcat(g_15_szLegacyGameDataPath,
                       "wc1\\gamedat");
            }
        }
        RegCloseKey(hKey);
    }
    return path;
#endif
}

/* Function start: 0x428D09 */
int ReadCheaterFlagFromRegistry(void)
{
    HKEY key;
    DWORD type;
    DWORD length;
    int enabled = 0;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      g_szOriginRegistryKey,
                      0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS) {
        type = REG_DWORD;
        length = sizeof(int);
        RegQueryValueExA(key, g_szCheaterValueName, 0, &type,
                         (LPBYTE)&enabled, &length);
        RegCloseKey(key);
    }
    if (enabled != 0)
        OutputDebugStringA("reg cheating\n");
    else
        OutputDebugStringA("reg not cheating\n");
    return enabled;
}

/* Function start: 0x428DA3 */
void SetMusicStreamVolume(unsigned short volume)
{
    ix_streamer_set_volume(volume);
    return;
}

/* Function start: 0x428DBF */
unsigned int GetMusicStreamVolume(void)
{
    return ix_streamer_get_volume();
}
