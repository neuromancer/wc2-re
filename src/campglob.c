/*
 *  Campaign globals and campaign scripts.
 *
 *  Retail object 0x428E00-0x42954F (link order; see docs/ORDER.md).
 */
#include "game.h"

static char g_szTemporaryCampaignGlobalsFile_00499bc0[] = "TEMPGLOB.000";

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
    globals->missionScore = g_nMissionScore_00493462;
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
           _itoa((int)campaignSlot, number, 10));
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
        bytes = (unsigned int)_filelength(handle);
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
