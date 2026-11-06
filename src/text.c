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

    if (IsCinematicFontCharacterPrintable((short)(signed char)character) == 0) {
        g_pCurrentTextContext_005c8d1c->cursorX =
            (short)(g_pCurrentTextContext_005c8d1c->cursorX + 5);
        return;
    }
    frame = (short)((signed char)character - 0x21);
    GetShapeFrameBounds(&bounds.left, 0, 0,
                        g_pCurrentTextContext_005c8d1c->font, frame);
    DrawSpriteDefault(g_pCurrentTextContext_005c8d1c->viewport,
                      g_pCurrentTextContext_005c8d1c->cursorX,
                      g_pCurrentTextContext_005c8d1c->cursorY,
                      g_pCurrentTextContext_005c8d1c->font, frame);
    g_pCurrentTextContext_005c8d1c->cursorX = (short)(
        bounds.right - bounds.left +
        g_pCurrentTextContext_005c8d1c->cursorX + 2);
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
        g_pCurrentTextContext_005c8d1c->viewport->right -
        g_pCurrentTextContext_005c8d1c->viewport->left + 1);
    while (*lineStart != 0) {
        usedWidth = 0;
        end = lineStart;
        while ((ch = (signed char)*end) != 0) {
            if (ch == '\n')
                break;
            advance = MeasureCinematicFontCharacterWidth(ch);
            usedWidth = (short)(usedWidth + advance);
            if (viewportWidth <= usedWidth) {
                if (isalnum(ch) != 0) {
                    while (isalpha(ch) != 0) {
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
        switch (g_pCurrentTextContext_005c8d1c->alignment) {
        case 0:
            g_pCurrentTextContext_005c8d1c->cursorX =
                g_pCurrentTextContext_005c8d1c->viewport->left;
            break;
        case 1:
            g_pCurrentTextContext_005c8d1c->cursorX = (short)(
                g_pCurrentTextContext_005c8d1c->viewport->right -
                usedWidth + 1);
            break;
        case 2:
            g_pCurrentTextContext_005c8d1c->cursorX = (short)(
                g_pCurrentTextContext_005c8d1c->viewport->left +
                (viewportWidth - usedWidth) / 2);
            break;
        }
        while (*lineStart != 0)
            DrawCinematicFontCharacter(*lineStart++);
        *lineStart = (char)terminator;
        if (terminator == '\n') {
            g_pCurrentTextContext_005c8d1c->cursorY = (short)(
                g_pCurrentTextContext_005c8d1c->cursorY +
                MeasureCinematicFontLineHeight() + 1);
            lineStart++;
        }
    }
}

/* Function start: 0x433A14 */
short MeasureCinematicFontCharacterWidth(short character)
{
    ShortRect bounds;

    if (IsCinematicFontCharacterPrintable(character) == 0)
        return 5;
    GetShapeFrameBounds(&bounds.left, 0, 0,
                        g_pCurrentTextContext_005c8d1c->font,
                        (short)(character - 0x21));
    return (short)(bounds.right - bounds.left + 2);
}

/* Function start: 0x433A75 */
short MeasureCinematicFontLineHeight(void)
{
    ShortRect bounds;

    GetShapeFrameBounds(&bounds.left, 0, 0,
                        g_pCurrentTextContext_005c8d1c->font, 0);
    return (short)(bounds.bottom - bounds.top + 2);
}

/* Function start: 0x433AB0 */
void InitializeCinematicTextRenderer(void)
{
    g_pfnRenderSceneFontCharacterHandler_005c8d30 =
        DrawCinematicFontCharacter;
    g_pfnRenderSceneTextStringHandler_005c8cf4 = DrawWrappedCinematicText;
}
