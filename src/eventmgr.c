/*
 *  Win32 library port: random, fixed-point math, text, strings and events.
 *
 *  Retail object 0x4618E0-0x46572F (link order; see docs/ORDER.md).
 */
#include "game.h"

#pragma function(acos, asin, cos, sin, sqrt)
/* Function start: 0x4618E0 */
short RandomBelow(short n)
{
    return (short)(rand() % (int)n);
}

/* Function start: 0x4618FF */
void SeedRandomFromClock(void)
{
    srand((unsigned int)time(0));
    return;
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
    if (n == -1 || n == 0)
        return 0;
    return (short)(rand() % (n + 1));
}

/* Function start: 0x4619E2 */
long MultiplyFixed(int left, int right)
{
    float leftFloat;
    float rightFloat;
    long product;

    leftFloat = (float)left / 256.0;
    rightFloat = (float)right / 256.0;
    product = (long)(leftFloat * rightFloat * 256.0);
    return product;
}

/* Function start: 0x461A2D */
long DivideFixed(int numerator, int denominator)
{
    float numeratorValue;
    float denominatorValue;
    long quotient;

    numeratorValue = (float)numerator / 256.0;
    if (denominator != 0)
        denominatorValue = (float)denominator / 256.0;
    else
        denominatorValue = 1.0f;
    quotient = (long)(numeratorValue / denominatorValue * 256.0);
    return quotient;
}

/* Function start: 0x461A91 */
long SinFixed(short degrees)
{
    float angle;
    long result;

    angle = (float)degrees;
    result = (long)(sin((double)angle * DEG2RAD) * 256.0);
    return result;
}

/* Function start: 0x461AD6 */
long CosFixed(short degrees)
{
    float angle;
    long result;

    angle = (float)degrees;
    result = (long)(cos((double)angle * DEG2RAD) * 256.0);
    return result;
}

/* Function start: 0x461B1B */
short ArcSin(int value)
{
    float scale;
    float normalized;
    short result;

    scale = 256.0f;
    normalized = (float)value;
    normalized /= scale;
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
    normalized = (float)value;
    normalized /= scale;
    result = (short)(acos((double)normalized) * 57.295779513082323);
    return result;
}

/* Function start: 0x461BBB */
long Magnitude(int value)
{
    float fixedScale;
    float normalized;
    long scaledMagnitude;

    fixedScale = 256.0f;
    normalized = (float)value;
    normalized /= fixedScale;
    scaledMagnitude = (long)(sqrt((double)normalized) * fixedScale);
    return scaledMagnitude;
}

/* Function start: 0x461C06 */
long PlanarMagnitude(int x, int y)
{
    float scaledX = (float)x / 256.0;
    float scaledY = (float)y / 256.0;
    long scaledMagnitude;

    scaledX *= scaledX;
    scaledY *= scaledY;
    scaledMagnitude = (long)(sqrt(scaledX + scaledY) * 256.0);
    return scaledMagnitude;
}

/* Function start: 0x461C71 */
long Vector_magnitude(const FixedVector *vector)
{
    float x = (float)vector->x / 256.0;
    float y = (float)vector->y / 256.0;
    float z = (float)vector->z / 256.0;
    long scaledMagnitude;

    x *= x;
    y *= y;
    z *= z;
    scaledMagnitude = (long)(sqrt(x + y + z) * 256.0);
    return scaledMagnitude;
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
    return;
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
short ReadWord(unsigned short *p)
{
    return *p;
}

/* Function start: 0x461DBD */
short GetFontCharWidth(char i)
{
#ifdef SDL_PORT
    /* The packed format reserves one table entry for every byte value. */
    return g_pCurrentTextContext_005c8d1c->font[4 + (unsigned char)i];
#else
    return g_pCurrentTextContext_005c8d1c->font[4 + (int)i];
#endif
}

/* Function start: 0x461DE0 */
void ApplySpacePaletteModeHook(void)
{
    return;
}

/* Function start: 0x461DF0 */
short GetShapeFrameBounds(short *bounds, short x, short y,
                          unsigned char *shape, short frame)
{
#ifdef SDL_PORT
    unsigned short frameData[4];
#endif

    frame <<= 2;
    if (*(unsigned short *)(shape + 4) > frame) {
        frame += 4;
        shape += *(unsigned short *)(shape + frame);
#ifdef SDL_PORT
        memcpy(frameData, shape, sizeof(frameData));
        bounds[2] = (short)(frameData[0] + x);
        bounds[0] = (short)(x - frameData[1]);
        bounds[1] = (short)(y - frameData[2]);
        bounds[3] = (short)(frameData[3] + y);
#else
        bounds[2] = (short)(*(unsigned short *)shape + x);
        bounds[0] = (short)(x - ((unsigned short *)shape)[1]);
        bounds[1] = (short)(y - ((unsigned short *)shape)[2]);
        bounds[3] = (short)(((unsigned short *)shape)[3] + y);
#endif
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
    return;
}

/* Function start: 0x461F22 */
void DrawTextString(const char *text)
{
    int wrapPending;
    const char *cursor;
    const char *drawPos;
    const char *lineBegin;
    char ch;
    int previousX;
    int isDone;
    int pixelWidth;

    isDone = 0;
    cursor = text;
    wrapPending = 0;
    if (g_pCurrentTextContext_005c8d1c == 0)
        return;
    g_pCurrentTextContext_005c8d1c->cursorX =
        g_pCurrentTextContext_005c8d1c->viewport->left;
    while (isDone == 0) {
        pixelWidth = g_pCurrentTextContext_005c8d1c->cursorX;
        while (*cursor == ' ')
            cursor++;
        lineBegin = cursor;
        while (pixelWidth <
               g_pCurrentTextContext_005c8d1c->viewport->right) {
            ch = *cursor;
            cursor++;
            if (ch == '\n' || ch == '\r')
                break;
            if (ch == 0) {
                isDone = 1;
                break;
            }
            pixelWidth +=
#ifdef SDL_PORT
                g_pCurrentTextContext_005c8d1c
                    ->font[4 + (unsigned char)ch];
#else
                g_pCurrentTextContext_005c8d1c->font[4 + ch];
#endif
            if (pixelWidth >=
                g_pCurrentTextContext_005c8d1c->viewport->right) {
                pixelWidth -=
#ifdef SDL_PORT
                    g_pCurrentTextContext_005c8d1c
                        ->font[4 + (unsigned char)ch];
#else
                    g_pCurrentTextContext_005c8d1c->font[4 + ch];
#endif
                cursor--;
                wrapPending = 1;
                if (*cursor != ' ') {
                    while (*cursor != ' ' && cursor > text) {
                        pixelWidth -=
                            g_pCurrentTextContext_005c8d1c
#ifdef SDL_PORT
                                ->font[4 + (unsigned char)*cursor];
#else
                                ->font[4 + *cursor];
#endif
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

        if (g_pCurrentTextContext_005c8d1c->alignment == 2) {
            previousX = g_pCurrentTextContext_005c8d1c->cursorX;
            g_pCurrentTextContext_005c8d1c->cursorX = (short)(
                g_pCurrentTextContext_005c8d1c->viewport->left +
                ((g_pCurrentTextContext_005c8d1c->viewport->right -
                  g_pCurrentTextContext_005c8d1c->viewport->left + 1) -
                 (pixelWidth - g_pCurrentTextContext_005c8d1c->cursorX + 1) +
                 1) / 2);
        }
        for (drawPos = lineBegin; drawPos < cursor; drawPos++) {
            DrawTextCharacter(*drawPos);
        }
        if (g_pCurrentTextContext_005c8d1c->alignment == 2)
            g_pCurrentTextContext_005c8d1c->cursorX = (short)previousX;
        if (wrapPending != 0) {
            g_pCurrentTextContext_005c8d1c->cursorX =
                g_pCurrentTextContext_005c8d1c->viewport->left;
            g_pCurrentTextContext_005c8d1c->cursorY +=
                *(short *)g_pCurrentTextContext_005c8d1c->font;
            wrapPending = 0;
        }
    }
    return;
}

/* Function start: 0x4621D5 */
void DrawTextCharacter(char character)
{
    int lineHeight;
    unsigned int glyphWidth;

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
        lineHeight = *(short *)g_pCurrentTextContext_005c8d1c->font;
#ifdef SDL_PORT
        glyphWidth = g_pCurrentTextContext_005c8d1c->font[
            (unsigned char)character + 4];
#else
        glyphWidth = g_pCurrentTextContext_005c8d1c->font[character + 4];
#endif
        DrawFontGlyph(character, g_pCurrentTextContext_005c8d1c,
                      lineHeight, glyphWidth,
                      g_pCurrentTextContext_005c8d1c->cursorY);
    }
    return;
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
    unsigned short lineLength;
    unsigned char command;
    int frameOffset;
    unsigned char *src;

    size = 0;
    if (shape == 0)
        return 0;
    if (frame < 0)
        return 0;
    frameOffset = frame;
    frameOffset++;
    frameOffset <<= 2;
    if (frameOffset < *(unsigned short *)(shape + 4)) {
        src = shape + *(int *)(shape + frameOffset);
        src += 8;
        lineLength = *(unsigned short *)src;
        src += 2;
        while (lineLength != 0) {
            src += 2;
            src += 2;
            if ((lineLength & 1) != 0) {
                lineLength >>= 1;
                while (lineLength > 0) {
                    command = *src;
                    src++;
                    if ((command & 1) != 0) {
                        command >>= 1;
                        lineLength =
                            (unsigned short)(lineLength - command);
                        src++;
                        size += command;
                    } else {
                        command >>= 1;
                        lineLength =
                            (unsigned short)(lineLength - command);
                        size += command;
                        src += command;
                    }
                }
            } else {
                lineLength >>= 1;
                size += lineLength;
                src += lineLength;
            }
            lineLength = *(unsigned short *)src;
            src += 2;
        }
    }
    return size;
}

#pragma function(memcpy, memset, strcpy, strlen)
/* Function start: 0x46243F */
unsigned int DosFarPtrToNear(void *v)
{
    return (unsigned int)v;
}

/* Function start: 0x462452 */
void *DosNearPtrToFar(unsigned int v)
{
    return (void *)v;
}

/* Function start: 0x462465 */
char *DosStrrchr(char *s, short c)
{
    return strrchr(s, (int)c);
}

/* Function start: 0x462486 */
char *DosStrchr(const char *s, short c)
{
    return strchr(s, (int)c);
}

/* Function start: 0x4624A7 */
char *DosStrcpy(char *dst, const char *src)
{
    return strcpy(dst, src);
}

/* Function start: 0x4624C7 */
char *CopyStringAndReturnEnd(char *destination, const char *source)
{
    strcpy(destination, source);
    return destination + strlen(destination);
    return strcpy(destination, source);
}

/* Function start: 0x46250B */
short DosStrlen(const char *s)
{
    return (short)strlen(s);
}

/* Function start: 0x462527 */
void DosMemcpy(void *dst, const void *src, size_t n)
{
    memcpy(dst, src, n & 0xffff);
    return;
}

/* Function start: 0x462550 */
void DosMemset(void *destination, unsigned int count, short value)
{
    memset(destination, value, (unsigned short)count);
    return;
}

/* Function start: 0x46257A */
short InitializePostPaletteState(void)
{
    return 0;
}

/* Function start: 0x46258D */
/* Nothing in the shipped executable calls this; the DOS start-time recorder. */
void RecordStartTimeHook(void)
{
    return;
}

/* Function start: 0x46259D */
short InitializeInputManagerMemory(short eventQueueSize)
{
    g_bInputManagerInitialized_005c80ca = 1;
    g_pInputManagerState_005c8464 =
        (InputManagerState *)AllocateTaggedMemory(0x21, 0x40);
    return 1;
}

/* Function start: 0x4625CB */
void ShutdownEventManager(void)
{
    g_bInputManagerInitialized_005c80ca = 0;
    ReleasePacketHandle(g_pInputManagerState_005c8464);
    return;
}

/* Function start: 0x4625F2 */
/* Nothing in the shipped executable calls this; the DOS interrupt-vector hook, reporting failure. */
short HookInterruptVector(short vector)
{
    return 0;
}

/* Function start: 0x462605 */
/* Nothing in the shipped executable calls this; the DOS interrupt-vector restore. */
void UnhookInterruptVectorHook(short vector)
{
    return;
}

/* Function start: 0x462615 */
/* Nothing in the shipped executable calls this; restores every hooked DOS vector. */
void UnhookAllInterruptVectorsHook(void)
{
    return;
}

/* Function start: 0x462625 */
void TranslatePolledInputEvent(unsigned short type, unsigned int value)
{
    HostMouseMessage *mouse;

    switch (type) {
    case 7:
        QueueInputEvent(type,
            g_aInputDeviceSamples_005d1780[
                g_nActiveInputDevice_005d1726].x,
            g_aInputDeviceSamples_005d1780[
                g_nActiveInputDevice_005d1726].y,
            0, value & 1, (value & 2) >> 1, 0, 0, 0);
        break;
    case 10:
        QueueInputEvent(1,
            g_aInputDeviceSamples_005d1780[
                g_nActiveInputDevice_005d1726].x,
            g_aInputDeviceSamples_005d1780[
                g_nActiveInputDevice_005d1726].y,
            0, value & 1, (value & 2) >> 1, 0, 0, 0);
        break;
    case 0x45:
        QueueInputEvent(2,
            g_aInputDeviceSamples_005d1780[
                g_nActiveInputDevice_005d1726].x,
            g_aInputDeviceSamples_005d1780[
                g_nActiveInputDevice_005d1726].y,
            0, value & 1, (value & 2) >> 1, 0, 0, 0);
        break;
    case 3:
        mouse = &g_stHostMouseMessage_005d10d0;
        QueueInputEvent(type,
                        g_nQueuedInputX_005c83f0,
                        g_nQueuedInputY_005c83f2,
                        0, 0, 0, 0, 0, 0);
        break;
    case 1:
        mouse = &g_stHostMouseMessage_005d10d0;
        QueueInputEvent(type,
                        g_nQueuedInputX_005c83f0,
                        g_nQueuedInputY_005c83f2,
                        0,
                        mouse->primaryButton,
                        mouse->secondaryButton,
                        mouse->secondaryButton * 2 |
                            mouse->primaryButton | value >> 2,
                        0, 0);
        break;
    case 4:
        break;
    }
}

/* Function start: 0x462849 */
void QueueInputEventAtCursor(unsigned int type, short primaryButton,
                             short secondaryButton)
{
    /* Preserve the original 16-bit event ID. */
    unsigned int eventType = type & 0xffff;

    QueueInputEvent(eventType, g_nQueuedInputX_005c83f0,
                    g_nQueuedInputY_005c83f2, 0,
                    primaryButton, secondaryButton, 0, 0, 0);
}

/* Function start: 0x462890 */
InputEvent *AllocateInputEvent(void)
{
    int index;

    if (g_nInputEventPoolInitialized_0049d4bc != 0) {
        for (index = 0; index < 0x100; index++) {
            if (g_anInputEventSlotUsed_005c87e0[index] == 0) {
                g_anInputEventSlotUsed_005c87e0[index] = 1;
                return &g_aInputEventPool_005c5890[index];
            }
        }
    } else {
        memset(g_aInputEventPool_005c5890, 0,
               sizeof(g_aInputEventPool_005c5890));
        memset(g_anInputEventSlotUsed_005c87e0, 0,
               sizeof(g_anInputEventSlotUsed_005c87e0));
        g_nInputEventPoolInitialized_0049d4bc = 1;
        g_anInputEventSlotUsed_005c87e0[0] = 1;
        return &g_aInputEventPool_005c5890[0];
    }
    return 0;
}

#pragma intrinsic(memset)
/* Function start: 0x46294F */
void ReleaseInputEvent(InputEvent *event)
{
    int slot;

    for (slot = 0; slot < 0x100; slot++) {
        if (&g_aInputEventPool_005c5890[slot] == event)
            g_anInputEventSlotUsed_005c87e0[slot] = 0;
    }
    return;
}

/* Function start: 0x4629A7 */
void QueueInputEvent(int type, int x, int y, int value,
                     int primaryButton, int secondaryButton,
                     unsigned int ignored, unsigned int field14,
                     unsigned int field18)
{
    unsigned int modifiers;

    (void)ignored;

    modifiers = 0;
    if (GetShiftKeyState() != 0)
        modifiers |= 0xe0;
    if (GetControlKeyState() != 0)
        modifiers |= 0x3800;
    if (GetKeyboardModifiers() != 0)
        modifiers |= 0x700;
    if (primaryButton != 0)
        modifiers |= 1;
    if (secondaryButton != 0)
        modifiers |= 2;

    if (g_pInputEventHead_0049d4b4 == 0) {
        g_pInputEventHead_0049d4b4 = AllocateInputEvent();
        if (g_pInputEventHead_0049d4b4 == 0) {
            ReleaseInputEventQueue();
            return;
        }
        g_pInputEventTail_0049d4b8 = g_pInputEventHead_0049d4b4;
        g_pInputEventHead_0049d4b4->next = 0;
        g_pInputEventTail_0049d4b8->previous = 0;
    } else {
        g_pInputEventTail_0049d4b8->next = AllocateInputEvent();
        if (g_pInputEventTail_0049d4b8->next == 0) {
            ReleaseInputEventQueue();
            return;
        }
        g_pInputEventTail_0049d4b8->next->previous =
            g_pInputEventTail_0049d4b8;
        g_pInputEventTail_0049d4b8 = g_pInputEventTail_0049d4b8->next;
        g_pInputEventTail_0049d4b8->next = 0;
    }
    g_pInputEventTail_0049d4b8->type = type;
    g_pInputEventTail_0049d4b8->modifiers = modifiers;
    g_pInputEventTail_0049d4b8->x = x;
    g_pInputEventTail_0049d4b8->y = y;
    g_pInputEventTail_0049d4b8->value = value;
    g_pInputEventTail_0049d4b8->primaryButton = (short)primaryButton;
    g_pInputEventTail_0049d4b8->secondaryButton = (short)secondaryButton;
    g_pInputEventTail_0049d4b8->field_14 = field14;
    g_pInputEventTail_0049d4b8->field_18 = field18;
    g_pInputEventTail_0049d4b8->status = 1;
    if (g_pInputEventTail_0049d4b8->type == 3) {
        g_nQueuedInputX_005c83f0 = g_pInputEventTail_0049d4b8->x;
        g_nQueuedInputY_005c83f2 = g_pInputEventTail_0049d4b8->y;
    }
    if (g_pInputEventTail_0049d4b8->type == 2) {
        g_nInputDoubleClickDeadline_0049d4c0 =
            g_nInputClock_005c84a8 + 25;
        g_nPreviousPrimaryButton_0049d4c4 = primaryButton;
        g_nPreviousSecondaryButton_0049d4c8 = secondaryButton;
    }
    if (g_pInputEventTail_0049d4b8->type == 1 ||
        g_pInputEventTail_0049d4b8->type == 8 ||
        g_pInputEventTail_0049d4b8->type == 9) {
        if ((g_pInputEventTail_0049d4b8->primaryButton != 0 &&
             g_nPreviousPrimaryButton_0049d4c4 != 0) ||
            (g_pInputEventTail_0049d4b8->secondaryButton != 0 &&
             g_nPreviousSecondaryButton_0049d4c8 != 0)) {
            if (g_nInputDoubleClickDeadline_0049d4c0 >
                g_nInputClock_005c84a8) {
                g_nInputDoubleClickDeadline_0049d4c0 = 0;
                g_pInputEventTail_0049d4b8->status = 2;
                g_nPreviousPrimaryButton_0049d4c4 = 0;
                g_nPreviousSecondaryButton_0049d4c8 = 0;
            }
        }
    }
    return;
}

/* Function start: 0x462C43 */
void ReleaseInputEventQueue(void)
{
    InputEvent *event;
    InputEvent *nextEvent;

    event = g_pInputEventHead_0049d4b4;
    while (event != 0) {
        nextEvent = event->next;
        ReleaseInputEvent(event);
        event = nextEvent;
    }
    g_pInputEventTail_0049d4b8 = 0;
    g_pInputEventHead_0049d4b4 = g_pInputEventTail_0049d4b8;
    return;
}

/* Function start: 0x462C9C */
void RetainInputEventsOfType(int type)
{
    InputEvent *event;
    InputEvent *nextEvent;

    event = g_pInputEventHead_0049d4b4;
    while (event != 0) {
        nextEvent = event->next;
        if (event->type != type) {
            if (event->previous != 0)
                event->previous->next = event->next;
            else
                g_pInputEventHead_0049d4b4 = event->next;
            if (event->next != 0)
                event->next->previous = event->previous;
            else
                g_pInputEventTail_0049d4b8 = event->previous;
            ReleaseInputEvent(event);
        }
        event = nextEvent;
    }
    return;
}

/* Function start: 0x462D48 */
void RemoveInputEvent(InputEvent *event)
{
    if (event->previous != 0) {
        if (event->next != 0) {
            event->previous->next = event->next;
            event->next->previous = event->previous;
        } else {
            event->previous->next = 0;
            g_pInputEventTail_0049d4b8 = event->previous;
        }
    } else {
        if (event->next != 0) {
            g_pInputEventHead_0049d4b4 = event->next;
            event->next->previous = 0;
        } else {
            g_pInputEventTail_0049d4b8 = 0;
            g_pInputEventHead_0049d4b4 = 0;
        }
    }
    ReleaseInputEvent(event);
    return;
}

/* Function start: 0x462DFC */
int GetNextInputEvent(InputEventState *state)
{
    int posX;
    int posY;
    int clearQueue;
    int type;

    clearQueue = 0;
    if (g_nNextInputQueueFlushTick_0049d4cc < g_nInputClock_005c84a8)
        FlushInputEvents();
    g_nNextInputQueueFlushTick_0049d4cc = g_nInputClock_005c84a8 + 20;
    type = 0;
    if (g_pInputEventHead_0049d4b4 != 0) {
        posX = (int)g_pInputEventHead_0049d4b4->x;
        posY = (int)g_pInputEventHead_0049d4b4->y;
        if (g_pInputViewport_005c8403 != 0) {
            if ((int)g_pInputViewport_005c8403->left > posX)
                g_pInputEventHead_0049d4b4->x =
                    g_pInputViewport_005c8403->left;
            else if ((int)g_pInputViewport_005c8403->right < posX)
                g_pInputEventHead_0049d4b4->x =
                    g_pInputViewport_005c8403->right;
            if ((int)g_pInputViewport_005c8403->top > posY)
                g_pInputEventHead_0049d4b4->y =
                    g_pInputViewport_005c8403->top;
            else if ((int)g_pInputViewport_005c8403->bottom < posY)
                g_pInputEventHead_0049d4b4->y =
                    g_pInputViewport_005c8403->bottom;
        }

        state->status = (short)g_pInputEventHead_0049d4b4->status;
        state->modifiers =
            (short)g_pInputEventHead_0049d4b4->modifiers;
        switch (g_pInputEventHead_0049d4b4->type) {
        case 2:
            g_nQueuedInputX_005c83f0 = g_pInputEventHead_0049d4b4->x;
            g_nQueuedInputY_005c83f2 = g_pInputEventHead_0049d4b4->y;
            g_bQueuedPrimaryButton_005c83f4 = 0;
            state->x = g_pInputEventHead_0049d4b4->x;
            state->y = g_pInputEventHead_0049d4b4->y;
            *(unsigned int *)&state->value =
                (int)g_pInputEventHead_0049d4b4->secondaryButton * 2 |
                (int)g_pInputEventHead_0049d4b4->primaryButton;
            state->value = (short)*(unsigned int *)&state->value;
            state->status = (short)g_pInputEventHead_0049d4b4->status;
            type = 2;
            break;
        case 1:
            g_nQueuedInputX_005c83f0 = g_pInputEventHead_0049d4b4->x;
            g_nQueuedInputY_005c83f2 = g_pInputEventHead_0049d4b4->y;
            g_bQueuedPrimaryButton_005c83f4 =
                (unsigned char)g_pInputEventHead_0049d4b4->primaryButton;
            g_bQueuedSecondaryButton_005c83f5 =
                (unsigned char)g_pInputEventHead_0049d4b4->secondaryButton;
            state->x = g_pInputEventHead_0049d4b4->x;
            state->y = g_pInputEventHead_0049d4b4->y;
            *(unsigned int *)&state->value =
                (int)g_pInputEventHead_0049d4b4->secondaryButton * 2 |
                (int)g_pInputEventHead_0049d4b4->primaryButton;
            state->value = (short)*(unsigned int *)&state->value;
            state->status = (short)g_pInputEventHead_0049d4b4->status;
            type = 1;
            if (state->status > 1)
                type = 1;
            break;
        case 4:
            *(unsigned int *)&state->value =
                (int)g_pInputEventHead_0049d4b4->value;
            state->value = (short)g_pInputEventHead_0049d4b4->field_18;
            state->status = (short)g_pInputEventHead_0049d4b4->field_14;
            state->x = g_nQueuedInputX_005c83f0;
            state->y = g_nQueuedInputY_005c83f2;
            type = 4;
            break;
        case 5:
            state->x = g_pInputEventHead_0049d4b4->value;
            *(unsigned int *)&state->value =
                (int)g_pInputEventHead_0049d4b4->value;
            state->value = (short)g_pInputEventHead_0049d4b4->field_18;
            state->status = (short)g_pInputEventHead_0049d4b4->field_14;
            state->x = g_nQueuedInputX_005c83f0;
            state->y = g_nQueuedInputY_005c83f2;
            type = 5;
            break;
        case 6:
            state->x = g_pInputEventHead_0049d4b4->value;
            type = 6;
            break;
        case 7:
        case 8:
        case 9:
            state->x = g_pInputEventHead_0049d4b4->x;
            state->y = g_pInputEventHead_0049d4b4->y;
            type = (int)g_pInputEventHead_0049d4b4->type;
            break;
        case 3:
            g_nQueuedInputX_005c83f0 = g_pInputEventHead_0049d4b4->x;
            g_nQueuedInputY_005c83f2 = g_pInputEventHead_0049d4b4->y;
            state->x = g_pInputEventHead_0049d4b4->x;
            state->y = g_pInputEventHead_0049d4b4->y;
            type = 3;
            clearQueue = 1;
            break;
        default:
            state->x = g_pInputEventHead_0049d4b4->x;
            state->y = g_pInputEventHead_0049d4b4->y;
            type = 0;
            break;
        }
        RemoveInputEvent(g_pInputEventHead_0049d4b4);
        if (clearQueue != 0) {
            ReleaseInputEventQueue();
            FlushInputEvents();
        }
    } else {
        state->x = g_nQueuedInputX_005c83f0;
        state->y = g_nQueuedInputY_005c83f2;
    }
    return type;
}

/* Function start: 0x46327F */
int PollInputEvent(InputEventState *event, short deviceMask)
{
    PumpWindowMessages(0);
    return GetNextInputEvent(event);
}

/* Function start: 0x4632A5 */
short PeekInputEvent(InputEventState *state, short type)
{
    unsigned int flags;
    int eventType;
    InputEvent *event;

    flags = 0;
    g_nNextInputQueueFlushTick_0049d4cc = g_nInputClock_005c84a8 + 20;
    for (event = g_pInputEventHead_0049d4b4;
         event != 0 && event->type != type;
         event = event->next)
        ;
    if (event != 0) {
        state->type = (int)event->type;
        *(unsigned int *)&state->value = event->modifiers;
        state->status = (short)event->field_14;
        state->value = (short)event->field_18;
        state->timestamp = event->timestamp;
        eventType = (int)event->type;
        if (eventType == 1 || eventType == 2)
            flags |= 8;
        if (event->primaryButton != 0)
            flags |= 1;
        else
            ;
        if (event->secondaryButton != 0)
            flags |= 2;
        else
            ;
        state->modifiers = (unsigned short)flags;
        state->x = event->x;
        state->y = event->y;
        state->status = (short)event->status;
        state->value = (short)*(unsigned int *)&state->value;
        return 1;
    }
    return 0;
}

/* Function start: 0x4633E7 */
InputEventState *FindQueuedInputEvent(int type)
{
    unsigned int modifiers;
    InputEvent *event;

    modifiers = 0;
    g_nNextInputQueueFlushTick_0049d4cc = g_nInputClock_005c84a8 + 20;
    for (event = g_pInputEventHead_0049d4b4;
         event != 0 && event->type != type;
         event = event->next)
        ;
    if (event != 0) {
        if (event->type == 3) {
            g_nQueuedInputX_005c83f0 = g_pInputEventHead_0049d4b4->x;
            g_nQueuedInputY_005c83f2 = g_pInputEventHead_0049d4b4->y;
        }
        g_stFoundInputEvent_005c3af8.type = (int)event->type;
        *(unsigned int *)&g_stFoundInputEvent_005c3af8.value =
            event->modifiers;
        g_stFoundInputEvent_005c3af8.value = (short)event->field_18;
        g_stFoundInputEvent_005c3af8.status = (short)event->field_14;
        g_stFoundInputEvent_005c3af8.timestamp = event->timestamp;
        if (event->type == 1 || event->type == 2)
            modifiers |= 8;
        if (event->primaryButton != 0)
            modifiers |= 1;
        else
            ;
        if (event->secondaryButton != 0)
            modifiers |= 2;
        else
            ;
        g_stFoundInputEvent_005c3af8.modifiers =
            (unsigned short)modifiers;
        g_stFoundInputEvent_005c3af8.x = event->x;
        g_stFoundInputEvent_005c3af8.y = event->y;
        g_stFoundInputEvent_005c3af8.status = (short)event->status;
        g_stFoundInputEvent_005c3af8.value =
            (short)*(unsigned int *)&g_stFoundInputEvent_005c3af8.value;
        return &g_stFoundInputEvent_005c3af8;
    } else
        return 0;
}

/* Function start: 0x46354F */
void FlushInputEvents(void)
{
    g_nNextInputQueueFlushTick_0049d4cc = g_nInputClock_005c84a8 + 20;
    ReleaseInputEventQueue();
    return;
}

/* Function start: 0x463571 */
short InitializeMouseCursorDepth(void)
{
    g_nMouseCursorDrawDepth_0049d4d4 = 1;
    return 1;
}

/* Function start: 0x46358F */
/* Nothing in the shipped executable calls this; the DOS mouse pointer set-up, reporting success. */
short InitializeMousePointer(void)
{
    return 1;
}

/* Function start: 0x4635A3 */
/* Nothing in the shipped executable calls this. */
void ResetPointerViewportsHook(void)
{
    return;
}

/* Function start: 0x4635B3 */
/* Nothing in the shipped executable calls this. */
void SetPointerViewportHook(void)
{
    return;
}

/* Function start: 0x4635C3 */
/* Nothing in the shipped executable calls this. */
void RemovePointerViewportHook(void)
{
    return;
}

/* Function start: 0x4635D3 */
void CheckCursor(void)
{
    return;
}

/* Function start: 0x4635E3 */
void CaptureMouseCursorBackground(void)
{
    if (g_nMouseCursorDrawDepth_0049d4d4 > 0 &&
        g_pInputViewport_005c8403 != 0 &&
        g_pInputCursorShape_005c83f9 != 0) {
        CaptureSpriteBackground(&g_stScreenViewport_005d21a0,
                                g_abInputCursorBackground_005c3b10,
                                g_nQueuedInputX_005c83f0,
                                g_nQueuedInputY_005c83f2,
                                g_pInputCursorShape_005c83f9,
                                g_nInputCursorFrame_005c83fd);
        g_nCapturedInputCursorX_005c817c = (int)g_nQueuedInputX_005c83f0;
        g_nCapturedInputCursorY_005c8178 = (int)g_nQueuedInputY_005c83f2;
        if (g_nCapturedInputCursorX_005c817c - 16 <
            g_nInputCursorDirtyLeft_005c85c0)
            g_nInputCursorDirtyLeft_005c85c0 =
                g_nCapturedInputCursorX_005c817c - 16;
        if (g_nInputCursorDirtyRight_005c845c <
            g_nCapturedInputCursorX_005c817c + 16)
            g_nInputCursorDirtyRight_005c845c =
                g_nCapturedInputCursorX_005c817c + 16;
        if (g_nCapturedInputCursorY_005c8178 - 16 <
            g_nInputCursorDirtyTop_005c85c4)
            g_nInputCursorDirtyTop_005c85c4 =
                g_nCapturedInputCursorY_005c8178 - 16;
        if (g_nInputCursorDirtyBottom_005c8460 <
            g_nCapturedInputCursorY_005c8178 + 16)
            g_nInputCursorDirtyBottom_005c8460 =
                g_nCapturedInputCursorY_005c8178 + 16;
        g_bInputCursorDirty_005c8450 = 1;
        g_bInputCursorBackgroundCaptured_005c80c4 = 1;
    }
    return;
}

/* Function start: 0x4636F7 */
void DrawMouseCursor(void)
{
    if (g_nMouseCursorDrawDepth_0049d4d4 > 0 &&
        g_pInputViewport_005c8403 != 0 &&
        g_pInputCursorShape_005c83f9 != 0) {
        DrawSpriteDefault(&g_stScreenViewport_005d21a0,
                          g_nQueuedInputX_005c83f0,
                          g_nQueuedInputY_005c83f2,
                          g_pInputCursorShape_005c83f9,
                          g_nInputCursorFrame_005c83fd);
        if ((int)g_nQueuedInputX_005c83f0 - 16 <
            g_nInputCursorDirtyLeft_005c85c0)
            g_nInputCursorDirtyLeft_005c85c0 =
                (int)g_nQueuedInputX_005c83f0 - 16;
        if (g_nInputCursorDirtyRight_005c845c <
            (int)g_nQueuedInputX_005c83f0 + 16)
            g_nInputCursorDirtyRight_005c845c =
                (int)g_nQueuedInputX_005c83f0 + 16;
        if ((int)g_nQueuedInputY_005c83f2 - 16 <
            g_nInputCursorDirtyTop_005c85c4)
            g_nInputCursorDirtyTop_005c85c4 =
                (int)g_nQueuedInputY_005c83f2 - 16;
        if (g_nInputCursorDirtyBottom_005c8460 <
            (int)g_nQueuedInputY_005c83f2 + 16)
            g_nInputCursorDirtyBottom_005c8460 =
                (int)g_nQueuedInputY_005c83f2 + 16;
        g_bInputCursorDirty_005c8450 = 1;
    }
    return;
}

/* Function start: 0x4637F4 */
void RestoreMouseCursorBackground(void)
{
    if (g_nMouseCursorDrawDepth_0049d4d4 > 0 &&
        g_pInputViewport_005c8403 != 0 &&
        g_pInputCursorShape_005c83f9 != 0 &&
        g_bInputCursorBackgroundCaptured_005c80c4 != 0) {
        RestoreSpriteBackground(&g_stScreenViewport_005d21a0,
                                g_abInputCursorBackground_005c3b10,
                                (short)g_nCapturedInputCursorX_005c817c,
                                (short)g_nCapturedInputCursorY_005c8178,
                                g_pInputCursorShape_005c83f9,
                                g_nInputCursorFrame_005c83fd);
        if (g_nCapturedInputCursorX_005c817c - 16 <
            g_nInputCursorDirtyLeft_005c85c0)
            g_nInputCursorDirtyLeft_005c85c0 =
                g_nCapturedInputCursorX_005c817c - 16;
        if (g_nInputCursorDirtyRight_005c845c <
            g_nCapturedInputCursorX_005c817c + 16)
            g_nInputCursorDirtyRight_005c845c =
                g_nCapturedInputCursorX_005c817c + 16;
        if (g_nCapturedInputCursorY_005c8178 - 16 <
            g_nInputCursorDirtyTop_005c85c4)
            g_nInputCursorDirtyTop_005c85c4 =
                g_nCapturedInputCursorY_005c8178 - 16;
        if (g_nInputCursorDirtyBottom_005c8460 <
            g_nCapturedInputCursorY_005c8178 + 16)
            g_nInputCursorDirtyBottom_005c8460 =
                g_nCapturedInputCursorY_005c8178 + 16;
        g_bInputCursorBackgroundCaptured_005c80c4 = 0;
    }
    return;
}

/* Function start: 0x4638F1 */
void RefreshMouseCursorDisplay(void)
{
    g_nInputCursorDirtyLeft_005c85c0 = 319;
    g_nInputCursorDirtyRight_005c845c = 0;
    g_nInputCursorDirtyTop_005c85c4 = 199;
    g_nInputCursorDirtyBottom_005c8460 = 0;
    g_bInputCursorDirty_005c8450 = 0;
    g_bInputCursorBackgroundCaptured_005c80c4 = 0;
    CaptureMouseCursorBackground();
    DrawMouseCursor();
    DIBupdate(g_nInputCursorDirtyLeft_005c85c0,
              g_nInputCursorDirtyTop_005c85c4,
              g_nInputCursorDirtyRight_005c845c,
              g_nInputCursorDirtyBottom_005c8460);
    RestoreMouseCursorBackground();
    return;
}

/* Function start: 0x46396C */
/* The Win32 port draws its own cursor; the software cursor redraw that
 * follows the early return is disabled. */
void ResumeMouseCursorHook(void)
{
    int changed;
    int minX;
    int minY;
    int maxX;
    int maxY;

    return;
    minX = 319;
    maxX = 0;
    minY = 199;
    maxY = 0;
    g_bInputCursorShapeChanged_005c840b = 1;
    changed = 0;
    if (g_pInputViewport_005c8403 != 0) {
        if (g_pDrawnInputCursorShape_0049d4d0 != 0) {
            RestoreSpriteBackground(g_pInputViewport_005c8403,
                                    g_abInputCursorBackground_005c3b10,
                                    (short)g_nDrawnInputCursorX_005c80ac,
                                    (short)g_nDrawnInputCursorY_005c80a8,
                                    g_pDrawnInputCursorShape_0049d4d0,
                                    g_nInputCursorFrame_005c83fd);
            g_pDrawnInputCursorShape_0049d4d0 = 0;
            if (g_nDrawnInputCursorX_005c80ac - 16 < minX)
                minX = g_nDrawnInputCursorX_005c80ac - 16;
            if (g_nDrawnInputCursorX_005c80ac + 16 > maxX)
                maxX = g_nDrawnInputCursorX_005c80ac + 16;
            if (g_nDrawnInputCursorY_005c80a8 - 16 < minY)
                minY = g_nDrawnInputCursorY_005c80a8 - 16;
            if (g_nDrawnInputCursorY_005c80a8 + 16 > maxY)
                maxY = g_nDrawnInputCursorY_005c80a8 + 16;
            changed = 1;
        }
        if (g_nMouseCursorDrawDepth_0049d4d4 > 0 &&
            g_pInputCursorShape_005c83f9 != 0) {
            CaptureSpriteBackground(g_pInputViewport_005c8403,
                                    g_abInputCursorBackground_005c3b10,
                                    g_nQueuedInputX_005c83f0,
                                    g_nQueuedInputY_005c83f2,
                                    g_pInputCursorShape_005c83f9,
                                    g_nInputCursorFrame_005c83fd);
            DrawSpriteDefault(g_pInputViewport_005c8403, g_nQueuedInputX_005c83f0,
                              g_nQueuedInputY_005c83f2,
                              g_pInputCursorShape_005c83f9,
                              g_nInputCursorFrame_005c83fd);
            g_pDrawnInputCursorShape_0049d4d0 = g_pInputCursorShape_005c83f9;
            g_nDrawnInputCursorX_005c80ac = g_nQueuedInputX_005c83f0;
            g_nDrawnInputCursorY_005c80a8 = g_nQueuedInputY_005c83f2;
            if (g_nDrawnInputCursorX_005c80ac - 16 < minX)
                minX = g_nDrawnInputCursorX_005c80ac - 16;
            if (g_nDrawnInputCursorX_005c80ac + 16 > maxX)
                maxX = g_nDrawnInputCursorX_005c80ac + 16;
            if (g_nDrawnInputCursorY_005c80a8 - 16 < minY)
                minY = g_nDrawnInputCursorY_005c80a8 - 16;
            if (g_nDrawnInputCursorY_005c80a8 + 16 > maxY)
                maxY = g_nDrawnInputCursorY_005c80a8 + 16;
            changed = 1;
        }
        if (changed != 0)
            DIBupdate(minX, minY, maxX, maxY);
    }
    return;
}

/* Function start: 0x463BA1 */
/* Shows the cursor.  Only the first statement is live in the Win32 port;
 * the earlier counting and software-cursor versions follow the returns. */
void EnableMouseCursorDrawing(void)
{
    int minX;
    int minY;
    int maxX;
    int maxY;

    g_nMouseCursorDrawDepth_0049d4d4 = 1;
    return;
    g_nMouseCursorDrawDepth_0049d4d4++;
    return;
    if (g_pInputViewport_005c8403 != 0 && g_pInputCursorShape_005c83f9 != 0)
        CaptureSpriteBackground(g_pInputViewport_005c8403,
                                g_abInputCursorBackground_005c3b10,
                                g_nQueuedInputX_005c83f0,
                                g_nQueuedInputY_005c83f2,
                                g_pInputCursorShape_005c83f9,
                                g_nInputCursorFrame_005c83fd);
    g_pDrawnInputCursorShape_0049d4d0 = g_pInputCursorShape_005c83f9;
    return;
    g_bInputCursorShapeChanged_005c840b = 1;
    minX = 319;
    maxX = 0;
    minY = 199;
    maxY = 0;
    if (g_nMouseCursorDrawDepth_0049d4d4 > 0 &&
        g_pInputViewport_005c8403 != 0 &&
        g_pInputCursorShape_005c83f9 != 0) {
        if (g_pDrawnInputCursorShape_0049d4d0 != 0) {
            RestoreSpriteBackground(g_pInputViewport_005c8403,
                                    g_abInputCursorBackground_005c3b10,
                                    (short)g_nDrawnInputCursorX_005c80ac,
                                    (short)g_nDrawnInputCursorY_005c80a8,
                                    g_pInputCursorShape_005c83f9,
                                    g_nInputCursorFrame_005c83fd);
            if (g_nDrawnInputCursorX_005c80ac - 16 < minX)
                minX = g_nDrawnInputCursorX_005c80ac - 16;
            if (g_nDrawnInputCursorX_005c80ac + 16 > maxX)
                maxX = g_nDrawnInputCursorX_005c80ac + 16;
            if (g_nDrawnInputCursorY_005c80a8 - 16 < minY)
                minY = g_nDrawnInputCursorY_005c80a8 - 16;
            if (g_nDrawnInputCursorY_005c80a8 + 16 > maxY)
                maxY = g_nDrawnInputCursorY_005c80a8 + 16;
        }
        CaptureSpriteBackground(g_pInputViewport_005c8403,
                                g_abInputCursorBackground_005c3b10,
                                g_nQueuedInputX_005c83f0,
                                g_nQueuedInputY_005c83f2,
                                g_pInputCursorShape_005c83f9,
                                g_nInputCursorFrame_005c83fd);
        DrawSpriteDefault(g_pInputViewport_005c8403, g_nQueuedInputX_005c83f0,
                          g_nQueuedInputY_005c83f2,
                          g_pInputCursorShape_005c83f9,
                          g_nInputCursorFrame_005c83fd);
        g_pDrawnInputCursorShape_0049d4d0 = (unsigned char *)1;
        g_nDrawnInputCursorX_005c80ac = g_nQueuedInputX_005c83f0;
        g_nDrawnInputCursorY_005c80a8 = g_nQueuedInputY_005c83f2;
        if (g_nDrawnInputCursorX_005c80ac - 16 < minX)
            minX = g_nDrawnInputCursorX_005c80ac - 16;
        if (g_nDrawnInputCursorX_005c80ac + 16 > maxX)
            maxX = g_nDrawnInputCursorX_005c80ac + 16;
        if (g_nDrawnInputCursorY_005c80a8 - 16 < minY)
            minY = g_nDrawnInputCursorY_005c80a8 - 16;
        if (g_nDrawnInputCursorY_005c80a8 + 16 > maxY)
            maxY = g_nDrawnInputCursorY_005c80a8 + 16;
        DIBupdate(minX, minY, maxX, maxY);
    }
    return;
}

/* Function start: 0x463E19 */
/* Hides the cursor.  Only the first statement is live in the Win32 port;
 * the earlier counting and software-cursor versions follow the returns. */
void DisableMouseCursorDrawing(void)
{
    g_nMouseCursorDrawDepth_0049d4d4 = -1;
    return;
    g_nMouseCursorDrawDepth_0049d4d4--;
    return;
    g_pDrawnInputCursorShape_0049d4d0 = 0;
    return;
    if (g_nMouseCursorDrawDepth_0049d4d4 > 0 &&
        g_pDrawnInputCursorShape_0049d4d0 != 0) {
        if (g_pInputViewport_005c8403 != 0 &&
            g_pInputCursorShape_005c83f9 != 0) {
            RestoreSpriteBackground(g_pInputViewport_005c8403,
                                    g_abInputCursorBackground_005c3b10,
                                    (short)g_nDrawnInputCursorX_005c80ac,
                                    (short)g_nDrawnInputCursorY_005c80a8,
                                    g_pInputCursorShape_005c83f9,
                                    g_nInputCursorFrame_005c83fd);
            DIBupdate(g_nDrawnInputCursorX_005c80ac - 16,
                      g_nDrawnInputCursorY_005c80a8 - 16,
                      g_nDrawnInputCursorX_005c80ac + 16,
                      g_nDrawnInputCursorY_005c80a8 + 16);
        }
        g_pDrawnInputCursorShape_0049d4d0 = 0;
    }
    g_nMouseCursorDrawDepth_0049d4d4--;
    return;
}

/* Function start: 0x463EEE */
void SetMouseCursorShape(unsigned char *shape, short frame)
{
    g_bInputCursorShapeChanged_005c840b = 1;
    if (g_pInputViewport_005c8403 != 0 &&
        g_nMouseCursorDrawDepth_0049d4d4 > 0 &&
        g_pDrawnInputCursorShape_0049d4d0 != 0) {
        RestoreSpriteBackground(g_pInputViewport_005c8403,
                                g_abInputCursorBackground_005c3b10,
                                (short)g_nDrawnInputCursorX_005c80ac,
                                (short)g_nDrawnInputCursorY_005c80a8,
                                g_pDrawnInputCursorShape_0049d4d0,
                                g_nInputCursorFrame_005c83fd);
        g_pDrawnInputCursorShape_0049d4d0 = 0;
    }
    g_pInputCursorShape_005c83f9 = shape;
    g_nInputCursorFrame_005c83fd = frame;
    return;
}

/* Function start: 0x463F74 */
void SetMouseHomePosition(short x, short y)
{
    g_nQueuedInputX_005c83f0 = x;
    g_nQueuedInputY_005c83f2 = y;
    SetInputCursorHostPosition(x, y);
    return;
}

/* Function start: 0x463FAA */
void ApplyPackedMousePosition(ShortPoint point)
{
    g_nQueuedInputX_005c83f0 = point.x;
    g_nQueuedInputY_005c83f2 = point.y;
    SetInputCursorHostPosition(point.x, point.y);
    return;
}

/* Function start: 0x463FE0 */
/* Nothing in the shipped executable calls this.  Reads back the mouse
 * position SetMouseHomePosition stored. */
void GetMouseHomePosition(ShortPoint *position)
{
    position->x = g_nQueuedInputX_005c83f0;
    position->y = g_nQueuedInputY_005c83f2;
    return;
}

/* Function start: 0x464009 */
void SetFrameTimerPeriod(short period)
{
    SetMultimediaTimerCallback((int)period);
}

/* Function start: 0x464021 */
void SetFrameTimerAndWait(short period)
{
    SetFrameTimerPeriod(period);
    WaitForFrameTick();
}

/* Function start: 0x46403D */
void SetFrameTimerPeriodDirect(short p)
{
    SetMultimediaTimerCallback((int)p);
}

/* Function start: 0x464055 */
void WaitForFrameTick(void)
{
    while (g_nFrameTimerPending_005c844c != 0) ;
}

/* Function start: 0x464072 */
short IsFrameTickElapsed(void)
{
    return g_nFrameTimerPending_005c844c == 0;
}

#pragma function(memset)
/* Function start: 0x46409B */
void *AllocateZeroedRecords(int count, short elementSize, short flags)
{
    void *allocation;

    allocation = AllocateTaggedMemory(
        count * elementSize, (short)(flags | 0x40));
    memset(allocation, 0, count * elementSize);
    return allocation;
}

/* Function start: 0x4640E4 */
/* Nothing in the shipped executable calls this; the DOS far-heap dump. */
void DumpFarHeapHook(void)
{
    return;
}

#pragma intrinsic(memset)
/* Function start: 0x4640F4 */
unsigned short EnableHighMemoryResources(void)
{
    g_bHighMemoryResourcesEnabled_005c80e4 = 1;
    DAT_005c8458 = 0x1000;
    DAT_005c82a4 = 0x1000;
    return 1;
}

/* Function start: 0x464123 */
void *AllocateDefaultMemory(unsigned int size)
{
    return AllocateTaggedMemory(size, 0);
}

/* Function start: 0x464141 */
int GetSoundHardwareFlag(void)
{
    return 0;
}

/* Function start: 0x464153 */
/* Nothing in the shipped executable calls this.  Frees a block from
 * AllocateDefaultMemory. */
void FreeDefaultMemory(void *block)
{
    ReleasePacketHandle(block);
}

/* Function start: 0x46416A */
void TimerResetHook(void)
{
    return;
}

/* Function start: 0x46417A */
DwordPtr IdentityDword(DwordPtr value)
{
    return value;
}

/* Function start: 0x46418D */
unsigned int IdentityWord(unsigned int v)
{
    return v;
}

/* Function start: 0x4641A0 */
void TimerStopHook(void)
{
    return;
}

/* Function start: 0x4641B0 */
unsigned int GetAvailableFarMemory(void)
{
    return 0x3e8000;
}

/* Function start: 0x4641C5 */
unsigned int GetLargestFreeMemoryBlock(void)
{
    return 0x3e8000;
}

/* Function start: 0x4641DA */
/* Nothing in the shipped executable calls this; the DOS EMS start-up, reporting no EMS. */
int InitializeEms(void)
{
    return 0;
}

/* Function start: 0x4641EC */
/* Nothing in the shipped executable calls this. */
void FormatEmsHeapHook(void)
{
    return;
}

/* Function start: 0x4641FC */
/* Nothing in the shipped executable calls this; EMS allocations always fail. */
void *AllocateEmsBlock(unsigned int size)
{
    return 0;
}

/* Function start: 0x46420E */
/* Nothing in the shipped executable calls this. */
unsigned int GetEmsTotalFree(void)
{
    return 0;
}

/* Function start: 0x464220 */
/* Nothing in the shipped executable calls this. */
short CheckEmsPointer(void *pointer)
{
    return 0;
}

/* Function start: 0x464233 */
/* Nothing in the shipped executable calls this. */
void FreeEmsBlockHook(void *block)
{
    return;
}

/* Function start: 0x464243 */
/* Nothing in the shipped executable calls this; no pointer is an EMS pointer. */
int IsEmsPointer(void *pointer)
{
    return 0;
}

/* Function start: 0x464255 */
/* Saves the EMS page mapping under DOS; there is none to save on Win32. */
short EmsSaveMapping(void)
{
    return 0;
}

/* Function start: 0x464268 */
/* Restores the EMS page mapping under DOS; there is none on Win32. */
short EmsRestoreMapping(void)
{
    return 0;
}

/* Function start: 0x46427B */
/* Nothing in the shipped executable calls this.  Flushes the keys the
 * keyboard handler has queued. */
void FlushBiosKeys(void)
{
    ClearInputKeyStatePreservingModifiers();
}

#pragma function(memset)
/* Function start: 0x46428B */
void ClearInputKeyStatePreservingModifiers(void)
{
    unsigned int control = g_abInputKeyState_005c80f0[0x1d];
    unsigned int alt = g_abInputKeyState_005c80f0[0x38];

    memset(g_abInputKeyState_005c80f0, 0,
           sizeof(g_abInputKeyState_005c80f0));
    g_abInputKeyState_005c80f0[0x1d] = control;
    g_abInputKeyState_005c80f0[0x38] = alt;
    ClearDebugPauseFlags();
}

/* Function start: 0x4642D6 */
void ClearInputKeyState(void)
{
    memset(g_abInputKeyState_005c80f0, 0,
           sizeof(g_abInputKeyState_005c80f0));
    ClearDebugPauseFlags();
}

/* Function start: 0x4642FA */
/* Nothing in the shipped executable calls this; the DOS keyboard handler's reboot request. */
void RebootMachineHook(void)
{
    return;
}

/* Function start: 0x46430A */
/* Nothing in the shipped executable calls this. */
void KeyboardNullHook(void)
{
    return;
}

#pragma intrinsic(memset)
/* Function start: 0x46431A */
void SetInputKeyState(int scanCode, unsigned char pressed)
{
    if (scanCode >= 0 && scanCode < 0x80) {
        g_abInputKeyState_005c80f0[scanCode] = pressed;
        return;
    }
    SystemDebugPrintf("keyboard almost messed up\n");
    ClearDebugPauseFlags();
    PumpMessagesDuringWait();
    exit(1);
}

#pragma function(memset)
/* Function start: 0x46436E */
void BuildObjectDepthOrder(void)
{
    int maximumDistance;
    int farthestObject;
    int nextObject;
    int sortedIndex;
    int nextDistance;
    int object;
    int candidate;

    maximumDistance = -999999999;
    farthestObject = -1;
    nextObject = farthestObject;
    memset(g_anObjectDepthPlaced_005c8180, 0,
           sizeof(g_anObjectDepthPlaced_005c8180));
    for (object = 0; object < SPACE_OBJECT_COUNT; object++) {
        if (maximumDistance <
            (int)(unsigned short)g_asObjectDistance_00493ae8[object]) {
            maximumDistance =
                (int)(unsigned short)g_asObjectDistance_00493ae8[object];
            farthestObject = object;
        }
    }
    sortedIndex = 0;
    nextObject = farthestObject;
    for (object = 0; object < SPACE_OBJECT_COUNT; object++) {
        g_anSortedObject_005c82c0[sortedIndex] = nextObject;
        if (nextObject == -1)
            return;
        g_anObjectDepthPlaced_005c8180[nextObject] = 1;
        nextDistance = -1;
        nextObject = nextDistance;
        for (candidate = 0; candidate < SPACE_OBJECT_COUNT;
             candidate++) {
            if (g_anObjectDepthPlaced_005c8180[candidate] == 0 &&
                g_asObjectScreenX_00493598[candidate] != (short)0x8001 &&
                nextDistance <
                    (int)(unsigned short)
                        g_asObjectDistance_00493ae8[candidate] &&
                (int)(unsigned short)g_asObjectDistance_00493ae8[candidate] <=
                    maximumDistance) {
                nextObject = candidate;
                nextDistance =
                    (int)(unsigned short)
                        g_asObjectDistance_00493ae8[candidate];
            }
        }
        sortedIndex++;
    }
}

/* Function start: 0x4644DA */
void draw_sorted_objects_to_buffer(void)
{
    int drawObject;
    int sortedIndex;
    int object;
#ifdef SDL_PORT
    int objectClass;
    float enhancedScreenX;
    float enhancedScreenY;
    short projectedScreenX;
    short projectedScreenY;
#endif

    for (sortedIndex = 0; sortedIndex < SPACE_OBJECT_COUNT;
         sortedIndex++) {
        object = g_anSortedObject_005c82c0[sortedIndex];
        if (object < 0)
            break;
        if (g_asObjectType_00495298[object] < 0)
            break;
#ifdef SDL_PORT
        objectClass = g_aeObjectClass_00495328[object];
        /* The enhanced backend records each sprite at a sub-pixel position
         * instead of stamping it into the indexed buffer.  Redo the
         * perspective divide in double precision, and only trust the result
         * when it rounds to the screen position the game already computed --
         * that is what proves it is the same projection and not an object the
         * game placed some other way. */
        enhancedScreenX = (float)(short)(
            g_asObjectScreenX_00493598[object] + g_nViewCenterX_005c80d8);
        enhancedScreenY = (float)(short)(
            g_asObjectScreenY_00493628[object] + g_nViewCenterY_005c80da);
        if (objectClass != OBJECT_CLASS_NULL &&
            objectClass != OBJECT_CLASS_FIXED_OBJECT &&
            object != g_nNavPointerObject_004931b8 &&
            g_aObjectViewPosition_0059afa0[object].z != 0) {
            projectedScreenX = (short)(DivideFixed(
                MultiplyFixed(
                    (short)(g_nScreenWidth_0049d4d8 & ~1) << 7,
                    g_aObjectViewPosition_0059afa0[object].x),
                g_aObjectViewPosition_0059afa0[object].z) >> 8);
            projectedScreenY = (short)(DivideFixed(
                MultiplyFixed(
                    (short)(g_nScreenWidth_0049d4d8 & ~1) << 7,
                    g_aObjectViewPosition_0059afa0[object].y),
                g_aObjectViewPosition_0059afa0[object].z) >> 8);
            if (projectedScreenX == g_asObjectScreenX_00493598[object] &&
                projectedScreenY == g_asObjectScreenY_00493628[object]) {
                enhancedScreenX =
                    (float)g_nViewCenterX_005c80d8 +
                    (float)(((double)(g_nScreenWidth_0049d4d8 & ~1) * 0.5 *
                             g_aObjectViewPosition_0059afa0[object].x) /
                            g_aObjectViewPosition_0059afa0[object].z);
                enhancedScreenY =
                    (float)g_nViewCenterY_005c80da +
                    (float)(((double)(g_nScreenWidth_0049d4d8 & ~1) * 0.5 *
                             g_aObjectViewPosition_0059afa0[object].y) /
                            g_aObjectViewPosition_0059afa0[object].z);
            }
        } else if (objectClass == OBJECT_CLASS_FIXED_OBJECT &&
                   g_asObjectType_00495298[object] ==
                       OBJECT_DATA_THRUSTERS) {
            SdlGetThrusterScreenPosition(
                (short)object, &enhancedScreenX, &enhancedScreenY);
        }
#endif
        if (g_aeObjectClass_00495328[object] != OBJECT_CLASS_NULL) {
            switch (g_aeObjectClass_00495328[object]) {
            case OBJECT_CLASS_STAR:
#ifndef SDL_PORT
            case OBJECT_CLASS_PLANET:
#else
                /* WCDX fix: planets use their own scaled sprite instead of
                 * a frame from the shared constellation shape. */
#endif
            case OBJECT_CLASS_DUST:
                drawObject = object;
                g_asObjectDrawX_00493748[drawObject] = (short)(
                    g_asObjectScreenX_00493598[drawObject] +
                    g_nViewCenterX_005c80d8);
                g_asObjectDrawY_004937d8[drawObject] = (short)(
                    g_asObjectScreenY_00493628[drawObject] +
                    g_nViewCenterY_005c80da);
                if (g_nNavPointerObject_004931b8 == drawObject) {
#ifdef SDL_PORT
                    if (!SdlRecordSpaceSprite(
                            &g_stViewBuffer_005d2b00, enhancedScreenX,
                            enhancedScreenY,
                            g_apObjectShape_00493868[drawObject],
                            g_asObjectViewFrame_00493508[drawObject], 0, 0x100, 0))
#endif
                    DrawSpriteDefault(
                        &g_stViewBuffer_005d2b00,
                        g_asObjectDrawX_00493748[drawObject],
                        g_asObjectDrawY_004937d8[drawObject],
                        g_apObjectShape_00493868[drawObject],
                        g_asObjectViewFrame_00493508[drawObject]);
                } else {
#ifdef SDL_PORT
                    if (!SdlRecordSpaceSprite(
                            &g_stViewBuffer_005d2b00, enhancedScreenX,
                            enhancedScreenY,
                            g_pConstellationShape_005d2c4c,
                            g_asObjectViewFrame_00493508[drawObject], 0, 0x100, 0))
#endif
                    DrawSpriteDefault(
                        &g_stViewBuffer_005d2b00,
                        g_asObjectDrawX_00493748[drawObject],
                        g_asObjectDrawY_004937d8[drawObject],
                        g_pConstellationShape_005d2c4c,
                        g_asObjectViewFrame_00493508[drawObject]);
                }
                break;
            default:
                drawObject = object;
                g_asObjectDrawX_00493748[drawObject] = (short)(
                    g_asObjectScreenX_00493598[drawObject] +
                    g_nViewCenterX_005c80d8);
                g_asObjectDrawY_004937d8[drawObject] = (short)(
                    g_asObjectScreenY_00493628[drawObject] +
                    g_nViewCenterY_005c80da);
                if (g_apObjectShape_00493868[drawObject] != 0) {
#ifdef SDL_PORT
                    if (!SdlRecordSpaceSprite(
                            &g_stViewBuffer_005d2b00, enhancedScreenX,
                            enhancedScreenY,
                            g_apObjectShape_00493868[drawObject],
                            g_asObjectViewFrame_00493508[drawObject],
                            g_asObjectScreenAngle_004936b8[drawObject],
                            g_asObjectScreenScale_00493a58[drawObject],
                            g_asObjectFlip_004939c8[drawObject]))
#endif
                    DrawSpriteScaled(
                        &g_stViewBuffer_005d2b00,
                        g_asObjectDrawX_00493748[drawObject],
                        g_asObjectDrawY_004937d8[drawObject],
                        g_apObjectShape_00493868[drawObject],
                        g_asObjectViewFrame_00493508[drawObject],
                        g_asObjectScreenAngle_004936b8[drawObject],
                        g_asObjectScreenScale_00493a58[drawObject],
                        g_asObjectFlip_004939c8[drawObject]);
                }
                break;
            }
        }
    }
    return;
}

/* Function start: 0x46470E */
void intro_drawbackgroundships(void)
{
    int drawObject;
    int object;

    for (object = 0; object < SPACE_OBJECT_COUNT; object++) {
        if (g_asObjectType_00495298[object] < 0)
            break;
        if (g_aeObjectClass_00495328[object] != OBJECT_CLASS_NULL) {
            switch (g_aeObjectClass_00495328[object]) {
            case OBJECT_CLASS_STAR:
#ifndef SDL_PORT
            case OBJECT_CLASS_PLANET:
#else
                /* Erase planets with the same scaled geometry used to draw
                 * them, leaving no stale pixels around the replacement. */
#endif
            case OBJECT_CLASS_DUST:
                drawObject = object;
                if (drawObject == g_nNavPointerObject_004931b8)
                    DrawSolidColourSprite(
                        &g_stViewBuffer_005d2b00,
                        g_asObjectDrawX_00493748[drawObject],
                        g_asObjectDrawY_004937d8[drawObject],
                        g_apObjectShape_00493868[drawObject],
                        g_asObjectViewFrame_00493508[drawObject],
                        g_cPrimaryViewBufferColour_0049cb88);
                else
                    DrawSolidColourSprite(
                        &g_stViewBuffer_005d2b00,
                        g_asObjectDrawX_00493748[drawObject],
                        g_asObjectDrawY_004937d8[drawObject],
                        g_pConstellationShape_005d2c4c,
                        g_asObjectViewFrame_00493508[drawObject],
                        g_cPrimaryViewBufferColour_0049cb88);
                break;
            default:
                drawObject = object;
                if (g_apObjectShape_00493868[drawObject] != 0)
                    DrawSolidColourSpriteScaled(
                        &g_stViewBuffer_005d2b00,
                        g_asObjectDrawX_00493748[drawObject],
                        g_asObjectDrawY_004937d8[drawObject],
                        g_apObjectShape_00493868[drawObject],
                        g_asObjectViewFrame_00493508[drawObject],
                        g_asObjectScreenAngle_004936b8[drawObject],
                        g_asObjectScreenScale_00493a58[drawObject],
                        g_asObjectFlip_004939c8[drawObject],
                        g_cPrimaryViewBufferColour_0049cb88);
                break;
            }
        }
    }
    return;
}

/* Function start: 0x4648C5 */
void set_up_screen_viewport(signed char mode)
{
    g_cScreenViewportMode_005c82a6 = mode;
    switch (g_cScreenViewportMode_005c82a6) {
    case 4:
    case 5:
        g_pScreenViewportGeometry_005c82b0 =
            &g_aScreenViewportGeometry_0049d4e8[
                (int)g_cScreenViewportMode_005c82a6];
        break;
    default:
        g_pScreenViewportGeometry_005c82b0 =
            (const ScreenViewportGeometry *)(
                g_pCockpitBackgroundPacket_0049a5f0 +
                ((ScreenViewportPacket *)
                     g_pCockpitBackgroundPacket_0049a5f0)
                    ->geometryOffsets[0]);
        break;
    }
    if (g_nCockpitDisplayMode_0049d71c != 0 &&
        g_nCockpitDisplayMode_0049d71c != -2) {
        g_nScreenWidth_0049d4d8 =
            g_pScreenViewportGeometry_005c82b0->width;
        g_nViewCenterX_005c80d8 =
            (short)(g_nScreenWidth_0049d4d8 / 2);
        g_nScreenHeight_0049d4dc =
            g_pScreenViewportGeometry_005c82b0->height;
        g_nViewCenterY_005c80da =
            (short)(g_nScreenHeight_0049d4dc / 2);
        g_nViewportOriginX_005c849e =
            g_pScreenViewportGeometry_005c82b0->originX;
        g_nViewportOriginY_005c849c =
            g_pScreenViewportGeometry_005c82b0->originY;
        g_nViewCenterX_005c80d8 += g_nViewportOriginX_005c849e;
        g_nViewCenterY_005c80da += g_nViewportOriginY_005c849c;
        g_nScreenWidth_0049d4d8 = 320;
        g_nScreenHeight_0049d4dc = 200;
    } else {
        g_nScreenWidth_0049d4d8 =
            g_pScreenViewportGeometry_005c82b0->width;
        g_nViewCenterX_005c80d8 =
            (short)(g_nScreenWidth_0049d4d8 / 2);
        g_nScreenHeight_0049d4dc =
            g_pScreenViewportGeometry_005c82b0->height;
        g_nViewCenterY_005c80da =
            (short)(g_nScreenHeight_0049d4dc / 2);
        g_nViewportOriginX_005c849e =
            g_pScreenViewportGeometry_005c82b0->originX;
        g_nViewportOriginY_005c849c =
            g_pScreenViewportGeometry_005c82b0->originY;
    }
    return;
}

/* Function start: 0x464A5E */
/* Nothing in the shipped executable calls this. */
int SetupBackBufferHook(void)
{
    return 0;
}

/* Function start: 0x464A70 */
/* Nothing in the shipped executable calls this. */
void FreeVideoPagesHook(void)
{
    return;
}

/* Function start: 0x464A80 */
/* Nothing in the shipped executable calls this. */
void DumpVideoPageMapHook(void)
{
    return;
}

/* Function start: 0x464A90 */
void FinalizeInputManagerHook(void)
{
    return;
}

/* Function start: 0x464AA0 */
void MouseIdleHook(void)
{
    return;
}

/* Function start: 0x464AB0 */
short GetNavRangeSentinel(void)
{
    return 0x8000;
}

/* Function start: 0x464AC4 */
short GetOriginalFreeMemory(void)
{
    return 0x8000;
}

/* Function start: 0x464AD8 */
/* Nothing in the shipped executable calls this. */
void CalibrateDelayHook(void)
{
    return;
}

/* Function start: 0x464AE8 */
void StartupHook(unsigned int (*callback)(unsigned int, short))
{
    (void)callback;
    return;
}

/* Function start: 0x464AF8 */
/* Nothing in the shipped executable calls this; no BIOS keyboard intercept on Win32. */
short HasKeyIntercept(void)
{
    return 0;
}

/* Function start: 0x464B0B */
unsigned int JoystickEdgeHook(int button)
{
    (void)button;
    return;
}

/* Function start: 0x464B1B */
/* Nothing in the shipped executable calls this.  Allocates a near block. */
void *AllocateNearMemory(short size)
{
    return AllocateTaggedMemory(size, 0x40);
}

/* Function start: 0x464B3A */
void FreeIfNotNull(void *p)
{
    if (p != 0)
        free(p);
    return;
}

/* Function start: 0x464B60 */
unsigned int GetStartupErrorCode(int vector)
{
    (void)vector;
    return 0;
}

/* Function start: 0x464B72 */
/* Nothing in the shipped executable calls this. */
void SpeedCalibrationHook(void)
{
    return;
}

/* Function start: 0x464B82 */
/* WC2 hands this the drive letter index; the body ignores it. */
void PersonnelDriveHook(short drive)
{
    return;
}

/* Function start: 0x464B92 */
unsigned short SelectDiskDriveHook(short drive)
{
    return 0;
}

/* Function start: 0x464BA5 */
short GetCurrentDiskDriveHook(void)
{
    return 0;
}

/* Function start: 0x464BB8 */
unsigned short GetShutdownErrorCode(unsigned char *driveState)
{
    (void)driveState;
    return 0;
}

/* Function start: 0x464BCB */
void VideoReleaseHook(void)
{
    return;
}

/* Function start: 0x464BDB */
void ExitCleanupHook(void)
{
    return;
}

/* Function start: 0x464BEB */
/* Nothing in the shipped executable calls this; never a Tandy keyboard on Win32. */
short DetectTandyKeyboard(void)
{
    return 0;
}

#pragma function(abs)
/* Function start: 0x464BFE */
short IsVectorWithinRange(FixedVector *vector, short range)
{
    int magnitude;
    int rangeFixed;

    rangeFixed = (int)range << 8;
    magnitude = Vector_magnitude(vector);
    if (abs(rangeFixed) >= magnitude)
        return 1;
    return 0;
}

/* Function start: 0x464C4B */
unsigned int shrink_vector(FixedVector *vector)
{
    unsigned int shrinking;

    do {
        shrinking = 0;
        shrinking |= shrink(&vector->x);
        shrinking |= shrink(&vector->y);
        shrinking |= shrink(&vector->z);
    } while (shrinking != 0);
    return 0;
}

/* Function start: 0x464CA4 */
unsigned int shrink(int *component)
{
    unsigned short whole;
    unsigned short fraction;

    *component = *component / 2;
    whole = (unsigned short)((unsigned int)*component >> 16);
    fraction = (unsigned short)(*component & 0xffff);
    if (whole == 0) {
        if (fraction <= 0x0f00)
            return 0;
        return 1;
    } else if ((short)whole == -1) {
        if (fraction >= 0xf100)
            return 0;
        return 1;
    } else if (whole != 0)
        return 1;
}

/* Function start: 0x464D5F */
short ServiceInputDevices(short deviceMask)
{
    (void)deviceMask;
    g_nNextInputQueueFlushTick_0049d4cc = g_nInputClock_005c84a8 + 20;
    if (g_nInputPollPeriod_0049d6d8 == 1)
        PumpWindowMessages(1);
    else
        PumpWindowMessages(0);
    if (g_nNextInputPollTick_0049d6d4 <= g_nInputClock_005c84a8) {
        FlushInputEvents();
        PumpWindowMessages(0);
        g_nNextInputPollTick_0049d6d4 +=
            g_nInputClock_005c84a8 + g_nInputPollPeriod_0049d6d8;
        return 1;
    }
    PumpWindowMessages(1);
    if (g_bSceneEscapeRequested_0049d4b0 != 0)
        return 1;
    return 0;
}

/* Function start: 0x464E06 */
void SetInputViewport(Viewport *viewport)
{
    g_pInputViewport_005c8403 = viewport;
    return;
}

/* Function start: 0x464E1E */
void ClearInputPump(void)
{
    ConfigureInputPump(1, 0);
}

/* Function start: 0x464E35 */
void SetMenuInputPump(void)
{
    ConfigureInputPump(1, PollMenuInputDevices);
}

/* Function start: 0x464E4F */
int ConfigureInputPump(int slot, void (*pump)(void))
{
    if (slot == 1)
        g_pfnInputPump_005c840c = pump;
    return 0;
}

/* Function start: 0x464E73 */
short LoadInputCursorShape(const char *filename, short section,
                           short flags)
{
    void *shape;

    (void)flags;
    if (filename != 0) {
        shape = LoadNamedPacket(filename, section, 0, 0, 0, 1);
        g_pInputCursorShape_005c83f9 = shape;
        g_pInputManagerState_005c8464->cursorShape = shape;
        if (shape == 0)
            return 0;
    }
    SetInputViewport(0);
    return 1;
}

/* Function start: 0x464EE0 */
void InitializeInputDriverHook(void)
{
    return;
}

/* Function start: 0x464EF0 */
void FinalizeInputDriverHook(void)
{
    return;
}

/* Function start: 0x464F00 */
/* Nothing in the shipped executable calls this. */
void InputDeviceShutdownHook(void)
{
    return;
}

/* Function start: 0x464F10 */
void FlushPendingInputEvents(void)
{
    FlushInputEvents();
    return;
}

/* Function start: 0x464F25 */
long GetNamedPacketSize(const char *filename, short section)
{
    return GetPacketSize(filename, section);
}

/* Function start: 0x464F45 */
void *LoadNamedPacket(const char *filename, short section,
                      void *destination, unsigned short flags,
                      void *decompressionWorkspace,
                      int registerHandle)
{
    return PacketLoad(filename, section, destination, flags,
                      decompressionWorkspace, registerHandle);
}

/* Function start: 0x464F75 */
short OffsetSceneHotspotBounds(ShortRect *bounds,
                               const SceneHotspot *hotspot,
                               short offsetX, short offsetY)
{
    bounds->left = (short)(hotspot->left + offsetX);
    bounds->right = (short)(hotspot->right + offsetX);
    bounds->bottom = (short)(hotspot->bottom + offsetY);
    bounds->top = (short)(hotspot->top + offsetY);
    return 1;
}

/* Function start: 0x464FD7 */
void ServiceCutsceneRuntimeHook(void)
{
    return;
}

#ifndef SDL_PORT
/* This caller pushes the palette index sign-extended, as for an int
 * parameter (warning C4028 against the header prototype). */
void DIBsetPalette(int index, short *rgb);
#endif

/* Function start: 0x464FE7 */
void SetPaletteEntryFromTriplet(const unsigned char *triplet, short index)
{
    short rgb[3];

    rgb[0] = triplet[0];
    rgb[1] = triplet[1];
    rgb[2] = triplet[2];
    DIBsetPalette(index, rgb);
    return;
}

/* Function start: 0x46502E */
void SetMusicTickRateHook(short period)
{
    (void)period;
    return;
}

/* Function start: 0x46503E */
/* Nothing in the shipped executable calls this. */
void StopMusicTimerHook(void)
{
    return;
}

/* Function start: 0x46504E */
/* Nothing in the shipped executable calls this. */
void MusicTimerIsrHook(void)
{
    return;
}

/* Function start: 0x46505E */
short IsSpriteFrameOverlappingRect(const ShortRect *rectangle,
                                   short x, short y,
                                   unsigned char *shape,
                                   short frame)
{
    int rectangleTop;
    int rectangleRight;
    int rectangleBottom;
    int frameLeft;
    int rectangleLeft;
    ShortRect frameBounds;
    int frameTop;
    int frameRight;
    int frameBottom;

    GetShapeFrameBounds(&frameBounds.left, x, y, shape, frame);
    rectangleLeft = rectangle->left;
    rectangleTop = rectangle->top;
    rectangleRight = rectangle->right;
    rectangleBottom = rectangle->bottom;
    frameLeft = frameBounds.left;
    frameTop = frameBounds.top;
    frameRight = frameBounds.right;
    frameBottom = frameBounds.bottom;
    if (((frameLeft <= rectangleLeft && rectangleLeft <= frameRight) ||
         (frameLeft <= rectangleRight && rectangleRight <= frameRight))) {
        if (frameTop <= rectangleTop && rectangleTop <= frameBottom)
            return 1;
        if (frameTop <= rectangleBottom && rectangleBottom <= frameBottom)
            return 1;
    }
    if (((rectangleLeft <= frameLeft && frameLeft <= rectangleRight) ||
         (rectangleLeft <= frameRight && frameRight <= rectangleRight))) {
        if (rectangleTop <= frameTop && frameTop <= rectangleBottom)
            return 1;
        if (rectangleTop <= frameBottom && frameBottom <= rectangleBottom)
            return 1;
    }
    return 0;
}

/* Function start: 0x4651B7 */
short GetViewportIntersection(ShortRect *intersection,
                              const ShortRect *first,
                              const ShortRect *second)
{
    int pointXs[16];
    int cornerIndex;
    int pointCount;
    int pointYs[16];
    int insideFlags[4];
    int containedCount;
    int x;
    int y;

    pointCount = 0;
    pointXs[pointCount] = first->left;
    pointYs[pointCount] = first->top;
    pointCount++;
    pointXs[pointCount] = first->left;
    pointYs[pointCount] = first->bottom;
    pointCount++;
    pointXs[pointCount] = first->right;
    pointYs[pointCount] = first->top;
    pointCount++;
    pointXs[pointCount] = first->right;
    pointYs[pointCount] = first->bottom;
    pointCount++;
    pointXs[pointCount] = second->left;
    pointYs[pointCount] = second->top;
    pointCount++;
    pointXs[pointCount] = second->left;
    pointYs[pointCount] = second->bottom;
    pointCount++;
    pointXs[pointCount] = second->right;
    pointYs[pointCount] = second->top;
    pointCount++;
    pointXs[pointCount] = second->right;
    pointYs[pointCount] = second->bottom;
    pointCount++;
    pointXs[pointCount] = first->left;
    pointYs[pointCount] = second->top;
    pointCount++;
    pointXs[pointCount] = first->left;
    pointYs[pointCount] = second->bottom;
    pointCount++;
    pointXs[pointCount] = first->right;
    pointYs[pointCount] = second->top;
    pointCount++;
    pointXs[pointCount] = first->right;
    pointYs[pointCount] = second->bottom;
    pointCount++;
    pointXs[pointCount] = second->left;
    pointYs[pointCount] = first->top;
    pointCount++;
    pointXs[pointCount] = second->left;
    pointYs[pointCount] = first->bottom;
    pointCount++;
    pointXs[pointCount] = second->right;
    pointYs[pointCount] = first->top;
    pointCount++;
    pointXs[pointCount] = second->right;
    pointYs[pointCount] = first->bottom;
    pointCount++;

#ifdef SDL_PORT
    containedCount = 0;
#endif
    /* The condition and step are transposed in the original: the loop
     * tests cornerIndex++ (zero on entry) and never runs. */
    for (cornerIndex = 0; cornerIndex++; cornerIndex < pointCount) {
        x = pointXs[cornerIndex];
        y = pointYs[cornerIndex];
        if (first->left <= x && x <= first->right &&
            second->left <= x && x <= second->right &&
            first->top <= y && y <= first->bottom &&
            second->top <= y && y <= second->bottom) {
            if (containedCount == 4)
                return 0;
            insideFlags[containedCount] = 1;
            containedCount++;
        } else {
            if (containedCount == 4)
                return 0;
            insideFlags[containedCount] = 0;
            containedCount++;
        }
    }
    if (containedCount != 4)
        return 0;

    intersection->left = MinShort(
        (short)pointXs[insideFlags[0]],
        MinShort((short)pointXs[insideFlags[1]],
                 (short)pointXs[insideFlags[2]]));
    intersection->top = MinShort(
        (short)pointYs[insideFlags[0]],
        MinShort((short)pointYs[insideFlags[1]],
                 (short)pointYs[insideFlags[2]]));
    intersection->left = MaxShort(
        (short)pointXs[insideFlags[0]],
        MaxShort((short)pointXs[insideFlags[1]],
                 (short)pointXs[insideFlags[2]]));
    intersection->top = MaxShort(
        (short)pointYs[insideFlags[0]],
        MaxShort((short)pointYs[insideFlags[1]],
                 (short)pointYs[insideFlags[2]]));
    return 1;
}

/* Function start: 0x465644 */
void SplitGameClockTicksForNav(unsigned char *parts)
{
    int ticks;

    ticks = (int)GetGameClockTicks();
    parts[0] = (unsigned char)(ticks % 60);
    ticks = ticks / 60;
    parts[1] = (unsigned char)(ticks % 60);
    ticks = ticks / 60;
    parts[2] = (unsigned char)(ticks % 60);
    ticks = ticks / 60;
    parts[3] = (unsigned char)(ticks % 24);
    return;
}

/* Function start: 0x4656CC */
void AllocateApplicationScratchBuffer(void)
{
    g_pApplicationScratchBuffer_005c8483 =
        AllocateTaggedMemory(0x1000, 0);
}

/* Function start: 0x4656EB */
void ReleaseApplicationScratchBuffer(void)
{
    ReleasePacketHandle(g_pApplicationScratchBuffer_005c8483);
    g_pApplicationScratchBuffer_005c8483 = 0;
}

/* Function start: 0x46570E */
unsigned char *GetInputCursorShape(void)
{
    return g_pInputCursorShape_005c83f9;
}
