/* SDL2-only OriginFX playback of WC2's DOS music and sound effects on an
 * AdLib or an MT-32, for DOS data and, by choice, Kilrathi Saga data.
 *
 *  A DOS install carries its music as GAMEDAT/MUSIC.A00: sixty-seven packet
 *  sections, one standard MIDI file each, indexed by the very track number the
 *  music script hands to ProcessMusicScriptCommand.  Eighteen of those sections
 *  are empty because the AdLib set never recorded a sequence for the track.
 *  GAMEDAT/WING2.TIM section 1 holds the hundred and twenty-four AdLib timbres
 *  the sequences play through.
 *  The orchestral title is the marker-synchronized sequence in MUSIC.R00
 *  section 19; it uses the same synthesizer and timbre bank here.
 *
 *  Sound effects are not sampled at all.  The game still carries the eight-byte
 *  OriginFX command records the DOS release drove its FM chip with, so the port
 *  reads that table straight out of the game's own memory and synthesizes the
 *  effects on the same YM3812 the music runs on.
 *
 *  The Kilrathi Saga release normally uses recordings.  It nevertheless
 *  retains the DOS music and timbre archives, which supply the restored title
 *  sequence while the regular Win32 streams and wave effects remain active.
 *  The player can choose the DOS scores instead, on an AdLib or an MT-32.
 *
 *  An MT-32 plays MUSIC.R00 (R01 and R02 for the expansions) with WING2.TIM
 *  section 0's instruments, as WC2.EXE does with sound option R, and plays
 *  the sound effects too, sharing its channels with the music
 *  (src/sdl/mt32.c).
 */
#include "game.h"

#include <stdio.h>
#include <stdlib.h>

#define SDL_MUSIC_PATH_SIZE 4096
/* WING2.TIM, like STRAX.DRV, has a section per device: 0 MT-32, 1 AdLib,
 * 2 PC speaker. */
#define SDL_MT32_TIMBRE_SECTION 0
#define SDL_ADLIB_TIMBRE_SECTION 1
/* The audio device's rate, which the OriginFX players render at. */
#define SDL_ORIGINFX_SAMPLE_RATE 22050
#define SDL_SOUND_RECORD_SIZE 8U
#define SDL_SOUND_RECORD_COUNT \
    (unsigned int)(sizeof(g_abSoundEffectDefinitions) / \
                   SDL_SOUND_RECORD_SIZE)
/* Guns fire faster than a single OriginFX channel can restart, so successive
 * shots alternate between two tags and overlap on two channels instead of
 * cutting one another off. */
#define SDL_GUN_SOUND_NUMBER 8
#define SDL_GUN_SOUND_FIRST_TAG 64
/* The afterburner record is held rather than timed, so it plays until the
 * flight code says the burn is over. */
#define SDL_AFTERBURNER_SOUND_NUMBER 12
/* Vector_magnitude measures in metres scaled by 256, and an OriginFX effect
 * carries a MIDI channel volume.  The DOS player drops one volume step per
 * five hundred metres. */
#define SDL_SOUND_METRES_PER_VOLUME_STEP 500L
#define SDL_SOUND_FULL_VOLUME 127
#define SDL_SOUND_AUDIBLE_VOLUME 10
#define SDL_SOUND_CENTRE_PAN 64

static SDL_mutex *g_pSdlDosMusicMutex;
static unsigned char *g_pSdlDosMusicArchive;
static unsigned char *g_pSdlDosAdlibTimbres;
static unsigned char *g_pSdlTitleMusicArchive;
static SdlOriginFxPlayer *g_pSdlOriginFxPlayer;
static SdlOriginFxPlayer *g_pSdlOriginFxSoundPlayer;
/* With MT-32 ROMs one MT-32 plays the music and the sound effects, which
 * share its channels as in WC2.EXE. */
static SdlMt32Synth *g_pSdlMt32;
static int g_nSdlMusicDevice = SDL_PORT_MUSIC_INSTALLED;
static size_t g_nSdlDosMusicArchiveSize;
static unsigned char *g_pSdlSo1MusicArchive;
static size_t g_nSdlSo1MusicArchiveSize;
static unsigned char *g_pSdlSo2MusicArchive;
static size_t g_nSdlSo2MusicArchiveSize;
static size_t g_nSdlDosAdlibTimbreSize;
static size_t g_nSdlTitleMusicArchiveSize;
static unsigned int g_nSdlDosMusicGain;
static unsigned int g_nSdlDosSoundGain;
static unsigned int g_nSdlDosRapidFireTag;
static int g_nSdlActiveMusicTrack = -1;
/* The track waiting for the playing song to end (DOS g_pendingScore). */
static int g_nSdlQueuedMusicTrack = -1;
static int g_nSdlMusicVolumeSetting = -1;
static int g_nSdlSoundVolumeSetting = -1;
static int g_bSdlDosMusicInitialized;
static int g_bSdlOriginFxMusicSelected;
static int g_bSdlOriginFxSoundSelected;
static int g_bSdlOriginalTitleMusicActive;

static unsigned char *SdlLoadDosMusicFile(
    const char *const *candidates, unsigned int candidateCount,
    size_t *fileSize)
{
    unsigned char *fileData;
    char resolved[SDL_MUSIC_PATH_SIZE];
    unsigned int candidateIndex;

    candidateIndex = 0;
    while (candidateIndex < candidateCount) {
        if (SdlResolvePath(
                candidates[candidateIndex], resolved, sizeof(resolved))) {
            fileData = (unsigned char *)SDL_LoadFile(resolved, fileSize);
            if (fileData != 0)
                return fileData;
        }
        candidateIndex++;
    }
    return 0;
}

static void SdlDeleteDosAdlibTrack(void)
{
    /* StopSong (062:11ee) silences the MT-32's music channels. */
    if (g_pSdlOriginFxPlayer != 0)
        SdlMt32StopSong(g_pSdlMt32);
    SdlDestroyOriginFxPlayer(g_pSdlOriginFxPlayer);
    g_pSdlOriginFxPlayer = 0;
    g_nSdlActiveMusicTrack = -1;
}

static unsigned int SdlCalculateDosAudioGain(int volumeSetting)
{
    int level;
    int tableIndex;

    tableIndex = volumeSetting / 2;
    if (tableIndex < 0)
        tableIndex = 0;
    else if (tableIndex > 10)
        tableIndex = 10;
    level = g_anVolumeLevels[tableIndex];
    if (level < 0)
        level = 0;
    else if (level > 64000)
        level = 64000;
    return (unsigned int)((long)level * 0x7fffL / 64000L);
}

static void SdlUpdateDosAdlibMusicVolume(void)
{
    if (g_nSdlMusicVolumeSetting == g_nMusicVolumeSetting &&
        g_nSdlSoundVolumeSetting == g_nSfxVolumeSetting)
        return;
    if (g_nSdlMusicVolumeSetting !=
        g_nMusicVolumeSetting) {
        g_nSdlMusicVolumeSetting = g_nMusicVolumeSetting;
        g_nSdlDosMusicGain =
            SdlCalculateDosAudioGain(g_nSdlMusicVolumeSetting);
    }
    if (g_nSdlSoundVolumeSetting !=
        g_nSfxVolumeSetting) {
        g_nSdlSoundVolumeSetting = g_nSfxVolumeSetting;
        g_nSdlDosSoundGain =
            SdlCalculateDosAudioGain(g_nSdlSoundVolumeSetting);
        /* Silent effects are off, and SoundEffectsOff (003:06fe) stops
         * the playing ones, which gives the MT-32's channels back to the
         * music. */
        if (g_nSdlDosSoundGain == 0) {
            if (g_pSdlMt32 != 0)
                SdlStopMt32Effects(g_pSdlMt32);
            else
                SdlStopOriginFxSoundEffects(g_pSdlOriginFxSoundPlayer);
        }
    }
    /* The MT-32 mixes the music and the effects itself, so the settings
     * scale their MIDI volumes instead, 256 being full. */
    SdlSetMt32Volumes(g_pSdlMt32,
                      (int)(g_nSdlDosMusicGain * 256U / 0x7fffU),
                      (int)(g_nSdlDosSoundGain * 256U / 0x7fffU));
}

void SdlSetMusicDevice(int device)
{
    g_nSdlMusicDevice = device;
}

int SdlGetMusicDevice(void)
{
    return g_nSdlMusicDevice;
}

/* The MT-32's song step for SdlMixMt32: one output frame of the song. The
 * service loop starts the queued song once this one has ended. */
static void SdlStepMt32Song(void *context)
{
    (void)context;
    if (g_pSdlOriginFxPlayer != 0)
        SdlStepOriginFxPlayer(g_pSdlOriginFxPlayer);
}

/* With MT-32 music chosen, one MT-32 with WING2.TIM's MT-32 instruments;
 * without usable ROMs the game keeps AdLib music. */
static void SdlStartMt32(const unsigned char *timbreArchive,
                         size_t timbreArchiveSize)
{
    unsigned char *timbres;
    size_t timbreSize;
    char description[PATH_MAX + 256];

    if (g_nSdlMusicDevice != SDL_PORT_MUSIC_MT32)
        return;
    if (!SdlDescribeMt32Roms(SdlGetMt32RomDirectory(), SdlGetMt32Machine(),
                             description, sizeof(description))) {
        fprintf(stderr, "MT-32 unavailable, playing AdLib music: %s\n", description);
        return;
    }
    if (!SdlExtractOriginPacketSection(timbreArchive, timbreArchiveSize,
                                       SDL_MT32_TIMBRE_SECTION,
                                       &timbres, &timbreSize)) {
        fprintf(stderr, "Unable to decode WING2.TIM's MT-32 instruments; "
                "playing AdLib music.\n");
        return;
    }
    g_pSdlMt32 = SdlCreateMt32Synth(SDL_ORIGINFX_SAMPLE_RATE, timbres, timbreSize,
                                    g_abSoundEffectDefinitions,
                                    SDL_SOUND_RECORD_COUNT);
    free(timbres);
    if (g_pSdlMt32 == 0) {
        fprintf(stderr, "MT-32 unavailable, playing AdLib music: %s\n", SDL_GetError());
        return;
    }
    fprintf(stderr, "MT-32: %s\n", description);
}

int SdlInitializeOriginFxAudio(int usingDosData)
{
    const char *adlibMusicCandidates[2] = {
        "GAMEDAT/MUSIC.A00",
        "MUSIC.A00"
    };
    const char *rolandMusicCandidates[2] = {
        "GAMEDAT/MUSIC.R00",
        "MUSIC.R00"
    };
    const char *titleMusicCandidates[2] = {
        "GAMEDAT/MUSIC.R00",
        "MUSIC.R00"
    };
    const char *adlibSo1MusicCandidates[2] = {
        "GAMEDAT/MUSIC.A01", "MUSIC.A01"
    };
    const char *rolandSo1MusicCandidates[2] = {
        "GAMEDAT/MUSIC.R01", "MUSIC.R01"
    };
    const char *adlibSo2MusicCandidates[2] = {
        "GAMEDAT/MUSIC.A02", "MUSIC.A02"
    };
    const char *rolandSo2MusicCandidates[2] = {
        "GAMEDAT/MUSIC.R02", "MUSIC.R02"
    };
    const char *timbreCandidates[2] = {
        "GAMEDAT/WING2.TIM",
        "WING2.TIM"
    };
    const char *const *musicCandidates;
    const char *const *so1MusicCandidates;
    const char *const *so2MusicCandidates;
    unsigned char *timbreArchive;
    size_t timbreArchiveSize;

    if (g_bSdlDosMusicInitialized != 0)
        return 1;
    timbreArchive = SdlLoadDosMusicFile(
        timbreCandidates, 2, &timbreArchiveSize);
    if (timbreArchive == 0) {
        fprintf(stderr, "Unable to load GAMEDAT/WING2.TIM.\n");
        goto failed;
    }
    if (!SdlExtractOriginPacketSection(
            timbreArchive, timbreArchiveSize,
            SDL_ADLIB_TIMBRE_SECTION,
            &g_pSdlDosAdlibTimbres,
            &g_nSdlDosAdlibTimbreSize)) {
        SDL_free(timbreArchive);
        fprintf(stderr, "Unable to decode OriginFX AdLib timbres.\n");
        goto failed;
    }
    SdlStartMt32(timbreArchive, timbreArchiveSize);
    SDL_free(timbreArchive);
    /* LoadMusicScores (127:08c7): MUSIC.R00 for the Roland, else MUSIC.A00;
     * the expansions' own scores likewise. */
    musicCandidates = g_pSdlMt32 != 0 ? rolandMusicCandidates : adlibMusicCandidates;
    so1MusicCandidates = g_pSdlMt32 != 0 ? rolandSo1MusicCandidates
                                         : adlibSo1MusicCandidates;
    so2MusicCandidates = g_pSdlMt32 != 0 ? rolandSo2MusicCandidates
                                         : adlibSo2MusicCandidates;
    if (g_nSdlCampaign != 0) {
        g_pSdlSo1MusicArchive = SdlLoadDosMusicFile(
            so1MusicCandidates, 2, &g_nSdlSo1MusicArchiveSize);
        if (g_pSdlSo1MusicArchive == 0)
            goto failed;
    }
    if (g_nSdlCampaign == 2) {
        g_pSdlSo2MusicArchive = SdlLoadDosMusicFile(
            so2MusicCandidates, 2, &g_nSdlSo2MusicArchiveSize);
        if (g_pSdlSo2MusicArchive == 0)
            goto failed;
    }
    if (usingDosData != 0 || g_nSdlCampaign != 0 ||
        g_nSdlMusicDevice != SDL_PORT_MUSIC_INSTALLED) {
        g_pSdlDosMusicArchive = SdlLoadDosMusicFile(
            musicCandidates, 2, &g_nSdlDosMusicArchiveSize);
        if (g_pSdlDosMusicArchive == 0) {
            fprintf(stderr, "Unable to load %s.\n", musicCandidates[0]);
            goto failed;
        }
    }
    g_pSdlTitleMusicArchive = SdlLoadDosMusicFile(
        titleMusicCandidates, 2, &g_nSdlTitleMusicArchiveSize);
    if (g_pSdlTitleMusicArchive == 0 &&
        usingDosData == 0) {
        fprintf(stderr, "Unable to load GAMEDAT/MUSIC.R00.\n");
        goto failed;
    }
    if (usingDosData != 0 && g_pSdlMt32 == 0) {
        /* The player reads the game's own record table because flight code
         * retunes entries in place. */
        g_pSdlOriginFxSoundPlayer = SdlCreateOriginFxSoundPlayer(
            g_abSoundEffectDefinitions,
            SDL_SOUND_RECORD_COUNT,
            g_pSdlDosAdlibTimbres,
            g_nSdlDosAdlibTimbreSize);
        if (g_pSdlOriginFxSoundPlayer == 0) {
            fprintf(stderr,
                    "Unable to initialize DOS AdLib sound effects.\n");
            goto failed;
        }
    }

    g_pSdlDosMusicMutex = SDL_CreateMutex();
    if (g_pSdlDosMusicMutex == 0)
        goto failed;
    g_bSdlDosMusicInitialized = 1;
    SdlUpdateDosAdlibMusicVolume();
    /* OriginFX plays every track for DOS data, the expansions and a
     * Kilrathi Saga install whose player chose the DOS scores. */
    if (usingDosData != 0 || g_nSdlCampaign != 0 ||
        g_nSdlMusicDevice != SDL_PORT_MUSIC_INSTALLED)
        g_bSdlOriginFxMusicSelected = 1;
    /* And the sound effects for DOS data, or on the MT-32, so that one MT-32
     * plays everything as in WC2.EXE. With digital or AdLib music the
     * Kilrathi Saga keeps its digital effects. */
    if (usingDosData != 0 || g_pSdlMt32 != 0)
        g_bSdlOriginFxSoundSelected = 1;
    if (usingDosData != 0) {
        fprintf(stderr, "DOS OriginFX audio enabled on the %s.\n",
                g_pSdlMt32 != 0 ? "MT-32" : "AdLib");
    } else if (g_bSdlOriginFxMusicSelected != 0 && g_bSdlCutsceneOnly == 0) {
        fprintf(stderr, "DOS music enabled on the %s.\n",
                g_pSdlMt32 != 0 ? "MT-32" : "AdLib");
    } else if (g_bSdlCutsceneOnly == 0) {
        fprintf(stderr, "Original orchestral intro music enabled.\n");
    }
    return 1;

failed:
    SdlShutdownOriginFxAudio();
    return 0;
}

int SdlUsingOriginFxMusic(void)
{
    return g_bSdlOriginFxMusicSelected;
}

int SdlUsingOriginFxSoundEffects(void)
{
    return g_bSdlOriginFxSoundSelected;
}

int SdlHandlesGameSoundEffects(void)
{
    return 1;
}

/* The music script normally queues a Kilrathi Saga stream here.  A DOS install
 * has no streams, so the track number becomes the MUSIC.A00 section the service
 * loop below plays, commanded as WC2's DOS MusicCommand (003:00fd) did: while
 * another track is queued, or for the one already playing, nothing happens.
 * Otherwise 1 and 3 queue the track to follow the playing song from that
 * song's next exit (QueueNextSong), so flight music changes on the beat and a
 * launch theme is not cut off.  0 plays at once, and so does 2, whose
 * cross-fade the game always asks to last one tick. */
int SdlOriginFxMusicCommand(int track, int command)
{
    if (g_bSdlOriginFxMusicSelected == 0 || track < 0)
        return 0;
    SDL_LockMutex(g_pSdlDosMusicMutex);
    if (g_nSdlQueuedMusicTrack >= 0 || track == g_nCurrentMusicTrack) {
        SDL_UnlockMutex(g_pSdlDosMusicMutex);
        return 0;
    }
    if ((command == 1 || command == 3) &&
        g_pSdlOriginFxPlayer != 0 &&
        !SdlOriginFxPlayerFinished(g_pSdlOriginFxPlayer) &&
        g_nSdlActiveMusicTrack == g_nCurrentMusicTrack) {
        g_nSdlQueuedMusicTrack = track;
        SdlLeaveOriginFxSongAtLoop(g_pSdlOriginFxPlayer);
    } else {
        g_nCurrentMusicTrack = track;
        g_nMusicTrackComplete = 0;
    }
    SDL_UnlockMutex(g_pSdlDosMusicMutex);
    return 1;
}

/* PlaySongNow (DOS 062:1980), as the gameflow's music opcode and the torpedo
 * lock use it: the song restarts at once and nothing stays queued. */
void SdlPlayOriginFxMusicNow(int track)
{
    if (g_bSdlOriginFxMusicSelected == 0)
        return;
    SDL_LockMutex(g_pSdlDosMusicMutex);
    g_nSdlQueuedMusicTrack = -1;
    if (track == g_nSdlActiveMusicTrack)
        SdlDeleteDosAdlibTrack();
    g_nCurrentMusicTrack = track;
    g_nMusicTrackComplete = 0;
    SDL_UnlockMutex(g_pSdlDosMusicMutex);
}

/* QueueNextSong(0, 0), which WC2 calls before waiting for the music to end:
 * the playing song leaves at its next exit instead of looping. */
void SdlLeaveOriginFxMusicAtLoop(void)
{
    if (g_bSdlOriginFxMusicSelected == 0)
        return;
    SDL_LockMutex(g_pSdlDosMusicMutex);
    SdlLeaveOriginFxSongAtLoop(g_pSdlOriginFxPlayer);
    SDL_UnlockMutex(g_pSdlDosMusicMutex);
}

/* FadeOutMusic(0) also drops the queued track. */
void SdlStopOriginFxMusic(void)
{
    if (g_bSdlOriginFxMusicSelected == 0)
        return;
    SDL_LockMutex(g_pSdlDosMusicMutex);
    g_nSdlQueuedMusicTrack = -1;
    SDL_UnlockMutex(g_pSdlDosMusicMutex);
}

int SdlOriginalTitleMusicReady(void)
{
    if (g_bSdlCutsceneOnly != 0)
        return 1;
    return g_bSdlDosMusicInitialized != 0 &&
        g_pSdlTitleMusicArchive != 0;
}

int SdlStartOriginalTitleMusic(void)
{
    if (!SdlOriginalTitleMusicReady())
        return 0;
    g_nTitleMusicSequenceStage = 0;
    g_nCurrentMusicTrack = 19;
    g_nMusicTrackComplete = 0;
    g_bSdlOriginalTitleMusicActive = 1;
    SdlServiceOriginFxMusic();
    if (g_nCurrentMusicTrack != 19)
        g_bSdlOriginalTitleMusicActive = 0;
    return g_nCurrentMusicTrack == 19;
}

static int SdlStartDosSoundEffect(int soundNumber, int volume, int pan,
                                     int tag, int priority)
{
    int result;

    if (g_bSdlDosMusicInitialized == 0 ||
        g_pSdlDosMusicMutex == 0 ||
        (g_pSdlOriginFxSoundPlayer == 0 && g_pSdlMt32 == 0))
        return 0;
    SDL_LockMutex(g_pSdlDosMusicMutex);
    /* PlayEffect (003:0748) starts nothing while the effects are off. */
    if (g_nSdlDosSoundGain == 0) {
        SDL_UnlockMutex(g_pSdlDosMusicMutex);
        return 0;
    }
    if (soundNumber == SDL_GUN_SOUND_NUMBER) {
        tag = SDL_GUN_SOUND_FIRST_TAG +
            (int)(g_nSdlDosRapidFireTag & 1U);
        g_nSdlDosRapidFireTag++;
    }
    if (g_pSdlMt32 != 0)
        result = SdlStartMt32Effect(
            g_pSdlMt32, (unsigned int)soundNumber, volume, pan, tag, priority);
    else
        result = SdlPlayOriginFxSoundEffect(
            g_pSdlOriginFxSoundPlayer,
            (unsigned int)soundNumber, volume, pan, tag, priority);
    SDL_UnlockMutex(g_pSdlDosMusicMutex);
    return result;
}

/* The pan, 0 left to 127 right as the DOS and Kilrathi Saga effect players
 * take it, for a unit direction from the camera. Its component along the
 * camera's right vector is positive for objects drawn right of the view's
 * centre. */
int SdlSoundEffectPan(const FixedVector *direction)
{
    int scaledPan;
    int pan;

    scaledPan = dot_product(direction, &g_aShipRightVector[ EYE_OBJECT]) *
        SDL_SOUND_CENTRE_PAN;
    if (scaledPan < 0)
        scaledPan = -((-scaledPan + 0xff) / 0x100);
    else
        scaledPan /= 0x100;
    pan = SDL_SOUND_CENTRE_PAN + scaledPan;
    if (pan < 0)
        pan = 0;
    else if (pan > 127)
        pan = 127;
    return pan;
}

int SdlPlayGameSoundEffect(int soundNumber, int sourceObject, int looping)
{
    FixedVector delta;
    long magnitude;
    int distance;
    int volume;
    int pan;

    magnitude = 0;
    pan = SDL_SOUND_CENTRE_PAN;
    if (sourceObject != -1) {
        if (sourceObject < 0 || sourceObject >=  SPACE_OBJECT_COUNT)
            return 0;
        ComputeVectorDelta(&g_aShipPosition[ EYE_OBJECT],
                           &g_aShipPosition[sourceObject],
                           &delta);
        magnitude = Vector_magnitude(&delta);
        NormalizeFixedVector(&delta);
        pan = SdlSoundEffectPan(&delta);
        g_aiSoundEffectSourceActiveValue[sourceObject] = 1;
    }

    if (g_bSdlOriginFxSoundSelected != 0) {
        volume = SDL_SOUND_FULL_VOLUME;
        if (sourceObject != -1) {
            volume -= (int)((magnitude /
                             SDL_SOUND_METRES_PER_VOLUME_STEP) >> 8);
        }
        if (volume < 0)
            volume = 0;
        if (volume < SDL_SOUND_AUDIBLE_VOLUME)
            return 0;
        if (!SdlStartDosSoundEffect(
                soundNumber, volume, pan, sourceObject, looping))
            return 0;
        if (sourceObject == -1) {
            g_3_bAfterburnerSfxActive =
                soundNumber == SDL_AFTERBURNER_SOUND_NUMBER;
        }
        return 1;
    }

    if (sourceObject != -1) {
        distance = magnitude > 32000 ? 32000 : (int)magnitude;
    } else {
        distance = 72000;
    }
    if (distance < 10)
        return 0;
    soundNumber--;
    SoundDebugPrintf(
        "Playing SFX #%d on obj: %d with volume of %d\n",
        soundNumber, sourceObject, distance);
    sprintf(g_szSfxWavePath, "sfx%02i.wav", soundNumber);
    SdlPlayWaveWithPan(
        g_szSfxWavePath, looping, distance, pan);
    return 1;
}

void SdlStopDosSoundEffects(void)
{
    /* stop_all_sounds calls this port-only hook before a cutscene releases
     * its object resources.  The armed mouth sprite is a non-owning pointer
     * into those resources, so it must not survive that sound teardown: a
     * later speech clip can otherwise make ServiceSoundSystem write through
     * the freed pointer when playback finishes. */
    g_pLinkedCutsceneSprite = 0;
    g_3_bAfterburnerSfxActive = 0;
    if (g_pSdlDosMusicMutex == 0 ||
        (g_pSdlOriginFxSoundPlayer == 0 && g_pSdlMt32 == 0))
        return;
    SDL_LockMutex(g_pSdlDosMusicMutex);
    if (g_pSdlMt32 != 0)
        SdlStopMt32Effects(g_pSdlMt32);
    else
        SdlStopOriginFxSoundEffects(g_pSdlOriginFxSoundPlayer);
    SDL_UnlockMutex(g_pSdlDosMusicMutex);
}

void SdlMixOriginFxAudio(short *samples, unsigned int frameCount)
{
    if (g_bSdlDosMusicInitialized == 0 ||
        g_pSdlDosMusicMutex == 0 || samples == 0)
        return;
    SDL_LockMutex(g_pSdlDosMusicMutex);
    if (g_pSdlMt32 != 0) {
        SdlMixMt32(g_pSdlMt32, samples, frameCount, 1, SdlStepMt32Song, 0);
        SDL_UnlockMutex(g_pSdlDosMusicMutex);
        return;
    }
    if (g_pSdlOriginFxPlayer != 0) {
        SdlMixOriginFxPlayer(
            g_pSdlOriginFxPlayer, samples,
            frameCount, g_nSdlDosMusicGain);
    }
    if (g_pSdlOriginFxSoundPlayer != 0) {
        SdlMixOriginFxSoundEffects(
            g_pSdlOriginFxSoundPlayer, samples,
            frameCount, g_nSdlDosSoundGain);
    }
    SDL_UnlockMutex(g_pSdlDosMusicMutex);
}

void SdlServiceOriginFxMusic(void)
{
    SdlOriginFxPlayer *player;
    const unsigned char *musicArchive;
    unsigned char *midi;
    size_t musicArchiveSize;
    size_t midiSize;
    int desiredTrack;
    int finishedTrack;
    int section;

    if (g_bSdlDosMusicInitialized == 0)
        return;
    SDL_LockMutex(g_pSdlDosMusicMutex);
    SdlUpdateDosAdlibMusicVolume();
    desiredTrack = g_nCurrentMusicTrack;
    if (g_bSdlOriginalTitleMusicActive != 0 &&
        desiredTrack != 19)
        g_bSdlOriginalTitleMusicActive = 0;
    if (g_bSdlOriginalTitleMusicActive != 0 &&
        g_nSdlActiveMusicTrack == 19 &&
        g_pSdlOriginFxPlayer != 0) {
        g_nTitleMusicSequenceStage = (short)
            SdlOriginFxPlayerSequencePosition(
                g_pSdlOriginFxPlayer);
    }
    if (g_bSdlOriginFxMusicSelected == 0 &&
        g_bSdlOriginalTitleMusicActive == 0) {
        SdlDeleteDosAdlibTrack();
        SDL_UnlockMutex(g_pSdlDosMusicMutex);
        return;
    }
    if (g_pSdlOriginFxPlayer != 0 &&
        SdlOriginFxPlayerFinished(g_pSdlOriginFxPlayer)) {
        finishedTrack = g_nSdlActiveMusicTrack;
        SdlDeleteDosAdlibTrack();
        if (finishedTrack == 19)
            g_bSdlOriginalTitleMusicActive = 0;
        if (g_nSdlQueuedMusicTrack >= 0) {
            /* The timer handler starts the queued song once the playing one
             * has stopped (062:125c). */
            g_nCurrentMusicTrack = g_nSdlQueuedMusicTrack;
            g_nSdlQueuedMusicTrack = -1;
        } else {
            g_nMusicTrackComplete = 1;
            if (g_nCurrentMusicTrack == finishedTrack)
                g_nCurrentMusicTrack = -1;
        }
    }
    /* The gameflow's opcode 0xb4 value selects the branches of a song's 0xFE
     * type 4 and 5 events (DOS g_musicBranch). */
    SdlSetOriginFxMusicBranch(g_pSdlOriginFxPlayer, g_nCutsceneMusicVolume);
    desiredTrack = g_nCurrentMusicTrack;
    if (desiredTrack == g_nSdlActiveMusicTrack) {
        SDL_UnlockMutex(g_pSdlDosMusicMutex);
        return;
    }
    if (desiredTrack < 0) {
        SdlDeleteDosAdlibTrack();
        g_nSdlQueuedMusicTrack = -1;
        g_nMusicTrackComplete = 1;
        SDL_UnlockMutex(g_pSdlDosMusicMutex);
        return;
    }
    SDL_UnlockMutex(g_pSdlDosMusicMutex);

    midi = 0;
    midiSize = 0;
    musicArchive = g_pSdlDosMusicArchive;
    musicArchiveSize = g_nSdlDosMusicArchiveSize;
    section = desiredTrack;
    /* Both expansions append MUSIC.R01 sections 0, 1, 2 after the 67 base
     * tracks. SO2 also appends MUSIC.R02 section 0. The AdLib archives use
     * the same section numbers as these GAMEFLOW MUS tables; with an MT-32
     * these archives are the Roland ones themselves. */
    if (g_nSdlCampaign != 0 && desiredTrack >= 67 && desiredTrack <= 69) {
        musicArchive = g_pSdlSo1MusicArchive;
        musicArchiveSize = g_nSdlSo1MusicArchiveSize;
        section = desiredTrack - 67;
    }
    if (g_nSdlCampaign == 2 && desiredTrack == 70) {
        musicArchive = g_pSdlSo2MusicArchive;
        musicArchiveSize = g_nSdlSo2MusicArchiveSize;
        section = 0;
    }
    if (g_bSdlOriginalTitleMusicActive != 0 &&
        desiredTrack == 19) {
        musicArchive = g_pSdlTitleMusicArchive;
        musicArchiveSize = g_nSdlTitleMusicArchiveSize;
    }
    /* Eighteen of the sections the script can ask for are empty, so a track
     * without a sequence is data, not damage: report it finished and let the
     * script pick the next one. */
    if (!SdlExtractOriginPacketSection(
            musicArchive, musicArchiveSize,
            (unsigned int)section, &midi, &midiSize)) {
        SoundDebugPrintf(
            "no AdLib sequence for track %d\n", desiredTrack);
        g_nCurrentMusicTrack = -1;
        g_nMusicTrackComplete = 1;
        return;
    }
    player = g_pSdlMt32 != 0
        ? SdlCreateOriginFxMt32Player(midi, midiSize, g_pSdlMt32)
        : SdlCreateOriginFxPlayer(
            midi, midiSize, g_pSdlDosAdlibTimbres,
            g_nSdlDosAdlibTimbreSize);
    free(midi);
    if (player == 0) {
        fprintf(stderr, "Unable to parse OriginFX music track %d.\n",
                desiredTrack);
        g_nCurrentMusicTrack = -1;
        g_nMusicTrackComplete = 1;
        return;
    }

    SDL_LockMutex(g_pSdlDosMusicMutex);
    if (g_nCurrentMusicTrack != desiredTrack) {
        SDL_UnlockMutex(g_pSdlDosMusicMutex);
        SdlDestroyOriginFxPlayer(player);
        return;
    }
    SdlDeleteDosAdlibTrack();
    SdlSetOriginFxMusicBranch(player, g_nCutsceneMusicVolume);
    g_pSdlOriginFxPlayer = player;
    g_nSdlActiveMusicTrack = desiredTrack;
    g_nMusicTrackComplete = 0;
    SDL_UnlockMutex(g_pSdlDosMusicMutex);
}

void SdlShutdownOriginFxAudio(void)
{
    if (g_pSdlDosMusicMutex != 0)
        SDL_LockMutex(g_pSdlDosMusicMutex);
    SdlDeleteDosAdlibTrack();
    SdlStopOriginFxSoundEffects(g_pSdlOriginFxSoundPlayer);
    SdlDestroyOriginFxPlayer(g_pSdlOriginFxSoundPlayer);
    g_pSdlOriginFxSoundPlayer = 0;
    SdlDestroyMt32Synth(g_pSdlMt32);
    g_pSdlMt32 = 0;
    if (g_pSdlDosMusicMutex != 0)
        SDL_UnlockMutex(g_pSdlDosMusicMutex);
    if (g_pSdlDosMusicMutex != 0) {
        SDL_DestroyMutex(g_pSdlDosMusicMutex);
        g_pSdlDosMusicMutex = 0;
    }
    SDL_free(g_pSdlDosMusicArchive);
    g_pSdlDosMusicArchive = 0;
    SDL_free(g_pSdlSo1MusicArchive);
    g_pSdlSo1MusicArchive = 0;
    g_nSdlSo1MusicArchiveSize = 0;
    SDL_free(g_pSdlSo2MusicArchive);
    g_pSdlSo2MusicArchive = 0;
    g_nSdlSo2MusicArchiveSize = 0;
    SDL_free(g_pSdlTitleMusicArchive);
    g_pSdlTitleMusicArchive = 0;
    free(g_pSdlDosAdlibTimbres);
    g_pSdlDosAdlibTimbres = 0;
    g_nSdlDosMusicArchiveSize = 0;
    g_nSdlDosAdlibTimbreSize = 0;
    g_nSdlTitleMusicArchiveSize = 0;
    g_nSdlMusicVolumeSetting = -1;
    g_nSdlSoundVolumeSetting = -1;
    g_nSdlDosMusicGain = 0;
    g_nSdlDosSoundGain = 0;
    g_nSdlDosRapidFireTag = 0;
    g_nSdlQueuedMusicTrack = -1;
    g_bSdlOriginFxMusicSelected = 0;
    g_bSdlOriginFxSoundSelected = 0;
    g_bSdlOriginalTitleMusicActive = 0;
    g_bSdlDosMusicInitialized = 0;
}
