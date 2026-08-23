/*
 *  Disk data files and packet fetching with retry.
 *
 *  Address range 0x41d250-0x41efff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: OpenDiskDataFile/FetchDiskPacketRetrying/PromptInsertNumberedDisk.
 */
#include "game.h"

/* Function start: 0x409120 */
short WaitForSceneAdvance(void *scenePacket)
{
    unsigned char *cursor;
    unsigned char *nextForm;
    short hotspotCount;
    unsigned int formType;

    if (g_pActiveScenePacket_00492654 != scenePacket) {
        ReleasePacketSlot(&g_stSceneHotspotTable_005d3bf8.data);
        nextForm = (unsigned char *)scenePacket +
            ((ScenePacketHeader *)scenePacket)->formOffset;
        do {
            cursor = nextForm;
            formType = ReadNextSceneForm(&cursor, &nextForm);
            if (formType == 0x52544f48) {
                DecodeSceneResourceChunk(
                    &cursor, &g_stSceneHotspotTable_005d3bf8);
                if (g_stSceneHotspotTable_005d3bf8.type != 0)
                    ReleasePacketSlot(
                        &g_stSceneHotspotTable_005d3bf8.data);
            } else if (formType == 0x54585448) {
                DecodeSceneResourceChunk(
                    &cursor, &g_stSceneTextTable_005d3c00);
                if (g_stSceneTextTable_005d3c00.type == 2)
                    break;
                ReleasePacketSlot(&g_stSceneTextTable_005d3c00.data);
            }
        } while (formType != 0);
        g_pActiveScenePacket_00492654 = scenePacket;
    }
    hotspotCount = g_stSceneHotspotTable_005d3bf8.count;
    g_pActiveScenePacket_00492654 = 0;
    ReleasePacketSlot(&g_stSceneHotspotTable_005d3bf8.data);
    ReleasePacketSlot(&g_stSceneTextTable_005d3c00.data);
    if (hotspotCount <= 2)
        return 0;
    else
        return 1;
}

/* Function start: 0x40926E */
void LoadSceneHotspotBoundsForSelection(void *scenePacket,
                                        unsigned short selection)
{
    unsigned char *cursor;
    unsigned char *nextForm;
    SceneHotspot *hotspot;
    short index;
    unsigned int formType;

    if (g_pActiveScenePacket_00492654 != scenePacket) {
        ReleasePacketSlot(&g_stSceneHotspotTable_005d3bf8.data);
        nextForm = (unsigned char *)scenePacket +
            ((ScenePacketHeader *)scenePacket)->formOffset;
        do {
            cursor = nextForm;
            formType = ReadNextSceneForm(&cursor, &nextForm);
            if (formType == 0x52544f48) {
                DecodeSceneResourceChunk(
                    &cursor, &g_stSceneHotspotTable_005d3bf8);
                if (g_stSceneHotspotTable_005d3bf8.type != 0)
                    ReleasePacketSlot(
                        &g_stSceneHotspotTable_005d3bf8.data);
            } else if (formType == 0x54585448) {
                DecodeSceneResourceChunk(
                    &cursor, &g_stSceneTextTable_005d3c00);
                if (g_stSceneTextTable_005d3c00.type == 2)
                    break;
                ReleasePacketSlot(&g_stSceneTextTable_005d3c00.data);
            }
        } while (formType != 0);
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
    ShortRect bounds;
    short selection;
    unsigned char *cursor;
    SceneHotspot *hotspot;
    short index;
    unsigned char *nextForm;
    unsigned int formType;
    ScenePacketHeader *packet;
    SceneHotspot **hotspots;
    char **textEntries;

    selection = 0;
    g_pszPersonnelFooter_00492658 = 0;
    packet = (ScenePacketHeader *)scenePacket;
    if (g_pActiveScenePacket_00492654 != scenePacket) {
        ReleasePacketSlot(&g_stSceneHotspotTable_005d3bf8.data);
        nextForm = (unsigned char *)packet + packet->formOffset;
        do {
            cursor = nextForm;
            formType = ReadNextSceneForm(&cursor, &nextForm);
            if (formType == 0x52544f48) {
                DecodeSceneResourceChunk(
                    &cursor, &g_stSceneHotspotTable_005d3bf8);
                if (g_stSceneHotspotTable_005d3bf8.type != 0)
                    ReleasePacketSlot(
                        &g_stSceneHotspotTable_005d3bf8.data);
            } else if (formType == 0x54585448) {
                DecodeSceneResourceChunk(
                    &cursor, &g_stSceneTextTable_005d3c00);
                if (g_stSceneTextTable_005d3c00.type == 2)
                    break;
                ReleasePacketSlot(&g_stSceneTextTable_005d3c00.data);
            }
        } while (formType != 0);
        g_pActiveScenePacket_00492654 = scenePacket;
    }

    hotspots = (SceneHotspot **)g_stSceneHotspotTable_005d3bf8.data;
    index = 0;
    while (index < g_stSceneHotspotTable_005d3bf8.count) {
        hotspot = hotspots[index];
        OffsetSceneHotspotBounds(&bounds, hotspot, offsetX, offsetY);
        if (IsPointInRect(x, y, &bounds.left) != 0) {
            selection = hotspot->selection;
            textEntries = (char **)g_stSceneTextTable_005d3c00.data;
            if (textEntries != 0) {
                g_pszPersonnelFooter_00492658 = textEntries[0];
                if (index == g_stSceneHotspotTable_005d3bf8.count - 1) {
                    g_pszPersonnelFooter_00492658 = 0;
                } else {
                    index = (short)(selection - 1);
                    while (index-- != 0) {
                        g_pszPersonnelFooter_00492658 =
                            DosStrchr(g_pszPersonnelFooter_00492658, 0) + 1;
                    }
                    if (*g_pszPersonnelFooter_00492658 == 0)
                        g_pszPersonnelFooter_00492658 = 0;
                }
            }
            break;
        }
        index++;
    }
    return selection;
}

/* Function start: 0x40963B */
short PollSceneHotspotInput(void *scenePacket, short offsetX,
                            short offsetY, short dismissOnBackground,
                            short maximumSelection)
{
    short selection;
    InputEventState event;
    InputEventState *inputEvent;
    short controlPressed;
    short jPressed;

    selection = 0;
    ServiceInputDevices(15);
    inputEvent = FindQueuedInputEvent(4);
    if (inputEvent != 0) {
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
        inputEvent = &event;
    GetNextInputEvent(&event);
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

/* Function start: 0x40D6E1 */
void SwapSceneChunkSizeEndian(int *value)
{
    char *bytes;

    bytes = (char *)value;
    bytes[0] ^= bytes[3];
    bytes[3] ^= bytes[0];
    bytes[0] ^= bytes[3];
    bytes[1] ^= bytes[2];
    bytes[2] ^= bytes[1];
    bytes[1] ^= bytes[2];
}

/* Function start: 0x40D762 */
signed char AreCutsceneResourceNamesEqual(const char *left,
                                          const char *right)
{
    short length;

    if (left == right)
        return 1;
    length = DosStrlen(left);
    if (DosStrlen(right) == length) {
        while (length-- != 0) {
            if (toupper((int)right[length]) !=
                toupper((int)left[length]))
                return 0;
        }
        return 1;
    }
    return 0;
}

/* Function start: 0x40D812 */
unsigned int ReadNextSceneForm(unsigned char **cursor,
                               unsigned char **nextForm)
{
    unsigned int chunkSize;
    unsigned int formType;
    unsigned char *data;

    data = *cursor;
    formType = 0;
    if (*(unsigned int *)data == 0x4d524f46) {
        data += 4;
        chunkSize = *(unsigned int *)data;
        data += 4;
        SwapSceneChunkSizeEndian((int *)&chunkSize);
        *nextForm = data + ((chunkSize + 1) & ~1);
        formType = *(unsigned int *)data;
        data += 4;
        *cursor = IdentityHandle(data);
    }
    return formType;
}

/* Function start: 0x40D88E */
void SkipCutsceneChunk(unsigned char **cursor)
{
    unsigned int chunkSize;

    chunkSize = *(unsigned int *)*cursor;
    SwapSceneChunkSizeEndian((int *)&chunkSize);
    *cursor = IdentityHandle(
        *cursor + ((chunkSize + 1) & ~1U) + 4);
}

/* Function start: 0x40D8D7 */
short FindCutsceneResourceSymbolIndex(CutsceneObjectResourceList *list,
                                      short scriptHalf,
                                      const char *symbol)
{
    CutsceneObjectResourceHalf *half;
    char **symbols;
    short count;
    short index;

    if (list != 0) {
        if (scriptHalf == 0)
            half = (CutsceneObjectResourceHalf *)list;
        else if (scriptHalf == 1)
            half = (CutsceneObjectResourceHalf *)
                ((char *)list + sizeof(CutsceneObjectResourceHalf));
        symbols = half->symbols;
        count = half->symbolCount;
        for (index = 0; index < count; index++, symbols++) {
            if (AreCutsceneResourceNamesEqual(*symbols, symbol) != 0)
                return half->symbolIndices[index];
        }
        return FindCutsceneResourceSymbolIndex(list->next, scriptHalf,
                                               symbol);
    }
    return -1;
}

/* Function start: 0x40D9AD */
void *AllocateScenePointerTable(int count, short elementSize,
                                unsigned short flags,
                                const char *errorMessage)
{
    void *allocation;

    allocation = AllocateZeroedRecords(count, elementSize, flags);
    if (count * elementSize <= 4)
        flags = 0;
    if (allocation == 0)
        FatalCutsceneError("%s", errorMessage);
    return allocation;
}

/* Function start: 0x40DA0C */
short CreateCutsceneResourceInstance(unsigned int formType,
                                     CutsceneObjectResourceList *list,
                                     short index)
{
    SceneFlicObject *sprite;
    CutscenePlane *plane;
    CutsceneSequence *sequence;
    CutsceneScene *scene;
    short slot;

    slot = 0;
    if (formType == 0x54525053) {
        while (slot < 0x80) {
            if (g_apSceneObjects_00499c38[slot] == 0) {
                sprite = AllocateScenePointerTable(
                    1, sizeof(SceneFlicObject), 0,
                    "Cannot Allocate Sprite Data");
                g_apSceneObjects_00499c38[slot] = sprite;
                sprite->owner = g_nCutsceneResourceLevel_00499d98;
                sprite->field_3 = -1;
                sprite->active = 1;
                sprite->visible = 1;
                sprite->scale = 0x100;
                sprite->scriptStart = list->scripts[index];
                sprite->scriptCursor = sprite->scriptStart;
                if (sprite->scriptCursor != 0)
                    UpdateCutsceneSpriteObject(sprite);
                sprite->scriptStart = sprite->scriptCursor;
                break;
            }
            slot++;
        }
        if (slot == 0x80)
            FatalCutsceneError("Too few sprite slots");
    } else if (formType == 0x454e4c50) {
        while (slot < 0x40) {
            if (g_apCutscenePlanes_00499c3c[slot] == 0) {
                plane = AllocateScenePointerTable(
                    1, sizeof(CutscenePlane), 0,
                    "Cannot Allocate Plane Data");
                g_apCutscenePlanes_00499c3c[slot] = plane;
                plane->owner = g_nCutsceneResourceLevel_00499d98;
                plane->active = 1;
                plane->visible = 1;
                plane->scriptStart = list->scripts[index];
                plane->scriptCursor = plane->scriptStart;
                plane->spriteIndices = AllocateScenePointerTable(
                    0x10, 1, 0, "Cannot allocate SpriteNumList");
                if (plane->scriptCursor != 0)
                    UpdateCutscenePlaneObject(plane, 0);
                plane->scriptStart = plane->scriptCursor;
                break;
            }
            slot++;
        }
        if (slot == 0x40)
            FatalCutsceneError("Too few plane slots");
    } else if (formType == 0x55514553) {
        while (slot < 0x100) {
            if (g_apCutsceneSequences_00499c40[slot] == 0) {
                sequence = AllocateScenePointerTable(
                    1, sizeof(CutsceneSequence), 0,
                    "Cannot allocate Sequence Data");
                g_apCutsceneSequences_00499c40[slot] = sequence;
                sequence->owner = g_nCutsceneResourceLevel_00499d98;
                sequence->active = 1;
                sequence->scriptStart = list->scripts[index];
                sequence->scriptCursor = sequence->scriptStart;
                sequence->planeIndices = AllocateScenePointerTable(
                    0x10, 1, 0, "Cannot Allocate PlaneNumList");
                if (sequence->scriptCursor != 0)
                    ExecuteCutsceneSequence(sequence, 0, 0);
                sequence->scriptStart = sequence->scriptCursor;
                break;
            }
            slot++;
        }
        if (slot == 0x100)
            FatalCutsceneError("Too few sequence slots");
    } else if (formType == 0x454e4353) {
        while (slot < 0x20) {
            if (g_apCutsceneScenes_00499c44[slot] == 0) {
                scene = AllocateScenePointerTable(
                    1, sizeof(CutsceneScene), 0,
                    "Cannot Allocate Script Data");
                g_apCutsceneScenes_00499c44[slot] = scene;
                scene->owner = g_nCutsceneResourceLevel_00499d98;
                scene->active = 1;
                scene->scriptStart = list->scripts[index];
                scene->scriptCursor = scene->scriptStart;
                scene->sequenceIndices = AllocateScenePointerTable(
                    0x10, 1, 0, "Cannot Allocate SequenceNumList");
                if (scene->scriptCursor != 0)
                    ExecuteCutsceneScene(scene);
                scene->scriptStart = scene->scriptCursor;
                break;
            }
            slot++;
        }
        if (slot == 0x20)
            FatalCutsceneError("Too few script slots");
    }
    return slot;
}

/* Function start: 0x40DE5A */
signed char LinkCutsceneObjectResources(CutsceneObjectResourceList *list,
                                        short scriptHalf,
                                        unsigned int formType)
{
    CutsceneObjectResourceList *parent;
    CutsceneObjectResourceHalf *half;
    short count;
    short runtimeIndex;
    SceneFlicObject *sprite;
    CutscenePlane *plane;
    CutsceneSequence *sequence;
    CutsceneScene *scene;

    if (scriptHalf == 0)
        half = (CutsceneObjectResourceHalf *)list;
    else if (scriptHalf == 1)
        half = (CutsceneObjectResourceHalf *)
            ((char *)list + sizeof(CutsceneObjectResourceHalf));
    count = half->symbolCount;
    parent = list->next;
    if (parent == 0) {
        while (count-- != 0)
            half->symbolIndices[count] = CreateCutsceneResourceInstance(
                formType, list, count);
        half->count = (unsigned char)half->symbolCount;
        return 1;
    }
    half->count = (unsigned char)(half->inheritedCount + count);
    while (count-- != 0) {
        runtimeIndex = FindCutsceneResourceSymbolIndex(
            parent, scriptHalf, half->symbols[count]);
        if (runtimeIndex != -1) {
            half->symbolIndices[count] = runtimeIndex;
            switch (formType) {
            case 0x54525053:
                sprite = g_apSceneObjects_00499c38[runtimeIndex];
                if (sprite->scriptCursor == 0) {
                    sprite->scriptStart = list->scripts[count];
                    sprite->scriptCursor = sprite->scriptStart;
                    UpdateCutsceneSpriteObject(sprite);
                }
                break;
            case 0x454e4c50:
                plane = g_apCutscenePlanes_00499c3c[runtimeIndex];
                if (plane->scriptCursor == 0) {
                    plane->scriptStart = list->scripts[count];
                    plane->scriptCursor = plane->scriptStart;
                    UpdateCutscenePlaneObject(plane, 0);
                }
                break;
            case 0x55514553:
                sequence = g_apCutsceneSequences_00499c40[runtimeIndex];
                if (sequence->scriptCursor == 0) {
                    sequence->scriptStart = list->scripts[count];
                    sequence->scriptCursor = sequence->scriptStart;
                    ExecuteCutsceneSequence(sequence, 0, 0);
                }
                break;
            case 0x454e4353:
                scene = g_apCutsceneScenes_00499c44[runtimeIndex];
                if (scene->scriptCursor == 0) {
                    scene->scriptStart = list->scripts[count];
                    scene->scriptCursor = scene->scriptStart;
                    ExecuteCutsceneScene(scene);
                }
                break;
            }
        } else {
            half->symbolIndices[count] = CreateCutsceneResourceInstance(
                formType, list, count);
        }
    }
    return 1;
}

/* Function start: 0x40E130 */
signed char DecodeSceneResourceChunk(unsigned char **cursor,
                                     SceneResourceTable *resource)
{
    unsigned char *chunk;
    signed char decoded;
    int chunkType;

    chunk = *cursor;
    decoded = 0;
    chunkType = *(int *)chunk;
    switch (chunkType) {
    case 0x43525453:
        decoded = DecodeSceneStructChunk(&chunk, &resource);
        break;
    case 0x5453464f:
        decoded = DecodeSceneOffsetChunk(&chunk, &resource);
        break;
    case 0x424d5953:
        decoded = DecodeSceneSymbolChunk(&chunk, &resource);
        break;
    case 0x454c4946:
        decoded = DecodeSceneFileChunk(&chunk, &resource);
        break;
    case 0x4d524f46:
        break;
    }
    *cursor = IdentityHandle(chunk);
    return decoded;
}

/* Function start: 0x40E230 */
CutsceneResourceTable *PushCutsceneFileResource(
    CutsceneResourceTable **head)
{
    CutsceneResourceTable *resource;

    resource = AllocateScenePointerTable(
        1, sizeof(CutsceneResourceTable), 0,
        "Cannot allocate FileChunk");
    if (resource != 0) {
        resource->owner = g_nCutsceneResourceLevel_00499d98;
        resource->next = *head;
        *head = resource;
    }
    return resource;
}

/* Function start: 0x40E285 */
void DecodeCutsceneFileResource(CutsceneResourceTable *resource,
                                unsigned char **cursor)
{
    SceneResourceTable decodedResource;

    DecodeSceneResourceChunk(cursor, &decodedResource);
    resource->count = decodedResource.count;
    resource->owner = g_nCutsceneResourceLevel_00499d98;
    resource->filenameIndices = decodedResource.data;
    resource->sectionIndices =
        resource->filenameIndices + resource->count;
    resource->packedFilenames = (char *)(
        resource->sectionIndices + resource->count);
    resource->loadedPackets = AllocateScenePointerTable(
        resource->count, HOST_POINTER_SIZE, 0,
        "Cannot Allocate InMemory List");
    if (resource->loadedPackets == 0)
        ReleasePacketSlot((void **)&resource);
}

/* Function start: 0x40E31F */
unsigned int ParseCutsceneContainer(void *scenePacket)
{
    unsigned char *cursor;
    unsigned char *containerEnd;
    unsigned char *formEnd;
    int formType;
    SceneResourceTable decodedResource;
    CutsceneTextResource *textResource;
    short globalCount;

    cursor = IdentityHandle(scenePacket);
    InitializeCutsceneRuntimeResources();
    if (ReadNextSceneForm(&cursor, &containerEnd) != 0x50435343)
        return 0;
    PushCutsceneObjectResource(&g_pCutsceneSpriteResources_0049288c);
    PushCutsceneObjectResource(&g_pCutscenePlaneResources_00492890);
    PushCutsceneObjectResource(&g_pCutsceneSequenceResources_00492894);
    PushCutsceneObjectResource(&g_pCutsceneSceneResources_00492898);
    PushCutsceneFileResource(&g_pCutsceneFxResources_004928b8);
    PushCutsceneFileResource(&g_pCutsceneMusicResources_004928bc);
    PushCutsceneFileResource(&g_pCutsceneMouseResources_004928ac);
    PushCutsceneFileResource(&g_pCutsceneShapeResources_004928a8);
    PushCutsceneFileResource(&g_pCutsceneFilmResources_004928b4);
    PushCutsceneFileResource(&g_pCutsceneFontResources_004928a0);
    PushCutsceneFileResource(&g_pCutsceneSpeechResources_004928a4);
    PushCutsceneFileResource(&g_pCutscenePaletteResources_004928b0);
    while (cursor < containerEnd) {
        formType = ReadNextSceneForm(&cursor, &formEnd);
        switch (formType) {
        case 0x54525053:
            DecodeCutsceneObjectResource(
                g_pCutsceneSpriteResources_0049288c,
                &cursor, formEnd, formType);
            break;
        case 0x454e4c50:
            DecodeCutsceneObjectResource(
                g_pCutscenePlaneResources_00492890,
                &cursor, formEnd, formType);
            break;
        case 0x55514553:
            DecodeCutsceneObjectResource(
                g_pCutsceneSequenceResources_00492894,
                &cursor, formEnd, formType);
            break;
        case 0x454e4353:
            DecodeCutsceneObjectResource(
                g_pCutsceneSceneResources_00492898,
                &cursor, formEnd, formType);
            break;
        case 0x20205846:
            DecodeCutsceneFileResource(
                g_pCutsceneFxResources_004928b8, &cursor);
            break;
        case 0x2053554d:
            DecodeCutsceneFileResource(
                g_pCutsceneMusicResources_004928bc, &cursor);
            break;
        case 0x204c4150:
            DecodeCutsceneFileResource(
                g_pCutscenePaletteResources_004928b0, &cursor);
            break;
        case 0x544e4f46:
            DecodeCutsceneFileResource(
                g_pCutsceneFontResources_004928a0, &cursor);
            break;
        case 0x48435053:
            DecodeCutsceneFileResource(
                g_pCutsceneSpeechResources_004928a4, &cursor);
            break;
        case 0x4d4c4946:
            DecodeCutsceneFileResource(
                g_pCutsceneFilmResources_004928b4, &cursor);
            break;
        case 0x50414853:
            DecodeCutsceneFileResource(
                g_pCutsceneShapeResources_004928a8, &cursor);
            break;
        case 0x53554f4d:
            DecodeCutsceneFileResource(
                g_pCutsceneMouseResources_004928ac, &cursor);
            break;
        case 0x424f4c47:
            DecodeSceneResourceChunk(&cursor, &decodedResource);
            globalCount = *(short *)(
                ((void **)decodedResource.data)[0]);
            if (g_pCampaignGlobals_00499c94 == 0) {
                g_pCampaignGlobals_00499c94 = AllocateScenePointerTable(
                    globalCount, 2, 0, "Cannot Reallocate Globals");
            }
            ReleasePacketSlot(&decodedResource.data);
            break;
        case 0x54584554:
            DecodeSceneResourceChunk(&cursor, &decodedResource);
            textResource = AllocateScenePointerTable(
                1, sizeof(CutsceneTextResource), 0,
                "Cannot Allocate Text Ptrs");
            textResource->next = g_pCutsceneTextResources_0049289c;
            textResource->owner = g_nCutsceneResourceLevel_00499d98;
            textResource->entries = decodedResource.data;
            g_pCutsceneTextResources_0049289c = textResource;
            break;
        default:
            cursor = formEnd;
            break;
        }
    }
    return 1;
}

/* Function start: 0x40E726 */
signed char DecodeCutsceneObjectResource(
    CutsceneObjectResourceList *resource, unsigned char **cursor,
    unsigned char *end, unsigned int formType)
{
    unsigned char *chunkCursor;
    unsigned char *formEnd;
    unsigned int nestedFormType;
    SceneResourceTable decodedResource;

    chunkCursor = *cursor;
    while (chunkCursor < end) {
        nestedFormType = ReadNextSceneForm(&chunkCursor, &formEnd);
        if (nestedFormType == 0x41544144) {
            while (chunkCursor < formEnd) {
                while (DecodeSceneResourceChunk(
                           &chunkCursor, &decodedResource) != 0) {
                    if (decodedResource.type == 0) {
                        resource->dataEntries = decodedResource.data;
                        resource->localDataCount = decodedResource.count;
                        resource->dataRuntimeIndices =
                            AllocateScenePointerTable(
                                decodedResource.count, 2, 0,
                                "Cannot Allocate Index Array");
                    } else if (decodedResource.type == 2) {
                        resource->dataSymbols = decodedResource.data;
                        resource->dataSymbolCount = decodedResource.count;
                        resource->dataSymbolIndices =
                            AllocateScenePointerTable(
                                decodedResource.count, 2, 0,
                                "Cannot Allocate Symbol Index Array");
                        if (LinkCutsceneObjectResources(
                                resource, 0, formType) == 0)
                            return 0;
                    }
                }
            }
        } else if (nestedFormType == 0x50524353) {
            while (chunkCursor < formEnd) {
                while (DecodeSceneResourceChunk(
                           &chunkCursor, &decodedResource) != 0) {
                    if (decodedResource.type == 1) {
                        resource->scripts = decodedResource.data;
                        resource->localScriptCount = decodedResource.count;
                        resource->scriptRuntimeIndices =
                            AllocateScenePointerTable(
                                decodedResource.count, 2, 0,
                                "Cannot Allocate Index Array");
                    } else if (decodedResource.type == 2) {
                        resource->scriptSymbols = decodedResource.data;
                        resource->scriptSymbolCount = decodedResource.count;
                        resource->scriptSymbolIndices =
                            AllocateScenePointerTable(
                                decodedResource.count, 2, 0,
                                "Cannot Allocate Symbol Index Array");
                        if (LinkCutsceneObjectResources(
                                resource, 1, formType) == 0)
                            return 0;
                    }
                }
            }
        } else {
            SkipCutsceneChunk(&chunkCursor);
        }
    }
    *cursor = chunkCursor;
    return 1;
}

/* Function start: 0x40E985 */
unsigned int ReleaseCutsceneObjectResourceData(
    CutsceneObjectResourceList *resource)
{
    ReleasePacketSlot((void **)&resource->dataEntries);
    ReleasePacketSlot((void **)&resource->dataRuntimeIndices);
    ReleasePacketSlot((void **)&resource->dataSymbols);
    ReleasePacketSlot((void **)&resource->dataSymbolIndices);
    ReleasePacketSlot((void **)&resource->scripts);
    ReleasePacketSlot((void **)&resource->scriptRuntimeIndices);
    ReleasePacketSlot((void **)&resource->scriptSymbols);
    ReleasePacketSlot((void **)&resource->scriptSymbolIndices);
    return 1;
}

/* Function start: 0x40EA0F */
CutsceneObjectResourceList *PushCutsceneObjectResource(
    CutsceneObjectResourceList **head)
{
    CutsceneObjectResourceList *resource;

    resource = AllocateScenePointerTable(
        1, sizeof(CutsceneObjectResourceList), 0,
        "Cannot Allocate Chunk Descriptor");
    if (resource != 0) {
        resource->next = *head;
        if (*head != 0) {
            resource->owner = g_nCutsceneResourceLevel_00499d98;
            resource->dataCount = (*head)->dataCount;
            resource->inheritedDataCount = resource->dataCount;
            resource->scriptCount = (*head)->scriptCount;
            resource->inheritedScriptCount = resource->scriptCount;
        }
        *head = resource;
    }
    return resource;
}

/* Function start: 0x40EAA4 */
void ReleaseCutsceneObjectResource(short owner,
                                   CutsceneObjectResourceList **head,
                                   unsigned int formType)
{
    CutsceneObjectResourceList *resource;
    CutscenePlane *plane;
    CutsceneSequence *sequence;
    CutsceneScene *scene;
    short index;

    resource = *head;
    if (resource->owner == owner) {
        ReleaseCutsceneObjectResourceData(resource);
        switch (formType) {
        case 0x54525053:
            for (index = 0; index < 0x80; index++) {
                if (g_apSceneObjects_00499c38[index] != 0 &&
                    g_apSceneObjects_00499c38[index]->owner == owner) {
                    ReleasePacketSlot(
                        (void **)&g_apSceneObjects_00499c38[index]->locals);
                    ReleasePacketSlot(
                        (void **)&g_apSceneObjects_00499c38[index]);
                }
            }
            break;
        case 0x454e4c50:
            for (index = 0; index < 0x40; index++) {
                if (g_apCutscenePlanes_00499c3c[index] != 0 &&
                    g_apCutscenePlanes_00499c3c[index]->owner == owner) {
                    plane = g_apCutscenePlanes_00499c3c[index];
                    ReleasePacketSlot((void **)&plane->locals);
                    ReleasePacketSlot((void **)&plane->spriteIndices);
                    ReleasePacketSlot(
                        (void **)&g_apCutscenePlanes_00499c3c[index]);
                }
            }
            break;
        case 0x55514553:
            for (index = 0; index < 0x100; index++) {
                if (g_apCutsceneSequences_00499c40[index] != 0 &&
                    g_apCutsceneSequences_00499c40[index]->owner == owner) {
                    sequence = g_apCutsceneSequences_00499c40[index];
                    ReleasePacketSlot((void **)&sequence->locals);
                    ReleasePacketSlot((void **)&sequence->planeIndices);
                    ReleasePacketSlot(
                        (void **)&g_apCutsceneSequences_00499c40[index]);
                }
            }
            break;
        case 0x454e4353:
            for (index = 0; index < 0x20; index++) {
                if (g_apCutsceneScenes_00499c44[index] != 0 &&
                    g_apCutsceneScenes_00499c44[index]->owner == owner) {
                    scene = g_apCutsceneScenes_00499c44[index];
                    ReleasePacketSlot((void **)&scene->locals);
                    ReleasePacketSlot((void **)&scene->sequenceIndices);
                    ReleasePacketSlot(
                        (void **)&g_apCutsceneScenes_00499c44[index]);
                }
            }
            break;
        }
        *head = resource->next;
        ReleasePacketHandle(resource);
    }
}

/* Function start: 0x40ED9F */
void ReleaseCutsceneFileResourceExceptPacket(
    short owner, CutsceneResourceTable **head, void *retainedPacket)
{
    CutsceneResourceTable *resource;
    short index;

    resource = *head;
    if (resource->owner == owner) {
        index = resource->count;
        while (index-- != 0) {
            if (resource->loadedPackets[index] != retainedPacket) {
                ReleasePacketSlot(
                    &resource->loadedPackets[index]);
            }
        }
        ReleasePacketSlot((void **)&resource->loadedPackets);
        *head = resource->next;
        ReleasePacketHandle(resource);
    }
}

/* Function start: 0x40EE41 */
void ReleaseCutsceneFileResource(short owner,
                                 CutsceneResourceTable **head)
{
    CutsceneResourceTable *resource;
    short index;

    resource = *head;
    if (resource->owner == owner) {
        index = resource->count;
        while (index != 0) {
            ReleaseLoadedCutsceneResource(resource, (short)(index - 1));
            index--;
        }
        ReleasePacketSlot((void **)&resource->loadedPackets);
        *head = resource->next;
        ReleasePacketHandle(resource);
    }
}

/* Function start: 0x40EEC7 */
void ReleaseCutsceneTextResource(short owner,
                                 CutsceneTextResource **head)
{
    CutsceneTextResource *resource;

    resource = *head;
    if (resource->owner == owner) {
        ReleasePacketSlot((void **)&resource->entries);
        *head = resource->next;
        ReleasePacketHandle(resource);
    }
}

/* Function start: 0x40EF15 */
void ReleaseCutsceneResourceLevel(short owner)
{
    ReleaseCutsceneObjectResource(
        owner, &g_pCutsceneSpriteResources_0049288c, 0x54525053);
    ReleaseCutsceneObjectResource(
        owner, &g_pCutscenePlaneResources_00492890, 0x454e4c50);
    ReleaseCutsceneObjectResource(
        owner, &g_pCutsceneSequenceResources_00492894, 0x55514553);
    ReleaseCutsceneObjectResource(
        owner, &g_pCutsceneSceneResources_00492898, 0x454e4353);
    if (owner != 0) {
        ReleaseCutsceneFileResource(
            owner, &g_pCutsceneMusicResources_004928bc);
    }
    ReleaseCutsceneFileResource(
        owner, &g_pCutsceneFxResources_004928b8);
    ReleaseCutsceneFileResource(
        owner, &g_pCutsceneFontResources_004928a0);
    ReleaseCutsceneFileResource(
        owner, &g_pCutsceneSpeechResources_004928a4);
    ReleaseCutsceneFileResource(
        owner, &g_pCutsceneShapeResources_004928a8);
    ReleaseCutsceneFileResource(
        owner, &g_pCutsceneMouseResources_004928ac);
    ReleaseCutsceneFileResource(
        owner, &g_pCutscenePaletteResources_004928b0);
    ReleaseCutsceneFileResource(
        owner, &g_pCutsceneFilmResources_004928b4);
    ReleaseCutsceneTextResource(
        owner, &g_pCutsceneTextResources_0049289c);
}

/* Function start: 0x40F0D2 */
void ReportPacketLoadError(void *packet, char *fileName,
                           short retry, short section,
                           const char *sourceTag)
{
    short error;
    const char *operation;
    unsigned int packetSize;

    if ((packet == 0 ||
         (g_nPacketError_0049ca90 != 0 &&
          g_nPacketError_0049ca90 != 8)) &&
        (packet != 0 || g_nPacketError_0049ca90 != 8)) {
        error = g_nPacketError_0049ca90;
        if (section != -1)
            packetSize = GetNamedPacketSize(fileName, section);
        LogMemoryUsage();
        g_nPacketError_0049ca90 = error;
        if (g_nOriginDevUnlock_0049d774 == 0) {
            if (packet == 0 || section == -1)
                operation = g_pszPacketAllocateOperation_00492b10;
            else
                operation = g_pszPacketReadOperation_00492b0c;
            sprintf(g_szDefaultTextBuffer_005d2b80,
                    g_pszPacketLoadErrorFormat_00492b14,
                    operation, fileName, (int)section, (int)error,
                    packetSize,
                    GetLargestFreeMemoryBlockByType(retry), (int)retry,
                    sourceTag);
            FatalErrorAndExit(g_szDefaultTextBuffer_005d2b80);
        } else {
            printf("Error: %Fs, packet %d\n", fileName, (int)section);
            printf("packet size: %ld\n", packetSize);
            printf("largest block: %ld  FREE: %ld\n",
                   GetLargestFreeMemoryBlockByType(retry),
                   GetAvailableFarMemoryByType(retry));
            printf("MAIN: %ld   EMS: %ld\n",
                   GetAvailableMainMemory(), GetAvailableFarMemory());
            exit(1);
        }
    }
}

#ifndef SDL_PORT
#pragma function(strlen, strcat)
#endif

/* Function start: 0x40F266 */
void AppendPacketLoadDebugLog(char *fileName, short section,
                              void *packet)
{
    short file;
    char truncatedFileName[36];
    char logLine[80];

    file = (short)_open("logfile.txt", 0x4109, 0x80);
    strncpy(truncatedFileName, fileName, strlen(truncatedFileName));
    sprintf(logLine, "\n%s, #%d, %Fp", truncatedFileName,
            (int)section, packet);
    if (g_nShowMemoryStatus_0049d784 != 0) {
        _ltoa((long)GetNamedPacketSize(fileName, section),
              truncatedFileName, 10);
        strcat(logLine, ",Size:");
        strcat(logLine, truncatedFileName);
        _ltoa((long)GetLargestMainMemoryBlock(), truncatedFileName, 10);
        strcat(logLine, ",LB:");
        strcat(logLine, truncatedFileName);
        _ltoa((long)GetAvailableMainMemory(), truncatedFileName, 10);
        strcat(logLine, ",Free:");
        strcat(logLine, truncatedFileName);
    }
    _write((int)file, logLine, strlen(logLine));
    _close((int)file);
}

#ifndef SDL_PORT
#pragma intrinsic(strlen, strcat)
#endif

/* Function start: 0x40F3AE */
void RewritePacketFilenameForInstalledData(char *fileName)
{
    char *extension;
    char *extensionStart;

    if (g_bRewritePacketExtensions_0049cb48 == 1) {
        extension = DosStrchr(fileName, '.');
        extensionStart = extension;
        extension++;
        if (extensionStart != 0 && toupper(*extension) == 'V')
            *extension = 'e';
    }
}

/* Function start: 0x40F40E */
void *LoadPacketIntoBuffer(char *fileName, short section,
                           void *destination, short registerHandle)
{
    void *packet;

    if (g_bRewritePacketExtensions_0049cb48 == 1)
        RewritePacketFilenameForInstalledData(fileName);
    packet = LoadNamedPacket(fileName, section, destination, 0, 0,
                             registerHandle);
    ReportPacketLoadError(destination, fileName, 0, section, "RP");
    if (g_nOriginDevUnlock_0049d774 != 0)
        AppendPacketLoadDebugLog(fileName, section, packet);
    return packet;
}

/* Function start: 0x40F49D */
void *LoadPacketAllocated(char *fileName, short section)
{
    short retries;
    char errorText[40];
    void *packet;
    int packetSize;

    retries = 5;
    if (g_bRewritePacketExtensions_0049cb48 == 1)
        RewritePacketFilenameForInstalledData(fileName);
    packetSize = (int)GetNamedPacketSize(fileName, section);
    packet = AllocateTaggedMemory((int)(short)packetSize, 0x40);
    if (packet != 0) {
        do {
            LoadNamedPacket(fileName, section, packet, 0, 0, 1);
            retries--;
            if (retries <= 0 || g_nPacketError_0049ca90 == 0)
                break;
        } while (g_nPacketError_0049ca90 != 8);
    } else {
        sprintf(errorText, "Error in LPN:  %Fs, #%d\n",
                fileName, (int)section);
        exit_squadron(errorText);
    }
    ReportPacketLoadError(packet, fileName, 0, section, "LPN");
    if (g_nOriginDevUnlock_0049d774 != 0)
        AppendPacketLoadDebugLog(fileName, section, packet);
    return packet;
}

/* Function start: 0x40F5B6 */
void *FetchDiskPacketRetrying(char *fileName, short section,
                              unsigned short flags)
{
    void *packet;
    unsigned int availableMemory;
    short retries;
    unsigned int largestBlock;

    packet = 0;
    retries = 5;
    if (g_bRewritePacketExtensions_0049cb48 == 1)
        RewritePacketFilenameForInstalledData(fileName);
    do {
        FreePacketAndClear(&packet, flags);
        largestBlock = GetLargestMainMemoryBlock();
        availableMemory = GetAvailableMainMemory();
        packet = LoadNamedPacket(fileName, section, 0, flags, 0, 1);
        largestBlock = GetLargestMainMemoryBlock();
        availableMemory = GetAvailableMainMemory();
        retries--;
        if (retries < 1 || g_nPacketError_0049ca90 == 0)
            break;
    } while (g_nPacketError_0049ca90 != 8);

    if (packet == 0 && g_stViewBuffer_005d2b00.pixels != 0) {
        free_viewport(&g_stViewBuffer_005d2b00);
        do {
            FreePacketAndClear(&packet, flags);
            largestBlock = GetLargestMainMemoryBlock();
            availableMemory = GetAvailableMainMemory();
            packet = LoadNamedPacket(fileName, section, 0, flags, 0, 1);
            largestBlock = GetLargestMainMemoryBlock();
            availableMemory = GetAvailableMainMemory();
            retries--;
            if (retries < 1 || g_nPacketError_0049ca90 == 0)
                break;
        } while (g_nPacketError_0049ca90 != 8);
        if (AllocateViewport(&g_stViewBuffer_005d2b00,
                             g_cPrimaryViewBufferColour_0049cb88,
                             0x20) == 0) {
            ReportPacketLoadError(0, fileName, flags, section, "LP2");
        }
    }
    if (packet == 0 &&
        g_stSecondaryViewBuffer_005d2c90.pixels != 0) {
        free_viewport(&g_stSecondaryViewBuffer_005d2c90);
        do {
            FreePacketAndClear(&packet, flags);
            packet = LoadNamedPacket(fileName, section, 0, flags, 0, 1);
            retries--;
            if (retries < 1 || g_nPacketError_0049ca90 == 0)
                break;
        } while (g_nPacketError_0049ca90 != 8);
        if (AllocateViewport(&g_stSecondaryViewBuffer_005d2c90,
                             g_cSecondaryViewBufferColour_0049cb4c,
                             0) == 0) {
            ReportPacketLoadError(0, fileName, flags, section, "LP3");
        }
    }
    if (packet == 0 && (flags & 4) == 0 &&
        g_nPacketError_0049ca90 != 0 && g_nPacketError_0049ca90 != 8) {
        ReportPacketLoadError(0, fileName, flags, section, "LP4");
    }
    if (g_nOriginDevUnlock_0049d774 != 0) {
        AppendPacketLoadDebugLog(fileName, section, packet);
    }
    return packet;
}

/* Function start: 0x40F882 */
void InitializeTextContextFromFont(TextContext *context, short fontIndex,
                                   unsigned char colour,
                                   signed char background)
{
    if (g_apTextFonts_005d2200[fontIndex] == 0) {
        if (fontIndex == 1) {
            g_apTextFonts_005d2200[fontIndex] =
                FetchDiskPacketRetrying("fonts.fnt", fontIndex, 0x10);
        } else {
            g_apTextFonts_005d2200[fontIndex] =
                FetchDiskPacketRetrying("fonts.fnt", fontIndex, 0);
        }
    }
    context->font = g_apTextFonts_005d2200[fontIndex];
    context->colour = colour;
    context->backgroundColour = (unsigned char)background;
    SetTextContext(context);
}

/* Function start: 0x40F91C */
void ReleaseTextFont(short fontIndex)
{
    if (fontIndex == 1)
        return;
    if (g_apTextFonts_005d2200[fontIndex] != 0) {
        ReleasePacketHandle(g_apTextFonts_005d2200[fontIndex]);
        g_apTextFonts_005d2200[fontIndex] = 0;
    }
}

/* Function start: 0x40F96E */
void DrawTextAt(TextContext *context, short x, short y,
                const char *text, unsigned char alignment)
{
    char *savedText;
    unsigned char savedAlignment;

    savedText = context->text;
    savedAlignment = context->alignment;
    if (x < 0 || y < 0)
        return;
    SetTextContext(context);
    SetTextCursor(x, y);
    context->text = (char *)text;
    context->alignment = alignment;
    DrawTextString(text);
    context->text = savedText;
    context->alignment = savedAlignment;
}

/* Function start: 0x40F9F7 */
void FlushPendingInputPresses(void)
{
    PumpWindowMessages(0);
    while (TakeInputPressCount() != 0)
        ServiceInputDevices(15);
}

/* Function start: 0x40FA2C */
short WaitForInputKey(void)
{
    void (*savedPump)(void);
    short key;
    InputEventState *event;

    key = 0;
    savedPump = g_pfnInputPump_005c840c;
    ConfigureInputPump(1, PollJoystickButtonEvents);
    ServiceInputDevices(15);
    event = FindQueuedInputEvent(1);
    if (event != 0) {
        key = (short)(event->value + 1);
    } else {
        event = FindQueuedInputEvent(4);
        if (event != 0)
            key = (short)event->status;
    }
    if (key != 0)
        FlushPendingInputPresses();
    FlushInputEvents();
    ConfigureInputPump(1, savedPump);
    return key;
}

/* Function start: 0x40FADE */
unsigned short WaitForAnyInputPress(void)
{
    signed char acknowledged;
    unsigned char key;
    int savedKeyboardMouseEnabled;
    InputEventState event;
    int eventType;

    acknowledged = 0;
    key = 0x0d;
    savedKeyboardMouseEnabled = g_bKeyboardMouseEnabled_0049be68;
    g_bKeyboardMouseEnabled_0049be68 = 0;
    while (acknowledged == 0) {
        eventType = PollInputEvent(&event);
        switch (eventType) {
        case 4:
        case 6:
            key = (unsigned char)event.value;
        case 1:
            ReleaseInputEventQueue();
            acknowledged++;
            break;
        }
    }
    g_bKeyboardMouseEnabled_0049be68 = savedKeyboardMouseEnabled;
    return key;
}

/* Function start: 0x410020 */
short CountShipProjectiles(short ship)
{
    short count;
    short obj;

    count = 0;
    for (obj = 0; obj < SPACE_OBJECT_COUNT; obj++) {
        if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_PROJECTILE &&
            g_acObjectOwner_00495208[obj] == ship)
            count++;
    }
    return count;
}

/* Function start: 0x41008E */
short ShipHasCloakingDevice(short ship)
{
    short weapon;
    unsigned char *loadout;

    loadout = g_aShipWeapons_004956b0[ship];
    weapon = 0;
    for (; (short)(signed char)loadout[0] > weapon; weapon++) {
        if (((ShipWeaponSlot *)(loadout + 1))[weapon].type == 0x3c)
            return 1;
    }
    return 0;
}

/* Function start: 0x410102 */
short CanShipWeaponDamageTarget(short ship, short target)
{
    if (g_aeObjectClass_00495328[ship] == OBJECT_CLASS_SHIP) {
        if (g_aasShipShield_00495518[target][0] >= 1000 ||
            g_aasShipShield_00495518[target][1] >= 1000)
            return 0;
    }
    return 1;
}

/* Function start: 0x410192 */
short HasShipCockpitGunDisplay(short ship)
{
    short weapon;
    short weaponCount;

    weaponCount = (short)(signed char)g_aShipWeapons_004956b0[ship][0];
    weapon = 0;
    for (; weaponCount > weapon; weapon++) {
        if (((ShipWeaponSlot *)&g_aShipWeapons_004956b0[ship][1])[
                weapon].type == 0x0b)
            return 1;
    }
    return 0;
}

/* Function start: 0x410215 */
short ShipHasTorpedo(short ship)
{
    unsigned char *loadout;
    short weapon;

    loadout = g_aShipWeapons_004956b0[ship];
    weapon = 0;
    for (; (short)(signed char)loadout[0] > weapon; weapon++) {
        if (((ShipWeaponSlot *)(loadout + 1))[weapon].type == 0x13)
            return 1;
    }
    return 0;
}

/* Function start: 0x4103A6 */
short initialize_object(short obj, short type, short owner,
                        short matchLoadedType)
{
    if (obj != -1) {
        set_objects_data(obj, type, owner, matchLoadedType);
        zero_vector(&g_aShipPosition_00494550[obj]);
        zero_vector(&g_aShipVelocity_00494898[obj]);
    }
    return obj;
}

/* Function start: 0x41040D */
short borrow_dust(void)
{
    short i;
    signed char found;

    found = 0;
    for (i = 10; i <= 66; i++) {
        if (g_aeObjectClass_00495328[i] == OBJECT_CLASS_DUST) {
            found++;
            break;
        }
    }
    if (!found) {
        for (i = 10; i <= 66; i++) {
            if (g_aeObjectClass_00495328[i] == OBJECT_CLASS_DEBRIS) {
                found++;
                break;
            }
        }
    }
    if (!found) {
        for (i = 10; i <= 66; i++) {
            if (g_aeObjectClass_00495328[i] == OBJECT_CLASS_PROJECTILE &&
                g_acObjectOwner_00495208[i] != 0 &&
                g_asObjectScreenX_00493598[i] == (short)0x8001) {
                found++;
                break;
            }
        }
    }
    if (!found) {
        for (i = 10; i <= 66; i++) {
            if ((g_aeObjectClass_00495328[i] == OBJECT_CLASS_ASTEROID ||
                 g_aeObjectClass_00495328[i] == OBJECT_CLASS_MINE) &&
                g_asObjectScreenX_00493598[i] == (short)0x8001) {
                found++;
                break;
            }
        }
    }
    if (!found) {
        i = -1;
    } else {
        g_asObjectScreenX_00493598[i] = (short)0x8001;
    }
    return i;

}

/* Function start: 0x4105BF */
short new_object(short type, short owner)
{
    short obj;

    obj = find_vacant_3d_object();
    if (obj == -1 && owner == 0)
        obj = borrow_dust();
    return initialize_object(obj, type, owner, 0);
}

/* Function start: 0x41062D */
short initialize_ship(short type, short owner,
                      short matchLoadedType)
{
    short obj = get_ship_slot();

    if (obj != -1) {
        initialize_object(obj, type, owner, matchLoadedType);
        g_asShipSide_004955d0[obj] = SIDE_NEUTRAL;
    }
    return obj;
}

/* Function start: 0x410680 */
short any_selected(unsigned char *loadout, short objectClass)
{
    short selected;
    short weapon;

    selected = 0;
    weapon = 0;
    for (; (short)(signed char)loadout[0] > weapon && selected == 0;
         weapon++) {
        if (g_aObjectTypeData_00496d30[
                ((ShipWeaponSlot *)(loadout + 1))[weapon].weaponType]
                .objectClass == objectClass &&
            ((ShipWeaponSlot *)(loadout + 1))[weapon].disabled == 0)
            selected = 1;
    }
    return selected;
}

/* Function start: 0x410715 */
void remove_weapon(short obj, short weapon)
{
    short type;
    short objectClass;
    short weaponType;
    unsigned char *loadout;
    short slot;

    loadout = g_aShipWeapons_004956b0[obj];
    weaponType = ((ShipWeaponSlot *)(g_aShipWeapons_004956b0[obj] + 1))[
        weapon].weaponType;
    type = ((ShipWeaponSlot *)(g_aShipWeapons_004956b0[obj] + 1))[
        weapon].type;
    objectClass = g_aObjectTypeData_00496d30[weaponType].objectClass;
    if ((signed char)loadout[0] - 1 > weapon) {
        for (slot = weapon; (signed char)loadout[0] - 1 > slot; slot++) {
            ((ShipWeaponSlot *)(loadout + 1))[slot].type =
                ((ShipWeaponSlot *)(loadout + 1))[slot + 1].type;
            ((ShipWeaponSlot *)(loadout + 1))[slot].mount =
                ((ShipWeaponSlot *)(loadout + 1))[slot + 1].mount;
            ((ShipWeaponSlot *)(loadout + 1))[slot].disabled =
                ((ShipWeaponSlot *)(loadout + 1))[slot + 1].disabled;
            ((ShipWeaponSlot *)(loadout + 1))[slot].weaponType =
                ((ShipWeaponSlot *)(loadout + 1))[slot + 1].weaponType;
            if (obj == 0) {
                g_aWeaponDisplayPositions_005d1de0[slot] =
                    g_aWeaponDisplayPositions_005d1de0[slot + 1];
            }
        }
        if (loadout[0] < 0x10)
            ((ShipWeaponSlot *)(loadout + 1))[loadout[0]].disabled = 1;
    }
    loadout[0]--;
    if (obj == 0) {
        if (any_selected(loadout, objectClass) == 0) {
            if (objectClass == OBJECT_CLASS_PROJECTILE) {
                select_new_gun();
            } else {
                g_nSelectedReleaseWeaponIndex_004934e0 = -1;
                select_new_release_weapon(type);
            }
        }
        if (get_mode(0) == 1)
            InvalidateVduMode(0);
    }
}

/* Function start: 0x41090F */
void InitializeShipWeaponTypeIndices(short obj)
{
    short weaponCount;
    short weapon;

    weaponCount = (short)(signed char)
        g_aObjectTypeData_00496d30[
            g_acObjectType_00493980[obj]].weaponLoadout[0];
    for (weapon = 0; weapon < weaponCount; weapon++) {
        g_aShipWeapons_004956b0[obj][
            weapon * sizeof(ShipWeaponSlot) + 1 +
            offsetof(ShipWeaponSlot, weaponType)] =
            g_aShipWeapons_004956b0[obj][
                weapon * sizeof(ShipWeaponSlot) + 1 +
                offsetof(ShipWeaponSlot, type)];
    }
}

/* Function start: 0x410999 */
void set_objects_data(short obj, short type, short owner,
                      short matchResourceType)
{
    short frame;
    ObjectTypeData *shapeSource;
    unsigned char *animation;
    short slot;
    ObjectTypeData *typeData;

    g_asObjectCreationFrame_005d3900[obj] = g_nSpaceFrame_00493134;
    g_abProjectileCollisionBonus_004960a8[obj] = 0;
    if (matchResourceType != 0) {
        for (slot = 0; slot < 5; slot++) {
            if (g_aObjectTypeData_00496d30[slot].resourceType == type)
                break;
        }
        typeData = &g_aObjectTypeData_00496d30[slot];
        g_acObjectType_00493980[obj] = (signed char)slot;
        g_apObjectExhaustShape_004953b8[obj] = typeData->animation;
        g_apObjectShape_00493868[obj] = typeData->shapeSet;
    } else {
        if (type == OBJECT_DATA_SPACE_DUST) {
            g_asObjectType_00495298[obj] = OBJECT_DATA_SPACE_DUST;
            g_aeObjectClass_00495328[obj] = OBJECT_CLASS_DUST;
            return;
        }
        if (g_aObjectTypeData_00496d30[type].shapeSet == 0) {
            switch (type) {
            case OBJECT_DATA_EXPLOSION_MEDIUM:
                type = OBJECT_DATA_EXPLOSION_SMALL;
                break;
            case OBJECT_DATA_EXPLOSION_LARGE:
                type = OBJECT_DATA_EXPLOSION_SMALL;
                break;
            case OBJECT_DATA_SHIP_WING:
                type = OBJECT_DATA_PIPE;
                break;
            case OBJECT_DATA_METAL_SHEET:
                type = OBJECT_DATA_GIRDER_CHUNK;
                break;
            case OBJECT_DATA_ASTEROID2:
                type = OBJECT_DATA_ASTEROID1;
                break;
            case OBJECT_DATA_ASTEROID4:
                type = OBJECT_DATA_ASTEROID3;
                break;
            case OBJECT_DATA_ASTEROID6:
                type = OBJECT_DATA_ASTEROID5;
                break;
            }
        }
        if (type == 0x3d)
            g_acObjectType_00493980[obj] = OBJECT_DATA_SPACE_DUST;
        else
            g_acObjectType_00493980[obj] = (signed char)type;
        if (type == 0x3e) {
            g_acObjectType_00493980[obj] =
                (signed char)g_nObjectType62Index_00492d64;
        }
        if (type == 0x3f) {
            g_acObjectType_00493980[obj] =
                (signed char)g_nObjectType63Index_00492d68;
        }
        typeData =
            &g_aObjectTypeData_00496d30[g_acObjectType_00493980[obj]];
        shapeSource = typeData;
        if (type == 0xe)
            shapeSource = &g_aObjectTypeData_00496d30[10];
        if (type == 0xd)
            shapeSource = &g_aObjectTypeData_00496d30[8];
        g_asObjectType_00495298[obj] = type;
        if (type == OBJECT_DATA_CHAFF_POD &&
            g_aObjectTypeData_00496d30[20].shapeSet == 0) {
            g_apObjectShape_00493868[obj] =
                g_aObjectTypeData_00496d30[9].shapeSet;
        }
        if (type == OBJECT_DATA_ROCK_CHUNK) {
            g_apObjectShape_00493868[obj] =
                g_aObjectTypeData_00496d30[
                    OBJECT_DATA_ASTEROID1].shapeSet;
        } else {
            g_apObjectShape_00493868[obj] = shapeSource->shapeSet;
        }
    }
    g_asObjectType_00495298[obj] = typeData->field_18;
    g_aeObjectClass_00495328[obj] = typeData->objectClass;
    if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_MISSILE) {
        if (g_asShipSide_004955d0[owner] == SIDE_KILRATHI && owner != 0 &&
            g_pGenericMissileShape_0049c8f0 != 0 &&
            g_pGenericMissileExhaustShape_0049c8f4 != 0 &&
            g_asObjectType_00495298[obj] != OBJECT_DATA_CHAFF_POD) {
            g_apObjectShape_00493868[obj] = g_pGenericMissileShape_0049c8f0;
            g_apObjectExhaustShape_004953b8[obj] =
                g_pGenericMissileExhaustShape_0049c8f4;
        } else {
            g_apObjectExhaustShape_004953b8[obj] = typeData->animation;
        }
    }
    init_ijk(obj);
    g_asObjectCollisionRadius_004950e8[obj] = typeData->collisionRadius;
    g_asObjectScale_00494d90[obj] = typeData->scale;
    if (type == 0xe) {
        g_asObjectScale_00494d90[obj] =
            (unsigned short)(g_asObjectScale_00494d90[obj] * 8);
    }
    if (type == 0xd) {
        g_asObjectScale_00494d90[obj] =
            (unsigned short)(g_asObjectScale_00494d90[obj] * 8);
    }
    g_asObjectDamage_00495178[obj] = 0;
    g_asObjectFlip_004939c8[obj] = g_asObjectDamage_00495178[obj];
    g_asObjectScreenAngle_004936b8[obj] = g_asObjectFlip_004939c8[obj];
    g_acObjectOwner_00495208[obj] = (signed char)owner;
    g_acLastCollisionObject_00495250[obj] = -1;
    if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_MISSILE) {
        g_asObjectViewFrame_00493508[obj] = 0;
        g_acShipTarget_00495f20[obj] = -1;
        if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_SHIP) {
            g_aasShipShield_00495518[obj][0] = typeData->shieldFore;
            g_aasShipMaximumShield_004954f0[obj][0] =
                g_aasShipShield_00495518[obj][0];
            g_aasShipShield_00495518[obj][1] = typeData->shieldAft;
            g_aasShipMaximumShield_004954f0[obj][1] =
                g_aasShipShield_00495518[obj][1];
            g_aasShipArmor_00495540[obj][0] = typeData->armorFront;
            g_aasShipArmor_00495540[obj][2] = typeData->armorLeft;
            g_aasShipArmor_00495540[obj][3] = typeData->armorRight;
            g_aasShipArmor_00495540[obj][1] = typeData->armorRear;
            g_anShipFuel_00495638[obj] = *(int *)&typeData->lifetime;
            g_acShipIonDriveDamage_004956a0[obj] = 0;
            g_acShipDamage_00495690[obj] = 0;
            recalc_max_velocity(obj);
            DAT_00495d78[obj] = 4;
            memcpy(g_aShipWeapons_004956b0[obj], typeData->weaponLoadout,
                   sizeof(typeData->weaponLoadout));
            InitializeShipWeaponTypeIndices(obj);
            if (obj == 0) {
                g_nSelectedReleaseWeaponIndex_004934e0 = -1;
                g_nSelectedGunType_004934dc = -1;
                slot = (short)(signed char)g_aShipWeapons_004956b0[obj][0];
                while (slot-- > 0) {
                    if (((ShipWeaponSlot *)(
                             g_aShipWeapons_004956b0[obj] + 1))[
                            slot].disabled != 0)
                        continue;
                    if (g_aObjectTypeData_00496d30[
                            ((ShipWeaponSlot *)(
                                 g_aShipWeapons_004956b0[obj] + 1))[
                                slot].weaponType].objectClass ==
                        OBJECT_CLASS_PROJECTILE) {
                        if (g_nSelectedGunType_004934dc != -1) {
                            if (((ShipWeaponSlot *)(
                                     g_aShipWeapons_004956b0[obj] + 1))[
                                    slot].type !=
                                g_nSelectedGunType_004934dc)
                                g_nSelectedGunType_004934dc = 0x80;
                        } else {
                            g_nSelectedGunType_004934dc =
                                ((ShipWeaponSlot *)(
                                     g_aShipWeapons_004956b0[obj] + 1))[
                                    slot].type;
                        }
                    } else {
                        g_nSelectedReleaseWeaponIndex_004934e0 = slot;
                    }
                }
            }
            if (HasShipCockpitGunDisplay(obj) != 0)
                g_asShipTurretFireEnabled_0049d470[obj] = 1;
            else
                g_asShipTurretFireEnabled_0049d470[obj] = 0;
            g_asShipWeaponEnergy_00495590[obj] = 100;
            g_acShipLastAttacker_004955c0[obj] = -1;
        }
    } else {
        animation = typeData->animation;
        if (animation == 0) {
            frame = 0;
            g_asObjectViewFrame_00493508[obj] =
                (short)(typeData->yawRate + frame);
        } else {
            g_acObjectAnimationDelay_00494d00[obj] = 1;
            g_asObjectAnimationIndex_00494c70[obj] = 0;
            frame = g_asObjectAnimationIndex_00494c70[obj];
            animate_shape(obj);
        }
    }
}

/* Function start: 0x411172 */
unsigned int match_rotation_goal(short *rotation, short *goal,
                                 short totalError, short rate)
{
    short step;

    if (totalError != 0) {
        if (*goal > 180)
            *goal = *goal - 360;
        if (*goal < -180)
            *goal = *goal + 360;
        step = MaxShort(1, (short)(abs(*rotation - *goal) *
                                  rate / totalError));
        if (*goal != *rotation || step < abs(*rotation)) {
            if (*goal < 1) {
                *rotation = (short)(*rotation +
                    MinShort(
                        MaxShort(
                            (short)(MaxShort(*goal, (short)-step) -
                                    *rotation),
                            (short)-step),
                        step));
            } else {
                *rotation = (short)(*rotation +
                    MaxShort(
                        MinShort(
                            (short)(MinShort(*goal, step) - *rotation),
                            step),
                        (short)-step));
            }
        }
    }
    if (*goal != 0) {
        if (*goal > 0)
            *goal = MaxShort((short)(*goal - *rotation), 0);
        else
            *goal = MinShort((short)(*goal - *rotation), 0);
    }
    return 0;
}

#pragma function(abs)

/* Function start: 0x41133D */
void rotate_object_to_goal(short obj)
{
    short turnScale;
    short rate;
    short totalError;
    ObjectTypeData *typeData;

    typeData = &g_aObjectTypeData_00496d30[g_acObjectType_00493980[obj]];
    if (g_aeSpecialManeuver_00495600[obj] ==
            SPECIAL_MANEUVER_BLOWING_UP) {
        if ((short)alert_flag(obj, 1) != 0) {
            set_special(obj, SPECIAL_MANEUVER_NONE);
        } else {
            /* The original passes a difficulty the callee ignores. */
            if (g_asObjectCounter_00494be0[obj] == -1 &&
                skill_check(obj, 7) != 0)
                g_aeSpecialManeuver_00495600[obj] = SPECIAL_MANEUVER_NONE;
            return;
        }
    }
    totalError = (short)(abs(g_anObjectRollRotation_00495058[obj] -
                             g_anRollGoal_004954d8[obj]) +
                         abs(g_anObjectYawRotation_00494fc8[obj] -
                             g_anYawGoal_004954c0[obj]) +
                         abs(g_anObjectPitchRotation_00494f38[obj] -
                             g_anPitchGoal_004954a8[obj]));
    rate = typeData->yawRate;
    turnScale = MinShort(180, MaxShort(100, GetAdaptiveTurnRate()));
    if (g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[0] &&
        g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP)
        rate = (short)(turnScale * rate / 100);
    match_rotation_goal(&g_anObjectPitchRotation_00494f38[obj],
                        &g_anPitchGoal_004954a8[obj], totalError,
                        typeData->yawRate);
    rate = typeData->pitchRate;
    if (g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[0] &&
        g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP)
        rate = (short)(turnScale * rate / 100);
    match_rotation_goal(&g_anObjectYawRotation_00494fc8[obj],
                        &g_anYawGoal_004954c0[obj], totalError,
                        typeData->pitchRate);
    rate = typeData->rollRate;
    if (g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[0] &&
        g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP)
        rate = (short)(turnScale * rate / 100);
    match_rotation_goal(&g_anObjectRollRotation_00495058[obj],
                        &g_anRollGoal_004954d8[obj], totalError,
                        typeData->rollRate);
}

/* Function start: 0x4115F8 */
/* A ship whose commanded rotation runs further ahead of its own turn rates
 * than its pilot can absorb tumbles out of control for a few frames. */
void check_for_lost_control(short obj)
{
    short turnScale;
    short type;
    short overshoot;
    short turnRate;

    if (obj != 0 &&
        g_aeSpecialManeuver_00495600[obj] != SPECIAL_MANEUVER_UNKNOWN_9) {
        if (g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[0] &&
            g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP)
            turnScale = MinShort(180, MaxShort(100, GetAdaptiveTurnRate()));
        else
            turnScale = 100;
        type = g_acObjectType_00493980[obj];
        turnRate = (short)(g_aObjectTypeData_00496d30[type].pitchRate *
                           turnScale / 100);
        turnRate = (short)(g_aObjectTypeData_00496d30[type].yawRate *
                           turnScale / 100 + turnRate);
        turnRate = (short)(g_aObjectTypeData_00496d30[type].rollRate *
                           turnScale / 100 + turnRate);
        overshoot = (short)(abs(g_anObjectRollRotation_00495058[obj]) +
                            abs(g_anObjectYawRotation_00494fc8[obj]) +
                            abs(g_anObjectPitchRotation_00494f38[obj]));
        overshoot = (short)(overshoot / turnRate);
        /* The original passes the overshoot the callee ignores. */
        if (skill_check(obj, overshoot) ==
            0) {
            set_special(obj, SPECIAL_MANEUVER_BLOWING_UP);
            g_asObjectCounter_00494be0[obj] =
                (short)(RandomBelowOrEqual(6) + 5);
        }
    }
}

#pragma intrinsic(abs)

/* Function start: 0x4117AC */
void celerate(short ship, int delta)
{
    int maximumSpeed;

    maximumSpeed = (int)g_asShipMaximumVelocity_00495f70[ship] << 8;
    g_anShipSpeed_00494e20[ship] += delta;
    if (g_anShipSpeed_00494e20[ship] > maximumSpeed)
        g_anShipSpeed_00494e20[ship] = maximumSpeed;
    if (g_anShipSpeed_00494e20[ship] < 0)
        g_anShipSpeed_00494e20[ship] = 0;
}

/* Function start: 0x41181D */
unsigned int approach_speed(short ship, int targetSpeed)
{
    int delta;
    int acceleration;

    acceleration = GetShipAccelerationRate(ship);
    delta = targetSpeed - g_anShipSpeed_00494e20[ship];

    if ((short)alert_flag(ship, 1))
        acceleration += acceleration;
    if ((delta < 0 ? -delta : delta) > acceleration)
        delta = MultiplyFixed(SignFixed(delta), acceleration);
    celerate(ship, delta);
    return 0;
}

/* Function start: 0x4118A9 */
void steady_object(short ship)
{
    g_anRollGoal_004954d8[ship] = g_anPitchGoal_004954a8[ship] =
        g_anYawGoal_004954c0[ship] = 0;
}

/* Function start: 0x4118F2 */
short real_velocity(short obj)
{
    return FixedToShortSaturating(
        Vector_magnitude(&g_aShipVelocity_00494898[obj]));
}

/* Function start: 0x411922 */
unsigned int fix_velocity(short obj)
{
    ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                     g_anShipSpeed_00494e20[obj],
                     &g_aShipVelocity_00494898[obj]);
    return 0;
}

/* Function start: 0x41195F */
void SortViableTargetsByDistance(void)
{
    short outer;
    short inner;
    short distance;
    short target;

    if (g_cViableTargetCount_00496178 > 1) {
        for (outer = 0;
             outer < (short)g_cViableTargetCount_00496178 - 1;
             outer++) {
            for (inner = (short)(outer + 1);
                 inner < (short)g_cViableTargetCount_00496178;
                 inner++) {
                distance = g_asViableTargetDistance_00496190[outer];
                if (g_asViableTargetDistance_00496190[inner] < distance) {
                    target = g_acViableTarget_00496180[outer];
                    g_asViableTargetDistance_00496190[outer] =
                        g_asViableTargetDistance_00496190[inner];
                    g_acViableTarget_00496180[outer] =
                        g_acViableTarget_00496180[inner];
                    g_asViableTargetDistance_00496190[inner] = distance;
                    g_acViableTarget_00496180[inner] = (signed char)target;
                }
            }
        }
    }
}
