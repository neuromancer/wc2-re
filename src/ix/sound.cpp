/*
 *  ix / D:\Rnd\prj\ix\src\sound\sound.cpp
 *
 *  Retail address range: 0x0046A4A0 - 0x0046B12F (14 functions)
 *  Object boundaries follow retail padding and alignment; diagnostic paths
 *  identify the source subsystem (see docs/ORDER.md).
 *
 *  Implement in ADDRESS ORDER -- MSVC emits functions in source order, so the
 *  list below is the original source order of this file.
 */
#define IX_SOUND_CPP
#include "ix.h"

#define IX_SOUND_FILE "D:\\Rnd\\prj\\ix\\src\\sound\\sound.cpp"

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
    next = g_1_pFreeSoundHead;
    previous = 0;
    if (g_1_pFreeSoundHead != 0)
        g_1_pFreeSoundHead->previous = this;
    g_1_pFreeSoundHead = this;
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
void IxSound::ix_sound_unlink_from_free_list(void)
{
    if (this == g_1_pFreeSoundHead)
        g_1_pFreeSoundHead = next;
    if (next != 0)
        next->previous = previous;
    if (previous != 0)
        previous->next = next;
    flags &= ~IX_SOUND_ALLOCATED;
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

/* Function start: 0x46A758 */   /* source line 103 */
extern "C" void __fastcall ix_sound_start(IxSound *sound)
{
    int voice;
    IxSound *lowestPlaying;
    int releasedVoice;

    if ((sound->flags & IX_SOUND_PLAYING) != 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_SOUND_FILE, 103);
        ix_log_printf("sound already started!");
        return;
    }

    sound->flags |= IX_SOUND_PLAYING;
    sound->startTime = ix_dsp_get_tick();
    if ((sound->sample->flags & IX_SAMPLE_LOOPING) != 0) {
        sound->flags |= IX_SOUND_LOOPING;
        sound->flags &= ~IX_SOUND_RELEASE_PENDING;
        sound->stopTime = (unsigned int)-1;
    } else {
        sound->stopTime = sound->startTime +
            (sound->sample->sampleCount * 15) /
            (sound->sample->frequency + sound->pitchOffset);
    }

    if (g_nActiveVoices < g_nSystemVoiceCount) {
        voice = ix_system_find_free_voice();

        if (sound == g_1_pFreeSoundHead)
            g_1_pFreeSoundHead = sound->next;
        if (sound->next != 0)
            sound->next->previous = sound->previous;
        if (sound->previous != 0)
            sound->previous->next = sound->next;
        sound->next = g_1_pActiveSoundList;
        sound->previous = 0;
        if (g_1_pActiveSoundList != 0)
            g_1_pActiveSoundList->previous = sound;
        g_1_pActiveSoundList = sound;
        ix_system_assign_voice(sound, voice);
    } else {
        lowestPlaying = ix_system_find_lowest_playing(
            g_1_pActiveSoundList, sound->priority);
        if (lowestPlaying != 0) {
            releasedVoice = ix_system_release_voice(lowestPlaying);

            if (lowestPlaying == g_1_pActiveSoundList)
                g_1_pActiveSoundList = lowestPlaying->next;
            if (lowestPlaying->next != 0)
                lowestPlaying->next->previous = lowestPlaying->previous;
            if (lowestPlaying->previous != 0)
                lowestPlaying->previous->next = lowestPlaying->next;
            lowestPlaying->next = g_1_pWaitingSoundList;
            lowestPlaying->previous = 0;
            if (g_1_pWaitingSoundList != 0)
                g_1_pWaitingSoundList->previous = lowestPlaying;
            g_1_pWaitingSoundList = lowestPlaying;

            if (sound == g_1_pFreeSoundHead)
                g_1_pFreeSoundHead = sound->next;
            if (sound->next != 0)
                sound->next->previous = sound->previous;
            if (sound->previous != 0)
                sound->previous->next = sound->next;
            sound->next = g_1_pActiveSoundList;
            sound->previous = 0;
            if (g_1_pActiveSoundList != 0)
                g_1_pActiveSoundList->previous = sound;
            g_1_pActiveSoundList = sound;
            ix_system_assign_voice(sound, releasedVoice);
        } else {
            if (sound == g_1_pFreeSoundHead)
                g_1_pFreeSoundHead = sound->next;
            if (sound->next != 0)
                sound->next->previous = sound->previous;
            if (sound->previous != 0)
                sound->previous->next = sound->next;
            sound->next = g_1_pWaitingSoundList;
            sound->previous = 0;
            if (g_1_pWaitingSoundList != 0)
                g_1_pWaitingSoundList->previous = sound;
            g_1_pWaitingSoundList = sound;
        }
    }
}

/* Function start: 0x46AA9E */   /* source lines 151, 152 */
extern "C" void __fastcall ix_sound_release(IxSound *sound)
{
    if ((sound->flags & IX_SOUND_PLAYING) == 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_SOUND_FILE, 151);
        ix_log_printf("can't release non playing sound!");
        return;
    }
    if ((sound->flags & IX_SOUND_LOOPING) == 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_SOUND_FILE, 152);
        ix_log_printf("can't release non looping sound!");
        return;
    }
    if ((sound->flags & IX_SOUND_HAS_VOICE) != 0) {
        ix_dspv_clear_flag4(sound->voice);
        sound->flags |= IX_SOUND_RELEASE_PENDING;
    } else {
        ix_sound_stop(sound);
    }
    return;
}

/* Function start: 0x46AB4F */   /* source line(s) 165: can't stop sound that's not playing! */
extern "C" void __fastcall ix_sound_stop(IxSound *sound)
{
    IxSound *replacement;
    int voice;

    if ((sound->flags & IX_SOUND_PLAYING) == 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_SOUND_FILE, 165);
        ix_log_printf("can't stop sound that's not playing!");
        return;
    }

    sound->flags &= ~IX_SOUND_PLAYING;
    if ((sound->flags & IX_SOUND_HAS_VOICE) != 0) {
        voice = ix_system_release_voice(sound);
        replacement = ix_system_find_highest_waiting(
            g_1_pWaitingSoundList, 0);

        if (sound == g_1_pActiveSoundList)
            g_1_pActiveSoundList = sound->next;
        if (sound->next != 0)
            sound->next->previous = sound->previous;
        if (sound->previous != 0)
            sound->previous->next = sound->next;
        sound->next = g_1_pFreeSoundHead;
        sound->previous = 0;
        if (g_1_pFreeSoundHead != 0)
            g_1_pFreeSoundHead->previous = sound;
        g_1_pFreeSoundHead = sound;

        if (replacement != 0) {
            ix_system_assign_voice(replacement, voice);
            if (replacement == g_1_pWaitingSoundList)
                g_1_pWaitingSoundList = replacement->next;
            if (replacement->next != 0)
                replacement->next->previous = replacement->previous;
            if (replacement->previous != 0)
                replacement->previous->next = replacement->next;
            replacement->next = g_1_pActiveSoundList;
            replacement->previous = 0;
            if (g_1_pActiveSoundList != 0)
                g_1_pActiveSoundList->previous = replacement;
            g_1_pActiveSoundList = replacement;
        }
    } else {
        if (sound == g_1_pWaitingSoundList)
            g_1_pWaitingSoundList = sound->next;
        if (sound->next != 0)
            sound->next->previous = sound->previous;
        if (sound->previous != 0)
            sound->previous->next = sound->next;
        sound->next = g_1_pFreeSoundHead;
        sound->previous = 0;
        if (g_1_pFreeSoundHead != 0)
            g_1_pFreeSoundHead->previous = sound;
        g_1_pFreeSoundHead = sound;
    }

    if ((sound->flags & IX_SOUND_DELETE_ON_STOP) != 0)
        ix_system_delete_sound(sound);
}

/* Function start: 0x46AD9A */
void IxSound::ix_sound_reprioritise(void)
{
    unsigned int revisedPriority;
    IxSound *waiting;
    int releasedVoice;
    IxSound *playingSound;
    int reassignedVoice;

    revisedPriority = (volume << 8) / 0xffff +
                  (pitchOffset << 8) / 0xac44 + basePriority;
    if ((flags & IX_SOUND_HAS_VOICE) != 0) {
        if (priority > revisedPriority) {
            waiting = ix_system_find_highest_waiting(
                g_1_pWaitingSoundList, revisedPriority);
            if (waiting != 0) {
                releasedVoice = ix_system_release_voice(this);

                if (this == g_1_pActiveSoundList)
                    g_1_pActiveSoundList = next;
                if (next != 0)
                    next->previous = previous;
                if (previous != 0)
                    previous->next = next;
                next = g_1_pWaitingSoundList;
                previous = 0;
                if (g_1_pWaitingSoundList != 0)
                    g_1_pWaitingSoundList->previous = this;
                g_1_pWaitingSoundList = this;

                if (waiting == g_1_pWaitingSoundList)
                    g_1_pWaitingSoundList = waiting->next;
                if (waiting->next != 0)
                    waiting->next->previous = waiting->previous;
                if (waiting->previous != 0)
                    waiting->previous->next = waiting->next;
                waiting->next = g_1_pActiveSoundList;
                waiting->previous = 0;
                if (g_1_pActiveSoundList != 0)
                    g_1_pActiveSoundList->previous = waiting;
                g_1_pActiveSoundList = waiting;
                ix_system_assign_voice(waiting, releasedVoice);
            }
        }
    } else if ((flags & IX_SOUND_PLAYING) != 0 && priority < revisedPriority) {
        playingSound = ix_system_find_lowest_playing(
            g_1_pActiveSoundList, revisedPriority);
        if (playingSound != 0) {
            reassignedVoice = ix_system_release_voice(playingSound);

            if (playingSound == g_1_pActiveSoundList)
                g_1_pActiveSoundList = playingSound->next;
            if (playingSound->next != 0)
                playingSound->next->previous = playingSound->previous;
            if (playingSound->previous != 0)
                playingSound->previous->next = playingSound->next;
            playingSound->next = g_1_pWaitingSoundList;
            playingSound->previous = 0;
            if (g_1_pWaitingSoundList != 0)
                g_1_pWaitingSoundList->previous = playingSound;
            g_1_pWaitingSoundList = playingSound;

            if (this == g_1_pWaitingSoundList)
                g_1_pWaitingSoundList = next;
            if (next != 0)
                next->previous = previous;
            if (previous != 0)
                previous->next = next;
            next = g_1_pActiveSoundList;
            previous = 0;
            if (g_1_pActiveSoundList != 0)
                g_1_pActiveSoundList->previous = this;
            g_1_pActiveSoundList = this;
            ix_system_assign_voice(this, reassignedVoice);
        }
    }
    priority = revisedPriority;
}

/* Function start: 0x46B0C5 */
void IxSound::ix_sound_set_delete_on_stop(int enabled)
{
    if (enabled != 0)
        flags |= IX_SOUND_DELETE_ON_STOP;
    else
        flags &= ~IX_SOUND_DELETE_ON_STOP;
}

/* Function start: 0x46B0F8 */
extern "C" int __fastcall ix_sound_is_playing(IxSound *sound)
{
    return (sound->flags & IX_SOUND_PLAYING) != 0;
}
