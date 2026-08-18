/*
 *  Wave playback, volume settings and INSTALL.DAT.
 *
 *  Address range 0x42b400-0x42cfff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: playWAVE/PlaySnowStaticSound/LoadInstallDat; string band 0x46A46C-0x46A710.
 */
#include "wc1.h"

#ifndef WC1_SDL
#pragma function(memcmp)
#pragma function(strcpy)
#endif

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
    WaveTableEntry *wave;
    ActiveSoundEntry *entry;

    if (g_nAudioEnabled_0049c244 != 0) {
        wave = FindWaveTableEntryByName(name);
        if (wave != 0) {
            for (;;) {
                entry = FindActiveSoundEntryBySample(wave->sample);
                if (entry == 0)
                    break;
                ix_system_delete_sound(entry->sound);
                RemoveActiveSoundEntry(entry);
            }
        }
    }
}

/* Function start: 0x42418C */
void playWAVE(const char *filename, int looping, int volume)
{
#if 0
    WaveTableEntry *wave;
    ActiveSoundEntry *active;
    IxSound *sound;
    unsigned char *fileData;
    long fileSize;
    int file;

    if (g_nAudioEnabled_0049c244 != 0) {
        ReleaseFinishedSoundEntries();
        wave = FindWaveTableEntryByName(filename);
        if (wave != 0) {
            if (looping != 0) {
                active = AllocateActiveSoundEntry();
                active->sound = ix_system_new_sound(wave->sample);
                active->sound->ix_system_sound_set_volume(volume);
                ix_sound_start(active->sound);
                return;
            }
            sound = ix_system_new_sound(wave->sample);
            sound->ix_sound_set_delete_on_stop(1);
            sound->ix_system_sound_set_volume(volume);
            ix_sound_start(sound);
            return;
        }

        file = _open(filename, 0x8000);
        if (file == -1) {
            MessageBoxA(0, g_szPlayWaveOpenError_004961ec,
                        filename, MB_ICONHAND);
            _exit(1);
        }
        fileSize = _filelength(file);
        fileData = (unsigned char *)malloc((unsigned int)fileSize);
        _read(file, fileData, (unsigned int)fileSize);
        _close(file);

        wave = AllocateWaveTableEntry();
        wave->sample = ix_system_new_sample();
        wave->sample->ix_sample_load_wav(fileData, fileSize);
        if (looping != 0) {
            wave->sample->flags |= 2;
            active = AllocateActiveSoundEntry();
            active->sound = ix_system_new_sound(wave->sample);
            active->sound->ix_system_sound_set_volume(volume);
            sound = active->sound;
        } else {
            sound = ix_system_new_sound(wave->sample);
            sound->ix_sound_set_delete_on_stop(1);
            sound->ix_system_sound_set_volume(volume);
        }
        ix_sound_start(sound);

        wave->name = (char *)malloc(strlen(filename) + 1);
        strcpy(wave->name, filename);
        free(fileData);
    }
#else
    char error[100];
    WaveTableEntry *wave;
    ActiveSoundEntry *active;
    IxSound *sound;
    unsigned char *fileData;
    long fileSize;
    int file;

    if (memcmp(filename, "sfx16.wav", 9) == 0) {
        if (g_nSpaceFrame_00493134 <= g_nNextSfx16PlaybackFrame_004a2668)
            return;
        g_nNextSfx16PlaybackFrame_004a2668 =
            g_nSpaceFrame_00493134 + 100;
    }
    if (g_nAudioEnabled_0049c244 == 0)
        return;
    volume /= 2;
    ReleaseFinishedSoundEntries();
    SoundDebugPrintf("%s\n", filename);
    wave = FindWaveTableEntryByName(filename);
    if (wave != 0) {
        if (looping != 0) {
            active = AllocateActiveSoundEntry();
            active->sound = ix_system_new_sound(wave->sample);
            active->sound->ix_system_sound_set_volume(volume);
            ix_sound_start(active->sound);
        } else {
            sound = ix_system_new_sound(wave->sample);
            sound->ix_sound_set_delete_on_stop(1);
            sound->ix_system_sound_set_volume(volume);
            ix_sound_start(sound);
        }
        return;
    }

    file = _open(filename, 0x8000);
    if (file == -1) {
        sprintf(error, "playWAVE Unable to open file '%s'", filename);
        MessageBoxA(0, error, "Error", MB_ICONHAND);
        _exit(1);
    }
    fileSize = _filelength(file);
    fileData = (unsigned char *)malloc((unsigned int)fileSize);
    _read(file, fileData, (unsigned int)fileSize);
    _close(file);

    wave = AllocateWaveTableEntry();
    wave->sample = ix_system_new_sample();
    wave->sample->ix_sample_load_wav(fileData, fileSize);
    if (looping == 0) {
        sound = ix_system_new_sound(wave->sample);
        sound->ix_sound_set_delete_on_stop(1);
        sound->ix_system_sound_set_volume(volume);
        ix_sound_start(sound);
    } else {
        wave->sample->ix_sample_set_looping(1);
        active = AllocateActiveSoundEntry();
        active->sound = ix_system_new_sound(wave->sample);
        active->sound->ix_system_sound_set_volume(volume);
        ix_sound_start(active->sound);
    }
    wave->name = (char *)malloc(strlen(filename) + 1);
    strcpy(wave->name, filename);
    free(fileData);
#endif
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
    if (g_pSpeechSound_004a2658 != 0 &&
        g_bSpeechSoundActive_004a2660 != 0) {
        if (interrupt == 0)
            return;
        ix_sound_stop(g_pSpeechSound_004a2658);
        ix_sound_release(g_pSpeechSound_004a2658);
    }
    g_pSpeechWave_004a2650 = AllocateWaveTableEntry();
    g_pSpeechWave_004a2650->sample = ix_system_new_sample();
    g_pSpeechWave_004a2650->sample->ix_sample_load_raw(
        buffer, size, 0x29ab, 8, 1);
    g_pSpeechSound_004a2658 = ix_system_new_sound(
        g_pSpeechWave_004a2650->sample);
    g_pSpeechSound_004a2658->ix_sound_set_delete_on_stop(1);
    g_pSpeechSound_004a2658->ix_system_sound_set_volume(65000);
    ix_sound_start(g_pSpeechSound_004a2658);
    g_pSpeechWave_004a2650->name =
        (char *)malloc(strlen("RAW") + 1);
    strcpy(g_pSpeechWave_004a2650->name, "RAW");
    g_bSpeechSoundActive_004a2660 = 1;
    g_nSpeechCompletionDelay_004a265c = 0;
    if (g_bSpaceFlightActive_005c586c == 0)
        SetCinematicFrameTiming(20.0f);
}

/* Function start: 0x4245A2 */
void stop_all_sounds(void)
{
#ifdef WC1_SDL
    Wc1SdlStopDosSoundEffects();
#endif
    ix_system_delete_all_sounds();
    ix_system_delete_all_samples();
    FreeWaveTable();
    if (g_pSnowStaticSound_00476550 != 0) {
        /* The bulk delete already stops, unlinks, and frees every IxSound.
           The original's following calls therefore use a stale pointer. */
#ifndef WC1_SDL
        ix_sound_stop(g_pSnowStaticSound_00476550);
        ix_sound_release(g_pSnowStaticSound_00476550);
#endif
        g_pSnowStaticSound_00476550 = 0;
        g_pSnowStaticWave_0047654c = 0;
    }
}

/* Function start: 0x4245F8 */
void PlaySnowStaticSound(void)
{
    unsigned char *fileData;
    long fileSize;
    int file;

    if (g_nAudioEnabled_0049c244 != 0) {
        ReleaseFinishedSoundEntries();
        if (g_pSnowStaticSound_00476550 == 0) {
            file = _open("sfx22.wav", 0x8000);
            if (file == -1) {
                MessageBoxA(0,
                            "playWAVE Unable to open file 'sfx23",
                            "Notice", MB_ICONHAND);
                _exit(1);
            }
            fileSize = _filelength(file);
            fileData = (unsigned char *)malloc((unsigned int)fileSize);
            _read(file, fileData, (unsigned int)fileSize);
            _close(file);

            g_pSnowStaticWave_0047654c = AllocateWaveTableEntry();
            g_pSnowStaticWave_0047654c->sample = ix_system_new_sample();
            g_pSnowStaticWave_0047654c->sample->ix_sample_load_wav(
                fileData, fileSize);
            g_pSnowStaticSound_00476550 = ix_system_new_sound(
                g_pSnowStaticWave_0047654c->sample);
            g_pSnowStaticSound_00476550->ix_system_sound_set_volume(50000);
            ix_sound_start(g_pSnowStaticSound_00476550);

            g_pSnowStaticWave_0047654c->name =
                (char *)malloc(strlen("SNOW") + 1);
            strcpy(g_pSnowStaticWave_0047654c->name, "SNOW");
            free(fileData);
        } else if (ix_sound_is_playing(
                       g_pSnowStaticSound_00476550) == 0) {
            ix_sound_start(g_pSnowStaticSound_00476550);
        }
    }
}

/* Function start: 0x42476B */
void ServiceSoundSystem(void)
{
#if 0
    ix_system_service_sounds();
#else
    if (g_nAudioEnabled_0049c244 != 0) {
        ix_system_service_sounds();
        if (g_pSpeechSound_004a2658 != 0 &&
            ix_sound_is_playing(g_pSpeechSound_004a2658) == 0 &&
            g_bSpeechSoundActive_004a2660 != 0) {
            g_nSpeechCompletionDelay_004a265c++;
            if (g_bSpaceFlightActive_005c586c != 0) {
                g_bSpeechPlaybackComplete_004a266c = 1;
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
    }
#endif
}

/* Function start: 0x42482E */
void SetSoundEffectsVolume(int volume)
{
#if 0
    if (volume >= 0 && volume < 65000)
        ix_system_set_master_volume((unsigned short)volume);
    SoundDebugPrintf("Setting SFX Volume to %d", volume);
#else
    if (g_nAudioEnabled_0049c244 == 0)
        return;
    ix_system_set_master_volume((unsigned short)volume);
#endif
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

/* Function start: 0x4248B4 */
void LoadVolumeSettingsFromRegistry(void)
{
    HKEY key;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "Software\\Origin Systems\\WC: Kilrathi Saga",
                      0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS) {
        if (RegistryQueryValue(key, "MusicVolume", REG_DWORD,
                               (LPBYTE)&g_nMusicVolumeSetting_0049d750,
                               sizeof(g_nMusicVolumeSetting_0049d750)) !=
            ERROR_SUCCESS) {
            g_nMusicVolumeSetting_0049d750 = 0x14;
            RegistryStoreValue(key, "MusicVolume", REG_DWORD,
                               (const BYTE *)&g_nMusicVolumeSetting_0049d750,
                               sizeof(g_nMusicVolumeSetting_0049d750));
        }
        if (RegistryQueryValue(key, "SFXVolume", REG_DWORD,
                               (LPBYTE)&g_nSfxVolumeSetting_0049d74c,
                               sizeof(g_nSfxVolumeSetting_0049d74c)) !=
            ERROR_SUCCESS) {
            g_nSfxVolumeSetting_0049d74c = 0x14;
            RegistryStoreValue(key, "SFXVolume", REG_DWORD,
                               (const BYTE *)&g_nSfxVolumeSetting_0049d74c,
                               sizeof(g_nSfxVolumeSetting_0049d74c));
        }
        RegCloseKey(key);
    }
}

/* Function start: 0x424980 */
void SaveVolumeSettingsToRegistry(void)
{
    HKEY key;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "Software\\Origin Systems\\WC: Kilrathi Saga",
                      0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS) {
        RegistryStoreValue(key, "MusicVolume", REG_DWORD,
                           (const BYTE *)&g_nMusicVolumeSetting_0049d750,
                           sizeof(g_nMusicVolumeSetting_0049d750));
        RegistryStoreValue(key, "SFXVolume", REG_DWORD,
                           (const BYTE *)&g_nSfxVolumeSetting_0049d74c,
                           sizeof(g_nSfxVolumeSetting_0049d74c));
    }
    RegCloseKey(key);
}

/* Function start: WC2_UNMAPPED */
void DrawLaunchDoorFrame(short distance)
{
    short bounds[4];
    short scale;

    if (distance > 10) {
        scale = (short)(0x1a00L / distance);
        GetTransformedShapeBounds(
            &g_stViewBuffer_005d2b00,
            (short)((short)g_nScreenWidth_0049d4d8 >> 1),
            (short)((short)g_nScreenHeight_0049d4dc >> 1),
            g_pLaunchDoorShape_005a77e8, 1, 0, scale, 0, bounds);
#ifdef WC1_SDL
        if (!Wc1SdlRecordSpaceSprite(
                &g_stViewBuffer_005d2b00, (short)(bounds[0] - 1),
                (short)((short)g_nScreenHeight_0049d4dc >> 1),
                g_pLaunchDoorShape_005a77e8, 0, 0, scale, 0))
#endif
        DrawSpriteScaled(
            &g_stViewBuffer_005d2b00, (short)(bounds[0] - 1),
            (short)((short)g_nScreenHeight_0049d4dc >> 1),
            g_pLaunchDoorShape_005a77e8, 0, 0, scale, 0);
#ifdef WC1_SDL
        if (!Wc1SdlRecordSpaceSprite(
                &g_stViewBuffer_005d2b00,
                (short)((short)g_nScreenWidth_0049d4d8 >> 1),
                (short)((short)g_nScreenHeight_0049d4dc >> 1),
                g_pLaunchDoorShape_005a77e8, 1, 0, scale, 0))
#endif
        DrawSpriteScaled(
            &g_stViewBuffer_005d2b00,
            (short)((short)g_nScreenWidth_0049d4d8 >> 1),
            (short)((short)g_nScreenHeight_0049d4dc >> 1),
            g_pLaunchDoorShape_005a77e8, 1, 0, scale, 0);
#ifdef WC1_SDL
        if (!Wc1SdlRecordSpaceSprite(
                &g_stViewBuffer_005d2b00, bounds[2],
                (short)((short)g_nScreenHeight_0049d4dc >> 1),
                g_pLaunchDoorShape_005a77e8, 2, 0, scale, 0))
#endif
        DrawSpriteScaled(
            &g_stViewBuffer_005d2b00, bounds[2],
            (short)((short)g_nScreenHeight_0049d4dc >> 1),
            g_pLaunchDoorShape_005a77e8, 2, 0, scale, 0);
    }
}

/* Function start: WC2_UNMAPPED */
void LaunchPlayerShip(void)
{
    short doorDistances[4];
    signed char distanceStep;
    signed char frame;
    signed char door;

    doorDistances[0] = 50;
    doorDistances[1] = 40;
    doorDistances[2] = 30;
    doorDistances[3] = 20;
    distanceStep = 1;

    spacetrack(changetrack(), 1, 0);
    if (g_bSceneEscapeRequested_0049d4b0 == 0) {
        g_pLaunchDoorShape_005a77e8 =
            (unsigned char *)FetchDiskPacketRetrying(
                (char *)(unsigned long)1, 7, 0);
        g_nCannedSceneMode_0049021c = 1;
        force_view(0, 0);
        PlaySfxWaveFileByNumber(20, -1, 0);
        g_nFrameSkipCountdown_0049d760 = 1;
        g_bSceneEscapeRequested_0049d4b0 = 0;
        frame = 0;
        do {
            PumpWindowMessages(0);
            if (RefreshCockpitStatus() != 0) {
                door = 0;
                do {
                    DrawLaunchDoorFrame(doorDistances[door]);
                    doorDistances[door] =
                        (short)(doorDistances[door] - distanceStep);
                    door++;
                } while (door < 4);
                dump_buffer_to_screen();
                update_cockpit();
            }
            MarkDibDirty();
            DIBslamReal();
            if (g_bSceneEscapeRequested_0049d4b0 == 1)
                break;
            if (frame % 5 == 0)
                distanceStep++;
            frame++;
        } while (frame < 25);

        if (g_bSceneEscapeRequested_0049d4b0 != 0) {
            StopMusicUnlessSuppressed();
            spacetrack(changetrack(), 1, 0);
        }
        ReleasePacketHandle(g_pLaunchDoorShape_005a77e8);
    } else {
        force_view(0, 0);
    }

    MarkDibDirty();
    DIBslamReal();
    clear_view_buffer();
    g_nCannedSceneMode_0049021c = 0;
    ResetSoundState();
    g_bSceneEscapeRequested_0049d4b0 = 0;
}

/* Function start: WC2_UNMAPPED */
unsigned int ShowCarrierLaunchSequence(signed char sceneObject)
{
    unsigned char *carrierShape;
    unsigned char *actorShape;
    unsigned char *fighterShape;
    short approachScale;
    short approachDistance;
    short fighterScreenX;
    short fighterScreenY;
    short carrierScreenX;
    short actorX;
    short actorY;
    short frame;
    short object;

    g_bIntroSceneResourcesActive_00469d48 = 0;
    carrierScreenX = 180;
    free_ship(1);
    free_ship(2);
    free_ship(3);
    remove_nav_point_objects();
    ResetSoundState();
    PreloadMusicTrackHook(0x1c);
    spacetrack(0x1c, 2, 1);
    carrierShape =
        (unsigned char *)FetchDiskPacketRetrying(
            (char *)(unsigned long)1, 8, 0);
    actorShape =
        (unsigned char *)FetchDiskPacketRetrying(
            (char *)(unsigned long)1, 4, 0);
    g_pScrambleViewport_005a86b4 = &g_stViewBuffer_005d2b00;
    object = (short)sceneObject;
    fighterShape = g_aObjectTypeData_00496d30[
        g_nPlayerShipType_00493464].shapeSet;
    fighterScreenY = 64;
    fighterScreenX = 20;
    g_nScriptedViewObject_0046a8d0 = object;
    initialize_scripted_view(&g_asCarrierLaunchViewData_0046a5dc[2]);
    g_nScrambleBackgroundY_005a8712 = 64;
    g_nScrambleBackgroundRightX_005a8714 = 520;
    g_asObjectFlip_004939c8[0] = 0;
    g_asObjectViewFrame_00493508[0] = 36;
    g_asObjectScreenAngle_004936b8[0] = 180;
    g_asObjectDistance_00493ae8[0] = 300;
    g_apObjectShape_00493868[0] = fighterShape;
    g_apObjectShape_00493868[object] = carrierShape;
    g_asObjectFlip_004939c8[object] = 0;
    g_asObjectViewFrame_00493508[object] = 3;
    g_bSceneEscapeRequested_0049d4b0 = 0;
    g_asObjectScreenAngle_004936b8[object] = 0;
    g_asObjectScreenScale_00493a58[object] = 0x100;
    g_asObjectDistance_00493ae8[object] = 2000;
    g_nFrameSkipCountdown_0049d760 = 1;
    frame = 0;
    approachDistance = 20;
    do {
        PumpWindowMessages(0);
        g_aeObjectClass_00495328[0] = OBJECT_CLASS_NULL;
        g_aeObjectClass_00495328[object] = OBJECT_CLASS_NULL;
        set_eye_direction_and_position();
        g_nFrameSkipCountdown_0049d760--;
        if (g_nFrameSkipCountdown_0049d760 < 1) {
            g_nFrameSkipCountdown_0049d760 = g_nFrameSkip_0049d764;
            g_nRenderedSpaceFrame_00493138++;
            UpdateSpacePaletteFade();
            clear_view_buffer();
            house_keep_objects();
            update_objects_in_space();
            transform_objects_to_your_view();
            update_star_field();
            if (frame < 24) {
                fighterScreenY = (short)(fighterScreenY +
                    g_asCarrierLaunchApproachDeltaX_0046a550[frame]);
                g_asObjectViewFrame_00493508[0] =
                    g_acCarrierLaunchApproachFrames_0046a580[frame];
            } else if (frame < 48) {
                fighterScreenY = (short)(fighterScreenY -
                    g_asCarrierLaunchApproachDeltaX_0046a550[47 - frame]);
                g_asObjectViewFrame_00493508[0] =
                    g_acCarrierLaunchApproachFrames_0046a580[47 - frame];
            }
            g_aeObjectClass_00495328[0] = OBJECT_CLASS_SHIP;
            approachScale = (short)(
                ((unsigned int)(unsigned short)g_asObjectScale_00494d90[0]
                    << 4) / approachDistance);
            g_aeObjectClass_00495328[object] = OBJECT_CLASS_SHIP;
            g_asObjectScreenX_00493598[0] =
                (short)(fighterScreenX - g_nViewCenterX_005c80d8);
            g_asObjectScreenY_00493628[0] =
                (short)(fighterScreenY - g_nViewCenterY_005c80da);
            g_asObjectScreenScale_00493a58[0] = approachScale;
            g_asObjectScreenX_00493598[object] =
                (short)(g_nScrambleBackgroundRightX_005a8714 -
                        g_nViewCenterX_005c80d8);
            g_asObjectScreenY_00493628[object] =
                (short)(g_nScrambleBackgroundY_005a8712 -
                        g_nViewCenterY_005c80da);
            BuildObjectDepthOrder();
#ifdef WC1_SDL
            Wc1SdlBeginSpaceFrame(
                g_pScreenViewportGeometry_005c82b0,
                (int)g_cScreenViewportMode_005c82a6,
                g_nCockpitDisplayMode_0049d71c > 0,
                (unsigned char)g_cPrimaryViewBufferColour_0049cb88);
#endif
            draw_sorted_objects_to_buffer();
            dump_buffer_to_screen();
        }
        fighterScreenX = (short)(fighterScreenX + 2);
        g_nSpaceFrame_00493134++;
        g_nScrambleBackgroundRightX_005a8714 =
            (short)(g_nScrambleBackgroundRightX_005a8714 - 2);
        approachDistance = (short)(approachDistance + 2);
        g_asObjectDistance_00493ae8[0] =
            (short)(g_asObjectDistance_00493ae8[0] + 10);
        AddFixedVectors(&g_aShipPosition_00494550[61],
                        &g_aShipVelocity_00494898[61],
                        &g_aShipPosition_00494550[61]);
        if (g_bSceneEscapeRequested_0049d4b0 == 1)
            break;
        frame++;
        MarkDibDirty();
        DIBslamReal();
    } while (frame < 100);

    g_aeObjectClass_00495328[0] = OBJECT_CLASS_NULL;
    g_aeObjectClass_00495328[object] = OBJECT_CLASS_NULL;
    if (g_bSceneEscapeRequested_0049d4b0 != 1) {
        copy_frame(object, 61);
        g_aShipPosition_00494550[61] = g_aShipPosition_00494550[object];
        g_nScrambleBackgroundRightX_005a8714 = 0;
        fighterScreenX = 200;
        fighterScreenY = 32;
        g_nScrambleBackgroundY_005a8712 = 0;
        g_asObjectCollisionRadius_004950e8[61] =
            g_asObjectCollisionRadius_004950e8[object];
        approachDistance = 100;
        ConfigureScrambleActor(100, 80, 1, 0, actorShape,
                               0x100, 0, 0, 0);
        ConfigureScrambleActor(116, 130, 0, 0, actorShape,
                               0x100, 0, 0, 1);
        ConfigureScrambleActor(300, 110, -4, 0, actorShape,
                               0xc0, 0, 0x10, 3);
        ConfigureScrambleActor(301, 110, -4, 0, actorShape,
                               0xc0, 0, 0x10, 4);
        PlaySfxWaveFileByNumber(18, -1, 0);
        frame = 0;
        actorX = 60;
        actorY = 10;
        g_nFrameSkipCountdown_0049d760 = 1;
        do {
            PumpWindowMessages(0);
            alter_yaw(-1, 61);
            if (RefreshCockpitStatus() != 0) {
                DrawSpriteDefault(
                    &g_stViewBuffer_005d2b00,
                    (short)(g_nScrambleBackgroundRightX_005a8714 + 239),
                    g_nScrambleBackgroundY_005a8712,
                    carrierShape, 0);
                DrawSpriteDefault(
                    &g_stViewBuffer_005d2b00,
                    (short)(g_nScrambleBackgroundRightX_005a8714 + 240),
                    g_nScrambleBackgroundY_005a8712,
                    carrierShape, 1);
                DrawScrambleActor(0);
                approachScale = (short)(0x6000L / approachDistance);
                DrawSpriteScaled(&g_stViewBuffer_005d2b00, fighterScreenX,
                                 fighterScreenY, fighterShape, 16, 0,
                                 approachScale, 0);
                DrawScrambleActor(3);
                DrawScrambleActor(4);
                DrawScrambleActor(1);
                DrawSpriteDefault(
                    &g_stViewBuffer_005d2b00,
                    (short)(g_nScrambleBackgroundRightX_005a8714 + 60),
                    (short)(g_nScrambleBackgroundY_005a8712 + 10),
                    actorShape, 16);
                DrawSpriteDefault(
                    &g_stViewBuffer_005d2b00,
                    (short)(g_nScrambleBackgroundRightX_005a8714 + 80),
                    (short)(g_nScrambleBackgroundY_005a8712 + 134),
                    actorShape, 8);
                DrawSpriteDefault(&g_stViewBuffer_005d2b00, carrierScreenX,
                                  g_nScrambleBackgroundY_005a8712,
                                  carrierShape, 2);
                WaitForVerticalBlankThunk();
                dump_buffer_to_screen();
            }
            g_nScrambleBackgroundRightX_005a8714 =
                (short)(g_nScrambleBackgroundRightX_005a8714 + 2);
            carrierScreenX = (short)(carrierScreenX + 4);
            fighterScreenX = (short)(fighterScreenX - 2);
            fighterScreenY++;
            approachDistance--;
            if (sceneObject != 0) {
                ((void (__cdecl *)(int, int, int, int))PaletteFadeHook)(
                    (int)sceneObject, 10, 0, 0);
            }
            if (g_bSceneEscapeRequested_0049d4b0 == 1)
                break;
            MarkDibDirty();
            DIBslamReal();
            frame++;
        } while (frame < 35);

        if (g_bSceneEscapeRequested_0049d4b0 != 1) {
            g_nFrameSkipCountdown_0049d760 = 1;
            frame = 0;
            do {
                PumpWindowMessages(0);
                if (RefreshCockpitStatus() != 0) {
                    DrawSpriteDefault(
                        &g_stViewBuffer_005d2b00,
                        (short)(g_nScrambleBackgroundRightX_005a8714 + 239),
                        g_nScrambleBackgroundY_005a8712,
                        carrierShape, 0);
                    DrawSpriteDefault(
                        &g_stViewBuffer_005d2b00,
                        (short)(g_nScrambleBackgroundRightX_005a8714 + 240),
                        g_nScrambleBackgroundY_005a8712,
                        carrierShape, 1);
                    DrawScrambleActor(0);
                    DrawSpriteScaled(&g_stViewBuffer_005d2b00, fighterScreenX,
                                     fighterScreenY, fighterShape, 16, 0,
                                     approachScale, 0);
                    DrawScrambleActor(3);
                    DrawScrambleActor(4);
                    DrawScrambleActor(1);
                    DrawSpriteDefault(
                        &g_stViewBuffer_005d2b00,
                        (short)(g_nScrambleBackgroundRightX_005a8714 + actorX),
                        (short)(g_nScrambleBackgroundY_005a8712 + actorY),
                        actorShape, 16);
                    DrawSpriteDefault(
                        &g_stViewBuffer_005d2b00,
                        (short)(g_nScrambleBackgroundRightX_005a8714 + 80),
                        (short)(g_nScrambleBackgroundY_005a8712 + 134),
                        actorShape, 8);
                    DrawSpriteDefault(&g_stViewBuffer_005d2b00, carrierScreenX,
                                      g_nScrambleBackgroundY_005a8712,
                                      carrierShape, 2);
                    WaitForVerticalBlankThunk();
                    dump_buffer_to_screen();
                }
                frame++;
                if (frame < 9) {
                    fighterScreenX = (short)(fighterScreenX +
                        g_aCarrierLaunchFighterPath_0046a598[frame].x);
                    fighterScreenY = (short)(fighterScreenY +
                        g_aCarrierLaunchFighterPath_0046a598[frame].y);
                } else if (frame < 23) {
                    if (frame == 9)
                        PlaySfxWaveFileByNumber(11, -1, 0);
                    fighterScreenY = (short)(fighterScreenY +
                        g_asCarrierLaunchFighterDeltaY_0046a5bc[frame - 7]);
                }
                if (frame == 23) {
                    ((void (__cdecl *)(int, int))
                        FlushSoundEffectsAndLog)((int)sceneObject, 0);
                    PlaySfxWaveFileByNumber(19, -1, 0);
                }
                actorX++;
                if (frame % 7 == 0)
                    actorY--;
                MarkDibDirty();
                DIBslamReal();
            } while (g_bSceneEscapeRequested_0049d4b0 != 1 && frame < 50);
        }
    }

    g_bSceneEscapeRequested_0049d4b0 = 0;
    ResetSoundState();
    StopMusicUnlessSuppressed();
    ReleaseMusicTrackHook(0x1c);
    free_ship(0);
    ReleasePacketHandle(carrierShape);
    ReleasePacketHandle(actorShape);
    g_bScriptedView_0046a8d4 = 0;
    g_bIntroSceneResourcesActive_00469d48 = 1;
    return 0;
}

/* Function start: WC2_UNMAPPED */
void FxDriverShutdownHook(void)
{
}

/* Function start: WC2_UNMAPPED */
unsigned short InitializeDiskPromptTextContext(void)
{
    short textWidth;
    short textHeight;

    g_nDiskPromptBorderColour_00469694 = 0x50;
    if (g_stScreenViewport_005d21a0.pixels == 0)
        InitializeGameTextContexts();
    g_stDiskPromptTextContext_005a7d60 = g_stDefaultTextContext_005d2d20;
    g_stDiskPromptTextContext_005a7d60.viewport =
        &g_stDiskPromptViewport_005a7d40;
    g_stDiskPromptTextContext_005a7d60.text =
        g_szTextScratchBuffer_005d1c40;
    g_stDiskPromptTextContext_005a7d60.alignment = 2;
    g_stDiskPromptViewport_005a7d40 = g_stScreenViewport_005d21a0;
    textWidth =
        (short)(MeasureTextPixelWidthClamped(
                    g_szPressAnyKeyWhenReady_0046a5fc) +
                10);
    textHeight =
        (short)((ReadWord((unsigned short *)
                              g_stDiskPromptTextContext_005a7d60.font) +
                 1) *
                3);
    ((short *)&g_dwDiskPromptTopLeft_005a7d80)[1] =
        (short)(100 - textHeight / 2);
    ((short *)&g_dwDiskPromptTopLeft_005a7d80)[0] =
        (short)(160 - textWidth / 2);
    ((short *)&g_dwDiskPromptBottomRight_005a7d84)[1] =
        (short)(((short *)&g_dwDiskPromptTopLeft_005a7d80)[1] +
                textHeight);
    ((short *)&g_dwDiskPromptBottomRight_005a7d84)[0] =
        (short)(((short *)&g_dwDiskPromptTopLeft_005a7d80)[0] + textWidth);
    InitializeTextContextFromFont(&g_stDiskPromptTextContext_005a7d60, 1,
                                  g_cSecondaryViewBufferColour_0049cb4c,
                                  (signed char)g_bPrimaryViewBufferColour_0049cb50);
    g_bOriginFxDriverActive_0049cbb0 = 1;
    return 0;
}

/* Function start: 0x432DCC */
char *GetPackedStringByIndex(CutsceneResourceTable *resources,
                             short index)
{
    char *text;
    short stringIndex;

    text = resources->packedFilenames;
    for (stringIndex = 0; stringIndex < index; stringIndex++) {
        text = DosStrchr(text, 0);
        text++;
    }
    return text;
}

/* Function start: 0x432E23 */
void RewriteDiskFileGraphicsExtensions(char *fileName)
{
#if 0
    DiskFileRecord *record;
    char *extensionPosition;
    char extension;

    record = g_pDiskFileRecords_005a7cf0;
    switch (videoMode) {
    case 0:
        extension = 'v';
        break;
    case 1:
        extension = 'e';
        break;
    case 3:
        extension = 't';
        break;
    }

    while (record->name[0] != '\0') {
        extensionPosition = strrchr(record->name, '.');
#ifdef WC1_SDL
        if (extensionPosition != 0) {
            extensionPosition++;
            if (toupper((int)*extensionPosition) == 'V')
                *extensionPosition = extension;
        }
#else
        if (extensionPosition++ != 0 &&
            toupper((int)*extensionPosition) == 'V')
            *extensionPosition = extension;
#endif
        record++;
    }
    return 0;
#else
    char *extensionPosition;

    if (g_cCutsceneVideoMode_00499c48 == 13) {
        extensionPosition = strchr(fileName, '.');
#ifdef WC1_SDL
        if (extensionPosition != 0) {
            extensionPosition++;
            if (toupper((int)*extensionPosition) == 'V')
                *extensionPosition = 'E';
        }
#else
        if (extensionPosition++ != 0 &&
            toupper((int)*extensionPosition) == 'V')
            *extensionPosition = 'E';
#endif
    }
#endif
}

/* Function start: 0x432E83 */
void RewriteCutsceneMusicExtensionForDriver(char *filename)
{
    char *extension;

    extension = DosStrchr(filename, '.');
    if (extension++ != 0 && *extension != 0) {
        if (g_nMusicDriverMode_0049be8c == 2)
            *extension = 'A';
        else
            *extension = 'R';
    }
}

/* Function start: 0x432EE4 */
void RewriteCutsceneMusicExtensionForRetry(char *filename)
{
    char *extension;

    extension = DosStrchr(filename, '.');
    if (extension++ != 0 && *extension != 0)
        *extension = 'R';
}

/* Function start: 0x401000 */
short LoadWingCmdrCfgFile(short argc, char **argv)
{
    FILE *file;
    short argumentCount;
    char *destination;
    short argumentIndex;
    short scanResult;
#ifdef WC1_SDL
    char resolvedPath[PATH_MAX];
#endif

    argumentIndex = 1;
    argumentCount = 0;
    destination = g_szTextScratchBuffer_005d1c40;
#ifdef WC1_SDL
    if (Wc1SdlResolvePath("wc2.cfg", resolvedPath,
                          sizeof(resolvedPath)))
        file = fopen(resolvedPath, "rb");
    else
        file = 0;
#else
    file = fopen("wc2.cfg", "rb");
#endif
    while (file != 0) {
#if 0
        if ((short)fscanf(file, "%s", destination) == -1) {
            fclose(file);
            break;
        }
#else
        scanResult = (short)fscanf(file, "%s", destination);
        if (scanResult == -1) {
            fclose(file);
            break;
        }
#endif
        g_pStartupArguments_005c57f0[argumentCount++] =
            destination;
        destination = strchr(destination, 0) + 1;
    }

    while (argc-- != 0) {
#if 0
        strcpy(destination, argv[argumentIndex]);
        g_pStartupArguments_005c57f0[argumentCount++] = destination;
        argumentIndex++;
#else
        strcpy(destination, argv[argumentIndex++]);
        g_pStartupArguments_005c57f0[argumentCount++] = destination;
#endif
        destination = strchr(destination, 0) + 1;
    }
    return (short)(argumentCount - 1);
}

/* Function start: WC2_UNMAPPED */
unsigned short LoadInstallDat(void)
{
    DiskFileRecord *records;
    DiskFileRecord *record;
    DiskFileRecord *entry;
    unsigned int size;
    short file;
    short maximumId;

    maximumId = 0;
    SystemDebugPrintf("Loading INSTALL.DAT\n");
    file = OpenDataFileOrDie("install.dat");
    if (file == -1) {
        SystemDebugPrintf("Unable to open INSTALL.DAT\n");
        SystemDebugPrintf(
            "[SYSTEM]: Exiting Prematurely (LoadInstallData)\n");
        ClearDebugPauseFlags();
        PumpMessagesDuringWait();
        exit(0);
    }
    size = (unsigned int)_filelength(file);
    records = (DiskFileRecord *)AllocateTaggedMemory(size, 0);
    if (records == 0) {
        SystemDebugPrintf("Unable to load INSTALL.DAT\n");
        SystemDebugPrintf(
            "[SYSTEM]: Exiting Prematurely (LoadInstallData)\n");
        ClearDebugPauseFlags();
        PumpMessagesDuringWait();
        exit(0);
    }
    ReadDataFileAtOffset(file, 0, size, records);
    CloseDataFile(file);

    record = records;
    while (record->name[0] != 0) {
        if (maximumId < record->logicalFile &&
            record->logicalFile != 0xff)
            maximumId = record->logicalFile;
        record++;
    }
    maximumId++;

#ifdef WC1_SDL
    g_pDiskFileRecords_005a7cf0 =
        (DiskFileRecord *)AllocateTaggedMemory(
            sizeof(DiskFileRecord) * 78, 0);
    if (g_pDiskFileRecords_005a7cf0 != 0)
        memset(g_pDiskFileRecords_005a7cf0, 0,
               sizeof(DiskFileRecord) * 78);
#else
    g_pDiskFileRecords_005a7cf0 =
        (DiskFileRecord *)AllocateTaggedMemory(0x4b0, 0);
    memset(g_pDiskFileRecords_005a7cf0, 0,
           (maximumId + 1) * sizeof(DiskFileRecord));
#endif
    if (g_pDiskFileRecords_005a7cf0 == 0) {
        SystemDebugPrintf("Unable to copy INSTALL.DAT\n");
        SystemDebugPrintf(
            "[SYSTEM]: Exiting Prematurely (LoadInstallData)\n");
        ClearDebugPauseFlags();
        PumpMessagesDuringWait();
        exit(0);
    }

    entry = g_pDiskFileRecords_005a7cf0;
    while (maximumId > 0) {
        entry->name[0] = ' ';
        entry++;
        maximumId--;
    }

    record = records;
    while (record->name[0] != 0) {
        if (record->logicalFile != 0xff)
            g_pDiskFileRecords_005a7cf0[record->logicalFile] = *record;
        record++;
    }
    ReleasePacketHandle(records);
    g_pDiskFileRecords_005a7cf0++;
#ifdef WC1_SDL
    if (Wc1SdlUsingDosData())
        Wc1SdlCompleteDosInstallTable(g_pDiskFileRecords_005a7cf0);
#endif
    return 0;
}

/* Function start: 0x401120 */
void show_damage_disp(void)
{
    signed char component;
    signed char plural;
    char message[40];

    g_nDamageDisplayState_005d42ae = 0;
    g_cDamagedComponentCount_005d42a7 = 0;
    for (component = 0; component < 9; component++) {
        if (g_acPlayerComponentDamage_00493470[component] >= 1)
            g_cDamagedComponentCount_005d42a7++;
    }

    set_new_vdu(0);
    DrawTextAt(&g_stLeftVduTextContext_005d2ae0,
               g_stLeftVduViewport_005d2180.left,
               g_stLeftVduViewport_005d2180.top,
               g_szDamageReport_00490190, 2);
    DrawViewportLine(&g_stLeftVduViewport_005d2180,
                     (short)(g_stLeftVduViewport_005d2180.left + 2),
                     (short)(g_stLeftVduViewport_005d2180.top + 6),
                     (short)(g_stLeftVduViewport_005d2180.right - 2),
                     (short)(g_stLeftVduViewport_005d2180.top + 6),
                     g_ucPrimaryTextColour_0049cb64);
    if (g_cDamagedComponentCount_005d42a7 == 0) {
        DrawTextAt(&g_stLeftVduTextContext_005d2ae0,
                   g_stLeftVduViewport_005d2180.left,
                   (short)(g_stLeftVduViewport_005d2180.top + 20),
                   g_szNoInternalDamage_004901a0, 2);
    } else {
        g_nWeaponDisplayOriginX_005d4254 =
            (short)(g_stLeftVduViewport_005d2180.left +
                    g_nWeaponDisplayOffsetX_0049ae8c);
        g_nWeaponDisplayOriginY_005d4256 =
            (short)(g_nWeaponDisplayOffsetY_0049ae8e +
                    g_stLeftVduViewport_005d2180.top);
        DrawSpriteDefault(&g_stLeftVduViewport_005d2180,
                          g_nWeaponDisplayOriginX_005d4254,
                          g_nWeaponDisplayOriginY_005d4256,
                          g_pCockpitWeaponShape_005d2b54, 0);
        if (g_bForceDamageDisplayRedraw_0049b2ec == 0) {
            g_nDamageDisplayTicks_005d4250 = 1;
            g_nDamageDisplayPhase_005d42b4 = 1;
        }
        if (g_cDamagedComponentCount_005d42a7 != 1)
            plural = 's';
        else
            plural = ' ';
        sprintf(message, g_szDamagedUnitCountFormat_004901b4,
                (int)g_cDamagedComponentCount_005d42a7, (int)plural);
        ShowComponentHitHudMessage(message,
                                   g_ucPrimaryTextColour_0049cb64, -1);
    }
}

/* Function start: 0x4012E1 */
void UpdateDamageDisplay(void)
{
    short componentCount;
    signed char component;
    signed char attempts;
    signed char damage;

    componentCount = (short)g_cDamagedComponentCount_005d42a7;
    g_cDamagedComponentCount_005d42a7 = 0;
    for (attempts = 0; attempts < 9; attempts++) {
        if (g_acPlayerComponentDamage_00493470[attempts] >= 1)
            g_cDamagedComponentCount_005d42a7++;
    }

    if ((short)g_cDamagedComponentCount_005d42a7 != componentCount) {
        InvalidateVduMode(0);
        return;
    }
    if (g_cDamagedComponentCount_005d42a7 == 0)
        return;

    if (g_nCockpitDisplayMode_0049d71c == 0) {
        g_nDamageDisplayTicks_005d4250--;
        if (g_nDamageDisplayTicks_005d4250 > 0)
            return;
        if (g_nDamageDisplayPhase_005d42b4 == 1) {
            g_nDamageDisplayTicks_005d4250 = 25;
            component = g_cDamageDisplayComponent_005d42a6;
            attempts = 0;
            do {
                component++;
                if (component >= 9)
                    component = 0;
                damage = g_acPlayerComponentDamage_00493470[component];
                if (damage >= 1) {
                    g_cDamageDisplayComponent_005d42a6 = component;
                    break;
                }
                attempts++;
            } while (attempts < 9);

            sprintf(
                g_szDamageStatusText_005d4260,
                g_szDamageStatusFormat_004901c8,
                g_apszComponentNames_00490090[
                    g_cDamageDisplayComponent_005d42a6],
                g_apszDamageSeverityNames_004900c8[(int)damage]);
            DrawTextAt(&g_stLeftVduTextContext_005d2ae0,
                       (short)(g_stLeftVduViewport_005d2180.left + 1),
                       (short)(g_stLeftVduViewport_005d2180.top + 7),
                       g_szDamageStatusText_005d4260, 2);
            g_cDamageDisplayFrame_005d42ac =
                (signed char)g_abDamageDisplayFrames_004900b8[
                    g_cDamageDisplayComponent_005d42a6];
            g_stDamageSpritePosition_005d42b0 =
                g_aDamageDisplayPositions_00490068[
                    g_cDamageDisplayComponent_005d42a6];
            g_stDamageSpritePosition_005d42b0.x =
                (short)(g_stDamageSpritePosition_005d42b0.x +
                        g_nWeaponDisplayOriginX_005d4254);
            g_stDamageSpritePosition_005d42b0.y =
                (short)(g_stDamageSpritePosition_005d42b0.y +
                        g_nWeaponDisplayOriginY_005d4256);
            CaptureSpriteBackground(
                &g_stLeftVduViewport_005d2180,
                g_pDamageDisplayBackground_00490060,
                g_stDamageSpritePosition_005d42b0.x,
                g_stDamageSpritePosition_005d42b0.y,
                g_pCockpitWeaponShape_005d2b54,
                (short)g_cDamageDisplayFrame_005d42ac);
            DrawViewportLine(
                &g_stLeftVduViewport_005d2180,
                (short)(g_stLeftVduViewport_005d2180.left + 36),
                (short)(g_stLeftVduViewport_005d2180.top + 22),
                g_stDamageSpritePosition_005d42b0.x,
                g_stDamageSpritePosition_005d42b0.y, 0xa9);
            DrawSpriteDefault(
                &g_stLeftVduViewport_005d2180,
                g_stDamageSpritePosition_005d42b0.x,
                g_stDamageSpritePosition_005d42b0.y,
                g_pCockpitWeaponShape_005d2b54,
                (short)g_cDamageDisplayFrame_005d42ac);
        } else {
            RestoreSpriteBackground(
                &g_stLeftVduViewport_005d2180,
                g_pDamageDisplayBackground_00490060,
                g_stDamageSpritePosition_005d42b0.x,
                g_stDamageSpritePosition_005d42b0.y,
                g_pCockpitWeaponShape_005d2b54,
                (short)g_cDamageDisplayFrame_005d42ac);
            g_stLeftVduTextContext_005d2ae0.colour =
                g_cSecondaryViewBufferColour_0049cb4c;
            DrawTextAt(&g_stLeftVduTextContext_005d2ae0,
                       (short)(g_stLeftVduViewport_005d2180.left + 1),
                       (short)(g_stLeftVduViewport_005d2180.top + 7),
                       g_szDamageStatusText_005d4260, 2);
            g_stLeftVduTextContext_005d2ae0.colour =
                g_ucPrimaryTextColour_0049cb64;
            DrawViewportLine(
                &g_stLeftVduViewport_005d2180,
                (short)(g_stLeftVduViewport_005d2180.left + 36),
                (short)(g_stLeftVduViewport_005d2180.top + 22),
                g_stDamageSpritePosition_005d42b0.x,
                g_stDamageSpritePosition_005d42b0.y,
                g_cSecondaryViewBufferColour_0049cb4c);
            g_nDamageDisplayTicks_005d4250 = 4;
        }
        g_nDamageDisplayPhase_005d42b4 =
            g_nDamageDisplayPhase_005d42b4 == 0;
        return;
    }

    g_nDamageDisplayTicks_005d4250--;
    if (g_nDamageDisplayTicks_005d4250 <= 0) {
        if (g_nDamageDisplayPhase_005d42b4 == 1) {
            g_nDamageDisplayTicks_005d4250 = 25;
            component = g_cDamageDisplayComponent_005d42a6;
            attempts = 0;
            do {
                component++;
                if (component >= 9)
                    component = 0;
                damage = g_acPlayerComponentDamage_00493470[component];
                if (damage >= 1) {
                    g_cDamageDisplayComponent_005d42a6 = component;
                    g_nDisplayedComponentDamage_005d42a8 = (int)damage;
                    break;
                }
                attempts++;
            } while (attempts < 9);
        }
        g_nDamageDisplayPhase_005d42b4 =
            g_nDamageDisplayPhase_005d42b4 == 0;
    }

    sprintf(
        g_szDamageStatusText_005d4260,
        g_szDamageStatusFormatHighRes_004901d8,
        g_apszComponentNames_00490090[g_cDamageDisplayComponent_005d42a6],
        g_apszDamageSeverityNames_004900c8[
            g_nDisplayedComponentDamage_005d42a8]);
    DrawTextAt(&g_stLeftVduTextContext_005d2ae0,
               (short)(g_stLeftVduViewport_005d2180.left + 1),
               (short)(g_stLeftVduViewport_005d2180.top + 7),
               g_szDamageStatusText_005d4260, 2);
    g_cDamageDisplayFrame_005d42ac =
        (signed char)g_abDamageDisplayFrames_004900b8[
            g_cDamageDisplayComponent_005d42a6];
    g_stDamageSpritePosition_005d42b0 =
        g_aDamageDisplayPositions_00490068[
            g_cDamageDisplayComponent_005d42a6];
    g_stDamageSpritePosition_005d42b0.x =
        (short)(g_stDamageSpritePosition_005d42b0.x +
                g_nWeaponDisplayOriginX_005d4254);
    g_stDamageSpritePosition_005d42b0.y =
        (short)(g_stDamageSpritePosition_005d42b0.y +
                g_nWeaponDisplayOriginY_005d4256);
    CaptureSpriteBackground(
        &g_stLeftVduViewport_005d2180,
        g_pDamageDisplayBackground_00490060,
        g_stDamageSpritePosition_005d42b0.x,
        g_stDamageSpritePosition_005d42b0.y,
        g_pCockpitWeaponShape_005d2b54,
        (short)g_cDamageDisplayFrame_005d42ac);
    DrawViewportLine(&g_stLeftVduViewport_005d2180,
                     (short)(g_stLeftVduViewport_005d2180.left + 36),
                     (short)(g_stLeftVduViewport_005d2180.top + 22),
                     g_stDamageSpritePosition_005d42b0.x,
                     g_stDamageSpritePosition_005d42b0.y, 0xa9);
    DrawSpriteDefault(&g_stLeftVduViewport_005d2180,
                      g_stDamageSpritePosition_005d42b0.x,
                      g_stDamageSpritePosition_005d42b0.y,
                      g_pCockpitWeaponShape_005d2b54,
                      (short)g_cDamageDisplayFrame_005d42ac);
}

/* Function start: WC2_UNMAPPED */
unsigned short GetJoystickPresentUnused(void)
{
    return 1;
}
