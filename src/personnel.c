/*
 *  Personnel database, campaign chalkboard and pilot transfer.
 *
 *  Retail object 0x433AD0-0x43775F (link order; see docs/ORDER.md).
 */
#include "game.h"

unsigned char *g_pCockpitBackgroundPacket = 0; /* 0x0049a5f0 */
unsigned char *g_apCockpitVduOverlayShapes[2] = { 0 }; /* 0x0049a5f8 */
int g_nLegacySaveSource = 0; /* 0x0049a600 */
const char *g_apszPilotRankNames[5] = {
    "2ND LT.",
    "1ST LT.",
    "CAPTAIN",
    "MAJOR",
    "LT. COL."
}; /* 0x0049a608 */
const char *g_apszShortPilotRankNames[5] = {
    "2ND LT",
    "1ST LT",
    "CAPT.",
    "MAJOR",
    "LT COL"
}; /* 0x0049a620 */

/* The save-slot signature is a 0x24-byte record, not a 34-byte string: the
 * original writes 36 bytes from it, the string and the two NULs after it.
 * Spelling the object at its record width keeps the write inside it. */
static char g_szSaveSlotSignature[0x24] =
    "Wing Commander II (c) 1991 Origin"; /* 0x0049a638 */

PersonnelFileSlot *g_apPersonnelFileSlots[8] = { 0 }; /* 0x0049a660 */
PilotProfile *g_apPersonnelPilotProfiles[8] = { 0 }; /* 0x0049a680 */
short g_nPersonnelMenuHighlight = -1; /* 0x0049a6a0 */
void *g_pPersonnelCampaignSnapshot = 0; /* 0x0049a6a4 */
static char g_szPersonnelFileName[] = "savegame.wc2"; /* 0x0049a6a8 */
unsigned char *g_pPersonnelMenuBackdrop = 0; /* 0x0049a6b8 */
short g_sPersonnelPreviousInput = -128; /* 0x0049a6bc */
short g_bPersonnelMenuDrawing = 1; /* 0x0049a6c0 */
const char *g_apszSaveGameFileNames[3] = {
    "savegame.wld",
    "savegame.wld",
    "crusade.wld"
}; /* 0x0049a6c8 */
signed char g_bPersonnelPrimaryInputDown = 0; /* 0x0049a6d4 */

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

    g_cPreviousKey =
        (signed char)g_sPersonnelPreviousInput;
    ServiceInputDevices(15);
    g_bPersonnelPrimaryInputDown = IsInputEventQueued(3);
    if (g_bPersonnelPrimaryInputDown == 0) {
        g_cCurrentKey |= (signed char)0x80;
        g_sPersonnelPreviousInput |= 0x80;
    }
    if (IsInputEventQueued(5) != 0) {
        g_cCurrentKey |= (signed char)0x80;
        g_sPersonnelPreviousInput |= 0x80;
        g_bPersonnelPrimaryInputDown = 0;
    }
    while ((eventType = GetNextInputEvent(&event)) != 0) {
        switch (eventType) {
        case 3:
            g_37_stPersonnelMousePointerState.x = event.x;
            g_37_stPersonnelMousePointerState.y = event.y;
            break;
        case 7:
            break;
        case 2:
            break;
        case 1:
            g_sPersonnelPreviousInput =
                (g_cCurrentKey = 0x1c);
            if ((event.modifiers & 1) != 0) {
                g_sPersonnelPreviousInput =
                    (g_cCurrentKey = 0x1c);
            }
            if ((event.modifiers & 2) != 0) {
                g_sPersonnelPreviousInput =
                    (g_cCurrentKey = 0x1c);
            }
            break;
        case 4:
        case 6:
            g_sPersonnelPreviousInput =
                (g_cCurrentKey = (signed char)event.status);
            break;
        }
    }
    if ((int)g_cPreviousKey ==
        (int)g_sPersonnelPreviousInput) {
        g_cCurrentKey = (signed char)0x80;
    }
}

/* Function start: 0x433C84 */
void InitializeCampaignChalkboardScreen(short cockpitVariant)
{
    void *workspace;
    short frame;

    if (g_2_stModalSourceWindow.pixels !=
        (unsigned char *)0xa000) {
        InitializeGameTextContexts();
    }
    DisableMouseCursorDrawing();
    g_stSecondaryViewBufferBlock.left = 0;
    g_stSecondaryViewBufferBlock.right = 319;
    g_stSecondaryViewBufferBlock.top = 0;
    g_stSecondaryViewBufferBlock.bottom = 199;
    if ((short)AllocateViewport(
            &g_stSecondaryViewBufferBlock,
            g_cSecondaryViewBufferColour, 0) == 0) {
        ReportFatalErrorCode("001");
    }
    g_1_nCockpitArchiveVariantNumber = cockpitVariant;
    g_pCampaignChalkboardShape = FetchDiskPacketRetrying(
        "options.v00",
        (short)(g_1_nCockpitArchiveVariantNumber * 3), 0);
    if (g_pCampaignGlobals->field_08 ==
        g_pCampaignGlobals->field_0a) {
        frame = 1;
        g_pCampaignGlobals->field_08 = 0;
    } else {
        frame = 2;
    }
    if (g_bNewPilotCampaignInitialized != 0)
        frame = 1;
    LoadSceneHotspotBoundsForSelection(
        g_pCampaignChalkboardShape,
        (unsigned short)frame);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock, 0, 0,
                      g_pCampaignChalkboardShape, 0);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                      g_128_nShotHotspotLeft,
                      g_27_nSceneHotspotTopValue,
                      g_pCampaignChalkboardShape, frame);
    if (g_bRoomTransitionAnimationEnabled != 0) {
        workspace = AllocateTaggedMemory(0xce, 0x40);
        if (workspace != 0) {
            InitializeViewportWipe(
                &g_stSecondaryViewBufferBlock,
                &g_101_stScrViewInfo,
                g_pCampaignGlobals->field_0e >> 8,
                0x3c, 1, workspace);
            while (AdvanceViewportWipe(workspace) == 0) {
            }
            ReleasePacketHandle(workspace);
        }
    }
    RefreshMemoryStatusOverlay();
    SetPersonnelMousePosition(
        (short)((g_8_nSceneRegionRightWord -
                 g_128_nShotHotspotLeft) / 2 +
                g_128_nShotHotspotLeft),
        (short)((g_15_nSceneAreaLowestWord -
                 g_27_nSceneHotspotTopValue) / 2 +
                g_27_nSceneHotspotTopValue));
}

/* Function start: 0x433E8C */
void RefreshCampaignChalkboardScreen(short cockpitVariant)
{
    short frame;

    (void)cockpitVariant;
    DisableMouseCursorDrawing();
    if (g_pCampaignGlobals->field_08 ==
        g_pCampaignGlobals->field_0a) {
        frame = 1;
    } else {
        frame = 2;
    }
    DrawCampaignChalkboardFrame((unsigned short)frame);
}

/* Function start: 0x433ED7 */
void DrawCampaignChalkboardFrame(unsigned short frame)
{
    if (g_bNewPilotCampaignInitialized != 0)
        frame = 1;
    LoadSceneHotspotBoundsForSelection(
        g_pCampaignChalkboardShape,
        (unsigned short)frame);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock, 0, 0,
                      g_pCampaignChalkboardShape, 0);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                      g_128_nShotHotspotLeft,
                      g_27_nSceneHotspotTopValue,
                      g_pCampaignChalkboardShape, frame);
    SetPersonnelMousePosition(
        (short)((g_8_nSceneRegionRightWord -
                 g_128_nShotHotspotLeft) / 2 +
                g_128_nShotHotspotLeft),
        (short)((g_15_nSceneAreaLowestWord -
                 g_27_nSceneHotspotTopValue) / 2 +
                g_27_nSceneHotspotTopValue));
    RefreshMemoryStatusOverlay();
}

/* Function start: 0x433F94 */
void DrawPersonnelMenuBackdrop(void *scenePacket)
{
    short selection;

    selection = 0;
    SetMenuInputPump();
    DisableMouseCursorDrawing();
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock, 0, 0,
                      g_pPersonnelMenuBackdrop, 0);
    selection = PollSceneHotspotInput(scenePacket, 0, 0, 0, 0);
    InitializeTextContextFromFont(&g_3_stDefaultStringContext, 0,
                                  g_bPrimaryViewBufferColour, -1);
    if (g_pszPersonnelFooter != 0) {
        SetTextCursor(
            (unsigned short)((320 -
                MeasureTextPixelWidthClamped(
                    g_pszPersonnelFooter)) >> 1),
            180);
        DrawFormattedText("%S", g_pszPersonnelFooter);
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
    campaignBytes = (unsigned int)g_pCampaignGlobals->wordCount * 2;
    file = OpenDataFileOrDie(g_szPersonnelFileName);
    if (file < 0)
        ReportFatalErrorCode("002");
    SeekDataFile((unsigned short)file, (campaignBytes + 0x86) << 3, 0);
    SeekDataFile((unsigned short)file, 2, 1);
    ReadDataFileAtOffset((unsigned short)file, -1, 0x60,
                         &g_stCurrentPilotProfile);
    SeekDataFile((unsigned short)file, 0x24, 1);
    ReadDataFileAtOffset((unsigned short)file, -1, campaignBytes,
                         g_pCampaignGlobals);
    CloseDataFile((unsigned short)file);
    strcpy(g_stPilotNameVariables.firstName,
           g_stCurrentPilotProfile.firstName);
    strcpy(g_stPilotNameVariables.lastName,
           g_stCurrentPilotProfile.lastName);
    strcpy(g_stPilotNameVariables.callsign,
           g_stCurrentPilotProfile.callsign);
    InitializeCampaignConstellationState(g_pCampaignGlobals, 1);
    loadedCampaign = g_pCampaignGlobals->campaignSlot;
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
    stateBytes = (unsigned short)g_pCampaignGlobals->wordCount * 2;
    ReleasePacketSlot((void **)&g_pCampaignGlobals);
    findHandle = (short)_findfirst(g_szPersonnelFileName, &findData);
    if (findHandle == -1 ||
        (stateBytes + 0x86) * 9 != findData.size)
        return 0;
    _findclose((long)findHandle);
    file = (short)_open(g_szPersonnelFileName, 0x8000);
    if (file < 0)
        return 0;
    /* The complements preserve the offset and make CL 4.1 evaluate it
     * before lseek, as in retail; neither complement emits an instruction. */
    if (~~((stateBytes + 0x86) << 3) !=
        lseek((int)file, (stateBytes + 0x86) << 3, 0)) {
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
        (unsigned int)g_pCampaignGlobals->wordCount * 2;
    searchHandle = (short)_findfirst(g_szPersonnelFileName, &findData);
    if (searchHandle == -1 ||
        (stateBytes + 0x86) * 9 != findData.size) {
        file = CreateDataFile(g_szPersonnelFileName);
        if (file < 0) {
            ReportFatalErrorCode("005");
        }
        for (index = 0; index < 9; index++) {
            WriteDataFileAtOffset(file, -1, 2, &blank);
            WriteDataFileAtOffset(file, -1, 0x60,
                                  &g_stDefaultPilotProfile);
            WriteDataFileAtOffset(
                file, -1, sizeof(g_szSaveSlotSignature),
                g_szSaveSlotSignature);
            WriteDataFileAtOffset(file, -1, stateBytes,
                                  g_pCampaignGlobals);
        }
        CloseDataFile(file);
    }
    _findclose((long)searchHandle);
    file = OpenDataFileOrDie(g_szPersonnelFileName);
    if (file < 0) {
        ReportFatalErrorCode("006");
    }
    strcpy(g_stCurrentPilotProfile.firstName,
           g_stPilotNameVariables.firstName);
    strcpy(g_stCurrentPilotProfile.lastName,
           g_stPilotNameVariables.lastName);
    strcpy(g_stCurrentPilotProfile.callsign,
           g_stPilotNameVariables.callsign);
    SeekDataFile(file, (stateBytes + 0x86) << 3, 0);
    WriteDataFileAtOffset(file, -1, 2, &inUse);
    WriteDataFileAtOffset(file, -1, 0x60,
                          &g_stCurrentPilotProfile);
    WriteDataFileAtOffset(file, -1, sizeof(g_szSaveSlotSignature),
                          g_szSaveSlotSignature);
    WriteDataFileAtOffset(file, -1, stateBytes,
                          g_pCampaignGlobals);
    CloseDataFile(file);
    if (logicalFile != 0) {
        ReleasePacketSlot((void **)&g_pCampaignGlobals);
    }
}

/* Function start: 0x43456E */
void RestorePersonnelMenuBackground(Viewport *viewport)
{
    Viewport savedViewport;

    savedViewport = *viewport;
    DrawSpriteDefault(&savedViewport, 0, 0,
                      g_pPersonnelMenuBackdrop, 1);
}

/* Function start: 0x4345A1 */
void DrawPilotSaveMenuChoices(short choice)
{
    if (choice == g_nPersonnelMenuHighlight)
        return;
    DisableMouseCursorDrawing();
    if (choice == PERSONNEL_KEY_L) {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_nMenuSelectedCol, -1);
    } else {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_10_nMenuDarkColour, -1);
    }
    SetTextCursor(0x5a, 0x38);
    DrawFormattedText("[L]oad a personnel file");
    if (choice == PERSONNEL_KEY_S) {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_nMenuSelectedCol, -1);
    } else {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_10_nMenuDarkColour, -1);
    }
    SetTextCursor(0x5a, 0x4c);
    DrawFormattedText("[S]ave a personnel file");
    if (choice == PERSONNEL_KEY_D) {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_nMenuSelectedCol, -1);
    } else {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_10_nMenuDarkColour, -1);
    }
    SetTextCursor(0x5a, 0x60);
    DrawFormattedText("[D]elete a personnel file");
    g_nPersonnelMenuHighlight = choice;
    EnableMouseCursorDrawing();
}

void DrawFormattedText(const char *format, ...);

/* Function start: 0x4346E9 */
void DrawPersonnelSourceChoices(short choice)
{
    if (choice == g_nPersonnelMenuHighlight)
        return;
    DisableMouseCursorDrawing();
    if (choice == PERSONNEL_KEY_W) {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_nMenuSelectedCol, -1);
    } else {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_10_nMenuDarkColour, -1);
    }
    SetTextCursor(0x50, 0x2e);
    DrawFormattedText("[W]ing Commander");
    if (choice == PERSONNEL_KEY_D) {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_nMenuSelectedCol, -1);
    } else {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_10_nMenuDarkColour, -1);
    }
    SetTextCursor(0x50, 0x42);
    DrawFormattedText("[D]os Version of Wing Commander");
    g_nPersonnelMenuHighlight = choice;
    EnableMouseCursorDrawing();
}

void InitializeTextContextFromFont(TextContext *context, short fontIndex, unsigned char colour, signed char background);

/* Function start: 0x4347D6 */
void DrawPersonnelMenuChoices(short choice)
{
    if (g_nPersonnelMenuHighlight == choice)
        return;
    DisableMouseCursorDrawing();
    if (choice == PERSONNEL_KEY_T) {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_nMenuSelectedCol, -1);
    } else {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_10_nMenuDarkColour, -1);
    }
    SetTextCursor(0x3c, 0x38);
    DrawFormattedText("[T]ransfer an existing personnel file");
    if (choice == PERSONNEL_KEY_C) {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_nMenuSelectedCol, -1);
    } else {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_10_nMenuDarkColour, -1);
    }
    SetTextCursor(0x3c, 0x4c);
    DrawFormattedText("[C]reate a new personnel file");
    g_nPersonnelMenuHighlight = choice;
    EnableMouseCursorDrawing();
}

#pragma function(strlen)
/* Function start: 0x4348C3 */
void DrawPersonnelFileSlot(short slot)
{
    if (g_apPersonnelFileSlots[slot]->occupied == 0) {
        SetTextCursor(0x32, (unsigned short)(slot * 12 + 0x12));
        DrawFormattedText(
            "%d ----------------------------------------------------------",
            slot);
        return;
    }

    SetTextCursor(0x32, (unsigned short)(slot * 12 + 0x12));
    if (strlen(g_apPersonnelPilotProfiles[slot]->lastName) +
            strlen(g_apPersonnelPilotProfiles[slot]->callsign) +
            strlen(g_apPersonnelPilotProfiles[slot]->firstName) >
        0x16) {
        DrawFormattedText(
            "%d--%s %c. \"%s\" %s",
            slot,
            g_apszShortPilotRankNames[
                g_apPersonnelPilotProfiles[slot]->rank],
            g_apPersonnelPilotProfiles[slot]->firstName[0],
            g_apPersonnelPilotProfiles[slot]->callsign,
            g_apPersonnelPilotProfiles[slot]->lastName);
    } else {
        DrawFormattedText(
            "%d--%s %s \"%s\" %s",
            slot,
            g_apszPilotRankNames[
                g_apPersonnelPilotProfiles[slot]->rank],
            g_apPersonnelPilotProfiles[slot]->firstName,
            g_apPersonnelPilotProfiles[slot]->callsign,
            g_apPersonnelPilotProfiles[slot]->lastName);
    }
    SetTextCursor(0xc6, (unsigned short)(slot * 12 + 0x12));
    DrawFormattedText(
        "%d",
        g_apPersonnelPilotProfiles[slot]->field_41);
    SetTextCursor(0xde, (unsigned short)(slot * 12 + 0x12));
    DrawFormattedText(
        "%d",
        g_apPersonnelPilotProfiles[slot]->field_43);
    SetTextCursor(0x36, (unsigned short)(slot * 12 + 0x18));
    DrawFormattedText(
        "%s",
        g_apPersonnelFileSlots[slot]->description);
}

void DrawTransferredPilotRow(short row);

/* Function start: 0x434ACE */
void HighlightPersonnelFileSlot(short choice)
{
    short slot;

    if (choice == g_nPersonnelMenuHighlight)
        return;
    if (choice == PERSONNEL_KEY_0) {
        slot = 0;
    } else {
        slot = (short)(choice - 1);
    }
    DisableMouseCursorDrawing();
    if (choice != -1) {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 2,
            (unsigned char)g_nMenuSelectedCol, -1);
        DrawPersonnelFileSlot(slot);
    }
    if (g_nPersonnelMenuHighlight != -1) {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 2,
            (unsigned char)g_10_nMenuDarkColour, -1);
        if (g_nPersonnelMenuHighlight == PERSONNEL_KEY_0) {
            slot = 0;
        } else {
            slot = (short)(g_nPersonnelMenuHighlight - 1);
        }
        DrawPersonnelFileSlot(slot);
    }
    g_nPersonnelMenuHighlight = choice;
    EnableMouseCursorDrawing();
}

/* Function start: 0x434BB5 */
short SelectPersonnelFileSlot(void)
{
    short hover;
    short index;

    DisableMouseCursorDrawing();
    InitializeTextContextFromFont(&g_3_stDefaultStringContext, 2,
                                  (unsigned char)
                                      g_10_nMenuDarkColour,
                                  -1);
    for (index = 0; index < 8; index++) {
        DrawPersonnelFileSlot(index);
    }
    g_nPersonnelMenuHighlight = -1;
    hover = 0x0b;
    HighlightPersonnelFileSlot(hover);
    DisableMouseCursorDrawing();
    g_cCurrentKey = (signed char)0x80;
    SetPersonnelMousePosition(0xdc, 0x12);
    while (g_cCurrentKey == (signed char)0x80) {
        PollPersonnelMenuInput();
        if (g_37_stPersonnelMousePointerState.x > 0x28 &&
            g_37_stPersonnelMousePointerState.x < 0xec) {
            if (g_37_stPersonnelMousePointerState.y >= 0x12 &&
                g_37_stPersonnelMousePointerState.y <= 0x1c) {
                hover = 0x0b;
                HighlightPersonnelFileSlot(hover);
                SetMouseCursorShape(
                    g_18_pEventManager->cursorShape, 1);
            } else if (g_37_stPersonnelMousePointerState.y >= 0x1d &&
                       g_37_stPersonnelMousePointerState.y <= 0x6e) {
                hover = (short)((g_37_stPersonnelMousePointerState.y - 0x1d) /
                                12 + 2);
                HighlightPersonnelFileSlot(hover);
                SetMouseCursorShape(
                    g_18_pEventManager->cursorShape, 1);
            } else {
                hover = -1;
                HighlightPersonnelFileSlot(hover);
                SetMouseCursorShape(
                    g_18_pEventManager->cursorShape, 0);
            }
        } else {
            hover = -1;
            HighlightPersonnelFileSlot(hover);
            SetMouseCursorShape(
                g_18_pEventManager->cursorShape, 0);
        }
        g_bPersonnelMenuDrawing = 0;
        DrawPersonnelMenuBackdrop(g_pPersonnelMenuBackdrop);
        g_bPersonnelMenuDrawing = 1;
        EnableMouseCursorDrawing();
        RefreshMemoryStatusOverlay();
        if ((g_cCurrentKey == PERSONNEL_KEY_ENTER ||
             g_cCurrentKey == PERSONNEL_KEY_SPACE) &&
            hover != -1) {
            g_cCurrentKey = (signed char)hover;
        }
        if (g_cCurrentKey == 1 ||
            ((g_cCurrentKey == PERSONNEL_KEY_ENTER ||
              g_cCurrentKey == PERSONNEL_KEY_SPACE) &&
             hover == -1)) {
            return -1;
        }
        switch (g_cCurrentKey) {
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
        g_cCurrentKey = (signed char)0x80;
    }
    /* Retail falls off the end here; EAX still holds the key loaded by
     * the loop test. */
#ifdef SDL_PORT
    return g_cCurrentKey;
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
    DrawFormattedText(g_3_apszPersonnelImportCaptionsTable[row]);
    SetTextCursor(0xc4, (unsigned short)(row * 10 + 0x1e));
    DrawFormattedText(
        g_3_apszPersonnelImportCaptionsTable[row] + 0x28);
    SetTextCursor(0xda, (unsigned short)(row * 10 + 0x1e));
    DrawFormattedText(
        g_3_apszPersonnelImportCaptionsTable[row] + 0x2c);
}

void SetTextCursor(unsigned short a, unsigned short b);
void CopyHugeMemoryOverlapSafe(void *destination, void *source, int count);
void DrawPersonnelMenuBackdrop(void *scenePacket);

/* Function start: 0x434F7D */
void HighlightTransferredPilot(short choice)
{
    short row;

    if (choice == g_nPersonnelMenuHighlight)
        return;
    if (choice == PERSONNEL_KEY_0) {
        row = 0;
    } else {
        row = (short)(choice - 1);
    }
    DisableMouseCursorDrawing();
    if (choice != -1) {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_nMenuSelectedCol, -1);
        DrawTransferredPilotRow(row);
    }
    if (g_nPersonnelMenuHighlight != -1) {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_10_nMenuDarkColour, -1);
        if (g_nPersonnelMenuHighlight == PERSONNEL_KEY_0) {
            row = 0;
        } else {
            row = (short)(g_nPersonnelMenuHighlight - 1);
        }
        DrawTransferredPilotRow(row);
    }
    g_nPersonnelMenuHighlight = choice;
    EnableMouseCursorDrawing();
}

/* Function start: 0x435064 */
short SelectTransferredPilot(short count)
{
    short hover;
    short index;

    DisableMouseCursorDrawing();
    RestorePersonnelMenuBackground(&g_stSecondaryViewBufferBlock);
    InitializeTextContextFromFont(&g_3_stDefaultStringContext, 2,
                                  (unsigned char)
                                      g_10_nMenuDarkColour,
                                  -1);
    SetTextCursor(0x32, 0x0f);
    DrawFormattedText("Select a character to transfer");
    SetTextCursor(0xba, 0x16);
    DrawFormattedText("Missions Kills");
    InitializeTextContextFromFont(&g_3_stDefaultStringContext, 1,
                                  (unsigned char)
                                      g_10_nMenuDarkColour,
                                  -1);
    for (index = 0; index < count; index++) {
        DrawTransferredPilotRow(index);
    }
    g_nPersonnelMenuHighlight = -1;
    hover = 0x0b;
    HighlightTransferredPilot(hover);
    DisableMouseCursorDrawing();
    g_cCurrentKey = (signed char)0x80;
    SetPersonnelMousePosition(0xdc, 0x12);
    while (g_cCurrentKey == (signed char)0x80) {
        PollPersonnelMenuInput();
        if (g_37_stPersonnelMousePointerState.x > 0x28 &&
            g_37_stPersonnelMousePointerState.x < 0xec) {
            if (g_37_stPersonnelMousePointerState.y >= 0x1e &&
                g_37_stPersonnelMousePointerState.y <= 0x27) {
                hover = 0x0b;
                HighlightTransferredPilot(hover);
                SetMouseCursorShape(
                    g_18_pEventManager->cursorShape, 1);
            } else if (g_37_stPersonnelMousePointerState.y >= 0x28 &&
                       g_37_stPersonnelMousePointerState.y <=
                           (count - 2) * 10 + 0x31) {
                hover = (short)((g_37_stPersonnelMousePointerState.y - 0x28) /
                                10 + 2);
                HighlightTransferredPilot(hover);
                SetMouseCursorShape(
                    g_18_pEventManager->cursorShape, 1);
            } else {
                hover = -1;
                HighlightTransferredPilot(hover);
                SetMouseCursorShape(g_18_pEventManager->cursorShape,
                                    0);
            }
        } else {
            hover = -1;
            HighlightTransferredPilot(hover);
            SetMouseCursorShape(
                g_18_pEventManager->cursorShape, 0);
        }
        g_bPersonnelMenuDrawing = 0;
        DrawPersonnelMenuBackdrop(g_pPersonnelMenuBackdrop);
        g_bPersonnelMenuDrawing = 1;
        EnableMouseCursorDrawing();
        RefreshMemoryStatusOverlay();
        if ((g_cCurrentKey == PERSONNEL_KEY_ENTER ||
             g_cCurrentKey == PERSONNEL_KEY_SPACE) &&
            hover != -1) {
            g_cCurrentKey = (signed char)hover;
        }
        if (g_cCurrentKey == 1 ||
            ((g_cCurrentKey == PERSONNEL_KEY_ENTER ||
              g_cCurrentKey == PERSONNEL_KEY_SPACE) &&
             hover == -1)) {
            return -1;
        }
        switch (g_cCurrentKey) {
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
        g_cCurrentKey = (signed char)0x80;
    }
    /* Retail falls off the end here; EAX still holds the key loaded by
     * the loop test. */
#ifdef SDL_PORT
    return g_cCurrentKey;
#endif
}

/* Function start: 0x4353D4 */
void RunPilotSaveLoadMenu(void)
{
    struct _finddata_t fileSearchData;
    int campaignBytes;
    char reply;
    short confirm;
    short file;
    short i;
    short hover;
    short redrawFlag;
    short finishedMenu;
    short slot;

    finishedMenu = 0;
    redrawFlag = 0;
    g_pPersonnelMenuBackdrop = FetchDiskPacketRetrying(
        "options.v00",
        (short)(g_1_nCockpitArchiveVariantNumber * 3 + 1), 0);
    ClearViewport(&g_stSecondaryViewBufferBlock, 0);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock, 0, 0,
                      g_pPersonnelMenuBackdrop, 0);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock, 0, 0,
                      g_pPersonnelMenuBackdrop, 1);
    SetInputViewport(&g_stSecondaryViewBufferBlock);
    campaignBytes = g_pCampaignGlobals->wordCount * 2;
    g_pPersonnelCampaignSnapshot =
        AllocateTaggedMemory(campaignBytes, 0x40);
    if (g_pPersonnelCampaignSnapshot == 0) {
        ReportFatalErrorCode("007");
    }

    /* The find handle shares storage with the menu's exit flag; the
     * original reuses the same short once the file has been sized. */
    finishedMenu = (short)_findfirst(g_szPersonnelFileName, &fileSearchData);
    if (finishedMenu == -1 ||
        (campaignBytes + 0x86) * 9 != fileSearchData.size) {
        file = (short)_open(g_szPersonnelFileName, 0x8301, 0x180);
        if (file < 0) {
            ReportFatalErrorCode("008");
        }
        CopyHugeMemoryOverlapSafe(g_pPersonnelCampaignSnapshot,
                                  g_pCampaignGlobals,
                                  campaignBytes);
        for (i = 0; i < 9; i++) {
            _write(file, &redrawFlag, 2);
            _write(file, &g_stDefaultPilotProfile, 0x60);
            _write(file, g_szSaveSlotSignature,
                   sizeof(g_szSaveSlotSignature));
            _write(file, g_pPersonnelCampaignSnapshot,
                   campaignBytes);
        }
        _close(file);
    }
    _findclose((long)finishedMenu);

    file = (short)_open(g_szPersonnelFileName, 0x8000);
    for (i = 0; i < 8; i++) {
        g_apPersonnelFileSlots[i] =
            AllocateTaggedMemory(0x26, 0x40);
        if (g_apPersonnelFileSlots[i] == 0) {
            ReportFatalErrorCode("009");
        }
        g_apPersonnelPilotProfiles[i] =
            AllocateTaggedMemory(0x60, 0x40);
        if (g_apPersonnelPilotProfiles[i] == 0) {
            ReportFatalErrorCode("010");
        }
        _read(file, g_apPersonnelFileSlots[i], 2);
        _read(file, g_apPersonnelPilotProfiles[i], 0x60);
        _read(file, g_apPersonnelFileSlots[i]->description,
              0x24);
        _read(file, g_pPersonnelCampaignSnapshot,
              campaignBytes);
    }
    _close(file);

    finishedMenu = 0;
    SetPersonnelMousePosition(0xc8, 0x36);
    while (finishedMenu == 0) {
        DisableMouseCursorDrawing();
        g_3_stDefaultStringContext.viewport =
            &g_stSecondaryViewBufferBlock;
        g_3_stDefaultStringContext.text =
            g_6_szDefaultTextBufferText;
        RestorePersonnelMenuBackground(&g_stSecondaryViewBufferBlock);
        SetTextContext(&g_3_stDefaultStringContext);
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_10_nMenuDarkColour, -1);
        SetTextCursor(0x50, 0x12);
        DrawFormattedText("TERRAN CONFEDERATION NAVY");
        SetTextCursor(0x60, 0x1c);
        DrawFormattedText("PERSONNEL DATABASE");
        hover = 0x26;
        g_nPersonnelMenuHighlight = -1;
        DrawPilotSaveMenuChoices(hover);
        DisableMouseCursorDrawing();
        RefreshMemoryStatusOverlay();
        g_cCurrentKey = (signed char)0x80;
        while (g_cCurrentKey == (signed char)0x80) {
            PollPersonnelMenuInput();
            if (g_37_stPersonnelMousePointerState.x > 0x28 &&
                g_37_stPersonnelMousePointerState.x < 0xea) {
                if (g_37_stPersonnelMousePointerState.y >= 0x30 &&
                    g_37_stPersonnelMousePointerState.y <= 0x43) {
                    hover = 0x26;
                    DrawPilotSaveMenuChoices(hover);
                    SetMouseCursorShape(
                        g_18_pEventManager->cursorShape, 1);
                } else if (g_37_stPersonnelMousePointerState.y >= 0x44 &&
                           g_37_stPersonnelMousePointerState.y <= 0x57) {
                    hover = 0x1f;
                    DrawPilotSaveMenuChoices(hover);
                    SetMouseCursorShape(
                        g_18_pEventManager->cursorShape, 1);
                } else if (g_37_stPersonnelMousePointerState.y >= 0x58 &&
                           g_37_stPersonnelMousePointerState.y <= 0x6b) {
                    hover = 0x20;
                    DrawPilotSaveMenuChoices(hover);
                    SetMouseCursorShape(
                        g_18_pEventManager->cursorShape, 1);
                } else {
                    hover = -1;
                    DrawPilotSaveMenuChoices(hover);
                    SetMouseCursorShape(
                        g_18_pEventManager->cursorShape, 0);
                }
            } else {
                hover = -1;
                DrawPilotSaveMenuChoices(hover);
                SetMouseCursorShape(
                    g_18_pEventManager->cursorShape, 0);
            }
            g_bPersonnelMenuDrawing = 0;
            DrawPersonnelMenuBackdrop(g_pPersonnelMenuBackdrop);
            g_bPersonnelMenuDrawing = 1;
            EnableMouseCursorDrawing();
            RefreshMemoryStatusOverlay();
            redrawFlag = 0;
            if ((g_cCurrentKey == PERSONNEL_KEY_ENTER ||
                 g_cCurrentKey == PERSONNEL_KEY_SPACE) &&
                hover != -1) {
                g_cCurrentKey = (signed char)hover;
            }
            if (g_cCurrentKey == 1 ||
                ((g_cCurrentKey == PERSONNEL_KEY_ENTER ||
                  g_cCurrentKey == PERSONNEL_KEY_SPACE) &&
                 hover == -1)) {
                finishedMenu++;
                redrawFlag++;
            }
            switch (toupper(g_cCurrentKey)) {
            case PERSONNEL_KEY_L:
                redrawFlag = 1;
                DisableMouseCursorDrawing();
                RestorePersonnelMenuBackground(
                    &g_stSecondaryViewBufferBlock);
                RefreshMemoryStatusOverlay();
                InitializeTextContextFromFont(
                    &g_3_stDefaultStringContext, 2,
                    (unsigned char)g_10_nMenuDarkColour, -1);
                SetTextCursor(0x50, 0x0c);
                DrawFormattedText("Select a game to load");
                SetTextCursor(0xb8, 0x0c);
                DrawFormattedText("Missions Kills");
                slot = SelectPersonnelFileSlot();
                if (slot != -1 &&
                    g_apPersonnelFileSlots[slot]->occupied != 0) {
                    file = (short)_open(g_szPersonnelFileName,
                                        0x8000);
                    if (file >= 0) {
                        /* Keep the slot-size addition signed so CL 4.1 does
                         * not fold it into the following profile size. */
                        lseek(file,
                               (campaignBytes + (int)sizeof(PersonnelFileSlot) +
                                sizeof(PilotProfile)) * slot, 0);
                        _read(file, g_apPersonnelFileSlots[slot], 2);
                        _read(file, g_apPersonnelPilotProfiles[slot],
                              0x60);
                        _read(file,
                              g_apPersonnelFileSlots[slot]->description,
                              0x24);
                        strcpy(g_stPilotNameVariables.firstName,
                               g_apPersonnelPilotProfiles[slot]->firstName);
                        strcpy(g_stCurrentPilotProfile.firstName,
                               g_apPersonnelPilotProfiles[slot]->firstName);
                        strcpy(g_stPilotNameVariables.lastName,
                               g_apPersonnelPilotProfiles[slot]->lastName);
                        strcpy(g_stCurrentPilotProfile.lastName,
                               g_apPersonnelPilotProfiles[slot]->lastName);
                        if (g_nOriginDevUnlock != 0) {
                            strcpy(
                                g_apPersonnelPilotProfiles[slot]->callsign,
                                "CHEATER");
                        }
                        strcpy(g_stPilotNameVariables.callsign,
                               g_apPersonnelPilotProfiles[slot]->callsign);
                        strcpy(g_stCurrentPilotProfile.callsign,
                               g_apPersonnelPilotProfiles[slot]->callsign);
                        g_stCurrentPilotProfile.field_43 =
                            g_apPersonnelPilotProfiles[slot]->field_43;
                        g_stCurrentPilotProfile.field_41 =
                            g_apPersonnelPilotProfiles[slot]->field_41;
                        g_stCurrentPilotProfile.rank =
                            g_apPersonnelPilotProfiles[slot]->rank;
                        _read(file, g_pPersonnelCampaignSnapshot,
                              campaignBytes);
                        CopyHugeMemoryOverlapSafe(
                            g_pCampaignGlobals,
                            g_pPersonnelCampaignSnapshot,
                            campaignBytes);
                        _close(file);
                    } else {
                        ReportFatalErrorCode("011");
                    }
                }
                break;
            case PERSONNEL_KEY_D:
                redrawFlag = 1;
                DisableMouseCursorDrawing();
                RestorePersonnelMenuBackground(
                    &g_stSecondaryViewBufferBlock);
                RefreshMemoryStatusOverlay();
                InitializeTextContextFromFont(
                    &g_3_stDefaultStringContext, 2,
                    (unsigned char)g_10_nMenuDarkColour, -1);
                SetTextCursor(0xb8, 0x0c);
                DrawFormattedText("Missions Kills");
                SetTextCursor(0x50, 0x0c);
                DrawFormattedText("Select a slot to delete");
                slot = SelectPersonnelFileSlot();
                if (slot != -1 &&
                    g_apPersonnelFileSlots[slot]->occupied != 0) {
                    DisableMouseCursorDrawing();
                    RestorePersonnelMenuBackground(
                        &g_stSecondaryViewBufferBlock);
                    InitializeTextContextFromFont(
                        &g_3_stDefaultStringContext, 2,
                        (unsigned char)g_10_nMenuDarkColour, -1);
                    confirm = 1;
                    SetTextCursor(0x32, 0x32);
                    DrawFormattedText("Are you sure?");
                    InitializeTextContextFromFont(
                        &g_3_stDefaultStringContext, 2,
                        (unsigned char)g_nMenuSelectedCol,
                        (unsigned char)g_3_nOptionMessageColour);
                    SetTextCursor(0x6a, 0x32);
                    DrawFormattedText("Yes");
                    RefreshMemoryStatusOverlay();
                    reply = (char)WaitForQueuedInputPress();
                    while (reply != 0x0d) {
                        SetTextCursor(0x6a, 0x32);
                        if (toupper(reply) == 'Y') {
                            DrawFormattedText("Yes");
                            confirm = 1;
                        } else {
                            DrawFormattedText("No ");
                            confirm = 0;
                        }
                        RefreshMemoryStatusOverlay();
                        reply = (char)WaitForQueuedInputPress();
                    }
                    if (confirm != 0) {
                        g_apPersonnelFileSlots[slot]->occupied = 0;
                        file = (short)_open(g_szPersonnelFileName, 0x8001);
                        lseek(file,
                               (campaignBytes + (int)sizeof(PersonnelFileSlot) +
                                sizeof(PilotProfile)) * slot, 0);
                        _write(file,
                               g_apPersonnelFileSlots[slot], 2);
                        _close(file);
                    }
                }
                break;
            case PERSONNEL_KEY_S:
                redrawFlag = 1;
                DisableMouseCursorDrawing();
                RestorePersonnelMenuBackground(
                    &g_stSecondaryViewBufferBlock);
                InitializeTextContextFromFont(
                    &g_3_stDefaultStringContext, 2,
                    (unsigned char)g_10_nMenuDarkColour, -1);
                SetTextCursor(0xb8, 0x0c);
                DrawFormattedText("Missions Kills");
                SetTextCursor(0x50, 0x0c);
                DrawFormattedText("Select a slot to save in");
                RefreshMemoryStatusOverlay();
                slot = SelectPersonnelFileSlot();
                if (slot != -1) {
                    DisableMouseCursorDrawing();
                    RestorePersonnelMenuBackground(
                        &g_stSecondaryViewBufferBlock);
                    RefreshMemoryStatusOverlay();
                    InitializeTextContextFromFont(
                        &g_3_stDefaultStringContext, 2,
                        (unsigned char)g_10_nMenuDarkColour, -1);
                    confirm = 1;
                    if (g_apPersonnelFileSlots[slot]->occupied != 0) {
                        SetTextCursor(0x32, 0x32);
                        DrawFormattedText("Are you sure?");
                        InitializeTextContextFromFont(
                            &g_3_stDefaultStringContext, 2,
                            (unsigned char)g_nMenuSelectedCol,
                            (unsigned char)g_3_nOptionMessageColour);
                        SetTextCursor(0x6a, 0x32);
                        DrawFormattedText("Yes");
                        RefreshMemoryStatusOverlay();
                        reply = (char)WaitForQueuedInputPress();
                        while (reply != 0x0d) {
                            SetTextCursor(0x6a, 0x32);
                            if (toupper(reply) == 'Y') {
                                DrawFormattedText("Yes");
                                confirm = 1;
                            } else {
                                DrawFormattedText("No ");
                                confirm = 0;
                            }
                            RefreshMemoryStatusOverlay();
                            reply = (char)WaitForQueuedInputPress();
                            ServiceSoundSystem();
                        }
                    }
                    InitializeTextContextFromFont(
                        &g_3_stDefaultStringContext, 2,
                        (unsigned char)g_10_nMenuDarkColour, -1);
                    if (confirm != 0) {
                        g_apPersonnelFileSlots[slot]->occupied = 1;
                        g_apPersonnelFileSlots[slot]->description[0] = 0;
                        SetTextCursor(0x32, 0x46);
                        DrawFormattedText("%s", "Saved game comments:");
                        RefreshMemoryStatusOverlay();
                        InitializeTextContextFromFont(
                            &g_3_stDefaultStringContext, 2,
                            (unsigned char)g_nMenuSelectedCol, -1);
                        ReadPersonnelTextField(
                            0x32, 0x4c, 2, "",
                            g_apPersonnelFileSlots[slot]->description,
                            0x20, 0, 0);
                        strcpy(g_apPersonnelPilotProfiles[slot]->firstName,
                               g_stCurrentPilotProfile.firstName);
                        strcpy(g_apPersonnelPilotProfiles[slot]->lastName,
                               g_stCurrentPilotProfile.lastName);
                        strcpy(g_apPersonnelPilotProfiles[slot]->callsign,
                               g_stCurrentPilotProfile.callsign);
                        g_apPersonnelPilotProfiles[slot]->rank =
                            g_stCurrentPilotProfile.rank;
                        g_apPersonnelPilotProfiles[slot]->field_41 =
                            g_stCurrentPilotProfile.field_41;
                        g_apPersonnelPilotProfiles[slot]->field_43 =
                            g_stCurrentPilotProfile.field_43;
                        file = (short)_open(g_szPersonnelFileName, 0x8001);
                        lseek(file,
                               (campaignBytes + (int)sizeof(PersonnelFileSlot) +
                                sizeof(PilotProfile)) * slot, 0);
                        _write(file, g_apPersonnelFileSlots[slot], 2);
                        _write(file, g_apPersonnelPilotProfiles[slot],
                               0x60);
                        _write(file,
                               g_apPersonnelFileSlots[slot]->description,
                               0x24);
                        CopyHugeMemoryOverlapSafe(
                            g_pPersonnelCampaignSnapshot,
                            g_pCampaignGlobals, campaignBytes);
                        _write(file, g_pPersonnelCampaignSnapshot,
                               campaignBytes);
                        _close(file);
                    }
                }
                break;
            }
            if (redrawFlag != 0) {
                g_cCurrentKey = 0;
            }
        }
        g_cCurrentKey = (signed char)0x80;
    }

    g_pActiveScenePacket = 0;
    ReleasePacketSlot(&g_stSceneZoneTable.data);
    ReleasePacketSlot(&g_stSceneTextList.data);
    FreePacketAndClear(&g_pPersonnelMenuBackdrop, 0);
    g_3_stDefaultStringContext.viewport = &g_101_stScrViewInfo;
    ReleasePacketHandle(g_pPersonnelCampaignSnapshot);
    for (i = 0; i < 8; i++) {
        ReleasePacketHandle(g_apPersonnelFileSlots[i]);
        ReleasePacketHandle(g_apPersonnelPilotProfiles[i]);
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
    RestorePersonnelMenuBackground(&g_stSecondaryViewBufferBlock);
    SetPersonnelMousePosition(200, 0x2c);
    while (done == 0) {
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_10_nMenuDarkColour, -1);
        SetTextCursor(0x3c, 0x1e);
        DrawFormattedText("Transfer a character from:");
        RefreshMemoryStatusOverlay();
        g_nPersonnelMenuHighlight = -1;
        DrawPersonnelSourceChoices(focus);
        DisableMouseCursorDrawing();
        g_cCurrentKey = (signed char)0x80;
        while (g_cCurrentKey == (signed char)0x80) {
            PollPersonnelMenuInput();
            if (g_37_stPersonnelMousePointerState.x > 0x1e &&
                g_37_stPersonnelMousePointerState.x < 0xe0) {
                if (g_37_stPersonnelMousePointerState.y >= 0x26 &&
                    g_37_stPersonnelMousePointerState.y <= 0x39) {
                    focus = PERSONNEL_KEY_W;
                    DrawPersonnelSourceChoices(focus);
                    SetMouseCursorShape(
                        g_18_pEventManager->cursorShape, 1);
                } else if (g_37_stPersonnelMousePointerState.y >= 0x3a &&
                           g_37_stPersonnelMousePointerState.y <= 0x4d) {
                    focus = PERSONNEL_KEY_D;
                    DrawPersonnelSourceChoices(focus);
                    SetMouseCursorShape(
                        g_18_pEventManager->cursorShape, 1);
                } else {
                    focus = -1;
                    DrawPersonnelSourceChoices(focus);
                    SetMouseCursorShape(
                        g_18_pEventManager->cursorShape, 0);
                }
            } else {
                focus = -1;
                DrawPersonnelSourceChoices(focus);
                SetMouseCursorShape(g_18_pEventManager->cursorShape,
                                    0);
            }
            g_bPersonnelMenuDrawing = 0;
            DrawPersonnelMenuBackdrop(g_pPersonnelMenuBackdrop);
            g_bPersonnelMenuDrawing = 1;
            EnableMouseCursorDrawing();
            RefreshMemoryStatusOverlay();
            handled = 0;
            choice = -1;
            if ((g_cCurrentKey == PERSONNEL_KEY_ENTER ||
                 g_cCurrentKey == PERSONNEL_KEY_SPACE) &&
                focus != -1) {
                g_cCurrentKey = (signed char)focus;
            }
            if (g_cCurrentKey == 1 ||
                ((g_cCurrentKey == PERSONNEL_KEY_ENTER ||
                  g_cCurrentKey == PERSONNEL_KEY_SPACE) &&
                 focus == -1)) {
                done++;
                handled++;
            }
            switch (toupper((int)g_cCurrentKey)) {
            case PERSONNEL_KEY_W:
                choice = 0;
                done++;
                handled++;
                g_nLegacySaveSource = 0;
                break;
            case PERSONNEL_KEY_D:
                choice = 0;
                done++;
                handled++;
                g_nLegacySaveSource = 1;
                break;
            }
            if (handled != 0) {
                g_cCurrentKey = 0;
            }
        }
    }
    RestorePersonnelMenuBackground(&g_stSecondaryViewBufferBlock);
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
        &g_3_stDefaultStringContext, 2,
        (unsigned char)g_10_nMenuDarkColour, -1);
    if (g_nLegacySaveSource == 0) {
        installedPath = GetLegacyGameDataPath();
        if (installedPath != 0) {
            strcpy(g_1_szLegacySaveFolder, installedPath);
        } else {
            strcpy(g_1_szLegacySaveFolder, "");
        }
    } else {
        strcpy(g_1_szLegacySaveFolder, "");
        RestorePersonnelMenuBackground(&g_stSecondaryViewBufferBlock);
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
            g_1_szLegacySaveFolder, 0x19, 0, 1);
        EnableMouseCursorDrawing();
    }
    if (pathEntered == 0) {
        return 0;
    }
    if (strlen(g_1_szLegacySaveFolder) >= 2) {
        for (index = (short)(strlen(g_1_szLegacySaveFolder) - 1);
             index >= 0; index--) {
            if (g_1_szLegacySaveFolder[index] == '\\')
                break;
            isAlnum = isalnum(g_1_szLegacySaveFolder[index]);
            if (isAlnum != 0) {
                strcat(g_1_szLegacySaveFolder, "\\");
                break;
            }
        }
    }
    strcat(g_1_szLegacySaveFolder,
           g_apszSaveGameFileNames[source]);
    for (index = 0;
         strlen(g_1_szLegacySaveFolder) > (unsigned int)index;
         index++) {
        if (g_1_szLegacySaveFolder[index] == ':') {
            if (index == 0) {
                return 0;
            }
            driveIndex = (short)(toupper(
                g_1_szLegacySaveFolder[index - 1]) - 'A');
            PersonnelDriveHook(driveIndex);
        }
    }
    handle = (short)_findfirst(g_1_szLegacySaveFolder, &findData);
    if (handle == -1) {
        return 0;
    }
    return 1;
}

/* Function start: 0x436A4D */
void ShowNoTransferablePilots(void)
{
    RestorePersonnelMenuBackground(&g_stSecondaryViewBufferBlock);
    SetTextCursor(0x50, 0x3c);
    DrawFormattedText("No valid characters found.");
    RefreshMemoryStatusOverlay();
    g_cCurrentKey = (signed char)0x80;
    WaitForAnyInputPress();
}

/* Function start: 0x436A8F */
short RunPilotDatabaseMenu(void)
{
    short finishedDatabase = 0;
    short actionHandledFlag = 0;
    char firstNameText[13] = "Christopher";
    char surname[13] = "Blair";
    char callsign[13] = "Maverick";
    short accepted = 0;
    char captionBuffer[80];
    SaveGameDiskRecord *record;
    PilotRecord *pilotRecord;
    short handle;
    short inputResult;
    short transferCount;
    short saveSlot;
    short choice;
    short selectedPilotIndex;
    short hoverCommand;
    short driveToRestore;

    g_stSecondaryViewBufferBlock.left = 0;
    g_stSecondaryViewBufferBlock.right = 319;
    g_stSecondaryViewBufferBlock.top = 0;
    g_stSecondaryViewBufferBlock.bottom = 199;
    if ((short)AllocateViewport(
            &g_stSecondaryViewBufferBlock,
            g_cSecondaryViewBufferColour, 0) == 0) {
        ReportFatalErrorCode("012");
    }
    SetInputViewport(&g_stSecondaryViewBufferBlock);
    DisableMouseCursorDrawing();
    InitializeTextContextFromFont(&g_3_stDefaultStringContext, 0,
                                  g_bPrimaryViewBufferColour, -1);
    InitializeTextContextFromFont(&g_3_stDefaultStringContext, 2,
                                  g_bPrimaryViewBufferColour, -1);
    g_pPersonnelMenuBackdrop = FetchDiskPacketRetrying(
        "options.v00", (short)(g_1_nCockpitArchiveVariantNumber * 3 + 1),
        0);
    SetMenuInputPump();
    ClearViewport(&g_stSecondaryViewBufferBlock, 0);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock, 0, 0,
                      g_pPersonnelMenuBackdrop, 0);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock, 0, 0,
                      g_pPersonnelMenuBackdrop, 1);
    record = AllocateTaggedMemory(0x33c, 0x40);
    if (record == 0) {
        ReportFatalErrorCode("013");
    }
    for (saveSlot = 0; saveSlot < 8; saveSlot++) {
        g_3_apszPersonnelImportCaptionsTable[saveSlot] =
            AllocateTaggedMemory(0x50, 0x40);
        if (g_3_apszPersonnelImportCaptionsTable[saveSlot] == 0) {
            ReportFatalErrorCode("014");
        }
    }
    SetInputViewport(&g_stSecondaryViewBufferBlock);
    while (finishedDatabase == 0) {
        DisableMouseCursorDrawing();
        g_3_stDefaultStringContext.viewport =
            &g_stSecondaryViewBufferBlock;
        g_3_stDefaultStringContext.text =
            g_6_szDefaultTextBufferText;
        RestorePersonnelMenuBackground(&g_stSecondaryViewBufferBlock);
        SetTextContext(&g_3_stDefaultStringContext);
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 1,
            (unsigned char)g_10_nMenuDarkColour, -1);
        SetTextCursor(0x50, 0x12);
        DrawFormattedText("TERRAN CONFEDERATION NAVY");
        SetTextCursor(0x60, 0x1c);
        DrawFormattedText("PERSONNEL DATABASE");
        hoverCommand = 0x26;
        g_nPersonnelMenuHighlight = -1;
        DrawPersonnelMenuChoices(hoverCommand);
        DisableMouseCursorDrawing();
        SetPersonnelMousePosition(200, 0x36);
        RefreshMemoryStatusOverlay();
        g_cCurrentKey = (signed char)0x80;
        while (g_cCurrentKey == (signed char)0x80) {
            PollPersonnelMenuInput();
            if (g_37_stPersonnelMousePointerState.x > 0x28 &&
                g_37_stPersonnelMousePointerState.x < 0xea) {
                if (g_37_stPersonnelMousePointerState.y >= 0x30 &&
                    g_37_stPersonnelMousePointerState.y <= 0x43) {
                    hoverCommand = 0x14;
                    DrawPersonnelMenuChoices(hoverCommand);
                    SetMouseCursorShape(
                        g_18_pEventManager->cursorShape, 1);
                } else if (g_37_stPersonnelMousePointerState.y >= 0x44 &&
                           g_37_stPersonnelMousePointerState.y <= 0x57) {
                    hoverCommand = 0x2e;
                    DrawPersonnelMenuChoices(hoverCommand);
                    SetMouseCursorShape(
                        g_18_pEventManager->cursorShape, 1);
                } else {
                    hoverCommand = -1;
                    DrawPersonnelMenuChoices(hoverCommand);
                    SetMouseCursorShape(
                        g_18_pEventManager->cursorShape, 0);
                }
            } else {
                hoverCommand = -1;
                DrawPersonnelMenuChoices(hoverCommand);
                SetMouseCursorShape(g_18_pEventManager->cursorShape,
                                    0);
            }
            g_bPersonnelMenuDrawing = 0;
            DrawPersonnelMenuBackdrop(g_pPersonnelMenuBackdrop);
            g_bPersonnelMenuDrawing = 1;
            EnableMouseCursorDrawing();
            RefreshMemoryStatusOverlay();
            actionHandledFlag = 0;
            if ((g_cCurrentKey == PERSONNEL_KEY_ENTER ||
                 g_cCurrentKey == PERSONNEL_KEY_SPACE) &&
                hoverCommand != -1) {
                g_cCurrentKey = (signed char)hoverCommand;
            }
            if ((g_cCurrentKey == PERSONNEL_KEY_ENTER ||
                 g_cCurrentKey == PERSONNEL_KEY_SPACE) &&
                hoverCommand == -1) {
                finishedDatabase++;
                actionHandledFlag++;
            }
            switch (toupper((int)g_cCurrentKey)) {
            case PERSONNEL_KEY_T:
                driveToRestore = 99;
                actionHandledFlag = 1;
                DisableMouseCursorDrawing();
                RestorePersonnelMenuBackground(
                    &g_stSecondaryViewBufferBlock);
                RefreshMemoryStatusOverlay();
                InitializeTextContextFromFont(
                    &g_3_stDefaultStringContext, 1,
                    (unsigned char)g_10_nMenuDarkColour, -1);
                choice = SelectLegacyGameSource();
                if (choice == -1) {
                    accepted = 0;
                } else {
                    finishedDatabase = (short)LocateLegacySaveGame(choice);
                    if (finishedDatabase == 0) {
                        ShowNoTransferablePilots();
                        accepted = 0;
                        g_cCurrentKey = 0x14;
                    } else {
                        handle = (short)_open(g_1_szLegacySaveFolder,
                                            0x8000);
                        transferCount = 0;
                        for (saveSlot = 0; saveSlot < 8; saveSlot++) {
                            _read(handle, record, 0x33c);
                            if (record->occupied != 0 &&
                                record->campaign.campaignIndex ==
                                    choice) {
                                pilotRecord = &record->pilots[8];
                                strcpy(
                                    g_3_apszPersonnelImportCaptionsTable[
                                        transferCount],
                                    g_apszShortPilotRankNames[
                                        pilotRecord->rank]);
                                strcat(
                                    g_3_apszPersonnelImportCaptionsTable[
                                        transferCount],
                                    " \"");
                                strcat(
                                    g_3_apszPersonnelImportCaptionsTable[
                                        transferCount],
                                    pilotRecord->callsign);
                                strcat(
                                    g_3_apszPersonnelImportCaptionsTable[
                                        transferCount],
                                    "\" ");
                                strcat(
                                    g_3_apszPersonnelImportCaptionsTable[
                                        transferCount],
                                    pilotRecord->name);
                                itoa(pilotRecord->missions, captionBuffer, 10);
                                strcpy(
                                    g_3_apszPersonnelImportCaptionsTable[
                                        transferCount] + 0x28,
                                    captionBuffer);
                                itoa(pilotRecord->kills, captionBuffer, 10);
                                strcpy(
                                    g_3_apszPersonnelImportCaptionsTable[
                                        transferCount] + 0x2c,
                                    captionBuffer);
                                itoa(saveSlot, captionBuffer, 10);
                                strcpy(
                                    g_3_apszPersonnelImportCaptionsTable[
                                        transferCount] + 0x3c,
                                    captionBuffer);
                                transferCount++;
                            }
                        }
                        _close(handle);
                        if (transferCount == 0) {
                            ShowNoTransferablePilots();
                            accepted = 0;
                            g_cCurrentKey = 0x14;
                            finishedDatabase = 0;
                        } else {
                            selectedPilotIndex = SelectTransferredPilot(
                                transferCount);
                            if (selectedPilotIndex == -1) {
                                finishedDatabase = 0;
                                accepted = 0;
                                g_cCurrentKey = 0x14;
                            } else {
                                PilotRecord *selectedPilot;

                                transferCount = (short)atoi(
                                    g_3_apszPersonnelImportCaptionsTable[
                                        selectedPilotIndex] + 0x3c);
                                handle = (short)_open(
                                    g_1_szLegacySaveFolder, 0x8000);
                                lseek(handle, (long)transferCount * 0x33c, 0);
                                _read(handle, record, 0x33c);
                                selectedPilot = &record->pilots[8];
                                _close(handle);
                                PersonnelDriveHook(driveToRestore);
                                InitializeTextContextFromFont(
                                    &g_3_stDefaultStringContext, 1,
                                    (unsigned char)
                                        g_nMenuSelectedCol,
                                    -1);
                                DisableMouseCursorDrawing();
                                RestorePersonnelMenuBackground(
                                    &g_stSecondaryViewBufferBlock);
                                RefreshMemoryStatusOverlay();
                                strcpy(captionBuffer,
                                       g_apszShortPilotRankNames[
                                           selectedPilot->rank]);
                                strcat(captionBuffer, " ");
                                strcat(captionBuffer, selectedPilot->name);
                                strcat(captionBuffer, "'s first name?");
                                SetTextCursor(0x32, 0x32);
                                DrawFormattedText(captionBuffer);
                                RefreshMemoryStatusOverlay();
                                ReadPersonnelTextField(
                                    0x32, 0x3c, 1, "", firstNameText,
                                    0x0c, 0, 0);
                                strcpy(
                                    g_stPilotNameVariables.firstName,
                                    firstNameText);
                                strcpy(
                                    g_stPilotNameVariables.lastName,
                                    selectedPilot->name);
                                strcpy(
                                    g_stPilotNameVariables.callsign,
                                    selectedPilot->callsign);
                                strcpy(
                                    g_stCurrentPilotProfile.firstName,
                                    firstNameText);
                                strcpy(
                                    g_stCurrentPilotProfile.lastName,
                                    selectedPilot->name);
                                strcpy(
                                    g_stCurrentPilotProfile.callsign,
                                    selectedPilot->callsign);
                                g_stCurrentPilotProfile.rank = 2;
                                g_stCurrentPilotProfile.field_41 =
                                    selectedPilot->missions;
                                g_stCurrentPilotProfile.field_43 =
                                    selectedPilot->kills;
                                LoadStartingCampaignGlobals(0);
                                SaveAndFreeTemporaryCampaignGlobals();
                                OpenDiskDataFile(1);
                                finishedDatabase = 1;
                                accepted = 1;
                            }
                        }
                    }
                }
                PersonnelDriveHook(driveToRestore);
                break;
            case PERSONNEL_KEY_C:
                accepted = 1;
                actionHandledFlag = 1;
                DisableMouseCursorDrawing();
                RestorePersonnelMenuBackground(
                    &g_stSecondaryViewBufferBlock);
                RefreshMemoryStatusOverlay();
                InitializeTextContextFromFont(
                    &g_3_stDefaultStringContext, 1,
                    (unsigned char)g_nMenuSelectedCol, -1);
                inputResult = ReadPersonnelTextField(
                    0x32, 0x1e, 2, "Enter first name: ", firstNameText,
                    0x0b, 0, 0);
                if (inputResult != 0) {
                    strcpy(g_stPilotNameVariables.firstName,
                           firstNameText);
                    strcpy(g_stCurrentPilotProfile.firstName,
                           firstNameText);
                    inputResult = ReadPersonnelTextField(
                        0x32, 0x32, 2, "Enter last name: ", surname,
                        0x0b, 0, 0);
                    if (inputResult != 0) {
                        strcpy(g_stPilotNameVariables.lastName,
                               surname);
                        strcpy(g_stCurrentPilotProfile.lastName,
                               surname);
                        inputResult = ReadPersonnelTextField(
                            0x32, 0x46, 2, "Enter callsign: ", callsign,
                            0x0b, 0, 0);
                        if (inputResult != 0) {
                            strcpy(g_stPilotNameVariables.callsign,
                                   callsign);
                            strcpy(
                                g_stCurrentPilotProfile.callsign,
                                callsign);
                            g_stCurrentPilotProfile.rank = 2;
                            g_stCurrentPilotProfile.field_41 = 0;
                            g_stCurrentPilotProfile.field_43 = 0;
                            LoadStartingCampaignGlobals(0);
                            SaveAndFreeTemporaryCampaignGlobals();
                            OpenDiskDataFile(1);
                        }
                    }
                }
                if (inputResult == 0) {
                    accepted = 0;
                }
                finishedDatabase++;
                g_cCurrentKey = 0x2e;
                break;
            }
            if (actionHandledFlag == 0) {
                g_cCurrentKey = (signed char)0x80;
            }
        }
        g_cCurrentKey = (signed char)0x80;
    }
    FlushInputEvents();
    ReleasePacketSlot(&g_stSceneZoneTable.data);
    ReleasePacketSlot(&g_stSceneTextList.data);
    g_pActiveScenePacket = 0;
    FreePacketAndClear(&g_pPersonnelMenuBackdrop, 0);
    g_3_stDefaultStringContext.viewport =
        &g_101_stScrViewInfo;
    ReleasePacketHandle(record);
    for (saveSlot = 0; saveSlot < 8; saveSlot++) {
        ReleasePacketHandle(
            g_3_apszPersonnelImportCaptionsTable[saveSlot]);
    }
    free_viewport(&g_stSecondaryViewBufferBlock);
    FlushInputEvents();
    return accepted;
}
