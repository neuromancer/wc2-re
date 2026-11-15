/* SDL2-only OriginFX MIDI/AdLib replayer for DOS data. */
#include "sdl_port.h"

#include "ymfm_opl.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <new>

#define ORIGINFX_OUTPUT_RATE 22050U
#define ORIGINFX_OPL_CLOCK 3579545U
#define ORIGINFX_SERVICE_RATE 60U
#define ORIGINFX_MELODIC_VOICE_COUNT 9
#define ORIGINFX_VOICE_STATE_COUNT 11
#define ORIGINFX_CHANNEL_COUNT 26
#define ORIGINFX_SOUND_CHANNEL_COUNT 8
#define ORIGINFX_FIRST_SOUND_CHANNEL 1
#define ORIGINFX_SOUND_SLOT_COUNT 32
#define ORIGINFX_PERCUSSION_NOTE_COUNT 77
#define ORIGINFX_TIMBRE_SIZE 48U
/* WC2's StartSong (DOS 062:1781) takes up to 32 tracks. */
#define ORIGINFX_MAX_TRACKS 32
/* A 60 Hz driver tick is 0x10000 sequencer clock units (062:125c). */
#define ORIGINFX_SEQUENCER_TICK 0x10000
/* A song whose loops take no time would keep the DOS sequencer in one tick
 * forever; the port ends it instead. */
#define ORIGINFX_SEQUENCER_EVENT_LIMIT 65536U

/* STRAX.DRV 0000:01dc-020e.  Logical voices 6-10 are the five OPL2
 * rhythm voices after the driver reduces the melodic allocator to six
 * channels.  The second half of each operator table is selected in that
 * mode; snare, tom, cymbal, and hi-hat each program only one operator. */
static const unsigned char g_abOriginFxRhythmBits[5] = {
    0x10, 0x08, 0x04, 0x02, 0x01
};

static const unsigned char g_abOriginFxCarrierOffsets[20] = {
    3, 4, 5, 11, 12, 13, 19, 20, 21,
    3, 4, 5, 11, 12, 13, 19, 20, 18, 21, 17
};

static const unsigned char g_abOriginFxModulatorOffsets[20] = {
    0, 1, 2, 8, 9, 10, 16, 17, 18,
    0, 1, 2, 8, 9, 10, 16, 20, 18, 21, 17
};

/* An OriginFX sound record is flags, program + 1, note, velocity, a
 * little-endian 60 Hz duration, a glide target and an unused byte.  WC2 keeps
 * its own table of them in the image at 0x49bf18, and rewrites entries in
 * place while it runs, so the player reads the game's array rather than a
 * copy.  WC.EXE holds the same shape at 2231:051e. */
#define ORIGINFX_SOUND_RECORD_SIZE 8U

/* WC.EXE 2231:7967 and 2231:79b4 translate MIDI channel 10 into
 * OriginFX's percussion pseudo-channels and their fixed pitches. */
static const unsigned char g_abOriginFxPercussionChannels[
    ORIGINFX_PERCUSSION_NOTE_COUNT] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 10, 10, 18, 11, 0, 12, 13, 17, 13, 16, 13, 14, 13,
    13, 15, 13, 19, 0, 0, 0, 0, 21, 0, 0, 0, 26, 26, 25, 20,
    20, 0, 0, 21, 21, 22, 23, 0, 0, 24, 0, 20, 0
};

static const unsigned char g_abOriginFxPercussionPitches[
    ORIGINFX_PERCUSSION_NOTE_COUNT] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 48, 48, 48, 48, 0, 48, 42, 71, 42, 71, 47, 71, 47,
    52, 79, 52, 77, 0, 0, 0, 0, 71, 0, 0, 0, 72, 79, 79, 64,
    58, 0, 0, 89, 84, 48, 72, 0, 0, 36, 0, 96, 0
};

/* WC.EXE 19c5:1682-174a selects these timbres for channels 10-26 when
 * STRAX.DRV's AdLib output is initialized. */
static const unsigned char g_abOriginFxPercussionPrograms[17] = {
    0x80, 0x72, 0x83, 0x71, 0x86, 0x87, 0x85, 0x84, 0x81,
    0x88, 0x8d, 0x8f, 0x90, 0x91, 0x93, 0x8c, 0x8b
};

enum OriginFxEventType {
    ORIGINFX_CHANNEL_EVENT
};

typedef struct OriginFxEvent {
    unsigned char type;
    unsigned char status;
    unsigned char data1;
    unsigned char data2;
} OriginFxEvent;

typedef struct OriginFxChannel {
    unsigned short pitchBend;
    unsigned short volume;
    unsigned short modulationDepth;
    unsigned char program;
    unsigned char modulationRate;
    unsigned char pan;
} OriginFxChannel;

typedef struct OriginFxVoice {
    const unsigned char *timbre;
    uint64_t age;
    unsigned char channel;
    unsigned char note;
    unsigned char velocity;
    unsigned char frequencyHigh;
    unsigned char envelopeState;
    unsigned char modulationPhase;
    int envelopePitch;
    int modulationPitch;
    int active;
} OriginFxVoice;

typedef struct OriginFxSoundEffect {
    const unsigned char *record;
    uint64_t age;
    int tag;
    int priority;
    unsigned short remainingTicks;
    unsigned char channel;
    unsigned char currentNote;
    unsigned char volume;
    unsigned char pan;
    int active;
} OriginFxSoundEffect;

class OriginFxYmfmInterface : public ymfm::ymfm_interface {
};

struct SdlOriginFxPlayer {
    OriginFxYmfmInterface oplInterface;
    ymfm::ym3812 oplChip;
    OriginFxYmfmInterface oplRightInterface;
    ymfm::ym3812 oplRightChip;
    unsigned char *timbres;
    const unsigned char *soundRecords;
    size_t soundRecordCount;
    size_t timbreSize;
    uint64_t currentFrame;
    /* The song, sequenced as WC2's music driver does (DOS module 062): each
     * track keeps its own read position, wait and running status. */
    unsigned char *midi;
    size_t trackStart[ORIGINFX_MAX_TRACKS];
    size_t trackEnd[ORIGINFX_MAX_TRACKS];
    size_t trackCursor[ORIGINFX_MAX_TRACKS];
    int64_t trackWait[ORIGINFX_MAX_TRACKS];
    unsigned char trackStatus[ORIGINFX_MAX_TRACKS];
    unsigned char trackDone[ORIGINFX_MAX_TRACKS];
    unsigned int trackCount;
    unsigned int ticksPerQuarter;
    int64_t timePerMidiTick;
    int64_t sequenceClock;
    int leaveAtLoop;
    int musicBranch;
    uint64_t nextVoiceAge;
    uint64_t nextSoundEffectAge;
    uint64_t nativeSampleAccumulator;
    uint64_t serviceAccumulator;
    uint32_t nativeSampleRate;
    unsigned int sequencePosition;
    int32_t lastNativeSample;
    int32_t lastNativeRightSample;
    unsigned char melodicVoiceCount;
    unsigned char rhythmRegister;
    int stereoPanningEnabled;
    int finished;
    OriginFxChannel channels[ORIGINFX_CHANNEL_COUNT];
    OriginFxVoice voices[ORIGINFX_VOICE_STATE_COUNT];
    OriginFxSoundEffect soundEffects[ORIGINFX_SOUND_SLOT_COUNT];

    SdlOriginFxPlayer() :
        oplChip(oplInterface),
        oplRightChip(oplRightInterface),
        timbres(0),
        soundRecords(0),
        soundRecordCount(0),
        timbreSize(0),
        currentFrame(0),
        midi(0),
        trackCount(0),
        ticksPerQuarter(0),
        timePerMidiTick(0),
        sequenceClock(0),
        leaveAtLoop(0),
        musicBranch(0),
        nextVoiceAge(1),
        nextSoundEffectAge(1),
        nativeSampleAccumulator(0),
        serviceAccumulator(0),
        nativeSampleRate(oplChip.sample_rate(ORIGINFX_OPL_CLOCK)),
        sequencePosition(0),
        lastNativeSample(0),
        lastNativeRightSample(0),
        melodicVoiceCount(ORIGINFX_MELODIC_VOICE_COUNT),
        rhythmRegister(0),
        stereoPanningEnabled(0),
        finished(0)
    {
        memset(channels, 0, sizeof(channels));
        memset(voices, 0, sizeof(voices));
        memset(soundEffects, 0, sizeof(soundEffects));
        memset(trackStart, 0, sizeof(trackStart));
        memset(trackEnd, 0, sizeof(trackEnd));
        memset(trackCursor, 0, sizeof(trackCursor));
        memset(trackWait, 0, sizeof(trackWait));
        memset(trackStatus, 0, sizeof(trackStatus));
        memset(trackDone, 0, sizeof(trackDone));
    }

    ~SdlOriginFxPlayer()
    {
        free(midi);
        free(timbres);
    }
};

static uint16_t OriginFxReadBigEndian16(const unsigned char *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static uint32_t OriginFxReadBigEndian32(const unsigned char *bytes)
{
    return ((uint32_t)bytes[0] << 24) |
        ((uint32_t)bytes[1] << 16) |
        ((uint32_t)bytes[2] << 8) |
        bytes[3];
}

static int16_t OriginFxReadLittleEndian16(const unsigned char *bytes)
{
    return (int16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

static int OriginFxReadVariableLength(const unsigned char **cursor,
                                         const unsigned char *end,
                                         uint32_t *value)
{
    uint32_t result;
    unsigned char byte;
    unsigned int count;

    result = 0;
    count = 0;
    do {
        if (*cursor >= end || count == 4)
            return 0;
        byte = *(*cursor)++;
        result = (result << 7) | (byte & 0x7fU);
        count++;
    } while ((byte & 0x80U) != 0);
    *value = result;
    return 1;
}

/* A MIDI variable-length number at a track's cursor, kept within the track
 * where the DOS sequencer read on blindly. */
static int OriginFxReadTrackNumber(const SdlOriginFxPlayer *player,
                                   unsigned int track, size_t *cursor,
                                   uint32_t *value)
{
    const unsigned char *position;

    position = player->midi + *cursor;
    if (!OriginFxReadVariableLength(
            &position, player->midi + player->trackEnd[track], value))
        return 0;
    *cursor = (size_t)(position - player->midi);
    return 1;
}

/* MidiMetaEvent (DOS 062:0a44): microseconds per quarter note become
 * sequencer units per MIDI tick, 0x1317 / 0x4db of a microsecond each. */
static void OriginFxSetTempo(SdlOriginFxPlayer *player, uint32_t tempo)
{
    uint32_t time;

    time = tempo / player->ticksPerQuarter;
    time *= 0x1317U;
    player->timePerMidiTick = (int64_t)(time / 0x4dbU);
}

/* RewindSong (DOS 062:137c): every track plays from its start, after its
 * first delta time. */
static void OriginFxRewindSong(SdlOriginFxPlayer *player)
{
    unsigned int track;
    uint32_t delta;

    for (track = 0; track < player->trackCount; track++) {
        player->trackCursor[track] = player->trackStart[track];
        player->trackStatus[track] = 0;
        player->trackWait[track] = 0;
        player->trackDone[track] = 0;
        if (OriginFxReadTrackNumber(
                player, track, &player->trackCursor[track], &delta))
            player->trackWait[track] = delta;
        else
            player->trackDone[track] = 1;
    }
    player->sequenceClock = 0;
    player->leaveAtLoop = 0;
    player->sequencePosition = 0;
}

/* StartSong (DOS 062:1781) accepts formats 0 to 2 and up to 32 tracks, and
 * skips any chunk that is not a track. */
static int OriginFxLoadMidi(SdlOriginFxPlayer *player,
                               const unsigned char *midi,
                               size_t midiSize)
{
    size_t offset;
    uint32_t length;
    unsigned int track;
    uint16_t format;
    uint16_t trackCount;
    uint16_t division;

    if (midi == 0 || midiSize < 14 ||
        memcmp(midi, "MThd", 4) != 0 ||
        OriginFxReadBigEndian32(midi + 4) != 6)
        return 0;
    format = OriginFxReadBigEndian16(midi + 8);
    trackCount = OriginFxReadBigEndian16(midi + 10);
    division = OriginFxReadBigEndian16(midi + 12);
    if (format > 2 || trackCount == 0 ||
        trackCount > ORIGINFX_MAX_TRACKS ||
        (division & 0x8000U) != 0 || division == 0)
        return 0;
    offset = 14;
    track = 0;
    while (track < trackCount) {
        if (midiSize - offset < 8)
            return 0;
        length = OriginFxReadBigEndian32(midi + offset + 4);
        if (length > midiSize - offset - 8)
            return 0;
        if (memcmp(midi + offset, "MTrk", 4) == 0) {
            player->trackStart[track] = offset + 8;
            player->trackEnd[track] = offset + 8 + length;
            track++;
        }
        offset += 8 + (size_t)length;
    }
    player->midi = (unsigned char *)malloc(midiSize);
    if (player->midi == 0)
        return 0;
    memcpy(player->midi, midi, midiSize);
    player->trackCount = trackCount;
    player->ticksPerQuarter = division;
    OriginFxSetTempo(player, 500000);
    OriginFxRewindSong(player);
    return 1;
}

static int OriginFxLoadTimbres(SdlOriginFxPlayer *player,
                                  const unsigned char *timbres,
                                  size_t timbreSize)
{
    size_t requiredSize;
    unsigned int count;

    if (timbres == 0 || timbreSize < ORIGINFX_TIMBRE_SIZE + 1)
        return 0;
    count = timbres[0];
    requiredSize = 1 + (size_t)count * ORIGINFX_TIMBRE_SIZE;
    if (count == 0 || requiredSize > timbreSize)
        return 0;
    player->timbres = (unsigned char *)malloc(requiredSize);
    if (player->timbres == 0)
        return 0;
    memcpy(player->timbres, timbres, requiredSize);
    player->timbreSize = requiredSize;
    return 1;
}

static void OriginFxWriteRegister(SdlOriginFxPlayer *player,
                                     unsigned int address,
                                     unsigned int value)
{
    player->oplChip.write(0, (uint8_t)address);
    player->oplChip.write(1, (uint8_t)value);
    player->oplRightChip.write(0, (uint8_t)address);
    player->oplRightChip.write(1, (uint8_t)value);
}

static void OriginFxWriteStereoRegister(
    SdlOriginFxPlayer *player, unsigned int address,
    unsigned int leftValue, unsigned int rightValue)
{
    player->oplChip.write(0, (uint8_t)address);
    player->oplChip.write(1, (uint8_t)leftValue);
    player->oplRightChip.write(0, (uint8_t)address);
    player->oplRightChip.write(1, (uint8_t)rightValue);
}

static void OriginFxResetOpl(SdlOriginFxPlayer *player)
{
    unsigned int voice;

    player->oplChip.reset();
    player->oplRightChip.reset();
    player->melodicVoiceCount = ORIGINFX_MELODIC_VOICE_COUNT;
    player->rhythmRegister = 0;
    OriginFxWriteRegister(player, 0x01, 0x20);
    OriginFxWriteRegister(player, 0x08, 0);
    OriginFxWriteRegister(player, 0xbd, 0);
    voice = 0;
    while (voice < ORIGINFX_MELODIC_VOICE_COUNT) {
        OriginFxWriteRegister(player, 0xa0 + voice, 0);
        OriginFxWriteRegister(player, 0xb0 + voice, 0);
        voice++;
    }
}

static unsigned int OriginFxGetOperatorTableIndex(
    const SdlOriginFxPlayer *player, unsigned int voiceIndex)
{
    if (player->melodicVoiceCount < ORIGINFX_MELODIC_VOICE_COUNT)
        voiceIndex += ORIGINFX_MELODIC_VOICE_COUNT;
    return voiceIndex;
}

static unsigned int OriginFxGetCarrierOffset(
    const SdlOriginFxPlayer *player, unsigned int voiceIndex)
{
    return g_abOriginFxCarrierOffsets[
        OriginFxGetOperatorTableIndex(player, voiceIndex)];
}

static unsigned int OriginFxGetModulatorOffset(
    const SdlOriginFxPlayer *player, unsigned int voiceIndex)
{
    return g_abOriginFxModulatorOffsets[
        OriginFxGetOperatorTableIndex(player, voiceIndex)];
}

static unsigned int OriginFxGetOplVoice(unsigned int voiceIndex)
{
    if (voiceIndex > 8)
        return 17 - voiceIndex;
    return voiceIndex;
}

static unsigned int OriginFxGetRhythmBit(unsigned int voiceIndex)
{
    if (voiceIndex < 6 || voiceIndex >= ORIGINFX_VOICE_STATE_COUNT)
        return 0;
    return g_abOriginFxRhythmBits[voiceIndex - 6];
}

static void OriginFxEnableRhythmMode(SdlOriginFxPlayer *player)
{
    unsigned int voiceIndex;

    if (player->melodicVoiceCount < ORIGINFX_MELODIC_VOICE_COUNT)
        return;
    OriginFxWriteRegister(player, 0xa6, 0);
    OriginFxWriteRegister(player, 0xb6, 0);
    OriginFxWriteRegister(player, 0xa7, 0);
    OriginFxWriteRegister(player, 0xb7, 0x0a);
    OriginFxWriteRegister(player, 0xa8, 0x54);
    OriginFxWriteRegister(player, 0xb8, 0x09);
    player->melodicVoiceCount = 6;
    player->rhythmRegister = 0x20;
    voiceIndex = 6;
    while (voiceIndex < ORIGINFX_VOICE_STATE_COUNT) {
        memset(&player->voices[voiceIndex], 0,
               sizeof(player->voices[voiceIndex]));
        voiceIndex++;
    }
    OriginFxWriteRegister(
        player, 0xbd, player->rhythmRegister);
}

static const unsigned char *OriginFxFindTimbre(
    const SdlOriginFxPlayer *player, unsigned int program)
{
    const unsigned char *timbre;
    unsigned int count;
    unsigned int index;

    count = player->timbres[0];
    timbre = player->timbres + 1;
    index = 0;
    while (index < count) {
        if (timbre[47] == program)
            return timbre;
        timbre += ORIGINFX_TIMBRE_SIZE;
        index++;
    }
    return player->timbres + 1;
}

static const unsigned char *OriginFxNextTimbre(
    const SdlOriginFxPlayer *player, const unsigned char *timbre)
{
    size_t offset;

    if (timbre[38] == 0)
        return 0;
    offset = (size_t)(timbre - player->timbres);
    if (offset > player->timbreSize ||
        player->timbreSize - offset < ORIGINFX_TIMBRE_SIZE * 2U)
        return 0;
    return timbre + ORIGINFX_TIMBRE_SIZE;
}

static int OriginFxArithmeticShiftRight(int value,
                                           unsigned int count)
{
    uint64_t roundedMagnitude;
    unsigned int magnitude;

    if (count == 0)
        return value;
    if (count >= sizeof(value) * 8U)
        return value < 0 ? -1 : 0;
    if (value >= 0)
        return value >> count;
    magnitude = (unsigned int)(-(value + 1)) + 1U;
    roundedMagnitude = (uint64_t)magnitude +
        ((UINT64_C(1) << count) - 1U);
    return -(int)(roundedMagnitude >> count);
}

static unsigned int OriginFxClampTotalLevel(
    unsigned int registerValue, int totalLevel)
{
    if (totalLevel < 0)
        totalLevel = 0;
    else if (totalLevel > 0x3f)
        totalLevel = 0x3f;
    return (registerValue & 0xc0U) | (unsigned int)totalLevel;
}

static unsigned int OriginFxCalculateVelocityLevel(
    unsigned int registerValue, unsigned int sensitivity,
    unsigned int velocity)
{
    int attenuation;
    int totalLevel;

    if (sensitivity == 0)
        return registerValue;
    if (sensitivity > 7)
        sensitivity = 7;
    attenuation = OriginFxArithmeticShiftRight(
        63 - (int)velocity, 7 - sensitivity);
    totalLevel = (int)(registerValue & 0x3fU) + attenuation;
    return OriginFxClampTotalLevel(registerValue, totalLevel);
}

static unsigned int OriginFxCalculateCarrierLevel(
    unsigned int registerValue, unsigned int sensitivity,
    unsigned int velocity, unsigned int volume)
{
    int attenuation;
    int totalLevel;

    if (sensitivity == 0 && volume >= 0x100U)
        return registerValue;
    if (sensitivity > 7)
        sensitivity = 7;
    attenuation = OriginFxArithmeticShiftRight(
        63 - (int)velocity, 7 - sensitivity);
    totalLevel = (int)(registerValue & 0x3fU) + attenuation;
    totalLevel = 63 - OriginFxArithmeticShiftRight(
        (int)volume * (63 - totalLevel), 8);
    return OriginFxClampTotalLevel(registerValue, totalLevel);
}

static unsigned int OriginFxCalculatePannedLevel(
    unsigned int registerValue, unsigned int pan, int rightChannel)
{
    unsigned int scale;
    unsigned int divisor;
    int totalLevel;

    if (pan > 127)
        pan = 127;
    if (rightChannel != 0) {
        if (pan >= 64)
            return registerValue;
        scale = pan;
        divisor = 64;
    } else {
        if (pan <= 64)
            return registerValue;
        scale = 127 - pan;
        divisor = 63;
    }
    totalLevel = (int)(registerValue & 0x3fU);
    totalLevel = 63 - (int)(scale * (63 - totalLevel) / divisor);
    return OriginFxClampTotalLevel(registerValue, totalLevel);
}

static void OriginFxWriteVoiceLevels(SdlOriginFxPlayer *player,
                                        unsigned int voiceIndex)
{
    OriginFxVoice *voice;
    OriginFxChannel *channel;
    const unsigned char *timbre;
    unsigned int carrierOffset;
    unsigned int carrierLevel;
    unsigned int modulatorOffset;
    unsigned int modulatorLevel;

    voice = &player->voices[voiceIndex];
    channel = &player->channels[voice->channel];
    timbre = voice->timbre;
    carrierOffset = OriginFxGetCarrierOffset(player, voiceIndex);
    modulatorOffset = OriginFxGetModulatorOffset(player, voiceIndex);
    if (timbre[12] != 0 || channel->volume < 0x100U) {
        carrierLevel = OriginFxCalculateCarrierLevel(
            timbre[6], timbre[12], voice->velocity, channel->volume);
        OriginFxWriteStereoRegister(
            player, 0x40 + carrierOffset,
            OriginFxCalculatePannedLevel(
                carrierLevel, channel->pan, 0),
            OriginFxCalculatePannedLevel(
                carrierLevel, channel->pan, 1));
    }
    if (timbre[13] != 0 ||
        (player->stereoPanningEnabled != 0 &&
         (timbre[10] & 1U) != 0)) {
        if (timbre[13] != 0) {
            modulatorLevel = OriginFxCalculateVelocityLevel(
                timbre[1], timbre[13], voice->velocity);
        } else {
            modulatorLevel = timbre[1];
        }
        if ((timbre[10] & 1U) != 0) {
            OriginFxWriteStereoRegister(
                player, 0x40 + modulatorOffset,
                OriginFxCalculatePannedLevel(
                    modulatorLevel, channel->pan, 0),
                OriginFxCalculatePannedLevel(
                    modulatorLevel, channel->pan, 1));
        } else {
            OriginFxWriteRegister(
                player, 0x40 + modulatorOffset, modulatorLevel);
        }
    }
}

static void OriginFxWriteVoiceFrequency(SdlOriginFxPlayer *player,
                                           unsigned int voiceIndex,
                                           int keyOn)
{
    static const unsigned short frequencyNumbers[13] = {
        485, 514, 544, 577, 611, 647, 686,
        727, 770, 816, 864, 915, 970
    };
    OriginFxVoice *voice;
    OriginFxChannel *channel;
    int pitch;
    int note;
    int fraction;
    int tableIndex;
    int block;
    int frequency;
    int bend;
    int keyPitch;
    int keyTracking;
    unsigned int trackingShift;
    unsigned int oplVoice;
    const unsigned char *channelTimbre;

    voice = &player->voices[voiceIndex];
    channel = &player->channels[voice->channel];
    bend = (int)channel->pitchBend - 0x2000;
    keyPitch = ((int)voice->note - 60) * 256;
    keyTracking = (signed char)voice->timbre[39];
    if (keyTracking < 0) {
        keyPitch = -keyPitch;
        trackingShift = (unsigned int)(unsigned char)~keyTracking;
    } else {
        trackingShift = (unsigned int)keyTracking;
    }
    keyPitch = OriginFxArithmeticShiftRight(
        keyPitch, trackingShift) + 60 * 256;
    channelTimbre = OriginFxFindTimbre(
        player, channel->program);
    pitch = keyPitch +
        voice->envelopePitch +
        OriginFxReadLittleEndian16(voice->timbre + 36) +
        voice->modulationPitch +
        OriginFxArithmeticShiftRight(
            bend * channelTimbre[14], 8);
    note = pitch / 256;
    fraction = pitch % 256;
    if (fraction < 0) {
        fraction += 256;
        note--;
    }
    tableIndex = (note + 6) % 12;
    if (tableIndex < 0)
        tableIndex += 12;
    block = (note + 6) / 12 - 2;
    if (note + 6 < 0 && (note + 6) % 12 != 0)
        block--;
    if (block < 0)
        block = 0;
    else if (block > 7)
        block = 7;
    frequency = frequencyNumbers[tableIndex] +
        (frequencyNumbers[tableIndex + 1] -
         frequencyNumbers[tableIndex]) * fraction / 256;
    if (frequency > 0x3ff)
        frequency = 0x3ff;

    voice->frequencyHigh = (unsigned char)(
        ((frequency >> 8) & 3) | (block << 2));
    oplVoice = OriginFxGetOplVoice(voiceIndex);
    OriginFxWriteRegister(player, 0xa0 + oplVoice, frequency & 0xff);
    OriginFxWriteRegister(
        player, 0xb0 + oplVoice,
        voice->frequencyHigh | (keyOn != 0 ? 0x20 : 0));
}

static void OriginFxProgramVoice(SdlOriginFxPlayer *player,
                                    unsigned int voiceIndex)
{
    const unsigned char *timbre;
    unsigned int carrierOffset;
    unsigned int modulatorOffset;
    unsigned int oplVoice;

    timbre = player->voices[voiceIndex].timbre;
    carrierOffset = OriginFxGetCarrierOffset(player, voiceIndex);
    modulatorOffset = OriginFxGetModulatorOffset(player, voiceIndex);
    oplVoice = OriginFxGetOplVoice(voiceIndex);
    OriginFxWriteRegister(
        player, 0x20 + modulatorOffset, timbre[0]);
    OriginFxWriteRegister(
        player, 0x40 + modulatorOffset, timbre[1]);
    OriginFxWriteRegister(
        player, 0x60 + modulatorOffset, timbre[2]);
    OriginFxWriteRegister(
        player, 0x80 + modulatorOffset, timbre[3]);
    OriginFxWriteRegister(
        player, 0xe0 + modulatorOffset, timbre[4]);
    if (player->melodicVoiceCount == ORIGINFX_MELODIC_VOICE_COUNT ||
        timbre[11] < 7) {
        OriginFxWriteRegister(
            player, 0x20 + carrierOffset, timbre[5]);
        OriginFxWriteRegister(
            player, 0x40 + carrierOffset, timbre[6]);
        OriginFxWriteRegister(
            player, 0x60 + carrierOffset, timbre[7]);
        OriginFxWriteRegister(
            player, 0x80 + carrierOffset, timbre[8]);
        OriginFxWriteRegister(
            player, 0xe0 + carrierOffset, timbre[9]);
        OriginFxWriteRegister(
            player, 0xc0 + oplVoice, timbre[10]);
    }
    OriginFxWriteVoiceLevels(player, voiceIndex);
}

static unsigned int OriginFxChooseVoice(SdlOriginFxPlayer *player)
{
    unsigned int oldestFreeVoice;
    unsigned int oldestVoice;
    unsigned int voiceIndex;

    oldestFreeVoice = player->melodicVoiceCount;
    oldestVoice = 0;
    voiceIndex = 0;
    while (voiceIndex < player->melodicVoiceCount) {
        if (player->voices[voiceIndex].active == 0) {
            if (oldestFreeVoice == player->melodicVoiceCount ||
                player->voices[voiceIndex].age <
                    player->voices[oldestFreeVoice].age)
                oldestFreeVoice = voiceIndex;
        } else if (player->voices[voiceIndex].age <
                   player->voices[oldestVoice].age ||
                   player->voices[oldestVoice].active == 0) {
            oldestVoice = voiceIndex;
        }
        voiceIndex++;
    }
    if (oldestFreeVoice != player->melodicVoiceCount)
        return oldestFreeVoice;
    return oldestVoice;
}

static void OriginFxStartTimbre(SdlOriginFxPlayer *player,
                                   unsigned int channelIndex,
                                   unsigned int note,
                                   unsigned int velocity,
                                   const unsigned char *timbre)
{
    OriginFxVoice *voice;
    unsigned int voiceIndex;
    unsigned int oplVoice;
    unsigned int rhythmBit;

    if (timbre[11] != 0) {
        OriginFxEnableRhythmMode(player);
        voiceIndex = timbre[11];
        if (voiceIndex < 6 ||
            voiceIndex >= ORIGINFX_VOICE_STATE_COUNT)
            return;
        rhythmBit = OriginFxGetRhythmBit(voiceIndex);
        player->rhythmRegister = (unsigned char)(
            player->rhythmRegister & ~rhythmBit);
        OriginFxWriteRegister(
            player, 0xbd, player->rhythmRegister);
    } else {
        voiceIndex = OriginFxChooseVoice(player);
        rhythmBit = 0;
    }
    voice = &player->voices[voiceIndex];
    if (voice->active != 0 && rhythmBit == 0) {
        oplVoice = OriginFxGetOplVoice(voiceIndex);
        OriginFxWriteRegister(player, 0xa0 + oplVoice, 0);
        OriginFxWriteRegister(player, 0xb0 + oplVoice, 0);
    }
    voice->timbre = timbre;
    voice->age = player->nextVoiceAge++;
    voice->channel = (unsigned char)channelIndex;
    voice->note = (unsigned char)note;
    voice->velocity = (unsigned char)velocity;
    voice->envelopeState = 2;
    voice->modulationPhase = 0;
    voice->envelopePitch =
        OriginFxReadLittleEndian16(timbre + 18);
    voice->modulationPitch = 0;
    voice->active = 1;
    OriginFxProgramVoice(player, voiceIndex);
    if (rhythmBit == 0) {
        OriginFxWriteVoiceFrequency(player, voiceIndex, 1);
    } else {
        if (voiceIndex == 6)
            OriginFxWriteVoiceFrequency(player, voiceIndex, 0);
        player->rhythmRegister = (unsigned char)(
            player->rhythmRegister | rhythmBit);
        OriginFxWriteRegister(
            player, 0xbd, player->rhythmRegister);
    }
}

static void OriginFxNoteOn(SdlOriginFxPlayer *player,
                              unsigned int channelIndex,
                              unsigned int note,
                              unsigned int velocity)
{
    const unsigned char *timbre;

    timbre = OriginFxFindTimbre(
        player, player->channels[channelIndex].program);
    while (timbre != 0) {
        OriginFxStartTimbre(
            player, channelIndex, note, velocity, timbre);
        timbre = OriginFxNextTimbre(player, timbre);
    }
}

static void OriginFxStopTimbre(SdlOriginFxPlayer *player,
                                  unsigned int channelIndex,
                                  unsigned int note,
                                  const unsigned char *timbre)
{
    unsigned int voiceIndex;
    unsigned int rhythmBit;
    unsigned int oplVoice;

    voiceIndex = 0;
    while (voiceIndex < ORIGINFX_VOICE_STATE_COUNT) {
        if (player->voices[voiceIndex].active != 0 &&
            player->voices[voiceIndex].channel == channelIndex &&
            player->voices[voiceIndex].note == note &&
            player->voices[voiceIndex].timbre == timbre) {
            rhythmBit = OriginFxGetRhythmBit(voiceIndex);
            if (timbre[11] != 0 && rhythmBit != 0) {
                player->rhythmRegister = (unsigned char)(
                    player->rhythmRegister & ~rhythmBit);
                OriginFxWriteRegister(
                    player, 0xbd, player->rhythmRegister);
            } else {
                oplVoice = OriginFxGetOplVoice(voiceIndex);
                OriginFxWriteRegister(
                    player, 0xb0 + oplVoice,
                    player->voices[voiceIndex].frequencyHigh);
            }
            player->voices[voiceIndex].active = 0;
            player->voices[voiceIndex].envelopeState = 0;
            player->voices[voiceIndex].age = player->nextVoiceAge++;
            return;
        }
        voiceIndex++;
    }
}

static void OriginFxNoteOff(SdlOriginFxPlayer *player,
                               unsigned int channelIndex,
                               unsigned int note)
{
    const unsigned char *timbre;

    timbre = OriginFxFindTimbre(
        player, player->channels[channelIndex].program);
    while (timbre != 0) {
        OriginFxStopTimbre(
            player, channelIndex, note, timbre);
        timbre = OriginFxNextTimbre(player, timbre);
    }
}

static void OriginFxAllNotesOff(SdlOriginFxPlayer *player,
                                   int channelIndex)
{
    unsigned int voiceIndex;
    unsigned int rhythmBit;
    unsigned int oplVoice;

    voiceIndex = 0;
    while (voiceIndex < ORIGINFX_VOICE_STATE_COUNT) {
        if (player->voices[voiceIndex].active != 0 &&
            (channelIndex < 0 ||
             player->voices[voiceIndex].channel == channelIndex)) {
            rhythmBit = OriginFxGetRhythmBit(voiceIndex);
            if (player->voices[voiceIndex].timbre != 0 &&
                player->voices[voiceIndex].timbre[11] != 0 &&
                rhythmBit != 0) {
                player->rhythmRegister = (unsigned char)(
                    player->rhythmRegister & ~rhythmBit);
                OriginFxWriteRegister(
                    player, 0xbd, player->rhythmRegister);
            } else {
                oplVoice = OriginFxGetOplVoice(voiceIndex);
                OriginFxWriteRegister(
                    player, 0xb0 + oplVoice,
                    player->voices[voiceIndex].frequencyHigh);
            }
            player->voices[voiceIndex].active = 0;
            player->voices[voiceIndex].envelopeState = 0;
            player->voices[voiceIndex].age = player->nextVoiceAge++;
        }
        voiceIndex++;
    }
    if (channelIndex < 0 &&
        player->melodicVoiceCount < ORIGINFX_MELODIC_VOICE_COUNT) {
        player->rhythmRegister = 0;
        player->melodicVoiceCount = ORIGINFX_MELODIC_VOICE_COUNT;
        OriginFxWriteRegister(player, 0xbd, 0);
    }
}

static void OriginFxDisableRhythmModeIfIdle(
    SdlOriginFxPlayer *player)
{
    unsigned int voiceIndex;

    if (player->melodicVoiceCount == ORIGINFX_MELODIC_VOICE_COUNT)
        return;
    voiceIndex = player->melodicVoiceCount;
    while (voiceIndex < ORIGINFX_VOICE_STATE_COUNT) {
        if (player->voices[voiceIndex].active != 0)
            return;
        voiceIndex++;
    }
    player->rhythmRegister = 0;
    OriginFxWriteRegister(player, 0xbd, 0);
    player->melodicVoiceCount = ORIGINFX_MELODIC_VOICE_COUNT;
}

static void OriginFxUpdateChannelVoices(
    SdlOriginFxPlayer *player, unsigned int channelIndex,
    int updateFrequency, int updateLevel)
{
    unsigned int voiceIndex;

    voiceIndex = 0;
    while (voiceIndex < player->melodicVoiceCount) {
        if (player->voices[voiceIndex].active != 0 &&
            player->voices[voiceIndex].channel == channelIndex) {
            if (updateFrequency != 0)
                OriginFxWriteVoiceFrequency(player, voiceIndex, 1);
            if (updateLevel != 0)
                OriginFxWriteVoiceLevels(player, voiceIndex);
        }
        voiceIndex++;
    }
}

static void OriginFxSetProgram(SdlOriginFxPlayer *player,
                                  unsigned int channelIndex,
                                  unsigned int program)
{
    OriginFxChannel *channel;
    const unsigned char *timbre;
    unsigned int voiceIndex;

    OriginFxAllNotesOff(player, (int)channelIndex);
    voiceIndex = 0;
    while (voiceIndex < ORIGINFX_VOICE_STATE_COUNT) {
        if (player->voices[voiceIndex].timbre != 0 &&
            player->voices[voiceIndex].channel == channelIndex) {
            player->voices[voiceIndex].timbre = 0;
            player->voices[voiceIndex].channel = 0xff;
        }
        voiceIndex++;
    }
    channel = &player->channels[channelIndex];
    channel->program = (unsigned char)program;
    timbre = OriginFxFindTimbre(player, program);
    channel->modulationRate = timbre[16];
    channel->modulationDepth = timbre[17];
    if (timbre[11] != 0)
        OriginFxEnableRhythmMode(player);
}

static void OriginFxService(SdlOriginFxPlayer *player)
{
    static const unsigned char envelopeRateOffsets[5] = {
        32, 0, 20, 24, 28
    };
    static const unsigned char envelopeTargetOffsets[5] = {
        34, 0, 22, 26, 30
    };
    OriginFxVoice *voice;
    OriginFxChannel *channel;
    const unsigned char *timbre;
    unsigned int voiceIndex;
    unsigned int state;
    unsigned int rate;
    unsigned int phase;
    int target;
    int distance;
    int triangle;
    int changed;

    voiceIndex = 0;
    while (voiceIndex < player->melodicVoiceCount) {
        voice = &player->voices[voiceIndex];
        timbre = voice->timbre;
        if (timbre == 0 || voice->channel >= ORIGINFX_CHANNEL_COUNT) {
            voiceIndex++;
            continue;
        }
        changed = 0;
        state = voice->envelopeState;
        if (state < sizeof(envelopeTargetOffsets) &&
            envelopeTargetOffsets[state] != 0) {
            target = OriginFxReadLittleEndian16(
                timbre + envelopeTargetOffsets[state]);
            rate = (unsigned int)(uint16_t)
                OriginFxReadLittleEndian16(
                    timbre + envelopeRateOffsets[state]);
            distance = target - voice->envelopePitch;
            if (distance < 0)
                distance = -distance;
            if ((unsigned int)distance < rate) {
                voice->envelopePitch = target;
                voice->envelopeState++;
            } else if (voice->envelopePitch < target) {
                voice->envelopePitch += (int)rate;
            } else {
                voice->envelopePitch -= (int)rate;
            }
            changed = 1;
        }

        channel = &player->channels[voice->channel];
        if (channel->modulationRate != 0) {
            phase = (voice->modulationPhase +
                     channel->modulationRate) & 0xffU;
            voice->modulationPhase = (unsigned char)phase;
            triangle = phase < 0x80U ? (int)phase : (int)phase - 0x100;
            if (triangle > 63 || triangle < -64) {
                phase = (0x80U - phase) & 0xffU;
                triangle = phase < 0x80U
                    ? (int)phase : (int)phase - 0x100;
            }
            voice->modulationPitch = OriginFxArithmeticShiftRight(
                (int)channel->modulationDepth * triangle, 4);
            changed = 1;
        }
        if (changed != 0)
            OriginFxWriteVoiceFrequency(
                player, voiceIndex, voice->envelopeState > 1);
        voiceIndex++;
    }
}

static void OriginFxServiceSoundEffects(
    SdlOriginFxPlayer *player);
static int OriginFxRunSequencer(SdlOriginFxPlayer *player);

/* Each 60 Hz tick, as WC2's timer handler (DOS 062:125c), first plays the
 * song's due events and then updates the voices. */
static void OriginFxAdvanceService(SdlOriginFxPlayer *player)
{
    player->serviceAccumulator += ORIGINFX_SERVICE_RATE;
    while (player->serviceAccumulator >= ORIGINFX_OUTPUT_RATE) {
        player->serviceAccumulator -= ORIGINFX_OUTPUT_RATE;
        if (player->midi != 0 && player->finished == 0) {
            if (OriginFxRunSequencer(player)) {
                OriginFxAllNotesOff(player, -1);
                player->finished = 1;
            }
            player->sequenceClock -= ORIGINFX_SEQUENCER_TICK;
        }
        OriginFxServiceSoundEffects(player);
        OriginFxService(player);
    }
}

static int OriginFxMapPercussionNote(unsigned int note,
                                        unsigned int *channelIndex,
                                        unsigned int *mappedNote)
{
    unsigned int channel;

    if (note >= ORIGINFX_PERCUSSION_NOTE_COUNT)
        return 0;
    channel = g_abOriginFxPercussionChannels[note];
    if (channel == 0)
        return 0;
    *channelIndex = channel - 1;
    *mappedNote = g_abOriginFxPercussionPitches[note];
    return 1;
}

static void OriginFxApplyControlChange(
    SdlOriginFxPlayer *player, unsigned int channelIndex,
    unsigned int controller, unsigned int value)
{
    OriginFxChannel *channel;
    const unsigned char *timbre;

    channel = &player->channels[channelIndex];
    if (controller == 1) {
        timbre = OriginFxFindTimbre(player, channel->program);
        channel->modulationDepth = (unsigned short)(
            ((unsigned int)timbre[15] * value >> 7) + timbre[17]);
    } else if (controller == 7) {
        channel->volume = (unsigned short)(value + 0x80U);
    } else if (controller == 10 && player->stereoPanningEnabled != 0) {
        if (value > 127)
            value = 127;
        channel->pan = (unsigned char)value;
        OriginFxUpdateChannelVoices(
            player, channelIndex, 0, 1);
    } else if (controller == 123) {
        OriginFxAllNotesOff(player, (int)channelIndex);
        OriginFxDisableRhythmModeIfIdle(player);
    } else if (controller == 121) {
        timbre = OriginFxFindTimbre(player, channel->program);
        channel->modulationDepth = timbre[17];
        channel->volume = 0xff;
        channel->pitchBend = 0x2000;
        OriginFxUpdateChannelVoices(
            player, channelIndex, 1, 0);
    }
}

static void OriginFxControlChange(SdlOriginFxPlayer *player,
                                     unsigned int channelIndex,
                                     unsigned int controller,
                                     unsigned int value)
{
    unsigned int percussionChannel;

    if (channelIndex == 9) {
        percussionChannel = 10;
        while (percussionChannel < ORIGINFX_CHANNEL_COUNT) {
            OriginFxApplyControlChange(
                player, percussionChannel, controller, value);
            percussionChannel++;
        }
    }
    OriginFxApplyControlChange(
        player, channelIndex, controller, value);
}

static void OriginFxStopSoundEffect(
    SdlOriginFxPlayer *player, unsigned int effectIndex,
    int releaseChannel)
{
    OriginFxSoundEffect *effect;

    effect = &player->soundEffects[effectIndex];
    if (effect->active == 0)
        return;
    if (releaseChannel != 0) {
        OriginFxControlChange(
            player, effect->channel, 123, 0);
    } else {
        OriginFxNoteOff(
            player, effect->channel, effect->currentNote);
    }
    effect->active = 0;
    effect->record = 0;
}

static int OriginFxStartSoundEffectRecord(
    SdlOriginFxPlayer *player, OriginFxSoundEffect *effect)
{
    const unsigned char *record;
    unsigned int program;

    record = effect->record;
    if (record == 0 || record[1] == 0) {
        effect->active = 0;
        effect->record = 0;
        return 0;
    }
    program = record[1] - 1U;
    OriginFxSetProgram(player, effect->channel, program);
    OriginFxControlChange(
        player, effect->channel, 7, effect->volume);
    OriginFxControlChange(
        player, effect->channel, 10, effect->pan);
    effect->currentNote = record[2];
    effect->remainingTicks = (unsigned short)(
        record[4] | ((unsigned int)record[5] << 8));
    OriginFxNoteOn(
        player, effect->channel, effect->currentNote, record[3]);
    return 1;
}

static int OriginFxChooseSoundEffectChannel(
    SdlOriginFxPlayer *player, int priority)
{
    int channelUsed[ORIGINFX_SOUND_CHANNEL_COUNT];
    int victim;
    unsigned int effectIndex;
    unsigned int channel;

    memset(channelUsed, 0, sizeof(channelUsed));
    effectIndex = 0;
    while (effectIndex < ORIGINFX_SOUND_SLOT_COUNT) {
        if (player->soundEffects[effectIndex].active != 0) {
            channel = player->soundEffects[effectIndex].channel -
                ORIGINFX_FIRST_SOUND_CHANNEL;
            if (channel < ORIGINFX_SOUND_CHANNEL_COUNT)
                channelUsed[channel] = 1;
        }
        effectIndex++;
    }
    channel = 0;
    while (channel < ORIGINFX_SOUND_CHANNEL_COUNT) {
        if (channelUsed[channel] == 0)
            return (int)(channel + ORIGINFX_FIRST_SOUND_CHANNEL);
        channel++;
    }

    victim = -1;
    effectIndex = 0;
    while (effectIndex < ORIGINFX_SOUND_SLOT_COUNT) {
        if (player->soundEffects[effectIndex].active != 0 &&
            player->soundEffects[effectIndex].priority <= priority &&
            (victim < 0 ||
             player->soundEffects[effectIndex].age <
                 player->soundEffects[victim].age))
            victim = (int)effectIndex;
        effectIndex++;
    }
    if (victim < 0)
        return -1;
    channel = player->soundEffects[victim].channel;
    OriginFxStopSoundEffect(player, (unsigned int)victim, 0);
    return (int)channel;
}

static void OriginFxServiceSoundEffects(
    SdlOriginFxPlayer *player)
{
    OriginFxSoundEffect *effect;
    const unsigned char *record;
    const unsigned char *recordsBegin;
    const unsigned char *recordsEnd;
    unsigned int effectIndex;
    unsigned int flags;
    unsigned int targetNote;
    int restart;

    recordsBegin = player->soundRecords;
    recordsEnd = recordsBegin +
        player->soundRecordCount * ORIGINFX_SOUND_RECORD_SIZE;
    effectIndex = 0;
    while (effectIndex < ORIGINFX_SOUND_SLOT_COUNT) {
        effect = &player->soundEffects[effectIndex];
        record = effect->record;
        if (effect->active == 0 || record == 0) {
            effectIndex++;
            continue;
        }
        flags = record[0];
        if ((flags & 4U) != 0) {
            effectIndex++;
            continue;
        }
        effect->remainingTicks--;
        if (effect->remainingTicks != 0) {
            effectIndex++;
            continue;
        }

        OriginFxNoteOff(
            player, effect->channel, effect->currentNote);
        restart = 0;
        if ((flags & 2U) != 0) {
            targetNote = record[6];
            if (targetNote > effect->currentNote) {
                effect->currentNote++;
                restart = 1;
            } else if (targetNote < effect->currentNote) {
                effect->currentNote--;
                restart = 1;
            } else {
                effect->currentNote = record[2];
            }
        } else {
            effect->currentNote = record[2];
        }
        if ((flags & 8U) != 0)
            restart = 1;
        if (restart != 0) {
            OriginFxNoteOn(
                player, effect->channel, effect->currentNote, record[3]);
            effect->remainingTicks = (unsigned short)(
                record[4] | ((unsigned int)record[5] << 8));
        } else if ((flags & 1U) != 0 &&
                   record + ORIGINFX_SOUND_RECORD_SIZE < recordsEnd &&
                   record + ORIGINFX_SOUND_RECORD_SIZE >= recordsBegin) {
            effect->record = record + ORIGINFX_SOUND_RECORD_SIZE;
            OriginFxStartSoundEffectRecord(player, effect);
        } else {
            effect->active = 0;
            effect->record = 0;
            OriginFxControlChange(
                player, effect->channel, 123, 0);
        }
        effectIndex++;
    }
}

static void OriginFxDispatchEvent(SdlOriginFxPlayer *player,
                                     const OriginFxEvent *event)
{
    OriginFxChannel *channel;
    unsigned int channelIndex;
    unsigned int command;
    unsigned int note;

    channelIndex = event->status & 0x0fU;
    command = event->status & 0xf0U;
    channel = &player->channels[channelIndex];
    switch (command) {
    case 0x80:
        note = event->data1;
        if (channelIndex != 9 || OriginFxMapPercussionNote(
                note, &channelIndex, &note))
            OriginFxNoteOff(player, channelIndex, note);
        break;
    case 0x90:
        note = event->data1;
        if (channelIndex != 9 || OriginFxMapPercussionNote(
                note, &channelIndex, &note)) {
            if (event->data2 == 0)
                OriginFxNoteOff(player, channelIndex, note);
            else
                OriginFxNoteOn(
                    player, channelIndex, note, event->data2);
        }
        break;
    case 0xb0:
        OriginFxControlChange(
            player, channelIndex, event->data1, event->data2);
        break;
    case 0xc0:
        OriginFxSetProgram(player, channelIndex, event->data1);
        break;
    case 0xe0:
        channel->pitchBend = (unsigned short)(
            event->data1 | ((unsigned int)event->data2 << 7));
        OriginFxUpdateChannelVoices(player, channelIndex, 1, 0);
        break;
    }
}

/* NextDueTrack (DOS 062:0b05): the unfinished track whose next event is due
 * first; the sequencer clock advances by its wait. */
static int OriginFxNextDueTrack(SdlOriginFxPlayer *player)
{
    int64_t minimum;
    unsigned int track;
    int best;

    best = -1;
    minimum = 0x7fffffffLL;
    for (track = 0; track < player->trackCount; track++) {
        if (player->trackDone[track] == 0 &&
            player->trackWait[track] < minimum) {
            best = (int)track;
            minimum = player->trackWait[track];
        }
    }
    if (best < 0)
        return -1;
    if (minimum != 0)
        player->sequenceClock += minimum * player->timePerMidiTick;
    for (track = 0; track < player->trackCount; track++)
        player->trackWait[track] -= minimum;
    return best;
}

/* An Origin control event's jump: back for a loop or, with a negative
 * offset, forward to an exit. The track's channel is silenced first. */
static int OriginFxJumpTrack(SdlOriginFxPlayer *player, unsigned int track,
                             size_t data, unsigned int length, size_t *cursor)
{
    int16_t offset;
    long long target;

    if (length < 2)
        return 0;
    offset = (int16_t)(player->midi[data] |
                       ((unsigned int)player->midi[data + 1] << 8));
    target = (long long)data - offset;
    if (target < (long long)player->trackStart[track] ||
        target >= (long long)player->trackEnd[track])
        return 0;
    OriginFxAllNotesOff(player, player->trackStatus[track] & 0x0f);
    *cursor = (size_t)target;
    return 1;
}

static int OriginFxAllTracksDone(const SdlOriginFxPlayer *player)
{
    unsigned int track;

    for (track = 0; track < player->trackCount; track++) {
        if (player->trackDone[track] == 0)
            return 0;
    }
    return 1;
}

/* WC2's sequencer (DOS 062:0ba4 MidiSequencer), once per 60 Hz tick: it
 * plays the events due within the tick and follows Origin's 0xFE control
 * events. Type 0 loops back unless the song is to end, 1 ends it then, and 2
 * jumps to the song's ending only then; 3 sets the marker the title sequence
 * follows; 4 and 5 branch on the gameflow's music branch. QueueNextSong sets
 * the song to end. Returns 1 when the song has ended. */
static int OriginFxRunSequencer(SdlOriginFxPlayer *player)
{
    OriginFxEvent event;
    unsigned int guard;
    unsigned int track;
    unsigned int length;
    unsigned int index;
    unsigned char status;
    unsigned char byte;
    unsigned char type;
    uint32_t value;
    uint32_t tempo;
    size_t cursor;
    size_t data;
    size_t end;
    int next;

    for (guard = 0; guard < ORIGINFX_SEQUENCER_EVENT_LIMIT; guard++) {
        next = OriginFxNextDueTrack(player);
        if (next < 0)
            return 1;
        if (player->sequenceClock > 0xffff)
            return 0;
        track = (unsigned int)next;
        cursor = player->trackCursor[track];
        end = player->trackEnd[track];
        if (cursor >= end)
            goto trackEnded;
        byte = player->midi[cursor++];
        status = player->trackStatus[track];
        if (byte >= 0x80U && byte < 0xf0U) {
            status = byte;
            player->trackStatus[track] = byte;
            if (cursor >= end)
                goto trackEnded;
            byte = player->midi[cursor++];
        }
        if (byte < 0xf0U) {
            if (status < 0x80U)
                goto trackEnded;
            event.type = ORIGINFX_CHANNEL_EVENT;
            event.status = status;
            event.data1 = byte;
            event.data2 = 0;
            switch (status & 0xf0U) {
            case 0x80:
            case 0x90:
            case 0xa0:
            case 0xb0:
            case 0xe0:
                if (cursor >= end)
                    goto trackEnded;
                event.data2 = player->midi[cursor++];
                break;
            }
            OriginFxDispatchEvent(player, &event);
        } else if (byte == 0xf0U || byte == 0xf7U) {
            if (!OriginFxReadTrackNumber(player, track, &cursor, &value) ||
                value > end - cursor)
                goto trackEnded;
            cursor += value;
        } else if (byte == 0xfeU) {
            if (end - cursor < 2)
                goto trackEnded;
            type = player->midi[cursor];
            length = player->midi[cursor + 1];
            cursor += 2;
            if (length > end - cursor)
                goto trackEnded;
            data = cursor;
            cursor += length;
            switch (type) {
            case 0:
                if (player->leaveAtLoop == 0)
                    OriginFxJumpTrack(
                        player, track, data, length, &cursor);
                break;
            case 1:
                if (player->leaveAtLoop != 0) {
                    player->trackCursor[track] = cursor;
                    OriginFxAllNotesOff(player, -1);
                    return 1;
                }
                break;
            case 2:
                if (player->leaveAtLoop != 0)
                    OriginFxJumpTrack(
                        player, track, data, length, &cursor);
                break;
            case 3:
                if (length != 0)
                    player->sequencePosition = player->midi[data];
                break;
            case 4:
                if (length >= 3 &&
                    player->midi[data + 2] != player->musicBranch)
                    OriginFxJumpTrack(
                        player, track, data, length, &cursor);
                break;
            case 5:
                if (length >= 3 &&
                    player->midi[data + 2] == player->musicBranch)
                    OriginFxJumpTrack(
                        player, track, data, length, &cursor);
                break;
            }
        } else if (byte == 0xffU) {
            if (cursor >= end)
                goto trackEnded;
            type = player->midi[cursor++];
            if (!OriginFxReadTrackNumber(player, track, &cursor, &value) ||
                value > end - cursor)
                goto trackEnded;
            if (type == 0x51U && value != 0) {
                tempo = 0;
                for (index = 0; index < value; index++)
                    tempo = (tempo << 8) + player->midi[cursor + index];
                OriginFxSetTempo(player, tempo);
            }
            cursor += value;
            if (type == 0x2fU)
                goto trackEnded;
        }
        player->trackCursor[track] = cursor;
        if (OriginFxReadTrackNumber(player, track, &cursor, &value)) {
            player->trackCursor[track] = cursor;
            player->trackWait[track] += value;
            continue;
        }
trackEnded:
        player->trackCursor[track] = cursor;
        player->trackDone[track] = 1;
        if (OriginFxAllTracksDone(player))
            return 1;
    }
    return 1;
}

static void OriginFxGenerateOutputSample(
    SdlOriginFxPlayer *player, int32_t *leftSample,
    int32_t *rightSample)
{
    ymfm::ym3812::output_data leftOutput;
    ymfm::ym3812::output_data rightOutput;
    int64_t leftSampleTotal;
    int64_t rightSampleTotal;
    unsigned int sampleCount;

    leftSampleTotal = 0;
    rightSampleTotal = 0;
    sampleCount = 0;
    player->nativeSampleAccumulator += player->nativeSampleRate;
    while (player->nativeSampleAccumulator >= ORIGINFX_OUTPUT_RATE) {
        player->oplChip.generate(&leftOutput);
        player->lastNativeSample = leftOutput.data[0];
        if (player->stereoPanningEnabled != 0) {
            player->oplRightChip.generate(&rightOutput);
            player->lastNativeRightSample = rightOutput.data[0];
        } else {
            player->lastNativeRightSample = player->lastNativeSample;
        }
        leftSampleTotal += player->lastNativeSample;
        rightSampleTotal += player->lastNativeRightSample;
        sampleCount++;
        player->nativeSampleAccumulator -= ORIGINFX_OUTPUT_RATE;
    }
    if (sampleCount == 0) {
        *leftSample = player->lastNativeSample;
        *rightSample = player->lastNativeRightSample;
    } else {
        *leftSample = (int32_t)(leftSampleTotal / sampleCount);
        *rightSample = (int32_t)(rightSampleTotal / sampleCount);
    }
}

static short OriginFxScaleOutputSample(int32_t sample,
                                         unsigned int gain)
{
    int64_t scaled;

    if (gain > 0x7fffU)
        gain = 0x7fffU;
    scaled = (int64_t)sample * gain / 0x7fff;
    if (scaled < -32768)
        scaled = -32768;
    else if (scaled > 32767)
        scaled = 32767;
    return (short)scaled;
}

static short OriginFxMixOutputSample(short destination, short source)
{
    int mixed;

    mixed = (int)destination + source;
    if (mixed < -32768)
        mixed = -32768;
    else if (mixed > 32767)
        mixed = 32767;
    return (short)mixed;
}

static int OriginFxInitializePlayer(
    SdlOriginFxPlayer *player,
    const unsigned char *timbres, size_t timbreSize)
{
    unsigned int channelIndex;
    unsigned int percussionIndex;
    unsigned char defaultProgram;

    if (!OriginFxLoadTimbres(player, timbres, timbreSize))
        return 0;
    OriginFxResetOpl(player);
    defaultProgram = player->timbres[1 + 47];
    channelIndex = 0;
    while (channelIndex < ORIGINFX_CHANNEL_COUNT) {
        player->channels[channelIndex].pitchBend = 0x2000;
        player->channels[channelIndex].volume = 0xff;
        player->channels[channelIndex].pan = 64;
        OriginFxSetProgram(player, channelIndex, defaultProgram);
        channelIndex++;
    }
    percussionIndex = 0;
    while (percussionIndex <
           sizeof(g_abOriginFxPercussionPrograms) /
               sizeof(g_abOriginFxPercussionPrograms[0])) {
        OriginFxSetProgram(
            player, 9 + percussionIndex,
            g_abOriginFxPercussionPrograms[percussionIndex]);
        percussionIndex++;
    }
    return 1;
}

SdlOriginFxPlayer *SdlCreateOriginFxPlayer(
    const unsigned char *midi, size_t midiSize,
    const unsigned char *timbres, size_t timbreSize)
{
    SdlOriginFxPlayer *player;

    player = new (std::nothrow) SdlOriginFxPlayer;
    if (player == 0)
        return 0;
    if (!OriginFxInitializePlayer(
            player, timbres, timbreSize) ||
        !OriginFxLoadMidi(player, midi, midiSize)) {
        delete player;
        return 0;
    }
    return player;
}

SdlOriginFxPlayer *SdlCreateOriginFxSoundPlayer(
    const unsigned char *records, unsigned int recordCount,
    const unsigned char *timbres, size_t timbreSize)
{
    SdlOriginFxPlayer *player;

    if (records == 0 || recordCount == 0)
        return 0;
    player = new (std::nothrow) SdlOriginFxPlayer;
    if (player == 0)
        return 0;
    if (!OriginFxInitializePlayer(player, timbres, timbreSize)) {
        delete player;
        return 0;
    }
    player->stereoPanningEnabled = 1;
    player->soundRecords = records;
    player->soundRecordCount = recordCount;
    return player;
}

int SdlPlayOriginFxSoundEffect(
    SdlOriginFxPlayer *player, unsigned int soundNumber,
    int volume, int pan, int tag, int priority)
{
    OriginFxSoundEffect *effect;
    uint64_t channelAge;
    unsigned int effectIndex;
    unsigned int freeEffectIndex;
    int channel;
    int channelPriority;

    if (player == 0 || soundNumber == 0 ||
        soundNumber > player->soundRecordCount)
        return 0;
    if (volume < 0)
        volume = 0;
    else if (volume > 127)
        volume = 127;
    if (pan < 0)
        pan = 0;
    else if (pan > 127)
        pan = 127;

    channel = -1;
    channelAge = 0;
    channelPriority = priority;
    freeEffectIndex = ORIGINFX_SOUND_SLOT_COUNT;
    effectIndex = 0;
    while (effectIndex < ORIGINFX_SOUND_SLOT_COUNT) {
        effect = &player->soundEffects[effectIndex];
        if (effect->active != 0 && effect->tag == tag) {
            channel = effect->channel;
            channelAge = effect->age;
            channelPriority = effect->priority;
            OriginFxStopSoundEffect(player, effectIndex, 0);
            freeEffectIndex = effectIndex;
            break;
        }
        if (effect->active == 0)
            freeEffectIndex = effectIndex;
        effectIndex++;
    }
    if (channel < 0)
        channel = OriginFxChooseSoundEffectChannel(player, priority);
    if (channel < 0)
        return 0;

    if (freeEffectIndex == ORIGINFX_SOUND_SLOT_COUNT)
        return 0;
    effect = &player->soundEffects[freeEffectIndex];
    memset(effect, 0, sizeof(*effect));
    effect->record = player->soundRecords +
        (size_t)(soundNumber - 1) * ORIGINFX_SOUND_RECORD_SIZE;
    if (channelAge != 0) {
        effect->age = channelAge;
    } else {
        effect->age = player->nextSoundEffectAge++;
    }
    effect->tag = tag;
    effect->priority = channelPriority;
    effect->channel = (unsigned char)channel;
    effect->volume = (unsigned char)volume;
    effect->pan = (unsigned char)pan;
    effect->active = 1;
    return OriginFxStartSoundEffectRecord(player, effect);
}

void SdlStopOriginFxSoundEffects(SdlOriginFxPlayer *player)
{
    unsigned int effectIndex;

    if (player == 0)
        return;
    effectIndex = 0;
    while (effectIndex < ORIGINFX_SOUND_SLOT_COUNT) {
        OriginFxStopSoundEffect(player, effectIndex, 1);
        effectIndex++;
    }
    OriginFxAllNotesOff(player, -1);
}

void SdlMixOriginFxSoundEffects(
    SdlOriginFxPlayer *player, short *samples,
    unsigned int frameCount, unsigned int gain)
{
    int32_t generatedLeft;
    int32_t generatedRight;
    short leftOutput;
    short rightOutput;
    unsigned int frame;

    if (player == 0 || samples == 0)
        return;
    frame = 0;
    while (frame < frameCount) {
        OriginFxAdvanceService(player);
        OriginFxGenerateOutputSample(
            player, &generatedLeft, &generatedRight);
        leftOutput = OriginFxScaleOutputSample(generatedLeft, gain);
        rightOutput = OriginFxScaleOutputSample(generatedRight, gain);
        samples[frame * 2] = OriginFxMixOutputSample(
            samples[frame * 2], leftOutput);
        samples[frame * 2 + 1] = OriginFxMixOutputSample(
            samples[frame * 2 + 1], rightOutput);
        player->currentFrame++;
        frame++;
    }
}

void SdlDestroyOriginFxPlayer(SdlOriginFxPlayer *player)
{
    delete player;
}

int SdlOriginFxPlayerFinished(const SdlOriginFxPlayer *player)
{
    return player == 0 || player->finished != 0;
}

unsigned int SdlOriginFxPlayerSequencePosition(
    const SdlOriginFxPlayer *player)
{
    if (player == 0)
        return 0;
    return player->sequencePosition;
}

/* QueueNextSong (DOS 062:1a19): the song no longer loops and leaves at its
 * next exit, so the queued one follows on the beat. */
void SdlLeaveOriginFxSongAtLoop(SdlOriginFxPlayer *player)
{
    if (player != 0)
        player->leaveAtLoop = 1;
}

void SdlSetOriginFxMusicBranch(SdlOriginFxPlayer *player, int branch)
{
    if (player != 0)
        player->musicBranch = branch;
}

void SdlRenderOriginFxPlayer(SdlOriginFxPlayer *player,
                                short *samples,
                                unsigned int frameCount,
                                unsigned int gain)
{
    if (samples == 0)
        return;
    memset(samples, 0, (size_t)frameCount * sizeof(short) * 2);
    SdlMixOriginFxPlayer(player, samples, frameCount, gain);
}

void SdlMixOriginFxPlayer(SdlOriginFxPlayer *player,
                             short *samples,
                             unsigned int frameCount,
                             unsigned int gain)
{
    int32_t generatedLeft;
    int32_t generatedRight;
    short leftOutput;
    short rightOutput;
    unsigned int frame;

    if (samples == 0 || player == 0 || player->finished != 0)
        return;

    frame = 0;
    while (frame < frameCount) {
        OriginFxAdvanceService(player);
        if (player->finished != 0)
            return;
        OriginFxGenerateOutputSample(
            player, &generatedLeft, &generatedRight);
        leftOutput = OriginFxScaleOutputSample(generatedLeft, gain);
        rightOutput = OriginFxScaleOutputSample(generatedRight, gain);
        samples[frame * 2] = OriginFxMixOutputSample(
            samples[frame * 2], leftOutput);
        samples[frame * 2 + 1] = OriginFxMixOutputSample(
            samples[frame * 2 + 1], rightOutput);
        player->currentFrame++;
        frame++;
    }
}
