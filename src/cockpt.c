/*
 *  Cockpit displays, HUD messages and VDUs.
 *
 *  Retail object 0x4380B0-0x43FABF (link order; see docs/ORDER.md).
 */
#include "game.h"

short g_bTargetLockActive_0049ae80 = 0;
short g_nCommDeathSequenceFrame_0049ae84 = 0;
short g_nCommMessageIndex_0049ae88 = -1;
ShortPoint g_stWeaponDisplayOffset_0049ae8c = { 0, 0x10 };
ShortPoint g_stHudMessageOrigin_0049ae90 = { 0 };
ShortPoint g_aCockpitDamagePositions_0049ae98[4] = { 0 };
short g_asCockpitLightX_0049aea8[7] = { 0 };
short g_asCockpitLightY_0049aeb8[7] = { 0 };
signed char g_acCockpitLightOffFrame_0049aec8[7] = { 0 };
signed char g_acCockpitLightOnFrame_0049aed0[7] = { 0 };
CockpitBarDefinition g_aCockpitBarDefinitions_0049aed8[8] = { 0 };
ShortPoint g_aCockpitReadoutOrigins_0049af58[4] = { 0 };
ShortRect g_stLeftVduBounds_0049af68 = { 0 };
ShortRect g_stRightVduBounds_0049af70 = { 0 };
CockpitScannerGeometry g_stCockpitScannerGeometry_0049af78 = { 0 };
ShortRect g_stPilotHandBounds_0049af88 = { 0 };
ShortPoint g_stPilotHandOrigin_0049af90 = { 0 };
signed char g_cHazardBaseTravelTime_0049af94 = 0;
short g_nHazardPitchRange_0049af98 = 0;
ShortPoint g_aDefaultWeaponDisplayPositions_0049afa0[16] = { 0 };
short g_bTargetLockDisplayEnabled_0049afe0 = 0;
short g_asVduSelectionSound_0049afe4[2] = { 0x7f, 0 };
short g_nTargetLeadIndicatorX_0049afe8 = (short)0x8001;
char *g_pszPendingHudMessage_0049afec = 0;
char *g_pszDisplayedHudMessage_0049aff0 = 0;
signed char g_cLastPilotHandFrame_0049aff4 = (signed char)0xff;
short g_asPilotHandOffsets_0049aff8[34] = {
    6, -3, 7, 2, 7, 9, 7, 12, 8, 13, 0, -1, -1, -1,
    -4, -1, -6, -1, 6, 0, 8, 0, 10, 0, 13, 3, 8, -7,
    6, -9, 5, -11, 5, -14
};
unsigned char *g_pPendingCockpitDamageShape_0049b03c = 0;
unsigned char *g_pCockpitExplosionBackground_0049b040 = 0;
unsigned char *g_pCockpitHudBackground_0049b044 = 0;
unsigned char * volatile g_pCockpitExplosionShape_0049b048 = 0;
volatile short g_nCockpitExplosionFrame_0049b04c = 8;
char *g_pszAutopilotWaitReason_0049b050 = g_szAutopilotWaitReason_0049b2f0;
short g_bFuelGaugeDamaged_0049b054 = 0;
short g_bForceCriticalDamageWarning_0049b058 = 0;
unsigned char *g_pReleaseWeaponDisplayBackground_0049b05c = 0;
signed char g_cReleaseWeaponDisplayFrame_0049b060 = -1;
signed char g_cReleaseWeaponDisplayTicks_0049b064 = 0;
signed char g_cReleaseWeaponDisplayState_0049b068 = 0;
static char g_szNoWeaponSelected_0049b070[8] = "None";
int g_nDisplayedObjectiveRange_0049b078 = 40000;
short g_nScannerTargetObject_0049b07c = -1;
int g_aiForwardScannerGridRows_0049b080[78] = {
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
int g_aiTargetCameraScannerGridRows_0049b1b8[50] = {
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
const char *g_pszMissileLocked_0049b280 = "MISSILE LOCKED ";
signed char g_bTargetBracketVisible_0049b284 = 1;
unsigned char *g_pHudMessageFrameShape_0049b288 = 0;
unsigned char *g_pHudMessageBackground_0049b28c = 0;
short g_bCaptureHudMessageBackground_0049b290 = 1;
short g_nHudMessageBackgroundDepth_0049b294 = 0;
short g_nTargetLockMarkerX_0049b298 = -0x7fff;
ShortRect g_stTargetBracketBounds_0049b2a0 = { -0x7fff, 0, 0, 0 };
ShortRect g_stCommTargetBracketBounds_0049b2a8 = { -0x7fff, 0, 0, 0 };
short g_bMissileLockAcquired_0049b2b0 = 0;
short g_bCockpitDamageFrame0Shown_0049b2b4 = 0;
short g_bCockpitDamageFrame2Shown_0049b2b8 = 0;
int g_nCommPortraitFrame_0049b2bc = -1;
Viewport g_stTrainSimRightVduSource_0049b2c0 = { 0 };
Viewport g_stTrainSimLeftVduSource_0049b2d8 = { 0 };
int g_bForceDamageDisplayRedraw_0049b2ec = 0;
char g_szAutopilotWaitReason_0049b2f0[28] = "Wait for ??????????????????";


/* Clearing the communication resources sets the speaker to -1.  WC2's
 * unchecked side-table load then treats the last two bytes of
 * g_acShipLastAttacker_004955c0 as a signed word.  Reassemble that
 * layout-derived value without crossing a native global's sanitizer redzone. */
#ifdef SDL_PORT
#define COMM_SPEAKER_SIDE(obj) \
    ((obj) != -1 \
         ? g_asShipSide_004955d0[obj] \
         : (short)((unsigned short)(unsigned char) \
                       g_acShipLastAttacker_004955c0[14] | \
                   ((unsigned short)(unsigned char) \
                        g_acShipLastAttacker_004955c0[15] << 8)))
#else
#define COMM_SPEAKER_SIDE(obj) g_asShipSide_004955d0[obj]
#endif

/* Function start: 0x4380B0 */
void FatalErrorAndExit(const char *format, ...)
{
    char text[0xfc];
    va_list argList;

    va_start(argList, format);
    vsprintf(text, format, argList);
    va_end(argList);
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
    short barLeft;
    unsigned char *sprite;
    short length;
    short tmp;
    short barDirection;
    short barTop;
    signed char frameNumberFilled;
    short barRight;
    short barBottom;
    signed char frameNumberEmpty;

    if (g_nCurrentView_00492fa8 == 0) {
        definition = g_aCockpitBarDefinitions_0049aed8[(int)bar];
        sprite = g_pCockpitPanelShape_005d2cd4;
    } else {
        definition = ((CockpitBarDefinition (*)[2])
            g_abGunDisplayConfiguration_0049d340)[
            g_nGunDisplaySlot_005c8dc0][(int)bar];
        sprite = g_apCockpitVduOverlayShapes_0049a5f8[
            g_nGunDisplaySlot_005c8dc0];
    }
    length = definition.length;
    percent = (short)(((int)percent * (int)length) / 100);
    g_stCockpitMeterPort_005d21e0.left = definition.left;
    barLeft = g_stCockpitMeterPort_005d21e0.left;
    if (barLeft == -99)
        return;
    g_stCockpitMeterPort_005d21e0.right = definition.right;
    barRight = g_stCockpitMeterPort_005d21e0.right;
    g_stCockpitMeterPort_005d21e0.top = definition.top;
    barTop = g_stCockpitMeterPort_005d21e0.top;
    g_stCockpitMeterPort_005d21e0.bottom = definition.bottom;
    barBottom = g_stCockpitMeterPort_005d21e0.bottom;
    frameNumberEmpty = (signed char)definition.emptyFrame;
    frameNumberFilled = (signed char)definition.filledFrame;
    barDirection = definition.direction;
    if (barDirection < 2) {
        if (barDirection == 1) {
            percent = (short)(length - percent);
            tmp = frameNumberEmpty;
            frameNumberEmpty = frameNumberFilled;
            frameNumberFilled = (signed char)tmp;
        }
        g_stCockpitMeterPort_005d21e0.bottom -= percent;
        if (g_stCockpitMeterPort_005d21e0.top <=
            g_stCockpitMeterPort_005d21e0.bottom)
            DrawSpriteDefault(&g_stCockpitMeterPort_005d21e0, barLeft, barTop,
                              sprite, frameNumberFilled);
        g_stCockpitMeterPort_005d21e0.top =
            (short)(g_stCockpitMeterPort_005d21e0.bottom + 1);
        g_stCockpitMeterPort_005d21e0.bottom = barBottom;
        if (g_stCockpitMeterPort_005d21e0.top <=
            g_stCockpitMeterPort_005d21e0.bottom)
            DrawSpriteDefault(&g_stCockpitMeterPort_005d21e0, barLeft, barTop,
                              sprite, frameNumberEmpty);
    } else {
        if (barDirection == 3) {
            percent = (short)(length - percent);
            tmp = frameNumberEmpty;
            frameNumberEmpty = frameNumberFilled;
            frameNumberFilled = (signed char)tmp;
        }
        g_stCockpitMeterPort_005d21e0.right -= percent;
        if (g_stCockpitMeterPort_005d21e0.left <=
            g_stCockpitMeterPort_005d21e0.right)
            DrawSpriteDefault(&g_stCockpitMeterPort_005d21e0, barLeft, barTop,
                              sprite, frameNumberFilled);
        g_stCockpitMeterPort_005d21e0.left =
            (short)(g_stCockpitMeterPort_005d21e0.right + 1);
        g_stCockpitMeterPort_005d21e0.right = barRight;
        if (g_stCockpitMeterPort_005d21e0.left <=
            g_stCockpitMeterPort_005d21e0.right)
            DrawSpriteDefault(&g_stCockpitMeterPort_005d21e0, barLeft, barTop,
                              sprite, frameNumberEmpty);
    }
}

/* Function start: 0x438454 */
unsigned int InitializeCockpitReadout(signed char slot,
                                      TextContext *context)
{
    g_aConsoleMetersTable_005d1e30[(int)slot].context = context;
    g_aConsoleMetersTable_005d1e30[(int)slot].x = context->cursorX;
    g_aConsoleMetersTable_005d1e30[(int)slot].y = context->cursorY;
    g_aConsoleMetersTable_005d1e30[(int)slot].previousRight = 0;
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x4384AD */
void DrawCockpitReadout(signed char slot, const char *text)
{
    if (g_aConsoleMetersTable_005d1e30[(int)slot].x == -99)
        return;
    SetTextContext(g_aConsoleMetersTable_005d1e30[(int)slot].context);
    SetTextCursor((unsigned short)g_aConsoleMetersTable_005d1e30[(int)slot].x,
                  (unsigned short)g_aConsoleMetersTable_005d1e30[(int)slot].y);
    DrawFormattedText(text);
    EraseCockpitReadoutRegion(
        &g_stScrViewInfo_005d21a0,
        g_aConsoleMetersTable_005d1e30[(int)slot].context->cursorX,
        g_aConsoleMetersTable_005d1e30[(int)slot].y,
        g_aConsoleMetersTable_005d1e30[(int)slot].previousRight,
        (short)(*(unsigned short *)
                    g_aConsoleMetersTable_005d1e30[(int)slot].context->font +
                g_aConsoleMetersTable_005d1e30[(int)slot].y - 1),
        g_cSecondaryViewBufferColour_0049cb4c);
    g_aConsoleMetersTable_005d1e30[(int)slot].previousRight =
        g_aConsoleMetersTable_005d1e30[(int)slot].context->cursorX;
}

/* Function start: 0x4385B3 */
short DrawHudMessageSlot(HudMessageSlot *slot)
{
    short textColour;
    short previousDrawColour;
    short blinkOff;

    if (g_nCurrentView_00492fa8 != 0)
        return 1;

    previousDrawColour = slot->drawColour;
    if (((int)g_nInputClock_005c84a8 / 40) % 3 == 0)
        slot->drawColour = g_cSecondaryViewBufferColour_0049cb4c;
    else
        slot->drawColour = slot->colour;

    if (slot->flashCount != -1) {
        if (slot->drawColour == g_cSecondaryViewBufferColour_0049cb4c &&
            previousDrawColour == slot->colour) {
            slot->flashCount = (signed char)MaxShort(
                0, (short)(slot->flashCount - 1));
        }
        if (slot->flashCount == 0)
            slot->drawColour = g_cSecondaryViewBufferColour_0049cb4c;
    }

    blinkOff = slot->drawColour == g_cSecondaryViewBufferColour_0049cb4c;
    textColour = slot->context->colour;
    slot->context->colour = (unsigned char)slot->drawColour;
    DrawTextAt(slot->context, slot->x, slot->y, slot->text, 2);
    slot->context->colour = (unsigned char)textColour;
    return blinkOff;
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
    g_aHudTextSlotsArray_005d1d40[0].text = 0;
    g_aHudTextSlotsArray_005d1d40[0].flashCount = 0;
    g_aHudTextSlotsArray_005d1d40[1].text = 0;
    g_aHudTextSlotsArray_005d1d40[1].flashCount = 0;
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
    slot->colour = colour;
    slot->drawColour = colour;
    slot->text = text;
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
    SetHudMessageSlot(&g_aHudTextSlotsArray_005d1d40[1], &g_stRightVduTextContextBlock_005d2ce0,
                      g_stRightVduView_005d2b20.left,
                      (short)(g_stRightVduView_005d2b20.bottom - 6),
                      text, colour, (signed char)flashCount);
}

/* Function start: 0x4388F5 */
void CockpitMessage(const char *text, unsigned short colour,
                    int flashCount)
{
    if (text != g_aHudTextSlotsArray_005d1d40[1].text)
        set_global_message(text, colour, flashCount);
}

/* Function start: 0x438923 */
/* Nothing in the shipped executable calls this.  Clears the global message
 * slot. */
void ClearGlobalMessage(void)
{
    ClearHudMessageSlot(&g_aHudTextSlotsArray_005d1d40[1]);
}

/* Function start: 0x43893F */
void remove_message(const char *text)
{
    ClearHudMessageIfMatching(&g_aHudTextSlotsArray_005d1d40[1], text);
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
    MissionObjective *record;

    record = &g_aMissionObjectives_004932a8[objectiveIndex];
    switch (record->type) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 6:
        navPoint = find_ships_sphere(record->index);
        if (navPoint == -1)
            return 0;
        break;
    case 0:
    case 5:
        navPoint = record->index;
        break;
    }
    if (g_aMissionNavPoints_00491e98[navPoint].systemIndex ==
        g_nCurStellarSystem_005d169c)
        return 0;
    else
        return 1;
}

/* Function start: 0x438ADD */
short auto_pilot_valid(short showReason)
{
    const char *reason;
    short range;
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
    range = distance_from_point(
        0, &g_aMissionObjectives_004932a8[
               (signed char)g_cCurrentObjective_004931cc].position);
    for (object = 1; object < 10; object++) {
        if (g_aeObjectClass_00495328[object] >= OBJECT_CLASS_CAPITAL_SHIP &&
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
    }
    if (range < 8000) {
        reason = "Already Near";
    } else if (kilrathi_near(0, 16000) != 0) {
        reason = "Enemy Near";
    } else if (g_pActiveHazardField_00493278 != 0) {
        reason = "Hazard Near";
    }
    if (showReason != 0 && reason != 0)
        CockpitMessage(reason, g_cYellowColour_0049cb58, 3);
    return reason == 0;
}

/* Function start: 0x438CEB */
void reset_cockpit(void)
{
    short light;

    for (light = 0; light < 7; light++) {
        g_abCockpitLightMode_005d1e70[light] = 0;
        g_abCockpitLightGoalTable_005d1eb8[light] = 0;
    }
}

/* Function start: 0x438D30 */
void SetCockpitLightBlink(signed char light, short interval)
{
    if (interval < 20) {
        if (interval == 0) {
            g_abCockpitLightGoalTable_005d1eb8[(int)light] ^= 1;
        } else if (g_nSpaceFrame_00493134 % interval == 0) {
            g_abCockpitLightGoalTable_005d1eb8[(int)light] ^= 1;
        }
    } else {
        g_abCockpitLightGoalTable_005d1eb8[(int)light] = 0;
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
            g_abCockpitLightGoalTable_005d1eb8[4] = 1;
        else
            g_abCockpitLightGoalTable_005d1eb8[4] = 0;
    }
    for (light = 0; light < 7; light++) {
        if (g_nCockpitDisplayMode_0049d71c != 0) {
            x = g_asCockpitLightX_0049aea8[(int)light];
            y = g_asCockpitLightY_0049aeb8[(int)light];
            if (g_abCockpitLightGoalTable_005d1eb8[(int)light] == 1)
                frame = g_acCockpitLightOnFrame_0049aed0[(int)light];
            else
                frame = g_acCockpitLightOffFrame_0049aec8[(int)light];
            DrawSpriteDefault(&g_stScrViewInfo_005d21a0, x, y,
                              g_pCockpitPanelShape_005d2cd4, frame);
            g_abCockpitLightMode_005d1e70[(int)light] =
                g_abCockpitLightGoalTable_005d1eb8[(int)light];
        } else if (g_abCockpitLightMode_005d1e70[(int)light] !=
                   g_abCockpitLightGoalTable_005d1eb8[(int)light]) {
            x = g_asCockpitLightX_0049aea8[(int)light];
            y = g_asCockpitLightY_0049aeb8[(int)light];
            if (g_abCockpitLightGoalTable_005d1eb8[(int)light] == 1)
                frame = g_acCockpitLightOnFrame_0049aed0[(int)light];
            else
                frame = g_acCockpitLightOffFrame_0049aec8[(int)light];
            DrawSpriteDefault(&g_stScrViewInfo_005d21a0, x, y,
                              g_pCockpitPanelShape_005d2cd4, frame);
            g_abCockpitLightMode_005d1e70[(int)light] =
                g_abCockpitLightGoalTable_005d1eb8[(int)light];
        }
    }
}

/* Function start: 0x438F62 */
void update_lights(void)
{
    short energy;
    short fuelPercent;

    if (g_bFuelGaugeDamaged_0049b054 == 0) {
        fuelPercent = (short)(
            (g_anShipFuel_00495638[0] * 100) /
            *(int *)&g_aObjectTypeData_00496d30[0].lifetime);
        SetCockpitLightBlink(6, fuelPercent);
        vdu_polygon(0, fuelPercent);
    }
    energy = g_asShipWeaponEnergy_00495590[0];
    vdu_polygon(1, energy);
    if ((calculate_damage_level() >= 3 &&
         (int)g_aasShipShield_00495518[0][1] +
                 (int)g_aasShipShield_00495518[0][0] <
             10) ||
        g_bForceCriticalDamageWarning_0049b058 != 0) {
        SetCockpitLightBlink(3, 2);
        if (g_nSevereDmgWarningSfxHandle_005d1ec0 == 0 ||
            g_nSpaceFrame_00493134 % 10 == 0)
            PlaySfxWaveFileByNumber(0x20, -1, 0);
    } else if (g_nSevereDmgWarningSfxHandle_005d1ec0 != 0) {
        FlushSoundEffectsAndLog(
            g_nSevereDmgWarningSfxHandle_005d1ec0, 1);
        g_nSevereDmgWarningSfxHandle_005d1ec0 = 0;
        g_abCockpitLightGoalTable_005d1eb8[3] = 0;
    }
}

/* Function start: 0x439076 */
void update_bars(void)
{
    ObjectTypeData *typeData;
    short ratio;
    short shieldPercent;
    short shieldShown;

    typeData = &g_aObjectTypeData_00496d30[g_acObjectType_00493980[0]];
    ratio = (short)((g_aasShipArmor_00495540[0][0] * 100) /
                         typeData->armorFront);
    vdu_polygon(2, ratio);
    ratio = (short)((g_aasShipArmor_00495540[0][1] * 100) /
                         typeData->armorRear);
    vdu_polygon(3, ratio);
    ratio = (short)((g_aasShipArmor_00495540[0][2] * 100) /
                         typeData->armorRight);
    vdu_polygon(4, ratio);
    ratio = (short)((g_aasShipArmor_00495540[0][3] * 100) /
                         typeData->armorLeft);
    vdu_polygon(5, ratio);
    shieldPercent = (short)((g_aasShipShield_00495518[0][0] * 100) /
                            typeData->shieldFore);
    SetCockpitLightBlink(0, shieldPercent);
    vdu_polygon(6, shieldPercent);
    shieldShown = g_aasShipShield_00495518[0][0];
    if (g_asObjectType_00495298[0] == 0x33)
        shieldShown = (short)(shieldShown / 10);
    DrawCockpitReadout(
        4, itoa((int)shieldShown,
                 g_szTextBufferBuffer_005d1c40, 10));
    shieldPercent = (short)((g_aasShipShield_00495518[0][1] * 100) /
                            typeData->shieldAft);
    SetCockpitLightBlink(1, shieldPercent);
    vdu_polygon(7, shieldPercent);
    shieldShown = g_aasShipShield_00495518[0][1];
    if (g_asObjectType_00495298[0] == 0x33)
        shieldShown = (short)(shieldShown / 10);
    DrawCockpitReadout(
        5, itoa((int)shieldShown,
                 g_szTextBufferBuffer_005d1c40, 10));
}

/* Function start: 0x439264 */
short get_mode(short i)
{
#ifdef SDL_PORT
    /* The depth starts at -1 for VDU 0, so until something calls set_mode on
     * it the original indexes one slot in front of the row and reads the last
     * component of g_vPlayerAcceleration_00493488 -- zero at mission init,
     * which is the only time this happens.  Say zero rather than read across.
     */
    if (g_acVduModeStackDepth_004934c8[(int)i] < 0)
        return 0;
#endif
    return (short)g_aaiVduModeStack_00493498[(int)i][
        (int)g_acVduModeStackDepth_004934c8[(int)i]];
}

/* Function start: 0x43928E */
void set_mode(short i, int state)
{
    if (get_mode(i) != state)
        ClearHudMessageSlot(&g_aHudTextSlotsArray_005d1d40[i]);
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
short GetVduModeStackDepth(short i)
{
    return g_acVduModeStackDepth_004934c8[(int)i];
}

/* Function start: 0x439369 */
void push_mode(short i, int state)
{
    ClearHudMessageSlot(&g_aHudTextSlotsArray_005d1d40[i]);
#ifdef SDL_PORT
    /* WC2 pushes without bounding the stack, so a fifth mode writes past the
     * four-deep row and into whatever the linker put next.  Drop the
     * overflowing push on the port instead. */
    if (g_acVduModeStackDepth_004934c8[(int)i] >= 3)
        return;
#endif
    g_acVduModeStackDepth_004934c8[(int)i]++;
    g_aaiVduModeStack_00493498[(int)i][
        (int)g_acVduModeStackDepth_004934c8[(int)i]] = state;
}

/* Function start: 0x4393B3 */
void pop_mode(short i)
{
    ClearHudMessageSlot(&g_aHudTextSlotsArray_005d1d40[i]);
    g_acVduModeStackDepth_004934c8[(int)i]--;
}

/* Function start: 0x4393E1 */
void set_new_vdu(short vdu)
{
    if (get_mode(vdu) == 0) {
        malf_noise(vdu, 1, g_cDarkGreenColour_0049cb60, 0x17, 0);
    } else {
        switch (vdu) {
        case 1:
            ClearViewport(
                &g_stRightVduView_005d2b20,
                g_cSecondaryViewBufferColour_0049cb4c);
            break;
        case 0:
            ClearViewport(
                &g_stLeftwardVduPortBlock_005d2180,
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
    g_nScreenMessageTimeNumber_005d1c32 = 0;
}

/* Function start: 0x43952E */
short message_showing(void)
{
    return g_nScreenMessageTimeNumber_005d1c32 != 0;
}

/* Function start: 0x439559 */
void set_message_time(short duration)
{
    g_nScreenMessageTimeNumber_005d1c32 = duration;
    if (g_nScreenMessageTimeNumber_005d1c32 == -1)
        g_bSpeechPlaybackDone_004a266c = 0;
}

/* Function start: 0x439588 */
void check_message(void)
{
    if (message_showing() != 0) {
        if (g_nScreenMessageTimeNumber_005d1c32 == -1) {
            if (g_bSpeechPlaybackDone_004a266c != 0) {
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
            g_nScreenMessageTimeNumber_005d1c32--;
            if (g_nScreenMessageTimeNumber_005d1c32 == 0) {
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

    SetTextContext(&g_stCockpitTextSettingsRecord_005d2d00);
    speed = (short)(g_anShipSpeed_00494e20[0] >> 8);
    speed = (short)(speed * 10);
    DrawCockpitReadout(
        2, itoa(speed, g_szTextBufferBuffer_005d1c40, 10));
    velocity = (short)(MultiplyFixed(
        Vector_magnitude(&g_aShipVelocity_00494898[0]), 0xa00) >> 8);
    DrawCockpitReadout(
        3, itoa(velocity, g_szTextBufferBuffer_005d1c40, 10));
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
        ((vdu == 0 && g_stLeftwardVduPortBlock_005d2180.top > 0) ||
         (vdu == 1 && g_stRightVduView_005d2b20.top > 0))) {
        malf_noise(vdu, 1, g_cDarkGreenColour_0049cb60, sound, 0);
        set_mode(vdu, 0);
    }
}

/* Function start: 0x43984F */
void ShowComponentHitHudMessage(const char *text, unsigned short colour,
                                short flashCount)
{
    if (get_mode(0) != 0) {
        if (g_aHudTextSlotsArray_005d1d40[0].text != 0)
            ClearHudMessageSlot(&g_aHudTextSlotsArray_005d1d40[0]);
        DosStrcpy(g_szComponentHitMsg_005d1da0, text);
        SetHudMessageSlot(&g_aHudTextSlotsArray_005d1d40[0], &g_stLeftVduStringContextRecord_005d2ae0,
                          g_stLeftwardVduPortBlock_005d2180.left,
                          (short)(g_stLeftwardVduPortBlock_005d2180.bottom - 6),
                          g_szComponentHitMsg_005d1da0,
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
    if (malf(component) != 0) {
        switch (component) {
        case 3:
            vdu_malf(0, 0x18);
            vdu_malf(1, 0x18);
            break;
        }
    }
    if (get_mode(0) == 2 || get_mode(0) == 1) {
        sprintf(text, "%s HIT",
                g_apszComponentNames_00490090[(int)component]);
        ShowComponentHitHudMessage(text, g_cRedColour_0049cb5c, 5);
    }
    return g_acPlayerComponentDamage_00493470[(int)component];
}

/* Function start: 0x4399C6 */
void RemovePlayerReleaseWeapon(signed char weapon)
{
    lock_off();
    g_nReleaseWeaponShowKindWord_005d1c28 =
        ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[weapon].type;
    g_cReleaseWeaponDisplayFrame_0049b060 = 1;
    g_stReleaseWeaponReadoutLocationBlock_005d1c24.x =
        (short)(g_aWeaponDisplayPositions_005d1de0[weapon].x +
                g_nWeaponDisplayOriginXValue_005d4254);
    g_stReleaseWeaponReadoutLocationBlock_005d1c24.y =
        (short)(g_aWeaponDisplayPositions_005d1de0[weapon].y +
                g_nArmsDisplayOriginY_005d4256);
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

    if (g_cReleaseWeaponDisplayFrame_0049b060 != -1) {
        visible = g_nCurrentView_00492fa8 == 0 && get_mode(0) == 1;
        if (g_cReleaseWeaponDisplayState_0049b068 != 0) {
            g_cReleaseWeaponDisplayState_0049b068 = 0;
            RestoreSpriteBackground(
                &g_stLeftwardVduPortBlock_005d2180,
                g_pReleaseWeaponDisplayBackground_0049b05c,
                g_stReleaseWeaponReadoutLocationBlock_005d1c24.x,
                g_stReleaseWeaponReadoutLocationBlock_005d1c24.y,
                g_pDashWeaponPicture_005d2b54,
                g_cReleaseWeaponDisplayFrame_0049b060);
        }
        if (g_stLeftwardVduPortBlock_005d2180.top - 10 <
                g_stReleaseWeaponReadoutLocationBlock_005d1c24.y &&
            g_stReleaseWeaponReadoutLocationBlock_005d1c24.y <
                g_stLeftwardVduPortBlock_005d2180.bottom) {
            g_stReleaseWeaponReadoutLocationBlock_005d1c24.y =
                (short)(g_stReleaseWeaponReadoutLocationBlock_005d1c24.y -
                        g_cReleaseWeaponDisplayTicks_0049b064);
            g_cReleaseWeaponDisplayTicks_0049b064++;
            if (visible != 0) {
                CaptureSpriteBackground(
                    &g_stLeftwardVduPortBlock_005d2180,
                    g_pReleaseWeaponDisplayBackground_0049b05c,
                    g_stReleaseWeaponReadoutLocationBlock_005d1c24.x,
                    g_stReleaseWeaponReadoutLocationBlock_005d1c24.y,
                    g_pDashWeaponPicture_005d2b54,
                    g_cReleaseWeaponDisplayFrame_0049b060);
                DrawSpriteDefault(
                    &g_stLeftwardVduPortBlock_005d2180,
                    g_stReleaseWeaponReadoutLocationBlock_005d1c24.x,
                    g_stReleaseWeaponReadoutLocationBlock_005d1c24.y,
                    g_pDashWeaponPicture_005d2b54,
                    g_cReleaseWeaponDisplayFrame_0049b060);
            }
            g_cReleaseWeaponDisplayState_0049b068 = (signed char)visible;
        } else {
            g_cReleaseWeaponDisplayFrame_0049b060 = -1;
        }
    }
}

/* Function start: 0x439BB7 */
void show_weapon_disp(void)
{
    signed char slotIndex;
    const char *releaseLabel;
    const char *gunCaption;
    short x;
    ShipWeaponSlot *entry;
    short frame;
    unsigned char *weapons;
    short y;

    weapons = g_aShipWeapons_004956b0[0];
#ifdef SDL_PORT
    /* With no selected release entry, retail forms a pointer ten bytes before
     * this array but does not dereference it in the name switch. */
    if (g_nSelectedReleaseWeaponIndex_004934e0 == -1) {
        entry = 0;
    } else {
#endif
    entry = (ShipWeaponSlot *)(
        weapons + g_nSelectedReleaseWeaponIndex_004934e0 *
                         sizeof(ShipWeaponSlot) +
        1);
#ifdef SDL_PORT
    }
#endif
    set_new_vdu(0);
    DrawTextAt(&g_stLeftVduStringContextRecord_005d2ae0,
               g_stLeftwardVduPortBlock_005d2180.left,
               g_stLeftwardVduPortBlock_005d2180.top, "WEAPON DISPLAY", 2);
    DrawViewportLine(&g_stLeftwardVduPortBlock_005d2180,
                     g_stLeftwardVduPortBlock_005d2180.left + 2,
                     g_stLeftwardVduPortBlock_005d2180.top + 5,
                     (short)(g_stLeftwardVduPortBlock_005d2180.right - 2),
                     g_stLeftwardVduPortBlock_005d2180.top + 5,
                     g_ucPrimaryTextColour_0049cb64);

    switch (g_nSelectedReleaseWeaponIndex_004934e0) {
    case -1:
        releaseLabel = g_szNoWeaponSelected_0049b070;
        break;
    default:
        releaseLabel = g_aObjectTypeData_00496d30[
            (int)entry->weaponType].displayName;
    }
    switch (g_nSelectedGunType_004934dc) {
    case -1:
        gunCaption = g_szNoWeaponSelected_0049b070;
        break;
    case 0x80:
        gunCaption = "Full Guns";
        break;
    default:
        gunCaption = g_aObjectTypeData_00496d30[
            g_nSelectedGunType_004934dc].displayName;
    }
    DrawFormattedText("\nWeapon:%s", releaseLabel);
    DrawFormattedText("\nGun:%s", gunCaption);

    g_nWeaponDisplayOriginXValue_005d4254 =
        (short)(g_stLeftwardVduPortBlock_005d2180.left +
                g_stWeaponDisplayOffset_0049ae8c.x);
    g_nArmsDisplayOriginY_005d4256 =
        (short)(g_stWeaponDisplayOffset_0049ae8c.y +
                g_stLeftwardVduPortBlock_005d2180.top);
    DrawSpriteDefault(
        &g_stLeftwardVduPortBlock_005d2180,
        g_nWeaponDisplayOriginXValue_005d4254,
        g_nArmsDisplayOriginY_005d4256,
        g_pDashWeaponPicture_005d2b54, 0);

    entry = (ShipWeaponSlot *)(weapons + 1);
    for (slotIndex = 0; slotIndex < (signed char)weapons[0];
         slotIndex++, entry++) {
        if (g_aObjectTypeData_00496d30[
                (int)entry->weaponType].field_18 == 0x0b) {
            frame = 0x1a;
            if (g_aWeaponDisplayPositions_005d1de0[
                    (int)slotIndex].x > 0x29)
                frame = (short)(frame + 2);
            if (g_aWeaponDisplayPositions_005d1de0[
                    (int)slotIndex].x < 0x1f)
                frame++;
        } else if (g_aObjectTypeData_00496d30[
                       (int)entry->weaponType].objectClass == 8) {
            frame = (short)((entry->type - 7) * 2 + entry->disabled + 3);
        } else {
            frame = (short)(
#ifdef SDL_PORT
                ((g_nSelectedReleaseWeaponIndex_004934e0 == -1
                      ? 0
                      : ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[
                            g_nSelectedReleaseWeaponIndex_004934e0].type) !=
                 entry->type) + 1);
#else
                (((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[
                     g_nSelectedReleaseWeaponIndex_004934e0].type !=
                 entry->type) + 1);
#endif
        }

        x = g_aWeaponDisplayPositions_005d1de0[(int)slotIndex].x;
        y = g_aWeaponDisplayPositions_005d1de0[(int)slotIndex].y;
        if (g_aObjectTypeData_00496d30[
                (int)entry->weaponType].field_18 != 0x14) {
            DrawSpriteDefault(
                &g_stLeftwardVduPortBlock_005d2180,
                (short)(g_nWeaponDisplayOriginXValue_005d4254 + x),
                (short)(g_nArmsDisplayOriginY_005d4256 + y),
                g_pDashWeaponPicture_005d2b54, frame);
        }
        if ((int)slotIndex == g_nSelectedReleaseWeaponIndex_004934e0 &&
            g_aObjectTypeData_00496d30[
                (int)entry->weaponType].field_18 == 0x14) {
            DrawSpriteDefault(
                &g_stLeftwardVduPortBlock_005d2180,
                (short)(g_nWeaponDisplayOriginXValue_005d4254 + 10),
                (short)(g_nArmsDisplayOriginY_005d4256 + 13),
                g_pDashWeaponPicture_005d2b54,
                (short)(entry->type + 4));
            DrawSpriteDefault(
                &g_stLeftwardVduPortBlock_005d2180,
                (short)(g_nWeaponDisplayOriginXValue_005d4254 + 60),
                (short)(g_nArmsDisplayOriginY_005d4256 + 13),
                g_pDashWeaponPicture_005d2b54,
                (short)(entry->type + 4));
        }
    }
}

short get_mode(short i);

/* Function start: 0x439F5F */
void update_status_text(void)
{
    short x;
    short frame;
    short y;
    short i;

    for (i = 0;
         i < (signed char)g_aShipWeapons_004956b0[0][0];
         i++) {
        if (((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[
                i].type == 11) {
            g_nWeaponDisplayOriginXValue_005d4254 =
                (short)(g_stLeftwardVduPortBlock_005d2180.left +
                        g_stWeaponDisplayOffset_0049ae8c.x);
            g_nArmsDisplayOriginY_005d4256 =
                (short)(g_stWeaponDisplayOffset_0049ae8c.y +
                        g_stLeftwardVduPortBlock_005d2180.top);
            x = g_aWeaponDisplayPositions_005d1de0[i].x;
            y = g_aWeaponDisplayPositions_005d1de0[i].y;

            if (g_aHudTextSlotsArray_005d1d40[0].text == 0 &&
                x < 0x29 && x > 0x1f) {
                if (g_aasGunCooldown_005c8d70[0][0] > 0) {
                    frame = 0x1d;
                    DrawSpriteDefault(
                        &g_stLeftwardVduPortBlock_005d2180,
                        (short)(g_nWeaponDisplayOriginXValue_005d4254 + x),
                        (short)(g_nArmsDisplayOriginY_005d4256 + y),
                        g_pDashWeaponPicture_005d2b54, frame);
                } else {
                    frame = 0x1a;
                    DrawSpriteDefault(
                        &g_stLeftwardVduPortBlock_005d2180,
                        (short)(g_nWeaponDisplayOriginXValue_005d4254 + x),
                        (short)(g_nArmsDisplayOriginY_005d4256 + y),
                        g_pDashWeaponPicture_005d2b54, frame);
                }
            }

            if (g_cReleaseWeaponDisplayFrame_0049b060 == -1 && x < 0x1f) {
                if (g_aasGunCooldown_005c8d70[0][1] > 0) {
                    frame = 0x1e;
                    DrawSpriteDefault(
                        &g_stLeftwardVduPortBlock_005d2180,
                        (short)(g_nWeaponDisplayOriginXValue_005d4254 + x),
                        (short)(g_nArmsDisplayOriginY_005d4256 + y),
                        g_pDashWeaponPicture_005d2b54, frame);
                } else {
                    frame = 0x1b;
                    DrawSpriteDefault(
                        &g_stLeftwardVduPortBlock_005d2180,
                        (short)(g_nWeaponDisplayOriginXValue_005d4254 + x),
                        (short)(g_nArmsDisplayOriginY_005d4256 + y),
                        g_pDashWeaponPicture_005d2b54, frame);
                }
            }

            if (g_cReleaseWeaponDisplayFrame_0049b060 == -1 && x > 0x29) {
                if (g_aasGunCooldown_005c8d70[0][2] > 0) {
                    frame = 0x1f;
                    DrawSpriteDefault(
                        &g_stLeftwardVduPortBlock_005d2180,
                        (short)(g_nWeaponDisplayOriginXValue_005d4254 + x),
                        (short)(g_nArmsDisplayOriginY_005d4256 + y),
                        g_pDashWeaponPicture_005d2b54, frame);
                } else {
                    frame = 0x1c;
                    DrawSpriteDefault(
                        &g_stLeftwardVduPortBlock_005d2180,
                        (short)(g_nWeaponDisplayOriginXValue_005d4254 + x),
                        (short)(g_nArmsDisplayOriginY_005d4256 + y),
                        g_pDashWeaponPicture_005d2b54, frame);
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

#pragma function(strcpy, strlen)
/* Function start: 0x43A2CD */
void DrawCalculatingLabel(void)
{
    if (IsMissionObjectiveOutOfSystem(
            (short)g_cCurrentObjective_004931cc) != 0) {
        DrawCockpitReadout(0, "Out-system");
    } else {
        if (g_nCurrentObjectiveRange_004931c8 <= 0) {
            DrawCockpitReadout(0, "CALCULATING");
        } else {
            strcpy(g_szTextBufferBuffer_005d1c40 +
                       strlen(ltoa((long)g_nCurrentObjectiveRange_004931c8,
                                    g_szTextBufferBuffer_005d1c40, 10)),
                   " km");
            DrawCockpitReadout(0, g_szTextBufferBuffer_005d1c40);
        }
        g_nDisplayedObjectiveRange_0049b078 =
            g_nCurrentObjectiveRange_004931c8;
    }
}

/* Function start: 0x43A374 */
const char *objective_name(short objective)
{
    if (objective >= (short)g_cMissionObjectiveCount_00493294)
        return "NONE";
    else if (*g_aMissionObjectives_004932a8[objective].name != '?' ||
             sighted(objective) != 0)
        return g_aMissionObjectives_004932a8[objective].displayName;
    else
        return "UNKNOWN";
}

/* Function start: 0x43A3F8 */
void show_navigation_disp(void)
{
    DrawTextAt(&g_stRightVduTextContextBlock_005d2ce0, g_stRightVduView_005d2b20.left, g_stRightVduView_005d2b20.top,
               "COMP NAVIGATION", 2);
    DrawFormattedText("\n\nDESTINATION\n %s",
                      objective_name((short)g_cCurrentObjective_004931cc));
    DrawFormattedText("\n\nRANGE\n ");
    InitializeCockpitReadout(0, &g_stRightVduTextContextBlock_005d2ce0);
    DrawFormattedText("\n\n(N)ew Objective");
    DrawCalculatingLabel();
}

/* Function start: 0x43A474 */
short hidden_objective(short objective)
{
    short concealed;
    short navPoint;
    short record;
    short ship;

    concealed = 0;
    if (g_aMissionObjectives_004932a8[objective].type == 0 ||
        g_aMissionObjectives_004932a8[objective].type == 5) {
        if (g_aMissionNavPoints_00491e98[
                g_aMissionObjectives_004932a8[objective].index].type != 0)
            concealed = 0;
        else
            return 1;
    }
    if (mobile_objective(objective) != 0 &&
        is_team_member(g_aMissionObjectives_004932a8[objective].index) == 0) {
        concealed = 1;
        for (navPoint = 0; navPoint < 10; navPoint++) {
            if (g_aMissionNavPoints_00491e98[navPoint].type == 0)
                continue;
            for (record = 0; record < 10; record++) {
                if (g_aMissionNavPoints_00491e98[navPoint].missionShips[
                        record] ==
                    g_aMissionObjectives_004932a8[objective].index)
                    break;
            }
            if (record < 10) {
                concealed = 0;
                break;
            }
        }
    }
    if (concealed != 0 ||
        *g_aMissionObjectives_004932a8[objective].displayName == '.')
        concealed = 1;
    else
        concealed = 0;
    if (concealed != 0 ||
        *g_aMissionObjectives_004932a8[objective].name == '.')
        concealed = 1;
    else
        concealed = 0;
    if (concealed != 0 ||
        (mobile_objective(objective) != 0 &&
         g_aMissionShips_00492290[
             g_aMissionObjectives_004932a8[objective].index].state != 0))
        concealed = 1;
    else
        concealed = 0;
    if (concealed == 0 && mobile_objective(objective) != 0 &&
        g_aMissionObjectives_004932a8[
            g_cMissionObjectiveCount_00493294].type == 0) {
        ship = find_ship_index(
            g_aMissionObjectives_004932a8[objective].index);
        if (g_aMissionShips_00492290[
                g_aMissionObjectives_004932a8[objective].index].missionType ==
                MISSION_TYPE_WARP_ARRIVE &&
            ship != -1)
            concealed = 1;
    }
    return concealed;
}

/* Function start: 0x43A75E */
short set_new_objective(short pathIndex)
{
    if (pathIndex < 0) {
        pathIndex = -1;
        while (g_abFlightPath_004932a0[++pathIndex] != -1 &&
               pathIndex < (short)g_cMissionObjectiveCount_00493294)
            ;
        pathIndex--;
    }
    if (pathIndex >= (short)g_cMissionObjectiveCount_00493294)
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
    while (wraps < 3 &&
           set_new_objective(
               (short)((short)g_cCurrentNavPointIndex_00493298 + 1)) == 0) {
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
    while (set_new_objective(
               (short)g_cCurrentNavPointIndex_00493298) == 0 ||
           visited((short)g_abFlightPath_004932a0[
               g_cCurrentNavPointIndex_00493298]) != 0) {
        g_cCurrentNavPointIndex_00493298++;
        if (g_cCurrentNavPointIndex_00493298 >=
                g_cMissionObjectiveCount_00493294 ||
            g_abFlightPath_004932a0[
                g_cCurrentNavPointIndex_00493298] == -1)
            break;
    }
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
short escorting_a_ship(void)
{
    return g_asShipMissionType_00495de8[0] == MISSION_TYPE_ESCORT ||
           is_any_ship_returning_to_current_nav_point() != 0;
}

/* Function start: 0x43AAFF */
void flag_reached(short objective, short reached)
{
    short visit;
    short carrierShip;
    short moveOn;
    short type;
    short carrierObject;

    type = g_aMissionObjectives_004932a8[objective].type;
    carrierShip = g_asShipMissionParameter_00495e00[0];
    carrierObject = find_ship_index(g_asShipMissionParameter_00495e00[0]);
    if (g_cCurrentObjective_004931cc != objective)
        visit = 1;
    else
        visit = 0;
    moveOn = 0;
    if (g_cCurrentObjective_004931cc == objective) {
        if (reached != 0 || escorting_a_ship() == 0 ||
            carrierObject == -1 ||
            g_aMissionObjectives_004932a8[objective].index ==
                g_asShipMissionParameter_00495e00[0]) {
            if (visited(objective) != 0) {
                CockpitMessage("Already Visited",
                               g_cYellowColour_0049cb58, 4);
            } else {
                CockpitMessage("Objective Reached",
                               g_cYellowColour_0049cb58, 4);
            }
            moveOn = visit = 1;
        } else if (type != 1 ||
                   g_aMissionShips_00492290[carrierShip].state != 1) {
            sprintf(g_pszAutopilotWaitReason_0049b050,
                    "Wait for %s",
                    g_apShipMissionRecord_00495da8[carrierObject]->name);
            CockpitMessage(g_pszAutopilotWaitReason_0049b050,
                           g_cYellowColour_0049cb58, 4);
        }
    }
    if (type != 1 && visit != 0) {
        if (visited(objective) == 0 && carrierObject != -1 &&
            g_aMissionObjectives_004932a8[objective].index ==
                g_asShipMissionParameter_00495e00[0] &&
            g_asShipSide_004955d0[carrierObject] ==
                g_asShipSide_004955d0[0] &&
            g_asShipMissionIndex_00495d00[carrierObject] !=
                g_nHomeOperationShipIndexValue_005d1e22 &&
            g_abShipArrivalMessageSent_005d1dd0[carrierObject] == 0) {
            send_message(carrierObject, 0x16);
            g_abShipArrivalMessageSent_005d1dd0[carrierObject]++;
        }
        flag_objective(objective, 1);
    }
    if (moveOn != 0)
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
    short type;
    int reachedRange;

    type = g_aMissionObjectives_004932a8[objective].type;
    if (type == 3 || type == 4)
        reachedRange = 6000;
    else
        reachedRange = 1500;
    if (range < reachedRange)
        flag_reached(objective, 1);
}

/* Function start: 0x43AE2F */
void update_objective_location(short objective)
{
    short savedSystem;
    FixedVector delta;
    short object;
    short range;

    if (g_aMissionObjectives_004932a8[objective].type == 6 ||
        g_aMissionObjectives_004932a8[objective].type == 5)
        return;
    object = LocateMobileObjective(objective);
    savedSystem = g_stElapsedCampaignDate_005d170c.year;
    g_stElapsedCampaignDate_005d170c.year = g_nCurStellarSystem_005d169c;
    if (IsObjectiveInDisplayedSystem(objective) == 0) {
        g_stElapsedCampaignDate_005d170c.year = savedSystem;
        return;
    }
    if (sighted(objective) == 0 || visited(objective) == 0 ||
        g_cCurrentObjective_004931cc == objective) {
        ComputeVectorDelta(
            &g_aShipPosition_00494550[0],
            &g_aMissionObjectives_004932a8[objective].position, &delta);
        range = FixedToShortSaturating(Vector_magnitude(&delta));
        check_sighting(objective, range, object);
        if (mobile_objective(objective) != 0) {
            if (object != -1)
                check_visit(objective, range);
        } else if (g_aMissionNavPoints_00491e98[
                       g_aMissionObjectives_004932a8[objective].index]
                       .type >= 1) {
            check_visit(objective, range);
        }
    }
    g_stElapsedCampaignDate_005d170c.year = savedSystem;
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

#pragma function(abs)
/* Function start: 0x43B0EB */
void rotational_pos_to_scanner_pos(signed char object,
                                   const SphericalVector *position,
                                   CockpitScannerGeometry geometry)
{
    if (abs((int)position->yaw) < 45)
        g_nScopePointerX_005d1e6c =
            (short)(geometry.centerX + position->yaw / 4);
    else
        g_nScopePointerX_005d1e6c =
            (short)(geometry.centerX + position->yaw / 6);

    g_nScannerCursorYValue_005d1e6e =
        (short)(geometry.centerY + -(position->pitch / 3));
    g_nScopePointerX_005d1e6c =
        MinShort(geometry.maximumX, g_nScopePointerX_005d1e6c);
    g_nScopePointerX_005d1e6c =
        MaxShort(geometry.minimumX, g_nScopePointerX_005d1e6c);
    g_nScannerCursorYValue_005d1e6e =
        MinShort(geometry.maximumY, g_nScannerCursorYValue_005d1e6e);
    g_nScannerCursorYValue_005d1e6e =
        MaxShort(geometry.minimumY, g_nScannerCursorYValue_005d1e6e);
    g_asScannerObjectX_005d1ea0[(int)object] =
        g_nScopePointerX_005d1e6c;
    g_asScannerObjectYTable_005d1e80[(int)object] =
        g_nScannerCursorYValue_005d1e6e;
}

/* Function start: 0x43B1F0 */
short mobile_objective(short objective)
{
    short type;

    type = g_aMissionObjectives_004932a8[objective].type;
    return type == 1 || type == 3 || type == 4 || type == 2;
}

/* Function start: 0x43B258 */
void ResetScannerContacts(void)
{
    short i;

    for (i = 10; i > 0; i--)
        g_asScannerObjectX_005d1ea0[i] = 0;
    g_nScannerTargetObject_0049b07c = -1;
}

/* Function start: 0x43B29D */
void clear_head_up_display(void)
{
    short object;

    if (g_nCurrentView_00492fa8 == 4)
        RestoreTargetCameraAttitudeIndicators();
    if (g_nScannerTargetObject_0049b07c != -1) {
        DrawViewportPixel(&g_stScrViewInfo_005d21a0,
                          g_asScannerObjectX_005d1ea0[g_nScannerTargetObject_0049b07c],
                          g_asScannerObjectYTable_005d1e80[g_nScannerTargetObject_0049b07c],
                          g_asScannerTargetUnderlay_005d1d80[0]);
        DrawViewportPixel(&g_stScrViewInfo_005d21a0,
                          (short)(g_asScannerObjectX_005d1ea0[g_nScannerTargetObject_0049b07c] + 1),
                          g_asScannerObjectYTable_005d1e80[g_nScannerTargetObject_0049b07c],
                          g_asScannerTargetUnderlay_005d1d80[1]);
        DrawViewportPixel(&g_stScrViewInfo_005d21a0,
                          (short)(g_asScannerObjectX_005d1ea0[g_nScannerTargetObject_0049b07c] - 1),
                          g_asScannerObjectYTable_005d1e80[g_nScannerTargetObject_0049b07c],
                          g_asScannerTargetUnderlay_005d1d80[2]);
        DrawViewportPixel(&g_stScrViewInfo_005d21a0,
                          g_asScannerObjectX_005d1ea0[g_nScannerTargetObject_0049b07c],
                          (short)(g_asScannerObjectYTable_005d1e80[g_nScannerTargetObject_0049b07c] + 1),
                          g_asScannerTargetUnderlay_005d1d80[3]);
        DrawViewportPixel(&g_stScrViewInfo_005d21a0,
                          g_asScannerObjectX_005d1ea0[g_nScannerTargetObject_0049b07c],
                          (short)(g_asScannerObjectYTable_005d1e80[g_nScannerTargetObject_0049b07c] - 1),
                          g_asScannerTargetUnderlay_005d1d80[4]);
        g_asScannerObjectX_005d1ea0[g_nScannerTargetObject_0049b07c] = 0;
    }
    if (g_asScannerObjectX_005d1ea0[10] != 0 &&
        g_nCurrentView_00492fa8 != 4 &&
        IsMissionObjectiveOutOfSystem(
            (short)g_cCurrentObjective_004931cc) == 0) {
        RestoreSpriteBackground(&g_stScrViewInfo_005d21a0,
                                g_pScannerBlipUnderlayPointer_005d1c2c,
                                g_asScannerObjectX_005d1ea0[10],
                                g_asScannerObjectYTable_005d1e80[10],
                                g_pCabinHudImage_005d21f4, 2);
        g_asScannerObjectX_005d1ea0[10] = 0;
    }
    for (object = 9; object > 0; object--) {
        if (g_asScannerObjectX_005d1ea0[object] != 0) {
            DrawViewportPixel(&g_stScrViewInfo_005d21a0,
                              g_asScannerObjectX_005d1ea0[object],
                              g_asScannerObjectYTable_005d1e80[object],
                              g_asScannerBackgroundColourList_005d1ed0[object]);
            g_asScannerObjectX_005d1ea0[object] = 0;
        }
    }
    g_nScannerTargetObject_0049b07c = -1;
}

/* Function start: 0x43B4CF */
void set_objective_range(short showOnScanner)
{
    FixedVector delta;
    FixedVector rotated;
    SphericalVector polar;

    LocateMobileObjective((short)g_cCurrentObjective_004931cc);
    ComputeVectorDelta(&g_aShipPosition_00494550[0],
                       &g_aMissionObjectives_004932a8[
                           (short)g_cCurrentObjective_004931cc].position,
                       &delta);
    transform_to_objects_frame(&delta, &rotated, 0);
    rectangular_to_spherical(&rotated, &polar);
    if (showOnScanner != 0)
        rotational_pos_to_scanner_pos(
            10, &polar, g_stCockpitScannerGeometry_0049af78);
    g_nCurrentObjectiveRange_004931c8 = polar.radius >> 8;
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
    if (*colour == g_cYellowColour_0049cb58)
        increment = 1;
    if (range < 0x138800)
        return;
    else if (range < 0x61a800)
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
        *colour = g_cYellowColour_0049cb58;
        return 1;
    }
    if (objectClass < OBJECT_CLASS_MISSILE)
        return 0;
    else if (objectClass == OBJECT_CLASS_SHIP) {
        if (g_asShipSide_004955d0[object] == SIDE_KILRATHI)
            *colour = g_cRedColour_0049cb5c;
        else if (g_asShipSide_004955d0[object] == SIDE_IMPERIAL)
            *colour = g_cBlueColour_0049cb54;
        else
            *colour = g_ucPrimaryTextColour_0049cb64;
    } else if (objectClass >= OBJECT_CLASS_CAPITAL_SHIP) {
        if (g_asShipSide_004955d0[object] == SIDE_KILRATHI)
            *colour = g_ucHostileCapitalScannerColour_0049cb68;
        else if (g_asShipMissionIndex_00495d00[object] ==
                 g_nHomeOperationShipIndexValue_005d1e22)
            *colour = g_bPrimaryViewBufferColour_0049cb50;
        else
            *colour = g_ucHomeCarrierScannerColour_0049cb70;
    } else {
        if (g_acShipTarget_00495f20[object] == 0)
            *colour = g_cYellowColour_0049cb58;
        else
            return 0;
    }
    return 1;
}

/* Function start: 0x43B7C0 */
void draw_3d_scanner(void)
{
    int cell;
    int lineBits;
    int gridIndex;
    CockpitScannerGeometry geometry;
    short obj;
    int baseColour;
    FixedVector difference;
    FixedVector inShipFrame;
    const int *gridPattern;
    unsigned int markerColour;
    SphericalVector angles;

    if (g_nCurrentView_00492fa8 == 0) {
        geometry = g_stCockpitScannerGeometry_0049af78;
        gridPattern = g_aiForwardScannerGridRows_0049b080;
        baseColour = 0x5b;
    } else if (g_nCurrentView_00492fa8 == 4) {
        geometry = ((CockpitScannerGeometry *)(
            g_abGunDisplayConfiguration_0049d340 + 0x40))[
                g_nGunDisplaySlot_005c8dc0];
        gridPattern = g_aiTargetCameraScannerGridRows_0049b1b8;
        baseColour = 0x25;
    }

    if (g_nCockpitDisplayMode_0049d71c != 0) {
        lineBits = 0;
        for (gridIndex = 0; gridPattern[gridIndex] != -2; gridIndex++) {
            if (gridPattern[gridIndex] == -1) {
                lineBits++;
            } else {
                cell = gridPattern[gridIndex];
                DrawViewportPixel(
                    &g_stScrViewInfo_005d21a0,
                    (short)(geometry.centerX + lineBits),
                    (short)(geometry.centerY + cell),
                    (short)baseColour);
                if (cell != 0)
                    DrawViewportPixel(
                        &g_stScrViewInfo_005d21a0,
                        (short)(geometry.centerX + lineBits),
                        (short)(geometry.centerY - cell),
                        (short)baseColour);
                if (lineBits != 0) {
                    DrawViewportPixel(
                        &g_stScrViewInfo_005d21a0,
                        (short)(geometry.centerX - lineBits),
                        (short)(geometry.centerY + cell),
                        (short)baseColour);
                    if (cell != 0)
                        DrawViewportPixel(
                            &g_stScrViewInfo_005d21a0,
                            (short)(geometry.centerX - lineBits),
                            (short)(geometry.centerY - cell),
                            (short)baseColour);
                }
            }
        }
    }

    clear_head_up_display();
    g_nScannerTargetObject_0049b07c = g_acShipTarget_00495f20[0];
    if ((g_nScannerTargetObject_0049b07c != -1 &&
         g_aeObjectClass_00495328[g_nScannerTargetObject_0049b07c] <
             OBJECT_CLASS_SHIP) ||
#ifdef SDL_PORT
        /* With no target the first test is false and the original reads the
         * cloak state at index -1, four bytes in front of the array.  The
         * outcome is the same either way: the target is already -1. */
        (g_nScannerTargetObject_0049b07c >= 0 &&
         g_anShipCloakState_00496020[
             g_nScannerTargetObject_0049b07c] == 1))
#else
        g_anShipCloakState_00496020[
            g_nScannerTargetObject_0049b07c] == 1)
#endif
        g_nScannerTargetObject_0049b07c = -1;

    for (obj = 1; obj <= 9; obj++) {
        if (g_anShipCloakState_00496020[obj] != 1 &&
            get_color(obj, (unsigned short *)&markerColour) != 0) {
            ComputeVectorDelta(
                &g_aShipPosition_00494550[EYE_OBJECT],
                &g_aShipPosition_00494550[obj], &difference);
            transform_to_objects_frame(
                &difference, &inShipFrame, EYE_OBJECT);
            rectangular_to_spherical(&inShipFrame, &angles);
            if (angles.radius < 0xea6000) {
                AdjustScannerContactColourForRange(
                    angles.radius, (short *)&markerColour);
                rotational_pos_to_scanner_pos(
                    (signed char)obj, &angles, geometry);
                if (g_nScannerTargetObject_0049b07c != obj) {
                    g_asScannerBackgroundColourList_005d1ed0[obj] =
                        (short)GetViewportPixel(
                            &g_stScrViewInfo_005d21a0,
                            g_nScopePointerX_005d1e6c,
                            g_nScannerCursorYValue_005d1e6e);
                    DrawViewportPixel(
                        &g_stScrViewInfo_005d21a0,
                        g_nScopePointerX_005d1e6c,
                        g_nScannerCursorYValue_005d1e6e,
                        (short)markerColour);
                }
            }
        }
    }

    if (get_mode(1) == 5 &&
        g_nCurrentView_00492fa8 != 4 &&
        IsMissionObjectiveOutOfSystem(
            (short)g_cCurrentObjective_004931cc) == 0) {
        set_objective_range(1);
        CaptureSpriteBackground(
            &g_stScrViewInfo_005d21a0,
            g_pScannerBlipUnderlayPointer_005d1c2c,
            g_nScopePointerX_005d1e6c,
            g_nScannerCursorYValue_005d1e6e,
            g_pCabinHudImage_005d21f4, 2);
        DrawSpriteDefault(
            &g_stScrViewInfo_005d21a0,
            g_nScopePointerX_005d1e6c,
            g_nScannerCursorYValue_005d1e6e,
            g_pCabinHudImage_005d21f4, 2);
    }

    if (g_nScannerTargetObject_0049b07c != -1 &&
        g_anShipCloakState_00496020[
            g_nScannerTargetObject_0049b07c] != 1 &&
        get_color(g_nScannerTargetObject_0049b07c,
                  (unsigned short *)&markerColour) != 0) {
        ComputeVectorDelta(
            &g_aShipPosition_00494550[EYE_OBJECT],
            &g_aShipPosition_00494550[obj], &difference);
        transform_to_objects_frame(
            &difference, &inShipFrame, EYE_OBJECT);
        rectangular_to_spherical(&inShipFrame, &angles);
        if (angles.radius < 0xea6000)
            AdjustScannerContactColourForRange(
                angles.radius, (short *)&markerColour);

        g_asScannerTargetUnderlay_005d1d80[0] =
            (short)GetViewportPixel(
                &g_stScrViewInfo_005d21a0,
                g_asScannerObjectX_005d1ea0[g_nScannerTargetObject_0049b07c],
                g_asScannerObjectYTable_005d1e80[g_nScannerTargetObject_0049b07c]);
        g_asScannerTargetUnderlay_005d1d80[1] =
            (short)GetViewportPixel(
                &g_stScrViewInfo_005d21a0,
                (short)(g_asScannerObjectX_005d1ea0[g_nScannerTargetObject_0049b07c] + 1),
                g_asScannerObjectYTable_005d1e80[g_nScannerTargetObject_0049b07c]);
        g_asScannerTargetUnderlay_005d1d80[2] =
            (short)GetViewportPixel(
                &g_stScrViewInfo_005d21a0,
                (short)(g_asScannerObjectX_005d1ea0[g_nScannerTargetObject_0049b07c] - 1),
                g_asScannerObjectYTable_005d1e80[g_nScannerTargetObject_0049b07c]);
        g_asScannerTargetUnderlay_005d1d80[3] =
            (short)GetViewportPixel(
                &g_stScrViewInfo_005d21a0,
                g_asScannerObjectX_005d1ea0[g_nScannerTargetObject_0049b07c],
                (short)(g_asScannerObjectYTable_005d1e80[g_nScannerTargetObject_0049b07c] + 1));
        g_asScannerTargetUnderlay_005d1d80[4] =
            (short)GetViewportPixel(
                &g_stScrViewInfo_005d21a0,
                g_asScannerObjectX_005d1ea0[g_nScannerTargetObject_0049b07c],
                (short)(g_asScannerObjectYTable_005d1e80[g_nScannerTargetObject_0049b07c] - 1));
        DrawViewportPixel(
            &g_stScrViewInfo_005d21a0,
            g_asScannerObjectX_005d1ea0[g_nScannerTargetObject_0049b07c],
            g_asScannerObjectYTable_005d1e80[g_nScannerTargetObject_0049b07c], (short)markerColour);
        DrawViewportPixel(
            &g_stScrViewInfo_005d21a0,
            (short)(g_asScannerObjectX_005d1ea0[g_nScannerTargetObject_0049b07c] + 1),
            g_asScannerObjectYTable_005d1e80[g_nScannerTargetObject_0049b07c], (short)markerColour);
        DrawViewportPixel(
            &g_stScrViewInfo_005d21a0,
            (short)(g_asScannerObjectX_005d1ea0[g_nScannerTargetObject_0049b07c] - 1),
            g_asScannerObjectYTable_005d1e80[g_nScannerTargetObject_0049b07c], (short)markerColour);
        DrawViewportPixel(
            &g_stScrViewInfo_005d21a0,
            g_asScannerObjectX_005d1ea0[g_nScannerTargetObject_0049b07c],
            (short)(g_asScannerObjectYTable_005d1e80[g_nScannerTargetObject_0049b07c] + 1),
            (short)markerColour);
        DrawViewportPixel(
            &g_stScrViewInfo_005d21a0,
            g_asScannerObjectX_005d1ea0[g_nScannerTargetObject_0049b07c],
            (short)(g_asScannerObjectYTable_005d1e80[g_nScannerTargetObject_0049b07c] - 1),
            (short)markerColour);
    }
    if (g_nCurrentView_00492fa8 == 4)
        DrawTargetCameraAttitudeIndicators(
            g_bPrimaryViewBufferColour_0049cb50);
}

/* Function start: 0x43BE1B */
void start_lock(unsigned short v)
{
    g_nTargetLockMusicCooldown_005d1e78 = -100;
    g_nTargetLockCountdown_004934ec = v;
    g_bTargetLockReadoutDirty_004934e8 = 0;
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
#ifdef SDL_PORT
        /* -1 is the no-weapon-selected sentinel, and the original indexes the
         * slot array with it anyway -- nine bytes before the array, into the
         * ion-drive damage table, where nothing is going to read as a
         * torpedo. */
        if (g_nSelectedReleaseWeaponIndex_004934e0 >= 0 &&
#else
        if (
#endif
            ((ShipWeaponSlot *)&g_aShipWeapons_004956b0[0][1])[
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
            (short)(0 - (RandomBelowOrEqual(30) + 10));
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
                               g_cRedColour_0049cb5c, 2);
            } else if (((ShipWeaponSlot *)(
                           g_aShipWeapons_004956b0[0] + 1))[
                           g_nSelectedReleaseWeaponIndex_004934e0]
                               .type != OBJECT_DATA_TORPEDO) {
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
#ifdef SDL_PORT
    /* With no release weapon selected the original indexes the slot array
     * with -1, nine bytes in front of it, and switches on whatever the
     * ion-drive damage table happens to hold.  That is never a missile type,
     * so it lands on the default arm below; take it directly. */
    if (g_nSelectedReleaseWeaponIndex_004934e0 < 0) {
        lock_off();
        return;
    }
#endif
    switch (((ShipWeaponSlot *)(g_aShipWeapons_004956b0[0] + 1))[
                g_nSelectedReleaseWeaponIndex_004934e0].type) {
    case OBJECT_DATA_JAVELIN_HEAT_SEEKING_MISSILE:
        get_facing_range_from_object(0, (short)target);
        if (g_nTargetFacing_00493198 > -0x41 ||
            CanShipWeaponDamageTarget(0, (short)target) == 0)
            lock_off();
        else if (starting_lock(0x0c) == 0)
            decrement_lock_time(x);
        break;
    case OBJECT_DATA_SPICULUM_IMAGE_RECOGNITION_MISSILE:
        if (CanShipWeaponDamageTarget(0, (short)target) == 0)
            lock_off();
        else if (starting_lock(0x16) == 0)
            decrement_lock_time(x);
        break;
    case OBJECT_DATA_TORPEDO:
        if (IsCapitalShipObject((short)target) == 0)
            lock_off();
        else if (g_nCurrentView_00492fa8 != 4) {
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
    Viewport menuViewport;
    char sourceText[84];
    short charsPerLine;
    char wrappedText[84];
    Viewport flightView;
    char *in;
    TextContext menuContext;
    TextContext flightText;
    short topY;
    short position;
    char *out;
    short breakPos;

    breakPos = -1;
    DosStrcpy(sourceText, text);
    strcpy(wrappedText, "");
    menuViewport = *g_stDefaultStringContext_005d2d20.viewport;
    menuContext = g_stDefaultStringContext_005d2d20;
    menuContext.viewport = &menuViewport;
    topY = (short)(g_stHudMessageOrigin_0049ae90.y +
                       g_nViewportOriginY_005c849c);
    SetRectBounds(&menuViewport,
                  g_stHudMessageOrigin_0049ae90.x,
                  topY,
                  (short)(319 - g_stHudMessageOrigin_0049ae90.x),
                  (short)(topY + 60));
    menuContext.colour = (unsigned char)colour;
    menuContext.backgroundColour = 0xff;
    flightView = *g_stFlightTextContext_005d21c0.viewport;
    flightText = g_stFlightTextContext_005d21c0;
    flightText.viewport = &flightView;
    SetRectBounds(&flightView,
                  g_stHudMessageOrigin_0049ae90.x,
                  g_stHudMessageOrigin_0049ae90.y,
                  (short)(319 - g_stHudMessageOrigin_0049ae90.x),
                  (short)(g_stHudMessageOrigin_0049ae90.y + 60));
    flightText.colour = (unsigned char)colour;
    flightText.backgroundColour = 0xff;
    charsPerLine = (short)(GetRectHeight(&menuViewport) / 6);
    in = sourceText;
    out = wrappedText;
    position = 0;
    while (*in != 0) {
        *out = *in;
        if (*out == ' ')
            breakPos = position;
        if ((position + 1) % charsPerLine == 0) {
            if (breakPos == -1) {
                out++;
                *out = '\n';
            } else {
                *(out + position - breakPos) = '\n';
            }
        }
        in++;
        out++;
        position++;
    }
    *out = 0;
    DrawTextAt(&menuContext,
               g_stHudMessageOrigin_0049ae90.x,
               topY,
               wrappedText, 2);
    g_pszDisplayedHudMessage_0049aff0 = g_pszPendingHudMessage_0049afec;
}

/* Function start: 0x43C570 */
void ShowHudTextLine(void)
{
    CaptureSpriteBackground(
        &g_stScrViewInfo_005d21a0,
        g_pHudMessageBackground_0049b28c,
        0,
        (short)(g_stHudMessageOrigin_0049ae90.y +
                g_nViewportOriginY_005c849c),
        g_pHudMessageFrameShape_0049b288,
        0);
    g_nHudMessageBackgroundDepth_0049b294++;
}

/* Function start: 0x43C5B0 */
void RestoreHudMessageBackground(void)
{
    if (g_nHudMessageBackgroundDepth_0049b294 != 0) {
        RestoreSpriteBackground(
            &g_stScrViewInfo_005d21a0,
            g_pHudMessageBackground_0049b28c,
            0,
            (short)(g_stHudMessageOrigin_0049ae90.y +
                    g_nViewportOriginY_005c849c),
            g_pHudMessageFrameShape_0049b288,
            0);
    }
    g_nHudMessageBackgroundDepth_0049b294 = 0;
}

/* Function start: 0x43C601 */
void SetHudTextColour(char *text, unsigned short colour)
{
    if (g_bCaptureHudMessageBackground_0049b290 != 0) {
        ShowHudTextLine();
        g_bCaptureHudMessageBackground_0049b290 = 0;
    }
    g_pszPendingHudMessage_0049afec = text;
    g_cOverlayMessageView_005d1c37 = (signed char)g_nCurrentView_00492fa8;
    print_message_text(g_pszPendingHudMessage_0049afec, colour);
}

/* Function start: 0x43C64D */
void ClearHudMessageDisplay(short closeCommMenu)
{
    if (closeCommMenu != 0)
        EndCommMenu();
    if (g_cOverlayMessageView_005d1c37 == g_nCurrentView_00492fa8 &&
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
    short midY;
    ShortRect bounds;
    short midX;
    short size;
    short segment;
    short visible;

    if ((int)(unsigned char)g_cPrimaryViewBufferColour_0049cb88 ==
        (int)(short)colour) {
        visible = savedBounds->left != -0x7fff;
        bounds = *savedBounds;
    } else {
        visible = object != -1 &&
                g_asObjectScreenX_00493598[object] != -0x7fff;
        if (visible != 0) {
            midX = (short)(g_asObjectScreenX_00493598[object] +
                              g_nViewCenterX_005c80d8);
            midY = (short)(g_asObjectScreenY_00493628[object] +
                              g_nViewCenterY_005c80da);
            if ((short)GetTransformedShapeBounds(
                    &g_stViewScratchInfo_005d2b00, midX, midY,
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
                visible = 0;
            }
        }
    }
    if (visible != 0) {
        if ((int)(unsigned char)g_cRedColour_0049cb5c ==
                (int)(short)colour &&
            g_asShipIdentified_00496078[object] == 0) {
            colour = g_cYellowColour_0049cb58;
        }
        if ((int)(unsigned char)g_cRedColour_0049cb5c ==
                (int)(short)colour &&
            g_asShipSide_004955d0[object] == SIDE_IMPERIAL) {
            colour = g_cBlueColour_0049cb54;
        }
        if (solid != 0) {
            DrawViewportBorder(&g_stViewScratchInfo_005d2b00, bounds.left, bounds.top,
                               bounds.right, bounds.bottom, colour);
        } else {
            segment =
                (short)(((int)bounds.right - bounds.left) / 6 + 1);
            DrawViewportLine(&g_stViewScratchInfo_005d2b00, bounds.left, bounds.top,
                             (short)(bounds.left + segment), bounds.top,
                             colour);
            DrawViewportLine(&g_stViewScratchInfo_005d2b00, bounds.left, bounds.bottom,
                             (short)(bounds.left + segment), bounds.bottom,
                             colour);
            DrawViewportLine(&g_stViewScratchInfo_005d2b00, bounds.right, bounds.top,
                             (short)(bounds.right - segment), bounds.top,
                             colour);
            DrawViewportLine(&g_stViewScratchInfo_005d2b00, bounds.right, bounds.bottom,
                             (short)(bounds.right - segment), bounds.bottom,
                             colour);
            segment =
                (short)(((int)bounds.bottom - bounds.top) / 6 + 1);
            DrawViewportLine(&g_stViewScratchInfo_005d2b00, bounds.left, bounds.top,
                             bounds.left, (short)(bounds.top + segment),
                             colour);
            DrawViewportLine(&g_stViewScratchInfo_005d2b00, bounds.left, bounds.bottom,
                             bounds.left,
                             (short)(bounds.bottom - segment), colour);
            DrawViewportLine(&g_stViewScratchInfo_005d2b00, bounds.right, bounds.top,
                             bounds.right, (short)(bounds.top + segment),
                             colour);
            DrawViewportLine(&g_stViewScratchInfo_005d2b00, bounds.right, bounds.bottom,
                             bounds.right,
                             (short)(bounds.bottom - segment), colour);
        }
        if (drawLockMarker != 0) {
            if ((int)(short)colour != (int)(unsigned char)g_cPrimaryViewBufferColour_0049cb88) {
                if (g_nTargetLockCountdown_004934ec > -1) {
                    g_nTargetLockMarkerAngle_004934f0 = (short)(
                        g_anObjectRollRotation_00495058[0] +
                        g_anObjectPitchRotation_00494f38[0] +
                        g_nTargetLockMarkerAngle_004934f0);
                    size =
                        (short)(g_nTargetLockCountdown_004934ec * 2);
                    if (((ShipWeaponSlot *)(g_aShipWeapons_004956b0[0] +
                             1))[g_nSelectedReleaseWeaponIndex_004934e0]
                            .type == 0x13)
                        size >>= 3;
                    midX = (short)(midX +
                        ((CosFixed(g_nTargetLockMarkerAngle_004934f0) *
                          size) >> 8));
                    midY = (short)(midY +
                        ((SinFixed(g_nTargetLockMarkerAngle_004934f0) *
                          size) >> 8));
                    DrawSpriteDefault(&g_stViewScratchInfo_005d2b00, midX, midY,
                                      g_pCabinHudImage_005d21f4, 1);
                    g_nTargetLockMarkerX_0049b298 = midX;
                    g_nTargetLockonMarkerY_005d1e20 = midY;
                }
            } else if (g_nTargetLockMarkerX_0049b298 != -0x7fff) {
                DrawSolidColourSprite(&g_stViewScratchInfo_005d2b00,
                                      g_nTargetLockMarkerX_0049b298,
                                      g_nTargetLockonMarkerY_005d1e20,
                                      g_pCabinHudImage_005d21f4, 1,
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
    int screenCoord;
    int projected;
    int magnitude;
    FixedVector direction;
    int scale;
    FixedVector eyePos;
    FixedVector objectivePos;
    short onScreen;

    if (get_mode(1) == 5 &&
        g_nTrainSimActive_0049d758 != 4 &&
        g_nCannedSceneMode_0049021c == 0 &&
        (g_nCurrentView_00492fa8 == 0 || g_nCurrentView_00492fa8 == 5) &&
        IsMissionObjectiveOutOfSystem(
            (short)g_cCurrentObjective_004931cc) == 0)
        onScreen = 1;
    else
        onScreen = 0;
    if (onScreen == 0) {
        remove_nav_pointer();
        return;
    }
    if (g_nNavPointerObject_004931b8 == -1) {
        g_nNavPointerObject_004931b8 = find_vacant_3d_object();
        if (g_nNavPointerObject_004931b8 == -1)
            return;
        g_aeObjectClass_00495328[g_nNavPointerObject_004931b8] =
            OBJECT_CLASS_PLANET;
        g_asObjectViewFrame_00493508[g_nNavPointerObject_004931b8] = 3;
        g_asObjectScreenAngle_004936b8[g_nNavPointerObject_004931b8] = 0;
        g_asObjectScreenScale_00493a58[g_nNavPointerObject_004931b8] = 0x100;
        g_apObjectShape_00493868[g_nNavPointerObject_004931b8] =
            g_pCabinHudImage_005d21f4;
        g_acObjectOwner_00495208[g_nNavPointerObject_004931b8] = -1;
    }
    g_asObjectScreenX_00493598[g_nNavPointerObject_004931b8] = (short)0x8001;
    g_asObjectDistance_00493ae8[g_nNavPointerObject_004931b8] = 0;
    objectivePos = g_aMissionObjectives_004932a8[
        (signed char)g_cCurrentObjective_004931cc].position;
    ComputeVectorDelta(&g_aShipPosition_00494550[EYE_OBJECT],
                       &objectivePos, &direction);
    magnitude = Vector_magnitude(&direction);
    if (g_asObjectCollisionRadius_004950e8[EYE_OBJECT] * 0x100 <
        magnitude) {
        transform_to_objects_frame(&direction, &eyePos,
                                   EYE_OBJECT);
        if (g_asObjectCollisionRadius_004950e8[EYE_OBJECT] * 0x100 <=
            eyePos.z) {
            if (DivideFixed(eyePos.z, magnitude) >= 0) {
                scale = ((g_nScreenWidth_0049d4d8 >> 1) << 1) << 7;
                projected = MultiplyFixed(scale, eyePos.x);
                screenCoord = DivideFixed(projected, eyePos.z);
                g_asObjectScreenX_00493598[
                    g_nNavPointerObject_004931b8] =
                    (short)(screenCoord >> 8);
                projected = MultiplyFixed(scale, eyePos.y);
                screenCoord = DivideFixed(projected, eyePos.z);
                g_asObjectScreenY_00493628[
                    g_nNavPointerObject_004931b8] =
                    (short)(screenCoord >> 8);
                g_asObjectDistance_00493ae8[
                    g_nNavPointerObject_004931b8] = 0x4a38;
            }
        }
    }
}

/* Function start: 0x43CE8F */
short HasInRangeGunForTargetLead(short targetRange)
{
    short prior;
    short count;

    count = (signed char)g_aShipWeapons_004956b0[0][0];
    while ((prior = count--) > 0) {
        if (((ShipWeaponSlot *)(
                g_aShipWeapons_004956b0[0] + 1))[count].disabled == 0 &&
            g_aObjectTypeData_00496d30[(signed char)((ShipWeaponSlot *)(
                g_aShipWeapons_004956b0[0] + 1))[count].weaponType]
                    .objectClass == OBJECT_CLASS_PROJECTILE &&
            g_aObjectTypeData_00496d30[(signed char)((ShipWeaponSlot *)(
                g_aShipWeapons_004956b0[0] + 1))[count].weaponType]
                    .maximumVelocity *
                    g_aObjectTypeData_00496d30[(signed char)(
                        (ShipWeaponSlot *)(
                            g_aShipWeapons_004956b0[0] + 1))[count]
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
    short tracked;
    short gun;
    short boltSpeed;
    FixedVector scratch;
    short rangeToTarget;
    int targetDistance;
    short frames;
    FixedVector targetMotion;
    FixedVector meetPoint;
    FixedVector eyeRelative;

    if ((signed char)g_bTargetLockDisplayEnabled_0049afe0 == 0)
        return;
    if (g_bTargetLockMode_00493500 == 0) {
        g_bTargetLockActive_0049ae80 = 0;
        return;
    }

    tracked = g_acShipTarget_00495f20[0];
    rangeToTarget = distance_from_object(0, tracked);
    boltSpeed = 100;
    if (g_nCurrentView_00492fa8 == 4) {
        boltSpeed =
            g_aObjectTypeData_00496d30[
                OBJECT_TYPE_TIGERS_CLAW].maximumVelocity;
    } else {
        gun = (signed char)g_aShipWeapons_004956b0[0][0];
        while (gun-- > 0) {
            if (((ShipWeaponSlot *)(
                    g_aShipWeapons_004956b0[0] + 1))[gun].disabled == 0 &&
                g_aObjectTypeData_00496d30[
                    ((ShipWeaponSlot *)(g_aShipWeapons_004956b0[0] + 1))[
                        gun].weaponType].objectClass ==
                    OBJECT_CLASS_PROJECTILE &&
                boltSpeed <
                    g_aObjectTypeData_00496d30[
                        ((ShipWeaponSlot *)(
                            g_aShipWeapons_004956b0[0] + 1))[
                            gun].weaponType].maximumVelocity)
                boltSpeed =
                    g_aObjectTypeData_00496d30[
                        ((ShipWeaponSlot *)(
                            g_aShipWeapons_004956b0[0] + 1))[
                            gun].weaponType].maximumVelocity;
        }
    }

    vector_component_in_dir(
        &g_aShipVelocity_00494898[0],
        &g_aShipForwardVector_00494208[0],
        &combinedProjectileVelocity);
    ScaleFixedVector(
        &g_aShipForwardVector_00494208[0],
        (int)boltSpeed << 8, &scratch);
    AddFixedVectors(
        &scratch, &combinedProjectileVelocity,
        &combinedProjectileVelocity);
    boltSpeed =
        (short)((Vector_magnitude(&combinedProjectileVelocity) >> 8) + 1);
    frames = (short)(rangeToTarget / boltSpeed);
    zero_vector(&targetMotion);
    if (frames > 0)
        ScaleFixedVector(
            &g_aShipVelocity_00494898[tracked],
            (int)frames << 8, &targetMotion);
    AddFixedVectors(
        &g_aShipPosition_00494550[tracked],
        &targetMotion, &meetPoint);
    if (HasInRangeGunForTargetLead(
            distance_from_point(0, &meetPoint)) == 0) {
        SetHudMessageText(
            "OUT OF RANGE",
            g_cRedColour_0049cb5c, 20);
        g_bTargetLockActive_0049ae80 = 0;
        return;
    }

    ComputeVectorDelta(
        &g_aShipPosition_00494550[0], &meetPoint, &scratch);
    targetDistance = Vector_magnitude(&scratch);
    if ((int)g_asObjectCollisionRadius_004950e8[
            EYE_OBJECT] * 0x100 < targetDistance) {
        transform_to_objects_frame(
            &scratch, &eyeRelative, EYE_OBJECT);
        if ((int)g_asObjectCollisionRadius_004950e8[0] * 0x100 <=
                eyeRelative.z &&
            DivideFixed(eyeRelative.z, targetDistance) >= 0) {
            g_nTargetLeadIndicatorX_0049afe8 =
                (short)DivideFixed(
                    MultiplyFixed(g_nScreenWidth_0049d4d8 >> 1,
                                  eyeRelative.x),
                    eyeRelative.z);
            g_nTargetLeadIndicatorX_0049afe8 =
                (short)(g_nViewCenterX_005c80d8 +
                        g_nTargetLeadIndicatorX_0049afe8);
            g_nTargetLeadingIndicatorY_005d1c2a =
                (short)DivideFixed(
                    MultiplyFixed(g_nScreenWidth_0049d4d8 >> 1,
                                  eyeRelative.y),
                    eyeRelative.z);
            g_nTargetLeadingIndicatorY_005d1c2a =
                (short)(g_nViewCenterY_005c80da +
                        g_nTargetLeadingIndicatorY_005d1c2a);
        }
    }
    if (g_nTargetLeadIndicatorX_0049afe8 >= 10 &&
        g_nTargetLeadIndicatorX_0049afe8 <= 310 &&
        g_nTargetLeadingIndicatorY_005d1c2a >= 5 &&
        g_nTargetLeadingIndicatorY_005d1c2a <= 190) {
        CaptureSpriteBackground(
            &g_stViewScratchInfo_005d2b00,
            g_pCockpitHudBackground_0049b044,
            g_nTargetLeadIndicatorX_0049afe8,
            g_nTargetLeadingIndicatorY_005d1c2a,
            g_pCabinHudImage_005d21f4, 5);
        DrawSpriteDefault(
            &g_stViewScratchInfo_005d2b00,
            g_nTargetLeadIndicatorX_0049afe8,
            g_nTargetLeadingIndicatorY_005d1c2a,
            g_pCabinHudImage_005d21f4, 5);
    } else {
        g_nTargetLeadIndicatorX_0049afe8 = 0x7fff;
    }
}

/* Function start: 0x43D323 */
void RestoreTargetLeadIndicator(void)
{
    if ((signed char)g_bTargetLockDisplayEnabled_0049afe0 == 0)
        return;
    if (g_nTargetLeadIndicatorX_0049afe8 != 0x7fff) {
        RestoreSpriteBackground(&g_stViewScratchInfo_005d2b00,
                                g_pCockpitHudBackground_0049b044,
                                g_nTargetLeadIndicatorX_0049afe8,
                                g_nTargetLeadingIndicatorY_005d1c2a,
                                g_pCabinHudImage_005d21f4, 5);
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
        (signed char)g_bTargetLockDisplayEnabled_0049afe0 != 0) {
        g_bTargetLockActive_0049ae80 = 1;
        SetHudMessageText(
            "ITTS ENGAGED",
            g_cRedColour_0049cb5c, 20);
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
        g_cEarlierTargetObjectValue_005d1d8e =
            (signed char)g_nCommSpeakerObject_0049b794;
        draw_target_box(g_bPrimaryViewBufferColour_0049cb50,
                        g_cEarlierTargetObjectValue_005d1d8e,
                        0, 0, 2,
                        &g_stCommTargetBracketBounds_0049b2a8);
    }
    if (g_nCurrentView_00492fa8 == 4 &&
        g_nAimCameraMode_005c8d50 == 1 &&
        g_acShipTarget_00495f20[0] != -1) {
        if (g_nTargetCameraFrame_0049d3e8 == 0) {
            draw_target_box(g_cYellowColour_0049cb58,
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
        g_nAimCameraMode_005c8d50 != 1) {
        if (g_nTargetLockCountdown_004934ec == 0) {
            if (g_nRenderedSpaceFrame_00493138 % 2 == 0)
                g_bTargetBracketVisible_0049b284 ^= 1;
            if (g_bTargetBracketVisible_0049b284 == 1)
                draw_target_box(
                    g_cRedColour_0049cb5c,
                    g_acShipTarget_00495f20[0],
                    g_bTargetLockMode_00493500,
                    1, 1, &g_stTargetBracketBounds_0049b2a0);
        } else {
            draw_target_box(
                g_cRedColour_0049cb5c,
                g_acShipTarget_00495f20[0],
                g_bTargetLockMode_00493500,
                1, 1, &g_stTargetBracketBounds_0049b2a0);
        }
    }
    if (g_nCurrentView_00492fa8 != 4 ||
        g_nAimCameraMode_005c8d50 != 1) {
        if (IsSpriteFrameOverlappingRect(
                &reticleBounds,
                g_nTargetLeadIndicatorX_0049afe8,
                g_nTargetLeadingIndicatorY_005d1c2a,
                g_pCabinHudImage_005d21f4, 5) != 0) {
            g_bMissileLockAcquired_0049b2b0++;
            DrawSpriteDefault(
                &g_stViewScratchInfo_005d2b00,
                (short)(g_nViewCenterX_005c80d8 + 1),
                g_nViewCenterY_005c80da,
                g_pCabinHudImage_005d21f4, 4);
        } else {
            g_bMissileLockAcquired_0049b2b0 = 0;
            DrawSpriteDefault(
                &g_stViewScratchInfo_005d2b00,
                g_nViewCenterX_005c80d8,
                g_nViewCenterY_005c80da,
                g_pCabinHudImage_005d21f4, 0);
        }
    } else {
        DrawSpriteDefault(
            &g_stViewScratchInfo_005d2b00,
            g_nViewCenterX_005c80d8,
            g_nViewCenterY_005c80da,
            g_pCabinHudImage_005d21f4, 0);
    }
    DAT_004934e4 = 0;
    if (g_nCockpitControlState_0049d7ac == 1) {
        CaptureSpriteBackground(
            &g_stViewScratchInfo_005d2b00,
            (unsigned char *)g_stPersonnelCursor_005c8470.scratchBuffer,
            g_stPersonnelCursor_005c8470.x,
            g_stPersonnelCursor_005c8470.y,
            GetInputCursorShape(), g_stPersonnelCursor_005c8470.frame);
        DrawSpriteDefault(
            &g_stViewScratchInfo_005d2b00,
            g_stPersonnelCursor_005c8470.x,
            g_stPersonnelCursor_005c8470.y,
            GetInputCursorShape(), g_stPersonnelCursor_005c8470.frame);
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
            &g_stViewScratchInfo_005d2b00,
            (unsigned char *)g_stPersonnelCursor_005c8470.scratchBuffer,
            g_stPersonnelCursor_005c8470.x,
            g_stPersonnelCursor_005c8470.y,
            GetInputCursorShape(), g_stPersonnelCursor_005c8470.frame);
    if (g_bTargetLockActive_0049ae80 != 0)
        RestoreTargetLeadIndicator();
    if (g_cEarlierTargetObjectValue_005d1d8e != -1) {
        draw_target_box(g_cPrimaryViewBufferColour_0049cb88,
                        g_cEarlierTargetObjectValue_005d1d8e,
                        0, 0, 2,
                        &g_stCommTargetBracketBounds_0049b2a8);
        g_cEarlierTargetObjectValue_005d1d8e = -1;
    }
    draw_target_box(g_cPrimaryViewBufferColour_0049cb88,
                    g_acShipTarget_00495f20[0],
                    g_bTargetLockMode_00493500, 1, 1,
                    &g_stTargetBracketBounds_0049b2a0);
    if (IsCockpitExplosionActive() &&
        g_pCockpitExplosionBackground_0049b040 != 0) {
        CaptureSpriteBackground(
            &g_stScrViewInfo_005d21a0,
            g_pCockpitExplosionBackground_0049b040,
            g_stCockpitExplosionPosBlock_005d1e98.x,
            g_stCockpitExplosionPosBlock_005d1e98.y,
            g_pCockpitExplosionShape_0049b048,
            g_nCockpitExplosionFrame_0049b04c);
        DrawSpriteDefault(&g_stScrViewInfo_005d21a0,
                          g_stCockpitExplosionPosBlock_005d1e98.x,
                          g_stCockpitExplosionPosBlock_005d1e98.y,
                          g_pCockpitExplosionShape_0049b048,
                          g_nCockpitExplosionFrame_0049b04c);
        g_cLastPilotHandFrame_0049aff4 = 0xff;
    }
}

/* Function start: 0x43D956 */
void SetHudMessageText(char *text, unsigned short colour,
                       unsigned short duration)
{
    if (g_bInflightComputerActive_0049bcb4 != 0)
        return;
    if (message_showing())
        ClearHudMessageDisplay(1);
    g_pszPendingHudMessage_0049afec = text;
    g_nHudMessageColour_005d1ef0 = colour;
    set_message_time(duration);
}

/* Function start: 0x43D9AB */
/* Take a HUD message down early so the next transmission can replace it. */
void DismissHudMessageIfShowing(void)
{
    if (message_showing() != 0)
        ClearHudMessageDisplay(1);
    set_message_time(-1);
}

/* Function start: 0x43D9DA */
void malf_noise(short vdu, int effect, unsigned short colour,
                short sound, short refresh)
{
    Viewport *viewport = vdu == 0 ? &g_stLeftwardVduPortBlock_005d2180 : &g_stRightVduView_005d2b20;

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
    short frameBounds[4];
    unsigned char specialSlot;
    short posY;
    short posX;
    unsigned short distance;
    signed char contact;

    *hasEnemy = 0;
    g_cViableTargetCount_00496178 = 0;
    if (g_nCurrentView_00492fa8 == 4 &&
        g_nAimCameraMode_005c8d50 == 1) {
        if (g_nTargetCameraZoom_0049d3e4 >= 33) {
            for (specialSlot = 0; specialSlot < 10;
                 specialSlot++) {
                if (((g_aeObjectClass_00495328[specialSlot] !=
                          OBJECT_CLASS_NULL &&
                      g_asObjectType_00495298[specialSlot] == 0x3d) ||
                     g_asObjectType_00495298[specialSlot] == 0x2c) &&
                    g_asObjectScreenX_00493598[specialSlot] !=
                        (short)0x8001) {
                    posX = (short)(
                        g_asObjectScreenX_00493598[specialSlot] +
                        g_nViewCenterX_005c80d8);
                    posY = (short)(
                        g_asObjectScreenY_00493628[specialSlot] +
                        g_nViewCenterY_005c80da);
                    GetTransformedShapeBounds(
                        &g_stViewScratchInfo_005d2b00, posX, posY,
                        g_apObjectShape_00493868[specialSlot],
                        g_asObjectViewFrame_00493508[specialSlot],
                        g_asObjectScreenAngle_004936b8[specialSlot],
                        g_asObjectScreenScale_00493a58[specialSlot],
                        g_asObjectFlip_004939c8[specialSlot], frameBounds);
                    if (IsSpriteFrameOverlappingRect(
                            (const ShortRect *)frameBounds,
                            g_nViewCenterX_005c80d8,
                            g_nViewCenterY_005c80da,
                            g_pCabinHudImage_005d21f4,
                            0) != 0) {
                        distance = (unsigned short)
                            g_asObjectDistance_00493ae8[specialSlot];
                        if (distance < 4000) {
                            g_acViableTarget_00496180[
                                (int)g_cViableTargetCount_00496178] =
                                    (signed char)specialSlot;
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
        for (contact = 1; contact <= 9; contact++) {
            if (g_aeObjectClass_00495328[(int)contact] >=
                    OBJECT_CLASS_SHIP &&
                g_aeSpecialManeuver_00495600[(int)contact] !=
                    SPECIAL_MANEUVER_UNKNOWN_9 &&
                g_asObjectScreenX_00493598[(int)contact] !=
                    (short)0x8001) {
                distance = (unsigned short)
                    g_asObjectDistance_00493ae8[(int)contact];
                if (distance < 12000) {
                    g_acViableTarget_00496180[
                        (int)g_cViableTargetCount_00496178] = contact;
                    g_asViableTargetDistance_00496190[
                        (int)g_cViableTargetCount_00496178] =
                            (short)distance;
                    g_cViableTargetCount_00496178++;
                    if (g_asShipSide_004955d0[(int)contact] !=
                        g_asShipSide_004955d0[0])
                        *hasEnemy = 1;
                }
            }
        }
    }
    if (g_cViableTargetCount_00496178 > 1)
        SortViableTargetsByDistance();
}

void build_your_target_list(short *hasEnemy);
short distance_from_object(short obj, short other);
short malf(char component);
short FindNearestMissileTargetingObject(short obj);

/* Function start: 0x43DD1B */
void cycle_onscreen_targets(void)
{
    short anyEnemy;
    signed char index;
    short entryTarget;

    entryTarget = g_acShipTarget_00495f20[0];
    build_your_target_list(&anyEnemy);
    if (g_cViableTargetCount_00496178 == 0) {
        g_acShipTarget_00495f20[0] = -1;
    } else {
        index = 0;
        while (index < g_cViableTargetCount_00496178 &&
               g_acViableTarget_00496180[(int)index] !=
                   g_acShipTarget_00495f20[0])
            index++;
        do {
            index = (signed char)((index + 1) %
                                  g_cViableTargetCount_00496178);
            g_acShipTarget_00495f20[0] =
                g_acViableTarget_00496180[(int)index];
        } while (anyEnemy != 0 &&
                 g_asShipSide_004955d0[g_acShipTarget_00495f20[0]] ==
                     g_asShipSide_004955d0[0]);
    }
    if (g_acShipTarget_00495f20[0] != entryTarget)
        g_nTargetLockCountdown_004934ec = -1;
}

/* Function start: 0x43DDFC */
void check_target(void)
{
    short reselect;
    short anyEnemy;
    short targetIndex;
    short previousTarget;
    short target;

    target = g_acShipTarget_00495f20[0];
    reselect = 1;
    if (g_nTargetCameraFrame_0049d3e8 == 1)
        return;
    if (target != -1 &&
        g_aeSpecialManeuver_00495600[target] ==
            SPECIAL_MANEUVER_UNKNOWN_9) {
        g_acShipTarget_00495f20[0] = -1;
        target = g_acShipTarget_00495f20[0];
    }
    previousTarget = target;
    if (g_bTargetLockMode_00493500 != 0 &&
        g_nRenderedSpaceFrame_00493138 % 8 == 0 &&
        malf(5) != 0) {
        g_bTargetLockMode_00493500 = 0;
        g_bTargetLockActive_0049ae80 = 0;
        malf_sound();
    }

    /* Keep the current target while the lock holds it, or while it is on
     * screen and hostile; otherwise fall through and pick a new one. */
    if (target == -1 ||
        (g_bTargetLockMode_00493500 == 0 &&
         g_asObjectScreenX_00493598[target] == (short)0x8001) ||
        (g_bTargetLockMode_00493500 == 0 &&
         g_asShipSide_004955d0[target] == g_asShipSide_004955d0[0])) {
        if (target == -1 && g_nCurrentView_00492fa8 != 4)
            g_bTargetLockMode_00493500 = 0;

        build_your_target_list(&anyEnemy);
        if (g_cViableTargetCount_00496178 == 0) {
            if (g_nCurrentView_00492fa8 == 4 &&
                g_nAimCameraMode_005c8d50 == 1) {
                g_acShipTarget_00495f20[0] = -1;
                if (previousTarget != -1)
                    g_nTargetCameraZoom_0049d3e4 = 0x21;
            } else if (g_bTargetLockMode_00493500 != 0) {
                g_acShipTarget_00495f20[0] = (signed char)previousTarget;
            } else {
                g_acShipTarget_00495f20[0] = -1;
            }
        } else {
            if (anyEnemy == 0 && previousTarget != -1 &&
                g_asShipSide_004955d0[previousTarget] ==
                    g_asShipSide_004955d0[0] &&
                (g_bTargetLockMode_00493500 != 0 ||
                 g_asObjectScreenX_00493598[target] != (short)0x8001)) {
                g_acShipTarget_00495f20[0] = (signed char)previousTarget;
                reselect = 0;
            }
            if (reselect != 0) {
                targetIndex = 0;
                while (g_cViableTargetCount_00496178 > targetIndex &&
                       g_asShipSide_004955d0[
                           (short)g_acViableTarget_00496180[
                               targetIndex]] ==
                           g_asShipSide_004955d0[0])
                    targetIndex++;
                g_acShipTarget_00495f20[0] =
                    g_acViableTarget_00496180[
                        targetIndex %
                        (short)g_cViableTargetCount_00496178];
            }
        }
        if (g_acShipTarget_00495f20[0] != previousTarget) {
            if (g_nCurrentView_00492fa8 != 4 && previousTarget != -1 &&
                (g_acShipTarget_00495f20[0] == -1 ||
                 g_asShipSide_004955d0[target] !=
                     g_asShipSide_004955d0[previousTarget]))
                g_bTargetLockMode_00493500 = 0;
            g_nTargetLockCountdown_004934ec = -1;
        }
    }
}

/* Function start: 0x43E101 */
/* The closest missile locked onto this object, or zero if none is. */
short FindNearestMissileTargetingObject(short obj)
{
    short nearest;
    short other;
    short distance;

    nearest = 0;
    for (other = 1; other < 10; other++) {
        if (g_aeObjectClass_00495328[other] == OBJECT_CLASS_MISSILE &&
            g_acShipTarget_00495f20[other] == obj) {
            distance = distance_from_object(other, obj);
            if (nearest == 0 ||
                distance_from_object(nearest, obj) > distance)
                nearest = other;
        }
    }
    return nearest;
}

/* Function start: 0x43E1B2 */
void update_missile_warning(void)
{
    short period;
    short distance;
    short missile;

    if (FindMissileTargetingObject(0) != 0) {
        missile = FindNearestMissileTargetingObject(0);
        distance = distance_from_object(0, missile);
        if (distance < 500)
            period = 6;
        else if (distance < 1000)
            period = 5;
        else if (distance < 2000)
            period = 4;
        else if (distance < 4000)
            period = 3;
        else if (distance < 8000)
            period = 2;
        else
            period = 1;
        SetCockpitLightBlink(2, period);
    } else {
        g_abCockpitLightGoalTable_005d1eb8[2] = 0;
    }
}

/* Function start: 0x43E297 */
void determine_pilot_hand(void)
{
    short yaw;
    short pitch;

    yaw = g_anFlightInput_004931a8[1] / 2;
    pitch = g_anFlightInput_004931a8[0] / 2;
    if (yaw > 0) {
        g_cPilotHandFrameCode_005d1c30 =
            (unsigned char)MinShort((short)(yaw + 8), 12);
        return;
    }
    if (yaw < 0) {
        g_cPilotHandFrameCode_005d1c30 =
            (unsigned char)MinShort((short)(4 - yaw), 8);
        return;
    }
    if (pitch > 0) {
        g_cPilotHandFrameCode_005d1c30 =
            (unsigned char)MinShort((short)(pitch + 12), 16);
        return;
    }
    if (pitch < 0) {
        g_cPilotHandFrameCode_005d1c30 =
            (unsigned char)MinShort((short)-pitch, 4);
        return;
    }
    g_cPilotHandFrameCode_005d1c30 = 0;
}

/* Function start: 0x43E371 */
void DrawPilotHandFrame(void)
{
    short x;
    short y;

    x = (short)(g_stPilotHandOrigin_0049af90.x -
                g_stDashViewport_005d2160.left);
    y = (short)(g_stPilotHandOrigin_0049af90.y -
                g_stDashViewport_005d2160.top);
    CopyViewportContents(&g_stPilotHandBackgroundViewportBlock_005d2b40,
                         &g_stPilotHandViewportRecord_005d2c70);
    DrawSpriteDefault(&g_stPilotHandViewportRecord_005d2c70, x, y,
                      g_pPilotHandAnimationShapePtr_005d2c64,
                      (short)(signed char)g_cPilotHandFrameCode_005d1c30);
    DrawSpriteDefault(
        &g_stPilotHandViewportRecord_005d2c70,
        (short)(x + g_asPilotHandOffsets_0049aff8[
            (signed char)g_cPilotHandFrameCode_005d1c30 * 2]),
        (short)(y + g_asPilotHandOffsets_0049aff8[
            (signed char)g_cPilotHandFrameCode_005d1c30 * 2 + 1]),
        g_pPilotHandAnimationShapePtr_005d2c64, 0x11);
    CopyViewportContents(&g_stPilotHandViewportRecord_005d2c70,
                         &g_stDashViewport_005d2160);
    g_cLastPilotHandFrame_0049aff4 = g_cPilotHandFrameCode_005d1c30;
}

/* Function start: 0x43E43A */
void animate_pilot(void)
{
    if (g_pPilotHandAnimationShapePtr_005d2c64 != 0) {
        determine_pilot_hand();
        if (g_cLastPilotHandFrame_0049aff4 != g_cPilotHandFrameCode_005d1c30)
            DrawPilotHandFrame();
    }
}

/* Function start: 0x43E472 */
void ResetPilotHandAnimation(void)
{
    if (g_pPilotHandAnimationShapePtr_005d2c64 != 0) {
        g_cLastPilotHandFrame_0049aff4 = 0xff;
        CopyViewportContents(&g_stDashViewport_005d2160,
                             &g_stPilotHandBackgroundViewportBlock_005d2b40);
        animate_pilot();
    }
}

void vid_equiv(short obj, short message);
void real_vid_transmit(short obj, short message);

/* Function start: 0x43E4A8 */
void send_message(short obj, signed char message)
{
    if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_NULL ||
        g_nTrainSimActive_0049d758 != 0)
        return;
    if (g_nYourWingman_0049346c != -1 &&
        g_nYourWingman_0049346c == obj &&
        g_bRadioSilence_0049b780 != 0) {
        g_acShipPendingMessage_00495d98[obj] = -1;
    } else if (obj > -1 && obj < 10 &&
               g_aeObjectClass_00495328[obj] >= OBJECT_CLASS_SHIP) {
        if (g_acShipPortrait_00495d88[obj] != -1) {
            g_acShipPendingMessage_00495d98[obj] = message;
        } else if (g_asShipMissionIndex_00495d00[obj] ==
                       g_nHomeOperationShipIndexValue_005d1e22 ||
                   g_asShipMissionIndex_00495d00[obj] ==
                       g_asShipMissionParameter_00495e00[0]) {
            g_acShipPendingMessage_00495d98[obj] = message;
        } else if (g_asShipSide_004955d0[obj] == SIDE_KILRATHI) {
            g_acShipPendingMessage_00495d98[obj] = message;
        }
    }
}

/* Function start: 0x43E5DA */
void npc_communication(void)
{
    signed char keptPortrait;
    signed char caller;
    signed char oldIndex;
    MissionShipRecord *savedRecord;
    signed char i;

    if (g_nTrainSimActive_0049d758 == 0) {
        if (g_cQueuedNpcPortrait_0049b8c4 != -1) {
            keptPortrait = g_acShipPortrait_00495d88[0];
            oldIndex = (signed char)g_asShipMissionIndex_00495d00[0];
            savedRecord = g_apShipMissionRecord_00495da8[0];
            g_acShipPortrait_00495d88[0] = g_cQueuedNpcPortrait_0049b8c4;
            for (i = 0; i < 16; i++) {
                if (g_aMissionShips_00492290[i].portrait ==
                    g_cQueuedNpcPortrait_0049b8c4)
                    break;
            }
            if (i == 16)
                ShowOnScreenMessage(0x270f, "SHOW_MESSAGE failed");
            g_asShipMissionIndex_00495d00[0] = i;
            g_apShipMissionRecord_00495da8[0] = &g_aMissionShips_00492290[i];
            real_vid_transmit(0, g_cQueuedNpcMessage_0049b8c8);
            g_apShipMissionRecord_00495da8[0] =
                &g_aMissionShips_00492290[g_asShipMissionIndex_00495d00[0]];
            g_acShipPortrait_00495d88[0] = keptPortrait;
            g_asShipMissionIndex_00495d00[0] = oldIndex;
            g_cQueuedNpcPortrait_0049b8c4 = g_cQueuedNpcMessage_0049b8c8 = -1;
        }
        for (caller = 1;
             message_showing() == 0 && g_nCommDeathSequenceFrame_0049ae84 == 0 &&
                 caller <= 9;
             caller++) {
            if (g_aeObjectClass_00495328[caller] >= OBJECT_CLASS_SHIP &&
                g_acShipPendingMessage_00495d98[caller] != -1) {
                vid_equiv(caller, g_acShipPendingMessage_00495d98[caller]);
                g_acShipPendingMessage_00495d98[caller] = -1;
            }
        }
        if (RandomBelowOrEqual(5000) > 4998) {
            for (caller = 1;
                 g_nCommSpeakerObject_0049b794 == -1 && caller <= 9;
                 caller++) {
                if (g_aeObjectClass_00495328[caller] >= OBJECT_CLASS_SHIP &&
                    g_asShipSide_004955d0[caller] == SIDE_KILRATHI &&
                    (g_aeShipObjective_00495f08[caller] == OBJECTIVE_ENGAGE_ENEMY ||
                     g_aeShipObjective_00495f08[caller] == OBJECTIVE_DESTROY_SHIP) &&
                    (g_asPilotLevel_00495d60[caller] == 5 ||
                     RandomBelowOrEqual(100) < 20)) {
                    g_acShipPendingMessage_00495d98[caller] =
                        (signed char)(RandomBelowOrEqual(2) + 0x19);
                    break;
                }
            }
        }
    }
}

/* Function start: 0x43E870 */
void clear_cockpit_damage(void)
{
    short *damageState;
    short damage;

    damageState = g_asCockpitDmgStateList_005d1ee8;
    for (damage = 0; damage < 4; damage++, damageState++)
        *damageState = 0;
}

/* Function start: 0x43E8B2 */
void explosion_draw(int useBackgroundViewport)
{
    short hits;
    short x;
    short y;

    if (g_pPendingCockpitDamageShape_0049b03c == 0)
        return;
    for (hits = 0; hits < 4; hits++) {
        if (g_asCockpitDmgStateList_005d1ee8[hits] == 1) {
            switch (hits) {
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
            x = g_aCockpitDamagePositions_0049ae98[hits].x;
            y = g_aCockpitDamagePositions_0049ae98[hits].y;
            if (useBackgroundViewport == 0) {
                DrawSpriteDefault(
                    &g_stScrViewInfo_005d21a0, x, y,
                    g_pPendingCockpitDamageShape_0049b03c, hits);
            } else {
                DrawSpriteDefault(
                    &g_stCockpitBackgroundViewportRecord_005d1050, x, y,
                    g_pPendingCockpitDamageShape_0049b03c, hits);
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
    switch (g_nPendingDashDamageValue_005d1c34) {
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
    DrawSpriteDefault(&g_stScrViewInfo_005d21a0,
                      g_stCockpitExplosionPosBlock_005d1e98.x,
                      g_stCockpitExplosionPosBlock_005d1e98.y,
                      g_pPendingCockpitDamageShape_0049b03c,
                      g_nPendingDashDamageValue_005d1c34);
    if (g_pPilotHandAnimationShapePtr_005d2c64 != 0) {
        DrawSpriteDefault(
            &g_stPilotHandBackgroundViewportBlock_005d2b40,
            (short)(g_stCockpitExplosionPosBlock_005d1e98.x -
                    g_stDashViewport_005d2160.left),
            (short)(g_stCockpitExplosionPosBlock_005d1e98.y -
                    g_stDashViewport_005d2160.top),
            g_pPendingCockpitDamageShape_0049b03c,
            g_nPendingDashDamageValue_005d1c34);
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
            &g_stScrViewInfo_005d21a0,
            g_pCockpitExplosionBackground_0049b040,
            g_stCockpitExplosionPosBlock_005d1e98.x,
            g_stCockpitExplosionPosBlock_005d1e98.y,
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
        g_nCockpitExplosionFrame_0049b04c++;
        if (g_nCockpitExplosionFrame_0049b04c == 3)
            DrawPendingCockpitDamage();
        if (IsCockpitExplosionActive() &&
            g_pCockpitExplosionShape_0049b048 != 0 &&
            g_pCockpitExplosionBackground_0049b040 != 0) {
            CaptureSpriteBackground(
                &g_stScrViewInfo_005d21a0,
                g_pCockpitExplosionBackground_0049b040,
                g_stCockpitExplosionPosBlock_005d1e98.x,
                g_stCockpitExplosionPosBlock_005d1e98.y,
                g_pCockpitExplosionShape_0049b048,
                g_nCockpitExplosionFrame_0049b04c);
            DrawSpriteDefault(&g_stScrViewInfo_005d21a0,
                              g_stCockpitExplosionPosBlock_005d1e98.x,
                              g_stCockpitExplosionPosBlock_005d1e98.y,
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
        g_asCockpitDmgStateList_005d1ee8[damage] == 0) {
        g_asCockpitDmgStateList_005d1ee8[damage] = 1;
        g_nPendingDashDamageValue_005d1c34 = damage;
        if (g_pCockpitExplosionShape_0049b048 == 0) {
            explosion_draw(0);
        } else if (IsCockpitExplosionActive() == 0) {
            g_stCockpitExplosionPosBlock_005d1e98 =
                g_aCockpitDamagePositions_0049ae98[damage];
            g_nCockpitExplosionFrame_0049b04c = 0x7fff;
        }
    }
}

/* Function start: 0x43ECD9 */
void vid_transmit(void)
{
    short frame;

    if (g_nCommDeathSequenceFrame_0049ae84 != 0) {
        if (g_nCommDeathSequenceFrame_0049ae84 == 1) {
            DrawSpriteDefault(&g_stRightVduView_005d2b20,
                              g_stRightVduView_005d2b20.left,
                              g_stRightVduView_005d2b20.top,
                              g_pCommPortraitResource_0049b788, 0);
            DrawSpriteDefault(&g_stRightVduView_005d2b20,
                              g_stRightVduView_005d2b20.left,
                              g_stRightVduView_005d2b20.top,
                              g_pCommPortraitResource_0049b788, 5);
            g_nCommDeathSequenceFrame_0049ae84++;
        } else if (g_nCommDeathSequenceFrame_0049ae84 == 2) {
            DrawSpriteDefault(&g_stRightVduView_005d2b20,
                              g_stRightVduView_005d2b20.left,
                              g_stRightVduView_005d2b20.top,
                              g_pCommPortraitResource_0049b788, 0);
            DrawSpriteDefault(&g_stRightVduView_005d2b20,
                              g_stRightVduView_005d2b20.left,
                              g_stRightVduView_005d2b20.top,
                              g_pCommPortraitResource_0049b788, 6);
            g_nCommDeathSequenceFrame_0049ae84++;
        } else if (g_nCommDeathSequenceFrame_0049ae84 == 3) {
            DrawSpriteDefault(&g_stRightVduView_005d2b20,
                              g_stRightVduView_005d2b20.left,
                              g_stRightVduView_005d2b20.top,
                              g_pCommVduFrameResource_0049b78c, 0);
            g_nCommDeathSequenceFrame_0049ae84++;
        } else {
            EndCommSessionWithWingman();
            g_nCommDeathSequenceFrame_0049ae84 = 0;
        }
    } else if (g_bCommSpeechPlaying_0049b7a0 == 0 &&
               COMM_SPEAKER_SIDE(g_nCommSpeakerObject_0049b794) ==
                   SIDE_NEUTRAL) {
        EndCommMenu();
    } else if (((g_nRenderedSpaceFrame_00493138 % 2 != 0 &&
                 g_nCockpitDisplayMode_0049d71c == 0) ||
                g_nCockpitDisplayMode_0049d71c != 0) &&
               g_nCommPortraitIndex_0049b79c != -1 &&
               g_bVideoImagesSuppressed_0049b784 == 0 &&
               g_szCommMessageTextText_005d18f0 != 0) {
        if (g_aeSpecialManeuver_00495600[g_nCommSpeakerObject_0049b794] ==
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_bCommSpeechPlaying_0049b7a0 == 0 &&
            g_asShipBurstStageCountdown_005d3850[
                g_nCommSpeakerObject_0049b794] < 7) {
            if (g_asShipBurstStageCountdown_005d3850[
                    g_nCommSpeakerObject_0049b794] < 3) {
                DrawSpriteDefault(&g_stRightVduView_005d2b20,
                                  g_stRightVduView_005d2b20.left,
                                  g_stRightVduView_005d2b20.top,
                                  g_pCommVduFrameResource_0049b78c, 0);
            } else {
                if (g_asShipBurstStageCountdown_005d3850[
                        g_nCommSpeakerObject_0049b794] >= 5)
                    frame = 5;
                else if (g_asShipBurstStageCountdown_005d3850[
                             g_nCommSpeakerObject_0049b794] >= 3)
                    frame = 6;
                DrawSpriteDefault(&g_stRightVduView_005d2b20,
                                  g_stRightVduView_005d2b20.left,
                                  g_stRightVduView_005d2b20.top,
                                  g_pCommPortraitResource_0049b788, 0);
                DrawSpriteDefault(&g_stRightVduView_005d2b20,
                                  g_stRightVduView_005d2b20.left,
                                  g_stRightVduView_005d2b20.top,
                                  g_pCommPortraitResource_0049b788, frame);
            }
        } else {
            if (g_nCommPortraitFrame_0049b2bc == -1)
                g_nCommPortraitFrame_0049b2bc = RandomInRange(1, 4);
            g_nCommPortraitAnimationTickWord_005d1d94 = RandomInRange(1, 8);
            if (g_nCommPortraitAnimationTickWord_005d1d94 < 5)
                g_nCommPortraitFrame_0049b2bc =
                    g_nCommPortraitAnimationTickWord_005d1d94;
            DrawSpriteDefault(&g_stRightVduView_005d2b20,
                              g_stRightVduView_005d2b20.left,
                              g_stRightVduView_005d2b20.top,
                              g_pCommPortraitResource_0049b788, 0);
            DrawSpriteDefault(&g_stRightVduView_005d2b20,
                              g_stRightVduView_005d2b20.left,
                              g_stRightVduView_005d2b20.top,
                              g_pCommPortraitResource_0049b788,
                              (short)g_nCommPortraitFrame_0049b2bc);
        }
    }
}

/* Function start: 0x43F042 */
void vid_equiv(short obj, short message)
{
    if (get_mode(1) != 4 &&
        g_nTrainSimActive_0049d758 == 0 &&
        g_nCurrentView_00492fa8 == 0 &&
        message_showing() == 0)
        real_vid_transmit(obj, message);
}

/* Function start: 0x43F09D */
void update_dead_disp(short a)
{
    malf_noise(a, 1, g_cDarkGreenColour_0049cb60, 0x17, 0);
}

/* Function start: 0x43F0C3 */
void check_stranded(void)
{
    if (g_aMissionShips_00492290[
            g_nHomeOperationShipIndexValue_005d1e22].state == 3 &&
        any_enemy(0, 30000) == 0)
        g_nArcadeState_0049d75c = 3;
}

/* Function start: 0x43F110 */
void CopyTrainSimPilotViewToVdus(void)
{
    ShortRect rightBounds;
    ShortRect cockpitBounds;
    Viewport handViewport;
    ShortRect overlap;
    ShortRect leftBounds;

    handViewport = g_stPilotHandBackgroundViewportBlock_005d2b40;
    rightBounds.left = g_stRightVduView_005d2b20.left;
    rightBounds.top = g_stRightVduView_005d2b20.top;
    rightBounds.right = g_stRightVduView_005d2b20.right;
    rightBounds.bottom = g_stRightVduView_005d2b20.bottom;
    leftBounds.left = g_stLeftwardVduPortBlock_005d2180.left;
    leftBounds.top = g_stLeftwardVduPortBlock_005d2180.top;
    leftBounds.right = g_stLeftwardVduPortBlock_005d2180.right;
    leftBounds.bottom = g_stLeftwardVduPortBlock_005d2180.bottom;
    cockpitBounds.left = g_stDashViewport_005d2160.left;
    cockpitBounds.top = g_stDashViewport_005d2160.top;
    cockpitBounds.right = g_stDashViewport_005d2160.right;
    cockpitBounds.bottom = g_stDashViewport_005d2160.bottom;

    if (GetViewportIntersection(&overlap, &cockpitBounds,
                                &rightBounds) != 0) {
        if (g_stTrainSimRightVduSource_0049b2c0.left == 0) {
            g_stTrainSimRightVduSource_0049b2c0 =
                g_stDashViewport_005d2160;
            g_stTrainSimRightVduSource_0049b2c0.left =
                g_stRightVduView_005d2b20.left;
            g_stTrainSimRightVduSource_0049b2c0.right =
                g_stDashViewport_005d2160.right;
            g_stTrainSimRightVduSource_0049b2c0.top =
                g_stDashViewport_005d2160.top;
            g_stTrainSimRightVduSource_0049b2c0.bottom =
                g_stRightVduView_005d2b20.bottom;
        }
        handViewport.left = (short)(g_stTrainSimRightVduSource_0049b2c0.left -
                                    g_stDashViewport_005d2160.left);
        handViewport.bottom =
            (short)(g_stTrainSimRightVduSource_0049b2c0.bottom -
                    g_stDashViewport_005d2160.top);
        CopyViewportContents(&g_stTrainSimRightVduSource_0049b2c0,
                             &handViewport);
    }

    if (GetViewportIntersection(&overlap, &cockpitBounds,
                                &leftBounds) != 0) {
        if (g_stTrainSimLeftVduSource_0049b2d8.left == 0) {
            g_stTrainSimLeftVduSource_0049b2d8 =
                g_stDashViewport_005d2160;
            g_stTrainSimLeftVduSource_0049b2d8.left =
                g_stDashViewport_005d2160.left;
            g_stTrainSimLeftVduSource_0049b2d8.right =
                g_stLeftwardVduPortBlock_005d2180.right;
            g_stTrainSimLeftVduSource_0049b2d8.top =
                g_stDashViewport_005d2160.top;
            g_stTrainSimLeftVduSource_0049b2d8.bottom =
                g_stLeftwardVduPortBlock_005d2180.bottom;
        }
        handViewport.left = (short)(g_stTrainSimLeftVduSource_0049b2d8.left -
                                    g_stDashViewport_005d2160.left);
        handViewport.bottom =
            (short)(g_stTrainSimLeftVduSource_0049b2d8.bottom -
                    g_stDashViewport_005d2160.top);
        CopyViewportContents(&g_stTrainSimLeftVduSource_0049b2d8,
                             &handViewport);
    }
}

/* Function start: 0x43F2F5 */
void update_VDUs(void)
{
    if (g_nCurrentView_00492fa8 == 4)
        return;
    if (g_nCockpitDisplayMode_0049d71c != 0) {
        if (IsCockpitWeaponShapeLoaded() != 0) {
            DrawFilledViewportRect(
                &g_stScrViewInfo_005d21a0,
                g_stLeftwardVduPortBlock_005d2180.left,
                g_stLeftwardVduPortBlock_005d2180.top,
                g_stLeftwardVduPortBlock_005d2180.right,
                g_stLeftwardVduPortBlock_005d2180.bottom, 0);
        }
        DrawFilledViewportRect(
            &g_stScrViewInfo_005d21a0,
            g_stRightVduView_005d2b20.left,
            g_stRightVduView_005d2b20.top,
            g_stRightVduView_005d2b20.right,
            g_stRightVduView_005d2b20.bottom, 0);
    }
    if (IsCockpitWeaponShapeLoaded() != 0) {
        SetTextContext(&g_stLeftVduStringContextRecord_005d2ae0);
        if (update_vid_disp(0) != 0) {
            switch (get_mode(0)) {
            case 0:
                update_dead_disp(0);
                break;
            case 2:
                show_damage_disp();
                break;
            case 1:
                show_weapon_disp();
                break;
            }
        } else {
            switch (get_mode(0)) {
            case 0:
                update_dead_disp(0);
                break;
            case 2:
                if (g_nCockpitDisplayMode_0049d71c != 0) {
                    g_bForceDamageDisplayRedraw_0049b2ec = 1;
                    show_damage_disp();
                    g_bForceDamageDisplayRedraw_0049b2ec = 0;
                }
                UpdateDamageDisplay();
                break;
            case 1:
                if (g_nCockpitDisplayMode_0049d71c != 0)
                    show_weapon_disp();
                update_status_text();
                break;
            }
        }
        if (get_mode(0) == 0)
            g_aHudTextSlotsArray_005d1d40[0].text = 0;
        else
            UpdateMessage(&g_aHudTextSlotsArray_005d1d40[0]);
    }

    SetTextContext(&g_stRightVduTextContextBlock_005d2ce0);
    if (update_vid_disp(1) != 0) {
        switch (get_mode(1)) {
        case 0:
            update_dead_disp(1);
            break;
        case 4:
            show_communications_disp();
            break;
        case 5:
            show_navigation_disp();
            break;
        case 3:
        case 9:
            show_target_disp();
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
        case 3:
        case 9:
            if (g_nCockpitDisplayMode_0049d71c != 0)
                show_target_disp();
            DrawTargetRangeReadout();
            break;
        case 6:
            vid_transmit();
            break;
        }
    }
    if (get_mode(1) == 6 || get_mode(1) == 0)
        g_aHudTextSlotsArray_005d1d40[1].text = 0;
    else
        UpdateMessage(&g_aHudTextSlotsArray_005d1d40[1]);
    if (g_nCockpitDisplayMode_0049d71c == 0 &&
        g_pPilotHandAnimationShapePtr_005d2c64 != 0)
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
        if (g_nAimCameraMode_005c8d50 == 1) {
            if (g_nTargetCameraFrame_0049d3e8 == 1)
                UpdateTargetCameraTracking();
            vdu_polygon(1, g_nTargetCameraZoom_0049d3e4);
        } else {
            vdu_polygon(0, g_nGunShowEnergyPercent_005c8d4e);
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
    if (g_pDashWeaponPicture_005d2b54 != 0)
        return 1;
    else
        return 0;
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
    while ((short)GetVduModeStackDepth(vdu) > 0) {
        switch (get_mode(vdu)) {
        case 6:
            EndCommMenu();
            break;
        default:
            pop_mode(vdu);
        }
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
            g_nDamageShowTicksNumber_005d4250 = 0;
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

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
ReleaseWeaponPosition g_stReleaseWeaponReadoutLocationBlock_005d1c24;
short g_nReleaseWeaponShowKindWord_005d1c28;
short g_nTargetLeadingIndicatorY_005d1c2a;
unsigned char *g_pScannerBlipUnderlayPointer_005d1c2c;
signed char g_cPilotHandFrameCode_005d1c30;
short g_nScreenMessageTimeNumber_005d1c32;
short g_nPendingDashDamageValue_005d1c34;
char g_nSpareCockptSlot_005d1c36; /* never referenced */
signed char g_cOverlayMessageView_005d1c37;
char g_szTextBufferBuffer_005d1c40[256];
HudMessageSlot g_aHudTextSlotsArray_005d1d40[2];
int g_nUnusedCockptWordWord_005d1d70[3]; /* never referenced */
short g_asScannerTargetUnderlay_005d1d80[5];
char g_nUnusedCockptWord_005d1d8a; /* never referenced */
short g_nUnusedCockptWord_005d1d8c; /* never referenced */
signed char g_cEarlierTargetObjectValue_005d1d8e;
int g_nUnusedCockptWord_005d1d90; /* never referenced */
short g_nCommPortraitAnimationTickWord_005d1d94;
char g_szComponentHitMsg_005d1da0[40];
signed char g_abShipArrivalMessageSent_005d1dd0[16];
ShortPoint g_aWeaponDisplayPositions_005d1de0[16];
short g_nTargetLockonMarkerY_005d1e20;
short g_nHomeOperationShipIndexValue_005d1e22;
CockpitReadout g_aConsoleMetersTable_005d1e30[6];
short g_nScopePointerX_005d1e6c;
short g_nScannerCursorYValue_005d1e6e;
signed char g_abCockpitLightMode_005d1e70[7];
short g_nTargetLockMusicCooldown_005d1e78;
short g_asScannerObjectYTable_005d1e80[11];
volatile ShortPoint g_stCockpitExplosionPosBlock_005d1e98;
short g_asScannerObjectX_005d1ea0[11];
signed char g_abCockpitLightGoalTable_005d1eb8[7];
int g_nSevereDmgWarningSfxHandle_005d1ec0;
short g_asScannerBackgroundColourList_005d1ed0[10];
short g_asCockpitDmgStateList_005d1ee8[4];
unsigned short g_nHudMessageColour_005d1ef0;
char g_szSoundTraceScratch_005d1f00[0x200];
char *g_apszPersonnelImportCaptionsTable_005d2100[8];
short g_nShotHotspotLeft_005d2120;
short g_nSceneHotspotTopValue_005d2122;
short g_nSceneRegionRightWord_005d2124;
short g_nSceneAreaLowestWord_005d2126;
char g_nUnusedCockptSlotWord_005d2128; /* never referenced */
void *g_pStartupStellarPacket_005d212c;
char g_szLegacySaveFolder_005d2130[0x30];
Viewport g_stDashViewport_005d2160;
Viewport g_stLeftwardVduPortBlock_005d2180;
Viewport g_stScrViewInfo_005d21a0;
TextContext g_stFlightTextContext_005d21c0;
Viewport g_stCockpitMeterPort_005d21e0;
unsigned char *g_pCabinHudImage_005d21f4;
unsigned char *g_apTextGlyphs_005d2200[4];
FixedVector g_aDirectionLookRightVector_005d2210[DIRECTION_VIEW_COUNT];
FixedVector g_aDirectionViewUpVectorArray_005d2500[DIRECTION_VIEW_COUNT];
FixedVector g_aDirectionViewForwardAxisTable_005d27f0[DIRECTION_VIEW_COUNT];
short g_bHighMemoryBuffersSetState_005d2ad8;
TextContext g_stLeftVduStringContextRecord_005d2ae0;
Viewport g_stViewScratchInfo_005d2b00;
Viewport g_stRightVduView_005d2b20;
Viewport g_stPilotHandBackgroundViewportBlock_005d2b40;
unsigned char *g_pDashWeaponPicture_005d2b54;
