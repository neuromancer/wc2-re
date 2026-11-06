/*
 *  Canned-scene playback.
 *
 *  Retail object 0x446950-0x44716F (link order; see docs/ORDER.md).
 */
#include "game.h"

extern unsigned int g_dwCannedSceneSnapshotEnd; /* 0x004961a4 */

/* Function start: 0x446950 */
void BeginInflightReplayHook(void)
{
    return;
}

/* Function start: 0x446960 */
void ResetCannedScenePlaybackBuffer(void)
{
    g_3_nCannedSceneWriteIndex = 0;
    if (g_nCannedSceneRecordedFrameCountNumber == 0)
        new_view(5, 0);
}

/* Function start: 0x44698F */
void FinishCannedScenePlayback(void)
{
    if (g_78_bHighMemoryBuffersSetState == 0)
        return;
    g_1_pCannedSceneStateBlockRef =
        (void *)IdentityDword((DwordPtr)g_pHighMemoryBlockB);
#ifndef SDL_PORT
    memmove(&g_dwCannedSceneSnapshotStart,
            g_1_pCannedSceneStateBlockRef,
            CANNED_SCENE_SNAPSHOT_BYTES);
#endif
    if (g_nArcadeState != 4) {
        force_view(0, 0);
        SetHudMessageText("END INFLIGHT REPLAY",
                          (unsigned short)g_cRedColour,
                          20);
    }
    _unlink(g_szCannedSceneTapeFile);
}

/* Function start: 0x446A1D */
unsigned short RestoreCannedSceneObjectState(
    CannedSceneObjectStateRecord *record)
{
    short obj;

    obj = record->object;
    g_aShipVelocity[obj] = record->velocity;
    g_aShipUpVector[obj] = record->up;
    g_aShipForwardVector[obj] = record->forward;
    switch (record->mode) {
    case 0:
        g_aeSpecialManeuver[obj] = 0;
        break;
    case 2:
        g_asShipManeuver[obj] = 1;
        break;
    case 3:
        g_asShipManeuver[obj] = 0;
        break;
    case 1:
        g_aeSpecialManeuver[obj] = 1;
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
            ClearViewport(&g_27_stViewScratchInfo,
                          g_bPrimaryViewBufferColour);
            g_bViewportDirty++;
        }
        set_objects_data(obj, kind, owner, 0);
        g_aShipVelocity[obj] = record->velocity;
        g_aShipPosition[obj] = record->position;
        g_asObjectCounter[obj] = record->counter;
        g_asObjectScale[obj] = record->scale;
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
    if (g_78_bHighMemoryBuffersSetState == 0)
        return;
    g_dwHighMemoryParagraphValue =
        IdentityDword((DwordPtr)g_pHighMemoryBlockA);
    while (finished == 0) {
        record = (CannedSceneRecordHeader *)(
            (unsigned int)(unsigned short)
                g_3_nCannedSceneWriteIndex +
            g_dwHighMemoryParagraphValue);
        if (record->frame == g_nSpaceFrame) {
            switch (record->opcode) {
            case 0:
                g_3_nCannedSceneWriteIndex = (short)(
                    (unsigned short)g_3_nCannedSceneWriteIndex +
                    RestoreCannedSceneObjectState(
                        (CannedSceneObjectStateRecord *)record));
                break;
            case 1:
                g_3_nCannedSceneWriteIndex = (short)(
                    (unsigned short)g_3_nCannedSceneWriteIndex +
                    ApplyCannedSceneSoundRecord(
                        (CannedSceneBriefingCharacterRecord *)record));
                break;
            case 2:
                g_3_nCannedSceneWriteIndex = (short)(
                    (unsigned short)g_3_nCannedSceneWriteIndex +
                    ApplyCannedSceneMusicRecord(
                        (CannedSceneMusicCommandRecord *)record));
                break;
            case 3:
                g_3_nCannedSceneWriteIndex = (short)(
                    (unsigned short)g_3_nCannedSceneWriteIndex +
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

    file = OpenDataFileOrDie(g_szCannedSceneTapeFile);
    if (file == -1)
        ReportFatalErrorCode("025");
#ifndef SDL_PORT
    ReadDataFileAtOffset((unsigned short)file,
                         g_4_nCannedSceneFileOffset,
                         CANNED_SCENE_SNAPSHOT_BYTES,
                         &g_dwCannedSceneSnapshotStart);
#endif
    g_4_nCannedSceneFileOffset += CANNED_SCENE_SNAPSHOT_BYTES;
    CloseDataFile((unsigned short)file);
}

/* Function start: 0x446F14 */
void LoadCannedScenePlaybackBuffer(void)
{
    short file;

    file = OpenDataFileOrDie(g_szCannedSceneTapeFile);
    if (file == -1)
        ReportFatalErrorCode("026");
    ReadDataFileAtOffset((unsigned short)file,
                         g_4_nCannedSceneFileOffset,
                         2, &g_nCannedSceneSegmentEndFrame);
    g_4_nCannedSceneFileOffset += 2;
    ReadDataFileAtOffset((unsigned short)file,
                         g_4_nCannedSceneFileOffset,
                         2, &g_3_nCannedSceneWriteIndex);
    g_4_nCannedSceneFileOffset += 2;
    ReadDataFileAtOffset(
        (unsigned short)file, g_4_nCannedSceneFileOffset,
        (unsigned int)(unsigned short)g_3_nCannedSceneWriteIndex,
        g_pHighMemoryBlockA);
    g_4_nCannedSceneFileOffset +=
        (unsigned short)g_3_nCannedSceneWriteIndex;
    CloseDataFile((unsigned short)file);
}

/* Function start: 0x446FC8 */
void ProcessCannedSceneInput(void)
{
    InputEventState nextEvent;
    short keyChanged;
    short eventType;

    g_cPreviousKey = g_cCurrentKey;
    ServiceInputDevices(15);
    eventType = GetNextInputEvent(&nextEvent);
    if (eventType == INPUT_EVENT_KEY_DOWN) {
        g_cCurrentKey = (signed char)nextEvent.status;
        if (g_cPreviousKey !=
            g_cCurrentKey)
            keyChanged = 1;
        else
            keyChanged = 0;
        switch (g_cCurrentKey) {
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
            if (g_acShipTarget[0] != -1)
                new_view(8, 0);
            break;
        case 0x43:
            if (keyChanged != 0) {
                SelectNextExternalViewObject();
                force_view(5, (short)g_cViewObject);
            }
            break;
        }
    }
}
