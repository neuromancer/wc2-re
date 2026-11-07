/*
 *  DirectDraw DIB display, frame timing and palettes.
 *
 *  Retail object 0x45CCB0-0x45E9EF (link order; see docs/ORDER.md).
 *
 *  A C++ unit: it opens with the frame-timing state, whose initializers are
 *  not constant expressions, so the compiler emits two dynamic initializer
 *  pairs (0x45CCB0-0x45CD2B) ahead of the first function.  Its DirectDraw
 *  calls keep the C interface (CINTERFACE), which compiles to the same calls
 *  as the C++ one.
 */
#define CINTERFACE
#define DIB_C
#include "game.h"

static int g_nDefaultFrameRate = 70; /* 0x0049ce88 */

static int g_nDisplayFrameRate = 70; /* 0x0049ce8c */

long g_17_nFramePeriodMillis =
    (long)(1000.0 / g_nDisplayFrameRate); /* 0x005c343c */

float g_1_fPreviousFrameSpeed =
    (float)g_nDisplayFrameRate; /* 0x005c3440 */

#ifndef SDL_PORT
/* The DirectDraw GUIDs, instantiated here in the order of DirectX 3's
 * ddraw.h (as INITGUID would).  The compiler's SDK ships the DirectX 2
 * header, which lacks CLSID_DirectDrawClipper, so they are spelled out. */
/* 0x0048e0f0 */
const GUID CLSID_DirectDraw = {
    0xd7b70ee0, 0x4340, 0x11cf,
    { 0xb0, 0x63, 0x00, 0x20, 0xaf, 0xc2, 0xcd, 0x35 }
};
/* 0x0048e100 */
const GUID CLSID_DirectDrawClipper = {
    0x593817a0, 0x7db3, 0x11cf,
    { 0xa2, 0xde, 0x00, 0xaa, 0x00, 0xb9, 0x33, 0x56 }
};
/* 0x0048e110 */
const GUID IID_IDirectDraw = {
    0x6c14db80, 0xa733, 0x11ce,
    { 0xa5, 0x21, 0x00, 0x20, 0xaf, 0x0b, 0xe5, 0x60 }
};
/* 0x0048e120 */
const GUID IID_IDirectDraw2 = {
    0xb3a6f3e0, 0x2b43, 0x11cf,
    { 0xa2, 0xde, 0x00, 0xaa, 0x00, 0xb9, 0x33, 0x56 }
};
/* 0x0048e130 */
const GUID IID_IDirectDrawSurface = {
    0x6c14db81, 0xa733, 0x11ce,
    { 0xa5, 0x21, 0x00, 0x20, 0xaf, 0x0b, 0xe5, 0x60 }
};
/* 0x0048e140 */
const GUID IID_IDirectDrawSurface2 = {
    0x57805885, 0x6eec, 0x11cf,
    { 0x94, 0x41, 0xa8, 0x23, 0x03, 0xc1, 0x0e, 0x27 }
};
/* 0x0048e150 */
const GUID IID_IDirectDrawPalette = {
    0x6c14db84, 0xa733, 0x11ce,
    { 0xa5, 0x21, 0x00, 0x20, 0xaf, 0x0b, 0xe5, 0x60 }
};
/* 0x0048e160 */
const GUID IID_IDirectDrawClipper = {
    0x6c14db85, 0xa733, 0x11ce,
    { 0xa5, 0x21, 0x00, 0x20, 0xaf, 0x0b, 0xe5, 0x60 }
};
#endif

LPDIRECTDRAW2 g_pDirectDraw2 = 0; /* 0x0049ce90 */

LPDIRECTDRAWSURFACE g_pPrimarySurface = 0; /* 0x0049ce94 */

LPDIRECTDRAWSURFACE g_pSecondarySurface = 0; /* 0x0049ce98 */

LPDIRECTDRAWPALETTE g_pDirectDrawPalette = 0; /* 0x0049ce9c */

int g_nDisplayModeCascade = -1; /* 0x0049cea0 */

DWORD g_dwNextFrameDeadline = 0; /* 0x0049cea4 */

int g_nUnusedDibWord1 = 0; /* 0x0049cea8 */

long g_lPreviousFrameTick = 0; /* 0x0049ceac */

int g_nFrameTimingMode = 0; /* 0x0049ceb0 */

float g_fSpaceFlightFrameRate = 20.0f; /* 0x0049ceb4 */

int g_nUnusedDibWord2 = 0; /* 0x0049ceb8 */

int g_bFrameTimingInitialized = 0; /* 0x0049cebc */

int g_bDibSlamEnabled = 1; /* 0x0049cec0 */

/* This unit's .bss: LINK orders C++ definitions by the identifiers' hash
 * keys, and these spellings follow the original addresses. */
char g_59_szFrameFrequencyString[32]; /* 0x005b3950 */
unsigned int g_38_nDibOriginX; /* 0x005b3970 */
unsigned int g_101_nDibOffsetY; /* 0x005b3974 */
unsigned char *g_18_pDisplayPixelBuffer; /* 0x005b3978 */
unsigned int g_28_nDisplayRowBytes; /* 0x005b397c */
unsigned int g_38_nSurfaceHeight; /* 0x005b3980 */
unsigned char g_11_abFrameBackingStore[0xfa00]; /* 0x005b3988 */
int g_36_nDibBitsEachPx; /* 0x005c3388 */
int g_4_nDibShowCount; /* 0x005c3390 */
HWND g_2_hFrameHwnd; /* 0x005c33a4 */
int g_nDibDirtyHitTotal; /* 0x005c33a8 */
char g_szFrameErrorMessage[0x8c]; /* 0x005c33b0 */
unsigned char g_3_abColourShadow[1024]; /* 0x005c3450 */
float g_fMeasuredFrameRate; /* 0x005c3850 */
char g_2_szDibDebugScratch[260]; /* 0x005c3858 */ /* MAX_PATH */
int g_4_bScreenStale; /* 0x005c395c */
int g_8_nDibFlipRealCount; /* 0x005c3960 */
unsigned int g_10_dwLastSurfaceSlamRealNoticeTick; /* 0x005c396c */
char g_18_szSpaceFlightBestFpsMessage[0x80]; /* 0x005c3970 */
unsigned int g_dwLastDibDirtyReportTime; /* 0x005c39f0 */
char g_3_szDibSlamRealDebugBuf[256]; /* 0x005c39f8 */
/* Never referenced; retail reserves their storage among the words above. */
int g_28_nUnusedFrameSlot; /* 0x005c338c */
int g_nUnusedScreenWord1; /* 0x005c3394 */
int g_nUnusedScreenWord2; /* 0x005c3398 */
int g_4_nUnusedFrameWord; /* 0x005c339c */
int g_5_nUnusedDibWord3; /* 0x005c33a0 */
int g_nUnusedDibWord4; /* 0x005c3444 */
int g_nUnusedDibWord5; /* 0x005c3448 */
int g_2_nReservedDibWord; /* 0x005c344c */
int g_12_nIdleFrameWord1; /* 0x005c3964 */
int g_11_nIdleFrameWord2; /* 0x005c3968 */

#pragma function(strlen)
/* Function start: 0x45CD2C */
void ThrottleFrameAndDrawFps(HDC dc)
{
    if (g_bFrameTimingInitialized == 0)
        SetDefaultFrameTiming();

    if (g_bShowFrameRate != 0) {
        sprintf(g_59_szFrameFrequencyString, "%f",
                g_fMeasuredFrameRate);
        TextOutA(dc, 0, 0, g_59_szFrameFrequencyString,
                 strlen(g_59_szFrameFrequencyString));
    }

#ifdef SDL_PORT
    while (timeGetTime() < g_dwNextFrameDeadline) {
        /* Match DIBslamReal's menu cursor eligibility, including screens
         * composed in the secondary buffer. Poll host events during the
         * wait without running game input or consuming queued clicks. */
        if (SdlCanDrawMenuCursor()) {
            SdlPumpEvents();
            RefreshMouseCursorDisplay();
        }
        Sleep(1);
    }
#else
    if (g_16_stEventQueuePointerData.viewport->pixels == g_18_pDisplayPixelBuffer) {
        while (timeGetTime() < g_dwNextFrameDeadline) {
            Sleep(0);
            RefreshMouseCursorDisplay();
        }
    } else {
        while (timeGetTime() < g_dwNextFrameDeadline)
            Sleep(0);
    }
#endif

    if (g_bShowFrameRate != 0) {
        if (g_lPreviousFrameTick != 0) {
            g_fMeasuredFrameRate =
                1000.0f / ((float)timeGetTime() -
                           g_lPreviousFrameTick);
        }
        g_lPreviousFrameTick = timeGetTime();
        sprintf(g_59_szFrameFrequencyString, "%f",
                g_fMeasuredFrameRate);
        TextOutA(dc, 0, 0, g_59_szFrameFrequencyString,
                 strlen(g_59_szFrameFrequencyString));
    }

    g_dwNextFrameDeadline =
        timeGetTime() + g_17_nFramePeriodMillis;
}

void SetHudMessageText(char *text, unsigned short colour, unsigned short duration);
void DIBerror(const char *tag, int hr);

/* Function start: 0x45CE9A */
void ReportSpaceFlightMaxFps(float adjustment)
{
    g_fSpaceFlightFrameRate += adjustment;
    if (g_fSpaceFlightFrameRate < 8.0)
        g_fSpaceFlightFrameRate = 8.0f;
    else if (g_fSpaceFlightFrameRate > 32.0)
        g_fSpaceFlightFrameRate = 32.0f;
    sprintf(g_18_szSpaceFlightBestFpsMessage,
            "Space Flight Max FPS : %.1f",
            (double)g_fSpaceFlightFrameRate);
    SetHudMessageText(g_18_szSpaceFlightBestFpsMessage, 0x50, 0x14);
    if (g_nFrameTimingMode == 1)
        SetSpaceFlightFrameTiming();
}

/* Function start: 0x45CF3B */
void SetSpaceFlightFrameTiming(void)
{
    g_nFrameTimingMode = 1;
    g_17_nFramePeriodMillis =
        (long)(1000.0 / g_fSpaceFlightFrameRate);
    g_dwNextFrameDeadline = 0;
    g_bFrameTimingInitialized = 1;
}

/* Function start: 0x45CF7F */
void SetDefaultFrameTiming(void)
{
    g_nFrameTimingMode = 0;
    g_17_nFramePeriodMillis =
        (long)(1000.0 / g_1_fPreviousFrameSpeed);
    g_dwNextFrameDeadline = 0;
    g_bFrameTimingInitialized = 1;
}

/* Function start: 0x45CFC3 */
void SetCinematicFrameTiming(float frameRate)
{
    g_nFrameTimingMode = 0;
    g_17_nFramePeriodMillis = (long)(1000.0 / frameRate);
    g_dwNextFrameDeadline = 0;
    g_bFrameTimingInitialized = 1;
}

/* Function start: 0x45D004 */
void DIBerror(const char *tag, int hr)
{
#ifdef SDL_PORT
    const char *text = SDL_GetError();

    sprintf(g_szFrameErrorMessage, "ERROR: %s - (%s)", tag, text);
    SdlShutdownVideo();
    OutputDebugStringA(g_szFrameErrorMessage);
    if (g_2_hFrameHwnd != 0)
        SDL_SetWindowSize((SDL_Window *)g_2_hFrameHwnd, 320, 200);
    MessageBoxA(0, g_szFrameErrorMessage, "SDL Video Error", MB_ICONERROR);
    exit(1);
#else
    FILE *errorFile;

    sprintf(g_szFrameErrorMessage, "ERROR: %i in %s",
            (unsigned short)hr, tag);
    COM_RELEASE(g_pSecondarySurface);
    COM_RELEASE(g_pPrimarySurface);
    IDirectDraw2_RestoreDisplayMode(g_pDirectDraw2);
    IDirectDraw2_Release(g_pDirectDraw2);
    WriteDebugString(g_szFrameErrorMessage);
    SetWindowPos(g_2_hFrameHwnd, HWND_BOTTOM, 0, 0, 320, 200,
                 SWP_SHOWWINDOW);
    errorFile = fopen("direct.err", "wt+");
    fprintf(errorFile, g_szFrameErrorMessage);
    fclose(errorFile);
    MessageBoxA(0, g_szFrameErrorMessage, "DirectDraw Error", MB_ICONERROR);
    exit(1);
    return;
#endif
}

/* Function start: 0x45D11F */
void DIBpositionWindow(void)
{
#ifdef SDL_PORT
    SdlShutdownVideo();
    if (g_2_hFrameHwnd != 0)
        SDL_SetWindowSize((SDL_Window *)g_2_hFrameHwnd, 320, 200);
#else
    COM_RELEASE(g_pSecondarySurface);
    COM_RELEASE(g_pPrimarySurface);
    IDirectDraw2_RestoreDisplayMode(g_pDirectDraw2);
    IDirectDraw2_Release(g_pDirectDraw2);
    SetWindowPos(g_2_hFrameHwnd, HWND_BOTTOM, 0, 0, 320, 200,
                 SWP_SHOWWINDOW);
#endif
}

/* Function start: 0x45D1BB */
void DIBreInstall(void)
{
#ifndef SDL_PORT
    int err;

    if (g_bConfigQuickModeEnabled != 0) {
        if (g_pDirectDraw2 == 0)
            return;
        IDirectDraw2_SetCooperativeLevel(
            g_pDirectDraw2, g_2_hFrameHwnd, 0x13);
        if (DIBcascade(-2, &err) == 0)
            DIBerror("DIBreInstall   DIBcascade Failure", err);
    } else {
        g_nDisplayModeCascade = -1;
    }
#else
    if (!SdlInitializeVideo((SDL_Window *)g_2_hFrameHwnd))
        DIBerror("DIBreInstall", -1);
    MarkDibDirty();
    DIBslamReal();
#endif
}

#pragma function(memset)
/* Function start: 0x45D23B */
void DIBinstall(HWND window)
{
#ifndef SDL_PORT
    LPDIRECTDRAW directDraw;
    HRESULT result;
#endif

    g_2_hFrameHwnd = window;
#ifdef SDL_PORT
    if (!SdlInitializeVideo((SDL_Window *)window))
        DIBerror("DIBinstall", -1);
    g_nDisplayModeCascade = 0;
#else
    result = DirectDrawCreate(0, &directDraw, 0);
    if (result != DD_OK)
        DIBerror("DIBInstall   Create", result);

    result = IDirectDraw_QueryInterface(
        directDraw, IID_IDirectDraw2,
        (void **)&g_pDirectDraw2);
    if (result != DD_OK)
        DIBerror("DIBInstall   Unable to acquire DirectDraw2 interface", result);

    if (g_bConfigQuickModeEnabled != 0)
        result = IDirectDraw2_SetCooperativeLevel(
            g_pDirectDraw2, g_2_hFrameHwnd, 0x13);
    else
        result = IDirectDraw2_SetCooperativeLevel(
            g_pDirectDraw2, g_2_hFrameHwnd, 0x17);
    if (result != DD_OK)
        DIBerror("DIBmakeInstall   SetCooperativeLevel", result);

    if (g_bConfigQuickModeEnabled != 0) {
        if (DIBcascade(-1, 0) == 0)
            DIBerror("DIBinstall   DIBcascade Failure", result);
    } else {
        g_nDisplayModeCascade = -1;
    }
#endif

    g_2_hFrameHwnd = window;
    memset(g_3_abColourShadow, 0, sizeof(g_3_abColourShadow));
    memset(g_11_abFrameBackingStore, 0, sizeof(g_11_abFrameBackingStore));
    DIBmakeDIB();
    g_4_bScreenStale = 1;
    g_4_nDibShowCount = 0;
    return;
}

/* Function start: 0x45D3A1 */
int DIBcascade(int mode, int *reportedResult)
{
#ifdef SDL_PORT
    g_nDisplayModeCascade = 0;
    if (reportedResult != 0)
        *reportedResult = 0;
    return mode < 3;
#else
    DDSURFACEDESC surface;
    HRESULT result;

    if (g_pDirectDraw2 == 0)
        return 1;

    if (mode != -2) {
        if (mode == -1)
            g_nDisplayModeCascade = 0;
        else
            g_nDisplayModeCascade++;
    }

    switch (g_nDisplayModeCascade) {
    case 0:
        result = g_pDirectDraw2->lpVtbl->SetDisplayMode(
            g_pDirectDraw2, 320, 200, 8, 0, 0);
        if (reportedResult != 0)
            *reportedResult = result;
        if (result != DD_OK)
            return DIBcascade(0, 0);
        WriteDebugString("320x200 achieved...testing\n");
        break;
    case 1:
        result = g_pDirectDraw2->lpVtbl->SetDisplayMode(
            g_pDirectDraw2, 640, 400, 8, 0, 0);
        if (reportedResult != 0)
            *reportedResult = result;
        if (result != DD_OK)
            return DIBcascade(0, 0);
        WriteDebugString("640x400 achieved...testing\n");
        break;
    case 2:
        result = g_pDirectDraw2->lpVtbl->SetDisplayMode(
            g_pDirectDraw2, 640, 480, 8, 0, 0);
        if (reportedResult != 0)
            *reportedResult = result;
        if (result != DD_OK)
            return DIBcascade(0, 0);
        WriteDebugString("640x480 achieved...testing\n");
        break;
    default:
        return 0;
    }

    if (mode == -2) {
        if (g_pSecondarySurface != 0) {
            result = IDirectDrawSurface_Restore(
                g_pSecondarySurface);
            if (reportedResult != 0)
                *reportedResult = result;
            if (result != DD_OK)
                DIBerror("DIBcascade   Unable to restore surface (secondary)",
                         result);
        }

        result = IDirectDrawSurface_Restore(g_pPrimarySurface);
        if (reportedResult != 0)
            *reportedResult = result;
        if (result != DD_OK)
            DIBerror("DIBcascade   Unable to restore surface", result);

        result = IDirectDrawSurface_SetPalette(
            g_pPrimarySurface, g_pDirectDrawPalette);
        if (result != DD_OK)
            DIBerror("DIBcascade   CreatePalette", result);
        return 1;
    }

    WriteDebugString(" acquiring surface:");
    memset(&surface, 0, sizeof(surface));
    surface.dwSize = sizeof(surface);
    surface.dwFlags = DDSD_CAPS;
    surface.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    result = IDirectDraw2_CreateSurface(
        g_pDirectDraw2, &surface,
        &g_pPrimarySurface, 0);
    if (reportedResult != 0)
        *reportedResult = result;
    if (result != DD_OK) {
        WriteDebugString(" failed\n");
        IDirectDraw2_RestoreDisplayMode(g_pDirectDraw2);
        return DIBcascade(0, 0);
    }

    WriteDebugString(" successful\n locking surface:");
    result = IDirectDrawSurface_Lock(
        g_pPrimarySurface, 0, &surface, DDLOCK_WAIT, 0);
    if (reportedResult != 0)
        *reportedResult = result;
    if (result != DD_OK) {
        WriteDebugString(" failed\n");
        IDirectDrawSurface_Release(g_pPrimarySurface);
        IDirectDraw2_RestoreDisplayMode(g_pDirectDraw2);
        return DIBcascade(0, 0);
    }

    IDirectDrawSurface_Unlock(g_pPrimarySurface,
                              surface.lpSurface);
    IDirectDrawSurface_Release(g_pPrimarySurface);
    g_pPrimarySurface = 0;
    WriteDebugString(" successful\n");
    return 1;
#endif
}

/* Function start: 0x45D78C */
/* Full teardown: destroy the DIB, release its surfaces, restore the display
 * mode and release DirectDraw itself. */
void DIBunInstall(void)
{
    DIBdestroyDIB();
#ifdef SDL_PORT
    SdlShutdownVideo();
    g_2_hFrameHwnd = 0;
#else
    COM_RELEASE(g_pSecondarySurface);
    COM_RELEASE(g_pPrimarySurface);
    IDirectDraw2_RestoreDisplayMode(g_pDirectDraw2);
    IDirectDraw2_Release(g_pDirectDraw2);
#endif
}

#pragma function(memcpy)
/* Function start: 0x45D80F */
void DIBmakeDIB(void)
{
#ifndef SDL_PORT
    DDSURFACEDESC surfaceDesc;
    PALETTEENTRY colours[256];
    HRESULT hr;
    int paletteIndex;

    memset(&surfaceDesc, 0, sizeof(surfaceDesc));
    surfaceDesc.dwSize = sizeof(surfaceDesc);
    surfaceDesc.dwFlags = DDSD_CAPS;
    surfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_MODEX;
    hr = IDirectDraw2_CreateSurface(
        g_pDirectDraw2, &surfaceDesc,
        &g_pPrimarySurface, 0);
    if (hr != DD_OK)
        DIBerror("DIBmakeDIB   CreateSurface (primary)", hr);
#endif

    g_36_nDibBitsEachPx = 8;
#ifndef SDL_PORT
    for (paletteIndex = 0; paletteIndex < 256; paletteIndex++) {
        colours[paletteIndex].peRed = g_3_abColourShadow[paletteIndex * 4 + 2];
        colours[paletteIndex].peGreen = g_3_abColourShadow[paletteIndex * 4 + 1];
        colours[paletteIndex].peBlue = g_3_abColourShadow[paletteIndex * 4];
        colours[paletteIndex].peFlags = 0;
    }

    if (g_36_nDibBitsEachPx == 8) {
        hr = IDirectDraw2_CreatePalette(
            g_pDirectDraw2, DDPCAPS_8BIT, colours,
            &g_pDirectDrawPalette, 0);
        if (hr != DD_OK)
            DIBerror("DIBmakeDIB   CreatePalette", hr);

        if (g_bUseHardwarePalette != 0) {
            hr = IDirectDrawSurface_SetPalette(
                g_pPrimarySurface,
                g_pDirectDrawPalette);
            if (hr != DD_OK)
                DIBerror("DIBmakeDIB   CreatePalette", hr);
        }
    }

    if (g_nDisplayModeCascade > 0) {
        memset(&surfaceDesc, 0, sizeof(surfaceDesc));
        surfaceDesc.dwSize = sizeof(surfaceDesc);
        surfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
        surfaceDesc.dwWidth = 320;
        surfaceDesc.dwHeight = 200;
        surfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
        hr = IDirectDraw2_CreateSurface(
            g_pDirectDraw2, &surfaceDesc,
            &g_pSecondarySurface, 0);
        if (hr != DD_OK)
            DIBerror("DIBmakeDIB   CreateSurface (secondary)", hr);
    }
#endif

    g_28_nDisplayRowBytes = 320;
    g_38_nSurfaceHeight = 200;
    g_101_nDibOffsetY = 0;
    g_38_nDibOriginX = 0;
    g_18_pDisplayPixelBuffer = (unsigned char *)malloc(64000);
#ifdef SDL_PORT
    if (g_18_pDisplayPixelBuffer == 0)
        DIBerror("DIBmakeDIB", -1);
#endif
    g_101_stScrViewInfo.allocation = GetDIBPixelBuffer();
    g_101_stScrViewInfo.pixels = g_101_stScrViewInfo.allocation;
    memcpy(g_18_pDisplayPixelBuffer, g_11_abFrameBackingStore,
           g_28_nDisplayRowBytes * g_38_nSurfaceHeight);
    return;
}

/* Function start: 0x45DA8C */
void DIBdestroyDIB(void)
{
#ifndef SDL_PORT
    int result;
#endif

#ifdef SDL_PORT
    if (g_18_pDisplayPixelBuffer != 0) {
        memcpy(g_11_abFrameBackingStore, g_18_pDisplayPixelBuffer,
               g_28_nDisplayRowBytes * g_38_nSurfaceHeight);
    }
#else
    memcpy(g_11_abFrameBackingStore, g_18_pDisplayPixelBuffer,
           g_28_nDisplayRowBytes * g_38_nSurfaceHeight);
#endif
#ifndef SDL_PORT
    if (g_nDisplayModeCascade > 0) {
        result = IDirectDrawSurface_Release(g_pSecondarySurface);
        if (result != 0)
            DIBerror("DIBdestroyDIB   secondary->Release", result);
        g_pSecondarySurface = 0;
    }
    if (g_pPrimarySurface != 0) {
        result = IDirectDrawSurface_Release(g_pPrimarySurface);
        if (result != 0)
            DIBerror("DIBdestroyDIB   primary->Release", result);
        g_pPrimarySurface = 0;
    }
#endif
    if (g_18_pDisplayPixelBuffer != 0) {
        free(g_18_pDisplayPixelBuffer);
        g_18_pDisplayPixelBuffer = 0;
    }
    g_38_nDibOriginX = 0;
    g_101_nDibOffsetY = 0;
    g_18_pDisplayPixelBuffer = 0;
#ifdef SDL_PORT
    g_101_stScrViewInfo.pixels = 0;
    g_101_stScrViewInfo.allocation = 0;
#endif
    return;
}

/* Function start: 0x45DB8C */
void MarkDibDirty(void)
{
    unsigned int tick;

    tick = GetTickCount();
    g_4_bScreenStale = 1;
    g_nDibDirtyHitTotal++;
    if (g_dwLastDibDirtyReportTime + 1000 <= tick) {
        sprintf(g_2_szDibDebugScratch,
                "DIBslam %d times in %d seconds\n",
                g_nDibDirtyHitTotal,
                tick - g_dwLastDibDirtyReportTime);
        g_nDibDirtyHitTotal = 0;
        g_dwLastDibDirtyReportTime = tick;
    }
}

/* Function start: 0x45DBFF */
void EnableDibSlam(void)
{
    g_bDibSlamEnabled = 1;
}

/* Function start: 0x45DC19 */
void DisableDibSlam(void)
{
    g_bDibSlamEnabled = 0;
}

/* Function start: 0x45DC33 */
void DIBslamReal(void)
{
#ifndef SDL_PORT
    DDSURFACEDESC surfaceDetails;
    unsigned char *destPixels;
    unsigned char *source;
    RECT outputRect;
    RECT sourceArea;
    HRESULT status;
    int scanline;
    HDC drawDC;
#endif
    unsigned int nowTick;

    UpdateStreamerStoppedFlag();
    nowTick = GetTickCount();
    if (g_bDibSlamEnabled == 0)
        return;

    g_8_nDibFlipRealCount++;
    if (g_10_dwLastSurfaceSlamRealNoticeTick + 1000 <= nowTick) {
        sprintf(g_3_szDibSlamRealDebugBuf,
                "DIBslamReal %d times in %d seconds\n",
                g_8_nDibFlipRealCount,
                nowTick - g_10_dwLastSurfaceSlamRealNoticeTick);
        g_8_nDibFlipRealCount = 0;
        g_10_dwLastSurfaceSlamRealNoticeTick = nowTick;
    }

    if (g_4_bScreenStale != 0) {
#ifndef SDL_PORT
        memset(&surfaceDetails, 0, sizeof(surfaceDetails));
        surfaceDetails.dwSize = sizeof(surfaceDetails);
#endif

#ifdef SDL_PORT
        /* Flight directs input to the offscreen space-view buffer while
         * retaining the menu cursor shape; every other screen -- including
         * menus that compose into the secondary view buffer, such as the
         * campaign chalkboard, barracks, and personnel screens -- directs
         * input to something that does end up on the presented screen.
         * Stamp the arrow only when input isn't targeting the flight
         * buffer specifically. */
        if (SdlCanDrawMenuCursor()) {
            /* A menu can present several times per game frame. Refresh the
             * host position for each presentation, even if vsync leaves no
             * time for the frame limiter's idle cursor redraws. */
            SdlPumpEvents();
            CaptureMouseCursorBackground();
            DrawMouseCursor();
        }
#else
        CaptureMouseCursorBackground();
        DrawMouseCursor();
#endif

#ifndef SDL_PORT
        if (g_nDisplayModeCascade > 0) {
            status = IDirectDrawSurface_Lock(
                g_pSecondarySurface,
                0, &surfaceDetails, DDLOCK_WAIT, 0);
        } else {
            status = IDirectDrawSurface_Lock(
                g_pPrimarySurface,
                0, &surfaceDetails, DDLOCK_WAIT, 0);
        }
        if (status != DD_OK) {
            if (status == DDERR_SURFACELOST) {
                if (g_nDisplayModeCascade > 0) {
                    status = IDirectDrawSurface_Restore(
                        g_pSecondarySurface);
                    if (status != DD_OK)
                        return;
                }
                status = IDirectDrawSurface_Restore(
                    g_pPrimarySurface);
                if (status != DD_OK)
                    return;

                if (g_nDisplayModeCascade > 0) {
                    status = IDirectDrawSurface_Lock(
                        g_pSecondarySurface,
                        0, &surfaceDetails, DDLOCK_WAIT, 0);
                } else {
                    status = IDirectDrawSurface_Lock(
                        g_pPrimarySurface,
                        0, &surfaceDetails, DDLOCK_WAIT, 0);
                }
                if (status != DD_OK)
                    return;
            } else {
                IDirectDrawSurface_Unlock(
                    g_pPrimarySurface, surfaceDetails.lpSurface);
                DIBerror("DIBslamReal   secondary->Lock", status);
            }
        }

        destPixels = (unsigned char *)surfaceDetails.lpSurface;
        source = g_18_pDisplayPixelBuffer;
        if (g_nDisplayModeCascade <= 0) {
            IDirectDraw2_WaitForVerticalBlank(
                g_pDirectDraw2, DDWAITVB_BLOCKBEGIN, 0);
        }
        for (scanline = 0; scanline < (int)g_38_nSurfaceHeight; scanline++) {
            memcpy(destPixels, source, g_28_nDisplayRowBytes);
            destPixels += surfaceDetails.lPitch;
            source += g_28_nDisplayRowBytes;
        }

        if (g_nDisplayModeCascade > 0) {
            status = IDirectDrawSurface_Unlock(
                g_pSecondarySurface, surfaceDetails.lpSurface);
        } else {
            status = IDirectDrawSurface_Unlock(
                g_pPrimarySurface, surfaceDetails.lpSurface);
        }
#else
        if (!SdlPresentIndexedFrame(g_18_pDisplayPixelBuffer, g_3_abColourShadow))
            DIBerror("DIBslamReal", -1);
#endif

#ifdef SDL_PORT
        if (g_16_stEventQueuePointerData.viewport != 0 &&
            g_16_stEventQueuePointerData.viewport->pixels == g_18_pDisplayPixelBuffer) {
            RestoreMouseCursorBackground();
        }
#else
        RestoreMouseCursorBackground();
#endif

#ifndef SDL_PORT
        if (g_nDisplayModeCascade > 0) {
            outputRect.left = 0;
            outputRect.top = 0;
            outputRect.right = 639;
            outputRect.bottom = 399;
            sourceArea.left = 0;
            sourceArea.top = 0;
            sourceArea.right = 319;
            sourceArea.bottom = 199;
            if (g_nDisplayModeCascade == 2) {
                outputRect.top += 40;
                outputRect.bottom += 40;
            }

            status = IDirectDrawSurface_Blt(
                g_pPrimarySurface, &outputRect,
                g_pSecondarySurface, &sourceArea,
                DDBLT_WAIT, 0);
            if (status != DD_OK) {
                if (status == DDERR_SURFACELOST) {
                    status = IDirectDrawSurface_Restore(
                        g_pPrimarySurface);
                    if (status != DD_OK)
                        return;
                    status = IDirectDrawSurface_Restore(
                        g_pSecondarySurface);
                    if (status != DD_OK)
                        return;
                    status = IDirectDrawSurface_Blt(
                        g_pPrimarySurface, &outputRect,
                        g_pSecondarySurface, &sourceArea,
                        DDBLT_WAIT, 0);
                    if (status != DD_OK)
                        return;
                } else {
                    DIBerror("DIBslamReal   primary->Blt", status);
                }
            }
        }
#endif
        g_4_bScreenStale = 0;
    }

    g_4_nDibShowCount++;
    ServiceSoundSystem();
#ifdef SDL_PORT
    ThrottleFrameAndDrawFps(0);
#else
    drawDC = GetDC(g_2_hFrameHwnd);
    ThrottleFrameAndDrawFps(drawDC);
    ReleaseDC(g_2_hFrameHwnd, drawDC);
#endif
}

/* Function start: 0x45E060 */
void DIBupdate(int left, int top, int right, int bottom)
{
#ifndef SDL_PORT
    unsigned char *destPixels;
    unsigned char *source;
    DDSURFACEDESC surfaceDetails;
    int byteWidth;
    RECT targetRect;
    RECT updateSource;
    int currentLine;
    int height;
    HRESULT hResult;
#endif

    if (g_nDisplayModeCascade > 0)
        return;
    if (left < 0)
        left = 0;
    else if (left > 319)
        return;
    if (right < 0)
        return;
    else if (right > 319)
        right = 319;
    if (top < 0)
        top = 0;
    else if (top > 199)
        return;
    if (bottom < 0)
        return;
    else if (bottom > 199)
        bottom = 199;

#ifdef SDL_PORT
    if (right < left || bottom < top)
        return;
    if (!SdlPresentIndexedFrame(g_18_pDisplayPixelBuffer, g_3_abColourShadow))
        DIBerror("DIBupdate", -1);
#else
    byteWidth = right - left + 1;
    height = bottom - top + 1;
    /* Retail initializes both rectangles even though Lock does not use them. */
    targetRect.left = left;
    targetRect.top = top;
    targetRect.right = byteWidth;
    targetRect.bottom = height;
    updateSource.left = left;
    updateSource.top = top;
    updateSource.right = byteWidth;
    updateSource.bottom = height;
    memset(&surfaceDetails, 0, sizeof(surfaceDetails));
    surfaceDetails.dwSize = sizeof(surfaceDetails);
    hResult = IDirectDrawSurface_Lock(
        g_pPrimarySurface, 0, &surfaceDetails,
        DDLOCK_WAIT, 0);
    if (hResult != DD_OK) {
        IDirectDrawSurface_Unlock(
            g_pPrimarySurface, surfaceDetails.lpSurface);
        DIBerror("DIBupdate   primary->Lock", hResult);
    }

    destPixels = (unsigned char *)surfaceDetails.lpSurface
                + surfaceDetails.lPitch * top + left;
    /* Convert the completed row address before applying the column offset. */
    source = (unsigned char *)((unsigned long)g_18_pDisplayPixelBuffer
                              + g_28_nDisplayRowBytes * top) + left;
    IDirectDraw2_WaitForVerticalBlank(
        g_pDirectDraw2, DDWAITVB_BLOCKBEGIN, 0);
    for (currentLine = 0; height > currentLine; currentLine++) {
        memcpy(destPixels, source, byteWidth);
        destPixels += surfaceDetails.lPitch;
        source += g_28_nDisplayRowBytes;
    }
    hResult = IDirectDrawSurface_Unlock(
        g_pPrimarySurface, surfaceDetails.lpSurface);
#endif
    return;
}

/* Function start: 0x45E276 */
unsigned char *GetDIBPixelBuffer(void)
{
    return g_18_pDisplayPixelBuffer;
}

/* Function start: 0x45E28B */
/* Nothing in the shipped executable calls this. */
void DIBpixelBufferHook(void)
{
    return;
}

/* Function start: 0x45E29B */
/* Nothing in the shipped executable calls this. */
HWND GetDIBWindowHandle(void)
{
    return g_2_hFrameHwnd;
}

/* Function start: 0x45E2B0 */
/* Nothing in the shipped executable calls this. */
unsigned int GetDIBWidth(void)
{
    return g_28_nDisplayRowBytes;
}

/* Function start: 0x45E2C5 */
unsigned int GetDIBHeight(void)
{
    return g_38_nSurfaceHeight;
}

/* Function start: 0x45E2DA */
void CachePaletteEntryFromWords(short index, unsigned short *rgb)
{
    g_3_abColourShadow[index * 4 + 2] =
        *(unsigned char *)&rgb[0];
    g_ausPaletteWords[index][0] =
        g_3_abColourShadow[index * 4 + 2];
    g_3_abColourShadow[index * 4 + 1] =
        *(unsigned char *)&rgb[1];
    g_ausPaletteWords[index][1] =
        g_3_abColourShadow[index * 4 + 1];
    g_3_abColourShadow[index * 4] =
        *(unsigned char *)&rgb[2];
    g_ausPaletteWords[index][2] =
        g_3_abColourShadow[index * 4];
    g_3_abColourShadow[index * 4 + 3] = 1;
}

/* Function start: 0x45E37C */
void DIBramPalette(void)
{
#ifdef SDL_PORT
    if (g_18_pDisplayPixelBuffer != 0 &&
        !SdlPresentIndexedFrame(g_18_pDisplayPixelBuffer, g_3_abColourShadow))
        DIBerror("DIBramPalette", -1);
#else
    PALETTEENTRY entries[256];
    int index;
    HRESULT error;

    for (index = 0; index < 256; index++) {
        entries[index].peRed =
            g_3_abColourShadow[index * 4 + 2];
        entries[index].peGreen =
            g_3_abColourShadow[index * 4 + 1];
        entries[index].peBlue =
            g_3_abColourShadow[index * 4];
        entries[index].peFlags = 0;
    }
    if (g_bUseHardwarePalette != 0) {
        error = IDirectDrawPalette_SetEntries(
            g_pDirectDrawPalette, 0, 0, 256, entries);
        if (error != DD_OK)
            DIBerror("DIBramPalette   SetEntries", error);
    }
#endif
}

/* Function start: 0x45E46C */
void DIBsetPalette(short index, short *rgb)
{
#ifndef SDL_PORT
    PALETTEENTRY entry;
    HRESULT error;
#endif

    if ((int)g_3_abColourShadow[index * 4 + 2] == (int)rgb[0] &&
        (int)g_3_abColourShadow[index * 4 + 1] == (int)rgb[1] &&
        (int)g_3_abColourShadow[index * 4] == (int)rgb[2])
        return;
    g_3_abColourShadow[index * 4 + 2] =
        *(unsigned char *)&rgb[0];
    g_ausPaletteWords[index][0] =
        g_3_abColourShadow[index * 4 + 2];
    g_3_abColourShadow[index * 4 + 1] =
        *(unsigned char *)&rgb[1];
    g_ausPaletteWords[index][1] =
        g_3_abColourShadow[index * 4 + 1];
    g_3_abColourShadow[index * 4] =
        *(unsigned char *)&rgb[2];
    g_ausPaletteWords[index][2] =
        g_3_abColourShadow[index * 4];
    g_3_abColourShadow[index * 4 + 3] = 1;

    /* SDL_PORT consumes this cache on the next normal frame submission.
       A DirectDraw palette entry update did not blit or wait for vertical
       blank, so flight fades must not submit additional SDL frames. */
#ifndef SDL_PORT
    entry.peRed = (unsigned char)g_ausPaletteWords[index][0];
    entry.peGreen = (unsigned char)g_ausPaletteWords[index][1];
    entry.peBlue = (unsigned char)g_ausPaletteWords[index][2];
    entry.peFlags = 0;
    if (g_bUseHardwarePalette != 0) {
        error = IDirectDrawPalette_SetEntries(
            g_pDirectDrawPalette, 0, (int)index, 1,
            &entry);
        if (error != DD_OK)
            DIBerror("DIBsetPalette   SetEntries", error);

        error = IDirectDrawSurface_SetPalette(
            g_pPrimarySurface,
            g_pDirectDrawPalette);
        if (error != DD_OK)
            DIBerror("DIBmakeDIB   CreatePalette", error);
    }
#endif
}

/* Function start: 0x45E61C */
void GetPaletteEntryAsWords(short i, unsigned short *rgb)
{
    rgb[0] = g_3_abColourShadow[i * 4 + 2];
    rgb[1] = g_3_abColourShadow[i * 4 + 1];
    rgb[2] = g_3_abColourShadow[i * 4];
}

/* Function start: 0x45E667 */
void DIBwholePaletteFromTriplets(unsigned char *palette)
{
#ifndef SDL_PORT
    unsigned char entries[0x400];
    int hr;
#endif
    int index = 0x100;

#ifndef SDL_PORT
    IDirectDraw2_WaitForVerticalBlank(
        g_pDirectDraw2, DDWAITVB_BLOCKBEGIN, 0);
#else
    SdlWaitForVerticalBlank();
#endif
    for (index = 0; index < 0x100; index++) {
        g_3_abColourShadow[index * 4 + 2] =
            palette[index * 3];
#ifndef SDL_PORT
        entries[index * 4] =
            g_3_abColourShadow[index * 4 + 2];
#endif
        g_3_abColourShadow[index * 4 + 1] =
            palette[index * 3 + 1];
#ifndef SDL_PORT
        entries[index * 4 + 1] =
            g_3_abColourShadow[index * 4 + 1];
#endif
        g_3_abColourShadow[index * 4] =
            palette[index * 3 + 2];
#ifndef SDL_PORT
        entries[index * 4 + 2] =
            g_3_abColourShadow[index * 4];
        entries[index * 4 + 3] = 0;
#endif
        g_3_abColourShadow[index * 4 + 3] = 4;
    }

#ifdef SDL_PORT
    DIBramPalette();
#else
    if (g_bUseHardwarePalette != 0) {
        hr = IDirectDrawPalette_SetEntries(
            g_pDirectDrawPalette, 0, 0, 0x100,
            (LPPALETTEENTRY)entries);
        if (hr != 0)
            DIBerror("DIBsetWholePalette   SetEntries", hr);

        hr = IDirectDrawSurface_SetPalette(
            g_pPrimarySurface, g_pDirectDrawPalette);
        if (hr != 0)
            DIBerror("DIBmakeDIB   CreatePalette", hr);
    }
#endif
}

/* Function start: 0x45E816 */
void DIBwholePaletteFromWords(unsigned short *palette)
{
#ifndef SDL_PORT
    unsigned char entries[0x400];
    int hr;
#endif
    int index = 0x100;

#ifndef SDL_PORT
    IDirectDraw2_WaitForVerticalBlank(
        g_pDirectDraw2, DDWAITVB_BLOCKBEGIN, 0);
#else
    SdlWaitForVerticalBlank();
#endif
    for (index = 0; index < 0x100; index++) {
        g_3_abColourShadow[index * 4 + 2] =
            *(unsigned char *)&palette[index * 3];
#ifndef SDL_PORT
        entries[index * 4] =
            g_3_abColourShadow[index * 4 + 2];
#endif
        g_3_abColourShadow[index * 4 + 1] =
            *(unsigned char *)&palette[index * 3 + 1];
#ifndef SDL_PORT
        entries[index * 4 + 1] =
            g_3_abColourShadow[index * 4 + 1];
#endif
        g_3_abColourShadow[index * 4] =
            *(unsigned char *)&palette[index * 3 + 2];
#ifndef SDL_PORT
        entries[index * 4 + 2] =
            g_3_abColourShadow[index * 4];
        entries[index * 4 + 3] = 0;
#endif
        g_3_abColourShadow[index * 4 + 3] = 4;
    }

#ifdef SDL_PORT
    DIBramPalette();
#else
    if (g_bUseHardwarePalette != 0) {
        hr = IDirectDrawPalette_SetEntries(
            g_pDirectDrawPalette, 0, 0, 0x100,
            (LPPALETTEENTRY)entries);
        if (hr != 0)
            DIBerror("DIBsetWholePalette   SetEntries", hr);

        hr = IDirectDrawSurface_SetPalette(
            g_pPrimarySurface, g_pDirectDrawPalette);
        if (hr != 0)
            DIBerror("DIBmakeDIB   CreatePalette", hr);
    }
#endif
}

/* Function start: 0x45E9C5 */
void DIBwaitForVerticalBlank(void)
{
#ifdef SDL_PORT
    SdlWaitForVerticalBlank();
#else
    IDirectDraw2_WaitForVerticalBlank(
        g_pDirectDraw2, DDWAITVB_BLOCKBEGIN, 0);
#endif
}
