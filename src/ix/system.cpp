/*
 *  ix / D:\Rnd\prj\ix\src\sound\system.cpp
 *
 *  Retail address range: 0x00469C80 - 0x0046A49F (19 functions)
 *  Object boundaries follow retail padding and alignment; diagnostic paths
 *  identify the source subsystem (see docs/ORDER.md).
 *
 *  Implement in ADDRESS ORDER -- MSVC emits functions in source order, so the
 *  list below is the original source order of this file.
 */
#include "ix.h"
#include <memory.h>

unsigned int g_dwIxSystemBits; /* 0x005c4b1c */
IxSound *g_1_pFreeSoundHead; /* 0x005c4b20 */
IxSample *g_7_pSampleList; /* 0x005c4b24 */
int g_nActiveVoices; /* 0x005c4b28 */
int g_nSystemVoiceCount; /* 0x005c4b2c */
IxSound *g_1_pActiveSoundList; /* 0x005c4b30 */
IxSound *g_1_pWaitingSoundList; /* 0x005c4b34 */

#define IX_SYSTEM_FILE "D:\\Rnd\\prj\\ix\\src\\sound\\system.cpp"

/* Function start: 0x469C80 */   /* source line 33 */
extern "C" int ix_system_init(void)
{
    if ((g_dwIxSystemBits & 1) == 0) {
        if (ix_dsp_init() != 0) {
            ix_log_printf("Warning [%s - %d]:\n", IX_SYSTEM_FILE, 33);
            ix_log_printf("Failed to init DSP");
            return -1;
        }
        g_nActiveVoices = 0;
        g_nSystemVoiceCount = ix_dsp_get_voice_count();
        g_1_pWaitingSoundList = 0;
        g_1_pActiveSoundList = g_1_pWaitingSoundList;
        g_1_pFreeSoundHead = g_1_pActiveSoundList;
        g_7_pSampleList = 0;
        ix_system_set_master_volume(0xffff);
        g_dwIxSystemBits |= 1;
    }
    return 0;
}

/* Function start: 0x469D27 */
extern "C" void ix_system_service_sounds(void)
{
    if (g_1_pWaitingSoundList != 0) {
        IxSound *sound = g_1_pWaitingSoundList;
        unsigned int now = ix_dsp_get_tick();

        while (sound != 0) {
            IxSound *next = sound->next;

            if (sound->stopTime != (unsigned int)-1 &&
                sound->stopTime < now)
                ix_sound_stop(sound);
            sound = next;
        }
    }

    if (g_1_pActiveSoundList != 0) {
        IxSound *sound = g_1_pActiveSoundList;

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
    if ((g_dwIxSystemBits & 1) != 0) {
        ix_system_delete_all_sounds();
        ix_system_delete_all_samples();
        ix_dsp_shutdown();
        g_dwIxSystemBits &= ~1U;
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
    return g_nSystemVoiceCount;
}

/* Function start: 0x469F0C */   /* source line 137 */
extern "C" void ix_system_set_voice_count(int voiceCount)
{
    if (g_nActiveVoices != 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_SYSTEM_FILE, 137);
        ix_log_printf("Not a good ideal to change the number of voices while some are playing.");
    }
    ix_dsp_set_voice_count(voiceCount);
    g_nSystemVoiceCount = ix_dsp_get_voice_count();
}

/* Function start: 0x469F63 */
extern "C" IxSample *ix_system_new_sample(void)
{
    IxSample *sample;

    sample = (IxSample *)g_pIxMalloc(sizeof(IxSample));
    if (sample != 0)
        sample->ix_sample_construct();
    return sample;
}

/* Function start: 0x469F99 */
extern "C" void ix_system_delete_sample(IxSample *sample)
{
    if (sample != 0) {
        sample->ix_sample_destruct();
        g_pIxFree(sample);
    }
}

/* Function start: 0x469FC8 */
extern "C" void ix_system_delete_all_samples(void)
{
    IxSample *sample;
    IxSample *next;

    sample = g_7_pSampleList;
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

    sound = (IxSound *)g_pIxMalloc(sizeof(IxSound));
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
        sound->ix_sound_unlink_from_free_list();
        g_pIxFree(sound);
    }
}

#ifdef SDL_PORT
/* Port-only; see ix.h.  The three lists together hold every sound the system
 * still owns, and they are short enough that a walk costs nothing. */
extern "C" int ix_sound_is_live(const IxSound *sound)
{
    const IxSound *walk;

    if (sound == 0)
        return 0;
    for (walk = g_1_pActiveSoundList; walk != 0; walk = walk->next) {
        if (walk == sound)
            return 1;
    }
    for (walk = g_1_pWaitingSoundList; walk != 0; walk = walk->next) {
        if (walk == sound)
            return 1;
    }
    for (walk = g_1_pFreeSoundHead; walk != 0; walk = walk->next) {
        if (walk == sound)
            return 1;
    }
    return 0;
}
#endif

/* Function start: 0x46A090 */
extern "C" void ix_system_delete_all_sounds(void)
{
    IxSound *current;
    IxSound *activeNext;
    IxSound *waitingNext;
    IxSound *freeNext;

#ifdef SDL_PORT
    /* Retail deletes the playing sounds first.  Stopping one hands its voice
     * to the highest waiting sound, which moves to the head of the active
     * list behind this walk and survives it, still mixing a sample that
     * stop_all_sounds frees next.  Delete the waiting sounds first. */
    current = g_1_pWaitingSoundList;
    while (current != 0) {
        waitingNext = current->next;
        ix_system_delete_sound(current);
        current = waitingNext;
    }
#endif
    current = g_1_pActiveSoundList;
    while (current != 0) {
        activeNext = current->next;
        ix_system_delete_sound(current);
        current = activeNext;
    }
    current = g_1_pWaitingSoundList;
    while (current != 0) {
        waitingNext = current->next;
        ix_system_delete_sound(current);
        current = waitingNext;
    }
    current = g_1_pFreeSoundHead;
    while (current != 0) {
        freeNext = current->next;
        ix_system_delete_sound(current);
        current = freeNext;
    }
}

/* Function start: 0x46A139 */
int ix_system_release_voice(IxSound *sound)
{
    int voice = sound->voice;

    ix_dspv_clear_active(voice);
    sound->voice = -1;
    sound->flags &= ~IX_SOUND_HAS_VOICE;
    g_nActiveVoices--;
    return voice;
}

/* Function start: 0x46A17A */
void ix_system_assign_voice(IxSound *sound, int voice)
{
    IxSample *sample = sound->sample;
    unsigned int position;
    unsigned int elapsedTime;
    unsigned int loopLength;

    elapsedTime = ix_dsp_get_tick() - sound->startTime;
    position = ((sound->pitchOffset + sample->frequency) * elapsedTime) / 15;
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
    g_nActiveVoices++;
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

    while (voice < g_nSystemVoiceCount) {
        if ((ix_dspv_get_flags(voice) & IX_VOICE_ACTIVE) == 0)
            return voice;
        voice++;
    }
    return -1;
}
