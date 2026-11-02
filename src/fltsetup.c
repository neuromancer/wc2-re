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
        g_szCockpitResourceFilename_005d1030, (section)))
#else
#define COCKPIT_VIEW_COPY_BYTES(section) \
    ((unsigned short)g_nCockpitBackgroundBytes_0049c720)
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

#pragma function(strcat, strcpy)
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
                    (DwordPtr)g_apCockpitViewShapes_005d1040[0]);
            DosMemcpy(g_pCockpitBackgroundPacket_0049a5f0,
                      g_pCockpitViewFrameData_005d1088,
                      COCKPIT_VIEW_COPY_BYTES(4));
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
                    (DwordPtr)g_apCockpitViewShapes_005d1040[1]);
            DosMemcpy(g_pCockpitBackgroundPacket_0049a5f0,
                      g_pCockpitViewFrameData_005d1088,
                      COCKPIT_VIEW_COPY_BYTES(5));
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
                        (DwordPtr)g_apCockpitViewShapes_005d1040[1]);
                DosMemcpy(
                    g_pCockpitBackgroundPacket_0049a5f0,
                    g_pCockpitViewFrameData_005d1088,
                    COCKPIT_VIEW_COPY_BYTES(5));
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
                        (DwordPtr)g_apCockpitViewShapes_005d1040[2]);
                DosMemcpy(
                    g_pCockpitBackgroundPacket_0049a5f0,
                    g_pCockpitViewFrameData_005d1088,
                    COCKPIT_VIEW_COPY_BYTES(6));
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
                    (DwordPtr)g_apCockpitViewShapes_005d1040[3]);
            DosMemcpy(g_pCockpitBackgroundPacket_0049a5f0,
                      g_pCockpitViewFrameData_005d1088,
                      COCKPIT_VIEW_COPY_BYTES(7));
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
/* WC2 passes the player's ship type; the body reads it from the global
 * instead and ignores the argument. */
void InitializeCockpitResources(short shipType)
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
    g_nHazardPitchRange_0049af98 = layout.field_f7;
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
#ifdef SDL_PORT
    /* See issue #30: LoadShapeSet aborts the whole list on the first
     * missing resource, and g_pCockpitWeaponShape_005d2b54 (the ship's
     * cockpit weapon-display shape, section 12 of the ship-specific
     * pcshipNN.v file) is legitimately absent for ship types that have
     * no cockpit weapon display -- code elsewhere already null-checks it
     * before use. But that abort also skips every later entry in the
     * same list, including g_pCockpitIndicatorShape_005d2c48 (the target
     * VDU's shield-strength indicator sprite, cockpit.vga section 4),
     * which has nothing to do with the ship-specific failure and is
     * always present. The result: any ship without a cockpit weapon
     * display never shows shield strength on a locked target, even at
     * full shields. Load it directly if the set above left it null. */
    if (g_pCockpitIndicatorShape_005d2c48 == 0) {
        g_pCockpitIndicatorShape_005d2c48 =
            FetchDiskPacketRetrying("cockpit.vga", 4, 0x40);
    }
#endif
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

#pragma function(strlen)
/* Function start: 0x458196 */
void free_cockpit(void)
{
    char shipTypeText[4];
    short packet;

    if (g_bCockpitResourcesActive_0049c8e8 == 0)
        return;
    else
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
    for (packet = 0; packet < 50; packet++) {
        if (g_apCommunicationTextPackets_005d17c0[packet] != 0) {
            ReleasePacketHandle(
                g_apCommunicationTextPackets_005d17c0[packet]);
            g_apCommunicationTextPackets_005d17c0[packet] = 0;
        }
    }
}

/* Function start: 0x458467 */
void init_3Space_objects(short scene)
{
    short slot;

    if (g_b3SpaceObjectsActive_0049c8ec == 1)
        return;
    else
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
    else
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
    g_aObjectTypeData_00496d30[OBJECT_DATA_PILUM_FF].shapeSet =
        g_aObjectTypeData_00496d30[OBJECT_DATA_SPICULUM_IR].shapeSet =
            g_aObjectTypeData_00496d30[OBJECT_DATA_DART_DF].shapeSet = 0;
    g_aObjectTypeData_00496d30[OBJECT_DATA_PILUM_FF].animation =
        g_aObjectTypeData_00496d30[OBJECT_DATA_SPICULUM_IR].animation =
            g_aObjectTypeData_00496d30[OBJECT_DATA_DART_DF].animation = 0;
    g_aObjectTypeData_00496d30[OBJECT_DATA_GIRDER_CHUNK].shapeSet =
        g_aObjectTypeData_00496d30[OBJECT_DATA_SHIP_TUBING].shapeSet =
            g_aObjectTypeData_00496d30[
                OBJECT_DATA_BURNING_DEBRIS].shapeSet =
                g_aObjectTypeData_00496d30[OBJECT_DATA_O_RING].shapeSet =
                    g_aObjectTypeData_00496d30[
                        OBJECT_DATA_PIPE].shapeSet;
    FreePacketAndClear(
        &g_aObjectTypeData_00496d30[OBJECT_DATA_EJECTED_PILOT].shapeSet,
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
