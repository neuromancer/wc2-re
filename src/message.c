/*
 *  Flight input setup, fatal errors and on-screen messages.
 *
 *  Retail object 0x4379F0-0x43801F (link order; see docs/ORDER.md).
 */
#include "game.h"

static const char *g_pszFatalErrorMessage_0049ac98 =
    "Sorry, an error has occurred.\n"
    "Please note the following information: %s.\n"
    "Check your configuration.  If this problem persists, please\n"
    "call Origin Systems' service line.  We are sorry for the inconvenience.";
int g_bPauseInputActive_0049ac9c = 0;

/* Function start: 0x4379F0 */
void InitializeSpaceFlightInput(void)
{
    ClearInputPump();
    InitializeInputDriverHook();
    g_bMouseCursorEnabled_005c80e6 = 1;
    g_nEventRepeatDelayState_005c80d6 = g_bMouseCursorEnabled_005c80e6;
    g_nFlightControlCommandValue_005d1798 = 0;
    ConfigureInputPump(1, get_player_input);
    SetInputViewport(&g_stViewScratchInfo_005d2b00);
    g_nInterfaceEventMode_005c8d3c = 0;
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
    sprintf(g_szDefaultTextBufferText_005d2b80,
            g_pszFatalErrorMessage_0049ac98, errorCode);
    FatalErrorAndExit(g_szDefaultTextBufferText_005d2b80);
}

/* Function start: 0x437AB4 */
void exit_squadron(const char *msg, ...)
{
    if (msg != 0)
        ShowNoticeMessageBox(msg);
    _unlink(g_szCannedSceneTapeFile_00490208);
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
    savedTextContext = g_stDefaultStringContext_005d2d20;
    InitializeTextContextFromFont(
        &g_stDefaultStringContext_005d2d20, 1,
        (unsigned char)g_bPrimaryViewBufferColour_0049cb50,
        g_cSecondaryViewBufferColour_0049cb4c);
    SetTextContext(&g_stDefaultStringContext_005d2d20);
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
    sprintf(value, "%ld", g_dwInitialSpareMemory_005c8dd0);
    DrawFormattedText("%X%YOriginal FMem %s.", 0, 0, value);
    sprintf(value, "%ld", GetAvailableFarMemory());
    DrawFormattedText("%X%YCurrent EMS %s.", 70, 184, value);
    g_stDefaultStringContext_005d2d20 = savedTextContext;
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
    FatalErrorAndExit(g_szNotEnoughMemoryMessage_0049cbb8);
}

/* Function start: 0x437CBF */
void InitializeConversationViewport(void)
{
    ClearViewport(&g_stModalSourceWindow_005d2c50, g_cSecondaryViewBufferColour_0049cb4c);
    if (g_stSecondaryViewBufferBlock_005d2c90.pixels != 0)
        free_viewport(&g_stSecondaryViewBufferBlock_005d2c90);
    g_stScrViewInfo_005d21a0.top = 24;
    g_stScrViewInfo_005d21a0.bottom = 151;
    g_stSecondaryViewBufferBlock_005d2c90.left = 0;
    g_stSecondaryViewBufferBlock_005d2c90.right = 319;
    g_stSecondaryViewBufferBlock_005d2c90.top = 0;
    g_stSecondaryViewBufferBlock_005d2c90.bottom = 127;
    if (AllocateViewport(&g_stSecondaryViewBufferBlock_005d2c90, (short)g_cSecondaryViewBufferColour_0049cb4c, 0) == 0)
        ReportPacketLoadError(0, 0, 0, 0,
                              "AB");
}

/* Function start: 0x437D68 */
void ResetScreenClipToFullHeight(void)
{
    free_viewport(&g_stSecondaryViewBufferBlock_005d2c90);
    g_stScrViewInfo_005d21a0.top = 0;
    g_stScrViewInfo_005d21a0.bottom = 199;
}

/* Function start: 0x437D92 */
void InitializeConversationText(void)
{
    g_stConversationTextViewport_005d2b60 =
        g_stModalSourceWindow_005d2c50;
    g_stConversationTextViewport_005d2b60.top = 152;
    g_stConversationTextCtx_005d2d40.viewport =
        &g_stConversationTextViewport_005d2b60;
    g_stConversationTextCtx_005d2d40.text =
        g_szDefaultTextBufferText_005d2b80;
    g_stConversationTextCtx_005d2d40.alignment = 2;
    InitializeTextContextFromFont(&g_stConversationTextCtx_005d2d40,
                                  0,
                                  g_bPrimaryViewBufferColour_0049cb50,
                                  g_cSecondaryViewBufferColour_0049cb4c);
    SetTextContext(&g_stConversationTextCtx_005d2d40);
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
                          g_cRedColour_0049cb5c, duration);
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
        SetHudMessageText("", g_cRedColour_0049cb5c, 2);
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

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
Viewport g_stConversationTextViewport_005d2b60;
char g_szDefaultTextBufferText_005d2b80[0xc8];
unsigned char *g_pCockpitIndicatorShape_005d2c48;
unsigned char *g_pConstellationSprite_005d2c4c;
Viewport g_stModalSourceWindow_005d2c50;
unsigned char *g_pPilotHandAnimationShapePtr_005d2c64;
Viewport g_stPilotHandViewportRecord_005d2c70;
Viewport g_stSecondaryViewBufferBlock_005d2c90;
short g_aasCockpitHitPaletteFades_005d2cb0[6][3];
unsigned char *g_pCockpitPanelShape_005d2cd4;
TextContext g_stRightVduTextContextBlock_005d2ce0;
TextContext g_stCockpitTextSettingsRecord_005d2d00;
TextContext g_stDefaultStringContext_005d2d20;
TextContext g_stConversationTextCtx_005d2d40;
short g_asSpacePaletteFade_005d2d60[3];
