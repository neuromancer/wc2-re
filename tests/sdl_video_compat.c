#include "wc1.h"

#include "video_internal.h"

#include <limits.h>
#include <string.h>

static int CheckViewport(int width, int height, int expectedLeft,
                         int expectedBottom, int expectedWidth,
                         int expectedHeight)
{
    int bottom;
    int left;
    int viewportHeight;
    int viewportWidth;

    Wc1SdlCalculateOutputViewport(width, height, &left, &bottom,
                                  &viewportWidth, &viewportHeight);
    if (left == expectedLeft && bottom == expectedBottom &&
        viewportWidth == expectedWidth && viewportHeight == expectedHeight)
        return 1;
    fprintf(stderr,
            "Viewport %dx%d was %d,%d %dx%d; expected %d,%d %dx%d.\n",
            width, height, left, bottom, viewportWidth, viewportHeight,
            expectedLeft, expectedBottom, expectedWidth, expectedHeight);
    return 0;
}

static int CheckCoordinateMapping(SDL_Window *window,
                                  Wc1SdlVideoBackend backend)
{
    int coordinate;
    int logicalX;
    int logicalY;
    int windowX;
    int windowY;

    Wc1SdlSetVideoBackend(backend);
    if (!Wc1SdlMapLogicalToWindow(window, 0, 0, &windowX, &windowY) ||
        windowX != 33 || windowY != 0)
        return 0;
    if (!Wc1SdlMapLogicalToWindow(window, 160, 100, &windowX, &windowY) ||
        windowX != 500 || windowY != 350)
        return 0;
    if (!Wc1SdlMapWindowToLogical(window, 500, 350, &logicalX, &logicalY) ||
        logicalX != 160 || logicalY != 100)
        return 0;

    coordinate = 0;
    while (coordinate < WC1_SDL_FRAME_WIDTH) {
        if (!Wc1SdlMapLogicalToWindow(window, coordinate, 100,
                                      &windowX, &windowY) ||
            !Wc1SdlMapWindowToLogical(window, windowX, windowY,
                                      &logicalX, &logicalY) ||
            logicalX != coordinate || logicalY != 100)
            return 0;
        coordinate++;
    }
    coordinate = 0;
    while (coordinate < WC1_SDL_FRAME_HEIGHT) {
        if (!Wc1SdlMapLogicalToWindow(window, 160, coordinate,
                                      &windowX, &windowY) ||
            !Wc1SdlMapWindowToLogical(window, windowX, windowY,
                                      &logicalX, &logicalY) ||
            logicalX != 160 || logicalY != coordinate)
            return 0;
        coordinate++;
    }
    if (!Wc1SdlMapWindowToLogical(window, 0, 350, &logicalX, &logicalY) ||
        logicalX >= 0)
        return 0;
    if (!Wc1SdlMapWindowToLogical(window, 999, 350, &logicalX, &logicalY) ||
        logicalX < WC1_SDL_FRAME_WIDTH)
        return 0;
    return 1;
}

static int ReadArgbPixel(SDL_Renderer *renderer, int x, int y, Uint32 *pixel)
{
    SDL_Rect sample;

    sample.x = x;
    sample.y = y;
    sample.w = 1;
    sample.h = 1;
    return SDL_RenderReadPixels(renderer, &sample, SDL_PIXELFORMAT_ARGB8888,
                                pixel, (int)sizeof(*pixel)) == 0;
}

static int CheckIndexedPresentation(SDL_Window *window)
{
    unsigned char palette[256 * 4];
    unsigned char pixels[WC1_SDL_FRAME_WIDTH * WC1_SDL_FRAME_HEIGHT];
    SDL_Renderer *renderer;
    Uint32 pixel;
    int result;

    memset(palette, 0, sizeof(palette));
    memset(pixels, 1, sizeof(pixels));
    palette[4] = 0xff;
    palette[5] = 0xff;
    palette[6] = 0xff;
    Wc1SdlSetVideoBackend(WC1_SDL_VIDEO_BACKEND_INDEXED);
    if (!Wc1SdlInitializeVideo(window))
        return 0;
    renderer = SDL_GetRenderer(window);
    result = renderer != 0 && Wc1SdlPresentIndexedFrame(pixels, palette) &&
             ReadArgbPixel(renderer, 0, 350, &pixel) &&
             (pixel & 0x00ffffffU) == 0;
    result = result && ReadArgbPixel(renderer, 500, 350, &pixel) &&
             (pixel & 0x00ffffffU) == 0x00ffffffU;
    Wc1SdlShutdownVideo();
    return result;
}

int main(int argumentCount, char **arguments)
{
    SDL_Window *window;
    int result;

    (void)argumentCount;
    (void)arguments;
    SDL_SetMainReady();
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return 1;
    window = SDL_CreateWindow("WC2 video test", 0, 0, 1000, 700,
                              SDL_WINDOW_HIDDEN);
    if (window == 0) {
        SDL_Quit();
        return 1;
    }

    result = CheckViewport(320, 200, 27, 0, 266, 200) &&
             CheckViewport(960, 600, 80, 0, 800, 600) &&
             CheckViewport(1000, 700, 33, 0, 933, 700) &&
             CheckViewport(1920, 1080, 240, 0, 1440, 1080) &&
             CheckViewport(600, 1000, 0, 275, 600, 450) &&
             CheckViewport(0, 0, 0, 0, 1, 1) &&
             CheckViewport(1, 1, 0, 0, 1, 1) &&
             CheckViewport(INT_MAX, INT_MAX, 0, 268435456, INT_MAX,
                           1610612735) &&
             CheckCoordinateMapping(window, WC1_SDL_VIDEO_BACKEND_INDEXED) &&
             CheckCoordinateMapping(
                 window, WC1_SDL_VIDEO_BACKEND_GL_SHARP_BILINEAR) &&
             CheckIndexedPresentation(window);
    if (!result)
        fprintf(stderr, "SDL video compatibility test failed.\n");
    Wc1SdlShutdownVideo();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return result ? 0 : 1;
}
