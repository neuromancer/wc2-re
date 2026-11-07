/*
 *  Win32 library port: random, fixed-point math, text, strings and events.
 *
 *  Retail object 0x4618E0-0x46572F (link order; see docs/ORDER.md).
 */
/* This unit owns storage declared in globals.h as statics. */
#define EVENTMGR_C
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
    g_29_pCurrentTextContext->cursorX = (short)a;
    g_29_pCurrentTextContext->cursorY = (short)b;
}

/* Function start: 0x461D29 */
void SetTextContext(TextContext *context)
{
    g_29_pCurrentTextContext = context;
    if (g_29_pCurrentTextContext != NULL &&
        g_29_pCurrentTextContext->text != NULL)
        *g_29_pCurrentTextContext->text = 0;
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
    return g_29_pCurrentTextContext->font[4 + (unsigned char)i];
#else
    return g_29_pCurrentTextContext->font[4 + (int)i];
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
    if (g_29_pCurrentTextContext == 0)
        return;
    g_29_pCurrentTextContext->cursorX =
        g_29_pCurrentTextContext->viewport->left;
    while (isDone == 0) {
        pixelWidth = g_29_pCurrentTextContext->cursorX;
        while (*cursor == ' ')
            cursor++;
        lineBegin = cursor;
        while (pixelWidth <
               g_29_pCurrentTextContext->viewport->right) {
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
                g_29_pCurrentTextContext
                    ->font[4 + (unsigned char)ch];
#else
                g_29_pCurrentTextContext->font[4 + ch];
#endif
            if (pixelWidth >=
                g_29_pCurrentTextContext->viewport->right) {
                pixelWidth -=
#ifdef SDL_PORT
                    g_29_pCurrentTextContext
                        ->font[4 + (unsigned char)ch];
#else
                    g_29_pCurrentTextContext->font[4 + ch];
#endif
                cursor--;
                wrapPending = 1;
                if (*cursor != ' ') {
                    while (*cursor != ' ' && cursor > text) {
                        pixelWidth -=
                            g_29_pCurrentTextContext
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

        if (g_29_pCurrentTextContext->alignment == 2) {
            previousX = g_29_pCurrentTextContext->cursorX;
            g_29_pCurrentTextContext->cursorX = (short)(
                g_29_pCurrentTextContext->viewport->left +
                ((g_29_pCurrentTextContext->viewport->right -
                  g_29_pCurrentTextContext->viewport->left + 1) -
                 (pixelWidth - g_29_pCurrentTextContext->cursorX + 1) +
                 1) / 2);
        }
        for (drawPos = lineBegin; drawPos < cursor; drawPos++) {
            DrawTextCharacter(*drawPos);
        }
        if (g_29_pCurrentTextContext->alignment == 2)
            g_29_pCurrentTextContext->cursorX = (short)previousX;
        if (wrapPending != 0) {
            g_29_pCurrentTextContext->cursorX =
                g_29_pCurrentTextContext->viewport->left;
            g_29_pCurrentTextContext->cursorY +=
                *(short *)g_29_pCurrentTextContext->font;
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

    if (g_29_pCurrentTextContext == 0)
        return;
    if (character == '\n') {
        g_29_pCurrentTextContext->cursorX =
            g_29_pCurrentTextContext->viewport->left;
        g_29_pCurrentTextContext->cursorY =
            (short)(g_29_pCurrentTextContext->cursorY +
                    *(short *)g_29_pCurrentTextContext->font);
    } else if (character == '\r') {
        g_29_pCurrentTextContext->cursorX =
            g_29_pCurrentTextContext->viewport->left;
    } else if (character != 0) {
        lineHeight = *(short *)g_29_pCurrentTextContext->font;
#ifdef SDL_PORT
        glyphWidth = g_29_pCurrentTextContext->font[
            (unsigned char)character + 4];
#else
        glyphWidth = g_29_pCurrentTextContext->font[character + 4];
#endif
        DrawFontGlyph(character, g_29_pCurrentTextContext,
                      lineHeight, glyphWidth,
                      g_29_pCurrentTextContext->cursorY);
    }
    return;
}

/* Function start: 0x4622BD */
void AppendTextCharacter(char character)
{
    *g_29_pCurrentTextContext->textCursor = character;
    g_29_pCurrentTextContext->textCursor++;
    *g_29_pCurrentTextContext->textCursor = 0;
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
void DosMemmove(void *dst, const void *src, size_t n)
{
    memmove(dst, src, n & 0xffff);
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
    g_18_bEventManagerInitializationState = 1;
    g_18_pEventManager =
        (InputManagerState *)AllocateTaggedMemory(0x21, 0x40);
    return 1;
}

/* Function start: 0x4625CB */
void ShutdownEventManager(void)
{
    g_18_bEventManagerInitializationState = 0;
    ReleasePacketHandle(g_18_pEventManager);
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
    case INPUT_EVENT_JOYSTICK_MOVE:
        QueueInputEvent(type,
            g_aInputDeviceSamplesList[
                g_8_nActiveInputDeviceWord].x,
            g_aInputDeviceSamplesList[
                g_8_nActiveInputDeviceWord].y,
            0, value & INPUT_BUTTON_PRIMARY,
            (value & INPUT_BUTTON_SECONDARY) >> 1, 0, 0, 0);
        break;
    case POLLED_INPUT_BUTTON_DOWN:
        QueueInputEvent(INPUT_EVENT_BUTTON_DOWN,
            g_aInputDeviceSamplesList[
                g_8_nActiveInputDeviceWord].x,
            g_aInputDeviceSamplesList[
                g_8_nActiveInputDeviceWord].y,
            0, value & INPUT_BUTTON_PRIMARY,
            (value & INPUT_BUTTON_SECONDARY) >> 1, 0, 0, 0);
        break;
    case POLLED_INPUT_BUTTON_UP:
        QueueInputEvent(INPUT_EVENT_BUTTON_UP,
            g_aInputDeviceSamplesList[
                g_8_nActiveInputDeviceWord].x,
            g_aInputDeviceSamplesList[
                g_8_nActiveInputDeviceWord].y,
            0, value & INPUT_BUTTON_PRIMARY,
            (value & INPUT_BUTTON_SECONDARY) >> 1, 0, 0, 0);
        break;
    case INPUT_EVENT_MOUSE_MOVE:
        mouse = &g_6_stHostMouseTextBlock;
        QueueInputEvent(type,
                        g_16_stEventQueuePointerData.x,
                        g_16_stEventQueuePointerData.y,
                        0, 0, 0, 0, 0, 0);
        break;
    case INPUT_EVENT_BUTTON_DOWN:
        mouse = &g_6_stHostMouseTextBlock;
        QueueInputEvent(type,
                        g_16_stEventQueuePointerData.x,
                        g_16_stEventQueuePointerData.y,
                        0,
                        mouse->primaryButton,
                        mouse->secondaryButton,
                        mouse->secondaryButton * 2 |
                            mouse->primaryButton | value >> 2,
                        0, 0);
        break;
    case INPUT_EVENT_KEY_DOWN:
        break;
    }
}

/* Function start: 0x462849 */
void QueueInputEventAtCursor(unsigned int type, short primaryButton,
                             short secondaryButton)
{
    /* Preserve the original 16-bit event ID. */
    unsigned int eventType = type & 0xffff;

    QueueInputEvent(eventType, g_16_stEventQueuePointerData.x,
                    g_16_stEventQueuePointerData.y, 0,
                    primaryButton, secondaryButton, 0, 0, 0);
}

/* Function start: 0x462890 */
InputEvent *AllocateInputEvent(void)
{
    int index;

    if (g_nInputEventPoolInitialized != 0) {
        for (index = 0; index < 0x100; index++) {
            if (g_3_anCurrentInputEventEntryAllocation[index] == 0) {
                g_3_anCurrentInputEventEntryAllocation[index] = 1;
                return &g_3_aInputEventArray[index];
            }
        }
    } else {
        memset(g_3_aInputEventArray, 0,
               sizeof(g_3_aInputEventArray));
        memset(g_3_anCurrentInputEventEntryAllocation, 0,
               sizeof(g_3_anCurrentInputEventEntryAllocation));
        g_nInputEventPoolInitialized = 1;
        g_3_anCurrentInputEventEntryAllocation[0] = 1;
        return &g_3_aInputEventArray[0];
    }
    return 0;
}

#pragma intrinsic(memset)
/* Function start: 0x46294F */
void ReleaseInputEvent(InputEvent *event)
{
    int slot;

    for (slot = 0; slot < 0x100; slot++) {
        if (&g_3_aInputEventArray[slot] == event)
            g_3_anCurrentInputEventEntryAllocation[slot] = 0;
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
        modifiers |= INPUT_MOD_SHIFT;
    if (GetControlKeyState() != 0)
        modifiers |= INPUT_MOD_CONTROL;
    if (GetKeyboardModifiers() != 0)
        modifiers |= INPUT_MOD_ALT;
    if (primaryButton != 0)
        modifiers |= INPUT_MOD_PRIMARY_BUTTON;
    if (secondaryButton != 0)
        modifiers |= INPUT_MOD_SECONDARY_BUTTON;

    if (g_pInputEventHead == 0) {
        g_pInputEventHead = AllocateInputEvent();
        if (g_pInputEventHead == 0) {
            ReleaseInputEventQueue();
            return;
        }
        g_pInputEventTail = g_pInputEventHead;
        g_pInputEventTail->next = 0;
        g_pInputEventTail->previous = 0;
    } else {
        g_pInputEventTail->next = AllocateInputEvent();
        if (g_pInputEventTail->next == 0) {
            ReleaseInputEventQueue();
            return;
        }
        g_pInputEventTail->next->previous =
            g_pInputEventTail;
        g_pInputEventTail = g_pInputEventTail->next;
        g_pInputEventTail->next = 0;
    }
    g_pInputEventTail->type = type;
    g_pInputEventTail->modifiers = modifiers;
    g_pInputEventTail->x = x;
    g_pInputEventTail->y = y;
    g_pInputEventTail->value = value;
    g_pInputEventTail->primaryButton = (short)primaryButton;
    g_pInputEventTail->secondaryButton = (short)secondaryButton;
    g_pInputEventTail->field_14 = field14;
    g_pInputEventTail->field_18 = field18;
    g_pInputEventTail->status = 1;
    if (g_pInputEventTail->type == INPUT_EVENT_MOUSE_MOVE) {
        g_16_stEventQueuePointerData.x = g_pInputEventTail->x;
        g_16_stEventQueuePointerData.y = g_pInputEventTail->y;
    }
    if (g_pInputEventTail->type == INPUT_EVENT_BUTTON_UP) {
        g_nInputDoubleClickDeadline =
            g_196_nInputTimerTicks + 25;
        g_nPreviousPrimaryButton = primaryButton;
        g_nPreviousSecondaryButton = secondaryButton;
    }
    if (g_pInputEventTail->type == INPUT_EVENT_BUTTON_DOWN ||
        g_pInputEventTail->type == INPUT_EVENT_LEGACY_8 ||
        g_pInputEventTail->type == INPUT_EVENT_LEGACY_9) {
        if ((g_pInputEventTail->primaryButton != 0 &&
             g_nPreviousPrimaryButton != 0) ||
            (g_pInputEventTail->secondaryButton != 0 &&
             g_nPreviousSecondaryButton != 0)) {
            if (g_nInputDoubleClickDeadline >
                g_196_nInputTimerTicks) {
                g_nInputDoubleClickDeadline = 0;
                g_pInputEventTail->status = 2;
                g_nPreviousPrimaryButton = 0;
                g_nPreviousSecondaryButton = 0;
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

    event = g_pInputEventHead;
    while (event != 0) {
        nextEvent = event->next;
        ReleaseInputEvent(event);
        event = nextEvent;
    }
    g_pInputEventTail = 0;
    g_pInputEventHead = g_pInputEventTail;
    return;
}

/* Function start: 0x462C9C */
void RetainInputEventsOfType(int type)
{
    InputEvent *event;
    InputEvent *nextEvent;

    event = g_pInputEventHead;
    while (event != 0) {
        nextEvent = event->next;
        if (event->type != type) {
            if (event->previous != 0)
                event->previous->next = event->next;
            else
                g_pInputEventHead = event->next;
            if (event->next != 0)
                event->next->previous = event->previous;
            else
                g_pInputEventTail = event->previous;
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
            g_pInputEventTail = event->previous;
        }
    } else {
        if (event->next != 0) {
            g_pInputEventHead = event->next;
            event->next->previous = 0;
        } else {
            g_pInputEventTail = 0;
            g_pInputEventHead = 0;
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
    if (g_nNextInputQueueFlushTick < g_196_nInputTimerTicks)
        FlushInputEvents();
    g_nNextInputQueueFlushTick = g_196_nInputTimerTicks + 20;
    type = INPUT_EVENT_NONE;
    if (g_pInputEventHead != 0) {
        posX = (int)g_pInputEventHead->x;
        posY = (int)g_pInputEventHead->y;
        if (g_16_stEventQueuePointerData.viewport != 0) {
            if ((int)g_16_stEventQueuePointerData.viewport->left > posX)
                g_pInputEventHead->x =
                    g_16_stEventQueuePointerData.viewport->left;
            else if ((int)g_16_stEventQueuePointerData.viewport->right < posX)
                g_pInputEventHead->x =
                    g_16_stEventQueuePointerData.viewport->right;
            if ((int)g_16_stEventQueuePointerData.viewport->top > posY)
                g_pInputEventHead->y =
                    g_16_stEventQueuePointerData.viewport->top;
            else if ((int)g_16_stEventQueuePointerData.viewport->bottom < posY)
                g_pInputEventHead->y =
                    g_16_stEventQueuePointerData.viewport->bottom;
        }

        state->status = (short)g_pInputEventHead->status;
        state->modifiers =
            (short)g_pInputEventHead->modifiers;
        switch (g_pInputEventHead->type) {
        case INPUT_EVENT_BUTTON_UP:
            g_16_stEventQueuePointerData.x = g_pInputEventHead->x;
            g_16_stEventQueuePointerData.y = g_pInputEventHead->y;
            g_16_stEventQueuePointerData.primaryButton = 0;
            state->x = g_pInputEventHead->x;
            state->y = g_pInputEventHead->y;
            *(unsigned int *)&state->value =
                (int)g_pInputEventHead->secondaryButton * 2 |
                (int)g_pInputEventHead->primaryButton;
            state->value = (short)*(unsigned int *)&state->value;
            state->status = (short)g_pInputEventHead->status;
            type = INPUT_EVENT_BUTTON_UP;
            break;
        case INPUT_EVENT_BUTTON_DOWN:
            g_16_stEventQueuePointerData.x = g_pInputEventHead->x;
            g_16_stEventQueuePointerData.y = g_pInputEventHead->y;
            g_16_stEventQueuePointerData.primaryButton =
                (unsigned char)g_pInputEventHead->primaryButton;
            g_16_stEventQueuePointerData.secondaryButton =
                (unsigned char)g_pInputEventHead->secondaryButton;
            state->x = g_pInputEventHead->x;
            state->y = g_pInputEventHead->y;
            *(unsigned int *)&state->value =
                (int)g_pInputEventHead->secondaryButton * 2 |
                (int)g_pInputEventHead->primaryButton;
            state->value = (short)*(unsigned int *)&state->value;
            state->status = (short)g_pInputEventHead->status;
            type = INPUT_EVENT_BUTTON_DOWN;
            if (state->status > 1)
                type = INPUT_EVENT_BUTTON_DOWN;
            break;
        case INPUT_EVENT_KEY_DOWN:
            *(unsigned int *)&state->value =
                (int)g_pInputEventHead->value;
            state->value = (short)g_pInputEventHead->field_18;
            state->status = (short)g_pInputEventHead->field_14;
            state->x = g_16_stEventQueuePointerData.x;
            state->y = g_16_stEventQueuePointerData.y;
            type = INPUT_EVENT_KEY_DOWN;
            break;
        case INPUT_EVENT_KEY_UP:
            state->x = g_pInputEventHead->value;
            *(unsigned int *)&state->value =
                (int)g_pInputEventHead->value;
            state->value = (short)g_pInputEventHead->field_18;
            state->status = (short)g_pInputEventHead->field_14;
            state->x = g_16_stEventQueuePointerData.x;
            state->y = g_16_stEventQueuePointerData.y;
            type = INPUT_EVENT_KEY_UP;
            break;
        case INPUT_EVENT_LEGACY_6:
            state->x = g_pInputEventHead->value;
            type = INPUT_EVENT_LEGACY_6;
            break;
        case INPUT_EVENT_JOYSTICK_MOVE:
        case INPUT_EVENT_LEGACY_8:
        case INPUT_EVENT_LEGACY_9:
            state->x = g_pInputEventHead->x;
            state->y = g_pInputEventHead->y;
            type = (int)g_pInputEventHead->type;
            break;
        case INPUT_EVENT_MOUSE_MOVE:
            g_16_stEventQueuePointerData.x = g_pInputEventHead->x;
            g_16_stEventQueuePointerData.y = g_pInputEventHead->y;
            state->x = g_pInputEventHead->x;
            state->y = g_pInputEventHead->y;
            type = INPUT_EVENT_MOUSE_MOVE;
            clearQueue = 1;
            break;
        default:
            state->x = g_pInputEventHead->x;
            state->y = g_pInputEventHead->y;
            type = INPUT_EVENT_NONE;
            break;
        }
        RemoveInputEvent(g_pInputEventHead);
        if (clearQueue != 0) {
            ReleaseInputEventQueue();
            FlushInputEvents();
        }
    } else {
        state->x = g_16_stEventQueuePointerData.x;
        state->y = g_16_stEventQueuePointerData.y;
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
    g_nNextInputQueueFlushTick = g_196_nInputTimerTicks + 20;
    for (event = g_pInputEventHead;
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
        if (eventType == INPUT_EVENT_BUTTON_DOWN ||
            eventType == INPUT_EVENT_BUTTON_UP)
            flags |= INPUT_MOD_BUTTON_EVENT;
        if (event->primaryButton != 0)
            flags |= INPUT_MOD_PRIMARY_BUTTON;
        else
            ;
        if (event->secondaryButton != 0)
            flags |= INPUT_MOD_SECONDARY_BUTTON;
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
    g_nNextInputQueueFlushTick = g_196_nInputTimerTicks + 20;
    for (event = g_pInputEventHead;
         event != 0 && event->type != type;
         event = event->next)
        ;
    if (event != 0) {
        if (event->type == INPUT_EVENT_MOUSE_MOVE) {
            g_16_stEventQueuePointerData.x = g_pInputEventHead->x;
            g_16_stEventQueuePointerData.y = g_pInputEventHead->y;
        }
        g_1_stFoundInputEvent.type = (int)event->type;
        *(unsigned int *)&g_1_stFoundInputEvent.value =
            event->modifiers;
        g_1_stFoundInputEvent.value = (short)event->field_18;
        g_1_stFoundInputEvent.status = (short)event->field_14;
        g_1_stFoundInputEvent.timestamp = event->timestamp;
        if (event->type == INPUT_EVENT_BUTTON_DOWN ||
            event->type == INPUT_EVENT_BUTTON_UP)
            modifiers |= INPUT_MOD_BUTTON_EVENT;
        if (event->primaryButton != 0)
            modifiers |= INPUT_MOD_PRIMARY_BUTTON;
        else
            ;
        if (event->secondaryButton != 0)
            modifiers |= INPUT_MOD_SECONDARY_BUTTON;
        else
            ;
        g_1_stFoundInputEvent.modifiers =
            (unsigned short)modifiers;
        g_1_stFoundInputEvent.x = event->x;
        g_1_stFoundInputEvent.y = event->y;
        g_1_stFoundInputEvent.status = (short)event->status;
        g_1_stFoundInputEvent.value =
            (short)*(unsigned int *)&g_1_stFoundInputEvent.value;
        return &g_1_stFoundInputEvent;
    } else
        return 0;
}

/* Function start: 0x46354F */
void FlushInputEvents(void)
{
    g_nNextInputQueueFlushTick = g_196_nInputTimerTicks + 20;
    ReleaseInputEventQueue();
    return;
}

/* Function start: 0x463571 */
short InitializeMouseCursorDepth(void)
{
    g_nMouseCursorDrawDepth = 1;
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
#ifdef SDL_PORT
    InputEvent *event;
    int x;
    int y;
#endif

    if (g_nMouseCursorDrawDepth > 0 &&
        g_16_stEventQueuePointerData.viewport != 0 &&
        g_16_stEventQueuePointerData.shape != 0) {
#ifdef SDL_PORT
        /* Motion events can be coalesced between game frames. Draw at the
         * newest queued position without consuming a click or moving the
         * pointer state used by game logic to its later coordinates. */
        x = g_16_stEventQueuePointerData.x;
        y = g_16_stEventQueuePointerData.y;
        for (event = g_pInputEventTail; event != 0; event = event->previous) {
            if (event->type == INPUT_EVENT_MOUSE_MOVE ||
                event->type == INPUT_EVENT_BUTTON_DOWN ||
                event->type == INPUT_EVENT_BUTTON_UP) {
                x = event->x;
                y = event->y;
                break;
            }
        }
        if (x < g_16_stEventQueuePointerData.viewport->left)
            x = g_16_stEventQueuePointerData.viewport->left;
        else if (x > g_16_stEventQueuePointerData.viewport->right)
            x = g_16_stEventQueuePointerData.viewport->right;
        if (y < g_16_stEventQueuePointerData.viewport->top)
            y = g_16_stEventQueuePointerData.viewport->top;
        else if (y > g_16_stEventQueuePointerData.viewport->bottom)
            y = g_16_stEventQueuePointerData.viewport->bottom;
        CaptureSpriteBackground(&g_101_stScrViewInfo,
                                g_abInputCursorBehind, x, y,
                                g_16_stEventQueuePointerData.shape,
                                g_16_stEventQueuePointerData.frame);
        g_4_nCapturedInputCursorX = x;
        g_10_nSavedPointerY = y;
#else
        CaptureSpriteBackground(&g_101_stScrViewInfo,
                                g_abInputCursorBehind,
                                g_16_stEventQueuePointerData.x,
                                g_16_stEventQueuePointerData.y,
                                g_16_stEventQueuePointerData.shape,
                                g_16_stEventQueuePointerData.frame);
        g_4_nCapturedInputCursorX = (int)g_16_stEventQueuePointerData.x;
        g_10_nSavedPointerY = (int)g_16_stEventQueuePointerData.y;
#endif
        if (g_4_nCapturedInputCursorX - 16 <
            g_148_nPointerDirtyLeft)
            g_148_nPointerDirtyLeft =
                g_4_nCapturedInputCursorX - 16;
        if (g_138_nCursorDirtyRightValue <
            g_4_nCapturedInputCursorX + 16)
            g_138_nCursorDirtyRightValue =
                g_4_nCapturedInputCursorX + 16;
        if (g_10_nSavedPointerY - 16 <
            g_146_nInputCursorDirtyTop)
            g_146_nInputCursorDirtyTop =
                g_10_nSavedPointerY - 16;
        if (g_15_nPointerDirtyBottom <
            g_10_nSavedPointerY + 16)
            g_15_nPointerDirtyBottom =
                g_10_nSavedPointerY + 16;
        g_26_bPointerDirtyValue = 1;
        g_6_bCurrentInputCursorBackgroundCapturedState = 1;
    }
    return;
}

/* Function start: 0x4636F7 */
void DrawMouseCursor(void)
{
    if (g_nMouseCursorDrawDepth > 0 &&
        g_16_stEventQueuePointerData.viewport != 0 &&
        g_16_stEventQueuePointerData.shape != 0) {
#ifdef SDL_PORT
        /* Both callers capture immediately before drawing. Use that same
         * position for drawing and restoration to avoid cursor trails;
         * the capture step has already recorded these dirty bounds. */
        DrawSpriteDefault(&g_101_stScrViewInfo,
                          (short)g_4_nCapturedInputCursorX,
                          (short)g_10_nSavedPointerY,
                          g_16_stEventQueuePointerData.shape,
                          g_16_stEventQueuePointerData.frame);
#else
        DrawSpriteDefault(&g_101_stScrViewInfo,
                          g_16_stEventQueuePointerData.x,
                          g_16_stEventQueuePointerData.y,
                          g_16_stEventQueuePointerData.shape,
                          g_16_stEventQueuePointerData.frame);
        if ((int)g_16_stEventQueuePointerData.x - 16 <
            g_148_nPointerDirtyLeft)
            g_148_nPointerDirtyLeft =
                (int)g_16_stEventQueuePointerData.x - 16;
        if (g_138_nCursorDirtyRightValue <
            (int)g_16_stEventQueuePointerData.x + 16)
            g_138_nCursorDirtyRightValue =
                (int)g_16_stEventQueuePointerData.x + 16;
        if ((int)g_16_stEventQueuePointerData.y - 16 <
            g_146_nInputCursorDirtyTop)
            g_146_nInputCursorDirtyTop =
                (int)g_16_stEventQueuePointerData.y - 16;
        if (g_15_nPointerDirtyBottom <
            (int)g_16_stEventQueuePointerData.y + 16)
            g_15_nPointerDirtyBottom =
                (int)g_16_stEventQueuePointerData.y + 16;
#endif
        g_26_bPointerDirtyValue = 1;
    }
    return;
}

/* Function start: 0x4637F4 */
void RestoreMouseCursorBackground(void)
{
    if (g_nMouseCursorDrawDepth > 0 &&
        g_16_stEventQueuePointerData.viewport != 0 &&
        g_16_stEventQueuePointerData.shape != 0 &&
        g_6_bCurrentInputCursorBackgroundCapturedState != 0) {
        RestoreSpriteBackground(&g_101_stScrViewInfo,
                                g_abInputCursorBehind,
                                (short)g_4_nCapturedInputCursorX,
                                (short)g_10_nSavedPointerY,
                                g_16_stEventQueuePointerData.shape,
                                g_16_stEventQueuePointerData.frame);
        if (g_4_nCapturedInputCursorX - 16 <
            g_148_nPointerDirtyLeft)
            g_148_nPointerDirtyLeft =
                g_4_nCapturedInputCursorX - 16;
        if (g_138_nCursorDirtyRightValue <
            g_4_nCapturedInputCursorX + 16)
            g_138_nCursorDirtyRightValue =
                g_4_nCapturedInputCursorX + 16;
        if (g_10_nSavedPointerY - 16 <
            g_146_nInputCursorDirtyTop)
            g_146_nInputCursorDirtyTop =
                g_10_nSavedPointerY - 16;
        if (g_15_nPointerDirtyBottom <
            g_10_nSavedPointerY + 16)
            g_15_nPointerDirtyBottom =
                g_10_nSavedPointerY + 16;
        g_6_bCurrentInputCursorBackgroundCapturedState = 0;
    }
    return;
}

/* Function start: 0x4638F1 */
void RefreshMouseCursorDisplay(void)
{
    g_148_nPointerDirtyLeft = 319;
    g_138_nCursorDirtyRightValue = 0;
    g_146_nInputCursorDirtyTop = 199;
    g_15_nPointerDirtyBottom = 0;
    g_26_bPointerDirtyValue = 0;
    g_6_bCurrentInputCursorBackgroundCapturedState = 0;
    CaptureMouseCursorBackground();
    DrawMouseCursor();
    DIBupdate(g_148_nPointerDirtyLeft,
              g_146_nInputCursorDirtyTop,
              g_138_nCursorDirtyRightValue,
              g_15_nPointerDirtyBottom);
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
    g_16_stEventQueuePointerData.shapeChanged = 1;
    changed = 0;
    if (g_16_stEventQueuePointerData.viewport != 0) {
        if (g_pDrawnInputCursorShape != 0) {
            RestoreSpriteBackground(g_16_stEventQueuePointerData.viewport,
                                    g_abInputCursorBehind,
                                    (short)g_4_nDrawnCursorX,
                                    (short)g_15_nMouseRenderY,
                                    g_pDrawnInputCursorShape,
                                    g_16_stEventQueuePointerData.frame);
            g_pDrawnInputCursorShape = 0;
            if (g_4_nDrawnCursorX - 16 < minX)
                minX = g_4_nDrawnCursorX - 16;
            if (g_4_nDrawnCursorX + 16 > maxX)
                maxX = g_4_nDrawnCursorX + 16;
            if (g_15_nMouseRenderY - 16 < minY)
                minY = g_15_nMouseRenderY - 16;
            if (g_15_nMouseRenderY + 16 > maxY)
                maxY = g_15_nMouseRenderY + 16;
            changed = 1;
        }
        if (g_nMouseCursorDrawDepth > 0 &&
            g_16_stEventQueuePointerData.shape != 0) {
            CaptureSpriteBackground(g_16_stEventQueuePointerData.viewport,
                                    g_abInputCursorBehind,
                                    g_16_stEventQueuePointerData.x,
                                    g_16_stEventQueuePointerData.y,
                                    g_16_stEventQueuePointerData.shape,
                                    g_16_stEventQueuePointerData.frame);
            DrawSpriteDefault(g_16_stEventQueuePointerData.viewport, g_16_stEventQueuePointerData.x,
                              g_16_stEventQueuePointerData.y,
                              g_16_stEventQueuePointerData.shape,
                              g_16_stEventQueuePointerData.frame);
            g_pDrawnInputCursorShape = g_16_stEventQueuePointerData.shape;
            g_4_nDrawnCursorX = g_16_stEventQueuePointerData.x;
            g_15_nMouseRenderY = g_16_stEventQueuePointerData.y;
            if (g_4_nDrawnCursorX - 16 < minX)
                minX = g_4_nDrawnCursorX - 16;
            if (g_4_nDrawnCursorX + 16 > maxX)
                maxX = g_4_nDrawnCursorX + 16;
            if (g_15_nMouseRenderY - 16 < minY)
                minY = g_15_nMouseRenderY - 16;
            if (g_15_nMouseRenderY + 16 > maxY)
                maxY = g_15_nMouseRenderY + 16;
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

    g_nMouseCursorDrawDepth = 1;
    return;
    g_nMouseCursorDrawDepth++;
    return;
    if (g_16_stEventQueuePointerData.viewport != 0 && g_16_stEventQueuePointerData.shape != 0)
        CaptureSpriteBackground(g_16_stEventQueuePointerData.viewport,
                                g_abInputCursorBehind,
                                g_16_stEventQueuePointerData.x,
                                g_16_stEventQueuePointerData.y,
                                g_16_stEventQueuePointerData.shape,
                                g_16_stEventQueuePointerData.frame);
    g_pDrawnInputCursorShape = g_16_stEventQueuePointerData.shape;
    return;
    g_16_stEventQueuePointerData.shapeChanged = 1;
    minX = 319;
    maxX = 0;
    minY = 199;
    maxY = 0;
    if (g_nMouseCursorDrawDepth > 0 &&
        g_16_stEventQueuePointerData.viewport != 0 &&
        g_16_stEventQueuePointerData.shape != 0) {
        if (g_pDrawnInputCursorShape != 0) {
            RestoreSpriteBackground(g_16_stEventQueuePointerData.viewport,
                                    g_abInputCursorBehind,
                                    (short)g_4_nDrawnCursorX,
                                    (short)g_15_nMouseRenderY,
                                    g_16_stEventQueuePointerData.shape,
                                    g_16_stEventQueuePointerData.frame);
            if (g_4_nDrawnCursorX - 16 < minX)
                minX = g_4_nDrawnCursorX - 16;
            if (g_4_nDrawnCursorX + 16 > maxX)
                maxX = g_4_nDrawnCursorX + 16;
            if (g_15_nMouseRenderY - 16 < minY)
                minY = g_15_nMouseRenderY - 16;
            if (g_15_nMouseRenderY + 16 > maxY)
                maxY = g_15_nMouseRenderY + 16;
        }
        CaptureSpriteBackground(g_16_stEventQueuePointerData.viewport,
                                g_abInputCursorBehind,
                                g_16_stEventQueuePointerData.x,
                                g_16_stEventQueuePointerData.y,
                                g_16_stEventQueuePointerData.shape,
                                g_16_stEventQueuePointerData.frame);
        DrawSpriteDefault(g_16_stEventQueuePointerData.viewport, g_16_stEventQueuePointerData.x,
                          g_16_stEventQueuePointerData.y,
                          g_16_stEventQueuePointerData.shape,
                          g_16_stEventQueuePointerData.frame);
        g_pDrawnInputCursorShape = (unsigned char *)1;
        g_4_nDrawnCursorX = g_16_stEventQueuePointerData.x;
        g_15_nMouseRenderY = g_16_stEventQueuePointerData.y;
        if (g_4_nDrawnCursorX - 16 < minX)
            minX = g_4_nDrawnCursorX - 16;
        if (g_4_nDrawnCursorX + 16 > maxX)
            maxX = g_4_nDrawnCursorX + 16;
        if (g_15_nMouseRenderY - 16 < minY)
            minY = g_15_nMouseRenderY - 16;
        if (g_15_nMouseRenderY + 16 > maxY)
            maxY = g_15_nMouseRenderY + 16;
        DIBupdate(minX, minY, maxX, maxY);
    }
    return;
}

/* Function start: 0x463E19 */
/* Hides the cursor.  Only the first statement is live in the Win32 port;
 * the earlier counting and software-cursor versions follow the returns. */
void DisableMouseCursorDrawing(void)
{
    g_nMouseCursorDrawDepth = -1;
    return;
    g_nMouseCursorDrawDepth--;
    return;
    g_pDrawnInputCursorShape = 0;
    return;
    if (g_nMouseCursorDrawDepth > 0 &&
        g_pDrawnInputCursorShape != 0) {
        if (g_16_stEventQueuePointerData.viewport != 0 &&
            g_16_stEventQueuePointerData.shape != 0) {
            RestoreSpriteBackground(g_16_stEventQueuePointerData.viewport,
                                    g_abInputCursorBehind,
                                    (short)g_4_nDrawnCursorX,
                                    (short)g_15_nMouseRenderY,
                                    g_16_stEventQueuePointerData.shape,
                                    g_16_stEventQueuePointerData.frame);
            DIBupdate(g_4_nDrawnCursorX - 16,
                      g_15_nMouseRenderY - 16,
                      g_4_nDrawnCursorX + 16,
                      g_15_nMouseRenderY + 16);
        }
        g_pDrawnInputCursorShape = 0;
    }
    g_nMouseCursorDrawDepth--;
    return;
}

/* Function start: 0x463EEE */
void SetMouseCursorShape(unsigned char *shape, short frame)
{
    g_16_stEventQueuePointerData.shapeChanged = 1;
    if (g_16_stEventQueuePointerData.viewport != 0 &&
        g_nMouseCursorDrawDepth > 0 &&
        g_pDrawnInputCursorShape != 0) {
        RestoreSpriteBackground(g_16_stEventQueuePointerData.viewport,
                                g_abInputCursorBehind,
                                (short)g_4_nDrawnCursorX,
                                (short)g_15_nMouseRenderY,
                                g_pDrawnInputCursorShape,
                                g_16_stEventQueuePointerData.frame);
        g_pDrawnInputCursorShape = 0;
    }
    g_16_stEventQueuePointerData.shape = shape;
    g_16_stEventQueuePointerData.frame = frame;
    return;
}

/* Function start: 0x463F74 */
void SetMouseHomePosition(short x, short y)
{
    g_16_stEventQueuePointerData.x = x;
    g_16_stEventQueuePointerData.y = y;
    SetInputCursorHostPosition(x, y);
    return;
}

/* Function start: 0x463FAA */
void ApplyPackedMousePosition(ShortPoint point)
{
    g_16_stEventQueuePointerData.x = point.x;
    g_16_stEventQueuePointerData.y = point.y;
    SetInputCursorHostPosition(point.x, point.y);
    return;
}

/* Function start: 0x463FE0 */
/* Nothing in the shipped executable calls this.  Reads back the mouse
 * position SetMouseHomePosition stored. */
void GetMouseHomePosition(ShortPoint *position)
{
    position->x = g_16_stEventQueuePointerData.x;
    position->y = g_16_stEventQueuePointerData.y;
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
    while (g_5_nFrameTimerWaitTicksValue != 0) ;
}

/* Function start: 0x464072 */
short IsFrameTickElapsed(void)
{
    return g_5_nFrameTimerWaitTicksValue == 0;
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
    g_19_bStoredHighMemoryEnabled = 1;
    g_9_nStoredHighMemoryResourceSetupWord = 0x1000;
    g_96_nHighMemoryInitValue = 0x1000;
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
    unsigned int control = g_77_abKeyboardEventStates[0x1d];
    unsigned int alt = g_77_abKeyboardEventStates[0x38];

    memset(g_77_abKeyboardEventStates, 0,
           sizeof(g_77_abKeyboardEventStates));
    g_77_abKeyboardEventStates[0x1d] = control;
    g_77_abKeyboardEventStates[0x38] = alt;
    ClearDebugPauseFlags();
}

/* Function start: 0x4642D6 */
void ClearInputKeyState(void)
{
    memset(g_77_abKeyboardEventStates, 0,
           sizeof(g_77_abKeyboardEventStates));
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
        g_77_abKeyboardEventStates[scanCode] = pressed;
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
    /* Two slots retail reserves and never touches. */
    int minimumDistance;
    int scan;
    int test;
    int nextObject;
    int ceiling;
    int furthest;
    int nextFarthest;
    int placed;
    int remaining;

    ceiling = -999999999;
    furthest = -1;
    nextObject = furthest;
    memset(g_11_anPlacedObjectDepths, 0,
           sizeof(g_11_anPlacedObjectDepths));
    for (scan = 0; scan < SPACE_OBJECT_COUNT; scan++) {
        if (ceiling <
            (int)(unsigned short)g_asObjectDistance[scan]) {
            ceiling =
                (int)(unsigned short)g_asObjectDistance[scan];
            furthest = scan;
        }
    }
    placed = 0;
    nextObject = furthest;
    for (scan = 0; scan < SPACE_OBJECT_COUNT; scan++) {
        g_56_anStoredObjectSortedIndicesData[placed] = nextObject;
        if (nextObject == -1)
            return;
        g_11_anPlacedObjectDepths[nextObject] = 1;
        nextFarthest = -1;
        nextObject = nextFarthest;
        for (test = 0; test < SPACE_OBJECT_COUNT;
             test++) {
            if (g_11_anPlacedObjectDepths[test] == 0 &&
                g_asObjectScreenX[test] != (short)0x8001 &&
                nextFarthest <
                    (int)(unsigned short)
                        g_asObjectDistance[test] &&
                (int)(unsigned short)g_asObjectDistance[test] <=
                    ceiling) {
                nextObject = test;
                nextFarthest =
                    (int)(unsigned short)
                        g_asObjectDistance[test];
            }
        }
        placed++;
    }
}

long DivideFixed(int numerator, int denominator);

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
        object = g_56_anStoredObjectSortedIndicesData[sortedIndex];
        if (object < 0)
            break;
        if (g_asObjectType[object] < 0)
            break;
#ifdef SDL_PORT
        objectClass = g_aeObjectClass[object];
        /* The enhanced backend records each sprite at a sub-pixel position
         * instead of stamping it into the indexed buffer.  Redo the
         * perspective divide in double precision, and only trust the result
         * when it rounds to the screen position the game already computed --
         * that is what proves it is the same projection and not an object the
         * game placed some other way. */
        enhancedScreenX = (float)(short)(
            g_asObjectScreenX[object] + g_15_nStoredScreenViewCentreX);
        enhancedScreenY = (float)(short)(
            g_asObjectScreenY[object] + g_37_nViewCenterY);
        if (objectClass != OBJECT_CLASS_NULL &&
            objectClass != OBJECT_CLASS_FIXED_OBJECT &&
            object != g_nNavPointerObject &&
            g_aObjectViewPosition[object].z != 0) {
            projectedScreenX = (short)(DivideFixed(
                MultiplyFixed(
                    (short)(g_nScreenWidth & ~1) << 7,
                    g_aObjectViewPosition[object].x),
                g_aObjectViewPosition[object].z) >> 8);
            projectedScreenY = (short)(DivideFixed(
                MultiplyFixed(
                    (short)(g_nScreenWidth & ~1) << 7,
                    g_aObjectViewPosition[object].y),
                g_aObjectViewPosition[object].z) >> 8);
            if (projectedScreenX == g_asObjectScreenX[object] &&
                projectedScreenY == g_asObjectScreenY[object]) {
                enhancedScreenX =
                    (float)g_15_nStoredScreenViewCentreX +
                    (float)(((double)(g_nScreenWidth & ~1) * 0.5 *
                             g_aObjectViewPosition[object].x) /
                            g_aObjectViewPosition[object].z);
                enhancedScreenY =
                    (float)g_37_nViewCenterY +
                    (float)(((double)(g_nScreenWidth & ~1) * 0.5 *
                             g_aObjectViewPosition[object].y) /
                            g_aObjectViewPosition[object].z);
            }
        } else if (objectClass == OBJECT_CLASS_FIXED_OBJECT &&
                   g_asObjectType[object] ==
                       OBJECT_DATA_THRUSTERS) {
            SdlGetThrusterScreenPosition(
                (short)object, &enhancedScreenX, &enhancedScreenY);
        }
#endif
        if (g_aeObjectClass[object] != OBJECT_CLASS_NULL) {
            switch (g_aeObjectClass[object]) {
            case OBJECT_CLASS_STAR:
#ifndef SDL_PORT
            case OBJECT_CLASS_PLANET:
#else
                /* WCDX fix: planets use their own scaled sprite instead of
                 * a frame from the shared constellation shape. */
#endif
            case OBJECT_CLASS_DUST:
                drawObject = object;
                g_asObjectDrawX[drawObject] = (short)(
                    g_asObjectScreenX[drawObject] +
                    g_15_nStoredScreenViewCentreX);
                g_asObjectDrawY[drawObject] = (short)(
                    g_asObjectScreenY[drawObject] +
                    g_37_nViewCenterY);
                if (g_nNavPointerObject == drawObject) {
#ifdef SDL_PORT
                    if (!SdlRecordSpaceSprite(
                            &g_27_stViewScratchInfo, enhancedScreenX,
                            enhancedScreenY,
                            g_apObjectShape[drawObject],
                            g_asObjectViewFrame[drawObject], 0, 0x100, 0))
#endif
                    DrawSpriteDefault(
                        &g_27_stViewScratchInfo,
                        g_asObjectDrawX[drawObject],
                        g_asObjectDrawY[drawObject],
                        g_apObjectShape[drawObject],
                        g_asObjectViewFrame[drawObject]);
                } else {
#ifdef SDL_PORT
                    if (!SdlRecordSpaceSprite(
                            &g_27_stViewScratchInfo, enhancedScreenX,
                            enhancedScreenY,
                            g_pConstellationSprite,
                            g_asObjectViewFrame[drawObject], 0, 0x100, 0))
#endif
                    DrawSpriteDefault(
                        &g_27_stViewScratchInfo,
                        g_asObjectDrawX[drawObject],
                        g_asObjectDrawY[drawObject],
                        g_pConstellationSprite,
                        g_asObjectViewFrame[drawObject]);
                }
                break;
            default:
                drawObject = object;
                g_asObjectDrawX[drawObject] = (short)(
                    g_asObjectScreenX[drawObject] +
                    g_15_nStoredScreenViewCentreX);
                g_asObjectDrawY[drawObject] = (short)(
                    g_asObjectScreenY[drawObject] +
                    g_37_nViewCenterY);
                if (g_apObjectShape[drawObject] != 0) {
#ifdef SDL_PORT
                    if (!SdlRecordSpaceSprite(
                            &g_27_stViewScratchInfo, enhancedScreenX,
                            enhancedScreenY,
                            g_apObjectShape[drawObject],
                            g_asObjectViewFrame[drawObject],
                            g_asObjectScreenAngle[drawObject],
                            g_asObjectScreenScale[drawObject],
                            g_asObjectFlip[drawObject]))
#endif
                    DrawSpriteScaled(
                        &g_27_stViewScratchInfo,
                        g_asObjectDrawX[drawObject],
                        g_asObjectDrawY[drawObject],
                        g_apObjectShape[drawObject],
                        g_asObjectViewFrame[drawObject],
                        g_asObjectScreenAngle[drawObject],
                        g_asObjectScreenScale[drawObject],
                        g_asObjectFlip[drawObject]);
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
        if (g_asObjectType[object] < 0)
            break;
        if (g_aeObjectClass[object] != OBJECT_CLASS_NULL) {
            switch (g_aeObjectClass[object]) {
            case OBJECT_CLASS_STAR:
#ifndef SDL_PORT
            case OBJECT_CLASS_PLANET:
#else
                /* Erase planets with the same scaled geometry used to draw
                 * them, leaving no stale pixels around the replacement. */
#endif
            case OBJECT_CLASS_DUST:
                drawObject = object;
                if (drawObject == g_nNavPointerObject)
                    DrawSolidColourSprite(
                        &g_27_stViewScratchInfo,
                        g_asObjectDrawX[drawObject],
                        g_asObjectDrawY[drawObject],
                        g_apObjectShape[drawObject],
                        g_asObjectViewFrame[drawObject],
                        g_cPrimaryViewBufferColour);
                else
                    DrawSolidColourSprite(
                        &g_27_stViewScratchInfo,
                        g_asObjectDrawX[drawObject],
                        g_asObjectDrawY[drawObject],
                        g_pConstellationSprite,
                        g_asObjectViewFrame[drawObject],
                        g_cPrimaryViewBufferColour);
                break;
            default:
                drawObject = object;
                if (g_apObjectShape[drawObject] != 0)
                    DrawSolidColourSpriteScaled(
                        &g_27_stViewScratchInfo,
                        g_asObjectDrawX[drawObject],
                        g_asObjectDrawY[drawObject],
                        g_apObjectShape[drawObject],
                        g_asObjectViewFrame[drawObject],
                        g_asObjectScreenAngle[drawObject],
                        g_asObjectScreenScale[drawObject],
                        g_asObjectFlip[drawObject],
                        g_cPrimaryViewBufferColour);
                break;
            }
        }
    }
    return;
}

/* Function start: 0x4648C5 */
void set_up_screen_viewport(signed char mode)
{
    g_1_cStoredViewDisplayModeValue = mode;
    switch (g_1_cStoredViewDisplayModeValue) {
    case 4:
    case 5:
        g_15_pScreenViewportGeometry =
            &g_aScreenViewportGeometry[
                (int)g_1_cStoredViewDisplayModeValue];
        break;
    default:
        g_15_pScreenViewportGeometry =
            (const ScreenViewportGeometry *)(
                g_pCockpitBackgroundPacket +
                ((ScreenViewportPacket *)
                     g_pCockpitBackgroundPacket)
                    ->geometryOffsets[0]);
        break;
    }
    if (g_nCockpitDisplayMode != 0 &&
        g_nCockpitDisplayMode != -2) {
        g_nScreenWidth =
            g_15_pScreenViewportGeometry->width;
        g_15_nStoredScreenViewCentreX =
            (short)(g_nScreenWidth / 2);
        g_nScreenHeight =
            g_15_pScreenViewportGeometry->height;
        g_37_nViewCenterY =
            (short)(g_nScreenHeight / 2);
        g_11_nViewportLeftOrigin =
            g_15_pScreenViewportGeometry->originX;
        g_49_nViewportOffsetY =
            g_15_pScreenViewportGeometry->originY;
        g_15_nStoredScreenViewCentreX += g_11_nViewportLeftOrigin;
        g_37_nViewCenterY += g_49_nViewportOffsetY;
        g_nScreenWidth = 320;
        g_nScreenHeight = 200;
    } else {
        g_nScreenWidth =
            g_15_pScreenViewportGeometry->width;
        g_15_nStoredScreenViewCentreX =
            (short)(g_nScreenWidth / 2);
        g_nScreenHeight =
            g_15_pScreenViewportGeometry->height;
        g_37_nViewCenterY =
            (short)(g_nScreenHeight / 2);
        g_11_nViewportLeftOrigin =
            g_15_pScreenViewportGeometry->originX;
        g_49_nViewportOffsetY =
            g_15_pScreenViewportGeometry->originY;
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
    g_nNextInputQueueFlushTick = g_196_nInputTimerTicks + 20;
    if (g_nInputPollPeriod == 1)
        PumpWindowMessages(1);
    else
        PumpWindowMessages(0);
    if (g_nNextInputPollTick <= g_196_nInputTimerTicks) {
        FlushInputEvents();
        PumpWindowMessages(0);
        g_nNextInputPollTick +=
            g_196_nInputTimerTicks + g_nInputPollPeriod;
        return 1;
    }
    PumpWindowMessages(1);
    if (g_bSceneEscapeRequested != 0)
        return 1;
    return 0;
}

/* Function start: 0x464E06 */
void SetInputViewport(Viewport *viewport)
{
    g_16_stEventQueuePointerData.viewport = viewport;
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
        g_176_pfnCurrentInputPump = pump;
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
        g_16_stEventQueuePointerData.shape = shape;
        g_18_pEventManager->cursorShape = shape;
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
    int spR;
    int spB;
    int spT;
    ShortRect spLimits;
    int spL;
    int rcL;
    int rcR;
    int rcB;
    int rcT;

    GetShapeFrameBounds(&spLimits.left, x, y, shape, frame);
    rcL = rectangle->left;
    rcT = rectangle->top;
    rcR = rectangle->right;
    rcB = rectangle->bottom;
    spL = spLimits.left;
    spT = spLimits.top;
    spR = spLimits.right;
    spB = spLimits.bottom;
    if (((spL <= rcL && rcL <= spR) ||
         (spL <= rcR && rcR <= spR))) {
        if (spT <= rcT && rcT <= spB)
            return 1;
        if (spT <= rcB && rcB <= spB)
            return 1;
    }
    if (((rcL <= spL && spL <= rcR) ||
         (rcL <= spR && spR <= rcR))) {
        if (rcT <= spT && spT <= rcB)
            return 1;
        if (rcT <= spB && spB <= rcB)
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
    g_37_stPersonnelMousePointerState.scratchBuffer =
        AllocateTaggedMemory(0x1000, 0);
}

/* Function start: 0x4656EB */
void ReleaseApplicationScratchBuffer(void)
{
    ReleasePacketHandle(g_37_stPersonnelMousePointerState.scratchBuffer);
    g_37_stPersonnelMousePointerState.scratchBuffer = 0;
}

/* Function start: 0x46570E */
unsigned char *GetInputCursorShape(void)
{
    return g_16_stEventQueuePointerData.shape;
}

/* This unit's initialized data, after its functions so that it does not
 * move the functions' symbol keys. */
signed char g_bSceneEscapeRequested = 0; /* 0x0049d4b0 */
InputEvent *g_pInputEventHead = 0; /* 0x0049d4b4 */
InputEvent *g_pInputEventTail = 0; /* 0x0049d4b8 */
int g_nInputEventPoolInitialized = 0; /* 0x0049d4bc */
int g_nInputDoubleClickDeadline = 0; /* 0x0049d4c0 */
int g_nPreviousPrimaryButton = 0; /* 0x0049d4c4 */
int g_nPreviousSecondaryButton = 0; /* 0x0049d4c8 */
int g_nNextInputQueueFlushTick = 0; /* 0x0049d4cc */
unsigned char *g_pDrawnInputCursorShape = 0; /* 0x0049d4d0 */
int g_nMouseCursorDrawDepth = 1; /* 0x0049d4d4 */
short g_nScreenWidth = 320; /* 0x0049d4d8 */
short g_nScreenHeight = 200; /* 0x0049d4dc */
int g_nUnusedEventWord1 = 0; /* 0x0049d4e0 */
int g_nUnusedEventWord2 = 0; /* 0x0049d4e4 */
ScreenViewportGeometry g_aScreenViewportGeometry[7] = {
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 320, 128, 0, 24, 0, 24, -24576, -1 },
    { 320, 200, 0, 0, 0, 0, -1536, -1 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }
}; /* 0x0049d4e8 */
signed char g_acDirectionShapeFrame[
    DIRECTION_VIEW_COUNT * DIRECTION_SHAPE_TABLE_COUNT] = {
    0, 1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2,
    14, 13, 12, 11, 10, 9, 8, 9, 10, 11, 12, 13,
    15, 16, 17, 18, 19, 20, 21, 20, 19, 18, 17, 16,
    28, 27, 26, 25, 24, 23, 22, 23, 24, 25, 26, 27,
    29, 30, 31, 32, 33, 34, 35, 34, 33, 32, 31, 30,
    36,
    0, 1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2,
    14, 13, 12, 11, 10, 9, 8, 9, 10, 11, 12, 13,
    15, 16, 17, 18, 19, 20, 21, 20, 19, 18, 17, 16,
    14, 13, 12, 11, 10, 9, 8, 9, 10, 11, 12, 13,
    1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2, 0,
    0, 1, 2, 3, 1, 3, 2, 1, 2, 3, 1, 3, 2,
    4, 5, 6, 4, 6, 5, 4, 5, 6, 4, 6, 5,
    7, 8, 9, 7, 9, 8, 7, 8, 9, 7, 9, 8,
    10, 11, 12, 10, 12, 11, 10, 11, 12, 10, 12, 11,
    13, 14, 15, 13, 15, 14, 13, 14, 15, 13, 15, 14, 16
}; /* 0x0049d558 */
signed char g_acDirectionShapeFlip[
    DIRECTION_VIEW_COUNT * DIRECTION_SHAPE_TABLE_COUNT] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0
}; /* 0x0049d618 */
int g_nNextInputPollTick = 0; /* 0x0049d6d4 */
short g_nInputPollPeriod = 0; /* 0x0049d6d8 */

/* The public input state occupies the retail commons block at
 * 0x005C80A8-0x005C8D3D, immediately after main.c's event pool.
 * Unused byte arrays represent gaps that alignment cannot account for.
 * Retail /DEBUG linking retains unused commons; their original names,
 * types and individual declaration boundaries are unknown. */
int g_15_nMouseRenderY; /* 0x005c80a8 */
int g_4_nDrawnCursorX; /* 0x005c80ac */
unsigned char g_9_abUnusedInputData[16]; /* 0x005c80b0 */
unsigned char *g_39_pCurrentCampaignDataOffset; /* 0x005c80c0 */
int g_6_bCurrentInputCursorBackgroundCapturedState; /* 0x005c80c4 */
unsigned char g_4_abUnreferencedEventStorage[2]; /* 0x005c80c8 */
short g_18_bEventManagerInitializationState; /* 0x005c80ca */
unsigned char g_18_abUnreferencedInputBuffer[4]; /* 0x005c80cc */
unsigned char g_5_abUnusedInputSpace1[2]; /* 0x005c80d0 */
short g_14_nCurrentGraphicsModeDetectedValue; /* 0x005c80d2 */
short g_6_wQueuedButtonsState; /* 0x005c80d4 */
unsigned char g_88_nEventRepeatDelayState; /* 0x005c80d6 */
short g_15_nStoredScreenViewCentreX; /* 0x005c80d8 */
short g_37_nViewCenterY; /* 0x005c80da */
unsigned char g_3_abUnusedLibraryBytes[4]; /* 0x005c80dc */
void *g_26_pLibraryCurrentLastPacketAllocation; /* 0x005c80e0 */
short g_19_bStoredHighMemoryEnabled; /* 0x005c80e4 */
unsigned char g_26_bMouseCursorEnabled; /* 0x005c80e6 */
unsigned char g_77_abKeyboardEventStates[0x80]; /* 0x005c80f0 */
unsigned char g_17_abUnusedLibraryInputBuffer[8]; /* 0x005c8170 */
int g_10_nSavedPointerY; /* 0x005c8178 */
int g_4_nCapturedInputCursorX; /* 0x005c817c */
int g_11_anPlacedObjectDepths[SPACE_OBJECT_COUNT]; /* 0x005c8180 */
unsigned char g_58_abUnreferencedInputStorage1[8]; /* 0x005c8298 */
unsigned char g_13_abUnusedLibrarySpaceData[4]; /* 0x005c82a0 */
short g_96_nHighMemoryInitValue; /* 0x005c82a4 */
signed char g_1_cStoredViewDisplayModeValue; /* 0x005c82a6 */
signed char g_9_bCinematicSpriteTextEnableStatus; /* 0x005c82a7 */
unsigned char g_97_abUnusedInputSystemBuffer[8]; /* 0x005c82a8 */
const ScreenViewportGeometry * volatile
    g_15_pScreenViewportGeometry; /* 0x005c82b0 */
int g_56_anStoredObjectSortedIndicesData[SPACE_OBJECT_COUNT]; /* 0x005c82c0 */
unsigned char g_5_abUnreferencedInputStorage2[4]; /* 0x005c83d8 */
unsigned char *g_13_pActiveCutscenePixelsAddress; /* 0x005c83dc */
unsigned char g_56_abUnusedEventBuffer[6]; /* 0x005c83e0 */
signed char g_cKeypadMouseSpeedValue; /* 0x005c83e6 */
QueuedMouseState g_16_stEventQueuePointerData; /* 0x005c83f0 */
void (*g_176_pfnCurrentInputPump)(void); /* 0x005c840c */
unsigned char g_39_abUnusedInputSpace2[12]; /* 0x005c8410 */
short g_105_nEmsIsrBusyValue; /* 0x005c841c */
unsigned char g_68_abUnusedInputSpace3[16]; /* 0x005c8420 */
signed char g_6_bCampaignFlightRunning; /* 0x005c8430 */
short g_12_wInputDeviceButtonStatus; /* 0x005c8432 */
unsigned char g_38_abUnusedLibraryInputBufferData[4]; /* 0x005c8434 */
unsigned char g_8_abUnusedInputSystemBytes[4]; /* 0x005c8438 */
int g_2_bIgnoreNextCursorMoveFlag; /* 0x005c843c */
unsigned char g_10_abUnusedEventSpace[8]; /* 0x005c8440 */
short g_86_nCurrentInputTickInterval; /* 0x005c8448 */
volatile int g_5_nFrameTimerWaitTicksValue; /* 0x005c844c */
int g_26_bPointerDirtyValue; /* 0x005c8450 */
unsigned char g_1_abUnreferencedInputData[4]; /* 0x005c8454 */
short g_9_nStoredHighMemoryResourceSetupWord; /* 0x005c8458 */
int g_138_nCursorDirtyRightValue; /* 0x005c845c */
int g_15_nPointerDirtyBottom; /* 0x005c8460 */
InputManagerState *g_18_pEventManager; /* 0x005c8464 */
MouseCursorState g_37_stPersonnelMousePointerState; /* 0x005c8470 */
unsigned char g_26_abUnusedInputStorage1[4]; /* 0x005c8494 */
unsigned char g_26_abUnusedInputStorage2[4]; /* 0x005c8498 */
short g_49_nViewportOffsetY; /* 0x005c849c */
short g_11_nViewportLeftOrigin; /* 0x005c849e */
unsigned char g_4_abUnusedEventStorage[8]; /* 0x005c84a0 */
volatile int g_196_nInputTimerTicks; /* 0x005c84a8 */
unsigned char g_37_abUnusedEventManagerBuffer[272]; /* 0x005c84b0 */
int g_148_nPointerDirtyLeft; /* 0x005c85c0 */
int g_146_nInputCursorDirtyTop; /* 0x005c85c4 */
unsigned char g_128_abPauseInputState[0x100]; /* 0x005c85d0 */
unsigned char g_12_abUnusedLibraryInputData[16]; /* 0x005c86d0 */
unsigned char g_13_abPreviousPauseKeyStatuses[0x100]; /* 0x005c86e0 */
int g_3_anCurrentInputEventEntryAllocation[0x100]; /* 0x005c87e0 */
void (*g_59_pfnEventShutdownCallback)(void); /* 0x005c8be0 */
unsigned char g_9_abUnusedLibrarySpace[260]; /* 0x005c8bf0 */
void (*g_3_pfnRenderSceneTextStringHandler)(char *text); /* 0x005c8cf4 */
PersonnelMouseSample g_6_stPersonnelMouseData; /* 0x005c8d00 */
unsigned char g_136_abUnusedInputMemoryData[12]; /* 0x005c8d10 */
TextContext *g_29_pCurrentTextContext; /* 0x005c8d1c */
unsigned char g_388_abUnusedLibraryBuffer1[4]; /* 0x005c8d20 */
int g_3_nEventClockScaleValue; /* 0x005c8d24 */
unsigned char g_86_abUnreferencedEventData[8]; /* 0x005c8d28 */
void (*g_36_pfnRenderSceneFontCharacterHandler)(char character); /* 0x005c8d30 */
unsigned char g_59_abUnusedLibraryBuffer2[4]; /* 0x005c8d34 */
unsigned char g_138_abUnusedLibraryData[4]; /* 0x005c8d38 */
short g_18_nInterfaceEventMode; /* 0x005c8d3c */
