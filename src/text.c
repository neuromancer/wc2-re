/*
 *  Cinematic sprite-font renderer.
 *
 *  Retail object 0x433690-0x433ACF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x433690 */
void DrawCinematicFontCharacter(char character)
{
    ShortRect bounds;
    short frame;

#ifdef SDL_PORT
    character = (char)SdlCinematicFontCharacter(character);
#endif
    if (IsCinematicFontCharacterPrintable((short)(signed char)character) == 0) {
        g_29_pCurrentTextContext->cursorX =
            (short)(g_29_pCurrentTextContext->cursorX + 5);
        return;
    }
    frame = (short)((signed char)character - 0x21);
    GetShapeFrameBounds(&bounds.left, 0, 0,
                        g_29_pCurrentTextContext->font, frame);
    DrawSpriteDefault(g_29_pCurrentTextContext->viewport,
                      g_29_pCurrentTextContext->cursorX,
                      g_29_pCurrentTextContext->cursorY,
                      g_29_pCurrentTextContext->font, frame);
    g_29_pCurrentTextContext->cursorX = (short)(
        bounds.right - bounds.left +
        g_29_pCurrentTextContext->cursorX + 2);
}

/* Function start: 0x43374E */
short IsCinematicFontCharacterPrintable(short character)
{
    if (character < 0x21)
        return 0;
    return 1;
}

/* Function start: 0x433777 */
void DrawWrappedCinematicText(char *text)
{
    char *lineStart;
    char *end;
    short terminator;
    short usedWidth;
    short viewportWidth;
    short advance;
    short ch;

    lineStart = text;
    viewportWidth = (short)(
        g_29_pCurrentTextContext->viewport->right -
        g_29_pCurrentTextContext->viewport->left + 1);
    while (*lineStart != 0) {
        usedWidth = 0;
        end = lineStart;
        while ((ch = (signed char)*end) != 0) {
            if (ch == '\n')
                break;
            advance = MeasureCinematicFontCharacterWidth(ch);
            usedWidth = (short)(usedWidth + advance);
            if (viewportWidth <= usedWidth) {
#ifdef SDL_PORT
                if (isalnum((unsigned char)ch) != 0) {
                    while (isalpha((unsigned char)ch) != 0) {
#else
                if (isalnum(ch) != 0) {
                    while (isalpha(ch) != 0) {
#endif
                        usedWidth = (short)(usedWidth - advance);
                        end--;
                        ch = (signed char)*end;
                        advance =
                            MeasureCinematicFontCharacterWidth(ch);
                    }
                } else {
                    usedWidth = (short)(usedWidth - advance);
                }
                break;
            }
            end++;
        }
        terminator = ch;
        *end = 0;
        switch (g_29_pCurrentTextContext->alignment) {
        case 0:
            g_29_pCurrentTextContext->cursorX =
                g_29_pCurrentTextContext->viewport->left;
            break;
        case 1:
            g_29_pCurrentTextContext->cursorX = (short)(
                g_29_pCurrentTextContext->viewport->right -
                usedWidth + 1);
            break;
        case 2:
            g_29_pCurrentTextContext->cursorX = (short)(
                g_29_pCurrentTextContext->viewport->left +
                (viewportWidth - usedWidth) / 2);
            break;
        }
        while (*lineStart != 0)
            DrawCinematicFontCharacter(*lineStart++);
        *lineStart = (char)terminator;
        if (terminator == '\n') {
            g_29_pCurrentTextContext->cursorY = (short)(
                g_29_pCurrentTextContext->cursorY +
                MeasureCinematicFontLineHeight() + 1);
            lineStart++;
        }
    }
}

/* Function start: 0x433A14 */
short MeasureCinematicFontCharacterWidth(short character)
{
    ShortRect bounds;

#ifdef SDL_PORT
    character = SdlCinematicFontCharacter(character);
#endif
    if (IsCinematicFontCharacterPrintable(character) == 0)
        return 5;
    GetShapeFrameBounds(&bounds.left, 0, 0,
                        g_29_pCurrentTextContext->font,
                        (short)(character - 0x21));
    return (short)(bounds.right - bounds.left + 2);
}

/* Function start: 0x433A75 */
short MeasureCinematicFontLineHeight(void)
{
    ShortRect bounds;

    GetShapeFrameBounds(&bounds.left, 0, 0,
                        g_29_pCurrentTextContext->font, 0);
    return (short)(bounds.bottom - bounds.top + 2);
}

/* Function start: 0x433AB0 */
void InitializeCinematicTextRenderer(void)
{
    g_36_pfnRenderSceneFontCharacterHandler =
        DrawCinematicFontCharacter;
    g_3_pfnRenderSceneTextStringHandler = DrawWrappedCinematicText;
}
