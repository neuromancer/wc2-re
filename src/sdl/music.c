/* SDL2-only OriginFX/AdLib playback for Wing Commander II data.
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
 */
#include "wc1.h"

#include <stdio.h>
#include <stdlib.h>

#define WC1_SDL_MUSIC_PATH_SIZE 4096
#define WC1_SDL_ADLIB_TIMBRE_SECTION 1
#define WC2_SDL_SOUND_RECORD_SIZE 8U
#define WC2_SDL_SOUND_RECORD_COUNT \
    (unsigned int)(sizeof(g_abSoundEffectDefinitions_0049bf18) / \
                   WC2_SDL_SOUND_RECORD_SIZE)
/* Guns fire faster than a single OriginFX channel can restart, so successive
 * shots alternate between two tags and overlap on two channels instead of
 * cutting one another off. */
#define WC2_SDL_GUN_SOUND_NUMBER 8
#define WC2_SDL_GUN_SOUND_FIRST_TAG 64
/* The afterburner record is held rather than timed, so it plays until the
 * flight code says the burn is over. */
#define WC2_SDL_AFTERBURNER_SOUND_NUMBER 12
/* Vector_magnitude measures in metres scaled by 256, and an OriginFX effect
 * carries a MIDI channel volume.  The DOS player drops one volume step per
 * five hundred metres. */
#define WC2_SDL_SOUND_METRES_PER_VOLUME_STEP 500L
#define WC2_SDL_SOUND_FULL_VOLUME 127
#define WC2_SDL_SOUND_AUDIBLE_VOLUME 10
#define WC2_SDL_SOUND_CENTRE_PAN 64

static SDL_mutex *g_pWc1SdlDosMusicMutex;
static unsigned char *g_pWc1SdlDosMusicArchive;
static unsigned char *g_pWc1SdlDosAdlibTimbres;
static unsigned char *g_pWc2SdlTitleMusicArchive;
static Wc1SdlOriginFxPlayer *g_pWc1SdlOriginFxPlayer;
static Wc1SdlOriginFxPlayer *g_pWc1SdlOriginFxSoundPlayer;
static size_t g_nWc1SdlDosMusicArchiveSize;
static size_t g_nWc1SdlDosAdlibTimbreSize;
static size_t g_nWc2SdlTitleMusicArchiveSize;
static unsigned int g_nWc1SdlDosMusicGain;
static unsigned int g_nWc1SdlDosSoundGain;
static unsigned int g_nWc1SdlDosRapidFireTag;
static int g_nWc1SdlActiveMusicTrack = -1;
static int g_nWc1SdlMusicVolumeSetting = -1;
static int g_nWc1SdlSoundVolumeSetting = -1;
static int g_bWc1SdlDosMusicInitialized;
static int g_bWc1SdlOriginFxMusicSelected;
static int g_bWc1SdlOriginFxSoundSelected;
static int g_bWc2SdlOriginalTitleMusicActive;

static unsigned char *Wc1SdlLoadDosMusicFile(
    const char *const *candidates, unsigned int candidateCount,
    size_t *fileSize)
{
    unsigned char *fileData;
    char resolved[WC1_SDL_MUSIC_PATH_SIZE];
    unsigned int candidateIndex;

    candidateIndex = 0;
    while (candidateIndex < candidateCount) {
        if (Wc1SdlResolvePath(
                candidates[candidateIndex], resolved, sizeof(resolved))) {
            fileData = (unsigned char *)SDL_LoadFile(resolved, fileSize);
            if (fileData != 0)
                return fileData;
        }
        candidateIndex++;
    }
    return 0;
}

static void Wc1SdlDeleteDosAdlibTrack(void)
{
    Wc1SdlDestroyOriginFxPlayer(g_pWc1SdlOriginFxPlayer);
    g_pWc1SdlOriginFxPlayer = 0;
    g_nWc1SdlActiveMusicTrack = -1;
}

static unsigned int Wc1SdlCalculateDosAudioGain(int volumeSetting)
{
    int level;
    int tableIndex;

    tableIndex = volumeSetting / 2;
    if (tableIndex < 0)
        tableIndex = 0;
    else if (tableIndex > 10)
        tableIndex = 10;
    level = g_anVolumeLevels_0049d720[tableIndex];
    if (level < 0)
        level = 0;
    else if (level > 64000)
        level = 64000;
    return (unsigned int)((long)level * 0x7fffL / 64000L);
}

static void Wc1SdlUpdateDosAdlibMusicVolume(void)
{
    if (g_nWc1SdlMusicVolumeSetting == g_nMusicVolumeSetting_0049d750 &&
        g_nWc1SdlSoundVolumeSetting == g_nSfxVolumeSetting_0049d74c)
        return;
    if (g_nWc1SdlMusicVolumeSetting !=
        g_nMusicVolumeSetting_0049d750) {
        g_nWc1SdlMusicVolumeSetting = g_nMusicVolumeSetting_0049d750;
        g_nWc1SdlDosMusicGain =
            Wc1SdlCalculateDosAudioGain(g_nWc1SdlMusicVolumeSetting);
    }
    if (g_nWc1SdlSoundVolumeSetting !=
        g_nSfxVolumeSetting_0049d74c) {
        g_nWc1SdlSoundVolumeSetting = g_nSfxVolumeSetting_0049d74c;
        g_nWc1SdlDosSoundGain =
            Wc1SdlCalculateDosAudioGain(g_nWc1SdlSoundVolumeSetting);
    }
}

int Wc1SdlInitializeOriginFxAudio(int usingDosData)
{
    const char *musicCandidates[2] = {
        "GAMEDAT/MUSIC.A00",
        "MUSIC.A00"
    };
    const char *titleMusicCandidates[2] = {
        "GAMEDAT/MUSIC.R00",
        "MUSIC.R00"
    };
    const char *timbreCandidates[2] = {
        "GAMEDAT/WING2.TIM",
        "WING2.TIM"
    };
    unsigned char *timbreArchive;
    size_t timbreArchiveSize;

    if (g_bWc1SdlDosMusicInitialized != 0)
        return 1;
    if (usingDosData != 0) {
        g_pWc1SdlDosMusicArchive = Wc1SdlLoadDosMusicFile(
            musicCandidates, 2, &g_nWc1SdlDosMusicArchiveSize);
        if (g_pWc1SdlDosMusicArchive == 0) {
            fprintf(stderr, "Unable to load GAMEDAT/MUSIC.A00.\n");
            return 0;
        }
    }
    g_pWc2SdlTitleMusicArchive = Wc1SdlLoadDosMusicFile(
        titleMusicCandidates, 2, &g_nWc2SdlTitleMusicArchiveSize);
    if (g_pWc2SdlTitleMusicArchive == 0 &&
        usingDosData == 0) {
        fprintf(stderr, "Unable to load GAMEDAT/MUSIC.R00.\n");
        goto failed;
    }
    timbreArchive = Wc1SdlLoadDosMusicFile(
        timbreCandidates, 2, &timbreArchiveSize);
    if (timbreArchive == 0) {
        fprintf(stderr, "Unable to load GAMEDAT/WING2.TIM.\n");
        goto failed;
    }
    if (!Wc1SdlExtractOriginPacketSection(
            timbreArchive, timbreArchiveSize,
            WC1_SDL_ADLIB_TIMBRE_SECTION,
            &g_pWc1SdlDosAdlibTimbres,
            &g_nWc1SdlDosAdlibTimbreSize)) {
        SDL_free(timbreArchive);
        fprintf(stderr, "Unable to decode OriginFX AdLib timbres.\n");
        goto failed;
    }
    SDL_free(timbreArchive);

    if (usingDosData != 0) {
        /* The player reads the game's own record table because flight code
         * retunes entries in place. */
        g_pWc1SdlOriginFxSoundPlayer = Wc1SdlCreateOriginFxSoundPlayer(
            g_abSoundEffectDefinitions_0049bf18,
            WC2_SDL_SOUND_RECORD_COUNT,
            g_pWc1SdlDosAdlibTimbres,
            g_nWc1SdlDosAdlibTimbreSize);
        if (g_pWc1SdlOriginFxSoundPlayer == 0) {
            fprintf(stderr,
                    "Unable to initialize DOS AdLib sound effects.\n");
            goto failed;
        }
    }

    g_pWc1SdlDosMusicMutex = SDL_CreateMutex();
    if (g_pWc1SdlDosMusicMutex == 0)
        goto failed;
    g_bWc1SdlDosMusicInitialized = 1;
    Wc1SdlUpdateDosAdlibMusicVolume();
    if (usingDosData != 0) {
        g_bWc1SdlOriginFxMusicSelected = 1;
        g_bWc1SdlOriginFxSoundSelected = 1;
        fprintf(stderr, "DOS OriginFX/AdLib audio enabled.\n");
    } else if (g_bWc2SdlCutsceneOnly == 0) {
        fprintf(stderr, "Original orchestral intro music enabled.\n");
    }
    return 1;

failed:
    Wc1SdlShutdownOriginFxAudio();
    return 0;
}

int Wc1SdlUsingOriginFxMusic(void)
{
    return g_bWc1SdlOriginFxMusicSelected;
}

int Wc1SdlUsingOriginFxSoundEffects(void)
{
    return g_bWc1SdlOriginFxSoundSelected;
}

int Wc2SdlHandlesGameSoundEffects(void)
{
    return 1;
}

/* The music script normally queues a Kilrathi Saga stream here.  A DOS install
 * has no streams, so the track number becomes the MUSIC.A00 section the service
 * loop below plays instead. */
void Wc1SdlSetOriginFxMusicTrack(int track)
{
    if (g_bWc1SdlOriginFxMusicSelected == 0)
        return;
    if (g_nCurrentMusicTrack_0049be98 == track)
        return;
    g_nCurrentMusicTrack_0049be98 = track;
    g_nMusicTrackComplete_0049be88 = 0;
}

int Wc2SdlOriginalTitleMusicReady(void)
{
    if (g_bWc2SdlCutsceneOnly != 0)
        return 1;
    return g_bWc1SdlDosMusicInitialized != 0 &&
        g_pWc2SdlTitleMusicArchive != 0;
}

int Wc2SdlStartOriginalTitleMusic(void)
{
    if (!Wc2SdlOriginalTitleMusicReady())
        return 0;
    g_nTitleMusicSequenceStage_0049be94 = 0;
    g_nCurrentMusicTrack_0049be98 = 19;
    g_nMusicTrackComplete_0049be88 = 0;
    g_bWc2SdlOriginalTitleMusicActive = 1;
    Wc1SdlServiceOriginFxMusic();
    if (g_nCurrentMusicTrack_0049be98 != 19)
        g_bWc2SdlOriginalTitleMusicActive = 0;
    return g_nCurrentMusicTrack_0049be98 == 19;
}

static int Wc1SdlStartDosSoundEffect(int soundNumber, int volume, int pan,
                                     int tag, int priority)
{
    int result;

    if (g_bWc1SdlDosMusicInitialized == 0 ||
        g_pWc1SdlDosMusicMutex == 0 ||
        g_pWc1SdlOriginFxSoundPlayer == 0)
        return 0;
    SDL_LockMutex(g_pWc1SdlDosMusicMutex);
    if (soundNumber == WC2_SDL_GUN_SOUND_NUMBER) {
        tag = WC2_SDL_GUN_SOUND_FIRST_TAG +
            (int)(g_nWc1SdlDosRapidFireTag & 1U);
        g_nWc1SdlDosRapidFireTag++;
    }
    result = Wc1SdlPlayOriginFxSoundEffect(
        g_pWc1SdlOriginFxSoundPlayer,
        (unsigned int)soundNumber, volume, pan, tag, priority);
    SDL_UnlockMutex(g_pWc1SdlDosMusicMutex);
    return result;
}

int Wc1SdlPlayGameSoundEffect(int soundNumber, int sourceObject, int looping)
{
    FixedVector delta;
    long magnitude;
    int scaledPan;
    int stereoOffset;
    int distance;
    int volume;
    int pan;

    magnitude = 0;
    pan = WC2_SDL_SOUND_CENTRE_PAN;
    if (sourceObject != -1) {
        if (sourceObject < 0 || sourceObject >= WC2_SPACE_OBJECT_COUNT)
            return 0;
        ComputeVectorDelta(&g_aShipPosition_00494550[WC2_EYE_OBJECT],
                           &g_aShipPosition_00494550[sourceObject],
                           &delta);
        magnitude = Vector_magnitude(&delta);
        NormalizeFixedVector(&delta);
        stereoOffset = dot_product(
            &delta, &g_aShipRightVector_00493b78[WC2_EYE_OBJECT]);
        scaledPan = stereoOffset * WC2_SDL_SOUND_CENTRE_PAN;
        if (scaledPan < 0)
            scaledPan = -((-scaledPan + 0xff) / 0x100);
        else
            scaledPan /= 0x100;
        pan -= scaledPan;
        if (pan < 0)
            pan = 0;
        else if (pan > 127)
            pan = 127;
        g_aiSoundEffectSourceActive_005d12c0[sourceObject] = 1;
    }

    if (g_bWc1SdlOriginFxSoundSelected != 0) {
        volume = WC2_SDL_SOUND_FULL_VOLUME;
        if (sourceObject != -1) {
            volume -= (int)((magnitude /
                             WC2_SDL_SOUND_METRES_PER_VOLUME_STEP) >> 8);
        }
        if (volume < 0)
            volume = 0;
        if (volume < WC2_SDL_SOUND_AUDIBLE_VOLUME)
            return 0;
        if (!Wc1SdlStartDosSoundEffect(
                soundNumber, volume, pan, sourceObject, looping))
            return 0;
        if (sourceObject == -1) {
            g_bAfterburnerSfxActive_005d3864 =
                soundNumber == WC2_SDL_AFTERBURNER_SOUND_NUMBER;
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
    sprintf(g_szSfxWavePath_005b3650, "sfx%02i.wav", soundNumber);
    Wc2SdlPlayWaveWithPan(
        g_szSfxWavePath_005b3650, looping, distance, pan);
    return 1;
}

void Wc1SdlStopDosSoundEffects(void)
{
    g_bAfterburnerSfxActive_005d3864 = 0;
    if (g_pWc1SdlDosMusicMutex == 0 ||
        g_pWc1SdlOriginFxSoundPlayer == 0)
        return;
    SDL_LockMutex(g_pWc1SdlDosMusicMutex);
    Wc1SdlStopOriginFxSoundEffects(g_pWc1SdlOriginFxSoundPlayer);
    SDL_UnlockMutex(g_pWc1SdlDosMusicMutex);
}

void Wc1SdlMixOriginFxAudio(short *samples, unsigned int frameCount)
{
    if (g_bWc1SdlDosMusicInitialized == 0 ||
        g_pWc1SdlDosMusicMutex == 0 || samples == 0)
        return;
    SDL_LockMutex(g_pWc1SdlDosMusicMutex);
    if (g_pWc1SdlOriginFxPlayer != 0) {
        Wc1SdlMixOriginFxPlayer(
            g_pWc1SdlOriginFxPlayer, samples,
            frameCount, g_nWc1SdlDosMusicGain);
    }
    if (g_pWc1SdlOriginFxSoundPlayer != 0) {
        Wc1SdlMixOriginFxSoundEffects(
            g_pWc1SdlOriginFxSoundPlayer, samples,
            frameCount, g_nWc1SdlDosSoundGain);
    }
    SDL_UnlockMutex(g_pWc1SdlDosMusicMutex);
}

void Wc1SdlServiceOriginFxMusic(void)
{
    Wc1SdlOriginFxPlayer *player;
    const unsigned char *musicArchive;
    unsigned char *midi;
    size_t musicArchiveSize;
    size_t midiSize;
    int desiredTrack;
    int finishedTrack;

    if (g_bWc1SdlDosMusicInitialized == 0)
        return;
    SDL_LockMutex(g_pWc1SdlDosMusicMutex);
    Wc1SdlUpdateDosAdlibMusicVolume();
    desiredTrack = g_nCurrentMusicTrack_0049be98;
    if (g_bWc2SdlOriginalTitleMusicActive != 0 &&
        desiredTrack != 19)
        g_bWc2SdlOriginalTitleMusicActive = 0;
    if (g_bWc2SdlOriginalTitleMusicActive != 0 &&
        g_nWc1SdlActiveMusicTrack == 19 &&
        g_pWc1SdlOriginFxPlayer != 0) {
        g_nTitleMusicSequenceStage_0049be94 = (short)
            Wc1SdlOriginFxPlayerSequencePosition(
                g_pWc1SdlOriginFxPlayer);
    }
    if (g_bWc1SdlOriginFxMusicSelected == 0 &&
        g_bWc2SdlOriginalTitleMusicActive == 0) {
        Wc1SdlDeleteDosAdlibTrack();
        SDL_UnlockMutex(g_pWc1SdlDosMusicMutex);
        return;
    }
    if (g_pWc1SdlOriginFxPlayer != 0 &&
        Wc1SdlOriginFxPlayerFinished(g_pWc1SdlOriginFxPlayer)) {
        finishedTrack = g_nWc1SdlActiveMusicTrack;
        Wc1SdlDeleteDosAdlibTrack();
        if (finishedTrack == 19)
            g_bWc2SdlOriginalTitleMusicActive = 0;
        g_nMusicTrackComplete_0049be88 = 1;
        if (g_nCurrentMusicTrack_0049be98 == finishedTrack)
            g_nCurrentMusicTrack_0049be98 = -1;
    }
    desiredTrack = g_nCurrentMusicTrack_0049be98;
    if (desiredTrack == g_nWc1SdlActiveMusicTrack) {
        SDL_UnlockMutex(g_pWc1SdlDosMusicMutex);
        return;
    }
    if (desiredTrack < 0) {
        Wc1SdlDeleteDosAdlibTrack();
        g_nMusicTrackComplete_0049be88 = 1;
        SDL_UnlockMutex(g_pWc1SdlDosMusicMutex);
        return;
    }
    SDL_UnlockMutex(g_pWc1SdlDosMusicMutex);

    midi = 0;
    midiSize = 0;
    musicArchive = g_pWc1SdlDosMusicArchive;
    musicArchiveSize = g_nWc1SdlDosMusicArchiveSize;
    if (g_bWc2SdlOriginalTitleMusicActive != 0 &&
        desiredTrack == 19) {
        musicArchive = g_pWc2SdlTitleMusicArchive;
        musicArchiveSize = g_nWc2SdlTitleMusicArchiveSize;
    }
    /* Eighteen of the sections the script can ask for are empty, so a track
     * without a sequence is data, not damage: report it finished and let the
     * script pick the next one. */
    if (!Wc1SdlExtractOriginPacketSection(
            musicArchive, musicArchiveSize,
            (unsigned int)desiredTrack, &midi, &midiSize)) {
        SoundDebugPrintf(
            "no AdLib sequence for track %d\n", desiredTrack);
        g_nCurrentMusicTrack_0049be98 = -1;
        g_nMusicTrackComplete_0049be88 = 1;
        return;
    }
    player = Wc1SdlCreateOriginFxPlayer(
        midi, midiSize, g_pWc1SdlDosAdlibTimbres,
        g_nWc1SdlDosAdlibTimbreSize);
    free(midi);
    if (player == 0) {
        fprintf(stderr, "Unable to parse OriginFX music track %d.\n",
                desiredTrack);
        g_nCurrentMusicTrack_0049be98 = -1;
        g_nMusicTrackComplete_0049be88 = 1;
        return;
    }

    SDL_LockMutex(g_pWc1SdlDosMusicMutex);
    if (g_nCurrentMusicTrack_0049be98 != desiredTrack) {
        SDL_UnlockMutex(g_pWc1SdlDosMusicMutex);
        Wc1SdlDestroyOriginFxPlayer(player);
        return;
    }
    Wc1SdlDeleteDosAdlibTrack();
    g_pWc1SdlOriginFxPlayer = player;
    g_nWc1SdlActiveMusicTrack = desiredTrack;
    g_nMusicTrackComplete_0049be88 = 0;
    SDL_UnlockMutex(g_pWc1SdlDosMusicMutex);
}

void Wc1SdlShutdownOriginFxAudio(void)
{
    if (g_pWc1SdlDosMusicMutex != 0)
        SDL_LockMutex(g_pWc1SdlDosMusicMutex);
    Wc1SdlDeleteDosAdlibTrack();
    Wc1SdlStopOriginFxSoundEffects(g_pWc1SdlOriginFxSoundPlayer);
    Wc1SdlDestroyOriginFxPlayer(g_pWc1SdlOriginFxSoundPlayer);
    g_pWc1SdlOriginFxSoundPlayer = 0;
    if (g_pWc1SdlDosMusicMutex != 0)
        SDL_UnlockMutex(g_pWc1SdlDosMusicMutex);
    if (g_pWc1SdlDosMusicMutex != 0) {
        SDL_DestroyMutex(g_pWc1SdlDosMusicMutex);
        g_pWc1SdlDosMusicMutex = 0;
    }
    SDL_free(g_pWc1SdlDosMusicArchive);
    g_pWc1SdlDosMusicArchive = 0;
    SDL_free(g_pWc2SdlTitleMusicArchive);
    g_pWc2SdlTitleMusicArchive = 0;
    free(g_pWc1SdlDosAdlibTimbres);
    g_pWc1SdlDosAdlibTimbres = 0;
    g_nWc1SdlDosMusicArchiveSize = 0;
    g_nWc1SdlDosAdlibTimbreSize = 0;
    g_nWc2SdlTitleMusicArchiveSize = 0;
    g_nWc1SdlMusicVolumeSetting = -1;
    g_nWc1SdlSoundVolumeSetting = -1;
    g_nWc1SdlDosMusicGain = 0;
    g_nWc1SdlDosSoundGain = 0;
    g_nWc1SdlDosRapidFireTag = 0;
    g_bWc1SdlOriginFxMusicSelected = 0;
    g_bWc1SdlOriginFxSoundSelected = 0;
    g_bWc2SdlOriginalTitleMusicActive = 0;
    g_bWc1SdlDosMusicInitialized = 0;
}
