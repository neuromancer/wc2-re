/*
 *  Barracks and campaign save/load room.
 *
 *  Address range 0x41ada0-0x41c75f (provisional -- see docs/ORDER.md).
 *  Boundary evidence: contiguous SAVEGAME.WLD helpers ending at BarracksScreen;
 *  the Win32 debug-overlay compilation unit begins at 0x41c760.
 */
#include "wc1.h"

/* Function start: WC2_UNMAPPED */
void CreateEmptySaveGameFile(void)
{
    SaveGameDiskRecord diskRecord;
    SaveGameRecord gameRecord;
    SaveGameDiskObjective *diskObjective;
    MissionObjective *gameObjective;
    unsigned char *diskBytes;
    unsigned char *gameBytes;
    short *diskDeathMission;
    int *gameDeathMission;
    short file;
    short slot;
    int index;

    slot = 0;
    gameRecord.occupied = 0;
    PromptInsertNumberedDisk(0x10);
    file = CreateDataFile("SAVEGAME.WLD");
    if (file >= 0) {
        for (; slot < 8; slot++) {
            sprintf(gameRecord.description, "game %d", (int)slot + 1);
            memcpy(diskRecord.description, gameRecord.description,
                   sizeof(diskRecord.description));
            diskRecord.occupied = gameRecord.occupied;
            memcpy(diskRecord.pilots, gameRecord.pilots,
                   sizeof(diskRecord.pilots));
            diskRecord.campaign.currentPilot =
                (short)(int)gameRecord.campaign.currentPilot;
            diskRecord.campaign.playerShipType =
                (short)gameRecord.campaign.playerShipType;

            diskBytes = diskRecord.campaign.medals;
            gameBytes = gameRecord.campaign.medals;
            for (index = 0; index < 5; index++) {
                diskBytes[index] = gameBytes[index];
            }

            diskBytes = diskRecord.campaign.badges;
            gameBytes = gameRecord.campaign.badges;
            for (index = 0; index < 12; index++) {
                diskBytes[index] = gameBytes[index];
            }

            diskRecord.campaign.currentMission =
                gameRecord.campaign.currentMission;
            diskRecord.campaign.currentSeries =
                gameRecord.campaign.currentSeries;
            diskRecord.campaign.seriesHistoryCount =
                gameRecord.campaign.seriesHistoryCount;
            diskBytes = (unsigned char *)
                diskRecord.campaign.seriesHistory;
            gameBytes = (unsigned char *)
                gameRecord.campaign.seriesHistory;
            for (index = 0; index < 8; index++) {
                diskBytes[index] = gameBytes[index];
            }

            gameDeathMission =
                gameRecord.campaign.personalityDeathMission;
            diskDeathMission =
                diskRecord.campaign.personalityDeathMission;
            for (; diskDeathMission <
                       diskRecord.campaign.personalityDeathMission + 8;
                 diskDeathMission++, gameDeathMission++) {
                *diskDeathMission = (short)*gameDeathMission;
            }

            diskBytes = diskRecord.campaign.aceFlags;
            gameBytes = gameRecord.campaign.aceFlags;
            for (index = 0; index < 4; index++) {
                diskBytes[index] = gameBytes[index];
            }
            diskRecord.campaign.currentDate =
                gameRecord.campaign.currentDate;
            diskRecord.campaign.elapsedDate.day =
                gameRecord.campaign.elapsedDate.day;
            diskRecord.campaign.elapsedDate.year =
                gameRecord.campaign.elapsedDate.year;
            diskRecord.campaign.promotionScore =
                gameRecord.campaign.promotionScore;
            diskRecord.campaign.missionScore =
                gameRecord.campaign.missionScore;
            diskRecord.campaign.seriesScore =
                gameRecord.campaign.seriesScore;
            diskRecord.campaign.campaignIndex =
                gameRecord.campaign.campaignIndex;

            gameObjective = gameRecord.objectives;
            diskObjective = diskRecord.objectives;
            for (; diskObjective < diskRecord.objectives + 16;
                 diskObjective++, gameObjective++) {
                diskObjective->mapX = gameObjective->mapX;
                diskObjective->mapY = gameObjective->mapY;
                diskObjective->field_4 = gameObjective->field_4;
                diskObjective->type = (short)gameObjective->type;
                diskObjective->index = gameObjective->index;
                diskObjective->flags = gameObjective->flags;
                diskObjective->displayName =
                    (short)(int)gameObjective->displayName;
                diskObjective->name = (short)(int)gameObjective->name;
                diskObjective->position = gameObjective->position;
            }

            WriteDataFileAtOffset((unsigned short)file,
                                  (int)slot * 0x33c, 0x33c,
                                  &diskRecord);
        }
        CloseDataFile((unsigned short)file);
    }
}

/* Function start: WC2_UNMAPPED */
void EnsureSaveGameFile(void)
{
    short file;
    long length;

    PromptInsertNumberedDisk(0x10);
    file = OpenDataFileOrDie("SAVEGAME.WLD");
    if (file < 0) {
        CreateEmptySaveGameFile();
        return;
    }
    length = _filelength((int)file);
    CloseDataFile((unsigned short)file);
    if (length != 0x19e0)
        CreateEmptySaveGameFile();
}

/* Function start: WC2_UNMAPPED */
void InitializeBarracksAnimation(BarracksAnimationState *state)
{
    short bunk;

    bunk = 0;
    do {
        state->bunks[bunk].animationFrame =
            RandomInRange(0, 13);
        state->bunks[bunk].animationPeriod =
            (short)(RandomInRange(0, 12) + 13);
        state->bunks[bunk].animationTick = 0;
        bunk++;
    } while (bunk < 8);
    state->fallingY = -99;
    state->fallingDelay = 20;
    state->impactFrame = 49;
    state->blinkDelay = 0;
    state->fallingVelocity = 0;
    state->animationTick = 0;
    state->menuLabel = 0;
    state->eyesOpen = 1;
}

/* Function start: WC2_UNMAPPED */
void FreeBarracksMenuLabel(char **label)
{
    if (*label != 0 &&
        *label != g_apszSaveCampaignMenuLabels_004693e8_WC1_UNMAPPED[0] &&
        *label != g_apszSaveCampaignMenuLabels_004693e8_WC1_UNMAPPED[1]) {
        ReleasePacketHandle(*label);
        *label = 0;
    }
}

/* Function start: WC2_UNMAPPED */
void SetAwakenBarracksMenuLabel(char **label, int series, int mission,
                                char *description)
{
    FreeBarracksMenuLabel(label);
#ifdef WC1_SDL
    /* MSVC 4.20 accepts %Fs as its legacy far-string conversion. */
    sprintf(g_szTextScratchBuffer_005d1c40, "Awaken %s.", description);
#else
    sprintf(g_szTextScratchBuffer_005d1c40, "Awaken %Fs.", description);
#endif
    *label = AllocateTaggedMemory(
        strlen(g_szTextScratchBuffer_005d1c40) + 2, 0);
    DosMemcpy(*label, g_szTextScratchBuffer_005d1c40,
              strlen(g_szTextScratchBuffer_005d1c40) + 2);
}

/* Function start: WC2_UNMAPPED */
void FreeBarracksMenuLabels(void)
{
    const char *saveCampaignLabel;
    short bunk;

    saveCampaignLabel = g_apszSaveCampaignMenuLabels_004693e8_WC1_UNMAPPED[0];
    bunk = 0;
    do {
        if (g_apszBarracksMenuLabels_004693f0_WC1_UNMAPPED[bunk * 2] !=
            saveCampaignLabel) {
            FreeBarracksMenuLabel(
                &g_apszBarracksMenuLabels_004693f0_WC1_UNMAPPED[bunk * 2]);
            FreeBarracksMenuLabel(
                &g_apszBarracksMenuLabels_004693f0_WC1_UNMAPPED[bunk * 2 + 1]);
            saveCampaignLabel =
                g_apszSaveCampaignMenuLabels_004693e8_WC1_UNMAPPED[0];
        }
        bunk++;
    } while (bunk < 8);
    g_pszCurrentRoomMenuLabel_00598aba = 0;
}

/* Function start: WC2_UNMAPPED */
short SaveGame(short slot, SaveGameRecord *gameRecord)
{
    SaveGameDiskRecord diskRecord;
    SaveGameDiskObjective *diskObjective;
    MissionObjective *gameObjective;
    unsigned char *diskBytes;
    short *diskDeathMission;
    int *gameDeathMission;
    short file;
    short written;
    int index;

    PromptInsertNumberedDisk(0x10);
    memcpy(diskRecord.description, gameRecord->description,
           sizeof(diskRecord.description));
    diskRecord.occupied = gameRecord->occupied;
    memcpy(diskRecord.pilots, gameRecord->pilots,
           sizeof(diskRecord.pilots));
    diskRecord.campaign.currentPilot =
        (short)(int)gameRecord->campaign.currentPilot;
    diskRecord.campaign.playerShipType =
        (short)gameRecord->campaign.playerShipType;
    diskBytes = diskRecord.campaign.medals;
    for (index = 0; index < 5; index++) {
        diskBytes[index] = gameRecord->campaign.medals[index];
    }

    diskBytes = diskRecord.campaign.badges;
    for (index = 0; index < 12; index++) {
        diskBytes[index] = gameRecord->campaign.badges[index];
    }
    diskRecord.campaign.currentMission =
        gameRecord->campaign.currentMission;
    diskRecord.campaign.currentSeries =
        gameRecord->campaign.currentSeries;
    diskRecord.campaign.seriesHistoryCount =
        gameRecord->campaign.seriesHistoryCount;
    diskBytes = (unsigned char *)diskRecord.campaign.seriesHistory;
    for (index = 0; index < 8; index++) {
        diskBytes[index] =
            (unsigned char)gameRecord->campaign.seriesHistory[index];
    }

    gameDeathMission = gameRecord->campaign.personalityDeathMission;
    diskDeathMission = diskRecord.campaign.personalityDeathMission;
    for (; diskDeathMission <
               diskRecord.campaign.personalityDeathMission + 8;
         diskDeathMission++, gameDeathMission++) {
        *diskDeathMission = (short)*gameDeathMission;
    }

    diskBytes = diskRecord.campaign.aceFlags;
    for (index = 0; index < 4; index++) {
        diskBytes[index] = gameRecord->campaign.aceFlags[index];
    }
    diskRecord.campaign.currentDate = gameRecord->campaign.currentDate;
    diskRecord.campaign.elapsedDate.day =
        gameRecord->campaign.elapsedDate.day;
    diskRecord.campaign.elapsedDate.year =
        gameRecord->campaign.elapsedDate.year;
    diskRecord.campaign.promotionScore =
        gameRecord->campaign.promotionScore;
    diskRecord.campaign.missionScore = gameRecord->campaign.missionScore;
    diskRecord.campaign.seriesScore = gameRecord->campaign.seriesScore;
    diskRecord.campaign.campaignIndex = gameRecord->campaign.campaignIndex;

    gameObjective = gameRecord->objectives;
    diskObjective = diskRecord.objectives;
    for (; diskObjective < diskRecord.objectives + 16;
         diskObjective++, gameObjective++) {
        diskObjective->mapX = gameObjective->mapX;
        diskObjective->mapY = gameObjective->mapY;
        diskObjective->field_4 = gameObjective->field_4;
        diskObjective->type = (short)gameObjective->type;
        diskObjective->index = gameObjective->index;
        diskObjective->flags = gameObjective->flags;
        diskObjective->displayName =
            (short)(int)gameObjective->displayName;
        diskObjective->name = (short)(int)gameObjective->name;
        diskObjective->position = gameObjective->position;
    }

    file = OpenDataFileOrDie("SAVEGAME.WLD");
    written = WriteDataFileAtOffset((unsigned short)file,
                                    (int)slot * 0x33c, 0x33c,
                                    &diskRecord);
    CloseDataFile((unsigned short)file);
    return (written != 0) & (file >= 0);
}

/* Function start: WC2_UNMAPPED */
short RunWc1TextInputPrompt(short x, short y, const char *prompt,
                            char *destination, short maximumLength,
                            short inputMode)
{
    ModalTextPanel panel;
    volatile unsigned int bounds[2];
    short promptWidth;
    short result;
    int widestCharacter;

    result = 0;
    ((short *)&bounds[0])[0] = x;
    ((short *)&bounds[0])[1] = y;
    ((short *)&bounds[1])[1] = (short)(y + 20);
    InitializeModalTextPanel(&panel, 0,
                             g_dwModalBoundsTopLeft_0049ca48,
                             g_dwModalBoundsBottomRight_0049ca4c,
                             g_cSecondaryViewBufferColour_0049cb4c, g_cSecondaryViewBufferColour_0049cb4c,
                             g_cSecondaryViewBufferColour_0049cb4c);
    widestCharacter = MeasureTextPixelWidthClamped("M");
    widestCharacter *= (int)maximumLength;
    promptWidth = MeasureTextPixelWidthClamped(prompt);
    widestCharacter += (int)promptWidth;
    ((short *)&bounds[1])[0] = (short)(x +
        (widestCharacter * 16) / 15);
    RestoreModalTextPanel(&panel);
    if (InitializeModalTextPanel(&panel, 0,
                                 (unsigned int)bounds[0],
                                 (unsigned int)bounds[1],
                                 g_bPrimaryViewBufferColour_0049cb50,
                                 g_abGamePaletteReservedColours_0049cb54[0],
                                 g_abGamePaletteReservedColours_0049cb54[8]) != 0) {
        DrawModalTextPanel(&panel, 3, 6, 0, prompt);
        MarkDibDirty();
        DIBslamReal();
        if (ReadTextInput(destination, maximumLength, inputMode, 0) != 0)
            result = 1;
        RestoreModalTextPanel(&panel);
    }
    return result;
}

/* Function start: WC2_UNMAPPED */
int WarnWc1LoadGameFirst(void)
{
    InputEventState event;
    short key;

    SuspendWc1MouseCursor();
    key = 0;
    if (ShowModalTextPanel(0, "Load a game first.") != 0) {
        while (PollInputEvent(&event) != 0) {
        }
        key = WaitForInputKey();
        while (PollInputEvent(&event) != 0) {
        }
        ReleaseModalTextPanel();
    }
    ResumeMouseCursorHook();
    return (int)key;
}

/* Function start: WC2_UNMAPPED */
void SaveGameWithNamePrompt(short slot, CampaignState *campaign,
                            PilotRecord *pilots,
                            MissionObjective *objectives)
{
    SaveGameRecord gameRecord;
    char oldLabel[40];
    char *separator;

    if (DAT_004688f0_WC1_UNMAPPED == 0) {
        WarnWc1LoadGameFirst();
        return;
    }
    DosStrcpy(oldLabel,
              g_apszBarracksMenuLabels_004693f0_WC1_UNMAPPED[slot * 2]);
    if (strcmp(oldLabel,
               g_apszSaveCampaignMenuLabels_004693e8_WC1_UNMAPPED[0]) == 0)
        oldLabel[0] = 0;
    separator = DosStrchr(oldLabel, ' ');
    if (separator != 0)
        DosStrcpy(oldLabel, separator + 1);
    separator = DosStrchr(oldLabel, '.');
    if (separator != 0)
        *separator = 0;
    DosStrcpy(gameRecord.description, oldLabel);
    if (RunWc1TextInputPrompt(40, 24, "Game Name: ",
                              gameRecord.description, 16, 1) != 0) {
        gameRecord.occupied = 1;
        memcpy(&gameRecord.campaign, campaign,
               sizeof(gameRecord.campaign));
        memcpy(gameRecord.pilots, pilots,
               sizeof(gameRecord.pilots));
        DosMemcpy(gameRecord.objectives, objectives, 0x1f0);
        if (SaveGame(slot, &gameRecord) == 0)
            ShowModalMessage("Error: Game %s not saved.",
                             gameRecord.description);
    }
}

/* Function start: WC2_UNMAPPED */
int LoadGame(short slot, SaveGameRecord *gameRecord)
{
    SaveGameDiskRecord diskRecord;
    SaveGameDiskObjective *diskObjective;
    MissionObjective *gameObjective;
    unsigned char *diskBytes;
    unsigned char *gameBytes;
    short *diskDeathMission;
    int *gameDeathMission;
    short file;
    short fileOpen;
    int index;
    int read;

    PromptInsertNumberedDisk(0x10);
    file = OpenDataFileOrDie("SAVEGAME.WLD");
    fileOpen = file >= 0;
    read = ReadDataFileAtOffset((unsigned short)file,
                                (int)slot * 0x33c, 0x33c,
                                &diskRecord);
    memcpy(gameRecord->description, diskRecord.description,
           sizeof(gameRecord->description));
    gameRecord->occupied = diskRecord.occupied;
    memcpy(gameRecord->pilots, diskRecord.pilots,
           sizeof(gameRecord->pilots));
    gameRecord->campaign.currentPilot =
        (PilotRecord *)(int)diskRecord.campaign.currentPilot;
    gameRecord->campaign.playerShipType =
        (enum ObjectType)diskRecord.campaign.playerShipType;
    gameBytes = gameRecord->campaign.medals;
    diskBytes = diskRecord.campaign.medals;
    for (index = 0; index < 5; index++) {
        gameBytes[index] = diskBytes[index];
    }

    gameBytes = gameRecord->campaign.badges;
    diskBytes = diskRecord.campaign.badges;
    for (index = 0; index < 12; index++) {
        gameBytes[index] = diskBytes[index];
    }
    gameRecord->campaign.currentMission =
        diskRecord.campaign.currentMission;
    gameRecord->campaign.currentSeries =
        diskRecord.campaign.currentSeries;
    gameRecord->campaign.seriesHistoryCount =
        diskRecord.campaign.seriesHistoryCount;
    gameBytes = (unsigned char *)gameRecord->campaign.seriesHistory;
    diskBytes = (unsigned char *)diskRecord.campaign.seriesHistory;
    for (index = 0; index < 8; index++) {
        gameBytes[index] = diskBytes[index];
    }

    gameDeathMission = gameRecord->campaign.personalityDeathMission;
    diskDeathMission = diskRecord.campaign.personalityDeathMission;
    for (; diskDeathMission <
               diskRecord.campaign.personalityDeathMission + 8;
         diskDeathMission++, gameDeathMission++) {
        *gameDeathMission = (int)*diskDeathMission;
    }

    gameBytes = gameRecord->campaign.aceFlags;
    diskBytes = diskRecord.campaign.aceFlags;
    for (index = 0; index < 4; index++) {
        gameBytes[index] = diskBytes[index];
    }
    gameRecord->campaign.currentDate = diskRecord.campaign.currentDate;
    gameRecord->campaign.elapsedDate.day =
        diskRecord.campaign.elapsedDate.day;
    gameRecord->campaign.elapsedDate.year =
        diskRecord.campaign.elapsedDate.year;
    gameRecord->campaign.promotionScore =
        diskRecord.campaign.promotionScore;
    gameRecord->campaign.missionScore = diskRecord.campaign.missionScore;
    gameRecord->campaign.seriesScore = diskRecord.campaign.seriesScore;
    gameRecord->campaign.campaignIndex =
        diskRecord.campaign.campaignIndex;

    gameObjective = gameRecord->objectives;
    diskObjective = diskRecord.objectives;
    for (; diskObjective < diskRecord.objectives + 16;
         diskObjective++, gameObjective++) {
        gameObjective->mapX = diskObjective->mapX;
        gameObjective->mapY = diskObjective->mapY;
        gameObjective->field_4 = diskObjective->field_4;
        gameObjective->type = (int)diskObjective->type;
        gameObjective->index = diskObjective->index;
        gameObjective->flags = diskObjective->flags;
        gameObjective->displayName =
            (const char *)(int)diskObjective->displayName;
        gameObjective->name = (char *)(int)diskObjective->name;
        gameObjective->position = diskObjective->position;
    }

    fileOpen &= read != 0;
    CloseDataFile((unsigned short)file);
    if (fileOpen != 0 && gameRecord->occupied != 0)
        return 1;
    return 0;
}

/* Function start: WC2_UNMAPPED */
void LoadGameFromSlot(short slot, CampaignState *campaign,
                      PilotRecord *pilots,
                      MissionObjective *objectives)
{
    SaveGameRecord gameRecord;
    int loaded;

    SuspendWc1MouseCursor();
    if (ShowModalTextPanel(0, "Loading Game...") != 0) {
        loaded = LoadGame(slot, &gameRecord);
        if (loaded != 0) {
            memcpy(pilots, gameRecord.pilots,
                   sizeof(gameRecord.pilots));
            memcpy(campaign, &gameRecord.campaign,
                   sizeof(gameRecord.campaign));
            DAT_005a8114 = campaign->campaignIndex;
            g_nCampaignDataSet_005a8118 = DAT_005a8114;
            LoadWc1PacketIntoBuffer(
                g_asCampaignPilotFiles_00469450_WC1_UNMAPPED[DAT_005a8114], 0,
                g_pConstellationDefinitions_00598a28);
            LoadWc1PacketIntoBuffer(
                g_asCampaignPilotFiles_00469450_WC1_UNMAPPED[
                    g_nCampaignDataSet_005a8118],
                1, g_pMissionCampaignData_005988bc);
            DosMemcpy(objectives, gameRecord.objectives, 0x1f0);
            CorrectPointers();
            DAT_004688f0_WC1_UNMAPPED = 1;
        }
        ReleaseModalTextPanel();
        if (loaded == 0) {
            ShowModalMessage("Error: Game %d not loaded.", (int)slot);
        } else if (g_nOriginDevUnlock_0049d774 != 0) {
            memcpy(campaign->currentPilot->callsign, "CHEATER", 8);
        }
    }
    ResumeMouseCursorHook();
}

/* Function start: WC2_UNMAPPED */
void SetBunkMenuLabel(short occupied, short side, char **label,
                      int series, int mission, char *description)
{
    if (occupied == 0) {
        *label = (char *)g_apszSaveCampaignMenuLabels_004693e8_WC1_UNMAPPED[0];
        return;
    }
    if (side == 0) {
        SetAwakenBarracksMenuLabel(label, series, mission, description);
        return;
    }
    *label = (char *)g_apszSaveCampaignMenuLabels_004693e8_WC1_UNMAPPED[1];
}

/* Function start: WC2_UNMAPPED */
void GetBunkInfo(BarracksAnimationState *state)
{
    SaveGameRecord gameRecord;
    short bunk;

    FreeBarracksMenuLabels();
    bunk = 0;
    do {
        state->bunks[bunk].occupied =
            (short)LoadGame(bunk, &gameRecord);
        SetBunkMenuLabel(
            state->bunks[bunk].occupied, 0,
            &g_apszBarracksMenuLabels_004693f0_WC1_UNMAPPED[bunk * 2],
            (int)gameRecord.campaign.currentSeries,
            (int)gameRecord.campaign.currentMission,
            gameRecord.description);
        SetBunkMenuLabel(
            state->bunks[bunk].occupied, 1,
            &g_apszBarracksMenuLabels_004693f0_WC1_UNMAPPED[bunk * 2 + 1],
            (int)gameRecord.campaign.currentSeries,
            (int)gameRecord.campaign.currentMission,
            gameRecord.pilots[0].name + 6);
        bunk++;
    } while (bunk < 8);
}

/* Function start: 0x46138D */
void DrawUnreferencedPilotHandFrame(Viewport *viewport, unsigned char *shape,
                       BarracksAnimationState *state)
{
    short x;
    short y;

    x = (short)(g_stPilotHandOrigin_0049af90.x -
                g_stCockpitViewport_005d2160.left);
    y = (short)(g_stPilotHandOrigin_0049af90.y -
                g_stCockpitViewport_005d2160.top);
    CopyViewportContents(&g_stPilotHandBackgroundViewport_005d2b40,
                         &g_stPilotHandViewport_005d2c70);
    DrawSpriteDefault(&g_stPilotHandViewport_005d2c70, x, y,
                      g_pPilotHandAnimationShape_005d2c64,
                      (short)(signed char)g_cPilotHandFrame_005d1c30);
    DrawSpriteDefault(
        &g_stPilotHandViewport_005d2c70,
        (short)(x + g_asPilotHandOffsets_0049aff8[
            (signed char)g_cPilotHandFrame_005d1c30 * 2]),
        (short)(y + g_asPilotHandOffsets_0049aff8[
            (signed char)g_cPilotHandFrame_005d1c30 * 2 + 1]),
        g_pPilotHandAnimationShape_005d2c64, 0x11);
    CopyViewportContents(&g_stPilotHandViewport_005d2c70,
                         &g_stCockpitViewport_005d2160);
    g_cLastPilotHandFrame_0049aff4 = g_cPilotHandFrame_005d1c30;
}

/* Function start: WC2_UNMAPPED */
void DrawBarracksStaticDetails(Viewport *viewport,
                               unsigned char *shape)
{
    DrawSpriteDefault(viewport, 147, 167, shape, 25);
    CheckCursor();
    DrawSpriteDefault(viewport, 304, 144, shape, 36);
    CheckCursor();
}

/* Function start: WC2_UNMAPPED */
void AnimateWc1Barracks(Viewport *viewport, unsigned char *shape,
                        BarracksAnimationState *state)
{
    BarracksBunkState *bunkState;
    int frameTick;
    int bunkTick;
    short bunk;
    short frame;

    frameTick = (int)DAT_0059ab54 / 3;
    for (bunk = 0; bunk < 8; bunk++) {
        frame = 11;
        bunkState = &state->bunks[bunk];
        if (bunkState->occupied != 0) {
            bunkTick = frameTick / 4;
            if (bunkState->animationTick != bunkTick) {
                bunkState->animationTick = bunkTick;
                bunkState->animationFrame++;
                if (bunkState->animationTick %
                        (int)bunkState->animationPeriod == 0)
                    bunkState->animationFrame = 0;
            }
            frame = 24;
            if (bunkState->animationFrame < 13)
                frame = (short)(24 - bunkState->animationFrame);
        }
        DrawSpriteDefault(viewport,
                          (short)((bunk % 2) * 14 + 148),
                          (short)((bunk / 2) * 5 + 166),
                          shape, frame);
    }

    if ((int)state->animationTick != frameTick) {
        state->animationTick = (short)frameTick;
        state->fallingDelay--;
        if (state->fallingDelay == 0) {
            state->fallingY = -5;
            state->fallingVelocity = 3;
        }
        if (state->fallingY != -99) {
            state->fallingY += state->fallingVelocity;
            state->fallingVelocity++;
            if (state->fallingY > 115) {
                state->fallingY = -99;
                state->impactFrame = 37;
                state->fallingDelay = 20;
                PlaySfxWaveFileByNumber(35, -1, 0);
            }
        }
    }
    if (state->fallingY != -99) {
        DrawSpriteDefault(viewport, 298, (short)state->fallingY,
                          shape,
                          (short)(state->fallingY / 40 + 27));
        DrawSpriteDefault(viewport, 305, 46, shape, 35);
    }
    frame = state->impactFrame;
    if (frame != 49) {
        state->impactFrame++;
        DrawSpriteDefault(viewport, 298, 139, shape, frame);
    }

    if (state->blinkDelay != 0) {
        state->eyesOpen = (short)(state->eyesOpen == 0);
        state->blinkDelay--;
        if (state->blinkDelay == 0 &&
            (unsigned short)RandomInRange(0, 100) < 90)
            state->eyesOpen = 1;
    } else if (RandomInRange(0, 70) == 0) {
        state->blinkDelay = (short)(RandomInRange(0, 15) + 2);
    }
    if (state->eyesOpen != 0)
        frame = 49;
    else
        frame = 26;
    DrawSpriteDefault(viewport, 45, 0, shape, frame);
    CheckCursor();
    if (state->menuLabel != g_pszCurrentRoomMenuLabel_00598aba) {
        state->menuLabel = g_pszCurrentRoomMenuLabel_00598aba;
        DrawSpriteDefault(viewport, 319, 199, shape, 50);
    }
}

/* Function start: WC2_UNMAPPED */
int ConfirmWc1QuitWingCommander(void)
{
    short confirmed;

    SuspendWc1MouseCursor();
    confirmed = 0;
    if (ShowModalTextPanel(0, "Quit Wing Commander? (Y/N)") != 0) {
        confirmed = (short)((short)toupper(
            WaitForStreamInputKey()) == 'Y');
        ReleaseModalTextPanel();
    }
    ResumeMouseCursorHook();
    return (int)confirmed;
}

/* Function start: WC2_UNMAPPED */
int ConfirmWc1AwakenAfterBadData(short slot)
{
    SaveGameRecord gameRecord;
    short confirmed;

    confirmed = 0;
    if (LoadGame(slot, &gameRecord) == 0)
        ShowModalMessage("Error: data may be bad.");
    SuspendWc1MouseCursor();
    if (ShowModalTextPanel(0, "Awaken %s? (Y/N)",
                           gameRecord.description) != 0) {
        confirmed = (short)((short)toupper(
            WaitForStreamInputKey()) == 'Y');
        ReleaseModalTextPanel();
    }
    ResumeMouseCursorHook();
    return (int)confirmed;
}

/* Function start: WC2_UNMAPPED */
int ConfirmWc1ReplaceFaultyData(short slot)
{
    SaveGameRecord gameRecord;
    short confirmed;

    confirmed = 0;
    if (DAT_004688f0_WC1_UNMAPPED == 0) {
        WarnWc1LoadGameFirst();
        return 0;
    }
    if (LoadGame(slot, &gameRecord) == 0)
        memcpy(gameRecord.description, "FAULTY DATA", 12);
    SuspendWc1MouseCursor();
    if (ShowModalTextPanel(0, "Replace %s? (Y/N)",
                           gameRecord.description) != 0) {
        confirmed = (short)((short)toupper(
            WaitForStreamInputKey()) == 'Y');
        ReleaseModalTextPanel();
    }
    ResumeMouseCursorHook();
    return (int)confirmed;
}

/* Function start: WC2_UNMAPPED */
void HandleWc1BarracksBunkSelection(Viewport *viewport,
                                     unsigned char *shape,
                                     BarracksAnimationState *state,
                                     short region)
{
    short slot;

    SuspendWc1MouseCursor();
    slot = (short)(region / 2);
    if (state->bunks[slot].occupied != 0) {
        if (region % 2 == 0) {
            if (ConfirmWc1AwakenAfterBadData(slot) != 0) {
                LoadGameFromSlot(
                    slot, &g_stCampaignState_0059ca50,
                    g_aPilotRecords_005988d0,
                    g_aMissionObjectives_004932a8);
            }
            goto refresh;
        }
        if (ConfirmWc1ReplaceFaultyData(slot) == 0)
            goto refresh;
    }
    SaveGameWithNamePrompt(slot, &g_stCampaignState_0059ca50,
                           g_aPilotRecords_005988d0,
                           g_aMissionObjectives_004932a8);

refresh:
    GetBunkInfo(state);
    DrawUnreferencedPilotHandFrame(viewport, shape, state);
    ResumeMouseCursorHook();
}

/* Function start: 0x418DAA */
short RunCampaignSelectionFrame(void *scene, void *field)
{
    short selection;

    selection = 0;
    do {
        selection = PollSceneHotspotInput(scene, 0, 0, 0, 0);
        if (selection == 0) {
            DisableMouseCursorDrawing();
            DrawSpriteDefault(
                &g_stSecondaryViewBuffer_005d2c90, 0, 0, field, 0);
            DrawConstellationField();
            DrawSpriteDefault(
                &g_stSecondaryViewBuffer_005d2c90, 0, 0, scene, 0);
            if (g_pszPersonnelFooter_00492658 != 0) {
                g_stDefaultTextContext_005d2d20.viewport =
                    &g_stSecondaryViewBuffer_005d2c90;
                InitializeTextContextFromFont(
                    &g_stDefaultTextContext_005d2d20, 1,
                    g_bPrimaryViewBufferColour_0049cb50, -1);
                SetTextCursor(
                    (unsigned short)((320 -
                        MeasureTextPixelWidthClamped(
                            g_pszPersonnelFooter_00492658)) >> 1),
                    180);
                DrawFormattedText("%S", g_pszPersonnelFooter_00492658);
                g_stDefaultTextContext_005d2d20.viewport =
                    &g_stScreenViewport_005d21a0;
            }
            EnableMouseCursorDrawing();
            WaitForVerticalBlankThunk();
            CopyViewportContents(&g_stSecondaryViewBuffer_005d2c90,
                                 &g_stScreenViewport_005d21a0);
        }
    } while (selection == 0);
    if (g_bSceneEscapeRequested_0049d4b0 != 0)
        selection = 0;
    return selection;
}

/* Function start: WC2_UNMAPPED */
void UpdateBarracksScreen(Viewport *viewport, unsigned char *shape,
                          BarracksAnimationState *state)
{
    DrawBarracksStaticDetails(viewport, shape);
    AnimateWc1Barracks(viewport, shape, state);
    RefreshRoomMenuLabel();
}

/* Function start: WC2_UNMAPPED */
short RunWc1BarracksScreen(void)
{
    InputEventState event;
    BarracksAnimationState animation;
    unsigned char *background;
    int lastMedalsTick;
    short clicked;
    short eventType;
    short region;
    short result;

    result = 0;
    lastMedalsTick = 0;
    PreloadMusicTrackHook(35);
    spacetrack(35, 2, 1);
    background = FetchDiskPacketRetrying(5, 12, 0);
    InitializeRoomMenu(g_aBarracksMenuRegions_00463008_WC1_UNMAPPED,
                       g_apszBarracksMenuLabels_004693f0_WC1_UNMAPPED,
                       &g_stRoomScreenViewport_005988a0,
                       g_szDefaultTextBuffer_005d2b80, 2);
    EnsureSaveGameFile();
    InitializeBarracksAnimation(&animation);
    GetBunkInfo(&animation);
    DrawUnreferencedPilotHandFrame(&g_stSecondaryViewBuffer_005d2c90, background, &animation);
    g_stMouseCursorState_0059ab10.viewport = &g_stRoomScreenViewport_005988a0;
    WarpWc1MouseTo(160, 100);
    ResumeMouseCursorHook();
    SetFrameTimerPeriodDirect(0);
    FlushInputEvents();
    g_nSavedRoomControllerX_005988b4 =
        g_nMenuInputRepeatDelay_005a8208;
    g_bInputMode_0059a848 = 1;
    g_nMenuPointerSpeed_00493054 = 1;
    g_nMenuInputRepeatDelay_005a8208 = 2;

    while (result == 0) {
        if (IsFrameTickElapsed() != 0) {
            UpdateBarracksScreen(&g_stRoomScreenViewport_005988a0,
                                 background, &animation);
            SetFrameTimerPeriodDirect(2);
        }
        eventType = PollInputEvent(&event);
        clicked = 0;
        if (eventType == 3 || eventType == 5) {
            ClearInputKeyStatePreservingModifiers();
            if ((short)event.value == 0x1c ||
                (short)event.value == 0x39)
                clicked = 1;
            else if ((short)event.value == 0x24)
                CalibrateJoystickInteractive();
            else
                MoveMenuPointerFromKeyboard(&event);
        } else if (eventType == 2 || eventType == 10) {
            clicked = 1;
        } else if (eventType == 13) {
            UpdateRoomMenuCursor();
        }

        if (clicked != 0) {
            region = FindMenuRegionAtPoint(
                event.x, event.y, g_aBarracksMenuRegions_00463008_WC1_UNMAPPED);
            if (region >= 0 && region < 16) {
                HandleWc1BarracksBunkSelection(
                    &g_stSecondaryViewBuffer_005d2c90, background, &animation, region);
            } else if (region == 16) {
                if (DAT_004688f0_WC1_UNMAPPED == 0)
                    WarnWc1LoadGameFirst();
                else
                    result = 7;
            } else if (region == 17) {
                if (DAT_004688f0_WC1_UNMAPPED == 0)
                    WarnWc1LoadGameFirst();
                else
                    result = 8;
            } else if (region == 18) {
                if (ConfirmWc1QuitWingCommander() != 0) {
                    ShutdownEventManager();
                    exit_squadron(
                        "You step out of the airlock and into...");
                }
            } else if (region == 19) {
                if (DAT_004688f0_WC1_UNMAPPED == 0) {
                    WarnWc1LoadGameFirst();
                } else {
                    LoadMissionData(
                        (short)g_stCampaignState_0059ca50.currentSeries,
                        (short)g_stCampaignState_0059ca50.currentMission);
                    FlushInputEvents();
                    if ((int)(DAT_0059ab54 - lastMedalsTick) >
                        g_nInputTickScale_005c8d24) {
                        SuspendWc1MouseCursor();
                        ClearViewport(&g_stRoomScreenViewport_005988a0,
                                      g_cSecondaryViewBufferColour_0049cb4c);
                        g_stSecondaryViewBuffer_005d2c90.bottom = 127;
                        g_stScreenViewport_005d21a0.top = 24;
                        g_stScreenViewport_005d21a0.bottom = 151;
                        ViewWc1Medals();
                        lastMedalsTick = (int)DAT_0059ab54;
                        ClearViewport(&g_stRoomScreenViewport_005988a0,
                                      g_cSecondaryViewBufferColour_0049cb4c);
                        g_stScreenViewport_005d21a0.top = 0;
                        g_stScreenViewport_005d21a0.bottom = 199;
                        g_stSecondaryViewBuffer_005d2c90.bottom = 199;
                        DrawUnreferencedPilotHandFrame(&g_stSecondaryViewBuffer_005d2c90, background,
                                          &animation);
                        ResumeMouseCursorHook();
                        UpdateBarracksScreen(
                            &g_stRoomScreenViewport_005988a0,
                            background, &animation);
                    }
                }
            }
        }
        ShowMemoryStatusDebug();
        MarkDibDirty();
        DIBslamReal();
    }

    SuspendWc1MouseCursor();
    g_nMenuPointerSpeed_00493054 = 2;
    g_nMenuInputRepeatDelay_005a8208 =
        g_nSavedRoomControllerX_005988b4;
    EventManagerHook(0);
    FreeBarracksMenuLabels();
    ReleasePacketHandle(background);
    ReleaseTextFont(0);
    free_viewport(&g_stSecondaryViewBuffer_005d2c90);
    StopMusicUnlessSuppressed();
    ReleaseMusicTrackHook(35);
    return result;
}

/* Function start: 0x419831 */
short BarracksScreen(void)
{
    signed char selection;
    void *campaignScene;
    void *field;

    selection = 0;
    campaignScene = 0;
    field = 0;
    DisableMouseCursorDrawing();
    ClearViewport(&g_stModalSourceViewport_005d2c50,
                  g_cSecondaryViewBufferColour_0049cb4c);
    g_stSecondaryViewBuffer_005d2c90.left = 0;
    g_stSecondaryViewBuffer_005d2c90.right = 319;
    g_stSecondaryViewBuffer_005d2c90.top = 0;
    g_stSecondaryViewBuffer_005d2c90.bottom = 199;
    if ((short)AllocateViewport(
            &g_stSecondaryViewBuffer_005d2c90,
            g_cSecondaryViewBufferColour_0049cb4c, 0) == 0) {
        ReportFatalErrorCode("033");
    }
    ClearViewport(&g_stSecondaryViewBuffer_005d2c90, 0);
    campaignScene = LoadNamedPacket(
        "campaign.vga", 0, 0, 0, 0, 1);
    if (campaignScene != 0 && WaitForSceneAdvance(campaignScene) != 0) {
        field = FetchDiskPacketRetrying("field.v00", 1, 0);
        init_constellation(0);
        InitializeConstellationField(
            &g_stSecondaryViewBuffer_005d2c90, 0, 16);
        SetMenuInputPump();
        SetInputViewport(&g_stSecondaryViewBuffer_005d2c90);
        SetPersonnelMousePosition(159, 99);
        EnableMouseCursorDrawing();
        while (selection == 0) {
            selection = (signed char)RunCampaignSelectionFrame(
                campaignScene, field);
        }
        FreePacketAndClear(&g_pConstellationShape_005d2c4c, 0);
        FreePacketAndClear(&campaignScene, 0);
        FreePacketAndClear(&field, 0);
        free_viewport(&g_stSecondaryViewBuffer_005d2c90);
        return (short)(selection - 1);
    }
    FreePacketAndClear(&campaignScene, 0);
    free_viewport(&g_stSecondaryViewBuffer_005d2c90);
    return 0;
}

/* Function start: 0x4225A0 */
short StepPaletteTransition(short *current,
                            const short *target,
                            short componentCount)
{
    short difference;
    short index;
    short previousCountdown;

    if (g_nPaletteTransitionInitialise_0049305c != 0) {
        g_pPaletteTransitionAccumulator_005d301c =
            AllocateTaggedMemory((unsigned int)(componentCount * 2), 0);
        g_pPaletteTransitionDelta_005d3014 =
            AllocateTaggedMemory((unsigned int)(componentCount * 2), 0);
        g_pPaletteTransitionDirection_005d3010 =
            AllocateTaggedMemory((unsigned int)(componentCount * 2), 0);
        if (g_pPaletteTransitionAccumulator_005d301c == 0 ||
            g_pPaletteTransitionDelta_005d3014 == 0 ||
            g_pPaletteTransitionDirection_005d3010 == 0) {
            if (g_pPaletteTransitionAccumulator_005d301c != 0)
                ReleasePacketHandle(g_pPaletteTransitionAccumulator_005d301c);
            if (g_pPaletteTransitionDelta_005d3014 != 0)
                ReleasePacketHandle(g_pPaletteTransitionDelta_005d3014);
            if (g_pPaletteTransitionDirection_005d3010 != 0)
                ReleasePacketHandle(g_pPaletteTransitionDirection_005d3010);
            return 0;
        }

        g_nPaletteTransitionMaxDelta_005d3018 = 0;
        for (index = 0; index < componentCount; index++) {
            difference = (short)(current[index] - target[index]);
            if (difference < 0) {
                difference = (short)-difference;
                g_pPaletteTransitionDirection_005d3010[index] = 1;
            } else {
                g_pPaletteTransitionDirection_005d3010[index] = -1;
            }
            g_pPaletteTransitionDelta_005d3014[index] = difference;
            if (g_nPaletteTransitionMaxDelta_005d3018 < difference)
                g_nPaletteTransitionMaxDelta_005d3018 = difference;
        }

        difference = (short)(g_nPaletteTransitionMaxDelta_005d3018 / 2);
        for (index = 0; index < componentCount; index++)
            g_pPaletteTransitionAccumulator_005d301c[index] = difference;
        g_nPaletteTransitionCountdown_005d3020 =
            g_nPaletteTransitionMaxDelta_005d3018;
        g_nPaletteTransitionInitialise_0049305c = 0;
    }

    previousCountdown = g_nPaletteTransitionCountdown_005d3020;
    g_nPaletteTransitionCountdown_005d3020--;
    if (previousCountdown == 0) {
        ReleasePacketHandle(g_pPaletteTransitionAccumulator_005d301c);
        ReleasePacketHandle(g_pPaletteTransitionDelta_005d3014);
        ReleasePacketHandle(g_pPaletteTransitionDirection_005d3010);
        g_nPaletteTransitionInitialise_0049305c = 1;
        return 0;
    }

    for (index = 0; index < componentCount; index++) {
        g_pPaletteTransitionAccumulator_005d301c[index] =
            (short)(g_pPaletteTransitionAccumulator_005d301c[index] +
                    g_pPaletteTransitionDelta_005d3014[index]);
        if (g_pPaletteTransitionAccumulator_005d301c[index] >
            g_nPaletteTransitionMaxDelta_005d3018) {
            g_pPaletteTransitionAccumulator_005d301c[index] =
                (short)(g_pPaletteTransitionAccumulator_005d301c[index] -
                        g_nPaletteTransitionMaxDelta_005d3018);
            current[index] =
                (short)(current[index] +
                        g_pPaletteTransitionDirection_005d3010[index]);
        }
    }
    return 1;
}

/* Function start: 0x446910 */
char *__stdcall DosStrcat(char *destination, const char *source)
{
    char *end;

    end = DosStrchr(destination, 0);
    DosStrcpy(end, source);
    return destination;
}
