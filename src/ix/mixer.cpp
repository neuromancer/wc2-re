/*
 *  ix / D:\rnd\prj\ix\win95\dsp\mixer.cpp
 *
 *  Original address range: 0x00445F60 - 0x004467C4   (4 functions)
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

#pragma function(memset)

#define IX_MIXER_FILE "D:\\rnd\\prj\\ix\\win95\\dsp\\mixer.cpp"

extern HANDLE g_hMixerTask_005c4ec0;
extern HWND g_hDspWnd_005c4ec4;
extern HANDLE g_hMixerRouseEvent_005c4ec8;
extern unsigned int g_dwMixerScratchSize_005c4ed4;
extern unsigned int g_dwMixWriteOffset_005c4ecc;
extern LPDIRECTSOUND g_pDirectSound_005c4ed0;
extern unsigned int g_dwSoundOptions_005c4ed8;
extern LPDIRECTSOUNDBUFFER g_pPrimarySoundBuffer_005c52ec;
extern unsigned int g_dwDsoundTick_005c52e8;
extern LPDIRECTSOUNDBUFFER g_pMixerBuffer_005c4ee0;

BOOL CALLBACK ix_dsp_open_driver(LPGUID guid, LPSTR description,
                                 LPSTR module, LPVOID context);

/* Function start: 0x48B7C0 */   /* source lines 46, 50, 60, 63, 81, 84, 96 */
DWORD WINAPI ix_mixer_thread_proc(void *parameter)
{
#ifdef SDL_PORT
    (void)parameter;
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    if (!SdlStartAudio(ix_dspv_mix, &g_csMixer_005c5730,
                          &g_dwDsoundTick_005c52e8)) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_MIXER_FILE, 50);
        ix_log_printf("Failed to init SDL audio: %s", SDL_GetError());
        exit(-1);
    }
    while ((g_dwSoundOptions_005c4ed8 & 4) != 0) {
        WaitForSingleObject(g_hMixerRouseEvent_005c4ec8, INFINITE);
        ResetEvent(g_hMixerRouseEvent_005c4ec8);
    }
    SdlStopAudio();
    return 0;
#else
    DSCAPS dsCaps;
    PCMWAVEFORMAT format;
    DSBUFFERDESC description;
    HRESULT result;
    DSBCAPS bufCaps;

    g_dwSoundOptions_005c4ed8 |= 4;
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    DirectSoundEnumerateA(ix_dsp_open_driver, 0);
    if ((g_dwSoundOptions_005c4ed8 & 0x10) == 0 &&
        g_pDirectSound_005c4ed0 == 0) {
        ix_log_printf("Warning [%s - %d]:\n", IX_MIXER_FILE, 46);
        ix_log_printf("Failed to find write primary driver, using secondary buffer");
        g_dwSoundOptions_005c4ed8 |= 0x10;
        DirectSoundEnumerateA(ix_dsp_open_driver, 0);
    }
    if (g_pDirectSound_005c4ed0 == 0) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_MIXER_FILE, 50);
        ix_log_printf("Failed to init directsound");
        exit(-1);
    }

    memset(&dsCaps, 0, 0x60);
    dsCaps.dwSize = 0x60;
    g_pDirectSound_005c4ed0->GetCaps(&dsCaps);
    if ((g_dwSoundOptions_005c4ed8 & 0x10) == 0 &&
        (dsCaps.dwFlags &
         (DSCAPS_PRIMARY16BIT | DSCAPS_PRIMARYSTEREO)) !=
        (DSCAPS_PRIMARY16BIT | DSCAPS_PRIMARYSTEREO)) {
        ix_log_printf("Warning [%s - %d]:\n", IX_MIXER_FILE, 60);
        ix_log_printf("sound driver doesn't support our primary buffer format, using secondary buffer");
        g_dwSoundOptions_005c4ed8 |= 0x10;
        DirectSoundEnumerateA(ix_dsp_open_driver, 0);
        if (g_pDirectSound_005c4ed0 == 0) {
            ix_log_printf("Fatal [%s - %d]:\n", IX_MIXER_FILE, 63);
            ix_log_printf("Failed to init directsound");
            exit(-1);
        }
    }

    memset(&format, 0, 0x10);
    format.wf.wFormatTag = WAVE_FORMAT_PCM;
    format.wf.nChannels = 2;
    format.wf.nSamplesPerSec = 22050;
    format.wf.nBlockAlign = 4;
    format.wf.nAvgBytesPerSec = 88200;
    format.wBitsPerSample = 16;

    memset(&description, 0, 0x14);
    description.dwSize = 0x14;
    description.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
    result = g_pDirectSound_005c4ed0->CreateSoundBuffer(
        &description, &g_pPrimarySoundBuffer_005c52ec, 0);
    if (result != DS_OK) {
        ix_log_printf("Fatal [%s - %d]:\n", IX_MIXER_FILE, 81);
        ix_log_printf("Failed to create primary buffer");
        exit(-1);
    }

    result = g_pPrimarySoundBuffer_005c52ec->SetFormat(
        (LPWAVEFORMATEX)&format);
    if (result != DS_OK) {
        ix_log_printf("Warning [%s - %d]:\n", IX_MIXER_FILE, 84);
        ix_log_printf("Failed to SetFormat, %s",
                      ix_dsp_result_to_text(result));
    }
    if ((g_dwSoundOptions_005c4ed8 & 0x10) != 0) {
        memset(&description, 0, 0x14);
        description.dwSize = 0x14;
        description.dwFlags = DSBCAPS_CTRLVOLUME;
        description.dwBufferBytes = 0x8000;
        description.lpwfxFormat = (LPWAVEFORMATEX)&format;
        result = g_pDirectSound_005c4ed0->CreateSoundBuffer(
            &description, &g_pMixerBuffer_005c4ee0, 0);
        if (result != DS_OK) {
            ix_log_printf("Warning [%s - %d]:\n", IX_MIXER_FILE, 96);
            ix_log_printf("Failed to CreateSoundBuffer, %s",
                          ix_dsp_result_to_text(result));
            return (DWORD)-1;
        }
        g_dwMixerScratchSize_005c4ed4 = 0x8000;
        ix_log_printf("secondary buffer, size = %d\n",
                      g_dwMixerScratchSize_005c4ed4);
    } else {
        memset(&bufCaps, 0, 0x14);
        bufCaps.dwSize = 0x14;
        g_pPrimarySoundBuffer_005c52ec->GetCaps(&bufCaps);
        g_pMixerBuffer_005c4ee0 = g_pPrimarySoundBuffer_005c52ec;
        g_dwMixerScratchSize_005c4ed4 = bufCaps.dwBufferBytes;
        ix_log_printf("primary buffer, size = %d\n",
                      g_dwMixerScratchSize_005c4ed4);
    }

    g_pPrimarySoundBuffer_005c52ec->SetVolume(
        10000 - (g_nMasterVolume_004a0c14 * 10000) / 0xffff);
    while ((g_dwSoundOptions_005c4ed8 & 4) != 0) {
        ix_mixer_service();
        if ((g_dwSoundOptions_005c4ed8 & 4) != 0) {
            WaitForSingleObject(g_hMixerRouseEvent_005c4ec8, 1000);
            ResetEvent(g_hMixerRouseEvent_005c4ec8);
            g_pMixerBuffer_005c4ee0->Restore();
        }
    }

    if (g_pMixerBuffer_005c4ee0 == g_pPrimarySoundBuffer_005c52ec) {
        g_pPrimarySoundBuffer_005c52ec = 0;
    } else if (g_pPrimarySoundBuffer_005c52ec != 0) {
        g_pPrimarySoundBuffer_005c52ec->Release();
        g_pPrimarySoundBuffer_005c52ec = 0;
    }
    if (g_pMixerBuffer_005c4ee0 != 0) {
        g_pMixerBuffer_005c4ee0->Release();
        g_pMixerBuffer_005c4ee0 = 0;
    }
    if (g_pDirectSound_005c4ed0 != 0) {
        g_pDirectSound_005c4ed0->Release();
        g_pDirectSound_005c4ed0 = 0;
    }
    return 0;
#endif
}

/* Function start: 0x48BC5C */   /* source line(s) 150;157;161;176;186;196: Failed to Lock, %s | Failed to Unlock, %s | Failed to Play, %s | Failed to get current pos */
int ix_mixer_service(void)
{
#ifdef SDL_PORT
    return 0;
#else
    DWORD delay;
    DWORD initialBytes;
    HRESULT hr;
    void *initialBuffer;

    delay = 0x42;
    hr = g_pMixerBuffer_005c4ee0->Lock(
        0, 0x2df0, &initialBuffer, &initialBytes, 0, 0, 0);
    if (hr != DS_OK) {
        ix_log_printf("Warning [%s - %d]:\n", IX_MIXER_FILE, 150);
        ix_log_printf("Failed to Lock, %s", ix_dsp_result_to_text(hr));
        return -1;
    }
    EnterCriticalSection(&g_csMixer_005c5730);
    ix_dspv_mix(initialBuffer, initialBytes);
    LeaveCriticalSection(&g_csMixer_005c5730);
    hr = g_pMixerBuffer_005c4ee0->Unlock(initialBuffer, initialBytes, 0, 0);
    if (hr != DS_OK) {
        ix_log_printf("Warning [%s - %d]:\n", IX_MIXER_FILE, 157);
        ix_log_printf("Failed to Unlock, %s", ix_dsp_result_to_text(hr));
        return -1;
    }
    hr = g_pMixerBuffer_005c4ee0->Play(0, 0, DSBPLAY_LOOPING);
    if (hr != DS_OK) {
        ix_log_printf("Warning [%s - %d]:\n", IX_MIXER_FILE, 161);
        ix_log_printf("Failed to Play, %s", ix_dsp_result_to_text(hr));
        return -1;
    }

    g_dwMixWriteOffset_005c4ecc = 0x2df0;
    while ((g_dwSoundOptions_005c4ed8 & 4) != 0) {
        DWORD writePosition;
        HRESULT hr;
        DWORD bufferHeadLen;
        int bufferedBytes;
        void *bufferTail;
        DWORD lockBytes;
        void *bufferHead;
        DWORD playCursor;
        DWORD bufferTailLen;

        Sleep(delay);
        g_dwDsoundTick_005c52e8++;
        hr = g_pMixerBuffer_005c4ee0->GetCurrentPosition(
            &playCursor, &writePosition);
        if (hr != DS_OK) {
            ix_log_printf("Warning [%s - %d]:\n", IX_MIXER_FILE, 176);
            ix_log_printf("Failed to get current position, %s",
                          ix_dsp_result_to_text(hr));
            return -1;
        }
        if (playCursor > g_dwMixWriteOffset_005c4ecc)
            playCursor -= g_dwMixerScratchSize_005c4ed4;
        bufferedBytes = g_dwMixWriteOffset_005c4ecc - playCursor;
        delay = (unsigned int)(bufferedBytes * 0x42) / 0x16f8;

        lockBytes = 0x16f8;
        hr = g_pMixerBuffer_005c4ee0->Lock(
            g_dwMixWriteOffset_005c4ecc, lockBytes,
            &bufferHead, &bufferHeadLen, &bufferTail, &bufferTailLen, 0);
        if (hr != DS_OK) {
            ix_log_printf("Warning [%s - %d]:\n", IX_MIXER_FILE, 186);
            ix_log_printf("Failed to Lock, %s", ix_dsp_result_to_text(hr));
            return -1;
        }
        EnterCriticalSection(&g_csMixer_005c5730);
        ix_dspv_mix(bufferHead, bufferHeadLen);
        if (bufferTail != 0)
            ix_dspv_mix(bufferTail, bufferTailLen);
        LeaveCriticalSection(&g_csMixer_005c5730);
        hr = g_pMixerBuffer_005c4ee0->Unlock(
            bufferHead, bufferHeadLen, bufferTail, bufferTailLen);
        if (hr != DS_OK) {
            ix_log_printf("Warning [%s - %d]:\n", IX_MIXER_FILE, 196);
            ix_log_printf("Failed to Unlock, %s", ix_dsp_result_to_text(hr));
            return -1;
        }
        g_dwMixWriteOffset_005c4ecc += bufferHeadLen;
        if (bufferTail != 0) {
            g_dwMixWriteOffset_005c4ecc = bufferTailLen;
        } else {
            if (g_dwMixWriteOffset_005c4ecc >= g_dwMixerScratchSize_005c4ed4)
                g_dwMixWriteOffset_005c4ecc = 0;
        }
    }
    return 0;
#endif
}
