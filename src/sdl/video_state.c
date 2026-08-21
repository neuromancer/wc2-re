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
    /* Target a 4:3 output, not the source frame's own 320x200 (8:5)
     * shape. 320x200 is square-pixel VGA Mode 13h; the original game
     * was always displayed on 4:3 CRT hardware, which rendered it with
     * non-square pixels (taller than wide) to fill that shape. Scaling
     * width and height by the same factor -- as this used to, to keep
     * the source's own 8:5 ratio and land on an exact integer multiple
     * of it -- reproduces the square-pixel shape instead, which comes
     * out visibly wider/flatter than the original ever looked. Fit the
     * largest 4:3 rectangle inside (width, height) instead; this can't
     * be an exact integer multiple of 320x200 in both axes at once,
     * since 320x200 itself isn't 4:3. */
    if (width * 3 > height * 4) {
        *viewportHeight = height;
        *viewportWidth = height * 4 / 3;
    } else {
        *viewportWidth = width;
        *viewportHeight = width * 3 / 4;
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
    /* Both backends now present into a manually computed 4:3 rect
     * (video.c/gl_renderer.c) rather than relying on SDL_RenderSet
     * LogicalSize's own native-320x200-aspect letterboxing, so mouse
     * mapping needs the same 4:3 calculation for both, not just GL. */
    Wc1SdlCalculateOutputViewport(width, height, left, &bottom,
                                  viewportWidth, viewportHeight);
    *top = height - bottom - *viewportHeight;
    return 1;
}

int Wc1SdlMapLogicalToWindow(SDL_Window *window, int logicalX, int logicalY,
                             int *windowX, int *windowY)
{
    int viewportHeight;
    int viewportLeft;
    int viewportTop;
    int viewportWidth;

    /* Both backends present into a manually computed 4:3 rect (see
     * Wc1SdlGetWindowViewport) rather than an SDL_Renderer logical size,
     * so SDL_RenderLogicalToWindow no longer has the right transform to
     * ask even when a renderer exists -- always use the same viewport
     * calculation the frame itself was drawn into. */
    if (window == 0 || windowX == 0 || windowY == 0)
        return 0;
    if (!Wc1SdlGetWindowViewport(window, &viewportLeft, &viewportTop,
                                 &viewportWidth, &viewportHeight))
        return 0;
    *windowX = viewportLeft + logicalX * viewportWidth / WC1_SDL_FRAME_WIDTH;
    *windowY = viewportTop + logicalY * viewportHeight / WC1_SDL_FRAME_HEIGHT;
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
    *logicalX = (windowX - viewportLeft) * WC1_SDL_FRAME_WIDTH / viewportWidth;
    *logicalY =
        (windowY - viewportTop) * WC1_SDL_FRAME_HEIGHT / viewportHeight;
    return 1;
}
