/*
 *  Campaign globals and campaign scripts.
 *
 *  Retail object 0x428E00-0x42954F (link order; see docs/ORDER.md).
 */
#include "game.h"

static char g_szTemporaryCampaignGlobalsFile_00499bc0[13];

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
        state++;
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
void StoreMissionResultsInCampaignGlobals(CampaignGlobals *globals)
{
    short index;
    short pilotTotal;

    if (globals == 0) {
        globals = AllocateScenePointerTable(
            0x50, 2, 0, "ModuleToScript");
    }
    pilotTotal = globals->pilotCount;
    for (index = 0; index < pilotTotal; index++)
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
        pilotTotal = GetInitialMissionShipPilotSaveIndex(index);
        if (pilotTotal != -1)
            pilotTotal = (short)(pilotTotal * 2);
        globals->shipPilotIndices[index] = pilotTotal;
    }
    globals->missionScore = g_stCurrentPilotProfile_00493408.missionScore;
    FreePacketAndClear(&g_pPilotStatus_005d2fcc, 0x80);
}

/* Function start: 0x42917D */
void InitializeCampaignConstellationState(CampaignGlobals *globals,
                                           short copyPosition)
{
    short pilot;
    short count;

    count = globals->pilotCount;
    if (g_pPilotStatus_005d2fcc == 0)
        g_pPilotStatus_005d2fcc =
            (short *)calloc((int)count, 2);
    if (g_pPilotStatus_005d2fcc == 0)
        ReportFatalErrorCode("017");
    if (count != 32)
        ReportFatalErrorCode("018");

    for (pilot = 0; pilot < count; pilot++)
        g_pPilotStatus_005d2fcc[pilot] =
            globals->pilotStatus[pilot];

    if (copyPosition != 0) {
        g_stCurrentPilotProfile_00493408.series = globals->series;
        g_stCurrentPilotProfile_00493408.mission = globals->mission;
    }
    g_nArcadeState_0049d75c = (int)globals->arcadeState;
}

#pragma function(strcat)
/* Function start: 0x429261 */
void RunCampaignScript(short campaignSlot)
{
    void *scenePacket;
    unsigned int totalMain;
    unsigned int largestMainBlock;
    short font;
    char number[4];
    short inputMode;
    char campaignFileName[13];
    unsigned int totalFar;
    unsigned int largestFarMemoryBlock;

    inputMode = g_nUiInputMode_005c8d3c;
    memcpy(campaignFileName, "campaign.s0", 12);
    campaignFileName[12] = 0;
    if (g_bSkipCampaignVideo_0049c270 != 0)
        return;
    SetMenuInputPump();
    strcat(campaignFileName,
           itoa((int)campaignSlot, number, 10));
    scenePacket = FetchDiskPacketRetrying(
        campaignFileName, 0, 0);
    for (font = 0; font < 4; font++) {
        if (g_apTextFonts_005d2200[font] != 0) {
            ReleasePacketHandle(g_apTextFonts_005d2200[font]);
            g_apTextFonts_005d2200[font] = 0;
        }
    }
    totalMain = GetAvailableMainMemory();
    largestMainBlock = GetLargestMainMemoryBlock();
    totalFar = GetAvailableFarMemory();
    largestFarMemoryBlock = GetLargestFreeMemoryBlock();
#ifdef SDL_PORT
    if (g_bCutsceneViewportPreallocated_00499c4c == 0)
#else
    /* This unit reads the byte flag as a word. */
    if (*(short *)&g_bCutsceneViewportPreallocated_00499c4c == 0)
#endif
        ClearViewport(&g_stModalSourceViewport_005d2c50, 0);
    if (g_bSkipCampaignScenes_0049cc78 == 0) {
        g_bSceneEscapeRequested_0049d4b0 = 0;
        ParseCutsceneContainer(scenePacket);
        g_bSceneDisplayUpdateActive_00499bb8 = 1;
        DisableMouseCursorDrawing();
        g_nUiInputMode_005c8d3c = 0;
        RunLoadedCutscene();
        g_nUiInputMode_005c8d3c = inputMode;
        g_bSceneDisplayUpdateActive_00499bb8 = 0;
        g_bSceneEscapeRequested_0049d4b0 = 0;
    }
    totalMain = GetAvailableMainMemory();
    largestMainBlock = GetLargestMainMemoryBlock();
    totalFar = GetAvailableFarMemory();
    largestFarMemoryBlock = GetLargestFreeMemoryBlock();
    ReleasePacketHandle(scenePacket);
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

    if (g_pCampaignGlobals_00499c94 == 0)
        return;
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

/* Function start: 0x42949F */
void *LoadWholeDataFile(const char *path)
{
    void *data;
    short handle;
    unsigned int bytes;

    data = 0;
    handle = OpenDataFileOrDie(path);
    if (handle >= 0) {
        bytes = (unsigned int)filelength(handle);
        if (bytes == (unsigned int)-1) {
            CloseDataFile((unsigned short)handle);
            return 0;
        }
        data = AllocateTaggedMemory(bytes, 0);
        if (data != 0)
            ReadDataFileAtOffset((unsigned short)handle, 0, bytes, data);
        CloseDataFile((unsigned short)handle);
    }
    return data;
}

/* This unit's initialized data, after its functions so that the animation
 * tables do not move the functions' symbol keys.  The object-type records and
 * their animation streams precede the campaign file name in the original. */
/* Frame lists the object-type records point at; 0xA000 and 0x9000 end a
 * sequence.  They sit immediately below the table in the retail image. */
static short g_asExplosionSmallAnimation_00496af0[8] = {
    0, 1, 2, 3, 4, 5, 6, (short)0xa000
};
static short g_asExplosionMediumAnimation_00496b00[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, (short)0xa000, 0, 0
};
static short g_asExplosionLargeAnimation_00496b20[20] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    (short)0xa000, 0, 0
};
static short g_asLaserSparkAnimation_00496b48[8] = {
    0, 1, 2, 3, 4, 5, (short)0xa000, 0
};
static short g_asBlueSparkAnimation_00496b58[8] = {
    0, 1, 2, 3, (short)0xa000, 0, 0, 0
};
static short g_asRedSparkAnimation_00496b68[8] = {
    0, 1, 2, 3, (short)0xa000, 0, 0, 0
};
static short g_asSparkTrailAnimation_00496b78[8] = {
    0, 1, 2, 3, (short)0xa000, 0, 0, 0
};
static short g_asGirderChunkAnimation_00496b88[8] = {
    0, 1, 2, 3, 4, 5, (short)0x9000, 0
};
static short g_asShipTubingAnimation_00496b98[8] = {
    6, 7, 8, 9, 10, 11, (short)0x9000, 0
};
static short g_asBurningDebrisAnimation_00496ba8[20] = {
    12, 13, 14, 15, 16, 17, 18, 19, 146, 145, 144, 143, 142, 141, 140,
    20, (short)0x9000, 0, 0, 0
};
static short g_asORingDebrisAnimation_00496bd0[8] = {
    21, 22, 23, 24, 25, 26, (short)0x9000, 0
};
static short g_asPipeDebrisAnimation_00496be0[8] = {
    27, 28, 29, 30, 31, 32, (short)0x9000, 0
};
static short g_asMetalSheetAnimation_00496bf0[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, (short)0x9000, 0, 0
};
static short g_asDataCapsuleAnimation_00496c10[12] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, (short)0x9000
};
static short g_asMineAnimation_00496c28[12] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, (short)0x9000
};
static short g_asChaffPodAnimation_00496c40[20] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, (short)0x9000, 0, 1, 2, 3, 4, 5,
    (short)0x9000, 0
};
static short g_asAsteroidSpinAnimation_00496c68[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, (short)0x9000, 0, 0
};
static short g_asAsteroidRollAnimation_00496c88[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, (short)0x9000, 0, 0
};
static short g_asAsteroidSpinReverseAnimation_00496ca8[16] = {
    12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, (short)0x9000, 0, 0
};
static short g_asAsteroidRollReverseAnimation_00496cc8[16] = {
    12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, (short)0x9000, 0, 0
};
static short g_asEjectedPilotAnimation_00496ce8[12] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, (short)0x9000, 0
};
static short g_asShipDebrisFlashAnimation_00496d00[24] = {
    0, 1026, 1028, 1028, 1032, 1032, 1032, 1040, 1040, 1040, 1040, 2080,
    2080, 2064, 2056, 2052, 1, 2052, 2052, 2052, 2052, (short)0xa000, 0,
    0
};

/* WC2 keeps its own 49-record table: slots 0-6 are the ship types the game
 * loads at runtime and stay zero in the image, and 7-48 are the static
 * projectile, missile, asteroid, debris and explosion records read straight
 * out of the retail .data. */
ObjectTypeData g_aObjectTypeData_00496d30[OBJECT_DATA_COUNT] = {
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    {
        "Laser cannon", OBJECT_CLASS_PROJECTILE, 0, 7, 10, 0, 512, 7,
        30, 25, 0, 160
    },
    {
        "Neutron gun", OBJECT_CLASS_PROJECTILE, 0, 8, 10, 1, 832, 14,
        20, 40, 0, 140
    },
    {
        "Mass driver", OBJECT_CLASS_PROJECTILE, 0, 9, 10, 0, 512, 9, 25, 30, 0, 120
    },
    {
        "Particle cannon", OBJECT_CLASS_PROJECTILE, 0, 10, 10, 0, 512,
        9, 30, 53, 0, 220
    },
    { "", OBJECT_CLASS_PROJECTILE, 0, 11, 10, 0, 512, 7, 30, 25, 0, 160 },
    {
        "", OBJECT_CLASS_PROJECTILE, 0, 12, 15, 0, 832, 15, 40, 50,
        4000, 180
    },
    {
        "", OBJECT_CLASS_PROJECTILE, 0, 13, 1, 0, 832, 15, 100, 300,
        0, 250
    },
    {
        "", OBJECT_CLASS_PROJECTILE, 0, 14, 1, 0, 832, 15, 80, 1200,
        0, 150
    },
    {
        "Dart DF", OBJECT_CLASS_MISSILE, 0, 15, 20, 5, 768, 500, 120,
        4, 14500, 150, 0, 0, 1433, 15, 15, 15, 100
    },
    {
        "Javelin HS", OBJECT_CLASS_MISSILE, 0, 16, 20, 5, 768, 400, 140, 4, 13500, 150, 0, 0, 1689, 11, 11, 11, 100
    },
    {
        "Pilum FF", OBJECT_CLASS_MISSILE, 0, 17, 20, 5, 768, 400, 160, 4, 10500, 150, 0, 0, 1689, 11, 11, 11, 100
    },
    {
        "Spiculum IR", OBJECT_CLASS_MISSILE, 0, 18, 20, 5, 768, 400,
        110, 4, 11500, 150, 0, 0, 1689, 11, 11, 11, 100
    },
    {
        "Torpedo", OBJECT_CLASS_MISSILE, 0, 19, 55, 10, 1280, 400, 8200, 20, 30000, 50, 0, 0, 512, 4, 4, 4, 500
    },
    {
        "Chaff Pod", OBJECT_CLASS_MISSILE, 0, 20, 50, 1, 768, 400, 160, 4, 1000, 90, 0, (unsigned char
        *)g_asChaffPodAnimation_00496c40, 1689, 11, 11, 11, 100
    },
    {
        "Porcupine", OBJECT_CLASS_MINE, 0, 21, 20, 5, 768, 110, 120,
        4, 20000, 20, 20, (unsigned char *)g_asMineAnimation_00496c28,
        0, 0, 2, 2
    },
    {
        "", OBJECT_CLASS_ASTEROID, 0, 22, 100, 300, 1024, 0, 0, -1,
        0, 0, 0, (unsigned char *)g_asAsteroidSpinAnimation_00496c68, 0,
        0, 1, 13
    },
    {
        "", OBJECT_CLASS_ASTEROID, 0, 23, 100, 300, 1024, 0, 0, -1,
        0, 0, 0, (unsigned char *)g_asAsteroidRollAnimation_00496c88, 0,
        0, 1, 12
    },
    {
        "", OBJECT_CLASS_ASTEROID, 0, 24, 100, 300, 1024, 0, 0, -1,
        0, 0, 0, (unsigned char
        *)g_asAsteroidSpinReverseAnimation_00496ca8, 0, 0, 1, 13
    },
    {
        "", OBJECT_CLASS_ASTEROID, 0, 25, 100, 300, 1024, 0, 0, -1,
        0, 0, 0, (unsigned char
        *)g_asAsteroidRollReverseAnimation_00496cc8, 0, 0, 1, 12
    },
    {
        "", OBJECT_CLASS_ASTEROID, 0, 26, 100, 300, 1024, 0, 0, -1,
        0, 0, 0, (unsigned char *)g_asAsteroidSpinAnimation_00496c68, 0,
        0, 2, 13
    },
    {
        "", OBJECT_CLASS_ASTEROID, 0, 27, 100, 300, 1024, 0, 0, -1,
        0, 0, 0, (unsigned char *)g_asAsteroidRollAnimation_00496c88, 0,
        0, 2, 12
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 28, 10, 4, 192, 0, 0, -1, 0, 0,
        0, (unsigned char *)g_asAsteroidSpinAnimation_00496c68, 0, 0, 2,
        13
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 29, 10, 1, 2048, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asGirderChunkAnimation_00496b88, 0, 0, 2,
        5
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 30, 10, 1, 2048, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asShipTubingAnimation_00496b98, 0, 0, 2, 5
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 31, 20, 2, 1280, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asMetalSheetAnimation_00496bf0, 0, 0, 1,
        11
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 32, 20, 2, 1280, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asDataCapsuleAnimation_00496c10, 0, 0, 1,
        2
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 33, 20, 2, 768, 0, 0, 0, 0, 0, 0,
        (unsigned char *)g_asBurningDebrisAnimation_00496ba8, 0, 0, 1,
        15
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 34, 2, 1, 1792, 0, 0, 0, 0, 0, 0,
        (unsigned char *)g_asORingDebrisAnimation_00496bd0, 0, 0, 1, 5
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 35, 6, 1, 1536, 0, 0, 0, 0, 0, 0,
        (unsigned char *)g_asPipeDebrisAnimation_00496be0, 0, 0, 1, 5
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 36, 0, 0, 768, 0, 0, -1, 6000,
        0, 0, (unsigned char *)g_asExplosionSmallAnimation_00496af0, 0,
        0, 1
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 37, 0, 0, 1024, 0, 0, -1,
        6000, 0, 0, (unsigned char
        *)g_asExplosionMediumAnimation_00496b00, 0, 0, 1
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 38, 0, 0, 336, 0, 0, -1, 6000,
        0, 0, (unsigned char *)g_asExplosionLargeAnimation_00496b20, 0,
        0, 1
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 39, 0, 0, 256, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asLaserSparkAnimation_00496b48, 0, 0, 1
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 40, 0, 0, 256, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asRedSparkAnimation_00496b68, 0, 0, 2, 3
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 41, 0, 0, 256, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asBlueSparkAnimation_00496b58, 0, 0, 2, 3
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 42, 1, 1, 256, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asSparkTrailAnimation_00496b78, 0, 0, 2, 3
    },
    { "", OBJECT_CLASS_FIXED_OBJECT, 0, 43, 0, 0, 256 },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 44, 6, 10, 512, 0, 0, 0, 0, 0, 0,
        (unsigned char *)g_asEjectedPilotAnimation_00496ce8, 0, 0, 1, 12
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 45, 0, 0, 1024, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asShipDebrisFlashAnimation_00496d00, 0, 0,
        1
    },
    {
        "data capsule", OBJECT_CLASS_DEBRIS, 0, 61, 1, 10, 1024, 0, 0, 0, 0, 0, 0, (unsigned char
        *)g_asDataCapsuleAnimation_00496c10, 0, 0, 1
    },
    {
        "", OBJECT_CLASS_FIXED_OBJECT, 0, 62, 0, 0, 192, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1
    },
    {
        "", OBJECT_CLASS_FIXED_OBJECT, 0, 63, 0, 0, 192, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1
    }
};
int g_nUnusedObjectDataWord_00499bb4 = 0;   /* no references in the original */
int g_bSceneDisplayUpdateActive_00499bb8 = 0;
static char g_szTemporaryCampaignGlobalsFile_00499bc0[13] = "TEMPGLOB.000";
