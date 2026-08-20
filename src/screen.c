/*
 *  Screen scopes, prompts and the comm menu.
 *
 *  Address range 0x42efc0-0x431fff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: PushMemoryStackFrame/ShowChoosePrompt/ShowEnemyTargetSelectMenu.
 */
#include "wc1.h"

/* Function start: 0x421910 */
void cleanup_objectives(void)
{
    short objectiveType;
    short proximity;
    short index;
    short object;
    short objective;
    short home;

    objective = 0;
    home = find_ship_index(g_stMissionHeader_005d3e70.wingmanMissionShip);
    proximity = g_aMissionNavPoints_00491e98[
        g_stMissionHeader_005d3e70.entryNavPoint].proximityRadius;
    while ((objectiveType =
                g_aMissionObjectives_004932a8[objective].type) != -1 &&
           objective < 8) {
        if (achieved(objective) == 0) {
            index = g_aMissionObjectives_004932a8[objective].index;
            switch (objectiveType) {
            case 0:
            case 1:
            case 3:
                if (visited(objective) != 0)
                    flag_objective(objective, 2);
                break;
            case 2:
                switch (g_aMissionShips_00492290[index].missionType) {
                case MISSION_TYPE_WARP_ARRIVE:
                case MISSION_TYPE_COME_HOME:
                    object = find_ship_index(index);
                    if (object != -1 && home != -1 &&
                        distance_from_object(object, home) < proximity)
                        g_aMissionShips_00492290[index].state = 1;
                    if (g_aMissionShips_00492290[index].state == 1 &&
                        sighted(objective) != 0) {
                        flag_objective(objective, 2);
                        affect_mission_score(0, 8, -1);
                    }
                    break;
                case MISSION_TYPE_GOTO_WARP:
                    if (g_aMissionShips_00492290[index].state == 2) {
                        flag_objective(objective, 2);
                        affect_mission_score(0, 8, -1);
                    }
                    break;
                default:
                    if (g_aMissionShips_00492290[index].state == 0 &&
                        sighted(objective) != 0) {
                        flag_objective(objective, 2);
                        affect_mission_score(0, 8, -1);
                    }
                    break;
                }
                break;
            case 4:
                if (g_aMissionShips_00492290[index].state == 3)
                    flag_objective(objective, 2);
                break;
            }
        }
        objective++;
    }
}

/* Function start: 0x421BC6 */
short IsShipRouting(short ship)
{
    return g_asShipMissionType_00495de8[ship] == MISSION_TYPE_ROUT;
}

/* Function start: 0x421BFC */
void SendWingmanCommandAcknowledgement(short ship, short accepted)
{
    if (accepted == 1) {
        send_message(ship, 0);
        return;
    }
    send_message(ship, 1);
}

/* Function start: 0x421C35 */
short disobey_formation(short ship)
{
    if (ship != g_nYourWingman_0049346c)
        return 0;
    switch (g_nWingmanFormationDisobeyMode_0049613a) {
    case 1:
        return report_kilrathi_rout(0);
    case 2:
        return detect_enemy_tail(0) != -1;
    case 3:
        return 1;
    case 4:
        return any_enemy(ship, 5000);
    }
    return 0;
}

/* Function start: 0x421CF6 */
short IsEngagementTargetDisallowed(short ship, short target)
{
    if (target != ship &&
        g_asShipSide_004955d0[target] != g_asShipSide_004955d0[ship] &&
        IsShipRouting(ship) == 0)
        return 0;
    return 1;
}

/* Function start: 0x421D59 */
short CanPlayerLand(void)
{
    short index;
    short result;

    result = 0;
    for (index = 0; index < 10; index++) {
        if (g_aeObjectClass_00495328[index] >= OBJECT_CLASS_SHIP &&
            g_asShipSide_004955d0[index] == SIDE_KILRATHI)
            return 0;
    }
    if (g_cPlayerKillCount_005d2fa8 > 0)
        result = 1;
    for (index = 0; index < g_cMissionObjectiveCount_00493294; index++) {
        if (g_aMissionObjectives_004932a8[index].type != 1 &&
            (achieved(index) != 0 ||
             (visited(index) != 0 &&
              g_aMissionObjectives_004932a8[index].type != 2)))
            result = 1;
    }
    return result;
}

/* Function start: 0x421E6D */
short ShouldWingmanAcceptRoutCommand(short ship, short pilot)
{
    if (pilot <= 4 || ship != g_nYourWingman_0049346c)
        return 1;
    switch (g_nWingmanRoutDecisionMode_00496138) {
    case 1:
        return 0;
    case 2:
        return any_enemy(0, 10000) == 0;
    case 3:
        return any_enemy(ship, 5000) == 0;
    case 4:
        return triumph(0);
    case 5:
        return g_asShipMissionType_00495de8[ship] ==
               MISSION_TYPE_CANNED_SEQUENCE;
    }
    return 1;
}

/* Function start: 0x421F86 */
void request(short requester, short ship, short command)
{
    short object;
    short target;

    target = g_acShipTarget_00495f20[requester];
    switch (command) {
    case 0:
        break;
    case 3:
        if (ShouldWingmanAcceptRoutCommand(ship,
                                           g_asPilotLevel_00495d60[ship]) != 0 &&
            try2rout(ship) != 0) {
            g_bEngageAllowed_0049612c = 0;
            SendWingmanCommandAcknowledgement(ship, 1);
        } else {
            SendWingmanCommandAcknowledgement(ship, 0);
        }
        break;
    case 1:
        allow_engage();
        if (IsEngagementTargetDisallowed(ship, target) != 0 ||
            (ShipHasTorpedo(ship) == 0 &&
             CanShipWeaponDamageTarget(ship, target) == 0)) {
            SendWingmanCommandAcknowledgement(ship, 0);
        } else {
            engage(ship, target, OBJECTIVE_ENGAGE_ENEMY);
            SendWingmanCommandAcknowledgement(ship, 1);
        }
        break;
    case 2:
        allow_engage();
        target = -1;
        for (object = 0; object < 10; object++) {
            if (g_aeObjectClass_00495328[object] >= OBJECT_CLASS_SHIP &&
                g_aeSpecialManeuver_00495600[object] !=
                    SPECIAL_MANEUVER_UNKNOWN_9 &&
                g_asShipSide_004955d0[ship] !=
                    g_asShipSide_004955d0[object] &&
                g_acShipTarget_00495f20[object] == requester) {
                target = object;
                break;
            }
        }
        if (target == -1) {
            request(requester, ship, 6);
        } else {
            engage(ship, target, OBJECTIVE_ENGAGE_ENEMY);
            SendWingmanCommandAcknowledgement(ship, 1);
        }
        break;
    case 4:
        allow_engage();
        if (g_aeShipObjective_00495f08[ship] == OBJECTIVE_HOLD_FORMATION) {
            reset_objective(ship, OBJECTIVE_BREAK_FORMATION);
            SendWingmanCommandAcknowledgement(ship, 1);
        } else {
            SendWingmanCommandAcknowledgement(ship, 0);
        }
        break;
    case 6:
        disallow_engage();
        if (disobey_formation(ship) == 0) {
            SendWingmanCommandAcknowledgement(ship, 0);
        } else {
            reset_objective(ship, OBJECTIVE_HOLD_FORMATION);
            g_nAutoEngageTimer_00496130 = -150;
            SendWingmanCommandAcknowledgement(ship, 1);
        }
        break;
    case 5:
        disallow_engage();
        if (disobey_formation(ship) == 0) {
            alter_objective(ship, OBJECTIVE_BREAK_FORMATION);
            SendWingmanCommandAcknowledgement(ship, 0);
        } else {
            g_nAutoEngageTimer_00496130 = -150;
            SendWingmanCommandAcknowledgement(ship, 1);
        }
        break;
    case 7:
    case 8:
        g_bRadioSilence_0049b780 = 0;
        SendWingmanCommandAcknowledgement(ship, 1);
        if (command == 7)
            g_bRadioSilence_0049b780 = 1;
        else
            g_bRadioSilence_0049b780 = 0;
        break;
    case 11:
        cleanup_objectives();
        if (CanPlayerLand() != 0) {
            g_bLandingCommRequestPending_00492fa0 = 1;
            send_message(ship, 0x12);
        } else {
            send_message(ship, 0x13);
        }
        break;
    case 12:
        cleanup_objectives();
        if (CanPlayerLand() != 0) {
            g_bLandingCommRequestPending_00492fa0 = 1;
            send_message(ship, 0x14);
            g_bEjectionSequencePending_00493058 = 1;
        } else {
            send_message(ship, 0x15);
        }
        break;
    case 9:
        if (g_ucPendingEjectionTransition_0049b8ac != 0xff &&
            g_bEjectionAwaitingCommCommand_0049b8b4 != 0) {
            ejection_sequence(g_ucPendingEjectionTransition_0049b8ac, 1);
            g_ucPendingEjectionTransition_0049b8ac = 0xff;
            g_bEjectionAwaitingCommCommand_0049b8b4 = 0;
        } else if (g_asShipMissionIndex_00495d00[ship] !=
                   g_nHomeMissionShipIndex_005d1e22) {
            send_message(ship, 0x11);
        } else {
            cleanup_objectives();
            if (CanPlayerLand() != 0) {
                g_bLandingCommRequestPending_00492fa0 = 1;
                send_message(ship, 0x10);
            } else {
                send_message(ship, 0x11);
            }
        }
        break;
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
        if (RandomBelow(100) < 70 ||
            (g_asPilotLevel_00495d60[ship] == 5 &&
             g_asShipSide_004955d0[ship] == SIDE_KILRATHI)) {
            send_message(ship,
                         (signed char)(command + 0x19 -
                                       g_nEnemyTauntCommandBase_0049b76c));
            g_nEnemyTauntCommandBase_0049b76c =
                (short)(g_nEnemyTauntCommandBase_0049b76c | 1);
        }
        if (g_acShipTarget_00495f20[ship] != requester &&
            IsShipRouting(ship) == 0)
            engage(ship, requester, OBJECTIVE_ENGAGE_ENEMY);
        break;
    case 10:
        if (g_asShipSide_004955d0[target] != SIDE_KILRATHI) {
            send_message(ship, 0xf);
            g_asShipIdentified_00496078[ship] = 1;
        }
        break;
    case 13:
        if (g_nYourWingman_0049346c != -1) {
            send_message(ship, 0xf);
            g_bDisplayWingmanTargetData_0049347c = 1;
        }
        break;
    }
}

/* Function start: 0x40FB80 */
unsigned short InitializeDIBScreenViewport(
    Viewport *viewport, unsigned short colour)
{
    short row;

    g_pDibScreenViewport_005d398c = viewport;
    g_pDibScreenAllocationState_005d3b4c =
        &g_nDibScreenAllocationState_005d3b40;
    g_nDibScreenAllocationState_005d3b40 = 0;
    g_pDibScreenViewportMirror_005d3b44 = viewport;
    viewport->left = 0;
    viewport->top = 0;
    viewport->right = 319;
    viewport->bottom = 199;
    if (g_nSpacePaletteFadeMode_004901e8 != 0x13) {
        SystemDebugPrintf("== BAD alloc_screen == : type: '%d'\n",
                          (int)(short)g_nSpacePaletteFadeMode_004901e8);
        exit(1);
        colour = (unsigned short)-1;
    }
    viewport->pixels = GetDIBPixelBuffer();
    viewport->rowOffsets = g_awDibScreenRowOffsets_005d39a0;
    for (row = 0; row < 202; row++)
        g_awDibScreenRowOffsets_005d39a0[row] =
            (unsigned short)(row * 320);
    if ((short)colour != -1)
        ClearViewport(viewport, colour);
    return 1;
}

/* Function start: 0x40FC7E */
/* Initialises a 320x200 viewport record (0x13F == 319, 199) then validates it. */
void InitFullScreenViewport(Viewport *viewport, short arg)
{
    g_pFullScreenViewport_005d3990 = viewport;
    viewport->left = 0;
    viewport->right = 319;
    viewport->top = 0;
    viewport->bottom = 199;
    AllocateViewport(viewport, arg, 0);
}

/* Function start: 0x453A70 */
unsigned int GetPacketSize(const char *filename, short section)
{
    PacketSectionHandle handle;
    unsigned int size;

    size = (unsigned int)-1;
    if (OpenPacketSection(filename, section, &handle) != 0) {
        switch (handle.compression) {
        case 1:
            ReadPacketSectionData(&handle, &size, 4);
            if (g_nPacketError_0049ca90 != 0)
                size = (unsigned int)-1;
            break;
        case 2:
            size = handle.dataSize;
            break;
        default:
            size = handle.dataSize;
            break;
        }
        CloseDataFileByHandle((unsigned short *)&handle);
    }
    return size;
}

/* Function start: 0x421144 */
int GetNearHeapBlockSize(void *pointer)
{
    int size;
    int address;
    NearHeapBlock *block;
    int descriptorAddress;

    size = 0;
    address = DosFarPtrToNear(pointer);
    descriptorAddress =
        g_nNearHeapSize_005d3050 + g_nNearHeapBase_005d3054 - 8;
    while (g_nNearHeapFirstDescriptor_005d3058 <= descriptorAddress) {
        block = DosNearPtrToFar(descriptorAddress);
        if (block->address <= address &&
            block->address + (block->sizeAndFlags & 0xfffff) > address) {
            size = block->sizeAndFlags & 0xfffff;
            /* Each tag flag reserves bytes the caller cannot use. */
            if ((block->sizeAndFlags & 0x10000000) != 0)
                size--;
            else if ((block->sizeAndFlags & 0x20000000) != 0)
                size -= 15;
            break;
        }
        descriptorAddress -= 8;
    }
    return size;
}

/* Function start: 0x458E20 */
unsigned short IsSoundHardwarePresent(short device)
{
    GetSoundHardwareFlag();
    return 1;
}

/* Function start: 0x458E39 */
void MessagePumpHook(int mode)
{
    TimerStopHook();
}

/* Function start: 0x458E49 */
void *PushMemoryStackFrame(void *memory, int offset)
{
    int index;

    if (offset != 0) {
        printf("push %p by %d\n", memory, offset);
        if (g_nPacketHandleCount_005d1020 == 0x1000)
            exit_squadron("qq mem push overflow");
        index = g_nPacketHandleCount_005d1020;
        g_aiPacketHandleOffsets_005cd020[index] = offset;
        if (offset < 0) {
            memory = (unsigned char *)memory - offset;
            g_apPacketHandles_005c9020[index] = memory;
            g_nPacketHandleCount_005d1020 = index + 1;
            return memory;
        }
        memory = (unsigned char *)memory + offset;
        g_apPacketHandles_005c9020[index] = memory;
        g_nPacketHandleCount_005d1020 = index + 1;
        return memory;
    }
    return memory;
}

/* Function start: 0x458EED */
int IsPushedPacketHandle(void *handle)
{
    int index;

    for (index = 0; index < g_nPacketHandleCount_005d1020; index++) {
        if (g_apPacketHandles_005c9020[index] == handle) {
            if (g_aiPacketHandleOffsets_005cd020[index] < 0)
                return 1;
            return 0;
        }
    }
    return 0;
}

/* Function start: 0x458F5A */
void *MapPacketHandleToBlock(void *handle)
{
    int i;

    for (i = 0; i < g_nPacketHandleCount_005d1020; i++) {
        if (g_apPacketHandles_005c9020[i] == handle) {
            if (g_aiPacketHandleOffsets_005cd020[i] < 0)
                handle = (unsigned char *)handle +
                    g_aiPacketHandleOffsets_005cd020[i];
            else
                handle = (unsigned char *)handle -
                    g_aiPacketHandleOffsets_005cd020[i];
            g_nPacketHandleCount_005d1020--;
            g_apPacketHandles_005c9020[i] =
                g_apPacketHandles_005c9020[g_nPacketHandleCount_005d1020];
            g_aiPacketHandleOffsets_005cd020[i] =
                g_aiPacketHandleOffsets_005cd020[
                    g_nPacketHandleCount_005d1020];
            return MapPacketHandleToBlock(handle);
        }
    }
    return handle;
}

#ifndef WC1_SDL
#pragma function(memcpy)
#endif

/* Function start: 0x45901D */
void *AllocateTaggedMemory(unsigned int size, short flags)
{
    void *memory;

#ifdef WC1_SDL
    if ((flags & 0x40) != 0)
        size += 8 + sizeof(unsigned char *);
#else
    if ((flags & 0x40) != 0)
        size += 8;
#endif
    memory = AllocateGuardedMemory(size);
    if ((flags & 0x40) != 0) {
#ifdef WC1_SDL
        *(unsigned char **)memory = 0;
        memcpy((unsigned char *)memory + sizeof(unsigned char *),
               g_abTaggedAllocationPrefix_0049ca40,
               sizeof(g_abTaggedAllocationPrefix_0049ca40));
        memory = PushMemoryStackFrame(
            memory, -(int)(8 + sizeof(unsigned char *)));
#else
        memcpy(memory, g_abTaggedAllocationPrefix_0049ca40,
               sizeof(g_abTaggedAllocationPrefix_0049ca40));
        memory = PushMemoryStackFrame(memory, -8);
#endif
    }
    return memory;
}

#ifndef WC1_SDL
#pragma intrinsic(memcpy)
#endif

/* Function start: 0x459082 */
void ReleasePacketHandle(void *handle)
{
    int group;
    int entry;

    if (handle != 0) {
        for (group = 0; group < 4; group++) {
            if (g_apPacketReferenceGroups_0049b898[group] != 0) {
                for (entry = 0; entry < 0x25; entry++) {
                    if (g_apPacketReferenceGroups_0049b898[group][entry] ==
                        handle)
                        g_apPacketReferenceGroups_0049b898[group][entry] = 0;
                }
            }
        }
        handle = MapPacketHandleToBlock(handle);
        FreeGuardedAllocation(handle);
    }
}

/* Function start: 0x459134 */
/* Tail-jump thunk to 0x004362E0, not a constant return -- Ghidra followed the
 * jump and folded the callee's body into the display.  The optimized compiler
 * emits the tail jump from this forwarding return. */
unsigned int GetAvailableFarMemoryByType(short memoryType)
{
    (void)memoryType;
    return GetAvailableFarMemory();
}

/* Function start: 0x459149 */
unsigned int GetLargestFreeMemoryBlockByType(short memoryType)
{
    (void)memoryType;
    return GetLargestFreeMemoryBlock();
}

#ifndef WC1_SDL
#pragma function(strcat)
#endif

/* Function start: 0x429261 */
void RunCampaignScript(short campaignSlot)
{
    void *scenePacket;
    unsigned int availableMainMemory;
    unsigned int largestMainMemoryBlock;
    short fontIndex;
    char campaignNumber[4];
    short savedInputMode;
    char campaignFileName[13];
    unsigned int availableFarMemory;
    unsigned int largestFarMemoryBlock;

    savedInputMode = g_nUiInputMode_005c8d3c;
    memcpy(campaignFileName, "campaign.s0", 12);
    campaignFileName[12] = 0;
    if (g_bSkipCampaignVideo_0049c270 != 0)
        return;
    SetMenuInputPump();
    strcat(campaignFileName,
           _itoa((int)campaignSlot, campaignNumber, 10));
    scenePacket = FetchDiskPacketRetrying(
        campaignFileName, 0, 0);
    for (fontIndex = 0; fontIndex < 4; fontIndex++) {
        if (g_apTextFonts_005d2200[fontIndex] != 0) {
            ReleasePacketHandle(g_apTextFonts_005d2200[fontIndex]);
            g_apTextFonts_005d2200[fontIndex] = 0;
        }
    }
    availableMainMemory = GetAvailableMainMemory();
    largestMainMemoryBlock = GetLargestMainMemoryBlock();
    availableFarMemory = GetAvailableFarMemory();
    largestFarMemoryBlock = GetLargestFreeMemoryBlock();
    if (g_bCutsceneViewportPreallocated_00499c4c == 0)
        ClearViewport(&g_stModalSourceViewport_005d2c50, 0);
    if (g_bSkipCampaignScenes_0049cc78 == 0) {
        g_bSceneEscapeRequested_0049d4b0 = 0;
        ParseCutsceneContainer(scenePacket);
        g_bSceneDisplayUpdateActive_00499bb8 = 1;
        DisableMouseCursorDrawing();
        g_nUiInputMode_005c8d3c = 0;
        RunLoadedCutscene();
        g_nUiInputMode_005c8d3c = savedInputMode;
        g_bSceneDisplayUpdateActive_00499bb8 = 0;
        g_bSceneEscapeRequested_0049d4b0 = 0;
    }
    availableMainMemory = GetAvailableMainMemory();
    largestMainMemoryBlock = GetLargestMainMemoryBlock();
    availableFarMemory = GetAvailableFarMemory();
    largestFarMemoryBlock = GetLargestFreeMemoryBlock();
    ReleasePacketHandle(scenePacket);
}

#ifndef WC1_SDL
#pragma intrinsic(strcat)
#endif

/* Function start: 0x421210 */
short UpdateInputDeviceTransitions(short raw)
{
    short changed;
    short buttons;
    short device;

    device = g_nActiveInputDevice_005d1726;
    if (raw == 0)
        ReadCalibratedJoystick();
    else
        SampleJoystickDevice(&g_aInputDeviceSamples_005d1780[device],
                             device, 0);

    g_asInputButton2DoubleClick_005d3048[device] = 0;
    g_asInputButton1DoubleClick_005d3030[device] =
        g_asInputButton2DoubleClick_005d3048[device];
    g_asInputButton2Changed_005d3038[device] = 0;
    g_asInputButton1Changed_005d3034[device] =
        g_asInputButton2Changed_005d3038[device];
    buttons = (short)g_aInputDeviceSamples_005d1780[device].buttons;
    if (buttons != g_asPreviousInputButtons_005d303c[device]) {
        changed = (short)((buttons & 1) ^
            (g_asPreviousInputButtons_005d303c[device] & 1));
        if (changed != 0) {
            g_asInputButton1Changed_005d3034[device]++;
            if ((buttons & 1) != 0) {
                if (g_nInputClock_005c84a8 -
                    g_anInputButton1PressTime_005d3040[device] <=
                        g_nInputDoubleClickInterval_00493050 *
                        g_nInputTickScale_005c8d24)
                    g_asInputButton1DoubleClick_005d3030[device]++;
                g_anInputButton1PressTime_005d3040[device] =
                    g_nInputClock_005c84a8;
            }
        }
        changed = (short)((buttons & 2) ^
            (g_asPreviousInputButtons_005d303c[device] & 2));
        if (changed != 0) {
            g_asInputButton2Changed_005d3038[device]++;
            if ((buttons & 2) != 0) {
                if (g_nInputClock_005c84a8 -
                    g_anInputButton2PressTime_005d3028[device] <=
                        g_nInputDoubleClickInterval_00493050 *
                        g_nInputTickScale_005c8d24)
                    g_asInputButton2DoubleClick_005d3048[device]++;
                g_anInputButton2PressTime_005d3028[device] =
                    g_nInputClock_005c84a8;
            }
        }
        g_asPreviousInputButtons_005d303c[device] = buttons;
    }
    return 1;
}

/* Function start: 0x421409 */
void PollJoystickButtonEvents(void)
{
    short doubleClick;

    if (g_nActiveInputDevice_005d1726 == -1)
        return;
    if (g_bInputPollingGuard_005d304c != 0)
        return;
    g_bInputPollingGuard_005d304c++;
    UpdateInputDeviceTransitions(1);
    if (g_asInputButton1Changed_005d3034
            [g_nActiveInputDevice_005d1726] != 0) {
        doubleClick = 0;
        if (g_asInputButton1DoubleClick_005d3030
                [g_nActiveInputDevice_005d1726] != 0)
            doubleClick = 3;
        QueueInputEventAtCursor(
            (g_aInputDeviceSamples_005d1780
                [g_nActiveInputDevice_005d1726].buttons & 1) + 1,
            0, doubleClick);
    }
    if (g_asInputButton2Changed_005d3038
            [g_nActiveInputDevice_005d1726] != 0) {
        doubleClick = 0;
        if (g_asInputButton2DoubleClick_005d3048
                [g_nActiveInputDevice_005d1726] != 0)
            doubleClick = 3;
        QueueInputEventAtCursor(
            ((g_aInputDeviceSamples_005d1780
                [g_nActiveInputDevice_005d1726].buttons >> 1) & 1) + 1,
            1, doubleClick);
    }
    g_bPersonnelSecondaryButton_005c8d05 =
        (unsigned char)g_aInputDeviceSamples_005d1780
            [g_nActiveInputDevice_005d1726].buttons;
    g_bPersonnelPrimaryButton_005c8d04 =
        g_bPersonnelSecondaryButton_005c8d05;
    g_bInputPollingGuard_005d304c--;
}

/* Function start: 0x421530 */
void PollMenuInputDevices(void)
{
    signed char doubleClick;
    unsigned char changes;

    changes = 0;
    if (g_nActiveInputDevice_005d1726 == -1)
        return;
    g_nPersonnelMouseX_005c8d00 = g_nQueuedInputX_005c83f0;
    g_nPersonnelMouseY_005c8d02 = g_nQueuedInputY_005c83f2;
    if (g_bInputPollingGuard_005d304c == 0) {
        g_bInputPollingGuard_005d304c++;
        UpdateInputDeviceTransitions(0);

        if (g_asInputButton1Changed_005d3034
                [g_nActiveInputDevice_005d1726] != 0) {
            g_nPersonnelMouseX_005c8d00 +=
                g_aInputDeviceSamples_005d1780
                    [g_nActiveInputDevice_005d1726].x *
                g_nMenuPointerSpeed_00493054;
            g_nPersonnelMouseY_005c8d02 +=
                g_aInputDeviceSamples_005d1780
                    [g_nActiveInputDevice_005d1726].y *
                g_nMenuPointerSpeed_00493054;
            doubleClick = 0;
            if (g_asInputButton1DoubleClick_005d3030
                    [g_nActiveInputDevice_005d1726] != 0)
                doubleClick = 3;
            QueueInputEventAtCursor(
                (g_aInputDeviceSamples_005d1780
                    [g_nActiveInputDevice_005d1726].buttons & 1) + 1,
                0, (short)doubleClick);
            changes++;
        }
        if (g_asInputButton2Changed_005d3038
                [g_nActiveInputDevice_005d1726] != 0) {
            g_nPersonnelMouseX_005c8d00 +=
                g_aInputDeviceSamples_005d1780
                    [g_nActiveInputDevice_005d1726].x *
                g_nMenuPointerSpeed_00493054;
            g_nPersonnelMouseY_005c8d02 +=
                g_aInputDeviceSamples_005d1780
                    [g_nActiveInputDevice_005d1726].y *
                g_nMenuPointerSpeed_00493054;
            doubleClick = 0;
            if (g_asInputButton1DoubleClick_005d3030
                    [g_nActiveInputDevice_005d1726] != 0)
                doubleClick = 3;
            QueueInputEventAtCursor(
                ((g_aInputDeviceSamples_005d1780
                    [g_nActiveInputDevice_005d1726].buttons >> 1) & 1) + 1,
                1, (short)doubleClick);
            changes++;
        }
        if (changes == 0) {
            g_nPersonnelMouseX_005c8d00 +=
                g_aInputDeviceSamples_005d1780
                    [g_nActiveInputDevice_005d1726].x *
                g_nMenuPointerSpeed_00493054;
            g_nPersonnelMouseY_005c8d02 +=
                g_aInputDeviceSamples_005d1780
                    [g_nActiveInputDevice_005d1726].y *
                g_nMenuPointerSpeed_00493054;
            changes = (unsigned char)(
                g_aInputDeviceSamples_005d1780
                    [g_nActiveInputDevice_005d1726].y |
                g_aInputDeviceSamples_005d1780
                    [g_nActiveInputDevice_005d1726].x);
            if (changes != 0) {
                FlushInputEvents();
                g_nPersonnelMouseX_005c8d00 =
                    g_nPersonnelMouseX_005c8d00 > 0
                        ? g_nPersonnelMouseX_005c8d00 : 0;
                g_nPersonnelMouseX_005c8d00 =
                    g_nPersonnelMouseX_005c8d00 < 319
                        ? g_nPersonnelMouseX_005c8d00 : 319;
                g_nPersonnelMouseY_005c8d02 =
                    g_nPersonnelMouseY_005c8d02 > 0
                        ? g_nPersonnelMouseY_005c8d02 : 0;
                g_nPersonnelMouseY_005c8d02 =
                    g_nPersonnelMouseY_005c8d02 < 199
                        ? g_nPersonnelMouseY_005c8d02 : 199;
                SetMousePosition(g_nPersonnelMouseX_005c8d00,
                                 g_nPersonnelMouseY_005c8d02);
            }
        }

        g_bPersonnelSecondaryButton_005c8d05 =
            (unsigned char)g_aInputDeviceSamples_005d1780
                [g_nActiveInputDevice_005d1726].buttons;
        g_bPersonnelPrimaryButton_005c8d04 =
            g_bPersonnelSecondaryButton_005c8d05;
        g_nPersonnelMouseX_005c8d00 =
            g_nPersonnelMouseX_005c8d00 > 0
                ? g_nPersonnelMouseX_005c8d00 : 0;
        g_nPersonnelMouseX_005c8d00 =
            g_nPersonnelMouseX_005c8d00 < 319
                ? g_nPersonnelMouseX_005c8d00 : 319;
        g_nPersonnelMouseY_005c8d02 =
            g_nPersonnelMouseY_005c8d02 > 0
                ? g_nPersonnelMouseY_005c8d02 : 0;
        g_nPersonnelMouseY_005c8d02 =
            g_nPersonnelMouseY_005c8d02 < 199
                ? g_nPersonnelMouseY_005c8d02 : 199;
        if (changes != 0) {
            if (g_bSceneDisplayUpdateActive_00499bb8 == 0)
                DisableMouseCursorDrawing();
            g_nQueuedInputX_005c83f0 = g_nPersonnelMouseX_005c8d00;
            g_nQueuedInputY_005c83f2 = g_nPersonnelMouseY_005c8d02;
            g_bQueuedPrimaryButton_005c83f4 =
                g_bPersonnelPrimaryButton_005c8d04;
            g_bQueuedSecondaryButton_005c83f5 =
                g_bPersonnelSecondaryButton_005c8d05;
            g_wQueuedInputFlags_005c83f7 = g_wPersonnelInputFlags_005c8d07;
            if (g_bSceneDisplayUpdateActive_00499bb8 == 0)
                EnableMouseCursorDrawing();
        }
        g_bInputPollingGuard_005d304c--;
    }
}

/* Function start: 0x447369 */
void ResetCommMenuChoices(short reuse)
{
    short choice;

    if (reuse == 0) {
        for (choice = 0; choice < 7; choice++) {
            g_apszCommMenuChoiceText_005d19a0[choice] = 0;
            g_abCommMenuChoiceCommand_005d1948[choice] = -1;
        }
    }
    g_nCommMenuChoiceCount_0049b770 = 0;
    g_nCommMenuReuseMode_0049b774 = reuse;
}

/* Function start: 0x4473D1 */
short IsCommMenuIdle(void)
{
    return g_nCommMenuChoiceCount_0049b770 == 0;
}

/* Function start: 0x4473FC */
void AppendCommMenuChoice(const char *text, short command)
{
    if (g_nCommMenuReuseMode_0049b774 == 1 &&
        (g_apszCommMenuChoiceText_005d19a0[
             g_nCommMenuChoiceCount_0049b770] != text ||
         g_abCommMenuChoiceCommand_005d1948[
             g_nCommMenuChoiceCount_0049b770] != command))
        g_nCommMenuReuseMode_0049b774 = 0;
    g_apszCommMenuChoiceText_005d19a0[
        g_nCommMenuChoiceCount_0049b770] = text;
    g_abCommMenuChoiceCommand_005d1948[
        g_nCommMenuChoiceCount_0049b770] = (signed char)command;
    g_nCommMenuChoiceCount_0049b770++;
}

/* Function start: 0x447479 */
void SendCommMenuChoice(short i)
{
    if (g_apCommunicationTextPackets_005d17c0[i] == 0)
        g_apCommunicationTextPackets_005d17c0[i] =
            LoadPacketAllocated("communic.you", i);
    AppendCommMenuChoice(g_apCommunicationTextPackets_005d17c0[i], i);
}

/* Function start: 0x4474CA */
void OpenCommMenuForTarget(const char *heading, const char *message)
{
    CockpitMessage(message, g_abGamePaletteReservedColours_0049cb54[4], -1);
    g_pszCommMenuHeading_005d1950 = heading;
}

/* Function start: 0x4474F4 */
short IsCommChoiceMenuOpen(void)
{
    return get_mode(1) == 4;
}

/* Function start: 0x447526 */
void OpenCommRecipientMenu(void)
{
    push_mode(1, 4);
    g_cPendingCommMenuAction_0049b77c = 1;
}

/* Function start: 0x447544 */
void CloseCommChoiceMenu(void)
{
    if (get_mode(1) == 4)
        pop_mode(1);
}

/* Function start: 0x44756F */
short wingman_dead(void)
{
    return g_nYourWingman_0049346c == -1;
}

/* Function start: 0x44759B */
short have_target(void)
{
    return unactive(g_acShipTarget_00495f20[0]) == 0;
}

/* Function start: 0x4475D3 */
short CanOpenCommMenu(void)
{
    return have_target() || !wingman_dead();
}

/* Function start: 0x44760F */
void SelectCommRecipient(short recipient)
{
    g_cCommMenuRecipient_0049b790 = recipient;
    g_cPendingCommMenuAction_0049b77c = 2;
}

/* Function start: 0x447629 */
void BuildCommunicationRecipientMenu(void)
{
    short target;

    ResetCommMenuChoices(g_nCommMenuReuseMode_0049b774);
    OpenCommMenuForTarget(g_szCommRecipientHeading_0049b7cc,
                          g_szSelectCommPrompt_0049b7c4);
    if (wingman_dead() != 0) {
        SelectCommRecipient(g_acShipTarget_00495f20[0]);
        return;
    }
    if (have_target() == 0 ||
        g_acShipTarget_00495f20[0] == g_nYourWingman_0049346c) {
        SelectCommRecipient(g_nYourWingman_0049346c);
        return;
    }
    AppendCommMenuChoice(
        g_apShipMissionRecord_00495da8[g_nYourWingman_0049346c]->name,
        1);
    target = g_acShipTarget_00495f20[0];
    if (target != -1) {
        if (g_asShipSide_004955d0[target] == SIDE_KILRATHI &&
            g_aeObjectClass_00495328[target] >= OBJECT_CLASS_SHIP) {
            if (g_asShipIdentified_00496078[target] != 0)
                AppendCommMenuChoice(g_szEnemyTarget_0049b7f0, 2);
            else
                AppendCommMenuChoice(
                    g_szUnidentifiedEnemyTarget_0049b800, 2);
        } else if (g_asShipSide_004955d0[target] == SIDE_IMPERIAL) {
            if (g_aeObjectClass_00495328[target] >= OBJECT_CLASS_SHIP &&
                g_asShipMissionType_00495de8[target] ==
                    MISSION_TYPE_CANNED_SEQUENCE)
                AppendCommMenuChoice(g_apShipMissionRecord_00495da8[
                                         g_acShipTarget_00495f20[0]]->name,
                                     3);
            if ((g_aeObjectClass_00495328[target] == OBJECT_CLASS_SHIP &&
                 any_enemy(0, 14000) != 0) ||
                g_asShipMissionIndex_00495d00[target] ==
                    g_nHomeMissionShipIndex_005d1e22) {
                if (g_asShipIdentified_00496078[target] != 0)
                    AppendCommMenuChoice(g_apShipMissionRecord_00495da8[
                                             g_acShipTarget_00495f20[0]]->name,
                                         3);
                else
                    AppendCommMenuChoice(
                        g_szUnidentifiedFriendlyTarget_0049b810, 3);
            } else if (g_ucPendingEjectionTransition_0049b8ac != 0xff &&
                       g_bEjectionAwaitingCommCommand_0049b8b4 != 0 &&
                       g_asObjectType_00495298[target] >
                           OBJECT_TYPE_EJECTED_PILOT) {
                AppendCommMenuChoice(
                    g_apShipMissionRecord_00495da8[
                        g_acShipTarget_00495f20[0]]->name,
                    3);
            }
        }
    }
    SendCommMenuChoice(0);
}

#ifndef WC1_SDL
#pragma function(strcpy, strcat)
#endif

/* Function start: 0x447890 */
void BuildCommunicationCommandMenu(void)
{
    short object;

    if (g_nEnemyCommPilotIndex_005d179e != -1 &&
        g_acShipPortrait_00495d88[g_cCommMenuRecipient_0049b790] ==
            g_nEnemyCommPilotIndex_005d179e &&
        g_asShipSide_004955d0[g_cCommMenuRecipient_0049b790] ==
            SIDE_KILRATHI) {
        g_nEnemyTauntCommandBase_0049b76c =
            (g_nEnemyTauntCommandBase_0049b76c & 1) |
            g_nEnemyCommCommandBase_005d179c;
    } else {
        g_nEnemyTauntCommandBase_0049b76c =
            (g_nEnemyTauntCommandBase_0049b76c & 1) | 20;
    }
    ResetCommMenuChoices(g_nCommMenuReuseMode_0049b774);
    if (g_asShipIdentified_00496078[g_cCommMenuRecipient_0049b790] == 0) {
        SendCommMenuChoice(10);
    } else {
        if (g_cCommMenuRecipient_0049b790 == g_nYourWingman_0049346c ||
            (g_aeObjectClass_00495328[g_cCommMenuRecipient_0049b790] ==
                 OBJECT_CLASS_SHIP &&
             g_asShipSide_004955d0[g_cCommMenuRecipient_0049b790] ==
                 SIDE_IMPERIAL)) {
            if (g_aeShipObjective_00495f08[
                    g_cCommMenuRecipient_0049b790] ==
                    OBJECTIVE_HOLD_FORMATION &&
                any_enemy(0, 14000) != 0)
                SendCommMenuChoice(4);
            if (have_target() != 0 &&
                g_asShipSide_004955d0[g_acShipTarget_00495f20[0]] ==
                    SIDE_KILRATHI)
                SendCommMenuChoice(1);
            for (object = 1; object < 10; object++) {
                if (g_aeObjectClass_00495328[object] ==
                        OBJECT_CLASS_SHIP &&
                    g_acShipTarget_00495f20[object] == 0) {
                    get_facing_range_from_object(0, object);
                    if (g_nFacingToTarget_00493194 < -70 &&
                        g_nTargetFacing_00493198 > 90 &&
                        g_nTargetRange_0049319c < 4000) {
                        SendCommMenuChoice(2);
                        break;
                    }
                }
            }
            if (g_cCommMenuRecipient_0049b790 ==
                g_nYourWingman_0049346c) {
                if (g_nAutoEngageTimer_00496130 != -1)
                    SendCommMenuChoice(5);
                else if (g_aeShipObjective_00495f08[
                             g_nYourWingman_0049346c] !=
                         OBJECTIVE_HOLD_FORMATION)
                    SendCommMenuChoice(6);
            }
            if (g_bRadioSilence_0049b780 == 0)
                SendCommMenuChoice(7);
            else
                SendCommMenuChoice(8);
            SendCommMenuChoice(3);
        }
        if (g_bEjectionAwaitingCommCommand_0049b8b4 != 0 &&
            g_ucPendingEjectionTransition_0049b8ac != 0xff) {
            if (g_asShipSide_004955d0[g_cCommMenuRecipient_0049b790] ==
                    SIDE_IMPERIAL &&
                g_asObjectType_00495298[
                    g_cCommMenuRecipient_0049b790] >
                    OBJECT_TYPE_EJECTED_PILOT)
                SendCommMenuChoice(9);
        } else {
            if (g_asShipSide_004955d0[g_cCommMenuRecipient_0049b790] ==
                SIDE_IMPERIAL) {
                if ((g_asShipMissionIndex_00495d00[
                         g_cCommMenuRecipient_0049b790] ==
                         g_nHomeMissionShipIndex_005d1e22 ||
                     g_asShipMissionIndex_00495d00[
                         g_cCommMenuRecipient_0049b790] ==
                         g_stMissionHeader_005d3e70.wingmanMissionShip) &&
                    g_bLandingCommRequestPending_00492fa0 == 0) {
                    if (g_aObjectTypeData_00496d30[
                            g_acObjectType_00493980[
                                g_cCommMenuRecipient_0049b790]]
                            .collisionRadius < 500)
                        SendCommMenuChoice(11);
                    else
                        SendCommMenuChoice(9);
                }
                if (g_asShipMissionType_00495de8[
                        g_cCommMenuRecipient_0049b790] ==
                        MISSION_TYPE_CANNED_SEQUENCE &&
                    g_bLandingCommRequestPending_00492fa0 == 0)
                    SendCommMenuChoice(12);
            }
        }
        if (g_asShipSide_004955d0[g_cCommMenuRecipient_0049b790] ==
            SIDE_KILRATHI) {
            SendCommMenuChoice(g_nEnemyTauntCommandBase_0049b76c);
            SendCommMenuChoice(g_nEnemyTauntCommandBase_0049b76c + 1);
            SendCommMenuChoice(g_nEnemyTauntCommandBase_0049b76c + 2);
        }
    }
    if (IsCommMenuIdle() != 0) {
        CloseCommChoiceMenu();
    } else if (g_nCommMenuChoiceCount_0049b770 < 5) {
        SendCommMenuChoice(0);
    }
    if (IsCommChoiceMenuOpen() != 0) {
        if (g_nCommMenuChoiceCount_0049b770 >= 5)
            strcpy(g_szCommMenuHeadingBuffer_005d1960,
                   g_szCommCompactHeadingPrefix_0049b820);
        else
            strcpy(g_szCommMenuHeadingBuffer_005d1960,
                   g_szCommHeadingPrefix_0049b834);
        if (g_asShipIdentified_00496078[
                g_cCommMenuRecipient_0049b790] == 0)
            strcat(g_szCommMenuHeadingBuffer_005d1960,
                   g_szUnidentifiedCommRecipient_0049b84c);
        else
            strcat(g_szCommMenuHeadingBuffer_005d1960,
                   g_apShipMissionRecord_00495da8[
                       g_cCommMenuRecipient_0049b790]->name);
        strcat(g_szCommMenuHeadingBuffer_005d1960,
               g_szCommHeadingNewline_0049b85c);
        OpenCommMenuForTarget(g_szCommMenuHeadingBuffer_005d1960,
                              g_szChooseCommPrompt_0049b860);
    }
}

#ifndef WC1_SDL
#pragma intrinsic(strcpy, strcat)
#endif

/* Function start: 0x447D85 */
void RefreshCommunicationMenu(void)
{
    if (IsCommChoiceMenuOpen() != 0) {
        if (g_cPendingCommMenuAction_0049b77c == 1)
            BuildCommunicationRecipientMenu();
        if (g_cPendingCommMenuAction_0049b77c == 2)
            BuildCommunicationCommandMenu();
        if (g_nCommMenuReuseMode_0049b774 == 0)
            InvalidateVduMode(1);
    }
}

/* Function start: 0x447DE3 */
void HandleCommunicationMenuRequest(void)
{
    if (IsCommChoiceMenuOpen() != 0)
        CloseCommChoiceMenu();
    if (message_showing() == 0 &&
        IsCommChoiceMenuOpen() == 0 && CanOpenCommMenu() != 0) {
        OpenCommRecipientMenu();
        ResetCommMenuChoices(0);
        RefreshCommunicationMenu();
    }
}

/* Function start: 0x447E47 */
void show_communications_disp(void)
{
    signed char choice;

    if (IsCommChoiceMenuOpen() == 0)
        HandleCommunicationMenuRequest();
    if (IsCommChoiceMenuOpen() != 0) {
        set_new_vdu(1);
        DrawTextAt(&g_stRightVduTextContext_005d2ce0,
                   g_stRightVduViewport_005d2b20.left,
                   g_stRightVduViewport_005d2b20.top,
                   g_pszCommMenuHeading_005d1950, 2);
        for (choice = 0; choice < g_nCommMenuChoiceCount_0049b770;
             choice++) {
            DrawFormattedText(g_szCommMenuChoiceFormat_0049b868,
                              (int)choice + 1,
                              g_apszCommMenuChoiceText_005d19a0[choice]);
        }
        g_nCommMenuReuseMode_0049b774 = 1;
    }
}

/* Function start: 0x447EF5 */
void Chosen_communicate_option(short choice)
{
    PlaySfxWaveFileByNumber(0x19, -1, 0);
    switch (g_cPendingCommMenuAction_0049b77c) {
    case 0:
        CloseCommChoiceMenu();
        break;
    case 1:
        if (g_abCommMenuChoiceCommand_005d1948[choice] == 0) {
            CloseCommChoiceMenu();
        } else {
            if (g_abCommMenuChoiceCommand_005d1948[choice] == 1)
                SelectCommRecipient(g_nYourWingman_0049346c);
            else
                SelectCommRecipient(g_acShipTarget_00495f20[0]);
            RefreshCommunicationMenu();
        }
        break;
    case 2:
        CloseCommChoiceMenu();
        request(0, (short)g_cCommMenuRecipient_0049b790,
                (short)g_abCommMenuChoiceCommand_005d1948[choice]);
        if (g_abCommMenuChoiceCommand_005d1948[choice] >=
            g_nEnemyTauntCommandBase_0049b76c) {
            g_nEnemyTauntCommandBase_0049b76c =
                (short)(g_nEnemyTauntCommandBase_0049b76c & 0xfe);
        }
        break;
    }
}

/* Function start: 0x448008 */
void IssueQuickCommCommand(short recipient, short command)
{
    request(0, recipient, command);
    if (g_nEnemyTauntCommandBase_0049b76c <= command)
        g_nEnemyTauntCommandBase_0049b76c =
            (short)(g_nEnemyTauntCommandBase_0049b76c & 0xfe);
}

/* Function start: 0x44804A */
void FreeCommDisplayResources(void)
{
    g_nCommSpeakerRating_0049b798 = -1;
    g_nCommSpeakerObject_0049b794 = -1;
    g_nCommPortraitIndex_0049b79c = -1;
}

/* Function start: 0x448070 */
void EndCommSessionWithWingman(void)
{
    if (g_pCommPortraitResource_0049b788 != 0)
        malf_noise(1, 1, g_abGamePaletteReservedColours_0049cb54[12],
                   23, 1);
    FreeCommDisplayResources();
    if (get_mode(1) == 6)
        pop_mode(1);
}

/* Function start: 0x4480C6 */
void EndCommMenu(void)
{
    RestoreHudMessageBackground();
    clear_message_time();
    if (get_mode(1) == 6)
        EndCommSessionWithWingman();
    g_pszPendingHudMessage_0049afec = 0;
}

/* Function start: 0x448100 */
void ShowCentredPrompt(char *text, unsigned short arg)
{
    DosStrcpy(g_szOnScreenMessageBuffer_005d1890, text);
    SetHudMessageText(g_szOnScreenMessageBuffer_005d1890,
                      g_bPrimaryViewBufferColour_0049cb50, arg);
}

/* Function start: 0x447300 */
void LoadCommPortraitResources(short portrait)
{
    if (g_bHighMemoryResourcesEnabled_005c80e4 != 0) {
        FreePacketAndClear(&g_pCommPortraitResource_0049b788, 4);
        g_pCommPortraitResource_0049b788 =
            FetchDiskPacketRetrying("vdu.v00", portrait, 4);
        if (g_pCommVduFrameResource_0049b78c == 0) {
            g_pCommVduFrameResource_0049b78c =
                FetchDiskPacketRetrying("vdu.v00", 0x1c, 4);
        }
    }
}

/* Function start: 0x448136 */
char *ExpandCommMessageTokens(const char *text)
{
    char *destination;
    const char *marker;
    short length;

    g_szTextScratchBuffer_005d1c40[0] = '\0';
    while ((marker = DosStrchr(text, '$')) != 0) {
        destination = DosStrchr(g_szTextScratchBuffer_005d1c40, '\0');
        while (marker != text)
            *destination++ = *text++;
        *destination = '\0';
        marker++;
        switch (*marker++) {
        case 'C':
            DosStrcat(g_szTextScratchBuffer_005d1c40,
                      g_stCurrentPilotProfile_00493408.callsign);
            break;
        case 'R':
            DosStrcat(g_szTextScratchBuffer_005d1c40,
                      g_apszPilotRankNames_0049a608[
                          g_stCurrentPilotProfile_00493408.rank]);
            if (g_szTextScratchBuffer_005d1c40[
                    DosStrlen(g_szTextScratchBuffer_005d1c40) - 1] == '.' &&
                *marker == '.') {
                length = DosStrlen(g_szTextScratchBuffer_005d1c40);
                g_szTextScratchBuffer_005d1c40[length - 1] = '\0';
            }
            break;
        case 'N':
        case 'P':
            DosStrcat(g_szTextScratchBuffer_005d1c40,
                      g_stCurrentPilotProfile_00493408.lastName);
            break;
        }
        text = marker;
    }
    DosStrcat(g_szTextScratchBuffer_005d1c40, text);
    return g_szTextScratchBuffer_005d1c40;
}

#pragma function(strcpy, strcat)

/* Function start: 0x4482CA */
void real_vid_transmit(short obj, short message)
{
    int packetSize;
    char packetName[16];
    char callsign[20];
    char text[84];
    char *expandedText;
    char numberText[8];

    g_nCommSpeakerObject_0049b794 = obj;
    g_nCommSpeakerRating_0049b798 =
        g_acShipPortrait_00495d88[g_nCommSpeakerObject_0049b794];
    g_nCommPortraitIndex_0049b79c = g_nCommSpeakerRating_0049b798;
    if (g_nCommPortraitIndex_0049b79c == -1)
        return;
    g_bCommSpeechPlaying_0049b7a0 = 0;
    g_nCommDeathSequenceFrame_0049ae84 = 0;
    if (g_bVideoImagesSuppressed_0049b784 == 0) {
        if (g_nCommPortraitIndex_0049b79c !=
            g_nLoadedCommPortraitPilot_004931c4) {
            LoadCommPortraitResources(g_nCommPortraitIndex_0049b79c);
            g_nLoadedCommPortraitPilot_004931c4 =
                g_nCommPortraitIndex_0049b79c;
        }
        if (g_pCommPortraitResource_0049b788 != 0) {
            push_mode(1, 6);
            malf_noise(1, 3, g_abGamePaletteReservedColours_0049cb54[12],
                       0x17, 1);
            g_nCommPortraitAnimationFrame_005d1d94 = 0;
            DrawSpriteDefault(&g_stRightVduViewport_005d2b20,
                              g_stRightVduViewport_005d2b20.left,
                              g_stRightVduViewport_005d2b20.top,
                              g_pCommPortraitResource_0049b788, 0);
            DrawSpriteDefault(&g_stRightVduViewport_005d2b20,
                              g_stRightVduViewport_005d2b20.left,
                              g_stRightVduViewport_005d2b20.top,
                              g_pCommPortraitResource_0049b788, 1);
        }
    }
    strcpy(packetName, "communic.s");
    if (g_nCommPortraitIndex_0049b79c < 10)
        strcat(packetName, "0");
    strcat(packetName,
           _itoa((int)g_nCommPortraitIndex_0049b79c, numberText, 10));
    packetSize = GetNamedPacketSize(packetName, message);
    g_nCommMessageIndex_0049ae88 = message;
    if (g_bSpeechCacheEnabled_005c8de8 != 0 && packetSize > 0 &&
        packetSize < g_wSpeechCacheCodeBytes_0048e0e0) {
        DismissHudMessageIfShowing();
        LoadAndPlaySpeechPacket(packetName, message);
        g_bCommSpeechPlaying_0049b7a0 = 1;
        return;
    }
    /* The sampled line and its subtitle share a packet number: "communic.s"
     * carries the speech, "communic.t" the text. */
    packetName[9] = 't';
    if (GetNamedPacketSize(packetName, message) > 0) {
        LoadPacketIntoBuffer(packetName, message,
                             g_szCommMessageText_005d18f0, 0);
        if (g_asShipIdentified_00496078[obj] != 0)
            sprintf(callsign, g_apShipMissionRecord_00495da8[obj]->name);
        else
            sprintf(callsign, "UNKNOWN");
        sprintf(text, g_szCommSpeakerTextFormat_0049b888, callsign,
                g_szCommMessageText_005d18f0);
        expandedText = ExpandCommMessageTokens(text);
        ShowCentredPrompt(expandedText, MeasureMessageWidth(text));
    }
}

#pragma intrinsic(strcpy, strcat)

/* Function start: WC2_UNMAPPED */
void __stdcall ShutdownVideoHook(short mode)
{
    ReleaseVideoResourcesHook();
}

/* Function start: 0x401859 */
short ReserveContiguousPaletteEntries(short entryCount)
{
    short firstEntry;
    short freeEntries;
    short entry;

    firstEntry = 0;
    freeEntries = firstEntry;
    for (entry = freeEntries; entry < 256; entry++) {
        if (g_awPaletteEntryAllocation_005d4050[entry] != 0) {
            freeEntries = 0;
            firstEntry = (short)(entry + 1);
        } else {
            freeEntries++;
        }
        if (entryCount == freeEntries) {
            for (entry = 0; entry < entryCount; entry++) {
                g_awPaletteEntryAllocation_005d4050[entry + firstEntry] =
                    entryCount;
            }
            return firstEntry;
        }
    }
    return -1;
}

/* Function start: 0x401922 */
void ReleaseContiguousPaletteEntries(short firstEntry)
{
    short count;
    short entry;

    count = g_awPaletteEntryAllocation_005d4050[firstEntry];
    for (entry = 0; count > entry; entry++)
        g_awPaletteEntryAllocation_005d4050[entry + firstEntry] = 0;
}

/* Function start: 0x401840 */
void ConfigureDefaultSpacePalette(short mode)
{
    (void)mode;
    g_nSpacePaletteFadeMode_004901e8 = 0x13;
    ApplySpacePaletteModeHook();
}

/* Function start: 0x401978 */
void PrintPaletteAllocationMap(void)
{
    short column;
    short index;
    short row;

    index = 0;
    for (row = index; row < 4; row++) {
        for (column = 0; column < 0x40; column++) {
            printf("%c",
                   g_awPaletteEntryAllocation_005d4050[index++] < 1 ?
                       '_' : '.');
        }
        printf("\n");
    }
}

/* Function start: 0x423480 */
short LoadJoystickCalibrationFile(short horizontalRange,
                                  short verticalRange,
                                  short horizontalDeadZone,
                                  short verticalDeadZone)
{
    unsigned short storedCentreX;
    unsigned short storedCentreY;
    unsigned short minimumX;
    unsigned short maximumX;
    unsigned short maximumY;
    unsigned short minimumY;
    short activeDevice;
    InputDeviceSample samples[2];
    int file;
    int failed;
    activeDevice = -1;
    failed = 1;
    if (horizontalRange == 0)
        horizontalRange++;
    if (verticalRange == 0)
        verticalRange++;
    g_nJoystickFailureValue_005d176c =
        g_nJoystickUnavailableSample_0048e054;
    SampleBothJoysticks(samples, g_nJoystickUnavailableSample_0048e054);
    if (samples[0].x != g_nJoystickUnavailableSample_0048e054 &&
        samples[0].y != g_nJoystickUnavailableSample_0048e054)
        activeDevice = 0;
    else if (samples[1].x != g_nJoystickUnavailableSample_0048e054 &&
             samples[1].y != g_nJoystickUnavailableSample_0048e054)
        activeDevice = 1;

    if (activeDevice != -1) {
        file = _open("j.cal", 0x8000);
        if (file != -1) {
            failed = _read(file, &activeDevice, 2) <= 0;
            if (failed == 0)
                failed = _read(file, &minimumX, 2) <= 0;
            if (failed == 0)
                failed = _read(file, &minimumY, 2) <= 0;
            if (failed == 0)
                failed = _read(file, &maximumX, 2) <= 0;
            if (failed == 0)
                failed = _read(file, &maximumY, 2) <= 0;
            if (failed == 0)
                failed = _read(file, &storedCentreX, 2) <= 0;
            if (failed == 0)
                failed = _read(file, &storedCentreY, 2) <= 0;
            g_nJoystickCentreX_005d1768 = (unsigned int)storedCentreX;
            g_nJoystickCentreY_005d1764 = (unsigned int)storedCentreY;
            _close(file);
            if (failed != 0)
                _unlink("j.cal");
        }

        if (failed != 0) {
            GetJoystickDevCaps(activeDevice,
                               (short *)&minimumX,
                               (short *)&maximumX,
                               (short *)&minimumY,
                               (short *)&maximumY);
            g_nJoystickCentreX_005d1768 =
                ((int)minimumX + (int)maximumX) / 2;
            g_nJoystickCentreY_005d1764 =
                ((int)minimumY + (int)maximumY) / 2;
        }

        if (g_nJoystickCentreX_005d1768 > 10) {
            g_nJoystickCalibrationMinimumX_005d2ff4 =
                g_nJoystickCentreX_005d1768 - 10;
            g_nJoystickCalibrationMaximumX_005d3000 =
                g_nJoystickCentreX_005d1768 + 10;
        } else {
            g_nJoystickCalibrationMaximumX_005d3000 =
                g_nJoystickCentreX_005d1768;
            g_nJoystickCalibrationMinimumX_005d2ff4 =
                g_nJoystickCalibrationMaximumX_005d3000;
        }
        if (g_nJoystickCentreY_005d1764 > 10) {
            g_nJoystickCalibrationMinimumY_005d2ff0 =
                g_nJoystickCentreY_005d1764 - 10;
            g_nJoystickCalibrationMaximumY_005d3004 =
                g_nJoystickCentreY_005d1764 + 10;
        } else {
            g_nJoystickCalibrationMaximumY_005d3004 =
                g_nJoystickCentreY_005d1764;
            g_nJoystickCalibrationMinimumY_005d2ff0 =
                g_nJoystickCalibrationMaximumY_005d3004;
        }

        g_nJoystickRightScale_005d175c =
            g_nJoystickCalibrationMinimumX_005d2ff4 /
            (int)horizontalRange;
        g_nJoystickLeftScale_005d1740 =
            g_nJoystickRightScale_005d175c;
        g_nJoystickDownScale_005d1760 =
            g_nJoystickCalibrationMinimumY_005d2ff0 /
            (int)verticalRange;
        g_nJoystickUpScale_005d173c =
            g_nJoystickDownScale_005d1760;
        if (g_nJoystickLeftScale_005d1740 == 0)
            g_nJoystickLeftScale_005d1740++;
        if (g_nJoystickUpScale_005d173c == 0)
            g_nJoystickUpScale_005d173c++;
        if (g_nJoystickRightScale_005d175c == 0)
            g_nJoystickRightScale_005d175c++;
        if (g_nJoystickDownScale_005d1760 == 0)
            g_nJoystickDownScale_005d1760++;

        g_nJoystickMinimumX_005d174c =
            g_nJoystickCentreX_005d1768 -
            horizontalRange * g_nJoystickLeftScale_005d1740;
        g_nJoystickMinimumY_005d1750 =
            g_nJoystickCentreY_005d1764 -
            verticalRange * g_nJoystickUpScale_005d173c;
        g_nJoystickMaximumX_005d1744 =
            horizontalRange * g_nJoystickLeftScale_005d1740 +
            g_nJoystickCentreX_005d1768;
        g_nJoystickMaximumY_005d1748 =
            verticalRange * g_nJoystickUpScale_005d173c +
            g_nJoystickCentreY_005d1764;
        g_nJoystickHorizontalDeadZone_005d172c = horizontalDeadZone;
        g_nJoystickVerticalDeadZone_005d1728 = verticalDeadZone;
    }
    g_nActiveInputDevice_005d1726 = activeDevice;
    return activeDevice;
}

/* Function start: 0x4238E9 */
short ReadCalibratedJoystick(void)
{
    short normalizedX;
    short normalizedY;
    short device;

    device = g_nActiveInputDevice_005d1726;
    if (g_nActiveInputDevice_005d1726 == -1)
        return 0;
    SampleJoystickDevice(
        &g_aInputDeviceSamples_005d1780[device], device,
        g_nJoystickFailureValue_005d176c);
    if (g_aInputDeviceSamples_005d1780[device].x ==
            g_nJoystickFailureValue_005d176c ||
        g_aInputDeviceSamples_005d1780[device].y ==
            g_nJoystickFailureValue_005d176c) {
        g_aInputDeviceSamples_005d1780[device].x =
            g_aInputDeviceSamples_005d1780[device].y =
            g_aInputDeviceSamples_005d1780[device].buttons = 0;
        g_nActiveInputDevice_005d1726 = -1;
        return 0;
    }

    g_nJoystickRawX_005d2ff8 =
        g_aInputDeviceSamples_005d1780[device].x;
    g_nJoystickRawY_005d2ffc =
        g_aInputDeviceSamples_005d1780[device].y;
    if (g_aInputDeviceSamples_005d1780[device].x <
        g_nJoystickMinimumX_005d174c)
        g_aInputDeviceSamples_005d1780[device].x =
            g_nJoystickMinimumX_005d174c;
    if (g_aInputDeviceSamples_005d1780[device].x >
        g_nJoystickMaximumX_005d1744)
        g_aInputDeviceSamples_005d1780[device].x =
            g_nJoystickMaximumX_005d1744;
    if (g_aInputDeviceSamples_005d1780[device].y <
        g_nJoystickMinimumY_005d1750)
        g_aInputDeviceSamples_005d1780[device].y =
            g_nJoystickMinimumY_005d1750;
    if (g_aInputDeviceSamples_005d1780[device].y >
        g_nJoystickMaximumY_005d1748)
        g_aInputDeviceSamples_005d1780[device].y =
            g_nJoystickMaximumY_005d1748;

    normalizedY = 0;
    normalizedX = normalizedY;
    if (g_aInputDeviceSamples_005d1780[device].x <
        g_nJoystickCentreX_005d1768) {
        normalizedX = (short)((g_nJoystickCentreX_005d1768 -
            g_aInputDeviceSamples_005d1780[device].x) /
            g_nJoystickLeftScale_005d1740);
        if (g_nJoystickHorizontalDeadZone_005d172c < normalizedX) {
            if (normalizedX != 0)
                normalizedX = (short)-normalizedX;
        } else {
            normalizedX = 0;
        }
    } else if (g_aInputDeviceSamples_005d1780[device].x >
               g_nJoystickCentreX_005d1768) {
        normalizedX = (short)((
            g_aInputDeviceSamples_005d1780[device].x -
            g_nJoystickCentreX_005d1768) /
            g_nJoystickRightScale_005d175c);
        if (normalizedX <= g_nJoystickHorizontalDeadZone_005d172c)
            normalizedX = 0;
    }

    if (g_aInputDeviceSamples_005d1780[device].y <
        g_nJoystickCentreY_005d1764) {
        normalizedY = (short)((g_nJoystickCentreY_005d1764 -
            g_aInputDeviceSamples_005d1780[device].y) /
            g_nJoystickUpScale_005d173c);
        if (g_nJoystickVerticalDeadZone_005d1728 < normalizedY) {
            if (normalizedY != 0)
                normalizedY = (short)-normalizedY;
        } else {
            normalizedY = 0;
        }
    } else if (g_aInputDeviceSamples_005d1780[device].y >
               g_nJoystickCentreY_005d1764) {
        normalizedY = (short)((
            g_aInputDeviceSamples_005d1780[device].y -
            g_nJoystickCentreY_005d1764) /
            g_nJoystickDownScale_005d1760);
        if (normalizedY <= g_nJoystickVerticalDeadZone_005d1728)
            normalizedY = 0;
    }
    g_aInputDeviceSamples_005d1780[device].x = (int)normalizedX;
    g_aInputDeviceSamples_005d1780[device].y = (int)normalizedY;
    return 1;
}

#pragma function(strlen)
/* Function start: 0x45CD2C */
void ThrottleFrameAndDrawFps(HDC dc)
{
    if (g_bFrameTimingInitialized_0049cebc == 0)
        SetDefaultFrameTiming();

    if (g_bShowFrameRate_0049c260 != 0) {
        sprintf(g_szFrameRateText_005b3950, "%f",
                g_fMeasuredFrameRate_005c3850);
        TextOutA(dc, 0, 0, g_szFrameRateText_005b3950,
                 strlen(g_szFrameRateText_005b3950));
    }

    if (g_pInputViewport_005c8403->pixels == g_pDibPixelBuffer_005b3978) {
        while (timeGetTime() < g_dwNextFrameDeadline_0049cea4) {
            Sleep(0);
            RefreshMouseCursorDisplay();
        }
    } else {
        while (timeGetTime() < g_dwNextFrameDeadline_0049cea4)
            Sleep(0);
    }

    if (g_bShowFrameRate_0049c260 != 0) {
        if (g_lPreviousFrameTick_0049ceac != 0) {
            g_fMeasuredFrameRate_005c3850 =
                1000.0f / ((float)timeGetTime() -
                           g_lPreviousFrameTick_0049ceac);
        }
        g_lPreviousFrameTick_0049ceac = timeGetTime();
        sprintf(g_szFrameRateText_005b3950, "%f",
                g_fMeasuredFrameRate_005c3850);
        TextOutA(dc, 0, 0, g_szFrameRateText_005b3950,
                 strlen(g_szFrameRateText_005b3950));
    }

    g_dwNextFrameDeadline_0049cea4 =
        timeGetTime() + g_nFramePeriodMilliseconds_005c343c;
}
