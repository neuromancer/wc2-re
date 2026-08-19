/*
 *  Floating-point helpers and the random-number generator.
 *
 *  Address range 0x434cd0-0x4353ff (provisional -- see docs/ORDER.md).
 *  The preceding smart unit ends at chase_speed (0x434c70).
 */
#include "wc1.h"

#pragma function(sin, cos, asin, acos, sqrt)

/* Function start: 0x4618E0 */
short RandomBelow(short n)
{
    return (short)(rand() % (int)n);
}

/* Function start: 0x4618FF */
void SeedRandomFromClock(void)
{
    srand((unsigned int)time(0));
}

/* Function start: 0x461922 */
unsigned short GetRandomWord(void)
{
    return rand() & 0xffff;
}

/* Function start: 0x461942 */
unsigned short RandomInRange(unsigned short lo, unsigned short hi)
{
    short span;

    span = (short)(hi - lo);
    if (span == 0)
        span = 1;
    span = (short)(rand() % (span + 1));
    return (short)(span + lo);
}

/* Function start: 0x4619A1 */
short RandomBelowOrEqual(short n)
{
    if (n != -1 && n != 0)
        return (short)(rand() % (n + 1));
    return 0;
}

/* Function start: 0x4619E2 */
long MultiplyFixed(int left, int right)
{
    double leftValue = (double)left * (1.0 / 256.0);
    double rightValue = (double)right * (1.0 / 256.0);

    return (long)(leftValue * rightValue * 256.0);
}

/* Function start: 0x461A2D */
long DivideFixed(int numerator, int denominator)
{
    float numeratorValue = (float)((double)numerator * (1.0 / 256.0));
    float denominatorValue;

    if (denominator != 0)
        denominatorValue = (float)((double)denominator * (1.0 / 256.0));
    else
        denominatorValue = 1.0f;
    return (long)(numeratorValue / denominatorValue * 256.0);
}

/* Function start: 0x461A91 */
long SinFixed(short degrees)
{
    float angle;
    long result;

    angle = (float)degrees;
    result = (long)(sin((double)angle * WC1_DEG2RAD) * 256.0);
    return result;
}

/* Function start: 0x461AD6 */
long CosFixed(short degrees)
{
    float angle;
    long result;

    angle = (float)degrees;
    result = (long)(cos((double)angle * WC1_DEG2RAD) * 256.0);
    return result;
}

/* Function start: 0x461B1B */
short ArcSin(int value)
{
    float scale;
    float normalized;
    short result;

    scale = 256.0f;
    normalized = (float)value / scale;
    result = (short)(asin((double)normalized) * 57.295779513082323);
    return result;
}

/* Function start: 0x461B6B */
short ArcCos(int value)
{
    float scale;
    float normalized;
    short result;

    scale = 256.0f;
    normalized = (float)value / scale;
    result = (short)(acos((double)normalized) * 57.295779513082323);
    return result;
}

/* Function start: 0x461BBB */
long Magnitude(int value)
{
    float scale;
    float normalized;
    long result;

    scale = 256.0f;
    normalized = (float)value / scale;
    result = (long)(sqrt((double)normalized) * scale);
    return result;
}

/* Function start: 0x461C06 */
long PlanarMagnitude(int x, int y)
{
    double scaledX = (double)x * (1.0 / 256.0);
    double scaledY = (double)y * (1.0 / 256.0);

    scaledX *= scaledX;
    scaledY *= scaledY;
    return (long)(sqrt(scaledX + scaledY) * 256.0);
}

/* Function start: 0x461C71 */
long Vector_magnitude(const FixedVector *vector)
{
    float x = (float)vector->x / 256.0;
    float y = (float)vector->y / 256.0;
    float z = (float)vector->z / 256.0;

    x *= x;
    y *= y;
    z *= z;
    return (long)(sqrt(x + y + z) * 256.0);
}

/* Function start: 0x461D02 */
void SetTextCursor(unsigned short a, unsigned short b)
{
    g_pCurrentTextContext_005c8d1c->cursorX = (short)a;
    g_pCurrentTextContext_005c8d1c->cursorY = (short)b;
}

/* Function start: 0x461D29 */
void SetTextContext(TextContext *context)
{
    g_pCurrentTextContext_005c8d1c = context;
    if (g_pCurrentTextContext_005c8d1c != NULL &&
        g_pCurrentTextContext_005c8d1c->text != NULL)
        *g_pCurrentTextContext_005c8d1c->text = 0;
    return;
}

/* Function start: 0x461D68 */
void WaitForVerticalBlankThunk(void)
{
    DIBwaitForVerticalBlank();
}

/* Function start: 0x461D7D */
void *IdentityHandle(void *v)
{
    return v;
}

/* Function start: 0x461D90 */
/* A forwarder that passes its argument through.  The Ghidra label claimed a
 * no-argument call, which was wrong. */
void SetWholePaletteFromTriplets(unsigned char *palette)
{
    DIBwholePaletteFromTriplets(palette);
}

/* Function start: 0x461DA7 */
unsigned short ReadWord(unsigned short *p)
{
    return *p;
}

/* Function start: 0x461DBD */
unsigned short GetFontCharWidth(char i)
{
    return g_pCurrentTextContext_005c8d1c->font[4 + (int)i];
}

/* Function start: WC2_UNMAPPED */
void ReleaseVideoResourcesHook(void)
{
}

/* Function start: 0x461DE0 */
void ApplySpacePaletteModeHook(void)
{
}

/* Function start: 0x461DF0 */
short GetShapeFrameBounds(short *bounds, short x, short y,
                          unsigned char *shape, short frame)
{
    short frameTableOffset;
#ifdef WC1_SDL
    short frameData[4];
#else
    short *frameData;
#endif

    frameTableOffset = (short)(frame * 4);
    if ((int)frameTableOffset < (int)*(unsigned short *)(shape + 4)) {
        frameTableOffset = (short)(frameTableOffset + 4);
#ifdef WC1_SDL
        memcpy(frameData,
               shape + *(unsigned short *)(shape + frameTableOffset),
               sizeof(frameData));
#else
        frameData = (short *)(shape +
            *(unsigned short *)(shape + frameTableOffset));
#endif
        bounds[2] = (short)(frameData[0] + x);
        bounds[0] = (short)(x - frameData[1]);
        bounds[1] = (short)(y - frameData[2]);
        bounds[3] = (short)(frameData[3] + y);
        return -1;
    }
    return 0;
}

/* Function start: 0x461E97 */
short IsPointInRect(short x, short y, const short *rect)
{
    if (rect[0] <= x && x <= rect[2] &&
        rect[1] <= y && y <= rect[3])
        return 1;
    return 0;
}

/* Function start: 0x461EFE */
void SplitPackedPoint(ShortPoint point, short *p)
{
    IsPointInRect(point.x, point.y, p);
}

/* Function start: 0x461F22 */
void DrawTextString(const char *text)
{
    int wrapped;
    int finished;
    int lineWidth;
    const char *iterator;
    char value;
    const char *cursor;
    const char *lineStart;
    int savedX;

    finished = 0;
    cursor = text;
    wrapped = 0;
    if (g_pCurrentTextContext_005c8d1c == 0)
        return;
    g_pCurrentTextContext_005c8d1c->cursorX =
        g_pCurrentTextContext_005c8d1c->viewport->left;
    while (finished == 0) {
        lineWidth = g_pCurrentTextContext_005c8d1c->cursorX;
        while (*cursor == ' ')
            cursor++;
        lineStart = cursor;
        if (lineWidth <
            g_pCurrentTextContext_005c8d1c->viewport->right) {
            for (;;) {
                value = *cursor;
                cursor++;
                if (value == '\n' || value == '\r')
                    break;
                if (value == 0) {
                    finished = 1;
                    break;
                }
                lineWidth +=
                    g_pCurrentTextContext_005c8d1c->font[4 + value];
                if (lineWidth >=
                    g_pCurrentTextContext_005c8d1c->viewport->right) {
                    lineWidth -=
                        g_pCurrentTextContext_005c8d1c->font[4 + value];
                    cursor--;
                    wrapped = 1;
                    if (*cursor != ' ') {
                        while (*cursor != ' ' && cursor > text) {
                            lineWidth -=
                                g_pCurrentTextContext_005c8d1c
                                    ->font[4 + *cursor];
                            cursor--;
                        }
                        if (cursor <= text) {
                            SystemDebugPrintf(
                                "FATAL : INVALID STRING '%s'n", text);
                            ClearDebugPauseFlags();
                            PumpMessagesDuringWait();
                            exit(0);
                        }
                    }
                    break;
                }
            }
        }

        if (g_pCurrentTextContext_005c8d1c->alignment == 2) {
            savedX = g_pCurrentTextContext_005c8d1c->cursorX;
            g_pCurrentTextContext_005c8d1c->cursorX = (short)(
                g_pCurrentTextContext_005c8d1c->viewport->left +
                ((g_pCurrentTextContext_005c8d1c->viewport->right -
                  g_pCurrentTextContext_005c8d1c->viewport->left + 1) -
                 (lineWidth - savedX + 1) + 1) / 2);
        }
        for (iterator = lineStart; iterator < cursor; iterator++) {
            DrawTextCharacter(*iterator);
        }
        if (g_pCurrentTextContext_005c8d1c->alignment == 2)
            g_pCurrentTextContext_005c8d1c->cursorX = (short)savedX;
        if (wrapped != 0) {
            g_pCurrentTextContext_005c8d1c->cursorX =
                g_pCurrentTextContext_005c8d1c->viewport->left;
            g_pCurrentTextContext_005c8d1c->cursorY +=
                *(short *)g_pCurrentTextContext_005c8d1c->font;
            wrapped = 0;
        } else {
            break;
        }
    }
}

/* Function start: 0x4621D5 */
void DrawTextCharacter(char character)
{
    TextContext *context;
    unsigned char *font;
    int fontHeight;
    unsigned int glyphWidth;
    int cursorY;

    if (g_pCurrentTextContext_005c8d1c == 0)
        return;
    if (character == '\n') {
        g_pCurrentTextContext_005c8d1c->cursorX =
            g_pCurrentTextContext_005c8d1c->viewport->left;
        g_pCurrentTextContext_005c8d1c->cursorY =
            (short)(g_pCurrentTextContext_005c8d1c->cursorY +
                    *(short *)g_pCurrentTextContext_005c8d1c->font);
    } else if (character == '\r') {
        g_pCurrentTextContext_005c8d1c->cursorX =
            g_pCurrentTextContext_005c8d1c->viewport->left;
    } else if (character != 0) {
        font = g_pCurrentTextContext_005c8d1c->font;
        fontHeight = *(short *)font;
        glyphWidth = font[4 + (int)(signed char)character];
        context = g_pCurrentTextContext_005c8d1c;
        cursorY = context->cursorY;
        DrawFontGlyph(character, context, fontHeight, glyphWidth, cursorY);
    }
}

/* Function start: 0x4622BD */
void AppendTextCharacter(char character)
{
    *g_pCurrentTextContext_005c8d1c->textCursor = character;
    g_pCurrentTextContext_005c8d1c->textCursor++;
    *g_pCurrentTextContext_005c8d1c->textCursor = 0;
    return;
}

/* Function start: 0x4622EE */
int MeasureShapeFrameStorage(unsigned char *shape, short frame)
{
    int size;
    unsigned short rowLength;
    unsigned char command;
    int frameTableOffset;
    unsigned char *run;

    size = 0;
    if (shape == 0)
        return 0;
    if (frame < 0)
        return 0;
    frameTableOffset = frame;
    frameTableOffset++;
    frameTableOffset <<= 2;
    if (frameTableOffset < *(unsigned short *)(shape + 4)) {
        run = shape + *(int *)(shape + frameTableOffset);
        run += 8;
        rowLength = *(unsigned short *)run;
        run += 2;
        while (rowLength != 0) {
            run += 2;
            run += 2;
            if ((rowLength & 1) != 0) {
                rowLength >>= 1;
                while (rowLength > 0) {
                    command = *run;
                    run++;
                    if ((command & 1) != 0) {
                        command >>= 1;
                        rowLength =
                            (unsigned short)(rowLength - command);
                        run++;
                        size += command;
                    } else {
                        command >>= 1;
                        rowLength =
                            (unsigned short)(rowLength - command);
                        size += command;
                        run += command;
                    }
                }
            } else {
                rowLength >>= 1;
                size += rowLength;
                run += rowLength;
            }
            rowLength = *(unsigned short *)run;
            run += 2;
        }
    }
    return size;
}
