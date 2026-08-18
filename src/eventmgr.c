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
    unsigned int state;

    switch (type) {
    case 2:
        state = (value >> 2) |
                (g_bHostSecondaryMouseButton_005d10dc << 1) |
                g_bHostPrimaryMouseButton_005d10d8;
        QueueInputEvent(type,
                        (short)g_nHostMouseMessageX_005d10d0,
                        (short)g_nHostMouseMessageY_005d10d4,
                        0,
                        g_bHostPrimaryMouseButton_005d10d8,
                        g_bHostSecondaryMouseButton_005d10dc,
                        state, 0, 0);
        return;
    case 6:
    {
        InputDeviceSample *sample =
            &g_aInputDeviceSamples_005a81f0[g_nActiveInputDevice_005a819c];

        QueueInputEvent(type, (short)sample->x, (short)sample->y,
                        0, 0, 0, 0, 0, 0);
        return;
    }
    case 13:
        QueueInputEvent(type,
                        (short)g_nHostMouseMessageX_005d10d0,
                        (short)g_nHostMouseMessageY_005d10d4,
                        0, 0, 0, 0, 0, 0);
        return;
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
#if 0
    int *used;
    int index;

    if (g_nInputEventPoolInitialized_0049d4bc != 0) {
        index = 0;
        used = g_anInputEventSlotUsed_005c87e0;
        do {
            if (*used == 0) {
                g_anInputEventSlotUsed_005c87e0[index] = 1;
                return &g_aInputEventPool_005c5890[index];
            }
            used++;
            index++;
        } while (used < &g_anInputEventSlotUsed_005c87e0[0x100]);
        return 0;
    }
    memset(g_aInputEventPool_005c5890, 0,
           sizeof(g_aInputEventPool_005c5890));
    memset(g_anInputEventSlotUsed_005c87e0, 0,
           sizeof(g_anInputEventSlotUsed_005c87e0));
    g_nInputEventPoolInitialized_0049d4bc = 1;
    g_anInputEventSlotUsed_005c87e0[0] = 1;
    return &g_aInputEventPool_005c5890[0];
#else
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
#endif
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
#if 0
    InputEvent *previous = event->previous;
    InputEvent *next = event->next;

    if (previous != 0) {
        if (next != 0) {
            previous->next = next;
            event->next->previous = event->previous;
            ReleaseInputEvent(event);
            return;
        }
        previous->next = 0;
        g_pInputEventTail_0049d4b8 = event->previous;
        ReleaseInputEvent(event);
        return;
    }
    if (next != 0) {
        g_pInputEventHead_0049d4b4 = next;
        event->next->previous = 0;
        ReleaseInputEvent(event);
        return;
    }
    g_pInputEventTail_0049d4b8 = 0;
    g_pInputEventHead_0049d4b4 = 0;
    ReleaseInputEvent(event);
#else
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
#endif
}

/* Function start: 0x462DFC */
short GetNextInputEvent(InputEventState *state)
{
#if 0
    short *eventX;
    int eventY;
    int type;

    type = 0;
    if (g_pInputEventHead_0049d4b4 != 0) {
        eventX = &g_pInputEventHead_0049d4b4->x;
        eventY = (int)g_pInputEventHead_0049d4b4->y;
        if ((int)g_stMouseCursorState_0059ab10.viewport->left > (int)*eventX)
            *eventX = g_stMouseCursorState_0059ab10.viewport->left;
        else if ((int)g_stMouseCursorState_0059ab10.viewport->right < (int)*eventX)
            *eventX = g_stMouseCursorState_0059ab10.viewport->right;
        if ((int)g_stMouseCursorState_0059ab10.viewport->top > eventY)
            g_pInputEventHead_0049d4b4->y = g_stMouseCursorState_0059ab10.viewport->top;
        else if ((int)g_stMouseCursorState_0059ab10.viewport->bottom <
                 eventY)
            g_pInputEventHead_0049d4b4->y =
                g_stMouseCursorState_0059ab10.viewport->bottom;

        state->modifiers =
            (short)g_pInputEventHead_0049d4b4->modifiers;
        switch (g_pInputEventHead_0049d4b4->type) {
        case 1:
            g_stMouseCursorState_0059ab10.x = g_pInputEventHead_0049d4b4->x;
            g_stMouseCursorState_0059ab10.y = g_pInputEventHead_0049d4b4->y;
            g_stMouseCursorState_0059ab10.primaryButton = 0;
            state->x = g_pInputEventHead_0049d4b4->x;
            state->y = g_pInputEventHead_0049d4b4->y;
            type = 1;
            break;
        case 2:
            g_stMouseCursorState_0059ab10.x = g_pInputEventHead_0049d4b4->x;
            g_stMouseCursorState_0059ab10.y = g_pInputEventHead_0049d4b4->y;
            g_stMouseCursorState_0059ab10.primaryButton =
                (unsigned char)g_pInputEventHead_0049d4b4->primaryButton;
            g_stMouseCursorState_0059ab10.secondaryButton =
                (unsigned char)g_pInputEventHead_0049d4b4->secondaryButton;
            state->x = g_pInputEventHead_0049d4b4->x;
            state->y = g_pInputEventHead_0049d4b4->y;
            type = 2;
            state->value =
                (int)g_pInputEventHead_0049d4b4->secondaryButton * 2 |
                (int)g_pInputEventHead_0049d4b4->primaryButton;
            break;
        case 3:
            type = 3;
            state->value = g_pInputEventHead_0049d4b4->value;
            state->x = g_stMouseCursorState_0059ab10.x;
            state->y = g_stMouseCursorState_0059ab10.y;
            break;
        case 4:
            type = 4;
            state->x = g_pInputEventHead_0049d4b4->value;
            state->x = g_stMouseCursorState_0059ab10.x;
            state->y = g_stMouseCursorState_0059ab10.y;
            break;
        case 5:
            type = 5;
            state->x = g_pInputEventHead_0049d4b4->value;
            break;
        case 6:
            type = 6;
            state->x = g_pInputEventHead_0049d4b4->x;
            state->y = g_pInputEventHead_0049d4b4->y;
            break;
        case 7:
            type = 7;
            state->x = g_pInputEventHead_0049d4b4->x;
            state->y = g_pInputEventHead_0049d4b4->y;
            break;
        case 8:
            type = 8;
            state->x = g_pInputEventHead_0049d4b4->x;
            state->y = g_pInputEventHead_0049d4b4->y;
            break;
        case 9:
            type = 9;
            state->x = g_pInputEventHead_0049d4b4->x;
            state->y = g_pInputEventHead_0049d4b4->y;
            break;
        case 10:
            type = 10;
            state->x = g_pInputEventHead_0049d4b4->x;
            state->y = g_pInputEventHead_0049d4b4->y;
            break;
        case 13:
            g_stMouseCursorState_0059ab10.x = g_pInputEventHead_0049d4b4->x;
            g_stMouseCursorState_0059ab10.y = g_pInputEventHead_0049d4b4->y;
            state->x = g_pInputEventHead_0049d4b4->x;
            state->y = g_pInputEventHead_0049d4b4->y;
            type = 13;
            break;
        }
        RemoveInputEvent(g_pInputEventHead_0049d4b4);
    }
    return type;
#else
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
#endif
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
#if 0
    InputEvent *event;
    int eventType;
    int modifiers;

    event = g_pInputEventHead_0049d4b4;
    modifiers = 0;
    while (event != 0 && event->type != type)
        event = event->next;
    if (event != 0) {
        state->type = event->type;
        state->value = event->modifiers;
        state->timestamp = event->timestamp;
        eventType = (int)event->type;
        if (eventType == 2 || eventType == 1)
            modifiers = 1;
        modifiers |= ((unsigned short)event->primaryButton < 1 ? 0 : -1) & 2;
        modifiers |= ((unsigned short)event->secondaryButton < 1 ? 0 : -1) & 4;
        state->modifiers = (short)modifiers;
        state->x = event->x;
        state->y = event->y;
        return 1;
    }
    return 0;
#else
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
#endif
}

/* Function start: 0x4633E7 */
InputEventState *FindQueuedInputEvent(int type)
{
#if 0
    InputEvent *event = g_pInputEventHead_0049d4b4;

    while (event != 0 && event->type != type)
        event = event->next;
    return event != 0;
#else
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
#endif
}

/* Function start: 0x46354F */
void FlushInputEvents(void)
{
    g_nNextInputQueueFlushTick_0049d4cc = g_nInputClock_005c84a8 + 20;
    ReleaseInputEventQueue();
}

/* Function start: 0x463571 */
#if 0
short __stdcall InitializeMouseCursorDepth(int x, int y)
{
    (void)x;
    (void)y;
    g_nMouseCursorDrawDepth_0049d4d4 = 0;
    return 1;
}
#else
short InitializeMouseCursorDepth(void)
{
    g_nMouseCursorDrawDepth_0049d4d4 = 1;
    return 1;
}
#endif

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

/* Function start: WC2_UNMAPPED */
void SuspendWc1MouseCursor(void)
{
    g_nMouseCursorDrawDepth_0049d4d4 = g_nMouseCursorDrawDepth_0049d4d4 - 1;
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

/* Function start: WC2_UNMAPPED */
unsigned int GetWc1VideoReleaseResult(void)
{
    return 0;
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
    unsigned int control = g_abInputKeyState_0059a860[0x1d];
    unsigned int alt = g_abInputKeyState_0059a860[0x38];

    memset(g_abInputKeyState_0059a860, 0,
           sizeof(g_abInputKeyState_0059a860));
    g_abInputKeyState_0059a860[0x1d] = control;
    g_abInputKeyState_0059a860[0x38] = alt;
    ClearDebugPauseFlags();
}

#pragma function(memset)
/* Function start: 0x4642D6 */
void ClearInputKeyState(void)
{
    memset(g_abInputKeyState_0059a860, 0,
           sizeof(g_abInputKeyState_0059a860));
    ClearDebugPauseFlags();
}
#pragma intrinsic(memset)

/* Function start: 0x46431A */
void SetInputKeyState(int scanCode, unsigned char pressed)
{
    if (scanCode >= 0 && scanCode < 0x80) {
        g_abInputKeyState_0059a860[scanCode] = pressed;
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
    unsigned int distance;
    int previous;
    int best;
    int bestObject;
    int obj;
    int screenOffset;
    int *placed;
    int *sortedEntry;
    int sorted;

    previous = -999999999;
    bestObject = -1;
    memset(g_anObjectDepthPlaced_0059a8f0, 0,
           sizeof(g_anObjectDepthPlaced_0059a8f0));
    obj = 0;
    for (; obj < WC2_SPACE_OBJECT_COUNT; obj++) {
        distance = (unsigned short)g_asObjectDistance_00493ae8[obj];
        if (previous < (int)distance) {
            previous = (int)distance;
            bestObject = obj;
        }
    }
    sorted = 0;
    sortedEntry = g_anSortedObject_0059aa00;
    for (; sorted < WC2_SPACE_OBJECT_COUNT; sorted++, sortedEntry++) {
        best = -1;
        *sortedEntry = bestObject;
        if (bestObject == -1)
            return;
        screenOffset = 0;
        obj = 0;
        g_anObjectDepthPlaced_0059a8f0[bestObject] = 1;
        bestObject = -1;
        placed = g_anObjectDepthPlaced_0059a8f0;
        for (; placed < g_anObjectDepthPlaced_0059a8f0 +
                           WC2_SPACE_OBJECT_COUNT;
             screenOffset += sizeof(short), placed++, obj++) {
            if (*placed == 0 &&
                *(short *)((unsigned char *)g_asObjectScreenX_00493598 +
                           screenOffset) != (short)0x8001) {
                distance = *(unsigned short *)(
                    (unsigned char *)g_asObjectDistance_00493ae8 +
                    screenOffset);
                if (best < (int)distance && previous >= (int)distance) {
                    bestObject = obj;
                    best = (int)distance;
                }
            }
        }
    }
}

/* Function start: 0x4644DA */
void draw_sorted_objects_to_buffer(void)
{
    int *sortedEntry;
    int obj;
    int objectClass;
    unsigned char *shape;
    short screenX;
    short screenY;
    int specialObject;
#ifdef WC1_SDL
    float enhancedScreenX;
    float enhancedScreenY;
    short projectedScreenX;
    short projectedScreenY;
#endif

    sortedEntry = g_anSortedObject_0059aa00;
    do {
        obj = *sortedEntry;
        if (obj < 0)
            return;
        if ((int)g_acObjectType_00493980[obj] < 0)
            return;
        objectClass = g_aeObjectClass_00495328[obj];
#ifdef WC1_SDL
        enhancedScreenX = (float)(short)(
            g_asObjectScreenX_00493598[obj] + g_nViewCenterX_005c80d8);
        enhancedScreenY = (float)(short)(
            g_asObjectScreenY_00493628[obj] + g_nViewCenterY_005c80da);
        if (objectClass != OBJECT_CLASS_NULL &&
            objectClass != OBJECT_CLASS_FIXED_OBJECT &&
            obj != DAT_00469208 &&
            g_aObjectViewPosition_0059afa0[obj].z != 0) {
            projectedScreenX = (short)(DivideFixed(
                MultiplyFixed(
                    (short)(g_nScreenWidth_0046daa4 & ~1) << 7,
                    g_aObjectViewPosition_0059afa0[obj].x),
                g_aObjectViewPosition_0059afa0[obj].z) >> 8);
            projectedScreenY = (short)(DivideFixed(
                MultiplyFixed(
                    (short)(g_nScreenWidth_0046daa4 & ~1) << 7,
                    g_aObjectViewPosition_0059afa0[obj].y),
                g_aObjectViewPosition_0059afa0[obj].z) >> 8);
            if (projectedScreenX == g_asObjectScreenX_00493598[obj] &&
                projectedScreenY == g_asObjectScreenY_00493628[obj]) {
                enhancedScreenX =
                    (float)g_nViewCenterX_005c80d8 +
                    (float)(((double)(g_nScreenWidth_0046daa4 & ~1) * 0.5 *
                             g_aObjectViewPosition_0059afa0[obj].x) /
                            g_aObjectViewPosition_0059afa0[obj].z);
                enhancedScreenY =
                    (float)g_nViewCenterY_005c80da +
                    (float)(((double)(g_nScreenWidth_0046daa4 & ~1) * 0.5 *
                             g_aObjectViewPosition_0059afa0[obj].y) /
                            g_aObjectViewPosition_0059afa0[obj].z);
            }
        } else if (objectClass == OBJECT_CLASS_FIXED_OBJECT &&
                   g_acObjectType_00493980[obj] == OBJECT_TYPE_THRUSTERS) {
            Wc1SdlGetThrusterScreenPosition(
                (short)obj, &enhancedScreenX, &enhancedScreenY);
        }
#endif
        if (objectClass != OBJECT_CLASS_NULL) {
            switch (objectClass) {
            default:
                screenY = g_asObjectScreenY_00493628[obj];
                screenX = (short)(g_asObjectScreenX_00493598[obj] +
                                  g_nViewCenterX_005c80d8);
                shape = g_apObjectShape_00493868[obj];
                g_asObjectDrawX_0059d000[obj] = screenX;
                screenY = (short)(screenY + g_nViewCenterY_005c80da);
                g_asObjectDrawY_0059cf80[obj] = screenY;
                if (shape != 0) {
#ifdef WC1_SDL
                    if (!Wc1SdlRecordSpaceSprite(
                            &g_stViewBuffer_005d2b00, enhancedScreenX, enhancedScreenY,
                            shape,
                            g_asObjectViewFrame_00493508[obj],
                            g_asObjectScreenAngle_004936b8[obj],
                            g_asObjectScreenScale_00493a58[obj],
                            g_asObjectFlip_004939c8[obj]))
#endif
                    DrawSpriteScaled(
                        &g_stViewBuffer_005d2b00, screenX, screenY, shape,
                        g_asObjectViewFrame_00493508[obj],
                        g_asObjectScreenAngle_004936b8[obj],
                        g_asObjectScreenScale_00493a58[obj],
                        g_asObjectFlip_004939c8[obj]);
                }
                break;
            case OBJECT_CLASS_STAR:
#ifdef WC1_SDL
                /* WCDX fix: planets use the per-object scaled path above. */
#else
            case OBJECT_CLASS_PLANET:
#endif
            case OBJECT_CLASS_DUST:
                specialObject = (int)DAT_00469208;
                screenY = g_asObjectScreenY_00493628[obj];
                screenX = (short)(g_asObjectScreenX_00493598[obj] +
                                  g_nViewCenterX_005c80d8);
                g_asObjectDrawX_0059d000[obj] = screenX;
                screenY = (short)(screenY + g_nViewCenterY_005c80da);
                g_asObjectDrawY_0059cf80[obj] = screenY;
                if (specialObject == obj)
                    shape = g_apObjectShape_00493868[obj];
                else
                    shape = g_pConstellationShape_005a765c;
#ifdef WC1_SDL
                if (!Wc1SdlRecordSpaceSprite(
                        &g_stViewBuffer_005d2b00, enhancedScreenX, enhancedScreenY, shape,
                        g_asObjectViewFrame_00493508[obj], 0, 0x100, 0))
#endif
                DrawSpriteDefault(&g_stViewBuffer_005d2b00, screenX, screenY, shape,
                                  g_asObjectViewFrame_00493508[obj]);
                break;
            }
        }
        sortedEntry++;
    } while (sortedEntry < g_anSortedObject_0059aa00 +
                           WC2_SPACE_OBJECT_COUNT);
}

/* Function start: 0x46470E */
void intro_drawbackgroundships(void)
{
    unsigned char *shape;
    int objectClass;
    int obj;
    int dwordOffset;
    int shortOffset;
    int zero;

    obj = 0;
    shortOffset = 0;
    zero = 0;
    dwordOffset = 0;
    for (; dwordOffset < WC2_SPACE_OBJECT_COUNT * (int)sizeof(int);
         shortOffset += sizeof(short),
         dwordOffset += sizeof(int),
         obj++) {
        if (*(enum ObjectType *)((unsigned char *)g_acObjectType_00493980 +
                                 dwordOffset) < zero)
            return;
        objectClass = *(enum ObjectClass *)(
            (unsigned char *)g_aeObjectClass_00495328 + dwordOffset);
        if (objectClass != OBJECT_CLASS_NULL) {
            switch (objectClass) {
            default:
#ifdef WC1_SDL
                shape = g_apObjectShape_00493868[obj];
#else
                shape = *(unsigned char **)(
                    (unsigned char *)g_apObjectShape_00493868 +
                    dwordOffset);
#endif
                if (shape != 0) {
                    DrawSolidColourSpriteScaled(
                        &g_stViewBuffer_005d2b00,
                        *(short *)((unsigned char *)g_asObjectDrawX_0059d000 +
                                   shortOffset),
                        *(short *)((unsigned char *)g_asObjectDrawY_0059cf80 +
                                   shortOffset),
                        shape,
                        *(short *)((unsigned char *)g_asObjectViewFrame_00493508 +
                                   shortOffset),
                        *(short *)((unsigned char *)g_asObjectScreenAngle_004936b8 +
                                   shortOffset),
                        *(short *)((unsigned char *)g_asObjectScreenScale_00493a58 +
                                   shortOffset),
                        *(short *)((unsigned char *)g_asObjectFlip_004939c8 +
                                   shortOffset),
                        g_cPrimaryViewBufferColour_0049cb88);
                }
                break;
            case OBJECT_CLASS_STAR:
#ifdef WC1_SDL
                /* Erase planets with the same scaled geometry used to draw. */
#else
            case OBJECT_CLASS_PLANET:
#endif
            case OBJECT_CLASS_DUST:
                if (obj == DAT_00469208)
#ifdef WC1_SDL
                    shape = g_apObjectShape_00493868[obj];
#else
                    shape = *(unsigned char **)(
                        (unsigned char *)g_apObjectShape_00493868 +
                        dwordOffset);
#endif
                else
                    shape = g_pConstellationShape_005a765c;
                DrawSolidColourSprite(
                    &g_stViewBuffer_005d2b00,
                    *(short *)((unsigned char *)g_asObjectDrawX_0059d000 +
                               shortOffset),
                    *(short *)((unsigned char *)g_asObjectDrawY_0059cf80 +
                               shortOffset),
                    shape,
                    *(short *)((unsigned char *)g_asObjectViewFrame_00493508 +
                               shortOffset),
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
    short viewportHeight;
    short viewportWidth;
    const ScreenViewportGeometry *viewportGeometry;

    g_cScreenViewportMode_005c82a6 = mode;
    modeIndex = (int)mode;
    if (modeIndex >= 4) {
        if (modeIndex <= 5)
            goto static_geometry;
    }

    g_pScreenViewportGeometry_0059a9f4 =
        (const ScreenViewportGeometry *)(
            (const unsigned char *)g_pScreenViewportPacket_005a6b94 +
            g_pScreenViewportPacket_005a6b94
                ->geometryOffsets[modeIndex]);
    goto geometry_ready;

static_geometry:
    g_pScreenViewportGeometry_0059a9f4 =
        &g_aScreenViewportGeometry_0046dab8[modeIndex];

geometry_ready:

    if (g_nCockpitDisplayMode_0049d71c != 0 && g_nCockpitDisplayMode_0049d71c != -2) {
        viewportWidth = g_pScreenViewportGeometry_0059a9f4->width;
        viewportGeometry = g_pScreenViewportGeometry_0059a9f4;
        *(short *)&g_nScreenWidth_0046daa4 = viewportWidth;
        g_nViewCenterX_005c80d8 = (short)(viewportWidth / 2);
        viewportHeight = viewportGeometry->height;
        g_nViewCenterY_005c80da = (short)(viewportHeight / 2);
        *(short *)&g_nScreenHeight_0046daa8 = viewportHeight;
        g_nViewportOriginX_0059ab52 = viewportGeometry->originX;
        g_nViewportOriginY_0059ab50 = viewportGeometry->originY;
        switch ((int)g_cCockpitView_0059dab0) {
        case 0:
            g_nViewportOriginY_0059ab50 += 10;
            g_nViewCenterY_005c80da += 10;
            break;
        case 1:
            g_nViewportOriginY_0059ab50 += 25;
            g_nViewCenterY_005c80da += 25;
            break;
        case 2:
            g_nViewportOriginY_0059ab50 += 50;
            g_nViewCenterY_005c80da += 50;
            break;
        }
        *(short *)&g_nScreenWidth_0046daa4 = 320;
        *(short *)&g_nScreenHeight_0046daa8 = 200;
        return;
    }

    viewportWidth = g_pScreenViewportGeometry_0059a9f4->width;
    viewportGeometry = g_pScreenViewportGeometry_0059a9f4;
    *(short *)&g_nScreenWidth_0046daa4 = viewportWidth;
    g_nViewCenterX_005c80d8 = (short)(viewportWidth / 2);
    viewportHeight = viewportGeometry->height;
    g_nViewCenterY_005c80da = (short)(viewportHeight / 2);
    *(short *)&g_nScreenHeight_0046daa8 = viewportHeight;
    g_nViewportOriginX_0059ab52 = viewportGeometry->originX;
    g_nViewportOriginY_0059ab50 = viewportGeometry->originY;
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
unsigned short GetNavRangeSentinel(void)
{
    return 0x8000;
}

/* Function start: 0x464AC4 */
unsigned short GetOriginalFreeMemory(void)
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

/* Function start: WC2_UNMAPPED */
void Wc1ShutdownHook(int vector, void *handler)
{
    (void)vector;
    (void)handler;
}

/* Function start: 0x464B82 */
void PersonnelDriveHook()
{
}

/* Function start: 0x464B92 */
unsigned short __stdcall SelectDiskDriveHook(short drive)
{
    return 0;
}

/* Function start: 0x464BA5 */
short GetCurrentDiskDriveHook(void)
{
    return 0;
}

/* Function start: 0x464BB8 */
unsigned short __stdcall GetShutdownErrorCode(unsigned char *driveState)
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
#if 0
    int magnitude = Vector_magnitude(vector);
    int fixedRange = abs((int)range << 8);

    if (fixedRange >= magnitude)
        return 1;
    return 0;
#else
    int magnitude;
    int fixedRange;

    fixedRange = (int)range << 8;
    magnitude = Vector_magnitude(vector);
    if (abs(fixedRange) >= magnitude)
        return 1;
    return 0;
#endif
}

/* Function start: WC2_UNMAPPED */
unsigned int shrink_vector(FixedVector *vector)
{
    unsigned int shrinking;

    do {
        shrinking = shrink(&vector->x);
        shrinking |= shrink(&vector->y);
        shrinking |= shrink(&vector->z);
    } while (shrinking != 0);
    return 0;
}

/* Function start: WC2_UNMAPPED */
unsigned int shrink(int *component)
{
    int value;
    unsigned short fraction;
    unsigned int integerPart;
    short signedIntegerPart;

    value = *component / 2;
    *component = value;
    fraction = (unsigned short)value;
    integerPart = (unsigned int)value >> 16;
    signedIntegerPart = (short)integerPart;
    if (signedIntegerPart == 0)
        return fraction > 0x0f00;
    if (signedIntegerPart == -1)
        return fraction < 0xf100;
    if (signedIntegerPart != 0)
        integerPart = 1;
    return integerPart;
}

/* Function start: 0x42BAC0 */
void __stdcall FillGraphicSuffix(char *path, short number, short digits)
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
void __stdcall ConvertChar_Int(char *text, short number, short digits)
{
    while (digits--) {
        text[digits] = (char)(number % 10) + '0';
        number /= 10;
    }
}
