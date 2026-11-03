/*
 *  Flight input setup, fatal errors and on-screen messages.
 *
 *  Retail object 0x4379F0-0x43801F (link order; see docs/ORDER.md).
 */
#include "game.h"

char g_szAlreadyNear_0049b334[16] = "Already Near";

char g_szEnemyNear_0049b344[12] = "Enemy Near";

char g_szHazardNear_0049b350[12] = "Hazard Near";

char g_szCalculating_0049b3a0[12] = "CALCULATING";

char g_szRangeKilometresSuffix_0049b3ac[4] = " km";

char *g_pszGameVersion_0049b528 = g_szGameVersion_0049b52c;

char g_szGameVersion_0049b52c[9] = "1.03F-95";

short g_nNavMapCoordinateScaling_0049bb80 = 0;

short g_nNavMapScale_0049bb84 = 1;

short g_bBriefingMapActive_0049bcb0 = 0;

static const char *g_pszFatalErrorMessage_0049ac98 =
    "Sorry, an error has occurred.\n"
    "Please note the following information: %s.\n"
    "Check your configuration.  If this problem persists, please\n"
    "call Origin Systems' service line.  We are sorry for the inconvenience.";

/* Function start: 0x4379F0 */
void InitializeSpaceFlightInput(void)
{
    ClearInputPump();
    InitializeInputDriverHook();
    g_bInputCursorEnabled_005c80e6 = 1;
    g_nInputRepeatDelay_005c80d6 = g_bInputCursorEnabled_005c80e6;
    g_nFlightInputCommand_005d1798 = 0;
    ConfigureInputPump(1, get_player_input);
    SetInputViewport(&g_stViewBuffer_005d2b00);
    g_nUiInputMode_005c8d3c = 0;
}

/* Function start: 0x437A44 */
void ReportFatalErrorCode(const char *errorCode)
{
    FILE *errorFile;

    errorFile = 0;
    errorFile = fopen("err.$$$", "w+");
    LogMemoryStateToFile(errorFile);
    if (errorFile != 0)
        fclose(errorFile);
    sprintf(g_szDefaultTextBuffer_005d2b80,
            g_pszFatalErrorMessage_0049ac98, errorCode);
    FatalErrorAndExit(g_szDefaultTextBuffer_005d2b80);
}

/* Function start: 0x437AB4 */
void exit_squadron(const char *msg, ...)
{
    if (msg != 0)
        ShowNoticeMessageBox(msg);
    _unlink("tape.tmp");
    LogMemoryUsage();
    ShutdownGameWindow();
}

/* Function start: 0x437AEC */
void ShowMemoryStatusDebug(void)
{
    TextContext *previousContext;
    char value[60];
    TextContext savedTextContext;

    previousContext = g_pCurrentTextContext_005c8d1c;
    savedTextContext = g_stDefaultTextContext_005d2d20;
    InitializeTextContextFromFont(
        &g_stDefaultTextContext_005d2d20, 1,
        (unsigned char)g_bPrimaryViewBufferColour_0049cb50,
        g_cSecondaryViewBufferColour_0049cb4c);
    SetTextContext(&g_stDefaultTextContext_005d2d20);
#ifdef SDL_PORT
    DrawFormattedText("%X%YCurrent NMem %d.", 70, 160,
                      GetOriginalFreeMemory());
#else
    /* Called without a prototype: the int return is pushed unextended. */
    DrawFormattedText("%X%YCurrent NMem %d.", 70, 160,
                      ((int (*)(void))GetOriginalFreeMemory)());
#endif
    sprintf(value, "%ld", GetLargestFreeMemoryBlockByType(0));
    DrawFormattedText("%X%YLargest Block FMem %s.", 70, 176, value);
    sprintf(value, "%ld", GetAvailableMainMemory());
    DrawFormattedText("%X%YCurrent FMem %s.", 70, 168, value);
    sprintf(value, "%ld", g_dwInitialFreeMemory_005c8dd0);
    DrawFormattedText("%X%YOriginal FMem %s.", 0, 0, value);
    sprintf(value, "%ld", GetAvailableFarMemory());
    DrawFormattedText("%X%YCurrent EMS %s.", 70, 184, value);
    g_stDefaultTextContext_005d2d20 = savedTextContext;
    g_pCurrentTextContext_005c8d1c = previousContext;
}

/* Function start: 0x437C2E */
void ShowModalMessage(const char *format, ...)
{
    va_list argList;
    char text[52];

    va_start(argList, format);
    vsprintf(text, format, argList);
    va_end(argList);
    if (ShowModalTextPanel(1, text) != 0) {
        AcknowledgeModalMessage();
        ReleaseModalTextPanel();
        return;
    }
    SystemDebugPrintf(text);
    AcknowledgeModalMessage();
}

/* Function start: 0x437C96 */
void ReportOutOfMemoryAndExit(const char *resource)
{
    ShowModalMessage("ERROR: Out of memory for %s", resource);
    FatalErrorAndExit("You do not have enough memory to run Wing Commander.");
}

/* Function start: 0x437CBF */
void InitializeConversationViewport(void)
{
    ClearViewport(&g_stModalSourceViewport_005d2c50, g_cSecondaryViewBufferColour_0049cb4c);
    if (g_stSecondaryViewBuffer_005d2c90.pixels != 0)
        free_viewport(&g_stSecondaryViewBuffer_005d2c90);
    g_stScreenViewport_005d21a0.top = 24;
    g_stScreenViewport_005d21a0.bottom = 151;
    g_stSecondaryViewBuffer_005d2c90.left = 0;
    g_stSecondaryViewBuffer_005d2c90.right = 319;
    g_stSecondaryViewBuffer_005d2c90.top = 0;
    g_stSecondaryViewBuffer_005d2c90.bottom = 127;
    if (AllocateViewport(&g_stSecondaryViewBuffer_005d2c90, (short)g_cSecondaryViewBufferColour_0049cb4c, 0) == 0)
        ReportPacketLoadError(0, 0, 0, 0,
                              g_szAllocateBufferTag_0049ae20);
}

/* Function start: 0x437D68 */
void ResetScreenClipToFullHeight(void)
{
    free_viewport(&g_stSecondaryViewBuffer_005d2c90);
    g_stScreenViewport_005d21a0.top = 0;
    g_stScreenViewport_005d21a0.bottom = 199;
}

/* Function start: 0x437D92 */
void InitializeConversationText(void)
{
    g_stConversationTextViewport_005d2b60 =
        g_stModalSourceViewport_005d2c50;
    g_stConversationTextViewport_005d2b60.top = 152;
    g_stConversationTextContext_005d2d40.viewport =
        &g_stConversationTextViewport_005d2b60;
    g_stConversationTextContext_005d2d40.text =
        g_szDefaultTextBuffer_005d2b80;
    g_stConversationTextContext_005d2d40.alignment = 2;
    InitializeTextContextFromFont(&g_stConversationTextContext_005d2d40,
                                  0,
                                  g_bPrimaryViewBufferColour_0049cb50,
                                  g_cSecondaryViewBufferColour_0049cb4c);
    SetTextContext(&g_stConversationTextContext_005d2d40);
}

/* Function start: 0x437DFA */
void ShowOnScreenMessage(short duration, const char *format, ...)
{
    char text[52];
    short modalShown;
    va_list argList;

    va_start(argList, format);
    vsprintf(text, format, argList);
    va_end(argList);
#ifdef SDL_PORT
    if (duration == 9999)
        SdlSuspendMouseGrab();
#endif
    modalShown = 0;
    if (duration == 9999)
        modalShown = (short)ShowModalTextPanel(1, text);
    if (modalShown == 0) {
        if (duration == 0)
            duration = MeasureMessageWidth(text);
        ClearHudMessageDisplay(1);
        DosStrcpy(g_szOnScreenMessageBuffer_005d1890, text);
        SetHudMessageText(g_szOnScreenMessageBuffer_005d1890,
                          g_abGamePaletteReservedColours_0049cb54[8], duration);
    }
    if (g_bPauseInputActive_0049ac9c != 0) {
        FlushPendingInputPresses();
        FlushInputEvents();
        ClearDebugPauseFlags();
    }
    if (duration == 9999) {
        while (WaitForInputKey() == 0)
            ServiceSoundSystem();
    }
    if (modalShown != 0)
        ReleaseModalTextPanel();
    else if (duration == 9999)
        SetHudMessageText("", g_abGamePaletteReservedColours_0049cb54[8], 2);
#ifdef SDL_PORT
    if (duration == 9999)
        SdlResumeMouseGrab();
#endif
}

/* Function start: 0x437F2F */
void ShowGamePausedBanner(short showBanner)
{
    if (showBanner != 0) {
        ShowOnScreenMessage(9999, "GAME PAUSED");
    } else {
#ifdef SDL_PORT
        SdlSuspendMouseGrab();
#endif
        while (WaitForInputKey() == 0)
            ServiceSoundSystem();
#ifdef SDL_PORT
        SdlResumeMouseGrab();
#endif
    }
}

/* Function start: 0x437F77 */
void ShowVersionBanner(void)
{
    ShowOnScreenMessage(9999, "WING COMMANDER VER. %s",
                        g_pszGameVersion_0049b528);
}

/* Function start: 0x437F9A */
void SetMessageDisplaySpeed(void)
{
    g_cMessageSpeed_0049b778 =
        (unsigned char)(((signed char)g_cMessageSpeed_0049b778 + 1) % 5);
    ShowOnScreenMessage(0, "MESSAGES SPEED IS NOW %d.",
                        (signed char)g_cMessageSpeed_0049b778 + 1);
}

short MinShort(short a, short b);
short MaxShort(short a, short b);

/* Function start: 0x437FD3 */
void ReportFramesSkipped(short adjustment)
{
    g_nFrameSkip_0049d764 = MinShort(
        MaxShort((short)(g_nFrameSkip_0049d764 + adjustment), 1), 5);
    ShowOnScreenMessage(0, "%d FRAMES SKIPPED.",
                        g_nFrameSkip_0049d764 - 1);
}
