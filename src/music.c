/*
 *  Music and sound-effect control.
 *
 *  Retail object 0x452A00-0x45381F (link order; see docs/ORDER.md).
 */
#include "game.h"

static int g_nActiveMusicStreamMask_0049be9c;

static signed char g_acMusicTrackStreamFlags_0049beb8[72] = {
    6, 6, 6, 6, 6, 6, 10, 10, 10, 10, 10, 10,
    6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0,
    0, 0, 0, 6, 16, 16, 0, 6, 6, 16, 16, 0,
    16, 16, 16, 16, 16, 6, 10, 1, 1, 1, 1, 7,
    1, 1, 1, 1, 1, 1, 1, 6, 6, 6, 1, 16,
    16, 1, 1, 16, 16, 16, 0, 0, 0, 0, 0, 0
};

/* Function start: 0x452A00 */
void StopMusicStream(void)
{
    Streamer_stop();
}

/* Function start: 0x452A10 */
void SetMusBreakpt(int first, int second)
{
}

/* Function start: 0x452A1B */
void SetMusicOn(short enabled)
{
}

/* Function start: 0x452A26 */
void StopMusic(short enabled)
{
    g_nCurrentMusicTrack_0049be98 = -1;
    Streamer_stop();
}

/* Function start: 0x452A40 */
short StartMusic(void *music)
{
    SoundDebugPrintf("startMusic %p", music);
#ifdef SDL_PORT
    SdlStartOriginalTitleMusic();
#endif
    return 0;
}

/* Function start: 0x452A64 */
short StartInteractiveMusic(int track)
{
    SoundDebugPrintf("startIntMusic %d\n", track);
    ProcessMusicScriptCommand(track, 1, 0);
    return track;
}

/* Function start: 0x452A99 */
/* Started a sound effect on the DOS music driver; the Win32 port plays
 * effects through PlaySfxWaveFileByNumber and always reports no handle. */
int StartSoundEffectHook(const unsigned char *definition, int effectHandle,
                         int volume, int pan, int sample, int looping)
{
    return 0;
}

/* Function start: 0x452AAB */
void FlushSoundEffect(void)
{
    SoundDebugPrintf("FLUSH_EFFECT");
    stop_all_sounds();
}

/* Function start: 0x452AC8 */
void FlushSoundEffects(void)
{
    SoundDebugPrintf("FLUSH_EFFECTS");
    stop_all_sounds();
}

/* Function start: 0x452AE5 */
void FadeMusic(int duration)
{
}

/* Function start: 0x452AF0 */
void SceneLeaveHook(void)
{
}

/* Function start: 0x452B03 */
unsigned short ProcessMusicScriptCommand(int track, int command, short enabled)
{
    int streamFlags;
    int streamerCommand;

    if (track == -1)
        return 0;

    streamFlags = g_acMusicTrackStreamFlags_0049beb8[track];
    SoundDebugPrintf("queue_next_music %d %x\n", track, streamFlags);
    if (track >= 0x40)
        streamerCommand = track - 0x2c;
    else
        streamerCommand = track;

    if (command == 4) {
        SoundDebugPrintf("queue_stop\n");
        StopMusic(enabled);
        g_nCurrentMusicTrack_0049be98 = -1;
        return;
    }

#ifdef SDL_PORT
    /* A DOS install has no streams to queue.  Its music is the MUSIC.A00
     * section that carries this very track number. */
    if (SdlUsingOriginFxMusic()) {
        SdlSetOriginFxMusicTrack(track);
        return 1;
    }
#endif
    if ((streamFlags & g_nActiveMusicStreamMask_0049be9c) == 0) {
        if ((streamFlags & 1) != 0) {
            Streamer_open("gameflow.str");
            g_nActiveMusicStreamMask_0049be9c = 1;
        } else if ((streamFlags & 2) != 0) {
            Streamer_open("spaceflt.str");
            g_nActiveMusicStreamMask_0049be9c = 2;
        } else if ((streamFlags & 0x10) != 0) {
            Streamer_open("gametwo.str");
            g_nActiveMusicStreamMask_0049be9c = 0x10;
        } else {
            SoundDebugPrintf("**** unknown stream for track %d\n", track);
            StopMusic(0);
            return;
        }
        SetStreamerIntensity(0x32);
    } else {
        g_nCurrentMusicTrack_0049be98 = track;
    }

    if (g_nActiveMusicStreamMask_0049be9c == 2) {
        if ((streamFlags & 4) != 0) {
            SetStreamerIntensity(streamerCommand);
        } else if ((streamFlags & 8) != 0) {
            SetStreamerTrigger(streamerCommand);
        } else {
            SoundDebugPrintf(
                "unknown int/trig stream for track %d\n", track);
            exit_squadron("unknown track %d", track);
        }
    } else {
        Streamer_stop();
        ForceStreamerTrigger(streamerCommand);
    }

    if (g_nCannedSceneMode_0049021c == 0)
        RecordMusicCommandHook(track, command, enabled);
    return 1;
}

/* Function start: 0x452CF4 */
void spacetrack(int track, int mode, int enabled)
{
    if (g_nMusicDriverMode_0049be8c != 0 &&
        g_nMusicDriverMode_0049be8c != 3)
        ProcessMusicScriptCommand(track, mode, enabled);
}

/* Function start: 0x452D32 */
void StopMusicUnlessSuppressed(void)
{
    if (g_nMusicDriverMode_0049be8c != 0 &&
        g_nMusicDriverMode_0049be8c != 3)
        StopMusic(0);
}

/* Function start: 0x452D66 */
unsigned int IsMusicTrackComplete(void)
{
    if (g_nMusicDriverMode_0049be8c != 0 &&
        g_nMusicDriverMode_0049be8c != 3 &&
        g_nMusicTrackComplete_0049be88 != 0)
        return 1;
    return 0;
}

/* Function start: 0x452DB0 */
void wait_for_end_of_music(void)
{
    if (g_nMusicDriverMode_0049be8c != 0 &&
        g_nMusicDriverMode_0049be8c != 3) {
        if (g_nWaitForMusicEnabled_0049beac == 0) {
            StopMusic(0);
            return;
        }
        SetMusBreakpt(0, 0);
        while (g_nMusicTrackComplete_0049be88 == 0) {
            if (g_bSceneEscapeRequested_0049d4b0 != 0 ||
                WaitForInputKey() != 0) {
                StopMusic(0);
                return;
            }
        }
    }
}

/* Function start: 0x452E46 */
void new_space_music_changes(short attacker, short victim)
{
    if (g_nInFlightMusicActive_0049bf08 != 0) {
        if (g_asShipSide_004955d0[victim] == SIDE_KILRATHI) {
            if (report_kilrathi_rout(1) == 0) {
                spacetrack(10, 1, 0);
            } else if (attacker == 0) {
                if (g_acShipPortrait_00495d88[victim] != -1 ||
                    RandomInRange(0, 3) == 0)
                    spacetrack(9, 3, 0);
                else
                    spacetrack(6, 3, 0);
            }
        } else {
            if (g_nYourWingman_0049346c == victim) {
                spacetrack(8, 3, 0);
                return;
            }
            if (g_asShipSide_004955d0[victim] == SIDE_IMPERIAL) {
                if ((g_asShipMissionType_00495de8[0] ==
                         MISSION_TYPE_DEFEND ||
                     g_asShipMissionType_00495de8[0] ==
                         MISSION_TYPE_ESCORT) &&
                    g_asShipMissionIndex_00495d00[victim] ==
                        g_asShipMissionParameter_00495e00[0]) {
                    spacetrack(11, 3, 0);
                    return;
                }
                spacetrack(7, 3, 0);
            }
        }
    }
}

/* Function start: 0x452F90 */
int changetrack(void)
{
    int track;

    switch (g_asShipMissionType_00495de8[0]) {
    case MISSION_TYPE_STRIKE:
        track = 17;
        break;
    case MISSION_TYPE_ESCORT:
        track = 18;
        break;
    case MISSION_TYPE_DEFEND:
    case MISSION_TYPE_RENDEZVOUS:
        track = 16;
        break;
    default:
        track = 15;
        break;
    }
    if (g_aMissionObjectives_004932a8[
            g_cCurrentObjective_004931cc].type == OBJECTIVE_HOME_BASE) {
        if (triumph(0) != 0) {
            if (g_asShipMissionType_00495de8[0] != MISSION_TYPE_PATROL)
                track = 14;
            else
                track = 13;
        } else
            track = 12;
    }
    return track;
}

/* Function start: 0x453085 */
void gametrack(void)
{
    short level;
    int track;

    track = -1;
    if (g_nInFlightMusicActive_0049bf08 == 0)
        return;
    if (g_nCombatMusicActive_0049bf04 != 0) {
        if ((g_nSpaceFrame_00493134 & 0xf) == 0 ||
            g_nMusicTrackComplete_0049be88 != 0) {
            if (g_nInitialFlightMusicPending_0049bf00 != 0)
                g_nInitialFlightMusicPending_0049bf00 = 0;
            if (FindMissileTargetingObject(0) != 0) {
                track = 3;
            } else if (detect_enemy_tail(0) != -1) {
                track = 1;
            } else if (is_ship_tailing_player_target(0) != 0) {
                track = 2;
            } else {
                level = (short)calculate_damage_level();
                if (level >= 2)
                    track = 4;
                else if (level == 1)
                    track = 5;
                else
                    track = 0;
            }
            if (g_nMissionMusicTrackOverride_00496144 != 0)
                track = g_nMissionMusicTrackOverride_00496144;
            if (report_kilrathi_rout(1) == 0)
                g_nCombatMusicActive_0049bf04 = 0;
        }
    } else if ((g_nSpaceFrame_00493134 & 0xf) == 0 ||
               g_nMusicTrackComplete_0049be88 != 0) {
        if (g_bJumpSequenceActive_004962f0 != 0)
            track = 0x2f;
        else
            track = changetrack();
        if (report_kilrathi_rout(2) != 0)
            g_nCombatMusicActive_0049bf04 = 1;
    }
    spacetrack(track, 1, 0);
}

void ScaleFixedVector(FixedVector *vector, int scale, FixedVector *result);
void AddFixedVectors(const FixedVector *left, const FixedVector *right,
                     FixedVector *sum);
void ComputeVectorDelta(FixedVector *from, FixedVector *to,
                        FixedVector *delta);

/* Function start: 0x453240 */
void servicetrack(void)
{
    short volume;
    short ship;
    FixedVector lead;
    FixedVector predicted;

    if (g_nMusicDriverMode_0049be8c == 0)
        return;
    gametrack();
    if (g_bFlightSoundEffectsEnabled_0049beb0 != 0) {
        for (ship = 0; ship < SPACE_OBJECT_COUNT;
             ship++) {
            if (g_nPassingShipSoundObject_0049bf10 == ship &&
                g_aeObjectClass_00495328[ship] < OBJECT_CLASS_SHIP)
                g_nPassingShipSoundObject_0049bf10 = -1;
            switch (g_aeObjectClass_00495328[ship]) {
            case OBJECT_CLASS_SHIP:
            case OBJECT_CLASS_CAPITAL_SHIP:
            case OBJECT_CLASS_BASE:
                if (g_asObjectScreenX_00493598[ship] != -32767 &&
                    (unsigned short)g_asObjectDistance_00493ae8[ship] < 0x384) {
                    if (g_nPassingShipSoundObject_0049bf10 == -1) {
                        ScaleFixedVector(&g_aShipVelocity_00494898[ship], 0x1400,
                                         &lead);
                        AddFixedVectors(&g_aShipPosition_00494550[ship], &lead,
                                        &predicted);
                        ComputeVectorDelta(
                            &g_aShipPosition_00494550[EYE_OBJECT],
                            &predicted, &lead);
                        ComputeVectorDelta(
                            &g_aShipPosition_00494550[EYE_OBJECT],
                            &g_aShipPosition_00494550[ship], &predicted);
                        if (dot_product(&lead, &predicted) < 0xdd) {
                            if (ship == 0 && g_bJumpSequenceActive_004962f0 != 0)
                                continue;
                            g_nPassingShipSoundObject_0049bf10 = ship;
                            g_nPassingShipSoundCountdown_0049bf14 = 6;
                            volume = 12;
                            g_abSoundEffectDefinitions_0049bf18[0xa] =
                                (unsigned char)volume;
                            *(short *)&g_abSoundEffectDefinitions_0049bf18[0xc] = 2;
                            g_abSoundEffectDefinitions_0049bf18[0xe] = 0x70;
                            if (g_nSpaceFrame_00493134 >
                                g_nPassingShipSoundCooldown_005d156c) {
                                g_nPassingShipSoundCooldown_005d156c =
                                    g_nSpaceFrame_00493134 + 24;
                                PlaySfxWaveFileByNumber(2, ship, 0);
                            }
                        }
                    } else if (g_nPassingShipSoundObject_0049bf10 == ship) {
                        g_nPassingShipSoundCountdown_0049bf14--;
                        if (g_nPassingShipSoundCountdown_0049bf14 == 0)
                            g_nPassingShipSoundObject_0049bf10 = -1;
                    }
                }
                break;
            }
        }
    }
}

/* Function start: 0x453496 */
void ResetSoundState(void)
{
    FlushSoundEffects();
    g_bAfterburnerSfxActive_005d3864 = 0;
    g_nCriticalDamageWarningSfxHandle_005d1ec0 = 0;
}

/* Function start: 0x4534BA */
void ResetSoundStateForScene(void)
{
    ResetSoundState();
    g_bFlightSoundEffectsEnabled_0049beb0 = 0;
}

/* Function start: 0x4534D3 */
/* Disabled by the Win32 port: the flight sound reset follows the early
 * return. */
void ResetSoundStateForFlight(void)
{
    return;
    ResetSoundState();
    g_bFlightSoundEffectsEnabled_0049beb0 = 1;
}

/* Function start: 0x4534F1 */
/* Nothing in the shipped executable calls this; the Win32 port left the
 * counterpart of EnableMusicForScene empty. */
void DisableMusicForSceneHook(void)
{
}

/* Function start: 0x4534FC */
void EnableMusicForScene(void)
{
    g_nWaitForMusicEnabled_0049beac = 1;
    SetMusicOn(1);
}

/* Function start: 0x45351A */
/* Logs an effect the Win32 sound table does not know.  The DOS driver
 * playback follows the early return. */
int LogUnknownSoundEffect(const unsigned char *definition,
                          short sourceObject, short pan, short volume,
                          short looping, short priority)
{
    SoundDebugPrintf("Playing SFX <UNKNOWN>\n");
    return 0;
    if (g_bFlightSoundEffectsEnabled_0049beb0 != 0) {
        return StartSoundEffectHook(definition, sourceObject, pan, volume,
                                    looping, priority);
    } else {
        return 0;
    }
}

/* Function start: 0x45357E */
/* Stops every effect.  The DOS single-effect stop follows the early
 * return. */
void FlushSoundEffectsAndLog(void *effect, short flags)
{
    FlushSoundEffect();
    SoundDebugPrintf("flushFX\n");
    return;
    if (effect != 0)
        ((void (__cdecl *)(void *, int))FlushSoundEffect)(effect, flags);
}

/* Function start: 0x4535BB */
/* Logs the effect number.  The DOS positional playback (distance volume,
 * stereo pan and per-object handles) follows the early return. */
void sound_effect(signed char soundNumber, short sourceObject, short looping)
{
    short volume;
    short effectHandle = 0;
    int length;
    FixedVector offset;
    int side;
    short balance;
    short sampleIndex;

    SoundDebugPrintf("Playing SFX #%d\n", soundNumber);
    return;
    if (g_nMusicDriverMode_0049be8c == 0 ||
        g_bFlightSoundEffectsEnabled_0049beb0 == 0)
        return;
    ComputeVectorDelta(&g_aShipPosition_00494550[EYE_OBJECT],
                       &g_aShipPosition_00494550[sourceObject], &offset);
    length = Vector_magnitude(&offset);
    volume = MaxShort(0, (short)(0x7f - ((length / 500) >> 8)));
    if (volume < 10)
        return;
    if (g_nCannedSceneMode_0049021c == 0)
        RecordCannedSceneBriefingCharacter(soundNumber, sourceObject,
                                           looping);
    if (soundNumber == 8) {
        sampleIndex = (short)((g_nAlternateSampleToggle_005d1570 & 1) + 0x40);
        g_nAlternateSampleToggle_005d1570++;
    } else {
        sampleIndex = sourceObject;
    }
    NormalizeFixedVector(&offset);
    side = dot_product(&offset, &g_aShipRightVector_00493b78[EYE_OBJECT]);
    balance = (short)(0x40 - ((side << 6) >> 8));
    g_anObjectSoundEffects_005d1580[sourceObject] = StartSoundEffectHook(
        &g_abSoundEffectDefinitions_0049bf18[(soundNumber - 1) * 8],
        effectHandle, volume, balance, sampleIndex, looping);
}

/* Function start: 0x45373B */
void PlaySfxWaveFileByNumber(int soundNumber, int sourceObject, int looping)
{
    FixedVector delta;
    int magnitude;
    int distance;
    int channel;

#ifdef SDL_PORT
    /* A DOS install ships no sfx waves; the effect is synthesized from the
     * OriginFX record this number selects. */
    if (SdlUsingOriginFxSoundEffects()) {
        SdlPlayGameSoundEffect(soundNumber, sourceObject, looping);
        return;
    }
#endif
    if (sourceObject != -1) {
        ComputeVectorDelta(&g_aShipPosition_00494550[EYE_OBJECT],
                           &g_aShipPosition_00494550[sourceObject],
                           &delta);
        magnitude = Vector_magnitude(&delta);
        distance = magnitude;
        if (distance > 32000)
            distance = 32000;
        g_aiSoundEffectSourceActive_005d12c0[sourceObject] = 1;
    } else {
        distance = 72000;
    }
    if (distance < 10)
        return;
    soundNumber--;
    SoundDebugPrintf(
        "Playing SFX #%d on obj: %d with volume of %d\n",
        soundNumber, sourceObject, distance);
    sprintf(g_szSfxWavePath_005b3650, "sfx%02i.wav", soundNumber);
    playWAVE(g_szSfxWavePath_005b3650, looping, distance);
}
