/* Exercises the real DrawTextAt, print_message_text and viewport-copy paths
 * against the player's FONTS.FNT, with the overlay's CPU rendering exposed.
 * No GL calls are made; a sentinel context enables recording. */
#include "../src/sdl/gl_renderer.c"

static void RequireHudFont(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static void CheckHudGlyphs(const unsigned char *pixels)
{
    BuildHiResLayer(pixels);
    RequireHudFont(g_renderer.hiResLayerVisible && g_nHiResGlyphCount != 0,
                   "text reaches the hi-res layer");
    RequireHudFont(g_pHiResFont[0] == 11,
                   "the overlay uses the larger 11-pixel font");
}

static void CheckHudMessageRedraw(unsigned char *pixels, char *message)
{
    unsigned char *layer = malloc(sizeof(g_renderer.hiResLayer));
    unsigned char background[SDL_FRAME_WIDTH * SDL_FRAME_HEIGHT];
    GlHiResGlyph *first;
    char *text;
    unsigned int count;
    int reply;
    int refresh;
    int pixel;
    int left;
    int right;
    int y;

    RequireHudFont(layer != 0, "allocate a message redraw snapshot");
    SdlSetHiResHudFonts(0);
    SdlSetHiResHudFonts(1);
    g_pHudMessageFrameShape = LoadNamedPacket("GAMEDAT/MESSAGE.V00", 0, 0, 0, 0, 1);
    RequireHudFont(g_pHudMessageFrameShape != 0, "load the real message background shape");
    g_pHudMessageBackground = AllocateTaggedMemory(
        MeasureShapeFrameStorage(g_pHudMessageFrameShape, 0), 0);
    for (reply = 0; reply < 2; reply++) {
        /* A shorter response moves both ends. Exercise black space and a
         * patterned cockpit background through the real capture/restore. */
        text = message + reply * 2;
        for (pixel = 0; pixel < (int)sizeof(background); pixel++)
            background[pixel] = reply ? (unsigned char)(1 + pixel % 7) : 0;
        memcpy(pixels, background, sizeof(background));
        g_bCaptureHudMessageBackground = 1;
        SetHudTextColour(text, 42);
        CheckHudGlyphs(pixels);
        count = g_nHiResGlyphCount;
        RequireHudFont(count == strlen(text), "a reply contains only its own glyphs");
        memcpy(layer, g_renderer.hiResLayer, sizeof(g_renderer.hiResLayer));
        /* A cockpit frame refresh can leave the ends of a subtitle untouched.
         * Draw the same message after restoring only the middle of its row. */
        for (refresh = 0; refresh < 4; refresh++) {
            left = g_aHiResGlyphs[0].x + g_aHiResGlyphs[0].width;
            right = g_aHiResGlyphs[count - 1].x;
            if (refresh & 1) {
                left -= g_aHiResGlyphs[0].width / 2;
                right += g_aHiResGlyphs[count - 1].width / 2;
            }
            for (y = 0; y < SDL_FRAME_HEIGHT; y++)
                memcpy(pixels + y * SDL_FRAME_WIDTH + left,
                       background + y * SDL_FRAME_WIDTH + left, (size_t)(right - left));
            SetHudTextColour(text, 42);
            CheckHudGlyphs(pixels);
            RequireHudFont(g_nHiResGlyphCount == count &&
                           !memcmp(layer, g_renderer.hiResLayer, sizeof(g_renderer.hiResLayer)),
                           "redrawing a subtitle preserves its first and last glyphs");
        }
        /* A newly rendered scene pixel must replace the older background
         * even where the previous frame contained subtitle ink. */
        first = &g_aHiResGlyphs[0];
        for (pixel = 0; pixel < first->width * first->height; pixel++) {
            if (first->before[pixel] != first->after[pixel])
                break;
        }
        RequireHudFont(pixel < first->width * first->height, "find subtitle ink");
        pixels[(first->y + pixel / first->width) * SDL_FRAME_WIDTH +
               first->x + pixel % first->width] = 43;
        SetHudTextColour(text, 42);
        CheckHudGlyphs(pixels);
        RequireHudFont(g_aHiResGlyphs[0].before[pixel] == 43,
                       "a refreshed scene pixel remains in the subtitle background");
        ClearHudMessageDisplay(0);
        BuildHiResLayer(pixels);
        RequireHudFont(!g_renderer.hiResLayerVisible && g_nHiResGlyphCount == 0,
                       "erasing a message removes all of its recorded glyphs");
        RequireHudFont(!memcmp(pixels, background, sizeof(background)),
                       "the message background is restored by the game");
    }
    ReleasePacketHandle(g_pHudMessageBackground);
    ReleasePacketHandle(g_pHudMessageFrameShape);
    g_pHudMessageBackground = 0;
    g_pHudMessageFrameShape = 0;
    g_pszPendingHudMessage = 0;
    g_pszDisplayedHudMessage = 0;
    free(layer);
}

static void CheckHudCursor(unsigned char *pixels)
{
    /* One 3x3 cross with hotspot (1,1), transparent corners, black arms
     * and a bright centre. Exercise both literal and repeated RLE spans. */
    static const unsigned char cursorShape[] = {
        42, 0, 0, 0, 8, 0, 0, 0,
        1, 0, 1, 0, 1, 0, 1, 0,
        3, 0, 0, 0, 255, 255, 3, 200,
        6, 0, 255, 255, 0, 0, 0, 200, 0,
        2, 0, 0, 0, 1, 0, 200,
        0, 0
    };
    static const ShortPoint positions[] = {
        {35, 45}, {36, 45}, {37, 45}, {60, 100},
        {0, 0}, {319, 0}, {0, 199}, {319, 199}
    };
    unsigned char snapshot[SDL_FRAME_WIDTH * SDL_FRAME_HEIGHT];
    unsigned char *layer = malloc(sizeof(g_renderer.hiResLayer));
    unsigned char *shape = AllocateTaggedMemory(sizeof(cursorShape), 0x40);
    unsigned int glyphCount = g_nHiResGlyphCount;
    size_t position;
    size_t offset;
    int x;
    int y;
    int dx;
    int dy;
    int covered;
    int sameColourOcclusion = 0;
    unsigned char expected;

    RequireHudFont(layer != 0 && shape != 0, "allocate cursor fixtures");
    memcpy(shape, cursorShape, sizeof(cursorShape));
    memcpy(snapshot, pixels, sizeof(snapshot));
    memcpy(layer, g_renderer.hiResLayer, sizeof(g_renderer.hiResLayer));
    g_nMouseCursorDrawDepth = 1;
    g_16_stEventQueuePointerData.viewport = &g_101_stScrViewInfo;
    g_16_stEventQueuePointerData.shape = shape;
    g_16_stEventQueuePointerData.frame = 0;
    for (position = 0; position < SDL_arraysize(positions); position++) {
        g_16_stEventQueuePointerData.x = positions[position].x;
        g_16_stEventQueuePointerData.y = positions[position].y;
        CaptureMouseCursorBackground();
        DrawMouseCursor();
        /* The pointer may advance again after capture: the layer must use
         * the position of the cursor actually drawn into this frame. */
        g_16_stEventQueuePointerData.x = 100;
        g_16_stEventQueuePointerData.y = 100;
        CheckHudGlyphs(pixels);
        RequireHudFont(g_nHiResGlyphCount == glyphCount,
                       "cursor overlap does not discard nav glyphs");
        for (y = 0; y < GL_HIRES_HEIGHT; y++) {
            for (x = 0; x < GL_HIRES_WIDTH; x++) {
                dx = x / 2 - positions[position].x;
                dy = y / 2 - positions[position].y;
                covered = (dx == 0 && dy >= -1 && dy <= 1) ||
                    (dy == 0 && dx >= -1 && dx <= 1);
                offset = ((size_t)y * GL_HIRES_WIDTH + x) * 2;
                if (covered) {
                    expected = dx == 0 ? 200 : 0;
                    RequireHudFont(pixels[(y / 2) * SDL_FRAME_WIDTH + x / 2] == expected,
                                   "the real cursor draw matches the fixture");
                    RequireHudFont(g_renderer.hiResLayer[offset] == expected &&
                                   g_renderer.hiResLayer[offset + 1] == 0xff,
                                   "the cursor stays above nav text, including at screen edges");
                    if (expected == snapshot[(y / 2) * SDL_FRAME_WIDTH + x / 2] &&
                        layer[offset + 1] != 0 && layer[offset] != expected)
                        sameColourOcclusion = 1;
                } else {
                    RequireHudFont(!memcmp(g_renderer.hiResLayer + offset,
                                           layer + offset, 2),
                                   "transparent cursor areas preserve hi-res text");
                }
            }
        }
        RestoreMouseCursorBackground();
        RequireHudFont(!memcmp(pixels, snapshot, sizeof(snapshot)),
                       "cursor restoration preserves the game's framebuffer");
        CheckHudGlyphs(pixels);
        RequireHudFont(!memcmp(g_renderer.hiResLayer, layer,
                               sizeof(g_renderer.hiResLayer)),
                       "moving or hiding the cursor leaves no overlay trails");
    }
    RequireHudFont(sameColourOcclusion,
                   "opaque cursor pixels matching the background still cover text");
    g_nMouseCursorDrawDepth = 0;
    g_16_stEventQueuePointerData.shape = 0;
    ReleasePacketHandle(GetPreparedShapeData(shape));
    ReleasePacketHandle(shape);
    free(layer);
}

static void CheckBlankHudFallback(unsigned char *pixels)
{
    unsigned char *font = g_36_apTextGlyphs[2];
    unsigned char saved[3] = {font[4 + 0xe1], font[260 + 0xe1], font[516 + 0xe1]};
    unsigned char snapshot[SDL_FRAME_WIDTH * SDL_FRAME_HEIGHT];
    TextContext text = {0};
    int width;
    int height;

    RequireHudFont(FindHiResGlyph(0xe1, &width, &height) == 0,
                   "an unused but structurally valid blank glyph is unavailable");
    RequireHudFont(FindHiResGlyph(' ', &width, &height) != 0,
                   "spaces remain valid replacement glyphs");
    /* Simulate a partial translation with a visible small-font character
     * and no corresponding replacement in the larger font. */
    font[4 + 0xe1] = font[4 + 'A'];
    font[260 + 0xe1] = font[260 + 'A'];
    font[516 + 0xe1] = font[516 + 'A'];
    SdlSetHiResHudFonts(0);
    SdlSetHiResHudFonts(1);
    memset(pixels, 0, sizeof(snapshot));
    text.viewport = &g_101_stScrViewInfo;
    text.font = font;
    text.colour = 42;
    SetTextContext(&text);
    SetTextCursor(20, 20);
    DrawFormattedText("\xe1");
    memcpy(snapshot, pixels, sizeof(snapshot));
    RequireHudFont(memchr(pixels, text.colour, sizeof(snapshot)) != 0,
                   "the source font supplies a visible fallback");
    BuildHiResLayer(pixels);
    RequireHudFont(g_nHiResGlyphCount == 0 && !g_renderer.hiResLayerVisible &&
                   !memcmp(pixels, snapshot, sizeof(snapshot)),
                   "a blank replacement leaves the native glyph visible");
    font[4 + 0xe1] = saved[0];
    font[260 + 0xe1] = saved[1];
    font[516 + 0xe1] = saved[2];
}

static void CheckTranslatedHudFonts(const char *catalogPath,
                                    const unsigned char *characters,
                                    unsigned char *pixels)
{
    TextContext text = {0};
    const unsigned char *character;
    unsigned char *font;
    unsigned char snapshot[SDL_FRAME_WIDTH * SDL_FRAME_HEIGHT];
    long sizes[3];
    size_t pixel;
    int section;
    int width;
    int height;
    int offset;
    int ink;
    int renderedInk;
    char letter[2] = {0, 0};

    RequireHudFont(SdlLoadTranslationCatalog(catalogPath), "load a translation catalog");
    RequireHudFont(g_pHiResFont == 0 && g_nHiResGlyphCount == 0 &&
                   g_nHiResOffscreenGlyphCount == 0 && !g_renderer.hiResLayerVisible,
                   "changing catalogs invalidates the font and recorded text");
    for (section = 0; section < 3; section++) {
        free(g_36_apTextGlyphs[section]);
        sizes[section] = GetNamedPacketSize("GAMEDAT/FONTS.FNT", (short)section);
        RequireHudFont(sizes[section] >= 772 && sizes[section] <= 65535,
                       "get the translated font packet size");
        font = malloc((size_t)sizes[section]);
        RequireHudFont(font != 0 && LoadNamedPacket("GAMEDAT/FONTS.FNT",
                       (short)section, font, 0, 0, 0) == font,
                       "load the game font through the translation pipeline");
        g_36_apTextGlyphs[section] = font;
        height = font[0] | (font[1] << 8);
        for (character = characters; *character; character++) {
            width = font[4 + *character];
            offset = font[260 + *character] | (font[516 + *character] << 8);
            RequireHudFont(width > 0 && offset >= 772 &&
                           offset + width * height <= sizes[section],
                           "every translated character has a valid glyph in all three fonts");
            ink = 0;
            for (pixel = 0; pixel < (size_t)width * height; pixel++)
                ink += font[offset + pixel] != font[3] && font[offset + pixel] != 0xff;
            RequireHudFont(ink > 0, "every translated character has visible ink");
        }
    }
    g_nPacketError = 7;
    RequireHudFont(LoadHiResFont() && g_nPacketError == 7,
                   "auxiliary font loading preserves the game's packet error state");
    RequireHudFont(g_nHiResFontSize == (size_t)sizes[0] &&
                   !memcmp(g_pHiResFont, g_36_apTextGlyphs[0], g_nHiResFontSize),
                   "the overlay receives the same translated font as the game");
    /* The overlay must own its copy even when a scene releases its font. */
    free(g_36_apTextGlyphs[0]);
    g_36_apTextGlyphs[0] = 0;
    text.viewport = &g_101_stScrViewInfo;
    text.font = g_36_apTextGlyphs[2];
    text.colour = 42;
    for (character = characters; *character; character++) {
        SdlSetHiResHudFonts(0);
        SdlSetHiResHudFonts(1);
        memset(pixels, 0, sizeof(snapshot));
        letter[0] = (char)*character;
        SetTextContext(&text);
        SetTextCursor(20, 20);
        DrawFormattedText("%s", letter);
        memcpy(snapshot, pixels, sizeof(snapshot));
        CheckHudGlyphs(pixels);
        RequireHudFont(g_nHiResGlyphCount == 1 &&
                       g_aHiResGlyphs[0].character == *character &&
                       !memcmp(pixels, snapshot, sizeof(snapshot)),
                       "accented cockpit text reaches the overlay without changing the base");
        font = g_pHiResFont;
        width = font[4 + *character];
        height = font[0] | (font[1] << 8);
        offset = font[260 + *character] | (font[516 + *character] << 8);
        ink = 0;
        renderedInk = 0;
        for (pixel = 0; pixel < (size_t)width * height; pixel++)
            ink += font[offset + pixel] != font[3] && font[offset + pixel] != 0xff;
        for (pixel = 0; pixel < sizeof(g_renderer.hiResLayer); pixel += 2)
            renderedInk += g_renderer.hiResLayer[pixel + 1] != 0 &&
                g_renderer.hiResLayer[pixel] == text.colour;
        RequireHudFont(renderedInk == ink,
                       "all translated glyph ink is rendered in the larger font");
    }
    SdlSetHiResHudFonts(0);
    SdlSetHiResHudFonts(1);
    memset(pixels, 0, sizeof(snapshot));
    g_3_stDefaultStringContext = text;
    g_3_stDefaultStringContext.font = g_36_apTextGlyphs[1];
    g_168_stFlightTextContext = g_3_stDefaultStringContext;
    print_message_text((char *)characters, text.colour);
    CheckHudGlyphs(pixels);
    RequireHudFont(g_nHiResGlyphCount == strlen((const char *)characters),
                   "radio subtitles capture every translated character");
    CheckHudMessageRedraw(pixels, (char *)characters);
}

int main(int argc, char **argv)
{
    unsigned char pixels[320 * 200] = {0};
    unsigned char snapshot[320 * 200];
    unsigned char offscreen[80 * 40] = {0};
    unsigned short rows[202];
    unsigned short offscreenRows[42];
    GuardedAllocation pixelAllocation = {offscreen, sizeof(offscreen), 0};
    GuardedAllocation rowAllocation = {offscreenRows, sizeof(offscreenRows), &pixelAllocation};
    Viewport source = {0};
    Viewport destination;
    TextContext text = {0};
    unsigned char *archive;
    unsigned char *bitmap;
    size_t archiveSize;
    size_t fontSize;
    char path[4096];
    int index;
    int x;
    int y;
    int width;
    int height;
    int offset;
    unsigned char expected;

    RequireHudFont(argc == 4 && _chdir(argv[1]) == 0,
                   "pass a WC2 installation directory and the Spanish/Polish catalogs");
    RequireHudFont(SdlResolvePath("GAMEDAT/FONTS.FNT", path, sizeof(path)),
                   "find the installed font archive");
    archive = SDL_LoadFile(path, &archiveSize);
    RequireHudFont(archive != 0, "read the font archive");
    for (index = 0; index < 3; index++) {
        RequireHudFont(SdlExtractOriginPacketSection(archive, archiveSize, index,
                        &g_36_apTextGlyphs[index], &fontSize) && fontSize >= 772,
                       "decode a game font");
    }
    SDL_free(archive);
    for (index = 0; index < 202; index++)
        rows[index] = (unsigned short)(index * 320);
    for (index = 0; index < 42; index++)
        offscreenRows[index] = (unsigned short)(index * 80);
    for (index = 0; index < 256; index++)
        g_abPaletteTranslation[index] = (unsigned char)index;
    g_101_stScrViewInfo.pixels = pixels;
    g_101_stScrViewInfo.rowOffsets = rows;
    g_101_stScrViewInfo.right = 319;
    g_101_stScrViewInfo.bottom = 199;
    g_28_nDisplayRowBytes = 320;
    g_38_nSurfaceHeight = 200;
    g_1_bSpaceFlightActive = 1;
    g_renderer.context = (SDL_GLContext)1;
    g_renderer.overlayProgram = 1;
    SdlSetHiResHudFonts(1);

    text.viewport = &g_101_stScrViewInfo;
    text.font = g_36_apTextGlyphs[2];
    text.colour = 42;
    SetTextContext(&text);
    SetTextCursor(20, 20);
    DrawFormattedText("8");
    memcpy(snapshot, pixels, sizeof(pixels));
    CheckHudGlyphs(pixels);
    RequireHudFont(!memcmp(snapshot, pixels, sizeof(pixels)),
                   "the overlay leaves the game's framebuffer unchanged");
    /* The original larger glyph occupies rows 40..50 inside the doubled
     * 6-pixel source cell. Compare with section 0's own bitmap. */
    width = g_36_apTextGlyphs[0][4 + '8'];
    height = 11;
    offset = g_36_apTextGlyphs[0][0x104 + '8'] |
        (g_36_apTextGlyphs[0][0x204 + '8'] << 8);
    bitmap = g_36_apTextGlyphs[0] + offset;
    for (y = 0; y < 12; y++) {
        for (x = 0; x < g_36_apTextGlyphs[2][4 + '8'] * 2; x++) {
            expected = 0;
            if (x < width && y < height) {
                expected = bitmap[y * width + x];
                if (expected == 0xff || expected == g_36_apTextGlyphs[0][3])
                    expected = 0;
                else if (expected == g_36_apTextGlyphs[0][2])
                    expected = text.colour;
            }
            offset = ((40 + y) * GL_HIRES_WIDTH + 40 + x) * 2;
            if (g_renderer.hiResLayer[offset + 1] != 0xff ||
                g_renderer.hiResLayer[offset] != expected)
                fprintf(stderr, "digit pixel %d,%d: got %u/%u expected %u/255; cell %d,%d %ux%u\n",
                        x, y, g_renderer.hiResLayer[offset],
                        g_renderer.hiResLayer[offset + 1], expected,
                        g_aHiResGlyphs[0].x, g_aHiResGlyphs[0].y,
                        g_aHiResGlyphs[0].width, g_aHiResGlyphs[0].height);
            RequireHudFont(g_renderer.hiResLayer[offset + 1] == 0xff &&
                           g_renderer.hiResLayer[offset] == expected,
                           "gauge digit is drawn in the native 11-pixel font");
        }
    }
    pixels[20 * 320 + 20] ^= 1;
    BuildHiResLayer(pixels);
    RequireHudFont(!g_renderer.hiResLayerVisible && g_nHiResGlyphCount == 0,
                   "overwritten text drops out of the overlay");

    /* The communications menu shares the small cockpit font. */
    DrawTextAt(&text, 40, 40, "VID-COM SYSTEM", 2);
    CheckHudGlyphs(pixels);
    SdlSetHiResHudFonts(0);
    SdlSetHiResHudFonts(1);

    /* The subtitle routine copies its text context to a local variable and
     * uses the cockpit's message origin plus the current view's offset. */
    memset(pixels, 0, sizeof(pixels));
    g_3_stDefaultStringContext = text;
    g_3_stDefaultStringContext.font = g_36_apTextGlyphs[1];
    g_168_stFlightTextContext = g_3_stDefaultStringContext;
    g_stHudMessageOrigin.x = 12;
    g_stHudMessageOrigin.y = 10;
    g_49_nViewportOffsetY = 5;
    print_message_text("Shadow: Returning to base.", 42);
    CheckHudGlyphs(pixels);
    CheckHudMessageRedraw(pixels, "Shadow: Returning to base.");
    SdlSetHiResHudFonts(0);
    SdlSetHiResHudFonts(1);

    /* Other uses of the 8-pixel font must stay at their original scale. */
    text.font = g_36_apTextGlyphs[1];
    DrawTextAt(&text, 155, 2, "Nav description", 0);
    DrawTextAt(&text, 20, 80, "Modal menu", 2);
    RequireHudFont(g_nHiResGlyphCount == 0,
                   "nav descriptions and modal menus keep their font");

    g_pGuardedAllocationHead = &rowAllocation;
    g_9_pGuardedAllocationLastRef = &pixelAllocation;
    g_apViewportAllocations[0] = offscreen;
    g_nViewportAllocationCount = 1;
    source.pixels = offscreen;
    source.rowOffsets = offscreenRows;
    source.right = 79;
    source.bottom = 39;
    text.viewport = &source;
    text.font = g_36_apTextGlyphs[2];
    SetTextContext(&text);
    SetTextCursor(4, 4);
    DrawFormattedText("NAV 1");
    RequireHudFont(g_nHiResOffscreenGlyphCount == 5, "record an offscreen nav label");
    destination = g_101_stScrViewInfo;
    destination.left = 30;
    destination.top = 40;
    destination.right = 109;
    destination.bottom = 79;
    CopyViewportContents(&source, &destination);
    CheckHudGlyphs(pixels);
    RequireHudFont(g_aHiResGlyphs[0].x == 34 && g_aHiResGlyphs[0].y == 44,
                   "nav labels follow the viewport copy");
    CheckHudCursor(pixels);

    SdlSetHiResHudFonts(0);
    DrawTextAt(&text, 4, 14, "NAV 2", 0);
    RequireHudFont(g_nHiResGlyphCount == 0 && g_nHiResOffscreenGlyphCount == 0,
                   "disabling the option stops recording");
    SdlSetHiResHudFonts(1);
    g_1_bSpaceFlightActive = 0;
    DrawTextAt(&text, 4, 24, "NAV 3", 0);
    RequireHudFont(g_nHiResOffscreenGlyphCount == 0,
                   "non-flight screens keep their original text");
    g_1_bSpaceFlightActive = 1;
    CheckBlankHudFallback(pixels);
    /* Full accented alphabets used by these catalogs, in CP1252/CP1250. */
    CheckTranslatedHudFonts(argv[2], (const unsigned char *)
        "\xa1\xbf\xd1\xe1\xe9\xed\xf1\xf3\xfa\xfc", pixels);
    CheckTranslatedHudFonts(argv[3], (const unsigned char *)
        "\x8c\x8f\x9c\x9f\xa3\xa5\xaf\xb3\xb9\xbf\xc6\xca\xd1\xd3\xe6\xea\xf1\xf3", pixels);
    SdlShutdownTranslations();
    RequireHudFont(LoadHiResFont() && FindHiResGlyph(0xe1, &width, &height) == 0,
                   "returning to English clears translated glyphs from the cache");
    for (index = 0; index < 3; index++)
        free(g_36_apTextGlyphs[index]);
    SdlResetHiResHudFont();
    puts("HUD fonts: larger text, subtitle redraws, cursor overlap, copying, translated glyphs and blank fallback passed.");
    return 0;
}
