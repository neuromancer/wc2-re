/*
 *  Scene hotspot selection.
 *
 *  Retail object 0x409120-0x40984F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x409120 */
short WaitForSceneAdvance(void *scenePacket)
{
    unsigned char *cursor;
    unsigned char *nextChunk;
    short hotspotCount;
    unsigned int formId;

    if (g_pActiveScenePacket_00492654 != scenePacket) {
        ReleasePacketSlot(&g_stSceneHotspotTable_005d3bf8.data);
        nextChunk = (unsigned char *)scenePacket +
            ((ScenePacketHeader *)scenePacket)->formOffset;
        for (;;) {
            cursor = nextChunk;
            formId = ReadNextSceneForm(&cursor, &nextChunk);
            if (formId == 0x52544f48) {
                DecodeSceneResourceChunk(
                    &cursor, &g_stSceneHotspotTable_005d3bf8);
                if (g_stSceneHotspotTable_005d3bf8.type != 0)
                    ReleasePacketSlot(
                        &g_stSceneHotspotTable_005d3bf8.data);
            } else if (formId == 0x54585448) {
                DecodeSceneResourceChunk(
                    &cursor, &g_stSceneTextTable_005d3c00);
                if (g_stSceneTextTable_005d3c00.type == 2)
                    break;
                ReleasePacketSlot(&g_stSceneTextTable_005d3c00.data);
            } else if (formId == 0)
                break;
        }
        g_pActiveScenePacket_00492654 = scenePacket;
    }
    hotspotCount = g_stSceneHotspotTable_005d3bf8.count;
    g_pActiveScenePacket_00492654 = 0;
    ReleasePacketSlot(&g_stSceneHotspotTable_005d3bf8.data);
    ReleasePacketSlot(&g_stSceneTextTable_005d3c00.data);
    if (hotspotCount <= 2)
        return 0;
    return 1;
}

/* Hit-test callee from the event manager. */
short IsPointInRect(short x, short y, const short *rect);

/* Function start: 0x40926E */
void LoadSceneHotspotBoundsForSelection(void *scenePacket,
                                        short selection)
{
    unsigned char *cursor;
    unsigned char *nextForm;
    SceneHotspot *hotspot;
    short index;
    unsigned int magic;

    if (g_pActiveScenePacket_00492654 != scenePacket) {
        ReleasePacketSlot(&g_stSceneHotspotTable_005d3bf8.data);
        nextForm = (unsigned char *)scenePacket +
            ((ScenePacketHeader *)scenePacket)->formOffset;
        for (;;) {
            cursor = nextForm;
            magic = ReadNextSceneForm(&cursor, &nextForm);
            if (magic == 0x52544f48) {
                DecodeSceneResourceChunk(
                    &cursor, &g_stSceneHotspotTable_005d3bf8);
                if (g_stSceneHotspotTable_005d3bf8.type != 0)
                    ReleasePacketSlot(
                        &g_stSceneHotspotTable_005d3bf8.data);
            } else if (magic == 0x54585448) {
                DecodeSceneResourceChunk(
                    &cursor, &g_stSceneTextTable_005d3c00);
                if (g_stSceneTextTable_005d3c00.type == 2)
                    break;
                ReleasePacketSlot(&g_stSceneTextTable_005d3c00.data);
            } else if (magic == 0)
                break;
        }
        g_pActiveScenePacket_00492654 = scenePacket;
    }

    for (index = 0; index < g_stSceneHotspotTable_005d3bf8.count;
         index++) {
        hotspot = ((SceneHotspot **)
            g_stSceneHotspotTable_005d3bf8.data)[index];
        if (hotspot->selection == selection) {
            g_nSceneHotspotLeft_005d2120 = hotspot->left;
            g_nSceneHotspotTop_005d2122 = hotspot->top;
            g_nSceneHotspotRight_005d2124 = hotspot->right;
            g_nSceneHotspotBottom_005d2126 = hotspot->bottom;
            break;
        }
    }
    g_pActiveScenePacket_00492654 = 0;
    ReleasePacketSlot(&g_stSceneHotspotTable_005d3bf8.data);
    ReleasePacketSlot(&g_stSceneTextTable_005d3c00.data);
}

/* Function start: 0x409417 */
short FindSceneHotspotAtPosition(void *scenePacket, short offsetX,
                                 short offsetY, short x, short y)
{
    ShortRect hitRect;
    short selection;
    unsigned char *cursor;
    SceneHotspot *hotspot;
    short index;
    unsigned char *nextForm;
    unsigned int magic;

    selection = 0;
    g_pszPersonnelFooter_00492658 = 0;
    if (g_pActiveScenePacket_00492654 != scenePacket) {
        ReleasePacketSlot(&g_stSceneHotspotTable_005d3bf8.data);
        nextForm = (unsigned char *)scenePacket +
            ((ScenePacketHeader *)scenePacket)->formOffset;
        for (;;) {
            cursor = nextForm;
            magic = ReadNextSceneForm(&cursor, &nextForm);
            if (magic == 0x52544f48) {
                DecodeSceneResourceChunk(
                    &cursor, &g_stSceneHotspotTable_005d3bf8);
                if (g_stSceneHotspotTable_005d3bf8.type != 0)
                    ReleasePacketSlot(
                        &g_stSceneHotspotTable_005d3bf8.data);
            } else if (magic == 0x54585448) {
                DecodeSceneResourceChunk(
                    &cursor, &g_stSceneTextTable_005d3c00);
                if (g_stSceneTextTable_005d3c00.type == 2)
                    break;
                ReleasePacketSlot(&g_stSceneTextTable_005d3c00.data);
            } else if (magic == 0)
                break;
        }
        g_pActiveScenePacket_00492654 = scenePacket;
    }

    for (index = 0; index < g_stSceneHotspotTable_005d3bf8.count; index++) {
        hotspot =
            ((SceneHotspot **)g_stSceneHotspotTable_005d3bf8.data)[index];
        OffsetSceneHotspotBounds(&hitRect, hotspot, offsetX, offsetY);
        if (IsPointInRect(x, y, &hitRect.left) != 0) {
            selection = hotspot->selection;
            if (g_stSceneTextTable_005d3c00.data != 0) {
                g_pszPersonnelFooter_00492658 =
                    ((char **)g_stSceneTextTable_005d3c00.data)[0];
                if (index != g_stSceneHotspotTable_005d3bf8.count - 1) {
                    index = (short)(selection - 1);
                    while (index-- != 0) {
                        g_pszPersonnelFooter_00492658 =
                            DosStrchr(g_pszPersonnelFooter_00492658, 0) + 1;
                    }
                    if (*g_pszPersonnelFooter_00492658 == 0)
                        g_pszPersonnelFooter_00492658 = 0;
                } else
                    g_pszPersonnelFooter_00492658 = 0;
            }
            break;
        }
    }
    return selection;
}

/* Function start: 0x40963B */
short PollSceneHotspotInput(void *scenePacket, short offsetX,
                            short offsetY, short dismissOnBackground,
                            short maximumSelection)
{
#ifdef SDL_PORT
    /* GetNextInputEvent leaves state->type untouched when the queue is
     * empty (it only ever sets x/y in that case) -- fine for its other
     * callers, who already know an polled is waiting, but this function
     * falls back to &polled below whenever FindQueuedInputEvent(1) finds
     * no click anywhere in the queue, then calls GetNextInputEvent(&polled)
     * regardless of whether anything is actually queued. An uninitialized
     * polled.type that happens to equal 1 reads as a real click at the
     * current cursor position, with no input at all -- the main screen
     * and mission-select door hotspots advancing with zero input. See
     * issue #22. */
    InputEventState polled = {0};
#else
    InputEventState polled;
#endif
    short selection;
    InputEventState *inputEvent;
    short controlPressed;
    short jPressed;

    selection = 0;
    ServiceInputDevices(15);
    inputEvent = FindQueuedInputEvent(4);
    /* The original test ends in a stray semicolon, so the block below runs
     * whether or not a key event is queued. */
    if (inputEvent != 0)
        ;
    {
        controlPressed = (short)GetControlKeyState();
        jPressed = (short)GetJKeyState();
        if ((controlPressed != 0 && jPressed != 0) ||
            g_bJoystickCalibrationHotkey_005d1284 != 0) {
            controlPressed = 0;
            jPressed = 0;
            g_bJoystickCalibrationHotkey_005d1284 = 0;
            CalibrateJoystickInteractive();
            ReleaseInputEventQueue();
        }
    }
    inputEvent = FindQueuedInputEvent(1);
    if (inputEvent == 0)
        inputEvent = &polled;
    GetNextInputEvent(&polled);
    selection = FindSceneHotspotAtPosition(
        scenePacket, offsetX, offsetY, inputEvent->x, inputEvent->y);
    if (maximumSelection != 0 && selection > maximumSelection)
        selection = 0;
    if (g_bDisableChalkboardReplay_0049ca58 != 0 && selection == 2) {
        selection = 0;
        g_pszPersonnelFooter_00492658 = 0;
    }
    if (selection != 0) {
        SetMouseCursorShape(g_pInputManagerState_005c8464->cursorShape, 1);
    } else if (g_bPersonnelMenuDrawing_0049a6c0 != 0) {
        SetMouseCursorShape(g_pInputManagerState_005c8464->cursorShape, 0);
    }
    g_bSceneBackgroundClicked_005c9018 = 0;
    if (inputEvent->type != 1) {
        selection = 0;
    } else if (selection == 0 && dismissOnBackground != 0) {
        g_bSceneBackgroundClicked_005c9018 = 1;
    }
    if (selection != 0 ||
        (dismissOnBackground != 0 &&
         g_bSceneBackgroundClicked_005c9018 != 0)) {
        g_pActiveScenePacket_00492654 = 0;
        ReleasePacketSlot(&g_stSceneHotspotTable_005d3bf8.data);
        ReleasePacketSlot(&g_stSceneTextTable_005d3c00.data);
    }
    return selection;
}
