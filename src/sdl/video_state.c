#include "video_internal.h"
#include "wcdata.h"

static Wc1SdlVideoBackend g_videoBackend;
static float g_afThrusterScreenX[WC2_SPACE_OBJECT_COUNT];
static float g_afThrusterScreenY[WC2_SPACE_OBJECT_COUNT];

void Wc1SdlSetThrusterScreenPosition(short object, float x, float y)
{
    g_afThrusterScreenX[object] = x;
    g_afThrusterScreenY[object] = y;
}

void Wc1SdlGetThrusterScreenPosition(short object, float *x, float *y)
{
    *x = g_afThrusterScreenX[object];
    *y = g_afThrusterScreenY[object];
}

void Wc1SdlSetVideoBackend(Wc1SdlVideoBackend backend)
{
    if (backend == WC1_SDL_VIDEO_BACKEND_GL_SHARP_BILINEAR)
        g_videoBackend = backend;
    else
        g_videoBackend = WC1_SDL_VIDEO_BACKEND_INDEXED;
}

int Wc1SdlUsingGlRenderer(void)
{
    return g_videoBackend == WC1_SDL_VIDEO_BACKEND_GL_SHARP_BILINEAR;
}

int Wc1SdlConfigureVideoWindow(Uint32 *windowFlags)
{
    if (windowFlags == 0)
        return 0;
    if (!Wc1SdlUsingGlRenderer())
        return 1;
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) != 0 ||
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2) != 0 ||
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_CORE) != 0 ||
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) != 0)
        return 0;
#ifdef __APPLE__
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                            SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG) != 0)
        return 0;
#endif
    *windowFlags |= SDL_WINDOW_OPENGL;
    return 1;
}

void Wc1SdlCalculateOutputViewport(int width, int height, int *left,
                                   int *bottom, int *viewportWidth,
                                   int *viewportHeight)
{
    if (width < 1 || height < 1) {
        *left = 0;
        *bottom = 0;
        *viewportWidth = 1;
        *viewportHeight = 1;
        return;
    }

    /* Mode 13h artwork is composed for a 4:3 display. */
    if ((Sint64)width * WC1_SDL_DISPLAY_ASPECT_HEIGHT >
        (Sint64)height * WC1_SDL_DISPLAY_ASPECT_WIDTH) {
        *viewportHeight = height;
        *viewportWidth =
            (int)((Sint64)height * WC1_SDL_DISPLAY_ASPECT_WIDTH /
                  WC1_SDL_DISPLAY_ASPECT_HEIGHT);
    } else {
        *viewportWidth = width;
        *viewportHeight =
            (int)((Sint64)width * WC1_SDL_DISPLAY_ASPECT_HEIGHT /
                  WC1_SDL_DISPLAY_ASPECT_WIDTH);
    }
    if (*viewportWidth < 1)
        *viewportWidth = 1;
    if (*viewportHeight < 1)
        *viewportHeight = 1;
    *left = (width - *viewportWidth) / 2;
    *bottom = (height - *viewportHeight) / 2;
}

static int Wc1SdlGetWindowViewport(SDL_Window *window, int *left, int *top,
                                   int *viewportWidth, int *viewportHeight)
{
    int bottom;
    int height;
    int width;

    SDL_GetWindowSize(window, &width, &height);
    if (width < 1 || height < 1)
        return 0;
    Wc1SdlCalculateOutputViewport(width, height, left, &bottom,
                                  viewportWidth, viewportHeight);
    *top = height - bottom - *viewportHeight;
    return 1;
}

static int Wc1SdlScaleCoordinate(int coordinate, int sourceExtent,
                                 int destinationExtent)
{
    Sint64 numerator;

    numerator = (Sint64)coordinate * destinationExtent;
    if (numerator < 0)
        numerator -= sourceExtent / 2;
    else
        numerator += sourceExtent / 2;
    return (int)(numerator / sourceExtent);
}

int Wc1SdlMapLogicalToWindow(SDL_Window *window, int logicalX, int logicalY,
                             int *windowX, int *windowY)
{
    int viewportHeight;
    int viewportLeft;
    int viewportTop;
    int viewportWidth;

    if (window == 0 || windowX == 0 || windowY == 0)
        return 0;
    if (!Wc1SdlGetWindowViewport(window, &viewportLeft, &viewportTop,
                                 &viewportWidth, &viewportHeight))
        return 0;
    *windowX = viewportLeft +
               Wc1SdlScaleCoordinate(logicalX, WC1_SDL_FRAME_WIDTH,
                                      viewportWidth);
    *windowY = viewportTop +
               Wc1SdlScaleCoordinate(logicalY, WC1_SDL_FRAME_HEIGHT,
                                      viewportHeight);
    return 1;
}

int Wc1SdlMapWindowToLogical(SDL_Window *window, int windowX, int windowY,
                             int *logicalX, int *logicalY)
{
    int viewportHeight;
    int viewportLeft;
    int viewportTop;
    int viewportWidth;

    if (window == 0 || logicalX == 0 || logicalY == 0)
        return 0;
    if (!Wc1SdlGetWindowViewport(window, &viewportLeft, &viewportTop,
                                 &viewportWidth, &viewportHeight))
        return 0;
    *logicalX = Wc1SdlScaleCoordinate(windowX - viewportLeft, viewportWidth,
                                      WC1_SDL_FRAME_WIDTH);
    *logicalY = Wc1SdlScaleCoordinate(windowY - viewportTop, viewportHeight,
                                      WC1_SDL_FRAME_HEIGHT);
    return 1;
}
