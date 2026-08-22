/*
 *  ix / D:\rnd\prj\ix\win95\dsp\dsp.cpp
 *
 *  Original address range: 0x00444910 - 0x004451B4   (21 functions)
 *  Range is exact: recovered from this module's own assert __FILE__ anchors in
 *  the shipped debug build (see docs/ORDER.md).
 *
 *  Implement in ADDRESS ORDER -- MSVC emits functions in source order, so the
 *  list below is the original source order of this file.
 */
#include "ix.h"
#ifndef SDL_PORT
#include <dsound.h>
#endif
#include <stdlib.h>
#include <string.h>

#define IX_DSP_FILE "D:\\rnd\\prj\\ix\\win95\\dsp\\dsp.cpp"

HANDLE g_hMixerThread_005c4ec0;
HWND g_hDspWindow_005c4ec4;
HANDLE g_hMixerWakeEvent_005c4ec8;
unsigned int g_dwMixerWriteOffset_005c4ecc;
LPDIRECTSOUND g_pDirectSound_005c4ed0;
unsigned int g_dwMixerBufferSize_005c4ed4;
unsigned int g_dwDspFlags_005c4ed8;
DWORD g_dwMixerThreadId_005c4edc;
LPDIRECTSOUNDBUFFER g_pMixerBuffer_005c4ee0;
unsigned int g_dwDspTick_005c52e8;
LPDIRECTSOUNDBUFFER g_pPrimarySoundBuffer_005c52ec;
int g_nVoicesAllocated_005c574c;
/* The host CRT's malloc takes size_t, which is not unsigned int on every
 * modern target; keep one definition so the globals audit sees one range. */
#ifdef SDL_PORT
#define IX_MALLOC_HOOK ((void *(__cdecl *)(unsigned int))malloc)
#else
#define IX_MALLOC_HOOK malloc
#endif
void *(__cdecl *g_pIxMalloc_004a0c18)(unsigned int) = IX_MALLOC_HOOK;
void (__cdecl *g_pIxFree_004a0c1c)(void *) = free;

/* Function start: 0x489990 */   /* source line 62 */
int ix_dsp_init(void)
{
    if ((g_dwDspFlags_005c4ed8 & 1) == 0) {
        g_nVoiceCount_005c5748 = 32;
        g_nStreamCount_005c5750 = 2;
        g_dwMixerWriteOffset_005c4ecc = 0;
        InitializeCriticalSection(&g_csMixer_005c5730);
        g_hMixerWakeEvent_005c4ec8 = CreateEventA(0, TRUE, FALSE, 0);
        ix_dsp_build_pan_tables();
#ifdef SDL_PORT
        g_dwDspFlags_005c4ed8 |= 4;
#endif
        g_hMixerThread_005c4ec0 = CreateThread(
            0, 0x1000, ix_mixer_thread_proc, 0, 0,
            &g_dwMixerThreadId_005c4edc);
        if (g_hMixerThread_005c4ec0 == 0) {
            ix_log_printf("Fatal [%s - %d]:\n", IX_DSP_FILE, 62);
            ix_log_printf("Failed to start mixer");
            exit(-1);
        }
        g_dwDspFlags_005c4ed8 |= 1;
    }
    return 0;
}

/* Function start: 0x489A4F */
void ix_dsp_shutdown(void)
{
    if ((g_dwDspFlags_005c4ed8 & 1) != 0) {
#ifdef SDL_PORT
        if (g_hMixerThread_005c4ec0 != 0) {
            g_dwDspFlags_005c4ed8 &= ~4U;
            SetEvent(g_hMixerWakeEvent_005c4ec8);
            WaitForSingleObject(g_hMixerThread_005c4ec0, INFINITE);
            CloseHandle(g_hMixerThread_005c4ec0);
            g_hMixerThread_005c4ec0 = 0;
        }
#else
        if ((g_dwDspFlags_005c4ed8 & 4) != 0) {
            g_dwDspFlags_005c4ed8 &= ~4U;
            SetEvent(g_hMixerWakeEvent_005c4ec8);
            WaitForSingleObject(g_hMixerThread_005c4ec0, INFINITE);
        }
        if (g_pDirectSound_005c4ed0 != 0) {
            g_pDirectSound_005c4ed0->Release();
            g_pDirectSound_005c4ed0 = 0;
        }
#endif
        DeleteCriticalSection(&g_csMixer_005c5730);
        CloseHandle(g_hMixerWakeEvent_005c4ec8);
        g_dwDspFlags_005c4ed8 &= 0x7ffffffe;
    }
}

/* Function start: 0x489AE2 */
void ix_dsp_configure(int option, void *value)
{
    switch (option) {
    case 0:
        g_hDspWindow_005c4ec4 = (HWND)value;
        break;
    case 1:
        g_pDirectSound_005c4ed0 = (LPDIRECTSOUND)value;
        break;
    case 2:
        if ((g_dwDspFlags_005c4ed8 & 1) != 0)
            SetEvent(g_hMixerWakeEvent_005c4ec8);
        break;
    case 3:
        if (value != 0)
            g_dwDspFlags_005c4ed8 |= 0x10;
        else
            g_dwDspFlags_005c4ed8 &= ~0x10U;
        break;
    case 4:
        g_pIxMalloc_004a0c18 = (void *(__cdecl *)(unsigned int))value;
        break;
    case 5:
        g_pIxFree_004a0c1c = (void (__cdecl *)(void *))value;
        break;
    }
}

/* Function start: 0x489BA5 */
BOOL CALLBACK ix_dsp_open_driver(LPGUID guid, LPSTR description,
                                 LPSTR module, LPVOID context)
{
#ifdef SDL_PORT
    (void)guid;
    (void)description;
    (void)module;
    (void)context;
    return TRUE;
#else
    HRESULT result;

    if (g_pDirectSound_005c4ed0 != 0) {
        g_pDirectSound_005c4ed0->Release();
        g_pDirectSound_005c4ed0 = 0;
    }
    result = DirectSoundCreate(guid, &g_pDirectSound_005c4ed0, 0);
    if (result == DS_OK) {
        result = g_pDirectSound_005c4ed0->SetCooperativeLevel(
            g_hDspWindow_005c4ec4,
            (g_dwDspFlags_005c4ed8 & 0x10) == 0
                ? DSSCL_WRITEPRIMARY : DSSCL_EXCLUSIVE);
        if (result == DS_OK) {
            ix_log_printf("sound driver: %s [%s]\n", description, module);
            return FALSE;
        }
        if (g_pDirectSound_005c4ed0 != 0) {
            g_pDirectSound_005c4ed0->Release();
            g_pDirectSound_005c4ed0 = 0;
        }
    }
    return TRUE;
#endif
}

/* Function start: 0x489C7D */
unsigned int ix_dsp_get_tick(void)
{
    return g_dwDspTick_005c52e8;
}

/* Function start: 0x489C92 */
unsigned int ix_dsp_get_flags(void)
{
    return g_dwDspFlags_005c4ed8;
}

/* Function start: 0x489CA7 */
void ix_dsp_set_config_bit1(int enabled)
{
    if (enabled != 0)
        g_dwDspFlags_005c4ed8 |= 2;
    else
        g_dwDspFlags_005c4ed8 &= ~2U;
}

/* Function start: 0x489CD4 */
void ix_dsp_set_config_bit3(int enabled)
{
    if (enabled != 0)
        g_dwDspFlags_005c4ed8 |= 8;
    else
        g_dwDspFlags_005c4ed8 &= ~8U;
}

/* Function start: 0x489D01 */
int ix_dsp_get_voice_count(void)
{
    return g_nVoiceCount_005c5748;
}

/* Function start: 0x489D16 */   /* source line(s) 176: can't change voice count while voices are in use */
void ix_dsp_set_voice_count(int voiceCount)
{
    if (g_nVoicesAllocated_005c574c != 0 ||
        g_nStreamsAllocated_005c5754 != 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_DSP_FILE, 176);
        ix_log_printf("can't change voice count while voices are in use");
    } else {
        if (voiceCount >= 0)
            g_nVoiceCount_005c5748 = voiceCount < 32 ? voiceCount : 32;
        else
            g_nVoiceCount_005c5748 = 0;
    }
}

/* Function start: 0x489D98 */
int ix_dsp_get_stream_count(void)
{
    return g_nStreamCount_005c5750;
}

/* Function start: 0x489DAD */   /* source line(s) 187: can't change stream count while voices are in use */
void ix_dsp_set_stream_count(int streamCount)
{
    if (g_nVoicesAllocated_005c574c != 0 ||
        g_nStreamsAllocated_005c5754 != 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_DSP_FILE, 187);
        ix_log_printf("can't change stream count while voices are in use");
    } else {
        if (streamCount >= 0)
            g_nStreamCount_005c5750 = streamCount < 2 ? streamCount : 2;
        else
            g_nStreamCount_005c5750 = 0;
    }
}

/* Function start: 0x489E2F */
unsigned short ix_dsp_get_master_volume(void)
{
    return g_nMasterVolume_004a0c14;
}

/* Function start: 0x489E45 */
void ix_dsp_set_master_volume(unsigned short volume)
{
    int voice;

    if ((volume & 0xffff) >= 0) {
        g_nMasterVolume_004a0c14 = (unsigned short)(
            (volume & 0xffff) < 0xffff ? (volume & 0xffff) : 0xffff);
    } else {
        g_nMasterVolume_004a0c14 = 0;
    }
    for (voice = 0; voice < g_nVoiceCount_005c5748; voice++) {
        if ((g_voices_005c52f0[voice].flags & IX_VOICE_ACTIVE) != 0)
            ix_dspv_recalc_mix(voice);
    }
}

/* Function start: 0x489ED2 */
void ix_dsp_build_pan_tables(void)
{
    short left;
    short distance;
    short position;
    short scale;
    short right;

    scale = 0x3ff;
    left = 0x7fff;
    for (position = 0; position < 0x40; position++) {
        distance = (short)(0x20 - position);
        right = (short)(distance * scale);
        g_anPanTable_005c4ee8[position * 2] = left;
        g_anPanTable_005c4ee8[position * 2 + 1] = right;
        g_anPanTable_005c4ee8[0x100 + position * 2] =
            (short)(left * -1);
        g_anPanTable_005c4ee8[0x100 + position * 2 + 1] =
            (short)(right * -1);
    }
    right = (short)0x8001;
    for (position = 0x40; position < 0x80; position++) {
        distance = (short)(0x60 - position);
        left = (short)(distance * scale);
        g_anPanTable_005c4ee8[position * 2] = left;
        g_anPanTable_005c4ee8[position * 2 + 1] = right;
        g_anPanTable_005c4ee8[0x100 + position * 2] =
            (short)(left * -1);
        g_anPanTable_005c4ee8[0x100 + position * 2 + 1] =
            (short)(right * -1);
    }
}

/* Function start: 0x48A017 */
const char *ix_dsp_result_to_text(int result)
{
#ifdef SDL_PORT
    (void)result;
    return SDL_GetError();
#else
    switch (result) {
    case DS_OK:
        return "The function succeeded.";
    case DSERR_ALLOCATED:
        return "The function failed because resources (such as a priority level) were already in use by another caller.";
    case DSERR_ALREADYINITIALIZED:
        return "This object is already initialized.";
    case DSERR_BADFORMAT:
        return "The specified wave format is not supported.";
    case DSERR_BUFFERLOST:
        return "The buffer memory has been lost and must be restored.";
    case DSERR_CONTROLUNAVAIL:
        return "The control (volume, pan, and so forth) requested by the caller is not available.";
    case DSERR_INVALIDCALL:
        return "This function is not valid for the current state of this object";
    case DSERR_INVALIDPARAM:
        return "An invalid parameter was passed to the returning function.";
    case DSERR_NOAGGREGATION:
        return "This object does not support aggregation.";
    case DSERR_NODRIVER:
        return "No sound driver is available for use.";
    case DSERR_OUTOFMEMORY:
        return "The DirectSound subsystem couldn't allocate sufficient memory to complete the caller's request.";
    case DSERR_PRIOLEVELNEEDED:
        return "The caller does not have the priority level required for the function to succeed.";
    case E_NOINTERFACE:
        return "The requested COM interface is not available.";
    default:
        return "Unknow dsound error!";
    }
#endif
}

/* Function start: 0x48A1A3 */
void *ix_dsp_alloc(unsigned int bytes)
{
    return g_pIxMalloc_004a0c18(bytes);
}

/* Function start: 0x48A1C0 */
#pragma function(memcpy)
void *ix_dsp_copy(void *destination, const void *source, unsigned int bytes)
{
    return memcpy(destination, source, bytes);
}

/* Function start: 0x48A1E4 */
void ix_dsp_free(void *memory)
{
    g_pIxFree_004a0c1c(memory);
}
