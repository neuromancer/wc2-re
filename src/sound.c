/*
 *  Wave playback, sound bookkeeping and volume settings.
 *
 *  Retail object 0x423CA0-0x424A5F (link order; see docs/ORDER.md).
 */
#include "game.h"

ActiveSoundEntry *g_pActiveSoundHead = 0; /* 0x004961a8 */
ActiveSoundEntry *g_pActiveSoundTail = 0; /* 0x004961ac */
int g_bAudioSystemInitialized = 0; /* 0x004961b0 */
WaveTableEntry *g_pWaveTableHead = 0; /* 0x004961b4 */
WaveTableEntry *g_pWaveTableTail = 0; /* 0x004961b8 */

/* Uninitialized C++ definitions: this unit's .bss, which LINK orders by the
 * identifiers' hash keys (the spellings follow the original addresses). */
WaveTableEntry *g_1_pVoiceWav; /* 0x004a2650 */
WaveTableEntry *g_pSnowStaticClip; /* 0x004a2654 */
IxSound *g_pSpeechSnd; /* 0x004a2658 */
int g_16_nSpeechCompletionDelay; /* 0x004a265c */
int g_bSpeechSoundActive; /* 0x004a2660 */
IxSound *g_9_pSnowStaticAudio; /* 0x004a2664 */
int g_9_nNextSfx16PlaybackFrame; /* 0x004a2668 */
int g_bSpeechPlaybackDone; /* 0x004a266c */

/* Function start: 0x423CA0 */
void InitializeAudioSystem(HWND window)
{
    if (g_nAudioEnabled == 0)
        return;
    if (g_bAudioSystemInitialized == 0) {
        ix_system_configure(3, (void *)1);
        ix_system_configure(0, window);
        ix_system_init();
        ix_system_set_voice_count(0x10);
        g_bAudioSystemInitialized = 1;
    }
}

/* Function start: 0x423D02 */
void ServiceAudioStream(void)
{
    if (g_nAudioEnabled == 0)
        return;
    if (g_bAudioSystemInitialized != 0) {
        ix_system_delete_all_sounds();
        ix_system_delete_all_samples();
        ix_system_shutdown();
        FreeWaveTable();
        g_bAudioSystemInitialized = 0;
    }
    return;
}

/* Function start: 0x423D4F */
WaveTableEntry *AllocateWaveTableEntry(void)
{
    if (g_pWaveTableHead == 0) {
        g_pWaveTableHead =
            (WaveTableEntry *)malloc(sizeof(WaveTableEntry));
        g_pWaveTableTail = g_pWaveTableHead;
    } else {
        g_pWaveTableTail->next =
            (WaveTableEntry *)malloc(sizeof(WaveTableEntry));
        g_pWaveTableTail = g_pWaveTableTail->next;
    }
    g_pWaveTableTail->next = 0;
    return g_pWaveTableTail;
}

#pragma function(strcmp)
/* Function start: 0x423DBB */
WaveTableEntry *FindWaveTableEntryByName(const char *name)
{
    WaveTableEntry *entry = g_pWaveTableHead;

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

    entry = g_pWaveTableHead;
    prev = 0;
    while (entry != 0 && entry != target) {
        prev = entry;
        entry = entry->next;
    }
    if (entry != 0) {
        if (prev != 0)
            prev->next = entry->next;
        if (entry == g_pWaveTableTail && prev != 0)
            g_pWaveTableTail = prev;
        if (entry == g_pWaveTableHead)
            g_pWaveTableHead = entry->next;
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

    entry = g_pWaveTableHead;
    next = 0;
    while (entry != 0) {
        next = entry->next;
        free(entry->name);
        free(entry);
        entry = next;
    }
    g_pWaveTableHead = g_pWaveTableTail = 0;
    return;
}

/* Function start: 0x423F3F */
ActiveSoundEntry *AllocateActiveSoundEntry(void)
{
    if (g_pActiveSoundHead == 0) {
        g_pActiveSoundHead =
            (ActiveSoundEntry *)malloc(sizeof(ActiveSoundEntry));
        g_pActiveSoundTail = g_pActiveSoundHead;
    } else {
        g_pActiveSoundTail->next =
            (ActiveSoundEntry *)malloc(sizeof(ActiveSoundEntry));
        g_pActiveSoundTail = g_pActiveSoundTail->next;
    }
    g_pActiveSoundTail->next = 0;
    return g_pActiveSoundTail;
}

/* Function start: 0x423FAB */
void RemoveActiveSoundEntry(ActiveSoundEntry *target)
{
    ActiveSoundEntry *entry = g_pActiveSoundHead;
    ActiveSoundEntry *prev = 0;

    while (entry != 0 && entry != target) {
        prev = entry;
        entry = entry->next;
    }
    if (entry != 0) {
        if (prev != 0)
            prev->next = entry->next;
        if (g_pActiveSoundTail == entry && prev != 0)
            g_pActiveSoundTail = prev;
        if (g_pActiveSoundHead == entry)
            g_pActiveSoundHead = entry->next;
        free(entry);
    }
}

/* Function start: 0x42405E */
ActiveSoundEntry *FindActiveSoundEntryBySample(IxSample *sample)
{
    ActiveSoundEntry *entry;

    entry = g_pActiveSoundHead;
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
    ActiveSoundEntry *entry = g_pActiveSoundHead;

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

    if (g_nAudioEnabled == 0)
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
        if (g_nSpaceFrame > g_9_nNextSfx16PlaybackFrame)
            g_9_nNextSfx16PlaybackFrame =
                g_nSpaceFrame + 100;
        else
            return;
#else
        /* This unit reads the frame counter as a dword. */
        if (*(int *)&g_nSpaceFrame >
            g_9_nNextSfx16PlaybackFrame)
            g_9_nNextSfx16PlaybackFrame =
                *(int *)&g_nSpaceFrame + 100;
        else
            return;
#endif
    }
    if (g_nAudioEnabled == 0)
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
    if (g_pSpeechSnd != 0 &&
        g_bSpeechSoundActive != 0) {
        if (interrupt != 0) {
            ix_sound_stop(g_pSpeechSnd);
#ifdef SDL_PORT
            /* This sound carries delete-on-stop, so the stop above has
               already freed it; the original released the dead block
               anyway. */
            if (ix_sound_is_live(g_pSpeechSnd))
                ix_sound_release(g_pSpeechSnd);
            g_pSpeechSnd = 0;
#else
            ix_sound_release(g_pSpeechSnd);
#endif
        } else
            return;
    }
    g_1_pVoiceWav = AllocateWaveTableEntry();
    g_1_pVoiceWav->sample = ix_system_new_sample();
    g_1_pVoiceWav->sample->ix_sample_load_raw(
        buffer, size, 0x29ab, 8, 1);
    g_pSpeechSnd = ix_system_new_sound(
        g_1_pVoiceWav->sample);
    g_pSpeechSnd->ix_sound_set_delete_on_stop(1);
    g_pSpeechSnd->ix_system_sound_set_volume(65000);
    ix_sound_start(g_pSpeechSnd);
    g_1_pVoiceWav->name =
        (char *)malloc(strlen("RAW") + 1);
    strcpy(g_1_pVoiceWav->name, "RAW");
    g_bSpeechSoundActive = 1;
    g_16_nSpeechCompletionDelay = 0;
    if (g_1_bSpaceFlightActive == 0)
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
    g_pSpeechSnd = 0;
    g_1_pVoiceWav = 0;
    g_bSpeechSoundActive = 0;
    g_16_nSpeechCompletionDelay = 0;
#endif
    if (g_9_pSnowStaticAudio != 0) {
        /* The bulk delete already stops, unlinks, and frees every IxSound.
           The original's following calls therefore use a stale pointer. */
#ifndef SDL_PORT
        ix_sound_stop(g_9_pSnowStaticAudio);
        ix_sound_release(g_9_pSnowStaticAudio);
#endif
        g_9_pSnowStaticAudio = 0;
        g_pSnowStaticClip = 0;
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
    if (g_nAudioEnabled == 0)
        return;
    ReleaseFinishedSoundEntries();
    if (g_9_pSnowStaticAudio == 0) {
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

        g_pSnowStaticClip = AllocateWaveTableEntry();
        g_pSnowStaticClip->sample = ix_system_new_sample();
        g_pSnowStaticClip->sample->ix_sample_load_wav(
            fileData, size);
        g_9_pSnowStaticAudio = ix_system_new_sound(
            g_pSnowStaticClip->sample);
        g_9_pSnowStaticAudio->ix_system_sound_set_volume(50000);
        ix_sound_start(g_9_pSnowStaticAudio);

        g_pSnowStaticClip->name =
            (char *)malloc(strlen("SNOW") + 1);
        strcpy(g_pSnowStaticClip->name, "SNOW");
        free(fileData);
    } else if (ix_sound_is_playing(
                   g_9_pSnowStaticAudio) == 0) {
        ix_sound_start(g_9_pSnowStaticAudio);
    }
}

/* Function start: 0x42476B */
void ServiceSoundSystem(void)
{
    if (g_nAudioEnabled == 0)
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
     * before touching g_pSpeechSnd, so a same-frame free
     * still runs the stopped-handling block below instead of being
     * skipped because the pointer already reads as 0. */
    int speechIsLive = g_pSpeechSnd != 0 &&
                        ix_sound_is_live(g_pSpeechSnd);
    if (g_pSpeechSnd != 0 &&
        (!speechIsLive || ix_sound_is_playing(g_pSpeechSnd) == 0) &&
        g_16_nSpeechCompletionDelay <= 20) {
        /* g_16_nSpeechCompletionDelay gates re-entry instead of
         * g_bSpeechSoundActive: it resets to 0 in
         * PlayRawSpeechBuffer whenever a new clip starts, so this
         * block still only fires once per stopped clip, but the reset
         * below can run on the first frame audio is observed stopped
         * rather than waiting for the full grace period. */
        if (g_16_nSpeechCompletionDelay == 0) {
            /* The cutscene script polls g_2_bMovieStringAdvanceState
             * (opcode 0x9c, screens.c) to know when speech has
             * finished before advancing to the next line, so clearing
             * these here -- immediately on stop, not after the grace
             * period below -- keeps both the mouth and the next
             * line's start in sync with when audio actually ends. */
            g_bSpeechSoundActive = 0;
            if (g_1_bSpaceFlightActive == 0) {
                g_bCutsceneSpeechActive = 0;
                g_2_bMovieStringAdvanceState = 0;
                g_pszCutsceneSpeechCursor = 0;
                /* g_pCurrentCutsceneSprite is whichever
                 * object happens to be executing its own script
                 * opcodes at the moment this runs (set/restored
                 * around every sprite/plane's dispatch) -- not
                 * necessarily the speaker. g_pLinkedCutsceneSprite_
                 * 00499c64 is the sprite opcode 0x8a actually armed
                 * for mouth animation, the same one
                 * AnimateCutsceneSpeakerMouth (screens.c) itself
                 * resets to frame 11 on its own early-return paths. */
                if (g_pLinkedCutsceneSprite != 0) {
                    g_pLinkedCutsceneSprite->currentFrame = 11;
                    g_pLinkedCutsceneSprite->waitTicks = 0;
                }
            }
        }
        g_16_nSpeechCompletionDelay++;
        if (g_1_bSpaceFlightActive != 0) {
            g_bSpeechPlaybackDone = 1;
        }
        if (g_16_nSpeechCompletionDelay > 20) {
            if (g_1_bSpaceFlightActive == 0)
                g_nInputPressCount = 1;
        }
    }
    if (!speechIsLive)
        g_pSpeechSnd = 0;
#else
    if (g_pSpeechSnd != 0 &&
        ix_sound_is_playing(g_pSpeechSnd) == 0 &&
        g_bSpeechSoundActive != 0) {
        g_16_nSpeechCompletionDelay++;
        if (g_1_bSpaceFlightActive != 0) {
            g_bSpeechPlaybackDone = 1;
            g_bSpeechSoundActive = 0;
        }
        if (g_16_nSpeechCompletionDelay > 20) {
            if (g_1_bSpaceFlightActive == 0)
                g_nInputPressCount = 1;
            g_bSpeechSoundActive = 0;
            if (g_1_bSpaceFlightActive == 0)
                SetCinematicFrameTiming(70.0f);
        }
    }
#endif
}

/* Function start: 0x42482E */
void SetSoundEffectsVolume(int volume)
{
    if (g_nAudioEnabled == 0)
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
static char g_szMusicVolumeValueName[] = "MusicVolume"; /* 0x004961c0 */
static char g_szSfxVolumeValueName[] = "SFXVolume"; /* 0x004961d0 */

/* Function start: 0x4248B4 */
void LoadVolumeSettingsFromRegistry(void)
{
    HKEY key;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "Software\\Origin Systems\\WC: Kilrathi Saga",
                      0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS) {
        if (RegistryQueryValue(key, g_szMusicVolumeValueName, REG_DWORD,
                               (LPBYTE)&g_nMusicVolumeSetting,
                               sizeof(g_nMusicVolumeSetting)) !=
            ERROR_SUCCESS) {
            g_nMusicVolumeSetting = 0x14;
            RegistryStoreValue(key, g_szMusicVolumeValueName, REG_DWORD,
                               (const BYTE *)&g_nMusicVolumeSetting,
                               sizeof(g_nMusicVolumeSetting));
        }
        if (RegistryQueryValue(key, g_szSfxVolumeValueName, REG_DWORD,
                               (LPBYTE)&g_nSfxVolumeSetting,
                               sizeof(g_nSfxVolumeSetting)) !=
            ERROR_SUCCESS) {
            g_nSfxVolumeSetting = 0x14;
            RegistryStoreValue(key, g_szSfxVolumeValueName, REG_DWORD,
                               (const BYTE *)&g_nSfxVolumeSetting,
                               sizeof(g_nSfxVolumeSetting));
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
        RegistryStoreValue(key, g_szMusicVolumeValueName, REG_DWORD,
                           (const BYTE *)&g_nMusicVolumeSetting,
                           sizeof(g_nMusicVolumeSetting));
        RegistryStoreValue(key, g_szSfxVolumeValueName, REG_DWORD,
                           (const BYTE *)&g_nSfxVolumeSetting,
                           sizeof(g_nSfxVolumeSetting));
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
