#include "wc1.h"

#define WC2_SDL_WAVE_CENTRE_PAN 64
#define WC2_SDL_WAVE_MAXIMUM_PAN 127

static unsigned short g_nWc2SdlPendingWavePan;
static int g_nWc2SdlPendingWavePanDepth;

extern "C" IxSound *Wc2SdlNewWaveSound(IxSample *sample)
{
    IxSound *sound;

    sound = ix_system_new_sound(sample);
    if (sound != 0 && g_nWc2SdlPendingWavePanDepth != 0)
        sound->ix_system_sound_set_pan(g_nWc2SdlPendingWavePan);
    return sound;
}

extern "C" void Wc2SdlPlayWaveWithPan(
    const char *filename, int looping, int volume, int pan)
{
    unsigned short previousPan;
    int previousDepth;

    if (pan < 0)
        pan = 0;
    else if (pan > WC2_SDL_WAVE_MAXIMUM_PAN)
        pan = WC2_SDL_WAVE_MAXIMUM_PAN;
    previousPan = g_nWc2SdlPendingWavePan;
    previousDepth = g_nWc2SdlPendingWavePanDepth;
    g_nWc2SdlPendingWavePan = (unsigned short)(
        (WC2_SDL_WAVE_CENTRE_PAN - pan) * 0x100);
    g_nWc2SdlPendingWavePanDepth++;
    playWAVE(filename, looping, volume);
    g_nWc2SdlPendingWavePanDepth = previousDepth;
    g_nWc2SdlPendingWavePan = previousPan;
}
