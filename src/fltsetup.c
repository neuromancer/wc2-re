/*
 *  Spaceflight setup: shapes, cockpit, constellations and music.
 *
 *  Retail object 0x4568E0-0x4589CF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* The retail heap lets a copy of the largest cockpit-view packet read through
 * the shorter cached packets.  Their trailing bytes are not part of the shape;
 * copy only the loaded packet on the bounds-checked native heap. */
#ifdef SDL_PORT
#define COCKPIT_VIEW_COPY_BYTES(section) \
    ((unsigned short)GetNamedPacketSize( \
        g_7_szCockpitAssetFilenameText, (section)))
#else
#define COCKPIT_VIEW_COPY_BYTES(section) \
    ((unsigned short)g_nCockpitBackgroundBytes)
#endif

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

void initialize_cockpit(signed char mode);

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
                else
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
        memcpy(g_9_aasCockpitHitPaletteFades[index], black,
               sizeof(black));
        SetPaletteEntry((short)(index + 0xf9),
                        g_9_aasCockpitHitPaletteFades[index]);
    }
    memcpy(g_1_asSpacePaletteFade, black, sizeof(black));
    g_1_asSpacePaletteFade[2] = 32;
    SetPaletteEntry((short)g_cPrimaryViewBufferColour,
                    g_1_asSpacePaletteFade);
}

void ClearViewport(Viewport *viewport, short colour);

#pragma function(strcat, strcpy, strlen)
/* Function start: 0x456B1A */
void initialize_cockpit(signed char mode)
{
    char resourceText[4];
    Viewport savedScreen;
    unsigned char *backdrop;

    backdrop = 0;

    if (message_showing())
        EndCommMenu();

    if (g_nCockpitDisplayMode == 0 && g_1_cStoredViewDisplayModeValue == mode) {
        if (g_27_stViewScratchInfo.pixels != 0)
            ClearViewport(&g_27_stViewScratchInfo, g_cPrimaryViewBufferColour);
        else
            initialize_view_buffer();
        return;
    }

    free_view_buffer();
    savedScreen = g_101_stScrViewInfo;
    g_101_stScrViewInfo.left = g_101_stScrViewInfo.top = 0;
    g_101_stScrViewInfo.right = 319;
    g_101_stScrViewInfo.bottom = 199;
    ClearViewport(&g_2_stModalSourceWindow, g_cSecondaryViewBufferColour);

    g_1_cStoredViewDisplayModeValue = mode;
    strcpy(g_7_szCockpitAssetFilenameText, "pcship.v");
    itoa((int)g_aObjectTypeData[
              g_acObjectType[0]].resourceType,
          resourceText, 10);
    if (strlen(resourceText) == 1)
        strcat(g_7_szCockpitAssetFilenameText, "0");
    strcat(g_7_szCockpitAssetFilenameText, resourceText);

    switch (mode) {
    case 0:
        if (g_2_apCockpitViewShapes[0] != 0) {
            g_pCockpitViewFrameInfo =
                (unsigned char *)IdentityDword(
                    (DwordPtr)g_2_apCockpitViewShapes[0]);
            DosMemmove(g_pCockpitBackgroundPacket,
                      g_pCockpitViewFrameInfo,
                      COCKPIT_VIEW_COPY_BYTES(4));
        } else {
            LoadPacketIntoBuffer(
                g_7_szCockpitAssetFilenameText, 4,
                g_pCockpitBackgroundPacket, 1);
        }
        g_stCockpitBackgroundViewportRecord.left =
            g_101_stScrViewInfo.left;
        g_stCockpitBackgroundViewportRecord.right =
            g_101_stScrViewInfo.right;
        g_stCockpitBackgroundViewportRecord.top =
            g_101_stScrViewInfo.top;
        g_stCockpitBackgroundViewportRecord.bottom =
            g_101_stScrViewInfo.bottom;
        g_stCockpitBackgroundViewportRecord.pixels = 0;
        g_stCockpitBackgroundViewportRecord.allocation = 0;
        g_stCockpitBackgroundViewportRecord.rowOffsets = 0;
        if (g_19_bStoredHighMemoryEnabled != 0 &&
            AllocateViewport(
                &g_stCockpitBackgroundViewportRecord,
                g_cSecondaryViewBufferColour, 0) == 0) {
            g_stCockpitBackgroundViewportRecord.allocation = 0;
        }
        if (g_2_apCockpitShipShapes[0] != 0) {
            if (g_stCockpitBackgroundViewportRecord.allocation != 0) {
                if (g_nCockpitDisplayMode == 0) {
                    DrawSpriteDefault(
                        &g_stCockpitBackgroundViewportRecord,
                        0, 0, g_2_apCockpitShipShapes[0], 0);
                } else if (g_nCockpitDisplayMode == 0) {
                    DrawSpriteDefault(
                        &g_101_stScrViewInfo,
                        0, 0, g_2_apCockpitShipShapes[0], 0);
                }
            }
        } else {
            backdrop = LoadNamedPacket(
                g_7_szCockpitAssetFilenameText, 0,
                0, 0, 0, 1);
            if (backdrop == 0) {
                free_viewport(&g_stCockpitBackgroundViewportRecord);
                g_stCockpitBackgroundViewportRecord.allocation = 0;
                backdrop = FetchDiskPacketRetrying(
                    g_7_szCockpitAssetFilenameText, 0, 0);
                if (g_nCockpitDisplayMode == 0) {
                    DrawSpriteDefault(
                        &g_101_stScrViewInfo,
                        0, 0, backdrop, 0);
                }
            } else if (g_stCockpitBackgroundViewportRecord.allocation !=
                       0) {
                if (g_nCockpitDisplayMode == 0) {
                    DrawSpriteDefault(
                        &g_stCockpitBackgroundViewportRecord,
                        0, 0, backdrop, 0);
                } else if (g_nCockpitDisplayMode == 0) {
                    DrawSpriteDefault(
                        &g_101_stScrViewInfo,
                        0, 0, backdrop, 0);
                }
            }
            ReleasePacketHandle(backdrop);
        }
        ResetCockpitPaletteEntries();
        if (g_stCockpitBackgroundViewportRecord.allocation != 0) {
            if (g_nCockpitDisplayMode == 0)
                explosion_draw(1);
            WaitForVerticalBlankThunk();
            CopyViewportContents(
                &g_stCockpitBackgroundViewportRecord,
                &g_101_stScrViewInfo);
            free_viewport(&g_stCockpitBackgroundViewportRecord);
        } else if (g_nCockpitDisplayMode == 0) {
            explosion_draw(0);
        }
        reset_cockpit();
        InvalidateVduMode(0);
        InvalidateVduMode(1);
        update_VDUs();
        if (g_bCockpitDamageFrame0Shown == 0)
            clear_head_up_display();
        if (g_nCockpitDisplayMode == 0)
            ResetPilotHandAnimation();
        set_up_screen_viewport(0);
        break;
    case 1:
        if (g_2_apCockpitViewShapes[1] != 0) {
            g_pCockpitViewFrameInfo =
                (unsigned char *)IdentityDword(
                    (DwordPtr)g_2_apCockpitViewShapes[1]);
            DosMemmove(g_pCockpitBackgroundPacket,
                      g_pCockpitViewFrameInfo,
                      COCKPIT_VIEW_COPY_BYTES(5));
        } else {
            LoadPacketIntoBuffer(
                g_7_szCockpitAssetFilenameText, 5,
                g_pCockpitBackgroundPacket, 1);
        }
        if (g_2_apCockpitShipShapes[1] != 0) {
            if (g_nCockpitDisplayMode == 0)
                DrawSpriteDefault(&g_101_stScrViewInfo, 0, 0,
                                  g_2_apCockpitShipShapes[1], 0);
        } else {
            backdrop = FetchDiskPacketRetrying(
                g_7_szCockpitAssetFilenameText, 1, 0);
            if (g_nCockpitDisplayMode == 0)
                DrawSpriteDefault(&g_101_stScrViewInfo, 0, 0, backdrop, 0);
            ReleasePacketHandle(backdrop);
        }
        set_up_screen_viewport(1);
        break;
    case 2:
        if (g_nCurrentView == 4) {
            if (g_2_apCockpitViewShapes[1] != 0) {
                g_pCockpitViewFrameInfo =
                    (unsigned char *)IdentityDword(
                        (DwordPtr)g_2_apCockpitViewShapes[1]);
                DosMemmove(
                    g_pCockpitBackgroundPacket,
                    g_pCockpitViewFrameInfo,
                    COCKPIT_VIEW_COPY_BYTES(5));
            } else {
                LoadPacketIntoBuffer(
                    g_7_szCockpitAssetFilenameText, 5,
                    g_pCockpitBackgroundPacket, 1);
            }
            if (g_2_apCockpitShipShapes[1] != 0) {
                if (g_nCockpitDisplayMode == 0) {
                    DrawSpriteDefault(
                        &g_101_stScrViewInfo, 0, 0,
                        g_2_apCockpitShipShapes[1], 0);
                }
            } else {
                backdrop = FetchDiskPacketRetrying(
                    g_7_szCockpitAssetFilenameText, 1, 0);
                if (g_nCockpitDisplayMode == 0) {
                    DrawSpriteDefault(
                        &g_101_stScrViewInfo,
                        0, 0, backdrop, 0);
                }
                ReleasePacketHandle(backdrop);
            }
        } else {
            if (g_2_apCockpitViewShapes[2] != 0) {
                g_pCockpitViewFrameInfo =
                    (unsigned char *)IdentityDword(
                        (DwordPtr)g_2_apCockpitViewShapes[2]);
                DosMemmove(
                    g_pCockpitBackgroundPacket,
                    g_pCockpitViewFrameInfo,
                    COCKPIT_VIEW_COPY_BYTES(6));
            } else {
                LoadPacketIntoBuffer(
                    g_7_szCockpitAssetFilenameText, 6,
                    g_pCockpitBackgroundPacket, 1);
            }
            if (g_2_apCockpitShipShapes[2] != 0) {
                if (g_nCockpitDisplayMode == 0) {
                    DrawSpriteDefault(
                        &g_101_stScrViewInfo, 0, 0,
                        g_2_apCockpitShipShapes[2], 0);
                }
            } else {
                backdrop = FetchDiskPacketRetrying(
                    g_7_szCockpitAssetFilenameText, 2, 0);
                if (g_nCockpitDisplayMode == 0) {
                    DrawSpriteDefault(
                        &g_101_stScrViewInfo,
                        0, 0, backdrop, 0);
                }
                ReleasePacketHandle(backdrop);
            }
        }
        set_up_screen_viewport(2);
        break;
    case 3:
        if (g_2_apCockpitViewShapes[3] != 0) {
            g_pCockpitViewFrameInfo =
                (unsigned char *)IdentityDword(
                    (DwordPtr)g_2_apCockpitViewShapes[3]);
            DosMemmove(g_pCockpitBackgroundPacket,
                      g_pCockpitViewFrameInfo,
                      COCKPIT_VIEW_COPY_BYTES(7));
        } else {
            LoadPacketIntoBuffer(
                g_7_szCockpitAssetFilenameText, 7,
                g_pCockpitBackgroundPacket, 1);
        }
        if (g_2_apCockpitShipShapes[3] != 0) {
            if (g_nCockpitDisplayMode == 0)
                DrawSpriteDefault(&g_101_stScrViewInfo, 0, 0,
                                  g_2_apCockpitShipShapes[3], 0);
        } else {
            backdrop = FetchDiskPacketRetrying(
                g_7_szCockpitAssetFilenameText, 3, 0);
            if (g_nCockpitDisplayMode == 0)
                DrawSpriteDefault(&g_101_stScrViewInfo, 0, 0, backdrop, 0);
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

    if (g_nCurrentView == 4) {
        switch (mode) {
        case 1:
            g_8_nGunDisplaySlot = 1;
            g_9_nTargetCameraOverlayMode = 2;
            break;
        case 2:
            g_8_nGunDisplaySlot = 1;
            g_9_nTargetCameraOverlayMode = 1;
            break;
        case 3:
            g_8_nGunDisplaySlot = 0;
            g_9_nTargetCameraOverlayMode = 0;
            break;
        }
        ResetTargetCameraView();
        ResetCockpitPaletteEntries();
        clear_head_up_display();
        if (g_nCockpitDisplayMode == 0)
            FinalizeTargetCameraViewHook();
    }

    g_bSpaceViewBufferEnabled = 1;
    SetViewportRect(&g_27_stViewScratchInfo, 0, 0,
                    (unsigned short)(g_nScreenWidth - 1),
                    (unsigned short)(g_nScreenHeight - 1));
    initialize_view_buffer();
    g_101_stScrViewInfo = savedScreen;
}

/* Function start: 0x457434 */
void InitializeConstellationObject(
    const ConstellationObjectDefinition *definition, short object)
{
    char packetName[13] = "planets.v0";
    FixedVector position;
    char campaignNumber[4];

    strcat(packetName,
           itoa((int)g_nSelectedCampaignSlot,
                 campaignNumber, 10));
    g_aeObjectClass[object] = OBJECT_CLASS_PLANET;
    init_ijk(69);
    alter_yaw((short)-definition->yaw, 69);
    alter_pitch((short)-definition->pitch, 69);
    alter_roll(definition->roll, 69);
    ScaleFixedVector(&g_aShipForwardVector[69],
                     0x753000, &position);
    g_aShipPosition[object] = position;
    g_asObjectScreenAngle[object] = 0;
    g_asObjectViewFrame[object] =
        g_asObjectScreenAngle[object];
    g_asObjectType[object] =
        g_asObjectViewFrame[object];
    g_asObjectScreenScale[object] = 0xff;
    g_apObjectShape[object] =
        FetchDiskPacketRetrying(packetName,
                                (short)(definition->shapePacket + 1), 0);
}

/* Function start: 0x457587 */
void FreeConstellationObject(short object)
{
    FreePacketAndClear(&g_apObjectShape[object], 0);
    remove_object(object);
}

/* Function start: 0x4575B4 */
void init_constellation(short scene)
{
    short slot;
    short sceneIndex;
    short object;

    if (g_pConstellationSprite == 0) {
        g_pConstellationSprite =
            FetchDiskPacketRetrying("planets.v00", 0, 0);
    }
    sceneIndex = (short)(scene - 1);
    if (g_nAvailableGameMemoryNumber < 6500)
        return;
    if (sceneIndex >= 0) {
        for (slot = 0; slot < 4; slot++) {
            if (((ConstellationObjectDefinition (*)[4])
                     g_13_pStartupStellarPacket)
                    [sceneIndex][slot].shapePacket != -1) {
                object = find_vacant_3d_object();
                if (object != -1) {
                    InitializeConstellationObject(
                        &((ConstellationObjectDefinition (*)[4])
                              g_13_pStartupStellarPacket)
                             [sceneIndex][slot],
                        object);
                }
                g_asConstellationObjectIndices[slot] = object;
            } else {
                g_asConstellationObjectIndices[slot] = -1;
            }
        }
    }
}

/* Function start: 0x4576AB */
void free_constellation(void)
{
    short object;
    short slot;

    FreePacketAndClear(&g_pConstellationSprite, 0);
    for (slot = 0; slot < 4; slot++) {
        object = g_asConstellationObjectIndices[slot];
        if (object != -1) {
            FreeConstellationObject(object);
            g_asConstellationObjectIndices[slot] = -1;
        }
    }
}

/* Function start: 0x457720 */
void init_vdus(void)
{
    g_4_stRightVduView = g_101_stScrViewInfo;
    g_2_stLeftwardVduPortBlock = g_4_stRightVduView;

    InitializeTextContextFromFont(
        &g_5_stLeftVduStringContextRecord, 2,
        g_ucPrimaryTextColour,
        (signed char)g_cSecondaryViewBufferColour);
    g_5_stLeftVduStringContextRecord.viewport =
        &g_2_stLeftwardVduPortBlock;
    g_5_stLeftVduStringContextRecord.text =
        g_6_szDefaultTextBufferText;
    set_mode(0, 1);
    g_anLastDrawnVduMode[0] = 0;

    InitializeTextContextFromFont(
        &g_1_stRightVduTextContextBlock, 2,
        g_ucPrimaryTextColour,
        (signed char)g_cSecondaryViewBufferColour);
    g_1_stRightVduTextContextBlock.viewport =
        &g_4_stRightVduView;
    g_1_stRightVduTextContextBlock.text =
        g_6_szDefaultTextBufferText;
    set_mode(1, 5);
    g_anLastDrawnVduMode[1] = 0;
}

/* Function start: 0x4577D7 */
/* WC2 passes the player's ship type; the body reads it from the global
 * instead and ignores the argument. */
void InitializeCockpitResources(short shipType)
{
    CockpitResourceLayout layout;
    short displayFrame;
    short maxSize;
    short gunDisplay;
    char resourceNumber[4];
    int packetSize;
    int slot;

    g_bCockpitResourcesActive = 1;
    strcpy(g_7_szCockpitAssetFilenameText, "pcship.v");
    itoa((int)g_aObjectTypeData[
              g_acObjectType[0]].resourceType,
          resourceNumber, 10);
    if (strlen(resourceNumber) == 1)
        strcat(g_7_szCockpitAssetFilenameText, "0");
    strcat(g_7_szCockpitAssetFilenameText, resourceNumber);
    ClearHudGunReadouts();
    reset_cockpit();
    free_view_buffer();
    if (g_1_nResourcePaletteState == 0) {
        clear_cockpit_damage();
        LoadShapeSet(g_aCockpitShipShapeResources, 4,
                     g_7_szCockpitAssetFilenameText);
        LoadShapeSet(g_aCockpitViewShapeResources, 4,
                     g_7_szCockpitAssetFilenameText);
    }
    LoadPacketIntoBuffer(g_7_szCockpitAssetFilenameText, 0x0d,
                         &layout, 0);
    g_stHudMessageOrigin = layout.hudMessageOrigin;
    for (slot = 0; slot < 4; slot++)
        g_aCockpitDamagePositions[slot] =
            layout.damagePositions[slot];
    for (slot = 0; slot < 7; slot++)
        g_asCockpitLightX[slot] = layout.lightX[slot];
    for (slot = 0; slot < 7; slot++)
        g_asCockpitLightY[slot] = layout.lightY[slot];
    for (slot = 0; slot < 7; slot++)
        g_acCockpitLightOffFrame[slot] =
            layout.lightOffFrame[slot];
    for (slot = 0; slot < 7; slot++)
        g_acCockpitLightOnFrame[slot] =
            layout.lightOnFrame[slot];
    for (slot = 0; slot < 8; slot++)
        g_aCockpitBarDefinitions[slot] = layout.bars[slot];
    g_aCockpitReadoutOrigins[0] = layout.readoutOrigins[0];
    g_aCockpitReadoutOrigins[1] = layout.readoutOrigins[1];
    g_aCockpitReadoutOrigins[2] = layout.readoutOrigins[2];
    g_aCockpitReadoutOrigins[3] = layout.readoutOrigins[3];
    g_stLeftVduBounds = layout.leftVduBounds;
    g_stRightVduBounds = layout.rightVduBounds;
    g_stCockpitScannerGeometry = layout.scanner;
    g_stPilotHandBounds = layout.pilotHandBounds;
    g_stPilotHandOrigin = layout.pilotHandOrigin;
    g_cHazardBaseTravelTime = layout.field_f6;
    g_nHazardPitchRange = layout.field_f7;
    for (slot = 0; slot < 16; slot++)
        g_aDefaultWeaponDisplayPositions[slot] =
            layout.weaponDisplayPositions[slot];
    g_bTargetLockDisplayEnabled =
        layout.targetLockDisplayEnabled;

    if (g_pCockpitBackgroundPacket == 0) {
        maxSize = 0;
        for (gunDisplay = 4; gunDisplay <= 7; gunDisplay++) {
            packetSize = (int)GetNamedPacketSize(
                g_7_szCockpitAssetFilenameText, gunDisplay);
            maxSize = MaxShort(maxSize, (short)packetSize);
        }
        g_nCockpitBackgroundBytes = maxSize;
        g_pCockpitBackgroundPacket =
            AllocateTaggedMemory(
                (int)g_nCockpitBackgroundBytes, 0x40);
        if (g_pCockpitBackgroundPacket == 0)
            ReportFatalErrorCode("019");
    }

    if (HasShipCockpitGunDisplay(0) != 0) {
        g_12_nGunDisplayCount = CountShipCockpitGunDisplays(0);
        if (g_12_nGunDisplayCount == 3)
            g_12_nGunDisplayCount--;
        for (gunDisplay = 0; gunDisplay < g_12_nGunDisplayCount; gunDisplay++) {
            g_apCockpitVduOverlayShapes[gunDisplay] =
                FetchDiskPacketRetrying(
                    g_7_szCockpitAssetFilenameText,
                    (short)(gunDisplay + 0x0e), 0);
        }
        LoadPacketIntoBuffer(
            g_7_szCockpitAssetFilenameText,
            (short)(g_12_nGunDisplayCount + 0x0e),
            g_abGunDisplayConfiguration, 0);
        g_1_nGunShowEnergyPercent = 100;
    }

    g_9_stCockpitMeterPort = g_101_stScrViewInfo;
    init_vdus();
    InitializeTextContextFromFont(
        &g_16_stCockpitTextSettingsRecord, 2,
        g_ucPrimaryTextColour,
        (signed char)g_cSecondaryViewBufferColour);
    g_16_stCockpitTextSettingsRecord.viewport =
        &g_101_stScrViewInfo;
    SetTextContext(&g_16_stCockpitTextSettingsRecord);
    SetTextCursor(
        (unsigned short)g_aCockpitReadoutOrigins[0].x,
        (unsigned short)g_aCockpitReadoutOrigins[0].y);
    InitializeCockpitReadout(4, &g_16_stCockpitTextSettingsRecord);
    SetTextCursor(
        (unsigned short)g_aCockpitReadoutOrigins[1].x,
        (unsigned short)g_aCockpitReadoutOrigins[1].y);
    InitializeCockpitReadout(5, &g_16_stCockpitTextSettingsRecord);
    SetTextCursor(
        (unsigned short)g_aCockpitReadoutOrigins[2].x,
        (unsigned short)g_aCockpitReadoutOrigins[2].y);
    InitializeCockpitReadout(2, &g_16_stCockpitTextSettingsRecord);
    SetTextCursor(
        (unsigned short)g_aCockpitReadoutOrigins[3].x,
        (unsigned short)g_aCockpitReadoutOrigins[3].y);
    InitializeCockpitReadout(3, &g_16_stCockpitTextSettingsRecord);

    *(ShortRect *)&g_2_stLeftwardVduPortBlock.left = g_stLeftVduBounds;
    *(ShortRect *)&g_4_stRightVduView.left = g_stRightVduBounds;
    if (g_pPilotHandAnimationShapePtr != 0) {
        g_3_stDashViewport = g_101_stScrViewInfo;
        *(ShortRect *)&g_3_stDashViewport.left = g_stPilotHandBounds;
        SetViewportRect(
            &g_stPilotHandViewportRecord, 0, 0,
            (unsigned short)(g_stPilotHandBounds.right -
                             g_stPilotHandBounds.left),
            (unsigned short)(g_stPilotHandBounds.bottom -
                             g_stPilotHandBounds.top));
        g_stPilotHandBackgroundViewportBlock =
            g_stPilotHandViewportRecord;
        AllocateViewport(&g_stPilotHandViewportRecord,
                         g_cSecondaryViewBufferColour, 0);
        AllocateViewport(&g_stPilotHandBackgroundViewportBlock,
                         g_cSecondaryViewBufferColour, 0);
    }

    g_nHudMessageBackgroundDepth = 0;
    LoadShapeSet(g_aCockpitCommonShapeResources, 0,
                 g_7_szCockpitAssetFilenameText);
#ifdef SDL_PORT
    /* See issue #30: LoadShapeSet aborts the whole list on the first
     * missing resource, and g_1_pDashWeaponPicture (the ship's
     * cockpit weapon-display shape, section 12 of the ship-specific
     * pcshipNN.v file) is legitimately absent for ship types that have
     * no cockpit weapon display -- code elsewhere already null-checks it
     * before use. But that abort also skips every later entry in the
     * same list, including g_18_pCockpitIndicatorShape (the target
     * VDU's shield-strength indicator sprite, cockpit.vga section 4),
     * which has nothing to do with the ship-specific failure and is
     * always present. The result: any ship without a cockpit weapon
     * display never shows shield strength on a locked target, even at
     * full shields. Load it directly if the set above left it null. */
    if (g_18_pCockpitIndicatorShape == 0) {
        g_18_pCockpitIndicatorShape =
            FetchDiskPacketRetrying("cockpit.vga", 4, 0x40);
    }
#endif
    g_4_pScannerBlipUnderlayPointer = AllocateTaggedMemory(
        (unsigned int)MeasureShapeFrameStorage(
            g_66_pCabinHudImage, 2),
        0x40);
    g_pReleaseWeaponDisplayBackground = 0;
    g_pDamageDisplayBackground = 0;
    g_pCockpitExplosionBackground = 0;
    if (g_1_pDashWeaponPicture != 0) {
        maxSize = (short)MeasureShapeFrameStorage(
            g_1_pDashWeaponPicture, 1);
        g_pReleaseWeaponDisplayBackground =
            AllocateTaggedMemory((int)maxSize, 0);
        maxSize = 0;
        for (displayFrame = 0x20; displayFrame < 0x29; displayFrame++) {
            maxSize = MaxShort(
                maxSize,
                (short)MeasureShapeFrameStorage(
                    g_1_pDashWeaponPicture, displayFrame));
        }
        if (maxSize != 0)
            g_pDamageDisplayBackground =
                AllocateTaggedMemory((int)maxSize, 0);
    }
    if (g_bTargetLockDisplayEnabled != 0) {
        g_pCockpitHudBackground = AllocateTaggedMemory(
            (unsigned int)MeasureShapeFrameStorage(
                g_66_pCabinHudImage, 5),
            0);
    }
    if (g_pCockpitExplosionShape != 0) {
        maxSize = 0;
        for (displayFrame = 0; displayFrame < 8; displayFrame++) {
            maxSize = MaxShort(
                maxSize,
                (short)MeasureShapeFrameStorage(
                    g_pCockpitExplosionShape, displayFrame));
        }
        if (maxSize != 0)
            g_pCockpitExplosionBackground =
                AllocateTaggedMemory((int)maxSize, 0);
    }
    g_pHudMessageFrameShape =
        FetchDiskPacketRetrying("message.v00", 0, 0);
    g_pHudMessageBackground = AllocateTaggedMemory(
        (unsigned int)MeasureShapeFrameStorage(
            g_pHudMessageFrameShape, 0),
        0);
    ResetScannerContacts();
    g_bRadioSilence = 0;
    g_nCockpitExplosionFrame = 8;
    for (g_nObjectType62Index = 0;
         g_nObjectType62Index <= 0x3f;
         g_nObjectType62Index++) {
        if (g_aObjectTypeData[
                g_nObjectType62Index].field_18 == 0x3e)
            break;
    }
    for (g_nObjectType63Index = 0;
         g_nObjectType63Index <= 0x3f;
         g_nObjectType63Index++) {
        if (g_aObjectTypeData[
                g_nObjectType63Index].field_18 == 0x3f)
            break;
    }
    if (g_19_bStoredHighMemoryEnabled != 0 &&
        HasShipCockpitGunDisplay(0) != 0) {
        g_aObjectTypeData[
            g_nObjectType62Index].shapeSet =
            LoadNamedPacket("objects.vga", 0x15, 0, 4, 0, 1);
        g_aObjectTypeData[
            g_nObjectType63Index].shapeSet =
            LoadNamedPacket("objects.vga", 0x16, 0, 4, 0, 1);
        if (g_aObjectTypeData[
                g_nObjectType62Index].shapeSet == 0 ||
            g_aObjectTypeData[
                g_nObjectType63Index].shapeSet == 0) {
            FreePacketAndClear(
                &g_aObjectTypeData[
                    g_nObjectType62Index].shapeSet,
                4);
            FreePacketAndClear(
                &g_aObjectTypeData[
                    g_nObjectType63Index].shapeSet,
                4);
        }
    }
    for (gunDisplay = 0; gunDisplay < 50; gunDisplay++) {
        g_48_apCommunicationTextPacketsTable[gunDisplay] = 0;
    }
}

/* Function start: 0x458196 */
void free_cockpit(void)
{
    char shipTypeText[4];
    short packet;

    if (g_bCockpitResourcesActive == 0)
        return;
    else
        g_bCockpitResourcesActive = 0;
    free_view_buffer();
    if (g_pCockpitBackgroundPacket != 0) {
        ReleasePacketHandle(g_pCockpitBackgroundPacket);
        g_pCockpitBackgroundPacket = 0;
    }
    FreePacketAndClear(&g_apCockpitVduOverlayShapes[0], 0);
    FreePacketAndClear(&g_apCockpitVduOverlayShapes[1], 0);
    FreePacketAndClear(&g_pHudMessageFrameShape, 0);
    FreePacketAndClear(&g_pHudMessageBackground, 0);
    if (g_nObjectType62Index != -1)
        FreePacketAndClear(
            &g_aObjectTypeData[
                g_nObjectType62Index].shapeSet,
            4);
    if (g_nObjectType63Index != -1)
        FreePacketAndClear(
            &g_aObjectTypeData[
                g_nObjectType63Index].shapeSet,
            4);
    ReleaseTextFont(2);
    if (g_pPilotHandAnimationShapePtr != 0) {
        free_viewport(&g_stPilotHandViewportRecord);
        free_viewport(&g_stPilotHandBackgroundViewportBlock);
    }
    strcpy(g_7_szCockpitAssetFilenameText, "pcship.v");
    itoa((int)g_aObjectTypeData[
              g_acObjectType[0]].resourceType,
          shipTypeText, 10);
    if (strlen(shipTypeText) == 1)
        strcat(g_7_szCockpitAssetFilenameText, "0");
    strcat(g_7_szCockpitAssetFilenameText, shipTypeText);
    if (g_1_nResourcePaletteState == 0) {
        FreeShapeSet(g_aCockpitShipShapeResources, 4);
        FreeShapeSet(g_aCockpitViewShapeResources, 4);
    }
    FreeShapeSet(g_aCockpitCommonShapeResources, 0);
    FreePacketAndClear(&g_pPendingCockpitDamageShape, 0);
    FreePacketAndClear(&g_pReleaseWeaponDisplayBackground, 0);
    FreePacketAndClear(&g_pCockpitExplosionBackground, 0);
    FreePacketAndClear(&g_pCockpitHudBackground, 0);
    FreePacketAndClear(&g_pDamageDisplayBackground, 0);
    FreeCommDisplayResources();
    FreePacketAndClear(&g_pCommPortraitResource, 4);
    FreePacketAndClear(&g_pCommVduFrameResource, 4);
    g_nLoadedCommPortraitPilot = -1;
    if (g_4_pScannerBlipUnderlayPointer != 0) {
        ReleasePacketHandle(g_4_pScannerBlipUnderlayPointer);
        g_4_pScannerBlipUnderlayPointer = 0;
    }
    for (packet = 0; packet < 50; packet++) {
        if (g_48_apCommunicationTextPacketsTable[packet] != 0) {
            ReleasePacketHandle(
                g_48_apCommunicationTextPacketsTable[packet]);
            g_48_apCommunicationTextPacketsTable[packet] = 0;
        }
    }
}

/* Function start: 0x458467 */
void init_3Space_objects(short scene)
{
    short slot;

    if (g_b3SpaceObjectsActive == 1)
        return;
    else
        g_b3SpaceObjectsActive = 1;
    g_1_cStoredViewDisplayModeValue = -1;
    remove_all_3d_objects();
    g_nSpaceFrame =
        g_nRenderedSpaceFrame =
            (short)(g_6_nPendingAfterburnerSfxFrame =
                g_nPassingShipSndCooldown = 0);
    g_bMissileCameraEnabled = 0;
    g_nExternalViewShip = -1;
    g_nPlayerCollisionObject = -1;
    for (slot = 0; slot <= 4; slot++)
        g_aObjectResourceSlots[slot].resourceType = -1;
    init_constellation(scene);
    load_common_3Space_objects();
}

/* Function start: 0x458532 */
void load_common_3Space_objects(void)
{
    FreeShapeSet(g_aCommon3SpaceResources, 0);
    LoadShapeSet(g_aCommon3SpaceResources, 0, "objects.vga");
    g_aObjectTypeData[34].shapeSet =
        g_aObjectTypeData[35].shapeSet;
    g_aObjectTypeData[33].shapeSet =
        g_aObjectTypeData[34].shapeSet;
    g_aObjectTypeData[30].shapeSet =
        g_aObjectTypeData[33].shapeSet;
    g_aObjectTypeData[29].shapeSet =
        g_aObjectTypeData[30].shapeSet;
    g_aObjectTypeData[12].shapeSet =
        g_aObjectTypeData[7].shapeSet;
    g_aObjectTypeData[12].animation =
        g_aObjectTypeData[7].animation;
    free_ship(4);
    load_ship(1, 0x10, OBJECT_CLASS_MISSILE, 4);
    g_aObjectTypeData[19].shapeSet =
        g_aObjectTypeData[16].shapeSet;
    g_aObjectTypeData[15].shapeSet =
        g_aObjectTypeData[19].shapeSet;
    g_aObjectTypeData[18].shapeSet =
        g_aObjectTypeData[15].shapeSet;
    g_aObjectTypeData[17].shapeSet =
        g_aObjectTypeData[18].shapeSet;
    g_aObjectTypeData[19].animation =
        g_aObjectTypeData[16].animation;
    g_aObjectTypeData[15].animation =
        g_aObjectTypeData[19].animation;
    g_aObjectTypeData[18].animation =
        g_aObjectTypeData[15].animation;
    g_aObjectTypeData[17].animation =
        g_aObjectTypeData[18].animation;
    if (g_19_bStoredHighMemoryEnabled != 0) {
        if (g_1_nResourcePaletteState == 0) {
            g_pGenericMissileShape =
                FetchDiskPacketRetrying("missile.v00", 0, 4);
        }
        if (g_pGenericMissileShape != 0) {
            g_pGenericMissileExhaustShape =
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
    if (g_b3SpaceObjectsActive == 0)
        return;
    else
        g_b3SpaceObjectsActive = 0;
    free_view_buffer();
    g_bSpaceViewBufferEnabled = 0;
    free_constellation();
    remove_all_hazards();
    free_3Space_objects();
    FreeShapeSet(g_aMissionResourceDescriptors, 0);
    if (g_pNavLocationText != 0) {
        ReleasePacketHandle(g_pNavLocationText);
        g_pNavLocationText = 0;
    }
}

/* Function start: 0x458716 */
void free_3Space_objects(void)
{
    FreeShapeSet(g_aCommon3SpaceResources, 0);
    free_ship(4);
    g_aObjectTypeData[OBJECT_DATA_PILUM_FF].shapeSet =
        g_aObjectTypeData[OBJECT_DATA_SPICULUM_IR].shapeSet =
            g_aObjectTypeData[OBJECT_DATA_DART_DF].shapeSet = 0;
    g_aObjectTypeData[OBJECT_DATA_PILUM_FF].animation =
        g_aObjectTypeData[OBJECT_DATA_SPICULUM_IR].animation =
            g_aObjectTypeData[OBJECT_DATA_DART_DF].animation = 0;
    g_aObjectTypeData[OBJECT_DATA_GIRDER_CHUNK].shapeSet =
        g_aObjectTypeData[OBJECT_DATA_SHIP_TUBING].shapeSet =
            g_aObjectTypeData[
                OBJECT_DATA_BURNING_DEBRIS].shapeSet =
                g_aObjectTypeData[OBJECT_DATA_O_RING].shapeSet =
                    g_aObjectTypeData[
                        OBJECT_DATA_PIPE].shapeSet;
    FreePacketAndClear(
        &g_aObjectTypeData[OBJECT_DATA_EJECTED_PILOT].shapeSet,
        0);
    if (g_1_nResourcePaletteState == 0)
        FreePacketAndClear(&g_pGenericMissileShape, 4);
    FreePacketAndClear(&g_pGenericMissileExhaustShape, 0);
}

/* Function start: 0x4587E0 */
void init_inflight_music(void)
{
    g_nInFlightMusicActive = 1;
    g_nInitialFlightMusicPending = 1;
    g_nCombatMusicActive = 0;

}

/* Function start: 0x458806 */
void free_inflight_music(void)
{
    StopMusicUnlessSuppressed();
    g_nInFlightMusicActive = 0;
    if (g_aLimitedMusicBuffers[0].state != -1) {
        g_aTuneResources[
            g_aLimitedMusicBuffers[0].state].packet = 0;
        g_aLimitedMusicBuffers[0].state = -1;
    }
    if (g_aLimitedMusicBuffers[1].state != -1) {
        g_aTuneResources[
            g_aLimitedMusicBuffers[1].state].packet = 0;
        g_aLimitedMusicBuffers[1].state = -1;
    }
}

/* Function start: 0x45887B */
void PreloadMusicTrack(int track)
{
    if (g_nMusicDriverMode == 1 &&
        g_aTuneResources[track].packet == 0) {
        g_aTuneResources[track].packet =
            FetchDiskPacketRetrying("music.r00", (short)track, 0);
    }
    if (g_nMusicDriverMode == 2 &&
        g_aTuneResources[track].packet == 0) {
        g_aTuneResources[track].packet =
            FetchDiskPacketRetrying("music.a00", (short)track, 0);
    }
}

/* Function start: 0x45890E */
void ReleaseMusicTrack(int track)
{
    if (g_nMusicDriverMode == 1 ||
        g_nMusicDriverMode == 2) {
        if (g_98_nMemConfiguration == 1) {
            if (g_aTuneResources[track].packet ==
                g_aLimitedMusicBuffers[0].buffer) {
                g_aLimitedMusicBuffers[0].state = -1;
            } else if (g_aTuneResources[track].packet ==
                       g_aLimitedMusicBuffers[1].buffer) {
                g_aLimitedMusicBuffers[1].state = -1;
            } else {
                FreePacketAndClear(
                    &g_aTuneResources[track].packet, 0);
            }
        } else {
            FreePacketAndClear(
                &g_aTuneResources[track].packet, 0);
        }
    }
}

/* This unit's initialized data, after its functions so that it does not
 * move the functions' symbol keys. */
short g_nCockpitBackgroundBytes = 0; /* 0x0049c720 */
PacketResourceDescriptor g_aCommon3SpaceResources[11] = {
    { &g_aObjectTypeData[43].shapeSet, 0, 0 },
    { &g_aObjectTypeData[36].shapeSet, 0, 1 },
    { &g_aObjectTypeData[7].shapeSet, 0, 6 },
    { &g_aObjectTypeData[9].shapeSet, 0, 7 },
    { &g_aObjectTypeData[8].shapeSet, 0, 8 },
    { &g_aObjectTypeData[39].shapeSet, 0, 9 },
    { &g_aObjectTypeData[41].shapeSet, 0, 10 },
    { &g_aObjectTypeData[40].shapeSet, 0, 11 },
    { &g_aObjectTypeData[42].shapeSet, 0, 12 },
    { &g_aObjectTypeData[10].shapeSet, 0, 18 },
    { 0, 0, 0 }
}; /* 0x0049c728 */
PacketResourceDescriptor g_aMissionResourceDescriptors[8] = {
    { &g_aObjectTypeData[21].shapeSet, 0, 15 },
    { &g_aObjectTypeData[20].shapeSet, 0, 20 },
    { &g_aObjectTypeData[45].shapeSet, 0, 14 },
    { &g_aObjectTypeData[37].shapeSet, 0, 2 },
    { &g_aObjectTypeData[38].shapeSet, 0, 3 },
    { &g_aObjectTypeData[35].shapeSet, 0, 4 },
    { &g_aObjectTypeData[31].shapeSet, 0, 5 },
    { 0, 0, 0 }
}; /* 0x0049c798 */
PacketResourceDescriptor g_aHighMemoryCockpitResources[5] = {
    { &g_66_pCabinHudImage, 0, 0 },
    { &g_pPilotHandAnimationShapePtr, "pilotanm.vga", 3 },
    { (unsigned char **)&g_pCockpitExplosionShape, 0, 5 },
    { &g_18_pCockpitIndicatorShape, 0, 4 },
    { 0, 0, 0 }
}; /* 0x0049c7e8 */
PacketResourceDescriptor g_aCockpitCommonShapeResources[5] = {
    { &g_4_pCockpitPanelShape, 0, 10 },
    { &g_66_pCabinHudImage, "cockpit.vga", 0 },
    { &g_1_pDashWeaponPicture, 0, 12 },
    { &g_18_pCockpitIndicatorShape, "cockpit.vga", 4 },
    { 0, 0, 0 }
}; /* 0x0049c820 */
PacketResourceDescriptor g_aCockpitShipShapeResources[8] = {
    { &g_pPendingCockpitDamageShape, 0, 8 },
    { &g_4_pCockpitPanelShape, 0, 10 },
    { &g_1_pDashWeaponPicture, 0, 12 },
    { &g_2_apCockpitShipShapes[0], 0, 0 },
    { &g_2_apCockpitShipShapes[1], 0, 1 },
    { &g_2_apCockpitShipShapes[2], 0, 2 },
    { &g_2_apCockpitShipShapes[3], 0, 3 },
    { 0, 0, 0 }
}; /* 0x0049c858 */
PacketResourceDescriptor g_aCockpitViewShapeResources[5] = {
    { &g_2_apCockpitViewShapes[0], 0, 4 },
    { &g_2_apCockpitViewShapes[1], 0, 5 },
    { &g_2_apCockpitViewShapes[2], 0, 6 },
    { &g_2_apCockpitViewShapes[3], 0, 7 },
    { 0, 0, 0 }
}; /* 0x0049c8a8 */
int g_nUnusedFltsetupWord1 = 1; /* 0x0049c8dc */
short g_asConstellationObjectIndices[4] = {
    -1, -1, -1, -1
}; /* 0x0049c8e0 */
short g_bCockpitResourcesActive = 0; /* 0x0049c8e8 */
short g_b3SpaceObjectsActive = 0; /* 0x0049c8ec */
unsigned char *g_pGenericMissileShape = 0; /* 0x0049c8f0 */
unsigned char *g_pGenericMissileExhaustShape = 0; /* 0x0049c8f4 */

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
char g_7_szCockpitAssetFilenameText[16]; /* 0x005d1030 */
unsigned char *g_2_apCockpitViewShapes[4]; /* 0x005d1040 */
Viewport g_stCockpitBackgroundViewportRecord; /* 0x005d1050 */
unsigned char *g_2_apCockpitShipShapes[4]; /* 0x005d1070 */
int g_nUnusedFltsetupWord2[2]; /* 0x005d1080 */ /* never referenced */
unsigned char *g_pCockpitViewFrameInfo; /* 0x005d1088 */
