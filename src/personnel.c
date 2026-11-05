/*
 *  Personnel database, campaign chalkboard and pilot transfer.
 *
 *  Retail object 0x433AD0-0x43775F (link order; see docs/ORDER.md).
 */
#include "game.h"

unsigned char *g_pCockpitBackgroundPacket_0049a5f0 = 0;
unsigned char *g_apCockpitVduOverlayShapes_0049a5f8[2] = { 0 };
int g_nLegacySaveSource_0049a600 = 0;
const char *g_apszPilotRankNames_0049a608[5] = {
    "2ND LT.",
    "1ST LT.",
    "CAPTAIN",
    "MAJOR",
    "LT. COL."
};
const char *g_apszShortPilotRankNames_0049a620[5] = {
    "2ND LT",
    "1ST LT",
    "CAPT.",
    "MAJOR",
    "LT COL"
};

/* The save-slot signature is a 0x24-byte record, not a 34-byte string: the
 * original writes 36 bytes from it, the string and the two NULs after it.
 * Spelling the object at its record width keeps the write inside it. */
static char g_szSaveSlotSignature_0049a638[0x24] =
    "Wing Commander II (c) 1991 Origin";

PersonnelFileSlot *g_apPersonnelFileSlots_0049a660[8] = { 0 };
PilotProfile *g_apPersonnelPilotProfiles_0049a680[8] = { 0 };
short g_nPersonnelMenuHighlight_0049a6a0 = -1;
void *g_pPersonnelCampaignSnapshot_0049a6a4 = 0;
static char g_szPersonnelFileName_0049a6a8[] = "savegame.wc2";
unsigned char *g_pPersonnelMenuBackdrop_0049a6b8 = 0;
short g_sPersonnelPreviousInput_0049a6bc = -128;
short g_bPersonnelMenuDrawing_0049a6c0 = 1;
const char *g_apszSaveGameFileNames_0049a6c8[3] = {
    "savegame.wld",
    "savegame.wld",
    "crusade.wld"
};
signed char g_bPersonnelPrimaryInputDown_0049a6d4 = 0;

/* Keyboard scan codes of the personnel menus' hot keys and confirm keys. */
enum PersonnelMenuKey {
    PERSONNEL_KEY_0 = 0x0b,         /* file slot 0 */
    PERSONNEL_KEY_W = 0x11,         /* [W]ing Commander */
    PERSONNEL_KEY_T = 0x14,         /* [T]ransfer */
    PERSONNEL_KEY_ENTER = 0x1c,
    PERSONNEL_KEY_S = 0x1f,         /* [S]ave */
    PERSONNEL_KEY_D = 0x20,         /* [D]elete, [D]os version */
    PERSONNEL_KEY_L = 0x26,         /* [L]oad */
    PERSONNEL_KEY_C = 0x2e,         /* [C]reate */
    PERSONNEL_KEY_SPACE = 0x39
};

/* Function start: 0x433AD0 */
void PollPersonnelMenuInput(void)
{
    InputEventState event;
    short eventType;

    g_cPreviousKey_0049312c =
        (signed char)g_sPersonnelPreviousInput_0049a6bc;
    ServiceInputDevices(15);
    g_bPersonnelPrimaryInputDown_0049a6d4 = IsInputEventQueued(3);
    if (g_bPersonnelPrimaryInputDown_0049a6d4 == 0) {
        g_cCurrentKey_00493128 |= (signed char)0x80;
        g_sPersonnelPreviousInput_0049a6bc |= 0x80;
    }
    if (IsInputEventQueued(5) != 0) {
        g_cCurrentKey_00493128 |= (signed char)0x80;
        g_sPersonnelPreviousInput_0049a6bc |= 0x80;
        g_bPersonnelPrimaryInputDown_0049a6d4 = 0;
    }
    while ((eventType = GetNextInputEvent(&event)) != 0) {
        switch (eventType) {
        case 3:
            g_stPersonnelCursor_005c8470.x = event.x;
            g_stPersonnelCursor_005c8470.y = event.y;
            break;
        case 7:
            break;
        case 2:
            break;
        case 1:
            g_sPersonnelPreviousInput_0049a6bc =
                (g_cCurrentKey_00493128 = 0x1c);
            if ((event.modifiers & 1) != 0) {
                g_sPersonnelPreviousInput_0049a6bc =
                    (g_cCurrentKey_00493128 = 0x1c);
            }
            if ((event.modifiers & 2) != 0) {
                g_sPersonnelPreviousInput_0049a6bc =
                    (g_cCurrentKey_00493128 = 0x1c);
            }
            break;
        case 4:
        case 6:
            g_sPersonnelPreviousInput_0049a6bc =
                (g_cCurrentKey_00493128 = (signed char)event.status);
            break;
        }
    }
    if ((int)g_cPreviousKey_0049312c ==
        (int)g_sPersonnelPreviousInput_0049a6bc) {
        g_cCurrentKey_00493128 = (signed char)0x80;
    }
}

/* Function start: 0x433C84 */
void InitializeCampaignChalkboardScreen(short cockpitVariant)
{
    void *workspace;
    short frame;

    if (g_stModalSourceWindow_005d2c50.pixels !=
        (unsigned char *)0xa000) {
        InitializeGameTextContexts();
    }
    DisableMouseCursorDrawing();
    g_stSecondaryViewBufferBlock_005d2c90.left = 0;
    g_stSecondaryViewBufferBlock_005d2c90.right = 319;
    g_stSecondaryViewBufferBlock_005d2c90.top = 0;
    g_stSecondaryViewBufferBlock_005d2c90.bottom = 199;
    if ((short)AllocateViewport(
            &g_stSecondaryViewBufferBlock_005d2c90,
            g_cSecondaryViewBufferColour_0049cb4c, 0) == 0) {
        ReportFatalErrorCode("001");
    }
    g_nCockpitArchiveVariantNumber_005c901a = cockpitVariant;
    g_pCampaignChalkboardShape_0049ca54 = FetchDiskPacketRetrying(
        "options.v00",
        (short)(g_nCockpitArchiveVariantNumber_005c901a * 3), 0);
    if (g_pCampaignGlobals_00499c94->field_08 ==
        g_pCampaignGlobals_00499c94->field_0a) {
        frame = 1;
        g_pCampaignGlobals_00499c94->field_08 = 0;
    } else {
        frame = 2;
    }
    if (g_bNewPilotCampaignInitialized_004926c0 != 0)
        frame = 1;
    LoadSceneHotspotBoundsForSelection(
        g_pCampaignChalkboardShape_0049ca54,
        (unsigned short)frame);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90, 0, 0,
                      g_pCampaignChalkboardShape_0049ca54, 0);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                      g_nShotHotspotLeft_005d2120,
                      g_nSceneHotspotTopValue_005d2122,
                      g_pCampaignChalkboardShape_0049ca54, frame);
    if (g_bRoomTransitionAnimationEnabled_00499c00 != 0) {
        workspace = AllocateTaggedMemory(0xce, 0x40);
        if (workspace != 0) {
            InitializeViewportWipe(
                &g_stSecondaryViewBufferBlock_005d2c90,
                &g_stScrViewInfo_005d21a0,
                g_pCampaignGlobals_00499c94->field_0e >> 8,
                0x3c, 1, workspace);
            while (AdvanceViewportWipe(workspace) == 0) {
            }
            ReleasePacketHandle(workspace);
        }
    }
    RefreshMemoryStatusOverlay();
    SetPersonnelMousePosition(
        (short)((g_nSceneRegionRightWord_005d2124 -
                 g_nShotHotspotLeft_005d2120) / 2 +
                g_nShotHotspotLeft_005d2120),
        (short)((g_nSceneAreaLowestWord_005d2126 -
                 g_nSceneHotspotTopValue_005d2122) / 2 +
                g_nSceneHotspotTopValue_005d2122));
}

/* Function start: 0x433E8C */
void RefreshCampaignChalkboardScreen(short cockpitVariant)
{
    short frame;

    (void)cockpitVariant;
    DisableMouseCursorDrawing();
    if (g_pCampaignGlobals_00499c94->field_08 ==
        g_pCampaignGlobals_00499c94->field_0a) {
        frame = 1;
    } else {
        frame = 2;
    }
    DrawCampaignChalkboardFrame((unsigned short)frame);
}

/* Function start: 0x433ED7 */
void DrawCampaignChalkboardFrame(unsigned short frame)
{
    if (g_bNewPilotCampaignInitialized_004926c0 != 0)
        frame = 1;
    LoadSceneHotspotBoundsForSelection(
        g_pCampaignChalkboardShape_0049ca54,
        (unsigned short)frame);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90, 0, 0,
                      g_pCampaignChalkboardShape_0049ca54, 0);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                      g_nShotHotspotLeft_005d2120,
                      g_nSceneHotspotTopValue_005d2122,
                      g_pCampaignChalkboardShape_0049ca54, frame);
    SetPersonnelMousePosition(
        (short)((g_nSceneRegionRightWord_005d2124 -
                 g_nShotHotspotLeft_005d2120) / 2 +
                g_nShotHotspotLeft_005d2120),
        (short)((g_nSceneAreaLowestWord_005d2126 -
                 g_nSceneHotspotTopValue_005d2122) / 2 +
                g_nSceneHotspotTopValue_005d2122));
    RefreshMemoryStatusOverlay();
}

/* Function start: 0x433F94 */
void DrawPersonnelMenuBackdrop(void *scenePacket)
{
    short selection;

    selection = 0;
    SetMenuInputPump();
    DisableMouseCursorDrawing();
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90, 0, 0,
                      g_pPersonnelMenuBackdrop_0049a6b8, 0);
    selection = PollSceneHotspotInput(scenePacket, 0, 0, 0, 0);
    InitializeTextContextFromFont(&g_stDefaultStringContext_005d2d20, 0,
                                  g_bPrimaryViewBufferColour_0049cb50, -1);
    if (g_pszPersonnelFooter_00492658 != 0) {
        SetTextCursor(
            (unsigned short)((320 -
                MeasureTextPixelWidthClamped(
                    g_pszPersonnelFooter_00492658)) >> 1),
            180);
        DrawFormattedText("%S", g_pszPersonnelFooter_00492658);
    }
}

#pragma function(strcpy)
/* Function start: 0x434043 */
short LoadSelectedPilotCampaign(void)
{
    int campaignBytes;
    short loadedCampaign;
    short file;

    LoadTemporaryCampaignGlobals();
    campaignBytes = (unsigned int)g_pCampaignGlobals_00499c94->wordCount * 2;
    file = OpenDataFileOrDie(g_szPersonnelFileName_0049a6a8);
    if (file < 0)
        ReportFatalErrorCode("002");
    SeekDataFile((unsigned short)file, (campaignBytes + 0x86) << 3, 0);
    SeekDataFile((unsigned short)file, 2, 1);
    ReadDataFileAtOffset((unsigned short)file, -1, 0x60,
                         &g_stCurrentPilotProfile_00493408);
    SeekDataFile((unsigned short)file, 0x24, 1);
    ReadDataFileAtOffset((unsigned short)file, -1, campaignBytes,
                         g_pCampaignGlobals_00499c94);
    CloseDataFile((unsigned short)file);
    strcpy(g_stPilotNameVariables_00499ef8.firstName,
           g_stCurrentPilotProfile_00493408.firstName);
    strcpy(g_stPilotNameVariables_00499ef8.lastName,
           g_stCurrentPilotProfile_00493408.lastName);
    strcpy(g_stPilotNameVariables_00499ef8.callsign,
           g_stCurrentPilotProfile_00493408.callsign);
    InitializeCampaignConstellationState(g_pCampaignGlobals_00499c94, 1);
    loadedCampaign = g_pCampaignGlobals_00499c94->campaignSlot;
    SaveAndFreeTemporaryCampaignGlobals();
    return loadedCampaign;
}

/* Function start: 0x434177 */
unsigned short HasSavedPilotCampaign(void)
{
    struct _finddata_t findData;
    int stateBytes;
    short occupied;
    short file;
    short findHandle;

    LoadTemporaryCampaignGlobals();
    stateBytes = (unsigned short)g_pCampaignGlobals_00499c94->wordCount * 2;
    ReleasePacketSlot((void **)&g_pCampaignGlobals_00499c94);
    findHandle = (short)_findfirst(g_szPersonnelFileName_0049a6a8, &findData);
    if (findHandle == -1 ||
        (stateBytes + 0x86) * 9 != findData.size)
        return 0;
    _findclose((long)findHandle);
    file = (short)_open(g_szPersonnelFileName_0049a6a8, 0x8000);
    if (file < 0)
        return 0;
    if (lseek((int)file, (stateBytes + 0x86) << 3, 0) !=
        (stateBytes + 0x86) << 3) {
        _close((int)file);
        return 0;
    }
    if (_read((int)file, &occupied, 2) != 2) {
        _close((int)file);
        return 0;
    }
    _close((int)file);
    if (occupied == 0)
        return 0;
    return 1;
}

/* Function start: 0x4342E8 */
void OpenDiskDataFile(short logicalFile)
{
    struct _finddata_t findData;
    unsigned int stateBytes;
    short blank;
    short searchHandle;
    short inUse;
    short index;
    short file;

    inUse = 1;
    blank = 0;
    if (logicalFile != 0) {
        LoadTemporaryCampaignGlobals();
    }
    stateBytes =
        (unsigned int)g_pCampaignGlobals_00499c94->wordCount * 2;
    searchHandle = (short)_findfirst(g_szPersonnelFileName_0049a6a8, &findData);
    if (searchHandle == -1 ||
        (stateBytes + 0x86) * 9 != findData.size) {
        file = CreateDataFile(g_szPersonnelFileName_0049a6a8);
        if (file < 0) {
            ReportFatalErrorCode("005");
        }
        for (index = 0; index < 9; index++) {
            WriteDataFileAtOffset(file, -1, 2, &blank);
            WriteDataFileAtOffset(file, -1, 0x60,
                                  &g_stDefaultPilotProfile_00492660);
            WriteDataFileAtOffset(
                file, -1, sizeof(g_szSaveSlotSignature_0049a638),
                g_szSaveSlotSignature_0049a638);
            WriteDataFileAtOffset(file, -1, stateBytes,
                                  g_pCampaignGlobals_00499c94);
        }
        CloseDataFile(file);
    }
    _findclose((long)searchHandle);
    file = OpenDataFileOrDie(g_szPersonnelFileName_0049a6a8);
    if (file < 0) {
        ReportFatalErrorCode("006");
    }
    strcpy(g_stCurrentPilotProfile_00493408.firstName,
           g_stPilotNameVariables_00499ef8.firstName);
    strcpy(g_stCurrentPilotProfile_00493408.lastName,
           g_stPilotNameVariables_00499ef8.lastName);
    strcpy(g_stCurrentPilotProfile_00493408.callsign,
           g_stPilotNameVariables_00499ef8.callsign);
    SeekDataFile(file, (stateBytes + 0x86) << 3, 0);
    WriteDataFileAtOffset(file, -1, 2, &inUse);
    WriteDataFileAtOffset(file, -1, 0x60,
                          &g_stCurrentPilotProfile_00493408);
    WriteDataFileAtOffset(file, -1, sizeof(g_szSaveSlotSignature_0049a638),
                          g_szSaveSlotSignature_0049a638);
    WriteDataFileAtOffset(file, -1, stateBytes,
                          g_pCampaignGlobals_00499c94);
    CloseDataFile(file);
    if (logicalFile != 0) {
        ReleasePacketSlot((void **)&g_pCampaignGlobals_00499c94);
    }
}

/* Function start: 0x43456E */
void RestorePersonnelMenuBackground(Viewport *viewport)
{
    Viewport savedViewport;

    savedViewport = *viewport;
    DrawSpriteDefault(&savedViewport, 0, 0,
                      g_pPersonnelMenuBackdrop_0049a6b8, 1);
}

/* Function start: 0x4345A1 */
void DrawPilotSaveMenuChoices(short choice)
{
    if (choice == g_nPersonnelMenuHighlight_0049a6a0)
        return;
    DisableMouseCursorDrawing();
    if (choice == PERSONNEL_KEY_L) {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuSelectedCol_005c5874, -1);
    } else {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuDarkColour_005c5876, -1);
    }
    SetTextCursor(0x5a, 0x38);
    DrawFormattedText("[L]oad a personnel file");
    if (choice == PERSONNEL_KEY_S) {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuSelectedCol_005c5874, -1);
    } else {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuDarkColour_005c5876, -1);
    }
    SetTextCursor(0x5a, 0x4c);
    DrawFormattedText("[S]ave a personnel file");
    if (choice == PERSONNEL_KEY_D) {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuSelectedCol_005c5874, -1);
    } else {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuDarkColour_005c5876, -1);
    }
    SetTextCursor(0x5a, 0x60);
    DrawFormattedText("[D]elete a personnel file");
    g_nPersonnelMenuHighlight_0049a6a0 = choice;
    EnableMouseCursorDrawing();
}

void DrawFormattedText(const char *format, ...);

/* Function start: 0x4346E9 */
void DrawPersonnelSourceChoices(short choice)
{
    if (choice == g_nPersonnelMenuHighlight_0049a6a0)
        return;
    DisableMouseCursorDrawing();
    if (choice == PERSONNEL_KEY_W) {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuSelectedCol_005c5874, -1);
    } else {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuDarkColour_005c5876, -1);
    }
    SetTextCursor(0x50, 0x2e);
    DrawFormattedText("[W]ing Commander");
    if (choice == PERSONNEL_KEY_D) {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuSelectedCol_005c5874, -1);
    } else {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuDarkColour_005c5876, -1);
    }
    SetTextCursor(0x50, 0x42);
    DrawFormattedText("[D]os Version of Wing Commander");
    g_nPersonnelMenuHighlight_0049a6a0 = choice;
    EnableMouseCursorDrawing();
}

void InitializeTextContextFromFont(TextContext *context, short fontIndex, unsigned char colour, signed char background);

/* Function start: 0x4347D6 */
void DrawPersonnelMenuChoices(short choice)
{
    if (g_nPersonnelMenuHighlight_0049a6a0 == choice)
        return;
    DisableMouseCursorDrawing();
    if (choice == PERSONNEL_KEY_T) {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuSelectedCol_005c5874, -1);
    } else {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuDarkColour_005c5876, -1);
    }
    SetTextCursor(0x3c, 0x38);
    DrawFormattedText("[T]ransfer an existing personnel file");
    if (choice == PERSONNEL_KEY_C) {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuSelectedCol_005c5874, -1);
    } else {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuDarkColour_005c5876, -1);
    }
    SetTextCursor(0x3c, 0x4c);
    DrawFormattedText("[C]reate a new personnel file");
    g_nPersonnelMenuHighlight_0049a6a0 = choice;
    EnableMouseCursorDrawing();
}

#pragma function(strlen)
/* Function start: 0x4348C3 */
void DrawPersonnelFileSlot(short slot)
{
    if (g_apPersonnelFileSlots_0049a660[slot]->occupied == 0) {
        SetTextCursor(0x32, (unsigned short)(slot * 12 + 0x12));
        DrawFormattedText(
            "%d ----------------------------------------------------------",
            slot);
        return;
    }

    SetTextCursor(0x32, (unsigned short)(slot * 12 + 0x12));
    if (strlen(g_apPersonnelPilotProfiles_0049a680[slot]->lastName) +
            strlen(g_apPersonnelPilotProfiles_0049a680[slot]->callsign) +
            strlen(g_apPersonnelPilotProfiles_0049a680[slot]->firstName) >
        0x16) {
        DrawFormattedText(
            "%d--%s %c. \"%s\" %s",
            slot,
            g_apszShortPilotRankNames_0049a620[
                g_apPersonnelPilotProfiles_0049a680[slot]->rank],
            g_apPersonnelPilotProfiles_0049a680[slot]->firstName[0],
            g_apPersonnelPilotProfiles_0049a680[slot]->callsign,
            g_apPersonnelPilotProfiles_0049a680[slot]->lastName);
    } else {
        DrawFormattedText(
            "%d--%s %s \"%s\" %s",
            slot,
            g_apszPilotRankNames_0049a608[
                g_apPersonnelPilotProfiles_0049a680[slot]->rank],
            g_apPersonnelPilotProfiles_0049a680[slot]->firstName,
            g_apPersonnelPilotProfiles_0049a680[slot]->callsign,
            g_apPersonnelPilotProfiles_0049a680[slot]->lastName);
    }
    SetTextCursor(0xc6, (unsigned short)(slot * 12 + 0x12));
    DrawFormattedText(
        "%d",
        g_apPersonnelPilotProfiles_0049a680[slot]->field_41);
    SetTextCursor(0xde, (unsigned short)(slot * 12 + 0x12));
    DrawFormattedText(
        "%d",
        g_apPersonnelPilotProfiles_0049a680[slot]->field_43);
    SetTextCursor(0x36, (unsigned short)(slot * 12 + 0x18));
    DrawFormattedText(
        "%s",
        g_apPersonnelFileSlots_0049a660[slot]->description);
}

void DrawTransferredPilotRow(short row);

/* Function start: 0x434ACE */
void HighlightPersonnelFileSlot(short choice)
{
    short slot;

    if (choice == g_nPersonnelMenuHighlight_0049a6a0)
        return;
    if (choice == PERSONNEL_KEY_0) {
        slot = 0;
    } else {
        slot = (short)(choice - 1);
    }
    DisableMouseCursorDrawing();
    if (choice != -1) {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 2,
            (unsigned char)g_nMenuSelectedCol_005c5874, -1);
        DrawPersonnelFileSlot(slot);
    }
    if (g_nPersonnelMenuHighlight_0049a6a0 != -1) {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 2,
            (unsigned char)g_nMenuDarkColour_005c5876, -1);
        if (g_nPersonnelMenuHighlight_0049a6a0 == PERSONNEL_KEY_0) {
            slot = 0;
        } else {
            slot = (short)(g_nPersonnelMenuHighlight_0049a6a0 - 1);
        }
        DrawPersonnelFileSlot(slot);
    }
    g_nPersonnelMenuHighlight_0049a6a0 = choice;
    EnableMouseCursorDrawing();
}

/* Function start: 0x434BB5 */
short SelectPersonnelFileSlot(void)
{
    short hover;
    short index;

    DisableMouseCursorDrawing();
    InitializeTextContextFromFont(&g_stDefaultStringContext_005d2d20, 2,
                                  (unsigned char)
                                      g_nMenuDarkColour_005c5876,
                                  -1);
    for (index = 0; index < 8; index++) {
        DrawPersonnelFileSlot(index);
    }
    g_nPersonnelMenuHighlight_0049a6a0 = -1;
    hover = 0x0b;
    HighlightPersonnelFileSlot(hover);
    DisableMouseCursorDrawing();
    g_cCurrentKey_00493128 = (signed char)0x80;
    SetPersonnelMousePosition(0xdc, 0x12);
    while (g_cCurrentKey_00493128 == (signed char)0x80) {
        PollPersonnelMenuInput();
        if (g_stPersonnelCursor_005c8470.x > 0x28 &&
            g_stPersonnelCursor_005c8470.x < 0xec) {
            if (g_stPersonnelCursor_005c8470.y >= 0x12 &&
                g_stPersonnelCursor_005c8470.y <= 0x1c) {
                hover = 0x0b;
                HighlightPersonnelFileSlot(hover);
                SetMouseCursorShape(
                    g_pInputManagerState_005c8464->cursorShape, 1);
            } else if (g_stPersonnelCursor_005c8470.y >= 0x1d &&
                       g_stPersonnelCursor_005c8470.y <= 0x6e) {
                hover = (short)((g_stPersonnelCursor_005c8470.y - 0x1d) /
                                12 + 2);
                HighlightPersonnelFileSlot(hover);
                SetMouseCursorShape(
                    g_pInputManagerState_005c8464->cursorShape, 1);
            } else {
                hover = -1;
                HighlightPersonnelFileSlot(hover);
                SetMouseCursorShape(
                    g_pInputManagerState_005c8464->cursorShape, 0);
            }
        } else {
            hover = -1;
            HighlightPersonnelFileSlot(hover);
            SetMouseCursorShape(
                g_pInputManagerState_005c8464->cursorShape, 0);
        }
        g_bPersonnelMenuDrawing_0049a6c0 = 0;
        DrawPersonnelMenuBackdrop(g_pPersonnelMenuBackdrop_0049a6b8);
        g_bPersonnelMenuDrawing_0049a6c0 = 1;
        EnableMouseCursorDrawing();
        RefreshMemoryStatusOverlay();
        if ((g_cCurrentKey_00493128 == PERSONNEL_KEY_ENTER ||
             g_cCurrentKey_00493128 == PERSONNEL_KEY_SPACE) &&
            hover != -1) {
            g_cCurrentKey_00493128 = (signed char)hover;
        }
        if (g_cCurrentKey_00493128 == 1 ||
            ((g_cCurrentKey_00493128 == PERSONNEL_KEY_ENTER ||
              g_cCurrentKey_00493128 == PERSONNEL_KEY_SPACE) &&
             hover == -1)) {
            return -1;
        }
        switch (g_cCurrentKey_00493128) {
        case 11:
            return 0;
        case 2:
            return 1;
        case 3:
            return 2;
        case 4:
            return 3;
        case 5:
            return 4;
        case 6:
            return 5;
        case 7:
            return 6;
        case 8:
            return 7;
        }
        g_cCurrentKey_00493128 = (signed char)0x80;
    }
    /* Retail falls off the end here; EAX still holds the key loaded by
     * the loop test. */
#ifdef SDL_PORT
    return g_cCurrentKey_00493128;
#endif
}

/* The transfer-list routines: one row per transferable pilot. */
void DrawTransferredPilotRow(short row);
void HighlightTransferredPilot(short choice);
short SelectTransferredPilot(short count);

/* Function start: 0x434EBC */
void DrawTransferredPilotRow(short row)
{
    SetTextCursor(0x32, (unsigned short)(row * 10 + 0x1e));
    DrawFormattedText("%d", row);
    SetTextCursor(0x3c, (unsigned short)(row * 10 + 0x1e));
    DrawFormattedText(g_apszPersonnelImportCaptionsTable_005d2100[row]);
    SetTextCursor(0xc4, (unsigned short)(row * 10 + 0x1e));
    DrawFormattedText(
        g_apszPersonnelImportCaptionsTable_005d2100[row] + 0x28);
    SetTextCursor(0xda, (unsigned short)(row * 10 + 0x1e));
    DrawFormattedText(
        g_apszPersonnelImportCaptionsTable_005d2100[row] + 0x2c);
}

void SetTextCursor(unsigned short a, unsigned short b);
void CopyHugeMemoryOverlapSafe(void *destination, void *source, int count);
void DrawPersonnelMenuBackdrop(void *scenePacket);

/* Function start: 0x434F7D */
void HighlightTransferredPilot(short choice)
{
    short row;

    if (choice == g_nPersonnelMenuHighlight_0049a6a0)
        return;
    if (choice == PERSONNEL_KEY_0) {
        row = 0;
    } else {
        row = (short)(choice - 1);
    }
    DisableMouseCursorDrawing();
    if (choice != -1) {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuSelectedCol_005c5874, -1);
        DrawTransferredPilotRow(row);
    }
    if (g_nPersonnelMenuHighlight_0049a6a0 != -1) {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuDarkColour_005c5876, -1);
        if (g_nPersonnelMenuHighlight_0049a6a0 == PERSONNEL_KEY_0) {
            row = 0;
        } else {
            row = (short)(g_nPersonnelMenuHighlight_0049a6a0 - 1);
        }
        DrawTransferredPilotRow(row);
    }
    g_nPersonnelMenuHighlight_0049a6a0 = choice;
    EnableMouseCursorDrawing();
}

/* Function start: 0x435064 */
short SelectTransferredPilot(short count)
{
    short hover;
    short index;

    DisableMouseCursorDrawing();
    RestorePersonnelMenuBackground(&g_stSecondaryViewBufferBlock_005d2c90);
    InitializeTextContextFromFont(&g_stDefaultStringContext_005d2d20, 2,
                                  (unsigned char)
                                      g_nMenuDarkColour_005c5876,
                                  -1);
    SetTextCursor(0x32, 0x0f);
    DrawFormattedText("Select a character to transfer");
    SetTextCursor(0xba, 0x16);
    DrawFormattedText("Missions Kills");
    InitializeTextContextFromFont(&g_stDefaultStringContext_005d2d20, 1,
                                  (unsigned char)
                                      g_nMenuDarkColour_005c5876,
                                  -1);
    for (index = 0; index < count; index++) {
        DrawTransferredPilotRow(index);
    }
    g_nPersonnelMenuHighlight_0049a6a0 = -1;
    hover = 0x0b;
    HighlightTransferredPilot(hover);
    DisableMouseCursorDrawing();
    g_cCurrentKey_00493128 = (signed char)0x80;
    SetPersonnelMousePosition(0xdc, 0x12);
    while (g_cCurrentKey_00493128 == (signed char)0x80) {
        PollPersonnelMenuInput();
        if (g_stPersonnelCursor_005c8470.x > 0x28 &&
            g_stPersonnelCursor_005c8470.x < 0xec) {
            if (g_stPersonnelCursor_005c8470.y >= 0x1e &&
                g_stPersonnelCursor_005c8470.y <= 0x27) {
                hover = 0x0b;
                HighlightTransferredPilot(hover);
                SetMouseCursorShape(
                    g_pInputManagerState_005c8464->cursorShape, 1);
            } else if (g_stPersonnelCursor_005c8470.y >= 0x28 &&
                       g_stPersonnelCursor_005c8470.y <=
                           (count - 2) * 10 + 0x31) {
                hover = (short)((g_stPersonnelCursor_005c8470.y - 0x28) /
                                10 + 2);
                HighlightTransferredPilot(hover);
                SetMouseCursorShape(
                    g_pInputManagerState_005c8464->cursorShape, 1);
            } else {
                hover = -1;
                HighlightTransferredPilot(hover);
                SetMouseCursorShape(g_pInputManagerState_005c8464->cursorShape,
                                    0);
            }
        } else {
            hover = -1;
            HighlightTransferredPilot(hover);
            SetMouseCursorShape(
                g_pInputManagerState_005c8464->cursorShape, 0);
        }
        g_bPersonnelMenuDrawing_0049a6c0 = 0;
        DrawPersonnelMenuBackdrop(g_pPersonnelMenuBackdrop_0049a6b8);
        g_bPersonnelMenuDrawing_0049a6c0 = 1;
        EnableMouseCursorDrawing();
        RefreshMemoryStatusOverlay();
        if ((g_cCurrentKey_00493128 == PERSONNEL_KEY_ENTER ||
             g_cCurrentKey_00493128 == PERSONNEL_KEY_SPACE) &&
            hover != -1) {
            g_cCurrentKey_00493128 = (signed char)hover;
        }
        if (g_cCurrentKey_00493128 == 1 ||
            ((g_cCurrentKey_00493128 == PERSONNEL_KEY_ENTER ||
              g_cCurrentKey_00493128 == PERSONNEL_KEY_SPACE) &&
             hover == -1)) {
            return -1;
        }
        switch (g_cCurrentKey_00493128) {
        case 11:
            return 0;
        case 2:
            return 1;
        case 3:
            return 2;
        case 4:
            return 3;
        case 5:
            return 4;
        case 6:
            return 5;
        case 7:
            return 6;
        case 8:
            return 7;
        }
        g_cCurrentKey_00493128 = (signed char)0x80;
    }
    /* Retail falls off the end here; EAX still holds the key loaded by
     * the loop test. */
#ifdef SDL_PORT
    return g_cCurrentKey_00493128;
#endif
}

/* Function start: 0x4353D4 */
void RunPilotSaveLoadMenu(void)
{
    struct _finddata_t findData;
    int campaignBytes;
    int action;
    char answer;
    short confirm;
    short file;
    short index;
    short hover;
    short refresh;
    short done;
    short slot;

    done = 0;
    refresh = 0;
    g_pPersonnelMenuBackdrop_0049a6b8 = FetchDiskPacketRetrying(
        "options.v00",
        (short)(g_nCockpitArchiveVariantNumber_005c901a * 3 + 1), 0);
    ClearViewport(&g_stSecondaryViewBufferBlock_005d2c90, 0);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90, 0, 0,
                      g_pPersonnelMenuBackdrop_0049a6b8, 0);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90, 0, 0,
                      g_pPersonnelMenuBackdrop_0049a6b8, 1);
    SetInputViewport(&g_stSecondaryViewBufferBlock_005d2c90);
    campaignBytes = g_pCampaignGlobals_00499c94->wordCount * 2;
    g_pPersonnelCampaignSnapshot_0049a6a4 =
        AllocateTaggedMemory(campaignBytes, 0x40);
    if (g_pPersonnelCampaignSnapshot_0049a6a4 == 0) {
        ReportFatalErrorCode("007");
    }

    /* The find handle shares storage with the menu's exit flag; the
     * original reuses the same short once the file has been sized. */
    done = (short)_findfirst(g_szPersonnelFileName_0049a6a8, &findData);
    if (done == -1 || campaignBytes * 9 + 0x4b6 != findData.size) {
        file = (short)_open(g_szPersonnelFileName_0049a6a8, 0x8301, 0x180);
        if (file < 0) {
            ReportFatalErrorCode("008");
        }
        CopyHugeMemoryOverlapSafe(g_pPersonnelCampaignSnapshot_0049a6a4,
                                  g_pCampaignGlobals_00499c94,
                                  campaignBytes);
        for (index = 0; index < 9; index++) {
            _write(file, &refresh, 2);
            _write(file, &g_stDefaultPilotProfile_00492660, 0x60);
            _write(file, g_szSaveSlotSignature_0049a638,
                   sizeof(g_szSaveSlotSignature_0049a638));
            _write(file, g_pPersonnelCampaignSnapshot_0049a6a4,
                   campaignBytes);
        }
        _close(file);
    }
    _findclose((long)done);

    file = (short)_open(g_szPersonnelFileName_0049a6a8, 0x8000);
    for (index = 0; index < 8; index++) {
        g_apPersonnelFileSlots_0049a660[index] =
            AllocateTaggedMemory(0x26, 0x40);
        if (g_apPersonnelFileSlots_0049a660[index] == 0) {
            ReportFatalErrorCode("009");
        }
        g_apPersonnelPilotProfiles_0049a680[index] =
            AllocateTaggedMemory(0x60, 0x40);
        if (g_apPersonnelPilotProfiles_0049a680[index] == 0) {
            ReportFatalErrorCode("010");
        }
        _read(file, g_apPersonnelFileSlots_0049a660[index], 2);
        _read(file, g_apPersonnelPilotProfiles_0049a680[index], 0x60);
        _read(file, g_apPersonnelFileSlots_0049a660[index]->description,
              0x24);
        _read(file, g_pPersonnelCampaignSnapshot_0049a6a4,
              campaignBytes);
    }
    _close(file);

    done = 0;
    SetPersonnelMousePosition(0xc8, 0x36);
    while (done == 0) {
        DisableMouseCursorDrawing();
        g_stDefaultStringContext_005d2d20.viewport =
            &g_stSecondaryViewBufferBlock_005d2c90;
        g_stDefaultStringContext_005d2d20.text =
            g_szDefaultTextBufferText_005d2b80;
        RestorePersonnelMenuBackground(&g_stSecondaryViewBufferBlock_005d2c90);
        SetTextContext(&g_stDefaultStringContext_005d2d20);
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuDarkColour_005c5876, -1);
        SetTextCursor(0x50, 0x12);
        DrawFormattedText("TERRAN CONFEDERATION NAVY");
        SetTextCursor(0x60, 0x1c);
        DrawFormattedText("PERSONNEL DATABASE");
        hover = 0x26;
        g_nPersonnelMenuHighlight_0049a6a0 = -1;
        DrawPilotSaveMenuChoices(hover);
        DisableMouseCursorDrawing();
        RefreshMemoryStatusOverlay();
        g_cCurrentKey_00493128 = (signed char)0x80;
        while (g_cCurrentKey_00493128 == (signed char)0x80) {
            PollPersonnelMenuInput();
            if (g_stPersonnelCursor_005c8470.x > 0x28 &&
                g_stPersonnelCursor_005c8470.x < 0xea) {
                if (g_stPersonnelCursor_005c8470.y >= 0x30 &&
                    g_stPersonnelCursor_005c8470.y <= 0x43) {
                    hover = 0x26;
                    DrawPilotSaveMenuChoices(hover);
                    SetMouseCursorShape(
                        g_pInputManagerState_005c8464->cursorShape, 1);
                } else if (g_stPersonnelCursor_005c8470.y >= 0x44 &&
                           g_stPersonnelCursor_005c8470.y <= 0x57) {
                    hover = 0x1f;
                    DrawPilotSaveMenuChoices(hover);
                    SetMouseCursorShape(
                        g_pInputManagerState_005c8464->cursorShape, 1);
                } else if (g_stPersonnelCursor_005c8470.y >= 0x58 &&
                           g_stPersonnelCursor_005c8470.y <= 0x6b) {
                    hover = 0x20;
                    DrawPilotSaveMenuChoices(hover);
                    SetMouseCursorShape(
                        g_pInputManagerState_005c8464->cursorShape, 1);
                } else {
                    hover = -1;
                    DrawPilotSaveMenuChoices(hover);
                    SetMouseCursorShape(
                        g_pInputManagerState_005c8464->cursorShape, 0);
                }
            } else {
                hover = -1;
                DrawPilotSaveMenuChoices(hover);
                SetMouseCursorShape(
                    g_pInputManagerState_005c8464->cursorShape, 0);
            }
            g_bPersonnelMenuDrawing_0049a6c0 = 0;
            DrawPersonnelMenuBackdrop(g_pPersonnelMenuBackdrop_0049a6b8);
            g_bPersonnelMenuDrawing_0049a6c0 = 1;
            EnableMouseCursorDrawing();
            RefreshMemoryStatusOverlay();
            refresh = 0;
            if ((g_cCurrentKey_00493128 == PERSONNEL_KEY_ENTER ||
                 g_cCurrentKey_00493128 == PERSONNEL_KEY_SPACE) &&
                hover != -1) {
                g_cCurrentKey_00493128 = (signed char)hover;
            }
            if (g_cCurrentKey_00493128 == 1 ||
                ((g_cCurrentKey_00493128 == PERSONNEL_KEY_ENTER ||
                  g_cCurrentKey_00493128 == PERSONNEL_KEY_SPACE) &&
                 hover == -1)) {
                done++;
                refresh++;
            }
            action = toupper(g_cCurrentKey_00493128);
            switch (action) {
            case PERSONNEL_KEY_L:
                refresh = 1;
                DisableMouseCursorDrawing();
                RestorePersonnelMenuBackground(
                    &g_stSecondaryViewBufferBlock_005d2c90);
                RefreshMemoryStatusOverlay();
                InitializeTextContextFromFont(
                    &g_stDefaultStringContext_005d2d20, 2,
                    (unsigned char)g_nMenuDarkColour_005c5876, -1);
                SetTextCursor(0x50, 0x0c);
                DrawFormattedText("Select a game to load");
                SetTextCursor(0xb8, 0x0c);
                DrawFormattedText("Missions Kills");
                slot = SelectPersonnelFileSlot();
                if (slot != -1 &&
                    g_apPersonnelFileSlots_0049a660[slot]->occupied != 0) {
                    file = (short)_open(g_szPersonnelFileName_0049a6a8,
                                        0x8000);
                    if (file >= 0) {
                        lseek(file,
                               (campaignBytes + sizeof(PersonnelFileSlot) +
                                sizeof(PilotProfile)) * slot, 0);
                        _read(file, g_apPersonnelFileSlots_0049a660[slot], 2);
                        _read(file, g_apPersonnelPilotProfiles_0049a680[slot],
                              0x60);
                        _read(file,
                              g_apPersonnelFileSlots_0049a660[slot]->description,
                              0x24);
                        strcpy(g_stPilotNameVariables_00499ef8.firstName,
                               g_apPersonnelPilotProfiles_0049a680[slot]->firstName);
                        strcpy(g_stCurrentPilotProfile_00493408.firstName,
                               g_apPersonnelPilotProfiles_0049a680[slot]->firstName);
                        strcpy(g_stPilotNameVariables_00499ef8.lastName,
                               g_apPersonnelPilotProfiles_0049a680[slot]->lastName);
                        strcpy(g_stCurrentPilotProfile_00493408.lastName,
                               g_apPersonnelPilotProfiles_0049a680[slot]->lastName);
                        if (g_nOriginDevUnlock_0049d774 != 0) {
                            strcpy(
                                g_apPersonnelPilotProfiles_0049a680[slot]->callsign,
                                "CHEATER");
                        }
                        strcpy(g_stPilotNameVariables_00499ef8.callsign,
                               g_apPersonnelPilotProfiles_0049a680[slot]->callsign);
                        strcpy(g_stCurrentPilotProfile_00493408.callsign,
                               g_apPersonnelPilotProfiles_0049a680[slot]->callsign);
                        g_stCurrentPilotProfile_00493408.field_43 =
                            g_apPersonnelPilotProfiles_0049a680[slot]->field_43;
                        g_stCurrentPilotProfile_00493408.field_41 =
                            g_apPersonnelPilotProfiles_0049a680[slot]->field_41;
                        g_stCurrentPilotProfile_00493408.rank =
                            g_apPersonnelPilotProfiles_0049a680[slot]->rank;
                        _read(file, g_pPersonnelCampaignSnapshot_0049a6a4,
                              campaignBytes);
                        CopyHugeMemoryOverlapSafe(
                            g_pCampaignGlobals_00499c94,
                            g_pPersonnelCampaignSnapshot_0049a6a4,
                            campaignBytes);
                        _close(file);
                    } else {
                        ReportFatalErrorCode("011");
                    }
                }
                break;
            case PERSONNEL_KEY_D:
                refresh = 1;
                DisableMouseCursorDrawing();
                RestorePersonnelMenuBackground(
                    &g_stSecondaryViewBufferBlock_005d2c90);
                RefreshMemoryStatusOverlay();
                InitializeTextContextFromFont(
                    &g_stDefaultStringContext_005d2d20, 2,
                    (unsigned char)g_nMenuDarkColour_005c5876, -1);
                SetTextCursor(0xb8, 0x0c);
                DrawFormattedText("Missions Kills");
                SetTextCursor(0x50, 0x0c);
                DrawFormattedText("Select a slot to delete");
                slot = SelectPersonnelFileSlot();
                if (slot != -1 &&
                    g_apPersonnelFileSlots_0049a660[slot]->occupied != 0) {
                    DisableMouseCursorDrawing();
                    RestorePersonnelMenuBackground(
                        &g_stSecondaryViewBufferBlock_005d2c90);
                    InitializeTextContextFromFont(
                        &g_stDefaultStringContext_005d2d20, 2,
                        (unsigned char)g_nMenuDarkColour_005c5876, -1);
                    confirm = 1;
                    SetTextCursor(0x32, 0x32);
                    DrawFormattedText("Are you sure?");
                    InitializeTextContextFromFont(
                        &g_stDefaultStringContext_005d2d20, 2,
                        (unsigned char)g_nMenuSelectedCol_005c5874,
                        (unsigned char)g_nOptionMessageColour_005c57e8);
                    SetTextCursor(0x6a, 0x32);
                    DrawFormattedText("Yes");
                    RefreshMemoryStatusOverlay();
                    answer = (char)WaitForQueuedInputPress();
                    while (answer != 0x0d) {
                        SetTextCursor(0x6a, 0x32);
                        if (toupper(answer) == 'Y') {
                            DrawFormattedText("Yes");
                            confirm = 1;
                        } else {
                            DrawFormattedText("No ");
                            confirm = 0;
                        }
                        RefreshMemoryStatusOverlay();
                        answer = (char)WaitForQueuedInputPress();
                    }
                    if (confirm != 0) {
                        g_apPersonnelFileSlots_0049a660[slot]->occupied = 0;
                        file = (short)_open(g_szPersonnelFileName_0049a6a8, 0x8001);
                        lseek(file,
                               (campaignBytes + sizeof(PersonnelFileSlot) +
                            sizeof(PilotProfile)) * slot, 0);
                        _write(file,
                               g_apPersonnelFileSlots_0049a660[slot], 2);
                        _close(file);
                    }
                }
                break;
            case PERSONNEL_KEY_S:
                refresh = 1;
                DisableMouseCursorDrawing();
                RestorePersonnelMenuBackground(
                    &g_stSecondaryViewBufferBlock_005d2c90);
                InitializeTextContextFromFont(
                    &g_stDefaultStringContext_005d2d20, 2,
                    (unsigned char)g_nMenuDarkColour_005c5876, -1);
                SetTextCursor(0xb8, 0x0c);
                DrawFormattedText("Missions Kills");
                SetTextCursor(0x50, 0x0c);
                DrawFormattedText("Select a slot to save in");
                RefreshMemoryStatusOverlay();
                slot = SelectPersonnelFileSlot();
                if (slot == -1) {
                    break;
                }
                DisableMouseCursorDrawing();
                RestorePersonnelMenuBackground(
                    &g_stSecondaryViewBufferBlock_005d2c90);
                RefreshMemoryStatusOverlay();
                InitializeTextContextFromFont(
                    &g_stDefaultStringContext_005d2d20, 2,
                    (unsigned char)g_nMenuDarkColour_005c5876, -1);
                confirm = 1;
                if (g_apPersonnelFileSlots_0049a660[slot]->occupied != 0) {
                    SetTextCursor(0x32, 0x32);
                    DrawFormattedText("Are you sure?");
                    InitializeTextContextFromFont(
                        &g_stDefaultStringContext_005d2d20, 2,
                        (unsigned char)g_nMenuSelectedCol_005c5874,
                        (unsigned char)g_nOptionMessageColour_005c57e8);
                    SetTextCursor(0x6a, 0x32);
                    DrawFormattedText("Yes");
                    RefreshMemoryStatusOverlay();
                    answer = (char)WaitForQueuedInputPress();
                    while (answer != 0x0d) {
                        SetTextCursor(0x6a, 0x32);
                        if (toupper(answer) == 'Y') {
                            DrawFormattedText("Yes");
                            confirm = 1;
                        } else {
                            DrawFormattedText("No ");
                            confirm = 0;
                        }
                        RefreshMemoryStatusOverlay();
                        answer = (char)WaitForQueuedInputPress();
                        ServiceSoundSystem();
                    }
                }
                InitializeTextContextFromFont(
                    &g_stDefaultStringContext_005d2d20, 2,
                    (unsigned char)g_nMenuDarkColour_005c5876, -1);
                if (confirm != 0) {
                    g_apPersonnelFileSlots_0049a660[slot]->occupied = 1;
                    g_apPersonnelFileSlots_0049a660[slot]->description[0] = 0;
                    SetTextCursor(0x32, 0x46);
                    DrawFormattedText("%s", "Saved game comments:");
                    RefreshMemoryStatusOverlay();
                    InitializeTextContextFromFont(
                        &g_stDefaultStringContext_005d2d20, 2,
                        (unsigned char)g_nMenuSelectedCol_005c5874, -1);
                    ReadPersonnelTextField(
                        0x32, 0x4c, 2, "",
                        g_apPersonnelFileSlots_0049a660[slot]->description,
                        0x20, 0, 0);
                    strcpy(g_apPersonnelPilotProfiles_0049a680[slot]->firstName,
                           g_stCurrentPilotProfile_00493408.firstName);
                    strcpy(g_apPersonnelPilotProfiles_0049a680[slot]->lastName,
                           g_stCurrentPilotProfile_00493408.lastName);
                    strcpy(g_apPersonnelPilotProfiles_0049a680[slot]->callsign,
                           g_stCurrentPilotProfile_00493408.callsign);
                    g_apPersonnelPilotProfiles_0049a680[slot]->rank =
                        g_stCurrentPilotProfile_00493408.rank;
                    g_apPersonnelPilotProfiles_0049a680[slot]->field_41 =
                        g_stCurrentPilotProfile_00493408.field_41;
                    g_apPersonnelPilotProfiles_0049a680[slot]->field_43 =
                        g_stCurrentPilotProfile_00493408.field_43;
                    file = (short)_open(g_szPersonnelFileName_0049a6a8, 0x8001);
                    lseek(file,
                           (campaignBytes + sizeof(PersonnelFileSlot) +
                            sizeof(PilotProfile)) * slot, 0);
                    _write(file, g_apPersonnelFileSlots_0049a660[slot], 2);
                    _write(file, g_apPersonnelPilotProfiles_0049a680[slot],
                           0x60);
                    _write(file,
                           g_apPersonnelFileSlots_0049a660[slot]->description,
                           0x24);
                    CopyHugeMemoryOverlapSafe(
                        g_pPersonnelCampaignSnapshot_0049a6a4,
                        g_pCampaignGlobals_00499c94, campaignBytes);
                    _write(file, g_pPersonnelCampaignSnapshot_0049a6a4,
                           campaignBytes);
                    _close(file);
                }
                break;
            }
            if (refresh != 0) {
                g_cCurrentKey_00493128 = 0;
            }
        }
        g_cCurrentKey_00493128 = (signed char)0x80;
    }

    g_pActiveScenePacket_00492654 = 0;
    ReleasePacketSlot(&g_stSceneZoneTable_005d3bf8.data);
    ReleasePacketSlot(&g_stSceneTextList_005d3c00.data);
    FreePacketAndClear(&g_pPersonnelMenuBackdrop_0049a6b8, 0);
    g_stDefaultStringContext_005d2d20.viewport = &g_stScrViewInfo_005d21a0;
    ReleasePacketHandle(g_pPersonnelCampaignSnapshot_0049a6a4);
    for (index = 0; index < 8; index++) {
        ReleasePacketHandle(g_apPersonnelFileSlots_0049a660[index]);
        ReleasePacketHandle(g_apPersonnelPilotProfiles_0049a680[index]);
    }
}

/* Function start: 0x43641C */
short SelectLegacyGameSource(void)
{
    short choice;
    short done;
    short handled;
    short focus;

    done = 0;
    choice = -1;
    RestorePersonnelMenuBackground(&g_stSecondaryViewBufferBlock_005d2c90);
    SetPersonnelMousePosition(200, 0x2c);
    while (done == 0) {
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuDarkColour_005c5876, -1);
        SetTextCursor(0x3c, 0x1e);
        DrawFormattedText("Transfer a character from:");
        RefreshMemoryStatusOverlay();
        g_nPersonnelMenuHighlight_0049a6a0 = -1;
        DrawPersonnelSourceChoices(focus);
        DisableMouseCursorDrawing();
        g_cCurrentKey_00493128 = (signed char)0x80;
        while (g_cCurrentKey_00493128 == (signed char)0x80) {
            PollPersonnelMenuInput();
            if (g_stPersonnelCursor_005c8470.x > 0x1e &&
                g_stPersonnelCursor_005c8470.x < 0xe0) {
                if (g_stPersonnelCursor_005c8470.y >= 0x26 &&
                    g_stPersonnelCursor_005c8470.y <= 0x39) {
                    focus = PERSONNEL_KEY_W;
                    DrawPersonnelSourceChoices(focus);
                    SetMouseCursorShape(
                        g_pInputManagerState_005c8464->cursorShape, 1);
                } else if (g_stPersonnelCursor_005c8470.y >= 0x3a &&
                           g_stPersonnelCursor_005c8470.y <= 0x4d) {
                    focus = PERSONNEL_KEY_D;
                    DrawPersonnelSourceChoices(focus);
                    SetMouseCursorShape(
                        g_pInputManagerState_005c8464->cursorShape, 1);
                } else {
                    focus = -1;
                    DrawPersonnelSourceChoices(focus);
                    SetMouseCursorShape(
                        g_pInputManagerState_005c8464->cursorShape, 0);
                }
            } else {
                focus = -1;
                DrawPersonnelSourceChoices(focus);
                SetMouseCursorShape(g_pInputManagerState_005c8464->cursorShape,
                                    0);
            }
            g_bPersonnelMenuDrawing_0049a6c0 = 0;
            DrawPersonnelMenuBackdrop(g_pPersonnelMenuBackdrop_0049a6b8);
            g_bPersonnelMenuDrawing_0049a6c0 = 1;
            EnableMouseCursorDrawing();
            RefreshMemoryStatusOverlay();
            handled = 0;
            choice = -1;
            if ((g_cCurrentKey_00493128 == PERSONNEL_KEY_ENTER ||
                 g_cCurrentKey_00493128 == PERSONNEL_KEY_SPACE) &&
                focus != -1) {
                g_cCurrentKey_00493128 = (signed char)focus;
            }
            if (g_cCurrentKey_00493128 == 1 ||
                ((g_cCurrentKey_00493128 == PERSONNEL_KEY_ENTER ||
                  g_cCurrentKey_00493128 == PERSONNEL_KEY_SPACE) &&
                 focus == -1)) {
                done++;
                handled++;
            }
            switch (toupper((int)g_cCurrentKey_00493128)) {
            case PERSONNEL_KEY_W:
                choice = 0;
                done++;
                handled++;
                g_nLegacySaveSource_0049a600 = 0;
                break;
            case PERSONNEL_KEY_D:
                choice = 0;
                done++;
                handled++;
                g_nLegacySaveSource_0049a600 = 1;
                break;
            }
            if (handled != 0) {
                g_cCurrentKey_00493128 = 0;
            }
        }
    }
    RestorePersonnelMenuBackground(&g_stSecondaryViewBufferBlock_005d2c90);
    return choice;
}

#pragma function(strcat)
/* Function start: 0x436722 */
short LocateLegacySaveGame(short source)
{
    unsigned int isAlnum;
    short pathEntered;
    struct _finddata_t findData;
    short handle;
    const char *nameList[3] = {
        "Wing Commander",
        "Secret Missions 1",
        "Crusade:Secret Missions 2"
    };
    char *installedPath;
    short driveIndex;
    short index;

    InitializeTextContextFromFont(
        &g_stDefaultStringContext_005d2d20, 2,
        (unsigned char)g_nMenuDarkColour_005c5876, -1);
    if (g_nLegacySaveSource_0049a600 == 0) {
        installedPath = GetLegacyGameDataPath();
        if (installedPath != 0) {
            strcpy(g_szLegacySaveFolder_005d2130, installedPath);
        } else {
            strcpy(g_szLegacySaveFolder_005d2130, "");
        }
    } else {
        strcpy(g_szLegacySaveFolder_005d2130, "");
        RestorePersonnelMenuBackground(&g_stSecondaryViewBufferBlock_005d2c90);
        SetTextCursor(0x32, 0x28);
        DrawFormattedText("Please enter the full path name of");
        SetTextCursor(0x32, 0x34);
        DrawFormattedText("your %s saved game file.", nameList[source]);
        SetTextCursor(0x32, 0x40);
        DrawFormattedText("Example: c:\\wing\\gamedat");
        RefreshMemoryStatusOverlay();
        DisableMouseCursorDrawing();
        pathEntered = ReadPersonnelTextField(
            0x32, 0x50, 2, "Path name: ",
            g_szLegacySaveFolder_005d2130, 0x19, 0, 1);
        EnableMouseCursorDrawing();
    }
    if (pathEntered == 0) {
        return 0;
    }
    if (strlen(g_szLegacySaveFolder_005d2130) >= 2) {
        for (index = (short)(strlen(g_szLegacySaveFolder_005d2130) - 1);
             index >= 0; index--) {
            if (g_szLegacySaveFolder_005d2130[index] == '\\')
                break;
            isAlnum = isalnum(g_szLegacySaveFolder_005d2130[index]);
            if (isAlnum != 0) {
                strcat(g_szLegacySaveFolder_005d2130, "\\");
                break;
            }
        }
    }
    strcat(g_szLegacySaveFolder_005d2130,
           g_apszSaveGameFileNames_0049a6c8[source]);
    for (index = 0;
         strlen(g_szLegacySaveFolder_005d2130) > (unsigned int)index;
         index++) {
        if (g_szLegacySaveFolder_005d2130[index] == ':') {
            if (index == 0) {
                return 0;
            }
            driveIndex = (short)(toupper(
                g_szLegacySaveFolder_005d2130[index - 1]) - 'A');
            PersonnelDriveHook(driveIndex);
        }
    }
    handle = (short)_findfirst(g_szLegacySaveFolder_005d2130, &findData);
    if (handle == -1) {
        return 0;
    }
    return 1;
}

/* Function start: 0x436A4D */
void ShowNoTransferablePilots(void)
{
    RestorePersonnelMenuBackground(&g_stSecondaryViewBufferBlock_005d2c90);
    SetTextCursor(0x50, 0x3c);
    DrawFormattedText("No valid characters found.");
    RefreshMemoryStatusOverlay();
    g_cCurrentKey_00493128 = (signed char)0x80;
    WaitForAnyInputPress();
}

/* Function start: 0x436A8F */
short RunPilotDatabaseMenu(void)
{
    short done = 0;
    short actionHandled = 0;
    char firstName[13] = "Christopher";
    char lastName[13] = "Blair";
    char callsign[13] = "Maverick";
    short result = 0;
    char text[80];
    SaveGameDiskRecord *record;
    PilotRecord *pilot;
    short file;
    short inputResult;
    short transferCount;
    short index;
    short selected;
    short hover;
    short previousDrive;
    int key;

    g_stSecondaryViewBufferBlock_005d2c90.left = 0;
    g_stSecondaryViewBufferBlock_005d2c90.right = 319;
    g_stSecondaryViewBufferBlock_005d2c90.top = 0;
    g_stSecondaryViewBufferBlock_005d2c90.bottom = 199;
    if ((short)AllocateViewport(
            &g_stSecondaryViewBufferBlock_005d2c90,
            g_cSecondaryViewBufferColour_0049cb4c, 0) == 0) {
        ReportFatalErrorCode("012");
    }
    SetInputViewport(&g_stSecondaryViewBufferBlock_005d2c90);
    DisableMouseCursorDrawing();
    InitializeTextContextFromFont(&g_stDefaultStringContext_005d2d20, 0,
                                  g_bPrimaryViewBufferColour_0049cb50, -1);
    InitializeTextContextFromFont(&g_stDefaultStringContext_005d2d20, 2,
                                  g_bPrimaryViewBufferColour_0049cb50, -1);
    g_pPersonnelMenuBackdrop_0049a6b8 = FetchDiskPacketRetrying(
        "options.v00", (short)(g_nCockpitArchiveVariantNumber_005c901a * 3 + 1),
        0);
    SetMenuInputPump();
    ClearViewport(&g_stSecondaryViewBufferBlock_005d2c90, 0);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90, 0, 0,
                      g_pPersonnelMenuBackdrop_0049a6b8, 0);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90, 0, 0,
                      g_pPersonnelMenuBackdrop_0049a6b8, 1);
    record = AllocateTaggedMemory(0x33c, 0x40);
    if (record == 0) {
        ReportFatalErrorCode("013");
    }
    for (index = 0; index < 8; index++) {
        g_apszPersonnelImportCaptionsTable_005d2100[index] =
            AllocateTaggedMemory(0x50, 0x40);
        if (g_apszPersonnelImportCaptionsTable_005d2100[index] == 0) {
            ReportFatalErrorCode("014");
        }
    }
    SetInputViewport(&g_stSecondaryViewBufferBlock_005d2c90);
    while (done == 0) {
        DisableMouseCursorDrawing();
        g_stDefaultStringContext_005d2d20.viewport =
            &g_stSecondaryViewBufferBlock_005d2c90;
        g_stDefaultStringContext_005d2d20.text =
            g_szDefaultTextBufferText_005d2b80;
        RestorePersonnelMenuBackground(&g_stSecondaryViewBufferBlock_005d2c90);
        SetTextContext(&g_stDefaultStringContext_005d2d20);
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 1,
            (unsigned char)g_nMenuDarkColour_005c5876, -1);
        SetTextCursor(0x50, 0x12);
        DrawFormattedText("TERRAN CONFEDERATION NAVY");
        SetTextCursor(0x60, 0x1c);
        DrawFormattedText("PERSONNEL DATABASE");
        hover = 0x26;
        g_nPersonnelMenuHighlight_0049a6a0 = -1;
        DrawPersonnelMenuChoices(hover);
        DisableMouseCursorDrawing();
        SetPersonnelMousePosition(200, 0x36);
        RefreshMemoryStatusOverlay();
        g_cCurrentKey_00493128 = (signed char)0x80;
        while (g_cCurrentKey_00493128 == (signed char)0x80) {
            PollPersonnelMenuInput();
            if (g_stPersonnelCursor_005c8470.x > 0x28 &&
                g_stPersonnelCursor_005c8470.x < 0xea) {
                if (g_stPersonnelCursor_005c8470.y >= 0x30 &&
                    g_stPersonnelCursor_005c8470.y <= 0x43) {
                    hover = 0x14;
                    DrawPersonnelMenuChoices(hover);
                    SetMouseCursorShape(
                        g_pInputManagerState_005c8464->cursorShape, 1);
                } else if (g_stPersonnelCursor_005c8470.y >= 0x44 &&
                           g_stPersonnelCursor_005c8470.y <= 0x57) {
                    hover = 0x2e;
                    DrawPersonnelMenuChoices(hover);
                    SetMouseCursorShape(
                        g_pInputManagerState_005c8464->cursorShape, 1);
                } else {
                    hover = -1;
                    DrawPersonnelMenuChoices(hover);
                    SetMouseCursorShape(
                        g_pInputManagerState_005c8464->cursorShape, 0);
                }
            } else {
                hover = -1;
                DrawPersonnelMenuChoices(hover);
                SetMouseCursorShape(g_pInputManagerState_005c8464->cursorShape,
                                    0);
            }
            g_bPersonnelMenuDrawing_0049a6c0 = 0;
            DrawPersonnelMenuBackdrop(g_pPersonnelMenuBackdrop_0049a6b8);
            g_bPersonnelMenuDrawing_0049a6c0 = 1;
            EnableMouseCursorDrawing();
            RefreshMemoryStatusOverlay();
            actionHandled = 0;
            if ((g_cCurrentKey_00493128 == PERSONNEL_KEY_ENTER ||
                 g_cCurrentKey_00493128 == PERSONNEL_KEY_SPACE) &&
                hover != -1) {
                g_cCurrentKey_00493128 = (signed char)hover;
            }
            if ((g_cCurrentKey_00493128 == PERSONNEL_KEY_ENTER ||
                 g_cCurrentKey_00493128 == PERSONNEL_KEY_SPACE) &&
                hover == -1) {
                done++;
                actionHandled++;
            }
            key = toupper((int)g_cCurrentKey_00493128);
            if (key == PERSONNEL_KEY_T) {
                previousDrive = 99;
                actionHandled = 1;
                DisableMouseCursorDrawing();
                RestorePersonnelMenuBackground(
                    &g_stSecondaryViewBufferBlock_005d2c90);
                RefreshMemoryStatusOverlay();
                InitializeTextContextFromFont(
                    &g_stDefaultStringContext_005d2d20, 1,
                    (unsigned char)g_nMenuDarkColour_005c5876, -1);
                selected = SelectLegacyGameSource();
                if (selected == -1) {
                    result = 0;
                } else {
                    done = (short)LocateLegacySaveGame(selected);
                    if (done == 0) {
                        ShowNoTransferablePilots();
                        result = 0;
                        g_cCurrentKey_00493128 = 0x14;
                    } else {
                        file = (short)_open(g_szLegacySaveFolder_005d2130,
                                            0x8000);
                        transferCount = 0;
                        for (index = 0; index < 8; index++) {
                            _read(file, record, 0x33c);
                            if (record->occupied != 0 &&
                                record->campaign.campaignIndex ==
                                    selected) {
                                pilot = &record->pilots[8];
                                strcpy(
                                    g_apszPersonnelImportCaptionsTable_005d2100[
                                        transferCount],
                                    g_apszShortPilotRankNames_0049a620[
                                        pilot->rank]);
                                strcat(
                                    g_apszPersonnelImportCaptionsTable_005d2100[
                                        transferCount],
                                    " \"");
                                strcat(
                                    g_apszPersonnelImportCaptionsTable_005d2100[
                                        transferCount],
                                    pilot->callsign);
                                strcat(
                                    g_apszPersonnelImportCaptionsTable_005d2100[
                                        transferCount],
                                    "\" ");
                                strcat(
                                    g_apszPersonnelImportCaptionsTable_005d2100[
                                        transferCount],
                                    pilot->name);
                                itoa(pilot->missions, text, 10);
                                strcpy(
                                    g_apszPersonnelImportCaptionsTable_005d2100[
                                        transferCount] + 0x28,
                                    text);
                                itoa(pilot->kills, text, 10);
                                strcpy(
                                    g_apszPersonnelImportCaptionsTable_005d2100[
                                        transferCount] + 0x2c,
                                    text);
                                itoa(index, text, 10);
                                strcpy(
                                    g_apszPersonnelImportCaptionsTable_005d2100[
                                        transferCount] + 0x3c,
                                    text);
                                transferCount++;
                            }
                        }
                        _close(file);
                        if (transferCount == 0) {
                            ShowNoTransferablePilots();
                            result = 0;
                            g_cCurrentKey_00493128 = 0x14;
                            done = 0;
                        } else {
                            selected = SelectTransferredPilot(
                                transferCount);
                            if (selected == -1) {
                                done = 0;
                                result = 0;
                                g_cCurrentKey_00493128 = 0x14;
                            } else {
                                index = (short)atoi(
                                    g_apszPersonnelImportCaptionsTable_005d2100[
                                        selected] + 0x3c);
                                file = (short)_open(
                                    g_szLegacySaveFolder_005d2130, 0x8000);
                                lseek(file, (long)index * 0x33c, 0);
                                _read(file, record, 0x33c);
                                pilot = &record->pilots[8];
                                _close(file);
                                PersonnelDriveHook(previousDrive);
                                InitializeTextContextFromFont(
                                    &g_stDefaultStringContext_005d2d20, 1,
                                    (unsigned char)
                                        g_nMenuSelectedCol_005c5874,
                                    -1);
                                DisableMouseCursorDrawing();
                                RestorePersonnelMenuBackground(
                                    &g_stSecondaryViewBufferBlock_005d2c90);
                                RefreshMemoryStatusOverlay();
                                strcpy(text,
                                       g_apszShortPilotRankNames_0049a620[
                                           pilot->rank]);
                                strcat(text, " ");
                                strcat(text, pilot->name);
                                strcat(text, "'s first name?");
                                SetTextCursor(0x32, 0x32);
                                DrawFormattedText(text);
                                RefreshMemoryStatusOverlay();
                                ReadPersonnelTextField(
                                    0x32, 0x3c, 1, "", firstName,
                                    0x0c, 0, 0);
                                strcpy(
                                    g_stPilotNameVariables_00499ef8.firstName,
                                    firstName);
                                strcpy(
                                    g_stPilotNameVariables_00499ef8.lastName,
                                    pilot->name);
                                strcpy(
                                    g_stPilotNameVariables_00499ef8.callsign,
                                    pilot->callsign);
                                strcpy(
                                    g_stCurrentPilotProfile_00493408.firstName,
                                    firstName);
                                strcpy(
                                    g_stCurrentPilotProfile_00493408.lastName,
                                    pilot->name);
                                strcpy(
                                    g_stCurrentPilotProfile_00493408.callsign,
                                    pilot->callsign);
                                g_stCurrentPilotProfile_00493408.rank = 2;
                                g_stCurrentPilotProfile_00493408.field_41 =
                                    pilot->missions;
                                g_stCurrentPilotProfile_00493408.field_43 =
                                    pilot->kills;
                                LoadStartingCampaignGlobals(0);
                                SaveAndFreeTemporaryCampaignGlobals();
                                OpenDiskDataFile(1);
                                done = 1;
                                result = 1;
                            }
                        }
                    }
                }
                PersonnelDriveHook(previousDrive);
            } else if (key == PERSONNEL_KEY_C) {
                result = 1;
                actionHandled = 1;
                DisableMouseCursorDrawing();
                RestorePersonnelMenuBackground(
                    &g_stSecondaryViewBufferBlock_005d2c90);
                RefreshMemoryStatusOverlay();
                InitializeTextContextFromFont(
                    &g_stDefaultStringContext_005d2d20, 1,
                    (unsigned char)g_nMenuSelectedCol_005c5874, -1);
                inputResult = ReadPersonnelTextField(
                    0x32, 0x1e, 2, "Enter first name: ", firstName,
                    0x0b, 0, 0);
                if (inputResult != 0) {
                    strcpy(g_stPilotNameVariables_00499ef8.firstName,
                           firstName);
                    strcpy(g_stCurrentPilotProfile_00493408.firstName,
                           firstName);
                    inputResult = ReadPersonnelTextField(
                        0x32, 0x32, 2, "Enter last name: ", lastName,
                        0x0b, 0, 0);
                    if (inputResult != 0) {
                        strcpy(g_stPilotNameVariables_00499ef8.lastName,
                               lastName);
                        strcpy(g_stCurrentPilotProfile_00493408.lastName,
                               lastName);
                        inputResult = ReadPersonnelTextField(
                            0x32, 0x46, 2, "Enter callsign: ", callsign,
                            0x0b, 0, 0);
                        if (inputResult != 0) {
                            strcpy(g_stPilotNameVariables_00499ef8.callsign,
                                   callsign);
                            strcpy(
                                g_stCurrentPilotProfile_00493408.callsign,
                                callsign);
                            g_stCurrentPilotProfile_00493408.rank = 2;
                            g_stCurrentPilotProfile_00493408.field_41 = 0;
                            g_stCurrentPilotProfile_00493408.field_43 = 0;
                            LoadStartingCampaignGlobals(0);
                            SaveAndFreeTemporaryCampaignGlobals();
                            OpenDiskDataFile(1);
                        }
                    }
                }
                if (inputResult == 0) {
                    result = 0;
                }
                done++;
                g_cCurrentKey_00493128 = 0x2e;
            }
            if (actionHandled == 0) {
                g_cCurrentKey_00493128 = (signed char)0x80;
            }
        }
        g_cCurrentKey_00493128 = (signed char)0x80;
    }
    FlushInputEvents();
    ReleasePacketSlot(&g_stSceneZoneTable_005d3bf8.data);
    ReleasePacketSlot(&g_stSceneTextList_005d3c00.data);
    g_pActiveScenePacket_00492654 = 0;
    FreePacketAndClear(&g_pPersonnelMenuBackdrop_0049a6b8, 0);
    g_stDefaultStringContext_005d2d20.viewport =
        &g_stScrViewInfo_005d21a0;
    ReleasePacketHandle(record);
    for (index = 0; index < 8; index++) {
        ReleasePacketHandle(
            g_apszPersonnelImportCaptionsTable_005d2100[index]);
    }
    free_viewport(&g_stSecondaryViewBufferBlock_005d2c90);
    FlushInputEvents();
    return result;
}
