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
    g_bSpaceFlightActive_005c586c = 1;
    g_nFrameSkipCountdown_0049d760 = 1;
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
    g_bSpaceFlightActive_005c586c = 0;
    pixels[2] = 2;
    MarkDibDirty();
    DIBslamReal();
    result = result && !ReadRedPixelNearCentre(
                            frame, width, height, readPixels, readBuffer);

    SdlCancelSpaceFrame();
    g_bSpaceFlightActive_005c586c = 0;
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
    g_stMouseCursorState_0059ab10.viewport = 0;
    g_pInputViewport_005c8403 = viewport;
    g_pInputCursorShape_005c83f9 = shape;
    g_nInputCursorFrame_005c83fd = 0;
    g_nQueuedInputX_005c83f0 = 40;
    g_nQueuedInputY_005c83f2 = 40;
    g_nMouseCursorDrawDepth_0049d4d4 = 1;
    g_bInputCursorBackgroundCaptured_005c80c4 = 0;
    MarkDibDirty();
    DIBslamReal();
    result = g_bInputCursorBackgroundCaptured_005c80c4 == 0;
    pixel = 0;
    while (pixel < 320 * 200) {
        if (pixels[pixel] != 0) {
            result = 0;
            break;
        }
        pixel++;
    }
    RestoreMouseCursorBackground();
    g_pInputCursorShape_005c83f9 = 0;
    ReleasePacketHandle(shape);
    return result;
}

int main(int argumentCount, char **arguments)
{
    unsigned short rowOffsets[202];
    unsigned char *pixels;
    short red[3];
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
    g_pInputViewport_005c8403 = &viewport;
    g_bFrameTimingInitialized_0049cebc = 1;
    g_nFramePeriodMilliseconds_005c343c = 0;
    g_dwNextFrameDeadline_0049cea4 = 0;
    row = 0;
    while (row < 202) {
        rowOffsets[row] = (unsigned short)(row * 320);
        row++;
    }
    red[0] = 255;
    red[1] = 0;
    red[2] = 0;
    DIBsetPalette(1, red);
    SDL_GL_GetDrawableSize(window, &drawableWidth, &drawableHeight);
    result = pixels != 0 && drawableWidth > 0 && drawableHeight > 0 &&
             CheckSpaceLayerLifetime(&viewport, pixels, drawableWidth,
                                     drawableHeight) &&
             CheckPresentedCursorIsRestored(&viewport, pixels);
    if (!result)
        fprintf(stderr, "GL frame-lifetime test failed.\n");

    g_pInputViewport_005c8403 = 0;
    DIBunInstall();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return result ? 0 : 1;
}
