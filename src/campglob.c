/*
 *  Campaign globals and campaign scripts.
 *
 *  Retail object 0x428E00-0x42954F (link order; see docs/ORDER.md).
 */
#include "game.h"

static char g_szTemporaryCampaignGlobalsFile[13]; /* 0x00499bc0 */

/* Function start: 0x428E00 */
short GetMissionShipPilotSaveIndex(short missionShip)
{
    short pilot;

    pilot = g_aMissionShips[missionShip].pilot;
    if (pilot < 6)
        pilot = 99;
    else
        pilot = g_aMissionShips[missionShip].portrait;
    return pilot;
}

/* Function start: 0x428E5C */
short GetMissionShipVelocityState(short missionShip)
{
    short state;

    state = -1;
    if (g_aMissionShips[missionShip].state == 3)
        state++;
    return state;
}

/* Function start: 0x428E98 */
short GetInitialMissionShipPilotSaveIndex(short initialShip)
{
    short pilot;

    pilot = g_13_stMissionHeaderBlock.initialMissionShips[initialShip];
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
        globals->pilotStatus[index] = g_pPilotStatus[index];

    for (index = 0; index < 8; index++) {
        /* Preserve retail's achieved/sighted/visited call order in CL 4.1;
         * the complements fold away without changing the flags. */
        globals->objectiveFlags[index] =
            ~~(achieved(index) | sighted(index)) | visited(index);
        globals->objectiveSighted[index] = sighted(index);
    }
    for (index = 0; index < 16; index++)
        globals->shipVelocityState[index] =
            (short)(GetMissionShipVelocityState(index) + 1);

    globals->shipKillCounts[0] = g_36_cPilotKillCountValue;
    globals->shipMissionFlags[0] = 0;
    globals->damageLevel = (short)calculate_damage_level();
    globals->arcadeState = (short)g_nArcadeState;
    if (g_nArcadeState == 2)
        globals->shipMissionFlags[0] |= 2;
    if (g_asShipSide[0] == 1)
        globals->shipMissionFlags[0] |= 4;
    if (g_nArcadeState >= 3)
        globals->shipMissionFlags[0] |= 1;

    for (index = 1; index < 8; index++) {
        globals->shipKillCounts[index] =
            g_4_acInitialShipKillNumber[index - 1];
        globals->shipMissionFlags[index] =
            g_acMissionShipStatusFlagsBytes[index - 1];
        if (index == 1 && g_6_bMissionWingmanState != 0)
            globals->shipMissionFlags[index] |= 8;
        pilotTotal = GetInitialMissionShipPilotSaveIndex(index);
        if (pilotTotal != -1)
            pilotTotal = (short)(pilotTotal * 2);
        globals->shipPilotIndices[index] = pilotTotal;
    }
    globals->missionScore = g_stCurrentPilotProfile.missionScore;
    FreePacketAndClear(&g_pPilotStatus, 0x80);
}

/* Function start: 0x42917D */
void InitializeCampaignConstellationState(CampaignGlobals *globals,
                                           short copyPosition)
{
    short pilot;
    short count;

    count = globals->pilotCount;
    if (g_pPilotStatus == 0)
        g_pPilotStatus =
            (short *)calloc((int)count, 2);
    if (g_pPilotStatus == 0)
        ReportFatalErrorCode("017");
    if (count != 32)
        ReportFatalErrorCode("018");

    for (pilot = 0; pilot < count; pilot++)
        g_pPilotStatus[pilot] =
            globals->pilotStatus[pilot];

    if (copyPosition != 0) {
        g_stCurrentPilotProfile.series = globals->series;
        g_stCurrentPilotProfile.mission = globals->mission;
    }
    g_nArcadeState = (int)globals->arcadeState;
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

    inputMode = g_18_nInterfaceEventMode;
    memcpy(campaignFileName, "campaign.s0", 12);
    campaignFileName[12] = 0;
    if (g_bSkipCampaignVideo != 0)
        return;
    SetMenuInputPump();
    strcat(campaignFileName,
           itoa((int)campaignSlot, number, 10));
    scenePacket = FetchDiskPacketRetrying(
        campaignFileName, 0, 0);
    for (font = 0; font < 4; font++) {
        if (g_36_apTextGlyphs[font] != 0) {
            ReleasePacketHandle(g_36_apTextGlyphs[font]);
            g_36_apTextGlyphs[font] = 0;
        }
    }
    totalMain = GetAvailableMainMemory();
    largestMainBlock = GetLargestMainMemoryBlock();
    totalFar = GetAvailableFarMemory();
    largestFarMemoryBlock = GetLargestFreeMemoryBlock();
#ifdef SDL_PORT
    if (g_bCutsceneViewportPreallocated == 0)
#else
    /* This unit reads the byte flag as a word. */
    if (*(short *)&g_bCutsceneViewportPreallocated == 0)
#endif
        ClearViewport(&g_2_stModalSourceWindow, 0);
    if (g_bSkipCampaignScenes == 0) {
        g_bSceneEscapeRequested = 0;
        ParseCutsceneContainer(scenePacket);
        g_bSceneDisplayUpdateActive = 1;
        DisableMouseCursorDrawing();
        g_18_nInterfaceEventMode = 0;
        RunLoadedCutscene();
        g_18_nInterfaceEventMode = inputMode;
        g_bSceneDisplayUpdateActive = 0;
        g_bSceneEscapeRequested = 0;
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
    ReleasePacketSlot(&g_pCampaignGlobals);
    g_pCampaignGlobals =
        LoadWholeDataFile(g_szTemporaryCampaignGlobalsFile);
}

/* Function start: 0x429423 */
void SaveAndFreeTemporaryCampaignGlobals(void)
{
    short file;

    if (g_pCampaignGlobals == 0)
        return;
    file = CreateDataFile(g_szTemporaryCampaignGlobalsFile);
    if (file >= 0) {
        WriteDataFileAtOffset(
            (unsigned short)file, 0,
            (unsigned int)*(unsigned short *)g_pCampaignGlobals *
                2,
            g_pCampaignGlobals);
        CloseDataFile((unsigned short)file);
    }
    ReleasePacketSlot(&g_pCampaignGlobals);
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
static short g_asExplosionSmallAnimation[8] = {
    0, 1, 2, 3, 4, 5, 6, (short)0xa000
}; /* 0x00496af0 */
static short g_asExplosionMediumAnimation[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, (short)0xa000, 0, 0
}; /* 0x00496b00 */
static short g_asExplosionLargeAnimation[20] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    (short)0xa000, 0, 0
}; /* 0x00496b20 */
static short g_asLaserSparkAnimation[8] = {
    0, 1, 2, 3, 4, 5, (short)0xa000, 0
}; /* 0x00496b48 */
static short g_asBlueSparkAnimation[8] = {
    0, 1, 2, 3, (short)0xa000, 0, 0, 0
}; /* 0x00496b58 */
static short g_asRedSparkAnimation[8] = {
    0, 1, 2, 3, (short)0xa000, 0, 0, 0
}; /* 0x00496b68 */
static short g_asSparkTrailAnimation[8] = {
    0, 1, 2, 3, (short)0xa000, 0, 0, 0
}; /* 0x00496b78 */
static short g_asGirderChunkAnimation[8] = {
    0, 1, 2, 3, 4, 5, (short)0x9000, 0
}; /* 0x00496b88 */
static short g_asShipTubingAnimation[8] = {
    6, 7, 8, 9, 10, 11, (short)0x9000, 0
}; /* 0x00496b98 */
static short g_asBurningDebrisAnimation[20] = {
    12, 13, 14, 15, 16, 17, 18, 19, 146, 145, 144, 143, 142, 141, 140,
    20, (short)0x9000, 0, 0, 0
}; /* 0x00496ba8 */
static short g_asORingDebrisAnimation[8] = {
    21, 22, 23, 24, 25, 26, (short)0x9000, 0
}; /* 0x00496bd0 */
static short g_asPipeDebrisAnimation[8] = {
    27, 28, 29, 30, 31, 32, (short)0x9000, 0
}; /* 0x00496be0 */
static short g_asMetalSheetAnimation[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, (short)0x9000, 0, 0
}; /* 0x00496bf0 */
static short g_asDataCapsuleAnimation[12] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, (short)0x9000
}; /* 0x00496c10 */
static short g_asMineAnimation[12] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, (short)0x9000
}; /* 0x00496c28 */
static short g_asChaffPodAnimation[20] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, (short)0x9000, 0, 1, 2, 3, 4, 5,
    (short)0x9000, 0
}; /* 0x00496c40 */
static short g_asAsteroidSpinAnimation[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, (short)0x9000, 0, 0
}; /* 0x00496c68 */
static short g_asAsteroidRollAnimation[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, (short)0x9000, 0, 0
}; /* 0x00496c88 */
static short g_asAsteroidSpinReverseAnimation[16] = {
    12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, (short)0x9000, 0, 0
}; /* 0x00496ca8 */
static short g_asAsteroidRollReverseAnimation[16] = {
    12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, (short)0x9000, 0, 0
}; /* 0x00496cc8 */
static short g_asEjectedPilotAnimation[12] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, (short)0x9000, 0
}; /* 0x00496ce8 */
static short g_asShipDebrisFlashAnimation[24] = {
    0, 1026, 1028, 1028, 1032, 1032, 1032, 1040, 1040, 1040, 1040, 2080,
    2080, 2064, 2056, 2052, 1, 2052, 2052, 2052, 2052, (short)0xa000, 0,
    0
}; /* 0x00496d00 */

/* WC2 keeps its own 49-record table: slots 0-6 are the ship types the game
 * loads at runtime and stay zero in the image, and 7-48 are the static
 * projectile, missile, asteroid, debris and explosion records read straight
 * out of the retail .data. */
ObjectTypeData g_aObjectTypeData[OBJECT_DATA_COUNT] = {
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
        *)g_asChaffPodAnimation, 1689, 11, 11, 11, 100
    },
    {
        "Porcupine", OBJECT_CLASS_MINE, 0, 21, 20, 5, 768, 110, 120,
        4, 20000, 20, 20, (unsigned char *)g_asMineAnimation,
        0, 0, 2, 2
    },
    {
        "", OBJECT_CLASS_ASTEROID, 0, 22, 100, 300, 1024, 0, 0, -1,
        0, 0, 0, (unsigned char *)g_asAsteroidSpinAnimation, 0,
        0, 1, 13
    },
    {
        "", OBJECT_CLASS_ASTEROID, 0, 23, 100, 300, 1024, 0, 0, -1,
        0, 0, 0, (unsigned char *)g_asAsteroidRollAnimation, 0,
        0, 1, 12
    },
    {
        "", OBJECT_CLASS_ASTEROID, 0, 24, 100, 300, 1024, 0, 0, -1,
        0, 0, 0, (unsigned char
        *)g_asAsteroidSpinReverseAnimation, 0, 0, 1, 13
    },
    {
        "", OBJECT_CLASS_ASTEROID, 0, 25, 100, 300, 1024, 0, 0, -1,
        0, 0, 0, (unsigned char
        *)g_asAsteroidRollReverseAnimation, 0, 0, 1, 12
    },
    {
        "", OBJECT_CLASS_ASTEROID, 0, 26, 100, 300, 1024, 0, 0, -1,
        0, 0, 0, (unsigned char *)g_asAsteroidSpinAnimation, 0,
        0, 2, 13
    },
    {
        "", OBJECT_CLASS_ASTEROID, 0, 27, 100, 300, 1024, 0, 0, -1,
        0, 0, 0, (unsigned char *)g_asAsteroidRollAnimation, 0,
        0, 2, 12
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 28, 10, 4, 192, 0, 0, -1, 0, 0,
        0, (unsigned char *)g_asAsteroidSpinAnimation, 0, 0, 2,
        13
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 29, 10, 1, 2048, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asGirderChunkAnimation, 0, 0, 2,
        5
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 30, 10, 1, 2048, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asShipTubingAnimation, 0, 0, 2, 5
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 31, 20, 2, 1280, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asMetalSheetAnimation, 0, 0, 1,
        11
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 32, 20, 2, 1280, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asDataCapsuleAnimation, 0, 0, 1,
        2
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 33, 20, 2, 768, 0, 0, 0, 0, 0, 0,
        (unsigned char *)g_asBurningDebrisAnimation, 0, 0, 1,
        15
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 34, 2, 1, 1792, 0, 0, 0, 0, 0, 0,
        (unsigned char *)g_asORingDebrisAnimation, 0, 0, 1, 5
    },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 35, 6, 1, 1536, 0, 0, 0, 0, 0, 0,
        (unsigned char *)g_asPipeDebrisAnimation, 0, 0, 1, 5
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 36, 0, 0, 768, 0, 0, -1, 6000,
        0, 0, (unsigned char *)g_asExplosionSmallAnimation, 0,
        0, 1
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 37, 0, 0, 1024, 0, 0, -1,
        6000, 0, 0, (unsigned char
        *)g_asExplosionMediumAnimation, 0, 0, 1
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 38, 0, 0, 336, 0, 0, -1, 6000,
        0, 0, (unsigned char *)g_asExplosionLargeAnimation, 0,
        0, 1
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 39, 0, 0, 256, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asLaserSparkAnimation, 0, 0, 1
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 40, 0, 0, 256, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asRedSparkAnimation, 0, 0, 2, 3
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 41, 0, 0, 256, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asBlueSparkAnimation, 0, 0, 2, 3
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 42, 1, 1, 256, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asSparkTrailAnimation, 0, 0, 2, 3
    },
    { "", OBJECT_CLASS_FIXED_OBJECT, 0, 43, 0, 0, 256 },
    {
        "", OBJECT_CLASS_DEBRIS, 0, 44, 6, 10, 512, 0, 0, 0, 0, 0, 0,
        (unsigned char *)g_asEjectedPilotAnimation, 0, 0, 1, 12
    },
    {
        "", OBJECT_CLASS_EXPLOSION, 0, 45, 0, 0, 1024, 0, 0, 0, 0, 0,
        0, (unsigned char *)g_asShipDebrisFlashAnimation, 0, 0,
        1
    },
    {
        "data capsule", OBJECT_CLASS_DEBRIS, 0, 61, 1, 10, 1024, 0, 0, 0, 0, 0, 0, (unsigned char
        *)g_asDataCapsuleAnimation, 0, 0, 1
    },
    {
        "", OBJECT_CLASS_FIXED_OBJECT, 0, 62, 0, 0, 192, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1
    },
    {
        "", OBJECT_CLASS_FIXED_OBJECT, 0, 63, 0, 0, 192, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1
    }
}; /* 0x00496d30 */
int g_nUnusedObjectDataWord = 0; /* 0x00499bb4 */   /* no references in the original */
int g_bSceneDisplayUpdateActive = 0; /* 0x00499bb8 */
static char g_szTemporaryCampaignGlobalsFile[13] = "TEMPGLOB.000"; /* 0x00499bc0 */

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short *g_pPilotStatus; /* 0x005d2fcc */
