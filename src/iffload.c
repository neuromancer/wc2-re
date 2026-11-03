/*
 *  Cutscene IFF resource parsing and release.
 *
 *  Retail object 0x40D6B0-0x40F01F (link order; see docs/ORDER.md).
 */
#include "game.h"

CutsceneObjectResourceList *g_pCutsceneSpriteResources_0049288c = 0;
CutsceneObjectResourceList *g_pCutscenePlaneResources_00492890 = 0;
CutsceneObjectResourceList *g_pCutsceneSequenceResources_00492894 = 0;
CutsceneObjectResourceList *g_pCutsceneSceneResources_00492898 = 0;
CutsceneTextResource *g_pCutsceneTextResources_0049289c = 0;
CutsceneResourceTable *g_pCutsceneFontResources_004928a0 = 0;
CutsceneResourceTable *g_pCutsceneSpeechResources_004928a4 = 0;
CutsceneResourceTable *g_pCutsceneShapeResources_004928a8 = 0;
CutsceneResourceTable *g_pCutsceneMouseResources_004928ac = 0;
CutsceneResourceTable *g_pCutscenePaletteResources_004928b0 = 0;
CutsceneResourceTable *g_pCutsceneFilmResources_004928b4 = 0;
CutsceneResourceTable *g_pCutsceneFxResources_004928b8 = 0;
CutsceneResourceTable *g_pCutsceneMusicResources_004928bc = 0;

/* Function start: 0x40D6B0 */
void ReleasePacketSlot(void **slot)
{
    if (slot == 0)
        return;
    ReleasePacketHandle(*slot);
    *slot = 0;
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
    unsigned char *read;

    read = *cursor;
    formType = 0;
    if (*(unsigned int *)read == 0x4d524f46) {
        read += 4;
        chunkSize = *(unsigned int *)read;
        read += 4;
        SwapSceneChunkSizeEndian((int *)&chunkSize);
        *nextForm = read + ((chunkSize + 1) & ~1);
        formType = *(unsigned int *)read;
        read += 4;
        *cursor = IdentityHandle(read);
    }
    return formType;
}

/* Function start: 0x40D88E */
void SkipCutsceneChunk(unsigned char **cursor)
{
    unsigned int size;
    unsigned char *data;

    data = *cursor;
    size = *(unsigned int *)data;
    data += 4;
    SwapSceneChunkSizeEndian((int *)&size);
    data += (size + 1) & ~1U;
    *cursor = IdentityHandle(data);
}

/* Function start: 0x40D8D7 */
short FindCutsceneResourceSymbolIndex(CutsceneObjectResourceList *list,
                                      short scriptHalf,
                                      const char *symbol)
{
    CutsceneObjectResourceHalf *halfInfo;
    char **symbols;
    short count;
    short symbolIndex;

    if (list != 0) {
        if (scriptHalf == 0)
            halfInfo = (CutsceneObjectResourceHalf *)list;
        else if (scriptHalf == 1)
            halfInfo = (CutsceneObjectResourceHalf *)
                ((char *)list + sizeof(CutsceneObjectResourceHalf));
        symbols = halfInfo->symbols;
        count = halfInfo->symbolCount;
        for (symbolIndex = 0; symbolIndex < count; symbolIndex++, symbols++) {
            if (AreCutsceneResourceNamesEqual(*symbols, symbol) != 0)
                return halfInfo->symbolIndices[symbolIndex];
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
short CreateCutsceneResourceInstance(int formType,
                                     CutsceneObjectResourceList *list,
                                     short index)
{
    SceneFlicObject *sprite;
    CutscenePlane *plane;
    CutsceneSequence *sequence;
    CutsceneScene *cutscene;
    short entry;

    entry = 0;
    switch (formType) {
    case 0x54525053:
        while (entry < 0x80) {
            if (g_apSceneObjects_00499c38[entry] == 0) {
                g_apSceneObjects_00499c38[entry] = AllocateScenePointerTable(
                    1, sizeof(SceneFlicObject), 0,
                    "Cannot Allocate Sprite Data");
                sprite = g_apSceneObjects_00499c38[entry];
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
            entry++;
        }
        if (entry == 0x80)
            FatalCutsceneError("Too few sprite slots");
        break;
    case 0x454e4c50:
        while (entry < 0x40) {
            if (g_apCutscenePlanes_00499c3c[entry] == 0) {
                g_apCutscenePlanes_00499c3c[entry] = AllocateScenePointerTable(
                    1, sizeof(CutscenePlane), 0,
                    "Cannot Allocate Plane Data");
                plane = g_apCutscenePlanes_00499c3c[entry];
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
            entry++;
        }
        if (entry == 0x40)
            FatalCutsceneError("Too few plane slots");
        break;
    case 0x55514553:
        while (entry < 0x100) {
            if (g_apCutsceneSequences_00499c40[entry] == 0) {
                g_apCutsceneSequences_00499c40[entry] = AllocateScenePointerTable(
                    1, sizeof(CutsceneSequence), 0,
                    "Cannot allocate Sequence Data");
                sequence = g_apCutsceneSequences_00499c40[entry];
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
            entry++;
        }
        if (entry == 0x100)
            FatalCutsceneError("Too few sequence slots");
        break;
    case 0x454e4353:
        while (entry < 0x20) {
            if (g_apCutsceneScenes_00499c44[entry] == 0) {
                g_apCutsceneScenes_00499c44[entry] = AllocateScenePointerTable(
                    1, sizeof(CutsceneScene), 0,
                    "Cannot Allocate Script Data");
                cutscene = g_apCutsceneScenes_00499c44[entry];
                cutscene->owner = g_nCutsceneResourceLevel_00499d98;
                cutscene->active = 1;
                cutscene->scriptStart = list->scripts[index];
                cutscene->scriptCursor = cutscene->scriptStart;
                cutscene->sequenceIndices = AllocateScenePointerTable(
                    0x10, 1, 0, "Cannot Allocate SequenceNumList");
                if (cutscene->scriptCursor != 0)
                    ExecuteCutsceneScene(cutscene);
                cutscene->scriptStart = cutscene->scriptCursor;
                break;
            }
            entry++;
        }
        if (entry == 0x20)
            FatalCutsceneError("Too few script slots");
        break;
    case 0x52435346:
        break;
    }
    return entry;
}

/* Function start: 0x40DE5A */
signed char LinkCutsceneObjectResources(CutsceneObjectResourceList *list,
                                        short scriptHalf,
                                        int formType)
{
    CutsceneObjectResourceList *node;
    CutsceneObjectResourceHalf *upperHalf;
    short id;
    SceneFlicObject *sprite;
    CutscenePlane *plane;
    CutsceneSequence *sequence;
    short num;
    CutsceneScene *cutscene;

    if (scriptHalf == 0)
        upperHalf = (CutsceneObjectResourceHalf *)list;
    else if (scriptHalf == 1)
        upperHalf = (CutsceneObjectResourceHalf *)
            ((char *)list + sizeof(CutsceneObjectResourceHalf));
    num = upperHalf->symbolCount;
    node = list->next;
    if (node == 0) {
        while (num-- != 0)
            upperHalf->symbolIndices[num] = CreateCutsceneResourceInstance(
                formType, list, num);
        upperHalf->count = (unsigned char)upperHalf->symbolCount;
    } else {
        upperHalf->count = (unsigned char)(upperHalf->inheritedCount + num);
        while (num-- != 0) {
            id = FindCutsceneResourceSymbolIndex(
                node, scriptHalf, upperHalf->symbols[num]);
            if (id != -1) {
                upperHalf->symbolIndices[num] = id;
                switch (formType) {
                case 0x54525053:
                    sprite = g_apSceneObjects_00499c38[id];
                    if (sprite->scriptCursor == 0) {
                        sprite->scriptStart = list->scripts[num];
                        sprite->scriptCursor = sprite->scriptStart;
                        UpdateCutsceneSpriteObject(sprite);
                    }
                    break;
                case 0x454e4c50:
                    plane = g_apCutscenePlanes_00499c3c[id];
                    if (plane->scriptCursor == 0) {
                        plane->scriptStart = list->scripts[num];
                        plane->scriptCursor = plane->scriptStart;
                        UpdateCutscenePlaneObject(plane, 0);
                    }
                    break;
                case 0x55514553:
                    sequence = g_apCutsceneSequences_00499c40[id];
                    if (sequence->scriptCursor == 0) {
                        sequence->scriptStart = list->scripts[num];
                        sequence->scriptCursor = sequence->scriptStart;
                        ExecuteCutsceneSequence(sequence, 0, 0);
                    }
                    break;
                case 0x454e4353:
                    cutscene = g_apCutsceneScenes_00499c44[id];
                    if (cutscene->scriptCursor == 0) {
                        cutscene->scriptStart = list->scripts[num];
                        cutscene->scriptCursor = cutscene->scriptStart;
                        ExecuteCutsceneScene(cutscene);
                    }
                    break;
                }
            } else {
                upperHalf->symbolIndices[num] = CreateCutsceneResourceInstance(
                    formType, list, num);
            }
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

    chunk = *cursor;
    decoded = 0;
    switch (*(int *)chunk) {
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
    default:
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
    short entryCount;

    DecodeSceneResourceChunk(cursor, &decodedResource);
    resource->count = decodedResource.count;
    entryCount = resource->count;
    resource->owner = g_nCutsceneResourceLevel_00499d98;
    resource->filenameIndices = decodedResource.data;
    resource->sectionIndices =
        entryCount + (const unsigned char *)decodedResource.data;
    resource->packedFilenames =
        entryCount * 2 + (char *)decodedResource.data;
    resource->loadedPackets = AllocateScenePointerTable(
        entryCount, HOST_POINTER_SIZE, 0,
        "Cannot Allocate InMemory List");
    if (resource->loadedPackets == 0)
        ReleasePacketSlot((void **)&resource);
}

/* Read into an unused local when a GLOB chunk is decoded (DOS g_ce02); no
 * code in the Win32 port writes it. */
extern short g_nEarlierGlobalCount_005d2ec4;

/* Function start: 0x40E31F */
unsigned char ParseCutsceneContainer(void *scenePacket)
{
    unsigned char *formEnd;
    SceneResourceTable decodedResource;
    unsigned char *containerEnd;
    CutsceneTextResource *text;
    unsigned char *location;
    short globalLength;
    short oldCount;

    location = IdentityHandle(scenePacket);
    InitializeCutsceneRuntimeResources();
    if (ReadNextSceneForm(&location, &containerEnd) != 0x50435343)
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
    while (location < containerEnd) {
        switch ((long)ReadNextSceneForm(&location, &formEnd)) {
        case 0x54525053:
            DecodeCutsceneObjectResource(
                g_pCutsceneSpriteResources_0049288c,
                &location, formEnd, 0x54525053);
            break;
        case 0x454e4c50:
            DecodeCutsceneObjectResource(
                g_pCutscenePlaneResources_00492890,
                &location, formEnd, 0x454e4c50);
            break;
        case 0x55514553:
            DecodeCutsceneObjectResource(
                g_pCutsceneSequenceResources_00492894,
                &location, formEnd, 0x55514553);
            break;
        case 0x454e4353:
            DecodeCutsceneObjectResource(
                g_pCutsceneSceneResources_00492898,
                &location, formEnd, 0x454e4353);
            break;
        case 0x20205846:
            DecodeCutsceneFileResource(
                g_pCutsceneFxResources_004928b8, &location);
            break;
        case 0x2053554d:
            DecodeCutsceneFileResource(
                g_pCutsceneMusicResources_004928bc, &location);
            break;
        case 0x204c4150:
            DecodeCutsceneFileResource(
                g_pCutscenePaletteResources_004928b0, &location);
            break;
        case 0x544e4f46:
            DecodeCutsceneFileResource(
                g_pCutsceneFontResources_004928a0, &location);
            break;
        case 0x48435053:
            DecodeCutsceneFileResource(
                g_pCutsceneSpeechResources_004928a4, &location);
            break;
        case 0x4d4c4946:
            DecodeCutsceneFileResource(
                g_pCutsceneFilmResources_004928b4, &location);
            break;
        case 0x50414853:
            DecodeCutsceneFileResource(
                g_pCutsceneShapeResources_004928a8, &location);
            break;
        case 0x53554f4d:
            DecodeCutsceneFileResource(
                g_pCutsceneMouseResources_004928ac, &location);
            break;
        case 0x424f4c47:
            oldCount = g_nEarlierGlobalCount_005d2ec4;
            DecodeSceneResourceChunk(&location, &decodedResource);
            globalLength = *(short *)(
                ((void **)decodedResource.data)[0]);
            if (g_pCampaignGlobals_00499c94 == 0) {
                g_pCampaignGlobals_00499c94 = AllocateScenePointerTable(
                    globalLength, 2, 0, "Cannot Reallocate Globals");
            }
            ReleasePacketSlot(&decodedResource.data);
            break;
        case 0x54584554:
            DecodeSceneResourceChunk(&location, &decodedResource);
            text = AllocateScenePointerTable(
                1, sizeof(CutsceneTextResource), 0,
                "Cannot Allocate Text Ptrs");
            text->next = g_pCutsceneTextResources_0049289c;
            text->owner = g_nCutsceneResourceLevel_00499d98;
            text->entries = decodedResource.data;
            g_pCutsceneTextResources_0049289c = text;
            break;
        default:
            location = formEnd;
            break;
        }
    }
    return 1;
}

unsigned int ReadNextSceneForm(unsigned char **cursor,
                               unsigned char **nextForm);
signed char DecodeSceneResourceChunk(unsigned char **cursor,
                                     SceneResourceTable *resource);
void SkipCutsceneChunk(unsigned char **cursor);

/* Function start: 0x40E726 */
signed char DecodeCutsceneObjectResource(
    CutsceneObjectResourceList *resource, unsigned char **cursor,
    unsigned char *end, unsigned int formType)
{
    unsigned char *chunkCursor;
    unsigned char *formEnd;
    SceneResourceTable decoded;

    chunkCursor = *cursor;
    while (chunkCursor < end) {
        switch (ReadNextSceneForm(&chunkCursor, &formEnd)) {
        case 0x41544144:
            while (chunkCursor < formEnd) {
                while (DecodeSceneResourceChunk(
                           &chunkCursor, &decoded) != 0) {
                    switch (decoded.type) {
                    case 0:
                        resource->dataEntries = decoded.data;
                        resource->localDataCount = decoded.count;
                        resource->dataRuntimeIndices =
                            AllocateScenePointerTable(
                                decoded.count, 2, 0,
                                "Cannot Allocate Index Array");
                        break;
                    case 2:
                        resource->dataSymbols = decoded.data;
                        resource->dataSymbolCount = decoded.count;
                        resource->dataSymbolIndices =
                            AllocateScenePointerTable(
                                decoded.count, 2, 0,
                                "Cannot Allocate Symbol Index Array");
                        if (LinkCutsceneObjectResources(
                                resource, 0, formType) == 0)
                            return 0;
                        break;
                    }
                }
            }
            break;
        case 0x50524353:
            while (chunkCursor < formEnd) {
                while (DecodeSceneResourceChunk(
                           &chunkCursor, &decoded) != 0) {
                    switch (decoded.type) {
                    case 1:
                        resource->scripts = decoded.data;
                        resource->localScriptCount = decoded.count;
                        resource->scriptRuntimeIndices =
                            AllocateScenePointerTable(
                                decoded.count, 2, 0,
                                "Cannot Allocate Index Array");
                        break;
                    case 2:
                        resource->scriptSymbols = decoded.data;
                        resource->scriptSymbolCount = decoded.count;
                        resource->scriptSymbolIndices =
                            AllocateScenePointerTable(
                                decoded.count, 2, 0,
                                "Cannot Allocate Symbol Index Array");
                        if (LinkCutsceneObjectResources(
                                resource, 1, formType) == 0)
                            return 0;
                        break;
                    }
                }
            }
            break;
        default:
            SkipCutsceneChunk(&chunkCursor);
            break;
        }
    }
    *cursor = chunkCursor;
    return 1;
}

/* Function start: 0x40E985 */
unsigned char ReleaseCutsceneObjectResourceData(
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
                                   int formType)
{
    CutsceneObjectResourceList *res;
    CutscenePlane *plane;
    CutsceneSequence *sequence;
    CutsceneScene *cutscene;
    short index;

    res = *head;
    if (res->owner == owner) {
        ReleaseCutsceneObjectResourceData(res);
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
                    cutscene = g_apCutsceneScenes_00499c44[index];
                    ReleasePacketSlot((void **)&cutscene->locals);
                    ReleasePacketSlot((void **)&cutscene->sequenceIndices);
                    ReleasePacketSlot(
                        (void **)&g_apCutsceneScenes_00499c44[index]);
                }
            }
            break;
        }
        *head = res->next;
        ReleasePacketHandle(res);
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
        while (index-- != 0)
            ReleaseLoadedCutsceneResource(resource, index);
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
