/*
 *  Process-level services: memory reporting and exit.
 *
 *  Address range 0x427000-0x4274df (provisional -- see docs/ORDER.md).
 *  Boundary evidence: exit_squadron/ShowMemoryStatusDebug; string band 0x46A064-0x46A10C.
 */
#include "game.h"

/* Function start: 0x465644 */
void SplitGameClockTicksForNav(unsigned char *parts)
{
    int ticks;

    ticks = (int)GetGameClockTicks();
    parts[0] = (unsigned char)(ticks % 60);
    ticks = ticks / 60;
    parts[1] = (unsigned char)(ticks % 60);
    ticks = ticks / 60;
    parts[2] = (unsigned char)(ticks % 60);
    ticks = ticks / 60;
    parts[3] = (unsigned char)(ticks % 24);
    return;
}

/* Function start: 0x4656CC */
void AllocateApplicationScratchBuffer(void)
{
    g_pApplicationScratchBuffer_005c8483 =
        AllocateTaggedMemory(0x1000, 0);
}

/* Function start: 0x4656EB */
void ReleaseApplicationScratchBuffer(void)
{
    ReleasePacketHandle(g_pApplicationScratchBuffer_005c8483);
    g_pApplicationScratchBuffer_005c8483 = 0;
}

/* Function start: 0x46570E */
unsigned char *GetInputCursorShape(void)
{
    return g_pInputCursorShape_005c83f9;
}

/* Function start: 0x465730 */
short WaitForQueuedInputPress(void)
{
    int savedWaitState;
    short key;

    savedWaitState = g_bInputEventQueueEnabled_0049c248;
    g_bInputEventQueueEnabled_0049c248 = 1;
    do {
        key = (short)WaitForAnyInputPress();
    } while (key == 0);
    g_bInputEventQueueEnabled_0049c248 = savedWaitState;
    return key;
}

/* Function start: 0x46579D */
short LogMemoryUsage(void)
{
    /* WC2's system.c declares the canned-scene tape file as a pointer where
     * it is really the string itself, so the unlink is handed the first four
     * bytes of "tape.tmp" and always fails.  The port cannot dereference a
     * 32-bit value as a pointer, so it removes the file as intended. */
#ifdef SDL_PORT
    _unlink(g_szCannedSceneTapeFile_00490208);
#else
    _unlink(*(char *const *)g_szCannedSceneTapeFile_00490208);
#endif
    StopMusicIfDriverActive();
    EMShutDown();
    if (g_bSpeechCacheEnabled_005c8de8 != 0)
        ShutdownSpeechCache();
    if (g_bParentDirectorySelected_0049d78c == 0) {
        _chdir("..");
        g_bParentDirectorySelected_0049d78c++;
    }
    ConfigureDefaultSpacePalette(g_nDetectedGraphicsMode_005c80d2);
    if (g_nOriginDevUnlock_0049d774 != 0) {
        printf("Series %d, Mission %d (%c)\n",
               (int)g_nCurrentSeries_005c5870,
               (int)g_nCurrentMission_005c5878,
               (int)g_nCurrentMission_005c5878 + 'A');
    }
    if (g_nShowMemoryStatus_0049d784 != 0) {
        printf(
            "initial far: %lu.  current far: %lu.  current near: %u.\n",
            g_dwInitialFreeMemory_005c8dd0,
            GetLargestFreeMemoryBlockByType(0), 0);
    }
    FinalizeInputManagerHook();
    MessagePumpHook(8);
    if (g_pMemoryAdjustment_0049d788 != 0)
        free(g_pMemoryAdjustment_0049d788);
    return 0;
}

/* Function start: 0x46589D */
void AdjustSpaceFramePeriod(short adjustment)
{
    g_nSpaceFramePeriod_0049d768 = MinShort(
        MaxShort((short)(g_nSpaceFramePeriod_0049d768 + adjustment), 1),
        0x13);
    ShowOnScreenMessage(0, "%d/60th SEC FRAME DURATION.",
                        g_nSpaceFramePeriod_0049d768 + 1);
}

static const char *g_pszFatalErrorMessage_0049ac98 =
    "Sorry, an error has occurred.\n"
    "Please note the following information: %s.\n"
    "Check your configuration.  If this problem persists, please\n"
    "call Origin Systems' service line.  We are sorry for the inconvenience.";

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
    TextContext savedContext;
    TextContext *previousContext;
    char value[60];

    previousContext = g_pCurrentTextContext_005c8d1c;
    savedContext = g_stDefaultTextContext_005d2d20;
    InitializeTextContextFromFont(
        &g_stDefaultTextContext_005d2d20, 1,
        (unsigned char)g_bPrimaryViewBufferColour_0049cb50,
        g_cSecondaryViewBufferColour_0049cb4c);
    SetTextContext(&g_stDefaultTextContext_005d2d20);
    DrawFormattedText("%X%YCurrent NMem %d.", 70, 160,
                      GetOriginalFreeMemory());
    sprintf(value, "%ld", GetLargestFreeMemoryBlockByType(0));
    DrawFormattedText("%X%YLargest Block FMem %s.", 70, 176, value);
    sprintf(value, "%ld", GetAvailableMainMemory());
    DrawFormattedText("%X%YCurrent FMem %s.", 70, 168, value);
    sprintf(value, "%ld", g_dwInitialFreeMemory_005c8dd0);
    DrawFormattedText("%X%YOriginal FMem %s.", 0, 0, value);
    sprintf(value, "%ld", GetAvailableFarMemory());
    DrawFormattedText("%X%YCurrent EMS %s.", 70, 184, value);
    g_stDefaultTextContext_005d2d20 = savedContext;
    g_pCurrentTextContext_005c8d1c = previousContext;
}
