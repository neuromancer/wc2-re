/*
 *  Mission setup, action spheres and ship loading.
 *
 *  Retail object 0x44B200-0x44EA4F (link order; see docs/ORDER.md).
 */
#include "game.h"

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
        &g_aMissionNavPoints_00491e98[record->navPoint].position,
        &record->position, position);
}

/* Function start: 0x44B239 */
short is_alive(signed char pilot)
{
    return g_pPilotStatus_005d2fcc[pilot];

}

/* Function start: 0x44B257 */
void check_futurion(short i)
{
    if (g_asShipMissionType_00495de8[i] == MISSION_TYPE_WARP_ARRIVE) {
        g_asObjectCounter_00494be0[i] = g_aeObjectClass_00495328[i];
        g_aeObjectClass_00495328[i] = OBJECT_CLASS_FUTURION;
    }
}

/* Function start: 0x44B29D */
void ResetWeaponDisplayPositions(void)
{
    short index;

    for (index = 0; index < 16; index++)
        g_aWeaponDisplayPositions_005d1de0[index] =
            g_aDefaultWeaponDisplayPositions_0049afa0[index];
}

/* Function start: 0x44B2E2 */
void init_mission(short series, short mission)
{
    short index;

    g_bTargetLockActive_0049ae80 = 0;
    g_bMissileLockAcquired_0049b2b0 = 0;
    g_nLastAdaptiveDifficultyChangeFrame_00492d60 = 0;
    g_ucPendingEjectionTransition_0049b8ac = 0xff;
    g_nEnemyCommCommandBase_005d179c = 20;
    g_nEnemyCommPilotIndex_005d179e = -1;
    g_nCommDeathSequenceFrame_0049ae84 = 0;
    g_bDisplayWingmanTargetData_0049347c = 0;
    LoadMissionData(series, mission);
    init_3Space_objects(series);
    g_nMissionResourceBudget_005c8de4 = 0x7c0600;
    LoadPacketResourceList(g_aMissionResourceDescriptors_0049c798, 0,
                           g_nAvailableGameMemory_005c8de0,
                           "objects.vga");
    LoadPacketIntoBuffer("difflevl.000", 0,
                         g_asDifficultyLevels_004930a8, 0);
    g_nEnemyTauntCommandBase_0049b76c = 20;
    g_bFriendlyFireWarningIssued_00492d5c = 0;
    g_nAdaptiveDifficulty_005d3844 = 5;
    for (index = 0; index < 10; index++) {
        g_asShipIntelSlot_00495d30[index] = -1;
        g_asShipIntelResourceKey_00495d48[index] = -1;
    }
    g_nWingmanRoutDecisionMode_00496138 = 0;
    g_nWingmanFormationDisobeyMode_0049613a = 0;
    g_nWingmanTargetingMode_0049613c = 0;
    g_nWingmanEngagementMode_0049613e = 0;
    prepare_mission();
    g_bCockpitDamageFrame0Shown_0049b2b4 = 0;
    g_bCockpitDamageFrame2Shown_0049b2b8 = 0;
    g_bFuelGaugeDamaged_0049b054 = 0;
    InitializeCockpitResources(g_nPlayerShipType_00493464);
    g_bMissionDeathSequencePending_0049b720 = 0;
    for (index = 0; index < 5; index++)
        g_acJumpDriveUsedBySystem_005d2fe8[index] = 0;
    ResetWeaponDisplayPositions();
    g_nCurrentStarSystem_005d169c =
        g_aMissionNavPoints_00491e98[
            g_stMissionHeader_005d3e70.entryNavPoint].systemIndex;
}

/* Function start: 0x44B48F */
/* Re-describe an object from the alternate ship record that
 * LoadAlternateShipType leaves in object-type slot 4. */
void ApplyAlternateShipType(short obj)
{
    ObjectTypeData *typeData;
    short weapon;
    short remaining;

    typeData = &g_aObjectTypeData_00496d30[4];
    g_acObjectType_00493980[obj] = 4;
    g_apObjectShape_00493868[obj] = typeData->shapeSet;
    g_apObjectExhaustShape_004953b8[obj] = typeData->animation;
    g_asObjectType_00495298[obj] = typeData->field_18;
    g_aeObjectClass_00495328[obj] = typeData->objectClass;
    init_ijk(obj);
    g_asObjectCollisionRadius_004950e8[obj] = typeData->collisionRadius;
    g_asObjectScale_00494d90[obj] = typeData->scale;
    g_asObjectDamage_00495178[obj] = 0;
    g_asObjectFlip_004939c8[obj] = g_asObjectDamage_00495178[obj];
    g_asObjectScreenAngle_004936b8[obj] = g_asObjectFlip_004939c8[obj];
    g_acObjectOwner_00495208[obj] = -1;
    g_acLastCollisionObject_00495250[obj] = -1;
    g_asObjectViewFrame_00493508[obj] = 0;
    g_acShipTarget_00495f20[obj] = -1;
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
    g_anShipFuel_00495638[obj] = *(const int *)&typeData->lifetime;
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
        weapon = (short)(signed char)g_aShipWeapons_004956b0[obj][0];
        remaining = weapon;
        while (remaining > 0) {
            weapon--;
            if (((ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1])[
                    weapon].disabled == 0) {
                if (g_aObjectTypeData_00496d30[
                        ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1])[
                            weapon].weaponType].objectClass == 8) {
                    if (g_nSelectedGunType_004934dc != -1 &&
                        ((ShipWeaponSlot *)
                             &g_aShipWeapons_004956b0[obj][1])[weapon].type !=
                            g_nSelectedGunType_004934dc)
                        g_nSelectedGunType_004934dc = 0x80;
                    else
                        g_nSelectedGunType_004934dc =
                            ((ShipWeaponSlot *)
                                 &g_aShipWeapons_004956b0[obj][1])[
                                weapon].type;
                } else {
                    g_nSelectedReleaseWeaponIndex_004934e0 = weapon;
                }
            }
            remaining = weapon;
        }
    }

    if (HasShipCockpitGunDisplay(obj) != 0)
        g_asShipTurretFireEnabled_0049d470[obj] = 1;
    else
        g_asShipTurretFireEnabled_0049d470[obj] = 0;
    g_asShipWeaponEnergy_00495590[obj] = 100;
    g_acShipLastAttacker_004955c0[obj] = -1;
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
    strcpy(nameCopy, g_aObjectTypeData_00496d30[slot].displayName);
    keptAnimation = g_aObjectTypeData_00496d30[slot].animation;
    savedShapeSet = g_aObjectTypeData_00496d30[slot].shapeSet;
    savedShape = g_aObjectTypeData_00496d30[slot].shape;
    strcpy(filename, "altships.");
    if (g_nSelectedCampaignSlot_005d3bf2 < 100)
        strcat(filename, "0");
    if (g_nSelectedCampaignSlot_005d3bf2 < 10)
        strcat(filename, "0");
    strcat(filename,
           _itoa(g_nSelectedCampaignSlot_005d3bf2, numberText, 10));
    LOAD_OBJECT_TYPE_RECORD(filename, logicalFile,
                                &g_aObjectTypeData_00496d30[4]);
    strcpy(g_aObjectTypeData_00496d30[4].displayName, nameCopy);
    g_aObjectTypeData_00496d30[4].resourceType = resourceType;
    g_aObjectTypeData_00496d30[4].animation = keptAnimation;
    g_aObjectTypeData_00496d30[4].shapeSet = savedShapeSet;
    g_aObjectTypeData_00496d30[4].shape = savedShape;
}

#pragma intrinsic(strcat, strcpy)
/* Function start: 0x44B981 */
/* Re-derive the nav point of every mission ship an objective names, so the
 * briefing map and the objective list agree on where each one is. */
void RefreshObjectiveShipNavPoints(void)
{
    short source;
    short missionShip;
    short type;

    for (source = 0; source < 8; source++) {
        type = g_aMissionObjectiveSources_005d3c70[source].type;
        if (type != -1) {
            switch (type) {
            case 1:
            case 2:
            case 3:
            case 4:
            case 6:
                missionShip =
                    g_aMissionObjectiveSources_005d3c70[source].index;
                if (g_bBriefingMapActive_0049bcb0 != 0 || type == 6 ||
                    g_aMissionShips_00492290[missionShip].navPoint != -1)
                    g_aMissionShips_00492290[missionShip].navPoint =
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

    g_nMissionScore_00493462 = 0;
    g_cPlayerKillCount_005d2fa8 = 0;
    for (slot = 0; slot < 7; slot++)
        g_acInitialShipKillCount_005d2fc0[slot] = 0;
    g_bWingmanKilledThisMission_005d2fb8 = 0;
    g_bPlayerDestroyed_005d2fa4 =
        g_bWingmanKilledThisMission_005d2fb8;

    playerMissionShip = g_stMissionHeader_005d3e70.playerMissionShip;
    g_nPlayerShipType_00493464 =
        g_aMissionShips_00492290[playerMissionShip].type;
    objectKind = OBJECT_CLASS_SHIP;
    load_ship(g_nPlayerShipType_00493464,
              g_aMissionShips_00492290[playerMissionShip].objectType,
              objectKind, 0);
    if (g_aMissionShips_00492290[playerMissionShip].field_13 != 0) {
        LoadAlternateShipType(
            g_nPlayerShipType_00493464,
            g_aMissionShips_00492290[playerMissionShip].field_13);
        ApplyAlternateShipType(0);
    } else {
        set_objects_data(0, g_nPlayerShipType_00493464, -1, 1);
    }
    g_aMissionShips_00492290[playerMissionShip].navPoint =
        (signed char)g_stMissionHeader_005d3e70.entryNavPoint;
    if (g_nMissionEntryNavOverride_0049d790 != -1) {
        g_aMissionShips_00492290[playerMissionShip].navPoint =
            (signed char)g_nMissionEntryNavOverride_0049d790;
    }
    Set_up_ship_info(0, playerMissionShip, -1);

    for (slot = 0; slot < 9; slot++)
        g_acPlayerComponentDamage_00493470[slot] = 0;
    g_bMissionShipInitializationInProgress_0049b890 = 1;
    g_nYourWingman_0049346c = -1;
    for (slot = 0; slot < 8; slot++) {
        playerMissionShip =
            g_stMissionHeader_005d3e70.initialMissionShips[slot];
#ifdef SDL_PORT
        /* S3M3 carries -2 in one otherwise empty initial-ship slot.  Retail
         * walks into the preceding nav table and eventually treats its zeroes
         * as a ship; negative mission-ship values are sentinels on the port. */
        if (playerMissionShip < 0)
            continue;
#endif
        if (playerMissionShip != -1) {
            portraitIndex = g_aMissionShips_00492290[playerMissionShip].portrait;
            if (portraitIndex == 25 ||
                g_aMissionShips_00492290[playerMissionShip].pilot != 5 ||
                is_alive(portraitIndex) != 0) {
                if (find_ships_sphere(playerMissionShip) == -1) {
                    init_ship(playerMissionShip,
                              g_stMissionHeader_005d3e70.entryNavPoint);
                }
            }
        }
    }
    g_bMissionShipInitializationInProgress_0049b890 = 0;
    Build_objective_list();
    g_nHomeMissionShipIndex_005d1e22 =
        g_stMissionHeader_005d3e70.homeMissionShip;
    g_bLandingCommRequestPending_00492fa0 = 0;
    g_bTargetLockMode_00493500 = 0;
}

void FreePacketAndClear(void *slot, unsigned short releaseFlags);
void ReportFatalErrorCode(const char *errorCode);
void release_capital_ship_shapes(short resourceType);
void free_ship(short slot);
short FindObjectResourceSlot(short resourceType);
void ReleasePacketHandle(void *handle);
void add_hazard_field(short type, FixedVector center, short radius, short density);
void flag_objective(short objective, unsigned char flags);
void *FetchDiskPacketRetrying(char *fileName, short section, unsigned short flags);

/* Function start: 0x44BCF7 */
void release_all_capital_ship_shapes(void)
{
    short obj;

    for (obj = 0; obj < 10; obj++) {
        if (g_aeObjectClass_00495328[obj] >=
                OBJECT_CLASS_CAPITAL_SHIP ||
            (g_bExpandedShipGraphicsEnabled_004931a4 != 0 &&
             g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP)) {
            FreePacketAndClear(&g_apObjectShape_00493868[obj], 0);
            g_asLoadedShipViewFrame_00495d18[obj] = -1;
        }
    }
}

/* Function start: 0x44BD83 */
void release_capital_ship_shapes(short resourceType)
{
    short slot;
    short obj;

    for (slot = 0; slot < 5; slot++) {
        if (g_aObjectTypeData_00496d30[slot].resourceType == resourceType)
            break;
    }
    if (g_aObjectTypeData_00496d30[slot].objectClass >=
            OBJECT_CLASS_CAPITAL_SHIP ||
        (g_bExpandedShipGraphicsEnabled_004931a4 != 0 &&
         g_aObjectTypeData_00496d30[slot].objectClass ==
             OBJECT_CLASS_SHIP)) {
        for (obj = 1; obj < 10; obj++) {
            if (g_aObjectTypeData_00496d30[
                    g_acObjectType_00493980[obj]].resourceType ==
                resourceType) {
                FreePacketAndClear(&g_apObjectShape_00493868[obj], 0);
                g_asLoadedShipViewFrame_00495d18[obj] = -1;
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
        if (g_aObjectResourceSlots_00493398[slot].resourceType ==
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
    short object;
    void **packetReferences;

    if (resourceType != -1) {
        if (g_aObjectResourceSlots_00493398[slot].shapeSet == 0) {
            g_aObjectResourceSlots_00493398[slot].resourceType = resourceType;
            if (objectClass == OBJECT_CLASS_MISSILE)
                strcpy(fileName, "missile.v");
            else
                strcpy(fileName, "ship.v");
            if (g_bExpandedShipGraphicsEnabled_004931a4 != 0 &&
                objectClass == OBJECT_CLASS_SHIP) {
                objectClass = OBJECT_CLASS_CAPITAL_SHIP;
                strcpy(fileName, "xship.v");
            }
            _itoa((int)resourceType, resourceNumber, 10);
            if (strlen(resourceNumber) == 1)
                strcat(fileName, "0");
            strcat(fileName, resourceNumber);
            g_aObjectResourceSlots_00493398[slot].objectType = objectType;
            g_aObjectResourceSlots_00493398[slot].objectClass = objectClass;

            if (objectType == 0x3d) {
                g_aObjectTypeData_00496d30[46].shapeSet =
                    FetchDiskPacketRetrying("objects.vga", 0x13, 0);
                g_aObjectResourceSlots_00493398[slot].shapeSet =
                    g_aObjectTypeData_00496d30[46].shapeSet;
                for (object = 1; object < 10; object++) {
                    if (g_asObjectType_00495298[object] == 0x3d) {
                        g_apObjectShape_00493868[object] =
                            g_aObjectTypeData_00496d30[
                                g_acObjectType_00493980[object]].shapeSet;
                    }
                }
            } else if (objectType == 0x2c) {
                if (g_aObjectTypeData_00496d30[44].shapeSet == 0) {
                    g_aObjectTypeData_00496d30[44].shapeSet =
                        FetchDiskPacketRetrying("pilotanm.vga", 2, 0);
                    g_aObjectResourceSlots_00493398[slot].shapeSet =
                        g_aObjectTypeData_00496d30[44].shapeSet;
                }
                for (object = 1; object < 70; object++) {
                    if (g_asObjectType_00495298[object] == 0x2c) {
                        g_apObjectShape_00493868[object] =
                            g_aObjectTypeData_00496d30[
                                g_acObjectType_00493980[object]].shapeSet;
                    }
                }
            } else if (objectType == 5) {
                g_aObjectTypeData_00496d30[26].shapeSet =
                    FetchDiskPacketRetrying("objects.vga", 0x10, 0);
                g_aObjectTypeData_00496d30[24].shapeSet =
                    g_aObjectTypeData_00496d30[26].shapeSet;
                g_aObjectTypeData_00496d30[22].shapeSet =
                    g_aObjectTypeData_00496d30[24].shapeSet;
                g_aObjectResourceSlots_00493398[slot].shapeSet =
                    g_aObjectTypeData_00496d30[22].shapeSet;
                if (g_nMemoryConfiguration_005c8dc8 == 2) {
                    g_aObjectTypeData_00496d30[27].shapeSet =
                        FetchDiskPacketRetrying("objects.vga", 0x11, 4);
                    g_aObjectTypeData_00496d30[25].shapeSet =
                        g_aObjectTypeData_00496d30[27].shapeSet;
                    g_aObjectTypeData_00496d30[23].shapeSet =
                        g_aObjectTypeData_00496d30[25].shapeSet;
                }
                for (object = 10; object <= SPACE_LAST_MOVING_OBJECT;
                     object++) {
                    if (g_aeObjectClass_00495328[object] ==
                        OBJECT_CLASS_ASTEROID) {
                        g_apObjectShape_00493868[object] =
                            g_aObjectTypeData_00496d30[
                                g_acObjectType_00493980[object]].shapeSet;
                    }
                }
            } else if (objectType == 6) {
                if (g_aObjectTypeData_00496d30[21].shapeSet == 0) {
                    g_aObjectTypeData_00496d30[21].shapeSet =
                        FetchDiskPacketRetrying("objects.vga", 0x0f, 0);
                    g_aObjectResourceSlots_00493398[slot].shapeSet =
                        g_aObjectTypeData_00496d30[21].shapeSet;
                }
                for (object = 10; object <= SPACE_LAST_MOVING_OBJECT;
                     object++) {
                    if (g_asObjectType_00495298[object] == 0x15)
                        g_apObjectShape_00493868[object] =
                            g_aObjectTypeData_00496d30[21].shapeSet;
                }
            } else if ((objectClass == OBJECT_CLASS_SHIP &&
                        objectType != 0x33) ||
                       objectClass == OBJECT_CLASS_MISSILE) {
                if (objectClass != OBJECT_CLASS_MISSILE &&
                    g_bShipResourceReloadInProgress_0049b894 == 0) {
                    LOAD_OBJECT_TYPE_RECORD(
                        fileName, 3,
                        &g_aObjectTypeData_00496d30[slot]);
                }
                g_aObjectResourceSlots_00493398[slot].shapeSet =
                    FetchDiskPacketRetrying(fileName, 0, 0);
                g_aObjectTypeData_00496d30[slot].shapeSet =
                    g_aObjectResourceSlots_00493398[slot].shapeSet;
                g_aObjectResourceSlots_00493398[slot].animation =
                    FetchDiskPacketRetrying(fileName, 2, 0);
                g_aObjectTypeData_00496d30[slot].animation =
                    g_aObjectResourceSlots_00493398[slot].animation;
                g_aObjectResourceSlots_00493398[slot].shape =
                    FetchDiskPacketRetrying(fileName, 1, 0);
                g_aObjectTypeData_00496d30[slot].shape =
                    g_aObjectResourceSlots_00493398[slot].shape;
                if (objectClass == OBJECT_CLASS_SHIP &&
                    g_bHighMemoryResourcesEnabled_005c80e4 != 0 &&
                    g_nResourcePaletteMode_005c57e6 == 0) {
                    g_aObjectResourceSlots_00493398[slot].field_12 =
                        FetchDiskPacketRetrying(fileName, 4, 4);
                    g_aObjectTypeData_00496d30[32].shapeSet =
                        g_aObjectResourceSlots_00493398[slot].field_12;
                }
                for (object = 0; object < 10; object++) {
                    if (g_aeObjectClass_00495328[object] >
                            OBJECT_CLASS_MINE &&
                        g_aObjectTypeData_00496d30[
                            g_acObjectType_00493980[object]].resourceType ==
                            resourceType) {
                        g_apObjectExhaustShape_004953b8[object] =
                            g_aObjectResourceSlots_00493398[slot].animation;
                        g_apObjectShape_00493868[object] =
                            g_aObjectResourceSlots_00493398[slot].shapeSet;
                    }
                }
                if (objectClass == OBJECT_CLASS_MISSILE) {
                    g_aObjectTypeData_00496d30[objectType].shapeSet =
                        g_aObjectTypeData_00496d30[slot].shapeSet;
                    g_aObjectTypeData_00496d30[objectType].animation =
                        g_aObjectTypeData_00496d30[slot].animation;
                    g_aObjectTypeData_00496d30[objectType].shape =
                        g_aObjectTypeData_00496d30[slot].shape;
                }
            } else {
                if (g_bHighMemoryResourcesEnabled_005c80e4 != 0 &&
                    g_nResourcePaletteMode_005c57e6 == 0) {
                    g_apPacketReferenceGroups_0049b898[slot] =
                        (void **)calloc(0x25, HOST_POINTER_SIZE);
                    packetReferences =
                        g_apPacketReferenceGroups_0049b898[slot];
                    if (packetReferences == 0)
                        ReportFatalErrorCode("023");
                    section = 0;
                    while (section < 0x25) {
                        packetReferences[section] =
                            FetchDiskPacketRetrying(fileName, section, 4);
                        if (packetReferences[section] == 0)
                            break;
                        section++;
                    }
                    g_aObjectResourceSlots_00493398[slot].field_12 =
                        FetchDiskPacketRetrying(fileName, 0x27, 4);
                }
                LOAD_OBJECT_TYPE_RECORD(
                    fileName, 0x26,
                    &g_aObjectTypeData_00496d30[slot]);
                g_aObjectTypeData_00496d30[slot].shapeSet = 0;
                g_aObjectResourceSlots_00493398[slot].shape =
                    FetchDiskPacketRetrying(fileName, 0x25, 0);
                g_aObjectTypeData_00496d30[slot].shape =
                    g_aObjectResourceSlots_00493398[slot].shape;
                g_aObjectResourceSlots_00493398[slot].animation =
                    FetchDiskPacketRetrying(fileName, 0x28, 0);
                g_aObjectTypeData_00496d30[slot].animation =
                    g_aObjectResourceSlots_00493398[slot].animation;
                for (object = 0; object < 10; object++) {
                    if (g_aObjectTypeData_00496d30[
                            g_acObjectType_00493980[object]].resourceType ==
                        resourceType) {
                        g_apObjectExhaustShape_004953b8[object] =
                            g_aObjectResourceSlots_00493398[slot].animation;
                        FreePacketAndClear(
                            &g_apObjectShape_00493868[object], 0);
                        g_asLoadedShipViewFrame_00495d18[object] = -1;
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

    resourceType = g_aObjectResourceSlots_00493398[slot].resourceType;
    if (g_aObjectResourceSlots_00493398[slot].objectClass >=
            OBJECT_CLASS_CAPITAL_SHIP ||
        g_aObjectResourceSlots_00493398[slot].objectType == 0x33 ||
        (g_bExpandedShipGraphicsEnabled_004931a4 != 0 &&
         g_aObjectResourceSlots_00493398[slot].objectClass ==
             OBJECT_CLASS_SHIP)) {
        if (g_nResourcePaletteMode_005c57e6 == 0)
            release_capital_ship_shapes(resourceType);
        if (g_bHighMemoryResourcesEnabled_005c80e4 != 0 &&
            g_nResourcePaletteMode_005c57e6 == 0 &&
            g_apPacketReferenceGroups_0049b898[slot] != 0) {
            refCount = g_apPacketReferenceGroups_0049b898[slot];
            for (section = 0; section < 0x25; section++) {
                FreePacketAndClear(&refCount[section], 8);
            }
            ReleasePacketHandle(refCount);
            g_apPacketReferenceGroups_0049b898[slot] = 0;
        }
        FreePacketAndClear(
            &g_aObjectResourceSlots_00493398[slot].shape, 8);
        g_aObjectTypeData_00496d30[slot].shape = 0;
        FreePacketAndClear(
            &g_aObjectResourceSlots_00493398[slot].animation, 8);
        if (g_nResourcePaletteMode_005c57e6 == 0) {
            FreePacketAndClear(
                &g_aObjectResourceSlots_00493398[slot].field_12, 8);
        }
    }
    if (g_aObjectResourceSlots_00493398[slot].shapeSet == 0)
        return;
    FreePacketAndClear(
        &g_aObjectResourceSlots_00493398[slot].shapeSet, 8);
    if (g_aObjectResourceSlots_00493398[slot].objectType == 0x3d) {
        for (obj = 1; obj < 10; obj++) {
            if (g_asObjectType_00495298[obj] == 0x3d)
                g_apObjectShape_00493868[obj] = 0;
        }
        return;
    }
    if (g_aObjectResourceSlots_00493398[slot].objectType == 0x2c) {
        for (obj = 1; obj < SPACE_OBJECT_COUNT; obj++) {
            if (g_asObjectType_00495298[obj] == 0x2c)
                g_apObjectShape_00493868[obj] = 0;
        }
        return;
    }
    if (g_aObjectResourceSlots_00493398[slot].objectType == 5) {
        FreePacketAndClear(
            &g_aObjectTypeData_00496d30[28].shapeSet, 8);
        FreePacketAndClear(
            &g_aObjectTypeData_00496d30[23].shapeSet, 8);
        g_aObjectTypeData_00496d30[27].shapeSet = 0;
        g_aObjectTypeData_00496d30[26].shapeSet =
            g_aObjectTypeData_00496d30[27].shapeSet;
        g_aObjectTypeData_00496d30[25].shapeSet =
            g_aObjectTypeData_00496d30[26].shapeSet;
        g_aObjectTypeData_00496d30[24].shapeSet =
            g_aObjectTypeData_00496d30[25].shapeSet;
        for (obj = 10; obj <= SPACE_LAST_MOVING_OBJECT; obj++) {
            if (g_asObjectType_00495298[obj] == 0x1c)
                remove_object(obj);
            else if (g_aeObjectClass_00495328[obj] ==
                     OBJECT_CLASS_ASTEROID)
                g_apObjectShape_00493868[obj] = 0;
        }
        return;
    }
    if (g_aObjectResourceSlots_00493398[slot].objectType == 6) {
        FreePacketAndClear(
            &g_aObjectTypeData_00496d30[21].shapeSet, 8);
        return;
    }
    g_aObjectTypeData_00496d30[slot].shapeSet = 0;
    FreePacketAndClear(
        &g_aObjectResourceSlots_00493398[slot].animation, 8);
    g_aObjectTypeData_00496d30[slot].animation = 0;
    FreePacketAndClear(
        &g_aObjectResourceSlots_00493398[slot].shape, 8);
    g_aObjectTypeData_00496d30[slot].shape = 0;
    if (g_nResourcePaletteMode_005c57e6 == 0) {
        FreePacketAndClear(
            &g_aObjectResourceSlots_00493398[slot].field_12, 8);
    }
    for (obj = 0; obj < 10; obj++) {
        if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_MISSILE &&
            g_acObjectType_00493980[obj] == slot)
            g_apObjectShape_00493868[obj] = 0;
    }
}

/* Function start: 0x44CC84 */
void free_all_slots(void)
{
    short slot;

    release_all_capital_ship_shapes();
    for (slot = 0; slot < 4; slot++) {
        if (g_aObjectResourceSlots_00493398[slot].resourceType != -1)
            free_ship(slot);
    }
}

/* Function start: 0x44CCE1 */
void load_all_slots(void)
{
    short slot;
    short type;
    short object;

    g_bShipResourceReloadInProgress_0049b894 = 1;
    free_view_buffer();
    release_all_capital_ship_shapes();
    for (slot = 0; slot < 4; slot++) {
        type = g_aObjectResourceSlots_00493398[slot].resourceType;
        if (type != -1) {
            load_ship(type,
                      g_aObjectResourceSlots_00493398[slot].objectType,
                      g_aObjectResourceSlots_00493398[slot].objectClass,
                      slot);
        }
    }
    for (object = 0; object < 10; object++) {
        g_apObjectExhaustShape_004953b8[object] =
            g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[object]].animation;
    }
    g_bShipResourceReloadInProgress_0049b894 = 0;
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
        if (g_aObjectResourceSlots_00493398[slot].resourceType == -1)
            return slot;
    }
    return -1;
}

/* Function start: 0x44CE68 */
short shape_loaded(short resourceType)
{
    short slot;

    for (slot = 0; slot < 5; slot++) {
        if (g_aObjectResourceSlots_00493398[slot].resourceType == resourceType)
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
        if (g_aObjectResourceSlots_00493398[slot].resourceType != -1 &&
            !shape_needed(navPoint,
                          g_aObjectResourceSlots_00493398[slot].resourceType)) {
            free_ship(slot);
            g_aObjectResourceSlots_00493398[slot].resourceType = -1;
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
    short commandOffset;
    char packetName[16];
    char numberText[4];
    short pilotIndex;
    short number;
    short transition;
    int commandType;
    int mode;

    commandOffset = 0;
    while (commandOffset < 8) {
        commandType =
            (unsigned char)navPoint->waveCommands[commandOffset];
        switch (commandType) {
        case 0:
            pilotIndex =
                (unsigned char)navPoint->waveCommands[commandOffset + 1];
            number =
                (unsigned char)navPoint->waveCommands[commandOffset + 2];
            strcpy(packetName, "altint.");
            if (number < 100)
                strcat(packetName, "0");
            if (number < 10)
                strcat(packetName, "0");
            strcat(packetName, _itoa((int)number, numberText, 10));
            LoadPacketIntoBuffer(
                packetName, 0,
                g_aShipIntelligenceData_005d3060[
                    g_asShipIntelSlot_00495d30[pilotIndex]],
                0);
            break;
        case 1:
            transition =
                (unsigned char)navPoint->waveCommands[commandOffset + 1];
            g_bEjectionWaitForEnemyWave_0049b8b0 = 0;
            g_nPendingEjectionSequenceCount_0049b8b8 = 0;
            g_bEjectionAwaitingCommCommand_0049b8b4 = 0;
            g_bEjectionTriggerImmediately_0049b8bc = 0;
            mode = (unsigned char)navPoint->waveCommands[commandOffset + 2];
            switch (mode) {
            case 0:
                g_nEjectionSequenceState_0049b8c0 = 0;
                g_ucPendingEjectionTransition_0049b8ac =
                    (signed char)transition;
                g_nPendingEjectionSequenceCount_0049b8b8 = 1;
                break;
            case 1:
                g_ucPendingEjectionTransition_0049b8ac =
                    (signed char)transition;
                break;
            case 2:
                g_ucPendingEjectionTransition_0049b8ac =
                    (signed char)transition;
                g_bEjectionWaitForEnemyWave_0049b8b0++;
                break;
            case 3:
                g_ucPendingEjectionTransition_0049b8ac =
                    (signed char)transition;
                g_bEjectionAwaitingCommCommand_0049b8b4++;
                break;
            case 4:
                ejection_sequence(
                    (unsigned char)navPoint->waveCommands[commandOffset + 1],
                    1);
                if (g_nArcadeState_0049d75c != 0)
                    return;
                break;
            case 5:
                g_ucPendingEjectionTransition_0049b8ac =
                    (signed char)transition;
                g_bEjectionWaitForEnemyWave_0049b8b0++;
                g_bEjectionTriggerImmediately_0049b8bc++;
                break;
            }
            navPoint->waveCommands[commandOffset] = -1;
            break;
        case 2:
            g_cQueuedNpcPortrait_0049b8c4 =
                navPoint->waveCommands[commandOffset + 1];
            g_cQueuedNpcMessage_0049b8c8 =
                navPoint->waveCommands[commandOffset + 2];
            break;
        }
        commandOffset = (short)(commandOffset + 3);
    }
}

#pragma intrinsic(strcat, strcpy)
/* Function start: 0x44D35D */
void set_up_action_sphere(short navPoint)
{
    short obj;
    short entry;
    short missionShip;
    short target;
    short trigger;
    MissionNavPoint *nav;

    load_all_slots();
    g_nCurrentNavPoint_004931bc = navPoint;
    nav = &g_aMissionNavPoints_00491e98[navPoint];
    g_nCurrentStarSystem_005d169c = nav->systemIndex;
    ProcessMissionWaveCommands(nav);
    if (g_aMissionNavPoints_00491e98[navPoint + 1].type == 2)
        g_nCurrentWave_004931c0 = 2;
    else
        g_nCurrentWave_004931c0 = -1;
    g_nEnemySighting_0049b670 = 0x7fff;

    for (target = 0; target < 10; target++) {
        if (g_asObjectType_00495298[target] == OBJECT_DATA_SHIP_WING) {
            remove_object(target);
        } else if (g_asObjectType_00495298[target] ==
                   OBJECT_DATA_EJECTED_PILOT) {
            remove_object(target);
            if (g_aObjectTypeData_00496d30[
                    OBJECT_DATA_EJECTED_PILOT].shapeSet != 0) {
                FreePacketAndClear(
                    &g_aObjectTypeData_00496d30[
                        OBJECT_DATA_EJECTED_PILOT].shapeSet, 0);
            }
        }
    }

    for (obj = 1; obj < 10; obj++) {
        if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_NULL)
            continue;
        if (g_acShipNavPoint_004953e0[obj] == -1)
            continue;
        if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_SHIP &&
            g_asShipMissionType_00495de8[obj] == MISSION_TYPE_ROUT) {
            g_aMissionShips_00492290[
                g_asShipMissionIndex_00495d00[obj]].state = 3;
        }
        if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_CAPITAL_SHIP ||
            (g_bExpandedShipGraphicsEnabled_004931a4 != 0 &&
             g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP))
            FreePacketAndClear(&g_apObjectShape_00493868[obj], 0);
        if (g_asObjectScreenX_00493598[obj] != -32767 &&
            g_bJumpSequenceActive_004962f0 == 0)
            explode(-1, obj);
        else
            remove_object(obj);
    }
    remove_all_hazards();
    g_nHazardFieldCount_004931d0 = 0;
    new_sphere_shapes(nav);

    for (entry = 0; entry < 10; entry++) {
        missionShip = nav->missionShips[entry];
        if (missionShip != -1)
            init_ship(missionShip, navPoint);
    }

    for (entry = 0; entry < 8; entry += 2) {
        if (((signed char *)nav->triggers)[entry] == -1)
            continue;
        trigger = ((signed char *)nav->triggers)[entry + 1];
        g_aMissionNavPoints_00491e98[trigger].type =
            ((signed char *)nav->triggers)[entry];
    }

    if (g_aMissionObjectives_004932a8[
            g_cCurrentObjective_004931cc].type == 0 &&
        g_aMissionNavPoints_00491e98[
            g_aMissionObjectives_004932a8[
                g_cCurrentObjective_004931cc].index].type == 0)
        flag_objective(g_cCurrentObjective_004931cc, 1);

    for (entry = 0; entry < g_cMissionObjectiveCount_00493294; entry++)
        LocateMobileObjective(entry);
    clean_up_cockpit();
    g_bLandingCommRequestPending_00492fa0 = 0;
    g_nEscapedEnemyCount_004962e8 = 0;
    if (g_bAutopilotSequenceActive_00493064 == 0 &&
        g_bHighMemoryBuffersReady_005d2ad8 != 0)
        ResetCannedSceneRecording();
}

short distance_from_object(short obj, short other);
void ComputeVectorDelta(FixedVector *from, FixedVector *to, FixedVector *delta);
short IsVectorWithinRange(FixedVector *vector, short range);
void random_radial(const FixedVector *center, short radius, FixedVector *position);
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
        if (other != obj && g_aeObjectClass_00495328[other] >= 8) {
            ComputeVectorDelta(&g_aShipPosition_00494550[obj],
                               &g_aShipPosition_00494550[other],
                               &g_vCollisionDelta_00493178);
            distance = (short)((g_asObjectCollisionRadius_004950e8[other] +
                             g_asObjectCollisionRadius_004950e8[obj]) * 2);
            if (IsVectorWithinRange(&g_vCollisionDelta_00493178,
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
        random_radial(&g_aShipPosition_00494550[0], maximum,
                      &g_aShipPosition_00494550[obj]);
    }
}

/* Function start: 0x44D888 */
short set_up_next_wave(void)
{
    MissionNavPoint *waveRecord;
    short prevWave;
    short obj;
    short entry;

    if (g_nCurrentWave_004931c0 == -1 ||
        g_nTrainSimActive_0049d758 != 0)
        return 0;

    waveRecord = &g_aMissionNavPoints_00491e98[
        g_nCurrentWave_004931c0 - 1 + g_nCurrentNavPoint_004931bc];
    prevWave = g_nCurrentWave_004931c0;
    g_nCurrentWave_004931c0++;
    if (waveRecord->type == prevWave) {
        if (g_bHighMemoryResourcesEnabled_005c80e4 != 0) {
            for (obj = 10; obj < SPACE_OBJECT_COUNT; obj++) {
                if (g_asObjectType_00495298[obj] == OBJECT_TYPE_TORPEDO)
                    remove_object(obj);
            }
        }
        ProcessMissionWaveCommands(waveRecord);
        new_sphere_shapes(waveRecord);
        waveRecord->type = -1;
        for (entry = 0; entry < 10; entry++) {
            place_ship_near_player_until_valid(
                init_ship(waveRecord->missionShips[entry],
                          g_nCurrentNavPoint_004931bc),
                5000, 10000);
        }
    } else {
        g_nCurrentWave_004931c0 = -1;
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
            leader = &g_aMissionShips_00492290[leader->leaderMissionIndex];
        }
        sub_int_vector(
            &g_aaFormationPositions_0049b8d0[leader->formationIndex]
                                            [record->formationSpot],
            &g_aaFormationPositions_0049b8d0[leader->formationIndex]
                                            [leader->formationSpot],
            &g_aShipFormationOffset_00495468[obj]);
        if (source == 0 &&
            g_bMissionShipInitializationInProgress_0049b890 == 0)
            return;
        copy_frame(source, obj);
        set_sphere_point(leader, &g_aShipPosition_00494550[obj]);
        offset_location(obj, &g_aShipFormationOffset_00495468[obj],
                        &g_aShipPosition_00494550[obj]);
        g_anShipSpeed_00494e20[obj] = leader->speed << 8;
    }
}

/* Function start: 0x44DB7E */
void Set_up_ship_info(short obj, short missionShip, signed char navPoint)
{
    MissionShipRecord *record;
    short gun;

    g_bKilrathiAceGreetingSent_00499bf4 = 0;
    g_asLoadedShipViewFrame_00495d18[obj] =
        g_acShipLastAttacker_004955c0[obj] =
            g_acShipPendingMessage_00495d98[obj] = -1;
    g_acShipDestroyedWeaponCount_00495680[obj] =
        g_acShipCommunicator_00495670[obj] =
            g_asShipFriendlyFireCooldown_00496090[obj] =
                g_asObjectDamage_00495178[obj] =
                    g_acShipIonDriveDamage_004956a0[obj] =
                        g_acShipDamage_00495690[obj] =
                            g_asActionCount_004955e8[obj] =
                                g_acShipExhaustHeat_00495660[obj] = 0;
    g_asShipMissionIndex_00495d00[obj] = missionShip;
    g_acShipNavPoint_004953e0[obj] = navPoint;
    record = &g_aMissionShips_00492290[missionShip];
    g_acShipPortrait_00495d88[obj] = record->portrait;

    set_sphere_point(record, &g_aShipPosition_00494550[obj]);
    alter_yaw((short)-record->pitch, obj);
    alter_pitch((short)-record->yaw, obj);
    alter_roll(record->roll, obj);
    g_asShipSide_004955d0[obj] = record->side;
    g_anShipSpeed_00494e20[obj] = record->speed << 8;
    g_asPilotLevel_00495d60[obj] = record->pilot;
    for (gun = 0; gun < 3; gun++)
        g_aasGunCooldown_005c8d70[obj][gun] = 0;
    g_acShipCollisionCooldown_00496010[obj] = 0;
    DAT_004960f0[obj] = 0;
    g_asWingmanDamageCredit_005d3830[obj] = 0;
    g_asPlayerDamageCredit_005d38c0[obj] = 0;
    DAT_005d1bd0[obj] = (short)(
        (unsigned short)RandomInRange(0, 200) + 600);
    if (g_asShipSide_004955d0[obj] == SIDE_KILRATHI)
        DAT_005d1bd0[obj] >>= 1;
    DAT_005d1bf0[obj] = (short)(
        (unsigned short)RandomInRange(0, 400) + 600);
    DAT_005d1c10[obj] = 0;
    recalc_max_velocity(obj);
    g_asCapitalHitDamageStep_005d17a0[obj] = 0;
    if (g_aObjectTypeData_00496d30[
            g_acObjectType_00493980[obj]].damageCapacity +
            g_aasShipArmor_00495540[obj][0] < 400) {
        g_asCapitalHitDamageStep_005d17a0[obj] =
            g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[obj]].damageCapacity;
    } else {
        g_asCapitalHitDamageStep_005d17a0[obj] = (short)(
            g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[obj]].damageCapacity / 2 + 1);
    }
    if (ShipHasCloakingDevice(obj) != 0) {
        g_anShipCloakState_00496020[obj] = 0;
        g_aShipWeapons_004956b0[obj][0]--;
    } else {
        g_anShipCloakState_00496020[obj] = 2;
    }
    g_asShipCloakCooldown_00496048[obj] = 0;
    reset_mission_type(obj, record->missionType);
    g_asShipMissionParameter_00495e00[obj] = record->missionParameter;
    g_asShipWingLeader_00495dd0[obj] =
        find_ship_index(record->leaderMissionIndex);
    set_formation_position(obj, record);
    zero_vector(&g_aShipVelocity_00494898[obj]);
    init_intelligence_data(obj);
    if (record->name[0] == '?')
        g_asShipIdentified_00496078[obj] = 0;
    else
        g_asShipIdentified_00496078[obj] = 1;
    g_apShipMissionRecord_00495da8[obj] = record;
    g_abPilotEjectionAttempted_00496110[obj] = 0;
    g_abShipEjectionSequenceEnabled_00496120[obj] = record->field_19;
    g_abShipArrivalMessageSent_005d1dd0[obj] = 0;
    if (g_asObjectType_00495298[obj] >= 0x39)
        g_asCarrierLaunchTimer_005d1090[obj] = 0;
}

/* Function start: 0x44E028 */
short is_team_member(short missionShip)
{
    short index;

    if (g_stMissionHeader_005d3e70.playerMissionShip == missionShip)
        return 1;
    for (index = 0; index < 8; index++) {
        if (g_stMissionHeader_005d3e70.initialMissionShips[index] == missionShip)
            return 1;
    }
    return 0;
}

/* Function start: 0x44E09C */
void find_next_ship_turn_slot(short obj)
{
    short other;

    g_acTurnInterval_00495628[obj] = (signed char)
        g_anPilotTurnInterval_0049b9c0[g_asPilotLevel_00495d60[obj]];
    g_acTurnRegulator_00495618[obj] = 1;
    for (other = 1; other <= 9; other++) {
        if (g_aeObjectClass_00495328[other] == OBJECT_CLASS_SHIP &&
            other != obj &&
            g_acTurnRegulator_00495618[obj] ==
                g_acTurnRegulator_00495618[other] &&
            g_acTurnInterval_00495628[other] ==
                g_acTurnInterval_00495628[obj]) {
            g_acTurnRegulator_00495618[obj]++;
            other = 1;
            if (g_acTurnInterval_00495628[obj] <
                g_acTurnRegulator_00495618[obj])
                return;
        }
    }
}

/* Function start: 0x44E187 */
short init_ship(short missionShip, short navPoint)
{
    MissionShipRecord *record;
    FixedVector center;
    short type;
    short objectType;
    short obj;

    if (missionShip == -1)
        return -1;
    record = &g_aMissionShips_00492290[missionShip];
    type = record->type;
    objectType = record->objectType;

    if (objectType == 0x3d || objectType == 0x2c) {
        obj = initialize_ship(objectType, -1, 0);
        g_asShipMissionIndex_00495d00[obj] = missionShip;
        AddFixedVectors(&g_aMissionNavPoints_00491e98[navPoint].position,
                        &record->position,
                        &g_aShipPosition_00494550[obj]);
        g_asShipSide_004955d0[obj] = 0;
        return obj;
    }
    if (objectType == 5 || objectType == 6) {
        AddFixedVectors(&g_aMissionNavPoints_00491e98[navPoint].position,
                        &record->position, &center);
        add_hazard_field(objectType, center,
                         (short)(record->speed + 3000),
                         record->pilot);
        return obj;
    }
    if (find_ship_index(missionShip) != -1 || record->state != 0)
        return -1;
    if (is_alive((signed char)record->pilot) == 0) {
        if (record->pilot == 5 && record->side == 0)
            return -1;
        record->pilot = 3;
    }
    record->navPoint = (signed char)navPoint;
    if (is_team_member(missionShip) != 0)
        navPoint = -1;
    if (record->field_13 != 0)
        LoadAlternateShipType(record->type, record->field_13);
    obj = initialize_ship(type, -1, 1);
    if (obj == -1)
        return obj;
    if (g_nYourWingman_0049346c == -1 && record->pilot > 4 &&
        record->side == 0)
        g_nYourWingman_0049346c = obj;
    Set_up_ship_info(obj, missionShip, (signed char)navPoint);
    if (record->field_13 != 0)
        ApplyAlternateShipType(obj);
    find_next_ship_turn_slot(obj);
    check_futurion(obj);
    return obj;
}

#pragma function(strcat, strcpy)
/* Function start: 0x44E44F */
void init_intelligence_data(short obj)
{
    short usedIntelSlots[5];
    char intelFilename[16];
    short foundMatchingIntel;
    ShipIntelligenceMetadata *metadata;
    short shipIndex;
    short shipIntelFile;
    short missionShipIndex;
    char intelNumber[8];
    short intelIndex;
    short missionSystem;

    metadata = AllocateTaggedMemory(0x0c, 0x40);
    g_abShipTurn_00495fd8[obj] = 0;
    clear_alert(obj);
    g_aeSpecialManeuver_00495600[obj] = SPECIAL_MANEUVER_NONE;
    g_asShipIntelState_0049b678[obj] = 0;
    missionSystem = g_asShipMissionParameter_00495e00[obj];
    g_aShipMissionSpot_00495e18[obj] = g_aMissionNavPoints_00491e98[
        g_nCurrentNavPoint_004931bc].position;

    switch (g_asShipMissionType_00495de8[obj]) {
    case MISSION_TYPE_ESCORT:
    case MISSION_TYPE_DEFEND:
    case MISSION_TYPE_WINGMAN:
        g_asShipMissionParameter_00495e00[obj] = missionSystem;
        break;
    case MISSION_TYPE_STRIKE:
        g_asShipMissionParameter_00495e00[obj] = missionSystem;
        for (shipIndex = 0; shipIndex < 10; shipIndex++) {
            for (missionShipIndex = 0; missionShipIndex < 10;
                 missionShipIndex++) {
                if (g_aMissionNavPoints_00491e98[shipIndex]
                        .missionShips[missionShipIndex] ==
                    g_asShipMissionParameter_00495e00[obj]) {
                    missionSystem = shipIndex;
                }
            }
        }
        g_aShipMissionSpot_00495e18[obj] =
            g_aMissionNavPoints_00491e98[missionSystem].position;
        break;
    case MISSION_TYPE_GOTO_WARP:
        g_aShipMissionSpot_00495e18[obj] =
            g_aMissionNavPoints_00491e98[missionSystem].position;
        break;
    case MISSION_TYPE_WARP_ARRIVE:
        g_asShipTactic_00495f30[obj] = TACTIC_WARP_IN;
        g_asShipManeuver_00495f48[obj] = MANEUVER_WARPING_IN;
        if (g_asShipSide_004955d0[obj] == SIDE_KILRATHI)
            break;
        /* fall through */
    case 9: /* WC2's home-base mission value in this dispatch table. */
        locate_ship(g_stMissionHeader_005d3e70.homeMissionShip,
                    &g_aShipMissionSpot_00495e18[obj]);
        break;
    default:
        break;
    }

    g_acShipStress_00496100[obj] = 0;
    g_asShipIntelSlot_00495d30[obj] = -1;
    if (obj != 0 && g_aeObjectClass_00495328[obj] == OBJECT_CLASS_SHIP &&
        (obj == g_nYourWingman_0049346c ||
         g_asShipSide_004955d0[obj] != SIDE_IMPERIAL)) {
        shipIntelFile = 0;
        if (g_asPilotLevel_00495d60[obj] == RATING_ACE_SPIRIT) {
            intelIndex = g_acShipPortrait_00495d88[obj];
        } else if (g_asPilotLevel_00495d60[obj] < RATING_ACE_SPIRIT) {
            intelIndex = g_aObjectTypeData_00496d30[
                g_acObjectType_00493980[obj]].resourceType;
            shipIntelFile = 1;
        } else {
            ReportFatalErrorCode("024");
        }

        for (shipIndex = 0; shipIndex < 5; shipIndex++)
            usedIntelSlots[shipIndex] = 0;

        foundMatchingIntel = 0;
        for (shipIndex = 0; shipIndex < 10; shipIndex++) {
            if (g_asShipIntelSlot_00495d30[shipIndex] != -1) {
                usedIntelSlots[
                    g_asShipIntelSlot_00495d30[shipIndex]] = 1;
            }
            if (g_asShipIntelResourceKey_00495d48[shipIndex] ==
                shipIntelFile * 100 + intelIndex) {
                foundMatchingIntel = 1;
                g_asShipIntelSlot_00495d30[obj] =
                    g_asShipIntelSlot_00495d30[shipIndex];
                g_asShipIntelResourceKey_00495d48[obj] =
                    (short)(shipIntelFile * 100 + intelIndex);
                break;
            }
        }
        if (foundMatchingIntel == 0) {
            for (shipIndex = 0; shipIndex < 5; shipIndex++) {
                if (usedIntelSlots[shipIndex] == 0)
                    break;
            }
            g_asShipIntelSlot_00495d30[obj] = shipIndex;
            g_asShipIntelResourceKey_00495d48[obj] =
                (short)(shipIntelFile * 100 + intelIndex);

            strcpy(intelFilename, "intel.0");
            if (shipIntelFile != 0)
                strcpy(intelFilename, "intelshp.0");
            if (intelIndex < 10)
                strcat(intelFilename, "0");
            _itoa((int)intelIndex, intelNumber, 10);
            strcat(intelFilename, intelNumber);

            LoadPacketIntoBuffer(
                intelFilename, 0,
                g_aShipIntelligenceData_005d3060[
                    g_asShipIntelSlot_00495d30[obj]],
                0);
            if (obj == g_nYourWingman_0049346c) {
#ifdef SDL_PORT
                /* Section 1 is a twelve-byte ShipIntelligenceMetadata, and
                 * the original reads it straight over the run of wingman
                 * behaviour shorts starting at 0x496138 -- the record's first
                 * eight bytes are those four modes.  They are separate
                 * objects here, so read the record and place them by hand.
                 * The music and comm fields that follow are only ever read
                 * from the metadata copy in the branch below, so nothing
                 * wants the last four bytes on this path. */
                {
                    ShipIntelligenceMetadata wingmanIntel;

                    LoadPacketIntoBuffer(intelFilename, 1,
                                         &wingmanIntel, 0);
                    memcpy(&g_nWingmanRoutDecisionMode_00496138,
                           wingmanIntel.field_0, sizeof(short));
                    memcpy(&g_nWingmanFormationDisobeyMode_0049613a,
                           wingmanIntel.field_0 + 2, sizeof(short));
                    memcpy(&g_nWingmanTargetingMode_0049613c,
                           wingmanIntel.field_0 + 4, sizeof(short));
                    memcpy(&g_nWingmanEngagementMode_0049613e,
                           wingmanIntel.field_0 + 6, sizeof(short));
                }
#else
                LoadPacketIntoBuffer(
                    intelFilename, 1,
                    &g_nWingmanRoutDecisionMode_00496138, 0);
#endif
                LoadCommPortraitResources(g_acShipPortrait_00495d88[obj]);
                g_nLoadedCommPortraitPilot_004931c4 = -1;
                if (g_pCommPortraitResource_0049b788 != 0) {
                    g_nLoadedCommPortraitPilot_004931c4 =
                        g_acShipPortrait_00495d88[obj];
                }
            } else if (shipIntelFile == 0) {
                LoadPacketIntoBuffer(intelFilename, 1, metadata, 1);
                if (metadata->musicTrack != 0) {
                    g_nMissionMusicTrackOverride_00496144 =
                        metadata->musicTrack;
                }
                if (metadata->enemyCommCommandBase != 0) {
                    g_nEnemyCommCommandBase_005d179c =
                        metadata->enemyCommCommandBase;
                    g_nEnemyCommPilotIndex_005d179e =
                        g_acShipPortrait_00495d88[obj];
                }
            }
        }
    }
}
