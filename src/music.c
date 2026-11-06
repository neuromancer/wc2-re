/*
 *  Music and sound-effect control.
 *
 *  Retail object 0x452A00-0x45381F (link order; see docs/ORDER.md).
 */
/* This unit owns storage declared in globals.h as statics. */
#define MUSIC_C
#include "game.h"

static int g_nActiveMusicStreamMask; /* 0x0049be9c */
static signed char g_acMusicTrackStreamFlags[72]; /* 0x0049beb8 */

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
    g_nCurrentMusicTrack = -1;
    Streamer_stop();
}

/* Function start: 0x452A40 */
short StartMusic(void *music)
{
    SoundDebugPrintf("startMusic %p\n", music);
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
    SoundDebugPrintf("FLUSH EFFECT\n");
    stop_all_sounds();
}

/* Function start: 0x452AC8 */
void FlushSoundEffects(void)
{
    SoundDebugPrintf("FLUSH EFFECTS\n");
    stop_all_sounds();
}

/* Function start: 0x452AE5 */
void FadeMusic(short duration)
{
}

/* Function start: 0x452AF0 */
void SceneLeaveHook(void)
{
    /* The body was compiled out; its one local still takes a slot. */
    int state;

    return;
}

/* Function start: 0x452B03 */
unsigned short ProcessMusicScriptCommand(int track, int command, short enabled)
{
    int streamFlags;
    int streamerCommand;

    if (track == -1)
        return 0;

    streamFlags = g_acMusicTrackStreamFlags[track];
    SoundDebugPrintf("queue_next_music %d %x\n", track, streamFlags);
    if (track >= 0x40)
        streamerCommand = track - 0x2c;
    else
        streamerCommand = track;

    if (command == 4) {
        SoundDebugPrintf("queue_stop\n");
        StopMusic(enabled);
        g_nCurrentMusicTrack = -1;
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
    if ((streamFlags & g_nActiveMusicStreamMask) == 0) {
        if ((streamFlags & 1) != 0) {
            Streamer_open("gameflow.str");
            g_nActiveMusicStreamMask = 1;
        } else if ((streamFlags & 2) != 0) {
            Streamer_open("spaceflt.str");
            g_nActiveMusicStreamMask = 2;
        } else if ((streamFlags & 0x10) != 0) {
            Streamer_open("gametwo.str");
            g_nActiveMusicStreamMask = 0x10;
        } else {
            SoundDebugPrintf("**** unknown stream for track %d\n", track);
            StopMusic(0);
            return;
        }
        SetStreamerIntensity(0x32);
    } else {
        g_nCurrentMusicTrack = track;
    }

    if (g_nActiveMusicStreamMask == 2) {
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

    if (g_nCannedSceneMode == 0)
        RecordMusicCommandHook(track, command, enabled);
    return 1;
}

/* Function start: 0x452CF4 */
void spacetrack(int track, int mode, int enabled)
{
    if (g_nMusicDriverMode != 0 &&
        g_nMusicDriverMode != 3)
        ProcessMusicScriptCommand(track, mode, enabled);
}

/* Function start: 0x452D32 */
void StopMusicUnlessSuppressed(void)
{
    if (g_nMusicDriverMode != 0 &&
        g_nMusicDriverMode != 3)
        StopMusic(0);
}

/* Function start: 0x452D66 */
unsigned int IsMusicTrackComplete(void)
{
    if (g_nMusicDriverMode != 0 &&
        g_nMusicDriverMode != 3 &&
        g_nMusicTrackComplete != 0)
        return 1;
    return 0;
}

/* Function start: 0x452DB0 */
void wait_for_end_of_music(void)
{
    if (g_nMusicDriverMode != 0 &&
        g_nMusicDriverMode != 3) {
        if (g_nWaitForMusicEnabled == 0) {
            StopMusic(0);
            return;
        }
        SetMusBreakpt(0, 0);
        while (g_nMusicTrackComplete == 0) {
            if (g_bSceneEscapeRequested != 0 ||
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
    if (g_nInFlightMusicActive != 0) {
        if (g_asShipSide[victim] == SIDE_KILRATHI) {
            if (report_kilrathi_rout(1) == 0) {
                spacetrack(10, 1, 0);
            } else if (attacker == 0) {
                if (g_acShipPortrait[victim] != -1 ||
                    RandomInRange(0, 3) == 0)
                    spacetrack(9, 3, 0);
                else
                    spacetrack(6, 3, 0);
            }
        } else {
            if (g_nYourWingman == victim) {
                spacetrack(8, 3, 0);
                return;
            }
            if (g_asShipSide[victim] == SIDE_IMPERIAL) {
                if ((g_asShipMissionType[0] ==
                         MISSION_TYPE_DEFEND ||
                     g_asShipMissionType[0] ==
                         MISSION_TYPE_ESCORT) &&
                    g_asShipMissionIndex[victim] ==
                        g_asShipMissionParameter[0]) {
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

    switch (g_asShipMissionType[0]) {
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
    if (g_aMissionObjectives[
            g_cCurrentObjective].type == OBJECTIVE_HOME_BASE) {
        if (triumph(0) != 0) {
            if (g_asShipMissionType[0] != MISSION_TYPE_PATROL)
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
    if (g_nInFlightMusicActive == 0)
        return;
    if (g_nCombatMusicActive != 0) {
        if ((g_nSpaceFrame & 0xf) == 0 ||
            g_nMusicTrackComplete != 0) {
            if (g_nInitialFlightMusicPending != 0)
                g_nInitialFlightMusicPending = 0;
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
            if (g_nMissionMusicTrackOverride != 0)
                track = g_nMissionMusicTrackOverride;
            if (report_kilrathi_rout(1) == 0)
                g_nCombatMusicActive = 0;
        }
    } else if ((g_nSpaceFrame & 0xf) == 0 ||
               g_nMusicTrackComplete != 0) {
        if (g_bJumpSequenceActive != 0)
            track = 0x2f;
        else
            track = changetrack();
        if (report_kilrathi_rout(2) != 0)
            g_nCombatMusicActive = 1;
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

    if (g_nMusicDriverMode == 0)
        return;
    gametrack();
    if (g_bFlightSoundEffectsEnabled != 0) {
        for (ship = 0; ship < SPACE_OBJECT_COUNT;
             ship++) {
            if (g_nPassingShipSoundObject == ship &&
                g_aeObjectClass[ship] < OBJECT_CLASS_SHIP)
                g_nPassingShipSoundObject = -1;
            switch (g_aeObjectClass[ship]) {
            case OBJECT_CLASS_SHIP:
            case OBJECT_CLASS_CAPITAL_SHIP:
            case OBJECT_CLASS_BASE:
                if (g_asObjectScreenX[ship] != -32767 &&
                    (unsigned short)g_asObjectDistance[ship] < 0x384) {
                    if (g_nPassingShipSoundObject == -1) {
                        ScaleFixedVector(&g_aShipVelocity[ship], 0x1400,
                                         &lead);
                        AddFixedVectors(&g_aShipPosition[ship], &lead,
                                        &predicted);
                        ComputeVectorDelta(
                            &g_aShipPosition[EYE_OBJECT],
                            &predicted, &lead);
                        ComputeVectorDelta(
                            &g_aShipPosition[EYE_OBJECT],
                            &g_aShipPosition[ship], &predicted);
                        if (dot_product(&lead, &predicted) < 0xdd) {
                            if (ship == 0 && g_bJumpSequenceActive != 0)
                                continue;
                            g_nPassingShipSoundObject = ship;
                            g_nPassingShipSoundCountdown = 6;
                            volume = 12;
                            g_abSoundEffectDefinitions[0xa] =
                                (unsigned char)volume;
                            *(short *)&g_abSoundEffectDefinitions[0xc] = 2;
                            g_abSoundEffectDefinitions[0xe] = 0x70;
                            if (g_nSpaceFrame >
                                g_nPassingShipSndCooldown) {
                                g_nPassingShipSndCooldown =
                                    g_nSpaceFrame + 24;
                                PlaySfxWaveFileByNumber(2, ship, 0);
                            }
                        }
                    } else if (g_nPassingShipSoundObject == ship) {
                        g_nPassingShipSoundCountdown--;
                        if (g_nPassingShipSoundCountdown == 0)
                            g_nPassingShipSoundObject = -1;
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
    g_3_bAfterburnerSfxActive = 0;
    g_4_nSevereDmgWarningSfxHandle = 0;
}

/* Function start: 0x4534BA */
void ResetSoundStateForScene(void)
{
    ResetSoundState();
    g_bFlightSoundEffectsEnabled = 0;
}

/* Function start: 0x4534D3 */
/* Disabled by the Win32 port: the flight sound reset follows the early
 * return. */
void ResetSoundStateForFlight(void)
{
    return;
    ResetSoundState();
    g_bFlightSoundEffectsEnabled = 1;
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
    g_nWaitForMusicEnabled = 1;
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
    if (g_bFlightSoundEffectsEnabled != 0) {
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
    if (g_nMusicDriverMode == 0 ||
        g_bFlightSoundEffectsEnabled == 0)
        return;
    ComputeVectorDelta(&g_aShipPosition[EYE_OBJECT],
                       &g_aShipPosition[sourceObject], &offset);
    length = Vector_magnitude(&offset);
    volume = MaxShort(0, (short)(0x7f - ((length / 500) >> 8)));
    if (volume < 10)
        return;
    if (g_nCannedSceneMode == 0)
        RecordCannedSceneBriefingCharacter(soundNumber, sourceObject,
                                           looping);
    if (soundNumber == 8) {
        sampleIndex = (short)((g_10_nAlternateSampleToggle & 1) + 0x40);
        g_10_nAlternateSampleToggle++;
    } else {
        sampleIndex = sourceObject;
    }
    NormalizeFixedVector(&offset);
    side = dot_product(&offset, &g_aShipRightVector[EYE_OBJECT]);
    balance = (short)(0x40 - ((side << 6) >> 8));
    g_14_anObjectSoundEffects[sourceObject] = StartSoundEffectHook(
        &g_abSoundEffectDefinitions[(soundNumber - 1) * 8],
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
        ComputeVectorDelta(&g_aShipPosition[EYE_OBJECT],
                           &g_aShipPosition[sourceObject],
                           &delta);
        magnitude = Vector_magnitude(&delta);
        distance = magnitude;
        if (distance > 32000)
            distance = 32000;
        g_aiSoundEffectSourceActiveValue[sourceObject] = 1;
    } else {
        distance = 72000;
    }
    if (distance < 10)
        return;
    soundNumber--;
    SoundDebugPrintf(
        "Playing SFX #%d on obj: %d with volume of %d\n",
        soundNumber, sourceObject, distance);
    sprintf(g_szSfxWavePath, "sfx%02i.wav", soundNumber);
    playWAVE(g_szSfxWavePath, looping, distance);
}

/* This unit's initialized data, after its functions so that the tables do
 * not move the functions' symbol keys. */
short g_nMusicTrackComplete = 1; /* 0x0049be88 */
short g_nMusicDriverMode = 1; /* 0x0049be8c */
short g_nCutsceneMusicVolume = 0; /* 0x0049be90 */
short g_nTitleMusicSequenceStage = 0; /* 0x0049be94 */
int g_nCurrentMusicTrack = -1; /* 0x0049be98 */
static int g_nActiveMusicStreamMask = 0; /* 0x0049be9c */
struct LimitedMusicBuffer g_aLimitedMusicBuffers[2] = {
    { 0, -1 },
    { 0, -1 }
}; /* 0x0049bea0 */
short g_nWaitForMusicEnabled = 1; /* 0x0049beac */
short g_bFlightSoundEffectsEnabled = 1; /* 0x0049beb0 */
static signed char g_acMusicTrackStreamFlags[72] = {
    6, 6, 6, 6, 6, 6, 10, 10, 10, 10, 10, 10,
    6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0,
    0, 0, 0, 6, 16, 16, 0, 6, 6, 16, 16, 0,
    16, 16, 16, 16, 16, 6, 10, 1, 1, 1, 1, 7,
    1, 1, 1, 1, 1, 1, 1, 6, 6, 6, 1, 16,
    16, 1, 1, 16, 16, 16, 0, 0, 0, 0, 0, 0
}; /* 0x0049beb8 */
short g_nInitialFlightMusicPending = 1; /* 0x0049bf00 */
short g_nCombatMusicActive = 0; /* 0x0049bf04 */
short g_nInFlightMusicActive = 0; /* 0x0049bf08 */
int g_nUnusedMusicWord1 = 0; /* 0x0049bf0c */
short g_nPassingShipSoundObject = -1; /* 0x0049bf10 */
short g_nPassingShipSoundCountdown = 0; /* 0x0049bf14 */
unsigned char g_abSoundEffectDefinitions[0x1b8] = {
    0, 104, 48, 64, 120, 0, 0, 0,
    2, 55, 48, 96, 2, 0, 0, 0,
    0, 39, 48, 64, 60, 0, 0, 0,
    0, 100, 48, 80, 30, 0, 0, 0,
    0, 7, 48, 127, 6, 0, 0, 0,
    0, 16, 48, 64, 30, 0, 0, 0,
    0, 9, 48, 64, 10, 0, 0, 0,
    0, 10, 48, 127, 15, 0, 0, 0,
    0, 11, 48, 64, 6, 0, 0, 0,
    0, 12, 48, 64, 10, 0, 0, 0,
    4, 13, 48, 64, 60, 0, 0, 0,
    4, 14, 64, 64, 60, 0, 0, 0,
    0, 15, 48, 64, 6, 0, 0, 0,
    8, 19, 48, 64, 60, 0, 0, 0,
    4, 20, 64, 64, 60, 0, 0, 0,
    0, 21, 48, 64, 60, 0, 0, 0,
    4, 22, 48, 64, 60, 0, 0, 0,
    2, 23, 119, 64, 5, 0, 52, 0,
    0, 24, 48, 96, 60, 0, 0, 0,
    2, 41, 24, 24, 2, 0, 100, 0,
    0, 42, 64, 64, 40, 0, 0, 0,
    0, 43, 64, 64, 40, 0, 0, 0,
    0, 61, 48, 64, 7, 0, 0, 0,
    0, 46, 48, 64, 7, 0, 0, 0,
    0, 47, 48, 64, 7, 0, 0, 0,
    0, 60, 48, 64, 20, 0, 0, 0,
    0, 64, 48, 127, 40, 0, 0, 0,
    0, 125, 48, 108, 40, 0, 0, 0,
    0, 54, 48, 96, 30, 0, 0, 0,
    0, 96, 64, 127, 40, 0, 0, 0,
    0, 106, 64, 86, 60, 0, 0, 0,
    8, 107, 48, 64, 40, 0, 0, 0,
    0, 110, 48, 96, 30, 0, 0, 0,
    0, 109, 48, 64, 80, 0, 0, 0,
    0, 111, 48, 64, 30, 0, 0, 0,
    0, 112, 48, 64, 60, 0, 0, 0,
    0, 128, 48, 64, 60, 0, 0, 0,
    4, 32, 64, 64, 60, 0, 0, 0,
    0, 126, 40, 96, 5, 0, 0, 0,
    0, 40, 48, 96, 180, 0, 0, 0,
    0, 44, 64, 96, 90, 0, 0, 0,
    0, 116, 48, 127, 20, 0, 0, 0,
    0, 117, 48, 127, 20, 0, 0, 0,
    0, 31, 64, 96, 200, 0, 0, 0,
    4, 84, 12, 127, 90, 0, 0, 0,
    0, 81, 64, 96, 20, 0, 0, 0,
    0, 18, 64, 84, 30, 0, 0, 0,
    4, 88, 12, 127, 90, 0, 0, 0,
    0, 68, 36, 96, 90, 0, 0, 0,
    0, 35, 36, 96, 60, 0, 0, 0,
    0, 35, 36, 96, 60, 0, 0, 0,
    0, 82, 60, 96, 60, 0, 0, 0,
    4, 77, 48, 96, 200, 0, 0, 0,
    4, 83, 48, 127, 90, 0, 0, 0,
    0, 92, 48, 96, 120
}; /* 0x0049bf18 */

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
int g_aiSoundEffectSourceActiveValue[SPACE_OBJECT_COUNT]; /* 0x005d12c0 */
MusicResource g_aTuneResources[66]; /* 0x005d13e0 */
int g_nPassingShipSndCooldown; /* 0x005d156c */
short g_10_nAlternateSampleToggle; /* 0x005d1570 */
int g_14_anObjectSoundEffects[SPACE_OBJECT_COUNT]; /* 0x005d1580 */
int g_2_nUnusedMusicWord2; /* 0x005d1698 */ /* never referenced */
