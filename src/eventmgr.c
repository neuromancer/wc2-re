/*
 *  Event manager, mouse pointer, and frame timer.
 *
 *  Address range 0x4355f0-0x436fff (provisional -- see docs/ORDER.md).
 *  The FM Towns executable preserves the original source name
 *  "source\\eventmgr.c" for this unit.
 */
#include "wc1.h"

/* Function start: 0x462625 */
void TranslatePolledInputEvent(unsigned short type, unsigned int value)
{
    HostMouseMessage *mouse;

    switch (type) {
    case 7:
        QueueInputEvent(type,
            (unsigned short)g_aInputDeviceSamples_005d1780[
                g_nActiveInputDevice_005d1726].x,
            (unsigned short)g_aInputDeviceSamples_005d1780[
                g_nActiveInputDevice_005d1726].y,
            0, value & 1, (value & 2) >> 1, 0, 0, 0);
        break;
    case 10:
        QueueInputEvent(1,
            (unsigned short)g_aInputDeviceSamples_005d1780[
                g_nActiveInputDevice_005d1726].x,
            (unsigned short)g_aInputDeviceSamples_005d1780[
                g_nActiveInputDevice_005d1726].y,
            0, value & 1, (value & 2) >> 1, 0, 0, 0);
        break;
    case 0x45:
        QueueInputEvent(2,
            (unsigned short)g_aInputDeviceSamples_005d1780[
                g_nActiveInputDevice_005d1726].x,
            (unsigned short)g_aInputDeviceSamples_005d1780[
                g_nActiveInputDevice_005d1726].y,
            0, value & 1, (value & 2) >> 1, 0, 0, 0);
        break;
    case 3:
        mouse = &g_stHostMouseMessage_005d10d0;
        QueueInputEvent(type,
                        (short)g_nQueuedInputX_005c83f0,
                        (short)g_nQueuedInputY_005c83f2,
                        0, 0, 0, 0, 0, 0);
        break;
    case 1:
        mouse = &g_stHostMouseMessage_005d10d0;
        QueueInputEvent(type,
                        (short)g_nQueuedInputX_005c83f0,
                        (short)g_nQueuedInputY_005c83f2,
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
    /* Preserve the original 16-bit event ID and sample each volatile axis. */
    unsigned int eventType = type & 0xffff;
    int x = g_stMouseCursorState_0059ab10.x;
    int y = g_stMouseCursorState_0059ab10.y;

    QueueInputEvent((unsigned short)eventType, (unsigned short)x,
                    (unsigned short)y, 0,
                    primaryButton, secondaryButton, 0, 0, 0);
}

#pragma function(memset)
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
}

/* Function start: 0x4629A7 */
void QueueInputEvent(unsigned short type, unsigned short x,
                     unsigned short y, unsigned short value,
                     int primaryButton, int secondaryButton,
                     unsigned int ignored, unsigned int field14,
                     unsigned int field18)
{
    unsigned int modifiers;
    InputEvent *event;

    (void)ignored;

    modifiers = 0;
    if (GetShiftKeyState() != 0)
        modifiers = 0xe0;
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
        event = AllocateInputEvent();
        g_pInputEventTail_0049d4b8->next = event;
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
}

/* Function start: 0x462C43 */
void ReleaseInputEventQueue(void)
{
    InputEvent *event = g_pInputEventHead_0049d4b4;

    while (event != 0) {
        InputEvent *next = event->next;

        ReleaseInputEvent(event);
        event = next;
    }
    g_pInputEventTail_0049d4b8 = 0;
    g_pInputEventHead_0049d4b4 = g_pInputEventTail_0049d4b8;
}

/* Function start: 0x462C9C */
void RetainInputEventsOfType(int type)
{
    InputEvent *event = g_pInputEventHead_0049d4b4;

    while (event != 0) {
        InputEvent *next = event->next;

        if (event->type != type) {
            if (event->previous != 0)
                event->previous->next = next;
            else
                g_pInputEventHead_0049d4b4 = next;
            if (event->next != 0)
                event->next->previous = event->previous;
            else
                g_pInputEventTail_0049d4b8 = event->previous;
            ReleaseInputEvent(event);
        }
        event = next;
    }
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
}

/* Function start: 0x462DFC */
short GetNextInputEvent(InputEventState *state)
{
    int clearQueue;
    int type;
    int eventX;
    int eventY;
    int eventType;

    clearQueue = 0;
    if (g_nNextInputQueueFlushTick_0049d4cc < g_nInputClock_005c84a8)
        FlushInputEvents();
    g_nNextInputQueueFlushTick_0049d4cc = g_nInputClock_005c84a8 + 20;
    type = 0;
    if (g_pInputEventHead_0049d4b4 != 0) {
        eventX = (int)g_pInputEventHead_0049d4b4->x;
        eventY = (int)g_pInputEventHead_0049d4b4->y;
        if (g_pInputViewport_005c8403 != 0) {
            if ((int)g_pInputViewport_005c8403->left > eventX)
                g_pInputEventHead_0049d4b4->x =
                    g_pInputViewport_005c8403->left;
            else if ((int)g_pInputViewport_005c8403->right < eventX)
                g_pInputEventHead_0049d4b4->x =
                    g_pInputViewport_005c8403->right;
            if ((int)g_pInputViewport_005c8403->top > eventY)
                g_pInputEventHead_0049d4b4->y =
                    g_pInputViewport_005c8403->top;
            else if ((int)g_pInputViewport_005c8403->bottom < eventY)
                g_pInputEventHead_0049d4b4->y =
                    g_pInputViewport_005c8403->bottom;
        }

        state->status = (short)g_pInputEventHead_0049d4b4->status;
        state->modifiers =
            (short)g_pInputEventHead_0049d4b4->modifiers;
        eventType = (int)g_pInputEventHead_0049d4b4->type;
        switch (eventType) {
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
            break;
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
        case 3:
            g_nQueuedInputX_005c83f0 = g_pInputEventHead_0049d4b4->x;
            g_nQueuedInputY_005c83f2 = g_pInputEventHead_0049d4b4->y;
            state->x = g_pInputEventHead_0049d4b4->x;
            state->y = g_pInputEventHead_0049d4b4->y;
            type = 3;
            clearQueue = 1;
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
        } else {
            state->x = g_nQueuedInputX_005c83f0;
            state->y = g_nQueuedInputY_005c83f2;
        }
    } else {
        state->x = g_nQueuedInputX_005c83f0;
        state->y = g_nQueuedInputY_005c83f2;
    }
    return type;
}

/* Function start: 0x46327F */
short PollInputEvent(InputEventState *event)
{
    PumpWindowMessages(0);
    return GetNextInputEvent(event);
}

/* Function start: 0x4632A5 */
short PeekInputEvent(InputEventState *state, short type)
{
    unsigned int modifiers;
    int eventType;
    InputEvent *event;

    modifiers = 0;
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
            modifiers |= 8;
        if (event->primaryButton != 0)
            modifiers |= 1;
        if (event->secondaryButton != 0)
            modifiers |= 2;
        state->modifiers = (unsigned short)modifiers;
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
    int eventType;
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
        eventType = (int)event->type;
        if (eventType == 1 || eventType == 2)
            modifiers |= 8;
        if (event->primaryButton != 0)
            modifiers |= 1;
        if (event->secondaryButton != 0)
            modifiers |= 2;
        g_stFoundInputEvent_005c3af8.modifiers =
            (unsigned short)modifiers;
        g_stFoundInputEvent_005c3af8.x = event->x;
        g_stFoundInputEvent_005c3af8.y = event->y;
        g_stFoundInputEvent_005c3af8.status = (short)event->status;
        g_stFoundInputEvent_005c3af8.value =
            (short)*(unsigned int *)&g_stFoundInputEvent_005c3af8.value;
        return &g_stFoundInputEvent_005c3af8;
    }
    return 0;
}

/* Function start: 0x46354F */
void FlushInputEvents(void)
{
    g_nNextInputQueueFlushTick_0049d4cc = g_nInputClock_005c84a8 + 20;
    ReleaseInputEventQueue();
}

/* Function start: 0x463571 */
short InitializeMouseCursorDepth(void)
{
    g_nMouseCursorDrawDepth_0049d4d4 = 1;
    return 1;
}

/* Function start: 0x4635D3 */
void CheckCursor(void)
{
}

/* Function start: 0x4635E3 */
void CaptureMouseCursorBackground(void)
{
    if (g_nMouseCursorDrawDepth_0049d4d4 <= 0 ||
        g_pInputViewport_005c8403 == 0 ||
        g_pInputCursorShape_005c83f9 == 0)
        return;

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

/* Function start: 0x4636F7 */
void DrawMouseCursor(void)
{
    if (g_nMouseCursorDrawDepth_0049d4d4 <= 0 ||
        g_pInputViewport_005c8403 == 0 ||
        g_pInputCursorShape_005c83f9 == 0)
        return;

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

/* Function start: 0x4637F4 */
void RestoreMouseCursorBackground(void)
{
    if (g_nMouseCursorDrawDepth_0049d4d4 <= 0 ||
        g_pInputViewport_005c8403 == 0 ||
        g_pInputCursorShape_005c83f9 == 0 ||
        g_bInputCursorBackgroundCaptured_005c80c4 == 0)
        return;

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
}

/* Function start: 0x46396C */
void ResumeMouseCursorHook(void)
{
}

/* Function start: 0x463BA1 */
void EnableMouseCursorDrawing(void)
{
    g_nMouseCursorDrawDepth_0049d4d4 = 1;
}

/* Function start: 0x463E19 */
void DisableMouseCursorDrawing(void)
{
    g_nMouseCursorDrawDepth_0049d4d4 = -1;
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
}

/* Function start: 0x463F74 */
void SetMouseHomePosition(short x, short y)
{
    g_nQueuedInputX_005c83f0 = x;
    g_nQueuedInputY_005c83f2 = y;
    SetInputCursorHostPosition(x, y);
}

/* Function start: 0x463FAA */
void ApplyPackedMousePosition(ShortPoint point)
{
    g_nQueuedInputX_005c83f0 = point.x;
    g_nQueuedInputY_005c83f2 = point.y;
    SetInputCursorHostPosition(point.x, point.y);
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
int IsFrameTickElapsed(void)
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
#pragma intrinsic(memset)

/* Function start: 0x464123 */
void *AllocateDefaultMemory(unsigned int size)
{
    return AllocateTaggedMemory(size, 0);
}

/* Function start: 0x464141 */
unsigned short GetSoundHardwareFlag(void)
{
    return 1;
}

/* Function start: 0x46416A */
void TimerResetHook(void)
{
}

/* Function start: 0x46417A */
unsigned int IdentityDword(unsigned int value)
{
    return value;
}

/* Function start: 0x46418D */
unsigned short IdentityWord(unsigned short v)
{
    return v;
}

/* Function start: 0x4641A0 */
void TimerStopHook(void)
{
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

#pragma function(memset)
/* Function start: 0x4642D6 */
void ClearInputKeyState(void)
{
    memset(g_abInputKeyState_005c80f0, 0,
           sizeof(g_abInputKeyState_005c80f0));
    ClearDebugPauseFlags();
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
    for (object = 0; object < WC2_SPACE_OBJECT_COUNT; object++) {
        if (maximumDistance <
            (int)(unsigned short)g_asObjectDistance_00493ae8[object]) {
            maximumDistance =
                (int)(unsigned short)g_asObjectDistance_00493ae8[object];
            farthestObject = object;
        }
    }
    sortedIndex = 0;
    nextObject = farthestObject;
    for (object = 0; object < WC2_SPACE_OBJECT_COUNT; object++) {
        g_anSortedObject_005c82c0[sortedIndex] = nextObject;
        if (nextObject == -1)
            return;
        g_anObjectDepthPlaced_005c8180[nextObject] = 1;
        nextDistance = -1;
        nextObject = nextDistance;
        for (candidate = 0; candidate < WC2_SPACE_OBJECT_COUNT;
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
    int obj;
    int objectClass;
    int sortedIndex;

    for (sortedIndex = 0; sortedIndex < WC2_SPACE_OBJECT_COUNT;
         sortedIndex++) {
        obj = g_anSortedObject_005c82c0[sortedIndex];
        if (obj < 0)
            return;
        if (g_asObjectType_00495298[obj] < 0)
            return;
        objectClass = g_aeObjectClass_00495328[obj];
        if (objectClass != OBJECT_CLASS_NULL) {
            switch (objectClass) {
            case OBJECT_CLASS_STAR:
            case OBJECT_CLASS_PLANET:
            case OBJECT_CLASS_DUST:
                g_asObjectDrawX_00493748[obj] = (short)(
                    g_asObjectScreenX_00493598[obj] +
                    g_nViewCenterX_005c80d8);
                g_asObjectDrawY_004937d8[obj] = (short)(
                    g_asObjectScreenY_00493628[obj] +
                    g_nViewCenterY_005c80da);
                if (g_nNavPointerObject_004931b8 == obj)
                    DrawSpriteDefault(
                        &g_stViewBuffer_005d2b00,
                        g_asObjectDrawX_00493748[obj],
                        g_asObjectDrawY_004937d8[obj],
                        g_apObjectShape_00493868[obj],
                        g_asObjectViewFrame_00493508[obj]);
                else
                    DrawSpriteDefault(
                        &g_stViewBuffer_005d2b00,
                        g_asObjectDrawX_00493748[obj],
                        g_asObjectDrawY_004937d8[obj],
                        g_pConstellationShape_005d2c4c,
                        g_asObjectViewFrame_00493508[obj]);
                break;
            default:
                g_asObjectDrawX_00493748[obj] = (short)(
                    g_asObjectScreenX_00493598[obj] +
                    g_nViewCenterX_005c80d8);
                g_asObjectDrawY_004937d8[obj] = (short)(
                    g_asObjectScreenY_00493628[obj] +
                    g_nViewCenterY_005c80da);
                if (g_apObjectShape_00493868[obj] != 0)
                    DrawSpriteScaled(
                        &g_stViewBuffer_005d2b00,
                        g_asObjectDrawX_00493748[obj],
                        g_asObjectDrawY_004937d8[obj],
                        g_apObjectShape_00493868[obj],
                        g_asObjectViewFrame_00493508[obj],
                        g_asObjectScreenAngle_004936b8[obj],
                        g_asObjectScreenScale_00493a58[obj],
                        g_asObjectFlip_004939c8[obj]);
                break;
            }
        }
    }
}

/* Function start: 0x46470E */
void intro_drawbackgroundships(void)
{
    int objectClass;
    int obj;

    for (obj = 0; obj < WC2_SPACE_OBJECT_COUNT; obj++) {
        if (g_asObjectType_00495298[obj] < 0)
            return;
        objectClass = g_aeObjectClass_00495328[obj];
        if (objectClass != OBJECT_CLASS_NULL) {
            switch (objectClass) {
            case OBJECT_CLASS_STAR:
            case OBJECT_CLASS_PLANET:
            case OBJECT_CLASS_DUST:
                if (obj == g_nNavPointerObject_004931b8)
                    DrawSolidColourSprite(
                        &g_stViewBuffer_005d2b00,
                        g_asObjectDrawX_00493748[obj],
                        g_asObjectDrawY_004937d8[obj],
                        g_apObjectShape_00493868[obj],
                        g_asObjectViewFrame_00493508[obj],
                        g_cPrimaryViewBufferColour_0049cb88);
                else
                    DrawSolidColourSprite(
                        &g_stViewBuffer_005d2b00,
                        g_asObjectDrawX_00493748[obj],
                        g_asObjectDrawY_004937d8[obj],
                        g_pConstellationShape_005d2c4c,
                        g_asObjectViewFrame_00493508[obj],
                        g_cPrimaryViewBufferColour_0049cb88);
                break;
            default:
                if (g_apObjectShape_00493868[obj] != 0)
                    DrawSolidColourSpriteScaled(
                        &g_stViewBuffer_005d2b00,
                        g_asObjectDrawX_00493748[obj],
                        g_asObjectDrawY_004937d8[obj],
                        g_apObjectShape_00493868[obj],
                        g_asObjectViewFrame_00493508[obj],
                        g_asObjectScreenAngle_004936b8[obj],
                        g_asObjectScreenScale_00493a58[obj],
                        g_asObjectFlip_004939c8[obj],
                        g_cPrimaryViewBufferColour_0049cb88);
                break;
            }
        }
    }
}

/* Function start: 0x4648C5 */
void set_up_screen_viewport(signed char mode)
{
    int modeIndex;

    g_cScreenViewportMode_005c82a6 = mode;
    modeIndex = (int)g_cScreenViewportMode_005c82a6;
    switch (modeIndex) {
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
}

/* Function start: 0x464A90 */
void FinalizeInputManagerHook(void)
{
}

/* Function start: 0x464AA0 */
void MouseIdleHook(void)
{
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

/* Function start: 0x464AE8 */
void StartupHook(unsigned int (*callback)(unsigned int, short))
{
    (void)callback;
}

/* Function start: 0x464B0B */
unsigned int JoystickEdgeHook(int button)
{
    (void)button;
}

/* Function start: 0x464B3A */
void FreeIfNotNull(void *p)
{
    if (p != 0)
        free(p);
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
    if (slot == 1) {
        g_pfnInputPump_005c840c = pump;
        return 0;
    }
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
}

/* Function start: 0x464EF0 */
void FinalizeInputDriverHook(void)
{
}

/* Function start: 0x464F10 */
void FlushPendingInputEvents(void)
{
    FlushInputEvents();
    return;
}

/* Function start: 0x464F25 */
unsigned int GetNamedPacketSize(const char *filename, short section)
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
}

/* Function start: 0x464FE7 */
void SetPaletteEntryFromTriplet(const unsigned char *triplet, short index)
{
    short rgb[3];

    rgb[0] = triplet[0];
    rgb[1] = triplet[1];
    rgb[2] = triplet[2];
    DIBsetPalette(index, rgb);
}

/* Function start: 0x46502E */
void SetMusicTickRateHook(short period)
{
    (void)period;
}

/* Function start: 0x464B60 */
unsigned int GetStartupErrorCode(int vector)
{
    (void)vector;
    return 0;
}

/* Function start: 0x464B82 */
void PersonnelDriveHook()
{
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
}

/* Function start: 0x464BDB */
void ExitCleanupHook(void)
{
}

#pragma function(abs)

/* Function start: 0x464BFE */
short IsVectorWithinRange(FixedVector *vector, short range)
{
    int magnitude;
    int fixedRange;

    fixedRange = (int)range << 8;
    magnitude = Vector_magnitude(vector);
    if (abs(fixedRange) >= magnitude)
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
    unsigned short fraction;
    unsigned short whole;

    *component = *component / 2;
    whole = (unsigned short)((unsigned int)*component >> 16);
    fraction = (unsigned short)(*component & 0xffff);
    if (whole == 0) {
        if (fraction <= 0x0f00)
            return 0;
        return 1;
    }
    if ((short)whole == -1) {
        if (fraction >= 0xf100)
            return 0;
        return 1;
    }
    if (whole != 0)
        return 1;
}

/* Function start: 0x42BAC0 */
void FillGraphicSuffix(char *path, short number, short digits)
{
    char *dot;
    char *suffix;

    suffix = DosStrchr(path, '.');
    dot = suffix;
    suffix++;
    if (dot == 0)
        suffix = path;
    suffix[digits] = '\0';
    ConvertChar_Int(suffix, number, digits);
}

/* Function start: 0x42BB17 */
void ConvertChar_Int(char *text, short number, short digits)
{
    while (digits--) {
        text[digits] = (char)(number % 10) + '0';
        number /= 10;
    }
}
