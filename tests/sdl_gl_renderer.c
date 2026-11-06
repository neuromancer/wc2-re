#include "game.h"

#include "video_internal.h"

#if defined(_WIN32) && !defined(APIENTRY)
#define APIENTRY __stdcall
#endif
#include <SDL_opengl.h>

#include <string.h>

typedef void(APIENTRY *TestGlReadPixelsProc)(GLint x, GLint y,
                                                GLsizei width,
                                                GLsizei height,
                                                GLenum format, GLenum type,
                                                void *pixels);
typedef void(APIENTRY *TestGlReadBufferProc)(GLenum source);

static unsigned char *CreateSinglePixelShape(void)
{
    unsigned char *shape;
    unsigned short rowOffset;

    shape = AllocateTaggedMemory(32, 0x40);
    if (shape == 0)
        return 0;
    memset(shape, 0, 32);
    *(int *)(shape + 4) = 8;
    rowOffset = 2;
    memcpy(shape + 16, &rowOffset, sizeof(rowOffset));
    shape[22] = 1;
    return shape;
}

static int ReadRedPixelNearCentre(unsigned char *frame, int width,
                                  int height,
                                  TestGlReadPixelsProc readPixels,
                                  TestGlReadBufferProc readBuffer)
{
    int bottom;
    int left;
    int right;
    int top;
    int x;
    int y;

    readBuffer(GL_FRONT);
    readPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, frame);
    left = width / 2 - width / 20;
    right = width / 2 + width / 20;
    bottom = height / 2 - height / 20;
    top = height / 2 + height / 20;
    y = bottom;
    while (y <= top) {
        x = left;
        while (x <= right) {
            const unsigned char *pixel;

            pixel = frame + ((size_t)y * (size_t)width + (size_t)x) * 4U;
            if (pixel[0] > 32 && pixel[0] > pixel[1] * 2 &&
                pixel[0] > pixel[2] * 2)
                return 1;
            x++;
        }
        y++;
    }
    return 0;
}

static int CheckSpaceLayerLifetime(Viewport *viewport, unsigned char *pixels,
                                   int width, int height)
{
    TestGlReadPixelsProc readPixels;
    TestGlReadBufferProc readBuffer;
    unsigned char *frame;
    unsigned char *shape;
    int result;

    readPixels =
        (TestGlReadPixelsProc)SDL_GL_GetProcAddress("glReadPixels");
    readBuffer =
        (TestGlReadBufferProc)SDL_GL_GetProcAddress("glReadBuffer");
    frame = malloc((size_t)width * (size_t)height * 4U);
    shape = CreateSinglePixelShape();
    if (readPixels == 0 || readBuffer == 0 || frame == 0 || shape == 0) {
        free(frame);
        if (shape != 0)
            ReleasePacketHandle(shape);
        return 0;
    }

    memset(pixels, 0, 320 * 200);
    g_1_bSpaceFlightActive = 1;
    g_nFrameSkipCountdown = 1;
    SdlBeginSpaceFrame(0, 5, 1, 0);
    result = SdlRecordSpaceSprite(
        viewport, 160.0f, 100.0f, shape, 0, 0, 0x100, 0);
    SdlCompleteSpaceFrame();
    MarkDibDirty();
    DIBslamReal();
    result = result && ReadRedPixelNearCentre(
                           frame, width, height, readPixels, readBuffer);

    /* WC2 presents while update_cockpit incrementally changes the indexed
       base.  Those same-frame presents must retain the completed layer. */
    pixels[0] = 2;
    MarkDibDirty();
    DIBslamReal();
    result = result && ReadRedPixelNearCentre(
                           frame, width, height, readPixels, readBuffer);

    pixels[1] = 2;
    MarkDibDirty();
    DIBslamReal();
    result = result && ReadRedPixelNearCentre(
                           frame, width, height, readPixels, readBuffer);

    /* Once flight ends, a changed base belongs to another screen and must
       invalidate the retained object layer. */
    g_1_bSpaceFlightActive = 0;
    pixels[2] = 2;
    MarkDibDirty();
    DIBslamReal();
    result = result && !ReadRedPixelNearCentre(
                            frame, width, height, readPixels, readBuffer);

    SdlCancelSpaceFrame();
    g_1_bSpaceFlightActive = 0;
    free(frame);
    ReleasePacketHandle(shape);
    return result;
}

static int CheckPresentedCursorIsRestored(Viewport *viewport,
                                          unsigned char *pixels)
{
    unsigned char *shape;
    int pixel;
    int result;

    shape = CreateSinglePixelShape();
    if (shape == 0)
        return 0;
    memset(pixels, 0, 320 * 200);
    g_16_stEventQueuePointerData.viewport = viewport;
    g_16_stEventQueuePointerData.shape = shape;
    g_16_stEventQueuePointerData.frame = 0;
    g_16_stEventQueuePointerData.x = 40;
    g_16_stEventQueuePointerData.y = 40;
    g_nMouseCursorDrawDepth = 1;
    g_6_bCurrentInputCursorBackgroundCapturedState = 0;
    MarkDibDirty();
    DIBslamReal();
    result = g_6_bCurrentInputCursorBackgroundCapturedState == 0;
    pixel = 0;
    while (pixel < 320 * 200) {
        if (pixels[pixel] != 0) {
            result = 0;
            break;
        }
        pixel++;
    }
    RestoreMouseCursorBackground();
    g_16_stEventQueuePointerData.shape = 0;
    ReleasePacketHandle(shape);
    return result;
}

static int CheckLogicalPixel(int width, int height, int x, int y,
                              int expectedIndex)
{
    TestGlReadPixelsProc readPixels;
    TestGlReadBufferProc readBuffer;
    unsigned char pixel[4];
    int left;
    int bottom;
    int viewportWidth;
    int viewportHeight;
    int red;
    int blue;

    readPixels =
        (TestGlReadPixelsProc)SDL_GL_GetProcAddress("glReadPixels");
    readBuffer =
        (TestGlReadBufferProc)SDL_GL_GetProcAddress("glReadBuffer");
    if (readPixels == 0 || readBuffer == 0)
        return 0;
    SdlCalculateOutputViewport(width, height, &left, &bottom,
                               &viewportWidth, &viewportHeight);
    readBuffer(GL_FRONT);
    readPixels(left + (2 * x + 1) * viewportWidth / 640,
               bottom + (399 - 2 * y) * viewportHeight / 400,
               1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    red = expectedIndex == 1 ? 255 : 0;
    blue = expectedIndex == 2 ? 255 : 0;
    if (abs((int)pixel[0] - red) <= 32 && pixel[1] <= 32 &&
        abs((int)pixel[2] - blue) <= 32)
        return 1;
    fprintf(stderr, "GL pixel %d,%d was %u,%u,%u; expected index %d.\n",
            x, y, pixel[0], pixel[1], pixel[2], expectedIndex);
    return 0;
}

static int CheckCockpitSpaceLayer(Viewport *viewport, unsigned char *pixels,
                                   int width, int height)
{
    /* PCSHIP.V01's forward Rapier view is 320x70 at (0,24).  Use a
       rectangular aperture with those bounds so no game assets are needed. */
    const ScreenViewportGeometry geometry = {
        320, 70, 0, 24, {0, 24, 22400, -1}
    };
    const int displayModes[3] = {0, 1, -2};
    unsigned char *shape;
    unsigned char *spacePixels;
    int mode;
    int object;
    int probeY;
    int result;

    shape = CreateSinglePixelShape();
    if (shape == 0)
        return 0;
    g_27_stViewScratchInfo = *viewport;
    g_nSpacePaletteFadeMode = 0x13;
    if (!AllocateViewport(&g_27_stViewScratchInfo, 0, 0)) {
        ReleasePacketHandle(shape);
        return 0;
    }
    spacePixels = g_27_stViewScratchInfo.pixels;
    for (object = 0; object < SPACE_OBJECT_COUNT; object++) {
        g_aeObjectClass[object] = OBJECT_CLASS_NULL;
        g_asObjectType[object] = -1;
        g_asObjectScreenX[object] = 0;
    }
    g_1_bSpaceFlightActive = 1;
    g_nCurrentView = 1;
    g_1_cStoredViewDisplayModeValue = 0;
    g_15_pScreenViewportGeometry = &geometry;
    g_cPrimaryViewBufferColour = 0;
    g_nFrameSkip = 1;
    result = 1;
    for (mode = 0; mode < 3; mode++) {
        g_nCockpitDisplayMode = displayModes[mode];
        g_bFullScreenSpaceView = displayModes[mode] == 0;
        g_27_stViewScratchInfo.bottom = displayModes[mode] > 0 ? 199 : 69;
        memset(spacePixels, 0, 320 * 200);
        memset(pixels, 2, 320 * 200);
        g_nFrameSkipCountdown = 1;

        /* Exercise the production begin-frame hook, with an empty object
           list, then put a red probe near the bottom of the source view. */
        result &= DrawSpaceSceneFrame() != 0;
        probeY = displayModes[mode] > 0 ? 160 : 60;
        result &= SdlRecordSpaceSprite(&g_27_stViewScratchInfo,
                                       160.0f, (float)probeY,
                                       shape, 0, 0, 0x100, 0);
        dump_buffer_to_screen();
        DIBslamReal();
        if (displayModes[mode] > 0) {
            result &= CheckLogicalPixel(width, height, 160, 160, 1);
            result &= CheckLogicalPixel(width, height, 160, 184, 0);
        } else {
            result &= CheckLogicalPixel(width, height, 160, 84, 1);
            result &= CheckLogicalPixel(width, height, 160, 60, 0);
            result &= CheckLogicalPixel(width, height, 160, 100, 2);
        }
    }
    SdlCancelSpaceFrame();
    g_1_bSpaceFlightActive = 0;
    g_nCockpitDisplayMode = 0;
    g_15_pScreenViewportGeometry = 0;
    free_viewport(&g_27_stViewScratchInfo);
    ReleasePacketHandle(shape);
    return result;
}

static int CheckFlightCursorDisplay(Viewport *viewport,
                                     unsigned char *pixels,
                                     int width, int height)
{
    unsigned char *shape;
    unsigned char flightPixel;
    int result;

    shape = CreateSinglePixelShape();
    if (shape == 0)
        return 0;
    memset(pixels, 0, 320 * 200);
    g_16_stEventQueuePointerData.viewport = viewport;
    g_16_stEventQueuePointerData.shape = shape;
    g_16_stEventQueuePointerData.frame = 0;
    g_16_stEventQueuePointerData.x = 40;
    g_16_stEventQueuePointerData.y = 40;
    g_nMouseCursorDrawDepth = 1;
    g_6_bCurrentInputCursorBackgroundCapturedState = 0;
    MarkDibDirty();
    DIBslamReal();
    result = CheckLogicalPixel(width, height, 40, 40, 1);

    /* Flight redirects input to its offscreen viewport but retains the
       menu cursor shape and draw depth.  It must not stamp that arrow. */
    g_27_stViewScratchInfo = *viewport;
    g_27_stViewScratchInfo.pixels = &flightPixel;
    g_1_bSpaceFlightActive = 1;
    InitializeSpaceFlightInput();
    MarkDibDirty();
    DIBslamReal();
    result &= CheckLogicalPixel(width, height, 40, 40, 0);

    /* A modal screen during flight can use the menu pointer again. */
    SetInputViewport(viewport);
    MarkDibDirty();
    DIBslamReal();
    result &= CheckLogicalPixel(width, height, 40, 40, 1);

    g_1_bSpaceFlightActive = 0;
    g_16_stEventQueuePointerData.shape = 0;
    ClearInputPump();
    memset(&g_27_stViewScratchInfo, 0, sizeof(g_27_stViewScratchInfo));
    ReleasePacketHandle(shape);
    return result;
}

int main(int argumentCount, char **arguments)
{
    unsigned short rowOffsets[202];
    unsigned char *pixels;
    short red[3];
    short blue[3];
    SDL_GLContext probeContext;
    SDL_Window *window;
    Uint32 windowFlags;
    Viewport viewport;
    int drawableHeight;
    int drawableWidth;
    int result;
    int row;

    (void)argumentCount;
    (void)arguments;
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "Skipping GL renderer tests: %s\n", SDL_GetError());
        return 77;
    }
    SdlSetVideoBackend(SDL_VIDEO_BACKEND_GL_SHARP_BILINEAR);
    windowFlags = SDL_WINDOW_HIDDEN;
    if (!SdlConfigureVideoWindow(&windowFlags)) {
        fprintf(stderr, "Skipping GL renderer tests: %s\n", SDL_GetError());
        SDL_Quit();
        return 77;
    }
    window = SDL_CreateWindow("GL renderer test", 0, 0, 640, 400,
                              windowFlags);
    if (window == 0) {
        fprintf(stderr, "Skipping GL renderer tests: %s\n", SDL_GetError());
        SDL_Quit();
        return 77;
    }
    probeContext = SDL_GL_CreateContext(window);
    if (probeContext == 0) {
        fprintf(stderr, "Skipping GL renderer tests: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 77;
    }
    SDL_GL_MakeCurrent(window, 0);
    SDL_GL_DeleteContext(probeContext);

    DIBinstall((HWND)window);
    SDL_GL_SetSwapInterval(0);
    pixels = GetDIBPixelBuffer();
    viewport.pixels = pixels;
    viewport.rowOffsets = rowOffsets;
    viewport.left = 0;
    viewport.top = 0;
    viewport.right = 319;
    viewport.bottom = 199;
    viewport.allocation = pixels;
    g_16_stEventQueuePointerData.viewport = &viewport;
    g_bFrameTimingInitialized = 1;
    g_17_nFramePeriodMillis = 0;
    g_dwNextFrameDeadline = 0;
    row = 0;
    while (row < 202) {
        rowOffsets[row] = (unsigned short)(row * 320);
        row++;
    }
    g_101_stScrViewInfo = viewport;
    red[0] = 255;
    red[1] = 0;
    red[2] = 0;
    DIBsetPalette(1, red);
    blue[0] = 0;
    blue[1] = 0;
    blue[2] = 255;
    DIBsetPalette(2, blue);
    SDL_GL_GetDrawableSize(window, &drawableWidth, &drawableHeight);
    result = pixels != 0 && drawableWidth > 0 && drawableHeight > 0 &&
             CheckSpaceLayerLifetime(&viewport, pixels, drawableWidth,
                                     drawableHeight) &&
             CheckPresentedCursorIsRestored(&viewport, pixels);
    if (pixels != 0 && drawableWidth > 0 && drawableHeight > 0) {
        result &= CheckCockpitSpaceLayer(&viewport, pixels, drawableWidth,
                                         drawableHeight);
        result &= CheckFlightCursorDisplay(&viewport, pixels, drawableWidth,
                                            drawableHeight);
    }
    if (!result)
        fprintf(stderr, "GL renderer test failed.\n");

    g_16_stEventQueuePointerData.viewport = 0;
    DIBunInstall();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return result ? 0 : 1;
}
