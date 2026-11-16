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

static void CheckHudGlyphs(const unsigned char *pixels, int font)
{
    unsigned int index;

    BuildHiResLayer(pixels);
    RequireHudFont(g_renderer.hiResLayerVisible && g_nHiResGlyphCount != 0,
                   "text reaches the hi-res layer");
    for (index = 0; index < g_nHiResGlyphCount; index++)
        RequireHudFont(g_aHiResGlyphs[index].font == font,
                       "each glyph uses the font for its text area");
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

    RequireHudFont(argc == 2 && _chdir(argv[1]) == 0,
                   "pass a WC2 installation directory");
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
    CheckHudGlyphs(pixels, GL_HIRES_COMPACT_FONT);
    RequireHudFont(!memcmp(snapshot, pixels, sizeof(pixels)),
                   "the overlay leaves the game's framebuffer unchanged");
    /* An 8-pixel glyph occupies rows 42..49 inside the doubled 6-pixel
     * source cell. Compare the rendered pixels with section 1's own bitmap. */
    width = g_36_apTextGlyphs[1][4 + '8'];
    height = 8;
    offset = g_36_apTextGlyphs[1][0x104 + '8'] |
        (g_36_apTextGlyphs[1][0x204 + '8'] << 8);
    bitmap = g_36_apTextGlyphs[1] + offset;
    for (y = 0; y < 12; y++) {
        for (x = 0; x < g_36_apTextGlyphs[2][4 + '8'] * 2; x++) {
            expected = 0;
            if (x < width && y >= 2 && y < 2 + height) {
                expected = bitmap[(y - 2) * width + x];
                if (expected == 0xff || expected == g_36_apTextGlyphs[1][3])
                    expected = 0;
                else if (expected == g_36_apTextGlyphs[1][2])
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
                           "gauge digit is drawn in the native 8-pixel font");
        }
    }
    pixels[20 * 320 + 20] ^= 1;
    BuildHiResLayer(pixels);
    RequireHudFont(!g_renderer.hiResLayerVisible && g_nHiResGlyphCount == 0,
                   "overwritten text drops out of the overlay");

    /* The communications menu shares the small cockpit font. */
    DrawTextAt(&text, 40, 40, "VID-COM SYSTEM", 2);
    CheckHudGlyphs(pixels, GL_HIRES_COMPACT_FONT);
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
    CheckHudGlyphs(pixels, GL_HIRES_MESSAGE_FONT);
    RequireHudFont(g_apHiResFonts[0][0] == 11,
                   "HUD messages load the 11-pixel conversation font");
    SdlSetHiResHudFonts(0);
    SdlSetHiResHudFonts(1);

    /* Other uses of the 8-pixel font must stay at their original scale. */
    text.font = g_36_apTextGlyphs[1];
    DrawTextAt(&text, 155, 2, "Nav description", 0);
    DrawTextAt(&text, 20, 80, "Modal menu", 2);
    RequireHudFont(g_nHiResGlyphCount == 0,
                   "nav descriptions and modal menus keep their font");

    g_pGuardedAllocationHead = &rowAllocation;
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
    CheckHudGlyphs(pixels, GL_HIRES_COMPACT_FONT);
    RequireHudFont(g_aHiResGlyphs[0].x == 34 && g_aHiResGlyphs[0].y == 44,
                   "nav labels follow the viewport copy");

    SdlSetHiResHudFonts(0);
    DrawTextAt(&text, 4, 14, "NAV 2", 0);
    RequireHudFont(g_nHiResGlyphCount == 0 && g_nHiResOffscreenGlyphCount == 0,
                   "disabling the option stops recording");
    SdlSetHiResHudFonts(1);
    g_1_bSpaceFlightActive = 0;
    DrawTextAt(&text, 4, 24, "NAV 3", 0);
    RequireHudFont(g_nHiResOffscreenGlyphCount == 0,
                   "non-flight screens keep their original text");
    for (index = 0; index < 3; index++)
        free(g_36_apTextGlyphs[index]);
    for (index = 0; index < 2; index++)
        free(g_apHiResFonts[index]);
    puts("HUD fonts: compact gauges/VDUs/nav labels, message subtitles, copying and fallback passed.");
    return 0;
}
