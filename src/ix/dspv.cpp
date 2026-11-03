/*
 *  ix / D:\rnd\prj\ix\win95\dsp\dspv.cpp
 *
 *  Original address range: 0x004467C5 - 0x004471FF   (15 functions)
 *  Range is exact: recovered from this module's own assert __FILE__ anchors in
 *  the shipped debug build (see docs/ORDER.md).
 *
 *  Built /Od like the rest of ix: full EBP frame, EBX/ESI/EDI saved whether used
 *  or not, one shared epilogue.  Do not optimise this file.
 *
 *  Implement in ADDRESS ORDER -- MSVC emits functions in source order, so the
 *  list below is the original source order of this file.
 */
#include "ix.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

unsigned short g_nMasterVolume_004a0c14 = 0xffff;
short          g_anPanTable_005c4ee8[512];
IxVoice        g_voices_005c52f0[34];
CRITICAL_SECTION g_csMixer_005c5730;
int            g_nVoiceCount_005c5748;

/* Function start: 0x48A235 */   /* source line 23 */
void ix_dspv_set_active(int voice)
{
    if (voice < 0 || voice >= g_nVoiceCount_005c5748) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_DSPV_FILE, 23);
        ix_log_printf("%d Invalid voice index!", voice);
        exit(-1);
    }
    g_voices_005c52f0[voice].flags |= IX_VOICE_ACTIVE;
}

/* Function start: 0x48A299 */   /* source line 29 */
void ix_dspv_clear_active(int voice)
{
    if (voice < 0 || voice >= g_nVoiceCount_005c5748) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_DSPV_FILE, 29);
        ix_log_printf("%d Invalid voice index!", voice);
        exit(-1);
    }
    g_voices_005c52f0[voice].flags &= ~IX_VOICE_ACTIVE;
}

unsigned int ix_dspv_get_flags(int voice);

/* Function start: 0x48A2FE */   /* source line 35 */
void ix_dspv_clear_flag4(int voice)
{
    if (voice < 0 || voice >= g_nVoiceCount_005c5748) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_DSPV_FILE, 35);
        ix_log_printf("%d Invalid voice index!", voice);
        exit(-1);
    }
    g_voices_005c52f0[voice].flags &= ~IX_VOICE_FLAG4;
}

/* Function start: 0x48A362 */   /* source line 41 */
unsigned int ix_dspv_get_flags(int voice)
{
    if (voice < 0 || voice >= g_nVoiceCount_005c5748) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_DSPV_FILE, 41);
        ix_log_printf("%d Invalid voice index!", voice);
        exit(-1);
    }
    return g_voices_005c52f0[voice].flags;
}

/* Function start: 0x48A3C6 */
/* No bounds check in the original -- callers are internal. */
void ix_dspv_set_flag4(int voice, int on)
{
    if (on != 0)
        g_voices_005c52f0[voice].flags |= IX_VOICE_FLAG4;
    else
        g_voices_005c52f0[voice].flags &= ~IX_VOICE_FLAG4;
}

/* Function start: 0x48A3FF */   /* source line 55 */
void ix_dspv_set_buffer(int voice, unsigned char *p, int len)
{
    if (voice < 0 || voice >= g_nVoiceCount_005c5748) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_DSPV_FILE, 55);
        ix_log_printf("%d Invalid voice index!", voice);
        exit(-1);
    }
    IxVoice *v = &g_voices_005c52f0[voice];

    EnterCriticalSection(&g_csMixer_005c5730);
    v->cursor = p;
    v->start = p;
    v->end = p + len;
    LeaveCriticalSection(&g_csMixer_005c5730);
}

/* Function start: 0x48A49B */   /* source line 69 */
int ix_dspv_get_position(int voice)
{
    if (voice < 0 || voice >= g_nVoiceCount_005c5748) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_DSPV_FILE, 69);
        ix_log_printf("%d Invalid voice index!", voice);
        exit(-1);
    }
    IxVoice *v = &g_voices_005c52f0[voice];

    return v->cursor - v->start;
}

void ix_dspv_recalc_mix(int voice);

/* Function start: 0x48A510 */   /* source line 77 */
void ix_dspv_set_position(int voice, int off)
{
    if (voice < 0 || voice >= g_nVoiceCount_005c5748) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_DSPV_FILE, 77);
        ix_log_printf("%d Invalid voice index!", voice);
        exit(-1);
    }
    IxVoice *v = &g_voices_005c52f0[voice];

    v->cursor = v->start + off;
}

void ix_dspv_set_pan(int voice, unsigned short angle);
void ix_dspv_set_frequency(int voice, unsigned int hz);

/* Function start: 0x48A588 */   /* source line 85 */
void ix_dspv_set_volume(int voice, short vol)
{
    if (voice < 0 || voice >= g_nVoiceCount_005c5748) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_DSPV_FILE, 85);
        ix_log_printf("%d Invalid voice index!", voice);
        exit(-1);
    }
    g_voices_005c52f0[voice].volume = vol;
    ix_dspv_recalc_mix(voice);
}

/* Function start: 0x48A5FC */   /* source line 93 */
void ix_dspv_set_pan(int voice, unsigned short angle)
{
    IxVoice *state;
    short step;
    short delta;
    short position;

    if (voice < 0 || g_nVoiceCount_005c5748 <= voice) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_DSPV_FILE, 93);
        ix_log_printf("%d Invalid voice index!", voice);
        exit(-1);
    }
    state = &g_voices_005c52f0[voice];
    step = (short)((unsigned short)angle >> 9);
    delta = step - state->panAngle;
    if (delta > 0x40)
        delta = delta - 0x80;
    if (delta < -0x40)
        delta = delta + 0x80;
    position = state->panPos + delta;
    if (position < 0)
        position = position + 0x100;
    if (position >= 0x100)
        position = position - 0x100;
    state->panAngle = step;
    state->panPos = position;
    ix_dspv_recalc_mix(voice);
}

/* Function start: 0x48A721 */   /* source line 131 */
void ix_dspv_set_frequency(int voice, unsigned int hz)
{
    if (voice < 0 || g_nVoiceCount_005c5748 <= voice) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_DSPV_FILE, 131);
        ix_log_printf("%d Invalid voice index!", voice);
        exit(-1);
    }
    g_voices_005c52f0[voice].rate =
        (short)(((int)(hz & 0xffff) << 8) / IX_MIXER_BASE_RATE);
}

/* Function start: 0x48A79C */   /* source lines 137, 138 */
void ix_dspv_set_bits_per_sample(int voice, int bps)
{
    if (voice < 0 || voice >= g_nVoiceCount_005c5748) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_DSPV_FILE, 137);
        ix_log_printf("%d Invalid voice index!", voice);
        exit(-1);
    }
    if (bps != 8 && bps != 16) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_DSPV_FILE, 138);
        ix_log_printf("%d is an unsupported bps!", bps);
        exit(-1);
    }
    if (bps == 16)
        g_voices_005c52f0[voice].flags |= IX_VOICE_16BIT;
    else
        g_voices_005c52f0[voice].flags &= ~IX_VOICE_16BIT;
}

/* Function start: 0x48A865 */   /* source lines 148, 149 */
void ix_dspv_set_channels(int voice, int channels)
{
    if (voice < 0 || voice >= g_nVoiceCount_005c5748) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_DSPV_FILE, 148);
        ix_log_printf("%d Invalid voice index!", voice);
        exit(-1);
    }
    if (channels != 1 && channels != 2) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_DSPV_FILE, 149);
        ix_log_printf("%d is an unsupported number of channels!", channels);
        exit(-1);
    }
    if (channels == 2)
        g_voices_005c52f0[voice].flags |= IX_VOICE_STEREO;
    else
        g_voices_005c52f0[voice].flags &= ~IX_VOICE_STEREO;
}

/* Function start: 0x48A92F */
void ix_dspv_recalc_mix(int voice)
{
    IxVoice *state;
    short position;
    int level;

    state = &g_voices_005c52f0[voice];
    position = state->panPos;
    level = ((unsigned int)state->volume
            * (unsigned int)g_nMasterVolume_004a0c14) / 0xffff;
    EnterCriticalSection(&g_csMixer_005c5730);
    state->leftGain =
        (short)((int)g_anPanTable_005c4ee8[position * 2] * level >> 0x10);
    state->rightGain =
        (short)((int)g_anPanTable_005c4ee8[position * 2 + 1] * level >> 0x10);
    state->leftGainHi = (unsigned char)(state->leftGain >> 8);
    state->rightGainHi = (unsigned char)(state->rightGain >> 8);
    LeaveCriticalSection(&g_csMixer_005c5730);
}

#ifdef SDL_PORT
/* The retail mixer is hand-written assembly in dspmix.asm. */
extern "C" void ix_dspv_mix(void *outputBuffer, unsigned int outputBytes)
{
    short *output;
    unsigned int frameCount;
    int voice;

    memset(outputBuffer, 0, outputBytes);
    output = (short *)outputBuffer;
    frameCount = outputBytes / 4;
    voice = 0;
    while (voice < g_nVoiceCount_005c5748 + g_nStreamCount_005c5750) {
        IxVoice *state;

        state = &g_voices_005c52f0[voice];
        if ((state->flags & IX_VOICE_ACTIVE) != 0) {
            unsigned char *cursor;
            unsigned int frame;

            cursor = state->cursor;
            frame = 0;
            while (frame < frameCount) {
                int left;
                int right;
                int mixed;
                unsigned int bytesPerFrame;
                unsigned short phase;
                unsigned int advance;

                bytesPerFrame = (state->flags & IX_VOICE_16BIT) != 0 ? 2 : 1;
                if ((state->flags & IX_VOICE_STEREO) != 0)
                    bytesPerFrame *= 2;
                if (cursor >= state->end) {
                    if ((state->flags & IX_VOICE_FLAG4) == 0) {
                        state->flags &= ~IX_VOICE_ACTIVE;
                        break;
                    }
                    cursor = state->start;
                }
                if ((state->flags & IX_VOICE_16BIT) != 0) {
                    short sampleLeft;
                    short sampleRight;

                    memcpy(&sampleLeft, cursor, sizeof(sampleLeft));
                    if ((state->flags & IX_VOICE_STEREO) != 0)
                        memcpy(&sampleRight, cursor + 2, sizeof(sampleRight));
                    else
                        sampleRight = sampleLeft;
                    left = ((int)sampleLeft * state->leftGain) >> 16;
                    right = ((int)sampleRight * state->rightGain) >> 16;
                } else {
                    signed char sampleLeft;
                    signed char sampleRight;

                    sampleLeft = *(signed char *)cursor;
                    if ((state->flags & IX_VOICE_STEREO) != 0)
                        sampleRight = *(signed char *)(cursor + 1);
                    else
                        sampleRight = sampleLeft;
                    left = sampleLeft * (signed char)state->leftGainHi;
                    right = sampleRight * (signed char)state->rightGainHi;
                }
                mixed = output[frame * 2] + left;
                if (mixed > 0x7fff)
                    mixed = 0x7fff;
                else if (mixed < -0x8000)
                    mixed = -0x8000;
                output[frame * 2] = (short)mixed;
                mixed = output[frame * 2 + 1] + right;
                if (mixed > 0x7fff)
                    mixed = 0x7fff;
                else if (mixed < -0x8000)
                    mixed = -0x8000;
                output[frame * 2 + 1] = (short)mixed;
                frame++;
                if (frame == frameCount)
                    break;
                phase = (unsigned short)state->field_10 +
                        (unsigned short)state->rate;
                state->field_10 = (short)(phase & 0xff);
                advance = phase >> 8;
                if (advance != 0) {
                    cursor += advance * bytesPerFrame;
                    if (cursor >= state->end) {
                        if ((state->flags & IX_VOICE_FLAG4) == 0) {
                            state->flags &= ~IX_VOICE_ACTIVE;
                            break;
                        }
                        cursor = state->start;
                    }
                }
            }
            state->cursor = cursor;
        }
        voice++;
    }
}
#endif
