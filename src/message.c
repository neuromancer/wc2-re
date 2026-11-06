/*
 *  Flight input setup, fatal errors and on-screen messages.
 *
 *  Retail object 0x4379F0-0x43801F (link order; see docs/ORDER.md).
 */
#include "game.h"

static const char *g_pszFatalErrorMessage =
    "Sorry, an error has occurred.\n"
    "Please note the following information: %s.\n"
    "Check your configuration.  If this problem persists, please\n"
    "call Origin Systems' service line.  We are sorry for the inconvenience."; /* 0x0049ac98 */
int g_bPauseInputActive = 0; /* 0x0049ac9c */

/* Function start: 0x4379F0 */
void InitializeSpaceFlightInput(void)
{
    ClearInputPump();
    InitializeInputDriverHook();
    g_26_bMouseCursorEnabled = 1;
    g_88_nEventRepeatDelayState = g_26_bMouseCursorEnabled;
    g_3_nFlightControlCommandValue = 0;
    ConfigureInputPump(1, get_player_input);
    SetInputViewport(&g_27_stViewScratchInfo);
    g_18_nInterfaceEventMode = 0;
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
    sprintf(g_6_szDefaultTextBufferText,
            g_pszFatalErrorMessage, errorCode);
    FatalErrorAndExit(g_6_szDefaultTextBufferText);
}

/* Function start: 0x437AB4 */
void exit_squadron(const char *msg, ...)
{
    if (msg != 0)
        ShowNoticeMessageBox(msg);
    _unlink(g_szCannedSceneTapeFile);
    LogMemoryUsage();
    ShutdownGameWindow();
}

/* Function start: 0x437AEC */
void ShowMemoryStatusDebug(void)
{
    TextContext *previousContext;
    char value[60];
    TextContext savedTextContext;

    previousContext = g_29_pCurrentTextContext;
    savedTextContext = g_3_stDefaultStringContext;
    InitializeTextContextFromFont(
        &g_3_stDefaultStringContext, 1,
        (unsigned char)g_bPrimaryViewBufferColour,
        g_cSecondaryViewBufferColour);
    SetTextContext(&g_3_stDefaultStringContext);
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
    sprintf(value, "%ld", g_dwInitialSpareMemory);
    DrawFormattedText("%X%YOriginal FMem %s.", 0, 0, value);
    sprintf(value, "%ld", GetAvailableFarMemory());
    DrawFormattedText("%X%YCurrent EMS %s.", 70, 184, value);
    g_3_stDefaultStringContext = savedTextContext;
    g_29_pCurrentTextContext = previousContext;
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
    printf(text);
    AcknowledgeModalMessage();
}

/* Function start: 0x437C96 */
void ReportOutOfMemoryAndExit(const char *resource)
{
    ShowModalMessage("ERROR: Out of memory for %s", resource);
    FatalErrorAndExit(g_szNotEnoughMemoryMessage);
}

/* Function start: 0x437CBF */
void InitializeConversationViewport(void)
{
    ClearViewport(&g_2_stModalSourceWindow, g_cSecondaryViewBufferColour);
    if (g_stSecondaryViewBufferBlock.pixels != 0)
        free_viewport(&g_stSecondaryViewBufferBlock);
    g_101_stScrViewInfo.top = 24;
    g_101_stScrViewInfo.bottom = 151;
    g_stSecondaryViewBufferBlock.left = 0;
    g_stSecondaryViewBufferBlock.right = 319;
    g_stSecondaryViewBufferBlock.top = 0;
    g_stSecondaryViewBufferBlock.bottom = 127;
    if (AllocateViewport(&g_stSecondaryViewBufferBlock, (short)g_cSecondaryViewBufferColour, 0) == 0)
        ReportPacketLoadError(0, 0, 0, 0,
                              "AB");
}

/* Function start: 0x437D68 */
void ResetScreenClipToFullHeight(void)
{
    free_viewport(&g_stSecondaryViewBufferBlock);
    g_101_stScrViewInfo.top = 0;
    g_101_stScrViewInfo.bottom = 199;
}

/* Function start: 0x437D92 */
void InitializeConversationText(void)
{
    g_6_stConversationTextViewport =
        g_2_stModalSourceWindow;
    g_6_stConversationTextViewport.top = 152;
    g_2_stConversationTextCtx.viewport =
        &g_6_stConversationTextViewport;
    g_2_stConversationTextCtx.text =
        g_6_szDefaultTextBufferText;
    g_2_stConversationTextCtx.alignment = 2;
    InitializeTextContextFromFont(&g_2_stConversationTextCtx,
                                  0,
                                  g_bPrimaryViewBufferColour,
                                  g_cSecondaryViewBufferColour);
    SetTextContext(&g_2_stConversationTextCtx);
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
        DosStrcpy(g_szOnScreenMessageBuffer, text);
        SetHudMessageText(g_szOnScreenMessageBuffer,
                          g_cRedColour, duration);
    }
    if (g_bPauseInputActive != 0) {
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
        SetHudMessageText("", g_cRedColour, 2);
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
                        g_pszGameVersion);
}

/* Function start: 0x437F9A */
void SetMessageDisplaySpeed(void)
{
    g_cMessageSpeed =
        (unsigned char)(((signed char)g_cMessageSpeed + 1) % 5);
    ShowOnScreenMessage(0, "MESSAGES SPEED IS NOW %d.",
                        (signed char)g_cMessageSpeed + 1);
}

short MinShort(short a, short b);
short MaxShort(short a, short b);

/* Function start: 0x437FD3 */
void ReportFramesSkipped(short adjustment)
{
    g_nFrameSkip = MinShort(
        MaxShort((short)(g_nFrameSkip + adjustment), 1), 5);
    ShowOnScreenMessage(0, "%d FRAMES SKIPPED.",
                        g_nFrameSkip - 1);
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
Viewport g_6_stConversationTextViewport; /* 0x005d2b60 */
char g_6_szDefaultTextBufferText[0xc8]; /* 0x005d2b80 */
unsigned char *g_18_pCockpitIndicatorShape; /* 0x005d2c48 */
unsigned char *g_pConstellationSprite; /* 0x005d2c4c */
Viewport g_2_stModalSourceWindow; /* 0x005d2c50 */
unsigned char *g_pPilotHandAnimationShapePtr; /* 0x005d2c64 */
Viewport g_stPilotHandViewportRecord; /* 0x005d2c70 */
Viewport g_stSecondaryViewBufferBlock; /* 0x005d2c90 */
short g_9_aasCockpitHitPaletteFades[6][3]; /* 0x005d2cb0 */
unsigned char *g_4_pCockpitPanelShape; /* 0x005d2cd4 */
TextContext g_1_stRightVduTextContextBlock; /* 0x005d2ce0 */
TextContext g_16_stCockpitTextSettingsRecord; /* 0x005d2d00 */
TextContext g_3_stDefaultStringContext; /* 0x005d2d20 */
TextContext g_2_stConversationTextCtx; /* 0x005d2d40 */
short g_1_asSpacePaletteFade[3]; /* 0x005d2d60 */
