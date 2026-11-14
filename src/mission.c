/*
 *  Mission setup, action spheres and ship loading.
 *
 *  Retail object 0x44B200-0x44EA4F (link order; see docs/ORDER.md).
 */
#include "game.h"

short g_bMissionShipInitializationInProgress = 0; /* 0x0049b890 */
short g_bShipResourceReloadInProgress = 0; /* 0x0049b894 */
void **g_apPacketReferenceGroups[5] = { 0 }; /* 0x0049b898 */
unsigned char g_ucPendingEjectionTransition = 0xff; /* 0x0049b8ac */
short g_bEjectionWaitForEnemyWave = 0; /* 0x0049b8b0 */
short g_bEjectionAwaitingCommCommand = 0; /* 0x0049b8b4 */
short g_nPendingEjectionSequenceCount = 0; /* 0x0049b8b8 */
short g_bEjectionTriggerImmediately = 0; /* 0x0049b8bc */
short g_nEjectionSequenceState = 0; /* 0x0049b8c0 */
signed char g_cQueuedNpcPortrait = -1; /* 0x0049b8c4 */
signed char g_cQueuedNpcMessage = -1; /* 0x0049b8c8 */
ShortVector g_aaFormationPositions[5][8] = {
    {
        { 0, 0, 0 }, { -750, 0, 0 }, { 750, 0, 0 }, { 0, 0, -750 },
        { 0, 0, 750 }, { -750, 0, -750 }, { 750, 0, -750 },
        { 0, 0, -1500 }
    },
    {
        { 0, 0, 0 }, { 750, 0, 0 }, { -750, -100, -250 },
        { 1500, -100, -250 }, { -1500, -200, -500 },
        { -2250, -300, -750 }, { 2250, -200, -500 },
        { 3000, -300, -750 }
    },
    {
        { 0, 0, 0 }, { 750, 0, -500 }, { -750, 0, -500 },
        { 0, 0, -1000 }, { -1500, 0, -1000 }, { -750, 0, -1500 },
        { 1500, 0, -1000 }, { 750, 0, -1500 }
    },
    {
        { 0, 0, 0 }, { 750, 0, -250 }, { 0, 325, -500 },
        { 750, -325, -750 }, { 0, 325, -500 }, { 750, -325, -750 },
        { 0, 0, -1000 }, { 750, 0, -1250 }
    },
    {
        { 0, 0, 0 }, { 0, 0, -750 }, { -750, 0, -500 },
        { -750, 0, -1250 }, { 750, 0, -500 }, { 750, 0, -1250 },
        { 0, 500, -500 }, { 0, 500, -1250 }
    }
}; /* 0x0049b8d0 */
short g_anPilotTurnInterval[8] = {
    5, 5, 4, 4, 3, 1, 1, 0
}; /* 0x0049b9c0 */

/* Function start: 0x44B200 */
void set_sphere_point(const MissionShipRecord *record,
                      FixedVector *position)
{
#ifdef SDL_PORT
    /* A nav point of -1 selects the zero-filled gap at 0x491E52 in the retail
     * layout, making the ship coordinates absolute. */
    if (record->navPoint == -1) {
        *position = record->position;
        return;
    }
#endif
    AddFixedVectors(
        &g_aMissionNavPoints[record->navPoint].position,
        &record->position, position);
}

/* Function start: 0x44B239 */
short is_alive(signed char pilot)
{
#ifdef SDL_PORT
    /* SO2 S2M1 gives Maniac comm portrait 36. Campaign saves contain only
     * 32 persistent pilot flags; a portrait outside that roster has no
     * recorded death to suppress its mission ship. */
    if (pilot < 0 || pilot >= 32)
        return 1;
#endif
    return g_pPilotStatus[pilot];

}

/* Function start: 0x44B257 */
void check_futurion(short i)
{
    if (g_asShipMissionType[i] == MISSION_TYPE_WARP_ARRIVE) {
        g_asObjectCounter[i] = g_aeObjectClass[i];
        g_aeObjectClass[i] = OBJECT_CLASS_FUTURION;
    }
}

/* Function start: 0x44B29D */
void ResetWeaponDisplayPositions(void)
{
    short index;

    for (index = 0; index < 16; index++)
        g_116_aWeaponDisplayPositions[index] =
            g_aDefaultWeaponDisplayPositions[index];
}

/* Function start: 0x44B2E2 */
void init_mission(short series, short mission)
{
    /* Two dwords the port kept from the DOS memory budget code; unused. */
    long freeMemory;
    long largestBlock;
    short index;

    g_bTargetLockActive = 0;
    g_bMissileLockAcquired = 0;
    g_nLastAdaptiveDifficultyChangeFrame = 0;
    g_ucPendingEjectionTransition = 0xff;
    g_1_nEnemyCommCommandBase = 20;
    g_nEnemyCommPilotIndex = -1;
    g_nCommDeathSequenceFrame = 0;
    g_bDisplayWingmanTargetData = 0;
    LoadMissionData(series, mission);
    init_3Space_objects(series);
    g_11_nMissionResourceBudget = 0x7c0600;
    LoadPacketResourceList(g_aMissionResourceDescriptors, 0,
                           g_nAvailableGameMemoryNumber,
                           "objects.vga");
    LoadPacketIntoBuffer("difflevl.000", 0,
                         g_asDifficultyLevels, 0);
    g_nEnemyTauntCommandBase = 20;
    g_bFriendlyFireWarningIssued = 0;
    g_nAdaptiveDifficulty = 5;
    for (index = 0; index < 10; index++) {
        g_asShipIntelSlot[index] = -1;
        g_asShipIntelResourceKey[index] = -1;
    }
    g_stWingmanIntelligence.routDecisionMode = 0;
    g_stWingmanIntelligence.formationDisobeyMode = 0;
    g_stWingmanIntelligence.targetingMode = 0;
    g_stWingmanIntelligence.engagementMode = 0;
    prepare_mission();
    g_bCockpitDamageFrame0Shown = 0;
    g_bCockpitDamageFrame2Shown = 0;
    g_bFuelGaugeDamaged = 0;
    InitializeCockpitResources(g_stCurrentPilotProfile.playerShipType);
    g_bMissionDeathSequencePending = 0;
    for (index = 0; index < 5; index++)
        g_acJumpDriveUsedBySystem[index] = 0;
    ResetWeaponDisplayPositions();
    g_nCurStellarSystem =
        g_aMissionNavPoints[
            g_13_stMissionHeaderBlock.entryNavPoint].systemIndex;
}

/* Function start: 0x44B48F */
/* Re-describe an object from the alternate ship record that
 * LoadAlternateShipType leaves in object-type slot 4. */
void ApplyAlternateShipType(short obj)
{
    ObjectTypeData *typeData;
    void InitializeShipWeaponTypeIndices(short ship);
    short weapon;

    typeData = &g_aObjectTypeData[4];
    g_acObjectType[obj] = 4;
    g_apObjectShape[obj] = typeData->shapeSet;
    g_apObjectExhaustShape[obj] = typeData->animation;
    g_asObjectType[obj] = typeData->field_18;
    g_aeObjectClass[obj] = typeData->objectClass;
    init_ijk(obj);
    g_asObjectCollisionRadius[obj] = typeData->collisionRadius;
    g_asObjectScale[obj] = typeData->scale;
    g_asObjectDamage[obj] = 0;
    g_asObjectFlip[obj] = g_asObjectDamage[obj];
    g_asObjectScreenAngle[obj] = g_asObjectFlip[obj];
    g_acObjectOwner[obj] = -1;
    g_acLastCollisionObject[obj] = -1;
    g_asObjectViewFrame[obj] = 0;
    g_acShipTarget[obj] = -1;
    g_aasShipShield[obj][0] = typeData->shieldFore;
    g_aasShipMaximumShield[obj][0] =
        g_aasShipShield[obj][0];
    g_aasShipShield[obj][1] = typeData->shieldAft;
    g_aasShipMaximumShield[obj][1] =
        g_aasShipShield[obj][1];
    g_aasShipArmor[obj][0] = typeData->armorFront;
    g_aasShipArmor[obj][2] = typeData->armorLeft;
    g_aasShipArmor[obj][3] = typeData->armorRight;
    g_aasShipArmor[obj][1] = typeData->armorRear;
    g_anShipFuel[obj] = *(const int *)&typeData->lifetime;
    g_acShipIonDriveDamage[obj] = 0;
    g_acShipDamage[obj] = 0;
    recalc_max_velocity(obj);
    g_acPilotHitPoints[obj] = 4;
    memcpy(g_aShipWeapons[obj], typeData->weaponLoadout,
           sizeof(typeData->weaponLoadout));
    InitializeShipWeaponTypeIndices(obj);

    if (obj == 0) {
        g_nSelectedReleaseWeaponIndex = -1;
        g_nSelectedGunType = -1;
        weapon = (short)(signed char)g_aShipWeapons[obj][0];
        while (weapon-- > 0) {
            if (((struct ShipLoadout *)g_aShipWeapons)[obj].slots[weapon].disabled == 0) {
                if (g_aObjectTypeData[
                        ((struct ShipLoadout *)g_aShipWeapons)[obj].slots[weapon].weaponType].objectClass == 8) {
                    if (g_nSelectedGunType != -1 &&
                        ((struct ShipLoadout *)g_aShipWeapons)[obj].slots[weapon].type !=
                            g_nSelectedGunType)
                        g_nSelectedGunType = 0x80;
                    else
                        g_nSelectedGunType =
                            ((struct ShipLoadout *)g_aShipWeapons)[obj].slots[weapon].type;
                } else {
                    g_nSelectedReleaseWeaponIndex = weapon;
                }
            }
        }
    }

    if (HasShipCockpitGunDisplay(obj) != 0)
        g_asShipTurretFireEnabled[obj] = 1;
    else
        g_asShipTurretFireEnabled[obj] = 0;
    g_asShipWeaponEnergy[obj] = 100;
    g_acShipLastAttacker[obj] = -1;
}

#pragma function(strcat, strcpy)
/* Function start: 0x44B83C */
/* Overlay object-type slot 4 with this campaign's altships.NNN record for
 * the given resource type, keeping the name, animation and shapes that the
 * type's own resource slot already carries. */
void LoadAlternateShipType(short resourceType, short logicalFile)
{
    unsigned char *keptAnimation;
    unsigned char *savedShape;
    char nameCopy[20];
    unsigned char *savedShapeSet;
    short slot;
    char filename[16];
    char numberText[4];

    slot = FindObjectResourceSlot(resourceType);
    strcpy(nameCopy, g_aObjectTypeData[slot].displayName);
    keptAnimation = g_aObjectTypeData[slot].animation;
    savedShapeSet = g_aObjectTypeData[slot].shapeSet;
    savedShape = g_aObjectTypeData[slot].shape;
    strcpy(filename, "altships.");
    if (g_nSelectedCampaignSlot < 100)
        strcat(filename, "0");
    if (g_nSelectedCampaignSlot < 10)
        strcat(filename, "0");
    strcat(filename,
           itoa(g_nSelectedCampaignSlot, numberText, 10));
    LOAD_OBJECT_TYPE_RECORD(filename, logicalFile,
                                &g_aObjectTypeData[4]);
    strcpy(g_aObjectTypeData[4].displayName, nameCopy);
    g_aObjectTypeData[4].resourceType = resourceType;
    g_aObjectTypeData[4].animation = keptAnimation;
    g_aObjectTypeData[4].shapeSet = savedShapeSet;
    g_aObjectTypeData[4].shape = savedShape;
}

#pragma intrinsic(strcat, strcpy)
short find_ships_sphere(short missionShip);

/* Function start: 0x44B981 */
/* Re-derive the nav point of every mission ship an objective names, so the
 * briefing map and the objective list agree on where each one is. */
void RefreshObjectiveShipNavPoints(void)
{
    short source;
    short missionShip;
    short type;

    for (source = 0; source < 8; source++) {
        type = g_69_aMissionObjectiveSources[source].type;
        if (type != -1) {
            switch (type) {
            case 1:
            case 2:
            case 3:
            case 4:
            case 6:
                missionShip =
                    g_69_aMissionObjectiveSources[source].index;
                if (g_bBriefingMapActive != 0 || type == 6 ||
                    g_aMissionShips[missionShip].navPoint != -1)
                    g_aMissionShips[missionShip].navPoint =
                        (signed char)find_ships_sphere(missionShip);
                break;
            default:
                break;
            }
        }
    }
}

/* Function start: 0x44BA73 */
void prepare_mission(void)
{
    short slot;
    short objectKind;
    short playerMissionShip;
    short portraitIndex;

    g_stCurrentPilotProfile.missionScore = 0;
    g_36_cPilotKillCountValue = 0;
    for (slot = 0; slot < 7; slot++)
        g_4_acInitialShipKillNumber[slot] = 0;
    g_bWingmanKilledThisMission = 0;
    g_8_bUserDestroyedFlag =
        g_bWingmanKilledThisMission;

    playerMissionShip = g_13_stMissionHeaderBlock.playerMissionShip;
    g_stCurrentPilotProfile.playerShipType =
        g_aMissionShips[playerMissionShip].type;
    objectKind = OBJECT_CLASS_SHIP;
    load_ship(g_stCurrentPilotProfile.playerShipType,
              g_aMissionShips[playerMissionShip].objectType,
              objectKind, 0);
    if (g_aMissionShips[playerMissionShip].field_13 != 0) {
        LoadAlternateShipType(
            g_stCurrentPilotProfile.playerShipType,
            g_aMissionShips[playerMissionShip].field_13);
        ApplyAlternateShipType(0);
    } else {
        set_objects_data(0, g_stCurrentPilotProfile.playerShipType, -1, 1);
    }
    g_aMissionShips[playerMissionShip].navPoint =
        (signed char)g_13_stMissionHeaderBlock.entryNavPoint;
    if (g_nMissionEntryNavOverride != -1) {
        g_aMissionShips[playerMissionShip].navPoint =
            (signed char)g_nMissionEntryNavOverride;
    }
    Set_up_ship_info(0, playerMissionShip, -1);

    for (slot = 0; slot < 9; slot++)
        g_acPlayerComponentDamage[slot] = 0;
    g_bMissionShipInitializationInProgress = 1;
    g_nYourWingman = -1;
    for (slot = 0; slot < 8; slot++) {
        playerMissionShip =
            g_13_stMissionHeaderBlock.initialMissionShips[slot];
#ifdef SDL_PORT
        /* S3M3 carries -2 in one otherwise empty initial-ship slot.  Retail
         * walks into the preceding nav table and eventually treats its zeroes
         * as a ship; negative mission-ship values are sentinels on the port. */
        if (playerMissionShip < 0)
            continue;
#endif
        if (playerMissionShip != -1) {
            portraitIndex = g_aMissionShips[playerMissionShip].portrait;
            if (portraitIndex == 25 ||
                g_aMissionShips[playerMissionShip].pilot != 5 ||
                is_alive(portraitIndex) != 0) {
                if (find_ships_sphere(playerMissionShip) == -1) {
                    init_ship(playerMissionShip,
                              g_13_stMissionHeaderBlock.entryNavPoint);
                }
            }
        }
    }
    g_bMissionShipInitializationInProgress = 0;
    Build_objective_list();
    g_6_nHomeOperationShipIndexValue =
        g_13_stMissionHeaderBlock.homeMissionShip;
    g_bLandingCommRequestPending = 0;
    g_bTargetLockMode = 0;
}

void FreePacketAndClear(void *slot, unsigned short releaseFlags);
void ReportFatalErrorCode(const char *errorCode);
void release_capital_ship_shapes(short resourceType);
void free_ship(short slot);
short FindObjectResourceSlot(short resourceType);
void ReleasePacketHandle(void *);
void add_hazard_field(short type, FixedVector center, short radius, short density);
void flag_objective(short objective, unsigned char flags);
void *FetchDiskPacketRetrying(char *fileName, short section, short flags);

/* Function start: 0x44BCF7 */
void release_all_capital_ship_shapes(void)
{
    short obj;

    for (obj = 0; obj < 10; obj++) {
        if (g_aeObjectClass[obj] >=
                OBJECT_CLASS_CAPITAL_SHIP ||
            (g_bExpandedShipGraphicsEnabled != 0 &&
             g_aeObjectClass[obj] == OBJECT_CLASS_SHIP)) {
#ifdef SDL_PORT
            /* As in remove_object: a preloaded view belongs to the resource
             * slot, and two ships of a type can share one, so retail could
             * free it twice here.  Free only the ship's own packet. */
            if (g_abObjectShapeOwned[obj] != 0)
                FreePacketAndClear(&g_apObjectShape[obj], 0);
            g_apObjectShape[obj] = 0;
            g_abObjectShapeOwned[obj] = 0;
#else
            FreePacketAndClear(&g_apObjectShape[obj], 0);
#endif
            g_asLoadedShipViewFrame[obj] = -1;
        }
    }
}

/* Function start: 0x44BD83 */
void release_capital_ship_shapes(short resourceType)
{
    short slot;
    short obj;

    for (slot = 0; slot < 5; slot++) {
        if (g_aObjectTypeData[slot].resourceType == resourceType)
            break;
    }
    if (g_aObjectTypeData[slot].objectClass >=
            OBJECT_CLASS_CAPITAL_SHIP ||
        (g_bExpandedShipGraphicsEnabled != 0 &&
         g_aObjectTypeData[slot].objectClass ==
             OBJECT_CLASS_SHIP)) {
        for (obj = 1; obj < 10; obj++) {
            if (g_aObjectTypeData[
                    g_acObjectType[obj]].resourceType ==
                resourceType) {
#ifdef SDL_PORT
                /* free_ship releases the preloaded views next; free only
                 * packets the ships loaded themselves (see remove_object). */
                if (g_abObjectShapeOwned[obj] != 0)
                    FreePacketAndClear(&g_apObjectShape[obj], 0);
                g_apObjectShape[obj] = 0;
                g_abObjectShapeOwned[obj] = 0;
#else
                FreePacketAndClear(&g_apObjectShape[obj], 0);
#endif
                g_asLoadedShipViewFrame[obj] = -1;
            }
        }
    }
}

/* Function start: 0x44BE84 */
/* The original scans one slot past the five records; index five lands in the
 * padding before the next global and never matches a live resource type. */
short FindObjectResourceSlot(short resourceType)
{
    short slot;

    for (slot = 0; slot <= 5; slot++) {
        if (g_aObjectResourceSlots[slot].resourceType ==
            resourceType)
            return slot;
    }
    return -1;
}

#pragma function(strcat, strcpy, strlen)
/* Function start: 0x44BEE5 */
void load_ship(short resourceType, short objectType,
               short objectClass, short slot)
{
    char fileName[20];
    short section;
    char resourceNumber[4];
    short entry;
    void **packetRefs;

    if (resourceType != -1) {
#ifdef SDL_PORT
        /* The capital-ship branch below never sets the slot's shapeSet, so
         * this check passes again on every call.  load_all_slots runs at
         * each nav sphere, and retail then refetches the slot's 41 packets
         * over the previous set, which is never freed.  Their handles fill
         * the table until PushMemoryStackFrame exits with "qq mem push
         * overflow".  Keep a capital slot that still holds this ship. */
        if (g_aObjectResourceSlots[slot].shapeSet == 0 &&
            g_aObjectResourceSlots[slot].shape != 0 &&
            g_aObjectResourceSlots[slot].resourceType == resourceType &&
            g_aObjectResourceSlots[slot].objectType == objectType)
            return;
#endif
        if (g_aObjectResourceSlots[slot].shapeSet == 0) {
            g_aObjectResourceSlots[slot].resourceType = resourceType;
            if (objectClass == OBJECT_CLASS_MISSILE)
                strcpy(fileName, "missile.v");
            else
                strcpy(fileName, "ship.v");
            if (g_bExpandedShipGraphicsEnabled != 0 &&
                objectClass == OBJECT_CLASS_SHIP) {
                objectClass = OBJECT_CLASS_CAPITAL_SHIP;
                strcpy(fileName, "xship.v");
            }
            itoa((int)resourceType, resourceNumber, 10);
            if (strlen(resourceNumber) == 1)
                strcat(fileName, "0");
            strcat(fileName, resourceNumber);
            g_aObjectResourceSlots[slot].objectType = objectType;
            g_aObjectResourceSlots[slot].objectClass = objectClass;

            if (objectType == 0x3d) {
                g_aObjectTypeData[46].shapeSet =
                    FetchDiskPacketRetrying("objects.vga", 0x13, 0);
                g_aObjectResourceSlots[slot].shapeSet =
                    g_aObjectTypeData[46].shapeSet;
                for (entry = 1; entry < 10; entry++) {
                    if (g_asObjectType[entry] == 0x3d) {
                        g_apObjectShape[entry] =
                            g_aObjectTypeData[
                                g_acObjectType[entry]].shapeSet;
                    }
                }
            } else if (objectType == 0x2c) {
                if (g_aObjectTypeData[44].shapeSet == 0) {
                    g_aObjectTypeData[44].shapeSet =
                        FetchDiskPacketRetrying("pilotanm.vga", 2, 0);
                    g_aObjectResourceSlots[slot].shapeSet =
                        g_aObjectTypeData[44].shapeSet;
                }
                for (entry = 1; entry < 70; entry++) {
                    if (g_asObjectType[entry] == 0x2c) {
                        g_apObjectShape[entry] =
                            g_aObjectTypeData[
                                g_acObjectType[entry]].shapeSet;
                    }
                }
            } else if (objectType == 5) {
                g_aObjectTypeData[26].shapeSet =
                    FetchDiskPacketRetrying("objects.vga", 0x10, 0);
                g_aObjectTypeData[24].shapeSet =
                    g_aObjectTypeData[26].shapeSet;
                g_aObjectTypeData[22].shapeSet =
                    g_aObjectTypeData[24].shapeSet;
                g_aObjectResourceSlots[slot].shapeSet =
                    g_aObjectTypeData[22].shapeSet;
                if (g_98_nMemConfiguration == 2) {
                    g_aObjectTypeData[27].shapeSet =
                        FetchDiskPacketRetrying("objects.vga", 0x11, 4);
                    g_aObjectTypeData[25].shapeSet =
                        g_aObjectTypeData[27].shapeSet;
                    g_aObjectTypeData[23].shapeSet =
                        g_aObjectTypeData[25].shapeSet;
                }
                for (entry = 10; entry <= SPACE_LAST_MOVING_OBJECT;
                     entry++) {
                    if (g_aeObjectClass[entry] ==
                        OBJECT_CLASS_ASTEROID) {
                        g_apObjectShape[entry] =
                            g_aObjectTypeData[
                                g_acObjectType[entry]].shapeSet;
                    }
                }
            } else if (objectType == 6) {
                if (g_aObjectTypeData[21].shapeSet == 0) {
                    g_aObjectTypeData[21].shapeSet =
                        FetchDiskPacketRetrying("objects.vga", 0x0f, 0);
                    g_aObjectResourceSlots[slot].shapeSet =
                        g_aObjectTypeData[21].shapeSet;
                }
                for (entry = 10; entry <= SPACE_LAST_MOVING_OBJECT;
                     entry++) {
                    if (g_asObjectType[entry] == 0x15)
                        g_apObjectShape[entry] =
                            g_aObjectTypeData[21].shapeSet;
                }
            } else if ((objectClass == OBJECT_CLASS_SHIP &&
                        objectType != 0x33) ||
                       objectClass == OBJECT_CLASS_MISSILE) {
                if (objectClass != OBJECT_CLASS_MISSILE &&
                    g_bShipResourceReloadInProgress == 0) {
                    LOAD_OBJECT_TYPE_RECORD(
                        fileName, 3,
                        &g_aObjectTypeData[slot]);
                }
                g_aObjectResourceSlots[slot].shapeSet =
                    FetchDiskPacketRetrying(fileName, 0, 0);
                g_aObjectTypeData[slot].shapeSet =
                    g_aObjectResourceSlots[slot].shapeSet;
                g_aObjectResourceSlots[slot].animation =
                    FetchDiskPacketRetrying(fileName, 2, 0);
                g_aObjectTypeData[slot].animation =
                    g_aObjectResourceSlots[slot].animation;
                g_aObjectResourceSlots[slot].shape =
                    FetchDiskPacketRetrying(fileName, 1, 0);
                g_aObjectTypeData[slot].shape =
                    g_aObjectResourceSlots[slot].shape;
                if (objectClass == OBJECT_CLASS_SHIP &&
                    g_19_bStoredHighMemoryEnabled != 0 &&
                    g_1_nResourcePaletteState == 0) {
                    g_aObjectResourceSlots[slot].field_12 =
                        FetchDiskPacketRetrying(fileName, 4, 4);
                    g_aObjectTypeData[32].shapeSet =
                        g_aObjectResourceSlots[slot].field_12;
                }
                for (entry = 0; entry < 10; entry++) {
                    if (g_aeObjectClass[entry] >=
                            OBJECT_CLASS_MISSILE &&
                        g_aObjectTypeData[
                            g_acObjectType[entry]].resourceType ==
                            resourceType) {
                        g_apObjectExhaustShape[entry] =
                            g_aObjectResourceSlots[slot].animation;
                        g_apObjectShape[entry] =
                            g_aObjectResourceSlots[slot].shapeSet;
                    }
                }
                if (objectClass == OBJECT_CLASS_MISSILE) {
                    g_aObjectTypeData[objectType].shapeSet =
                        g_aObjectTypeData[slot].shapeSet;
                    g_aObjectTypeData[objectType].animation =
                        g_aObjectTypeData[slot].animation;
                    g_aObjectTypeData[objectType].shape =
                        g_aObjectTypeData[slot].shape;
                }
            } else {
                if (g_19_bStoredHighMemoryEnabled != 0 &&
                    g_1_nResourcePaletteState == 0) {
                    g_apPacketReferenceGroups[slot] =
                        (void **)calloc(0x25, HOST_POINTER_SIZE);
                    packetRefs =
                        g_apPacketReferenceGroups[slot];
                    if (packetRefs == 0)
                        ReportFatalErrorCode("023");
                    for (section = 0; section < 0x25; section++) {
                        packetRefs[section] =
                            FetchDiskPacketRetrying(fileName, section, 4);
                        if (packetRefs[section] == 0)
                            break;
                    }
                    g_aObjectResourceSlots[slot].field_12 =
                        FetchDiskPacketRetrying(fileName, 0x27, 4);
                }
                LOAD_OBJECT_TYPE_RECORD(
                    fileName, 0x26,
                    &g_aObjectTypeData[slot]);
                g_aObjectTypeData[slot].shapeSet = 0;
                g_aObjectResourceSlots[slot].shape =
                    FetchDiskPacketRetrying(fileName, 0x25, 0);
                g_aObjectTypeData[slot].shape =
                    g_aObjectResourceSlots[slot].shape;
                g_aObjectResourceSlots[slot].animation =
                    FetchDiskPacketRetrying(fileName, 0x28, 0);
                g_aObjectTypeData[slot].animation =
                    g_aObjectResourceSlots[slot].animation;
                for (entry = 0; entry < 10; entry++) {
                    if (g_aObjectTypeData[
                            g_acObjectType[entry]].resourceType ==
                        resourceType) {
                        g_apObjectExhaustShape[entry] =
                            g_aObjectResourceSlots[slot].animation;
#ifdef SDL_PORT
                        /* Only the ship's own view packet is freed; a
                         * preloaded one belongs to a resource slot (see
                         * remove_object). */
                        if (g_abObjectShapeOwned[entry] != 0)
                            FreePacketAndClear(
                                &g_apObjectShape[entry], 0);
                        g_apObjectShape[entry] = 0;
                        g_abObjectShapeOwned[entry] = 0;
#else
                        FreePacketAndClear(
                            &g_apObjectShape[entry], 0);
#endif
                        g_asLoadedShipViewFrame[entry] = -1;
                    }
                }
            }
        }
    }
}

/* Function start: 0x44C796 */
void free_ship(short slot)
{
    short section;
    short resourceType;
    short obj;
    void **refCount;

    resourceType = g_aObjectResourceSlots[slot].resourceType;
    if (g_aObjectResourceSlots[slot].objectClass >=
            OBJECT_CLASS_CAPITAL_SHIP ||
        g_aObjectResourceSlots[slot].objectType == 0x33 ||
        (g_bExpandedShipGraphicsEnabled != 0 &&
         g_aObjectResourceSlots[slot].objectClass ==
             OBJECT_CLASS_SHIP)) {
        if (g_1_nResourcePaletteState == 0)
            release_capital_ship_shapes(resourceType);
        if (g_19_bStoredHighMemoryEnabled != 0 &&
            g_1_nResourcePaletteState == 0 &&
            g_apPacketReferenceGroups[slot] != 0) {
            refCount = g_apPacketReferenceGroups[slot];
            for (section = 0; section < 0x25; section++) {
                FreePacketAndClear(&refCount[section], 8);
            }
            ReleasePacketHandle(refCount);
            g_apPacketReferenceGroups[slot] = 0;
        }
        FreePacketAndClear(
            &g_aObjectResourceSlots[slot].shape, 8);
        g_aObjectTypeData[slot].shape = 0;
#ifdef SDL_PORT
        /* new_sphere_shapes frees a slot the next nav does not preload, but
         * its ships can stay in the sphere, still drawn: get_right_shape
         * loads their views itself.  Retail leaves their exhaust records,
         * and the type's for the next load_all_slots, in the freed
         * packet; place_exhaust_on_ships then reads it. */
        for (obj = 0; obj < 10; obj++) {
            if (g_apObjectExhaustShape[obj] ==
                g_aObjectResourceSlots[slot].animation)
                g_apObjectExhaustShape[obj] = 0;
        }
        if (g_aObjectTypeData[slot].animation ==
            g_aObjectResourceSlots[slot].animation)
            g_aObjectTypeData[slot].animation = 0;
#endif
        FreePacketAndClear(
            &g_aObjectResourceSlots[slot].animation, 8);
        if (g_1_nResourcePaletteState == 0) {
            FreePacketAndClear(
                &g_aObjectResourceSlots[slot].field_12, 8);
        }
    }
    if (g_aObjectResourceSlots[slot].shapeSet == 0)
        return;
    FreePacketAndClear(
        &g_aObjectResourceSlots[slot].shapeSet, 8);
    if (g_aObjectResourceSlots[slot].objectType == 0x3d) {
        for (obj = 1; obj < 10; obj++) {
            if (g_asObjectType[obj] == 0x3d)
                g_apObjectShape[obj] = 0;
        }
        return;
    }
    if (g_aObjectResourceSlots[slot].objectType == 0x2c) {
        for (obj = 1; obj < SPACE_OBJECT_COUNT; obj++) {
            if (g_asObjectType[obj] == 0x2c)
                g_apObjectShape[obj] = 0;
        }
        return;
    }
    if (g_aObjectResourceSlots[slot].objectType == 5) {
        FreePacketAndClear(
            &g_aObjectTypeData[28].shapeSet, 8);
        FreePacketAndClear(
            &g_aObjectTypeData[23].shapeSet, 8);
        g_aObjectTypeData[27].shapeSet = 0;
        g_aObjectTypeData[26].shapeSet =
            g_aObjectTypeData[27].shapeSet;
        g_aObjectTypeData[25].shapeSet =
            g_aObjectTypeData[26].shapeSet;
        g_aObjectTypeData[24].shapeSet =
            g_aObjectTypeData[25].shapeSet;
        for (obj = 10; obj <= SPACE_LAST_MOVING_OBJECT; obj++) {
            if (g_asObjectType[obj] == 0x1c)
                remove_object(obj);
            else if (g_aeObjectClass[obj] ==
                     OBJECT_CLASS_ASTEROID)
                g_apObjectShape[obj] = 0;
        }
        return;
    }
    if (g_aObjectResourceSlots[slot].objectType == 6) {
        FreePacketAndClear(
            &g_aObjectTypeData[21].shapeSet, 8);
        return;
    }
    g_aObjectTypeData[slot].shapeSet = 0;
    FreePacketAndClear(
        &g_aObjectResourceSlots[slot].animation, 8);
    g_aObjectTypeData[slot].animation = 0;
    FreePacketAndClear(
        &g_aObjectResourceSlots[slot].shape, 8);
    g_aObjectTypeData[slot].shape = 0;
    if (g_1_nResourcePaletteState == 0) {
        FreePacketAndClear(
            &g_aObjectResourceSlots[slot].field_12, 8);
    }
    for (obj = 0; obj < 10; obj++) {
        if (g_aeObjectClass[obj] >= OBJECT_CLASS_MISSILE &&
            g_acObjectType[obj] == slot)
            g_apObjectShape[obj] = 0;
    }
}

/* Function start: 0x44CC84 */
void free_all_slots(void)
{
    short slot;

    release_all_capital_ship_shapes();
    for (slot = 0; slot < 4; slot++) {
        if (g_aObjectResourceSlots[slot].resourceType != -1)
            free_ship(slot);
    }
}

/* Function start: 0x44CCE1 */
void load_all_slots(void)
{
    short slot;
    short type;
    short object;

    g_bShipResourceReloadInProgress = 1;
    free_view_buffer();
    release_all_capital_ship_shapes();
    for (slot = 0; slot < 4; slot++) {
        type = g_aObjectResourceSlots[slot].resourceType;
        if (type != -1) {
            load_ship(type,
                      g_aObjectResourceSlots[slot].objectType,
                      g_aObjectResourceSlots[slot].objectClass,
                      slot);
        }
    }
    for (object = 0; object < 10; object++) {
        g_apObjectExhaustShape[object] =
            g_aObjectTypeData[
                g_acObjectType[object]].animation;
    }
    g_bShipResourceReloadInProgress = 0;
}

/* Function start: 0x44CDCF */
void remove_nav_point_objects(void)
{
    short i;

    for (i = 0; i < 10; i++)
        remove_object(i);
}

/* Function start: 0x44CE0A */
short get_shape_slot(void)
{
    short slot;

    for (slot = 0; slot < 5; slot++) {
        if (g_aObjectResourceSlots[slot].resourceType == -1)
            return slot;
    }
    return -1;
}

/* Function start: 0x44CE68 */
short shape_loaded(short resourceType)
{
    short slot;

    for (slot = 0; slot < 5; slot++) {
        if (g_aObjectResourceSlots[slot].resourceType == resourceType)
            return 1;
    }
    return 0;
}

/* Function start: 0x44CEC8 */
short shape_needed(const MissionNavPoint *navPoint, short resourceType)
{
    short preload;

    if (resourceType != -1) {
        for (preload = 0; preload < 3; preload++) {
            if (navPoint->preloadObjectTypes[preload] == resourceType)
                return 1;
        }
    }
    return 0;
}

/* Function start: 0x44CF2D */
void new_sphere_shapes(MissionNavPoint *navPoint)
{
    short objectClass;
    short slot;
    short preload;
    short type;

    free_view_buffer();
    release_all_capital_ship_shapes();
    for (slot = 1; slot < 4; slot++) {
        if (g_aObjectResourceSlots[slot].resourceType != -1 &&
            !shape_needed(navPoint,
                          g_aObjectResourceSlots[slot].resourceType)) {
            free_ship(slot);
            g_aObjectResourceSlots[slot].resourceType = -1;
        }
    }

    for (preload = 0; preload < 3; preload++) {
        type = navPoint->preloadObjectTypes[preload];
        if (type != -1) {
            if (navPoint->preloadLogicalFiles[preload] == 0x3d ||
                navPoint->preloadLogicalFiles[preload] == 0x2c) {
                slot = get_shape_slot();
                load_ship(0,
                          navPoint->preloadLogicalFiles[preload], 0,
                          slot);
            } else if (!shape_loaded(type)) {
                slot = get_shape_slot();
                if (slot != -1) {
                    objectClass = navPoint->preloadObjectClasses[preload];
                    load_ship(type,
                              navPoint->preloadLogicalFiles[preload],
                              objectClass,
                              slot);
                }
            }
        }
    }
    initialize_view_buffer();
}

/* Function start: 0x44D0C7 */
void ProcessMissionWaveCommands(MissionNavPoint *navPoint)
{
    short offset;
    char fileName[16];
    char digits[4];
    short pilotIndex;
    short number;
    short ejectTarget;

    offset = 0;
    while (offset < 8) {
        switch ((unsigned char)navPoint->waveCommands[offset]) {
        case 0:
            pilotIndex =
                (unsigned char)navPoint->waveCommands[offset + 1];
            number =
                (unsigned char)navPoint->waveCommands[offset + 2];
            strcpy(fileName, "altint.");
            if (number < 100)
                strcat(fileName, "0");
            if (number < 10)
                strcat(fileName, "0");
            strcat(fileName, itoa((int)number, digits, 10));
            LoadPacketIntoBuffer(
                fileName, 0,
                g_aShipIntelligenceData[
                    g_asShipIntelSlot[pilotIndex]],
                0);
            break;
        case 1:
            ejectTarget =
                (unsigned char)navPoint->waveCommands[offset + 1];
            g_bEjectionWaitForEnemyWave = 0;
            g_nPendingEjectionSequenceCount = 0;
            g_bEjectionAwaitingCommCommand = 0;
            g_bEjectionTriggerImmediately = 0;
            switch ((unsigned char)
                        navPoint->waveCommands[offset + 2]) {
            case 0:
                g_nEjectionSequenceState = 0;
                g_ucPendingEjectionTransition =
                    (signed char)ejectTarget;
                g_nPendingEjectionSequenceCount = 1;
                break;
            case 1:
                g_ucPendingEjectionTransition =
                    (signed char)ejectTarget;
                break;
            case 2:
                g_ucPendingEjectionTransition =
                    (signed char)ejectTarget;
                g_bEjectionWaitForEnemyWave++;
                break;
            case 3:
                g_ucPendingEjectionTransition =
                    (signed char)ejectTarget;
                g_bEjectionAwaitingCommCommand++;
                break;
            case 4:
                ejection_sequence(
                    (unsigned char)navPoint->waveCommands[offset + 1],
                    1);
                if (g_nArcadeState != 0)
                    return;
                break;
            case 5:
                g_ucPendingEjectionTransition =
                    (signed char)ejectTarget;
                g_bEjectionWaitForEnemyWave++;
                g_bEjectionTriggerImmediately++;
                break;
            }
            navPoint->waveCommands[offset] = -1;
            break;
        case 2:
            g_cQueuedNpcPortrait =
                navPoint->waveCommands[offset + 1];
            g_cQueuedNpcMessage =
                navPoint->waveCommands[offset + 2];
            break;
        case 0xff:
            break;
        }
        offset = (short)(offset + 3);
    }
}

#pragma intrinsic(strcat, strcpy)
void new_sphere_shapes(MissionNavPoint *navPoint);

/* Function start: 0x44D35D */
void set_up_action_sphere(short navPoint)
{
    short leftover;
    short slot;
    short spawn;
    short stale;
    short trigger;
    MissionNavPoint *point;

    load_all_slots();
    g_nCurrentNavPoint = navPoint;
    point = &g_aMissionNavPoints[navPoint];
    g_nCurStellarSystem = point->systemIndex;
    ProcessMissionWaveCommands(point);
    if (g_aMissionNavPoints[navPoint + 1].type == 2)
        g_nCurrentWave = 2;
    else
        g_nCurrentWave = -1;
    g_nEnemySighting = 0x7fff;

    for (stale = 0; stale < 10; stale++) {
        if (g_asObjectType[stale] == OBJECT_DATA_SHIP_WING) {
            remove_object(stale);
        } else if (g_asObjectType[stale] ==
                   OBJECT_DATA_EJECTED_PILOT) {
            remove_object(stale);
            if (g_aObjectTypeData[
                    OBJECT_DATA_EJECTED_PILOT].shapeSet != 0) {
                FreePacketAndClear(
                    &g_aObjectTypeData[
                        OBJECT_DATA_EJECTED_PILOT].shapeSet, 0);
            }
        }
    }

    for (leftover = 1; leftover < 10; leftover++) {
        if (g_aeObjectClass[leftover] != OBJECT_CLASS_NULL &&
            g_acShipNavPoint[leftover] != -1) {
            if (g_aeObjectClass[leftover] >= OBJECT_CLASS_SHIP &&
                g_asShipMissionType[leftover] == MISSION_TYPE_ROUT) {
                g_aMissionShips[
                    g_asShipMissionIndex[leftover]].state = 3;
            }
            if (g_aeObjectClass[leftover] >= OBJECT_CLASS_CAPITAL_SHIP ||
                (g_bExpandedShipGraphicsEnabled != 0 &&
                 g_aeObjectClass[leftover] == OBJECT_CLASS_SHIP))
#ifdef SDL_PORT
            {
                /* As in remove_object: free only a view packet the ship
                 * loaded itself, not one its type shares. */
                if (g_abObjectShapeOwned[leftover] != 0)
                    FreePacketAndClear(&g_apObjectShape[leftover], 0);
                g_apObjectShape[leftover] = 0;
                g_abObjectShapeOwned[leftover] = 0;
            }
#else
                FreePacketAndClear(&g_apObjectShape[leftover], 0);
#endif
            if (g_asObjectScreenX[leftover] != -32767 &&
                g_bJumpSequenceActive == 0)
                explode(-1, leftover);
            else
                remove_object(leftover);
        }
    }
    remove_all_hazards();
    g_nHazardFieldCount = 0;
    new_sphere_shapes(point);

    for (slot = 0; slot < 10; slot++) {
        spawn = point->missionShips[slot];
        if (spawn != -1)
            init_ship(spawn, navPoint);
    }

    for (slot = 0; slot < 8; slot += 2) {
        if (((signed char *)point->triggers)[slot] != -1) {
            trigger = ((signed char *)point->triggers)[slot + 1];
            g_aMissionNavPoints[trigger].type =
                ((signed char *)point->triggers)[slot];
        }
    }

    if (g_aMissionObjectives[
            g_cCurrentObjective].type == 0 &&
        g_aMissionNavPoints[
            g_aMissionObjectives[
                g_cCurrentObjective].index].type == 0)
        flag_objective(g_cCurrentObjective, 1);

    for (slot = 0; slot < g_cMissionObjectiveCount; slot++)
        LocateMobileObjective(slot);
    clean_up_cockpit();
    g_bLandingCommRequestPending = 0;
    g_nEscapedEnemyCount = 0;
    if (g_bAutopilotSequenceActive == 0 &&
        g_78_bHighMemoryBuffersSetState != 0)
        ResetCannedSceneRecording();
}

void ComputeVectorDelta(FixedVector *from, FixedVector *to, FixedVector *delta);
void random_radial(const FixedVector *center, short radius, FixedVector *position);
void place_ship_near_player_until_valid(short obj, short minimum,
                                        short maximum);
void Set_up_ship_info(short obj, short missionShip, signed char navPoint);
void remove_object(short obj);

/* Function start: 0x44D72E */
short room_for_me(short obj, short minimum)
{
    short other;
    short distance;

    if (distance_from_object(obj, 0) < minimum)
        return 0;
    for (other = 0; other < SPACE_OBJECT_COUNT; other++) {
        if (other != obj && g_aeObjectClass[other] >= 8) {
            ComputeVectorDelta(&g_aShipPosition[obj],
                               &g_aShipPosition[other],
                               &g_vCollisionDelta);
            distance = (short)((g_asObjectCollisionRadius[other] +
                             g_asObjectCollisionRadius[obj]) * 2);
            if (IsVectorWithinRange(&g_vCollisionDelta,
                                    distance) != 0)
                return 0;
        }
    }
    return 1;
}

/* Function start: 0x44D81F */
void place_ship_near_player_until_valid(short obj, short minimum, short maximum)
{
    if (obj == -1 || obj == 0)
        return;
    while (room_for_me(obj, minimum) == 0) {
        random_radial(&g_aShipPosition[0], maximum,
                      &g_aShipPosition[obj]);
    }
}

/* Function start: 0x44D888 */
short set_up_next_wave(void)
{
    MissionNavPoint *waveRecord;
    short prevWave;
    short obj;
    short entry;

    if (g_nCurrentWave == -1 ||
        g_nTrainSimActive != 0)
        return 0;

    waveRecord = &g_aMissionNavPoints[
        g_nCurrentWave - 1 + g_nCurrentNavPoint];
    prevWave = g_nCurrentWave;
    g_nCurrentWave++;
    if (waveRecord->type == prevWave) {
        if (g_19_bStoredHighMemoryEnabled != 0) {
            for (obj = 10; obj < SPACE_OBJECT_COUNT; obj++) {
                if (g_asObjectType[obj] == OBJECT_TYPE_TORPEDO)
                    remove_object(obj);
            }
        }
        ProcessMissionWaveCommands(waveRecord);
        new_sphere_shapes(waveRecord);
        waveRecord->type = -1;
        for (entry = 0; entry < 10; entry++) {
            place_ship_near_player_until_valid(
                init_ship(waveRecord->missionShips[entry],
                          g_nCurrentNavPoint),
                5000, 10000);
        }
    } else {
        g_nCurrentWave = -1;
        return 0;
    }
    return 1;
}

/* Function start: 0x44D9E2 */
void sub_int_vector(const ShortVector *left,
                    const ShortVector *right,
                    ShortVector *difference)
{
    difference->x = left->x - right->x;
    difference->y = left->y - right->y;
    difference->z = left->z - right->z;
}

short find_ship_index(short missionShip);

/* Function start: 0x44DA2F */
void set_formation_position(short obj, const MissionShipRecord *record)
{
    const MissionShipRecord *leader;
    short source;

    if (record->formationIndex != -1) {
        leader = record;
        source = obj;
        while (leader->leaderMissionIndex != -1) {
            source = find_ship_index(leader->leaderMissionIndex);
            leader = &g_aMissionShips[leader->leaderMissionIndex];
        }
        sub_int_vector(
            &g_aaFormationPositions[leader->formationIndex]
                                            [record->formationSpot],
            &g_aaFormationPositions[leader->formationIndex]
                                            [leader->formationSpot],
            &g_aShipFormationOffset[obj]);
        if (source == 0 &&
            g_bMissionShipInitializationInProgress == 0)
            return;
#ifdef SDL_PORT
        if (source == -1) {
            /* A formation can arrive before its leader is in the active
             * sphere. Use the leader's mission orientation in that case. */
            init_ijk(obj);
            alter_yaw((short)-leader->pitch, obj);
            alter_pitch((short)-leader->yaw, obj);
            alter_roll(leader->roll, obj);
        } else
#endif
        copy_frame(source, obj);
        set_sphere_point(leader, &g_aShipPosition[obj]);
        offset_location(obj, &g_aShipFormationOffset[obj],
                        &g_aShipPosition[obj]);
        g_anShipSpeed[obj] = leader->speed << 8;
    }
}

/* Function start: 0x44DB7E */
void Set_up_ship_info(short obj, short missionShip, signed char navPoint)
{
    MissionShipRecord *record;
    short gun;

    g_bKilrathiAceGreetingSent = 0;
    g_asLoadedShipViewFrame[obj] =
        g_acShipLastAttacker[obj] =
            g_acShipPendingMessage[obj] = -1;
    g_acShipCommunicator[obj] =
        g_acShipComputerDamage[obj] =
            g_asShipFriendlyFireCooldown[obj] =
                g_asObjectDamage[obj] =
                    g_acShipIonDriveDamage[obj] =
                        g_acShipDamage[obj] =
                            g_asActionCount[obj] =
                                g_acShipExhaustHeat[obj] = 0;
    g_asShipMissionIndex[obj] = missionShip;
    g_acShipNavPoint[obj] = navPoint;
    record = &g_aMissionShips[missionShip];
    g_acShipPortrait[obj] = record->portrait;

    set_sphere_point(record, &g_aShipPosition[obj]);
    alter_yaw((short)-record->pitch, obj);
    alter_pitch((short)-record->yaw, obj);
    alter_roll(record->roll, obj);
    g_asShipSide[obj] = record->side;
    g_anShipSpeed[obj] = record->speed << 8;
    g_asPilotLevel[obj] = record->pilot;
    for (gun = 0; gun < 3; gun++)
        g_27_aasGunCooldown[obj][gun] = 0;
    g_acShipCollisionCooldown[obj] = 0;
    g_asProjectileEvasionTicks[obj] = 0;
    g_asWingmanDamageCredit[obj] = 0;
    g_asPlayerHitsCreditValues[obj] = 0;
    g_2_asFlakCooldownTable[obj] = (short)(
        (unsigned short)RandomInRange(0, 200) + 600);
    if (g_asShipSide[obj] == SIDE_KILRATHI)
        g_2_asFlakCooldownTable[obj] >>= 1;
    g_asSpikeriCooldown[obj] = (short)(
        (unsigned short)RandomInRange(0, 400) + 600);
    g_asSpikeriLaunchCountdown[obj] = 0;
    recalc_max_velocity(obj);
    g_asCapitalHitDamageStepTable[obj] = 0;
    if (g_aObjectTypeData[
            g_acObjectType[obj]].damageCapacity +
            g_aasShipArmor[obj][0] < 400) {
        g_asCapitalHitDamageStepTable[obj] =
            g_aObjectTypeData[
                g_acObjectType[obj]].damageCapacity;
    } else {
        g_asCapitalHitDamageStepTable[obj] = (short)(
            g_aObjectTypeData[
                g_acObjectType[obj]].damageCapacity / 2 + 1);
    }
    if (ShipHasCloakingDevice(obj) != 0) {
        g_anShipCloakState[obj] = 0;
        g_aShipWeapons[obj][0]--;
    } else {
        g_anShipCloakState[obj] = 2;
    }
    g_asShipCloakCooldown[obj] = 0;
    reset_mission_type(obj, record->missionType);
    g_asShipMissionParameter[obj] = record->missionParameter;
    g_asShipWingLeader[obj] =
        find_ship_index(record->leaderMissionIndex);
    set_formation_position(obj, record);
    zero_vector(&g_aShipVelocity[obj]);
    init_intelligence_data(obj);
    if (record->name[0] == '?')
        g_asShipIdentified[obj] = 0;
    else
        g_asShipIdentified[obj] = 1;
    g_apShipMissionRecord[obj] = record;
    g_abPilotEjectionAttempted[obj] = 0;
    g_abShipEjectionSequenceEnabled[obj] = record->field_19;
    g_13_abShipArrivalMessageSent[obj] = 0;
    if (g_asObjectType[obj] >= 0x39)
        g_2_asCarrierLaunchTimer[obj] = 0;
}

/* Function start: 0x44E028 */
short is_team_member(short missionShip)
{
    short index;

    if (g_13_stMissionHeaderBlock.playerMissionShip == missionShip)
        return 1;
    for (index = 0; index < 8; index++) {
        if (g_13_stMissionHeaderBlock.initialMissionShips[index] == missionShip)
            return 1;
    }
    return 0;
}

/* Function start: 0x44E09C */
void find_next_ship_turn_slot(short obj)
{
    short other;

    g_acTurnInterval[obj] = (signed char)
        g_anPilotTurnInterval[g_asPilotLevel[obj]];
    g_acTurnRegulator[obj] = 1;
    for (other = 1; other <= 9; other++) {
        if (g_aeObjectClass[other] == OBJECT_CLASS_SHIP &&
            other != obj &&
            g_acTurnRegulator[obj] ==
                g_acTurnRegulator[other] &&
            g_acTurnInterval[other] ==
                g_acTurnInterval[obj]) {
            g_acTurnRegulator[obj]++;
            other = 1;
            if (g_acTurnInterval[obj] <
                g_acTurnRegulator[obj])
                return;
        }
    }
}

/* Function start: 0x44E187 */
short init_ship(short missionShip, short navPoint)
{
    MissionShipRecord *record;
    FixedVector hazardCenter;
    short shipType;
    short objectType;
    short obj;

    if (missionShip == -1)
        return -1;
    record = &g_aMissionShips[missionShip];
    shipType = record->type;
    objectType = record->objectType;

    if (objectType == 0x3d || objectType == 0x2c) {
        obj = initialize_ship(objectType, -1, 0);
        g_asShipMissionIndex[obj] = missionShip;
        AddFixedVectors(&g_aMissionNavPoints[navPoint].position,
                        &record->position,
                        &g_aShipPosition[obj]);
        g_asShipSide[obj] = 0;
    } else if (objectType == 5 || objectType == 6) {
        AddFixedVectors(&g_aMissionNavPoints[navPoint].position,
                        &record->position, &hazardCenter);
        add_hazard_field(objectType, hazardCenter,
                         (short)(record->speed + 3000),
                         record->pilot);
    } else {
        if (find_ship_index(missionShip) != -1 || record->state != 0)
            return -1;
        if (is_alive((signed char)record->pilot) == 0) {
            if (record->pilot == 5 && record->side == 0)
                return -1;
            else
                record->pilot = 3;
        }
        record->navPoint = (signed char)navPoint;
        if (is_team_member(missionShip) != 0)
            navPoint = -1;
        if (record->field_13 != 0)
            LoadAlternateShipType(record->type, record->field_13);
        obj = initialize_ship(shipType, -1, 1);
        if (obj != -1) {
            if (g_nYourWingman == -1 && record->pilot > 4 &&
                record->side == 0)
                g_nYourWingman = obj;
            Set_up_ship_info(obj, missionShip, (signed char)navPoint);
            if (record->field_13 != 0)
                ApplyAlternateShipType(obj);
#ifdef SDL_PORT
            /* SO2.exe 0x43A980: the Gamal Gan reuses the Dorkathi
             * transport's resource, drawn at scale 2000 with a collision
             * radius of 10. The name is compared without regard to case. */
            if (g_nSdlCampaign == 2 &&
                SDL_strcasecmp(record->name, "Gamal Gan") == 0) {
                g_asObjectCollisionRadius[obj] = 10;
                g_asObjectScale[obj] = 2000;
            }
#endif
            find_next_ship_turn_slot(obj);
            check_futurion(obj);
        }
    }
    return obj;
}

#pragma function(strcat, strcpy)
void clear_alert(short obj);
void LoadCommPortraitResources(short portrait);

/* Function start: 0x44E44F */
void init_intelligence_data(short obj)
{
    char filename[16];
    short usedSlots[5];
    short matched;
    short navEntry;
    ShipIntelligenceMetadata *meta;
    short shipIntelFile;
    short intelNum;
    char intelDigits[8];
    short member;
    short index;

    meta = AllocateTaggedMemory(0x0c, 0x40);
    g_abShipTurn[obj] = 0;
    clear_alert(obj);
    g_aeSpecialManeuver[obj] = SPECIAL_MANEUVER_NONE;
    g_asShipIntelState[obj] = 0;
    index = g_asShipMissionParameter[obj];
    g_aShipMissionSpot[obj] = g_aMissionNavPoints[
        g_nCurrentNavPoint].position;

    switch (g_asShipMissionType[obj]) {
    case 0:
        break;
    case MISSION_TYPE_ESCORT:
    case MISSION_TYPE_DEFEND:
    case MISSION_TYPE_WINGMAN:
        g_asShipMissionParameter[obj] = index;
        break;
    case MISSION_TYPE_STRIKE:
        g_asShipMissionParameter[obj] = index;
        for (navEntry = 0; navEntry < 10; navEntry++) {
            for (member = 0; member < 10;
                 member++) {
                if (g_aMissionNavPoints[navEntry]
                        .missionShips[member] ==
                    g_asShipMissionParameter[obj]) {
                    index = navEntry;
                }
            }
        }
#ifdef SDL_PORT
        if (index < 0 || index >= MISSION_NAV_POINT_COUNT) {
            /* No nav point lists this target, so index is still its
             * mission-ship number.  Retail reads that "nav point" past the
             * ten-entry table: six bytes of padding later, the mission-ship
             * records start 0x3F8 bytes after the table, at 0x492290.  The
             * raiders aimed at targets 10 and 15 in series 6's first and
             * series 4's fourth missions take their spot from those record
             * bytes.  Copy the same bytes; an index whose read would leave
             * the records keeps the current nav point. */
            int retailOffset = index * (int)sizeof(MissionNavPoint) +
                               (int)offsetof(MissionNavPoint, position) -
                               0x3f8;

            if (retailOffset >= 0 &&
                retailOffset <= (int)(sizeof(g_aMissionShips) -
                                      sizeof(FixedVector)))
                memcpy(&g_aShipMissionSpot[obj],
                       (unsigned char *)g_aMissionShips + retailOffset,
                       sizeof(FixedVector));
            break;
        }
#endif
        g_aShipMissionSpot[obj] =
            g_aMissionNavPoints[index].position;
        break;
    case MISSION_TYPE_GOTO_WARP:
        g_aShipMissionSpot[obj] =
            g_aMissionNavPoints[index].position;
        break;
    case MISSION_TYPE_WARP_ARRIVE:
        g_asShipTactic[obj] = TACTIC_WARP_IN;
        g_asShipManeuver[obj] = MANEUVER_WARPING_IN;
        if (g_asShipSide[obj] == SIDE_KILRATHI)
            break;
        /* fall through */
    case 9: /* WC2's home-base mission value in this dispatch table. */
        locate_ship(g_13_stMissionHeaderBlock.homeMissionShip,
                    &g_aShipMissionSpot[obj]);
        break;
    }

    g_acShipStress[obj] = 0;
    g_asShipIntelSlot[obj] = -1;
    if (obj != 0 && g_aeObjectClass[obj] == OBJECT_CLASS_SHIP &&
        (obj == g_nYourWingman ||
         g_asShipSide[obj] != SIDE_IMPERIAL)) {
        shipIntelFile = 0;
        if (g_asPilotLevel[obj] == RATING_ACE_SPIRIT) {
            intelNum = g_acShipPortrait[obj];
#ifdef SDL_PORT
            /* SO1's portrait-21 aces (Landis in S4M0, Zoe in S5M0) have no
             * INTEL.021 in any release, and retail exits with a disk error
             * if one becomes the player's wingman (see cruise_home).  Fly
             * them on their ship type's file, as other pilots fly. */
            snprintf(filename, sizeof(filename), "intel.0%02d", intelNum);
            if (GetNamedPacketSize(filename, 0) <= 0) {
                intelNum = g_aObjectTypeData[
                    g_acObjectType[obj]].resourceType;
                shipIntelFile = 1;
            }
#endif
        } else if (g_asPilotLevel[obj] < RATING_ACE_SPIRIT) {
            intelNum = g_aObjectTypeData[
                g_acObjectType[obj]].resourceType;
            shipIntelFile = 1;
        } else {
            ReportFatalErrorCode("024");
        }

        for (index = 0; index < 5; index++)
            usedSlots[index] = 0;

        matched = 0;
        for (index = 0; index < 10; index++) {
            if (g_asShipIntelSlot[index] != -1) {
                usedSlots[
                    g_asShipIntelSlot[index]] = 1;
            }
            if (g_asShipIntelResourceKey[index] ==
                shipIntelFile * 100 + intelNum) {
                matched = 1;
                g_asShipIntelSlot[obj] =
                    g_asShipIntelSlot[index];
                g_asShipIntelResourceKey[obj] =
                    (short)(shipIntelFile * 100 + intelNum);
                break;
            }
        }
        if (matched == 0) {
            for (index = 0; index < 5; index++) {
                if (usedSlots[index] == 0)
                    break;
            }
            g_asShipIntelSlot[obj] = index;
            g_asShipIntelResourceKey[obj] =
                (short)(shipIntelFile * 100 + intelNum);

            strcpy(filename, "intel.0");
            if (shipIntelFile != 0)
                strcpy(filename, "intelshp.0");
            if (intelNum < 10)
                strcat(filename, "0");
            itoa((int)intelNum, intelDigits, 10);
            strcat(filename, intelDigits);

            LoadPacketIntoBuffer(
                filename, 0,
                g_aShipIntelligenceData[
                    g_asShipIntelSlot[obj]],
                0);
            if (obj == g_nYourWingman) {
#ifdef SDL_PORT
                /* Section 1 is a twelve-byte ShipIntelligenceMetadata, and
                 * the original reads it straight over the run of wingman
                 * behaviour shorts starting at 0x496138 -- the record's first
                 * eight bytes are those four modes.  They are separate
                 * objects here, so read the record and place them by hand.
                 * The music and comm fields that follow are only ever read
                 * from the meta copy in the branch below, so nothing
                 * wants the last four bytes on this path. */
                /* A ship type's file has no section 1; such a wingman keeps
                 * the modes init_mission cleared. */
                if (shipIntelFile == 0) {
                    ShipIntelligenceMetadata wingmanIntel;

                    LoadPacketIntoBuffer(filename, 1,
                                         &wingmanIntel, 0);
                    g_stWingmanIntelligence.routDecisionMode =
                        wingmanIntel.routDecisionMode;
                    g_stWingmanIntelligence.formationDisobeyMode =
                        wingmanIntel.formationDisobeyMode;
                    g_stWingmanIntelligence.targetingMode =
                        wingmanIntel.targetingMode;
                    g_stWingmanIntelligence.engagementMode =
                        wingmanIntel.engagementMode;
                }
#else
                LoadPacketIntoBuffer(
                    filename, 1,
                    &g_stWingmanIntelligence, 0);
#endif
                LoadCommPortraitResources(g_acShipPortrait[obj]);
                g_nLoadedCommPortraitPilot = -1;
                if (g_pCommPortraitResource != 0) {
                    g_nLoadedCommPortraitPilot =
                        g_acShipPortrait[obj];
                }
            } else if (shipIntelFile == 0) {
                LoadPacketIntoBuffer(filename, 1, meta, 1);
                if (meta->musicTrack != 0) {
                    g_nMissionMusicTrackOverride =
                        meta->musicTrack;
                }
                if (meta->enemyCommCommandBase != 0) {
                    g_1_nEnemyCommCommandBase =
                        meta->enemyCommCommandBase;
                    g_nEnemyCommPilotIndex =
                        g_acShipPortrait[obj];
                }
            }
        }
    }
#ifdef SDL_PORT
    /* Retail never frees this buffer, so each ship set up leaks it and a
     * packet handle (see load_ship). */
    ReleasePacketHandle(meta);
#endif
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short g_1_nEnemyCommCommandBase; /* 0x005d179c */
short g_nEnemyCommPilotIndex; /* 0x005d179e */
short g_asCapitalHitDamageStepTable[10]; /* 0x005d17a0 */
