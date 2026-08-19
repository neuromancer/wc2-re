/*
 *  Ship mission logic (Mac `logic` compilation unit), with adjacent Win32
 *  ship and game-mode initialization routines.
 *
 *  Address range 0x421000-0x424fff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: exact nested Mac CODE 5 `logic` unit at
 *  0x422010-0x423cdf; LoadOriginFxDrivers/EMStartUp and string band
 *  0x469A28-0x469B9C anchor the provisional enclosing file.
 */
#include "wc1.h"

#pragma function(strcat, strcpy)

static char g_szTemporaryCampaignGlobalsFile_00499bc0[] = "TEMPGLOB.000";
static const ShortVector g_aTargetCameraEyeOffsets_0049d430[3] = {
    {0, 0, -80},
    {-50, 0, 0},
    {50, 0, 0}
};
static const ShortVector g_aaTurretProjectileOffsets_0049d448[2][2] = {
    {{-25, 0, -80}, {25, 0, -80}},
    {{-50, 0, 25}, {-50, 0, -25}}
};

/* Function start: 0x416690 */
short find_weapon(short obj, enum ObjectType weaponType)
{
    short weapon;

    for (weapon = 0;
         weapon < (signed char)g_aShipWeapons_004956b0[obj][0];
         weapon++) {
        ShipWeaponSlot *weaponSlot =
            &((ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1])[weapon];

        if (weaponSlot->type == weaponType)
            return weapon;
    }
    return -1;
}

/* Function start: 0x41670E */
short fire_missile(short ship)
{
    short weapon;
    ShipWeaponSlot *slot;

    slot = (ShipWeaponSlot *)&g_aShipWeapons_004956b0[ship][1];
    for (weapon = 0;
         weapon < (signed char)g_aShipWeapons_004956b0[ship][0];
         weapon++, slot++) {
        if (g_aObjectTypeData_00496d30[slot->weaponType].objectClass ==
                OBJECT_CLASS_MISSILE) {
            if (ship != 0) {
                if (slot->type != 0x13 ||
                    IsCapitalShipObject(g_acShipTarget_00495f20[ship]) != 0)
                    return fire_weapon(ship, weapon);
            } else if (slot->disabled == 0) {
                if ((slot->type == 0x10 || slot->type == 0x12 ||
                     slot->type == 0x13) &&
                    g_nTargetLockCountdown_004934ec != 0) {
                    if (get_mode(0) == 1)
                        ShowComponentHitHudMessage(
                            g_szNeedLock_00492e4c,
                            g_abGamePaletteReservedColours_0049cb54[4], 3);
                    return -1;
                }
                return fire_weapon(ship, weapon);
            }
        }
    }
    return -1;
}

/* Function start: 0x41687F */
void fire_fixed_projectile_weapon(short obj)
{
    ShipWeaponSlot *slot;
    short weapon;

    slot = (ShipWeaponSlot *)&g_aShipWeapons_004956b0[obj][1];
    for (weapon = 0;
         weapon < (signed char)g_aShipWeapons_004956b0[obj][0];
         weapon++, slot++) {
        if (g_aObjectTypeData_00496d30[slot->weaponType].objectClass ==
                OBJECT_CLASS_PROJECTILE &&
            slot->type != 0x0b && slot->disabled == 0) {
            if (fire_weapon(obj, weapon) == -1)
                break;
        }
    }
}

/* Function start: 0x41693A */
void fire_afterburner(short obj, short time)
{
    if (g_asObjectType_00495298[obj] == 0x33)
        return;
    if ((long)get_ship_max_velocity(obj) * 0x500L -
            Vector_magnitude(&g_aShipVelocity_00494898[obj]) > 0) {
        set_special(obj, SPECIAL_MANEUVER_AFTERBURNER);
        if (g_aeSpecialManeuver_00495600[obj] !=
            SPECIAL_MANEUVER_AFTERBURNER)
            time = 0;
        g_asShipAfterburnerTimer_004955a8[obj] = time;
    }
}

/* Function start: 0x4169CC */
void fire_super_brake(short ship)
{
    g_asShipAfterburnerTimer_004955a8[ship] = 10;
    set_special(ship, SPECIAL_MANEUVER_SUPER_BRAKE);
}

/* Function start: 0x4169F3 */
short flip_angle(short ship, short angle)
{
    if ((g_asObjectFlip_004939c8[ship] & 0x10) != 0)
        angle = (short)(180 - angle);
    if ((g_asObjectFlip_004939c8[ship] & 0x20) != 0)
        angle = (short)-angle;
    angle += g_asObjectScreenAngle_004936b8[ship];
    angle %= 360;
    if (angle < 0)
        angle += 360;
    return angle;
}

/* Function start: 0x416A87 */
void place_exhaust_on_ships(void)
{
    short *animation;
    short frame;
    short scale;
    short object;
    short ship;
    short viewFrame;
    short animationOffset;

    for (ship = 0; ship < 10; ship++) {
        if (g_aeObjectClass_00495328[ship] < OBJECT_CLASS_MISSILE ||
            g_anShipSpeed_00494e20[ship] == 0 ||
            g_asObjectType_00495298[ship] == 0x14 ||
            g_aeSpecialManeuver_00495600[ship] ==
                SPECIAL_MANEUVER_UNKNOWN_9 ||
            g_aeSpecialManeuver_00495600[ship] ==
                SPECIAL_MANEUVER_KILL_ENGINES ||
            g_anShipCloakState_00496020[ship] == 1)
            continue;
        if (ship == 0 && g_bJumpSequenceActive_004962f0 != 0)
            continue;
        if (g_asObjectScreenX_00493598[ship] == (short)0x8001)
            continue;

        animation = (short *)g_apObjectExhaustShape_004953b8[ship];
        if (animation == 0)
            continue;
        viewFrame = g_asObjectViewFrame_00493508[ship];
        if (g_aeObjectClass_00495328[ship] < OBJECT_CLASS_CAPITAL_SHIP &&
            g_asObjectType_00495298[ship] != 0x33 &&
            g_bExpandedShipGraphicsEnabled_004931a4 != 0 &&
            g_aeObjectClass_00495328[ship] == OBJECT_CLASS_SHIP)
            viewFrame = g_asLoadedShipViewFrame_00495d18[ship];
        animationOffset = animation[viewFrame];
        if (animationOffset == -1)
            continue;
        animation =
            (short *)((unsigned char *)animation + animationOffset);
        if (*animation == -1)
            continue;

        while (*animation != -1) {
            object = find_vacant_3d_object();
            if (object == -1)
                return;
            set_objects_data(object, 0x2b, ship, 0);
            frame = *animation++;
            scale = *animation++;
            if (g_aeObjectClass_00495328[ship] >=
                    OBJECT_CLASS_CAPITAL_SHIP)
                scale = (short)(scale +
                    (unsigned short)RandomInRange(0x10, 0x30));
            else
                scale = (short)(scale +
                    (unsigned short)RandomInRange(0, 0x20));
            if (g_acShipExhaustHeat_00495660[ship] == 0)
                scale = (short)(scale - 0x20);
            g_asObjectScale_00494d90[object] = scale;
            g_asObjectDistance_00493ae8[object] = *animation++;
            g_asObjectScreenAngle_004936b8[object] =
                flip_angle(ship, *animation++);
            g_asObjectFlip_004939c8[object] = 0;
            g_asObjectScreenX_00493598[object] = *animation++;
            g_asObjectScreenY_00493628[object] = *animation++;
            if (g_aeSpecialManeuver_00495600[ship] ==
                    SPECIAL_MANEUVER_AFTERBURNER) {
                g_asObjectViewFrame_00493508[object] =
                    (short)(frame * 3 +
                            (unsigned short)RandomInRange(0, 2));
            } else {
                g_asObjectViewFrame_00493508[object] =
                    (short)((unsigned short)RandomInRange(0, 1) +
                            frame * 2 + 12);
            }
        }
    }
}

/* Function start: 0x416DC3 */
void reposition_fixed_child_objects(void)
{
    int right;
    int up;
    long sine;
    long cosine;
    short angle;
    short parent;
    short object;
#ifdef WC1_SDL
    int fixedCosine;
    int fixedSine;
    float attachmentRight;
    float attachmentUp;
    float cosineFloat;
    float parentScreenX;
    float parentScreenY;
    float sineFloat;
#endif

    for (object = 10; object <= WC2_SPACE_LAST_MOVING_OBJECT; object++) {
        if (g_aeObjectClass_00495328[object] ==
            OBJECT_CLASS_FIXED_OBJECT) {
            parent = (short)g_acObjectOwner_00495208[object];
            switch (g_asObjectType_00495298[object]) {
            case 0x3e:
                g_asObjectDistance_00493ae8[object] =
                    (short)((unsigned short)
                                g_asObjectDistance_00493ae8[parent] -
                            100);
                g_asObjectScreenX_00493598[object] =
                    g_asObjectScreenX_00493598[parent];
                g_asObjectScreenY_00493628[object] =
                    g_asObjectScreenY_00493628[parent];
                break;
            case 0x3f:
                g_asObjectDistance_00493ae8[object] =
                    (short)((unsigned short)
                                g_asObjectDistance_00493ae8[parent] +
                            100);
                g_asObjectScreenX_00493598[object] =
                    g_asObjectScreenX_00493598[parent];
                g_asObjectScreenY_00493628[object] =
                    g_asObjectScreenY_00493628[parent];
                break;
            case 0x0c:
            case 0x2b:
                angle = g_asObjectScreenAngle_004936b8[parent];
                sine = SinFixed(angle);
                cosine = CosFixed(angle);
                g_asObjectDistance_00493ae8[object] =
                    (short)((unsigned short)
                                g_asObjectDistance_00493ae8[parent] +
                            (unsigned short)
                                g_asObjectDistance_00493ae8[object]);
                right = g_asObjectScreenX_00493598[object];
                right *= (unsigned short)
                    g_asObjectScreenScale_00493a58[parent];
                if ((g_asObjectFlip_004939c8[parent] & 0x10) != 0)
                    right = -right;
                up = g_asObjectScreenY_00493628[object];
                up *= (unsigned short)
                    g_asObjectScreenScale_00493a58[parent];
                if ((g_asObjectFlip_004939c8[parent] & 0x20) != 0)
                    up = -up;
                g_asObjectScreenX_00493598[object] = (short)(
                    (MultiplyFixed(right, (int)cosine) -
                     MultiplyFixed(up, (int)sine)) >> 8);
                g_asObjectScreenY_00493628[object] = (short)(
                    (MultiplyFixed(up, (int)cosine) +
                     MultiplyFixed(right, (int)sine)) >> 8);
                g_asObjectScreenX_00493598[object] +=
                    g_asObjectScreenX_00493598[parent];
                g_asObjectScreenY_00493628[object] +=
                    g_asObjectScreenY_00493628[parent];
#ifdef WC1_SDL
                if (g_asObjectType_00495298[object] == 0x2b) {
                    /* Match the anchor to the enhanced parent transform. */
                    parentScreenX =
                        (float)g_nViewCenterX_005c80d8 +
                        (float)(((double)(g_nScreenWidth_0049d4d8 & ~1) *
                                 0.5 *
                                 g_aObjectViewPosition_0059afa0[parent].x) /
                                g_aObjectViewPosition_0059afa0[parent].z);
                    parentScreenY =
                        (float)g_nViewCenterY_005c80da +
                        (float)(((double)(g_nScreenWidth_0049d4d8 & ~1) *
                                 0.5 *
                                 g_aObjectViewPosition_0059afa0[parent].y) /
                                g_aObjectViewPosition_0059afa0[parent].z);
                    GetRLETransformTrig((int)angle * 10,
                                        &fixedCosine, &fixedSine);
                    cosineFloat = (float)fixedCosine / 65536.0f;
                    sineFloat = (float)fixedSine / 65536.0f;
                    attachmentRight = (float)right / 256.0f;
                    attachmentUp = (float)up / 256.0f;
                    Wc1SdlSetThrusterScreenPosition(
                        object,
                        parentScreenX + attachmentRight * cosineFloat -
                            attachmentUp * sineFloat,
                        parentScreenY + attachmentRight * sineFloat +
                            attachmentUp * cosineFloat);
                }
#endif
                break;
            }
            g_asObjectScreenScale_00493a58[object] = (short)(
                (unsigned short)g_asObjectScale_00494d90[object] *
                (unsigned short)g_asObjectScreenScale_00493a58[parent] >>
                8);
        }
    }
}

/* Function start: 0x4170F9 */
void housekeep_power_plant_and_fuel(short ship)
{
    if (0 < g_anShipSpeed_00494e20[ship])
        drain_fuel(ship, 5);
}

/* Function start: 0x417124 */
void replenish_shields(short ship)
{
    signed char shield;

    if (ship == 0 && g_acPlayerComponentDamage_00493470[1] > 0 &&
        g_nSpaceFrame_00493134 %
            (g_acPlayerComponentDamage_00493470[1] + 1) != 0)
        return;
    for (shield = 0; shield <= 1; shield++) {
        if (g_aasShipMaximumShield_004954f0[ship][shield] <
            g_aasShipShield_00495518[ship][shield])
            g_aasShipShield_00495518[ship][shield] =
                g_aasShipMaximumShield_004954f0[ship][shield];
        if (g_aasShipMaximumShield_004954f0[ship][shield] >
                g_aasShipShield_00495518[ship][shield] &&
            g_nSpaceFrame_00493134 %
                g_aObjectTypeData_00496d30[
                    g_acObjectType_00493980[ship]].animationDelay == 0) {
            g_aasShipShield_00495518[ship][shield]++;
        }
    }
}

/* Function start: 0x41724A */
void replenish_weapon_energy_bank(short ship)
{
    short shieldEnergy;
    short maximumShield;

    if (ship == 0 && g_acPlayerComponentDamage_00493470[1] != 0 &&
        (int)(unsigned short)RandomInRange(0, 4) <
            g_acPlayerComponentDamage_00493470[1])
        return;
    if (ship == 0 && HasShipCockpitGunDisplay(0) != 0) {
        if (g_nTargetCameraMode_005c8d50 == 0) {
            g_nGunDisplayEnergyPercent_005c8d4e = MinShort(
                (short)(g_nGunDisplayEnergyPercent_005c8d4e + 2), 100);
        } else if (g_nTargetCameraZoom_0049d3e4 < 0x21 &&
                   g_nSpaceFrame_00493134 % 2 == 0) {
            g_nTargetCameraZoom_0049d3e4 = MinShort(
                (short)(g_nTargetCameraZoom_0049d3e4 + 1), 0x21);
        }
    }
    if (g_asShipWeaponEnergy_00495590[ship] < 100) {
        shieldEnergy = (short)(g_aasShipShield_00495518[ship][1] +
                               g_aasShipShield_00495518[ship][0]);
        maximumShield =
            (short)(g_aasShipMaximumShield_004954f0[ship][1] +
                    g_aasShipMaximumShield_004954f0[ship][0]);
        if (maximumShield > shieldEnergy) {
            g_asShipWeaponEnergy_00495590[ship] =
                MinShort(
                    (short)(g_asShipWeaponEnergy_00495590[ship] + 1),
                    100);
        } else {
            g_asShipWeaponEnergy_00495590[ship] =
                MinShort(
                    (short)(g_asShipWeaponEnergy_00495590[ship] + 2),
                    100);
        }
    }
    if (g_asShipWeaponEnergy_00495590[ship] < -5) {
        g_asShipWeaponEnergy_00495590[ship] = MinShort(
            (short)(g_asShipWeaponEnergy_00495590[ship] + 1), 100);
    }
}

/* Function start: 0x41740F */
void accelerate(short amount)
{
    if (malf(0) != 0) {
        amount = (short)(amount - 2);
        if (g_nSpaceFrame_00493134 % 3 == 0)
            PlaySfxWaveFileByNumber(3, -1, 0);
    }
    celerate(0, (int)amount << 8);
}

/* Function start: 0x417471 */
void your_afterburner(void)
{
    short time;

    if (g_anShipFuel_00495638[0] > 0) {
        if (malf(0) != 0) {
            PlaySfxWaveFileByNumber(3, -1, 0);
        } else {
            if (g_aeSpecialManeuver_00495600[0] !=
                    SPECIAL_MANEUVER_AFTERBURNER) {
                time = 8;
            } else if (g_asShipAfterburnerTimer_004955a8[0] == 0) {
                time = 8;
            } else {
                if (g_asShipAfterburnerTimer_004955a8[0] > 2)
                    return;
                time = 2;
            }
            fire_afterburner(0, time);
            if (g_nSpaceFrame_00493134 >
                g_nNextAfterburnerSfxFrame_005d3820) {
                g_nNextAfterburnerSfxFrame_005d3820 =
                    (int)g_nSpaceFrame_00493134 + 6;
                PlaySfxWaveFileByNumber(12, -1, 0);
            }
        }
    }
}

/* Function start: 0x45B7E0 */
short QueryCurrentGraphicsMode(void)
{
    if ((short)g_nSpacePaletteFadeMode_004901e8 != -1)
        return (short)g_nSpacePaletteFadeMode_004901e8;
    return 0x13;
}

/* Function start: WC2_UNMAPPED */
unsigned int LoadWc1GamePaletteFile(void)
{
    short index;

    PromptInsertNumberedDisk(0);
    switch ((int)(short)g_nSpacePaletteFadeMode_004901e8) {
    case 9:
    case 13:
        index = 0;
        do {
            g_abGamePaletteReservedColours_0049cb54[index] =
                g_abLegacyVideoModeColours_0049cb90[index];
            index++;
        } while ((unsigned int)(int)index < 14);
        index = 0;
        do {
            g_asConversationTextColours_004699f0_WC1_UNMAPPED[index] =
                g_asConversationTextColours_004699f0_WC1_UNMAPPED[index + 12];
            index++;
        } while ((unsigned int)(int)index < 12);
        return 0;
    case 0x13:
        LoadWc1PaletteTripletsFile("game.pal");
        ResetCockpitPaletteEntries();
        SaveGamePalette();
        return 0;
    }
    return 0;
}

/* Function start: 0x45B810 */
void LoadGamePaletteFile(void)
{
    short index;
    unsigned char *reservedColours;
    unsigned char *palette;
    int graphicsMode;

    reservedColours = g_abGamePaletteReservedColours_0049cb54;
    graphicsMode = (int)(short)g_nSpacePaletteFadeMode_004901e8;
    switch (graphicsMode) {
    case 9:
    case 13:
        for (index = 0; (unsigned short)index < 14; index++)
            reservedColours[index] =
                g_abLegacyVideoModeColours_0049cb90[index];
        break;
    case 0x13:
        palette = FetchDiskPacketRetrying("brief.pal", 0, 0);
        for (index = 0; index < 0x100; index++)
            SetPaletteEntryFromTriplet(palette + index * 3, index);
        ResetCockpitPaletteEntries();
        SaveGamePalette();
        break;
    }
}

/* Function start: 0x45B905 */
void EMShutDown(void)
{
    if (g_bInputManagerInitialized_005c80ca != 0)
        ShutdownEventManager();
}

/* Function start: WC2_UNMAPPED */
unsigned short InitializeEventManagerResources(void)
{
    DAT_0059a9f0 = 20;
    g_nInputTickScale_005c8d24 = 20;
    DAT_0059ab64 = 1;
    g_stMouseCursorState_0059ab10.shape = g_pMouseCursorResource_005a7cdc =
        FetchDiskPacketRetrying(14, 0, 0x10);
    g_stMouseCursorState_0059ab10.frame = 0;
    g_stMouseCursorState_0059ab10.viewport = &g_stScreenViewport_005d21a0;
    return 0;
}

/* Function start: WC2_UNMAPPED */
unsigned int StartWc1EventManager(void)
{
    PromptInsertNumberedDisk(0);
    RegisterEventManagerShutdown((void (*)(void))LogMemoryUsage);
    if (InitializeEventManager(20, InitializeEventManagerResources, 0) == 0)
        exit_squadron("EMStartUp Failed");
    ConfigureEventManagerPointer(g_stMouseCursorState_0059ab10.shape, 0);
    SetEventManagerPump(PollJoystickButtonEvents);
    g_nMenuInputRepeatDelay_005a8208 = 6;
    return 0;
}

/* Function start: 0x45B924 */
void EMStartUp(void)
{
    if (InitializeInputManagerMemory() == 0)
        ReportFatalErrorCode("020");
    g_pfnEventManagerShutdown_005c8be0 = (void (*)(void))LogMemoryUsage;
    InitializeInputDriverHook();
    if (g_bRewritePacketExtensions_0049cb48 == 0) {
        LoadInputCursorShape(g_szInputCursorPacketName_0049cba0,
                             0, 0x13);
    } else {
        RewritePacketFilenameForInstalledData(
            g_szInputCursorPacketName_0049cba0);
        LoadInputCursorShape(g_szInputCursorPacketName_0049cba0,
                             0, 0x0d);
    }
    FinalizeInputDriverHook();
    SetInputViewport(&g_stScreenViewport_005d21a0);
    g_nInputTickInterval_005c8448 = 20;
    g_nInputRepeatDelay_005c80d6 = 20;
    g_bInputCursorEnabled_005c80e6 = 1;
    g_nInputDeviceMode_005c83e6 = 3;
}

/* Function start: WC2_UNMAPPED */
unsigned int LoadWc1OriginFxDrivers(void)
{
    int memoryThreshold;
    int videoModeMemory;
    short requestedGraphicsMode;

    memoryThreshold = 100000;
    _chdir("gamedat");
    g_nNearHeapMaxDescriptors_00493048 = 0x80;
    IsSoundHardwarePresent(8);
    if (DAT_0059a856 == 0)
        SystemDebugPrintf("No ");
    SystemDebugPrintf("Expanded Memory Detected.\n");
    g_bOriginFxDriverActive_0049cbb0 = 0;
    PromptInsertNumberedDisk(0x38);
    if (GetMusicDriverPresent(g_bSlowSceneAnimation_00469998_WC1_UNMAPPED) == 0)
        exit_squadron("Failed to load Origin-FX drivers");
    LoadJoystickCalibrationFile(9, 9, 1, 1);
    g_nInputDoubleClickInterval_00493050 = 2;
    StartWc1EventManager();
    GetFxDriverInitResult();
    g_dwOriginalFreeMemory_005a7cd8 = GetLargestFreeMemoryBlockByType(0);
    if (g_nMusicDriverMode_0049be8c != 0 &&
        g_nMusicDriverMode_0049be8c != 3)
        memoryThreshold = 210000;
    SetFrameTimerPeriodDirect(0x78);
    if ((int)GetAvailableFarMemoryByType(4) > memoryThreshold) {
        g_nAvailableGameMemory_005c8de0 =
            (int)g_dwOriginalFreeMemory_005a7cd8 -
            g_anExpandedMemoryReservationByVideoMode_00469ab0_WC1_UNMAPPED[
                g_bSlowSceneAnimation_00469998_WC1_UNMAPPED];
        if (g_nAvailableGameMemory_005c8de0 < 0)
            exit_squadron(
                "You do not have enough memory to play Wing Commander.\n"
                "Refer to your reference guide for assistance.");
        g_nMemoryConfiguration_005c8dc8 = 2;
        SystemDebugPrintf("Expanded Memory fully used.\n");
    } else {
        g_nAvailableGameMemory_005c8de0 =
            (int)g_dwOriginalFreeMemory_005a7cd8 -
            g_anBaseMemoryReservationByVideoMode_00469a90_WC1_UNMAPPED[
                g_bSlowSceneAnimation_00469998_WC1_UNMAPPED];
        if (g_nAvailableGameMemory_005c8de0 < 0)
            exit_squadron(
                "You do not have enough memory to play Wing Commander.\n"
                "Refer to your reference guide for assistance.");
        g_nMemoryConfiguration_005c8dc8 = 0;
        if (g_nMusicDriverMode_0049be8c == 1 ||
            g_nMusicDriverMode_0049be8c == 2) {
            videoModeMemory =
                g_anFullMusicMemoryReservationByVideoMode_00469aa0_WC1_UNMAPPED[
                    g_bSlowSceneAnimation_00469998_WC1_UNMAPPED];
            if ((int)g_dwOriginalFreeMemory_005a7cd8 > videoModeMemory) {
                g_nMemoryConfiguration_005c8dc8 = 1;
                g_nAvailableGameMemory_005c8de0 =
                    (int)g_dwOriginalFreeMemory_005a7cd8 - videoModeMemory;
                SystemDebugPrintf("Full");
            } else {
                SystemDebugPrintf("Limited");
            }
            SystemDebugPrintf(" music will play.");
        }
    }

    LoadSpaceflightResources();
    FxDriverShutdownHook();
    SetEventManagerPump(PollJoystickButtonEvents);
    PromptInsertNumberedDisk(0);
    ShutdownVideoHook(
        g_acExpectedGraphicsModes_00493078[
            g_bSlowSceneAnimation_00469998_WC1_UNMAPPED]);
    requestedGraphicsMode =
        g_acExpectedGraphicsModes_00493078[
            g_bSlowSceneAnimation_00469998_WC1_UNMAPPED];
    if (GetTargetColourIndex() != requestedGraphicsMode)
        exit_squadron("Requested graphics display mode not available.");
    LoadWc1GamePaletteFile();
    InitializeGameTextContexts();
    InitializeDiskPromptTextContext();
    GetEventManagerStatus();
    g_nFrameSkip_0049d764 = 1;
    if (g_nMusicDriverMode_0049be8c != 0 &&
        g_nMusicDriverMode_0049be8c != 3)
        GetFxDriverStatus();
    InitializeWc1DirectionViewFrames();
    g_pConstellationDefinitions_00598a28 =
        LoadPacketAllocated(0x3a, 0);
    g_pMissionCampaignData_005988bc = LoadPacketAllocated(0x3a, 1);
    SystemDebugPrintf("\n[SYSTEM] : Exiting initialize()\n");
    return 0;
}

/* Function start: 0x45B9D3 */
void LoadOriginFxDrivers(void)
{
    int memoryThreshold;
    char campaignNumber[4];
    FILE *memoryLogFile;
    char startupPacketName[13] = "stars.00";
    int availableMemory;
    int expandedShipMemoryBytes;
    short postPaletteStatus;

    memoryThreshold = 100000;
    g_nDetectedGraphicsMode_005c80d2 = QueryCurrentGraphicsMode();
    g_nGraphicsInitializationFlags_00493048 = 0x20c;
    IsSoundHardwarePresent(8);
    if (g_bPumpMessagesWhileLoading_005c8ddc != 0)
        MessagePumpHook(4);
    if (g_bHighMemoryResourcesEnabled_005c80e4 == 0)
        printf("No ");
    printf("Expanded Memory Detected.\n");
    g_bOriginFxDriverActive_0049cbb0 = 0;
    if (LoadGraphicsDriver(g_bRewritePacketExtensions_0049cb48) == 0)
        exit_squadron("Failed to load Origin-FX drivers");
    InitializeMouseCursorDepth(0);
    g_nInputDoubleClickInterval_00493050 = 2;
    EMStartUp();
    g_dwInitialFreeMemory_005c8dd0 =
        GetLargestFreeMemoryBlockByType(0);
    availableMemory = (int)GetAvailableFarMemoryByType(4);
    g_nAvailableGameMemory_005c8de0 = 0xc63e0;
    availableMemory = 0x7c0600;
    if (g_bSpeechResourcesEnabled_0049cc2c != 0 &&
        g_bHighMemoryResourcesEnabled_005c80e4 != 0) {
        g_bSpeechCacheEnabled_005c8de8 = 0;
        if (g_nMinimumMemoryNoMusic_0049cc20 + 0x1000 <
            (int)g_dwInitialFreeMemory_005c8dd0) {
            if (InitializeSpeechCache(
                    g_nSpeechCacheUnitCount_005c8dde,
                    g_nSpeechCacheSizeCode_005c8dd8) == 0) {
                g_bSpeechCacheEnabled_005c8de8 = 1;
                printf("Speech Enabled.\n");
            }
        }
    }
    if (g_bHighMemoryResourcesEnabled_005c80e4 != 0)
        InitializeHighMemoryGraphicsBuffers();
    memoryLogFile = fopen("mem.$$$", "w+");
    LogMemoryStateToFile(memoryLogFile);
    if (memoryLogFile != 0)
        fclose(memoryLogFile);
    if (g_bHighMemoryResourcesEnabled_005c80e4 == 0) {
        g_bExpandedShipGraphicsEnabled_004931a4 = 0;
        g_nMinimumMemoryNoMusic_0049cc20 = 0x42680;
    }
    if (g_bRewritePacketExtensions_0049cb48 == 1) {
        g_nMinimumMemoryNoMusic_0049cc20 -= 20000;
        g_nMinimumMemoryFullMusic_0049cc24 -= 20000;
        g_nMinimumMemoryAlternate_0049cc28 -= 20000;
    }
    expandedShipMemoryBytes =
        (int)g_nExpandedShipMemoryKb_0049cbac << 10;
    if (g_nMusicDriverMode_0049be8c != 0 &&
        g_nMusicDriverMode_0049be8c != 3) {
        if (g_nMusicDriverMode_0049be8c == 1)
            g_nMusicMemoryRequirement_005c8dcc = 0x1b800;
        else if (g_nMusicDriverMode_0049be8c == 2)
            g_nMusicMemoryRequirement_005c8dcc = 0x13800;
        if (g_bHighMemoryResourcesEnabled_005c80e4 != 0)
            memoryThreshold += g_nMusicMemoryRequirement_005c8dcc;
    }
    SetFrameTimerPeriodDirect(0x78);
    g_nAvailableGameMemory_005c8de0 = 0xc63e0;
    availableMemory = 0x7c0600;
    if (availableMemory > memoryThreshold) {
        g_nAvailableGameMemory_005c8de0 = 0xc63e0;
        g_nMemoryConfiguration_005c8dc8 = 2;
        printf("Expanded Memory fully used.\n");
    } else {
        g_nAvailableGameMemory_005c8de0 =
            (int)g_dwInitialFreeMemory_005c8dd0 -
            g_nMinimumMemoryNoMusic_0049cc20;
        if (g_nAvailableGameMemory_005c8de0 < 0)
            exit_squadron(
                "You do not have enough memory to play Wing Commander 2.\n"
                "Refer to your reference guide for assistance.");
        g_nMemoryConfiguration_005c8dc8 = 0;
        if (g_nMusicDriverMode_0049be8c == 1 ||
            g_nMusicDriverMode_0049be8c == 2) {
            if (g_nMinimumMemoryFullMusic_0049cc24 <
                    (int)g_dwInitialFreeMemory_005c8dd0 &&
                (int)GetAvailableFarMemoryByType(4) >
                    g_nMusicMemoryRequirement_005c8dcc) {
                g_nAvailableGameMemory_005c8de0 =
                    (int)g_dwInitialFreeMemory_005c8dd0 -
                    g_nMinimumMemoryFullMusic_0049cc24;
                g_nMemoryConfiguration_005c8dc8 = 1;
                printf("Full");
            } else {
                printf("Limited");
            }
            printf(" music/FX will play.");
        }
    }
    LoadSpaceflightResources();
    LoadJoystickCalibrationFile(9, 9, 1, 1);
    ConfigureInputPump(1, PollJoystickButtonEvents);
    ConfigureDefaultSpacePalette(
        g_acExpectedGraphicsModes_00493078[
            g_bRewritePacketExtensions_0049cb48]);
    if (g_acExpectedGraphicsModes_00493078[
            g_bRewritePacketExtensions_0049cb48] !=
        QueryCurrentGraphicsMode())
        exit_squadron(
            "Requested graphics display mode not available.");
    LoadGamePaletteFile();
    InitializeGameTextContexts();
    postPaletteStatus = InitializePostPaletteState();
    if (g_nMusicDriverMode_0049be8c == 3)
        SetMusicTickRateHook(0x1e00);
    else
        SetMusicTickRateHook(0x3c);
    g_nFrameSkip_0049d764 = 1;
    if (g_nMusicDriverMode_0049be8c != 0 &&
        g_nMusicDriverMode_0049be8c != 3)
        InitializeMusicResources();
    initialize_direction_view_frames();
    strcat(startupPacketName,
           _itoa((int)g_nSelectedCampaignSlot_005d3bf2,
                 campaignNumber, 10));
    g_pStartupStarPacket_005d212c =
        LoadPacketAllocated(startupPacketName, 0);
    if (g_nOriginDevUnlock_0049d774 != 0)
        _unlink("logfile.txt");
    if (g_nShowMemoryStatus_0049d784 != 0) {
        _unlink("track0.txt");
        _unlink("track1.txt");
        _unlink("track2.txt");
        _unlink("track3.txt");
        _unlink("track4.txt");
        _unlink("track5.txt");
        _unlink("track6.txt");
        _unlink("track7.txt");
        _unlink("track8.txt");
        _unlink("track9.txt");
    }
    (void)expandedShipMemoryBytes;
    (void)postPaletteStatus;
}

/* Function start: 0x45BF30 */
void InitializeHighMemoryGraphicsBuffers(void)
{
    unsigned int highMemoryEnd;

    g_pHighMemoryBlockA_004901f8 =
        AllocateDefaultMemory((unsigned int)g_wHighMemoryBlockBytes_004901fc);
    g_dwHighMemoryParagraph_005d3fb4 =
        IdentityDword((Wc2DwordPtr)g_pHighMemoryBlockA_004901f8);
    highMemoryEnd = (unsigned int)g_wHighMemoryBlockBytes_004901fc +
        g_dwHighMemoryParagraph_005d3fb4;
    g_pHighMemoryBlockB_00490200 =
        AllocateDefaultMemory(WC2_CANNED_SCENE_SNAPSHOT_BYTES);
    if (g_pHighMemoryBlockB_00490200 == 0 ||
        g_pHighMemoryBlockA_004901f8 == 0) {
        FreePacketAndClear(&g_pHighMemoryBlockA_004901f8, 4);
        FreePacketAndClear(&g_pHighMemoryBlockB_00490200, 4);
        g_bHighMemoryBuffersReady_005d2ad8 = 0;
    } else {
        g_bHighMemoryBuffersReady_005d2ad8 = 1;
    }
    (void)highMemoryEnd;
}

/* Function start: 0x45C088 */
void InitializeGameTextContexts(void)
{
    InitializeDIBScreenViewport(&g_stScreenViewport_005d21a0,
                                (unsigned char)g_cSecondaryViewBufferColour_0049cb4c);
    g_stModalSourceViewport_005d2c50 = g_stScreenViewport_005d21a0;
    g_stDefaultTextContext_005d2d20.viewport =
        &g_stModalSourceViewport_005d2c50;
    g_stDefaultTextContext_005d2d20.text =
        g_szDefaultTextBuffer_005d2b80;
    InitializeTextContextFromFont(&g_stDefaultTextContext_005d2d20, 1,
                                  (unsigned char)g_ucPrimaryTextColour_0049cb64,
                                  g_cSecondaryViewBufferColour_0049cb4c);
    SetTextContext(&g_stDefaultTextContext_005d2d20);

    g_stSpaceTextContext_005d21c0.viewport = &g_stViewBuffer_005d2b00;
    g_stSpaceTextContext_005d21c0.text = g_szDefaultTextBuffer_005d2b80;
    g_stSpaceTextContext_005d21c0.alignment = 2;
    InitializeTextContextFromFont(&g_stSpaceTextContext_005d21c0, 1,
                                  g_abGamePaletteReservedColours_0049cb54[8], -1);
}

/* Function start: 0x45C1A1 */
unsigned int initialize_direction_view_frame(short yaw, short pitch,
                                             signed char frame)
{
    FixedVector right = { 0x100, 0, 0 };
    FixedVector up = { 0, 0x100, 0 };
    FixedVector forward = { 0, 0, 0x100 };
    short index;

    rotate_about_j(yaw, &right, &forward);
    rotate_about_i(pitch, &up, &forward);
    index = (short)frame;
    g_aDirectionViewRightVector_005d2210[index] = right;
    g_aDirectionViewUpVector_005d2500[index] = up;
    g_aDirectionViewForwardVector_005d27f0[index] = forward;
    return 0;
}

/* Function start: WC2_UNMAPPED */
unsigned int InitializeWc1DirectionViewFrames(void)
{
    signed char frame;
    signed char pitchBands;
    signed char yawSectors;
    short yaw;
    short pitch;

    frame = 1;
    pitch = 90;
    initialize_direction_view_frame(0, pitch, 0);
    pitchBands = 5;
    do {
        pitch -= 30;
        yaw = 0;
        yawSectors = 12;
        do {
            initialize_direction_view_frame(yaw, pitch, frame++);
            yaw += 30;
            yawSectors--;
        } while (yawSectors != 0);
        pitchBands--;
    } while (pitchBands != 0);
    initialize_direction_view_frame(0, -90, frame);
    return 0;
}

/* Function start: 0x45C279 */
void initialize_direction_view_frames(void)
{
    short yaw;
    short pitch;
    signed char frame;
    signed char pitchBand;
    signed char yawSector;

    yaw = 0;
    pitch = 90;
    frame = 0;
    initialize_direction_view_frame(yaw, pitch, frame++);
    for (pitchBand = 0; pitchBand < 5; pitchBand++) {
        pitch -= 30;
        yaw = 0;
        for (yawSector = 0; yawSector < 12; yawSector++) {
            initialize_direction_view_frame(yaw, pitch, frame++);
            yaw += 30;
        }
    }
    initialize_direction_view_frame(0, -90, frame);
}

/* Function start: 0x45C35C */
void LoadSpaceflightResources(void)
{
    if (g_bHighMemoryResourcesEnabled_005c80e4 == 0)
        return;
    if (LoadShapeSet(g_aCommon3SpaceResources_0049c728,
                     4, "objects.vga") == 0)
        return;
    LoadShapeSet(g_aMissionResourceDescriptors_0049c798,
                 4, "objects.vga");
    g_aObjectTypeData_00496d30[34].shapeSet =
        g_aObjectTypeData_00496d30[35].shapeSet;
    g_aObjectTypeData_00496d30[33].shapeSet =
        g_aObjectTypeData_00496d30[34].shapeSet;
    g_aObjectTypeData_00496d30[30].shapeSet =
        g_aObjectTypeData_00496d30[33].shapeSet;
    g_aObjectTypeData_00496d30[29].shapeSet =
        g_aObjectTypeData_00496d30[30].shapeSet;
    LoadShapeSet(g_aHighMemoryCockpitResources_0049c7e8,
                 4, "cockpit.vga");
}

/* Function start: 0x45C3FA */
void StopMusicIfDriverActive(void)
{
    if (g_nMusicDriverMode_0049be8c != 0)
        StopMusicStream();
}

/* Function start: 0x45C419 */
void InitializeMusicResources(void)
{
    int resource;

    FadeMusic(200);
    if (g_bHighMemoryResourcesEnabled_005c80e4 != 0) {
        for (resource = 0; resource < 0x42; resource++) {
            if (g_abMusicResourcePresent_0049cc30[resource] != 0) {
                if (g_nMusicDriverMode_0049be8c == 1) {
                    g_aMusicResources_005d13e0[resource].packet =
                        FetchDiskPacketRetrying(
                            "music.r00", (short)resource, 4);
                } else {
                    g_aMusicResources_005d13e0[resource].packet =
                        FetchDiskPacketRetrying(
                            "music.a00", (short)resource, 4);
                }
            } else {
                g_aMusicResources_005d13e0[resource].packet = 0;
            }
            if (g_aMusicResources_005d13e0[resource].packet == 0)
                continue;
            g_aMusicResources_005d13e0[resource].loaded = 1;
        }
    } else if (g_nMemoryConfiguration_005c8dc8 == 1) {
        g_pLimitedMusicBufferA_0049bea0 =
            AllocateTaggedMemory(0x33d0, 0x10);
        g_nLimitedMusicBufferAState_0049bea4 = -1;
        g_pLimitedMusicBufferB_0049bea6 =
            AllocateTaggedMemory(0x33d0, 0x10);
        g_nLimitedMusicBufferBState_0049beaa = -1;
    }
}

#pragma function(strcmp)

/* Function start: 0x45C558 */
#ifdef WC1_SDL
/* The port's entry point is the SDL launcher, so the game's own main() is
 * compiled under a name that does not collide with it. */
void Wc2GameMain(short argc, char **argv)
#else
void main(short argc, char **argv)
#endif
{
    short argumentIndex;
    int campaignIsNumeric;
    int missionIsNumeric;

    g_bRewritePacketExtensions_0049cb48 = 0;
    g_bSpeechResourcesEnabled_0049cc2c = 1;
    g_nSpeechCacheUnitCount_005c8dde = 2;
    g_nSpeechCacheSizeCode_005c8dd8 = 5;
    for (argumentIndex = 0; argumentIndex < argc; argumentIndex++) {
        if (strcmp("Origin", argv[argumentIndex]) == 0)
            g_nOriginDevUnlock_0049d774 = 1;

        switch (argv[argumentIndex][0]) {
        case '?':
            printf("Version %s.\n", g_pszGameVersion_0049b528);
        case '+':
            if (argv[argumentIndex][1] == 'w')
                g_bRoomTransitionAnimationEnabled_00499c00 = 1;
            if (argv[argumentIndex][1] == 'l')
                g_bAutopilotDebugEnabled_00499bfc = 1;
            if (argv[argumentIndex][1] == 'e') {
                g_bMemoryAdjustmentEnabled_0049cc84 = 1;
                g_nMemoryAdjustmentKb_005c8dda =
                    (short)atoi(argv[argumentIndex] + 2);
            }
            break;
        case '-':
            if (argv[argumentIndex][1] == 'm')
                g_nShowMemoryStatus_0049d784 = 1;
            if (argv[argumentIndex][1] == 'x')
                g_bPumpMessagesWhileLoading_005c8ddc = 1;
            if (argv[argumentIndex][1] == 't')
                g_bPumpMessagesDuringLoad_0049cc7c = 1;
            if (argv[argumentIndex][1] == 'f')
                g_bSkipCampaignScenes_0049cc78 = 1;
            if (g_nOriginDevUnlock_0049d774 != 0) {
                switch (argv[argumentIndex][1]) {
                case 'k':
                    g_bPlayerDamageEnabled_0049d77c = 0;
                    break;
                case 'b':
                    g_bPlayerCollisionEnabled_0049d780 = 0;
                    break;
                }
            }
            break;
        case 'E':
        case 'e':
            if (g_nOriginDevUnlock_0049d774 != 0)
                g_bDirectMissionLaunch_0049d798 = 1;
            break;
        case 'T':
        case 't':
            if (g_nOriginDevUnlock_0049d774 != 0) {
                missionIsNumeric = isdigit(argv[argumentIndex][1]);
                if (missionIsNumeric != 0) {
                    g_nDirectMission_0049d79a =
                        (short)atoi(argv[argumentIndex] + 1);
                } else {
                    g_nDirectMission_0049d79a = (short)(
                        toupper(argv[argumentIndex][1]) - 'A');
                }
                if (g_nDirectMission_0049d79a > 3)
                    g_nDirectMission_0049d79a = 0;
                g_bDirectCampaignSelection_0049cc74 = 1;
            }
            break;
        case 'V':
        case 'v':
            if (g_nOriginDevUnlock_0049d774 != 0) {
                g_nDirectSeries_0049d79c =
                    (short)atoi(argv[argumentIndex] + 1);
                g_bDirectCampaignSelection_0049cc74 = 1;
            }
            break;
        case 'Z':
        case 'z':
            campaignIsNumeric = isdigit(argv[argumentIndex][1]);
            if (campaignIsNumeric != 0) {
                g_nSelectedStartingCampaign_005d3bf0 =
                    (short)(argv[argumentIndex][1] - '0');
            } else {
                g_nSelectedStartingCampaign_005d3bf0 = 0;
            }
            break;
        case 'X':
        case 'x':
            g_bExpandedShipGraphicsEnabled_004931a4 = 1;
            g_nExpandedShipMemoryKb_0049cbac =
                (short)atoi(argv[argumentIndex] + 1);
            break;
        case 'A':
        case 'a':
        case 'P':
        case 'p':
        case 'R':
        case 'r':
            break;
        case 'C':
        case 'c':
            g_bSpeechResourcesEnabled_0049cc2c = 1;
            g_nSpeechCacheUnitCount_005c8dde =
                (short)(argv[argumentIndex][1] - '0');
            g_nSpeechCacheSizeCode_005c8dd8 =
                (short)(argv[argumentIndex][2] - '0');
            break;
        }
    }
}

#pragma intrinsic(strcmp)

/* Function start: WC2_UNMAPPED */
unsigned int GetFxDriverInitResult(void)
{
    return 0;
}

/* Function start: WC2_UNMAPPED */
unsigned int GetFxDriverStatus(void)
{
    return 0;
}

/* Function start: WC2_UNMAPPED */
short ace_status(short ace, unsigned char bits)
{
    return (g_stCampaignState_0059ca50.aceFlags[ace] & bits) == bits;
}

/* Function start: 0x429550 */
void SendKilrathiAceGreetingOnce(short obj)
{
    if (g_bKilrathiAceGreetingSent_00499bf4 == 0) {
        send_message(obj, 0x17);
        g_bKilrathiAceGreetingSent_00499bf4 = 1;
    }
}

/* Function start: 0x428E00 */
short GetMissionShipPilotSaveIndex(short missionShip)
{
    short pilot;

    pilot = g_aMissionShips_00492290[missionShip].pilot;
    if (pilot < 6)
        pilot = 99;
    else
        pilot = g_aMissionShips_00492290[missionShip].portrait;
    return pilot;
}

/* Function start: 0x428E5C */
short GetMissionShipVelocityState(short missionShip)
{
    short state;

    state = -1;
    if (g_aMissionShips_00492290[missionShip].state == 3)
        state = 0;
    return state;
}

/* Function start: 0x428E98 */
short GetInitialMissionShipPilotSaveIndex(short initialShip)
{
    short pilot;

    pilot = g_stMissionHeader_005d3e70.initialMissionShips[initialShip];
    if (pilot != -1)
        pilot = GetMissionShipPilotSaveIndex(pilot);
    return pilot;
}

/* Function start: 0x428EDC */
void StoreMissionResultsInCampaignGlobals(Wc2CampaignGlobals *globals)
{
    short value;
    short index;

    if (globals == 0) {
        globals = AllocateScenePointerTable(
            0x50, 2, 0, "ModuleToScript");
    }
    value = globals->pilotCount;
    for (index = 0; index < value; index++)
        globals->pilotStatus[index] = g_pPilotStatus_005d2fcc[index];

    for (index = 0; index < 8; index++) {
        globals->objectiveFlags[index] =
            achieved(index) | sighted(index) | visited(index);
        globals->objectiveSighted[index] = sighted(index);
    }
    for (index = 0; index < 16; index++)
        globals->shipVelocityState[index] =
            (short)(GetMissionShipVelocityState(index) + 1);

    globals->shipKillCounts[0] = g_cPlayerKillCount_005d2fa8;
    globals->shipMissionFlags[0] = 0;
    globals->damageLevel = (short)calculate_damage_level();
    globals->arcadeState = (short)g_nArcadeState_0049d75c;
    if (g_nArcadeState_0049d75c == 2)
        globals->shipMissionFlags[0] |= 2;
    if (g_asShipSide_004955d0[0] == 1)
        globals->shipMissionFlags[0] |= 4;
    if (g_nArcadeState_0049d75c >= 3)
        globals->shipMissionFlags[0] |= 1;

    for (index = 1; index < 8; index++) {
        globals->shipKillCounts[index] =
            g_acInitialShipKillCount_005d2fc0[index - 1];
        globals->shipMissionFlags[index] =
            g_acMissionShipStatusFlags_005d2fb0[index - 1];
        if (index == 1 && g_bMissionWingmanFlag_005c8dbe != 0)
            globals->shipMissionFlags[index] |= 8;
        value = GetInitialMissionShipPilotSaveIndex(index);
        if (value != -1)
            value = (short)(value * 2);
        globals->shipPilotIndices[index] = value;
    }
    globals->missionScore = g_nMissionScore_00493462;
    FreePacketAndClear(&g_pPilotStatus_005d2fcc, 0x80);
}

/* Function start: 0x42917D */
void InitializeCampaignConstellationState(Wc2CampaignGlobals *globals,
                                           short copyPosition)
{
    short pilotIndex;
    short pilotCount;

    pilotCount = globals->pilotCount;
    if (g_pPilotStatus_005d2fcc == 0)
        g_pPilotStatus_005d2fcc =
            (short *)calloc((int)pilotCount, 2);
    if (g_pPilotStatus_005d2fcc == 0)
        ReportFatalErrorCode("017");
    if (pilotCount != 32)
        ReportFatalErrorCode("018");

    for (pilotIndex = 0; pilotIndex < pilotCount; pilotIndex++)
        g_pPilotStatus_005d2fcc[pilotIndex] =
            globals->pilotStatus[pilotIndex];

    if (copyPosition != 0) {
        g_stCurrentPilotProfile_00493408.series = globals->series;
        g_stCurrentPilotProfile_00493408.mission = globals->mission;
    }
    g_nArcadeState_0049d75c = (int)globals->arcadeState;
}

/* Function start: 0x4293F9 */
void LoadTemporaryCampaignGlobals(void)
{
    ReleasePacketSlot(&g_pCampaignGlobals_00499c94);
    g_pCampaignGlobals_00499c94 =
        LoadWholeDataFile(g_szTemporaryCampaignGlobalsFile_00499bc0);
}

/* Function start: 0x429423 */
void SaveAndFreeTemporaryCampaignGlobals(void)
{
    short file;

    if (g_pCampaignGlobals_00499c94 != 0) {
        file = CreateDataFile(g_szTemporaryCampaignGlobalsFile_00499bc0);
        if (file >= 0) {
            WriteDataFileAtOffset(
                (unsigned short)file, 0,
                (unsigned int)*(unsigned short *)g_pCampaignGlobals_00499c94 *
                    2,
                g_pCampaignGlobals_00499c94);
            CloseDataFile((unsigned short)file);
        }
        ReleasePacketSlot(&g_pCampaignGlobals_00499c94);
    }
}

/* Function start: 0x42949F */
void *LoadWholeDataFile(const char *path)
{
    void *data;
    short file;
    unsigned int size;

    data = 0;
    file = OpenDataFileOrDie(path);
    if (file >= 0) {
        size = (unsigned int)_filelength(file);
        if (size == (unsigned int)-1) {
            CloseDataFile((unsigned short)file);
            return 0;
        }
        data = AllocateTaggedMemory(size, 0);
        if (data != 0)
            ReadDataFileAtOffset((unsigned short)file, 0, size, data);
        CloseDataFile((unsigned short)file);
    }
    return data;
}

/* Function start: 0x429581 */
short signed_random(short range)
{
    return RandomBelowOrEqual((short)(range * 2)) - range;
}

/* Function start: 0x4295A9 */
int alert_flag(short ship, unsigned int bits)
{
    return (g_anShipAlertFlags_00495f88[ship] & bits) != 0;
}

/* Function start: 0x4295D9 */
void set_alert(short i, unsigned int bits)
{
    g_anShipAlertFlags_00495f88[i] |= bits;
}

/* Function start: 0x4295F2 */
void clear_alert(short i)
{
    g_anShipAlertFlags_00495f88[i] = 0;
    g_acShipAlertSource_00495fb0[i] = -1;
    g_asShipAlertCountdown_00495fc0[i] = 0;
}

/* Function start: 0x429625 */
void start_collision_alert(short obj, short other)
{
    if ((short)g_acShipAlertSource_00495fb0[obj] != other) {
        g_acShipAlertSource_00495fb0[obj] = (signed char)other;
        steady_object(obj);
    }
    g_asShipAlertCountdown_00495fc0[obj] = 3;
    set_alert(obj, 1);
    g_asShipAfterburnerTimer_004955a8[obj] = 0;
    set_special(obj, SPECIAL_MANEUVER_NONE);
}

/* Function start: 0x429698 */
void try2end_collision_alert(short obj)
{
    g_asShipAlertCountdown_00495fc0[obj]--;
    if (g_asShipAlertCountdown_00495fc0[obj] <= 0)
        clear_alert(obj);
    else
        set_alert(obj, 2);
}

/* Function start: 0x4296E2 */
short normal_speed(short obj)
{
    if (g_aeSpecialManeuver_00495600[obj] !=
            SPECIAL_MANEUVER_AFTERBURNER &&
        real_velocity(obj) <= g_asShipMaximumVelocity_00495f70[obj])
        return 1;
    return 0;
}

/* Function start: 0x429736 */
short real_crash_time(short obj, short other)
{
    short collisionRadius;
    short time;
    short range;
    short elapsed;
    short step;
    int collisionFound;
    long distance;
    long relativeSpeed;
    FixedVector relativePosition;
    FixedVector relativeVelocity;
    FixedVector travel;
    FixedVector separation;

    collisionRadius = g_asObjectCollisionRadius_004950e8[obj];
    collisionRadius += g_asObjectCollisionRadius_004950e8[other];
    collisionRadius += 30;
    collisionFound = 0;
    ComputeVectorDelta(&g_aShipPosition_00494550[obj],
                       &g_aShipPosition_00494550[other],
                       &relativePosition);
    distance = Vector_magnitude(&relativePosition);
    if (((long)collisionRadius + 1500) * 0x100 < distance)
        return 0x7fff;

    ComputeVectorDelta(&g_aShipVelocity_00494898[obj],
                       &g_aShipVelocity_00494898[other],
                       &relativeVelocity);
    relativeSpeed = Vector_magnitude(&relativeVelocity);
    if (relativeSpeed == 0)
        return 0x7fbc;

    time = FixedToShortSaturating(
        DivideFixed((int)distance, (int)relativeSpeed));
    if (time >= 30)
        return time;

    ScaleFixedVector(&relativeVelocity, (int)time << 8, &travel);
    AddFixedVectors(&relativePosition, &travel, &separation);
    range = FixedToShortSaturating(
        Vector_magnitude(&separation));
    if (range > collisionRadius) {
        if ((collisionRadius * 2 >> 4) > range)
            return 25;
        return 32000;
    }

    elapsed = 0;
    do {
        if (elapsed >= time)
            break;
        step = MaxShort(1, (short)((time - elapsed) >> 1));
        elapsed += step;
        ScaleFixedVector(&relativeVelocity, (int)elapsed << 8, &travel);
        AddFixedVectors(&relativePosition, &travel, &separation);
        range = FixedToShortSaturating(
            Vector_magnitude(&separation));
        if (collisionRadius >= range)
            collisionFound = 1;
    } while (collisionFound == 0);
    return elapsed;
}

/* Function start: 0x429958 */
short crash_time(short obj, short other)
{
    if (g_aeObjectClass_00495328[other] == OBJECT_CLASS_ASTEROID &&
        (g_asObjectScreenX_00493598[other] == -0x7fff ||
         g_asObjectScreenX_00493598[obj] == -0x7fff))
        return 0x7fff;
    return real_crash_time(obj, other);
}

/* Function start: 0x4299C9 */
short detect_collisions(short obj)
{
    short time;
    short closestTime;
    short candidate;
    short other;

    candidate = -1;
    closestTime = 30;
    g_nCollisionCountdown_005d2faa = 1000;
    candidate = -1;
    if (g_pActiveHazardField_00493278 != 0) {
        for (other = 0; other <= 0x42; other++) {
            if (g_aeObjectClass_00495328[other] != OBJECT_CLASS_NULL &&
                other != obj &&
                g_aeObjectClass_00495328[other] >= OBJECT_CLASS_ASTEROID &&
                g_aeObjectClass_00495328[other] != OBJECT_CLASS_MISSILE) {
                time = crash_time(obj, other);
                if (time < closestTime) {
                    closestTime = time;
                    candidate = other;
                }
            }
        }
    } else {
        for (other = 0; other <= 9; other++) {
            if (g_aeObjectClass_00495328[other] != OBJECT_CLASS_NULL &&
                other != obj &&
                g_aeObjectClass_00495328[other] != OBJECT_CLASS_MISSILE) {
                time = crash_time(obj, other);
                if (time < closestTime) {
                    closestTime = time;
                    candidate = other;
                }
            }
        }
    }
    if (candidate != -1)
        g_nCollisionCountdown_005d2faa = closestTime;
    return candidate;
}

/* Function start: 0x429B55 */
short unactive(short ship)
{
    if (ship == -1 ||
        g_aeObjectClass_00495328[ship] < OBJECT_CLASS_SHIP ||
        g_asShipManeuver_00495f48[ship] == MANEUVER_HARD_BRAKE)
        return 1;
    return 0;
}

/* Function start: 0x429BA8 */
int are_alive(short obj)
{
    if (unactive(obj) == 0 &&
        g_aeShipObjective_00495f08[obj] != WC2_SHIP_OBJECTIVE_NOT_ALIVE)
        return 1;
    return 0;
}

/* Function start: 0x429BF0 */
void trim_goals(short obj, short amount)
{
    if (g_anYawGoal_004954c0[obj] > amount)
        g_anYawGoal_004954c0[obj] = amount;
    else if (g_anYawGoal_004954c0[obj] < -amount)
        g_anYawGoal_004954c0[obj] = -amount;
    if (g_anPitchGoal_004954a8[obj] > amount)
        g_anPitchGoal_004954a8[obj] = amount;
    else if (g_anPitchGoal_004954a8[obj] < -amount)
        g_anPitchGoal_004954a8[obj] = -amount;
}

/* Function start: 0x429CAD */
short report_kilrathi_rout(int mode)
{
    short obj;

    for (obj = 0; obj < 10; obj++) {
        if (g_aeObjectClass_00495328[obj] < OBJECT_CLASS_SHIP ||
            g_aeSpecialManeuver_00495600[obj] ==
                SPECIAL_MANEUVER_UNKNOWN_9)
            continue;
        switch (mode) {
        case 0:
            if (g_asShipSide_004955d0[obj] == SIDE_KILRATHI)
                return 1;
            break;
        case 1:
            if (g_asShipSide_004955d0[obj] == SIDE_KILRATHI &&
                g_asShipMissionType_00495de8[obj] != MISSION_TYPE_ROUT &&
                distance_from_object(0, obj) < 16000)
                return 1;
            break;
        case 2:
            if (g_asShipSide_004955d0[obj] == SIDE_KILRATHI &&
                (g_aeShipObjective_00495f08[obj] ==
                     OBJECTIVE_ENGAGE_ENEMY ||
                 g_aeShipObjective_00495f08[obj] ==
                     OBJECTIVE_DESTROY_SHIP))
                return 1;
            break;
        }
    }
    if (mode == 1 && g_nCurrentWave_004931c0 != -1)
        check_next_wave();
    return 0;
}

/* Function start: 0x429E24 */
short find_ship_index(short missionShip)
{
    short obj;

    g_nLastFoundShip_005d2fc8 = -1;
    if (missionShip != -1) {
        for (obj = 0; obj < 10; obj++) {
            if (((g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_SHIP &&
                  g_aeSpecialManeuver_00495600[obj] !=
                      SPECIAL_MANEUVER_UNKNOWN_9) ||
                 g_aeObjectClass_00495328[obj] == OBJECT_CLASS_FUTURION) &&
                g_asShipMissionIndex_00495d00[obj] == missionShip) {
                g_nLastFoundShip_005d2fc8 = obj;
                return g_nLastFoundShip_005d2fc8;
            }
        }
    }
    return g_nLastFoundShip_005d2fc8;
}

/* Function start: 0x429EE0 */
short try2rout(short obj)
{
    short other;
    short friendlyBase;

    friendlyBase = 0;
    if (any_enemy(obj, 10000) != 0) {
        for (other = 0; other < 10; other++) {
            if (g_aeObjectClass_00495328[other] >=
                    OBJECT_CLASS_CAPITAL_SHIP &&
                g_aeSpecialManeuver_00495600[other] !=
                    SPECIAL_MANEUVER_UNKNOWN_9 &&
                g_asShipSide_004955d0[other] ==
                    g_asShipSide_004955d0[obj])
                friendlyBase = 1;
        }
    }
    if (friendlyBase != 0) {
        g_acShipStress_00496100[obj] = 0;
        maneuver_complete(obj);
    } else {
        reset_mission_type(obj, MISSION_TYPE_ROUT);
        if (obj == g_nYourWingman_0049346c) {
            g_acMissionShipStatusFlags_005d2fb0[0] |= 4;
            send_message(obj, 0xb);
        }
    }
    return (short)(friendlyBase == 0);
}

/* Function start: 0x42A003 */
signed char CanSetNewShipTurnGoal(short ship)
{
    if (GetAdaptiveTurnRate() > 120)
        return 1;
    return (g_anYawGoal_004954c0[ship] |
            g_anPitchGoal_004954a8[ship] |
            g_anRollGoal_004954d8[ship]) == 0;
}

/* Function start: 0x42A062 */
signed char no_goal(short ship)
{
    return (g_anYawGoal_004954c0[ship] |
            g_anPitchGoal_004954a8[ship] |
            g_anRollGoal_004954d8[ship]) == 0;
}

/* Function start: 0x42A0A9 */
int being_tailed(short obj, short other)
{
    get_facing_range_from_object(obj, other);
    return g_nFacingToTarget_00493194 < -60 &&
           g_nTargetFacing_00493198 > 85 &&
           g_nTargetRange_0049319c < 7000;
}

/* Function start: 0x42A108 */
short detect_enemy_tail(short obj)
{
    short other;

    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass_00495328[other] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver_00495600[other] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[other] &&
            g_acShipTarget_00495f20[other] == obj &&
            being_tailed(obj, other) != 0)
            return other;
    }
    return -1;
}

/* Function start: 0x42A1C5 */
short is_ship_tailing_player_target(short obj)
{
    short target = g_acShipTarget_00495f20[0];

    if (unactive(target) == 0 && being_tailed(target, obj) != 0)
        return 1;
    return 0;
}

/* Function start: 0x42A222 */
short FindMissileTargetingObject(short obj)
{
    short other;

    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass_00495328[other] == OBJECT_CLASS_MISSILE &&
            g_acShipTarget_00495f20[other] == obj)
            return other;
    }
    return 0;
}

/* Function start: 0x42A28E */
unsigned int build_squad_list(short leader)
{
    short obj;
    short index = 1;

    g_acShipList_00496148[0] = (signed char)leader;
    g_acShipList_00496148[1] = -1;
    for (obj = 0; obj < 10; obj++) {
        if (g_asShipWingLeader_00495dd0[obj] == leader) {
            g_acShipList_00496148[index++] = (signed char)obj;
            g_acShipList_00496148[index] = -1;
        }
    }
    return 0;
}

/* Function start: 0x42A313 */
unsigned int find_squad_center(FixedVector *center)
{
    short count = 0;

    zero_vector(center);
    while (g_acShipList_00496148[count] != -1) {
        AddFixedVectors(center, &g_aShipPosition_00494550[
                            (short)g_acShipList_00496148[count]],
                        center);
        count++;
    }
    if (count != 0)
        divide_vector(center, (int)count << 8, center);
    return 0;
}

/* Function start: 0x42A39A */
unsigned int init_formation_burst(short obj)
{
    FixedVector center;
    FixedVector *destination;
    short member;
    short index = 0;

    build_squad_list(obj);
    find_squad_center(&center);
    member = g_acShipList_00496148[0];
    while (member != -1) {
        destination = &g_aShipDestination_004953f0[member];
        ComputeVectorDelta(&center, &g_aShipPosition_00494550[member],
                           destination);
        ScaleFixedVector(destination, 0xa00, destination);
        AddFixedVectors(destination, &g_aShipPosition_00494550[member],
                        destination);
        steady_object(member);
        reset_objective(member, OBJECTIVE_BREAK_FORMATION);
        member = g_acShipList_00496148[++index];
    }
    return 0;
}

/* Function start: 0x42A490 */
void reset_mission_type(short obj, enum ShipMissionType missionType)
{
    if (missionType == MISSION_TYPE_ROUT &&
        g_asShipSide_004955d0[obj] == SIDE_KILRATHI)
        report_kilrathi_rout(1);
    reset_objective(obj, OBJECTIVE_NONE);
    g_asShipMissionType_00495de8[obj] = missionType;
}

/* Function start: 0x42A4E5 */
unsigned int change_mission_type(short obj,
                                 enum ShipMissionType missionType)
{
    if (g_aeShipObjective_00495f08[obj] == OBJECTIVE_ENGAGE_ENEMY)
        g_asShipMissionType_00495de8[obj] = missionType;
    else
        reset_mission_type(obj, missionType);
    return 0;
}

/* Function start: 0x42A52A */
unsigned int reset_objective(short ship, enum ShipObjective objective)
{
    steady_object(ship);
    reset_tactic(ship, TACTIC_NONE);
    g_aeShipObjective_00495f08[ship] = objective;
    return 0;
}

/* Function start: 0x42A55F */
unsigned int alter_objective(short ship, enum ShipObjective objective)
{
    steady_object(ship);
    alter_tactic(ship, TACTIC_NONE);
    g_aeShipObjective_00495f08[ship] = objective;
    return 0;
}

/* Function start: 0x42A594 */
unsigned int reset_tactic(short ship, enum ShipTactic tactic)
{
    reset_maneuver(ship, MANEUVER_NONE);
    g_asShipTactic_00495f30[ship] = tactic;
    g_acShipTarget_00495f20[ship] = -1;
    return 0;
}

/* Function start: 0x42A5C8 */
unsigned int alter_tactic(short ship, enum ShipTactic tactic)
{
    reset_maneuver(ship, MANEUVER_NONE);
    g_asShipTactic_00495f30[ship] = tactic;
    return 0;
}

/* Function start: 0x42A5F1 */
void reset_maneuver(short ship, short maneuver)
{
    g_asShipManeuver_00495f48[ship] = (enum ShipManeuver)maneuver;
    g_asShipCount_00495ff8[ship] = 0;
    g_acShipSequence_00495fe8[ship] = 0;
}

/* Function start: 0x42A625 */
void try2reset_maneuver(short obj, short maneuver)
{
    if (g_asShipManeuver_00495f48[obj] != maneuver) {
        reset_maneuver(obj, maneuver);
        steady_object(obj);
    }
}

/* Function start: 0x42A664 */
void set_special(short ship, short special)
{

    if (g_aeSpecialManeuver_00495600[ship] <
            SPECIAL_MANEUVER_LOST_CONTROL ||
        special > g_aeSpecialManeuver_00495600[ship])
        g_aeSpecialManeuver_00495600[ship] = special;

    if (g_aeSpecialManeuver_00495600[ship] ==
            SPECIAL_MANEUVER_BLOWING_UP &&
        (short)alert_flag(ship, 1) != 0)
        g_aeSpecialManeuver_00495600[ship] = SPECIAL_MANEUVER_NONE;
}

/* Function start: 0x42A6E8 */
unsigned int approach_zero_speed(short ship)
{
    approach_speed(ship, 0);
    return 0;
}

/* Function start: 0x42A701 */
unsigned int approach_min_speed(short obj)
{
    approach_speed(obj, 0x500);
    return 0;
}

/* Function start: 0x42A71D */
void approach_half_speed(short obj)
{
    approach_speed(
        obj,
        (int)(short)(g_aObjectTypeData_00496d30[
            g_acObjectType_00493980[obj]].cruiseVelocity & 0xfffe) << 7);
}

/* Function start: 0x42A754 */
unsigned int approach_cruise_speed(short ship)
{
    approach_speed(ship,
        (int)g_aObjectTypeData_00496d30[
            g_acObjectType_00493980[ship]].cruiseVelocity << 8);
    return 0;
}

/* Function start: 0x42A788 */
unsigned int approach_full_speed(short ship)
{
    approach_speed(ship,
                      (int)g_asShipMaximumVelocity_00495f70[ship] << 8);
    return 0;
}

/* Function start: 0x42A7AF */
unsigned int approach_ship_speed(short obj, short other)
{
    approach_speed(obj, g_anShipSpeed_00494e20[other]);
    return 0;
}

/* Function start: 0x42A7D2 */
void ComputePointAheadOfObject(short obj, unsigned short distance,
                               FixedVector *point)
{
    ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                     (int)distance << 8, point);
    AddFixedVectors(&g_aShipPosition_00494550[obj], point, point);
}

/* Function start: 0x42A823 */
void ComputePointBehindObject(short obj, unsigned short distance,
                              FixedVector *point)
{
    ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                     -((int)distance << 8), point);
    AddFixedVectors(&g_aShipPosition_00494550[obj], point, point);
}

/* Function start: 0x42A876 */
unsigned int close_behind(short range)
{
    if (g_nTargetRange_0049319c < range &&
        g_nTargetFacing_00493198 < -0x32)
        return 1;
    return 0;
}

/* Function start: 0x42A8B5 */
short scan_for_enemy(short obj, unsigned short range)
{
    short target;
    short other;
    short distance;

    target = -1;
    g_nTargetRange_0049319c = 0;
    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass_00495328[other] < OBJECT_CLASS_SHIP ||
            g_aeSpecialManeuver_00495600[other] ==
                SPECIAL_MANEUVER_UNKNOWN_9)
            continue;
        g_nTargetShip_004931a0 = target;
        if (g_asShipSide_004955d0[obj] == g_asShipSide_004955d0[other])
            continue;
        distance = distance_from_point(other, &g_aShipPosition_00494550[obj]);
        target = g_nTargetShip_004931a0;
        if (distance < range &&
            (target == -1 ||
             distance < g_nTargetRange_0049319c)) {
            target = other;
            g_nTargetRange_0049319c = distance;
        }
    }
    if (target != -1) {
        g_nTargetShip_004931a0 = target;
        get_facing_range_from_object(obj, g_nTargetShip_004931a0);
        g_nTargetRange_0049319c =
            distance_from_object(obj, g_nTargetShip_004931a0);
        target = g_nTargetShip_004931a0;
    }
    g_nTargetShip_004931a0 = target;
    return target;
}

/* Function start: 0x42AA0D */
short any_enemy(short obj, short range)
{
    short other;

    g_nTargetShip_004931a0 = -1;
    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass_00495328[other] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver_00495600[other] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[other]) {
            g_nTargetRange_0049319c = distance_from_object(obj, other);
            if (g_nTargetRange_0049319c < range) {
                g_nTargetShip_004931a0 = other;
                return 1;
            }
        }
    }
    return 0;
}

/* Function start: 0x42AB81 */
unsigned int ships_within_range(short obj, short other, short range)
{
    FixedVector delta;

    ComputeVectorDelta(&g_aShipPosition_00494550[obj],
                       &g_aShipPosition_00494550[other], &delta);
    return IsVectorWithinRange(&delta, range);
}

/* Function start: 0x42ABCE */
int attacker_in_range(short obj, short range)
{
    short other;

    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass_00495328[other] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver_00495600[other] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[other] &&
            g_acShipTarget_00495f20[other] == obj) {
            g_nTargetRange_0049319c = distance_from_object(other, obj);
            if ((unsigned short)g_nTargetRange_0049319c <
                (unsigned short)range) {
                g_nTargetShip_004931a0 = other;
                return 1;
            }
        }
    }
    g_nTargetShip_004931a0 = -1;
    return 0;
}

/* Function start: 0x42ACAF */
int in_danger(short obj)
{
    short target;
    short other;
    unsigned short range;

    target = -1;
    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass_00495328[other] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver_00495600[other] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[other]) {
            g_nTargetShip_004931a0 = target;
            if (g_acShipTarget_00495f20[other] != obj)
                continue;
            range = (unsigned short)distance_from_object(other, obj);
            target = g_nTargetShip_004931a0;
            if (target == -1 || range < g_nTargetRange_0049319c) {
                target = other;
                g_nTargetRange_0049319c = (short)range;
            }
        }
    }
    g_nTargetShip_004931a0 = target;
    return target != -1;
}

/* Function start: 0x42ADB8 */
short target_within_range(short obj)
{
    short target = g_acShipTarget_00495f20[obj];

    if (unactive(target) != 0)
        return 0;
    if (target_valid(obj, target) == 0)
        return 0;
    return ships_within_range(obj, target, 7000);
}

/* Function start: 0x42AE32 */
short build_target_list(short obj, short range)
{
    short count;
    short other;
    short distance;

    count = 0;
    for (other = 0; other < 10; other++) {
        if (g_aeObjectClass_00495328[other] >= OBJECT_CLASS_SHIP &&
            g_aeSpecialManeuver_00495600[other] !=
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[other]) {
            distance = distance_from_object(obj, other);
            if (distance < range) {
                g_acShipList_00496148[count] =
                    (signed char)other;
                g_asShipListRange_00496158[count] = distance;
                count++;
            }
        }
    }
    g_acShipList_00496148[count] = -1;
    return count;
}

/* Function start: 0x42AF60 */
void inherit_leader_mission(short obj)
{
    short leader = g_asShipWingLeader_00495dd0[obj];

    if (leader != -1 &&
        g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_SHIP) {
        g_asShipMissionType_00495de8[obj] =
            g_asShipMissionType_00495de8[leader];
        g_asShipMissionParameter_00495e00[obj] =
            g_asShipMissionParameter_00495e00[leader];
        g_asShipWingLeader_00495dd0[obj] = -1;
        g_aShipMissionSpot_00495e18[obj] =
            g_aShipMissionSpot_00495e18[leader];
    }
}

/* Function start: 0x42B00A */
void inherit_leader(short obj)
{
    short leader = g_asShipWingLeader_00495dd0[obj];
    short other;

    if (leader == -1 || g_aeObjectClass_00495328[obj] < OBJECT_CLASS_SHIP)
        return;
    inherit_leader_mission(obj);
    g_asShipWingLeader_00495dd0[obj] = g_asShipWingLeader_00495dd0[leader];
    for (other = 0; other < 10; other++) {
        if (g_asShipWingLeader_00495dd0[other] == leader)
            g_asShipWingLeader_00495dd0[other] = obj;
    }
}

/* Function start: 0x42B0B7 */
short dead_ship(short i)
{
    return i == -1 || g_aMissionShips_00492290[i].state == 3;
}

/* Function start: 0x42B0FB */
int gone_ship(short missionShip)
{
    if (missionShip == -1 ||
        g_aMissionShips_00492290[missionShip].state == 3 ||
        g_aMissionShips_00492290[missionShip].state == 2)
        return 1;
    return 0;
}

/* Function start: 0x42B15A */
short skill_check(short obj)
{
    if (g_asPilotLevel_00495d60[obj] < 5 &&
        RandomBelowOrEqual((short)(g_asPilotLevel_00495d60[obj] + 4)) == 0)
        return 0;
    return 1;
}

/* Function start: 0x42B1AE */
short find_ships_sphere(short missionShip)
{
    MissionNavPoint *navPoint;
    short fallback;
    short navIndex;
    short shipIndex;

    fallback = -1;
    navPoint = g_aMissionNavPoints_00491e98;
    navIndex = 0;
    for (; navIndex < WC2_MISSION_NAV_POINT_COUNT;
         navIndex++, navPoint++) {
        for (shipIndex = 0; shipIndex < WC2_MISSION_NAV_POINT_COUNT;
             shipIndex++) {
            if (navPoint->missionShips[shipIndex] == missionShip) {
                if (navPoint->type == 1)
                    return navIndex;
                if (fallback == -1)
                    fallback = navIndex;
            }
        }
    }
    if (fallback == -1)
        return -1;
    navIndex = fallback;
    while (g_aMissionNavPoints_00491e98[navIndex].type > 1)
        navIndex--;
    return navIndex;
}

/* Function start: 0x42B2A7 */
int locate_ship(short missionShip, FixedVector *point)
{
    short navPoint;
    short obj;

    obj = find_ship_index(missionShip);
    if (obj != -1) {
        *point = g_aShipPosition_00494550[obj];
        return 1;
    }
    if (dead_ship(missionShip) != 0)
        return 0;
    navPoint = find_ships_sphere(missionShip);
    if (navPoint != -1) {
        AddFixedVectors(&g_aMissionNavPoints_00491e98[navPoint].position,
                        &g_aMissionShips_00492290[missionShip].position,
                        point);
        return 1;
    }
    zero_vector(point);
    return 0;
}

/* Function start: 0x42B38F */
unsigned int get_follow_point(short obj, FixedVector *point)
{
    short pathIndex;
    short objective;
    short missionShip;
    int type;

    if (g_asShipSide_004955d0[obj] == SIDE_KILRATHI) {
        *point = g_aMissionNavPoints_00491e98[
            g_asShipMissionParameter_00495e00[obj]].position;
        return 0;
    }
    pathIndex = (short)g_abShipNavPointIndex_00495f60[obj];
    while (++pathIndex < WC2_MISSION_OBJECTIVE_COUNT) {
        objective = (short)g_abFlightPath_004932a0[pathIndex];
        type = g_aMissionObjectives_004932a8[objective].type;
        switch (type) {
        case 0:
            *point = g_aMissionObjectives_004932a8[objective].position;
            g_abShipNavPointIndex_00495f60[obj] = (signed char)pathIndex;
            return 0;
        case 1:
            missionShip = (short)g_aMissionObjectives_004932a8[objective].index;
            objective = find_ship_index(missionShip);
            if (objective == -1)
                *point = g_aMissionNavPoints_00491e98[missionShip].position;
            else
                *point = g_aShipPosition_00494550[objective];
            g_abShipNavPointIndex_00495f60[obj] = (signed char)pathIndex;
            return 0;
        }
    }
    return 0;
}

/* Function start: 0x42B524 */
unsigned int get_first_follow_point(short obj, FixedVector *point)
{
    if (g_asShipSide_004955d0[obj] == SIDE_IMPERIAL)
        g_abShipNavPointIndex_00495f60[obj] =
            g_cCurrentNavPointIndex_00493298 - 1;
    get_follow_point(obj, point);
    return 0;
}

/* Function start: 0x42B565 */
short hostile_sphere(short obj, short navPoint)
{
    short index;
    short missionShip;

    for (index = 0; index < 10; index++) {
        missionShip =
            g_aMissionNavPoints_00491e98[navPoint].missionShips[index];
        if (missionShip != -1 &&
            g_aMissionShips_00492290[missionShip].side !=
                g_asShipSide_004955d0[obj])
            return 1;
    }
    return 0;
}

/* Function start: 0x42B5F7 */
short abandoned(short obj, short other)
{
    short navPoint;

    if ((g_abShipTurn_00495fd8[obj] & 7) == 0 &&
        g_asShipSide_004955d0[obj] != SIDE_KILRATHI) {
        if (RandomBelowOrEqual(8) != 0)
            return 0;
        navPoint = FindNearestNavPoint(obj);
        if (g_nCurrentNavPoint_004931bc != navPoint &&
            hostile_sphere(obj, navPoint) != 0 &&
            distance_from_object(obj, other) > 10000) {
            explode(-1, obj);
            return 1;
        }
    }
    return 0;
}

/* Function start: 0x42B6C5 */
void engage(short obj, short target, enum ShipObjective objective)
{
    if (g_aeShipObjective_00495f08[obj] != objective) {
        reset_objective(obj, objective);
        if (g_asPilotLevel_00495d60[obj] == 5 &&
            g_asShipSide_004955d0[obj] == SIDE_KILRATHI)
            SendKilrathiAceGreetingOnce(obj);
    }
    g_acShipTarget_00495f20[obj] = target;
}

/* Function start: 0x42B73B */
short target_valid(short obj, short target)
{
    if (target == -1)
        return 0;
    if (g_anShipCloakState_00496020[target] == 1 &&
        g_asShipCloakElapsedFrames_00496060[target] >= 40)
        return 0;
    if (ShipHasTorpedo(obj) == 0 &&
        CanShipWeaponDamageTarget(obj, target) == 0)
        return 0;
    if (unactive(target) == 0 &&
        g_asShipSide_004955d0[obj] != g_asShipSide_004955d0[target])
        return 1;
    return 0;
}

/* Function start: 0x42B80C */
short triumph(short obj)
{
    short result;
    short objective;

    result = 0;
    switch (g_asShipMissionType_00495de8[obj]) {
    case MISSION_TYPE_PATROL:
        objective = 0;
        while (g_abFlightPath_004932a0[objective] != -1) {
            if (visited((short)g_abFlightPath_004932a0[objective]) != 0)
                objective++;
            else
                break;
        }
        result = 1;
        break;
    case MISSION_TYPE_ESCORT:
    case MISSION_TYPE_DEFEND:
    case MISSION_TYPE_WINGMAN:
        result = dead_ship(g_asShipMissionParameter_00495e00[obj]) == 0;
        break;
    case MISSION_TYPE_STRIKE:
        result = dead_ship(g_asShipMissionParameter_00495e00[obj]);
        break;
    }
    return result;
}

/* Function start: 0x42B916 */
short find_ratio(short inputMinimum, short inputMaximum,
                         short input, short outputMinimum,
                         short outputMaximum)
{
    if (input < inputMinimum)
        return outputMinimum;
    if (input > inputMaximum)
        return outputMaximum;
    return (short)(((int)(outputMaximum - outputMinimum) *
                   (int)(input - inputMinimum)) /
                   (int)(inputMaximum - inputMinimum)) + outputMinimum;
}

/* Function start: 0x42B985 */
short evaluate_damage(short obj)
{
    ObjectTypeData *typeData =
        &g_aObjectTypeData_00496d30[g_acObjectType_00493980[obj]];

    if (g_aeObjectClass_00495328[obj] < OBJECT_CLASS_SHIP)
        return 100;
    return (short)((g_acShipDamage_00495690[obj] * -26) /
                       typeData->damageCapacity +
                   (g_aasShipArmor_00495540[obj][1] * 27) /
                       typeData->armorRear +
                   (g_aasShipArmor_00495540[obj][0] * 23) /
                       typeData->armorFront +
                   (g_aasShipArmor_00495540[obj][2] * 12) /
                       typeData->armorLeft +
                   (g_aasShipArmor_00495540[obj][3] * 12) /
                       typeData->armorRight + 26);
}

/* Function start: 0x4564F0 */
short has_opposing_ship(short side)
{
    short obj;

    for (obj = 0; obj < 10; obj++) {
        if (g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_SHIP &&
            g_asShipSide_004955d0[obj] != side)
            return 1;
    }
    return 0;
}

/* Function start: 0x45655D */
void launch_carrier_fighter(short carrier)
{
    short elapsed;
    short source;
    short fighter;

    if (has_opposing_ship(g_asShipSide_004955d0[carrier]) == 0)
        return;

    switch (g_asObjectType_00495298[carrier]) {
    case 0x35:
        g_nCarrierLaunchInterval_005d108c = 300;
        break;
    case 0x38:
        g_nCarrierLaunchInterval_005d108c = 250;
        break;
    case 0x39:
        g_nCarrierLaunchInterval_005d108c = 150;
        break;
    case 0x3A:
        g_nCarrierLaunchInterval_005d108c = 100;
        break;
    case 0x3B:
        g_nCarrierLaunchInterval_005d108c = 75;
        break;
    default:
        return;
    }

    elapsed = g_asCarrierLaunchTimer_005d1090[carrier]++;
    if (elapsed < g_nCarrierLaunchInterval_005d108c)
        return;
    g_asCarrierLaunchTimer_005d1090[carrier] = 0;

    for (source = 0; source < 10; source++) {
        if (g_aeObjectClass_00495328[source] == OBJECT_CLASS_SHIP &&
            g_asShipSide_004955d0[source] ==
                g_asShipSide_004955d0[carrier])
            break;
    }
    if (source == 10)
        return;
    if (g_aObjectTypeData_00496d30[
            g_acObjectType_00493980[source]].resourceType == 10)
        return;

    fighter = initialize_ship(
        g_aObjectTypeData_00496d30[
            g_acObjectType_00493980[source]].resourceType,
        -1, 1);
    if (fighter == -1)
        return;

    Set_up_ship_info(fighter, g_asShipMissionIndex_00495d00[source],
                     (signed char)g_nCurrentNavPoint_004931bc);
    g_asShipMissionType_00495de8[fighter] = MISSION_TYPE_PATROL;
    g_acShipTarget_00495f20[fighter] = 0;
    g_aeShipObjective_00495f08[fighter] = OBJECTIVE_ENGAGE_ENEMY;
    g_asShipWingLeader_00495dd0[fighter] = -1;
    g_abShipEjectionSequenceEnabled_00496120[fighter] = 0;
    g_apShipMissionRecord_00495da8[fighter] =
        (MissionShipRecord *)&g_aObjectTypeData_00496d30[
            g_acObjectType_00493980[fighter]];
    if (g_asPilotLevel_00495d60[fighter] >= 5)
        g_asPilotLevel_00495d60[fighter] = 4;

    if (g_asShipSide_004955d0[fighter] == SIDE_KILRATHI) {
        g_acShipPortrait_00495d88[fighter] = 0x1A;
    } else if (RandomBelowOrEqual(1) != 0) {
        g_acShipPortrait_00495d88[fighter] = 0x14;
    } else {
        g_acShipPortrait_00495d88[fighter] = 0x15;
    }

    position_relative(
        &g_aShipPosition_00494550[fighter],
        g_aShipForwardVector_00494208[carrier],
        (short)(g_asObjectCollisionRadius_004950e8[carrier] + 500));
    g_anShipSpeed_00494e20[fighter] =
        (int)g_aObjectTypeData_00496d30[
            g_acObjectType_00493980[source]].maximumVelocity << 8;
    ScaleFixedVector(&g_aShipForwardVector_00494208[carrier],
                     g_anShipSpeed_00494e20[fighter],
                     &g_aShipVelocity_00494898[fighter]);
}

/* Function start: 0x4568E0 */
short LoadShapeSet(PacketResourceDescriptor *resources,
                   short flags, char *defaultFileName)
{
    while (resources->resource != 0) {
        if (*resources->resource == 0) {
            if (resources->fileName == 0) {
                *resources->resource = FetchDiskPacketRetrying(
                    defaultFileName, resources->section,
                    (unsigned short)(flags | 0x40));
            } else {
                *resources->resource = FetchDiskPacketRetrying(
                    resources->fileName, resources->section,
                    (unsigned short)(flags | 0x40));
            }
            if (*resources->resource == 0)
                return 0;
        }
        resources++;
    }
    return 1;
}

/* Function start: 0x456988 */
void FreeShapeSet(PacketResourceDescriptor *resources,
                  unsigned short releaseFlags)
{
    while (resources->resource != 0) {
        if (*resources->resource != 0)
            FreePacketAndClear(resources->resource, releaseFlags);
        resources++;
    }
}

/* Function start: 0x4569C8 */
int LoadPacketResourceList(PacketResourceDescriptor *resources,
                           short flags, int availableBytes,
                           char *defaultFileName)
{
    int packetSize;

    while (resources->resource != 0) {
        if (*resources->resource == 0) {
            packetSize = (int)GetNamedPacketSize(
                defaultFileName, resources->section);
            if (packetSize < availableBytes) {
                *resources->resource = FetchDiskPacketRetrying(
                    defaultFileName, resources->section, flags);
                if (*resources->resource == 0)
                    return availableBytes;
                availableBytes -= packetSize;
            }
        }
        resources++;
    }
    return availableBytes;
}

/* Function start: 0x456A68 */
void ResetCockpitPaletteEntries(void)
{
    short black[3];
    short index;

    black[0] = 0;
    black[1] = 0;
    black[2] = 0;
    for (index = 0; index < 6; index++) {
        memcpy(g_aasCockpitHitPaletteFades_005d2cb0[index], black,
               sizeof(black));
        SetPaletteEntry((short)(index + 0xf9),
                        g_aasCockpitHitPaletteFades_005d2cb0[index]);
    }
    memcpy(g_asSpacePaletteFade_005d2d60, black, sizeof(black));
    g_asSpacePaletteFade_005d2d60[2] = 32;
    SetPaletteEntry((short)g_cPrimaryViewBufferColour_0049cb88,
                    g_asSpacePaletteFade_005d2d60);
}

/* Function start: 0x456B1A */
void initialize_cockpit(signed char mode)
{
    Viewport savedScreen;
    char resourceNumber[4];
    unsigned char *backdrop;

    backdrop = 0;

    if (message_showing())
        EndCommMenu();

    if (g_nCockpitDisplayMode_0049d71c == 0 && mode == g_cScreenViewportMode_005c82a6) {
        if (g_stViewBuffer_005d2b00.pixels != 0)
            ClearViewport(&g_stViewBuffer_005d2b00, g_cPrimaryViewBufferColour_0049cb88);
        else
            initialize_view_buffer();
        return;
    }

    free_view_buffer();
    savedScreen = g_stScreenViewport_005d21a0;
    g_stScreenViewport_005d21a0.left = 0;
    g_stScreenViewport_005d21a0.top = 0;
    g_stScreenViewport_005d21a0.right = 319;
    g_stScreenViewport_005d21a0.bottom = 199;
    ClearViewport(&g_stModalSourceViewport_005d2c50, g_cSecondaryViewBufferColour_0049cb4c);

    g_cScreenViewportMode_005c82a6 = mode;
    strcpy(g_szCockpitResourceFilename_005d1030, "pcship.v");
    _itoa((int)g_aObjectTypeData_00496d30[
              g_acObjectType_00493980[0]].resourceType,
          resourceNumber, 10);
    if (strlen(resourceNumber) == 1)
        strcat(g_szCockpitResourceFilename_005d1030, "0");
    strcat(g_szCockpitResourceFilename_005d1030, resourceNumber);

    switch (mode) {
    case 0:
        if (g_apCockpitViewShapes_005d1040[0] != 0) {
            g_pCockpitViewFrameData_005d1088 =
                (unsigned char *)IdentityDword(
                    (unsigned int)g_apCockpitViewShapes_005d1040[0]);
            DosMemcpy(g_pCockpitBackgroundPacket_0049a5f0,
                      g_pCockpitViewFrameData_005d1088,
                      (unsigned short)g_nCockpitBackgroundBytes_0049c720);
        } else {
            LoadPacketIntoBuffer(
                g_szCockpitResourceFilename_005d1030, 4,
                g_pCockpitBackgroundPacket_0049a5f0, 1);
        }
        g_stCockpitBackgroundViewport_005d1050.left =
            g_stScreenViewport_005d21a0.left;
        g_stCockpitBackgroundViewport_005d1050.right =
            g_stScreenViewport_005d21a0.right;
        g_stCockpitBackgroundViewport_005d1050.top =
            g_stScreenViewport_005d21a0.top;
        g_stCockpitBackgroundViewport_005d1050.bottom =
            g_stScreenViewport_005d21a0.bottom;
        g_stCockpitBackgroundViewport_005d1050.pixels = 0;
        g_stCockpitBackgroundViewport_005d1050.allocation = 0;
        g_stCockpitBackgroundViewport_005d1050.rowOffsets = 0;
        if (g_bHighMemoryResourcesEnabled_005c80e4 != 0 &&
            AllocateViewport(
                &g_stCockpitBackgroundViewport_005d1050,
                g_cSecondaryViewBufferColour_0049cb4c, 0) == 0) {
            g_stCockpitBackgroundViewport_005d1050.allocation = 0;
        }
        if (g_apCockpitShipShapes_005d1070[0] != 0) {
            if (g_stCockpitBackgroundViewport_005d1050.allocation != 0) {
                if (g_nCockpitDisplayMode_0049d71c == 0) {
                    DrawSpriteDefault(
                        &g_stCockpitBackgroundViewport_005d1050,
                        0, 0, g_apCockpitShipShapes_005d1070[0], 0);
                } else if (g_nCockpitDisplayMode_0049d71c == 0) {
                    DrawSpriteDefault(
                        &g_stScreenViewport_005d21a0,
                        0, 0, g_apCockpitShipShapes_005d1070[0], 0);
                }
            }
        } else {
            backdrop = LoadNamedPacket(
                g_szCockpitResourceFilename_005d1030, 0,
                0, 0, 0, 1);
            if (backdrop == 0) {
                free_viewport(&g_stCockpitBackgroundViewport_005d1050);
                g_stCockpitBackgroundViewport_005d1050.allocation = 0;
                backdrop = FetchDiskPacketRetrying(
                    g_szCockpitResourceFilename_005d1030, 0, 0);
                if (g_nCockpitDisplayMode_0049d71c == 0) {
                    DrawSpriteDefault(
                        &g_stScreenViewport_005d21a0,
                        0, 0, backdrop, 0);
                }
            } else if (g_stCockpitBackgroundViewport_005d1050.allocation !=
                       0) {
                if (g_nCockpitDisplayMode_0049d71c == 0) {
                    DrawSpriteDefault(
                        &g_stCockpitBackgroundViewport_005d1050,
                        0, 0, backdrop, 0);
                } else if (g_nCockpitDisplayMode_0049d71c == 0) {
                    DrawSpriteDefault(
                        &g_stScreenViewport_005d21a0,
                        0, 0, backdrop, 0);
                }
            }
            ReleasePacketHandle(backdrop);
        }
        ResetCockpitPaletteEntries();
        if (g_stCockpitBackgroundViewport_005d1050.allocation != 0) {
            if (g_nCockpitDisplayMode_0049d71c == 0)
                explosion_draw(1);
            WaitForVerticalBlankThunk();
            CopyViewportContents(
                &g_stCockpitBackgroundViewport_005d1050,
                &g_stScreenViewport_005d21a0);
            free_viewport(&g_stCockpitBackgroundViewport_005d1050);
        } else if (g_nCockpitDisplayMode_0049d71c == 0) {
            explosion_draw(0);
        }
        reset_cockpit();
        InvalidateVduMode(0);
        InvalidateVduMode(1);
        update_VDUs();
        if (g_bCockpitDamageFrame0Shown_0049b2b4 == 0)
            clear_head_up_display();
        if (g_nCockpitDisplayMode_0049d71c == 0)
            ResetPilotHandAnimation();
        set_up_screen_viewport(0);
        break;
    case 1:
        if (g_apCockpitViewShapes_005d1040[1] != 0) {
            g_pCockpitViewFrameData_005d1088 =
                (unsigned char *)IdentityDword(
                    (unsigned int)g_apCockpitViewShapes_005d1040[1]);
            DosMemcpy(g_pCockpitBackgroundPacket_0049a5f0,
                      g_pCockpitViewFrameData_005d1088,
                      (unsigned short)g_nCockpitBackgroundBytes_0049c720);
        } else {
            LoadPacketIntoBuffer(
                g_szCockpitResourceFilename_005d1030, 5,
                g_pCockpitBackgroundPacket_0049a5f0, 1);
        }
        if (g_apCockpitShipShapes_005d1070[1] != 0) {
            if (g_nCockpitDisplayMode_0049d71c == 0)
                DrawSpriteDefault(&g_stScreenViewport_005d21a0, 0, 0,
                                  g_apCockpitShipShapes_005d1070[1], 0);
        } else {
            backdrop = FetchDiskPacketRetrying(
                g_szCockpitResourceFilename_005d1030, 1, 0);
            if (g_nCockpitDisplayMode_0049d71c == 0)
                DrawSpriteDefault(&g_stScreenViewport_005d21a0, 0, 0, backdrop, 0);
            ReleasePacketHandle(backdrop);
        }
        set_up_screen_viewport(1);
        break;
    case 2:
        if (g_nCurrentView_00492fa8 == 4) {
            if (g_apCockpitViewShapes_005d1040[1] != 0) {
                g_pCockpitViewFrameData_005d1088 =
                    (unsigned char *)IdentityDword(
                        (unsigned int)g_apCockpitViewShapes_005d1040[1]);
                DosMemcpy(
                    g_pCockpitBackgroundPacket_0049a5f0,
                    g_pCockpitViewFrameData_005d1088,
                    (unsigned short)g_nCockpitBackgroundBytes_0049c720);
            } else {
                LoadPacketIntoBuffer(
                    g_szCockpitResourceFilename_005d1030, 5,
                    g_pCockpitBackgroundPacket_0049a5f0, 1);
            }
            if (g_apCockpitShipShapes_005d1070[1] != 0) {
                if (g_nCockpitDisplayMode_0049d71c == 0) {
                    DrawSpriteDefault(
                        &g_stScreenViewport_005d21a0, 0, 0,
                        g_apCockpitShipShapes_005d1070[1], 0);
                }
            } else {
                backdrop = FetchDiskPacketRetrying(
                    g_szCockpitResourceFilename_005d1030, 1, 0);
                if (g_nCockpitDisplayMode_0049d71c == 0) {
                    DrawSpriteDefault(
                        &g_stScreenViewport_005d21a0,
                        0, 0, backdrop, 0);
                }
                ReleasePacketHandle(backdrop);
            }
        } else {
            if (g_apCockpitViewShapes_005d1040[2] != 0) {
                g_pCockpitViewFrameData_005d1088 =
                    (unsigned char *)IdentityDword(
                        (unsigned int)g_apCockpitViewShapes_005d1040[2]);
                DosMemcpy(
                    g_pCockpitBackgroundPacket_0049a5f0,
                    g_pCockpitViewFrameData_005d1088,
                    (unsigned short)g_nCockpitBackgroundBytes_0049c720);
            } else {
                LoadPacketIntoBuffer(
                    g_szCockpitResourceFilename_005d1030, 6,
                    g_pCockpitBackgroundPacket_0049a5f0, 1);
            }
            if (g_apCockpitShipShapes_005d1070[2] != 0) {
                if (g_nCockpitDisplayMode_0049d71c == 0) {
                    DrawSpriteDefault(
                        &g_stScreenViewport_005d21a0, 0, 0,
                        g_apCockpitShipShapes_005d1070[2], 0);
                }
            } else {
                backdrop = FetchDiskPacketRetrying(
                    g_szCockpitResourceFilename_005d1030, 2, 0);
                if (g_nCockpitDisplayMode_0049d71c == 0) {
                    DrawSpriteDefault(
                        &g_stScreenViewport_005d21a0,
                        0, 0, backdrop, 0);
                }
                ReleasePacketHandle(backdrop);
            }
        }
        set_up_screen_viewport(2);
        break;
    case 3:
        if (g_apCockpitViewShapes_005d1040[3] != 0) {
            g_pCockpitViewFrameData_005d1088 =
                (unsigned char *)IdentityDword(
                    (unsigned int)g_apCockpitViewShapes_005d1040[3]);
            DosMemcpy(g_pCockpitBackgroundPacket_0049a5f0,
                      g_pCockpitViewFrameData_005d1088,
                      (unsigned short)g_nCockpitBackgroundBytes_0049c720);
        } else {
            LoadPacketIntoBuffer(
                g_szCockpitResourceFilename_005d1030, 7,
                g_pCockpitBackgroundPacket_0049a5f0, 1);
        }
        if (g_apCockpitShipShapes_005d1070[3] != 0) {
            if (g_nCockpitDisplayMode_0049d71c == 0)
                DrawSpriteDefault(&g_stScreenViewport_005d21a0, 0, 0,
                                  g_apCockpitShipShapes_005d1070[3], 0);
        } else {
            backdrop = FetchDiskPacketRetrying(
                g_szCockpitResourceFilename_005d1030, 3, 0);
            if (g_nCockpitDisplayMode_0049d71c == 0)
                DrawSpriteDefault(&g_stScreenViewport_005d21a0, 0, 0, backdrop, 0);
            ReleasePacketHandle(backdrop);
        }
        set_up_screen_viewport(3);
        break;
    case 4:
        set_up_screen_viewport(4);
        break;
    case 5:
        set_up_screen_viewport(4);
        break;
    case 6:
        set_up_screen_viewport(5);
        break;
    case 7:
        break;
    }

    if (g_nCurrentView_00492fa8 == 4) {
        switch (mode) {
        case 1:
            g_nGunDisplayIndex_005c8dc0 = 1;
            g_nTargetCameraOverlayMode_005c8db8 = 2;
            break;
        case 2:
            g_nGunDisplayIndex_005c8dc0 = 1;
            g_nTargetCameraOverlayMode_005c8db8 = 1;
            break;
        case 3:
            g_nGunDisplayIndex_005c8dc0 = 0;
            g_nTargetCameraOverlayMode_005c8db8 = 0;
            break;
        }
        ResetTargetCameraView();
        ResetCockpitPaletteEntries();
        clear_head_up_display();
        if (g_nCockpitDisplayMode_0049d71c == 0)
            FinalizeTargetCameraViewHook();
    }

    g_bSpaceViewBufferEnabled_0049d7a4 = 1;
    SetViewportRect(&g_stViewBuffer_005d2b00, 0, 0,
                    (unsigned short)(g_nScreenWidth_0049d4d8 - 1),
                    (unsigned short)(g_nScreenHeight_0049d4dc - 1));
    initialize_view_buffer();
    g_stScreenViewport_005d21a0 = savedScreen;
}

/* Function start: 0x457434 */
void InitializeConstellationObject(
    const ConstellationObjectDefinition *definition, short object)
{
    char packetName[13] = "planets.v0";
    FixedVector position;
    char campaignNumber[4];

    strcat(packetName,
           _itoa((int)g_nSelectedCampaignSlot_005d3bf2,
                 campaignNumber, 10));
    g_aeObjectClass_00495328[object] = OBJECT_CLASS_PLANET;
    init_ijk(69);
    alter_yaw((short)-definition->yaw, 69);
    alter_pitch((short)-definition->pitch, 69);
    alter_roll(definition->roll, 69);
    ScaleFixedVector(&g_aShipForwardVector_00494208[69],
                     0x753000, &position);
    g_aShipPosition_00494550[object] = position;
    g_asObjectScreenAngle_004936b8[object] = 0;
    g_asObjectViewFrame_00493508[object] =
        g_asObjectScreenAngle_004936b8[object];
    g_asObjectType_00495298[object] =
        g_asObjectViewFrame_00493508[object];
    g_asObjectScreenScale_00493a58[object] = 0xff;
    g_apObjectShape_00493868[object] =
        FetchDiskPacketRetrying(packetName,
                                (short)(definition->shapePacket + 1), 0);
}

/* Function start: 0x457587 */
void FreeConstellationObject(short object)
{
    FreePacketAndClear(&g_apObjectShape_00493868[object], 0);
    remove_object(object);
}

/* Function start: 0x4575B4 */
void init_constellation(short scene)
{
    short slot;
    short sceneIndex;
    short object;

    if (g_pConstellationShape_005d2c4c == 0) {
        g_pConstellationShape_005d2c4c =
            FetchDiskPacketRetrying("planets.v00", 0, 0);
    }
    sceneIndex = (short)(scene - 1);
    if (g_nAvailableGameMemory_005c8de0 < 6500)
        return;
    if (sceneIndex >= 0) {
        for (slot = 0; slot < 4; slot++) {
            if (((ConstellationObjectDefinition (*)[4])
                     g_pStartupStarPacket_005d212c)
                    [sceneIndex][slot].shapePacket != -1) {
                object = find_vacant_3d_object();
                if (object != -1) {
                    InitializeConstellationObject(
                        &((ConstellationObjectDefinition (*)[4])
                              g_pStartupStarPacket_005d212c)
                             [sceneIndex][slot],
                        object);
                }
                g_asConstellationObjectIndices_0049c8e0[slot] = object;
            } else {
                g_asConstellationObjectIndices_0049c8e0[slot] = -1;
            }
        }
    }
}

/* Function start: 0x4576AB */
void free_constellation(void)
{
    short object;
    short slot;

    FreePacketAndClear(&g_pConstellationShape_005d2c4c, 0);
    for (slot = 0; slot < 4; slot++) {
        object = g_asConstellationObjectIndices_0049c8e0[slot];
        if (object != -1) {
            FreeConstellationObject(object);
            g_asConstellationObjectIndices_0049c8e0[slot] = -1;
        }
    }
}

/* Function start: 0x457720 */
void init_vdus(void)
{
    g_stRightVduViewport_005d2b20 = g_stScreenViewport_005d21a0;
    g_stLeftVduViewport_005d2180 = g_stRightVduViewport_005d2b20;

    InitializeTextContextFromFont(
        &g_stLeftVduTextContext_005d2ae0, 2,
        g_ucPrimaryTextColour_0049cb64,
        (signed char)g_cSecondaryViewBufferColour_0049cb4c);
    g_stLeftVduTextContext_005d2ae0.viewport =
        &g_stLeftVduViewport_005d2180;
    g_stLeftVduTextContext_005d2ae0.text =
        g_szDefaultTextBuffer_005d2b80;
    set_mode(0, 1);
    g_anLastDrawnVduMode_004934d0[0] = 0;

    InitializeTextContextFromFont(
        &g_stRightVduTextContext_005d2ce0, 2,
        g_ucPrimaryTextColour_0049cb64,
        (signed char)g_cSecondaryViewBufferColour_0049cb4c);
    g_stRightVduTextContext_005d2ce0.viewport =
        &g_stRightVduViewport_005d2b20;
    g_stRightVduTextContext_005d2ce0.text =
        g_szDefaultTextBuffer_005d2b80;
    set_mode(1, 5);
    g_anLastDrawnVduMode_004934d0[1] = 0;
}

/* Function start: 0x4577D7 */
void InitializeCockpitResources(void)
{
    CockpitResourceLayout layout;
    short frame;
    short maximumSize;
    short gunDisplay;
    char resourceNumber[4];
    int packetSize;
    int index;

    g_bCockpitResourcesActive_0049c8e8 = 1;
    strcpy(g_szCockpitResourceFilename_005d1030, "pcship.v");
    _itoa((int)g_aObjectTypeData_00496d30[
              g_acObjectType_00493980[0]].resourceType,
          resourceNumber, 10);
    if (strlen(resourceNumber) == 1)
        strcat(g_szCockpitResourceFilename_005d1030, "0");
    strcat(g_szCockpitResourceFilename_005d1030, resourceNumber);
    ClearHudGunReadouts();
    reset_cockpit();
    free_view_buffer();
    if (g_nResourcePaletteMode_005c57e6 == 0) {
        clear_cockpit_damage();
        LoadShapeSet(g_aCockpitShipShapeResources_0049c858, 4,
                     g_szCockpitResourceFilename_005d1030);
        LoadShapeSet(g_aCockpitViewShapeResources_0049c8a8, 4,
                     g_szCockpitResourceFilename_005d1030);
    }
    LoadPacketIntoBuffer(g_szCockpitResourceFilename_005d1030, 0x0d,
                         &layout, 0);
    g_stHudMessageOrigin_0049ae90 = layout.hudMessageOrigin;
    for (index = 0; index < 4; index++)
        g_aCockpitDamagePositions_0049ae98[index] =
            layout.damagePositions[index];
    for (index = 0; index < 7; index++)
        g_asCockpitLightX_0049aea8[index] = layout.lightX[index];
    for (index = 0; index < 7; index++)
        g_asCockpitLightY_0049aeb8[index] = layout.lightY[index];
    for (index = 0; index < 7; index++)
        g_acCockpitLightOffFrame_0049aec8[index] =
            layout.lightOffFrame[index];
    for (index = 0; index < 7; index++)
        g_acCockpitLightOnFrame_0049aed0[index] =
            layout.lightOnFrame[index];
    for (index = 0; index < 8; index++)
        g_aCockpitBarDefinitions_0049aed8[index] = layout.bars[index];
    g_aCockpitReadoutOrigins_0049af58[0] = layout.readoutOrigins[0];
    g_aCockpitReadoutOrigins_0049af58[1] = layout.readoutOrigins[1];
    g_aCockpitReadoutOrigins_0049af58[2] = layout.readoutOrigins[2];
    g_aCockpitReadoutOrigins_0049af58[3] = layout.readoutOrigins[3];
    g_stLeftVduBounds_0049af68 = layout.leftVduBounds;
    g_stRightVduBounds_0049af70 = layout.rightVduBounds;
    g_stCockpitScannerGeometry_0049af78 = layout.scanner;
    g_stPilotHandBounds_0049af88 = layout.pilotHandBounds;
    g_stPilotHandOrigin_0049af90 = layout.pilotHandOrigin;
    g_cHazardBaseTravelTime_0049af94 = layout.field_f6;
    DAT_0049af98 = layout.field_f7;
    for (index = 0; index < 16; index++)
        g_aDefaultWeaponDisplayPositions_0049afa0[index] =
            layout.weaponDisplayPositions[index];
    g_bTargetLockDisplayEnabled_0049afe0 =
        layout.targetLockDisplayEnabled;

    if (g_pCockpitBackgroundPacket_0049a5f0 == 0) {
        maximumSize = 0;
        gunDisplay = 4;
        while (gunDisplay <= 7) {
            packetSize = (int)GetNamedPacketSize(
                g_szCockpitResourceFilename_005d1030, gunDisplay);
            maximumSize = MaxShort(maximumSize, (short)packetSize);
            gunDisplay++;
        }
        g_nCockpitBackgroundBytes_0049c720 = maximumSize;
        g_pCockpitBackgroundPacket_0049a5f0 =
            AllocateTaggedMemory(
                (int)g_nCockpitBackgroundBytes_0049c720, 0x40);
        if (g_pCockpitBackgroundPacket_0049a5f0 == 0)
            ReportFatalErrorCode("019");
    }

    if (HasShipCockpitGunDisplay(0) != 0) {
        g_nGunDisplayCount_005c8dbc = CountShipCockpitGunDisplays(0);
        if (g_nGunDisplayCount_005c8dbc == 3)
            g_nGunDisplayCount_005c8dbc--;
        gunDisplay = 0;
        while (gunDisplay < g_nGunDisplayCount_005c8dbc) {
            g_apCockpitVduOverlayShapes_0049a5f8[gunDisplay] =
                FetchDiskPacketRetrying(
                    g_szCockpitResourceFilename_005d1030,
                    (short)(gunDisplay + 0x0e), 0);
            gunDisplay++;
        }
        LoadPacketIntoBuffer(
            g_szCockpitResourceFilename_005d1030,
            (short)(g_nGunDisplayCount_005c8dbc + 0x0e),
            g_abGunDisplayConfiguration_0049d340, 0);
        g_nGunDisplayEnergyPercent_005c8d4e = 100;
    }

    g_stCockpitBarViewport_005d21e0 = g_stScreenViewport_005d21a0;
    init_vdus();
    InitializeTextContextFromFont(
        &g_stCockpitTextContext_005d2d00, 2,
        g_ucPrimaryTextColour_0049cb64,
        (signed char)g_cSecondaryViewBufferColour_0049cb4c);
    g_stCockpitTextContext_005d2d00.viewport =
        &g_stScreenViewport_005d21a0;
    SetTextContext(&g_stCockpitTextContext_005d2d00);
    SetTextCursor(
        (unsigned short)g_aCockpitReadoutOrigins_0049af58[0].x,
        (unsigned short)g_aCockpitReadoutOrigins_0049af58[0].y);
    InitializeCockpitReadout(4, &g_stCockpitTextContext_005d2d00);
    SetTextCursor(
        (unsigned short)g_aCockpitReadoutOrigins_0049af58[1].x,
        (unsigned short)g_aCockpitReadoutOrigins_0049af58[1].y);
    InitializeCockpitReadout(5, &g_stCockpitTextContext_005d2d00);
    SetTextCursor(
        (unsigned short)g_aCockpitReadoutOrigins_0049af58[2].x,
        (unsigned short)g_aCockpitReadoutOrigins_0049af58[2].y);
    InitializeCockpitReadout(2, &g_stCockpitTextContext_005d2d00);
    SetTextCursor(
        (unsigned short)g_aCockpitReadoutOrigins_0049af58[3].x,
        (unsigned short)g_aCockpitReadoutOrigins_0049af58[3].y);
    InitializeCockpitReadout(3, &g_stCockpitTextContext_005d2d00);

    memcpy(&g_stLeftVduViewport_005d2180.left,
           &g_stLeftVduBounds_0049af68, sizeof(ShortRect));
    memcpy(&g_stRightVduViewport_005d2b20.left,
           &g_stRightVduBounds_0049af70, sizeof(ShortRect));
    if (g_pPilotHandAnimationShape_005d2c64 != 0) {
        g_stCockpitViewport_005d2160 = g_stScreenViewport_005d21a0;
        memcpy(&g_stCockpitViewport_005d2160.left,
               &g_stPilotHandBounds_0049af88, sizeof(ShortRect));
        SetViewportRect(
            &g_stPilotHandViewport_005d2c70, 0, 0,
            (unsigned short)(g_stPilotHandBounds_0049af88.right -
                             g_stPilotHandBounds_0049af88.left),
            (unsigned short)(g_stPilotHandBounds_0049af88.bottom -
                             g_stPilotHandBounds_0049af88.top));
        g_stPilotHandBackgroundViewport_005d2b40 =
            g_stPilotHandViewport_005d2c70;
        AllocateViewport(&g_stPilotHandViewport_005d2c70,
                         g_cSecondaryViewBufferColour_0049cb4c, 0);
        AllocateViewport(&g_stPilotHandBackgroundViewport_005d2b40,
                         g_cSecondaryViewBufferColour_0049cb4c, 0);
    }

    g_nHudMessageBackgroundDepth_0049b294 = 0;
    LoadShapeSet(g_aCockpitCommonShapeResources_0049c820, 0,
                 g_szCockpitResourceFilename_005d1030);
    g_pScannerMarkerBackground_005d1c2c = AllocateTaggedMemory(
        (unsigned int)MeasureShapeFrameStorage(
            g_pCockpitHudShape_005d21f4, 2),
        0x40);
    g_pReleaseWeaponDisplayBackground_0049b05c = 0;
    g_pDamageDisplayBackground_00490060 = 0;
    g_pCockpitExplosionBackground_0049b040 = 0;
    if (g_pCockpitWeaponShape_005d2b54 != 0) {
        maximumSize = (short)MeasureShapeFrameStorage(
            g_pCockpitWeaponShape_005d2b54, 1);
        g_pReleaseWeaponDisplayBackground_0049b05c =
            AllocateTaggedMemory((int)maximumSize, 0);
        maximumSize = 0;
        frame = 0x20;
        while (frame < 0x29) {
            maximumSize = MaxShort(
                maximumSize,
                (short)MeasureShapeFrameStorage(
                    g_pCockpitWeaponShape_005d2b54, frame));
            frame++;
        }
        if (maximumSize != 0)
            g_pDamageDisplayBackground_00490060 =
                AllocateTaggedMemory((int)maximumSize, 0);
    }
    if (g_bTargetLockDisplayEnabled_0049afe0 != 0) {
        g_pCockpitHudBackground_0049b044 = AllocateTaggedMemory(
            (unsigned int)MeasureShapeFrameStorage(
                g_pCockpitHudShape_005d21f4, 5),
            0);
    }
    if (g_pCockpitExplosionShape_0049b048 != 0) {
        maximumSize = 0;
        frame = 0;
        while (frame < 8) {
            maximumSize = MaxShort(
                maximumSize,
                (short)MeasureShapeFrameStorage(
                    g_pCockpitExplosionShape_0049b048, frame));
            frame++;
        }
        if (maximumSize != 0)
            g_pCockpitExplosionBackground_0049b040 =
                AllocateTaggedMemory((int)maximumSize, 0);
    }
    g_pHudMessageFrameShape_0049b288 =
        FetchDiskPacketRetrying("message.v00", 0, 0);
    g_pHudMessageBackground_0049b28c = AllocateTaggedMemory(
        (unsigned int)MeasureShapeFrameStorage(
            g_pHudMessageFrameShape_0049b288, 0),
        0);
    ResetScannerContacts();
    g_bRadioSilence_0049b780 = 0;
    g_nCockpitExplosionFrame_0049b04c = 8;
    for (g_nObjectType62Index_00492d64 = 0;
         g_nObjectType62Index_00492d64 < 0x40;
         g_nObjectType62Index_00492d64++) {
        if (g_aObjectTypeData_00496d30[
                g_nObjectType62Index_00492d64].field_18 == 0x3e)
            break;
    }
    for (g_nObjectType63Index_00492d68 = 0;
         g_nObjectType63Index_00492d68 < 0x40;
         g_nObjectType63Index_00492d68++) {
        if (g_aObjectTypeData_00496d30[
                g_nObjectType63Index_00492d68].field_18 == 0x3f)
            break;
    }
    if (g_bHighMemoryResourcesEnabled_005c80e4 != 0 &&
        HasShipCockpitGunDisplay(0) != 0) {
        g_aObjectTypeData_00496d30[
            g_nObjectType62Index_00492d64].shapeSet =
            LoadNamedPacket("objects.vga", 0x15, 0, 4, 0, 1);
        g_aObjectTypeData_00496d30[
            g_nObjectType63Index_00492d68].shapeSet =
            LoadNamedPacket("objects.vga", 0x16, 0, 4, 0, 1);
        if (g_aObjectTypeData_00496d30[
                g_nObjectType62Index_00492d64].shapeSet == 0 ||
            g_aObjectTypeData_00496d30[
                g_nObjectType63Index_00492d68].shapeSet == 0) {
            FreePacketAndClear(
                &g_aObjectTypeData_00496d30[
                    g_nObjectType62Index_00492d64].shapeSet,
                4);
            FreePacketAndClear(
                &g_aObjectTypeData_00496d30[
                    g_nObjectType63Index_00492d68].shapeSet,
                4);
        }
    }
    gunDisplay = 0;
    while (gunDisplay < 50) {
        g_apCommunicationTextPackets_005d17c0[gunDisplay] = 0;
        gunDisplay++;
    }
}

/* Function start: 0x458196 */
void free_cockpit(void)
{
    char shipTypeText[4];
    short packet;

    if (g_bCockpitResourcesActive_0049c8e8 == 0)
        return;
    g_bCockpitResourcesActive_0049c8e8 = 0;
    free_view_buffer();
    if (g_pCockpitBackgroundPacket_0049a5f0 != 0) {
        ReleasePacketHandle(g_pCockpitBackgroundPacket_0049a5f0);
        g_pCockpitBackgroundPacket_0049a5f0 = 0;
    }
    FreePacketAndClear(&g_apCockpitVduOverlayShapes_0049a5f8[0], 0);
    FreePacketAndClear(&g_apCockpitVduOverlayShapes_0049a5f8[1], 0);
    FreePacketAndClear(&g_pHudMessageFrameShape_0049b288, 0);
    FreePacketAndClear(&g_pHudMessageBackground_0049b28c, 0);
    if (g_nObjectType62Index_00492d64 != -1)
        FreePacketAndClear(
            &g_aObjectTypeData_00496d30[
                g_nObjectType62Index_00492d64].shapeSet,
            4);
    if (g_nObjectType63Index_00492d68 != -1)
        FreePacketAndClear(
            &g_aObjectTypeData_00496d30[
                g_nObjectType63Index_00492d68].shapeSet,
            4);
    ReleaseTextFont(2);
    if (g_pPilotHandAnimationShape_005d2c64 != 0) {
        free_viewport(&g_stPilotHandViewport_005d2c70);
        free_viewport(&g_stPilotHandBackgroundViewport_005d2b40);
    }
    strcpy(g_szCockpitResourceFilename_005d1030, "pcship.v");
    _itoa((int)g_aObjectTypeData_00496d30[
              g_acObjectType_00493980[0]].resourceType,
          shipTypeText, 10);
    if (strlen(shipTypeText) == 1)
        strcat(g_szCockpitResourceFilename_005d1030, "0");
    strcat(g_szCockpitResourceFilename_005d1030, shipTypeText);
    if (g_nResourcePaletteMode_005c57e6 == 0) {
        FreeShapeSet(g_aCockpitShipShapeResources_0049c858, 4);
        FreeShapeSet(g_aCockpitViewShapeResources_0049c8a8, 4);
    }
    FreeShapeSet(g_aCockpitCommonShapeResources_0049c820, 0);
    FreePacketAndClear(&g_pPendingCockpitDamageShape_0049b03c, 0);
    FreePacketAndClear(&g_pReleaseWeaponDisplayBackground_0049b05c, 0);
    FreePacketAndClear(&g_pCockpitExplosionBackground_0049b040, 0);
    FreePacketAndClear(&g_pCockpitHudBackground_0049b044, 0);
    FreePacketAndClear(&g_pDamageDisplayBackground_00490060, 0);
    FreeCommDisplayResources();
    FreePacketAndClear(&g_pCommPortraitResource_0049b788, 4);
    FreePacketAndClear(&g_pCommVduFrameResource_0049b78c, 4);
    g_nLoadedCommPortraitPilot_004931c4 = -1;
    if (g_pScannerMarkerBackground_005d1c2c != 0) {
        ReleasePacketHandle(g_pScannerMarkerBackground_005d1c2c);
        g_pScannerMarkerBackground_005d1c2c = 0;
    }
    packet = 0;
    while (packet < 50) {
        if (g_apCommunicationTextPackets_005d17c0[packet] != 0) {
            ReleasePacketHandle(
                g_apCommunicationTextPackets_005d17c0[packet]);
            g_apCommunicationTextPackets_005d17c0[packet] = 0;
        }
        packet++;
    }
}

/* Function start: 0x458467 */
void init_3Space_objects(short scene)
{
    short slot;

    if (g_b3SpaceObjectsActive_0049c8ec == 1)
        return;
    g_b3SpaceObjectsActive_0049c8ec = 1;
    g_cScreenViewportMode_005c82a6 = -1;
    remove_all_3d_objects();
    g_nSpaceFrame_00493134 =
        g_nRenderedSpaceFrame_00493138 =
            (short)(g_nNextAfterburnerSfxFrame_005d3820 =
                g_nPassingShipSoundCooldown_005d156c = 0);
    g_bMissileCameraEnabled_00493504 = 0;
    g_nExternalViewShip_00493468 = -1;
    g_nPlayerCollisionObject_00493480 = -1;
    for (slot = 0; slot <= 4; slot++)
        g_aObjectResourceSlots_00493398[slot].resourceType = -1;
    init_constellation(scene);
    load_common_3Space_objects();
}

/* Function start: 0x458532 */
void load_common_3Space_objects(void)
{
    FreeShapeSet(g_aCommon3SpaceResources_0049c728, 0);
    LoadShapeSet(g_aCommon3SpaceResources_0049c728, 0, "objects.vga");
    g_aObjectTypeData_00496d30[34].shapeSet =
        g_aObjectTypeData_00496d30[35].shapeSet;
    g_aObjectTypeData_00496d30[33].shapeSet =
        g_aObjectTypeData_00496d30[34].shapeSet;
    g_aObjectTypeData_00496d30[30].shapeSet =
        g_aObjectTypeData_00496d30[33].shapeSet;
    g_aObjectTypeData_00496d30[29].shapeSet =
        g_aObjectTypeData_00496d30[30].shapeSet;
    g_aObjectTypeData_00496d30[12].shapeSet =
        g_aObjectTypeData_00496d30[7].shapeSet;
    g_aObjectTypeData_00496d30[12].animation =
        g_aObjectTypeData_00496d30[7].animation;
    free_ship(4);
    load_ship(1, 0x10, OBJECT_CLASS_MISSILE, 4);
    g_aObjectTypeData_00496d30[19].shapeSet =
        g_aObjectTypeData_00496d30[16].shapeSet;
    g_aObjectTypeData_00496d30[15].shapeSet =
        g_aObjectTypeData_00496d30[19].shapeSet;
    g_aObjectTypeData_00496d30[18].shapeSet =
        g_aObjectTypeData_00496d30[15].shapeSet;
    g_aObjectTypeData_00496d30[17].shapeSet =
        g_aObjectTypeData_00496d30[18].shapeSet;
    g_aObjectTypeData_00496d30[19].animation =
        g_aObjectTypeData_00496d30[16].animation;
    g_aObjectTypeData_00496d30[15].animation =
        g_aObjectTypeData_00496d30[19].animation;
    g_aObjectTypeData_00496d30[18].animation =
        g_aObjectTypeData_00496d30[15].animation;
    g_aObjectTypeData_00496d30[17].animation =
        g_aObjectTypeData_00496d30[18].animation;
    if (g_bHighMemoryResourcesEnabled_005c80e4 != 0) {
        if (g_nResourcePaletteMode_005c57e6 == 0) {
            g_pGenericMissileShape_0049c8f0 =
                FetchDiskPacketRetrying("missile.v00", 0, 4);
        }
        if (g_pGenericMissileShape_0049c8f0 != 0) {
            g_pGenericMissileExhaustShape_0049c8f4 =
                FetchDiskPacketRetrying("missile.v00", 1, 0);
        }
    }
}

/* Function start: 0x45865D */
void remove_all_3d_objects(void)
{
    short i;

    for (i = 0; i < 70; i++)
        remove_object(i);
}

/* Function start: 0x458698 */
void free_3Space(void)
{
    if (g_b3SpaceObjectsActive_0049c8ec == 0)
        return;
    g_b3SpaceObjectsActive_0049c8ec = 0;
    free_view_buffer();
    g_bSpaceViewBufferEnabled_0049d7a4 = 0;
    free_constellation();
    remove_all_hazards();
    free_3Space_objects();
    FreeShapeSet(g_aMissionResourceDescriptors_0049c798, 0);
    if (g_pNavLocationText_0049bc54 != 0) {
        ReleasePacketHandle(g_pNavLocationText_0049bc54);
        g_pNavLocationText_0049bc54 = 0;
    }
}

/* Function start: 0x458716 */
void free_3Space_objects(void)
{
    FreeShapeSet(g_aCommon3SpaceResources_0049c728, 0);
    free_ship(4);
    g_aObjectTypeData_00496d30[WC2_OBJECT_TYPE_PILUM_FF].shapeSet =
        g_aObjectTypeData_00496d30[WC2_OBJECT_TYPE_SPICULUM_IR].shapeSet =
            g_aObjectTypeData_00496d30[WC2_OBJECT_TYPE_DART_DF].shapeSet = 0;
    g_aObjectTypeData_00496d30[WC2_OBJECT_TYPE_PILUM_FF].animation =
        g_aObjectTypeData_00496d30[WC2_OBJECT_TYPE_SPICULUM_IR].animation =
            g_aObjectTypeData_00496d30[WC2_OBJECT_TYPE_DART_DF].animation = 0;
    g_aObjectTypeData_00496d30[WC2_OBJECT_TYPE_GIRDER_CHUNK].shapeSet =
        g_aObjectTypeData_00496d30[WC2_OBJECT_TYPE_SHIP_TUBING].shapeSet =
            g_aObjectTypeData_00496d30[
                WC2_OBJECT_TYPE_BURNING_DEBRIS].shapeSet =
                g_aObjectTypeData_00496d30[WC2_OBJECT_TYPE_O_RING].shapeSet =
                    g_aObjectTypeData_00496d30[
                        WC2_OBJECT_TYPE_PIPE].shapeSet;
    FreePacketAndClear(
        &g_aObjectTypeData_00496d30[WC2_OBJECT_TYPE_EJECTED_PILOT].shapeSet,
        0);
    if (g_nResourcePaletteMode_005c57e6 == 0)
        FreePacketAndClear(&g_pGenericMissileShape_0049c8f0, 4);
    FreePacketAndClear(&g_pGenericMissileExhaustShape_0049c8f4, 0);
}

/* Function start: 0x4587E0 */
void init_inflight_music(void)
{
    g_nInFlightMusicActive_0049bf08 = 1;
    g_nInitialFlightMusicPending_0049bf00 = 1;
    g_nCombatMusicActive_0049bf04 = 0;

}

/* Function start: 0x458806 */
void free_inflight_music(void)
{
    StopMusicUnlessSuppressed();
    g_nInFlightMusicActive_0049bf08 = 0;
    if (g_nLimitedMusicBufferAState_0049bea4 != -1) {
        g_aMusicResources_005d13e0[
            g_nLimitedMusicBufferAState_0049bea4].packet = 0;
        g_nLimitedMusicBufferAState_0049bea4 = -1;
    }
    if (g_nLimitedMusicBufferBState_0049beaa != -1) {
        g_aMusicResources_005d13e0[
            g_nLimitedMusicBufferBState_0049beaa].packet = 0;
        g_nLimitedMusicBufferBState_0049beaa = -1;
    }
}

/* Function start: 0x45887B */
void PreloadMusicTrack(int track)
{
    if (g_nMusicDriverMode_0049be8c == 1 &&
        g_aMusicResources_005d13e0[track].packet == 0) {
        g_aMusicResources_005d13e0[track].packet =
            FetchDiskPacketRetrying("music.r00", (short)track, 0);
    }
    if (g_nMusicDriverMode_0049be8c == 2 &&
        g_aMusicResources_005d13e0[track].packet == 0) {
        g_aMusicResources_005d13e0[track].packet =
            FetchDiskPacketRetrying("music.a00", (short)track, 0);
    }
}

/* Function start: 0x45890E */
void ReleaseMusicTrack(int track)
{
    if (g_nMusicDriverMode_0049be8c == 1 ||
        g_nMusicDriverMode_0049be8c == 2) {
        if (g_nMemoryConfiguration_005c8dc8 == 1) {
            if (g_aMusicResources_005d13e0[track].packet ==
                g_pLimitedMusicBufferA_0049bea0) {
                g_nLimitedMusicBufferAState_0049bea4 = -1;
            } else if (g_aMusicResources_005d13e0[track].packet ==
                       g_pLimitedMusicBufferB_0049bea6) {
                g_nLimitedMusicBufferBState_0049beaa = -1;
            } else {
                FreePacketAndClear(
                    &g_aMusicResources_005d13e0[track].packet, 0);
            }
        } else {
            FreePacketAndClear(
                &g_aMusicResources_005d13e0[track].packet, 0);
        }
    }
}

/* Function start: 0x458A90 */
signed char DecodeSceneStructChunk(unsigned char **cursor,
                                   SceneResourceTable **resource)
{
    short pointerSize;
    unsigned int chunkSize;
    unsigned char *data;
    void **entries;
    short index;
    unsigned short recordSize;
    unsigned short count;

    data = *cursor;
    data += 4;
    chunkSize = *(unsigned int *)data;
    data += 4;
    SwapSceneChunkSizeEndian((int *)&chunkSize);
    recordSize = *(unsigned short *)data;
    data += 2;
    count = (unsigned short)((chunkSize - 2) / recordSize);
    (*resource)->type = 0;
    (*resource)->count = count;
    pointerSize = WC2_SCENE_POINTER_SIZE;
    entries = AllocateScenePointerTable(
        count, pointerSize, 0, "Cannot Allocate STRC stuff");
    (*resource)->data = entries;
    index = 1;
    while ((unsigned short)index <= count) {
        *entries = IdentityHandle(data);
        entries++;
        data += recordSize;
        index++;
    }
    *cursor += ((chunkSize + 1) & ~1) + 8;
    return 1;
}

/* Function start: 0x458B8C */
signed char DecodeSceneOffsetChunk(unsigned char **cursor,
                                   SceneResourceTable **resource)
{
    unsigned int chunkSize;
    unsigned char *data;
    void **entries;
    unsigned short count;
    short index;
    void *resolved;
    int offset;

    data = *cursor;
    data += 4;
    chunkSize = *(unsigned int *)data;
    data += 4;
    SwapSceneChunkSizeEndian((int *)&chunkSize);
    count = *(unsigned short *)data;
    data += 2;
    (*resource)->type = 1;
    (*resource)->count = count;
    entries = AllocateScenePointerTable(
        count, WC2_SCENE_POINTER_SIZE, 0, "Cannot Alloc OFST stuff");
    (*resource)->data = entries;
    index = 1;
    while (index <= (short)count) {
        offset = *(int *)data;
        if (offset != -1)
            resolved = data + offset;
        else
            resolved = 0;
        data += 4;
        *entries = resolved;
        entries++;
        index++;
    }
    *cursor += ((chunkSize + 1) & ~1) + 8;
    return 1;
}

/* Function start: 0x458C81 */
signed char DecodeSceneSymbolChunk(unsigned char **cursor,
                                   SceneResourceTable **resource)
{
    short pointerSize;
    unsigned int chunkSize;
    unsigned char *nextString;
    unsigned char *data;
    void **entries;
    unsigned char *start;
    short count;
    short remaining;

    data = *cursor;
    start = data;
    data += 4;
    chunkSize = *(unsigned int *)data;
    data += 4;
    SwapSceneChunkSizeEndian((int *)&chunkSize);
    start = data;
    count = 0;
    pointerSize = WC2_SCENE_POINTER_SIZE;
    do {
        nextString = (unsigned char *)DosStrchr((char *)data, 0) + 1;
        count++;
        data = nextString;
    } while (nextString - start < (int)chunkSize);
    data = start;
    (*resource)->type = 2;
    (*resource)->count = count;
    entries = AllocateScenePointerTable(
        count, pointerSize, 0, "Cannot Alloc SYMB stuff");
    (*resource)->data = entries;
    data = start;
    remaining = count;
    do {
        *entries = IdentityHandle(data);
        entries++;
        data = (unsigned char *)DosStrchr((char *)data, 0) + 1;
        remaining--;
    } while (remaining != 0);
    *cursor += ((chunkSize + 1) & ~1) + 8;
    return 1;
}

/* Function start: 0x458D94 */
signed char DecodeSceneFileChunk(unsigned char **cursor,
                                 SceneResourceTable **resource)
{
    unsigned int chunkSize;
    unsigned char *data;

    data = *cursor;
    data += 4;
    chunkSize = *(unsigned int *)data;
    data += 4;
    SwapSceneChunkSizeEndian((int *)&chunkSize);
    (*resource)->count = *(unsigned short *)data;
    data += 2;
    (*resource)->data = data;
    (*resource)->type = 3;
    *cursor += ((chunkSize + 1) & ~1) + 8;
    return 1;
}

/* Function start: 0x45F7EC */
void UpdateShipTurretGuns(short ship)
{
    short projectileSpeed;
    short target;
    short gunSide;
    short turretCount;
    int turret;
    FixedVector gunRight;
    FixedVector gunUp;
    FixedVector gunForward;
    short candidate;
    int facing;
    FixedVector facingRight;
    ShortVector gunOffset;
    FixedVector gunPosition;
    short projectileType;
    FixedVector aimJitter;
    short shot;
    short projectile;
    ObjectTypeData *projectileData;
    FixedVector aimPoint;
    short leadDistance;

    projectileSpeed = 10;
    target = -1;
    gunSide = 0;
    turretCount = CountShipCockpitGunDisplays(ship);
    if (turretCount == 0)
        return;

    for (turret = 0; turret < turretCount; turret++) {
        if (ship == 0 && g_nCurrentView_00492fa8 == 4 &&
            turret == g_nTargetCameraOverlayMode_005c8db8)
            continue;
        if (g_asShipSide_004955d0[projectile] == SIDE_IMPERIAL &&
            (unsigned short)RandomInRange(0, 10) < 8)
            continue;
        if (g_asShipSide_004955d0[projectile] == SIDE_KILRATHI &&
            (unsigned short)RandomInRange(0, 10) < 5)
            continue;
        if (g_asGunCooldown_005c8d70[ship * 3 + turret] > 0)
            continue;

        switch (turret) {
        case 0:
            gunRight = g_aShipRightVector_00493b78[ship];
            negate_vector(&gunRight);
            gunUp = g_aShipUpVector_00493ec0[ship];
            gunForward = g_aShipForwardVector_00494208[ship];
            negate_vector(&gunForward);
            gunSide = 0;
            break;
        case 1:
            gunRight = g_aShipRightVector_00493b78[ship];
            gunUp = g_aShipUpVector_00493ec0[ship];
            gunForward = g_aShipForwardVector_00494208[ship];
            negate_vector(&gunForward);
            gunSide = 1;
            break;
        case 2:
            gunRight = g_aShipRightVector_00493b78[ship];
            negate_vector(&gunRight);
            gunUp = g_aShipUpVector_00493ec0[ship];
            gunForward = g_aShipForwardVector_00494208[ship];
            gunSide = 1;
            break;
        }

        NormalizeFixedVector(&gunRight);
        NormalizeFixedVector(&gunUp);
        NormalizeFixedVector(&gunForward);
        for (candidate = 0; candidate < 10; candidate++) {
            if (g_aeObjectClass_00495328[candidate] ==
                    OBJECT_CLASS_SHIP &&
                g_asShipSide_004955d0[candidate] !=
                    g_asShipSide_004955d0[ship] &&
                g_anShipCloakState_00496020[candidate] != 1) {
                g_nTargetRange_0049319c = (short)(
                    distance_from_point(
                        ship, &g_aShipPosition_00494550[candidate]) -
                    g_asObjectCollisionRadius_004950e8[ship]);
                g_vNormalizedToTarget_005d3bd0 =
                    g_vToTarget_00493188;
                NormalizeFixedVector(&g_vNormalizedToTarget_005d3bd0);
                if (turret == 2) {
                    facing = dot_product(
                        &g_vNormalizedToTarget_005d3bd0,
                        &g_aShipRightVector_00493b78[ship]);
                } else if (turret == 1) {
                    facingRight = g_aShipRightVector_00493b78[ship];
                    negate_vector(&facingRight);
                    facing = dot_product(
                        &g_vNormalizedToTarget_005d3bd0, &facingRight);
                } else {
                    facing = dot_product(
                        &g_vNormalizedToTarget_005d3bd0, &gunForward);
                }
                g_nFacingToTarget_00493194 =
                    (short)(((short)facing * 100) >> 8);
                if (g_nFacingToTarget_00493194 > 85 &&
                    g_nTargetRange_0049319c < 4000)
                    target = candidate;
            }
        }
        if (target == -1)
            continue;

        gunOffset = g_aTargetCameraEyeOffsets_0049d430[turret];
        gunPosition.x =
            g_aShipUpVector_00493ec0[ship].x * gunOffset.y +
            g_aShipForwardVector_00494208[ship].x * gunOffset.z +
            g_aShipRightVector_00493b78[ship].x * gunOffset.x +
            g_aShipPosition_00494550[ship].x;
        gunPosition.y =
            g_aShipUpVector_00493ec0[ship].y * gunOffset.y +
            g_aShipForwardVector_00494208[ship].y * gunOffset.z +
            g_aShipRightVector_00493b78[ship].y * gunOffset.x +
            g_aShipPosition_00494550[ship].y;
        gunPosition.z =
            g_aShipUpVector_00493ec0[ship].z * gunOffset.y +
            g_aShipRightVector_00493b78[ship].z * gunOffset.x +
            g_aShipForwardVector_00494208[ship].z * gunOffset.z +
            g_aShipPosition_00494550[ship].z;
        projectileType = 8;
        aimJitter.x =
            ((unsigned short)RandomInRange(0, 500) - 250) << 8;
        aimJitter.y =
            ((unsigned short)RandomInRange(0, 500) - 250) << 8;
        aimJitter.z =
            ((unsigned short)RandomInRange(0, 500) - 250) << 8;

        for (shot = 0; shot < 2; shot++) {
            projectile = new_object(projectileType, ship);
            if (projectile != -1) {
                projectileData =
                    &g_aObjectTypeData_00496d30[projectileType];
                g_aShipRightVector_00493b78[projectile] = gunRight;
                g_aShipUpVector_00493ec0[projectile] = gunUp;
                g_aShipForwardVector_00494208[projectile] = gunForward;
                g_asObjectDamage_00495178[projectile] =
                    projectileData->damageCapacity;
                projectileSpeed =
                    g_aObjectTypeData_00496d30[
                        g_acObjectType_00493980[projectile]]
                        .maximumVelocity;
                g_aShipPosition_00494550[projectile].x =
                    g_aaTurretProjectileOffsets_0049d448[gunSide][shot].y *
                        gunUp.x +
                    g_aaTurretProjectileOffsets_0049d448[gunSide][shot].z *
                        gunForward.x +
                    g_aaTurretProjectileOffsets_0049d448[gunSide][shot].x *
                        gunRight.x +
                    gunPosition.x;
                g_aShipPosition_00494550[projectile].y =
                    g_aaTurretProjectileOffsets_0049d448[gunSide][shot].y *
                        gunUp.y +
                    g_aaTurretProjectileOffsets_0049d448[gunSide][shot].z *
                        gunForward.y +
                    g_aaTurretProjectileOffsets_0049d448[gunSide][shot].x *
                        gunRight.y +
                    gunPosition.y;
                g_aShipPosition_00494550[projectile].z =
                    g_aaTurretProjectileOffsets_0049d448[gunSide][shot].z *
                        gunForward.z +
                    g_aaTurretProjectileOffsets_0049d448[gunSide][shot].y *
                        gunUp.z +
                    g_aaTurretProjectileOffsets_0049d448[gunSide][shot].x *
                        gunRight.z +
                    gunPosition.z;
                g_asObjectCounter_00494be0[projectile] =
                    projectileData->lifetime;
                zero_vector(&g_aShipVelocity_00494898[projectile]);
                aimPoint = gunForward;
                NormalizeFixedVector(&aimPoint);
                leadDistance = (short)(
                    (projectileData->lifetime + 5) *
                    projectileData->maximumVelocity);
                ScaleFixedVector(
                    &aimPoint, (int)leadDistance << 8, &aimPoint);
                aimPoint = g_aShipPosition_00494550[target];
                AddFixedVectors(&aimJitter, &aimPoint, &aimPoint);
                point_at(projectile, aimPoint);
                ScaleFixedVector(
                    &g_aShipForwardVector_00494208[projectile],
                    (int)projectileSpeed << 8, &aimPoint);
                g_aShipVelocity_00494898[projectile] = aimPoint;
                RecordCannedSceneObjectEvent(projectile, 0);
                PlaySfxWaveFileByNumber(8, projectile, 0);
                g_asGunCooldown_005c8d70[ship * 3 + turret] = 12;
                if (ship != 0 &&
                    g_asShipSide_004955d0[ship] == SIDE_IMPERIAL)
                    g_asGunCooldown_005c8d70[ship * 3 + turret] =
                        (short)(
                            g_asGunCooldown_005c8d70[ship * 3 + turret] +
                            6);
            }
        }
        if (ship != 0)
            return;
    }
}

/* Function start: 0x4601F1 */
void ResetTargetCameraView(void)
{
    g_nFleetOverviewYawVelocity_0049d3ec =
        g_nFleetOverviewPitchVelocity_0049d3f0 =
            g_nFleetOverviewYaw_0049d3f4 =
                g_nFleetOverviewPitch_0049d3f8 = 0;
    ResetCockpitPaletteEntries();
    ResetScannerContacts();
    g_nTargetCameraMode_005c8d50 = 1;
    ToggleTargetCameraOverlay();
}

/* Function start: 0x460242 */
short CountShipCockpitGunDisplays(short ship)
{
    short count;
    short weapon;

    count = 0;
    weapon = 0;
    while (weapon <
           (short)(signed char)g_aShipWeapons_004956b0[ship][0]) {
        if (((ShipWeaponSlot *)&g_aShipWeapons_004956b0[ship][1])[
                weapon].type == 0x0b)
            count++;
        weapon++;
    }
    return count;
}

/* Function start: 0x4602BE */
void ToggleTargetCameraOverlay(void)
{
    if (g_nTargetCameraMode_005c8d50 == 0) {
        g_nTargetCameraZoom_0049d3e4 = 0x21;
        g_nTargetCameraFrame_0049d3e8 = 0;
        g_nTargetCameraMode_005c8d50 = 1;
        g_acShipTarget_00495f20[0] = -1;
        DrawSpriteDefault(
            &g_stScreenViewport_005d21a0,
            ((CockpitTargetCameraOverlay *)
                 g_abGunDisplayConfiguration_0049d340)
                [g_nGunDisplayIndex_005c8dc0].enabledX,
            ((CockpitTargetCameraOverlay *)
                 g_abGunDisplayConfiguration_0049d340)
                [g_nGunDisplayIndex_005c8dc0].enabledY,
            g_apCockpitVduOverlayShapes_0049a5f8[
                g_nGunDisplayIndex_005c8dc0],
            ((CockpitTargetCameraOverlay *)
                 g_abGunDisplayConfiguration_0049d340)
                [g_nGunDisplayIndex_005c8dc0].enabledFrame);
    } else {
        g_acShipTarget_00495f20[0] = -1;
        g_nTargetCameraZoom_0049d3e4 = 0;
        g_nTargetCameraFrame_0049d3e8 = 0;
        g_nTargetCameraMode_005c8d50 = 0;
        DrawSpriteDefault(
            &g_stScreenViewport_005d21a0,
            ((CockpitTargetCameraOverlay *)
                 g_abGunDisplayConfiguration_0049d340)
                [g_nGunDisplayIndex_005c8dc0].disabledX,
            ((CockpitTargetCameraOverlay *)
                 g_abGunDisplayConfiguration_0049d340)
                [g_nGunDisplayIndex_005c8dc0].disabledY,
            g_apCockpitVduOverlayShapes_0049a5f8[
                g_nGunDisplayIndex_005c8dc0],
            ((CockpitTargetCameraOverlay *)
                 g_abGunDisplayConfiguration_0049d340)
                [g_nGunDisplayIndex_005c8dc0].disabledFrame);
    }
}

/* Function start: 0x4603C5 */
void ClearTargetCameraView(void)
{
    FlushSoundEffectsAndLog(g_nTargetCameraSoundHandle_0049d3fc, 0);
    if (g_nTargetCameraObject_0049d338 != -1)
        zero_vector(&g_aShipVelocity_00494898[
            g_nTargetCameraObject_0049d338]);
    g_nTargetCameraSoundHandle_0049d3fc = 0;
    if (g_nTargetCameraEffectObjectA_0049d468 != -1)
        remove_object(g_nTargetCameraEffectObjectA_0049d468);
    if (g_nTargetCameraEffectObjectB_0049d46c != -1)
        remove_object(g_nTargetCameraEffectObjectB_0049d46c);
    g_nTargetCameraEffectObjectA_0049d468 = -1;
    g_nTargetCameraEffectObjectB_0049d46c = -1;
    g_nTargetCameraFrame_0049d3e8 = 0;
    g_nTargetCameraZoom_0049d3e4 = 0;
    g_nTargetCameraObject_0049d338 = -1;
    g_acShipTarget_00495f20[0] = -1;
    g_nFleetOverviewYawVelocity_0049d3ec = 0;
    g_nFleetOverviewPitchVelocity_0049d3f0 = 0;
}

/* Function start: 0x460498 */
void ToggleTargetCameraTracking(void)
{
    if (g_nTargetCameraFrame_0049d3e8 == 1) {
        ClearTargetCameraView();
    } else if (g_acShipTarget_00495f20[0] != -1) {
        g_nTargetCameraObject_0049d338 =
            (short)g_acShipTarget_00495f20[0];
        if (g_bHighMemoryResourcesEnabled_005c80e4 != 0 &&
            g_aObjectTypeData_00496d30[
                g_nObjectType62Index_00492d64].shapeSet != 0 &&
            g_aObjectTypeData_00496d30[
                g_nObjectType63Index_00492d68].shapeSet != 0) {
            g_nTargetCameraEffectObjectA_0049d468 =
                find_vacant_3d_object();
            set_objects_data(
                g_nTargetCameraEffectObjectA_0049d468, 0x3e,
                g_nTargetCameraObject_0049d338, 0);
            g_aShipPosition_00494550[
                g_nTargetCameraEffectObjectA_0049d468] =
                g_aShipPosition_00494550[
                    g_nTargetCameraObject_0049d338];
            g_aShipPosition_00494550[
                g_nTargetCameraEffectObjectA_0049d468].z -= 0x6400;
            g_asObjectViewFrame_00493508[
                g_nTargetCameraEffectObjectA_0049d468] = 0;

            g_nTargetCameraEffectObjectB_0049d46c =
                find_vacant_3d_object();
            set_objects_data(
                g_nTargetCameraEffectObjectB_0049d46c, 0x3f,
                g_nTargetCameraObject_0049d338, 0);
            g_aShipPosition_00494550[
                g_nTargetCameraEffectObjectB_0049d46c] =
                g_aShipPosition_00494550[
                    g_nTargetCameraObject_0049d338];
            g_aShipPosition_00494550[
                g_nTargetCameraEffectObjectB_0049d46c].z += 0x6400;
            g_asObjectViewFrame_00493508[
                g_nTargetCameraEffectObjectB_0049d46c] = 0;
        }
        if ((g_nSpaceFrame_00493134 < 1) & 7)
            PlaySfxWaveFileByNumber(0x26, -1, 0);
        g_nTargetCameraFrame_0049d3e8 = 1;
        g_nTargetCameraZoom_0049d3e4 = 100;
    }
}

/* Function start: 0x4608E8 */
void UpdateTargetCameraTracking(void)
{
    short yawMagnitude;
    short pitchMagnitude;
    short yawExcess;
    short pitchExcess;
    short rangeScale;
    unsigned short distance;

    rangeScale = 0;
    pitchExcess = rangeScale;
    yawExcess = pitchExcess;
    g_nFleetOverviewYawVelocity_0049d3ec = 0;
    if (g_asObjectScreenX_00493598[
            g_nTargetCameraObject_0049d338] > 1)
        g_nFleetOverviewYawVelocity_0049d3ec = -1;
    if (g_asObjectScreenX_00493598[
            g_nTargetCameraObject_0049d338] < -1)
        g_nFleetOverviewYawVelocity_0049d3ec = 1;
    g_nFleetOverviewPitchVelocity_0049d3f0 = 0;
    if (g_asObjectScreenY_00493628[
            g_nTargetCameraObject_0049d338] > 1)
        g_nFleetOverviewPitchVelocity_0049d3f0 = 1;
    if (g_asObjectScreenY_00493628[
            g_nTargetCameraObject_0049d338] < -1)
        g_nFleetOverviewPitchVelocity_0049d3f0 = -1;
    distance = (unsigned short)distance_from_object(
        0, g_nTargetCameraObject_0049d338);
    if (g_nFleetOverviewYawVelocity_0049d3ec <= 65 &&
        g_nFleetOverviewYawVelocity_0049d3ec >= -65) {
        if (g_nFleetOverviewPitchVelocity_0049d3f0 <= 65 &&
            g_nFleetOverviewPitchVelocity_0049d3f0 >= -65) {
            if (distance <= 4000) {
                yawMagnitude = (short)(abs(
                    (int)g_nFleetOverviewYaw_0049d3f4) - 17);
                pitchMagnitude = (short)(abs(
                    (int)g_nFleetOverviewPitch_0049d3f8) - 17);
                if (yawMagnitude > 0)
                    yawExcess = yawMagnitude;
                if (pitchMagnitude > 0)
                    pitchExcess = pitchMagnitude;
                rangeScale = (short)(distance >> 6);
                g_nTargetCameraZoom_0049d3e4 = 100;
                if (pitchExcess < yawExcess &&
                    rangeScale < yawExcess)
                    g_nTargetCameraZoom_0049d3e4 =
                        (short)(g_nTargetCameraZoom_0049d3e4 -
                                yawExcess);
                if (pitchExcess > yawExcess &&
                    rangeScale < pitchExcess)
                    g_nTargetCameraZoom_0049d3e4 =
                        (short)(g_nTargetCameraZoom_0049d3e4 -
                                pitchExcess);
                if (rangeScale > yawExcess &&
                    rangeScale > pitchExcess)
                    g_nTargetCameraZoom_0049d3e4 =
                        (short)(g_nTargetCameraZoom_0049d3e4 -
                                rangeScale);
                if (g_nTargetCameraZoom_0049d3e4 > 100)
                    g_nTargetCameraZoom_0049d3e4 = 100;
                if (g_nTargetCameraZoom_0049d3e4 < 67)
                    g_nTargetCameraZoom_0049d3e4 = 67;
                return;
            }
            ClearTargetCameraView();
            zero_vector(&g_aShipVelocity_00494898[
                g_nTargetCameraObject_0049d338]);
            return;
        }
        ClearTargetCameraView();
        zero_vector(&g_aShipVelocity_00494898[
            g_nTargetCameraObject_0049d338]);
        return;
    }
    ClearTargetCameraView();
    zero_vector(&g_aShipVelocity_00494898[
        g_nTargetCameraObject_0049d338]);
}

/* Function start: 0x460BAF */
void ClearSavedTorpedoTargetLock(void)
{
    g_bSavedPlayerTarget_0049d464 = 0;
    g_bSavedTargetLockAcquired_0049d488 = 0;
}

/* Function start: 0x460BCC */
void UpdateSavedTorpedoTargetLock(void)
{
    if (g_bSavedPlayerTarget_0049d464 != 0) {
        get_facing_range_from_object(0, g_nSavedPlayerTarget_0049d460);
        if (g_nFacingToTarget_00493194 < 100)
            point_ship_at_object(0, g_nSavedPlayerTarget_0049d460);
        if (g_nTargetRange_0049319c < 3000)
            approach_zero_speed(0);
        if ((g_nTargetLockMusicCooldown_005d1e78 -
                     g_nFacingToTarget_00493194 > 5 &&
                 g_nFacingToTarget_00493194 < 100) ||
            unactive(g_nSavedPlayerTarget_0049d460) != 0 ||
            g_nTargetRange_0049319c > 10000 ||
            g_acPlayerComponentDamage_00493470[5] >= 4) {
            ClearSavedTorpedoTargetLock();
            SetHudMessageText(
                "Torpedo Lock Lost",
                (unsigned short)g_abGamePaletteReservedColours_0049cb54[4],
                20);
            handle_collisions(0);
        } else {
            g_nTargetLockMusicCooldown_005d1e78 =
                g_nFacingToTarget_00493194;
            if (g_nTargetLockMusicCooldown_005d1e78 > 100)
                g_nTargetLockMusicCooldown_005d1e78 = 100;
            g_nSavedTargetLockCountdown_0049d484--;
            g_bSavedTargetLockAcquired_0049d488 = 0;
            if (g_nSavedTargetLockCountdown_0049d484 == 0) {
                SetHudMessageText(
                    "Torpedo Locked!",
                    (unsigned short)g_abGamePaletteReservedColours_0049cb54[4],
                    20);
                g_bSavedTargetLockAcquired_0049d488 = 1;
            }
        }
    } else
        handle_collisions(0);
}

/* Function start: 0x460D3E */
void SetTargetCameraEyePosition(short cameraMode)
{
    ShortVector offset;

    offset = g_aTargetCameraEyeOffsets_0049d430[cameraMode];
    g_aShipPosition_00494550[WC2_EYE_OBJECT].x =
        offset.y * g_aShipUpVector_00493ec0[0].x +
        offset.z * g_aShipForwardVector_00494208[0].x +
        offset.x * g_aShipRightVector_00493b78[0].x +
        g_aShipPosition_00494550[0].x;
    g_aShipPosition_00494550[WC2_EYE_OBJECT].y =
        offset.y * g_aShipUpVector_00493ec0[0].y +
        offset.z * g_aShipForwardVector_00494208[0].y +
        offset.x * g_aShipRightVector_00493b78[0].y +
        g_aShipPosition_00494550[0].y;
    g_aShipPosition_00494550[WC2_EYE_OBJECT].z =
        offset.y * g_aShipUpVector_00493ec0[0].z +
        offset.z * g_aShipForwardVector_00494208[0].z +
        offset.x * g_aShipRightVector_00493b78[0].z +
        g_aShipPosition_00494550[0].z;
}

/* Function start: 0x460DFE */
void DrawTargetCameraAttitudeIndicators(unsigned int colour)
{
    CockpitScannerGeometry scanner;

    scanner = ((CockpitScannerGeometry *)(
        g_abGunDisplayConfiguration_0049d340 + 0x40))[
            g_nGunDisplayIndex_005c8dc0];
    if (g_nFleetOverviewYaw_0049d3f4 != 0) {
        if (g_nFleetOverviewYaw_0049d3f4 < 0)
            g_nTargetCameraYawIndicatorX_005c8d3e =
                (short)(scanner.centerX +
                        g_nFleetOverviewYaw_0049d3f4 * 14 / 65 + 14);
        else
            g_nTargetCameraYawIndicatorX_005c8d3e =
                (short)(scanner.centerX -
                        (g_nFleetOverviewYaw_0049d3f4 * -14 / 65 + 14));
        g_nTargetCameraYawIndicatorY_005c8d40 = scanner.centerY;
        g_asTargetCameraYawIndicatorBackground_005c8d48[0] =
            (short)GetViewportPixel(
                &g_stScreenViewport_005d21a0,
                g_nTargetCameraYawIndicatorX_005c8d3e,
                (short)(g_nTargetCameraYawIndicatorY_005c8d40 - 1));
        DrawViewportPixel(
            &g_stScreenViewport_005d21a0,
            g_nTargetCameraYawIndicatorX_005c8d3e,
            (short)(g_nTargetCameraYawIndicatorY_005c8d40 - 1),
            (short)colour);
        g_asTargetCameraYawIndicatorBackground_005c8d48[1] =
            (short)GetViewportPixel(
                &g_stScreenViewport_005d21a0,
                g_nTargetCameraYawIndicatorX_005c8d3e,
                g_nTargetCameraYawIndicatorY_005c8d40);
        DrawViewportPixel(
            &g_stScreenViewport_005d21a0,
            g_nTargetCameraYawIndicatorX_005c8d3e,
            g_nTargetCameraYawIndicatorY_005c8d40,
            (short)colour);
        g_asTargetCameraYawIndicatorBackground_005c8d48[2] =
            (short)GetViewportPixel(
                &g_stScreenViewport_005d21a0,
                g_nTargetCameraYawIndicatorX_005c8d3e,
                (short)(g_nTargetCameraYawIndicatorY_005c8d40 + 1));
        DrawViewportPixel(
            &g_stScreenViewport_005d21a0,
            g_nTargetCameraYawIndicatorX_005c8d3e,
            (short)(g_nTargetCameraYawIndicatorY_005c8d40 + 1),
            (short)colour);
    }
    if (g_nFleetOverviewPitch_0049d3f8 != 0) {
        if (g_nFleetOverviewPitch_0049d3f8 > 0)
            g_nTargetCameraPitchIndicatorY_005c8d6e =
                (short)(scanner.centerY +
                        g_nFleetOverviewPitch_0049d3f8 * -14 / 65 + 14);
        else
            g_nTargetCameraPitchIndicatorY_005c8d6e =
                (short)(scanner.centerY -
                        (g_nFleetOverviewPitch_0049d3f8 * 14 / 65 + 14));
        g_nTargetCameraPitchIndicatorX_005c8d6c = scanner.centerX;
        g_asTargetCameraPitchIndicatorBackground_005c8db0[0] =
            (short)GetViewportPixel(
                &g_stScreenViewport_005d21a0,
                (short)(g_nTargetCameraPitchIndicatorX_005c8d6c - 1),
                g_nTargetCameraPitchIndicatorY_005c8d6e);
        DrawViewportPixel(
            &g_stScreenViewport_005d21a0,
            (short)(g_nTargetCameraPitchIndicatorX_005c8d6c - 1),
            g_nTargetCameraPitchIndicatorY_005c8d6e,
            (short)colour);
        g_asTargetCameraPitchIndicatorBackground_005c8db0[1] =
            (short)GetViewportPixel(
                &g_stScreenViewport_005d21a0,
                g_nTargetCameraPitchIndicatorX_005c8d6c,
                g_nTargetCameraPitchIndicatorY_005c8d6e);
        DrawViewportPixel(
            &g_stScreenViewport_005d21a0,
            g_nTargetCameraPitchIndicatorX_005c8d6c,
            g_nTargetCameraPitchIndicatorY_005c8d6e,
            (short)colour);
        g_asTargetCameraPitchIndicatorBackground_005c8db0[2] =
            (short)GetViewportPixel(
                &g_stScreenViewport_005d21a0,
                (short)(g_nTargetCameraPitchIndicatorX_005c8d6c + 1),
                g_nTargetCameraPitchIndicatorY_005c8d6e);
        DrawViewportPixel(
            &g_stScreenViewport_005d21a0,
            (short)(g_nTargetCameraPitchIndicatorX_005c8d6c + 1),
            g_nTargetCameraPitchIndicatorY_005c8d6e,
            (short)colour);
    }
}

/* Function start: 0x4610BC */
void RestoreTargetCameraAttitudeIndicators(void)
{
    CockpitScannerGeometry scanner;
    short colour;

    colour = 0xfa;
    scanner = ((CockpitScannerGeometry *)(
        g_abGunDisplayConfiguration_0049d340 + 0x40))[
            g_nGunDisplayIndex_005c8dc0];
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        g_nTargetCameraYawIndicatorX_005c8d3e,
        (short)(g_nTargetCameraYawIndicatorY_005c8d40 - 1),
        g_asTargetCameraYawIndicatorBackground_005c8d48[0]);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        g_nTargetCameraYawIndicatorX_005c8d3e,
        g_nTargetCameraYawIndicatorY_005c8d40,
        g_asTargetCameraYawIndicatorBackground_005c8d48[1]);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        g_nTargetCameraYawIndicatorX_005c8d3e,
        (short)(g_nTargetCameraYawIndicatorY_005c8d40 + 1),
        g_asTargetCameraYawIndicatorBackground_005c8d48[2]);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        (short)(g_nTargetCameraPitchIndicatorX_005c8d6c - 1),
        g_nTargetCameraPitchIndicatorY_005c8d6e,
        g_asTargetCameraPitchIndicatorBackground_005c8db0[0]);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        g_nTargetCameraPitchIndicatorX_005c8d6c,
        g_nTargetCameraPitchIndicatorY_005c8d6e,
        g_asTargetCameraPitchIndicatorBackground_005c8db0[1]);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        (short)(g_nTargetCameraPitchIndicatorX_005c8d6c + 1),
        g_nTargetCameraPitchIndicatorY_005c8d6e,
        g_asTargetCameraPitchIndicatorBackground_005c8db0[2]);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        scanner.centerX, scanner.centerY, colour);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        (short)(scanner.centerX + 1), scanner.centerY, colour);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        (short)(scanner.centerX - 1), scanner.centerY, colour);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        scanner.centerX, (short)(scanner.centerY + 1), colour);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        (short)(scanner.centerX + 1),
        (short)(scanner.centerY + 1), colour);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        (short)(scanner.centerX - 1),
        (short)(scanner.centerY + 1), colour);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        scanner.centerX, (short)(scanner.centerY - 1), colour);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        (short)(scanner.centerX + 1),
        (short)(scanner.centerY - 1), colour);
    DrawViewportPixel(
        &g_stScreenViewport_005d21a0,
        (short)(scanner.centerX - 1),
        (short)(scanner.centerY - 1), colour);
}

/* Function start: 0x461456 */
void UpdateTargetCameraCockpitHook(void)
{
}

/* Function start: 0x461466 */
void FinalizeTargetCameraViewHook(void)
{
}

/* Function start: 0x469AD0 */
unsigned int GetAvailableMainMemory(void)
{
    return 0x7c0600;
}

/* Function start: 0x469B49 */
unsigned int GetLargestMainMemoryBlock(void)
{
    return 0x7c0600;
}

/* Function start: WC2_UNMAPPED */
unsigned int PreloadMusicTrackHook(short track)
{
    (void)track;
    return 0;
}

/* Function start: WC2_UNMAPPED */
unsigned int ReleaseMusicTrackHook(short track)
{
    (void)track;
    return 0;
}

/* Function start: WC2_UNMAPPED */
signed char *__stdcall FindSceneAnimationCommand(
    signed char *script, signed char command)
{
    signed char opcode;

    while (*script != 0) {
        opcode = *script++;
        if (opcode == command) {
            script--;
            break;
        }
        switch (opcode) {
        case 'A':
        case 'L':
        case 'Q':
            script++;
        case 'B':
        case 'G':
        case 'J':
        case 'R':
        case 'W':
            script += 2;
            break;
        case 'D':
            while (*script++ != -1) {
            }
            break;
        case 'E':
        case 'P':
        case 'S':
            script++;
            break;
        case 'X':
            script += 10;
            break;
        }
    }
    return *script != 0 ? script : 0;
}

/* Function start: WC2_UNMAPPED */
short __stdcall SceneAnimationGoalReached(short delta, short current,
                                          short goal)
{
    if (delta < 0) {
        if (current <= goal)
            return 1;
    } else if (delta > 0) {
        if (current >= goal)
            return 1;
    } else if (current == goal) {
        return 1;
    }
    return 0;
}

/* Function start: WC2_UNMAPPED */
unsigned int __stdcall UpdateSceneAnimationObject(
    SceneAnimationObject *object, Viewport *viewport)
{
    SceneAnimationObject *source;
    signed char *commandStart;
    signed char *cursor;
    signed char *label;
    signed char opcode;
    signed char property;
    unsigned short complete;
    unsigned short goalFlags;
    short delay;
    short value;
    short frame;
    short xOffset;
    short labelNumber;
    short objectIndex;
    short objectCount;
    signed char stop;

    complete = 0;
    delay = object->delay;
    stop = 0;
    if (delay != 0)
        cursor = object->repeatCursor;
    else
        cursor = object->scriptCursor;

    while (*cursor != 0 && stop == 0) {
        opcode = *cursor++;
        switch (opcode) {
        case 'A':
            property = *cursor++;
            value = *(short *)cursor;
            cursor += 2;
            switch (property) {
            case 'F':
                object->frame = (short)(object->frame + value);
                object->deltaFrame = value;
                break;
            case 'R':
                object->rotation = (short)(object->rotation + value);
                object->deltaRotation = value;
                if (object->rotation < 0)
                    object->rotation = (short)(
                        ((unsigned short)(0x167 - object->rotation) /
                         0x168) * 0x168 + object->rotation);
                if (object->rotation > 0x167)
                    object->rotation = (short)(
                        object->rotation -
                        ((unsigned short)object->rotation / 0x168) *
                            0x168);
                break;
            case 'S':
                object->scale = (short)(object->scale + value);
                object->deltaScale = value;
                if (object->scale < 0x40)
                    object->scale = 0x40;
                else if (object->scale > 0x1fff)
                    object->scale = 0x1fff;
                break;
            case 'T':
                delay = (short)(delay + value);
                break;
            case 'X':
                object->x = (short)(object->x + value);
                object->deltaX = value;
                break;
            case 'Y':
                object->y = (short)(object->y + value);
                object->deltaY = value;
                break;
            }
            break;

        case 'B':
            cursor += 2;
            break;

        case 'D':
            commandStart = cursor - 1;
            object->repeatCursor = commandStart;
            xOffset = 0;
            frame = (short)*cursor++;
            while (frame != -1) {
                if (object->layer != 2 && g_nFrameSkipCountdown_0049d760 < 1)
                    DrawSpriteScaled(
                        viewport, (short)(object->x + xOffset), object->y,
                        object->shape, frame, object->rotation,
                        object->scale, object->frame);
                if (object->layer == 0)
                    xOffset = (short)(xOffset + 320);
                frame = (short)*cursor++;
            }
            stop = 1;
            break;

        case 'E':
            commandStart = cursor - 1;
            xOffset = 0;
            complete = 1;
            frame = (short)*cursor++;
            while (frame != -1) {
                if (object->layer != 2 && g_nFrameSkipCountdown_0049d760 < 1)
                    DrawSpriteScaled(
                        viewport, (short)(object->x + xOffset), object->y,
                        object->shape, frame, object->rotation,
                        object->scale, object->frame);
                xOffset = (short)(xOffset + 320);
                frame = (short)*cursor++;
            }
            cursor = commandStart;
            stop = 1;
            break;

        case 'G':
        case 'J':
            if (opcode == 'J')
                stop = 1;
            labelNumber = *(short *)cursor;
            label = object->scriptStart;
            do {
                label = FindSceneAnimationCommand(label, 'B');
                cursor = label + 3;
                value = *(short *)(label + 1);
                label = cursor;
            } while (value != labelNumber);
            break;

        case 'L':
            property = *cursor++;
            value = *(short *)cursor;
            cursor += 2;
            switch (property) {
            case 'F':
                object->frame = value;
                break;
            case 'R':
                object->rotation = value;
                if (object->rotation < 0)
                    object->rotation = (short)(
                        ((unsigned short)(0x167 - object->rotation) /
                         0x168) * 0x168 + object->rotation);
                if (object->rotation > 0x167)
                    object->rotation = (short)(
                        object->rotation -
                        ((unsigned short)object->rotation / 0x168) *
                            0x168);
                break;
            case 'S':
                object->scale = value;
                if (object->scale < 0x40)
                    object->scale = 0x40;
                else if (object->scale > 0x1fff)
                    object->scale = 0x1fff;
                break;
            case 'T':
                delay = value;
                break;
            case 'X':
                object->x = value;
                break;
            case 'Y':
                object->y = value;
                break;
            }
            break;

        case 'P':
            stop = 1;
            break;

        case 'Q':
            property = *cursor++;
            value = *(short *)cursor;
            cursor += 2;
            switch (property) {
            case 'F':
                object->goalFlags |= 0x10;
                object->goalFrame = value;
                break;
            case 'R':
                object->goalFlags |= 1;
                object->goalRotation = value;
                break;
            case 'S':
                object->goalFlags |= 2;
                object->goalScale = value;
                break;
            case 'X':
                object->goalFlags |= 4;
                object->goalX = value;
                break;
            case 'Y':
                object->goalFlags |= 8;
                object->goalY = value;
                break;
            }
            break;

        case 'R':
            objectCount = *(short *)g_pSceneAnimationDefinitions_005a7c6c;
            objectIndex = (short)(objectCount * (short)*cursor++);
            objectIndex = (short)(objectIndex + (short)*cursor++);
            source = &g_pSceneAnimationObjects_005a7c64[objectIndex];
            object->x = source->x;
            object->y = source->y;
            object->rotation = source->rotation;
            object->scale = source->scale;
            object->frame = source->frame;
            break;

        case 'W':
            g_nSceneAnimationWaitFrames_005a7c68 = *(short *)cursor;
            cursor += 2;
            g_bSceneAnimationWaitCommand_00469d70_WC1_UNMAPPED = 1;
            break;

        case 'X':
            object->x = *(short *)cursor;
            cursor += 2;
            object->y = *(short *)cursor;
            cursor += 2;
            object->rotation = *(short *)cursor;
            cursor += 2;
            object->scale = *(short *)cursor;
            cursor += 2;
            object->frame = *(short *)cursor;
            cursor += 2;
            break;
        }
    }

    if (object->delay != 0) {
        object->delay--;
        return 0;
    }

    object->scriptCursor = cursor;
    goalFlags = object->goalFlags;
    object->delay = delay;
    if (complete == 0 && goalFlags != 0) {
        if ((goalFlags & 0x10) != 0)
            complete = SceneAnimationGoalReached(
                object->deltaFrame, object->frame, object->goalFrame);
        if ((goalFlags & 4) != 0)
            complete |= SceneAnimationGoalReached(
                object->deltaX, object->x, object->goalX);
        if ((goalFlags & 8) != 0)
            complete |= SceneAnimationGoalReached(
                object->deltaY, object->y, object->goalY);
        if ((goalFlags & 2) != 0)
            complete |= SceneAnimationGoalReached(
                object->deltaScale, object->scale, object->goalScale);
        if ((goalFlags & 1) != 0)
            complete |= SceneAnimationGoalReached(
                object->deltaRotation, object->rotation,
                object->goalRotation);
    }
    return complete;
}

/* Function start: WC2_UNMAPPED */
void PlaySceneAnimation(char *text, short animation, short duration)
{
    SceneAnimationObject *object;
    SceneAnimationObject *objects;
    unsigned short complete;
    short objectCount;
    short remaining;

    g_nSceneAnimationWaitFrames_005a7c68 = -1;
    complete = 0;
    g_bSceneAnimationWaitCommand_00469d70_WC1_UNMAPPED = 0;
    AddPCName(text);
    ClearViewport(&g_stConversationTextViewport_005d2b60,
                  g_cSecondaryViewBufferColour_0049cb4c);
    FormatTextBufferFromStart(g_szSceneAnimationTextFormat_00469d74_WC1_UNMAPPED,
                              0, 160,
                              g_nConversationTextColour_00598c10,
                              g_szTextScratchBuffer_005d1c40);

    objectCount = *(short *)g_pSceneAnimationDefinitions_005a7c6c;
    objects = g_pSceneAnimationObjects_005a7c64 +
        (short)(objectCount * animation);
    remaining = objectCount;
    object = objects;
    while (remaining > 0) {
        if (object->layer == 0)
            object->shape = g_pSceneAnimationPrimaryShape_005a7c58;
        else
            object->shape = g_pSceneAnimationSecondaryShape_005a7c70;
        object->scriptStart =
            (signed char *)g_pSceneAnimationDefinitions_005a7c6c +
            object->scriptOffset;
        object->scriptCursor = object->scriptStart;
        object++;
        remaining--;
    }

    g_nFrameSkipCountdown_0049d760 = 1;
    g_bSceneEscapeRequested_0049d4b0 = 0;
    ClearInputKeyState();
    MarkDibDirty();
    DIBslamReal();
    for (;;) {
        do {
            g_nFrameSkipCountdown_0049d760--;
            object = objects;
            remaining = objectCount;
            while (remaining > 0) {
                complete |= (unsigned short)
                    UpdateSceneAnimationObject(object, &g_stSecondaryViewBuffer_005d2c90);
                object++;
                remaining--;
            }
            if (g_nSceneAnimationWaitFrames_005a7c68 != -1) {
                if (g_nSceneAnimationWaitFrames_005a7c68 == 0)
                    complete++;
                else
                    g_nSceneAnimationWaitFrames_005a7c68--;
            }
            if (complete == 0) {
                RefreshMemoryStatusOverlay();
                MarkDibDirty();
                DIBslamReal();
            }
            if (g_nFrameSkipCountdown_0049d760 == 0) {
                g_nFrameSkipCountdown_0049d760 = g_nFrameSkip_0049d764;
                if (g_bSlowSceneAnimation_00469998_WC1_UNMAPPED != 0)
                    g_nFrameSkipCountdown_0049d760++;
            }
            if ((complete == 0 && CheckEscaped() != 0) ||
                g_bSceneEscapeRequested_0049d4b0 != 0) {
                if (g_nSceneAnimationWaitFrames_005a7c68 == -1) {
                    while (complete == 0 &&
                           g_bSceneAnimationWaitCommand_00469d70_WC1_UNMAPPED == 0) {
                        object = objects;
                        remaining = objectCount;
                        while (remaining > 0) {
                            g_nFrameSkipCountdown_0049d760 = 2;
                            complete |= (unsigned short)
                                UpdateSceneAnimationObject(
                                    object, &g_stSecondaryViewBuffer_005d2c90);
                            object++;
                            remaining--;
                        }
                    }
                }
                g_nSceneAnimationWaitFrames_005a7c68 = 0;
            }
        } while (complete == 0);

        if (g_nSceneAnimationWaitFrames_005a7c68 == -1) {
            SetFrameTimerPeriodDirect((short)(duration / 2));
            do {
                if (IsFrameTickElapsed() != 0 || CheckEscaped() != 0)
                    break;
            } while (g_bSceneEscapeRequested_0049d4b0 == 0);
        }
        if (complete != 0)
            return;
    }
}
