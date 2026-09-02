#include "game.h"

#include "video_internal.h"

static SDL_Renderer *g_pSdlRenderer;
static unsigned int g_dwSdlStaticNoiseSeed = 0x1f123bb5U;
static SDL_Texture *g_pSdlFrameTexture;
static Uint32 g_adwSdlFramePixels[
    SDL_FRAME_WIDTH * SDL_FRAME_HEIGHT];

/* EGA compatibility filter (--ega). WC2's own reconstructed engine is
 * hardcoded to MCGA/VGA mode 0x13 (g_szRasterDriverName_004902ac is always
 * "MCGA.DLL", and several LogDisplayMode("not MCGA")-style fatal checks
 * abort on anything else) -- unlike WC1, no EGA path exists anywhere in
 * what this project reconstructs. But WC2's *original* 1991 floppy
 * release (predating the VGA-only "Kilrathi Saga" CD-ROM release this
 * project is based on) did ship a real EGA installer, confirmed directly:
 * its own INSTALL.EXE (from an original install disk) has a genuine
 * VGA->EGA conversion routine, structurally identical to WC1's (traced
 * live under a paused DOSBox Staging debugger session, same technique as
 * WC1's --ega filter) -- a 256-entry table where each byte packs two 4-bit
 * EGA color indices (high nibble / low nibble), with the per-pixel loop
 * alternating (`xor bx, 100h`) between the two on every pixel, checkerboard
 * -seeded per scanline. The table itself was read directly out of live
 * DOSBox memory mid-conversion. Unlike WC1's table, WC2's own reserves its
 * first 16 VGA palette indices as an exact identity mapping onto the EGA
 * palette (byte i == (i << 4) | i for i in 0..15) -- no dithering needed
 * for those indices, only for the rest of the 256-color palette.
 * The per-scanline checkerboard seed `(x+y)&1` reproduces the confirmed
 * "toggle every pixel" mechanism; the installer's own precise seed
 * computation was not independently re-derived bit-for-bit, so this is the
 * standard ordered-dither seed, not a byte-exact trace of that one
 * instruction sequence. */
static int g_bEgaDitherEnabled;
static unsigned char g_abEgaDitherPixels[
    SDL_FRAME_WIDTH * SDL_FRAME_HEIGHT];
/* Both host renderers accept a complete 256-entry [B,G,R,pad] palette even
 * though converted pixels only use the first 16 entries. */
static unsigned char g_abEgaDitherPalette[256 * 4];

static const unsigned char g_abEgaDitherTable[256] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
    0x88, 0xff, 0xf7, 0x77, 0x77, 0x77, 0x77, 0x77, 0xc7, 0x66, 0x68, 0x88, 0x88, 0x88, 0x00, 0x00,
    0xff, 0xf7, 0x77, 0x79, 0x99, 0x99, 0x91, 0x11, 0x11, 0x11, 0x11, 0x10, 0x10, 0x11, 0x00, 0x00,
    0xff, 0x77, 0x77, 0x77, 0x77, 0x88, 0x88, 0x88, 0x98, 0x98, 0x91, 0x10, 0x10, 0x00, 0x00, 0x00,
    0xff, 0xee, 0xee, 0xee, 0xee, 0xee, 0xc6, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x00, 0x00,
    0xff, 0xf7, 0x77, 0xcc, 0xc7, 0xcc, 0xcc, 0x44, 0xee, 0xea, 0xaa, 0xaa, 0xaa, 0x22, 0x22, 0x00,
    0xff, 0xf7, 0x77, 0x77, 0x7e, 0x7e, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x68, 0x60, 0x00, 0x00,
    0xff, 0xf7, 0x77, 0x77, 0xc7, 0xc7, 0x66, 0x66, 0x66, 0x64, 0x64, 0x44, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xfc, 0xcc, 0xcc, 0xcc, 0xcc, 0x66, 0x66, 0x64, 0x48, 0x40, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xfb, 0xb7, 0xb7, 0x33, 0x33, 0x33, 0xbb, 0x33, 0x99, 0x88, 0x11, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xf7, 0x77, 0x77, 0x77, 0x77, 0x77, 0x88, 0x88, 0x37, 0x88, 0x88, 0x77, 0x88, 0x00,
    0xff, 0xfc, 0xfc, 0xcc, 0xc5, 0x54, 0x54, 0x54, 0x55, 0x54, 0x54, 0x58, 0x50, 0x00, 0x00, 0x00,
    0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
    0xff, 0xf7, 0x77, 0x77, 0x77, 0x77, 0x7f, 0x77, 0xa7, 0xa7, 0xaa, 0x82, 0x72, 0x28, 0x22, 0x00,
    0xff, 0x77, 0x77, 0x76, 0x66, 0x88, 0x88, 0x00, 0x66, 0x66, 0x66, 0x64, 0x44, 0x48, 0x40, 0xdd,
    0xee, 0xe7, 0x77, 0x72, 0x22, 0x28, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdd,
};

/* Standard IBM EGA/VGA 16-color hardware palette, R/G/B 0-255. */
static const unsigned char g_abEgaHardwarePaletteRgb[16][3] = {
    {0x00, 0x00, 0x00}, {0x00, 0x00, 0xaa}, {0x00, 0xaa, 0x00}, {0x00, 0xaa, 0xaa},
    {0xaa, 0x00, 0x00}, {0xaa, 0x00, 0xaa}, {0xaa, 0x55, 0x00}, {0xaa, 0xaa, 0xaa},
    {0x55, 0x55, 0x55}, {0x55, 0x55, 0xff}, {0x55, 0xff, 0x55}, {0x55, 0xff, 0xff},
    {0xff, 0x55, 0x55}, {0xff, 0x55, 0xff}, {0xff, 0xff, 0x55}, {0xff, 0xff, 0xff},
};

void SdlEnableEgaDither(void)
{
    int index;

    g_bEgaDitherEnabled = 1;
    for (index = 0; index < 16; index++) {
        /* Same [B,G,R,pad] layout SdlPresentIndexedFrame reads below. */
        g_abEgaDitherPalette[index * 4 + 0] =
            g_abEgaHardwarePaletteRgb[index][2];
        g_abEgaDitherPalette[index * 4 + 1] =
            g_abEgaHardwarePaletteRgb[index][1];
        g_abEgaDitherPalette[index * 4 + 2] =
            g_abEgaHardwarePaletteRgb[index][0];
        g_abEgaDitherPalette[index * 4 + 3] = 0;
    }
}

int SdlInitializeVideo(SDL_Window *window)
{
    SdlShutdownVideo();
    if (SdlUsingGlRenderer())
        return SdlGlRendererInitialize(window);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    g_pSdlRenderer =
        SDL_CreateRenderer(window, -1,
                           SDL_RENDERER_ACCELERATED |
                               SDL_RENDERER_PRESENTVSYNC);
    if (g_pSdlRenderer == 0)
        g_pSdlRenderer =
            SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (g_pSdlRenderer == 0)
        return 0;
    g_pSdlFrameTexture =
        SDL_CreateTexture(g_pSdlRenderer, SDL_PIXELFORMAT_ARGB8888,
                          SDL_TEXTUREACCESS_STREAMING,
                          SDL_FRAME_WIDTH,
                          SDL_FRAME_HEIGHT);
    if (g_pSdlFrameTexture == 0) {
        SdlShutdownVideo();
        return 0;
    }
    SDL_SetRenderDrawColor(g_pSdlRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(g_pSdlRenderer);
    SDL_RenderPresent(g_pSdlRenderer);
    return 1;
}

static int SdlComputeFrameDestRect(SDL_Rect *dest)
{
    int bottom;
    int height;
    int width;

    if (SDL_GetRendererOutputSize(g_pSdlRenderer, &width, &height) != 0 ||
        width < 1 || height < 1)
        return 0;
    SdlCalculateOutputViewport(width, height, &dest->x, &bottom, &dest->w,
                                  &dest->h);
    dest->y = height - bottom - dest->h;
    return 1;
}

void SdlShutdownVideo(void)
{
    SdlGlRendererShutdown();
    if (g_pSdlFrameTexture != 0) {
        SDL_DestroyTexture(g_pSdlFrameTexture);
        g_pSdlFrameTexture = 0;
    }
    if (g_pSdlRenderer != 0) {
        SDL_DestroyRenderer(g_pSdlRenderer);
        g_pSdlRenderer = 0;
    }
}

int SdlPresentIndexedFrame(const unsigned char *pixels,
                              const unsigned char *palette)
{
    SDL_Rect dest;
    int pixel;
    int result;
    int restoreMouseCursor;

    restoreMouseCursor =
        g_bInputCursorBackgroundCaptured_005c80c4 != 0 &&
        g_stScreenViewport_005d21a0.pixels == pixels;

    if (g_bEgaDitherEnabled && pixels != 0) {
        for (pixel = 0; pixel < SDL_FRAME_WIDTH * SDL_FRAME_HEIGHT;
             pixel++) {
            int x = pixel % SDL_FRAME_WIDTH;
            int y = pixel / SDL_FRAME_WIDTH;
            unsigned char packed = g_abEgaDitherTable[pixels[pixel]];

            g_abEgaDitherPixels[pixel] =
                ((x + y) & 1) ? (packed >> 4) : (packed & 0x0f);
        }
        pixels = g_abEgaDitherPixels;
        palette = g_abEgaDitherPalette;
    }

    if (SdlUsingGlRenderer()) {
        result = SdlGlRendererPresent(pixels, palette);
        if (result != 0 && restoreMouseCursor)
            RestoreMouseCursorBackground();
        return result;
    }
    if (g_pSdlRenderer == 0 || g_pSdlFrameTexture == 0 || pixels == 0 ||
        palette == 0 || !SdlComputeFrameDestRect(&dest))
        return 0;
    pixel = 0;
    while (pixel < SDL_FRAME_WIDTH * SDL_FRAME_HEIGHT) {
        int paletteOffset;
        unsigned char colour;

        colour = pixels[pixel];
        paletteOffset = colour * 4;
        g_adwSdlFramePixels[pixel] =
            0xff000000U |
            (Uint32)palette[paletteOffset + 2] << 16 |
            (Uint32)palette[paletteOffset + 1] << 8 |
            palette[paletteOffset];
        pixel++;
    }
    if (SDL_UpdateTexture(g_pSdlFrameTexture, 0, g_adwSdlFramePixels,
                          SDL_FRAME_WIDTH *
                              (int)sizeof(Uint32)) != 0)
        return 0;
    if (SDL_RenderClear(g_pSdlRenderer) != 0)
        return 0;
    if (SDL_RenderCopy(g_pSdlRenderer, g_pSdlFrameTexture, 0, &dest) != 0)
        return 0;
    SDL_RenderPresent(g_pSdlRenderer);
    if (restoreMouseCursor)
        RestoreMouseCursorBackground();
    return 1;
}

void SdlWaitForVerticalBlank(void)
{
    SDL_Rect dest;

    if (SdlUsingGlRenderer()) {
        SdlGlRendererWaitForVerticalBlank();
        return;
    }
    if (g_pSdlRenderer == 0 || g_pSdlFrameTexture == 0) {
        SDL_Delay(1);
        return;
    }
    if (!SdlComputeFrameDestRect(&dest)) {
        SDL_Delay(1);
        return;
    }
    SDL_RenderClear(g_pSdlRenderer);
    SDL_RenderCopy(g_pSdlRenderer, g_pSdlFrameTexture, 0, &dest);
    SDL_RenderPresent(g_pSdlRenderer);
}

void SdlBeginSpaceFrame(
    const struct ScreenViewportGeometry *geometry, int viewportMode,
    int fullViewportCopy, unsigned char clearColour)
{
    if (SdlUsingGlRenderer()) {
        SdlGlRendererBeginSpaceFrame(
            geometry, viewportMode, fullViewportCopy, clearColour);
    }
}

void SdlCompleteSpaceFrame(void)
{
    if (SdlUsingGlRenderer())
        SdlGlRendererCompleteSpaceFrame();
}

void SdlCancelSpaceFrame(void)
{
    /* RunSpaceFlight calls this when the spaceflight session ends. */
    SdlEndJoystickSpaceflight();
    if (SdlUsingGlRenderer())
        SdlGlRendererCancelSpaceFrame();
}

int SdlRecordSpaceSprite(
    const struct Viewport *viewport, float x, float y,
    unsigned char *shape, short frame, short angle, short scale,
    short flip)
{
    /* The enhanced/GL layer records sprites using raw VGA palette indices;
     * once EGA dithering has converted the composed frame those indices no
     * longer mean anything, so keep the composition point instead. */
    if (!SdlUsingGlRenderer() || g_bEgaDitherEnabled)
        return 0;
    return SdlGlRendererRecordSpaceSprite(
        viewport, x, y, shape, frame, angle, scale, flip);
}

static unsigned int SdlNextStaticNoise(void)
{
    g_dwSdlStaticNoiseSeed ^= g_dwSdlStaticNoiseSeed << 13;
    g_dwSdlStaticNoiseSeed ^= g_dwSdlStaticNoiseSeed >> 17;
    g_dwSdlStaticNoiseSeed ^= g_dwSdlStaticNoiseSeed << 5;
    return g_dwSdlStaticNoiseSeed;
}

void SdlDrawViewportStatic(struct Viewport *viewport, int effect,
                              unsigned short colour)
{
    unsigned int sample;
    short threshold;
    short x;
    short y;

    if (viewport == 0 || viewport->pixels == 0)
        return;
    /* Comm dropouts use effect 3, while a damaged display uses effect 1. */
    threshold = (short)(effect >= 3 ? 1 : 2);
    for (y = viewport->top; y <= viewport->bottom; y++) {
        for (x = viewport->left; x <= viewport->right; x++) {
            sample = SdlNextStaticNoise() >> 16;
            if ((short)(sample & 3) > threshold)
                continue;
            DrawViewportPixel(viewport, x, y,
                              (short)((sample & 4) != 0
                                          ? colour
                                          : 0));
        }
    }
}
