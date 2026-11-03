/*
 *  Canned-scene tape recorder.
 *
 *  Retail object 0x401A10-0x40243B (link order; see docs/ORDER.md).
 */
#include "game.h"

void *g_pHighMemoryBlockA_004901f8 = 0;
unsigned short g_wHighMemoryBlockBytes_004901fc = 0xd4e4;
void *g_pHighMemoryBlockB_00490200 = 0;
char g_szCannedSceneTapeFile_00490208[9] = "tape.tmp";
short g_nCannedSceneBufferNearCapacityFlag_00490214 = 0;
short g_nCannedSceneSegmentEndFrame_00490218 = -1;
short g_nCannedSceneMode_0049021c = 0;
short g_nCannedSceneStateRecordMarker_00490220 = 0;

extern unsigned int g_dwCannedSceneSnapshotEnd_004961a4;

/* Function start: 0x401A10 */
void ResetCannedSceneRecording(void)
{
    if (g_bHighMemoryBuffersSetState_005d2ad8 == 0)
        return;
    unlink(g_szCannedSceneTapeFile_00490208);
    g_nCannedSceneFrameCount_005d404c = 0;
    g_nCannedSceneFileOffset_005d3fac = 0;
    g_nCannedSceneMode_0049021c = 0;
    WriteTapeInitialState();
    InitializeCannedSceneFrameIndex();
}

/* Function start: 0x401A62 */
void InitializeCannedSceneFrameIndex(void)
{
    CannedSceneBufferHeader *header;
    short index;

    for (index = 0; index < 70; index++)
        g_asCannedSceneFrameStarts_005d3fc0[index] = -1;
    g_nCannedSceneWriteIndex_005d3fa8 = 0;
    header = (CannedSceneBufferHeader *)IdentityDword(
        (DwordPtr)g_pHighMemoryBlockA_004901f8);
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

    if (g_bHighMemoryBuffersSetState_005d2ad8 == 0)
        return;
    if (g_nCannedSceneMode_0049021c != 0) {
        savedPoint = g_aShipPosition_00494550[EYE_OBJECT];
        savedVelocity = g_aShipVelocity_00494898[EYE_OBJECT];
        savedUp = g_aShipUpVector_00493ec0[EYE_OBJECT];
        savedForward = g_aShipForwardVector_00494208[EYE_OBJECT];
        savedView = g_nCurrentView_00492fa8;
        LoadCannedScenePlaybackBuffer();
        ResetCannedScenePlaybackBuffer();
        g_aShipPosition_00494550[EYE_OBJECT] = savedPoint;
        g_aShipVelocity_00494898[EYE_OBJECT] = savedVelocity;
        g_aShipUpVector_00493ec0[EYE_OBJECT] = savedUp;
        g_aShipForwardVector_00494208[EYE_OBJECT] = savedForward;
        g_nCurrentView_00492fa8 = savedView;
    } else {
        FlushCannedSceneRecordingBuffer();
        InitializeCannedSceneFrameIndex();
    }
    g_nCannedSceneBufferNearCapacityFlag_00490214 = 0;
}

/* Function start: 0x401C1A */
void CheckCannedSceneBufferCapacity(void)
{
    if ((int)(unsigned short)g_wHighMemoryBlockBytes_004901fc - 1000 <=
            (int)(unsigned short)g_nCannedSceneWriteIndex_005d3fa8) {
        if ((int)(unsigned short)g_wHighMemoryBlockBytes_004901fc <=
                (int)(unsigned short)g_nCannedSceneWriteIndex_005d3fa8) {
            ReportFatalErrorCode("028");
        }
        g_nCannedSceneBufferNearCapacityFlag_00490214 = 1;
    }
}

/* Function start: 0x401C72 */
int GetCannedSceneObjectMode(short obj)
{
    if (g_aeObjectClass_00495328[obj] < OBJECT_CLASS_SHIP)
        return 0;
    if (g_anShipCloakState_00496020[obj] == 1 &&
        g_asShipCloakElapsedFrames_00496060[obj] == 0)
        return 7;
    else if (g_anShipCloakState_00496020[obj] == 0 &&
             g_asShipCloakCooldown_00496048[obj] == 40)
        return 8;
    if (g_asShipTactic_00495f30[obj] == TACTIC_WARP_OUT)
        return 2;
    if (g_asShipTactic_00495f30[obj] == TACTIC_WARP_IN)
        return 3;
    switch (g_aeSpecialManeuver_00495600[obj]) {
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

    if (g_bHighMemoryBuffersSetState_005d2ad8 == 0 ||
        g_nCannedSceneMode_0049021c != 0)
        return;
    {
        g_dwHighMemoryParagraphValue_005d3fb4 =
            IdentityDword((DwordPtr)g_pHighMemoryBlockA_004901f8);
        objectMode = GetCannedSceneObjectMode(obj);
        if ((unsigned short)g_asCannedSceneFrameStarts_005d3fc0[obj] !=
            0xffff) {
            record = (CannedSceneObjectStateRecord *)(
                (unsigned int)(unsigned short)
                    g_asCannedSceneFrameStarts_005d3fc0[obj] +
                g_dwHighMemoryParagraphValue_005d3fb4);
            recordedVelocity = record->velocity;
            recordedUp = record->up;
            recordedForward = record->forward;
            if (equ_vector(&g_aShipVelocity_00494898[obj],
                           &recordedVelocity) != 0 &&
                equ_vector(&g_aShipUpVector_00493ec0[obj],
                           &recordedUp) != 0 &&
                equ_vector(&g_aShipForwardVector_00494208[obj],
                           &recordedForward) != 0 &&
                record->mode == objectMode)
                return;
        }
        g_nCannedSceneStateRecordMarker_00490220 = 0;
        record = (CannedSceneObjectStateRecord *)(
            (unsigned int)(unsigned short)g_nCannedSceneWriteIndex_005d3fa8 +
            g_dwHighMemoryParagraphValue_005d3fb4);
        g_asCannedSceneFrameStarts_005d3fc0[obj] =
            g_nCannedSceneWriteIndex_005d3fa8;
        if (0) {
            LogMemoryUsage();
            printf("ship script error:  segment wrap");
            getch();
            exit(1);
        }
        record->opcode = 0;
        record->frame = g_nSpaceFrame_00493134;
        record->object = (signed char)obj;
        record->velocity = g_aShipVelocity_00494898[obj];
        record->forward = g_aShipForwardVector_00494208[obj];
        record->up = g_aShipUpVector_00493ec0[obj];
        record->mode = objectMode;
        record->endMarker = -1;
        record->nextOffset = 0x29b;
        g_nCannedSceneWriteIndex_005d3fa8 = (short)(
            (unsigned short)g_nCannedSceneWriteIndex_005d3fa8 + 0x2f);
        CheckCannedSceneBufferCapacity();
    }
}

/* Function start: 0x401FDD */
void RecordCannedSceneObjectEvent(short obj, int event)
{
    CannedSceneObjectEventRecord *record;

    if (g_bHighMemoryBuffersSetState_005d2ad8 == 0 ||
        g_nCannedSceneMode_0049021c != 0)
        return;
    g_dwHighMemoryParagraphValue_005d3fb4 =
        IdentityDword((DwordPtr)g_pHighMemoryBlockA_004901f8);
    record = (CannedSceneObjectEventRecord *)(
        (unsigned int)(unsigned short)
            g_nCannedSceneWriteIndex_005d3fa8 +
        g_dwHighMemoryParagraphValue_005d3fb4);
    if (0) {
        LogMemoryUsage();
        printf("master script error:  segment wrap");
        getch();
        exit(1);
    }
    record->opcode = 3;
    record->event = event;
    record->frame = g_nSpaceFrame_00493134;
    record->object = (signed char)obj;
    switch (event) {
    case 0:
        record->position = g_aShipPosition_00494550[obj];
        record->velocity = g_aShipVelocity_00494898[obj];
        record->objectType = g_asObjectType_00495298[obj];
        record->owner = g_acObjectOwner_00495208[obj];
        record->counter = g_asObjectCounter_00494be0[obj];
        record->scale = g_asObjectScale_00494d90[obj];
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
    g_nCannedSceneWriteIndex_005d3fa8 = (short)(
        (unsigned short)g_nCannedSceneWriteIndex_005d3fa8 + 0x2a);
    CheckCannedSceneBufferCapacity();
}

/* Function start: 0x4021A7 */
void RecordCannedSceneBriefingCharacter(signed char character,
                                        signed char pose,
                                        short animationFrame)
{
    CannedSceneBriefingCharacterRecord *record;

    if (g_bHighMemoryBuffersSetState_005d2ad8 == 0 ||
        g_nCannedSceneMode_0049021c != 0)
        return;
    g_dwHighMemoryParagraphValue_005d3fb4 =
        IdentityDword((DwordPtr)g_pHighMemoryBlockA_004901f8);
    record = (CannedSceneBriefingCharacterRecord *)(
        (unsigned int)(unsigned short)
            g_nCannedSceneWriteIndex_005d3fa8 +
        g_dwHighMemoryParagraphValue_005d3fb4);
    record->opcode = 1;
    record->frame = g_nSpaceFrame_00493134;
    record->character = character;
    record->pose = pose;
    record->animationFrame = animationFrame;
    record->endMarker = -1;
    record->nextOffset = 0x29d;
    g_nCannedSceneWriteIndex_005d3fa8 = (short)(
        (unsigned short)g_nCannedSceneWriteIndex_005d3fa8 + 0xa);
    CheckCannedSceneBufferCapacity();
}

/* Function start: 0x402258 */
void RecordMusicCommandHook(int track, int command, short enabled)
{
    CannedSceneMusicCommandRecord *record;

    return;
    if (g_bHighMemoryBuffersSetState_005d2ad8 == 0 ||
        g_nCannedSceneMode_0049021c != 0)
        return;
    g_dwHighMemoryParagraphValue_005d3fb4 =
        IdentityDword((DwordPtr)g_pHighMemoryBlockA_004901f8);
    record = (CannedSceneMusicCommandRecord *)(
        (unsigned int)(unsigned short)g_nCannedSceneWriteIndex_005d3fa8 +
        g_dwHighMemoryParagraphValue_005d3fb4);
    record->opcode = 2;
    record->frame = g_nSpaceFrame_00493134;
    record->track = track;
    record->command = command;
    record->enabled = enabled;
    record->endMarker = -1;
    record->nextOffset = 0x29e;
    g_nCannedSceneWriteIndex_005d3fa8 = (short)(
        (unsigned short)g_nCannedSceneWriteIndex_005d3fa8 + 0x10);
    CheckCannedSceneBufferCapacity();
}

/* Function start: 0x40230E */
void WriteTapeInitialState(void)
{
    short file;

    file = CreateDataFile(g_szCannedSceneTapeFile_00490208);
    if (file == -1)
        ReportFatalErrorCode("029");
#ifndef SDL_PORT
    WriteDataFileAtOffset((unsigned short)file,
                          g_nCannedSceneFileOffset_005d3fac,
                          CANNED_SCENE_SNAPSHOT_BYTES,
                          &g_dwCannedSceneSnapshotStart_00493130);
#endif
    g_nCannedSceneFileOffset_005d3fac += CANNED_SCENE_SNAPSHOT_BYTES;
    CloseDataFile((unsigned short)file);
}

/* Function start: 0x402385 */
void FlushCannedSceneRecordingBuffer(void)
{
    short file;

    file = OpenDataFileOrDie(g_szCannedSceneTapeFile_00490208);
    if (file == -1)
        ReportFatalErrorCode("030");
    WriteDataFileAtOffset((unsigned short)file,
                          g_nCannedSceneFileOffset_005d3fac,
                          2, &g_nSpaceFrame_00493134);
    g_nCannedSceneFileOffset_005d3fac += 2;
    WriteDataFileAtOffset((unsigned short)file,
                          g_nCannedSceneFileOffset_005d3fac,
                          2, &g_nCannedSceneWriteIndex_005d3fa8);
    g_nCannedSceneFileOffset_005d3fac += 2;
    WriteDataFileAtOffset(
        (unsigned short)file, g_nCannedSceneFileOffset_005d3fac,
        (unsigned int)(unsigned short)g_nCannedSceneWriteIndex_005d3fa8,
        g_pHighMemoryBlockA_004901f8);
    g_nCannedSceneFileOffset_005d3fac +=
        (unsigned short)g_nCannedSceneWriteIndex_005d3fa8;
    CloseDataFile((unsigned short)file);
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short g_nCannedSceneWriteIndex_005d3fa8;
short g_nCannedSceneRecordedFrameCountNumber_005d3faa;
int g_nCannedSceneFileOffset_005d3fac;
void *g_pCannedSceneStateBlockRef_005d3fb0;
DwordPtr g_dwHighMemoryParagraphValue_005d3fb4;
short g_asCannedSceneFrameStarts_005d3fc0[70];
short g_nCannedSceneFrameCount_005d404c;
