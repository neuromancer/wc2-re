/*
 *  Pilot name entry, high scores, inter-scene transitions and ownership of
 *  the Win32 developer overlay console.
 *
 *  Address range 0x425000-0x426fff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: EnterPilotNameAndCallsign/ShowTrainSimHighScores; string band 0x469D74-0x469F98.
 */
#include "wc1.h"

/* Function start: 0x4333F8 */
int MapCutsceneSoundEffectNumber(int soundIndex)
{
    return soundIndex + 1;
}

/* Function start: 0x40FCD0 */
void CreateDebugOverlayConsole(HINSTANCE module, HWND window,
                               short columns, short rows)
{
    g_pDebugOverlay_004a2548 =
        new DebugOverlayConsole(module, window, columns, rows, 0);
}

/* Function start: 0x40FD73 */
void DestroyGlobalDebugOverlayConsole(void)
{
    delete g_pDebugOverlay_004a2548;
#ifdef WC1_SDL
    g_pDebugOverlay_004a2548 = 0;
#endif
}

#pragma function(strcpy)
/* Function start: 0x40FDAD */
void SystemDebugPrintf(const char *format, ...)
{
    va_list arguments;

    va_start(arguments, format);
    if (format != 0)
        vsprintf(g_szSystemDebugBuffer_004a2550, format, arguments);
    else
        strcpy(g_szSystemDebugBuffer_004a2550, "\n");
    va_end(arguments);
    DebugOverlayPrintf(g_pDebugOverlay_004a2548,
                       g_szSystemDebugBuffer_004a2550);
    WriteDebugString(g_szSystemDebugBuffer_004a2550);
}

/* Function start: 0x40FE23 */
char PumpMessagesDuringWait(void)
{
    return g_pDebugOverlay_004a2548->WaitForKey();
}

/* Function start: 0x40FE3E */
unsigned char TakeDebugStepFlag(void)
{
    return g_pDebugOverlay_004a2548->TakeStepFlag();
}

/* Function start: 0x40FE59 */
void SetDebugTextPosition(short row, short column)
{
    g_pDebugOverlay_004a2548->SetTextPosition(row, column);
}

/* Function start: 0x40FE7E */
void SetDebugCursorPosition(short column, short row)
{
    g_pDebugOverlay_004a2548->SetCursorPosition(column, row);
}

/* Function start: 0x40FEA3 */
void ResetDebugOverlay(void)
{
    g_pDebugOverlay_004a2548->Reset();
}

/* Function start: 0x40FEBE */
void ClearDebugOverlay(void)
{
    g_pDebugOverlay_004a2548->Clear();
}

/* Function start: 0x40FED9 */
void ClearDebugPauseFlags(void)
{
    g_pDebugOverlay_004a2548->ClearPauseFlags();
}

/* Function start: 0x40FF40 */
void DebugOverlayConsole::SetTextPosition(int row, int column)
{
    cursorColumn = column;
    cursorRow = row;
}

/* Function start: 0x40FF70 */
void DebugOverlayConsole::SetCursorPosition(int column, int row)
{
    SetTextPosition(row, column);
}

/* Function start: 0x40FFA0 */
void DebugOverlayConsole::Reset(void)
{
    Clear();
}

/* Function start: 0x40FFC0 */
unsigned char DebugOverlayConsole::TakeStepFlag(void)
{
    DWORD value;

    value = g_dwDebugOverlayKeyLatch_0049cb2c;
    g_dwDebugOverlayKeyLatch_0049cb2c = 0;
    return (unsigned char)value;
}

/* Function start: 0x40FFF0 */
void DebugOverlayConsole::ClearPauseFlags(void)
{
    g_dwDebugOverlayKeyLatch_0049cb2c = 0;
    g_dwDebugOverlayKey_0049cb28 = 0;
}

/* Function start: WC2_UNMAPPED */
int DisplayTrainSimHighScoreTable(short phase)
{
    short completed;

    (void)phase;
    completed = 1;
    DrawFilledViewportRect(
        &g_stModalSourceViewport_005d2c50,
        g_stTrainSimPanelBounds_00469dc0_WC1_UNMAPPED.left,
        g_stTrainSimPanelBounds_00469dc0_WC1_UNMAPPED.top,
        g_stTrainSimPanelBounds_00469dc0_WC1_UNMAPPED.right,
        g_stTrainSimPanelBounds_00469dc0_WC1_UNMAPPED.bottom,
        g_cSecondaryViewBufferColour_0049cb4c);
    *(ShortRect *)&g_stTrainSimHighScoreDisplayViewport_005a7bf0.left =
        g_stTrainSimPanelBounds_00469dc0_WC1_UNMAPPED;
    CopyViewportContents(
        &g_stTrainSimHighScoreBufferViewport_005a7bb0,
        &g_stTrainSimHighScoreDisplayViewport_005a7bf0);
    SetFrameTimerPeriodDirect(0x2d0);
    do {
        if ((short)IsFrameTickElapsed() != 0)
            break;
        MarkDibDirty();
        DIBslamReal();
        if (CheckEscaped() != 0) {
            completed = 0;
            break;
        }
    } while (1);
    return completed;
}

/* Function start: WC2_UNMAPPED */
int AnimateTrainSimTitle(void)
{
    short completed;
    short y;

    completed = 0;
    y = 0x6b;
    DrawFilledViewportRect(
        &g_stModalSourceViewport_005d2c50,
        g_stTrainSimPanelBounds_00469dc0_WC1_UNMAPPED.left,
        g_stTrainSimPanelBounds_00469dc0_WC1_UNMAPPED.top,
        g_stTrainSimPanelBounds_00469dc0_WC1_UNMAPPED.right,
        g_stTrainSimPanelBounds_00469dc0_WC1_UNMAPPED.bottom,
        g_cSecondaryViewBufferColour_0049cb4c);
    MarkDibDirty();
    DIBslamReal();
    y = (short)(y - ReadWord(
        (unsigned short *)g_stTrainSimTextContext_005a7bd0.font));
    g_stTrainSimTitleDisplayViewport_005a7b90.top = y;
    g_stTrainSimTitleDisplayViewport_005a7b90.bottom =
        (short)(y + ReadWord(
            (unsigned short *)g_stTrainSimTextContext_005a7bd0.font) + 2);
    if (CheckEscaped() == 0) {
        for (;;) {
            SetFrameTimerPeriodDirect(3);
            if (g_stTrainSimTitleDisplayViewport_005a7b90.top <=
                g_stTrainSimPanelBounds_00469dc0_WC1_UNMAPPED.top)
                break;
            g_stTrainSimTitleDisplayViewport_005a7b90.top--;
            g_stTrainSimTitleDisplayViewport_005a7b90.bottom--;
            CopyViewportContents(
                &g_stTrainSimPanelViewport_00469da8_WC1_UNMAPPED,
                &g_stTrainSimTitleDisplayViewport_005a7b90);
            MarkDibDirty();
            DIBslamReal();
            WaitForFrameTick();
            if (CheckEscaped() != 0)
                return completed;
        }
        completed = 1;
    }
    return completed;
}

/* Function start: WC2_UNMAPPED */
unsigned char *GetHighScoreEntry(short i)
{
    short k = (short)g_aHighScoreEntries_005a7c30[i].pilotIndex;

    if (8 < k)
        return g_apszBuiltInHighScoreNames_00469de0_WC1_UNMAPPED[k - 9];
    return (unsigned char *)g_aPilotRecords_005988d0[k].callsign;
}

/* Function start: WC2_UNMAPPED */
unsigned int GetHighScoreValue(short i)
{
    return g_aHighScoreEntries_005a7c30[i].score;
}

/* Function start: WC2_UNMAPPED */
short IsHighScoreSlotUsed(short i)
{
    return g_aHighScoreEntries_005a7c30[i].pilotIndex != -1;
}

/* Function start: WC2_UNMAPPED */
void PromptForWc1PilotField(short x, short y, const char *label,
                            char *destination, short maximumLength,
                            const char *defaultText)
{
    g_stTrainSimTextContext_005a7bd0.alignment = 0;
    SetTextCursor((unsigned short)(g_stTrainSimPanelViewport_00469da8_WC1_UNMAPPED.left + x),
                  (unsigned short)y);
    DrawFormattedText(label);
    do {
        DosStrcpy(destination, defaultText);
    } while (ReadTextInput(destination, maximumLength, 0, 0) == 0);
}

/* Function start: WC2_UNMAPPED */
void InitializeWc1TrainSimTextPanel(void)
{
    g_stTrainSimPanelViewport_00469da8_WC1_UNMAPPED = g_stScreenViewport_005d21a0;
    *(ShortRect *)&g_stTrainSimPanelViewport_00469da8_WC1_UNMAPPED.left =
        g_stTrainSimPanelBounds_00469dc0_WC1_UNMAPPED;
    ClearViewport(&g_stTrainSimPanelViewport_00469da8_WC1_UNMAPPED, g_cSecondaryViewBufferColour_0049cb4c);
    g_stTrainSimTextContext_005a7bd0.text = g_szDefaultTextBuffer_005d2b80;
    ResetStringBuilder(&g_stTrainSimTextContext_005a7bd0);
    SetTextContext(&g_stTrainSimTextContext_005a7bd0);
    InitializeTextContextFromFont(&g_stTrainSimTextContext_005a7bd0, 1,
                                  g_ucDefaultTextColour_0049cb7c,
                                  (signed char)g_cSecondaryViewBufferColour_0049cb4c);
    g_stTrainSimTextContext_005a7bd0.viewport =
        &g_stTrainSimPanelViewport_00469da8_WC1_UNMAPPED;
    EraseTextContextBackground(&g_stTrainSimTextContext_005a7bd0);
}

/* Function start: WC2_UNMAPPED */
void ShowTrainSimTextMessage(const char *message)
{
    SetTextCursor(
        (unsigned short)g_stTrainSimPanelViewport_00469da8_WC1_UNMAPPED.left,
        (unsigned short)(g_stTrainSimPanelViewport_00469da8_WC1_UNMAPPED.top + 2));
    g_stTrainSimTextContext_005a7bd0.alignment = 2;
    FormatTextBufferFromStart(message);
    FormatTextBufferFromStart(g_szTextFlushToken_00469e6c_WC1_UNMAPPED);
    MarkDibDirty();
    DIBslamReal();
}

/* Function start: WC2_UNMAPPED */
void EnterPilotNameAndCallsign(void)
{
    unsigned char *backdrop;

    ClearViewport(&g_stScreenViewport_005d21a0, 0);
    backdrop = (unsigned char *)FetchDiskPacketRetrying(
        reinterpret_cast<char *>(static_cast<unsigned int>(
            (unsigned short)g_cCockpitLogicalFile_005a7c74)), 0, 0);
    DrawSpriteDefault(&g_stScreenViewport_005d21a0, 0, 0, backdrop, 0);
    InitializeWc1TrainSimTextPanel();
    ShowTrainSimTextMessage(g_szNewPilotPrompt_00469e70_WC1_UNMAPPED);
    PromptForWc1PilotField(
        10,
        (short)(g_stTrainSimTextContext_005a7bd0.cursorY + 2),
        g_szPilotNameLabel_00469ec8_WC1_UNMAPPED,
        g_stCampaignState_0059ca50.currentPilot->name,
        13, g_szDefaultPilotName_0049ab4c);
    PromptForWc1PilotField(
        10,
        (short)(g_stTrainSimTextContext_005a7bd0.cursorY + 10),
        g_szCallsignLabel_00469ee0_WC1_UNMAPPED,
        g_stCampaignState_0059ca50.currentPilot->callsign,
        13, g_szDefaultCallsign_0049ab54);
    if (g_nOriginDevUnlock_0049d774 != 0) {
        memcpy(g_stCampaignState_0059ca50.currentPilot->callsign,
               g_szCheaterCallsign_00469eec_WC1_UNMAPPED, 8);
    }
    ReleasePacketHandle(backdrop);
}

/* Function start: WC2_UNMAPPED */
void ShowTrainSimHighScores(void)
{
    unsigned char *backdrop;
    char score[20];
    short titleWidth;
    short lineHeight;
    short titleLeft;
    short row;

    SetEventManagerPump(PollJoystickButtonEvents);
    ClearViewport(&g_stScreenViewport_005d21a0, 0);
    backdrop = (unsigned char *)FetchDiskPacketRetrying(
        reinterpret_cast<char *>(static_cast<unsigned int>(
            (unsigned short)g_cCockpitLogicalFile_005a7c74)), 0, 0);
    DrawSpriteDefault(&g_stScreenViewport_005d21a0, 0, 0, backdrop, 0);
    InitializeTextContextFromFont(&g_stTrainSimTextContext_005a7bd0, 1,
                                  g_ucDefaultTextColour_0049cb7c,
                                  (signed char)g_cSecondaryViewBufferColour_0049cb4c);
    SetTextContext(&g_stTrainSimTextContext_005a7bd0);
    SetViewportRect(&g_stTrainSimPanelViewport_00469da8_WC1_UNMAPPED,
                    0, 0, 319, 199);
    g_stTrainSimTextContext_005a7bd0.text =
        g_szDefaultTextBuffer_005d2b80;
    ResetStringBuilder(&g_stTrainSimTextContext_005a7bd0);
    titleWidth = (short)((MeasureTextPixelWidthClamped(
        g_szTrainSimTitle_00469dc8_WC1_UNMAPPED) & 0xfff8) + 8);
    g_stTrainSimPanelViewport_00469da8_WC1_UNMAPPED.right = titleWidth;
    g_stTrainSimPanelViewport_00469da8_WC1_UNMAPPED.bottom =
        (short)(ReadWord((unsigned short *)
            g_stTrainSimTextContext_005a7bd0.font) + 2);
    AllocateViewport(&g_stTrainSimPanelViewport_00469da8_WC1_UNMAPPED,
                     g_cSecondaryViewBufferColour_0049cb4c, 0);
    g_stTrainSimTextContext_005a7bd0.viewport =
        &g_stTrainSimPanelViewport_00469da8_WC1_UNMAPPED;
    EraseTextContextBackground(&g_stTrainSimTextContext_005a7bd0);
    SetTextCursor(0, 1);
    DrawFormattedText(g_szTrainSimTitle_00469dc8_WC1_UNMAPPED);
    lineHeight = (short)ReadWord((unsigned short *)
        g_stTrainSimTextContext_005a7bd0.font);

    g_stTrainSimTitleDisplayViewport_005a7b90 = g_stScreenViewport_005d21a0;
    *(ShortRect *)&g_stTrainSimTitleDisplayViewport_005a7b90.left =
        g_stTrainSimPanelBounds_00469dc0_WC1_UNMAPPED;
    ClearViewport(&g_stTrainSimTitleDisplayViewport_005a7b90,
                  g_cSecondaryViewBufferColour_0049cb4c);
    titleLeft = (short)((160 - titleWidth / 2) & 0xfffe);
    g_stTrainSimTitleDisplayViewport_005a7b90.left = titleLeft;
    g_stTrainSimTitleDisplayViewport_005a7b90.right =
        (short)(titleLeft + titleWidth);
    g_stTrainSimTitleDisplayViewport_005a7b90.top =
        (short)(RandomInRange(0, 0x4e) + 0x1d);
    g_stTrainSimTitleDisplayViewport_005a7b90.bottom =
        (short)(g_stTrainSimTitleDisplayViewport_005a7b90.top +
                lineHeight + 2);
    if (g_stTrainSimTitleDisplayViewport_005a7b90.bottom > 109) {
        g_stTrainSimTitleDisplayViewport_005a7b90.bottom = 109;
    }

    row = 0;
    InitializeTextContextFromFont(
        &g_stTrainSimHighScoreTextContext_005a7c10, 1,
        g_ucDefaultTextColour_0049cb7c,
        (signed char)g_cSecondaryViewBufferColour_0049cb4c);
    *(ShortRect *)&g_stTrainSimHighScoreBufferViewport_005a7bb0.left =
        g_stTrainSimPanelBounds_00469dc0_WC1_UNMAPPED;
    AllocateViewport(&g_stTrainSimHighScoreBufferViewport_005a7bb0,
                     g_cSecondaryViewBufferColour_0049cb4c, 0);
    g_stTrainSimHighScoreDisplayViewport_005a7bf0 = g_stScreenViewport_005d21a0;
    lineHeight = (short)(lineHeight + 3);
    g_stTrainSimHighScoreTextContext_005a7c10.viewport =
        &g_stTrainSimHighScoreBufferViewport_005a7bb0;
    g_stTrainSimHighScoreTextContext_005a7c10.text =
        g_szDefaultTextBuffer_005d2b80;
    *(ShortRect *)&g_stTrainSimHighScoreDisplayViewport_005a7bf0.left =
        g_stTrainSimPanelBounds_00469dc0_WC1_UNMAPPED;
    SetTextContext(&g_stTrainSimHighScoreTextContext_005a7c10);
    ResetStringBuilder(&g_stTrainSimHighScoreTextContext_005a7c10);
    EraseTextContextBackground(&g_stTrainSimHighScoreTextContext_005a7c10);
    SetTextCursor(
        (unsigned short)g_stTrainSimHighScoreBufferViewport_005a7bb0.left,
        (unsigned short)(
            g_stTrainSimHighScoreBufferViewport_005a7bb0.top + 1));
    FormatTextBufferFromStart(g_szHighScoresHeading_00469f70_WC1_UNMAPPED, 2);
    g_stTrainSimHighScoreTextContext_005a7c10.alignment = 0;
    do {
        if (IsHighScoreSlotUsed(row)) {
            ResetStringBuilder(&g_stTrainSimHighScoreTextContext_005a7c10);
            sprintf(score, g_szHighScoreNumberFormat_00469f80_WC1_UNMAPPED,
                    GetHighScoreValue(row));
            DrawFormattedText(g_szHighScoreRowFormat_00469f88_WC1_UNMAPPED,
                g_stTrainSimHighScoreBufferViewport_005a7bb0.left + 10,
                lineHeight * (row + 1) +
                    g_stTrainSimHighScoreBufferViewport_005a7bb0.top + 1,
                row + 1, GetHighScoreEntry(row),
                g_stTrainSimHighScoreBufferViewport_005a7bb0.left + 150,
                score);
        }
        row++;
    } while (row < 6);
    FlushInputEvents();
    MarkDibDirty();
    DIBslamReal();
    do {
        if (DisplayTrainSimHighScoreTable(1) == 0)
            break;
    } while (AnimateTrainSimTitle() != 0);
    ReleaseTextFont(1);
    free_viewport(&g_stTrainSimPanelViewport_00469da8_WC1_UNMAPPED);
    free_viewport(&g_stTrainSimHighScoreBufferViewport_005a7bb0);
    ReleasePacketHandle(backdrop);
}
