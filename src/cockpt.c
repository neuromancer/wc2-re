/*
 *  Cockpit HUD (Mac `cockpt` compilation unit): weapon, damage, target,
 *  objective, message, and pilot displays.
 *
 *  Address range 0x413000-0x417fff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: the Mac `cockpt` symbol list identifies the surviving
 *  functions throughout this Win32 range; port-specific split helpers remain
 *  interleaved in their original address order.
 */
#include "wc1.h"
Viewport g_stTrainSimVduSource_00469210_WC1_UNMAPPED /* no-address */ = {0};
ShortPoint g_aaCockpitDamagePositions_00469228_WC1_UNMAPPED /* no-address */[5][4] = {
    {{224, 5}, {132, 96}, {233, 107}, {149, 161}},
    {{177, 6}, {153, 142}, {103, 140}, {55, 183}},
    {{107, 25}, {211, 32}, {21, 178}, {300, 178}},
    {{74, 10}, {294, 19}, {197, 105}, {105, 134}},
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}}
};
unsigned char *g_pConfedCommBackground_00469278_WC1_UNMAPPED /* no-address */ = 0;
unsigned char *g_pCommStaticShape_0046927c_WC1_UNMAPPED /* no-address */ = 0;
unsigned char *g_pKilrathiCommBackground_00469280 = 0;
short g_bDisplayWingmanTargetData_0049347c;
ShortPoint g_stHudMessageOrigin_0049ae90;

#pragma function(strlen, strcpy, abs)

short g_asVduSelectionSound_0049afe4[2] = { 0x7f, 0 };
char *g_pszPendingHudMessage_0049afec = 0;
char *g_pszDisplayedHudMessage_0049aff0 = 0;
unsigned char g_cLastPilotHandFrame_0049aff4 = 0xff;
short g_asPilotHandOffsets_0049aff8[34] = {
    6, -3, 7, 2, 7, 9, 7, 12, 8, 13, 0, -1, -1, -1,
    -4, -1, -6, -1, 6, 0, 8, 0, 10, 0, 13, 3, 8, -7,
    6, -9, 5, -11, 5, -14
};
unsigned char *g_pCockpitPilotShape_0046905c_WC1_UNMAPPED /* no-address */ = 0;
unsigned char * volatile g_pCockpitExplosionShape_00469064_WC1_UNMAPPED /* no-address */ = 0;
int g_nDisplayedObjectiveRange_0049b078 = 40000;
short g_nScannerTargetObject_0049b07c = -1;
const int g_aiForwardScannerGridRows_0049b080[78] = {
    5, 13, 16, -1,
    5, 13, 16, -1,
    5, 13, 16, -1,
    4, 13, 16, -1,
    4, 12, 16, -1,
    2, 3, 4, 12, 15, -1,
    0, 1, 5, 12, 15, -1,
    6, 11, 15, -1,
    7, 11, 14, -1,
    8, 10, 14, -1,
    9, 13, -1,
    8, 13, -1,
    6, 7, 12, -1,
    4, 5, 11, -1,
    0, 1, 2, 3, 10, -1,
    9, -1,
    7, 8, -1,
    4, 5, 6, -1,
    0, 1, 2, 3,
    -2
};
const int g_aiTargetCameraScannerGridRows_0049b1b8[50] = {
    5, 13, -1,
    5, 13, -1,
    5, 13, -1,
    4, 13, -1,
    4, 12, -1,
    2, 3, 4, 12, -1,
    0, 1, 5, 11, 12, -1,
    6, 11, -1,
    7, 11, -1,
    8, 10, -1,
    9, -1,
    8, -1,
    6, 7, -1,
    4, 5, -1,
    0, 1, 2, 3,
    -2
};
const char *g_pszMissileLocked_0049b280 = g_szMissileLocked_0049b30c;
unsigned char *g_pHudMessageFrameShape_0049b288 = 0;
unsigned char *g_pHudMessageBackground_0049b28c = 0;
short g_bCaptureHudMessageBackground_0049b290 = 1;
short g_nHudMessageBackgroundDepth_0049b294;
short g_nTargetLockMarkerX_0049b298 = -0x7fff;
ShortRect g_stPreviousTargetBracketBounds_00469200_WC1_UNMAPPED /* no-address */ = {-0x7fff, 0, 0, 0};
int g_nCommPortraitFrame_0049b2bc = -1;
int g_bForceDamageDisplayRedraw_0049b2ec = 0;
char g_szMissileLocked_0049b30c[16] = "MISSILE LOCKED ";
char g_szAlreadyNear_0049b334[16] = "Already Near";
char g_szEnemyNear_0049b344[12] = "Enemy Near";
char g_szHazardNear_0049b350[12] = "Hazard Near";
char g_szComponentHitFormat_0049b35c[8] = "%s HIT";
char g_szCalculating_0049b3a0[12] = "CALCULATING";
char g_szRangeKilometresSuffix_0049b3ac[4] = " km";
char g_szNoObjective_0049b3b0[8] = "NONE";
char g_szUnknownObjective_0049b3b8[8] = "UNKNOWN";
char g_szCompNavigation_0049b3c0[16] = "COMP NAVIGATION";
char g_szDestinationFormat_0049b3d0[20] =
    "\n\nDESTINATION\n %s";
char g_szNavigationRangeLabel_0049b3e4[12] = "\n\nRANGE\n ";
char g_szNewObjectivePrompt_0049b3f0[20] =
    "\n\n(N)ew Objective";
char g_szAlreadyVisited_0049b404[16] = "Already Visited";
char g_szObjectiveReached_0049b414[20] = "Objective Reached";
char g_szWaitForFormat_0049b428[12] = "Wait for %s";
char *g_pszGameVersion_0049b528 = g_szGameVersion_0049b52c;
char g_szGameVersion_0049b52c[9] = "1.03F-95";
short g_nViewportOriginY_005c849c;
signed char g_cHudMessageView_005d1c37;

/* Function start: 0x420340 */
void EmitTextString(void (*writer)(int), const char *text)
{
    short character;

    character = (signed char)*text++;
    while (character != 0) {
        writer((short)character);
        character = (signed char)*text++;
    }
}

/* Function start: 0x420378 */
void FormatTextTokens(void (*writer)(int),
#ifdef WC1_SDL
                      const char *format, va_list *arguments)
#else
                      const char *format, va_list arguments)
#endif
{
    short character;
    char number[64];

    if (g_pCurrentTextContext_005c8d1c == 0)
        return;
    character = (signed char)*format++;
    while (character != 0) {
        if (character == '%') {
            character = (signed char)*format++;
            switch (character) {
            case 'd':
#ifdef WC1_SDL
                EmitTextString(
                    writer,
                    _itoa((short)va_arg(*arguments, int), number, 10));
#else
                EmitTextString(writer,
                               _itoa(va_arg(arguments, short), number, 10));
#endif
                break;
            case 'u':
#ifdef WC1_SDL
                EmitTextString(
                    writer,
                    _ultoa((unsigned short)va_arg(*arguments, unsigned int),
                           number, 10));
#else
                EmitTextString(
                    writer,
                    _ultoa(va_arg(arguments, unsigned short), number, 10));
#endif
                break;
            case 'D':
#ifdef WC1_SDL
                EmitTextString(writer,
                               _ltoa(va_arg(*arguments, long), number, 10));
#else
                EmitTextString(writer,
                               _ltoa(va_arg(arguments, long), number, 10));
#endif
                break;
            case 'U':
#ifdef WC1_SDL
                EmitTextString(
                    writer,
                    _ultoa(va_arg(*arguments, unsigned long), number, 10));
#else
                EmitTextString(
                    writer,
                    _ultoa(va_arg(arguments, unsigned long), number, 10));
#endif
                break;
            case 'x':
#ifdef WC1_SDL
                EmitTextString(
                    writer,
                    _strupr(_ultoa(
                        (unsigned short)va_arg(*arguments, unsigned int),
                        number, 16)));
#else
                EmitTextString(
                    writer,
                    _strupr(_ultoa(va_arg(arguments, unsigned short),
                                   number, 16)));
#endif
                break;
            case 'c':
#ifdef WC1_SDL
                writer((short)va_arg(*arguments, int));
#else
                writer(va_arg(arguments, short));
#endif
                break;
            case 's':
#ifdef WC1_SDL
                EmitTextString(writer, va_arg(*arguments, char *));
#else
                EmitTextString(writer, va_arg(arguments, char *));
#endif
                break;
            case 'X':
#ifdef WC1_SDL
                g_pCurrentTextContext_005c8d1c->cursorX =
                    (short)va_arg(*arguments, int);
#else
                g_pCurrentTextContext_005c8d1c->cursorX =
                    va_arg(arguments, short);
#endif
                break;
            case 'Y':
#ifdef WC1_SDL
                g_pCurrentTextContext_005c8d1c->cursorY =
                    (short)va_arg(*arguments, int);
#else
                g_pCurrentTextContext_005c8d1c->cursorY =
                    va_arg(arguments, short);
#endif
                break;
            case 'B':
#ifdef WC1_SDL
                g_pCurrentTextContext_005c8d1c->backgroundColour =
                    (unsigned char)va_arg(*arguments, int);
#else
                g_pCurrentTextContext_005c8d1c->backgroundColour =
                    va_arg(arguments, unsigned char);
#endif
                break;
            case 'F':
#ifdef WC1_SDL
                g_pCurrentTextContext_005c8d1c->colour =
                    (unsigned char)va_arg(*arguments, int);
#else
                g_pCurrentTextContext_005c8d1c->colour =
                    va_arg(arguments, unsigned char);
#endif
                break;
            case 'P':
                if (g_bCinematicSpriteFontEnabled_005c82a7 == 0)
                    DrawTextString(g_pCurrentTextContext_005c8d1c->text);
                else
                    DrawWrappedCinematicText(
                        g_pCurrentTextContext_005c8d1c->text);
                break;
            case 'J':
#ifdef WC1_SDL
                g_pCurrentTextContext_005c8d1c->alignment =
                    (unsigned char)va_arg(*arguments, int);
#else
                g_pCurrentTextContext_005c8d1c->alignment =
                    va_arg(arguments, unsigned char);
#endif
                break;
            default:
                writer(character);
                break;
            }
        } else {
            writer(character);
        }
        character = (signed char)*format++;
    }
}

/* Function start: 0x42067F */
void DrawFormattedText(const char *format, ...)
{
    va_list arguments;

    va_start(arguments, format);
    if (g_bCinematicSpriteFontEnabled_005c82a7 == 0) {
        FormatTextTokens((void (*)(int))DrawTextCharacter, format,
#ifdef WC1_SDL
                         &arguments);
#else
                         arguments);
#endif
    } else {
        FormatTextTokens((void (*)(int))DrawCinematicFontCharacter,
                         format,
#ifdef WC1_SDL
                         &arguments);
#else
                         arguments);
#endif
    }
    va_end(arguments);
    if (g_pCurrentTextContext_005c8d1c->viewport->pixels ==
        g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
}

/* Function start: 0x4206F2 */
void FormatTextBufferFromStart(const char *format, ...)
{
    va_list arguments;

    va_start(arguments, format);
    if (g_pCurrentTextContext_005c8d1c != 0) {
        g_pCurrentTextContext_005c8d1c->textCursor =
            g_pCurrentTextContext_005c8d1c->text;
        FormatTextTokens((void (*)(int))AppendTextCharacter, format,
#ifdef WC1_SDL
                         &arguments);
#else
                         arguments);
#endif
        va_end(arguments);
        if (g_pCurrentTextContext_005c8d1c->viewport->pixels ==
            g_stScreenViewport_005d21a0.pixels)
            MarkDibDirty();
    }
}

/* Function start: 0x42075F */
void AppendFormattedText(const char *format, ...)
{
    va_list arguments;

    va_start(arguments, format);
    FormatTextTokens((void (*)(int))AppendTextCharacter, format,
#ifdef WC1_SDL
                     &arguments);
#else
                     arguments);
#endif
    va_end(arguments);
    if (g_pCurrentTextContext_005c8d1c->viewport->pixels ==
        g_stScreenViewport_005d21a0.pixels)
        MarkDibDirty();
}

/* Function start: 0x438020 */
void ShowPlayerEjectionSequence(void)
{
    InputPumpContext *savedInputContext;

    savedInputContext = g_pActiveInputContext_005c8487;
    ejection_sequence((short)(g_nPlayerShipType_00493464 + 0x19), 0);
    if (savedInputContext != 0)
        SetInputViewport(savedInputContext->viewport);
    ConfigureInputPump(1, 0);
    g_nUiInputMode_005c8d3c = 1;
    g_nInputRepeatDelay_005c80d6 = 0x14;
}

/* Function start: 0x438080 */
void ShowPlayerStrandingSequence(void)
{
    ejection_sequence((short)(g_nPlayerShipType_00493464 + 500), 0);
}

/* Function start: 0x4380B0 */
void FatalErrorAndExit(const char *format, ...)
{
    char text[0xfc];

#ifdef WC1_SDL
    va_list arguments;

    va_start(arguments, format);
    vsprintf(text, format, arguments);
    va_end(arguments);
#else
    vsprintf(text, format, (char *)(&format + 1));
#endif
    ShutdownEventManager();
    exit_squadron(text);
}

/* Function start: 0x4380FD */
short IsCockpitExplosionActive(void)
{
    return g_nCockpitExplosionFrame_0049b04c < 8;
}

/* Function start: 0x438129 */
void EraseCockpitReadoutRegion(Viewport *viewport, short left,
                               short top, short right, short bottom,
                               short colour)
{
    Viewport clippedViewport;

    clippedViewport = *viewport;
    if (right >= left && bottom >= top) {
        clippedViewport.left = left;
        clippedViewport.top = top;
        clippedViewport.right = right;
        clippedViewport.bottom = bottom;
        ClearViewport(&clippedViewport, colour);
    }
}

/* Function start: 0x438194 */
void vdu_polygon(signed char bar, short percent)
{
    CockpitBarDefinition definition;
    unsigned char *shape;
    short length;
    short left;
    short swapFrame;
    short direction;
    signed char filledFrame;
    short top;
    signed char emptyFrame;
    short right;
    short bottom;

    if (g_nCurrentView_00492fa8 == 0) {
        definition = g_aCockpitBarDefinitions_0049aed8[(int)bar];
        shape = g_pCockpitPanelShape_005d2cd4;
    } else {
        definition = ((CockpitBarDefinition *)
            (g_abGunDisplayConfiguration_0049d340 +
             g_nGunDisplayIndex_005c8dc0 * 0x20))[(int)bar];
        shape = g_apCockpitVduOverlayShapes_0049a5f8[
            g_nGunDisplayIndex_005c8dc0];
    }
    length = definition.length;
    percent = (short)(((int)percent * (int)length) / 100);
    g_stCockpitBarViewport_005d21e0.left = definition.left;
    left = g_stCockpitBarViewport_005d21e0.left;
    if (left == -99)
        return;
    g_stCockpitBarViewport_005d21e0.right = definition.right;
    right = g_stCockpitBarViewport_005d21e0.right;
    g_stCockpitBarViewport_005d21e0.top = definition.top;
    top = g_stCockpitBarViewport_005d21e0.top;
    g_stCockpitBarViewport_005d21e0.bottom = definition.bottom;
    bottom = g_stCockpitBarViewport_005d21e0.bottom;
    emptyFrame = (signed char)definition.emptyFrame;
    filledFrame = (signed char)definition.filledFrame;
    direction = definition.direction;
    if (direction < 2) {
        if (direction == 1) {
            percent = (short)(length - percent);
            swapFrame = emptyFrame;
            emptyFrame = filledFrame;
            filledFrame = (signed char)swapFrame;
        }
        g_stCockpitBarViewport_005d21e0.bottom -= percent;
        if (g_stCockpitBarViewport_005d21e0.top <=
            g_stCockpitBarViewport_005d21e0.bottom)
            DrawSpriteDefault(&g_stCockpitBarViewport_005d21e0, left, top,
                              shape, filledFrame);
        g_stCockpitBarViewport_005d21e0.top =
            (short)(g_stCockpitBarViewport_005d21e0.bottom + 1);
        g_stCockpitBarViewport_005d21e0.bottom = bottom;
        if (g_stCockpitBarViewport_005d21e0.top <= bottom)
            DrawSpriteDefault(&g_stCockpitBarViewport_005d21e0, left, top,
                              shape, emptyFrame);
    } else {
        if (direction == 3) {
            percent = (short)(length - percent);
            swapFrame = emptyFrame;
            emptyFrame = filledFrame;
            filledFrame = (signed char)swapFrame;
        }
        g_stCockpitBarViewport_005d21e0.right -= percent;
        if (g_stCockpitBarViewport_005d21e0.left <=
            g_stCockpitBarViewport_005d21e0.right)
            DrawSpriteDefault(&g_stCockpitBarViewport_005d21e0, left, top,
                              shape, filledFrame);
        g_stCockpitBarViewport_005d21e0.left =
            (short)(g_stCockpitBarViewport_005d21e0.right + 1);
        g_stCockpitBarViewport_005d21e0.right = right;
        if (g_stCockpitBarViewport_005d21e0.left <= right)
            DrawSpriteDefault(&g_stCockpitBarViewport_005d21e0, left, top,
                              shape, emptyFrame);
    }
}

/* Function start: 0x438454 */
unsigned int InitializeCockpitReadout(signed char slot,
                                      TextContext *context)
{
    g_aCockpitReadouts_005d1e30[(int)slot].context = context;
    g_aCockpitReadouts_005d1e30[(int)slot].x = context->cursorX;
    g_aCockpitReadouts_005d1e30[(int)slot].y = context->cursorY;
    g_aCockpitReadouts_005d1e30[(int)slot].previousRight = 0;
    return 0;
}

/* Function start: 0x4384AD */
void DrawCockpitReadout(signed char slot, const char *text)
{
    if (g_aCockpitReadouts_005d1e30[(int)slot].x != -99) {
        SetTextContext(
            g_aCockpitReadouts_005d1e30[(int)slot].context);
        SetTextCursor(
            (unsigned short)g_aCockpitReadouts_005d1e30[(int)slot].x,
            (unsigned short)g_aCockpitReadouts_005d1e30[(int)slot].y);
        DrawFormattedText(text);
        EraseCockpitReadoutRegion(
            &g_stScreenViewport_005d21a0,
            g_aCockpitReadouts_005d1e30[(int)slot].context->cursorX,
            g_aCockpitReadouts_005d1e30[(int)slot].y,
            g_aCockpitReadouts_005d1e30[(int)slot].previousRight,
            (short)(*(unsigned short *)
                        g_aCockpitReadouts_005d1e30[(int)slot]
                            .context->font +
                    g_aCockpitReadouts_005d1e30[(int)slot].y - 1),
            g_cSecondaryViewBufferColour_0049cb4c);
        g_aCockpitReadouts_005d1e30[(int)slot].previousRight =
            g_aCockpitReadouts_005d1e30[(int)slot].context->cursorX;
    }
}

/* Function start: WC2_UNMAPPED */
void EraseCockpitReadoutAtPosition(signed char slot, short left,
                                   short top)
{
    CockpitReadout *readout;

    readout = &g_aCockpitReadouts_005d1e30[(int)slot];
    EraseCockpitReadoutRegion(
        &g_stScreenViewport_005d21a0, left, top, readout->previousRight,
        (short)(*(short *)readout->context->font + readout->y),
        g_cSecondaryViewBufferColour_0049cb4c);
}

/* Function start: 0x4385B3 */
short DrawHudMessageSlot(HudMessageSlot *slot)
{
    unsigned char savedColour;
    short oldDrawColour;
    short showingEraseColour;

    if (g_nCurrentView_00492fa8 != 0)
        return 1;

    oldDrawColour = slot->drawColour;
    if (((int)g_nInputClock_005c84a8 / 40) % 3 == 0)
        slot->drawColour = g_cSecondaryViewBufferColour_0049cb4c;
    else
        slot->drawColour = slot->colour;

    if (slot->flashCount != -1) {
        if (slot->drawColour == g_cSecondaryViewBufferColour_0049cb4c &&
            oldDrawColour == slot->colour) {
            slot->flashCount = (signed char)MaxShort(
                0, (short)(slot->flashCount - 1));
        }
        if (slot->flashCount == 0)
            slot->drawColour = g_cSecondaryViewBufferColour_0049cb4c;
    }

    showingEraseColour = slot->drawColour == g_cSecondaryViewBufferColour_0049cb4c;
    savedColour = slot->context->colour;
    slot->context->colour = (unsigned char)slot->drawColour;
    DrawTextAt(slot->context, slot->x, slot->y, slot->text, 2);
    slot->context->colour = savedColour;
    return showingEraseColour;
}

/* Function start: 0x438711 */
void ClearHudMessageSlot(HudMessageSlot *slot)
{
    slot->flashCount = 0;
    if (slot->text != 0)
        DrawHudMessageSlot(slot);
    if (slot->text != 0) {
        slot->text = 0;
        if (slot->text == 0 && g_nCurrentView_00492fa8 == 0 &&
            get_mode(0) == 1 && IsCockpitWeaponShapeLoaded() != 0)
            show_weapon_disp();
    } else {
        slot->text = 0;
    }
}

/* Function start: 0x4387A7 */
void ClearHudMessageIfMatching(HudMessageSlot *slot, const char *text)
{
    if (slot->text == text)
        ClearHudMessageSlot(slot);
}

/* Function start: 0x4387CD */
void ClearHudGunReadouts(void)
{
    g_aHudMessageSlots_005d1d40[0].text = 0;
    g_aHudMessageSlots_005d1d40[0].flashCount = 0;
    g_aHudMessageSlots_005d1d40[1].text = 0;
    g_aHudMessageSlots_005d1d40[1].flashCount = 0;
}

/* Function start: 0x4387FA */
void SetHudMessageSlot(HudMessageSlot *slot, TextContext *context,
                       short x, short y, const char *text,
                       unsigned short colour, signed char flashCount)
{
    if (slot->text != 0)
        ClearHudMessageSlot(slot);
    slot->context = context;
    slot->x = x;
    slot->y = y;
    slot->text = text;
    slot->colour = colour;
    slot->drawColour = colour;
    slot->flashCount = flashCount;
}

/* Function start: 0x438864 */
void UpdateMessage(HudMessageSlot *slot)
{
    short showingEraseColour;

    if (slot->text != 0) {
        showingEraseColour = DrawHudMessageSlot(slot);
        if (slot->flashCount == 0 && showingEraseColour != 0)
            ClearHudMessageSlot(slot);
    }
}

/* Function start: 0x4388B6 */
void set_global_message(const char *text, unsigned short colour,
                        int flashCount)
{
    SetHudMessageSlot(&g_aHudMessageSlots_005d1d40[1], &g_stRightVduTextContext_005d2ce0,
                      g_stRightVduViewport_005d2b20.left,
                      (short)(g_stRightVduViewport_005d2b20.bottom - 6),
                      text, colour, (signed char)flashCount);
}

/* Function start: 0x4388F5 */
void CockpitMessage(const char *text, unsigned short colour,
                    int flashCount)
{
    if (text != g_aHudMessageSlots_005d1d40[1].text)
        set_global_message(text, colour, flashCount);
}

/* Function start: 0x43893F */
void remove_message(const char *text)
{
    ClearHudMessageIfMatching(&g_aHudMessageSlots_005d1d40[1], text);
}

/* Function start: 0x43895F */
short kilrathi_near(short obj, short range)
{
    short ship;

    for (ship = 1; ship < 10; ship++) {
        if (g_aeObjectClass_00495328[ship] >= OBJECT_CLASS_SHIP &&
            g_asShipSide_004955d0[ship] == SIDE_KILRATHI &&
            IsPointWithinRange(&g_aShipPosition_00494550[obj],
                               &g_aShipPosition_00494550[ship],
                               range) != 0)
            return 1;
    }
    return 0;
}

/* Function start: 0x4389FE */
short IsMissionObjectiveOutOfSystem(short objectiveIndex)
{
    short navPoint;
    MissionObjective *objective;

    objective = &g_aMissionObjectives_004932a8[objectiveIndex];
    switch (objective->type) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 6:
        navPoint = find_ships_sphere(objective->index);
        if (navPoint == -1)
            return 0;
        break;
    case 0:
    case 5:
        navPoint = objective->index;
        break;
    }
    if (g_aMissionNavPoints_00491e98[navPoint].systemIndex ==
        g_nCurrentNavPoint_004931bc)
        return 0;
    return 1;
}

/* Function start: 0x438ADD */
short auto_pilot_valid(short showReason)
{
    const char *reason;
    short distance;
    short object;

    reason = 0;
    if (g_cMissionObjectiveCount_00493294 == 0)
        return 0;
    if (g_asShipSide_004955d0[0] == SIDE_KILRATHI)
        return 0;
    if (IsCockpitExplosionActive() != 0)
        return 0;
    if (IsMissionObjectiveOutOfSystem(
            (short)g_cCurrentObjective_004931cc) != 0)
        reason = "Jump Point";
    distance = distance_from_point(
        0, &g_aMissionObjectives_004932a8[
               (signed char)g_cCurrentObjective_004931cc].position);
    object = 1;
    while (object < 10) {
        if (g_aeObjectClass_00495328[object] > OBJECT_CLASS_SHIP &&
            g_asShipMissionType_00495de8[object] ==
                MISSION_TYPE_GOTO_WARP &&
            g_asShipMissionParameter_00495e00[object] ==
                g_nCurrentNavPoint_004931bc &&
            equ_vector(
                &g_aMissionObjectives_004932a8[
                    (signed char)g_cCurrentObjective_004931cc].position,
                &g_aShipDestination_004953f0[object]) == 0) {
            sprintf(g_pszAutopilotWaitReason_0049b050, "Wait for %s",
                    g_apShipMissionRecord_00495da8[object]->name);
            reason = g_pszAutopilotWaitReason_0049b050;
            break;
        }
        object++;
    }
    if (distance < 8000) {
        reason = "Already Near";
    } else if (kilrathi_near(0, 16000) != 0) {
        reason = "Enemy Near";
    } else if (g_pActiveHazardField_00493278 != 0) {
        reason = "Hazard Near";
    }
    if (showReason != 0 && reason != 0)
        CockpitMessage(reason, g_abGamePaletteReservedColours_0049cb54[4], 3);
    return reason == 0;
}

/* Function start: 0x438CEB */
void reset_cockpit(void)
{
    short light;

    for (light = 0; light < 7; light++) {
        g_abCockpitLightState_005d1e70[light] = 0;
        g_abCockpitLightGoal_005d1eb8[light] = 0;
    }
}

/* Function start: 0x438D30 */
void SetCockpitLightBlink(signed char light, short interval)
{
    if (interval < 20) {
        if (interval == 0) {
            g_abCockpitLightGoal_005d1eb8[(int)light] ^= 1;
        } else if (g_nSpaceFrame_00493134 % interval == 0) {
            g_abCockpitLightGoal_005d1eb8[(int)light] ^= 1;
        }
    } else {
        g_abCockpitLightGoal_005d1eb8[(int)light] = 0;
    }
}

/* Function start: 0x438DAD */
void draw_cockpit_lights(void)
{
    signed char light;
    short x;
    short frame;
    short y;

    if (g_nRenderedSpaceFrame_00493138 % 4 == 0) {
        if (auto_pilot_valid(0) != 0)
            g_abCockpitLightGoal_005d1eb8[4] = 1;
        else
            g_abCockpitLightGoal_005d1eb8[4] = 0;
    }
    for (light = 0; light < 7; light++) {
        if (g_nCockpitDisplayMode_0049d71c != 0) {
            x = g_asCockpitLightX_0049aea8[(int)light];
            y = g_asCockpitLightY_0049aeb8[(int)light];
            if (g_abCockpitLightGoal_005d1eb8[(int)light] == 1)
                frame = g_acCockpitLightOnFrame_0049aed0[(int)light];
            else
                frame = g_acCockpitLightOffFrame_0049aec8[(int)light];
            DrawSpriteDefault(&g_stScreenViewport_005d21a0, x, y,
                              g_pCockpitPanelShape_005d2cd4, frame);
            g_abCockpitLightState_005d1e70[(int)light] =
                g_abCockpitLightGoal_005d1eb8[(int)light];
        } else if (g_abCockpitLightState_005d1e70[(int)light] !=
                   g_abCockpitLightGoal_005d1eb8[(int)light]) {
            x = g_asCockpitLightX_0049aea8[(int)light];
            y = g_asCockpitLightY_0049aeb8[(int)light];
            if (g_abCockpitLightGoal_005d1eb8[(int)light] == 1)
                frame = g_acCockpitLightOnFrame_0049aed0[(int)light];
            else
                frame = g_acCockpitLightOffFrame_0049aec8[(int)light];
            DrawSpriteDefault(&g_stScreenViewport_005d21a0, x, y,
                              g_pCockpitPanelShape_005d2cd4, frame);
            g_abCockpitLightState_005d1e70[(int)light] =
                g_abCockpitLightGoal_005d1eb8[(int)light];
        }
    }
}

/* Function start: 0x438F62 */
void update_lights(void)
{
    short weaponEnergy;
    short fuelPercent;

    if (g_bFuelGaugeDamaged_0049b054 == 0) {
        fuelPercent = (short)(
            (g_anShipFuel_00495638[0] * 100) /
            *(int *)&g_aObjectTypeData_00496d30[0].lifetime);
        SetCockpitLightBlink(6, fuelPercent);
        vdu_polygon(0, fuelPercent);
    }
    weaponEnergy = g_asShipWeaponEnergy_00495590[0];
    vdu_polygon(1, weaponEnergy);
    if ((calculate_damage_level() >= 3 &&
         (int)g_aasShipShield_00495518[0][1] +
                 (int)g_aasShipShield_00495518[0][0] <
             10) ||
        g_bForceCriticalDamageWarning_0049b058 != 0) {
        SetCockpitLightBlink(3, 2);
        if (g_nCriticalDamageWarningSfxHandle_005d1ec0 == 0 ||
            g_nSpaceFrame_00493134 % 10 == 0)
            PlaySfxWaveFileByNumber(0x20, -1, 0);
    } else if (g_nCriticalDamageWarningSfxHandle_005d1ec0 != 0) {
        FlushSoundEffectsAndLog(
            g_nCriticalDamageWarningSfxHandle_005d1ec0, 1);
        g_nCriticalDamageWarningSfxHandle_005d1ec0 = 0;
        g_abCockpitLightGoal_005d1eb8[3] = 0;
    }
}

/* Function start: 0x439076 */
void update_bars(void)
{
    ObjectTypeData *typeData;
    short percentage;
    short shieldPercent;
    short displayedShield;

    typeData = &g_aObjectTypeData_00496d30[g_acObjectType_00493980[0]];
    percentage = (short)((g_aasShipArmor_00495540[0][0] * 100) /
                         typeData->armorFront);
    vdu_polygon(2, percentage);
    percentage = (short)((g_aasShipArmor_00495540[0][1] * 100) /
                         typeData->armorRear);
    vdu_polygon(3, percentage);
    percentage = (short)((g_aasShipArmor_00495540[0][2] * 100) /
                         typeData->armorRight);
    vdu_polygon(4, percentage);
    percentage = (short)((g_aasShipArmor_00495540[0][3] * 100) /
                         typeData->armorLeft);
    vdu_polygon(5, percentage);
    shieldPercent = (short)((g_aasShipShield_00495518[0][0] * 100) /
                            typeData->shieldFore);
    SetCockpitLightBlink(0, shieldPercent);
    vdu_polygon(6, shieldPercent);
    displayedShield = g_aasShipShield_00495518[0][0];
    if (g_asObjectType_00495298[0] == 0x33)
        displayedShield = (short)(displayedShield / 10);
    DrawCockpitReadout(
        4, _itoa((int)displayedShield,
                 g_szTextScratchBuffer_005d1c40, 10));
    shieldPercent = (short)((g_aasShipShield_00495518[0][1] * 100) /
                            typeData->shieldAft);
    SetCockpitLightBlink(1, shieldPercent);
    vdu_polygon(7, shieldPercent);
    displayedShield = g_aasShipShield_00495518[0][1];
    if (g_asObjectType_00495298[0] == 0x33)
        displayedShield = (short)(displayedShield / 10);
    DrawCockpitReadout(
        5, _itoa((int)displayedShield,
                 g_szTextScratchBuffer_005d1c40, 10));
}

/* Function start: 0x439264 */
short get_mode(short i)
{
    return (short)g_aaiVduModeStack_00493498[(int)i][
        (int)g_acVduModeStackDepth_004934c8[(int)i]];
}

/* Function start: 0x43928E */
void set_mode(short i, int state)
{
    if (get_mode(i) != state)
        ClearHudMessageSlot(&g_aHudMessageSlots_005d1d40[i]);
    g_acVduModeStackDepth_004934c8[(int)i] = 0;
    g_aaiVduModeStack_00493498[(int)i][
        (int)g_acVduModeStackDepth_004934c8[(int)i]] = state;
}

/* Function start: 0x4392F1 */
unsigned short SetVduModeIfChanged(short i, int state)
{
    short changed;

    changed = get_mode(i) != state;
    if (changed != 0)
        set_mode(i, state);
    return changed;
}

/* Function start: 0x43934D */
int GetVduModeStackDepth(short i)
{
    return g_acVduModeStackDepth_004934c8[(int)i];
}

/* Function start: 0x439369 */
void push_mode(short i, int state)
{
    ClearHudMessageSlot(&g_aHudMessageSlots_005d1d40[i]);
    g_acVduModeStackDepth_004934c8[(int)i]++;
    g_aaiVduModeStack_00493498[(int)i][
        (int)g_acVduModeStackDepth_004934c8[(int)i]] = state;
}

/* Function start: 0x4393B3 */
void pop_mode(short i)
{
    ClearHudMessageSlot(&g_aHudMessageSlots_005d1d40[i]);
    g_acVduModeStackDepth_004934c8[(int)i]--;
}

/* Function start: 0x4393E1 */
void set_new_vdu(short vdu)
{
    if (get_mode(vdu) == 0) {
        malf_noise(vdu, 1, g_abGamePaletteReservedColours_0049cb54[12], 0x17, 0);
    } else {
        switch (vdu) {
        case 1:
            ClearViewport(
                &g_stRightVduViewport_005d2b20,
                g_cSecondaryViewBufferColour_0049cb4c);
            break;
        case 0:
            ClearViewport(
                &g_stLeftVduViewport_005d2180,
                g_cSecondaryViewBufferColour_0049cb4c);
            break;
        }
    }
    g_anLastDrawnVduMode_004934d0[(int)vdu] = get_mode(vdu);
}

/* Function start: 0x4394A0 */
short update_vid_disp(short vdu)
{
    short changed;

    changed =
        get_mode(vdu) != g_anLastDrawnVduMode_004934d0[(int)vdu];
    if (changed != 0)
        set_new_vdu(vdu);
    return changed;
}

/* Function start: 0x439500 */
void InvalidateVduMode(short i)
{
    g_anLastDrawnVduMode_004934d0[(int)i] = 0;
}

/* Function start: 0x43951A */
void clear_message_time(void)
{
    g_nHudMessageTime_005d1c32 = 0;
}

/* Function start: 0x43952E */
short message_showing(void)
{
    return 0 < g_nHudMessageTime_005d1c32;
}

/* Function start: 0x439559 */
void set_message_time(short duration)
{
    g_nHudMessageTime_005d1c32 = duration;
    if (g_nHudMessageTime_005d1c32 == -1)
        g_bSpeechPlaybackComplete_004a266c = 0;
}

/* Function start: 0x439588 */
void check_message(void)
{
    if (message_showing() != 0) {
        if (g_nHudMessageTime_005d1c32 == -1) {
            if (g_bSpeechPlaybackComplete_004a266c != 0) {
                clear_message_time();
                if (g_aeSpecialManeuver_00495600[
                        g_nCommSpeakerObject_0049b794] ==
                        SPECIAL_MANEUVER_UNKNOWN_9 &&
                    g_pCommPortraitResource_0049b788 != 0) {
                    g_nCommDeathSequenceFrame_0049ae84 = 1;
                } else {
                    EndCommMenu();
                }
                if (g_bEjectionSequencePending_00493058 != 0) {
                    ejection_sequence(
                        g_aObjectTypeData_00496d30[
                            g_acObjectType_00493980[0]].resourceType,
                        1);
                    g_anShipFuel_00495638[0] =
                        *(int *)&g_aObjectTypeData_00496d30[
                            g_acObjectType_00493980[0]].lifetime;
                    g_bEjectionSequencePending_00493058 = 0;
                }
            }
        } else {
            g_nHudMessageTime_005d1c32--;
            if (g_nHudMessageTime_005d1c32 == 0) {
                EndCommMenu();
                if (g_bEjectionSequencePending_00493058 != 0) {
                    ejection_sequence(
                        g_aObjectTypeData_00496d30[
                            g_acObjectType_00493980[0]].resourceType,
                        1);
                    g_anShipFuel_00495638[0] =
                        *(int *)&g_aObjectTypeData_00496d30[
                            g_acObjectType_00493980[0]].lifetime;
                    g_bEjectionSequencePending_00493058 = 0;
                }
            }
        }
    }
}

/* Function start: 0x4396BF */
void update_digital_readouts(void)
{
    short speed;
    short velocity;

    SetTextContext(&g_stCockpitTextContext_005d2d00);
    speed = (short)(g_anShipSpeed_00494e20[0] >> 8);
    speed = (short)(speed * 10);
    DrawCockpitReadout(
        2, _itoa(speed, g_szTextScratchBuffer_005d1c40, 10));
    velocity = (short)(MultiplyFixed(
        Vector_magnitude(&g_aShipVelocity_00494898[0]), 0xa00) >> 8);
    DrawCockpitReadout(
        3, _itoa(velocity, g_szTextScratchBuffer_005d1c40, 10));
}

/* Function start: WC2_UNMAPPED */
void PlayTargetLockSfx(void)
{
    PlaySfxWaveFileByNumber(0x18, -1, 0);
}

/* Function start: 0x439753 */
void malf_sound(void)
{
    LogUnknownSoundEffect(
        g_abSoundEffectDefinitions_0049bf18 + 0xf0,
        0, 0x7f, 0x64, 1, 1);
    PlaySfxWaveFileByNumber(0x1f, -1, 0);
}

/* Function start: 0x439789 */
short malf(char component)
{
    return g_acPlayerComponentDamage_00493470[(int)component] *
               g_acPlayerComponentDamage_00493470[(int)component] >
           (unsigned short)RandomInRange(0, 15);
}

/* Function start: 0x4397D5 */
void vdu_malf(short vdu, short sound)
{
    if (g_nCurrentView_00492fa8 == 0 &&
        ((vdu == 0 && g_stLeftVduViewport_005d2180.top > 0) ||
         (vdu == 1 && g_stRightVduViewport_005d2b20.top > 0))) {
        malf_noise(vdu, 1, g_abGamePaletteReservedColours_0049cb54[12], sound, 0);
        set_mode(vdu, 0);
    }
}

/* Function start: 0x43984F */
void ShowComponentHitHudMessage(const char *text, unsigned short colour,
                                short flashCount)
{
    if (get_mode(0) != 0) {
        if (g_aHudMessageSlots_005d1d40[0].text != 0)
            ClearHudMessageSlot(&g_aHudMessageSlots_005d1d40[0]);
        DosStrcpy(g_szComponentHitMessage_005d1da0, text);
        SetHudMessageSlot(&g_aHudMessageSlots_005d1d40[0], &g_stLeftVduTextContext_005d2ae0,
                          g_stLeftVduViewport_005d2180.left,
                          (short)(g_stLeftVduViewport_005d2180.bottom - 6),
                          g_szComponentHitMessage_005d1da0,
                          colour, (signed char)flashCount);
    }
}

/* Function start: 0x4398CB */
signed char damage_your_component(char component, char amount,
                                  char maximum)
{
    char text[40];

    g_acPlayerComponentDamage_00493470[(int)component] =
        (signed char)MinShort(
        (short)(g_acPlayerComponentDamage_00493470[(int)component] + amount),
        (short)maximum);
    if (malf(component) != 0 && component == 3) {
        vdu_malf(0, 0x18);
        vdu_malf(1, 0x18);
    }
    if (get_mode(0) == 2 || get_mode(0) == 1) {
        sprintf(text, g_szComponentHitFormat_0049b35c,
                g_apszComponentNames_00490090[(int)component]);
        ShowComponentHitHudMessage(text, g_abGamePaletteReservedColours_0049cb54[8], 5);
    }
    return g_acPlayerComponentDamage_00493470[(int)component];
}

/* Function start: 0x4399C6 */
void RemovePlayerReleaseWeapon(signed char weapon)
{
    lock_off();
    g_nReleaseWeaponDisplayType_005d1c28 =
        ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[weapon].type;
    g_cReleaseWeaponDisplayFrame_0049b060 = 1;
    g_nReleaseWeaponDisplayX_005d1c24 =
        (short)(g_aWeaponDisplayPositions_005d1de0[weapon].x +
                g_nWeaponDisplayOriginX_005d4254);
    g_nReleaseWeaponDisplayY_005d1c26 =
        (short)(g_aWeaponDisplayPositions_005d1de0[weapon].y +
                g_nWeaponDisplayOriginY_005d4256);
    g_cReleaseWeaponDisplayTicks_0049b064 = 3;
    g_cReleaseWeaponDisplayState_0049b068 = 0;
    if (g_pReleaseWeaponDisplayBackground_0049b05c == 0)
        g_cReleaseWeaponDisplayFrame_0049b060 = -1;
    remove_weapon(0, weapon);
}

/* Function start: 0x439A5A */
void fire_computer_graphic_missile(void)
{
    short visible;

    if (g_cReleaseWeaponDisplayFrame_0049b060 == -1)
        return;
    visible = g_nCurrentView_00492fa8 == 0 && get_mode(0) == 1;
    if (g_cReleaseWeaponDisplayState_0049b068 != 0) {
        g_cReleaseWeaponDisplayState_0049b068 = 0;
        RestoreSpriteBackground(
            &g_stLeftVduViewport_005d2180,
            g_pReleaseWeaponDisplayBackground_0049b05c,
            g_nReleaseWeaponDisplayX_005d1c24,
            g_nReleaseWeaponDisplayY_005d1c26,
            g_pCockpitWeaponShape_005d2b54,
            g_cReleaseWeaponDisplayFrame_0049b060);
    }
    if (g_stLeftVduViewport_005d2180.top - 10 <
            g_nReleaseWeaponDisplayY_005d1c26 &&
        g_nReleaseWeaponDisplayY_005d1c26 <
            g_stLeftVduViewport_005d2180.bottom) {
        g_nReleaseWeaponDisplayY_005d1c26 =
            (short)(g_nReleaseWeaponDisplayY_005d1c26 -
                    g_cReleaseWeaponDisplayTicks_0049b064);
        g_cReleaseWeaponDisplayTicks_0049b064++;
        if (visible != 0) {
            CaptureSpriteBackground(
                &g_stLeftVduViewport_005d2180,
                g_pReleaseWeaponDisplayBackground_0049b05c,
                g_nReleaseWeaponDisplayX_005d1c24,
                g_nReleaseWeaponDisplayY_005d1c26,
                g_pCockpitWeaponShape_005d2b54,
                g_cReleaseWeaponDisplayFrame_0049b060);
            DrawSpriteDefault(
                &g_stLeftVduViewport_005d2180,
                g_nReleaseWeaponDisplayX_005d1c24,
                g_nReleaseWeaponDisplayY_005d1c26,
                g_pCockpitWeaponShape_005d2b54,
                g_cReleaseWeaponDisplayFrame_0049b060);
        }
        g_cReleaseWeaponDisplayState_0049b068 = (signed char)visible;
    } else {
        g_cReleaseWeaponDisplayFrame_0049b060 = -1;
    }
}

/* Function start: 0x439BB7 */
void show_weapon_disp(void)
{
    unsigned char *weaponData;
    ShipWeaponSlot *weapon;
    const char *releaseName;
    const char *gunName;
    int selectedGunType;
    short frame;
    short x;
    short y;
    signed char count;

    weaponData = g_aShipWeapons_004956b0[0];
    weapon = (ShipWeaponSlot *)(
        weaponData + g_nSelectedReleaseWeaponIndex_004934e0 *
                         sizeof(ShipWeaponSlot) +
        1);
    set_new_vdu(0);
    DrawTextAt(&g_stLeftVduTextContext_005d2ae0,
               g_stLeftVduViewport_005d2180.left,
               g_stLeftVduViewport_005d2180.top, "WEAPON DISPLAY", 2);
    DrawViewportLine(&g_stLeftVduViewport_005d2180,
                     (short)(g_stLeftVduViewport_005d2180.left + 2),
                     (short)(g_stLeftVduViewport_005d2180.top + 5),
                     (short)(g_stLeftVduViewport_005d2180.right - 2),
                     (short)(g_stLeftVduViewport_005d2180.top + 5),
                     g_ucPrimaryTextColour_0049cb64);

    switch (g_nSelectedReleaseWeaponIndex_004934e0) {
    case -1:
        releaseName = "";
        break;
    default:
        releaseName = g_aObjectTypeData_00496d30[
            (int)weapon->weaponType].displayName;
        break;
    }
    selectedGunType = g_nSelectedGunType_004934dc;
    switch (selectedGunType) {
    case -1:
        gunName = "";
        break;
    case 0x80:
        gunName = "Full Guns";
        break;
    default:
        gunName = g_aObjectTypeData_00496d30[
            selectedGunType].displayName;
        break;
    }
    DrawFormattedText("\nWeapon: %s", releaseName);
    DrawFormattedText("\nGun: %s", gunName);

    g_nWeaponDisplayOriginX_005d4254 =
        (short)(g_stLeftVduViewport_005d2180.left +
                g_nWeaponDisplayOffsetX_0049ae8c);
    g_nWeaponDisplayOriginY_005d4256 =
        (short)(g_nWeaponDisplayOffsetY_0049ae8e_WC1_UNMAPPED +
                g_stLeftVduViewport_005d2180.top);
    DrawSpriteDefault(
        &g_stLeftVduViewport_005d2180,
        g_nWeaponDisplayOriginX_005d4254,
        g_nWeaponDisplayOriginY_005d4256,
        g_pCockpitWeaponShape_005d2b54, 0);

    weapon = (ShipWeaponSlot *)(weaponData + 1);
    count = 0;
    while (count < (signed char)weaponData[0]) {
        if (g_aObjectTypeData_00496d30[
                (int)weapon->weaponType].field_18 == 0x0b) {
            frame = 0x1a;
            if (g_aWeaponDisplayPositions_005d1de0[
                    (int)count].x > 0x29)
                frame = (short)(frame + 2);
            if (g_aWeaponDisplayPositions_005d1de0[
                    (int)count].x < 0x1f)
                frame++;
        } else if (g_aObjectTypeData_00496d30[
                       (int)weapon->weaponType].objectClass == 8) {
            frame = (short)(
                weapon->disabled + weapon->type * 2 - 0x0b);
        } else {
            frame = (short)(
                2 -
                (((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[
                     g_nSelectedReleaseWeaponIndex_004934e0].type ==
                 weapon->type));
        }

        x = g_aWeaponDisplayPositions_005d1de0[(int)count].x;
        y = g_aWeaponDisplayPositions_005d1de0[(int)count].y;
        if (g_aObjectTypeData_00496d30[
                (int)weapon->weaponType].field_18 != 0x14) {
            DrawSpriteDefault(
                &g_stLeftVduViewport_005d2180,
                (short)(g_nWeaponDisplayOriginX_005d4254 + x),
                (short)(g_nWeaponDisplayOriginY_005d4256 + y),
                g_pCockpitWeaponShape_005d2b54, frame);
        }
        if ((int)count == g_nSelectedReleaseWeaponIndex_004934e0 &&
            g_aObjectTypeData_00496d30[
                (int)weapon->weaponType].field_18 == 0x14) {
            DrawSpriteDefault(
                &g_stLeftVduViewport_005d2180,
                (short)(g_nWeaponDisplayOriginX_005d4254 + 10),
                (short)(g_nWeaponDisplayOriginY_005d4256 + 13),
                g_pCockpitWeaponShape_005d2b54,
                (short)(weapon->type + 4));
            DrawSpriteDefault(
                &g_stLeftVduViewport_005d2180,
                (short)(g_nWeaponDisplayOriginX_005d4254 + 60),
                (short)(g_nWeaponDisplayOriginY_005d4256 + 13),
                g_pCockpitWeaponShape_005d2b54,
                (short)(weapon->type + 4));
        }
        count++;
        weapon++;
    }
}

/* Function start: 0x439F5F */
void update_status_text(void)
{
    short x;
    short index;
    short y;
    short frame;

    for (index = 0;
         index < (signed char)g_aShipWeapons_004956b0[0][0];
         index++) {
        if (((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[
                index].type == 11) {
            g_nWeaponDisplayOriginX_005d4254 =
                (short)(g_stLeftVduViewport_005d2180.left +
                        g_nWeaponDisplayOffsetX_0049ae8c);
            g_nWeaponDisplayOriginY_005d4256 =
                (short)(g_nWeaponDisplayOffsetY_0049ae8e_WC1_UNMAPPED +
                        g_stLeftVduViewport_005d2180.top);
            x = g_aWeaponDisplayPositions_005d1de0[index].x;
            y = g_aWeaponDisplayPositions_005d1de0[index].y;

            if (g_aHudMessageSlots_005d1d40[0].text == 0 &&
                x < 0x29 && x > 0x1f) {
                if (g_asGunCooldown_005c8d70[0] > 0) {
                    frame = 0x1d;
                    DrawSpriteDefault(
                        &g_stLeftVduViewport_005d2180,
                        (short)(g_nWeaponDisplayOriginX_005d4254 + x),
                        (short)(g_nWeaponDisplayOriginY_005d4256 + y),
                        g_pCockpitWeaponShape_005d2b54, frame);
                } else {
                    frame = 0x1a;
                    DrawSpriteDefault(
                        &g_stLeftVduViewport_005d2180,
                        (short)(g_nWeaponDisplayOriginX_005d4254 + x),
                        (short)(g_nWeaponDisplayOriginY_005d4256 + y),
                        g_pCockpitWeaponShape_005d2b54, frame);
                }
            }

            if (g_cReleaseWeaponDisplayFrame_0049b060 == -1 && x < 0x1f) {
                if (g_asGunCooldown_005c8d70[1] > 0) {
                    frame = 0x1e;
                    DrawSpriteDefault(
                        &g_stLeftVduViewport_005d2180,
                        (short)(g_nWeaponDisplayOriginX_005d4254 + x),
                        (short)(g_nWeaponDisplayOriginY_005d4256 + y),
                        g_pCockpitWeaponShape_005d2b54, frame);
                } else {
                    frame = 0x1b;
                    DrawSpriteDefault(
                        &g_stLeftVduViewport_005d2180,
                        (short)(g_nWeaponDisplayOriginX_005d4254 + x),
                        (short)(g_nWeaponDisplayOriginY_005d4256 + y),
                        g_pCockpitWeaponShape_005d2b54, frame);
                }
            }

            if (g_cReleaseWeaponDisplayFrame_0049b060 == -1 && x > 0x29) {
                if (g_asGunCooldown_005c8d70[2] > 0) {
                    frame = 0x1f;
                    DrawSpriteDefault(
                        &g_stLeftVduViewport_005d2180,
                        (short)(g_nWeaponDisplayOriginX_005d4254 + x),
                        (short)(g_nWeaponDisplayOriginY_005d4256 + y),
                        g_pCockpitWeaponShape_005d2b54, frame);
                } else {
                    frame = 0x1c;
                    DrawSpriteDefault(
                        &g_stLeftVduViewport_005d2180,
                        (short)(g_nWeaponDisplayOriginX_005d4254 + x),
                        (short)(g_nWeaponDisplayOriginY_005d4256 + y),
                        g_pCockpitWeaponShape_005d2b54, frame);
                }
            }
        }
    }
}

/* Function start: 0x43A1EB */
short sighted(short objective)
{
    return (g_aMissionObjectives_004932a8[objective].flags & 4) == 4;
}

/* Function start: 0x43A21E */
short visited(short objective)
{
    return (g_aMissionObjectives_004932a8[objective].flags & 1) == 1;
}

/* Function start: 0x43A251 */
short achieved(short objective)
{
    return (g_aMissionObjectives_004932a8[objective].flags & 2) == 2 ||
           g_aMissionObjectives_004932a8[objective].type == -1;
}

/* Function start: 0x43A29E */
void flag_objective(short objective, unsigned char flags)
{
    g_aMissionObjectives_004932a8[objective].flags |= flags;
}

/* Function start: 0x43A2CD */
void DrawCalculatingLabel(void)
{
    if (IsMissionObjectiveOutOfSystem(
            (short)g_cCurrentObjective_004931cc) != 0) {
        DrawCockpitReadout(0, "Out-system");
    } else {
        if (g_nCurrentObjectiveRange_004931c8 < 1) {
            DrawCockpitReadout(0, "CALCULATING");
        } else {
            strcpy(g_szTextScratchBuffer_005d1c40 +
                       strlen(_ltoa((long)g_nCurrentObjectiveRange_004931c8,
                                    g_szTextScratchBuffer_005d1c40, 10)),
                   " km");
            DrawCockpitReadout(0, g_szTextScratchBuffer_005d1c40);
        }
        g_nDisplayedObjectiveRange_0049b078 =
            g_nCurrentObjectiveRange_004931c8;
    }
}

/* Function start: 0x43A374 */
const char *objective_name(short objective)
{
    if (objective >= (short)g_cMissionObjectiveCount_00493294)
        return g_szNoObjective_0049b3b0;
    if (*g_aMissionObjectives_004932a8[objective].name == '?' &&
        sighted(objective) == 0)
        return g_szUnknownObjective_0049b3b8;
    return g_aMissionObjectives_004932a8[objective].displayName;
}

/* Function start: 0x43A3F8 */
void show_navigation_disp(void)
{
    DrawTextAt(&g_stRightVduTextContext_005d2ce0, g_stRightVduViewport_005d2b20.left, g_stRightVduViewport_005d2b20.top,
               g_szCompNavigation_0049b3c0, 2);
    DrawFormattedText(g_szDestinationFormat_0049b3d0,
                      objective_name((short)g_cCurrentObjective_004931cc));
    DrawFormattedText(g_szNavigationRangeLabel_0049b3e4);
    InitializeCockpitReadout(0, &g_stRightVduTextContext_005d2ce0);
    DrawFormattedText(g_szNewObjectivePrompt_0049b3f0);
    DrawCalculatingLabel();
}

/* Function start: 0x43A474 */
short hidden_objective(short objective)
{
    short nameHidden;
    short hidden;
    short ship;

    nameHidden =
        *g_aMissionObjectives_004932a8[objective].displayName == '.' ||
        *g_aMissionObjectives_004932a8[objective].name == '.';
    if (nameHidden != 0 ||
        (mobile_objective(objective) != 0 &&
         g_aMissionShips_00492290[
             (signed char)g_aMissionObjectives_004932a8[
                 objective].index].state != 0))
        hidden = 1;
    else
        hidden = 0;
    if (hidden == 0 && mobile_objective(objective) != 0 &&
        g_aMissionObjectives_004932a8[
            g_cMissionObjectiveCount_00493294].type == 0) {
        ship = find_ship_index(
            (short)g_aMissionObjectives_004932a8[objective].index);
        if (g_aMissionShips_00492290[
                (signed char)g_aMissionObjectives_004932a8[
                    objective].index].missionType ==
                MISSION_TYPE_WARP_ARRIVE &&
            ship != -1)
            hidden = 1;
    }
    return hidden;
}

/* Function start: 0x43A75E */
short set_new_objective(short pathIndex)
{
    if (pathIndex < 0) {
        pathIndex = -1;
        do {
            pathIndex++;
        } while (g_abFlightPath_004932a0[pathIndex] != -1 &&
                 pathIndex < (short)g_cMissionObjectiveCount_00493294);
        pathIndex--;
    }
    if (pathIndex > (short)g_cMissionObjectiveCount_00493294)
        pathIndex = 0;
    if (g_abFlightPath_004932a0[pathIndex] == -1)
        pathIndex = 0;
    g_cCurrentNavPointIndex_00493298 = (signed char)pathIndex;
    if (hidden_objective(
            (short)g_abFlightPath_004932a0[pathIndex]) != 0)
        return 0;
    g_cCurrentObjective_004931cc =
        g_abFlightPath_004932a0[g_cCurrentNavPointIndex_00493298];
    g_aeShipObjective_00495f08[0] =
        (enum ShipObjective)g_aMissionObjectives_004932a8[
            (short)g_cCurrentObjective_004931cc].type;
    set_objective_range(0);
    return 1;
}

/* Function start: 0x43A851 */
short cycle_next_objective(void)
{
    short wraps;

    wraps = 0;
    while (wraps < 3) {
        if (set_new_objective(
                (short)((short)g_cCurrentNavPointIndex_00493298 + 1)) != 0)
            break;
        if (g_cCurrentNavPointIndex_00493298 == 0)
            wraps++;
    }
    if (wraps >= 3) {
        g_cCurrentNavPointIndex_00493298 = 0;
        g_cCurrentObjective_004931cc = g_abFlightPath_004932a0[
            g_cCurrentNavPointIndex_00493298];
    }
    return wraps < 3;
}

/* Function start: 0x43A8EA */
void set_next_destination(void)
{
    set_new_objective(0);
    do {
        if (set_new_objective(
                (short)g_cCurrentNavPointIndex_00493298) != 0 &&
            visited((short)g_abFlightPath_004932a0[
                g_cCurrentNavPointIndex_00493298]) == 0)
            break;
        g_cCurrentNavPointIndex_00493298++;
    } while (g_cCurrentNavPointIndex_00493298 <
                 g_cMissionObjectiveCount_00493294 &&
             g_abFlightPath_004932a0[
                 g_cCurrentNavPointIndex_00493298] != -1);
    if (g_cCurrentNavPointIndex_00493298 >=
            g_cMissionObjectiveCount_00493294 ||
        g_abFlightPath_004932a0[
            g_cCurrentNavPointIndex_00493298] == -1) {
        set_new_objective(0);
        cycle_next_objective();
    }
    if (get_mode(1) == 5)
        InvalidateVduMode(1);
}

/* Function start: 0x43A9D7 */
short LocateMobileObjective(short objective)
{
    short ship = -1;

    if (mobile_objective(objective) != 0) {
        ship = locate_ship(
            g_aMissionObjectives_004932a8[objective].index,
            &g_aMissionObjectives_004932a8[objective].position);
    }
    return ship;
}

/* Function start: 0x43AA3F */
short is_any_ship_returning_to_current_nav_point(void)
{
    short ship;

    for (ship = 0; ship < 10; ship++) {
        if (g_aeObjectClass_00495328[ship] >= OBJECT_CLASS_SHIP &&
            g_asShipMissionType_00495de8[ship] == MISSION_TYPE_COME_HOME &&
            g_abShipNavPointIndex_00495f60[ship] <= g_cCurrentNavPointIndex_00493298)
            return 1;
    }
    return 0;
}

/* Function start: 0x43AAC3 */
unsigned int escorting_a_ship(void)
{
    if (g_asShipMissionType_00495de8[0] != MISSION_TYPE_ESCORT) {
        if (is_any_ship_returning_to_current_nav_point() == 0)
            return 0;
    }
    return 1;
}

/* Function start: 0x43AAFF */
void flag_reached(short objective, short reached)
{
    short carrierMissionShip;
    short carrierObject;
    short objectiveType;
    short advanceDestination;
    short markVisited;

    carrierMissionShip = g_asShipMissionParameter_00495e00[0];
    objectiveType = (short)g_aMissionObjectives_004932a8[objective].type;
    carrierObject = find_ship_index(carrierMissionShip);
    markVisited = objective != g_cCurrentObjective_004931cc;
    advanceDestination = 0;
    if (objective == g_cCurrentObjective_004931cc) {
        if (reached == 0 && escorting_a_ship() != 0 &&
            carrierObject != -1 &&
            g_aMissionObjectives_004932a8[objective].index !=
                g_asShipMissionParameter_00495e00[0]) {
            if (objectiveType != 1 ||
                g_aMissionShips_00492290[carrierMissionShip].state != 1) {
                sprintf(g_pszAutopilotWaitReason_0049b050,
                        g_szWaitForFormat_0049b428,
                        g_aObjectTypeData_00496d30[
                            g_acObjectType_00493980[carrierObject]].
                                displayName);
                CockpitMessage(g_pszAutopilotWaitReason_0049b050,
                               g_abGamePaletteReservedColours_0049cb54[4], 4);
            }
        } else {
            advanceDestination = 1;
            CockpitMessage(
                visited(objective) != 0
                    ? (char *)g_szAlreadyVisited_0049b404
                    : (char *)g_szObjectiveReached_0049b414,
                g_abGamePaletteReservedColours_0049cb54[4], 4);
            markVisited = advanceDestination;
        }
    }
    if (objectiveType != 1 && markVisited != 0) {
        if (visited(objective) == 0 && carrierObject != -1 &&
            g_aMissionObjectives_004932a8[objective].index ==
                g_asShipMissionParameter_00495e00[0] &&
            g_acObjectType_00493980[carrierObject] !=
                OBJECT_TYPE_TIGERS_CLAW)
            send_message(carrierObject, 6);
        flag_objective(objective, 1);
    }
    if (advanceDestination != 0)
        set_next_destination();
}

/* Function start: 0x43AD61 */
void check_sighting(short objective, short range, short object)
{
    if (sighted(objective) == 0 && range < 16000 &&
        (object == -1 ||
         g_asObjectScreenX_00493598[object] != (short)0x8001))
        flag_objective(objective, 4);
}

/* Function start: 0x43ADC4 */
void check_visit(short objective, short range)
{
    int reachedRange;

    if ((short)g_aMissionObjectives_004932a8[objective].type == 3 ||
        (short)g_aMissionObjectives_004932a8[objective].type == 4)
        reachedRange = 6000;
    else
        reachedRange = 1500;
    if (range < reachedRange)
        flag_reached(objective, 1);
}

/* Function start: 0x43AE2F */
void update_objective_location(short objective)
{
    FixedVector delta;
    short object;
    short range;

    object = LocateMobileObjective(objective);
    if (sighted(objective) != 0 && visited(objective) != 0 &&
        g_cCurrentObjective_004931cc != objective)
        return;
    ComputeVectorDelta(&g_aShipPosition_00494550[0],
                       &g_aMissionObjectives_004932a8[objective].position,
                       &delta);
    range = FixedToShortSaturating(
        Vector_magnitude(&delta));
    check_sighting(objective, range, object);
    if (mobile_objective(objective) != 0) {
        if (object != -1)
            check_visit(objective, range);
    } else if (g_aMissionNavPoints_00491e98[
                   g_aMissionObjectives_004932a8[objective].index].type >= 1) {
        check_visit(objective, range);
    }
}

/* Function start: 0x43AFD3 */
short objective_lost(short objective)
{
    short state;

    state = g_aMissionShips_00492290[
        g_aMissionObjectives_004932a8[objective].index].state;
    switch (g_aMissionObjectives_004932a8[objective].type) {
        case 2:
        case 3:
            return state != 0;
        case 4:
            return state == 3;
        default:
            return 0;
    }
}

/* Function start: 0x43B089 */
void check_objectives(void)
{
    if (objective_lost((short)g_cCurrentObjective_004931cc) != 0) {
        cycle_next_objective();
        InvalidateVduMode(1);
    } else {
        update_objective_location((short)g_cCurrentObjective_004931cc);
    }
    if (g_nDisplayedObjectiveRange_0049b078 !=
        g_nCurrentObjectiveRange_004931c8)
        DrawCalculatingLabel();
}

/* Function start: 0x43B0EB */
void rotational_pos_to_scanner_pos(signed char object,
                                   const SphericalVector *position,
                                   CockpitScannerGeometry geometry)
{
    if (abs((int)position->yaw) < 45)
        g_nScannerCursorX_005d1e6c =
            (short)(geometry.centerX + position->yaw / 4);
    else
        g_nScannerCursorX_005d1e6c =
            (short)(geometry.centerX + position->yaw / 6);

    g_nScannerCursorY_005d1e6e =
        (short)(geometry.centerY - position->pitch / 3);
    g_nScannerCursorX_005d1e6c =
        MinShort(geometry.maximumX, g_nScannerCursorX_005d1e6c);
    g_nScannerCursorX_005d1e6c =
        MaxShort(geometry.minimumX, g_nScannerCursorX_005d1e6c);
    g_nScannerCursorY_005d1e6e =
        MinShort(geometry.maximumY, g_nScannerCursorY_005d1e6e);
    g_nScannerCursorY_005d1e6e =
        MaxShort(geometry.minimumY, g_nScannerCursorY_005d1e6e);
    g_asScannerObjectX_005d1ea0[(int)object] =
        g_nScannerCursorX_005d1e6c;
    g_asScannerObjectY_005d1e80[(int)object] =
        g_nScannerCursorY_005d1e6e;
}

/* Function start: 0x43B1F0 */
short mobile_objective(short objective)
{
    int type = g_aMissionObjectives_004932a8[objective].type;

    if (type == 1 || type == 3 || type == 4 || type == 2)
        return 1;
    return 0;
}

/* Function start: 0x43B258 */
void ResetScannerContacts(void)
{
    short i = 10;

    do {
        g_asScannerObjectX_005d1ea0[i] = 0;
        i = i - 1;
    } while (i > 0);
    g_nScannerTargetObject_0049b07c = -1;
}

/* Function start: 0x43B29D */
void clear_head_up_display(void)
{
    short object;

    if (g_nCurrentView_00492fa8 == 4)
        RestoreTargetCameraAttitudeIndicators();
    if (g_nScannerTargetObject_0049b07c != -1) {
        object = g_nScannerTargetObject_0049b07c;
        DrawViewportPixel(&g_stScreenViewport_005d21a0,
                          g_asScannerObjectX_005d1ea0[object],
                          g_asScannerObjectY_005d1e80[object],
                          g_asScannerTargetBackground_005d1d80[0]);
        DrawViewportPixel(&g_stScreenViewport_005d21a0,
                          (short)(g_asScannerObjectX_005d1ea0[object] + 1),
                          g_asScannerObjectY_005d1e80[object],
                          g_asScannerTargetBackground_005d1d80[1]);
        DrawViewportPixel(&g_stScreenViewport_005d21a0,
                          (short)(g_asScannerObjectX_005d1ea0[object] - 1),
                          g_asScannerObjectY_005d1e80[object],
                          g_asScannerTargetBackground_005d1d80[2]);
        DrawViewportPixel(&g_stScreenViewport_005d21a0,
                          g_asScannerObjectX_005d1ea0[object],
                          (short)(g_asScannerObjectY_005d1e80[object] + 1),
                          g_asScannerTargetBackground_005d1d80[3]);
        DrawViewportPixel(&g_stScreenViewport_005d21a0,
                          g_asScannerObjectX_005d1ea0[object],
                          (short)(g_asScannerObjectY_005d1e80[object] - 1),
                          g_asScannerTargetBackground_005d1d80[4]);
        g_asScannerObjectX_005d1ea0[
            g_nScannerTargetObject_0049b07c] = 0;
    }
    if (g_asScannerObjectX_005d1ea0[10] != 0 &&
        g_nCurrentView_00492fa8 != 4 &&
        IsMissionObjectiveOutOfSystem(
            (short)g_cCurrentObjective_004931cc) == 0) {
        RestoreSpriteBackground(&g_stScreenViewport_005d21a0,
                                g_pScannerMarkerBackground_005d1c2c,
                                g_asScannerObjectX_005d1ea0[10],
                                g_asScannerObjectY_005d1e80[10],
                                g_pCockpitHudShape_005d21f4, 2);
        g_asScannerObjectX_005d1ea0[10] = 0;
    }
    object = 9;
    do {
        if (g_asScannerObjectX_005d1ea0[object] != 0) {
            DrawViewportPixel(&g_stScreenViewport_005d21a0,
                              g_asScannerObjectX_005d1ea0[object],
                              g_asScannerObjectY_005d1e80[object],
                              g_asScannerBackgroundColour_005d1ed0[object]);
            g_asScannerObjectX_005d1ea0[object] = 0;
        }
        object--;
    } while (object != 0);
    g_nScannerTargetObject_0049b07c = -1;
}

/* Function start: 0x43B4CF */
void set_objective_range(short showOnScanner)
{
    FixedVector relative;
    FixedVector rotated;
    SphericalVector spherical;

    LocateMobileObjective((short)g_cCurrentObjective_004931cc);
    ComputeVectorDelta(&g_aShipPosition_00494550[0],
                       &g_aMissionObjectives_004932a8[
                           (short)g_cCurrentObjective_004931cc].position,
                       &relative);
    transform_to_objects_frame(&relative, &rotated, 0);
    rectangular_to_spherical(&rotated, &spherical);
    if (showOnScanner != 0)
        rotational_pos_to_scanner_pos(
            10, &spherical, g_stCockpitScannerGeometry_0049af78);
    g_nCurrentObjectiveRange_004931c8 = spherical.radius >> 8;
}

/* Function start: 0x43B570 */
void AdjustScannerContactColourForRange(
    int range, short *colour)
{
    short increment;

    increment = 2;
    if (g_bRewritePacketExtensions_0049cb48 == 1)
        return;
    if (*colour == g_bPrimaryViewBufferColour_0049cb50)
        *colour = 0xa0;
    if (*colour == g_abGamePaletteReservedColours_0049cb54[4])
        increment = 1;
    if (range < 0x138800)
        return;
    if (range < 0x61a800)
        *colour = (short)(*colour + increment);
    else
        *colour = (short)(*colour + increment * 2);
}

/* Function start: 0x43B61F */
short get_color(short object, unsigned short *colour)
{
    short objectClass;

    objectClass = g_aeObjectClass_00495328[object];
    if (objectClass != OBJECT_CLASS_NULL &&
        (g_asObjectType_00495298[object] == 0x3d ||
         g_asObjectType_00495298[object] == 0x2c)) {
        *colour = g_abGamePaletteReservedColours_0049cb54[4];
        return 1;
    }
    if (objectClass < OBJECT_CLASS_MISSILE)
        return 0;
    if (objectClass == OBJECT_CLASS_SHIP) {
        if (g_asShipSide_004955d0[object] == SIDE_KILRATHI)
            *colour = g_abGamePaletteReservedColours_0049cb54[8];
        else if (g_asShipSide_004955d0[object] == SIDE_IMPERIAL)
            *colour = g_abGamePaletteReservedColours_0049cb54[0];
        else
            *colour = g_ucPrimaryTextColour_0049cb64;
        return 1;
    }
    if (objectClass >= OBJECT_CLASS_CAPITAL_SHIP) {
        if (g_asShipSide_004955d0[object] == SIDE_KILRATHI)
            *colour = g_ucHostileCapitalScannerColour_0049cb68;
        else if (g_asShipMissionIndex_00495d00[object] ==
                 g_nHomeMissionShipIndex_005d1e22)
            *colour = g_bPrimaryViewBufferColour_0049cb50;
        else
            *colour = g_ucHomeCarrierScannerColour_0049cb70;
    } else {
        if (g_acShipTarget_00495f20[object] != 0)
            return 0;
        *colour = g_abGamePaletteReservedColours_0049cb54[4];
    }
    return 1;
}

/* Function start: 0x43B7C0 */
void draw_3d_scanner(void)
{
    int gridValue;
    int gridRow;
    int gridIndex;
    CockpitScannerGeometry scanner;
    short object;
    int gridColour;
    FixedVector relative;
    FixedVector rotated;
    const int *grid;
    unsigned int colour;
    SphericalVector spherical;

    if (g_nCurrentView_00492fa8 == 0) {
        scanner = g_stCockpitScannerGeometry_0049af78;
        grid = g_aiForwardScannerGridRows_0049b080;
        gridColour = g_ucPrimaryTextColour_0049cb64;
    } else if (g_nCurrentView_00492fa8 == 4) {
        scanner = ((CockpitScannerGeometry *)(
            g_abGunDisplayConfiguration_0049d340 + 0x40))[
                g_nGunDisplayIndex_005c8dc0];
        grid = g_aiTargetCameraScannerGridRows_0049b1b8;
        gridColour = g_abGamePaletteReservedColours_0049cb54[0];
    }

    if (g_nCockpitDisplayMode_0049d71c != 0) {
        gridRow = 0;
        gridIndex = 0;
        while (grid[gridIndex] != -2) {
            if (grid[gridIndex] == -1) {
                gridRow++;
            } else {
                gridValue = grid[gridIndex];
                DrawViewportPixel(
                    &g_stScreenViewport_005d21a0,
                    (short)(scanner.centerX + gridRow),
                    (short)(scanner.centerY + gridValue),
                    (short)gridColour);
                if (gridValue != 0)
                    DrawViewportPixel(
                        &g_stScreenViewport_005d21a0,
                        (short)(scanner.centerX + gridRow),
                        (short)(scanner.centerY - gridValue),
                        (short)gridColour);
                if (gridRow != 0) {
                    DrawViewportPixel(
                        &g_stScreenViewport_005d21a0,
                        (short)(scanner.centerX - gridRow),
                        (short)(scanner.centerY + gridValue),
                        (short)gridColour);
                    if (gridValue != 0)
                        DrawViewportPixel(
                            &g_stScreenViewport_005d21a0,
                            (short)(scanner.centerX - gridRow),
                            (short)(scanner.centerY - gridValue),
                            (short)gridColour);
                }
            }
            gridIndex++;
        }
    }

    clear_head_up_display();
    g_nScannerTargetObject_0049b07c = g_acShipTarget_00495f20[0];
    if ((g_nScannerTargetObject_0049b07c != -1 &&
         g_aeObjectClass_00495328[g_nScannerTargetObject_0049b07c] <
             OBJECT_CLASS_SHIP) ||
        g_anShipCloakState_00496020[
            g_nScannerTargetObject_0049b07c] == 1)
        g_nScannerTargetObject_0049b07c = -1;

    object = 1;
    while (object <= 9) {
        if (g_anShipCloakState_00496020[object] != 1 &&
            get_color(object, (unsigned short *)&colour) != 0) {
            ComputeVectorDelta(
                &g_aShipPosition_00494550[WC2_EYE_OBJECT],
                &g_aShipPosition_00494550[object], &relative);
            transform_to_objects_frame(
                &relative, &rotated, WC2_EYE_OBJECT);
            rectangular_to_spherical(&rotated, &spherical);
            if (spherical.radius < 0xea6000) {
                AdjustScannerContactColourForRange(
                    spherical.radius, (short *)&colour);
                rotational_pos_to_scanner_pos(
                    (signed char)object, &spherical, scanner);
                if (g_nScannerTargetObject_0049b07c != object) {
                    g_asScannerBackgroundColour_005d1ed0[object] =
                        (short)GetViewportPixel(
                            &g_stScreenViewport_005d21a0,
                            g_nScannerCursorX_005d1e6c,
                            g_nScannerCursorY_005d1e6e);
                    DrawViewportPixel(
                        &g_stScreenViewport_005d21a0,
                        g_nScannerCursorX_005d1e6c,
                        g_nScannerCursorY_005d1e6e,
                        (short)colour);
                }
            }
        }
        object++;
    }

    if (get_mode(1) == 5 &&
        g_nCurrentView_00492fa8 != 4 &&
        IsMissionObjectiveOutOfSystem(
            (short)g_cCurrentObjective_004931cc) == 0) {
        set_objective_range(1);
        CaptureSpriteBackground(
            &g_stScreenViewport_005d21a0,
            g_pScannerMarkerBackground_005d1c2c,
            g_nScannerCursorX_005d1e6c,
            g_nScannerCursorY_005d1e6e,
            g_pCockpitHudShape_005d21f4, 2);
        DrawSpriteDefault(
            &g_stScreenViewport_005d21a0,
            g_nScannerCursorX_005d1e6c,
            g_nScannerCursorY_005d1e6e,
            g_pCockpitHudShape_005d21f4, 2);
    }

    if (g_nScannerTargetObject_0049b07c != -1 &&
        g_anShipCloakState_00496020[
            g_nScannerTargetObject_0049b07c] != 1 &&
        get_color(g_nScannerTargetObject_0049b07c,
                  (unsigned short *)&colour) != 0) {
        ComputeVectorDelta(
            &g_aShipPosition_00494550[WC2_EYE_OBJECT],
            &g_aShipPosition_00494550[object], &relative);
        transform_to_objects_frame(
            &relative, &rotated, WC2_EYE_OBJECT);
        rectangular_to_spherical(&rotated, &spherical);
        if (spherical.radius < 0xea6000)
            AdjustScannerContactColourForRange(
                spherical.radius, (short *)&colour);

        object = g_nScannerTargetObject_0049b07c;
        g_asScannerTargetBackground_005d1d80[0] =
            (short)GetViewportPixel(
                &g_stScreenViewport_005d21a0,
                g_asScannerObjectX_005d1ea0[object],
                g_asScannerObjectY_005d1e80[object]);
        g_asScannerTargetBackground_005d1d80[1] =
            (short)GetViewportPixel(
                &g_stScreenViewport_005d21a0,
                (short)(g_asScannerObjectX_005d1ea0[object] + 1),
                g_asScannerObjectY_005d1e80[object]);
        g_asScannerTargetBackground_005d1d80[2] =
            (short)GetViewportPixel(
                &g_stScreenViewport_005d21a0,
                (short)(g_asScannerObjectX_005d1ea0[object] - 1),
                g_asScannerObjectY_005d1e80[object]);
        g_asScannerTargetBackground_005d1d80[3] =
            (short)GetViewportPixel(
                &g_stScreenViewport_005d21a0,
                g_asScannerObjectX_005d1ea0[object],
                (short)(g_asScannerObjectY_005d1e80[object] + 1));
        g_asScannerTargetBackground_005d1d80[4] =
            (short)GetViewportPixel(
                &g_stScreenViewport_005d21a0,
                g_asScannerObjectX_005d1ea0[object],
                (short)(g_asScannerObjectY_005d1e80[object] - 1));
        DrawViewportPixel(
            &g_stScreenViewport_005d21a0,
            g_asScannerObjectX_005d1ea0[object],
            g_asScannerObjectY_005d1e80[object], (short)colour);
        DrawViewportPixel(
            &g_stScreenViewport_005d21a0,
            (short)(g_asScannerObjectX_005d1ea0[object] + 1),
            g_asScannerObjectY_005d1e80[object], (short)colour);
        DrawViewportPixel(
            &g_stScreenViewport_005d21a0,
            (short)(g_asScannerObjectX_005d1ea0[object] - 1),
            g_asScannerObjectY_005d1e80[object], (short)colour);
        DrawViewportPixel(
            &g_stScreenViewport_005d21a0,
            g_asScannerObjectX_005d1ea0[object],
            (short)(g_asScannerObjectY_005d1e80[object] + 1),
            (short)colour);
        DrawViewportPixel(
            &g_stScreenViewport_005d21a0,
            g_asScannerObjectX_005d1ea0[object],
            (short)(g_asScannerObjectY_005d1e80[object] - 1),
            (short)colour);
    }
    if (g_nCurrentView_00492fa8 == 4)
        DrawTargetCameraAttitudeIndicators(
            g_bPrimaryViewBufferColour_0049cb50);
}

/* Function start: 0x43BE1B */
void start_lock(unsigned short v)
{
    g_nTargetLockMusicCooldown_005d1e78 = -100;
    g_bTargetLockReadoutDirty_004934e8 = 0;
    g_nTargetLockCountdown_004934ec = v;
    g_nTargetLockMarkerAngle_004934f0 =
        RandomBelowOrEqual(0x167);
}

/* Function start: 0x43BE55 */
short starting_lock(unsigned short v)
{
    if (g_nTargetLockCountdown_004934ec == -1) {
        start_lock(v);
        return 1;
    }
    return 0;
}

/* Function start: 0x43BE8D */
void lock_off(void)
{
    if (g_nTargetLockCountdown_004934ec > -1) {
        g_bTargetLockReadoutDirty_004934e8 = 1;
        if (((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[
                g_nSelectedReleaseWeaponIndex_004934e0].type == 0x13) {
            StopMusic(0);
            g_bTargetLockMode_00493500 = 0;
        }
    }
    remove_message(g_pszMissileLocked_0049b280);
    g_nTargetLockCountdown_004934ec = -1;
    g_nTargetLockMusicCooldown_005d1e78 = -98;
}

/* Function start: 0x43BEFF */
short CheckTargetLockMalfunction(void)
{
    if (malf(5) != 0) {
        g_bTargetLockActive_0049ae80 = 0;
        lock_off();
        g_nTargetLockCountdown_004934ec =
            (short)-(RandomBelowOrEqual(30) + 10);
        PlaySfxWaveFileByNumber(7, -1, 0);
        return 1;
    }
    return 0;
}

/* Function start: 0x43BF67 */
short decrement_lock_time(short screenX)
{
    (void)screenX;
    if (g_nTargetLockCountdown_004934ec > 0) {
        if (malf(5) == 0) {
            g_nTargetLockCountdown_004934ec--;
            g_bTargetLockAcquired_004934fc =
                g_nTargetLockCountdown_004934ec == 0;
            if (g_bTargetLockAcquired_004934fc != 0) {
                if (CheckTargetLockMalfunction() == 0)
                    PlaySfxWaveFileByNumber(0x16, -1, 0);
                CockpitMessage(g_pszMissileLocked_0049b280,
                               g_abGamePaletteReservedColours_0049cb54[8], 2);
            } else if (((ShipWeaponSlot *)(
                           g_aShipWeapons_004956b0[0] + 1))[
                           g_nSelectedReleaseWeaponIndex_004934e0]
                               .type != WC2_OBJECT_TYPE_TORPEDO) {
                PlaySfxWaveFileByNumber(0x15, -1, 0);
            }
        }
        return 1;
    }
    return 0;
}

/* Function start: 0x43C048 */
void target_locking(signed char target)
{
    enum Wc2ReleaseWeaponObjectType weaponType;
    short x;
    short y;

    if (g_nCurrentView_00492fa8 == 4)
        return;
    if (target == -1 ||
        g_asShipSide_004955d0[(short)target] == g_asShipSide_004955d0[0] ||
        g_acPlayerComponentDamage_00493470[5] >= 4) {
        g_bTargetLockActive_0049ae80 = 0;
        lock_off();
        return;
    }
    if (g_asObjectScreenX_00493598[(short)target] == -0x7fff)
        return;
    x = g_asObjectScreenX_00493598[(short)target];
    y = g_asObjectScreenY_00493628[(short)target];
    if (g_nTargetLockCountdown_004934ec < -1) {
        g_nTargetLockCountdown_004934ec++;
        return;
    }
    if (y * y + x * x > 0xe10) {
        lock_off();
        return;
    }
    weaponType = (enum Wc2ReleaseWeaponObjectType)((ShipWeaponSlot *)(
        g_aShipWeapons_004956b0[0] + 1))[
            g_nSelectedReleaseWeaponIndex_004934e0].type;
    switch (weaponType) {
    case WC2_OBJECT_TYPE_JAVELIN_HEAT_SEEKING_MISSILE:
        get_facing_range_from_object(0, (short)target);
        if (g_nTargetFacing_00493198 > -0x41 ||
            CanShipWeaponDamageTarget(0, (short)target) == 0) {
            lock_off();
            break;
        }
        if (starting_lock(0x0c) == 0)
            decrement_lock_time(x);
        break;
    case WC2_OBJECT_TYPE_SPICULUM_IMAGE_RECOGNITION_MISSILE:
        if (CanShipWeaponDamageTarget(0, (short)target) == 0) {
            lock_off();
            break;
        }
        if (starting_lock(0x16) == 0)
            decrement_lock_time(x);
        break;
    case WC2_OBJECT_TYPE_TORPEDO:
        if (IsCapitalShipObject((short)target) == 0) {
            lock_off();
            break;
        }
        if (g_nCurrentView_00492fa8 != 4) {
            get_facing_range_from_object(0, (short)target);
            if (g_bTargetLockMode_00493500 == 0 ||
                g_nTargetLockMusicCooldown_005d1e78 -
                        g_nFacingToTarget_00493194 >
                    4) {
                lock_off();
            } else {
                g_nTargetLockMusicCooldown_005d1e78 =
                    g_nFacingToTarget_00493194;
                if (g_nTargetLockMusicCooldown_005d1e78 > 100)
                    g_nTargetLockMusicCooldown_005d1e78 = 100;
                if (starting_lock(300) != 0)
                    ProcessMusicScriptCommand(0x2a, 1, 0);
                else
                    decrement_lock_time(x);
            }
        }
        break;
    default:
        lock_off();
        break;
    }
}

/* Function start: 0x43C30D */
void SetRectBounds(Viewport *viewport, unsigned short a, unsigned short b,
                   unsigned short c, unsigned short d)
{
    viewport->left = (short)a;
    viewport->top = (short)b;
    viewport->right = (short)c;
    viewport->bottom = (short)d;
}

/* Function start: 0x43C344 */
short GetRectHeight(const Viewport *viewport)
{
    return viewport->right - viewport->left;
}

/* Function start: 0x43C364 */
void print_message_text(char *text, unsigned short colour)
{
    Viewport defaultViewport;
    char source[84];
    char wrapped[84];
    short charactersPerLine;
    Viewport spaceViewport;
    TextContext defaultContext;
    char *input;
    TextContext spaceContext;
    char *output;
    short position;
    short messageY;
    short lastSpace;

    lastSpace = -1;
    DosStrcpy(source, text);
    strcpy(wrapped, "");
    defaultViewport = *g_stDefaultTextContext_005d2d20.viewport;
    defaultContext = g_stDefaultTextContext_005d2d20;
    defaultContext.viewport = &defaultViewport;
    messageY = (short)(g_stHudMessageOrigin_0049ae90.y +
                       g_nViewportOriginY_005c849c);
    SetRectBounds(&defaultViewport,
                  g_stHudMessageOrigin_0049ae90.x,
                  messageY,
                  (short)(319 - g_stHudMessageOrigin_0049ae90.x),
                  (short)(messageY + 60));
    defaultContext.colour = (unsigned char)colour;
    defaultContext.backgroundColour = 0xff;
    spaceViewport = *g_stSpaceTextContext_005d21c0.viewport;
    spaceContext = g_stSpaceTextContext_005d21c0;
    spaceContext.viewport = &spaceViewport;
    SetRectBounds(&spaceViewport,
                  g_stHudMessageOrigin_0049ae90.x,
                  g_stHudMessageOrigin_0049ae90.y,
                  (short)(319 - g_stHudMessageOrigin_0049ae90.x),
                  (short)(g_stHudMessageOrigin_0049ae90.y + 60));
    spaceContext.colour = (unsigned char)colour;
    spaceContext.backgroundColour = 0xff;
    charactersPerLine = (short)(GetRectHeight(&defaultViewport) / 6);
    input = source;
    output = wrapped;
    position = 0;
    while (*input != 0) {
        *output = *input;
        if (*output == ' ')
            lastSpace = position;
        if ((position + 1) % charactersPerLine == 0) {
            if (lastSpace == -1) {
                output++;
                *output = '\n';
            } else {
                output[position - lastSpace] = '\n';
            }
        }
        input++;
        output++;
        position++;
    }
    *output = 0;
    DrawTextAt(&defaultContext,
               g_stHudMessageOrigin_0049ae90.x,
               messageY,
               wrapped, 2);
    g_pszDisplayedHudMessage_0049aff0 = g_pszPendingHudMessage_0049afec;
}

/* Function start: 0x43C5B0 */
void RestoreHudMessageBackground(void)
{
    if (g_nHudMessageBackgroundDepth_0049b294 != 0) {
        RestoreSpriteBackground(
            &g_stScreenViewport_005d21a0,
            g_pHudMessageBackground_0049b28c,
            0,
            (short)(g_stHudMessageOrigin_0049ae90.y +
                    g_nViewportOriginY_005c849c),
            g_pHudMessageFrameShape_0049b288,
            0);
    }
    g_nHudMessageBackgroundDepth_0049b294 = 0;
}

/* Function start: 0x43C570 */
void ShowHudTextLine(void)
{
    CaptureSpriteBackground(
        &g_stScreenViewport_005d21a0,
        g_pHudMessageBackground_0049b28c,
        0,
        (short)(g_stHudMessageOrigin_0049ae90.y +
                g_nViewportOriginY_005c849c),
        g_pHudMessageFrameShape_0049b288,
        0);
    g_nHudMessageBackgroundDepth_0049b294++;
}

/* Function start: 0x43C601 */
void SetHudTextColour(char *text, int colour)
{
    if (g_bCaptureHudMessageBackground_0049b290 != 0) {
        ShowHudTextLine();
        g_bCaptureHudMessageBackground_0049b290 = 0;
    }
    g_pszPendingHudMessage_0049afec = text;
    g_cHudMessageView_005d1c37 = (signed char)g_nCurrentView_00492fa8;
    print_message_text(g_pszPendingHudMessage_0049afec, colour);
}

/* Function start: 0x43C64D */
void ClearHudMessageDisplay(short closeCommMenu)
{
    if (closeCommMenu != 0)
        EndCommMenu();
    if (g_cHudMessageView_005d1c37 == g_nCurrentView_00492fa8 &&
        g_pszDisplayedHudMessage_0049aff0 != 0 &&
        g_nHudMessageBackgroundDepth_0049b294 != 0) {
        print_message_text(g_pszDisplayedHudMessage_0049aff0,
                           g_cPrimaryViewBufferColour_0049cb88);
        RestoreHudMessageBackground();
    }
    g_bCaptureHudMessageBackground_0049b290 = 1;
    if (g_bDisplayWingmanTargetData_0049347c != 0)
        SelectCockpitVduMode(1, 9);
}

/* Function start: 0x43C6D8 */
void draw_target_box(unsigned short colour, signed char object,
                     short solid, short drawLockMarker, short padding,
                     ShortRect *savedBounds)
{
    short centerY;
    ShortRect bounds;
    int colourValue;
    short centerX;
    short segmentLength;
    short valid;

    colourValue = (short)colour;
    if ((int)(unsigned char)g_cPrimaryViewBufferColour_0049cb88 == colourValue) {
        valid = savedBounds->left != -0x7fff;
        bounds = *savedBounds;
    } else {
        valid = object != -1 &&
                g_asObjectScreenX_00493598[object] != -0x7fff;
        if (valid != 0) {
            centerX = (short)(g_asObjectScreenX_00493598[object] +
                              g_nViewCenterX_005c80d8);
            centerY = (short)(g_asObjectScreenY_00493628[object] +
                              g_nViewCenterY_005c80da);
            if ((short)GetTransformedShapeBounds(
                    &g_stViewBuffer_005d2b00, centerX, centerY,
                    g_apObjectShape_00493868[object],
                    g_asObjectViewFrame_00493508[object],
                    g_asObjectScreenAngle_004936b8[object],
                    g_asObjectScreenScale_00493a58[object],
                    g_asObjectFlip_004939c8[object],
                    (short *)&bounds) != 0) {
                bounds.left = (short)(bounds.left - padding);
                bounds.top = (short)(bounds.top - padding);
                bounds.right = (short)(bounds.right + padding);
                bounds.bottom = (short)(bounds.bottom + padding);
            } else {
                valid = 0;
            }
        }
    }
    if (valid != 0) {
        if ((int)g_abGamePaletteReservedColours_0049cb54[8] == colourValue &&
            g_asShipSide_004955d0[object] == g_asShipSide_004955d0[0]) {
            colour = g_abGamePaletteReservedColours_0049cb54[0];
        }
        if (solid != 0) {
            DrawViewportBorder(&g_stViewBuffer_005d2b00, bounds.left, bounds.top,
                               bounds.right, bounds.bottom, colour);
        } else {
            segmentLength =
                (short)(((int)bounds.right - bounds.left) / 6 + 1);
            DrawViewportLine(&g_stViewBuffer_005d2b00, bounds.left, bounds.top,
                             (short)(bounds.left + segmentLength), bounds.top,
                             colour);
            DrawViewportLine(&g_stViewBuffer_005d2b00, bounds.left, bounds.bottom,
                             (short)(bounds.left + segmentLength), bounds.bottom,
                             colour);
            DrawViewportLine(&g_stViewBuffer_005d2b00, bounds.right, bounds.top,
                             (short)(bounds.right - segmentLength), bounds.top,
                             colour);
            DrawViewportLine(&g_stViewBuffer_005d2b00, bounds.right, bounds.bottom,
                             (short)(bounds.right - segmentLength), bounds.bottom,
                             colour);
            segmentLength =
                (short)(((int)bounds.bottom - bounds.top) / 6 + 1);
            DrawViewportLine(&g_stViewBuffer_005d2b00, bounds.left, bounds.top,
                             bounds.left, (short)(bounds.top + segmentLength),
                             colour);
            DrawViewportLine(&g_stViewBuffer_005d2b00, bounds.left, bounds.bottom,
                             bounds.left,
                             (short)(bounds.bottom - segmentLength), colour);
            DrawViewportLine(&g_stViewBuffer_005d2b00, bounds.right, bounds.top,
                             bounds.right, (short)(bounds.top + segmentLength),
                             colour);
            DrawViewportLine(&g_stViewBuffer_005d2b00, bounds.right, bounds.bottom,
                             bounds.right,
                             (short)(bounds.bottom - segmentLength), colour);
        }
        if (drawLockMarker != 0) {
            if ((int)(short)colour != (int)(unsigned char)g_cPrimaryViewBufferColour_0049cb88) {
                if (g_nTargetLockCountdown_004934ec > -1) {
                    g_nTargetLockMarkerAngle_004934f0 = (short)(
                        g_nTargetLockMarkerAngle_004934f0 +
                        g_anObjectRollRotation_00495058[0] +
                        g_anObjectPitchRotation_00494f38[0]);
                    centerX = (short)(centerX +
                        ((CosFixed(g_nTargetLockMarkerAngle_004934f0) *
                          g_nTargetLockCountdown_004934ec * 2) >> 8));
                    centerY = (short)(centerY +
                        ((SinFixed(g_nTargetLockMarkerAngle_004934f0) *
                          g_nTargetLockCountdown_004934ec * 2) >> 8));
                    DrawSpriteDefault(&g_stViewBuffer_005d2b00, centerX, centerY,
                                      g_pTargetLockShape_005a6bf4, 1);
                    g_nTargetLockMarkerX_0049b298 = centerX;
                    g_nTargetLockMarkerY_005d1e20 = centerY;
                }
            } else if (g_nTargetLockMarkerX_0049b298 != -0x7fff) {
                DrawSolidColourSprite(&g_stViewBuffer_005d2b00,
                                      g_nTargetLockMarkerX_0049b298,
                                      g_nTargetLockMarkerY_005d1e20,
                                      g_pTargetLockShape_005a6bf4, 1,
                                      g_cPrimaryViewBufferColour_0049cb88);
                g_nTargetLockMarkerX_0049b298 = -0x7fff;
            }
        }
        if ((int)(unsigned char)g_cPrimaryViewBufferColour_0049cb88 == (int)(short)colour) {
            savedBounds->left = -0x7fff;
        } else {
            *savedBounds = bounds;
        }
    } else {
        savedBounds->left = -0x7fff;
    }
}

/* Function start: 0x43CBD3 */
void remove_nav_pointer(void)
{
    if (g_nNavPointerObject_004931b8 != -1)
        remove_object(g_nNavPointerObject_004931b8);
}

/* Function start: 0x43CBFD */
void draw_nav_pointer(void)
{
    FixedVector objectivePosition;
    FixedVector direction;
    FixedVector viewPosition;
    int distance;
    short active;
    short object;

    if (get_mode(1) == 5 &&
        g_nCannedSceneMode_0049021c != 4 &&
        (g_nCurrentView_00492fa8 == 0 || g_nCurrentView_00492fa8 == 4))
        active = 1;
    else
        active = 0;
    if (active == 0) {
        remove_nav_pointer();
        return;
    }
    object = g_nNavPointerObject_004931b8;
    if (object == -1) {
        object = find_vacant_3d_object();
        g_nNavPointerObject_004931b8 = object;
        if (object == -1)
            return;
        g_asObjectViewFrame_00493508[object] = 3;
        g_acObjectOwner_00495208[object] = -1;
        g_asObjectScreenAngle_004936b8[object] = 0;
        g_asObjectScreenScale_00493a58[object] = 0x100;
        g_aeObjectClass_00495328[object] = OBJECT_CLASS_PLANET;
        g_apObjectShape_00493868[object] =
            g_pTargetLockShape_005a6bf4;
        g_nNavPointerObject_004931b8 = object;
        g_asObjectScreenX_00493598[object] = (short)0x8001;
        g_asObjectDistance_00493ae8[object] = 0;
    }
    objectivePosition = g_aMissionObjectives_004932a8[
        (signed char)g_cCurrentObjective_004931cc].position;
    ComputeVectorDelta(&g_aShipPosition_00494550[WC2_EYE_OBJECT],
                       &objectivePosition, &direction);
    distance = Vector_magnitude(&direction);
    if (g_asObjectCollisionRadius_004950e8[WC2_EYE_OBJECT] * 0x100 >=
        distance)
        return;
    transform_to_objects_frame(&direction, &viewPosition,
                               WC2_EYE_OBJECT);
    if (g_asObjectCollisionRadius_004950e8[WC2_EYE_OBJECT] * 0x100 >
        viewPosition.z)
        return;
    if (DivideFixed(viewPosition.z, distance) < 0x94)
        return;
    g_asObjectScreenX_00493598[object] = (short)(DivideFixed(
        MultiplyFixed(
            ((short)g_nScreenWidth_0049d4d8 & ~1) << 7,
            viewPosition.x),
        viewPosition.z) >> 8);
    g_asObjectScreenY_00493628[object] = (short)(DivideFixed(
        MultiplyFixed(
            ((short)g_nScreenWidth_0049d4d8 & ~1) << 7,
            viewPosition.y),
        viewPosition.z) >> 8);
    g_asObjectDistance_00493ae8[object] = 0x4a38;
}

/* Function start: 0x43CE8F */
short HasInRangeGunForTargetLead(short targetRange)
{
    short previousRemaining;
    short remaining;

    remaining = (signed char)g_aShipWeapons_004956b0[0][0];
    while ((previousRemaining = remaining--) > 0) {
        if (((ShipWeaponSlot *)(
                g_aShipWeapons_004956b0[0] + 1))[remaining].disabled == 0 &&
            g_aObjectTypeData_00496d30[(signed char)((ShipWeaponSlot *)(
                g_aShipWeapons_004956b0[0] + 1))[remaining].weaponType]
                    .objectClass == OBJECT_CLASS_PROJECTILE &&
            g_aObjectTypeData_00496d30[(signed char)((ShipWeaponSlot *)(
                g_aShipWeapons_004956b0[0] + 1))[remaining].weaponType]
                    .maximumVelocity *
                    g_aObjectTypeData_00496d30[(signed char)(
                        (ShipWeaponSlot *)(
                            g_aShipWeapons_004956b0[0] + 1))[remaining]
                                                         .weaponType]
                        .lifetime >
                targetRange)
            return 1;
    }
    return 0;
}

/* Function start: 0x43CF5A */
void UpdateTargetLeadIndicator(void)
{
    FixedVector combinedProjectileVelocity;
    short targetObject;
    short weapon;
    short projectileSpeed;
    FixedVector projectileVelocity;
    int targetDistance;
    short interceptFrames;
    FixedVector targetVelocityOffset;
    FixedVector interceptPoint;
    FixedVector relative;
    FixedVector eyeRelative;

    if (g_bTargetLockDisplayEnabled_0049afe0 == 0)
        return;
    if (g_bTargetLockMode_00493500 == 0) {
        g_bTargetLockActive_0049ae80 = 0;
        return;
    }

    targetObject = g_acShipTarget_00495f20[0];
    targetDistance = (unsigned short)distance_from_object(
        0, targetObject);
    projectileSpeed = 100;
    if (g_nCurrentView_00492fa8 == 4) {
        projectileSpeed =
            g_aObjectTypeData_00496d30[
                OBJECT_TYPE_TIGERS_CLAW].maximumVelocity;
    } else {
        weapon = (signed char)g_aShipWeapons_004956b0[0][0];
        while (weapon > 0) {
            signed char weaponType;

            weapon--;
            weaponType = ((ShipWeaponSlot *)(
                g_aShipWeapons_004956b0[0] + 1))[weapon].weaponType;
            if (((ShipWeaponSlot *)(
                    g_aShipWeapons_004956b0[0] + 1))[weapon].disabled == 0 &&
                g_aObjectTypeData_00496d30[weaponType].objectClass ==
                    OBJECT_CLASS_PROJECTILE &&
                projectileSpeed <
                    g_aObjectTypeData_00496d30[
                        weaponType].maximumVelocity)
                projectileSpeed =
                    g_aObjectTypeData_00496d30[
                        weaponType].maximumVelocity;
        }
    }

    vector_component_in_dir(
        &g_aShipVelocity_00494898[0],
        &g_aShipForwardVector_00494208[0],
        &combinedProjectileVelocity);
    ScaleFixedVector(
        &g_aShipForwardVector_00494208[0],
        (int)projectileSpeed << 8, &projectileVelocity);
    AddFixedVectors(
        &projectileVelocity, &combinedProjectileVelocity,
        &combinedProjectileVelocity);
    projectileSpeed =
        (short)((Vector_magnitude(&combinedProjectileVelocity) >> 8) + 1);
    interceptFrames = (short)(targetDistance / projectileSpeed);
    zero_vector(&targetVelocityOffset);
    if (interceptFrames > 0)
        ScaleFixedVector(
            &g_aShipVelocity_00494898[targetObject],
            (int)interceptFrames << 8, &targetVelocityOffset);
    AddFixedVectors(
        &g_aShipPosition_00494550[targetObject],
        &targetVelocityOffset, &interceptPoint);
    if (HasInRangeGunForTargetLead(
            distance_from_point(0, &interceptPoint)) == 0) {
        SetHudMessageText(
            g_szTargetOutOfRange_0049b438,
            g_abGamePaletteReservedColours_0049cb54[8], 20);
        g_bTargetLockActive_0049ae80 = 0;
        return;
    }

    ComputeVectorDelta(
        &g_aShipPosition_00494550[0], &interceptPoint, &relative);
    targetDistance = Vector_magnitude(&relative);
    if ((int)g_asObjectCollisionRadius_004950e8[
            WC2_EYE_OBJECT] * 0x100 < targetDistance) {
        transform_to_objects_frame(
            &relative, &eyeRelative, WC2_EYE_OBJECT);
        if ((int)g_asObjectCollisionRadius_004950e8[
                WC2_EYE_OBJECT] * 0x100 <= eyeRelative.z &&
            DivideFixed(eyeRelative.z, targetDistance) >= 0) {
            g_nTargetLeadIndicatorX_0049afe8 =
                (short)(g_nViewCenterX_005c80d8 +
                        DivideFixed(
                            MultiplyFixed(
                                g_nScreenWidth_0049d4d8 >> 1,
                                eyeRelative.x),
                            eyeRelative.z));
            g_nTargetLeadIndicatorY_005d1c2a =
                (short)(g_nViewCenterY_005c80da +
                        DivideFixed(
                            MultiplyFixed(
                                g_nScreenWidth_0049d4d8 >> 1,
                                eyeRelative.y),
                            eyeRelative.z));
        }
    }
    if (g_nTargetLeadIndicatorX_0049afe8 < 10 ||
        g_nTargetLeadIndicatorX_0049afe8 > 310 ||
        g_nTargetLeadIndicatorY_005d1c2a < 5 ||
        g_nTargetLeadIndicatorY_005d1c2a > 190) {
        g_nTargetLeadIndicatorX_0049afe8 = 0x7fff;
    } else {
        CaptureSpriteBackground(
            &g_stViewBuffer_005d2b00,
            g_pCockpitHudBackground_0049b044,
            g_nTargetLeadIndicatorX_0049afe8,
            g_nTargetLeadIndicatorY_005d1c2a,
            g_pCockpitHudShape_005d21f4, 5);
        DrawSpriteDefault(
            &g_stViewBuffer_005d2b00,
            g_nTargetLeadIndicatorX_0049afe8,
            g_nTargetLeadIndicatorY_005d1c2a,
            g_pCockpitHudShape_005d21f4, 5);
    }
}

/* Function start: 0x43D323 */
void RestoreTargetLeadIndicator(void)
{
    if (g_bTargetLockDisplayEnabled_0049afe0 != 0 &&
        g_nTargetLeadIndicatorX_0049afe8 != 0x7fff) {
        RestoreSpriteBackground(
            &g_stViewBuffer_005d2b00,
            g_pCockpitHudBackground_0049b044,
            g_nTargetLeadIndicatorX_0049afe8,
            g_nTargetLeadIndicatorY_005d1c2a,
            g_pCockpitHudShape_005d21f4, 5);
        g_nTargetLeadIndicatorX_0049afe8 = 0x7fff;
    }
}

/* Function start: 0x43D386 */
void overlay_head_up_display(void)
{
    ShortRect reticleBounds;

    reticleBounds.left = (short)(g_nViewCenterX_005c80d8 - 2);
    reticleBounds.right = (short)(g_nViewCenterX_005c80d8 + 2);
    reticleBounds.top = (short)(g_nViewCenterY_005c80da - 2);
    reticleBounds.bottom = (short)(g_nViewCenterY_005c80da + 2);
    target_locking(g_acShipTarget_00495f20[0]);
    if (g_nTargetCameraFrame_0049d3e8 == 0 &&
        g_acShipTarget_00495f20[0] != -1 &&
        g_aeObjectClass_00495328[
            g_acShipTarget_00495f20[0]] <
                OBJECT_CLASS_CAPITAL_SHIP &&
        g_bTargetLockActive_0049ae80 == 0 &&
        g_acPlayerComponentDamage_00493470[5] < 4 &&
        g_bTargetLockMode_00493500 != 0 &&
        HasInRangeGunForTargetLead(
            distance_from_object(
                0, g_acShipTarget_00495f20[0])) != 0 &&
        g_bTargetLockDisplayEnabled_0049afe0 != 0) {
        g_bTargetLockActive_0049ae80 = 1;
        SetHudMessageText(
            g_szIttsEngaged_0049b448,
            g_abGamePaletteReservedColours_0049cb54[8], 20);
    }
    if (g_acShipTarget_00495f20[0] == -1 ||
        (g_anShipCloakState_00496020[
             g_acShipTarget_00495f20[0]] == 1 &&
         g_asShipCloakElapsedFrames_00496060[
             g_acShipTarget_00495f20[0]] >= 40)) {
        g_bTargetLockActive_0049ae80 = 0;
        g_bTargetLockMode_00493500 = 0;
    }
    if (g_bTargetLockActive_0049ae80 != 0 &&
        g_nTargetCameraFrame_0049d3e8 == 0)
        UpdateTargetLeadIndicator();
    if (message_showing() && g_nCommSpeakerObject_0049b794 != -1) {
        g_cPreviousTargetObject_005d1d8e =
            (signed char)g_nCommSpeakerObject_0049b794;
        draw_target_box(g_bPrimaryViewBufferColour_0049cb50,
                        g_cPreviousTargetObject_005d1d8e,
                        0, 0, 2,
                        &g_stCommTargetBracketBounds_0049b2a8);
    }
    if (g_nCurrentView_00492fa8 == 4 &&
        g_nTargetCameraMode_005c8d50 == 1 &&
        g_acShipTarget_00495f20[0] != -1) {
        if (g_nTargetCameraFrame_0049d3e8 == 0) {
            draw_target_box(g_abGamePaletteReservedColours_0049cb54[4],
                            g_acShipTarget_00495f20[0],
                            g_bTargetLockMode_00493500,
                            1, 1,
                            &g_stTargetBracketBounds_0049b2a0);
            g_nTargetCameraZoom_0049d3e4 = 0x42;
        } else if (g_aObjectTypeData_00496d30[
                       g_nObjectType62Index_00492d64].shapeSet == 0) {
            draw_target_box(g_ucDefaultTextColour_0049cb7c,
                            g_acShipTarget_00495f20[0],
                            g_bTargetLockMode_00493500,
                            1, 1,
                            &g_stTargetBracketBounds_0049b2a0);
        }
    }
    if (g_nCurrentView_00492fa8 != 4 ||
        g_nTargetCameraMode_005c8d50 != 1) {
        if (g_nTargetLockCountdown_004934ec == 0) {
            if ((short)(g_nRenderedSpaceFrame_00493138 % 2) == 0)
                g_bTargetBracketVisible_0049b284 ^= 1;
            if (g_bTargetBracketVisible_0049b284 == 1)
                draw_target_box(
                    g_abGamePaletteReservedColours_0049cb54[8],
                    g_acShipTarget_00495f20[0],
                    g_bTargetLockMode_00493500,
                    1, 1, &g_stTargetBracketBounds_0049b2a0);
        } else {
            draw_target_box(
                g_abGamePaletteReservedColours_0049cb54[8],
                g_acShipTarget_00495f20[0],
                g_bTargetLockMode_00493500,
                1, 1, &g_stTargetBracketBounds_0049b2a0);
        }
    }
    if (g_nCurrentView_00492fa8 == 4 &&
        g_nTargetCameraMode_005c8d50 == 1) {
        DrawSpriteDefault(
            &g_stViewBuffer_005d2b00,
            g_nViewCenterX_005c80d8,
            g_nViewCenterY_005c80da,
            g_pCockpitHudShape_005d21f4, 0);
    } else if (IsSpriteFrameOverlappingRect(
                   &reticleBounds,
                   g_nTargetLeadIndicatorX_0049afe8,
                   g_nTargetLeadIndicatorY_005d1c2a,
                   g_pCockpitHudShape_005d21f4, 5) == 0) {
        g_bMissileLockAcquired_0049b2b0 = 0;
        DrawSpriteDefault(
            &g_stViewBuffer_005d2b00,
            g_nViewCenterX_005c80d8,
            g_nViewCenterY_005c80da,
            g_pCockpitHudShape_005d21f4, 0);
    } else {
        g_bMissileLockAcquired_0049b2b0++;
        DrawSpriteDefault(
            &g_stViewBuffer_005d2b00,
            (short)(g_nViewCenterX_005c80d8 + 1),
            g_nViewCenterY_005c80da,
            g_pCockpitHudShape_005d21f4, 4);
    }
    DAT_004934e4 = 0;
    if (g_nCockpitControlState_0049d7ac == 1) {
        CaptureSpriteBackground(
            &g_stViewBuffer_005d2b00,
            (unsigned char *)g_pApplicationScratchBuffer_005c8483,
            g_nPersonnelCursorX_005c8470,
            g_nPersonnelCursorY_005c8472,
            GetInputCursorShape(), g_nUiCursorFrame_005c8481);
        DrawSpriteDefault(
            &g_stViewBuffer_005d2b00,
            g_nPersonnelCursorX_005c8470,
            g_nPersonnelCursorY_005c8472,
            GetInputCursorShape(), g_nUiCursorFrame_005c8481);
    }
}

/* Function start: 0x43D7F2 */
void RestoreCockpitExplosionIfVisible(void)
{
    if (IsCockpitExplosionActive() &&
        g_pCockpitExplosionBackground_0049b040 != 0) {
        RestoreCockpitExplosionBackground();
    }
}

/* Function start: 0x43D81F */
void RestoreTransientCockpitGraphics(void)
{
    if (g_nCockpitControlState_0049d7ac == 1)
        RestoreSpriteBackground(
            &g_stViewBuffer_005d2b00,
            (unsigned char *)g_pApplicationScratchBuffer_005c8483,
            g_nPersonnelCursorX_005c8470,
            g_nPersonnelCursorY_005c8472,
            GetInputCursorShape(), g_nUiCursorFrame_005c8481);
    if (g_bTargetLockActive_0049ae80 != 0)
        RestoreTargetLeadIndicator();
    if (g_cPreviousTargetObject_005d1d8e != -1) {
        draw_target_box(g_cPrimaryViewBufferColour_0049cb88,
                        g_cPreviousTargetObject_005d1d8e,
                        0, 0, 2,
                        &g_stCommTargetBracketBounds_0049b2a8);
        g_cPreviousTargetObject_005d1d8e = -1;
    }
    draw_target_box(g_cPrimaryViewBufferColour_0049cb88,
                    g_acShipTarget_00495f20[0],
                    g_bTargetLockMode_00493500, 1, 1,
                    &g_stTargetBracketBounds_0049b2a0);
    if (IsCockpitExplosionActive() &&
        g_pCockpitExplosionBackground_0049b040 != 0) {
        CaptureSpriteBackground(
            &g_stScreenViewport_005d21a0,
            g_pCockpitExplosionBackground_0049b040,
            g_stCockpitExplosionPosition_005d1e98.x,
            g_stCockpitExplosionPosition_005d1e98.y,
            g_pCockpitExplosionShape_0049b048,
            g_nCockpitExplosionFrame_0049b04c);
        DrawSpriteDefault(&g_stScreenViewport_005d21a0,
                          g_stCockpitExplosionPosition_005d1e98.x,
                          g_stCockpitExplosionPosition_005d1e98.y,
                          g_pCockpitExplosionShape_0049b048,
                          g_nCockpitExplosionFrame_0049b04c);
        g_cLastPilotHandFrame_0049aff4 = 0xff;
    }
}

/* Function start: 0x43D956 */
void SetHudMessageText(char *text, unsigned short colour,
                       unsigned short duration)
{
    if (g_bInflightComputerActive_0049bcb4 == 0) {
        if (message_showing())
            ClearHudMessageDisplay(1);
        g_nHudMessageColour_005d1ef0 = colour;
        g_pszPendingHudMessage_0049afec = text;
        set_message_time(duration);
    }
}

/* Function start: 0x43D9DA */
void malf_noise(short vdu, int effect, unsigned short colour,
                short sound, short refresh)
{
    Viewport *viewport = vdu == 0 ? &g_stLeftVduViewport_005d2180 : &g_stRightVduViewport_005d2b20;

    if (sound != -1) {
        if (sound == 0x17)
            PlaySnowStaticSound();
        else
            PlaySfxWaveFileByNumber(sound, -1, 0);
    }
    snow_viewport(viewport, effect, colour);
    if (refresh != 0)
        set_new_vdu(vdu);
}

/* Function start: 0x43DA68 */
void build_your_target_list(short *hasEnemy)
{
    short bounds[4];
    unsigned char specialObject;
    short screenY;
    short screenX;
    unsigned short distance;
    signed char object;

    *hasEnemy = 0;
    g_cViableTargetCount_00496178 = 0;
    if (g_nCurrentView_00492fa8 == 4 &&
        g_nTargetCameraMode_005c8d50 == 1) {
        if (g_nTargetCameraZoom_0049d3e4 > 32) {
            for (specialObject = 0; specialObject < 10;
                 specialObject++) {
                if (((g_aeObjectClass_00495328[specialObject] !=
                          OBJECT_CLASS_NULL &&
                      g_asObjectType_00495298[specialObject] == 0x3d) ||
                     g_asObjectType_00495298[specialObject] == 0x2c) &&
                    g_asObjectScreenX_00493598[specialObject] !=
                        (short)0x8001) {
                    screenX = (short)(
                        g_asObjectScreenX_00493598[specialObject] +
                        g_nViewCenterX_005c80d8);
                    screenY = (short)(
                        g_asObjectScreenY_00493628[specialObject] +
                        g_nViewCenterY_005c80da);
                    GetTransformedShapeBounds(
                        &g_stViewBuffer_005d2b00, screenX, screenY,
                        g_apObjectShape_00493868[specialObject],
                        g_asObjectViewFrame_00493508[specialObject],
                        g_asObjectScreenAngle_004936b8[specialObject],
                        g_asObjectScreenScale_00493a58[specialObject],
                        g_asObjectFlip_004939c8[specialObject], bounds);
                    if (IsSpriteFrameOverlappingRect(
                            (const ShortRect *)bounds,
                            g_nViewCenterX_005c80d8,
                            g_nViewCenterY_005c80da,
                            g_pCockpitHudShape_005d21f4,
                            0) != 0) {
                        distance = (unsigned short)
                            g_asObjectDistance_00493ae8[specialObject];
                        if (distance < 4000) {
                            g_acViableTarget_00496180[
                                (int)g_cViableTargetCount_00496178] =
                                    (signed char)specialObject;
                            g_asViableTargetDistance_00496190[
                                (int)g_cViableTargetCount_00496178] =
                                    (short)distance;
                            g_cViableTargetCount_00496178++;
                        }
                    }
                }
            }
        }
    } else {
        for (object = 1; object < 10; object++) {
            if (g_aeObjectClass_00495328[(int)object] >=
                    OBJECT_CLASS_SHIP &&
                g_aeSpecialManeuver_00495600[(int)object] !=
                    SPECIAL_MANEUVER_UNKNOWN_9 &&
                g_asObjectScreenX_00493598[(int)object] !=
                    (short)0x8001) {
                distance = (unsigned short)
                    g_asObjectDistance_00493ae8[(int)object];
                if (distance < 12000) {
                    g_acViableTarget_00496180[
                        (int)g_cViableTargetCount_00496178] = object;
                    g_asViableTargetDistance_00496190[
                        (int)g_cViableTargetCount_00496178] =
                            (short)distance;
                    g_cViableTargetCount_00496178++;
                    if (g_asShipSide_004955d0[(int)object] !=
                        g_asShipSide_004955d0[0])
                        *hasEnemy = 1;
                }
            }
        }
    }
    if (g_cViableTargetCount_00496178 > 1)
        SortViableTargetsByDistance();
}

/* Function start: 0x43DD1B */
void cycle_onscreen_targets(void)
{
    signed char previousTarget;
    signed char index;
    short hasEnemy;

    previousTarget = g_acShipTarget_00495f20[0];
    build_your_target_list(&hasEnemy);
    if (g_cViableTargetCount_00496178 == 0) {
        g_acShipTarget_00495f20[0] = -1;
    } else {
        for (index = 0;
             index < g_cViableTargetCount_00496178 &&
             g_acViableTarget_00496180[(int)index] !=
                 g_acShipTarget_00495f20[0];
             index++)
            ;
        do {
            index = (signed char)((index + 1) %
                                  g_cViableTargetCount_00496178);
            g_acShipTarget_00495f20[0] =
                g_acViableTarget_00496180[(int)index];
            if (hasEnemy == 0)
                break;
        } while (g_asShipSide_004955d0[
                     g_acShipTarget_00495f20[0]] ==
                 g_asShipSide_004955d0[0]);
    }
    if (g_acShipTarget_00495f20[0] != previousTarget)
        g_nTargetLockCountdown_004934ec = -1;
}

/* Function start: 0x43DDFC */
void check_target(void)
{
    short selectNewTarget;
    short oldTarget;
    short targetIndex;
    short hasEnemy;

    selectNewTarget = 1;
    oldTarget = g_acShipTarget_00495f20[0];
    if (oldTarget != -1 &&
        g_aeSpecialManeuver_00495600[oldTarget] ==
            SPECIAL_MANEUVER_UNKNOWN_9) {
        g_acShipTarget_00495f20[0] = -1;
        oldTarget = -1;
    }
    if (g_bTargetLockMode_00493500 != 0 &&
        (short)(g_nRenderedSpaceFrame_00493138 % 8) == 0 &&
        malf(5) != 0) {
        g_bTargetLockMode_00493500 = 0;
        malf_sound();
    }
    if (oldTarget != -1 &&
        (g_bTargetLockMode_00493500 != 0 ||
         (g_asObjectScreenX_00493598[oldTarget] != (short)0x8001 &&
          (g_bTargetLockMode_00493500 != 0 ||
           g_asShipSide_004955d0[oldTarget] !=
               g_asShipSide_004955d0[0]))))
        return;
    if (oldTarget == -1)
        g_bTargetLockMode_00493500 = 0;

    build_your_target_list(&hasEnemy);
    if (g_cViableTargetCount_00496178 == 0) {
        if (g_bTargetLockMode_00493500 != 0)
            g_acShipTarget_00495f20[0] = oldTarget;
        else
            g_acShipTarget_00495f20[0] = -1;
    } else {
        if (hasEnemy == 0 && oldTarget != -1 &&
            g_asShipSide_004955d0[oldTarget] ==
                g_asShipSide_004955d0[0] &&
            (g_bTargetLockMode_00493500 != 0 ||
             g_asObjectScreenX_00493598[oldTarget] != (short)0x8001)) {
            selectNewTarget = 0;
            g_acShipTarget_00495f20[0] = oldTarget;
        }
        if (selectNewTarget != 0) {
            for (targetIndex = 0;
                 targetIndex < g_cViableTargetCount_00496178 &&
                 g_asShipSide_004955d0[
                     (short)g_acViableTarget_00496180[targetIndex]] ==
                     g_asShipSide_004955d0[0];
                 targetIndex++)
                ;
            g_acShipTarget_00495f20[0] =
                g_acViableTarget_00496180[
                    targetIndex % (short)g_cViableTargetCount_00496178];
        }
    }
    if (g_acShipTarget_00495f20[0] != oldTarget) {
        if (oldTarget != -1 && g_acShipTarget_00495f20[0] == -1)
            g_bTargetLockMode_00493500 = 0;
        g_nTargetLockCountdown_004934ec = -1;
    }
}

/* Function start: 0x43E1B2 */
void update_missile_warning(void)
{
    if (FindMissileTargetingObject(0) != 0) {
        SetCockpitLightBlink(2, 1);
        if (g_nTrainSimActive_0049d758 == 0)
            spacetrack(3, 1, -1);
    } else {
        g_abCockpitLightGoal_005d1eb8[2] = 0;
    }
}

/* Function start: 0x43E297 */
void determine_pilot_hand(void)
{
    short yaw;
    short pitch;

    yaw = g_nYawInput_004931aa / 2;
    pitch = g_nPitchInput_004931a8 / 2;
    if (yaw > 0) {
        g_cPilotHandFrame_005d1c30 =
            (unsigned char)MinShort((short)(yaw + 8), 12);
        return;
    }
    if (yaw < 0) {
        g_cPilotHandFrame_005d1c30 =
            (unsigned char)MinShort((short)(4 - yaw), 8);
        return;
    }
    if (pitch > 0) {
        g_cPilotHandFrame_005d1c30 =
            (unsigned char)MinShort((short)(pitch + 12), 16);
        return;
    }
    if (pitch < 0) {
        g_cPilotHandFrame_005d1c30 =
            (unsigned char)MinShort((short)-pitch, 4);
        return;
    }
    g_cPilotHandFrame_005d1c30 = 0;
}

/* Function start: 0x43E371 */
void DrawPilotHandFrame(void)
{
    short x;
    short y;

    x = (short)(g_stPilotHandOrigin_0049af90.x -
                g_stCockpitViewport_005d2160.left);
    y = (short)(g_stPilotHandOrigin_0049af90.y -
                g_stCockpitViewport_005d2160.top);
    CopyViewportContents(&g_stPilotHandBackgroundViewport_005d2b40,
                         &g_stPilotHandViewport_005d2c70);
    DrawSpriteDefault(&g_stPilotHandViewport_005d2c70, x, y,
                      g_pPilotHandAnimationShape_005d2c64,
                      (short)(signed char)g_cPilotHandFrame_005d1c30);
    DrawSpriteDefault(
        &g_stPilotHandViewport_005d2c70,
        (short)(x + g_asPilotHandOffsets_0049aff8[
            (signed char)g_cPilotHandFrame_005d1c30 * 2]),
        (short)(y + g_asPilotHandOffsets_0049aff8[
            (signed char)g_cPilotHandFrame_005d1c30 * 2 + 1]),
        g_pPilotHandAnimationShape_005d2c64, 0x11);
    CopyViewportContents(&g_stPilotHandViewport_005d2c70,
                         &g_stCockpitViewport_005d2160);
    g_cLastPilotHandFrame_0049aff4 = g_cPilotHandFrame_005d1c30;
}

/* Function start: 0x43F110 */
void CopyTrainSimPilotViewToVdus(void)
{
    Viewport destination;
    ShortRect intersection;
    ShortRect rightBounds;
    ShortRect leftBounds;
    ShortRect cockpitBounds;

    destination = g_stPilotHandBackgroundViewport_005d2b40;
    rightBounds.left = g_stRightVduViewport_005d2b20.left;
    rightBounds.top = g_stRightVduViewport_005d2b20.top;
    rightBounds.right = g_stRightVduViewport_005d2b20.right;
    rightBounds.bottom = g_stRightVduViewport_005d2b20.bottom;
    leftBounds.left = g_stLeftVduViewport_005d2180.left;
    leftBounds.top = g_stLeftVduViewport_005d2180.top;
    leftBounds.right = g_stLeftVduViewport_005d2180.right;
    leftBounds.bottom = g_stLeftVduViewport_005d2180.bottom;
    cockpitBounds.left = g_stCockpitViewport_005d2160.left;
    cockpitBounds.top = g_stCockpitViewport_005d2160.top;
    cockpitBounds.right = g_stCockpitViewport_005d2160.right;
    cockpitBounds.bottom = g_stCockpitViewport_005d2160.bottom;

    if (GetViewportIntersection(&intersection, &cockpitBounds,
                                &rightBounds) != 0) {
        if (g_stTrainSimRightVduSource_0049b2c0.left == 0) {
            g_stTrainSimRightVduSource_0049b2c0 =
                g_stCockpitViewport_005d2160;
            g_stTrainSimRightVduSource_0049b2c0.left = rightBounds.left;
            g_stTrainSimRightVduSource_0049b2c0.right = cockpitBounds.right;
            g_stTrainSimRightVduSource_0049b2c0.top = cockpitBounds.top;
            g_stTrainSimRightVduSource_0049b2c0.bottom = rightBounds.bottom;
        }
        destination.left = (short)(
            g_stTrainSimRightVduSource_0049b2c0.left - cockpitBounds.left);
        destination.bottom = (short)(
            g_stTrainSimRightVduSource_0049b2c0.bottom - cockpitBounds.top);
        CopyViewportContents(&g_stTrainSimRightVduSource_0049b2c0,
                             &destination);
    }

    if (GetViewportIntersection(&intersection, &cockpitBounds,
                                &leftBounds) != 0) {
        if (g_stTrainSimLeftVduSource_0049b2d8.left == 0) {
            g_stTrainSimLeftVduSource_0049b2d8 =
                g_stCockpitViewport_005d2160;
            g_stTrainSimLeftVduSource_0049b2d8.left = cockpitBounds.left;
            g_stTrainSimLeftVduSource_0049b2d8.right = leftBounds.right;
            g_stTrainSimLeftVduSource_0049b2d8.top = cockpitBounds.top;
            g_stTrainSimLeftVduSource_0049b2d8.bottom = leftBounds.bottom;
        }
        destination.left = (short)(
            g_stTrainSimLeftVduSource_0049b2d8.left - cockpitBounds.left);
        destination.bottom = (short)(
            g_stTrainSimLeftVduSource_0049b2d8.bottom - cockpitBounds.top);
        CopyViewportContents(&g_stTrainSimLeftVduSource_0049b2d8,
                             &destination);
    }
}

/* Function start: 0x43E43A */
void animate_pilot(void)
{
    if (g_pPilotHandAnimationShape_005d2c64 != 0) {
        determine_pilot_hand();
        if (g_cLastPilotHandFrame_0049aff4 != g_cPilotHandFrame_005d1c30)
            DrawPilotHandFrame();
    }
}

/* Function start: 0x43E472 */
void ResetPilotHandAnimation(void)
{
    if (g_pPilotHandAnimationShape_005d2c64 != 0) {
        g_cLastPilotHandFrame_0049aff4 = 0xff;
        CopyViewportContents(&DAT_005a6b60, &DAT_005a7550);
        animate_pilot();
    }
}

/* Function start: 0x43E4A8 */
void send_message(short obj, signed char message)
{
    if (g_aeObjectClass_00495328[obj] != OBJECT_CLASS_NULL) {
        if (g_nTrainSimActive_0049d758 == 0) {
            if (g_nYourWingman_0049346c != -1 &&
                g_nYourWingman_0049346c == obj &&
                g_bRadioSilence_0049b780 != 0) {
                g_acShipPendingMessage_00495d98[obj] = -1;
            } else if (obj > -1 && obj < 10 &&
                       g_aeObjectClass_00495328[obj] >=
                           OBJECT_CLASS_SHIP) {
                if (g_acShipPortrait_00495d88[obj] != -1) {
                    g_acShipPendingMessage_00495d98[obj] = message;
                } else if (g_asShipMissionIndex_00495d00[obj] ==
                               g_nHomeMissionShipIndex_005d1e22 ||
                           g_asShipMissionIndex_00495d00[obj] ==
                               g_asShipMissionParameter_00495e00[0]) {
                    g_acShipPendingMessage_00495d98[obj] = message;
                } else if (g_asShipSide_004955d0[obj] ==
                           SIDE_KILRATHI) {
                    g_acShipPendingMessage_00495d98[obj] = message;
                }
            }
        }
    }
}

/* Function start: 0x43E5DA */
void npc_communication(void)
{
    signed char message;
    signed char obj;
    short messageActive;

    if (g_nCannedSceneMode_0049021c == 0 &&
        g_nTrainSimActive_0049d758 == 0) {
        messageActive = message_showing();
        obj = 1;
        while (messageActive == 0 && obj < 10) {
            if (g_aeObjectClass_00495328[(short)obj] >=
                    OBJECT_CLASS_SHIP &&
                g_acShipPendingMessage_00495d98[(short)obj] != -1) {
                message = g_acShipPendingMessage_00495d98[(short)obj];
                vid_equiv((short)obj, (short)message);
                g_acShipPendingMessage_00495d98[(short)obj] = -1;
            }
            messageActive = message_showing();
            obj++;
        }
        if (RandomBelowOrEqual(5000) > 4998 &&
            g_nCommSpeakerObject_0049b794 == -1) {
            obj = 1;
            while (obj < 10) {
                if (g_aeObjectClass_00495328[(short)obj] >=
                        OBJECT_CLASS_SHIP &&
                    g_asShipSide_004955d0[(short)obj] == SIDE_KILRATHI &&
                    (g_aeShipObjective_00495f08[(short)obj] ==
                         OBJECTIVE_ENGAGE_ENEMY ||
                     g_aeShipObjective_00495f08[(short)obj] ==
                         OBJECTIVE_DESTROY_SHIP) &&
                    (g_acShipRating_0059cd80[(short)obj] != -1 ||
                     RandomBelowOrEqual(100) < 20)) {
                    g_acShipPendingMessage_00495d98[(short)obj] =
                        (signed char)(RandomBelowOrEqual(2) + 2);
                    return;
                }
                obj++;
                if (g_nCommSpeakerObject_0049b794 != -1)
                    return;
            }
        }
    }
}

/* Function start: 0x43E870 */
void clear_cockpit_damage(void)
{
    short *damageState;
    short damage;

    damageState = g_asCockpitDamageState_005d1ee8;
    for (damage = 0; damage < 4; damage++, damageState++)
        *damageState = 0;
}

/* Function start: 0x43E8B2 */
void explosion_draw(int useBackgroundViewport)
{
    short damage;
    short x;
    short y;

    if (g_pPendingCockpitDamageShape_0049b03c == 0)
        return;
    for (damage = 0; damage < 4; damage++) {
        if (g_asCockpitDamageState_005d1ee8[damage] == 1) {
            switch (damage) {
            case 0:
                g_bCockpitDamageFrame0Shown_0049b2b4 = 1;
                break;
            case 2:
                g_bCockpitDamageFrame2Shown_0049b2b8 = 1;
                break;
            case 3:
                g_bFuelGaugeDamaged_0049b054 = 1;
                break;
            }
            x = g_aCockpitDamagePositions_0049ae98[damage].x;
            y = g_aCockpitDamagePositions_0049ae98[damage].y;
            if (useBackgroundViewport == 0) {
                DrawSpriteDefault(
                    &g_stScreenViewport_005d21a0, x, y,
                    g_pPendingCockpitDamageShape_0049b03c, damage);
            } else {
                DrawSpriteDefault(
                    &g_stCockpitBackgroundViewport_005d1050, x, y,
                    g_pPendingCockpitDamageShape_0049b03c, damage);
            }
        }
    }
    FreePacketAndClear(&g_pPendingCockpitDamageShape_0049b03c, 0);
}

/* Function start: 0x43E9E2 */
void DrawPendingCockpitDamage(void)
{
    if (g_pPendingCockpitDamageShape_0049b03c == 0)
        return;
    switch (g_nPendingCockpitDamage_005d1c34) {
    case 0:
        g_bCockpitDamageFrame0Shown_0049b2b4 = 1;
        break;
    case 2:
        g_bCockpitDamageFrame2Shown_0049b2b8 = 1;
        break;
    case 3:
        g_bFuelGaugeDamaged_0049b054 = 1;
        break;
    }
    DrawSpriteDefault(&g_stScreenViewport_005d21a0,
                      g_stCockpitExplosionPosition_005d1e98.x,
                      g_stCockpitExplosionPosition_005d1e98.y,
                      g_pPendingCockpitDamageShape_0049b03c,
                      g_nPendingCockpitDamage_005d1c34);
    if (g_pPilotHandAnimationShape_005d2c64 != 0) {
        DrawSpriteDefault(
            &g_stPilotHandBackgroundViewport_005d2b40,
            (short)(g_stCockpitExplosionPosition_005d1e98.x -
                    g_stCockpitViewport_005d2160.left),
            (short)(g_stCockpitExplosionPosition_005d1e98.y -
                    g_stCockpitViewport_005d2160.top),
            g_pPendingCockpitDamageShape_0049b03c,
            g_nPendingCockpitDamage_005d1c34);
    }
    FreePacketAndClear(&g_pPendingCockpitDamageShape_0049b03c, 0);
}

/* Function start: 0x43EAE3 */
void RestoreCockpitExplosionBackground(void)
{
    if (IsCockpitExplosionActive() &&
        g_pCockpitExplosionShape_0049b048 != 0 &&
        g_pCockpitExplosionBackground_0049b040 != 0) {
        RestoreSpriteBackground(
            &g_stScreenViewport_005d21a0,
            g_pCockpitExplosionBackground_0049b040,
            g_stCockpitExplosionPosition_005d1e98.x,
            g_stCockpitExplosionPosition_005d1e98.y,
            g_pCockpitExplosionShape_0049b048,
            g_nCockpitExplosionFrame_0049b04c);
    }
}

/* Function start: 0x43EB46 */
void cockpit_explosion(void)
{
    if (g_nCockpitExplosionFrame_0049b04c == 0x7fff)
        g_nCockpitExplosionFrame_0049b04c = 0;
    if (IsCockpitExplosionActive()) {
        if (g_nCockpitExplosionFrame_0049b04c == 0)
            PlaySfxWaveFileByNumber(4, -1, 0);
        if (++g_nCockpitExplosionFrame_0049b04c == 3)
            DrawPendingCockpitDamage();
        if (IsCockpitExplosionActive() &&
            g_pCockpitExplosionShape_0049b048 != 0 &&
            g_pCockpitExplosionBackground_0049b040 != 0) {
            CaptureSpriteBackground(
                &g_stScreenViewport_005d21a0,
                g_pCockpitExplosionBackground_0049b040,
                g_stCockpitExplosionPosition_005d1e98.x,
                g_stCockpitExplosionPosition_005d1e98.y,
                g_pCockpitExplosionShape_0049b048,
                g_nCockpitExplosionFrame_0049b04c);
            DrawSpriteDefault(&g_stScreenViewport_005d21a0,
                              g_stCockpitExplosionPosition_005d1e98.x,
                              g_stCockpitExplosionPosition_005d1e98.y,
                              g_pCockpitExplosionShape_0049b048,
                              g_nCockpitExplosionFrame_0049b04c);
            g_cLastPilotHandFrame_0049aff4 = 0xff;
        }
        return;
    }
    FreePacketAndClear(
        (unsigned char **)&g_pCockpitExplosionShape_0049b048, 0);
}

/* Function start: 0x43EC50 */
void place_damage_on_cockpit(short damage)
{
    if (g_nCurrentView_00492fa8 == 0 &&
        g_asCockpitDamageState_005d1ee8[damage] == 0) {
        g_asCockpitDamageState_005d1ee8[damage] = 1;
        g_nPendingCockpitDamage_005d1c34 = damage;
        if (g_pCockpitExplosionShape_0049b048 == 0) {
            explosion_draw(0);
        } else if (IsCockpitExplosionActive() == 0) {
            g_stCockpitExplosionPosition_005d1e98 =
                g_aCockpitDamagePositions_0049ae98[damage];
            g_nCockpitExplosionFrame_0049b04c = 0x7fff;
        }
    }
}

/* Function start: 0x43ECD9 */
void vid_transmit(void)
{
    short randomFrame;
    short speaker;
    unsigned char *background;

    speaker = g_nCommSpeakerObject_0049b794;
    if (g_asShipSide_004955d0[speaker] == SIDE_NEUTRAL) {
        EndCommSessionWithWingman();
        return;
    }
    if ((g_nCockpitDisplayMode_0049d71c != 0 ||
         g_nRenderedSpaceFrame_00493138 % 2 != 0) &&
        g_nCommPortraitIndex_0049b79c != -1 &&
        g_aapszPilotSpeech_0059e220[g_nCommPortraitIndex_0049b79c] != 0 &&
        g_bVideoImagesSuppressed_0049b784 == 0) {
        if (g_aeSpecialManeuver_00495600[speaker] ==
            SPECIAL_MANEUVER_UNKNOWN_9) {
            if (g_asShipSide_004955d0[speaker] == SIDE_IMPERIAL) {
                DrawSpriteDefault(
                    &g_stRightVduViewport_005d2b20, g_stRightVduViewport_005d2b20.left, g_stRightVduViewport_005d2b20.top,
                    g_pCommStaticShape_0046927c_WC1_UNMAPPED,
                    (short)(g_asObjectCounter_00494be0[speaker] / 5));
                return;
            }
            DrawSpriteDefault(&g_stRightVduViewport_005d2b20, g_stRightVduViewport_005d2b20.left,
                              g_stRightVduViewport_005d2b20.top,
                              g_pCommStaticShape_0046927c_WC1_UNMAPPED, 2);
            return;
        }
        if (g_nCommPortraitFrame_0049b2bc == -1) {
            g_nCommPortraitFrame_0049b2bc =
                (unsigned short)RandomInRange(0, 2);
        }
        randomFrame = RandomInRange(0, 3);
        if (randomFrame < 3)
            g_nCommPortraitFrame_0049b2bc = randomFrame;
        set_new_vdu(1);
        if (g_asShipSide_004955d0[g_nCommSpeakerObject_0049b794] ==
            SIDE_IMPERIAL) {
            background = g_pConfedCommBackground_00469278_WC1_UNMAPPED;
        } else {
            background = g_pKilrathiCommBackground_00469280;
        }
        DrawSpriteDefault(&g_stRightVduViewport_005d2b20, g_stRightVduViewport_005d2b20.left,
                          g_stRightVduViewport_005d2b20.top, background, 0);
        DrawSpriteDefault(
            &g_stRightVduViewport_005d2b20, g_stRightVduViewport_005d2b20.left, g_stRightVduViewport_005d2b20.top,
            g_apCommPortraitShapes_0059e180[g_nCommPortraitIndex_0049b79c],
            (short)g_nCommPortraitFrame_0049b2bc);
    }
}

/* Function start: 0x43F042 */
void vid_equiv(short obj, short message)
{
    if (get_mode(1) != 4 &&
        g_nTrainSimActive_0049d758 == 0 &&
        g_nCannedSceneMode_0049021c == 0 && g_nCurrentView_00492fa8 == 0 &&
        message_showing() == 0)
        real_vid_transmit(obj, message);
}

/* Function start: 0x43F09D */
void update_dead_disp(short a)
{
    malf_noise(a, 1, g_abGamePaletteReservedColours_0049cb54[12], 0x17, 0);
}

/* Function start: 0x43F0C3 */
void check_stranded(void)
{
    if (g_nTrainSimActive_0049d758 == 0 &&
        g_aMissionShips_00492290[
            g_nHomeMissionShipIndex_005d1e22].state == 3 &&
        any_enemy(0, 30000) == 0)
        g_nArcadeState_0049d75c = 3;
}

/* Function start: 0x43F2F5 */
void update_VDUs(void)
{
    if (g_nCurrentView_00492fa8 == 4)
        return;
    if (g_nCockpitDisplayMode_0049d71c != 0) {
        if (IsCockpitWeaponShapeLoaded() != 0) {
            DrawFilledViewportRect(
                &g_stScreenViewport_005d21a0,
                g_stLeftVduViewport_005d2180.left,
                g_stLeftVduViewport_005d2180.top,
                g_stLeftVduViewport_005d2180.right,
                g_stLeftVduViewport_005d2180.bottom, 0);
            DrawFilledViewportRect(
                &g_stScreenViewport_005d21a0,
                g_stRightVduViewport_005d2b20.left,
                g_stRightVduViewport_005d2b20.top,
                g_stRightVduViewport_005d2b20.right,
                g_stRightVduViewport_005d2b20.bottom, 0);
        }
    }
    if (IsCockpitWeaponShapeLoaded() != 0) {
        SetTextContext(&g_stLeftVduTextContext_005d2ae0);
        if (update_vid_disp(0) != 0) {
            switch (get_mode(0)) {
            case 0:
                update_dead_disp(0);
                break;
            case 1:
                show_weapon_disp();
                break;
            case 2:
                show_damage_disp();
                break;
            }
        } else {
            switch (get_mode(0)) {
            case 0:
                update_dead_disp(0);
                break;
            case 1:
                if (g_nCockpitDisplayMode_0049d71c != 0)
                    show_weapon_disp();
                update_status_text();
                break;
            case 2:
                if (g_nCockpitDisplayMode_0049d71c != 0) {
                    g_bForceDamageDisplayRedraw_0049b2ec = 1;
                    show_damage_disp();
                    g_bForceDamageDisplayRedraw_0049b2ec = 0;
                }
                UpdateDamageDisplay();
                break;
            }
        }
        if (get_mode(0) == 0)
            g_aHudMessageSlots_005d1d40[0].text = 0;
        else
            UpdateMessage(&g_aHudMessageSlots_005d1d40[0]);
    }

    SetTextContext(&g_stRightVduTextContext_005d2ce0);
    if (update_vid_disp(1) != 0) {
        switch (get_mode(1)) {
        case 0:
            update_dead_disp(1);
            break;
        case 3:
            show_target_disp();
            break;
        case 4:
            show_communications_disp();
            break;
        case 5:
            show_navigation_disp();
            break;
        case 6:
            vid_transmit();
            break;
        }
    } else {
        switch (get_mode(1)) {
        case 0:
            update_dead_disp(1);
            break;
        case 3:
            if (g_nCockpitDisplayMode_0049d71c != 0)
                show_target_disp();
            DrawTargetRangeReadout();
            break;
        case 4:
            if (g_nCockpitDisplayMode_0049d71c != 0)
                show_communications_disp();
            RefreshCommunicationMenu();
            break;
        case 5:
            if (g_nCockpitDisplayMode_0049d71c != 0)
                show_navigation_disp();
            check_objectives();
            break;
        case 6:
            vid_transmit();
            break;
        }
    }
    if (get_mode(1) == 6)
        g_aHudMessageSlots_005d1d40[1].text = 0;
    else {
        if (get_mode(1) == 0)
            g_aHudMessageSlots_005d1d40[1].text = 0;
        else
            UpdateMessage(&g_aHudMessageSlots_005d1d40[1]);
    }
    if (g_nCockpitDisplayMode_0049d71c == 0 &&
        g_pPilotHandAnimationShape_005d2c64 != 0)
        CopyTrainSimPilotViewToVdus();
}

/* Function start: 0x43F6C9 */
void update_cockpit(void)
{
    check_target();
    repair_internal_damage();
    if (g_cMissionObjectiveCount_00493294 != 0)
        update_objective_location(
            (short)(g_nSpaceFrame_00493134 %
                    (short)g_cMissionObjectiveCount_00493294));
    if (g_nCurrentView_00492fa8 == 0) {
        if (g_nCockpitDisplayMode_0049d71c == 0)
            RestoreCockpitExplosionBackground();
        update_lights();
        update_missile_warning();
        if (g_bCockpitDamageFrame0Shown_0049b2b4 == 0)
            draw_3d_scanner();
        else
            set_objective_range(0);
        update_digital_readouts();
        if (g_nCockpitDisplayMode_0049d71c == 0)
            animate_pilot();
        if (g_bCockpitDamageFrame2Shown_0049b2b8 == 0)
            update_bars();
        draw_cockpit_lights();
        if (g_nCockpitDisplayMode_0049d71c == 0)
            cockpit_explosion();
        npc_communication();
        update_VDUs();
    } else if (g_nCurrentView_00492fa8 == 4) {
        if (g_nTargetCameraMode_005c8d50 == 1) {
            if (g_nTargetCameraFrame_0049d3e8 == 1)
                UpdateTargetCameraTracking();
            vdu_polygon(1, g_nTargetCameraZoom_0049d3e4);
        } else {
            vdu_polygon(0, g_nGunDisplayEnergyPercent_005c8d4e);
        }
        draw_3d_scanner();
        if (g_nCockpitDisplayMode_0049d71c == 0)
            UpdateTargetCameraCockpitHook();
    }
    fire_computer_graphic_missile();
    check_stranded();
}

/* Function start: 0x43F81B */
short IsCockpitWeaponShapeLoaded(void)
{
    return g_pCockpitWeaponShape_005d2b54 != 0;
}

/* Function start: 0x43F849 */
void PlayCockpitSelectionSfx(short selectionSound)
{
    (void)selectionSound;
    PlaySfxWaveFileByNumber(0x19, -1, 0);
}

/* Function start: 0x43F862 */
void vdu_pop_all(short vdu)
{
    int mode;

    while ((short)GetVduModeStackDepth(vdu) > 0) {
        mode = get_mode(vdu);
        if (mode != 6)
            pop_mode(vdu);
        else
            EndCommMenu();
    }
}

/* Function start: 0x43F8CD */
void SelectCockpitVduMode(short vdu, int mode)
{
    short changed;

    if (vdu == 1 && mode == 9 && get_mode(vdu) == mode)
        return;
    if (g_nCurrentView_00492fa8 == 0) {
        if (malf(3) != 0 ||
            (mode == 4 && malf(4) != 0)) {
            vdu_malf(vdu, 0x17);
            return;
        }
        PlayCockpitSelectionSfx(g_asVduSelectionSound_0049afe4[vdu]);
        changed = get_mode(vdu) != mode;
        if (changed != 0) {
            vdu_pop_all(vdu);
            InvalidateVduMode(vdu);
            if (mode != 4)
                set_mode(vdu, mode);
            else
                show_communications_disp();
            update_VDUs();
            return;
        }
        switch (mode) {
        case 0:
            break;
        case 2:
            g_nDamageDisplayTicks_005d4250 = 0;
            break;
        case 4:
            RefreshCommunicationMenu();
            break;
        case 5:
            InflightComputer();
            break;
        case 3:
            cycle_onscreen_targets();
            break;
        case 9:
            break;
        case 1:
            if (g_cCurrentKey_00493128 == 0x22) {
                select_new_gun();
            } else if (g_cCurrentKey_00493128 == 0x11) {
                lock_off();
                select_new_release_weapon((enum ObjectType)-1);
            }
            break;
        }
    }
}
