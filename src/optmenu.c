/*
 *  Text input and modal text panels.
 *
 *  Retail object 0x459160-0x45A00F (link order; see docs/ORDER.md).
 */
#include "game.h"

unsigned char g_abTaggedAllocationPrefix[8] = {
    'j', 'e', 'f', 'f', 0, 0, 0, 0
}; /* 0x0049ca40 */
ShortRect g_stModalBounds = {0x18, 0x28, 0x128, 0x3c}; /* 0x0049ca48 */
ModalTextPanel *g_pModalTextPanel = 0; /* 0x0049ca50 */
unsigned char *g_pCampaignChalkboardShape = 0; /* 0x0049ca54 */
short g_bDisableChalkboardReplay = 0; /* 0x0049ca58 */

/* Function start: 0x459160 */
void EraseLastTextInputCharacter(void)
{
    short pixelWidth;
    Viewport clearArea;
    char *text;
    short characterWidth;
    short length;

    text = g_29_pCurrentTextContext->text;
    pixelWidth = MeasureTextPixelWidthClamped(text);
    length = DosStrlen(text);
    if (length != 0) {
        characterWidth = (short)GetFontCharWidth(text[length - 1]);
        clearArea = *g_29_pCurrentTextContext->viewport;
        clearArea.left += pixelWidth - characterWidth;
        clearArea.right = (short)(clearArea.left + characterWidth - 1);
        clearArea.top = g_29_pCurrentTextContext->cursorY;
        clearArea.bottom = (short)(clearArea.top +
            ReadWord((unsigned short *)
                g_29_pCurrentTextContext->font) - 1);
        ClearViewport(&clearArea,
                      g_29_pCurrentTextContext->backgroundColour);
        g_29_pCurrentTextContext->cursorX = (short)(
            g_29_pCurrentTextContext->cursorX - characterWidth);
    }
}

/* Function start: 0x45924D */
void EraseTextContextBackground(TextContext *context)
{
    short colour;

    colour = context->backgroundColour;
    if (colour == 0xff)
        colour = g_cSecondaryViewBufferColour;
    ClearViewport(context->viewport, colour);
}

/* Function start: 0x459294 */
void DrawTextInputCursor(char character)
{
    unsigned char savedBackground =
        g_29_pCurrentTextContext->backgroundColour;
    unsigned char ink = g_29_pCurrentTextContext->colour;
    short savedX;
    char glyph[2] = { character, 0 };

    savedX = g_29_pCurrentTextContext->cursorX;
    g_29_pCurrentTextContext->cursorX++;
    g_29_pCurrentTextContext->backgroundColour = ink;
    DrawFormattedText(glyph);
    g_29_pCurrentTextContext->backgroundColour = savedBackground;
    g_29_pCurrentTextContext->cursorX = savedX;
}

/* Function start: 0x45930A */
void EraseCharacterAfterTextCursor(char character)
{
    short savedX;

    savedX = g_29_pCurrentTextContext->cursorX;
    g_29_pCurrentTextContext->cursorX++;
    ClearTextInputCharacter(character);
    g_29_pCurrentTextContext->cursorX = savedX;
}

/* Function start: 0x459348 */
void ClearTextInputCharacter(char character)
{
    Viewport clearArea;
    short characterWidth;

    characterWidth = (short)GetFontCharWidth(character);
    clearArea = *g_29_pCurrentTextContext->viewport;
    clearArea.left = g_29_pCurrentTextContext->cursorX;
    clearArea.right = (short)(clearArea.left + characterWidth - 1);
    clearArea.top = g_29_pCurrentTextContext->cursorY;
    clearArea.bottom = (short)(clearArea.top +
        (short)ReadWord((unsigned short *)g_29_pCurrentTextContext->font) - 1);
    ClearViewport(&clearArea,
                  g_29_pCurrentTextContext->backgroundColour);
}

/* Function start: 0x4593D6 */
short InitializeModalTextPanel(ModalTextPanel *panel, short fontIndex,
                               ModalPanelBounds bounds, short clearColour,
                               short backgroundColour, short borderColour)
{
    *(ShortRect *)&panel->left = bounds;
    panel->previousContext = g_29_pCurrentTextContext;
    g_29_pCurrentTextContext = &panel->context;
    panel->context = g_3_stDefaultStringContext;
    if (fontIndex == -1)
        fontIndex = 1;
    InitializeTextContextFromFont(&panel->context, fontIndex,
                                  (unsigned char)clearColour,
                                  (signed char)backgroundColour);
    panel->viewport = g_2_stModalSourceWindow;
    /* The panel and its viewports keep their bounds as four loose shorts;
     * the rectangle is copied between them as one 8-byte record. */
    *(ShortRect *)&panel->savedBackground.left = *(ShortRect *)&panel->left;
    *(ShortRect *)&panel->viewport.left = *(ShortRect *)&panel->left;
    if (AllocateViewport(&panel->savedBackground, clearColour, 0) == 0)
        return 0;
    CopyViewportContents(&panel->viewport, &panel->savedBackground);
    panel->context.viewport = &panel->viewport;
    panel->context.text = g_16_szTextBufferBuffer;
    ResetStringBuilder(&panel->context);
    EraseTextContextBackground(&panel->context);
    DrawViewportBorder(&panel->viewport, panel->left, panel->top,
                       panel->right, panel->bottom, borderColour);
    return 1;
}

#pragma function(strcat)
/* Function start: 0x459527 */
void DrawModalTextPanel(ModalTextPanel *panel, short x, short y,
                        unsigned char alignment,
                        const char *format, ...)
{
    va_list argList;
    char text[84];

    va_start(argList, format);
    vsprintf(text, format, argList);
    va_end(argList);
    SetTextCursor((unsigned short)(panel->left + x),
                  (unsigned short)(panel->top + y));
    panel->context.alignment = alignment;
    strcat(text, "%P");
    FormatTextBufferFromStart(text);
    MarkDibDirty();
    DIBslamReal();
}

/* Function start: 0x4595AA */
void RestoreModalTextPanel(ModalTextPanel *panel)
{
    CopyViewportContents(&panel->savedBackground, &panel->viewport);
    MarkDibDirty();
    DIBslamReal();
    free_viewport(&panel->savedBackground);
    g_29_pCurrentTextContext = panel->previousContext;
}

/* Function start: 0x4595EF */
short ShowModalTextPanel(short fontIndex, const char *format, ...)
{
    ShortRect bounds;
    short halfWidth;
    va_list argList;
    char text[52];

    va_start(argList, format);
    vsprintf(text, format, argList);
    va_end(argList);
    bounds = g_stModalBounds;
    if (g_pModalTextPanel == 0) {
        g_pModalTextPanel = AllocateTaggedMemory(sizeof(ModalTextPanel), 0x40);
    }
    if (g_pModalTextPanel == 0)
        return 0;
    if (InitializeModalTextPanel(g_pModalTextPanel, fontIndex,
                                 g_stModalBounds,
                                 g_cSecondaryViewBufferColour,
                                 g_cSecondaryViewBufferColour,
                                 g_cSecondaryViewBufferColour) == 0) {
        ReleasePacketHandle(g_pModalTextPanel);
        g_pModalTextPanel = 0;
        return 0;
    }
    halfWidth = MeasureTextPixelWidthClamped(text) * 8 / 16;
    RestoreModalTextPanel(g_pModalTextPanel);
    bounds.left = 159 - halfWidth;
    bounds.right = 161 + halfWidth;
    if (InitializeModalTextPanel(g_pModalTextPanel, fontIndex,
                                 bounds,
                                 g_bPrimaryViewBufferColour,
                                 g_cBlueColour,
                                 g_cRedColour) == 0) {
        ReleasePacketHandle(g_pModalTextPanel);
        g_pModalTextPanel = 0;
        return 0;
    }
    DrawModalTextPanel(g_pModalTextPanel, 0, 6, 2, text);
    MarkDibDirty();
    DIBslamReal();
    return 1;
}

/* Function start: 0x45979B */
void ReleaseModalTextPanel(void)
{
    if (g_pModalTextPanel != 0) {
        RestoreModalTextPanel(g_pModalTextPanel);
        ReleasePacketHandle(g_pModalTextPanel);
        g_pModalTextPanel = 0;
    }
    MarkDibDirty();
    DIBslamReal();
}

/* This callee declaration preserves CL 4.1's comparison operand order below. */
void DrawTextInputCursor(char character);

/* Function start: 0x4597E3 */
short ReadTextInput(char *destination, short maximumLength,
                    volatile short mode,
                    volatile short allowPathSeparators)
{
    Viewport textArea;
    Viewport *oldViewport;
    char *oldText;
    short textX;
    short textY;
    short done;
    short keyHandled;
    unsigned char backgroundInk;
    short inputCode;
    char textBuffer[40];
    short characterCount;

    done = 0;
    backgroundInk = g_29_pCurrentTextContext->backgroundColour;
    if (g_bRewritePacketExtensions == 0)
        g_29_pCurrentTextContext->backgroundColour = 0x5e;
    else
        g_29_pCurrentTextContext->backgroundColour = 2;
    oldText = g_29_pCurrentTextContext->text;
    oldViewport = g_29_pCurrentTextContext->viewport;
    g_29_pCurrentTextContext->text = textBuffer;
    textX = g_29_pCurrentTextContext->cursorX;
    textY = g_29_pCurrentTextContext->cursorY;
    textArea = *g_29_pCurrentTextContext->viewport;
    g_29_pCurrentTextContext->viewport = &textArea;
    DosStrcpy(textBuffer, destination);
    characterCount = DosStrlen(textBuffer);

    textArea.left = textX;
    textArea.top = textY;
    textArea.bottom = (short)(
        (short)ReadWord((unsigned short *)g_29_pCurrentTextContext->font) +
        textY);
    textArea.right = (short)(textArea.left +
        MeasureTextPixelWidthClamped(textBuffer));
    ClearViewport(&textArea,
                  g_29_pCurrentTextContext->backgroundColour);
    DrawFormattedText(textBuffer);
    DrawTextInputCursor(' ');

    MarkDibDirty();
    DIBslamReal();
    while (done == 0) {
        keyHandled = 0;
        while (keyHandled == 0) {
            ServiceSoundSystem();
            inputCode = WaitForQueuedInputPress();
            switch (inputCode) {
            case 27:
                keyHandled++;
                if (textBuffer[0] != 0)
                    EraseCharacterAfterTextCursor(' ');
                g_29_pCurrentTextContext->text = oldText;
                g_29_pCurrentTextContext->viewport = oldViewport;
                return 0;
            case 13:
                keyHandled++;
                if (textBuffer[0] == 0)
                    break;
                EraseCharacterAfterTextCursor(' ');
                done++;
                break;
            default:
                if (inputCode == 8 && characterCount != 0) {
                    EraseCharacterAfterTextCursor(' ');
                    EraseLastTextInputCharacter();
                    DrawTextInputCursor(' ');
                    characterCount--;
                    textBuffer[characterCount] = 0;
                } else if (characterCount < maximumLength &&
                           ((allowPathSeparators != 0 &&
                             (inputCode == ':' || inputCode == '\\')) ||
#ifdef SDL_PORT
                            isalnum((unsigned char)inputCode) ||
#else
                            isalnum(inputCode) ||
#endif
                            (inputCode == ' ' && characterCount != 0))) {
                    switch (mode) {
                    case 1:
                        inputCode = (short)toupper(inputCode);
                        break;
                    case 2:
                        if (inputCode < '0' || inputCode > '9')
                            inputCode = 0;
                        break;
                    }
                    if (inputCode == 0)
                        break;
                    EraseCharacterAfterTextCursor(' ');
                    textBuffer[characterCount] = (char)inputCode;
                    characterCount++;
                    textBuffer[characterCount] = 0;
                    SetTextCursor((unsigned short)textX,
                                  (unsigned short)textY);
                    DrawFormattedText(textBuffer);
                    DrawTextInputCursor(' ');
                }
                keyHandled++;
                break;
            }
        }
        MarkDibDirty();
        DIBslamReal();
    }

    DosStrcpy(destination, textBuffer);
    g_29_pCurrentTextContext->text = oldText;
    g_29_pCurrentTextContext->viewport = oldViewport;
    g_29_pCurrentTextContext->backgroundColour = backgroundInk;
    return 1;
}

/* Function start: 0x459BC8 */
short ReadPersonnelTextField(unsigned int x, unsigned int y,
                             int font, const char *prompt,
                             char *destination, int maximumLength,
                             int mode, int required)
{
    short result;

    result = 0;
    g_3_stDefaultStringContext.viewport =
        &g_101_stScrViewInfo;
    SetTextCursor((unsigned short)x, (unsigned short)y);
    DrawFormattedText("%s", prompt);
    MarkDibDirty();
    DIBslamReal();
    if (ReadTextInput(destination, (short)maximumLength, (short)mode,
                      (short)required) != 0) {
        result = 1;
    }
    g_3_stDefaultStringContext.viewport =
        &g_stSecondaryViewBufferBlock;
    return result;
}

#pragma function(strcmp)
/* Function start: 0x459C4D */
short PollCampaignChalkboardMenu(unsigned char *scene)
{
    short selection;
    short done;

    selection = 0;
    done = 0;
    SetMenuInputPump();
    FlushInputEvents();
    while ((selection = PollSceneHotspotInput(scene, 0, 0, 0, 0)) == 0) {
        DisableMouseCursorDrawing();
        DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                          58, 175,
                          g_pCampaignChalkboardShape, 3);
        g_3_stDefaultStringContext.viewport =
            &g_stSecondaryViewBufferBlock;
        InitializeTextContextFromFont(
            &g_3_stDefaultStringContext, 0,
            g_bPrimaryViewBufferColour, -1);
        if (g_pszPersonnelFooter != 0) {
            SetTextCursor(
                (unsigned short)((320 -
                    MeasureTextPixelWidthClamped(
                        C_("hotspot", g_pszPersonnelFooter))) >> 1),
                180);
            if (strcmp(g_pszPersonnelFooter,
                       "Exit to DOS") != 0) {
                DrawFormattedText("%S",
                                  C_("hotspot", g_pszPersonnelFooter));
            } else {
                DrawFormattedText("%S", _("Exit the Game"));
            }
        }
        g_3_stDefaultStringContext.viewport =
            &g_101_stScrViewInfo;
        EnableMouseCursorDrawing();
        RefreshMemoryStatusOverlay();
    }
    return selection;
}

#pragma function(strcpy)
/* Function start: 0x459D74 */
short RunCampaignChalkboardMenu(short campaignSlot)
{
    short again;
    short result;
    short selection;

    selection = 0;
    result = 1;
    again = 1;
    PreloadMusicTrack(0x35);
    spacetrack(0x35, 2, 1);
    SetInputViewport(&g_stSecondaryViewBufferBlock);
    while (selection == 0) {
        g_bDisableChalkboardReplay =
            (short)(g_pCampaignGlobals->field_0a == 0);
        selection = PollCampaignChalkboardMenu(
            g_pCampaignChalkboardShape);
        switch (selection) {
        case 1:
            g_bDisableChalkboardReplay = 0;
            DisableMouseCursorDrawing();
            FreePacketAndClear(&g_pCampaignChalkboardShape, 0);
            free_viewport(&g_stSecondaryViewBufferBlock);
            StopMusicUnlessSuppressed();
            ReleaseMusicTrack(0x35);
            g_pCampaignGlobals->field_08 =
                g_pCampaignGlobals->field_0a;
            RunCampaignScript(campaignSlot);
            g_pCampaignGlobals->field_08 = 0;
            result = 0;
            break;
        case 4:
            exit_squadron(0);
            break;
        case 3:
            g_bDisableChalkboardReplay = 0;
            DisableMouseCursorDrawing();
            RunPilotSaveLoadMenu();
            if (g_nOriginDevUnlock != 0)
                strcpy(g_stCurrentPilotProfile.callsign,
                       "CHEATER");
            DisableMouseCursorDrawing();
            RefreshCampaignChalkboardScreen(
                (short)(g_pCampaignGlobals->field_0e & 0xff));
            result = 1;
            break;
        case 5:
            g_bDisableChalkboardReplay = 0;
            result = 1;
            break;
        case 2:
            g_bDisableChalkboardReplay = 0;
            DisableMouseCursorDrawing();
            FreePacketAndClear(&g_pCampaignChalkboardShape, 0);
            free_viewport(&g_stSecondaryViewBufferBlock);
            StopMusicUnlessSuppressed();
            ReleaseMusicTrack(0x35);
            ClearViewport(&g_2_stModalSourceWindow, 0);
            while (again != 0) {
                RunCampaignScript(campaignSlot);
                again = 0;
                if (g_pCampaignGlobals->field_16 != 0) {
                    again++;
                    g_pCampaignGlobals->field_08++;
                }
                g_pCampaignGlobals->field_16 = 0;
            }
            g_pCampaignGlobals->field_08 =
                g_pCampaignGlobals->field_0a;
            InitializeCampaignChalkboardScreen(
                (short)(g_pCampaignGlobals->field_0e & 0xff));
            result = 1;
            break;
        }
    }
    return result;
}
