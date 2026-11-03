/*
 *  ix / D:\Rnd\prj\ix\src\sound\sample.cpp
 *
 *  Original address range: 0x0044879C - 0x004492DF   (4 functions)
 *  Range is exact: recovered from this module's own assert __FILE__ anchors in
 *  the shipped debug build (see docs/ORDER.md).
 *
 *  Implement in ADDRESS ORDER -- MSVC emits functions in source order, so the
 *  list below is the original source order of this file.
 */
#include "ix.h"
#include <math.h>

#define IX_SAMPLE_FILE "D:\\Rnd\\prj\\ix\\src\\sound\\sample.cpp"

#define IX_SWAP_WORD(value) \
    (short)((((value) & 0x00ff) << 8) | \
            (((value) >> 8) & 0x00ff))

#define IX_SWAP_DWORD(value) \
    ((((unsigned int)(value) & 0x0000ff00) << 8) | \
     (((unsigned int)(value) >> 8) & 0x0000ff00) | \
     ((unsigned int)(value) << 24) | \
     ((unsigned int)(value) >> 24))

#define IX_UNSIGNED_TO_DOUBLE(value) \
    ((double)(long)((value) - 2147483647L - 1) + 2147483648.0)

#ifdef SDL_PORT
#pragma pack(push, 1)
#endif
struct IxSampleChunk {
    unsigned int id;
    unsigned int size;
};

struct IxAiffCommonChunk {
    short channels;
    unsigned int sampleFrames;
    short bitsPerSample;
    unsigned char sampleRate[10];
};

struct IxAiffMarker {
    short id;
    unsigned int position;
    unsigned char nameLength;
};

struct IxWaveFormat {
    unsigned short format;
    unsigned short channels;
    int frequency;
    unsigned int bytesPerSecond;
    unsigned short blockAlignment;
    unsigned short bitsPerSample;
};
#ifdef SDL_PORT
#pragma pack(pop)
#endif

/* Function start: 0x46B21C */   /* source line(s) 166;169;170;193;195;197: sample already ready! | FORM id not found! | AIFF id not found! | Unsupported number of ch */
int IxSample::ix_sample_load_aiff(void *data, int bytes)
{
    unsigned char *end;
    IxSampleChunk *chunk;

    unsigned char *form;
    form = (unsigned char *)data;
    if ((flags & IX_SAMPLE_READY) != 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_SAMPLE_FILE, 166);
        ix_log_printf("sample already ready!");
        return -1;
    }
    flags |= IX_SAMPLE_LOADING;
    if (*(unsigned int *)form != *(unsigned int *)"FORM") {
        ix_log_printf("Warning [%s - %d]:\n", IX_SAMPLE_FILE, 169);
        ix_log_printf("FORM id not found!");
        return -1;
    }
    if (*(unsigned int *)(form + 8) != *(unsigned int *)"AIFF") {
        ix_log_printf("Warning [%s - %d]:\n", IX_SAMPLE_FILE, 170);
        ix_log_printf("AIFF id not found!");
        return -1;
    }
    end = (unsigned char *)data + bytes;
    chunk = (IxSampleChunk *)((unsigned char *)data + 12);
    while ((unsigned char *)chunk < end) {
        chunk->size = IX_SWAP_DWORD(chunk->size);
        switch (chunk->id) {
        case 0x4d4d4f43: {
            int sampleRate;
            IxAiffCommonChunk *common;

            common = (IxAiffCommonChunk *)(chunk + 1);
            common->channels = IX_SWAP_WORD(common->channels);
            common->sampleFrames = IX_SWAP_DWORD(common->sampleFrames);
            common->bitsPerSample = IX_SWAP_WORD(common->bitsPerSample);
            sampleRate = (int)ix_sample_convert_from_ieee_extended(common->sampleRate);
            rateNumerator = 1;
            rateDenominator = rateNumerator;
            if (common->channels != 1 && common->channels != 2) {
                ix_log_printf("Warning [%s - %d]:\n", IX_SAMPLE_FILE, 193);
                ix_log_printf("Unsupported number of channels in AIFF!");
                return -1;
            }
            if (sampleRate < 4000 || sampleRate > 44100) {
                ix_log_printf("Warning [%s - %d]:\n", IX_SAMPLE_FILE, 195);
                ix_log_printf("Unsupported AIFF sample rate!");
                return -1;
            }
            if (common->bitsPerSample != 8 && common->bitsPerSample != 16) {
                ix_log_printf("Warning [%s - %d]:\n", IX_SAMPLE_FILE, 197);
                ix_log_printf("Unsupported AIFF bits per sample!");
                return -1;
            }
            bitsPerSample = common->bitsPerSample;
            channels = common->channels;
            frequency = sampleRate;
            loopStart = 0;
            loopEnd = (unsigned int)-1;
            sampleCount = loopEnd;
            bytesPerFrame = ((int)bitsPerSample / 8) * (int)channels;
            break;
        }

        case 0x4b52414d: {
            IxAiffMarker *marker;

            marker = (IxAiffMarker *)(chunk + 1);
            marker = (IxAiffMarker *)((unsigned char *)marker + 2);
            loopStart = IX_SWAP_DWORD(marker->position);
            marker = (IxAiffMarker *)((unsigned char *)marker +
                                      marker->nameLength + sizeof(IxAiffMarker));
#ifdef SDL_PORT
            while (((uintptr_t)marker & 1) != 0)
#else
            while (((unsigned int)marker & 1) != 0)
#endif
                marker = (IxAiffMarker *)((unsigned char *)marker + 1);
            loopEnd = IX_SWAP_DWORD(marker->position);
            flags |= IX_SAMPLE_LOOPING;
            break;
        }

        case 0x444e5353: {
            int samplesRemaining;
            short *samples;

            buffer = (unsigned char *)ix_dsp_alloc(chunk->size);
            sampleCount = chunk->size / bytesPerFrame;
            if (loopEnd == (unsigned int)-1)
                loopEnd = sampleCount;
            if (bitsPerSample == 16) {
                samples = (short *)(chunk + 1);
                samplesRemaining = sampleCount * channels;
                while (samplesRemaining--) {
                    *samples = IX_SWAP_WORD(*samples);
                    samples++;
                }
            }
            ix_dsp_copy(buffer, chunk + 1, chunk->size);
            break;
        }
        }
        chunk = (IxSampleChunk *)((unsigned char *)chunk +
                                  chunk->size + sizeof(IxSampleChunk));
    }
    flags &= ~IX_SAMPLE_LOADING;
    flags |= IX_SAMPLE_READY;
    return 0;
}

/* Function start: 0x46B70B */   /* source line(s) 261;264;265;279;281;283;285: sample already ready! | RIFF id not found! | WAVE id not found! | Unsupported number of ch */
int IxSample::ix_sample_load_wav(void *data, int bytes)
{
    unsigned char *end;
    IxSampleChunk *chunk;

    unsigned char *riff;
    riff = (unsigned char *)data;
    if ((flags & IX_SAMPLE_READY) != 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_SAMPLE_FILE, 261);
        ix_log_printf("sample already ready!");
        return -1;
    }
    flags |= IX_SAMPLE_LOADING;
    if (*(unsigned int *)riff != *(unsigned int *)"RIFF") {
        ix_log_printf("Warning [%s - %d]:\n", IX_SAMPLE_FILE, 264);
        ix_log_printf("RIFF id not found!");
        return -1;
    }
    if (*(unsigned int *)(riff + 8) != *(unsigned int *)"WAVE") {
        ix_log_printf("Warning [%s - %d]:\n", IX_SAMPLE_FILE, 265);
        ix_log_printf("WAVE id not found!");
        return -1;
    }
    end = (unsigned char *)data + bytes;
    chunk = (IxSampleChunk *)((unsigned char *)data + 12);
    while ((unsigned char *)chunk < end) {
        switch (chunk->id) {
        case 0x20746d66: {
            IxWaveFormat *format;

            format = (IxWaveFormat *)(chunk + 1);
            if (format->channels != 1 && format->channels != 2) {
                ix_log_printf("Warning [%s - %d]:\n", IX_SAMPLE_FILE, 279);
                ix_log_printf("Unsupported number of channels in WAV!");
                return -1;
            }
            if (format->format != 1) {
                ix_log_printf("Warning [%s - %d]:\n", IX_SAMPLE_FILE, 281);
                ix_log_printf("Unsupported WAV format!");
                return -1;
            }
            if (format->frequency < 4000 || format->frequency > 44100) {
                ix_log_printf("Warning [%s - %d]:\n", IX_SAMPLE_FILE, 283);
                ix_log_printf("Unsupported WAV sample rate!");
                return -1;
            }
            if (format->bitsPerSample != 8 && format->bitsPerSample != 16) {
                ix_log_printf("Warning [%s - %d]:\n", IX_SAMPLE_FILE, 285);
                ix_log_printf("Unsupported WAV bits per sample!");
                return -1;
            }
            bitsPerSample = format->bitsPerSample;
            channels = format->channels;
            frequency = format->frequency;
            loopStart = 0;
            loopEnd = (unsigned int)-1;
            sampleCount = loopEnd;
            bytesPerFrame = ((int)bitsPerSample / 8) * (int)channels;
            rateNumerator = 1;
            rateDenominator = rateNumerator;
            break;
        }

        case 0x61746164: {
            int samplesRemaining;
            signed char *samples;

            buffer = (unsigned char *)ix_dsp_alloc(chunk->size);
            sampleCount = chunk->size / bytesPerFrame;
            loopEnd = sampleCount;
            if (bitsPerSample == 8) {
                samples = (signed char *)(chunk + 1);
                samplesRemaining = bytesPerFrame * sampleCount;
                while (samplesRemaining--) {
                    *samples = (signed char)(*samples - 0x80);
                    samples++;
                }
            }
            ix_dsp_copy(buffer, chunk + 1, chunk->size);
            break;
        }
        default:
            break;
        }
        chunk = (IxSampleChunk *)((unsigned char *)chunk +
                                  chunk->size + sizeof(IxSampleChunk));
    }
    flags &= ~IX_SAMPLE_LOADING;
    flags |= IX_SAMPLE_READY;
    return 0;
}

/* Function start: 0x46BAC4 */   /* source line(s) 333: sample already ready! */
int IxSample::ix_sample_load_raw(void *data, int bytes, int newFrequency,
                                 int newBitsPerSample, int newChannels)
{
    if ((flags & IX_SAMPLE_READY) != 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_SAMPLE_FILE, 333);
        ix_log_printf("sample already ready!");
        return -1;
    }
    flags |= IX_SAMPLE_LOADING;
    buffer = (unsigned char *)ix_dsp_alloc(bytes);
    ix_dsp_copy(buffer, data, bytes);
    bitsPerSample = newBitsPerSample;
    channels = newChannels;
    frequency = newFrequency;
    loopStart = 0;
    bytesPerFrame = ((int)bitsPerSample / 8) * (int)channels;
    loopEnd = bytes / (int)bytesPerFrame;
    sampleCount = loopEnd;
    rateNumerator = 1;
    rateDenominator = rateNumerator;
    flags &= ~IX_SAMPLE_LOADING;
    flags |= IX_SAMPLE_READY;
    return 0;
}

void *ix_dsp_copy(void *destination, const void *source,
                  unsigned int bytes);

/* Function start: 0x46BBCC */
long IxSample::ix_sample_convert_from_ieee_extended(unsigned char *bytes)
{
    unsigned long loMant;
    int expon;
    double f;
    unsigned long hiMant;
    unsigned char *src;

    src = bytes;
    expon = ((src[0] & 0x7f) << 8) | (src[1] & 0xff);
    hiMant = ((unsigned long)(src[2] & 0xff) << 24) |
                   ((unsigned long)(src[3] & 0xff) << 16) |
                   ((unsigned long)(src[4] & 0xff) << 8) |
                   ((unsigned long)(src[5] & 0xff));
    loMant = ((unsigned long)(src[6] & 0xff) << 24) |
                  ((unsigned long)(src[7] & 0xff) << 16) |
                  ((unsigned long)(src[8] & 0xff) << 8) |
                  ((unsigned long)(src[9] & 0xff));
    if (expon == 0 && hiMant == 0 && loMant == 0) {
        f = 0;
    } else if (expon == 0x7fff) {
        f = HUGE_VAL;
    } else {
        expon -= 16383;
        f = ldexp(IX_UNSIGNED_TO_DOUBLE(hiMant), expon -= 31);
        f += ldexp(IX_UNSIGNED_TO_DOUBLE(loMant), expon -= 32);
    }
    if ((src[0] & 0x80) != 0)
        return (long)-f;
    else
        return (long)f;
}
