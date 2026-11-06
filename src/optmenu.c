/*
 *  Text input and modal text panels.
 *
 *  Retail object 0x459160-0x45A00F (link order; see docs/ORDER.md).
 */
#include "game.h"

unsigned char g_abTaggedAllocationPrefix_0049ca40[8] = {
    'j', 'e', 'f', 'f', 0, 0, 0, 0
};
ShortRect g_stModalBounds_0049ca48 = {0x18, 0x28, 0x128, 0x3c};
ModalTextPanel *g_pModalTextPanel_0049ca50 = 0;
unsigned char *g_pCampaignChalkboardShape_0049ca54 = 0;
short g_bDisableChalkboardReplay_0049ca58 = 0;

/* Function start: 0x459160 */
void EraseLastTextInputCharacter(void)
{
    short pixelWidth;
    Viewport clearArea;
    char *text;
    short characterWidth;
    short length;

    text = g_pCurrentTextContext_005c8d1c->text;
    pixelWidth = MeasureTextPixelWidthClamped(text);
    length = DosStrlen(text);
    if (length != 0) {
        characterWidth = (short)GetFontCharWidth(text[length - 1]);
        clearArea = *g_pCurrentTextContext_005c8d1c->viewport;
        clearArea.left += pixelWidth - characterWidth;
        clearArea.right = (short)(clearArea.left + characterWidth - 1);
        clearArea.top = g_pCurrentTextContext_005c8d1c->cursorY;
        clearArea.bottom = (short)(clearArea.top +
            ReadWord((unsigned short *)
                g_pCurrentTextContext_005c8d1c->font) - 1);
        ClearViewport(&clearArea,
                      g_pCurrentTextContext_005c8d1c->backgroundColour);
        g_pCurrentTextContext_005c8d1c->cursorX = (short)(
            g_pCurrentTextContext_005c8d1c->cursorX - characterWidth);
    }
}

/* Function start: 0x45924D */
void EraseTextContextBackground(TextContext *context)
{
    short colour;

    colour = context->backgroundColour;
    if (colour == 0xff)
        colour = g_cSecondaryViewBufferColour_0049cb4c;
    ClearViewport(context->viewport, colour);
}

/* Function start: 0x459294 */
void DrawTextInputCursor(char character)
{
    unsigned char savedBackground =
        g_pCurrentTextContext_005c8d1c->backgroundColour;
    unsigned char ink = g_pCurrentTextContext_005c8d1c->colour;
    short savedX;
    char glyph[2] = { character, 0 };

    savedX = g_pCurrentTextContext_005c8d1c->cursorX;
    g_pCurrentTextContext_005c8d1c->cursorX++;
    g_pCurrentTextContext_005c8d1c->backgroundColour = ink;
    DrawFormattedText(glyph);
    g_pCurrentTextContext_005c8d1c->backgroundColour = savedBackground;
    g_pCurrentTextContext_005c8d1c->cursorX = savedX;
}

/* Function start: 0x45930A */
void EraseCharacterAfterTextCursor(char character)
{
    short savedX;

    savedX = g_pCurrentTextContext_005c8d1c->cursorX;
    g_pCurrentTextContext_005c8d1c->cursorX++;
    ClearTextInputCharacter(character);
    g_pCurrentTextContext_005c8d1c->cursorX = savedX;
}

/* Function start: 0x459348 */
void ClearTextInputCharacter(char character)
{
    Viewport clearArea;
    short characterWidth;

    characterWidth = (short)GetFontCharWidth(character);
    clearArea = *g_pCurrentTextContext_005c8d1c->viewport;
    clearArea.left = g_pCurrentTextContext_005c8d1c->cursorX;
    clearArea.right = (short)(clearArea.left + characterWidth - 1);
    clearArea.top = g_pCurrentTextContext_005c8d1c->cursorY;
    clearArea.bottom = (short)(clearArea.top +
        (short)ReadWord((unsigned short *)g_pCurrentTextContext_005c8d1c->font) - 1);
    ClearViewport(&clearArea,
                  g_pCurrentTextContext_005c8d1c->backgroundColour);
}

/* Function start: 0x4593D6 */
short InitializeModalTextPanel(ModalTextPanel *panel, short fontIndex,
                               ModalPanelBounds bounds, short clearColour,
                               short backgroundColour, short borderColour)
{
    *(ShortRect *)&panel->left = bounds;
    panel->previousContext = g_pCurrentTextContext_005c8d1c;
    g_pCurrentTextContext_005c8d1c = &panel->context;
    panel->context = g_stDefaultStringContext_005d2d20;
    if (fontIndex == -1)
        fontIndex = 1;
    InitializeTextContextFromFont(&panel->context, fontIndex,
                                  (unsigned char)clearColour,
                                  (signed char)backgroundColour);
    panel->viewport = g_stModalSourceWindow_005d2c50;
    /* The panel and its viewports keep their bounds as four loose shorts;
     * the rectangle is copied between them as one 8-byte record. */
    *(ShortRect *)&panel->savedBackground.left = *(ShortRect *)&panel->left;
    *(ShortRect *)&panel->viewport.left = *(ShortRect *)&panel->left;
    if (AllocateViewport(&panel->savedBackground, clearColour, 0) == 0)
        return 0;
    CopyViewportContents(&panel->viewport, &panel->savedBackground);
    panel->context.viewport = &panel->viewport;
    panel->context.text = g_szTextBufferBuffer_005d1c40;
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
    g_pCurrentTextContext_005c8d1c = panel->previousContext;
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
    bounds = g_stModalBounds_0049ca48;
    if (g_pModalTextPanel_0049ca50 == 0) {
        g_pModalTextPanel_0049ca50 = AllocateTaggedMemory(sizeof(ModalTextPanel), 0x40);
    }
    if (g_pModalTextPanel_0049ca50 == 0)
        return 0;
    if (InitializeModalTextPanel(g_pModalTextPanel_0049ca50, fontIndex,
                                 g_stModalBounds_0049ca48,
                                 g_cSecondaryViewBufferColour_0049cb4c,
                                 g_cSecondaryViewBufferColour_0049cb4c,
                                 g_cSecondaryViewBufferColour_0049cb4c) == 0) {
        ReleasePacketHandle(g_pModalTextPanel_0049ca50);
        g_pModalTextPanel_0049ca50 = 0;
        return 0;
    }
    halfWidth = MeasureTextPixelWidthClamped(text) * 8 / 16;
    RestoreModalTextPanel(g_pModalTextPanel_0049ca50);
    bounds.left = 159 - halfWidth;
    bounds.right = 161 + halfWidth;
    if (InitializeModalTextPanel(g_pModalTextPanel_0049ca50, fontIndex,
                                 bounds,
                                 g_bPrimaryViewBufferColour_0049cb50,
                                 g_cBlueColour_0049cb54,
                                 g_cRedColour_0049cb5c) == 0) {
        ReleasePacketHandle(g_pModalTextPanel_0049ca50);
        g_pModalTextPanel_0049ca50 = 0;
        return 0;
    }
    DrawModalTextPanel(g_pModalTextPanel_0049ca50, 0, 6, 2, text);
    MarkDibDirty();
    DIBslamReal();
    return 1;
}

/* Function start: 0x45979B */
void ReleaseModalTextPanel(void)
{
    if (g_pModalTextPanel_0049ca50 != 0) {
        RestoreModalTextPanel(g_pModalTextPanel_0049ca50);
        ReleasePacketHandle(g_pModalTextPanel_0049ca50);
        g_pModalTextPanel_0049ca50 = 0;
    }
    MarkDibDirty();
    DIBslamReal();
}

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
    short length;
    short done;
    short keyHandled;
    unsigned char backgroundInk;
    short inputCode;
    char textBuffer[40];

    done = 0;
    backgroundInk = g_pCurrentTextContext_005c8d1c->backgroundColour;
    if (g_bRewritePacketExtensions_0049cb48 == 0)
        g_pCurrentTextContext_005c8d1c->backgroundColour = 0x5e;
    else
        g_pCurrentTextContext_005c8d1c->backgroundColour = 2;
    oldText = g_pCurrentTextContext_005c8d1c->text;
    oldViewport = g_pCurrentTextContext_005c8d1c->viewport;
    g_pCurrentTextContext_005c8d1c->text = textBuffer;
    textX = g_pCurrentTextContext_005c8d1c->cursorX;
    textY = g_pCurrentTextContext_005c8d1c->cursorY;
    textArea = *g_pCurrentTextContext_005c8d1c->viewport;
    g_pCurrentTextContext_005c8d1c->viewport = &textArea;
    DosStrcpy(textBuffer, destination);
    length = DosStrlen(textBuffer);

    textArea.left = textX;
    textArea.top = textY;
    textArea.bottom = (short)(
        (short)ReadWord((unsigned short *)g_pCurrentTextContext_005c8d1c->font) +
        textY);
    textArea.right = (short)(textArea.left +
        MeasureTextPixelWidthClamped(textBuffer));
    ClearViewport(&textArea,
                  g_pCurrentTextContext_005c8d1c->backgroundColour);
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
                g_pCurrentTextContext_005c8d1c->text = oldText;
                g_pCurrentTextContext_005c8d1c->viewport = oldViewport;
                return 0;
            case 13:
                keyHandled++;
                if (textBuffer[0] == 0)
                    break;
                EraseCharacterAfterTextCursor(' ');
                done++;
                break;
            default:
                if (inputCode == 8 && length != 0) {
                    EraseCharacterAfterTextCursor(' ');
                    EraseLastTextInputCharacter();
                    DrawTextInputCursor(' ');
                    length--;
                    textBuffer[length] = 0;
                } else if (length < maximumLength &&
                           ((allowPathSeparators != 0 &&
                             (inputCode == ':' || inputCode == '\\')) ||
#ifdef SDL_PORT
                            isalnum((unsigned char)inputCode) ||
#else
                            isalnum(inputCode) ||
#endif
                            (inputCode == ' ' && length != 0))) {
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
                    textBuffer[length] = (char)inputCode;
                    length++;
                    textBuffer[length] = 0;
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
    g_pCurrentTextContext_005c8d1c->text = oldText;
    g_pCurrentTextContext_005c8d1c->viewport = oldViewport;
    g_pCurrentTextContext_005c8d1c->backgroundColour = backgroundInk;
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
    g_stDefaultStringContext_005d2d20.viewport =
        &g_stScrViewInfo_005d21a0;
    SetTextCursor((unsigned short)x, (unsigned short)y);
    DrawFormattedText("%s", prompt);
    MarkDibDirty();
    DIBslamReal();
    if (ReadTextInput(destination, (short)maximumLength, (short)mode,
                      (short)required) != 0) {
        result = 1;
    }
    g_stDefaultStringContext_005d2d20.viewport =
        &g_stSecondaryViewBufferBlock_005d2c90;
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
        DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90,
                          58, 175,
                          g_pCampaignChalkboardShape_0049ca54, 3);
        g_stDefaultStringContext_005d2d20.viewport =
            &g_stSecondaryViewBufferBlock_005d2c90;
        InitializeTextContextFromFont(
            &g_stDefaultStringContext_005d2d20, 0,
            g_bPrimaryViewBufferColour_0049cb50, -1);
        if (g_pszPersonnelFooter_00492658 != 0) {
            SetTextCursor(
                (unsigned short)((320 -
                    MeasureTextPixelWidthClamped(
                        g_pszPersonnelFooter_00492658)) >> 1),
                180);
            if (strcmp(g_pszPersonnelFooter_00492658,
                       "Exit to DOS") != 0) {
                DrawFormattedText("%S",
                                  g_pszPersonnelFooter_00492658);
            } else {
                DrawFormattedText("%S", "Exit the Game");
            }
        }
        g_stDefaultStringContext_005d2d20.viewport =
            &g_stScrViewInfo_005d21a0;
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
    SetInputViewport(&g_stSecondaryViewBufferBlock_005d2c90);
    while (selection == 0) {
        g_bDisableChalkboardReplay_0049ca58 =
            (short)(g_pCampaignGlobals_00499c94->field_0a == 0);
        selection = PollCampaignChalkboardMenu(
            g_pCampaignChalkboardShape_0049ca54);
        switch (selection) {
        case 1:
            g_bDisableChalkboardReplay_0049ca58 = 0;
            DisableMouseCursorDrawing();
            FreePacketAndClear(&g_pCampaignChalkboardShape_0049ca54, 0);
            free_viewport(&g_stSecondaryViewBufferBlock_005d2c90);
            StopMusicUnlessSuppressed();
            ReleaseMusicTrack(0x35);
            g_pCampaignGlobals_00499c94->field_08 =
                g_pCampaignGlobals_00499c94->field_0a;
            RunCampaignScript(campaignSlot);
            g_pCampaignGlobals_00499c94->field_08 = 0;
            result = 0;
            break;
        case 4:
            exit_squadron(0);
            break;
        case 3:
            g_bDisableChalkboardReplay_0049ca58 = 0;
            DisableMouseCursorDrawing();
            RunPilotSaveLoadMenu();
            if (g_nOriginDevUnlock_0049d774 != 0)
                strcpy(g_stCurrentPilotProfile_00493408.callsign,
                       "CHEATER");
            DisableMouseCursorDrawing();
            RefreshCampaignChalkboardScreen(
                (short)(g_pCampaignGlobals_00499c94->field_0e & 0xff));
            result = 1;
            break;
        case 5:
            g_bDisableChalkboardReplay_0049ca58 = 0;
            result = 1;
            break;
        case 2:
            g_bDisableChalkboardReplay_0049ca58 = 0;
            DisableMouseCursorDrawing();
            FreePacketAndClear(&g_pCampaignChalkboardShape_0049ca54, 0);
            free_viewport(&g_stSecondaryViewBufferBlock_005d2c90);
            StopMusicUnlessSuppressed();
            ReleaseMusicTrack(0x35);
            ClearViewport(&g_stModalSourceWindow_005d2c50, 0);
            while (again != 0) {
                RunCampaignScript(campaignSlot);
                again = 0;
                if (g_pCampaignGlobals_00499c94->field_16 != 0) {
                    again++;
                    g_pCampaignGlobals_00499c94->field_08++;
                }
                g_pCampaignGlobals_00499c94->field_16 = 0;
            }
            g_pCampaignGlobals_00499c94->field_08 =
                g_pCampaignGlobals_00499c94->field_0a;
            InitializeCampaignChalkboardScreen(
                (short)(g_pCampaignGlobals_00499c94->field_0e & 0xff));
            result = 1;
            break;
        }
    }
    return result;
}
