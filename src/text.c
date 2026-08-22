/*
 *  Cockpit information display.
 *
 *  Address range 0x433690-0x433abf (provisional -- see docs/ORDER.md).
 *  WC2 adds the cinematic sprite-font renderer before show_info_disp.
 *  The DirectDraw error switch and its generated lookup tables end at
 *  0x43390f; the Mac symbol order proves that the ship-AI `smart` unit starts
 *  at 0x433ac0.
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
        g_pCurrentTextContext_005c8d1c->cursorX +
        bounds.right - bounds.left + 2);
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
    char *cursor;
    char *lineEnd;
    short viewportWidth;
    short lineWidth;
    short character;
    short characterWidth;
    char savedCharacter;

    cursor = text;
    viewportWidth = (short)(
        g_pCurrentTextContext_005c8d1c->viewport->right -
        g_pCurrentTextContext_005c8d1c->viewport->left + 1);
    while (*cursor != 0) {
        lineWidth = 0;
        lineEnd = cursor;
        for (;;) {
            character = (signed char)*lineEnd;
            if (character == 0 || character == '\n')
                break;
            characterWidth = MeasureCinematicFontCharacterWidth(character);
            lineWidth = (short)(lineWidth + characterWidth);
            if (viewportWidth <= lineWidth) {
                if (isalnum(character) != 0) {
                    while (isalpha(character) != 0) {
                        lineWidth = (short)(lineWidth - characterWidth);
                        lineEnd--;
                        character = (signed char)*lineEnd;
                        characterWidth =
                            MeasureCinematicFontCharacterWidth(character);
                    }
                } else {
                    lineWidth = (short)(lineWidth - characterWidth);
                }
                break;
            }
            lineEnd++;
        }
        savedCharacter = (char)character;
        *lineEnd = 0;
        switch (g_pCurrentTextContext_005c8d1c->alignment) {
        case 0:
            g_pCurrentTextContext_005c8d1c->cursorX =
                g_pCurrentTextContext_005c8d1c->viewport->left;
            break;
        case 1:
            g_pCurrentTextContext_005c8d1c->cursorX = (short)(
                g_pCurrentTextContext_005c8d1c->viewport->right -
                lineWidth + 1);
            break;
        case 2:
            g_pCurrentTextContext_005c8d1c->cursorX = (short)(
                g_pCurrentTextContext_005c8d1c->viewport->left +
                (viewportWidth - lineWidth) / 2);
            break;
        }
        while (*cursor != 0)
            DrawCinematicFontCharacter(*cursor++);
        *cursor = savedCharacter;
        if (savedCharacter == '\n') {
            g_pCurrentTextContext_005c8d1c->cursorY = (short)(
                g_pCurrentTextContext_005c8d1c->cursorY +
                MeasureCinematicFontLineHeight() + 1);
            cursor++;
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
    g_pfnDrawCinematicFontCharacter_005c8d30 =
        DrawCinematicFontCharacter;
    g_pfnDrawCinematicTextString_005c8cf4 = DrawWrappedCinematicText;
}
