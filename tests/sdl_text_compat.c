#include "game.h"

#include <string.h>

int main(int argumentCount, char **arguments)
{
    unsigned short fontStorage[0x183];
    unsigned char *font;
    unsigned char pixels[8];
    unsigned short rowOffsets[2] = {0, 4};
    const char wrappedText[] = {
        (char)0x84, ' ', (char)0x84, (char)0x84, 0
    };
    TextContext context;
    Viewport viewport;
    unsigned int character;

    (void)argumentCount;
    (void)arguments;
    /* Packed fonts contain 256 widths and two 256-byte offset tables. */
    memset(fontStorage, 0, sizeof(fontStorage));
    memset(pixels, 0xff, sizeof(pixels));
    memset(&context, 0, sizeof(context));
    memset(&viewport, 0, sizeof(viewport));
    font = (unsigned char *)fontStorage;
    font[0] = 1;
    font[2] = 7;
    font[3] = 0xff;
    font[4 + ' '] = 1;
    font[0x304] = 7;
    for (character = 0x80; character <= 0xff; character++) {
        font[4 + character] = 1;
        font[0x104 + character] = 4;
        font[0x204 + character] = 3;
    }

    viewport.pixels = pixels;
    viewport.rowOffsets = rowOffsets;
    viewport.right = 3;
    viewport.bottom = 1;
    context.viewport = &viewport;
    context.font = font;
    context.colour = 7;
    context.backgroundColour = 0xff;
    SetTextContext(&context);

    for (character = 0x80; character <= 0xff; character++) {
        context.cursorX = 0;
        pixels[0] = 0xff;
        if (GetFontCharWidth((char)character) != 1)
            return 1;
        DrawTextCharacter((char)character);
        if (context.cursorX != 1 || pixels[0] != 7)
            return 1;
    }

    context.colour = 8;
    context.cursorX = 0;
    pixels[0] = 0xff;
    DrawTextCharacter((char)0xe1);
    if (context.cursorX != 1 || pixels[0] != 8)
        return 1;

    context.colour = 7;
    context.cursorX = 0;
    context.cursorY = 0;
    memset(pixels, 0xff, sizeof(pixels));
    DrawTextString(wrappedText);
    if (context.cursorX != 2 || context.cursorY != 1 ||
        pixels[0] != 7 || pixels[4] != 7 || pixels[5] != 7)
        return 1;
    return 0;
}
