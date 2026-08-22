/*
 *  DirectDraw back end.
 *
 *  Address range 0x432000-0x43390f (see docs/ORDER.md).
 *  Boundary evidence: PROVEN: the named DirectDraw routines are followed by
 *  DirectDrawResultToText and its compiler-generated switch tables.
 */
#include "game.h"

#pragma function(memcpy, memset)

const GUID g_guidDirectDraw2_0048e120 = {
    0xb3a6f3e0, 0x2b43, 0x11cf,
    { 0xa2, 0xde, 0x00, 0xaa, 0x00, 0xb9, 0x33, 0x56 }
};
LPDIRECTDRAW2 g_pDirectDraw2_0049ce90;
LPDIRECTDRAWSURFACE g_pPrimarySurface_0049ce94;
LPDIRECTDRAWSURFACE g_pSecondarySurface_0049ce98;
LPDIRECTDRAWPALETTE g_pDirectDrawPalette_0049ce9c;
int g_nDisplayModeCascade_0049cea0 = -1;
DWORD g_dwNextFrameDeadline_0049cea4;
long g_lPreviousFrameTick_0049ceac;
int g_nFrameTimingMode_0049ceb0;
float g_fSpaceFlightFrameRate_0049ceb4 = 20.0f;
int g_bFrameTimingInitialized_0049cebc;
int g_bDibSlamEnabled_0049cec0 = 1;
char g_szFrameRateText_005b3950[32];
unsigned int DAT_005b3970;
unsigned int DAT_005b3974;
unsigned char *g_pDibPixelBuffer_005b3978;
unsigned int g_nDibRowBytes_005b397c;
unsigned int g_nDibHeight_005b3980;
unsigned char g_abDibBackingStore_005b3988[0xfa00];
int g_nDibBitsPerPixel_005c3388;
int g_nDibPresentCount_005c3390;
HWND g_hDibWindow_005c33a4;
int g_nDibDirtyMarkCount_005c33a8;
char g_szDibErrorMessage_005c33b0[0x8c];
unsigned char g_abPaletteCache_005c3450[1024];
float g_fMeasuredFrameRate_005c3850;
char g_szDibDebugBuffer_005c3858[256];
int g_bDibDirty_005c395c;
int g_nDibSlamRealCount_005c3960;
unsigned int g_dwLastDibSlamRealReportTick_005c396c;
unsigned int g_dwLastDibDirtyReportTick_005c39f0;
char g_szDibSlamRealDebugBuffer_005c39f8[256];

/* Function start: 0x45CE9A */
void ReportSpaceFlightMaxFps(float adjustment)
{
    g_fSpaceFlightFrameRate_0049ceb4 += adjustment;
    if (g_fSpaceFlightFrameRate_0049ceb4 < 8.0)
        g_fSpaceFlightFrameRate_0049ceb4 = 8.0f;
    else if (g_fSpaceFlightFrameRate_0049ceb4 > 32.0)
        g_fSpaceFlightFrameRate_0049ceb4 = 32.0f;
    sprintf(g_szSpaceFlightMaxFpsMessage_005c3970,
            "Space Flight Max FPS : %.1f",
            (double)g_fSpaceFlightFrameRate_0049ceb4);
    SetHudMessageText(g_szSpaceFlightMaxFpsMessage_005c3970, 0x50, 0x14);
    if (g_nFrameTimingMode_0049ceb0 == 1)
        SetSpaceFlightFrameTiming();
}

/* Function start: 0x45CF3B */
void SetSpaceFlightFrameTiming(void)
{
    g_nFrameTimingMode_0049ceb0 = 1;
    g_nFramePeriodMilliseconds_005c343c =
        (long)(1000.0 / g_fSpaceFlightFrameRate_0049ceb4);
    g_dwNextFrameDeadline_0049cea4 = 0;
    g_bFrameTimingInitialized_0049cebc = 1;
}

/* Function start: 0x45CF7F */
void SetDefaultFrameTiming(void)
{
    g_nFrameTimingMode_0049ceb0 = 0;
    g_nFramePeriodMilliseconds_005c343c =
        (long)(1000.0 / g_fPreviousFrameRate_005c3440);
    g_dwNextFrameDeadline_0049cea4 = 0;
    g_bFrameTimingInitialized_0049cebc = 1;
}

/* Function start: 0x45CFC3 */
void SetCinematicFrameTiming(float frameRate)
{
    g_nFrameTimingMode_0049ceb0 = 0;
    g_nFramePeriodMilliseconds_005c343c = (long)(1000.0 / frameRate);
    g_dwNextFrameDeadline_0049cea4 = 0;
    g_bFrameTimingInitialized_0049cebc = 1;
}

/* Function start: 0x45D004 */
void DIBerror(const char *tag, int hr)
{
#ifdef SDL_PORT
    const char *text = SDL_GetError();

    sprintf(g_szDibErrorMessage_005c33b0, "ERROR: %s - (%s)", tag, text);
    SdlShutdownVideo();
    OutputDebugStringA(g_szDibErrorMessage_005c33b0);
    if (g_hDibWindow_005c33a4 != 0)
        SDL_SetWindowSize((SDL_Window *)g_hDibWindow_005c33a4, 320, 200);
    MessageBoxA(0, g_szDibErrorMessage_005c33b0, "SDL Video Error", MB_ICONERROR);
    exit(1);
#else
    FILE *errorFile;

    sprintf(g_szDibErrorMessage_005c33b0, "ERROR: %i in %s",
            (unsigned short)hr, tag);
    COM_RELEASE(g_pSecondarySurface_0049ce98);
    COM_RELEASE(g_pPrimarySurface_0049ce94);
    IDirectDraw2_RestoreDisplayMode(g_pDirectDraw2_0049ce90);
    IDirectDraw2_Release(g_pDirectDraw2_0049ce90);
    WriteDebugString(g_szDibErrorMessage_005c33b0);
    SetWindowPos(g_hDibWindow_005c33a4, HWND_BOTTOM, 0, 0, 320, 200,
                 SWP_SHOWWINDOW);
    errorFile = fopen("direct.err", "wt+");
    fprintf(errorFile, g_szDibErrorMessage_005c33b0);
    fclose(errorFile);
    MessageBoxA(0, g_szDibErrorMessage_005c33b0, "DirectDraw Error", MB_ICONERROR);
    exit(1);
    return;
#endif
}

/* Function start: 0x45D11F */
void DIBpositionWindow(void)
{
#ifdef SDL_PORT
    SdlShutdownVideo();
    if (g_hDibWindow_005c33a4 != 0)
        SDL_SetWindowSize((SDL_Window *)g_hDibWindow_005c33a4, 320, 200);
#else
    COM_RELEASE(g_pSecondarySurface_0049ce98);
    COM_RELEASE(g_pPrimarySurface_0049ce94);
    IDirectDraw2_RestoreDisplayMode(g_pDirectDraw2_0049ce90);
    IDirectDraw2_Release(g_pDirectDraw2_0049ce90);
    SetWindowPos(g_hDibWindow_005c33a4, HWND_BOTTOM, 0, 0, 320, 200,
                 SWP_SHOWWINDOW);
#endif
}

/* Function start: 0x45D1BB */
void DIBreInstall(void)
{
#ifndef SDL_PORT
    int err;

    if (g_bConfigQuickModeEnabled_0049c264 != 0) {
        if (g_pDirectDraw2_0049ce90 == 0)
            return;
        IDirectDraw2_SetCooperativeLevel(
            g_pDirectDraw2_0049ce90, g_hDibWindow_005c33a4, 0x13);
        if (DIBcascade(-2, &err) == 0)
            DIBerror("DIBreInstall   DIBcascade Failure", err);
    } else {
        g_nDisplayModeCascade_0049cea0 = -1;
    }
#else
    if (!SdlInitializeVideo((SDL_Window *)g_hDibWindow_005c33a4))
        DIBerror("DIBreInstall", -1);
    MarkDibDirty();
    DIBslamReal();
#endif
}

/* Function start: 0x45D23B */
void DIBinstall(HWND window)
{
#ifndef SDL_PORT
    LPDIRECTDRAW directDraw;
    HRESULT result;
#endif

    g_hDibWindow_005c33a4 = window;
#ifdef SDL_PORT
    if (!SdlInitializeVideo((SDL_Window *)window))
        DIBerror("DIBinstall", -1);
    g_nDisplayModeCascade_0049cea0 = 0;
#else
    result = DirectDrawCreate(0, &directDraw, 0);
    if (result != DD_OK)
        DIBerror("DIBInstall   Create", result);

    result = IDirectDraw_QueryInterface(
        directDraw, &g_guidDirectDraw2_0048e120,
        (void **)&g_pDirectDraw2_0049ce90);
    if (result != DD_OK)
        DIBerror("DIBInstall   Unable to acquire DirectDraw2 interface", result);

    if (g_bConfigQuickModeEnabled_0049c264 != 0)
        result = IDirectDraw2_SetCooperativeLevel(
            g_pDirectDraw2_0049ce90, g_hDibWindow_005c33a4, 0x13);
    else
        result = IDirectDraw2_SetCooperativeLevel(
            g_pDirectDraw2_0049ce90, g_hDibWindow_005c33a4, 0x17);
    if (result != DD_OK)
        DIBerror("DIBmakeInstall   SetCooperativeLevel", result);

    if (g_bConfigQuickModeEnabled_0049c264 != 0) {
        if (DIBcascade(-1, 0) == 0)
            DIBerror("DIBinstall   DIBcascade Failure", result);
    } else {
        g_nDisplayModeCascade_0049cea0 = -1;
    }
#endif

    g_hDibWindow_005c33a4 = window;
    memset(g_abPaletteCache_005c3450, 0, sizeof(g_abPaletteCache_005c3450));
    memset(g_abDibBackingStore_005b3988, 0, sizeof(g_abDibBackingStore_005b3988));
    DIBmakeDIB();
    g_bDibDirty_005c395c = 1;
    g_nDibPresentCount_005c3390 = 0;
    return;
}

/* Function start: 0x45D3A1 */
int DIBcascade(int mode, int *reportedResult)
{
#ifdef SDL_PORT
    g_nDisplayModeCascade_0049cea0 = 0;
    if (reportedResult != 0)
        *reportedResult = 0;
    return mode < 3;
#else
    DDSURFACEDESC surface;
    HRESULT result;

    if (g_pDirectDraw2_0049ce90 == 0)
        return 1;

    if (mode != -2) {
        if (mode == -1)
            g_nDisplayModeCascade_0049cea0 = 0;
        else
            g_nDisplayModeCascade_0049cea0++;
    }

    switch (g_nDisplayModeCascade_0049cea0) {
    case 0:
        result = g_pDirectDraw2_0049ce90->lpVtbl->SetDisplayMode(
            g_pDirectDraw2_0049ce90, 320, 200, 8, 0, 0);
        if (reportedResult != 0)
            *reportedResult = result;
        if (result != DD_OK)
            return DIBcascade(0, 0);
        WriteDebugString("320x200 achieved...testing\n");
        break;
    case 1:
        result = g_pDirectDraw2_0049ce90->lpVtbl->SetDisplayMode(
            g_pDirectDraw2_0049ce90, 640, 400, 8, 0, 0);
        if (reportedResult != 0)
            *reportedResult = result;
        if (result != DD_OK)
            return DIBcascade(0, 0);
        WriteDebugString("640x400 achieved...testing\n");
        break;
    case 2:
        result = g_pDirectDraw2_0049ce90->lpVtbl->SetDisplayMode(
            g_pDirectDraw2_0049ce90, 640, 480, 8, 0, 0);
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
        if (g_pSecondarySurface_0049ce98 != 0) {
            result = IDirectDrawSurface_Restore(
                g_pSecondarySurface_0049ce98);
            if (reportedResult != 0)
                *reportedResult = result;
            if (result != DD_OK)
                DIBerror("DIBcascade   Unable to restore surface (secondary)",
                         result);
        }

        result = IDirectDrawSurface_Restore(g_pPrimarySurface_0049ce94);
        if (reportedResult != 0)
            *reportedResult = result;
        if (result != DD_OK)
            DIBerror("DIBcascade   Unable to restore surface", result);

        result = IDirectDrawSurface_SetPalette(
            g_pPrimarySurface_0049ce94, g_pDirectDrawPalette_0049ce9c);
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
        g_pDirectDraw2_0049ce90, &surface,
        &g_pPrimarySurface_0049ce94, 0);
    if (reportedResult != 0)
        *reportedResult = result;
    if (result != DD_OK) {
        WriteDebugString(" failed\n");
        IDirectDraw2_RestoreDisplayMode(g_pDirectDraw2_0049ce90);
        return DIBcascade(0, 0);
    }

    WriteDebugString(" successful\n locking surface:");
    result = IDirectDrawSurface_Lock(
        g_pPrimarySurface_0049ce94, 0, &surface, DDLOCK_WAIT, 0);
    if (reportedResult != 0)
        *reportedResult = result;
    if (result != DD_OK) {
        WriteDebugString(" failed\n");
        IDirectDrawSurface_Release(g_pPrimarySurface_0049ce94);
        IDirectDraw2_RestoreDisplayMode(g_pDirectDraw2_0049ce90);
        return DIBcascade(0, 0);
    }

    IDirectDrawSurface_Unlock(g_pPrimarySurface_0049ce94,
                              surface.lpSurface);
    IDirectDrawSurface_Release(g_pPrimarySurface_0049ce94);
    g_pPrimarySurface_0049ce94 = 0;
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
    g_hDibWindow_005c33a4 = 0;
#else
    COM_RELEASE(g_pSecondarySurface_0049ce98);
    COM_RELEASE(g_pPrimarySurface_0049ce94);
    IDirectDraw2_RestoreDisplayMode(g_pDirectDraw2_0049ce90);
    IDirectDraw2_Release(g_pDirectDraw2_0049ce90);
#endif
}

/* Function start: 0x45D80F */
void DIBmakeDIB(void)
{
#ifndef SDL_PORT
    DDSURFACEDESC surface;
    PALETTEENTRY entries[256];
    HRESULT result;
    int entry;

    memset(&surface, 0, sizeof(surface));
    surface.dwSize = sizeof(surface);
    surface.dwFlags = DDSD_CAPS;
    surface.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_MODEX;
    result = IDirectDraw2_CreateSurface(
        g_pDirectDraw2_0049ce90, &surface,
        &g_pPrimarySurface_0049ce94, 0);
    if (result != DD_OK)
        DIBerror("DIBmakeDIB   CreateSurface (primary)", result);
#endif

    g_nDibBitsPerPixel_005c3388 = 8;
#ifndef SDL_PORT
    for (entry = 0; entry < 256; entry++) {
        entries[entry].peRed = g_abPaletteCache_005c3450[entry * 4 + 2];
        entries[entry].peGreen = g_abPaletteCache_005c3450[entry * 4 + 1];
        entries[entry].peBlue = g_abPaletteCache_005c3450[entry * 4];
        entries[entry].peFlags = 0;
    }

    if (g_nDibBitsPerPixel_005c3388 == 8) {
        result = IDirectDraw2_CreatePalette(
            g_pDirectDraw2_0049ce90, DDPCAPS_8BIT, entries,
            &g_pDirectDrawPalette_0049ce9c, 0);
        if (result != DD_OK)
            DIBerror("DIBmakeDIB   CreatePalette", result);

        if (g_bUseHardwarePalette_0049c268 != 0) {
            result = IDirectDrawSurface_SetPalette(
                g_pPrimarySurface_0049ce94,
                g_pDirectDrawPalette_0049ce9c);
            if (result != DD_OK)
                DIBerror("DIBmakeDIB   CreatePalette", result);
        }
    }

    if (g_nDisplayModeCascade_0049cea0 > 0) {
        memset(&surface, 0, sizeof(surface));
        surface.dwSize = sizeof(surface);
        surface.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
        surface.dwWidth = 320;
        surface.dwHeight = 200;
        surface.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
        result = IDirectDraw2_CreateSurface(
            g_pDirectDraw2_0049ce90, &surface,
            &g_pSecondarySurface_0049ce98, 0);
        if (result != DD_OK)
            DIBerror("DIBmakeDIB   CreateSurface (secondary)", result);
    }
#endif

    g_nDibRowBytes_005b397c = 320;
    g_nDibHeight_005b3980 = 200;
    DAT_005b3974 = 0;
    DAT_005b3970 = 0;
    g_pDibPixelBuffer_005b3978 = malloc(64000);
#ifdef SDL_PORT
    if (g_pDibPixelBuffer_005b3978 == 0)
        DIBerror("DIBmakeDIB", -1);
#endif
    g_stScreenViewport_005d21a0.allocation = GetDIBPixelBuffer();
    g_stScreenViewport_005d21a0.pixels = g_stScreenViewport_005d21a0.allocation;
    memcpy(g_pDibPixelBuffer_005b3978, g_abDibBackingStore_005b3988,
           g_nDibRowBytes_005b397c * g_nDibHeight_005b3980);
    return;
}

/* Function start: 0x45DA8C */
void DIBdestroyDIB(void)
{
#ifndef SDL_PORT
    int result;
#endif

#ifdef SDL_PORT
    if (g_pDibPixelBuffer_005b3978 != 0) {
        memcpy(g_abDibBackingStore_005b3988, g_pDibPixelBuffer_005b3978,
               g_nDibRowBytes_005b397c * g_nDibHeight_005b3980);
    }
#else
    memcpy(g_abDibBackingStore_005b3988, g_pDibPixelBuffer_005b3978,
           g_nDibRowBytes_005b397c * g_nDibHeight_005b3980);
#endif
#ifndef SDL_PORT
    if (g_nDisplayModeCascade_0049cea0 > 0) {
        result = IDirectDrawSurface_Release(g_pSecondarySurface_0049ce98);
        if (result != 0)
            DIBerror("DIBdestroyDIB   secondary->Release", result);
        g_pSecondarySurface_0049ce98 = 0;
    }
    if (g_pPrimarySurface_0049ce94 != 0) {
        result = IDirectDrawSurface_Release(g_pPrimarySurface_0049ce94);
        if (result != 0)
            DIBerror("DIBdestroyDIB   primary->Release", result);
        g_pPrimarySurface_0049ce94 = 0;
    }
#endif
    if (g_pDibPixelBuffer_005b3978 != 0) {
        free(g_pDibPixelBuffer_005b3978);
        g_pDibPixelBuffer_005b3978 = 0;
    }
    DAT_005b3970 = 0;
    DAT_005b3974 = 0;
    g_pDibPixelBuffer_005b3978 = 0;
#ifdef SDL_PORT
    g_stScreenViewport_005d21a0.pixels = 0;
    g_stScreenViewport_005d21a0.allocation = 0;
#endif
    return;
}

/* Function start: 0x45DB8C */
void MarkDibDirty(void)
{
    unsigned int tick;

    tick = GetTickCount();
    g_bDibDirty_005c395c = 1;
    g_nDibDirtyMarkCount_005c33a8++;
    if (g_dwLastDibDirtyReportTick_005c39f0 + 1000 <= tick) {
        sprintf(g_szDibDebugBuffer_005c3858,
                "DIBslam %d times in %d seconds",
                g_nDibDirtyMarkCount_005c33a8,
                tick - g_dwLastDibDirtyReportTick_005c39f0);
        g_nDibDirtyMarkCount_005c33a8 = 0;
        g_dwLastDibDirtyReportTick_005c39f0 = tick;
    }
}

/* Function start: 0x45DBFF */
void EnableDibSlam(void)
{
    g_bDibSlamEnabled_0049cec0 = 1;
}

/* Function start: 0x45DC19 */
void DisableDibSlam(void)
{
    g_bDibSlamEnabled_0049cec0 = 0;
}

/* Function start: 0x45DC33 */
void DIBslamReal(void)
{
#ifndef SDL_PORT
    DDSURFACEDESC surface;
    unsigned char *destination;
    unsigned char *source;
    RECT destinationRect;
    RECT sourceRect;
    HRESULT result;
    int row;
    HDC dc;
#endif
    unsigned int tick;

    UpdateStreamerStoppedFlag();
    tick = GetTickCount();
    if (g_bDibSlamEnabled_0049cec0 == 0)
        return;

    g_nDibSlamRealCount_005c3960++;
    if (g_dwLastDibSlamRealReportTick_005c396c + 1000 <= tick) {
        sprintf(g_szDibSlamRealDebugBuffer_005c39f8,
                "DIBslamReal %d times in %d seconds",
                g_nDibSlamRealCount_005c3960,
                tick - g_dwLastDibSlamRealReportTick_005c396c);
        g_nDibSlamRealCount_005c3960 = 0;
        g_dwLastDibSlamRealReportTick_005c396c = tick;
    }

    if (g_bDibDirty_005c395c != 0) {
#ifndef SDL_PORT
        memset(&surface, 0, sizeof(surface));
        surface.dwSize = sizeof(surface);
#endif

#ifdef SDL_PORT
        /* Only stamp the software cursor into the buffer actually being
         * presented.  The WC1 cursor-state struct this used to test is filled
         * in by InitializeEventManagerResources, which the WC2 path never
         * reaches; EMStartUp is what loads the shape and points the input
         * viewport at the screen, and DrawMouseCursor draws into the screen
         * viewport, so those are what decide it here. */
        if (g_pInputCursorShape_005c83f9 != 0 &&
            g_pInputViewport_005c8403 != 0 &&
            g_stScreenViewport_005d21a0.pixels == g_pDibPixelBuffer_005b3978) {
            CaptureMouseCursorBackground();
            DrawMouseCursor();
        }
#else
        CaptureMouseCursorBackground();
        DrawMouseCursor();
#endif

#ifndef SDL_PORT
        if (g_nDisplayModeCascade_0049cea0 > 0) {
            result = IDirectDrawSurface_Lock(
                g_pSecondarySurface_0049ce98,
                0, &surface, DDLOCK_WAIT, 0);
        } else {
            result = IDirectDrawSurface_Lock(
                g_pPrimarySurface_0049ce94,
                0, &surface, DDLOCK_WAIT, 0);
        }
        if (result != DD_OK) {
            if (result == DDERR_SURFACELOST) {
                if (g_nDisplayModeCascade_0049cea0 > 0) {
                    result = IDirectDrawSurface_Restore(
                        g_pSecondarySurface_0049ce98);
                    if (result != DD_OK)
                        return;
                }
                result = IDirectDrawSurface_Restore(
                    g_pPrimarySurface_0049ce94);
                if (result != DD_OK)
                    return;

                if (g_nDisplayModeCascade_0049cea0 > 0) {
                    result = IDirectDrawSurface_Lock(
                        g_pSecondarySurface_0049ce98,
                        0, &surface, DDLOCK_WAIT, 0);
                } else {
                    result = IDirectDrawSurface_Lock(
                        g_pPrimarySurface_0049ce94,
                        0, &surface, DDLOCK_WAIT, 0);
                }
                if (result != DD_OK)
                    return;
            } else {
                IDirectDrawSurface_Unlock(
                    g_pPrimarySurface_0049ce94, surface.lpSurface);
                DIBerror("DIBslamReal   secondary->Lock", result);
            }
        }

        destination = surface.lpSurface;
        source = g_pDibPixelBuffer_005b3978;
        if (g_nDisplayModeCascade_0049cea0 <= 0) {
            IDirectDraw2_WaitForVerticalBlank(
                g_pDirectDraw2_0049ce90, DDWAITVB_BLOCKBEGIN, 0);
        }
        for (row = 0; row < (int)g_nDibHeight_005b3980; row++) {
            memcpy(destination, source, g_nDibRowBytes_005b397c);
            destination += surface.lPitch;
            source += g_nDibRowBytes_005b397c;
        }

        if (g_nDisplayModeCascade_0049cea0 > 0) {
            IDirectDrawSurface_Unlock(
                g_pSecondarySurface_0049ce98, surface.lpSurface);
        } else {
            IDirectDrawSurface_Unlock(
                g_pPrimarySurface_0049ce94, surface.lpSurface);
        }
#else
        if (!SdlPresentIndexedFrame(g_pDibPixelBuffer_005b3978, g_abPaletteCache_005c3450))
            DIBerror("DIBslamReal", -1);
#endif

#ifdef SDL_PORT
        if (g_stMouseCursorState_0059ab10.viewport != 0 &&
            g_stMouseCursorState_0059ab10.viewport->pixels == g_pDibPixelBuffer_005b3978) {
            RestoreMouseCursorBackground();
        }
#else
        RestoreMouseCursorBackground();
#endif

#ifndef SDL_PORT
        if (g_nDisplayModeCascade_0049cea0 > 0) {
            destinationRect.left = 0;
            destinationRect.top = 0;
            destinationRect.right = 639;
            destinationRect.bottom = 399;
            sourceRect.left = 0;
            sourceRect.top = 0;
            sourceRect.right = 319;
            sourceRect.bottom = 199;
            if (g_nDisplayModeCascade_0049cea0 == 2) {
                destinationRect.top += 40;
                destinationRect.bottom += 40;
            }

            result = IDirectDrawSurface_Blt(
                g_pPrimarySurface_0049ce94, &destinationRect,
                g_pSecondarySurface_0049ce98, &sourceRect,
                DDBLT_WAIT, 0);
            if (result != DD_OK) {
                if (result == DDERR_SURFACELOST) {
                    result = IDirectDrawSurface_Restore(
                        g_pPrimarySurface_0049ce94);
                    if (result != DD_OK)
                        return;
                    result = IDirectDrawSurface_Restore(
                        g_pSecondarySurface_0049ce98);
                    if (result != DD_OK)
                        return;
                    result = IDirectDrawSurface_Blt(
                        g_pPrimarySurface_0049ce94, &destinationRect,
                        g_pSecondarySurface_0049ce98, &sourceRect,
                        DDBLT_WAIT, 0);
                    if (result != DD_OK)
                        return;
                } else {
                    DIBerror("DIBslamReal   primary->Blt", result);
                }
            }
        }
#endif
        g_bDibDirty_005c395c = 0;
    }

    g_nDibPresentCount_005c3390++;
    ServiceSoundSystem();
#ifdef SDL_PORT
    ThrottleFrameAndDrawFps(0);
#else
    dc = GetDC(g_hDibWindow_005c33a4);
    ThrottleFrameAndDrawFps(dc);
    ReleaseDC(g_hDibWindow_005c33a4, dc);
#endif
}

/* Function start: 0x45E060 */
void DIBupdate(int left, int top, int right, int bottom)
{
#ifndef SDL_PORT
    unsigned char *destination;
    unsigned char *source;
    DDSURFACEDESC surface;
    int width;
    int height;
    HRESULT result;
#endif

    if (g_nDisplayModeCascade_0049cea0 > 0)
        return;
    if (left < 0)
        left = 0;
    else if (left > 319)
        return;
    if (right < 0)
        return;
    if (right > 319)
        right = 319;
    if (top < 0)
        top = 0;
    else if (top > 199)
        return;
    if (bottom < 0)
        return;
    if (bottom > 199)
        bottom = 199;

#ifdef SDL_PORT
    if (right < left || bottom < top)
        return;
    if (!SdlPresentIndexedFrame(g_pDibPixelBuffer_005b3978, g_abPaletteCache_005c3450))
        DIBerror("DIBupdate", -1);
#else
    width = right - left + 1;
    height = bottom - top + 1;
    memset(&surface, 0, sizeof(surface));
    surface.dwSize = sizeof(surface);
    result = IDirectDrawSurface_Lock(
        g_pPrimarySurface_0049ce94, 0, &surface,
        DDLOCK_WAIT, 0);
    if (result != DD_OK) {
        IDirectDrawSurface_Unlock(
            g_pPrimarySurface_0049ce94, surface.lpSurface);
        DIBerror("DIBupdate   primary->Lock", result);
    }

    destination = (unsigned char *)surface.lpSurface
                + surface.lPitch * top + left;
    source = g_pDibPixelBuffer_005b3978 + g_nDibRowBytes_005b397c * top + left;
    IDirectDraw2_WaitForVerticalBlank(
        g_pDirectDraw2_0049ce90, DDWAITVB_BLOCKBEGIN, 0);
    while (height > 0) {
        memcpy(destination, source, width);
        destination += surface.lPitch;
        source += g_nDibRowBytes_005b397c;
        height--;
    }
    IDirectDrawSurface_Unlock(
        g_pPrimarySurface_0049ce94, surface.lpSurface);
#endif
}

/* Function start: 0x45E276 */
unsigned char *GetDIBPixelBuffer(void)
{
    return g_pDibPixelBuffer_005b3978;
}

/* Function start: 0x45E2C5 */
unsigned int GetDIBHeight(void)
{
    return g_nDibHeight_005b3980;
}

/* Function start: 0x45E2DA */
void CachePaletteEntryFromWords(short index, unsigned short *rgb)
{
    g_abPaletteCache_005c3450[index * 4 + 2] =
        *(unsigned char *)&rgb[0];
    g_ausPaletteWords_005d3220[index][0] =
        g_abPaletteCache_005c3450[index * 4 + 2];
    g_abPaletteCache_005c3450[index * 4 + 1] =
        *(unsigned char *)&rgb[1];
    g_ausPaletteWords_005d3220[index][1] =
        g_abPaletteCache_005c3450[index * 4 + 1];
    g_abPaletteCache_005c3450[index * 4] =
        *(unsigned char *)&rgb[2];
    g_ausPaletteWords_005d3220[index][2] =
        g_abPaletteCache_005c3450[index * 4];
    g_abPaletteCache_005c3450[index * 4 + 3] = 1;
}

/* Function start: 0x45E37C */
void DIBramPalette(void)
{
#ifdef SDL_PORT
    if (g_pDibPixelBuffer_005b3978 != 0 &&
        !SdlPresentIndexedFrame(g_pDibPixelBuffer_005b3978, g_abPaletteCache_005c3450))
        DIBerror("DIBramPalette", -1);
#else
    PALETTEENTRY entries[256];
    int index;
    HRESULT result;

    for (index = 0; index < 256; index++) {
        entries[index].peRed =
            g_abPaletteCache_005c3450[index * 4 + 2];
        entries[index].peGreen =
            g_abPaletteCache_005c3450[index * 4 + 1];
        entries[index].peBlue =
            g_abPaletteCache_005c3450[index * 4];
        entries[index].peFlags = 0;
    }
    if (g_bUseHardwarePalette_0049c268 != 0) {
        result = IDirectDrawPalette_SetEntries(
            g_pDirectDrawPalette_0049ce9c, 0, 0, 256, entries);
        if (result != DD_OK)
            DIBerror("DIBramPalette   SetEntries", result);
    }
#endif
}

/* Function start: 0x45E46C */
void DIBsetPalette(short index, short *rgb)
{
#ifndef SDL_PORT
    PALETTEENTRY entry;
    HRESULT result;
#endif

    if ((int)g_abPaletteCache_005c3450[index * 4 + 2] != (int)rgb[0] ||
        (int)g_abPaletteCache_005c3450[index * 4 + 1] != (int)rgb[1] ||
        (int)g_abPaletteCache_005c3450[index * 4] != (int)rgb[2]) {
        g_abPaletteCache_005c3450[index * 4 + 2] =
            *(unsigned char *)&rgb[0];
        g_ausPaletteWords_005d3220[index][0] =
            g_abPaletteCache_005c3450[index * 4 + 2];
        g_abPaletteCache_005c3450[index * 4 + 1] =
            *(unsigned char *)&rgb[1];
        g_ausPaletteWords_005d3220[index][1] =
            g_abPaletteCache_005c3450[index * 4 + 1];
        g_abPaletteCache_005c3450[index * 4] =
            *(unsigned char *)&rgb[2];
        g_ausPaletteWords_005d3220[index][2] =
            g_abPaletteCache_005c3450[index * 4];
        g_abPaletteCache_005c3450[index * 4 + 3] = 1;

        /* SDL_PORT consumes this cache on the next normal frame submission.
           A DirectDraw palette entry update did not blit or wait for vertical
           blank, so flight fades must not submit additional SDL frames. */
#ifndef SDL_PORT
        entry.peRed = (unsigned char)g_ausPaletteWords_005d3220[index][0];
        entry.peGreen = (unsigned char)g_ausPaletteWords_005d3220[index][1];
        entry.peBlue = (unsigned char)g_ausPaletteWords_005d3220[index][2];
        entry.peFlags = 0;
        if (g_bUseHardwarePalette_0049c268 != 0) {
            result = IDirectDrawPalette_SetEntries(
                g_pDirectDrawPalette_0049ce9c, 0, (int)index, 1,
                &entry);
            if (result != DD_OK)
                DIBerror("DIBsetPalette   SetEntries", result);

            result = IDirectDrawSurface_SetPalette(
                g_pPrimarySurface_0049ce94,
                g_pDirectDrawPalette_0049ce9c);
            if (result != DD_OK)
                DIBerror("DIBmakeDIB   CreatePalette", result);
        }
#endif
    }
}

/* Function start: 0x45E61C */
void GetPaletteEntryAsWords(short i, unsigned short *rgb)
{
    rgb[0] = g_abPaletteCache_005c3450[i * 4 + 2];
    rgb[1] = g_abPaletteCache_005c3450[i * 4 + 1];
    rgb[2] = g_abPaletteCache_005c3450[i * 4];
}

/* Function start: 0x45E667 */
void DIBwholePaletteFromTriplets(unsigned char *palette)
{
#ifndef SDL_PORT
    unsigned char entries[0x400];
    int error;
#endif
    int index = 0x100;

#ifndef SDL_PORT
    IDirectDraw2_WaitForVerticalBlank(
        g_pDirectDraw2_0049ce90, DDWAITVB_BLOCKBEGIN, 0);
#else
    SdlWaitForVerticalBlank();
#endif
    for (index = 0; index < 0x100; index++) {
        g_abPaletteCache_005c3450[index * 4 + 2] =
            palette[index * 3];
#ifndef SDL_PORT
        entries[index * 4] =
            g_abPaletteCache_005c3450[index * 4 + 2];
#endif
        g_abPaletteCache_005c3450[index * 4 + 1] =
            palette[index * 3 + 1];
#ifndef SDL_PORT
        entries[index * 4 + 1] =
            g_abPaletteCache_005c3450[index * 4 + 1];
#endif
        g_abPaletteCache_005c3450[index * 4] =
            palette[index * 3 + 2];
#ifndef SDL_PORT
        entries[index * 4 + 2] =
            g_abPaletteCache_005c3450[index * 4];
        entries[index * 4 + 3] = 0;
#endif
        g_abPaletteCache_005c3450[index * 4 + 3] = 4;
    }

#ifdef SDL_PORT
    DIBramPalette();
#else
    if (g_bUseHardwarePalette_0049c268 != 0) {
        error = IDirectDrawPalette_SetEntries(
            g_pDirectDrawPalette_0049ce9c, 0, 0, 0x100,
            (LPPALETTEENTRY)entries);
        if (error != 0)
            DIBerror("DIBsetWholePalette   SetEntries", error);

        error = IDirectDrawSurface_SetPalette(
            g_pPrimarySurface_0049ce94, g_pDirectDrawPalette_0049ce9c);
        if (error != 0)
            DIBerror("DIBmakeDIB   CreatePalette", error);
    }
#endif
}

/* Function start: 0x45E816 */
void DIBwholePaletteFromWords(unsigned short *palette)
{
#ifndef SDL_PORT
    unsigned char entries[0x400];
    int error;
#endif
    int index = 0x100;

#ifndef SDL_PORT
    IDirectDraw2_WaitForVerticalBlank(
        g_pDirectDraw2_0049ce90, DDWAITVB_BLOCKBEGIN, 0);
#else
    SdlWaitForVerticalBlank();
#endif
    for (index = 0; index < 0x100; index++) {
        g_abPaletteCache_005c3450[index * 4 + 2] =
            *(unsigned char *)&palette[index * 3];
#ifndef SDL_PORT
        entries[index * 4] =
            g_abPaletteCache_005c3450[index * 4 + 2];
#endif
        g_abPaletteCache_005c3450[index * 4 + 1] =
            *(unsigned char *)&palette[index * 3 + 1];
#ifndef SDL_PORT
        entries[index * 4 + 1] =
            g_abPaletteCache_005c3450[index * 4 + 1];
#endif
        g_abPaletteCache_005c3450[index * 4] =
            *(unsigned char *)&palette[index * 3 + 2];
#ifndef SDL_PORT
        entries[index * 4 + 2] =
            g_abPaletteCache_005c3450[index * 4];
        entries[index * 4 + 3] = 0;
#endif
        g_abPaletteCache_005c3450[index * 4 + 3] = 4;
    }

#ifdef SDL_PORT
    DIBramPalette();
#else
    if (g_bUseHardwarePalette_0049c268 != 0) {
        error = IDirectDrawPalette_SetEntries(
            g_pDirectDrawPalette_0049ce9c, 0, 0, 0x100,
            (LPPALETTEENTRY)entries);
        if (error != 0)
            DIBerror("DIBsetWholePalette   SetEntries", error);

        error = IDirectDrawSurface_SetPalette(
            g_pPrimarySurface_0049ce94, g_pDirectDrawPalette_0049ce9c);
        if (error != 0)
            DIBerror("DIBmakeDIB   CreatePalette", error);
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
        g_pDirectDraw2_0049ce90, DDWAITVB_BLOCKBEGIN, 0);
#endif
}

#ifndef SDL_PORT

#endif
