/*
 *  Packet loading, text contexts and input waits.
 *
 *  Retail object 0x40F020-0x40FB7F (link order; see docs/ORDER.md).
 */
#include "game.h"

const char *g_pszPacketReadOperation = "reading from disk"; /* 0x00492b0c */
const char *g_pszPacketAllocateOperation = "allocating memory"; /* 0x00492b10 */
const char *g_pszPacketLoadErrorFormat =
    "Sorry, an error has occurred while %s.\n"
    "Please note the following information:\n"
    "%Fs #%d (ERR %d  PS%ld  LB%ld  FL%d) at %s\n"
    "Check your configuration.  If this problem persists, please\n"
    "call Origin Systems' service line.  We are sorry for the inconvenience."; /* 0x00492b14 */

#pragma function(memset)
/* Function start: 0x40F020 */
/* Nothing in the shipped executable calls this.  Fills count bytes with a
 * value. */
void FillMemoryBytes(void *destination, unsigned int count, short value)
{
    memset(destination, value, count);
}

/* Function start: 0x40F040 */
short MinShort(short a, short b)
{
    if (a < b)
        return a;
    else
        return b;
}

/* Function start: 0x40F072 */
short MaxShort(short a, short b)
{
    if (a > b)
        return a;
    else
        return b;
}

/* Function start: 0x40F0A4 */
void FreePacketAndClear(void *slot, unsigned short releaseFlags)
{
#ifdef SDL_PORT
    void *packet;

    memcpy(&packet, slot, sizeof(packet));
    if (packet != 0) {
        ReleasePacketHandle(packet);
        packet = 0;
        memcpy(slot, &packet, sizeof(packet));
    }
#else
    if (*(void **)slot != 0) {
        ReleasePacketHandle(*(void **)slot);
        *(void **)slot = 0;
    }
#endif
}

/* Function start: 0x40F0D2 */
void ReportPacketLoadError(void *packet, char *fileName,
                           short retry, short section,
                           const char *sourceTag)
{
    unsigned int packetSize;
    short savedPacketError;
    const char *action;

    if (packet != 0 &&
        (g_nPacketError == 0 || g_nPacketError == 8))
        return;
    if (packet == 0 && g_nPacketError == 8)
        return;
    savedPacketError = g_nPacketError;
    if (section != -1)
        packetSize = GetNamedPacketSize(fileName, section);
    LogMemoryUsage();
    g_nPacketError = savedPacketError;
    if (g_nOriginDevUnlock == 0) {
        if (packet == 0 || section == -1)
            action = g_pszPacketAllocateOperation;
        else
            action = g_pszPacketReadOperation;
        sprintf(g_6_szDefaultTextBufferText,
                g_pszPacketLoadErrorFormat,
                action, fileName, (int)section,
                (int)g_nPacketError, packetSize,
                GetLargestFreeMemoryBlockByType(retry), (int)retry,
                sourceTag);
        FatalErrorAndExit(g_6_szDefaultTextBufferText);
    } else {
        printf("Error: %Fs, packet %d\n", fileName, (int)section);
        printf("packet size: %ld\n", packetSize);
        printf("largest block: %ld  FREE: %ld\n",
               GetLargestFreeMemoryBlockByType(retry),
               GetAvailableFarMemoryByType(retry));
        printf("MAIN: %ld   EMS: %ld\n",
               GetAvailableMainMemory(), GetAvailableFarMemory());
        exit(1);
    }
}

#pragma function(strcat, strlen)
/* Function start: 0x40F266 */
void AppendPacketLoadDebugLog(char *fileName, short section,
                              void *packet)
{
    short file;
    char truncatedFileName[36];
    char logLine[80];

    file = (short)open("logfile.txt", 0x4109, 0x80);
    strncpy(truncatedFileName, fileName, strlen(truncatedFileName));
    sprintf(logLine, "\n%s, #%d, %Fp", truncatedFileName,
            (int)section, packet);
    if (g_nShowMemoryStatus != 0) {
        ltoa((long)GetNamedPacketSize(fileName, section),
              truncatedFileName, 10);
        strcat(logLine, ",Size:");
        strcat(logLine, truncatedFileName);
        ltoa((long)GetLargestMainMemoryBlock(), truncatedFileName, 10);
        strcat(logLine, ",LB:");
        strcat(logLine, truncatedFileName);
        ltoa((long)GetAvailableMainMemory(), truncatedFileName, 10);
        strcat(logLine, ",Free:");
        strcat(logLine, truncatedFileName);
    }
    write((int)file, logLine, strlen(logLine));
    close((int)file);
}

/* Function start: 0x40F3AE */
void RewritePacketFilenameForInstalledData(char *fileName)
{
    char *extension;
    char *dot;

    if (g_bRewritePacketExtensions == 1) {
        extension = DosStrchr(fileName, '.');
        dot = extension;
        extension++;
        if (dot != 0 && toupper(*extension) == 'V')
            *extension = 'e';
    }
}

/* Function start: 0x40F40E */
void *LoadPacketIntoBuffer(char *fileName, short section,
                           void *destination, int registerHandle)
{
    void *packet;

    if (g_bRewritePacketExtensions == 1)
        RewritePacketFilenameForInstalledData(fileName);
    packet = LoadNamedPacket(fileName, section, destination, 0, 0,
                             registerHandle);
    ReportPacketLoadError(destination, fileName, 0, section, "RP");
    if (g_nOriginDevUnlock != 0)
        AppendPacketLoadDebugLog(fileName, section, packet);
    return packet;
}

/* Function start: 0x40F49D */
void *LoadPacketAllocated(char *fileName, short section)
{
    short triesLeft;
    char errorText[40];
    void *buffer;
    int bytes;

    triesLeft = 5;
    if (g_bRewritePacketExtensions == 1)
        RewritePacketFilenameForInstalledData(fileName);
    bytes = (int)GetNamedPacketSize(fileName, section);
    buffer = AllocateTaggedMemory((int)(short)bytes, 0x40);
    if (buffer != 0) {
        do {
            LoadNamedPacket(fileName, section, buffer, 0, 0, 1);
            triesLeft--;
        } while (triesLeft > 0 && g_nPacketError != 0 &&
                 g_nPacketError != 8);
    } else {
        sprintf(errorText, "Error in LPN:  %Fs, #%d\n",
                fileName, (int)section);
        exit_squadron(errorText);
    }
    ReportPacketLoadError(buffer, fileName, 0, section, "LPN");
    if (g_nOriginDevUnlock != 0)
        AppendPacketLoadDebugLog(fileName, section, buffer);
    return buffer;
}

/* Function start: 0x40F5B6 */
void *FetchDiskPacketRetrying(char *fileName, short section,
                              short flags)
{
    unsigned int available;
    void *result;
    short retries;
    unsigned int largestBlock;

    result = 0;
    retries = 5;
    if (g_bRewritePacketExtensions == 1)
        RewritePacketFilenameForInstalledData(fileName);
    do {
        FreePacketAndClear(&result, flags);
        largestBlock = GetLargestMainMemoryBlock();
        available = GetAvailableMainMemory();
        result = LoadNamedPacket(fileName, section, 0, flags, 0, 1);
        largestBlock = GetLargestMainMemoryBlock();
        available = GetAvailableMainMemory();
        retries--;
    } while (retries > 0 && g_nPacketError != 0 &&
             g_nPacketError != 8);

    if (result == 0 && g_27_stViewScratchInfo.pixels != 0) {
        free_viewport(&g_27_stViewScratchInfo);
        do {
            FreePacketAndClear(&result, flags);
            largestBlock = GetLargestMainMemoryBlock();
            available = GetAvailableMainMemory();
            result = LoadNamedPacket(fileName, section, 0, flags, 0, 1);
            largestBlock = GetLargestMainMemoryBlock();
            available = GetAvailableMainMemory();
            retries--;
        } while (retries > 0 && g_nPacketError != 0 &&
                 g_nPacketError != 8);
        if (AllocateViewport(&g_27_stViewScratchInfo,
                             g_cPrimaryViewBufferColour,
                             0x20) == 0) {
            ReportPacketLoadError(0, fileName, flags, flags, "LP2");
        }
    }
    if (result == 0 &&
        g_stSecondaryViewBufferBlock.pixels != 0) {
        free_viewport(&g_stSecondaryViewBufferBlock);
        do {
            FreePacketAndClear(&result, flags);
            result = LoadNamedPacket(fileName, section, 0, flags, 0, 1);
            retries--;
        } while (retries > 0 && g_nPacketError != 0 &&
                 g_nPacketError != 8);
        if (AllocateViewport(&g_stSecondaryViewBufferBlock,
                             g_cSecondaryViewBufferColour,
                             0) == 0) {
            ReportPacketLoadError(0, fileName, flags, section, "LP3");
        }
    }
    if (result == 0 && (flags & 4) == 0 &&
        g_nPacketError != 0 && g_nPacketError != 8) {
        ReportPacketLoadError(result, fileName, flags, section, "LP4");
    }
    if (g_nOriginDevUnlock != 0) {
        AppendPacketLoadDebugLog(fileName, section, result);
    }
    return result;
}

/* Function start: 0x40F882 */
void InitializeTextContextFromFont(TextContext *context, short fontIndex,
                                   unsigned char colour,
                                   signed char background)
{
    if (g_36_apTextGlyphs[fontIndex] == 0) {
        if (fontIndex == 1) {
            g_36_apTextGlyphs[fontIndex] =
                FetchDiskPacketRetrying("fonts.fnt", fontIndex, 0x10);
        } else {
            g_36_apTextGlyphs[fontIndex] =
                FetchDiskPacketRetrying("fonts.fnt", fontIndex, 0);
        }
    }
    context->font = g_36_apTextGlyphs[fontIndex];
    context->colour = colour;
    context->backgroundColour = (unsigned char)background;
    SetTextContext(context);
}

/* Function start: 0x40F91C */
void ReleaseTextFont(short fontIndex)
{
    if (fontIndex == 1)
        return;
    if (g_36_apTextGlyphs[fontIndex] != 0) {
        ReleasePacketHandle(g_36_apTextGlyphs[fontIndex]);
        g_36_apTextGlyphs[fontIndex] = 0;
    }
}

/* Function start: 0x40F96E */
void DrawTextAt(TextContext *context, short x, short y,
                const char *text, unsigned char alignment)
{
    char *savedText;
    unsigned char savedAlign;

    savedText = context->text;
    savedAlign = context->alignment;
    if (x < 0 || y < 0)
        return;
    SetTextContext(context);
    SetTextCursor(x, y);
    context->text = (char *)text;
    context->alignment = alignment;
    DrawTextString(text);
    context->text = savedText;
    context->alignment = savedAlign;
}

/* Function start: 0x40F9F7 */
void FlushPendingInputPresses(void)
{
    short done;

    /* The DOS release wait looped on this flag; the port left it set. */
    done = 0;
    /* Retail calls the pump with no argument (no prototype in scope); the
     * SDL build passes the zero the other callers use. */
#ifdef SDL_PORT
    PumpWindowMessages(0);
#else
    ((unsigned int (__cdecl *)(void))PumpWindowMessages)();
#endif
    while (TakeInputPressCount() != 0)
        ServiceInputDevices(15);
}

/* Function start: 0x40FA2C */
short WaitForInputKey(void)
{
    InputEventState *press;
    short key;
    short done;
    void (*previousPump)(void);

    key = 0;
    done = 0;
    previousPump = g_176_pfnCurrentInputPump;
    ConfigureInputPump(1, PollJoystickButtonEvents);
    ServiceInputDevices(15);
    press = FindQueuedInputEvent(INPUT_EVENT_BUTTON_DOWN);
    if (press != 0) {
        key = (short)(press->value + 1);
    } else {
        press = FindQueuedInputEvent(INPUT_EVENT_KEY_DOWN);
        if (press != 0)
            key = (short)press->status;
    }
    if (key != 0)
        FlushPendingInputPresses();
    FlushInputEvents();
    ConfigureInputPump(1, previousPump);
    return key;
}

/* Function start: 0x40FADE */
unsigned short WaitForAnyInputPress(void)
{
    unsigned char result;
    InputEventState polled;
    signed char acknowledged;
    int savedKeyboardMouse;

    acknowledged = 0;
    result = 0x0d;
    savedKeyboardMouse = g_bKeyboardMouseEnabled;
    g_bKeyboardMouseEnabled = 0;
    while (acknowledged == 0) {
        switch (PollInputEvent(&polled, 15)) {
        case INPUT_EVENT_KEY_DOWN:
        case INPUT_EVENT_LEGACY_6:
            result = (unsigned char)polled.value;
            ReleaseInputEventQueue();
        case INPUT_EVENT_BUTTON_DOWN:
            acknowledged++;
            break;
        }
    }
    g_bKeyboardMouseEnabled = savedKeyboardMouse;
    return result;
}

/* Unreferenced in WC2.  WC1's disk unit has the same 0x50 word as its
 * prompt border colour. */
int g_nUnusedDiskWord = 0; /* 0x00492b18 */
short g_nUnusedDiskPromptBorderColour = 0x50; /* 0x00492b1c */
