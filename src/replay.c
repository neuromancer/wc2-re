/*
 *  Canned-scene playback.
 *
 *  Retail object 0x446950-0x44716F (link order; see docs/ORDER.md).
 */
#include "game.h"

extern unsigned int g_dwCannedSceneSnapshotEnd_004961a4;

/* Function start: 0x446950 */
void BeginInflightReplayHook(void)
{
    return;
}

/* Function start: 0x446960 */
void ResetCannedScenePlaybackBuffer(void)
{
    g_nCannedSceneWriteIndex_005d3fa8 = 0;
    if (g_nCannedSceneRecordedFrameCountNumber_005d3faa == 0)
        new_view(5, 0);
}

/* Function start: 0x44698F */
void FinishCannedScenePlayback(void)
{
    if (g_bHighMemoryBuffersSetState_005d2ad8 == 0)
        return;
    g_pCannedSceneStateBlockRef_005d3fb0 =
        (void *)IdentityDword((DwordPtr)g_pHighMemoryBlockB_00490200);
#ifndef SDL_PORT
    memmove(&g_dwCannedSceneSnapshotStart_00493130,
            g_pCannedSceneStateBlockRef_005d3fb0,
            CANNED_SCENE_SNAPSHOT_BYTES);
#endif
    if (g_nArcadeState_0049d75c != 4) {
        force_view(0, 0);
        SetHudMessageText("END INFLIGHT REPLAY",
                          (unsigned short)g_cRedColour_0049cb5c,
                          20);
    }
    _unlink(g_szCannedSceneTapeFile_00490208);
}

/* Function start: 0x446A1D */
unsigned short RestoreCannedSceneObjectState(
    CannedSceneObjectStateRecord *record)
{
    short obj;

    obj = record->object;
    g_aShipVelocity_00494898[obj] = record->velocity;
    g_aShipUpVector_00493ec0[obj] = record->up;
    g_aShipForwardVector_00494208[obj] = record->forward;
    switch (record->mode) {
    case 0:
        g_aeSpecialManeuver_00495600[obj] = 0;
        break;
    case 2:
        g_asShipManeuver_00495f48[obj] = 1;
        break;
    case 3:
        g_asShipManeuver_00495f48[obj] = 0;
        break;
    case 1:
        g_aeSpecialManeuver_00495600[obj] = 1;
        break;
    case 7:
        BeginShipCloak(obj);
        break;
    case 8:
        ResetShipCloakState(obj);
        break;
    case 4:
    case 5:
    case 6:
        break;
    }
    return 0x2f;
}

/* Function start: 0x446B6A */
unsigned short ApplyCannedSceneObjectEventRecord(
    CannedSceneObjectEventRecord *record)
{
    short obj;
    short kind;
    signed char owner;

    obj = record->object;
    kind = record->objectType;
    owner = record->owner;
    switch (record->event) {
    case 0:
        if (kind == OBJECT_DATA_STAR) {
            ClearViewport(&g_stViewScratchInfo_005d2b00,
                          g_bPrimaryViewBufferColour_0049cb50);
            g_bViewportDirty_0049d76c++;
        }
        set_objects_data(obj, kind, owner, 0);
        g_aShipVelocity_00494898[obj] = record->velocity;
        g_aShipPosition_00494550[obj] = record->position;
        g_asObjectCounter_00494be0[obj] = record->counter;
        g_asObjectScale_00494d90[obj] = record->scale;
        break;
    case 1:
        remove_object(obj);
        break;
    case 4:
        auto_pilot_sequence();
        break;
    case 3:
        approach(obj);
        break;
    case 2:
        break;
    }
    return 0x2a;
}

/* Function start: 0x446CC2 */
unsigned short ApplyCannedSceneSoundRecord(
    CannedSceneBriefingCharacterRecord *record)
{
    signed char sound;
    short emitterObject;
    short animationFrame;

    sound = record->character;
    emitterObject = record->pose;
    animationFrame = record->animationFrame;
    PlaySfxWaveFileByNumber(sound, emitterObject, 0);
    return 0xa;
}

/* Function start: 0x446D0D */
unsigned short ApplyCannedSceneMusicRecord(
    CannedSceneMusicCommandRecord *record)
{
    int trackIndex;
    int command;
    short enabled;

    trackIndex = record->track;
    command = record->command;
    enabled = record->enabled;
    ProcessMusicScriptCommand(trackIndex, command, enabled);
    return 0x10;
}

/* Function start: 0x446D55 */
void ApplyCannedSceneFrameEvents(void)
{
    signed char finished;
    CannedSceneRecordHeader *record;

    finished = 0;
    if (g_bHighMemoryBuffersSetState_005d2ad8 == 0)
        return;
    g_dwHighMemoryParagraphValue_005d3fb4 =
        IdentityDword((DwordPtr)g_pHighMemoryBlockA_004901f8);
    while (finished == 0) {
        record = (CannedSceneRecordHeader *)(
            (unsigned int)(unsigned short)
                g_nCannedSceneWriteIndex_005d3fa8 +
            g_dwHighMemoryParagraphValue_005d3fb4);
        if (record->frame == g_nSpaceFrame_00493134) {
            switch (record->opcode) {
            case 0:
                g_nCannedSceneWriteIndex_005d3fa8 = (short)(
                    (unsigned short)g_nCannedSceneWriteIndex_005d3fa8 +
                    RestoreCannedSceneObjectState(
                        (CannedSceneObjectStateRecord *)record));
                break;
            case 1:
                g_nCannedSceneWriteIndex_005d3fa8 = (short)(
                    (unsigned short)g_nCannedSceneWriteIndex_005d3fa8 +
                    ApplyCannedSceneSoundRecord(
                        (CannedSceneBriefingCharacterRecord *)record));
                break;
            case 2:
                g_nCannedSceneWriteIndex_005d3fa8 = (short)(
                    (unsigned short)g_nCannedSceneWriteIndex_005d3fa8 +
                    ApplyCannedSceneMusicRecord(
                        (CannedSceneMusicCommandRecord *)record));
                break;
            case 3:
                g_nCannedSceneWriteIndex_005d3fa8 = (short)(
                    (unsigned short)g_nCannedSceneWriteIndex_005d3fa8 +
                    ApplyCannedSceneObjectEventRecord(
                        (CannedSceneObjectEventRecord *)record));
                break;
            default:
                break;
            }
        } else {
            finished++;
        }
    }
}

/* Function start: 0x446E9D */
/* Nothing in the shipped executable calls this.  Reads back the state block
 * that WriteTapeInitialState saves at the start of a recording. */
void ReadTapeInitialState(void)
{
    short file;

    file = OpenDataFileOrDie(g_szCannedSceneTapeFile_00490208);
    if (file == -1)
        ReportFatalErrorCode("025");
#ifndef SDL_PORT
    ReadDataFileAtOffset((unsigned short)file,
                         g_nCannedSceneFileOffset_005d3fac,
                         CANNED_SCENE_SNAPSHOT_BYTES,
                         &g_dwCannedSceneSnapshotStart_00493130);
#endif
    g_nCannedSceneFileOffset_005d3fac += CANNED_SCENE_SNAPSHOT_BYTES;
    CloseDataFile((unsigned short)file);
}

/* Function start: 0x446F14 */
void LoadCannedScenePlaybackBuffer(void)
{
    short file;

    file = OpenDataFileOrDie(g_szCannedSceneTapeFile_00490208);
    if (file == -1)
        ReportFatalErrorCode("026");
    ReadDataFileAtOffset((unsigned short)file,
                         g_nCannedSceneFileOffset_005d3fac,
                         2, &g_nCannedSceneSegmentEndFrame_00490218);
    g_nCannedSceneFileOffset_005d3fac += 2;
    ReadDataFileAtOffset((unsigned short)file,
                         g_nCannedSceneFileOffset_005d3fac,
                         2, &g_nCannedSceneWriteIndex_005d3fa8);
    g_nCannedSceneFileOffset_005d3fac += 2;
    ReadDataFileAtOffset(
        (unsigned short)file, g_nCannedSceneFileOffset_005d3fac,
        (unsigned int)(unsigned short)g_nCannedSceneWriteIndex_005d3fa8,
        g_pHighMemoryBlockA_004901f8);
    g_nCannedSceneFileOffset_005d3fac +=
        (unsigned short)g_nCannedSceneWriteIndex_005d3fa8;
    CloseDataFile((unsigned short)file);
}

/* Function start: 0x446FC8 */
void ProcessCannedSceneInput(void)
{
    InputEventState nextEvent;
    short keyChanged;
    short eventType;

    g_cPreviousKey_0049312c = g_cCurrentKey_00493128;
    ServiceInputDevices(15);
    eventType = GetNextInputEvent(&nextEvent);
    if (eventType == 4) {
        g_cCurrentKey_00493128 = (signed char)nextEvent.status;
        if (g_cPreviousKey_0049312c !=
            g_cCurrentKey_00493128)
            keyChanged = 1;
        else
            keyChanged = 0;
        switch (g_cCurrentKey_00493128) {
        case 0x34:
            break;
        case 0x33:
            break;
        case 0x3b:
            new_view(0, 0);
            break;
        case 0x3c:
            new_view(2, 0);
            break;
        case 0x3d:
            new_view(1, 0);
            break;
        case 0x3e:
            new_view(3, 0);
            break;
        case 0x3f:
            new_view(5, 0);
            break;
        case 0x40:
            new_view(15, 0);
            break;
        case 0x41:
            if (g_acShipTarget_00495f20[0] != -1)
                new_view(8, 0);
            break;
        case 0x43:
            if (keyChanged != 0) {
                SelectNextExternalViewObject();
                force_view(5, (short)g_cViewObject_0049313c);
            }
            break;
        }
    }
}
