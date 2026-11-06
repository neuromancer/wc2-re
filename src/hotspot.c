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

    if (g_pActiveScenePacket != scenePacket) {
        ReleasePacketSlot(&g_stSceneZoneTable.data);
        nextChunk = (unsigned char *)scenePacket +
            ((ScenePacketHeader *)scenePacket)->formOffset;
        for (;;) {
            cursor = nextChunk;
            formId = ReadNextSceneForm(&cursor, &nextChunk);
            if (formId == 0x52544f48) {
                DecodeSceneResourceChunk(
                    &cursor, &g_stSceneZoneTable);
                if (g_stSceneZoneTable.type != 0)
                    ReleasePacketSlot(
                        &g_stSceneZoneTable.data);
            } else if (formId == 0x54585448) {
                DecodeSceneResourceChunk(
                    &cursor, &g_stSceneTextList);
                if (g_stSceneTextList.type == 2)
                    break;
                ReleasePacketSlot(&g_stSceneTextList.data);
            } else if (formId == 0)
                break;
        }
        g_pActiveScenePacket = scenePacket;
    }
    hotspotCount = g_stSceneZoneTable.count;
    g_pActiveScenePacket = 0;
    ReleasePacketSlot(&g_stSceneZoneTable.data);
    ReleasePacketSlot(&g_stSceneTextList.data);
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

    if (g_pActiveScenePacket != scenePacket) {
        ReleasePacketSlot(&g_stSceneZoneTable.data);
        nextForm = (unsigned char *)scenePacket +
            ((ScenePacketHeader *)scenePacket)->formOffset;
        for (;;) {
            cursor = nextForm;
            magic = ReadNextSceneForm(&cursor, &nextForm);
            if (magic == 0x52544f48) {
                DecodeSceneResourceChunk(
                    &cursor, &g_stSceneZoneTable);
                if (g_stSceneZoneTable.type != 0)
                    ReleasePacketSlot(
                        &g_stSceneZoneTable.data);
            } else if (magic == 0x54585448) {
                DecodeSceneResourceChunk(
                    &cursor, &g_stSceneTextList);
                if (g_stSceneTextList.type == 2)
                    break;
                ReleasePacketSlot(&g_stSceneTextList.data);
            } else if (magic == 0)
                break;
        }
        g_pActiveScenePacket = scenePacket;
    }

    for (index = 0; index < g_stSceneZoneTable.count;
         index++) {
        hotspot = ((SceneHotspot **)
            g_stSceneZoneTable.data)[index];
        if (hotspot->selection == selection) {
            g_128_nShotHotspotLeft = hotspot->left;
            g_27_nSceneHotspotTopValue = hotspot->top;
            g_8_nSceneRegionRightWord = hotspot->right;
            g_15_nSceneAreaLowestWord = hotspot->bottom;
            break;
        }
    }
    g_pActiveScenePacket = 0;
    ReleasePacketSlot(&g_stSceneZoneTable.data);
    ReleasePacketSlot(&g_stSceneTextList.data);
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
    g_pszPersonnelFooter = 0;
    if (g_pActiveScenePacket != scenePacket) {
        ReleasePacketSlot(&g_stSceneZoneTable.data);
        nextForm = (unsigned char *)scenePacket +
            ((ScenePacketHeader *)scenePacket)->formOffset;
        for (;;) {
            cursor = nextForm;
            magic = ReadNextSceneForm(&cursor, &nextForm);
            if (magic == 0x52544f48) {
                DecodeSceneResourceChunk(
                    &cursor, &g_stSceneZoneTable);
                if (g_stSceneZoneTable.type != 0)
                    ReleasePacketSlot(
                        &g_stSceneZoneTable.data);
            } else if (magic == 0x54585448) {
                DecodeSceneResourceChunk(
                    &cursor, &g_stSceneTextList);
                if (g_stSceneTextList.type == 2)
                    break;
                ReleasePacketSlot(&g_stSceneTextList.data);
            } else if (magic == 0)
                break;
        }
        g_pActiveScenePacket = scenePacket;
    }

    for (index = 0; index < g_stSceneZoneTable.count; index++) {
        hotspot =
            ((SceneHotspot **)g_stSceneZoneTable.data)[index];
        OffsetSceneHotspotBounds(&hitRect, hotspot, offsetX, offsetY);
        if (IsPointInRect(x, y, &hitRect.left) != 0) {
            selection = hotspot->selection;
            if (g_stSceneTextList.data != 0) {
                g_pszPersonnelFooter =
                    ((char **)g_stSceneTextList.data)[0];
                if (index != g_stSceneZoneTable.count - 1) {
                    index = (short)(selection - 1);
                    while (index-- != 0) {
                        g_pszPersonnelFooter =
                            DosStrchr(g_pszPersonnelFooter, 0) + 1;
                    }
                    if (*g_pszPersonnelFooter == 0)
                        g_pszPersonnelFooter = 0;
                } else
                    g_pszPersonnelFooter = 0;
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
            g_bJoystickCalibrationHotkey != 0) {
            controlPressed = 0;
            jPressed = 0;
            g_bJoystickCalibrationHotkey = 0;
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
    if (g_bDisableChalkboardReplay != 0 && selection == 2) {
        selection = 0;
        g_pszPersonnelFooter = 0;
    }
    if (selection != 0) {
        SetMouseCursorShape(g_18_pEventManager->cursorShape, 1);
    } else if (g_bPersonnelMenuDrawing != 0) {
        SetMouseCursorShape(g_18_pEventManager->cursorShape, 0);
    }
    g_8_bSceneBackdropClicked = 0;
    if (inputEvent->type != 1) {
        selection = 0;
    } else if (selection == 0 && dismissOnBackground != 0) {
        g_8_bSceneBackdropClicked = 1;
    }
    if (selection != 0 ||
        (dismissOnBackground != 0 &&
         g_8_bSceneBackdropClicked != 0)) {
        g_pActiveScenePacket = 0;
        ReleasePacketSlot(&g_stSceneZoneTable.data);
        ReleasePacketSlot(&g_stSceneTextList.data);
    }
    return selection;
}

/* This unit's initialized data, after its functions so that the two unused
 * words do not move the functions' symbol keys.  Nothing between intro.c's
 * literals and campaign.c's data marks the boundary between the two units,
 * so placing the mission arrays here is evidence of position, not of
 * ownership.  Nothing in the original references the two unused words. */
MissionNavPoint g_aMissionNavPoints[MISSION_NAV_POINT_COUNT] = {
    { 0 }
}; /* 0x00491e98 */
MissionShipRecord g_aMissionShips[MISSION_SHIP_COUNT] = { { 0 } }; /* 0x00492290 */
int g_nUnusedSceneWord = 0; /* 0x00492650 */
void *g_pActiveScenePacket = 0; /* 0x00492654 */
char *g_pszPersonnelFooter = 0; /* 0x00492658 */
int g_nUnusedSceneFlag = 1; /* 0x0049265c */

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
SceneResourceTable g_stSceneZoneTable; /* 0x005d3bf8 */
SceneResourceTable g_stSceneTextList; /* 0x005d3c00 */
unsigned char g_5_abMissionAuxBlockBytes[0x28]; /* 0x005d3c10 */
unsigned char g_abSeriesAuxData[0x28]; /* 0x005d3c40 */
MissionObjectiveSource g_69_aMissionObjectiveSources[8]; /* 0x005d3c70 */
MissionHeader g_13_stMissionHeaderBlock; /* 0x005d3e70 */
char g_10_nUnusedHotspotWord; /* 0x005d3e88 */ /* never referenced */
CampaignDate *g_4_pElapsedCampaignDatePointer; /* 0x005d3e8c */
unsigned char g_abMissionBriefingData[0xde]; /* 0x005d3e90 */
