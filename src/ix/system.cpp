/*
 *  ix / D:\Rnd\prj\ix\src\sound\system.cpp
 *
 *  Original address range: 0x00447200 - 0x00447CD7   (27 functions)
 *  Range is exact: recovered from this module's own assert __FILE__ anchors in
 *  the shipped debug build (see docs/ORDER.md).
 *
 *  Implement in ADDRESS ORDER -- MSVC emits functions in source order, so the
 *  list below is the original source order of this file.
 */
#include "ix.h"

unsigned int g_dwIxSystemFlags_005c4b1c;
IxSound *g_pFreeSoundList_005c4b20;
IxSample *g_pSampleList_005c4b24;
int g_nActiveVoices_005c4b28;
int g_nSystemVoiceCount_005c4b2c;
IxSound *g_pActiveSoundList_005c4b30;
IxSound *g_pWaitingSoundList_005c4b34;

#define IX_SYSTEM_FILE "D:\\Rnd\\prj\\ix\\src\\sound\\system.cpp"

/* Function start: 0x469C80 */   /* source line 33 */
extern "C" int ix_system_init(void)
{
    if ((g_dwIxSystemFlags_005c4b1c & 1) == 0) {
        if (ix_dsp_init() != 0) {
            ix_log_printf("Warning [%s - %d]:\n", IX_SYSTEM_FILE, 33);
            ix_log_printf("Failed to init DSP");
            return -1;
        }
        g_nActiveVoices_005c4b28 = 0;
        g_nSystemVoiceCount_005c4b2c = ix_dsp_get_voice_count();
        g_pWaitingSoundList_005c4b34 = 0;
        g_pActiveSoundList_005c4b30 = g_pWaitingSoundList_005c4b34;
        g_pFreeSoundList_005c4b20 = g_pActiveSoundList_005c4b30;
        g_pSampleList_005c4b24 = 0;
        ix_system_set_master_volume(0xffff);
        g_dwIxSystemFlags_005c4b1c |= 1;
    }
    return 0;
}

/* Function start: 0x469D27 */
extern "C" void ix_system_service_sounds(void)
{
    if (g_pWaitingSoundList_005c4b34 != 0) {
        IxSound *sound = g_pWaitingSoundList_005c4b34;
        unsigned int now = ix_dsp_get_tick();

        while (sound != 0) {
            IxSound *next = sound->next;

            if (sound->stopTime != (unsigned int)-1 &&
                sound->stopTime < now)
                ix_sound_stop(sound);
            sound = next;
        }
    }

    if (g_pActiveSoundList_005c4b30 != 0) {
        IxSound *sound = g_pActiveSoundList_005c4b30;

        while (sound != 0) {
            IxSound *next = sound->next;

            if ((sound->flags & (IX_SOUND_VOLUME_DIRTY |
                                 IX_SOUND_FREQUENCY_DIRTY |
                                 IX_SOUND_PAN_DIRTY)) != 0) {
                unsigned int flags = sound->flags;
                int voice = sound->voice;

                sound->flags &= ~(IX_SOUND_VOLUME_DIRTY |
                                  IX_SOUND_FREQUENCY_DIRTY |
                                  IX_SOUND_PAN_DIRTY);
                if ((flags & IX_SOUND_VOLUME_DIRTY) != 0)
                    ix_dspv_set_volume(voice, sound->volume);
                if ((flags & IX_SOUND_PAN_DIRTY) != 0)
                    ix_dspv_set_pan(voice, sound->pan);
                if ((flags & IX_SOUND_FREQUENCY_DIRTY) != 0)
                    ix_dspv_set_frequency(
                        voice, sound->sample->frequency + sound->pitchOffset);
            }
            if ((ix_dspv_get_flags(sound->voice) & IX_VOICE_ACTIVE) == 0)
                ix_sound_stop(sound);
            sound = next;
        }
    }
}

/* Function start: 0x469E73 */
extern "C" void ix_system_shutdown(void)
{
    if ((g_dwIxSystemFlags_005c4b1c & 1) != 0) {
        ix_system_delete_all_sounds();
        ix_system_delete_all_samples();
        ix_dsp_shutdown();
        g_dwIxSystemFlags_005c4b1c &= ~1U;
    }
}

/* Function start: 0x469EA6 */
extern "C" void ix_system_configure(int option, void *value)
{
    ix_dsp_configure(option, value);
}

/* Function start: 0x469EC6 */
extern "C" unsigned short ix_system_get_master_volume(void)
{
    return ix_dsp_get_master_volume();
}

/* Function start: 0x469EDB */
extern "C" void ix_system_set_master_volume(unsigned short volume)
{
    ix_dsp_set_master_volume(volume);
}

/* Function start: 0x469EF7 */
extern "C" int ix_system_get_voice_count(void)
{
    return g_nSystemVoiceCount_005c4b2c;
}

/* Function start: 0x469F0C */   /* source line 137 */
extern "C" void ix_system_set_voice_count(int voiceCount)
{
    if (g_nActiveVoices_005c4b28 != 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_SYSTEM_FILE, 137);
        ix_log_printf("Not a good ideal to change the number of voices while some are playing.");
    }
    ix_dsp_set_voice_count(voiceCount);
    g_nSystemVoiceCount_005c4b2c = ix_dsp_get_voice_count();
}

/* Function start: 0x469F63 */
extern "C" IxSample *ix_system_new_sample(void)
{
    IxSample *sample;

    sample = (IxSample *)g_pIxMalloc_004a0c18(sizeof(IxSample));
    if (sample != 0)
        sample->ix_sample_construct();
    return sample;
}

/* Function start: 0x469F99 */
extern "C" void ix_system_delete_sample(IxSample *sample)
{
    if (sample != 0) {
        sample->ix_sample_destruct();
        g_pIxFree_004a0c1c(sample);
    }
}

/* Function start: 0x469FC8 */
extern "C" void ix_system_delete_all_samples(void)
{
    IxSample *sample;
    IxSample *next;

    sample = g_pSampleList_005c4b24;
    while (sample != 0) {
        next = sample->next;
        ix_system_delete_sample(sample);
        sample = next;
    }
}

/* Function start: 0x46A00D */
extern "C" IxSound *ix_system_new_sound(IxSample *sample)
{
    IxSound *sound;

    sound = (IxSound *)g_pIxMalloc_004a0c18(sizeof(IxSound));
    if (sound != 0)
        sound->ix_system_sound_construct_centred(sample);
    return sound;
}

/* Function start: 0x46A047 */
extern "C" void ix_system_delete_sound(IxSound *sound)
{
    if (sound != 0) {
        if ((sound->flags & IX_SOUND_PLAYING) != 0) {
            sound->flags &= ~IX_SOUND_DELETE_ON_STOP;
            ix_sound_stop(sound);
        }
        ix_sound_unlink_from_free_list(sound);
        g_pIxFree_004a0c1c(sound);
    }
}

#ifdef WC1_SDL
/* Port-only; see ix.h.  The three lists together hold every sound the system
 * still owns, and they are short enough that a walk costs nothing. */
extern "C" int ix_sound_is_live(const IxSound *sound)
{
    const IxSound *walk;

    if (sound == 0)
        return 0;
    for (walk = g_pActiveSoundList_005c4b30; walk != 0; walk = walk->next) {
        if (walk == sound)
            return 1;
    }
    for (walk = g_pWaitingSoundList_005c4b34; walk != 0; walk = walk->next) {
        if (walk == sound)
            return 1;
    }
    for (walk = g_pFreeSoundList_005c4b20; walk != 0; walk = walk->next) {
        if (walk == sound)
            return 1;
    }
    return 0;
}
#endif

/* Function start: 0x46A090 */
extern "C" void ix_system_delete_all_sounds(void)
{
    IxSound *sound;
    IxSound *nextActive;
    IxSound *nextWaiting;
    IxSound *nextFree;

    sound = g_pActiveSoundList_005c4b30;
    while (sound != 0) {
        nextActive = sound->next;
        ix_system_delete_sound(sound);
        sound = nextActive;
    }
    sound = g_pWaitingSoundList_005c4b34;
    while (sound != 0) {
        nextWaiting = sound->next;
        ix_system_delete_sound(sound);
        sound = nextWaiting;
    }
    sound = g_pFreeSoundList_005c4b20;
    while (sound != 0) {
        nextFree = sound->next;
        ix_system_delete_sound(sound);
        sound = nextFree;
    }
}

/* Function start: 0x46A139 */
int ix_system_release_voice(IxSound *sound)
{
    int voice = sound->voice;

    ix_dspv_clear_active(voice);
    sound->voice = -1;
    sound->flags &= ~IX_SOUND_HAS_VOICE;
    g_nActiveVoices_005c4b28--;
    return voice;
}

/* Function start: 0x46A17A */
void ix_system_assign_voice(IxSound *sound, int voice)
{
    IxSample *sample = sound->sample;
    unsigned int elapsed;
    unsigned int position;
    unsigned int loopLength;

    elapsed = ix_dsp_get_tick() - sound->startTime;
    position = ((sound->pitchOffset + sample->frequency) * elapsed) / 15;
    if ((sound->flags & IX_SOUND_LOOPING) != 0) {
        loopLength = sample->loopEnd - sample->loopStart;
        if (sample->loopEnd < position)
            position = sample->loopStart +
                       (position - sample->loopStart) % loopLength;
        if (sample->rateNumerator == sample->rateDenominator)
            position = sample->bytesPerFrame * position;
        else
            position = (sample->bytesPerFrame * sample->rateNumerator *
                        position) / sample->rateDenominator;
        ix_dspv_set_buffer(
            voice, sample->buffer + sample->loopStart * sample->bytesPerFrame,
            sample->bytesPerFrame * loopLength);
        ix_dspv_set_position(voice, position);
        ix_dspv_set_flag4(voice, 1);
    } else {
        if (sample->rateNumerator == sample->rateDenominator)
            position = sample->bytesPerFrame * position;
        else
            position = (sample->bytesPerFrame * sample->rateNumerator *
                        position) / sample->rateDenominator;
        ix_dspv_set_buffer(voice, sample->buffer,
                           sample->bytesPerFrame * sample->sampleCount);
        ix_dspv_set_position(voice, position);
        ix_dspv_set_flag4(voice, 0);
    }
    ix_dspv_set_channels(voice, sample->channels);
    ix_dspv_set_bits_per_sample(voice, sample->bitsPerSample);
    ix_dspv_set_frequency(voice, sample->frequency + sound->pitchOffset);
    ix_dspv_set_pan(voice, sound->pan);
    ix_dspv_set_volume(voice, sound->volume);
    ix_dspv_set_active(voice);
    sound->voice = voice;
    sound->flags |= IX_SOUND_HAS_VOICE;
    sound->flags &= ~(IX_SOUND_VOLUME_DIRTY |
                      IX_SOUND_FREQUENCY_DIRTY |
                      IX_SOUND_PAN_DIRTY);
    g_nActiveVoices_005c4b28++;
}

/* Function start: 0x46A3A1 */
IxSound *ix_system_find_highest_waiting(IxSound *sound,
                                        unsigned int minimumPriority)
{
    IxSound *best = 0;

    while (sound != 0) {
        if (sound->priority > minimumPriority) {
            minimumPriority = sound->priority;
            best = sound;
        }
        sound = sound->next;
    }
    return best;
}

/* Function start: 0x46A3F4 */
IxSound *ix_system_find_lowest_playing(IxSound *sound,
                                       unsigned int maximumPriority)
{
    IxSound *best = 0;

    while (sound != 0) {
        if (sound->priority < maximumPriority) {
            maximumPriority = sound->priority;
            best = sound;
        }
        sound = sound->next;
    }
    return best;
}

/* Function start: 0x46A447 */
int ix_system_find_free_voice(void)
{
    int voice = 0;

    while (voice < g_nSystemVoiceCount_005c4b2c) {
        if ((ix_dspv_get_flags(voice) & IX_VOICE_ACTIVE) == 0)
            return voice;
        voice++;
    }
    return -1;
}

/* Function start: 0x46A4A0 */
void IxSound::ix_system_sound_init(IxSample *newSample,
                                   unsigned char newBasePriority,
                                   unsigned int initialVolume,
                                   int newPitchOffset,
                                   unsigned short newPan)
{
    flags = IX_SOUND_ALLOCATED;
    voice = -1;
    sample = newSample;
    pitchOffset = newPitchOffset;
    volume = 0xffff;
    pan = newPan;
    basePriority = newBasePriority;
    previous = 0;
    next = previous;
    stopTime = (unsigned int)-1;
    startTime = stopTime;
    priority = ((int)volume << 8) / 0xffff +
               (pitchOffset << 8) / 0xac44 + basePriority;
    next = g_pFreeSoundList_005c4b20;
    previous = 0;
    if (g_pFreeSoundList_005c4b20 != 0)
        g_pFreeSoundList_005c4b20->previous = this;
    g_pFreeSoundList_005c4b20 = this;
}

/* Function start: 0x46A597 */
void IxSound::ix_system_sound_construct(IxSample *newSample,
                                        unsigned char newBasePriority)
{
    ix_system_sound_init(newSample, newBasePriority, 0xffff, 0, 0);
}

/* Function start: 0x46A5C8 */
void IxSound::ix_system_sound_construct_centred(IxSample *newSample)
{
    ix_system_sound_init(newSample, 0x80, 0xffff, 0, 0);
}

/* Function start: 0x46A5FA */
void __fastcall ix_sound_unlink_from_free_list(IxSound *sound)
{
    if (sound == g_pFreeSoundList_005c4b20)
        g_pFreeSoundList_005c4b20 = sound->next;
    if (sound->next != 0)
        sound->next->previous = sound->previous;
    if (sound->previous != 0)
        sound->previous->next = sound->next;
    sound->flags &= ~IX_SOUND_ALLOCATED;
}

/* Function start: 0x46A667 */
void IxSound::ix_system_sound_set_volume(unsigned short newVolume)
{
    flags |= IX_SOUND_VOLUME_DIRTY;
    volume = newVolume;
    ix_sound_reprioritise();
}

/* Function start: 0x46A698 */
void IxSound::ix_system_sound_set_pan(unsigned short newPan)
{
    flags |= IX_SOUND_PAN_DIRTY;
    pan = newPan;
}

/* Function start: 0x46A6C4 */
void IxSound::ix_system_sound_set_frequency(int newPitchOffset)
{
    flags |= IX_SOUND_FREQUENCY_DIRTY;
    if (-(int)(sample->frequency - 1) <= newPitchOffset) {
        pitchOffset = newPitchOffset < 0xac44 ? newPitchOffset : 0xac44;
    } else {
        pitchOffset = -(int)(sample->frequency - 1);
    }
    ix_sound_reprioritise();
}

/* Function start: 0x46A72F */
void IxSound::ix_system_sound_set_priority(unsigned char newBasePriority)
{
    basePriority = newBasePriority;
    ix_sound_reprioritise();
}
