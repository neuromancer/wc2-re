/*
 *  Wave playback, sound bookkeeping and volume settings.
 *
 *  Retail object 0x423CA0-0x424A5F (link order; see docs/ORDER.md).
 */
#include "game.h"

ActiveSoundEntry *g_pActiveSoundHead_004961a8 = 0;
ActiveSoundEntry *g_pActiveSoundTail_004961ac = 0;
int g_bAudioSystemInitialized_004961b0 = 0;
WaveTableEntry *g_pWaveTableHead_004961b4 = 0;
WaveTableEntry *g_pWaveTableTail_004961b8 = 0;

/* Uninitialized C++ definitions: this unit's .bss, which LINK orders by the
 * identifiers' hash keys (the spellings follow the original addresses). */
WaveTableEntry *g_pVoiceWav_004a2650;
WaveTableEntry *g_pSnowStaticClip_004a2654;
IxSound *g_pSpeechSnd_004a2658;
int g_nSpeechCompletionDelay_004a265c;
int g_bSpeechSoundActive_004a2660;
IxSound *g_pSnowStaticAudio_004a2664;
int g_nNextSfx16PlaybackFrame_004a2668;
int g_bSpeechPlaybackDone_004a266c;

/* Function start: 0x423CA0 */
void InitializeAudioSystem(HWND window)
{
    if (g_nAudioEnabled_0049c244 == 0)
        return;
    if (g_bAudioSystemInitialized_004961b0 == 0) {
        ix_system_configure(3, (void *)1);
        ix_system_configure(0, window);
        ix_system_init();
        ix_system_set_voice_count(0x10);
        g_bAudioSystemInitialized_004961b0 = 1;
    }
}

/* Function start: 0x423D02 */
void ServiceAudioStream(void)
{
    if (g_nAudioEnabled_0049c244 == 0)
        return;
    if (g_bAudioSystemInitialized_004961b0 != 0) {
        ix_system_delete_all_sounds();
        ix_system_delete_all_samples();
        ix_system_shutdown();
        FreeWaveTable();
        g_bAudioSystemInitialized_004961b0 = 0;
    }
    return;
}

/* Function start: 0x423D4F */
WaveTableEntry *AllocateWaveTableEntry(void)
{
    if (g_pWaveTableHead_004961b4 == 0) {
        g_pWaveTableHead_004961b4 =
            (WaveTableEntry *)malloc(sizeof(WaveTableEntry));
        g_pWaveTableTail_004961b8 = g_pWaveTableHead_004961b4;
    } else {
        g_pWaveTableTail_004961b8->next =
            (WaveTableEntry *)malloc(sizeof(WaveTableEntry));
        g_pWaveTableTail_004961b8 = g_pWaveTableTail_004961b8->next;
    }
    g_pWaveTableTail_004961b8->next = 0;
    return g_pWaveTableTail_004961b8;
}

#pragma function(strcmp)
/* Function start: 0x423DBB */
WaveTableEntry *FindWaveTableEntryByName(const char *name)
{
    WaveTableEntry *entry = g_pWaveTableHead_004961b4;

    while (entry != 0) {
        if (strcmp(entry->name, name) == 0)
            return entry;
        entry = entry->next;
    }
    return 0;
}

/* Sound-object callees from the ix library. */
int __fastcall ix_sound_is_playing(IxSound *sound);
void ix_system_delete_sound(IxSound *sound);

/* Function start: 0x423E12 */
void RemoveWaveTableEntry(WaveTableEntry *target)
{
    WaveTableEntry *entry;
    WaveTableEntry *prev;

    entry = g_pWaveTableHead_004961b4;
    prev = 0;
    while (entry != 0 && entry != target) {
        prev = entry;
        entry = entry->next;
    }
    if (entry != 0) {
        if (prev != 0)
            prev->next = entry->next;
        if (entry == g_pWaveTableTail_004961b8 && prev != 0)
            g_pWaveTableTail_004961b8 = prev;
        if (entry == g_pWaveTableHead_004961b4)
            g_pWaveTableHead_004961b4 = entry->next;
        free(entry->name);
        free(entry);
    }
    return;
}

/* Function start: 0x423ED1 */
void FreeWaveTable(void)
{
    WaveTableEntry *entry;
    WaveTableEntry *next;

    entry = g_pWaveTableHead_004961b4;
    next = 0;
    while (entry != 0) {
        next = entry->next;
        free(entry->name);
        free(entry);
        entry = next;
    }
    g_pWaveTableHead_004961b4 = g_pWaveTableTail_004961b8 = 0;
    return;
}

/* Function start: 0x423F3F */
ActiveSoundEntry *AllocateActiveSoundEntry(void)
{
    if (g_pActiveSoundHead_004961a8 == 0) {
        g_pActiveSoundHead_004961a8 =
            (ActiveSoundEntry *)malloc(sizeof(ActiveSoundEntry));
        g_pActiveSoundTail_004961ac = g_pActiveSoundHead_004961a8;
    } else {
        g_pActiveSoundTail_004961ac->next =
            (ActiveSoundEntry *)malloc(sizeof(ActiveSoundEntry));
        g_pActiveSoundTail_004961ac = g_pActiveSoundTail_004961ac->next;
    }
    g_pActiveSoundTail_004961ac->next = 0;
    return g_pActiveSoundTail_004961ac;
}

/* Function start: 0x423FAB */
void RemoveActiveSoundEntry(ActiveSoundEntry *target)
{
    ActiveSoundEntry *entry = g_pActiveSoundHead_004961a8;
    ActiveSoundEntry *prev = 0;

    while (entry != 0 && entry != target) {
        prev = entry;
        entry = entry->next;
    }
    if (entry != 0) {
        if (prev != 0)
            prev->next = entry->next;
        if (g_pActiveSoundTail_004961ac == entry && prev != 0)
            g_pActiveSoundTail_004961ac = prev;
        if (g_pActiveSoundHead_004961a8 == entry)
            g_pActiveSoundHead_004961a8 = entry->next;
        free(entry);
    }
}

/* Function start: 0x42405E */
ActiveSoundEntry *FindActiveSoundEntryBySample(IxSample *sample)
{
    ActiveSoundEntry *entry;

    entry = g_pActiveSoundHead_004961a8;
    while (entry != 0) {
        if (ix_sound_get_sample(entry->sound) == sample)
            return entry;
        entry = entry->next;
    }
    return 0;
}

/* Function start: 0x4240AE */
void ReleaseFinishedSoundEntries(void)
{
    ActiveSoundEntry *entry = g_pActiveSoundHead_004961a8;

    while (entry != 0) {
        ActiveSoundEntry *next = entry->next;

        if (ix_sound_is_playing(entry->sound) == 0) {
            ix_system_delete_sound(entry->sound);
            RemoveActiveSoundEntry(entry);
        }
        entry = next;
    }
}

/* Function start: 0x424113 */
void StopSoundsUsingWave(const char *name)
{
    WaveTableEntry *waveEntry;
    ActiveSoundEntry *entry;

    if (g_nAudioEnabled_0049c244 == 0)
        return;
    waveEntry = FindWaveTableEntryByName(name);
    if (waveEntry != 0) {
        while ((entry = FindActiveSoundEntryBySample(waveEntry->sample)) != 0) {
            ix_system_delete_sound(entry->sound);
            RemoveActiveSoundEntry(entry);
        }
    }
}

#pragma function(memcmp, strcpy, strlen)
/* Function start: 0x42418C */
void playWAVE(const char *filename, int looping, int volume)
{
    char errorMsg[100];
    long size;
    IxSound *voice;
    int fd;
    unsigned char *fileData;
    WaveTableEntry *wave;

    if (memcmp(filename, "sfx16.wav", 9) == 0) {
#ifdef SDL_PORT
        if (g_nSpaceFrame_00493134 > g_nNextSfx16PlaybackFrame_004a2668)
            g_nNextSfx16PlaybackFrame_004a2668 =
                g_nSpaceFrame_00493134 + 100;
        else
            return;
#else
        /* This unit reads the frame counter as a dword. */
        if (*(int *)&g_nSpaceFrame_00493134 >
            g_nNextSfx16PlaybackFrame_004a2668)
            g_nNextSfx16PlaybackFrame_004a2668 =
                *(int *)&g_nSpaceFrame_00493134 + 100;
        else
            return;
#endif
    }
    if (g_nAudioEnabled_0049c244 == 0)
        return;
    volume /= 2;
    ReleaseFinishedSoundEntries();
    SoundDebugPrintf("%s\n", filename);
    wave = FindWaveTableEntryByName(filename);
    if (wave != 0) {
        if (looping != 0) {
            ActiveSoundEntry *active;

            active = AllocateActiveSoundEntry();
            active->sound = ix_system_new_sound(wave->sample);
            active->sound->ix_system_sound_set_volume(volume);
            ix_sound_start(active->sound);
        } else {
            voice = ix_system_new_sound(wave->sample);
            voice->ix_sound_set_delete_on_stop(1);
            voice->ix_system_sound_set_volume(volume);
            ix_sound_start(voice);
        }
    } else {
        fd = _open(filename, 0x8000);
        if (fd == -1) {
            sprintf(errorMsg, "playWAVE Unable to open file '%s'", filename);
            MessageBoxA(0, errorMsg, "Error", MB_ICONHAND);
            exit(1);
        }
        size = filelength(fd);
        fileData = (unsigned char *)malloc((unsigned int)size);
        _read(fd, fileData, (unsigned int)size);
        _close(fd);

        wave = AllocateWaveTableEntry();
        wave->sample = ix_system_new_sample();
        wave->sample->ix_sample_load_wav(fileData, size);
        if (looping != 0) {
            ActiveSoundEntry *active;

            wave->sample->ix_sample_set_looping(1);
            active = AllocateActiveSoundEntry();
            active->sound = ix_system_new_sound(wave->sample);
            active->sound->ix_system_sound_set_volume(volume);
            ix_sound_start(active->sound);
        } else {
            voice = ix_system_new_sound(wave->sample);
            voice->ix_sound_set_delete_on_stop(1);
            voice->ix_system_sound_set_volume(volume);
            ix_sound_start(voice);
        }
        wave->name = (char *)malloc(strlen(filename) + 1);
        strcpy(wave->name, filename);
        free(fileData);
    }
    return;
}

/* Function start: 0x424417 */
void PlayRawSpeechBuffer(void *buffer, size_t size, int interrupt)
{
    int index;

    if (buffer == 0)
        return;
    for (index = 0; index < (int)size; index++) {
        ((unsigned char *)buffer)[index] =
            (unsigned char)(((unsigned char *)buffer)[index] + 0x80);
    }
    SoundDebugPrintf(
        "Playing RAW with length of %d stop=%d\n", size, interrupt);
    if (g_pSpeechSnd_004a2658 != 0 &&
        g_bSpeechSoundActive_004a2660 != 0) {
        if (interrupt != 0) {
            ix_sound_stop(g_pSpeechSnd_004a2658);
#ifdef SDL_PORT
            /* This sound carries delete-on-stop, so the stop above has
               already freed it; the original released the dead block
               anyway. */
            if (ix_sound_is_live(g_pSpeechSnd_004a2658))
                ix_sound_release(g_pSpeechSnd_004a2658);
            g_pSpeechSnd_004a2658 = 0;
#else
            ix_sound_release(g_pSpeechSnd_004a2658);
#endif
        } else
            return;
    }
    g_pVoiceWav_004a2650 = AllocateWaveTableEntry();
    g_pVoiceWav_004a2650->sample = ix_system_new_sample();
    g_pVoiceWav_004a2650->sample->ix_sample_load_raw(
        buffer, size, 0x29ab, 8, 1);
    g_pSpeechSnd_004a2658 = ix_system_new_sound(
        g_pVoiceWav_004a2650->sample);
    g_pSpeechSnd_004a2658->ix_sound_set_delete_on_stop(1);
    g_pSpeechSnd_004a2658->ix_system_sound_set_volume(65000);
    ix_sound_start(g_pSpeechSnd_004a2658);
    g_pVoiceWav_004a2650->name =
        (char *)malloc(strlen("RAW") + 1);
    strcpy(g_pVoiceWav_004a2650->name, "RAW");
    g_bSpeechSoundActive_004a2660 = 1;
    g_nSpeechCompletionDelay_004a265c = 0;
    if (g_bSpaceFlightActive_005c586c == 0)
        SetCinematicFrameTiming(20.0f);
}

/* Function start: 0x4245A2 */
void stop_all_sounds(void)
{
#ifdef SDL_PORT
    SdlStopDosSoundEffects();
#endif
    ix_system_delete_all_sounds();
    ix_system_delete_all_samples();
    FreeWaveTable();
#ifdef SDL_PORT
    g_pSpeechSnd_004a2658 = 0;
    g_pVoiceWav_004a2650 = 0;
    g_bSpeechSoundActive_004a2660 = 0;
    g_nSpeechCompletionDelay_004a265c = 0;
#endif
    if (g_pSnowStaticAudio_004a2664 != 0) {
        /* The bulk delete already stops, unlinks, and frees every IxSound.
           The original's following calls therefore use a stale pointer. */
#ifndef SDL_PORT
        ix_sound_stop(g_pSnowStaticAudio_004a2664);
        ix_sound_release(g_pSnowStaticAudio_004a2664);
#endif
        g_pSnowStaticAudio_004a2664 = 0;
        g_pSnowStaticClip_004a2654 = 0;
    }
}

/* Function start: 0x4245F8 */
void PlaySnowStaticSound(void)
{
    unsigned char *fileData;
    long size;
    int file;

#ifdef SDL_PORT
    /* DOS data has no sampled static sound. */
    if (SdlUsingOriginFxSoundEffects())
        return;
#endif
    if (g_nAudioEnabled_0049c244 == 0)
        return;
    ReleaseFinishedSoundEntries();
    if (g_pSnowStaticAudio_004a2664 == 0) {
        file = _open("sfx22.wav", 0x8000);
        if (file == -1) {
            MessageBoxA(0,
                        "playWAVE Unable to open file 'sfx23",
                        "Notice", MB_ICONHAND);
            exit(1);
        }
        size = filelength(file);
        fileData = (unsigned char *)malloc((unsigned int)size);
        _read(file, fileData, (unsigned int)size);
        _close(file);

        g_pSnowStaticClip_004a2654 = AllocateWaveTableEntry();
        g_pSnowStaticClip_004a2654->sample = ix_system_new_sample();
        g_pSnowStaticClip_004a2654->sample->ix_sample_load_wav(
            fileData, size);
        g_pSnowStaticAudio_004a2664 = ix_system_new_sound(
            g_pSnowStaticClip_004a2654->sample);
        g_pSnowStaticAudio_004a2664->ix_system_sound_set_volume(50000);
        ix_sound_start(g_pSnowStaticAudio_004a2664);

        g_pSnowStaticClip_004a2654->name =
            (char *)malloc(strlen("SNOW") + 1);
        strcpy(g_pSnowStaticClip_004a2654->name, "SNOW");
        free(fileData);
    } else if (ix_sound_is_playing(
                   g_pSnowStaticAudio_004a2664) == 0) {
        ix_sound_start(g_pSnowStaticAudio_004a2664);
    }
}

/* Function start: 0x42476B */
void ServiceSoundSystem(void)
{
    if (g_nAudioEnabled_0049c244 == 0)
        return;
    ix_system_service_sounds();
#ifdef SDL_PORT
    /* This whole block is a port-only rewrite of the original's
     * speech-completion handling (see the #else branch for that);
     * kept behind SDL_PORT so the reference build stays byte-
     * comparable to the original for verification.
     *
     * The speech sound is delete-on-stop, so the call above can have
     * freed it already this frame. Evaluate liveness into a local
     * before touching g_pSpeechSnd_004a2658, so a same-frame free
     * still runs the stopped-handling block below instead of being
     * skipped because the pointer already reads as 0. */
    int speechIsLive = g_pSpeechSnd_004a2658 != 0 &&
                        ix_sound_is_live(g_pSpeechSnd_004a2658);
    if (g_pSpeechSnd_004a2658 != 0 &&
        (!speechIsLive || ix_sound_is_playing(g_pSpeechSnd_004a2658) == 0) &&
        g_nSpeechCompletionDelay_004a265c <= 20) {
        /* g_nSpeechCompletionDelay_004a265c gates re-entry instead of
         * g_bSpeechSoundActive_004a2660: it resets to 0 in
         * PlayRawSpeechBuffer whenever a new clip starts, so this
         * block still only fires once per stopped clip, but the reset
         * below can run on the first frame audio is observed stopped
         * rather than waiting for the full grace period. */
        if (g_nSpeechCompletionDelay_004a265c == 0) {
            /* The cutscene script polls g_bMovieStringAdvanceState_005d2ed0
             * (opcode 0x9c, screens.c) to know when speech has
             * finished before advancing to the next line, so clearing
             * these here -- immediately on stop, not after the grace
             * period below -- keeps both the mouth and the next
             * line's start in sync with when audio actually ends. */
            g_bSpeechSoundActive_004a2660 = 0;
            if (g_bSpaceFlightActive_005c586c == 0) {
                g_bCutsceneSpeechActive_00499eb8 = 0;
                g_bMovieStringAdvanceState_005d2ed0 = 0;
                g_pszCutsceneSpeechCursor_00499eb0 = 0;
                /* g_pCurrentCutsceneSprite_00499c78 is whichever
                 * object happens to be executing its own script
                 * opcodes at the moment this runs (set/restored
                 * around every sprite/plane's dispatch) -- not
                 * necessarily the speaker. g_pLinkedCutsceneSprite_
                 * 00499c64 is the sprite opcode 0x8a actually armed
                 * for mouth animation, the same one
                 * AnimateCutsceneSpeakerMouth (screens.c) itself
                 * resets to frame 11 on its own early-return paths. */
                if (g_pLinkedCutsceneSprite_00499c64 != 0) {
                    g_pLinkedCutsceneSprite_00499c64->currentFrame = 11;
                    g_pLinkedCutsceneSprite_00499c64->waitTicks = 0;
                }
            }
        }
        g_nSpeechCompletionDelay_004a265c++;
        if (g_bSpaceFlightActive_005c586c != 0) {
            g_bSpeechPlaybackDone_004a266c = 1;
        }
        if (g_nSpeechCompletionDelay_004a265c > 20) {
            if (g_bSpaceFlightActive_005c586c == 0)
                g_nInputPressCount_0049c258 = 1;
        }
    }
    if (!speechIsLive)
        g_pSpeechSnd_004a2658 = 0;
#else
    if (g_pSpeechSnd_004a2658 != 0 &&
        ix_sound_is_playing(g_pSpeechSnd_004a2658) == 0 &&
        g_bSpeechSoundActive_004a2660 != 0) {
        g_nSpeechCompletionDelay_004a265c++;
        if (g_bSpaceFlightActive_005c586c != 0) {
            g_bSpeechPlaybackDone_004a266c = 1;
            g_bSpeechSoundActive_004a2660 = 0;
        }
        if (g_nSpeechCompletionDelay_004a265c > 20) {
            if (g_bSpaceFlightActive_005c586c == 0)
                g_nInputPressCount_0049c258 = 1;
            g_bSpeechSoundActive_004a2660 = 0;
            if (g_bSpaceFlightActive_005c586c == 0)
                SetCinematicFrameTiming(70.0f);
        }
    }
#endif
}

/* Function start: 0x42482E */
void SetSoundEffectsVolume(int volume)
{
    if (g_nAudioEnabled_0049c244 == 0)
        return;
    ix_system_set_master_volume((unsigned short)volume);
}

/* Function start: 0x42485C */
LONG RegistryQueryValue(HKEY key, LPCSTR name, DWORD type,
                        LPBYTE data, DWORD size)
{
    return RegQueryValueExA(key, name, 0, &type, data, &size);
}

/* Function start: 0x424888 */
void RegistryStoreValue(HKEY key, LPCSTR name, DWORD type,
                        const BYTE *data, DWORD size)
{
    RegSetValueExA(key, name, 0, type, data, size);
}

/* One copy of each registry value name serves both the load and the save:
 * the original has a single address for each. */
static char g_szMusicVolumeValueName_004961c0[] = "MusicVolume";
static char g_szSfxVolumeValueName_004961d0[] = "SFXVolume";

/* Function start: 0x4248B4 */
void LoadVolumeSettingsFromRegistry(void)
{
    HKEY key;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "Software\\Origin Systems\\WC: Kilrathi Saga",
                      0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS) {
        if (RegistryQueryValue(key, g_szMusicVolumeValueName_004961c0, REG_DWORD,
                               (LPBYTE)&g_nMusicVolumeSetting_0049d750,
                               sizeof(g_nMusicVolumeSetting_0049d750)) !=
            ERROR_SUCCESS) {
            g_nMusicVolumeSetting_0049d750 = 0x14;
            RegistryStoreValue(key, g_szMusicVolumeValueName_004961c0, REG_DWORD,
                               (const BYTE *)&g_nMusicVolumeSetting_0049d750,
                               sizeof(g_nMusicVolumeSetting_0049d750));
        }
        if (RegistryQueryValue(key, g_szSfxVolumeValueName_004961d0, REG_DWORD,
                               (LPBYTE)&g_nSfxVolumeSetting_0049d74c,
                               sizeof(g_nSfxVolumeSetting_0049d74c)) !=
            ERROR_SUCCESS) {
            g_nSfxVolumeSetting_0049d74c = 0x14;
            RegistryStoreValue(key, g_szSfxVolumeValueName_004961d0, REG_DWORD,
                               (const BYTE *)&g_nSfxVolumeSetting_0049d74c,
                               sizeof(g_nSfxVolumeSetting_0049d74c));
        }
    }
    RegCloseKey(key);
    return;
}

/* Function start: 0x424980 */
void SaveVolumeSettingsToRegistry(void)
{
    HKEY key;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "Software\\Origin Systems\\WC: Kilrathi Saga",
                      0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS) {
        RegistryStoreValue(key, g_szMusicVolumeValueName_004961c0, REG_DWORD,
                           (const BYTE *)&g_nMusicVolumeSetting_0049d750,
                           sizeof(g_nMusicVolumeSetting_0049d750));
        RegistryStoreValue(key, g_szSfxVolumeValueName_004961d0, REG_DWORD,
                           (const BYTE *)&g_nSfxVolumeSetting_0049d74c,
                           sizeof(g_nSfxVolumeSetting_0049d74c));
    }
    RegCloseKey(key);
}

/* Function start: 0x424A00 */
void IxSample::ix_sample_set_looping(int enabled)
{
    if (enabled != 0)
        flags |= 2;
    else
        flags &= ~2U;
}

/* The inline accessor called by FindActiveSoundEntryBySample. Its declaration
 * makes the debug compiler emit a separate COMDAT, matching the aligned
 * out-of-line copy after this unit's main code. */
/* Function start: 0x424A40 */
IxSample * __fastcall ix_sound_get_sample(IxSound *sound)
{
    return sound->sample;
}
