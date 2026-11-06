/*
 *  Canned-scene tape recorder.
 *
 *  Retail object 0x401A10-0x40243B (link order; see docs/ORDER.md).
 */
#include "game.h"

void *g_pHighMemoryBlockA = 0; /* 0x004901f8 */
unsigned short g_wHighMemoryBlockBytes = 0xd4e4; /* 0x004901fc */
void *g_pHighMemoryBlockB = 0; /* 0x00490200 */
char g_szCannedSceneTapeFile[9] = "tape.tmp"; /* 0x00490208 */
short g_nCannedSceneBufferNearCapacityFlag = 0; /* 0x00490214 */
short g_nCannedSceneSegmentEndFrame = -1; /* 0x00490218 */
short g_nCannedSceneMode = 0; /* 0x0049021c */
short g_nCannedSceneStateRecordMarker = 0; /* 0x00490220 */

extern unsigned int g_dwCannedSceneSnapshotEnd; /* 0x004961a4 */

/* Function start: 0x401A10 */
void ResetCannedSceneRecording(void)
{
    if (g_78_bHighMemoryBuffersSetState == 0)
        return;
    unlink(g_szCannedSceneTapeFile);
    g_5_nCannedSceneFrameCount = 0;
    g_4_nCannedSceneFileOffset = 0;
    g_nCannedSceneMode = 0;
    WriteTapeInitialState();
    InitializeCannedSceneFrameIndex();
}

/* Function start: 0x401A62 */
void InitializeCannedSceneFrameIndex(void)
{
    CannedSceneBufferHeader *header;
    short index;

    for (index = 0; index < 70; index++)
        g_1_asCannedSceneFrameStarts[index] = -1;
    g_3_nCannedSceneWriteIndex = 0;
    header = (CannedSceneBufferHeader *)IdentityDword(
        (DwordPtr)g_pHighMemoryBlockA);
    header->nextFrame = -1;
    header->byteCount = 0x29a;
}

/* Function start: 0x401ACB */
void HandleCannedSceneBufferBoundary(void)
{
    FixedVector savedUp;
    FixedVector savedPoint;
    FixedVector savedForward;
    int savedView;
    FixedVector savedVelocity;

    if (g_78_bHighMemoryBuffersSetState == 0)
        return;
    if (g_nCannedSceneMode != 0) {
        savedPoint = g_aShipPosition[EYE_OBJECT];
        savedVelocity = g_aShipVelocity[EYE_OBJECT];
        savedUp = g_aShipUpVector[EYE_OBJECT];
        savedForward = g_aShipForwardVector[EYE_OBJECT];
        savedView = g_nCurrentView;
        LoadCannedScenePlaybackBuffer();
        ResetCannedScenePlaybackBuffer();
        g_aShipPosition[EYE_OBJECT] = savedPoint;
        g_aShipVelocity[EYE_OBJECT] = savedVelocity;
        g_aShipUpVector[EYE_OBJECT] = savedUp;
        g_aShipForwardVector[EYE_OBJECT] = savedForward;
        g_nCurrentView = savedView;
    } else {
        FlushCannedSceneRecordingBuffer();
        InitializeCannedSceneFrameIndex();
    }
    g_nCannedSceneBufferNearCapacityFlag = 0;
}

/* Function start: 0x401C1A */
void CheckCannedSceneBufferCapacity(void)
{
    if ((int)(unsigned short)g_wHighMemoryBlockBytes - 1000 <=
            (int)(unsigned short)g_3_nCannedSceneWriteIndex) {
        if ((int)(unsigned short)g_wHighMemoryBlockBytes <=
                (int)(unsigned short)g_3_nCannedSceneWriteIndex) {
            ReportFatalErrorCode("028");
        }
        g_nCannedSceneBufferNearCapacityFlag = 1;
    }
}

/* Function start: 0x401C72 */
int GetCannedSceneObjectMode(short obj)
{
    if (g_aeObjectClass[obj] < OBJECT_CLASS_SHIP)
        return 0;
    if (g_anShipCloakState[obj] == 1 &&
        g_asShipCloakElapsedFrames[obj] == 0)
        return 7;
    else if (g_anShipCloakState[obj] == 0 &&
             g_asShipCloakCooldown[obj] == 40)
        return 8;
    if (g_asShipTactic[obj] == TACTIC_WARP_OUT)
        return 2;
    if (g_asShipTactic[obj] == TACTIC_WARP_IN)
        return 3;
    switch (g_aeSpecialManeuver[obj]) {
    case 1:
        return 1;
    default:
        return 0;
    }
}

/* Function start: 0x401D79 */
void RecordCannedSceneObjectState(short obj)
{
    FixedVector recordedVelocity;
    FixedVector recordedForward;
    CannedSceneObjectStateRecord *record;
    int objectMode;
    FixedVector recordedUp;

    if (g_78_bHighMemoryBuffersSetState == 0 ||
        g_nCannedSceneMode != 0)
        return;
    {
        g_dwHighMemoryParagraphValue =
            IdentityDword((DwordPtr)g_pHighMemoryBlockA);
        objectMode = GetCannedSceneObjectMode(obj);
        if ((unsigned short)g_1_asCannedSceneFrameStarts[obj] !=
            0xffff) {
            record = (CannedSceneObjectStateRecord *)(
                (unsigned int)(unsigned short)
                    g_1_asCannedSceneFrameStarts[obj] +
                g_dwHighMemoryParagraphValue);
            recordedVelocity = record->velocity;
            recordedUp = record->up;
            recordedForward = record->forward;
            if (equ_vector(&g_aShipVelocity[obj],
                           &recordedVelocity) != 0 &&
                equ_vector(&g_aShipUpVector[obj],
                           &recordedUp) != 0 &&
                equ_vector(&g_aShipForwardVector[obj],
                           &recordedForward) != 0 &&
                record->mode == objectMode)
                return;
        }
        g_nCannedSceneStateRecordMarker = 0;
        record = (CannedSceneObjectStateRecord *)(
            (unsigned int)(unsigned short)g_3_nCannedSceneWriteIndex +
            g_dwHighMemoryParagraphValue);
        g_1_asCannedSceneFrameStarts[obj] =
            g_3_nCannedSceneWriteIndex;
        if (0) {
            LogMemoryUsage();
            printf("ship script error:  segment wrap");
            getch();
            exit(1);
        }
        record->opcode = 0;
        record->frame = g_nSpaceFrame;
        record->object = (signed char)obj;
        record->velocity = g_aShipVelocity[obj];
        record->forward = g_aShipForwardVector[obj];
        record->up = g_aShipUpVector[obj];
        record->mode = objectMode;
        record->endMarker = -1;
        record->nextOffset = 0x29b;
        g_3_nCannedSceneWriteIndex = (short)(
            (unsigned short)g_3_nCannedSceneWriteIndex + 0x2f);
        CheckCannedSceneBufferCapacity();
    }
}

/* Function start: 0x401FDD */
void RecordCannedSceneObjectEvent(short obj, int event)
{
    CannedSceneObjectEventRecord *record;

    if (g_78_bHighMemoryBuffersSetState == 0 ||
        g_nCannedSceneMode != 0)
        return;
    g_dwHighMemoryParagraphValue =
        IdentityDword((DwordPtr)g_pHighMemoryBlockA);
    record = (CannedSceneObjectEventRecord *)(
        (unsigned int)(unsigned short)
            g_3_nCannedSceneWriteIndex +
        g_dwHighMemoryParagraphValue);
    if (0) {
        LogMemoryUsage();
        printf("master script error:  segment wrap");
        getch();
        exit(1);
    }
    record->opcode = 3;
    record->event = event;
    record->frame = g_nSpaceFrame;
    record->object = (signed char)obj;
    switch (event) {
    case 0:
        record->position = g_aShipPosition[obj];
        record->velocity = g_aShipVelocity[obj];
        record->objectType = g_asObjectType[obj];
        record->owner = g_acObjectOwner[obj];
        record->counter = g_asObjectCounter[obj];
        record->scale = g_asObjectScale[obj];
        record->endMarker = -1;
        record->nextOffset = 0x29c;
        break;
    case 1:
        record->endMarker = -1;
        record->nextOffset = 0x29f;
        break;
    case 3:
        record->endMarker = -1;
        record->nextOffset = 0x2a0;
        break;
    default:
        break;
    }
    g_3_nCannedSceneWriteIndex = (short)(
        (unsigned short)g_3_nCannedSceneWriteIndex + 0x2a);
    CheckCannedSceneBufferCapacity();
}

/* Function start: 0x4021A7 */
void RecordCannedSceneBriefingCharacter(signed char character,
                                        signed char pose,
                                        short animationFrame)
{
    CannedSceneBriefingCharacterRecord *record;

    if (g_78_bHighMemoryBuffersSetState == 0 ||
        g_nCannedSceneMode != 0)
        return;
    g_dwHighMemoryParagraphValue =
        IdentityDword((DwordPtr)g_pHighMemoryBlockA);
    record = (CannedSceneBriefingCharacterRecord *)(
        (unsigned int)(unsigned short)
            g_3_nCannedSceneWriteIndex +
        g_dwHighMemoryParagraphValue);
    record->opcode = 1;
    record->frame = g_nSpaceFrame;
    record->character = character;
    record->pose = pose;
    record->animationFrame = animationFrame;
    record->endMarker = -1;
    record->nextOffset = 0x29d;
    g_3_nCannedSceneWriteIndex = (short)(
        (unsigned short)g_3_nCannedSceneWriteIndex + 0xa);
    CheckCannedSceneBufferCapacity();
}

/* Function start: 0x402258 */
void RecordMusicCommandHook(int track, int command, short enabled)
{
    CannedSceneMusicCommandRecord *record;

    return;
    if (g_78_bHighMemoryBuffersSetState == 0 ||
        g_nCannedSceneMode != 0)
        return;
    g_dwHighMemoryParagraphValue =
        IdentityDword((DwordPtr)g_pHighMemoryBlockA);
    record = (CannedSceneMusicCommandRecord *)(
        (unsigned int)(unsigned short)g_3_nCannedSceneWriteIndex +
        g_dwHighMemoryParagraphValue);
    record->opcode = 2;
    record->frame = g_nSpaceFrame;
    record->track = track;
    record->command = command;
    record->enabled = enabled;
    record->endMarker = -1;
    record->nextOffset = 0x29e;
    g_3_nCannedSceneWriteIndex = (short)(
        (unsigned short)g_3_nCannedSceneWriteIndex + 0x10);
    CheckCannedSceneBufferCapacity();
}

/* Function start: 0x40230E */
void WriteTapeInitialState(void)
{
    short file;

    file = CreateDataFile(g_szCannedSceneTapeFile);
    if (file == -1)
        ReportFatalErrorCode("029");
#ifndef SDL_PORT
    WriteDataFileAtOffset((unsigned short)file,
                          g_4_nCannedSceneFileOffset,
                          CANNED_SCENE_SNAPSHOT_BYTES,
                          &g_dwCannedSceneSnapshotStart);
#endif
    g_4_nCannedSceneFileOffset += CANNED_SCENE_SNAPSHOT_BYTES;
    CloseDataFile((unsigned short)file);
}

/* Function start: 0x402385 */
void FlushCannedSceneRecordingBuffer(void)
{
    short file;

    file = OpenDataFileOrDie(g_szCannedSceneTapeFile);
    if (file == -1)
        ReportFatalErrorCode("030");
    WriteDataFileAtOffset((unsigned short)file,
                          g_4_nCannedSceneFileOffset,
                          2, &g_nSpaceFrame);
    g_4_nCannedSceneFileOffset += 2;
    WriteDataFileAtOffset((unsigned short)file,
                          g_4_nCannedSceneFileOffset,
                          2, &g_3_nCannedSceneWriteIndex);
    g_4_nCannedSceneFileOffset += 2;
    WriteDataFileAtOffset(
        (unsigned short)file, g_4_nCannedSceneFileOffset,
        (unsigned int)(unsigned short)g_3_nCannedSceneWriteIndex,
        g_pHighMemoryBlockA);
    g_4_nCannedSceneFileOffset +=
        (unsigned short)g_3_nCannedSceneWriteIndex;
    CloseDataFile((unsigned short)file);
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short g_3_nCannedSceneWriteIndex; /* 0x005d3fa8 */
short g_nCannedSceneRecordedFrameCountNumber; /* 0x005d3faa */
int g_4_nCannedSceneFileOffset; /* 0x005d3fac */
void *g_1_pCannedSceneStateBlockRef; /* 0x005d3fb0 */
DwordPtr g_dwHighMemoryParagraphValue; /* 0x005d3fb4 */
short g_1_asCannedSceneFrameStarts[70]; /* 0x005d3fc0 */
short g_5_nCannedSceneFrameCount; /* 0x005d404c */
