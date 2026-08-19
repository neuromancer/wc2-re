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
    int objectiveType;
    short objective;
    short home;
    short proximity;
    short index;
    short object;

    objective = 0;
    home = find_ship_index(g_stMissionHeader_005d3e70.homeMissionShip);
    proximity = g_aMissionNavPoints_00491e98[
        g_stMissionHeader_005d3e70.entryNavPoint].proximityRadius;
    objectiveType = g_aMissionObjectives_004932a8[objective].type;
    for (; objectiveType != -1;
         objective++,
         objectiveType = g_aMissionObjectives_004932a8[objective].type) {
        if (achieved(objective) == 0) {
            index = (short)g_aMissionObjectives_004932a8[objective].index;
            switch (objectiveType) {
            case 0:
            case 1:
            case 3:
                if (visited(objective) != 0)
                    flag_objective(objective, 2);
                break;
            case 2:
                if (g_aMissionShips_00492290[index].missionType ==
                        MISSION_TYPE_GOTO_WARP) {
                    if (g_aMissionShips_00492290[index].state == 2) {
                        flag_objective(objective, 2);
                        if (g_aMissionShips_00492290[index].type ==
                                OBJECT_TYPE_HORNET ||
                            g_aMissionShips_00492290[index].type ==
                                OBJECT_TYPE_DRAYMAN)
                            affect_mission_score(0, 5, -1);
                        else
                            affect_mission_score(0, 9, -1);
                    }
                } else if (g_aMissionShips_00492290[index].missionType ==
                               MISSION_TYPE_WARP_ARRIVE ||
                           g_aMissionShips_00492290[index].missionType ==
                               MISSION_TYPE_COME_HOME) {
                    object = find_ship_index(index);
                    if (object != -1 && home != -1 &&
                        distance_from_object(object, home) < proximity)
                        g_aMissionShips_00492290[index].state = 1;
                    if (g_aMissionShips_00492290[index].state == 1 &&
                        sighted(objective) != 0) {
                        flag_objective(objective, 2);
                        if (g_aMissionShips_00492290[index].type ==
                                OBJECT_TYPE_HORNET ||
                            g_aMissionShips_00492290[index].type ==
                                OBJECT_TYPE_DRAYMAN)
                            affect_mission_score(0, 5, -1);
                        else
                            affect_mission_score(0, 9, -1);
                    }
                } else if (g_aMissionShips_00492290[index].state == 0 &&
                           sighted(objective) != 0) {
                    flag_objective(objective, 2);
                    if (g_aMissionShips_00492290[index].type ==
                            OBJECT_TYPE_HORNET ||
                        g_aMissionShips_00492290[index].type ==
                            OBJECT_TYPE_DRAYMAN)
                        affect_mission_score(0, 5, -1);
                    else
                        affect_mission_score(0, 9, -1);
                }
                break;
            case 4:
                if (g_aMissionShips_00492290[index].state == 3)
                    flag_objective(objective, 2);
                break;
            }
        }
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
        return (short)report_kilrathi_rout(0);
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
    signed char *requesterTarget;
    short target;
    short object;

    requesterTarget = &g_acShipTarget_00495f20[requester];

    for (;;) {
        target = (short)*requesterTarget;
        switch (command) {
        case 1:
            allow_engage();
            if (IsEngagementTargetDisallowed(ship, target) == 0) {
                engage(ship, target, OBJECTIVE_ENGAGE_ENEMY);
                SendWingmanCommandAcknowledgement(ship, 1);
                return;
            }
            SendWingmanCommandAcknowledgement(ship, 0);
            return;
        case 2:
            allow_engage();
            target = -1;
            for (object = 0; object < 10; object++) {
                if (g_aeObjectClass_00495328[object] >=
                        OBJECT_CLASS_SHIP &&
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
                command = 9;
                continue;
            }
            engage(ship, target, OBJECTIVE_ENGAGE_ENEMY);
            SendWingmanCommandAcknowledgement(ship, 1);
            return;
        case 3:
            if (ShouldWingmanAcceptRoutCommand(ship, g_asPilotLevel_00495d60[ship]) != 0 &&
                try2rout(ship) != 0) {
                g_bEngageAllowed_0049612c = 0;
                SendWingmanCommandAcknowledgement(ship, 1);
                return;
            }
            SendWingmanCommandAcknowledgement(ship, 0);
            return;
        case 4:
        case 5:
        case 6:
            if (RandomBelow(100) < 70 ||
                ((signed char)g_acShipRating_0059cd80[ship] > 8 &&
                 (signed char)g_acShipRating_0059cd80[ship] < 13))
                send_message(ship, (signed char)(command - 2));
            if (g_acShipTarget_00495f20[ship] != requester &&
                IsShipRouting(ship) == 0) {
                engage(ship, requester, OBJECTIVE_ENGAGE_ENEMY);
                return;
            }
            break;
        case 7:
            allow_engage();
            if (g_aeShipObjective_00495f08[ship] ==
                    OBJECTIVE_HOLD_FORMATION) {
                reset_objective(ship, OBJECTIVE_BREAK_FORMATION);
                SendWingmanCommandAcknowledgement(ship, 1);
                return;
            }
            SendWingmanCommandAcknowledgement(ship, 0);
            return;
        case 8:
            disallow_engage();
            if (disobey_formation(ship) != 0) {
                alter_objective(ship, OBJECTIVE_BREAK_FORMATION);
                SendWingmanCommandAcknowledgement(ship, 0);
                return;
            }
            g_nAutoEngageTimer_00496130 = -150;
            SendWingmanCommandAcknowledgement(ship, 1);
            return;
        case 9:
            disallow_engage();
            if (disobey_formation(ship) != 0) {
                SendWingmanCommandAcknowledgement(ship, 0);
                return;
            }
            reset_objective(ship, OBJECTIVE_HOLD_FORMATION);
            g_nAutoEngageTimer_00496130 = -150;
            SendWingmanCommandAcknowledgement(ship, 1);
            return;
        case 10:
        case 11:
            g_bRadioSilence_0049b780 = 0;
            SendWingmanCommandAcknowledgement(ship, 1);
            g_bRadioSilence_0049b780 = command == 10;
            return;
        case 12:
            cleanup_objectives();
            if (CanPlayerLand() != 0) {
                g_bLandingCommRequestPending_00492fa0 = 1;
                send_message(ship, 8);
                return;
            }
            send_message(ship, 9);
            return;
        }
        return;
    }
}

/* Function start: WC2_UNMAPPED */
unsigned short __stdcall ShouldSuspendCursorForRect(const ShortRect *bounds)
{
    (void)bounds;
    return 0;
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
        _exit(1);
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
unsigned int __stdcall GetPacketSize(const char *filename, short section)
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
int GetFreeNearHeapBytes(void)
{
    NearHeapBlock *block;
    int descriptorAddress;
    int freeBytes;

    freeBytes = 0;
    descriptorAddress =
        g_nNearHeapBase_005d3054 + g_nNearHeapSize_005d3050 - 8;
    for (; descriptorAddress >= g_nNearHeapFirstDescriptor_005d3058;
         descriptorAddress -= 8) {
        block = DosNearPtrToFar(descriptorAddress);
        if ((block->sizeAndFlags & 0x80000000) == 0)
            freeBytes += block->sizeAndFlags & 0xfffff;
    }
    return freeBytes;
}

/* Function start: WC2_UNMAPPED */
void FrameStartHook(int mode)
{
    TimerResetHook();
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

/* Function start: WC2_UNMAPPED */
void CreateCannedSceneObject(short *object, short yaw, short unusedPitch,
                             short distance, unsigned char *shape,
                             short frame, short type, short scale)
{
    (void)unusedPitch;
    *object = find_vacant_3d_object();
    if (*object != -1) {
        g_aeObjectClass_00495328[*object] = OBJECT_CLASS_PLANET;
        init_ijk(63);
        alter_yaw(yaw, 63);
        ScaleFixedVector(&g_aShipForwardVector_00494208[63],
                         (int)distance << 8,
                         &g_aShipPosition_00494550[*object]);
        g_asObjectViewFrame_00493508[*object] = frame;
        g_asObjectScreenAngle_004936b8[*object] = type;
        g_acObjectType_00493980[*object] =
            (enum ObjectType)g_asObjectScreenAngle_004936b8[*object];
        g_asObjectScreenScale_00493a58[*object] = scale;
        g_apObjectShape_00493868[*object] = shape;
    }
}

/* Function start: WC2_UNMAPPED */
unsigned int ShowCampaignVictorySequence(void)
{
    CampaignVictoryProjectile projectiles[16];
    unsigned char *planetShape;
    unsigned char *projectileShape;
    const ShortVector *origin;
    CampaignVictoryProjectile *projectile;
    short planetObject;
    short spawnCountdown;
    short vacant[2];
    short vacantCount;
    short textIndex;
    short slot;
    volatile short frame;
    short animationFrame;
    short elapsed;
    int planetScale;
    int verticalOffset;
    int planetDepth;

    PreloadMusicTrackHook(0x21);
    spacetrack(0x21, 2, 1);
    InitializeConversationText();
    init_3Space_objects(0);
    g_nCannedSceneMode_0049021c = 2;
    g_bIntroSceneResourcesActive_00469d48_WC1_UNMAPPED = 0;
    set_up_action_sphere(0x12);
    planetShape =
        FetchDiskPacketRetrying(9, 3, 0);
    projectileShape =
        FetchDiskPacketRetrying(9, 2, 0);
    CreateCannedSceneObject(&planetObject, -4, 0, 30000,
                            planetShape, 0, 0, 0x50);
    g_nScriptedViewObject_0046a8d0_WC1_UNMAPPED = 1;
    initialize_scripted_view(g_asCampaignVictoryViewScript_0046c160_WC1_UNMAPPED);
    slot = 16;
    projectile = projectiles;
    do {
        projectile->scale = -1;
        projectile++;
        slot--;
    } while (slot != 0);

    planetDepth = -1500;
    frame = 0;
    g_bSceneEscapeRequested_0049d4b0 = 0;
    verticalOffset = -70000;
    g_nFrameSkipCountdown_0049d760 = 1;
    do {
        if (frame == 0)
            textIndex = 0;
        else if (frame == 100)
            textIndex = 1;
        else if (frame == 180)
            textIndex = 2;
        else
            textIndex = -1;
        if (textIndex != -1) {
            ClearViewport(&g_stConversationTextViewport_005a7570,
                          g_cSecondaryViewBufferColour_0049cb4c);
            SetTextContext(&g_stConversationTextContext_005d2d40);
            FormatTextBufferFromStart(
                g_szCampaignVictoryTextFormat_0046af24_WC1_UNMAPPED, 0, 160,
                g_apszCampaignVictoryText_0046ad90_WC1_UNMAPPED[textIndex]);
        }

        Update_3Space();
        if (Draw_3Space_Frame() != 0) {
            if (frame > 90)
                g_asObjectScreenScale_00493a58[planetObject]++;
            if (g_asObjectCollisionRadius_004950e8[61] < planetDepth) {
                slot = 0;
                projectile = projectiles;
                do {
                    if (projectile->scale != -1 &&
                        g_asObjectCollisionRadius_004950e8[61] <
                            projectile->depth) {
                        projectile->screenX =
                            (short)(projectile->x / projectile->depth);
                        projectile->screenY =
                            (short)(projectile->y / projectile->depth);
                        projectile->scale = 0x10000L / projectile->depth;
                        if (projectile->scale < 16) {
                            projectile->scale = -1;
                        } else {
                            DrawSpriteScaled(
                                &g_stViewBuffer_005d2b00,
                                (short)(projectile->screenX +
                                        g_nViewCenterX_005c80d8),
                                (short)(projectile->screenY +
                                        g_nViewCenterY_005c80da),
                                projectileShape, 1, 0,
                                (short)projectile->scale,
                                projectile->flip);
                            projectile->depth += 100;
                            projectile->y += 4000;
                        }
                    }
                    projectile++;
                    slot++;
                } while (slot < 16);

                if (frame < 170 && --spawnCountdown < 1) {
                    vacantCount = 0;
                    slot = 0;
                    projectile = projectiles;
                    do {
                        if (projectile->scale == -1) {
                            vacant[vacantCount] = slot;
                            vacantCount++;
                            if (vacantCount == 2)
                                break;
                        }
                        slot++;
                        projectile++;
                    } while (slot < 16);

                    if (vacantCount > 1) {
                        origin =
                            &g_aCampaignVictoryProjectileOrigins_0046adb0_WC1_UNMAPPED[
                                RandomBelowOrEqual(3)];
                        projectile = &projectiles[vacant[0]];
                        projectile->depth = planetDepth;
                        projectile->x =
                            ((origin->x * planetScale) >> 8) * planetDepth;
                        projectile->y =
                            ((origin->y * planetScale) >> 8) *
                                projectile->depth + verticalOffset;
                        projectile->scale = 0x100;
                        projectile->flip = origin->z;
                        projectile->depth += 40;

                        projectile = &projectiles[vacant[1]];
                        projectile->depth = planetDepth;
                        projectile->x =
                            (((origin->x - 4) * planetScale) >> 8) *
                                planetDepth;
                        projectile->y =
                            ((origin->y * planetScale) >> 8) *
                                projectile->depth + verticalOffset;
                        projectile->scale = 0x100;
                        projectile->flip = origin->z;
                        projectile->depth += 40;
                    }
                    spawnCountdown = 8;
                }

                planetScale = 0x40000L / planetDepth;
                DrawSpriteScaled(
                    &g_stViewBuffer_005d2b00, g_nViewCenterX_005c80d8,
                    (short)(g_nViewCenterY_005c80da +
                            verticalOffset / planetDepth),
                    planetShape, 0, 0, (short)planetScale, 0);
                verticalOffset += 200;
            }
            dump_buffer_to_screen();
            clear_view_buffer();
            MarkDibDirty();
            DIBslamReal();
        }
        planetDepth += 15;
        if (g_bSceneEscapeRequested_0049d4b0 == 1)
            break;
        MarkDibDirty();
        DIBslamReal();
        frame++;
    } while (frame < 250);

    ReleasePacketHandle(projectileShape);
    ReleasePacketHandle(planetShape);
    free_all_slots();
    ReleaseTextFont(0);
    free_3Space();
    if (g_bSceneEscapeRequested_0049d4b0 != 1) {
        planetShape =
            FetchDiskPacketRetrying(9, 5, 0);
        animationFrame = 1;
        ClearViewport(&g_stScreenViewport_005d21a0, g_cSecondaryViewBufferColour_0049cb4c);
        WaitForVerticalBlankThunk();
        DrawSpriteDefault(&g_stScreenViewport_005d21a0, 0, 0, planetShape, 0);
        elapsed = 0;
        WaitForWc1SceneAdvance(14, 0);
        do {
            SetFrameTimerPeriodDirect(8);
            DrawSpriteDefault(&g_stScreenViewport_005d21a0, 0, 0, planetShape,
                              animationFrame++);
            if (animationFrame > 17)
                animationFrame = 12;
            while ((short)IsFrameTickElapsed() == 0) {
                if (g_bSceneEscapeRequested_0049d4b0 != 0 || CheckEscaped() != 0) {
                    elapsed = 1000;
                    break;
                }
            }
            elapsed++;
            MarkDibDirty();
            DIBslamReal();
        } while (elapsed < 40);
        ReleasePacketHandle(planetShape);
        FadeViewportPaletteToColour(&g_stModalSourceViewport_005d2c50,
                                    g_cSecondaryViewBufferColour_0049cb4c, 1);
        ClearViewport(&g_stModalSourceViewport_005d2c50,
                      g_cSecondaryViewBufferColour_0049cb4c);
        MarkDibDirty();
        DIBslamReal();
        RestoreGamePalette();
    }

    g_bSceneEscapeRequested_0049d4b0 = 0;
    StopMusicUnlessSuppressed();
    ReleaseMusicTrackHook(0x21);
    g_bScriptedView_0046a8d4_WC1_UNMAPPED = 0;
    g_bIntroSceneResourcesActive_00469d48_WC1_UNMAPPED = 1;
    g_nCannedSceneMode_0049021c = 0;
    FadeViewportPaletteToColour(&g_stModalSourceViewport_005d2c50,
                                g_cSecondaryViewBufferColour_0049cb4c, 1);
    ClearViewport(&g_stModalSourceViewport_005d2c50,
                  g_cSecondaryViewBufferColour_0049cb4c);
    MarkDibDirty();
    DIBslamReal();
    RestoreGamePalette();
    return 0;
}

/* Function start: WC2_UNMAPPED */
unsigned int ShowTigerClawEscapeScene(void)
{
    unsigned char *escapeShape;
    FixedVector jumpOffset;
    short approachStep;
    short frame;
    short effect;
    int depth;
    int verticalOffset;

    approachStep = 15;
    PreloadMusicTrackHook(0x22);
    spacetrack(0x22, 2, 1);
    init_3Space_objects((short)g_stCampaignState_0059ca50.currentSeries);
    g_nCannedSceneMode_0049021c = 2;
    g_bIntroSceneResourcesActive_00469d48_WC1_UNMAPPED = 0;
    InitializeConversationText();
    set_up_action_sphere(0x13);
    escapeShape =
        FetchDiskPacketRetrying(9, 2, 0);
    if (g_aObjectTypeData_00496d30[
            OBJECT_TYPE_HYPERSPACE_JUMP_FLASH].shapeSet == 0) {
        g_aObjectTypeData_00496d30[
            OBJECT_TYPE_HYPERSPACE_JUMP_FLASH].shapeSet =
                FetchDiskPacketRetrying(3, 14, 0);
    }
    g_nScriptedViewObject_0046a8d0_WC1_UNMAPPED = 1;
    initialize_scripted_view(g_asTigerClawEscapeViewScript_0046c238_WC1_UNMAPPED);
    ClearViewport(&g_stConversationTextViewport_005a7570,
                  g_cSecondaryViewBufferColour_0049cb4c);
    SetTextContext(&g_stConversationTextContext_005d2d40);
    FormatTextBufferFromStart(
        g_szTigerClawEscapeOpeningFormat_0046af30_WC1_UNMAPPED, 0, 160,
        g_pszTigerClawEscapeOpening_0046ada0_WC1_UNMAPPED);
    g_nFrameSkipCountdown_0049d760 = 1;
    g_bSceneEscapeRequested_0049d4b0 = 0;
    depth = -1000;
    verticalOffset = -70000;
    frame = 0;
    do {
        Update_3Space();
        if (Draw_3Space_Frame() != 0) {
            if (g_asObjectCollisionRadius_004950e8[61] < depth &&
                frame < 198) {
                DrawSpriteScaled(
                    &g_stViewBuffer_005d2b00, g_nViewCenterX_005c80d8,
                    (short)(g_nViewCenterY_005c80da +
                            verticalOffset / depth),
                    escapeShape, 0, 0, (short)(0x40000L / depth), 0);
            }
            dump_buffer_to_screen();
            clear_view_buffer();
        }
        if (g_asObjectCollisionRadius_004950e8[61] < depth)
            verticalOffset += 400;
        depth += approachStep;
        if (frame > 170)
            approachStep = (short)(approachStep + 10);

        switch (frame) {
        case 150:
            ClearViewport(&g_stConversationTextViewport_005a7570,
                          g_cSecondaryViewBufferColour_0049cb4c);
            SetTextContext(&g_stConversationTextContext_005d2d40);
            FormatTextBufferFromStart(
                g_szTigerClawEscapeJumpFormat_0046af3c_WC1_UNMAPPED, 0, 160,
                g_pszTigerClawEscapeJump_0046ada4_WC1_UNMAPPED);
            break;
        case 190:
            effect = find_vacant_3d_object();
            if (effect != -1) {
                set_objects_data(effect,
                                 OBJECT_TYPE_HYPERSPACE_JUMP_FLASH,
                                 -1, 0);
                ScaleFixedVector(&g_aShipForwardVector_00494208[61],
                                 0x271000, &jumpOffset);
                g_asObjectScale_00494d90[effect] =
                    (short)(g_asObjectScale_00494d90[effect] << 2);
                zero_vector(&g_aShipVelocity_00494898[effect]);
                AddFixedVectors(&g_aShipPosition_00494550[61],
                                &jumpOffset,
                                &g_aShipPosition_00494550[effect]);
            }
            break;
        case 198:
            ClearViewport(&g_stViewBuffer_005d2b00,
                          g_bPrimaryViewBufferColour_0049cb50);
            g_bViewportDirty_0049d76c = 1;
            break;
        case 210:
            ClearViewport(&g_stConversationTextViewport_005a7570,
                          g_cSecondaryViewBufferColour_0049cb4c);
            SetTextContext(&g_stConversationTextContext_005d2d40);
            FormatTextBufferFromStart(
                g_szTigerClawEscapeClosingFormat_0046af48_WC1_UNMAPPED, 0, 160,
                g_pszTigerClawEscapeClosing_0046ada8_WC1_UNMAPPED);
            break;
        }
        if (g_bSceneEscapeRequested_0049d4b0 == 1)
            break;
        frame++;
        MarkDibDirty();
        DIBslamReal();
    } while (frame < 260);

    ReleasePacketHandle(g_aObjectTypeData_00496d30[
        OBJECT_TYPE_HYPERSPACE_JUMP_FLASH].shapeSet);
    g_aObjectTypeData_00496d30[
        OBJECT_TYPE_HYPERSPACE_JUMP_FLASH].shapeSet = 0;
    ReleasePacketHandle(escapeShape);
    free_all_slots();
    ReleaseTextFont(0);
    free_3Space();
    ClearViewport(&g_stConversationTextViewport_005a7570,
                  g_cSecondaryViewBufferColour_0049cb4c);
    StopMusicUnlessSuppressed();
    ReleaseMusicTrackHook(0x22);
    g_bScriptedView_0046a8d4_WC1_UNMAPPED = 0;
    g_nCannedSceneMode_0049021c = 0;
    g_bIntroSceneResourcesActive_00469d48_WC1_UNMAPPED = 1;
    return 0;
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

/* Function start: WC2_UNMAPPED */
unsigned int ShowWc1EndScreen(short enableFireworks)
{
    FireworkState *firework;
    short activeFireworks;
    short frame;
    short slot;

    SetEventManagerPump(get_player_input);
    PreloadMusicTrackHook(0x17);
    spacetrack(0x17, 2, 1);
    InitializeConversationViewport();
    ViewWc1Medals();
    ReleaseTextFont(0);
    ClearViewport(&g_stModalSourceViewport_005d2c50, g_cSecondaryViewBufferColour_0049cb4c);
    ClearViewport(&g_stSecondaryViewBuffer_005d2c90, g_cSecondaryViewBufferColour_0049cb4c);
    InitializeFireworks();
    g_pFireworkShape_005a6a68 =
        FetchDiskPacketRetrying(9, 0x11, 0);
    g_pIntroFont_005a8960 =
        FetchDiskPacketRetrying(9, 1, 0);
    print_subtitle(&g_stSecondaryViewBuffer_005d2c90, 0x3a, g_pszTheEnd_0046adc8_WC1_UNMAPPED);
    PanToScreen(&g_stSecondaryViewBuffer_005d2c90, &g_stScreenViewport_005d21a0);
    g_bSceneEscapeRequested_0049d4b0 = 0;
    g_nFrameSkipCountdown_0049d760 = 1;
    activeFireworks = 0;
    frame = 0;
    do {
        ClearViewport(&g_stSecondaryViewBuffer_005d2c90, g_cSecondaryViewBufferColour_0049cb4c);
        if (enableFireworks != 0 && activeFireworks != 0 &&
            (RandomBelowOrEqual(100) < 40 || frame > 280)) {
            slot = 0;
            do {
                firework = &g_aFireworks_005c8df0[slot];
                if (firework->frame == -1) {
                    firework->frame = 0;
                    firework->x = RandomInRange(0, g_stViewBuffer_005d2b00.right);
                    firework->y = RandomInRange(0, g_stViewBuffer_005d2b00.bottom);
                    firework->variant = RandomInRange(0, 2);
                    break;
                }
                slot++;
            } while (slot < 30);
        }
        activeFireworks = TheEndFireWorks(&g_stSecondaryViewBuffer_005d2c90, 30);
        if (frame < 160) {
            print_subtitle(&g_stSecondaryViewBuffer_005d2c90, 0x3a,
                           g_pszTheEnd_0046adc8_WC1_UNMAPPED);
        } else if (frame > 190) {
            StopMusic(0);
            print_subtitle(&g_stSecondaryViewBuffer_005d2c90, 0x3a,
                           g_pszForNow_0046adcc_WC1_UNMAPPED);
        }
        frame++;
        RefreshMemoryStatusOverlay();
        MarkDibDirty();
        DIBslamReal();
    } while (frame < 320);

    ReleasePacketHandle(g_pFireworkShape_005a6a68);
    ReleasePacketHandle(g_pIntroFont_005a8960);
    ResetScreenClipToFullHeight();
    StopMusicUnlessSuppressed();
    ReleaseMusicTrackHook(0x17);
    return 0;
}

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

/* Function start: WC2_UNMAPPED */
short get_face(short rating, unsigned int side)
{
    if (rating == -1)
        return 13 + (side < SIDE_KILRATHI ? -1 : 0);
    if (side == SIDE_KILRATHI)
        rating--;
    return rating;
}

/* Function start: WC2_UNMAPPED */
void LoadCommPortraitShape(short face, signed char alternate)
{
    short section;

    if (face >= 0 && face <= 7)
        section = face + 1;
    else if ((face > 7 && face < 12) || face == 13)
        section = 10;
    else
        section = -1;
    if (section != -1)
        g_apCommPortraitShapes_0059e180[face] =
            FetchDiskPacketRetrying(11, section,
                                                     (short)alternate);
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

/* Function start: WC2_UNMAPPED */
short GetPendingMenuAction(void)
{
    return (short)g_cPendingCommMenuAction_0049b77c;
}

/* Function start: WC2_UNMAPPED */
void SetPendingMenuAction(unsigned char v)
{
    g_cPendingCommMenuAction_0049b77c = (signed char)v;
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
    if (have_target() == 0) {
        SelectCommRecipient(g_nYourWingman_0049346c);
        return;
    }
    if (g_acShipTarget_00495f20[0] == g_nYourWingman_0049346c) {
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
#ifdef WC1_SDL
    int selectedChoice;
    unsigned char normalColour;
#endif

    if (IsCommChoiceMenuOpen() == 0)
        HandleCommunicationMenuRequest();
    if (IsCommChoiceMenuOpen() != 0) {
        set_new_vdu(1);
        choice = 0;
        DrawTextAt(&g_stRightVduTextContext_005d2ce0, g_stRightVduViewport_005d2b20.left,
                   g_stRightVduViewport_005d2b20.top, g_pszCommMenuHeading_005d1950, 2);
#ifdef WC1_SDL
        selectedChoice = Wc1SdlGetCommunicationMenuSelection();
        normalColour = g_stRightVduTextContext_005d2ce0.colour;
#endif
        if (g_nCommMenuChoiceCount_0049b770 > 0) {
            do {
#ifdef WC1_SDL
                if ((int)choice == selectedChoice)
                    g_stRightVduTextContext_005d2ce0.colour = g_abGamePaletteReservedColours_0049cb54[4];
#endif
                DrawFormattedText("\n%d %s", (int)choice + 1,
                                  g_apszCommMenuChoiceText_005d19a0[
                                      (int)choice]);
#ifdef WC1_SDL
                g_stRightVduTextContext_005d2ce0.colour = normalColour;
#endif
                choice++;
            } while (choice < g_nCommMenuChoiceCount_0049b770);
        }
        DrawSpriteDefault(&g_stRightVduViewport_005d2b20,
                          (short)(g_stRightVduViewport_005d2b20.left + 36),
                          (short)(g_stRightVduViewport_005d2b20.top + 10),
                          g_pCommMenuCursorShape_005a7660, 0x19);
        g_nCommMenuReuseMode_0049b774 = 1;
    }
}

/* Function start: 0x447EF5 */
unsigned int Chosen_communicate_option(short choice)
{
    int action;

    PlaySfxWaveFileByNumber(0x19, -1, 0);
    action = GetPendingMenuAction();
    switch (action) {
    case 0:
        CloseCommChoiceMenu();
        return 0;
    case 1:
        if (g_abCommMenuChoiceCommand_005d1948[choice] == 0) {
            CloseCommChoiceMenu();
            return 0;
        }
        if (g_abCommMenuChoiceCommand_005d1948[choice] == 1) {
            SelectCommRecipient(g_nYourWingman_0049346c);
            RefreshCommunicationMenu();
            return 0;
        }
        SelectCommRecipient(g_acShipTarget_00495f20[0]);
        RefreshCommunicationMenu();
        return 0;
    case 2:
        CloseCommChoiceMenu();
        request(0, (short)g_cCommMenuRecipient_0049b790,
                (short)g_abCommMenuChoiceCommand_005d1948[choice]);
        return 0;
    }
    return 0;
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
#ifdef WC1_SDL
    if (g_nCommPortraitIndex_0049b79c != -1 &&
#else
    if (
#endif
        g_apCommPortraitShapes_0059e180[g_nCommPortraitIndex_0049b79c] != 0)
        malf_noise(1, 1, 12, 23, 1);
    FreeCommDisplayResources();
    if (get_mode(1) == 6)
        pop_mode(1);
}

/* Function start: 0x4480C6 */
void EndCommMenu(void)
{
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
    for (;;) {
        marker = DosStrchr(text, '$');
        if (marker == 0) {
            DosStrcat(g_szTextScratchBuffer_005d1c40, text);
            return g_szTextScratchBuffer_005d1c40;
        }
        destination = DosStrchr(g_szTextScratchBuffer_005d1c40, '\0');
        while (marker != text)
            *destination++ = *text++;
        *destination = '\0';
        text = marker + 2;
        switch (marker[1]) {
        case 'C':
            DosStrcat(
                g_szTextScratchBuffer_005d1c40,
                g_stCampaignState_0059ca50.currentPilot->callsign);
            break;
        case 'N':
        case 'P':
            DosStrcat(g_szTextScratchBuffer_005d1c40,
                      g_stCampaignState_0059ca50.currentPilot->name);
            break;
        case 'R':
            DosStrcat(g_szTextScratchBuffer_005d1c40,
                      g_apszPilotRankNames_0049a608[
                          g_stCampaignState_0059ca50.currentPilot->rank]);
            length = DosStrlen(g_szTextScratchBuffer_005d1c40);
            if (g_szTextScratchBuffer_005d1c40[length - 1] == '.' &&
                *text == '.') {
                length = DosStrlen(g_szTextScratchBuffer_005d1c40);
                g_szTextScratchBuffer_005d1c40[length - 1] = '\0';
            }
            break;
        }
    }
}

/* Function start: 0x4482CA */
void real_vid_transmit(short obj, short message)
{
    char text[84];
    char *expandedText;
    char *speech;
    int objectOffset;

    g_nCommSpeakerObject_0049b794 = obj;
    g_nCommSpeakerRating_0049b798 =
        (short)g_acShipRating_0059cd80[obj];
    g_nCommPortraitIndex_0049b79c =
        get_face(g_nCommSpeakerRating_0049b798,
                 g_asShipSide_004955d0[obj]);
    if (g_nCommPortraitIndex_0049b79c == -1)
        return;
    objectOffset = (int)obj * sizeof(enum ObjectType);
    if (g_bCommSpeechPlaying_0049b7a0 != 0 &&
        g_bVideoImagesSuppressed_0049b784 == 0) {
        if (g_apCommPortraitShapes_0059e180[
                g_nCommPortraitIndex_0049b79c] == 0)
            LoadCommPortraitShape(g_nCommPortraitIndex_0049b79c, 0);
        if (g_apCommPortraitShapes_0059e180[
                g_nCommPortraitIndex_0049b79c] != 0) {
            LoadCommPortraitResources(g_nCommPortraitIndex_0049b79c);
        }
        if (g_apCommPortraitShapes_0059e180[
                g_nCommPortraitIndex_0049b79c] != 0 &&
            g_pCommPortraitResource_0049b788 != 0) {
            push_mode(1, 6);
            malf_noise(1, 3, 12, 23, 1);
            DrawSpriteDefault(
                &g_stRightVduViewport_005d2b20, g_stRightVduViewport_005d2b20.left, g_stRightVduViewport_005d2b20.top,
                g_asShipSide_004955d0[
                    g_nCommSpeakerObject_0049b794] == SIDE_IMPERIAL ?
                    g_pConfedCommBackground_00469278_WC1_UNMAPPED :
                    g_pKilrathiCommBackground_00469280,
                0);
            DrawSpriteDefault(
                &g_stRightVduViewport_005d2b20, g_stRightVduViewport_005d2b20.left, g_stRightVduViewport_005d2b20.top,
                g_apCommPortraitShapes_0059e180[
                    g_nCommPortraitIndex_0049b79c],
                0);
        }
    }
    speech = g_aapszPilotSpeech_0059e220[
        g_nCommPortraitIndex_0049b79c][message];
    if (g_nCommSpeakerRating_0049b798 >= 0 &&
        g_nCommSpeakerRating_0049b798 <= 7) {
#ifdef WC1_SDL
        /* MSVC 4.20 accepts %Fs as its legacy far-string conversion. */
        sprintf(text, "%s: %s",
                g_apWingmanPilots_00598a30[
                    g_nCommSpeakerRating_0049b798]->callsign,
                speech);
#else
        sprintf(text, g_szCommSpeakerTextFormat_0049b888,
                g_apWingmanPilots_00598a30[
                    g_nCommSpeakerRating_0049b798]->callsign,
                speech);
#endif
    } else if (g_nCommSpeakerRating_0049b798 >= 9 &&
               g_nCommSpeakerRating_0049b798 <= 12) {
#ifdef WC1_SDL
        sprintf(text, "%s: %s",
                g_apszKilrathiAceNames_0046af80_WC1_UNMAPPED[
                    g_nCommSpeakerRating_0049b798 - 9],
                speech);
#else
        sprintf(text, g_szCommSpeakerTextFormat_0049b888,
                g_apszKilrathiAceNames_0046af80_WC1_UNMAPPED[
                    g_nCommSpeakerRating_0049b798 - 9],
                speech);
#endif
    } else {
#ifdef WC1_SDL
        sprintf(text, "%s: %s",
                g_aObjectTypeData_00496d30[
                    *(enum ObjectType *)(void *)
                        ((unsigned char *)g_acObjectType_00493980 +
                         objectOffset)].displayName,
                speech);
#else
        sprintf(text, g_szCommSpeakerTextFormat_0049b888,
                g_aObjectTypeData_00496d30[
                    *(enum ObjectType *)(void *)
                        ((unsigned char *)g_acObjectType_00493980 +
                         objectOffset)].displayName,
                speech);
#endif
    }
    expandedText = ExpandCommMessageTokens(text);
    ShowCentredPrompt(expandedText, (unsigned short)MeasureMessageWidth(text));
}

/* Function start: WC2_UNMAPPED */
void __stdcall ShutdownVideoHook(short mode)
{
    ReleaseVideoResourcesHook();
}

/* Function start: WC2_UNMAPPED */
short __stdcall ReserveWc1ContiguousPaletteEntries(short entryCount)
{
    short entry;
    short freeEntries;
    short firstEntry;
    short fillEntry;

    freeEntries = 0;
    entry = 0;
    firstEntry = 0;
    for (;;) {
        if (g_awPaletteEntryAllocation_0059df80[entry] != 0) {
            freeEntries = 0;
            firstEntry = (short)(entry + 1);
        } else
            freeEntries++;
        if (freeEntries == entryCount)
            break;
        entry++;
        if (entry >= 256)
            return -1;
    }

    fillEntry = 0;
    while (fillEntry < entryCount) {
        g_awPaletteEntryAllocation_0059df80[firstEntry + fillEntry] =
            entryCount;
        fillEntry++;
    }
    return firstEntry;
}

/* Function start: WC2_UNMAPPED */
void __stdcall ReleaseContiguousPaletteEntries(short firstEntry)
{
    short entry;
    short entryCount;

    entry = 0;
    entryCount = g_awPaletteEntryAllocation_0059df80[firstEntry];
    while (entry < entryCount) {
        g_awPaletteEntryAllocation_0059df80[firstEntry + entry] = 0;
        entry++;
    }
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
    short index;
    short row;
    short column;

    index = 0;
    row = 4;
    do {
        column = 0x40;
        do {
            printf("%c", g_awPaletteEntryAllocation_0059df80[index++] < 1 ?
                   '_' : '.');
            column--;
        } while (column != 0);
        printf("\n");
        row--;
    } while (row != 0);
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

/* Function start: WC2_UNMAPPED */
void __stdcall UnionRectBounds(ShortRect *destination,
                               const ShortRect *first,
                               const ShortRect *second)
{
    destination->left = first->left < second->left ?
        first->left : second->left;
    destination->top = first->top < second->top ?
        first->top : second->top;
    destination->right = first->right > second->right ?
        first->right : second->right;
    destination->bottom = first->bottom > second->bottom ?
        first->bottom : second->bottom;
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
