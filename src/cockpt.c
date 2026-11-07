/*
 *  Cockpit displays, HUD messages and VDUs.
 *
 *  Retail object 0x4380B0-0x43FABF (link order; see docs/ORDER.md).
 */
#define COCKPT_C
#include "game.h"

short g_bTargetLockActive = 0; /* 0x0049ae80 */
short g_nCommDeathSequenceFrame = 0; /* 0x0049ae84 */
short g_nCommMessageIndex = -1; /* 0x0049ae88 */
ShortPoint g_stWeaponDisplayOffset = { 0, 0x10 }; /* 0x0049ae8c */
ShortPoint g_stHudMessageOrigin = { 0 }; /* 0x0049ae90 */
ShortPoint g_aCockpitDamagePositions[4] = { 0 }; /* 0x0049ae98 */
short g_asCockpitLightX[7] = { 0 }; /* 0x0049aea8 */
short g_asCockpitLightY[7] = { 0 }; /* 0x0049aeb8 */
signed char g_acCockpitLightOffFrame[7] = { 0 }; /* 0x0049aec8 */
signed char g_acCockpitLightOnFrame[7] = { 0 }; /* 0x0049aed0 */
CockpitBarDefinition g_aCockpitBarDefinitions[8] = { 0 }; /* 0x0049aed8 */
ShortPoint g_aCockpitReadoutOrigins[4] = { 0 }; /* 0x0049af58 */
ShortRect g_stLeftVduBounds = { 0 }; /* 0x0049af68 */
ShortRect g_stRightVduBounds = { 0 }; /* 0x0049af70 */
CockpitScannerGeometry g_stCockpitScannerGeometry = { 0 }; /* 0x0049af78 */
ShortRect g_stPilotHandBounds = { 0 }; /* 0x0049af88 */
ShortPoint g_stPilotHandOrigin = { 0 }; /* 0x0049af90 */
signed char g_cHazardBaseTravelTime = 0; /* 0x0049af94 */
short g_nHazardPitchRange = 0; /* 0x0049af98 */
ShortPoint g_aDefaultWeaponDisplayPositions[16] = { 0 }; /* 0x0049afa0 */
short g_bTargetLockDisplayEnabled = 0; /* 0x0049afe0 */
short g_asVduSelectionSound[2] = { 0x7f, 0 }; /* 0x0049afe4 */
short g_nTargetLeadIndicatorX = (short)0x8001; /* 0x0049afe8 */
char *g_pszPendingHudMessage = 0; /* 0x0049afec */
char *g_pszDisplayedHudMessage = 0; /* 0x0049aff0 */
signed char g_cLastPilotHandFrame = (signed char)0xff; /* 0x0049aff4 */
short g_asPilotHandOffsets[34] = {
    6, -3, 7, 2, 7, 9, 7, 12, 8, 13, 0, -1, -1, -1,
    -4, -1, -6, -1, 6, 0, 8, 0, 10, 0, 13, 3, 8, -7,
    6, -9, 5, -11, 5, -14
}; /* 0x0049aff8 */
unsigned char *g_pPendingCockpitDamageShape = 0; /* 0x0049b03c */
unsigned char *g_pCockpitExplosionBackground = 0; /* 0x0049b040 */
unsigned char *g_pCockpitHudBackground = 0; /* 0x0049b044 */
unsigned char * volatile g_pCockpitExplosionShape = 0; /* 0x0049b048 */
volatile short g_nCockpitExplosionFrame = 8; /* 0x0049b04c */
char *g_pszAutopilotWaitReason = g_szAutopilotWaitReason; /* 0x0049b050 */
short g_bFuelGaugeDamaged = 0; /* 0x0049b054 */
short g_bForceCriticalDamageWarning = 0; /* 0x0049b058 */
unsigned char *g_pReleaseWeaponDisplayBackground = 0; /* 0x0049b05c */
signed char g_cReleaseWeaponDisplayFrame = -1; /* 0x0049b060 */
signed char g_cReleaseWeaponDisplayTicks = 0; /* 0x0049b064 */
signed char g_cReleaseWeaponDisplayState = 0; /* 0x0049b068 */
static char g_szNoWeaponSelected[8] = "None"; /* 0x0049b070 */
int g_nDisplayedObjectiveRange = 40000; /* 0x0049b078 */
short g_nScannerTargetObject = -1; /* 0x0049b07c */
int g_aiForwardScannerGridRows[78] = {
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
}; /* 0x0049b080 */
int g_aiTargetCameraScannerGridRows[50] = {
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
}; /* 0x0049b1b8 */
const char *g_pszMissileLocked = "MISSILE LOCKED "; /* 0x0049b280 */
signed char g_bTargetBracketVisible = 1; /* 0x0049b284 */
unsigned char *g_pHudMessageFrameShape = 0; /* 0x0049b288 */
unsigned char *g_pHudMessageBackground = 0; /* 0x0049b28c */
short g_bCaptureHudMessageBackground = 1; /* 0x0049b290 */
short g_nHudMessageBackgroundDepth = 0; /* 0x0049b294 */
short g_nTargetLockMarkerX = -0x7fff; /* 0x0049b298 */
ShortRect g_stTargetBracketBounds = { -0x7fff, 0, 0, 0 }; /* 0x0049b2a0 */
ShortRect g_stCommTargetBracketBounds = { -0x7fff, 0, 0, 0 }; /* 0x0049b2a8 */
short g_bMissileLockAcquired = 0; /* 0x0049b2b0 */
short g_bCockpitDamageFrame0Shown = 0; /* 0x0049b2b4 */
short g_bCockpitDamageFrame2Shown = 0; /* 0x0049b2b8 */
int g_nCommPortraitFrame = -1; /* 0x0049b2bc */
Viewport g_stTrainSimRightVduSource = { 0 }; /* 0x0049b2c0 */
Viewport g_stTrainSimLeftVduSource = { 0 }; /* 0x0049b2d8 */
int g_bForceDamageDisplayRedraw = 0; /* 0x0049b2ec */
char g_szAutopilotWaitReason[28] = "Wait for ??????????????????"; /* 0x0049b2f0 */


/* Clearing the communication resources sets the speaker to -1.  WC2's
 * unchecked side-table load then treats the last two bytes of
 * g_acShipLastAttacker as a signed word.  Reassemble that
 * layout-derived value without crossing a native global's sanitizer redzone. */
#ifdef SDL_PORT
#define COMM_SPEAKER_SIDE(obj) \
    ((obj) != -1 \
         ? g_asShipSide[obj] \
         : (short)((unsigned short)(unsigned char) \
                       g_acShipLastAttacker[14] | \
                   ((unsigned short)(unsigned char) \
                        g_acShipLastAttacker[15] << 8)))
#else
#define COMM_SPEAKER_SIDE(obj) g_asShipSide[obj]
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
    return g_nCockpitExplosionFrame < 8;
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

    if (g_nCurrentView == 0) {
        definition = g_aCockpitBarDefinitions[(int)bar];
        sprite = g_4_pCockpitPanelShape;
    } else {
        definition = ((CockpitBarDefinition (*)[2])
            g_abGunDisplayConfiguration)[
            g_8_nGunDisplaySlot][(int)bar];
        sprite = g_apCockpitVduOverlayShapes[
            g_8_nGunDisplaySlot];
    }
    length = definition.length;
    percent = (short)(((int)percent * (int)length) / 100);
    g_9_stCockpitMeterPort.left = definition.left;
    barLeft = g_9_stCockpitMeterPort.left;
    if (barLeft == -99)
        return;
    g_9_stCockpitMeterPort.right = definition.right;
    barRight = g_9_stCockpitMeterPort.right;
    g_9_stCockpitMeterPort.top = definition.top;
    barTop = g_9_stCockpitMeterPort.top;
    g_9_stCockpitMeterPort.bottom = definition.bottom;
    barBottom = g_9_stCockpitMeterPort.bottom;
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
        g_9_stCockpitMeterPort.bottom -= percent;
        if (g_9_stCockpitMeterPort.top <=
            g_9_stCockpitMeterPort.bottom)
            DrawSpriteDefault(&g_9_stCockpitMeterPort, barLeft, barTop,
                              sprite, frameNumberFilled);
        g_9_stCockpitMeterPort.top =
            (short)(g_9_stCockpitMeterPort.bottom + 1);
        g_9_stCockpitMeterPort.bottom = barBottom;
        if (g_9_stCockpitMeterPort.top <=
            g_9_stCockpitMeterPort.bottom)
            DrawSpriteDefault(&g_9_stCockpitMeterPort, barLeft, barTop,
                              sprite, frameNumberEmpty);
    } else {
        if (barDirection == 3) {
            percent = (short)(length - percent);
            tmp = frameNumberEmpty;
            frameNumberEmpty = frameNumberFilled;
            frameNumberFilled = (signed char)tmp;
        }
        g_9_stCockpitMeterPort.right -= percent;
        if (g_9_stCockpitMeterPort.left <=
            g_9_stCockpitMeterPort.right)
            DrawSpriteDefault(&g_9_stCockpitMeterPort, barLeft, barTop,
                              sprite, frameNumberFilled);
        g_9_stCockpitMeterPort.left =
            (short)(g_9_stCockpitMeterPort.right + 1);
        g_9_stCockpitMeterPort.right = barRight;
        if (g_9_stCockpitMeterPort.left <=
            g_9_stCockpitMeterPort.right)
            DrawSpriteDefault(&g_9_stCockpitMeterPort, barLeft, barTop,
                              sprite, frameNumberEmpty);
    }
}

/* Function start: 0x438454 */
unsigned int InitializeCockpitReadout(signed char slot,
                                      TextContext *context)
{
    g_9_aConsoleMetersTable[(int)slot].context = context;
    g_9_aConsoleMetersTable[(int)slot].x = context->cursorX;
    g_9_aConsoleMetersTable[(int)slot].y = context->cursorY;
    g_9_aConsoleMetersTable[(int)slot].previousRight = 0;
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x4384AD */
void DrawCockpitReadout(signed char slot, const char *text)
{
    if (g_9_aConsoleMetersTable[(int)slot].x == -99)
        return;
    SetTextContext(g_9_aConsoleMetersTable[(int)slot].context);
    SetTextCursor((unsigned short)g_9_aConsoleMetersTable[(int)slot].x,
                  (unsigned short)g_9_aConsoleMetersTable[(int)slot].y);
    DrawFormattedText(text);
    EraseCockpitReadoutRegion(
        &g_101_stScrViewInfo,
        g_9_aConsoleMetersTable[(int)slot].context->cursorX,
        g_9_aConsoleMetersTable[(int)slot].y,
        g_9_aConsoleMetersTable[(int)slot].previousRight,
        (short)(*(unsigned short *)
                    g_9_aConsoleMetersTable[(int)slot].context->font +
                g_9_aConsoleMetersTable[(int)slot].y - 1),
        g_cSecondaryViewBufferColour);
    g_9_aConsoleMetersTable[(int)slot].previousRight =
        g_9_aConsoleMetersTable[(int)slot].context->cursorX;
}

/* Function start: 0x4385B3 */
short DrawHudMessageSlot(HudMessageSlot *slot)
{
    short textColour;
    short previousDrawColour;
    short blinkOff;

    if (g_nCurrentView != 0)
        return 1;

    previousDrawColour = slot->drawColour;
    if (((int)g_196_nInputTimerTicks / 40) % 3 == 0)
        slot->drawColour = g_cSecondaryViewBufferColour;
    else
        slot->drawColour = slot->colour;

    if (slot->flashCount != -1) {
        if (slot->drawColour == g_cSecondaryViewBufferColour &&
            previousDrawColour == slot->colour) {
            slot->flashCount = (signed char)MaxShort(
                0, (short)(slot->flashCount - 1));
        }
        if (slot->flashCount == 0)
            slot->drawColour = g_cSecondaryViewBufferColour;
    }

    blinkOff = slot->drawColour == g_cSecondaryViewBufferColour;
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
        if (slot->text == 0 && g_nCurrentView == 0 &&
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
    g_12_aHudTextSlotsArray[0].text = 0;
    g_12_aHudTextSlotsArray[0].flashCount = 0;
    g_12_aHudTextSlotsArray[1].text = 0;
    g_12_aHudTextSlotsArray[1].flashCount = 0;
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
    SetHudMessageSlot(&g_12_aHudTextSlotsArray[1], &g_1_stRightVduTextContextBlock,
                      g_4_stRightVduView.left,
                      (short)(g_4_stRightVduView.bottom - 6),
                      text, colour, (signed char)flashCount);
}

/* Function start: 0x4388F5 */
void CockpitMessage(const char *text, unsigned short colour,
                    int flashCount)
{
    if (text != g_12_aHudTextSlotsArray[1].text)
        set_global_message(text, colour, flashCount);
}

/* Function start: 0x438923 */
/* Nothing in the shipped executable calls this.  Clears the global message
 * slot. */
void ClearGlobalMessage(void)
{
    ClearHudMessageSlot(&g_12_aHudTextSlotsArray[1]);
}

/* Function start: 0x43893F */
void remove_message(const char *text)
{
    ClearHudMessageIfMatching(&g_12_aHudTextSlotsArray[1], text);
}

/* Function start: 0x43895F */
short kilrathi_near(short obj, short range)
{
    short ship;

    for (ship = 1; ship < 10; ship++) {
        if (g_aeObjectClass[ship] >= OBJECT_CLASS_SHIP &&
            g_asShipSide[ship] == SIDE_KILRATHI &&
            IsPointWithinRange(&g_aShipPosition[obj],
                               &g_aShipPosition[ship],
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

    record = &g_aMissionObjectives[objectiveIndex];
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
    if (g_aMissionNavPoints[navPoint].systemIndex ==
        g_nCurStellarSystem)
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
    if (g_cMissionObjectiveCount == 0)
        return 0;
    if (g_asShipSide[0] == SIDE_KILRATHI)
        return 0;
    if (IsCockpitExplosionActive() != 0)
        return 0;
    if (IsMissionObjectiveOutOfSystem(
            (short)g_cCurrentObjective) != 0)
        reason = "Jump Point";
    range = distance_from_point(
        0, &g_aMissionObjectives[
               (signed char)g_cCurrentObjective].position);
    for (object = 1; object < 10; object++) {
        if (g_aeObjectClass[object] >= OBJECT_CLASS_CAPITAL_SHIP &&
            g_asShipMissionType[object] ==
                MISSION_TYPE_GOTO_WARP &&
            g_asShipMissionParameter[object] ==
                g_nCurrentNavPoint &&
            equ_vector(
                &g_aMissionObjectives[
                    (signed char)g_cCurrentObjective].position,
                &g_aShipDestination[object]) == 0) {
            sprintf(g_pszAutopilotWaitReason, "Wait for %s",
                    g_apShipMissionRecord[object]->name);
            reason = g_pszAutopilotWaitReason;
            break;
        }
    }
    if (range < 8000) {
        reason = "Already Near";
    } else if (kilrathi_near(0, 16000) != 0) {
        reason = "Enemy Near";
    } else if (g_pActiveHazardField != 0) {
        reason = "Hazard Near";
    }
    if (showReason != 0 && reason != 0)
        CockpitMessage(reason, g_cYellowColour, 3);
    return reason == 0;
}

/* Function start: 0x438CEB */
void reset_cockpit(void)
{
    short light;

    for (light = 0; light < 7; light++) {
        g_4_abCockpitLightMode[light] = 0;
        g_12_abCockpitLightGoalTable[light] = 0;
    }
}

/* Function start: 0x438D30 */
void SetCockpitLightBlink(signed char light, short interval)
{
    if (interval < 20) {
        if (interval == 0) {
            g_12_abCockpitLightGoalTable[(int)light] ^= 1;
        } else if (g_nSpaceFrame % interval == 0) {
            g_12_abCockpitLightGoalTable[(int)light] ^= 1;
        }
    } else {
        g_12_abCockpitLightGoalTable[(int)light] = 0;
    }
}

/* Function start: 0x438DAD */
void draw_cockpit_lights(void)
{
    signed char light;
    short x;
    short frame;
    short y;

    if (g_nRenderedSpaceFrame % 4 == 0) {
        if (auto_pilot_valid(0) != 0)
            g_12_abCockpitLightGoalTable[4] = 1;
        else
            g_12_abCockpitLightGoalTable[4] = 0;
    }
    for (light = 0; light < 7; light++) {
        if (g_nCockpitDisplayMode != 0) {
            x = g_asCockpitLightX[(int)light];
            y = g_asCockpitLightY[(int)light];
            if (g_12_abCockpitLightGoalTable[(int)light] == 1)
                frame = g_acCockpitLightOnFrame[(int)light];
            else
                frame = g_acCockpitLightOffFrame[(int)light];
            DrawSpriteDefault(&g_101_stScrViewInfo, x, y,
                              g_4_pCockpitPanelShape, frame);
            g_4_abCockpitLightMode[(int)light] =
                g_12_abCockpitLightGoalTable[(int)light];
        } else if (g_4_abCockpitLightMode[(int)light] !=
                   g_12_abCockpitLightGoalTable[(int)light]) {
            x = g_asCockpitLightX[(int)light];
            y = g_asCockpitLightY[(int)light];
            if (g_12_abCockpitLightGoalTable[(int)light] == 1)
                frame = g_acCockpitLightOnFrame[(int)light];
            else
                frame = g_acCockpitLightOffFrame[(int)light];
            DrawSpriteDefault(&g_101_stScrViewInfo, x, y,
                              g_4_pCockpitPanelShape, frame);
            g_4_abCockpitLightMode[(int)light] =
                g_12_abCockpitLightGoalTable[(int)light];
        }
    }
}

/* Function start: 0x438F62 */
void update_lights(void)
{
    short energy;
    short fuelPercent;

    if (g_bFuelGaugeDamaged == 0) {
        fuelPercent = (short)(
            (g_anShipFuel[0] * 100) /
            *(int *)&g_aObjectTypeData[0].lifetime);
        SetCockpitLightBlink(6, fuelPercent);
        vdu_polygon(0, fuelPercent);
    }
    energy = g_asShipWeaponEnergy[0];
    vdu_polygon(1, energy);
    if ((calculate_damage_level() >= 3 &&
         (int)g_aasShipShield[0][1] +
                 (int)g_aasShipShield[0][0] <
             10) ||
        g_bForceCriticalDamageWarning != 0) {
        SetCockpitLightBlink(3, 2);
        if (g_4_nSevereDmgWarningSfxHandle == 0 ||
            g_nSpaceFrame % 10 == 0)
            PlaySfxWaveFileByNumber(0x20, -1, 0);
    } else if (g_4_nSevereDmgWarningSfxHandle != 0) {
        FlushSoundEffectsAndLog(
            g_4_nSevereDmgWarningSfxHandle, 1);
        g_4_nSevereDmgWarningSfxHandle = 0;
        g_12_abCockpitLightGoalTable[3] = 0;
    }
}

/* Function start: 0x439076 */
void update_bars(void)
{
    ObjectTypeData *typeData;
    short ratio;
    short shieldPercent;
    short shieldShown;

    typeData = &g_aObjectTypeData[g_acObjectType[0]];
    ratio = (short)((g_aasShipArmor[0][0] * 100) /
                         typeData->armorFront);
    vdu_polygon(2, ratio);
    ratio = (short)((g_aasShipArmor[0][1] * 100) /
                         typeData->armorRear);
    vdu_polygon(3, ratio);
    ratio = (short)((g_aasShipArmor[0][2] * 100) /
                         typeData->armorRight);
    vdu_polygon(4, ratio);
    ratio = (short)((g_aasShipArmor[0][3] * 100) /
                         typeData->armorLeft);
    vdu_polygon(5, ratio);
    shieldPercent = (short)((g_aasShipShield[0][0] * 100) /
                            typeData->shieldFore);
    SetCockpitLightBlink(0, shieldPercent);
    vdu_polygon(6, shieldPercent);
    shieldShown = g_aasShipShield[0][0];
    if (g_asObjectType[0] == 0x33)
        shieldShown = (short)(shieldShown / 10);
    DrawCockpitReadout(
        4, itoa((int)shieldShown,
                 g_16_szTextBufferBuffer, 10));
    shieldPercent = (short)((g_aasShipShield[0][1] * 100) /
                            typeData->shieldAft);
    SetCockpitLightBlink(1, shieldPercent);
    vdu_polygon(7, shieldPercent);
    shieldShown = g_aasShipShield[0][1];
    if (g_asObjectType[0] == 0x33)
        shieldShown = (short)(shieldShown / 10);
    DrawCockpitReadout(
        5, itoa((int)shieldShown,
                 g_16_szTextBufferBuffer, 10));
}

/* Function start: 0x439264 */
short get_mode(short i)
{
#ifdef SDL_PORT
    /* The depth starts at -1 for VDU 0, so until something calls set_mode on
     * it the original indexes one slot in front of the row and reads the last
     * component of g_vPlayerAcceleration -- zero at mission init,
     * which is the only time this happens.  Say zero rather than read across.
     */
    if (g_acVduModeStackDepth[(int)i] < 0)
        return 0;
#endif
    return (short)g_aaiVduModeStack[(int)i][
        (int)g_acVduModeStackDepth[(int)i]];
}

/* Function start: 0x43928E */
void set_mode(short i, int state)
{
    if (get_mode(i) != state)
        ClearHudMessageSlot(&g_12_aHudTextSlotsArray[i]);
    g_acVduModeStackDepth[(int)i] = 0;
    g_aaiVduModeStack[(int)i][
        (int)g_acVduModeStackDepth[(int)i]] = state;
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
    return g_acVduModeStackDepth[(int)i];
}

/* Function start: 0x439369 */
void push_mode(short i, int state)
{
    ClearHudMessageSlot(&g_12_aHudTextSlotsArray[i]);
#ifdef SDL_PORT
    /* WC2 pushes without bounding the stack, so a fifth mode writes past the
     * four-deep row and into whatever the linker put next.  Drop the
     * overflowing push on the port instead. */
    if (g_acVduModeStackDepth[(int)i] >= 3)
        return;
#endif
    g_acVduModeStackDepth[(int)i]++;
    g_aaiVduModeStack[(int)i][
        (int)g_acVduModeStackDepth[(int)i]] = state;
}

/* Function start: 0x4393B3 */
void pop_mode(short i)
{
    ClearHudMessageSlot(&g_12_aHudTextSlotsArray[i]);
    g_acVduModeStackDepth[(int)i]--;
}

/* Function start: 0x4393E1 */
void set_new_vdu(short vdu)
{
    if (get_mode(vdu) == 0) {
        malf_noise(vdu, 1, g_cDarkGreenColour, 0x17, 0);
    } else {
        switch (vdu) {
        case 1:
            ClearViewport(
                &g_4_stRightVduView,
                g_cSecondaryViewBufferColour);
            break;
        case 0:
            ClearViewport(
                &g_2_stLeftwardVduPortBlock,
                g_cSecondaryViewBufferColour);
            break;
        }
    }
    g_anLastDrawnVduMode[(int)vdu] = get_mode(vdu);
}

/* Function start: 0x4394A0 */
short update_vid_disp(short vdu)
{
    short changed;

    changed =
        get_mode(vdu) != g_anLastDrawnVduMode[(int)vdu];
    if (changed != 0)
        set_new_vdu(vdu);
    return changed;
}

/* Function start: 0x439500 */
void InvalidateVduMode(short i)
{
    g_anLastDrawnVduMode[(int)i] = 0;
}

/* Function start: 0x43951A */
void clear_message_time(void)
{
    g_9_nScreenMessageTimeNumber = 0;
}

/* Function start: 0x43952E */
short message_showing(void)
{
    return g_9_nScreenMessageTimeNumber != 0;
}

/* Function start: 0x439559 */
void set_message_time(short duration)
{
    g_9_nScreenMessageTimeNumber = duration;
    if (g_9_nScreenMessageTimeNumber == -1)
        g_bSpeechPlaybackDone = 0;
}

/* Function start: 0x439588 */
void check_message(void)
{
    if (message_showing() != 0) {
        if (g_9_nScreenMessageTimeNumber == -1) {
            if (g_bSpeechPlaybackDone != 0) {
                clear_message_time();
                if (g_aeSpecialManeuver[
                        g_nCommSpeakerObject] ==
                        SPECIAL_MANEUVER_UNKNOWN_9 &&
                    g_pCommPortraitResource != 0) {
                    g_nCommDeathSequenceFrame = 1;
                } else {
                    EndCommMenu();
                }
                if (g_bEjectionSequencePending != 0) {
                    ejection_sequence(
                        g_aObjectTypeData[
                            g_acObjectType[0]].resourceType,
                        1);
                    g_anShipFuel[0] =
                        *(int *)&g_aObjectTypeData[
                            g_acObjectType[0]].lifetime;
                    g_bEjectionSequencePending = 0;
                }
            }
        } else {
            g_9_nScreenMessageTimeNumber--;
            if (g_9_nScreenMessageTimeNumber == 0) {
                EndCommMenu();
                if (g_bEjectionSequencePending != 0) {
                    ejection_sequence(
                        g_aObjectTypeData[
                            g_acObjectType[0]].resourceType,
                        1);
                    g_anShipFuel[0] =
                        *(int *)&g_aObjectTypeData[
                            g_acObjectType[0]].lifetime;
                    g_bEjectionSequencePending = 0;
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

    SetTextContext(&g_16_stCockpitTextSettingsRecord);
    speed = (short)(g_anShipSpeed[0] >> 8);
    speed = (short)(speed * 10);
    DrawCockpitReadout(
        2, itoa(speed, g_16_szTextBufferBuffer, 10));
    velocity = (short)(MultiplyFixed(
        Vector_magnitude(&g_aShipVelocity[0]), 0xa00) >> 8);
    DrawCockpitReadout(
        3, itoa(velocity, g_16_szTextBufferBuffer, 10));
}

/* Function start: 0x439753 */
void malf_sound(void)
{
    LogUnknownSoundEffect(
        g_abSoundEffectDefinitions + 0xf0,
        0, 0x7f, 0x64, 1, 1);
    PlaySfxWaveFileByNumber(0x1f, -1, 0);
}

/* Function start: 0x439789 */
short malf(char component)
{
    return g_acPlayerComponentDamage[(int)component] *
               g_acPlayerComponentDamage[(int)component] >
           (unsigned short)RandomInRange(0, 15);
}

/* Function start: 0x4397D5 */
void vdu_malf(short vdu, short sound)
{
    if (g_nCurrentView == 0 &&
        ((vdu == 0 && g_2_stLeftwardVduPortBlock.top > 0) ||
         (vdu == 1 && g_4_stRightVduView.top > 0))) {
        malf_noise(vdu, 1, g_cDarkGreenColour, sound, 0);
        set_mode(vdu, 0);
    }
}

/* Function start: 0x43984F */
void ShowComponentHitHudMessage(const char *text, unsigned short colour,
                                short flashCount)
{
    if (get_mode(0) != 0) {
        if (g_12_aHudTextSlotsArray[0].text != 0)
            ClearHudMessageSlot(&g_12_aHudTextSlotsArray[0]);
        DosStrcpy(g_16_szComponentHitMsg, text);
        SetHudMessageSlot(&g_12_aHudTextSlotsArray[0], &g_5_stLeftVduStringContextRecord,
                          g_2_stLeftwardVduPortBlock.left,
                          (short)(g_2_stLeftwardVduPortBlock.bottom - 6),
                          g_16_szComponentHitMsg,
                          colour, (signed char)flashCount);
    }
}

/* Function start: 0x4398CB */
signed char damage_your_component(char component, char amount,
                                  char maximum)
{
    char text[40];

    g_acPlayerComponentDamage[(int)component] =
        (signed char)MinShort(
        (short)(g_acPlayerComponentDamage[(int)component] + amount),
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
                g_apszComponentNames[(int)component]);
        ShowComponentHitHudMessage(text, g_cRedColour, 5);
    }
    return g_acPlayerComponentDamage[(int)component];
}

/* Function start: 0x4399C6 */
void RemovePlayerReleaseWeapon(signed char weapon)
{
    lock_off();
    g_3_nReleaseWeaponShowKindWord =
        ((ShipWeaponSlot *)&g_aShipWeapons[0][1])[weapon].type;
    g_cReleaseWeaponDisplayFrame = 1;
    g_3_stReleaseWeaponReadoutLocationBlock.x =
        (short)(g_116_aWeaponDisplayPositions[weapon].x +
                g_5_nWeaponDisplayOriginXValue);
    g_3_stReleaseWeaponReadoutLocationBlock.y =
        (short)(g_116_aWeaponDisplayPositions[weapon].y +
                g_nArmsDisplayOriginY);
    g_cReleaseWeaponDisplayTicks = 3;
    g_cReleaseWeaponDisplayState = 0;
    if (g_pReleaseWeaponDisplayBackground == 0)
        g_cReleaseWeaponDisplayFrame = -1;
    remove_weapon(0, weapon);
}

/* Function start: 0x439A5A */
void fire_computer_graphic_missile(void)
{
    short visible;

    if (g_cReleaseWeaponDisplayFrame != -1) {
        visible = g_nCurrentView == 0 && get_mode(0) == 1;
        if (g_cReleaseWeaponDisplayState != 0) {
            g_cReleaseWeaponDisplayState = 0;
            RestoreSpriteBackground(
                &g_2_stLeftwardVduPortBlock,
                g_pReleaseWeaponDisplayBackground,
                g_3_stReleaseWeaponReadoutLocationBlock.x,
                g_3_stReleaseWeaponReadoutLocationBlock.y,
                g_1_pDashWeaponPicture,
                g_cReleaseWeaponDisplayFrame);
        }
        if (g_2_stLeftwardVduPortBlock.top - 10 <
                g_3_stReleaseWeaponReadoutLocationBlock.y &&
            g_3_stReleaseWeaponReadoutLocationBlock.y <
                g_2_stLeftwardVduPortBlock.bottom) {
            g_3_stReleaseWeaponReadoutLocationBlock.y =
                (short)(g_3_stReleaseWeaponReadoutLocationBlock.y -
                        g_cReleaseWeaponDisplayTicks);
            g_cReleaseWeaponDisplayTicks++;
            if (visible != 0) {
                CaptureSpriteBackground(
                    &g_2_stLeftwardVduPortBlock,
                    g_pReleaseWeaponDisplayBackground,
                    g_3_stReleaseWeaponReadoutLocationBlock.x,
                    g_3_stReleaseWeaponReadoutLocationBlock.y,
                    g_1_pDashWeaponPicture,
                    g_cReleaseWeaponDisplayFrame);
                DrawSpriteDefault(
                    &g_2_stLeftwardVduPortBlock,
                    g_3_stReleaseWeaponReadoutLocationBlock.x,
                    g_3_stReleaseWeaponReadoutLocationBlock.y,
                    g_1_pDashWeaponPicture,
                    g_cReleaseWeaponDisplayFrame);
            }
            g_cReleaseWeaponDisplayState = (signed char)visible;
        } else {
            g_cReleaseWeaponDisplayFrame = -1;
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

    weapons = g_aShipWeapons[0];
#ifdef SDL_PORT
    /* With no selected release entry, retail forms a pointer ten bytes before
     * this array but does not dereference it in the name switch. */
    if (g_nSelectedReleaseWeaponIndex == -1) {
        entry = 0;
    } else {
#endif
    entry = (ShipWeaponSlot *)(
        weapons + g_nSelectedReleaseWeaponIndex *
                         sizeof(ShipWeaponSlot) +
        1);
#ifdef SDL_PORT
    }
#endif
    set_new_vdu(0);
    DrawTextAt(&g_5_stLeftVduStringContextRecord,
               g_2_stLeftwardVduPortBlock.left,
               g_2_stLeftwardVduPortBlock.top, "WEAPON DISPLAY", 2);
    DrawViewportLine(&g_2_stLeftwardVduPortBlock,
                     g_2_stLeftwardVduPortBlock.left + 2,
                     g_2_stLeftwardVduPortBlock.top + 5,
                     (short)(g_2_stLeftwardVduPortBlock.right - 2),
                     g_2_stLeftwardVduPortBlock.top + 5,
                     g_ucPrimaryTextColour);

    switch (g_nSelectedReleaseWeaponIndex) {
    case -1:
        releaseLabel = g_szNoWeaponSelected;
        break;
    default:
        releaseLabel = g_aObjectTypeData[
            (int)entry->weaponType].displayName;
    }
    switch (g_nSelectedGunType) {
    case -1:
        gunCaption = g_szNoWeaponSelected;
        break;
    case 0x80:
        gunCaption = "Full Guns";
        break;
    default:
        gunCaption = g_aObjectTypeData[
            g_nSelectedGunType].displayName;
    }
    DrawFormattedText("\nWeapon:%s", releaseLabel);
    DrawFormattedText("\nGun:%s", gunCaption);

    g_5_nWeaponDisplayOriginXValue =
        (short)(g_2_stLeftwardVduPortBlock.left +
                g_stWeaponDisplayOffset.x);
    g_nArmsDisplayOriginY =
        (short)(g_stWeaponDisplayOffset.y +
                g_2_stLeftwardVduPortBlock.top);
    DrawSpriteDefault(
        &g_2_stLeftwardVduPortBlock,
        g_5_nWeaponDisplayOriginXValue,
        g_nArmsDisplayOriginY,
        g_1_pDashWeaponPicture, 0);

    entry = (ShipWeaponSlot *)(weapons + 1);
    for (slotIndex = 0; slotIndex < (signed char)weapons[0];
         slotIndex++, entry++) {
        if (g_aObjectTypeData[
                (int)entry->weaponType].field_18 == 0x0b) {
            frame = 0x1a;
            if (g_116_aWeaponDisplayPositions[
                    (int)slotIndex].x > 0x29)
                frame = (short)(frame + 2);
            if (g_116_aWeaponDisplayPositions[
                    (int)slotIndex].x < 0x1f)
                frame++;
        } else if (g_aObjectTypeData[
                       (int)entry->weaponType].objectClass == 8) {
            frame = (short)((entry->type - 7) * 2 + entry->disabled + 3);
        } else {
            frame = (short)(
#ifdef SDL_PORT
                ((g_nSelectedReleaseWeaponIndex == -1
                      ? 0
                      : ((ShipWeaponSlot *)&g_aShipWeapons[0][1])[
                            g_nSelectedReleaseWeaponIndex].type) !=
                 entry->type) + 1);
#else
                (((ShipWeaponSlot *)&g_aShipWeapons[0][1])[
                     g_nSelectedReleaseWeaponIndex].type !=
                 entry->type) + 1);
#endif
        }

        x = g_116_aWeaponDisplayPositions[(int)slotIndex].x;
        y = g_116_aWeaponDisplayPositions[(int)slotIndex].y;
        if (g_aObjectTypeData[
                (int)entry->weaponType].field_18 != 0x14) {
            DrawSpriteDefault(
                &g_2_stLeftwardVduPortBlock,
                (short)(g_5_nWeaponDisplayOriginXValue + x),
                (short)(g_nArmsDisplayOriginY + y),
                g_1_pDashWeaponPicture, frame);
        }
        if ((int)slotIndex == g_nSelectedReleaseWeaponIndex &&
            g_aObjectTypeData[
                (int)entry->weaponType].field_18 == 0x14) {
            DrawSpriteDefault(
                &g_2_stLeftwardVduPortBlock,
                (short)(g_5_nWeaponDisplayOriginXValue + 10),
                (short)(g_nArmsDisplayOriginY + 13),
                g_1_pDashWeaponPicture,
                (short)(entry->type + 4));
            DrawSpriteDefault(
                &g_2_stLeftwardVduPortBlock,
                (short)(g_5_nWeaponDisplayOriginXValue + 60),
                (short)(g_nArmsDisplayOriginY + 13),
                g_1_pDashWeaponPicture,
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
         i < (signed char)g_aShipWeapons[0][0];
         i++) {
        if (((ShipWeaponSlot *)&g_aShipWeapons[0][1])[
                i].type == 11) {
            g_5_nWeaponDisplayOriginXValue =
                (short)(g_2_stLeftwardVduPortBlock.left +
                        g_stWeaponDisplayOffset.x);
            g_nArmsDisplayOriginY =
                (short)(g_stWeaponDisplayOffset.y +
                        g_2_stLeftwardVduPortBlock.top);
            x = g_116_aWeaponDisplayPositions[i].x;
            y = g_116_aWeaponDisplayPositions[i].y;

            if (g_12_aHudTextSlotsArray[0].text == 0 &&
                x < 0x29 && x > 0x1f) {
                if (g_27_aasGunCooldown[0][0] > 0) {
                    frame = 0x1d;
                    DrawSpriteDefault(
                        &g_2_stLeftwardVduPortBlock,
                        (short)(g_5_nWeaponDisplayOriginXValue + x),
                        (short)(g_nArmsDisplayOriginY + y),
                        g_1_pDashWeaponPicture, frame);
                } else {
                    frame = 0x1a;
                    DrawSpriteDefault(
                        &g_2_stLeftwardVduPortBlock,
                        (short)(g_5_nWeaponDisplayOriginXValue + x),
                        (short)(g_nArmsDisplayOriginY + y),
                        g_1_pDashWeaponPicture, frame);
                }
            }

            if (g_cReleaseWeaponDisplayFrame == -1 && x < 0x1f) {
                if (g_27_aasGunCooldown[0][1] > 0) {
                    frame = 0x1e;
                    DrawSpriteDefault(
                        &g_2_stLeftwardVduPortBlock,
                        (short)(g_5_nWeaponDisplayOriginXValue + x),
                        (short)(g_nArmsDisplayOriginY + y),
                        g_1_pDashWeaponPicture, frame);
                } else {
                    frame = 0x1b;
                    DrawSpriteDefault(
                        &g_2_stLeftwardVduPortBlock,
                        (short)(g_5_nWeaponDisplayOriginXValue + x),
                        (short)(g_nArmsDisplayOriginY + y),
                        g_1_pDashWeaponPicture, frame);
                }
            }

            if (g_cReleaseWeaponDisplayFrame == -1 && x > 0x29) {
                if (g_27_aasGunCooldown[0][2] > 0) {
                    frame = 0x1f;
                    DrawSpriteDefault(
                        &g_2_stLeftwardVduPortBlock,
                        (short)(g_5_nWeaponDisplayOriginXValue + x),
                        (short)(g_nArmsDisplayOriginY + y),
                        g_1_pDashWeaponPicture, frame);
                } else {
                    frame = 0x1c;
                    DrawSpriteDefault(
                        &g_2_stLeftwardVduPortBlock,
                        (short)(g_5_nWeaponDisplayOriginXValue + x),
                        (short)(g_nArmsDisplayOriginY + y),
                        g_1_pDashWeaponPicture, frame);
                }
            }
        }
    }
}

/* Function start: 0x43A1EB */
short sighted(short objective)
{
    return (g_aMissionObjectives[objective].flags & 4) == 4;
}

/* Function start: 0x43A21E */
short visited(short objective)
{
    return (g_aMissionObjectives[objective].flags & 1) == 1;
}

/* Function start: 0x43A251 */
short achieved(short objective)
{
    return (g_aMissionObjectives[objective].flags & 2) == 2 ||
           g_aMissionObjectives[objective].type == -1;
}

/* Function start: 0x43A29E */
void flag_objective(short objective, unsigned char flags)
{
    g_aMissionObjectives[objective].flags |= flags;
}

#pragma function(strcpy, strlen)
/* Function start: 0x43A2CD */
void DrawCalculatingLabel(void)
{
    if (IsMissionObjectiveOutOfSystem(
            (short)g_cCurrentObjective) != 0) {
        DrawCockpitReadout(0, "Out-system");
    } else {
        if (g_nCurrentObjectiveRange <= 0) {
            DrawCockpitReadout(0, "CALCULATING");
        } else {
            strcpy(g_16_szTextBufferBuffer +
                       strlen(ltoa((long)g_nCurrentObjectiveRange,
                                    g_16_szTextBufferBuffer, 10)),
                   " km");
            DrawCockpitReadout(0, g_16_szTextBufferBuffer);
        }
        g_nDisplayedObjectiveRange =
            g_nCurrentObjectiveRange;
    }
}

/* Function start: 0x43A374 */
const char *objective_name(short objective)
{
    if (objective >= (short)g_cMissionObjectiveCount)
        return "NONE";
    else if (*g_aMissionObjectives[objective].name != '?' ||
             sighted(objective) != 0)
        return g_aMissionObjectives[objective].displayName;
    else
        return "UNKNOWN";
}

/* Function start: 0x43A3F8 */
void show_navigation_disp(void)
{
    DrawTextAt(&g_1_stRightVduTextContextBlock, g_4_stRightVduView.left, g_4_stRightVduView.top,
               "COMP NAVIGATION", 2);
    DrawFormattedText("\n\nDESTINATION\n %s",
                      objective_name((short)g_cCurrentObjective));
    DrawFormattedText("\n\nRANGE\n ");
    InitializeCockpitReadout(0, &g_1_stRightVduTextContextBlock);
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
    if (g_aMissionObjectives[objective].type == 0 ||
        g_aMissionObjectives[objective].type == 5) {
        if (g_aMissionNavPoints[
                g_aMissionObjectives[objective].index].type != 0)
            concealed = 0;
        else
            return 1;
    }
    if (mobile_objective(objective) != 0 &&
        is_team_member(g_aMissionObjectives[objective].index) == 0) {
        concealed = 1;
        for (navPoint = 0; navPoint < 10; navPoint++) {
            if (g_aMissionNavPoints[navPoint].type == 0)
                continue;
            for (record = 0; record < 10; record++) {
                if (g_aMissionNavPoints[navPoint].missionShips[
                        record] ==
                    g_aMissionObjectives[objective].index)
                    break;
            }
            if (record < 10) {
                concealed = 0;
                break;
            }
        }
    }
    if (concealed != 0 ||
        *g_aMissionObjectives[objective].displayName == '.')
        concealed = 1;
    else
        concealed = 0;
    if (concealed != 0 ||
        *g_aMissionObjectives[objective].name == '.')
        concealed = 1;
    else
        concealed = 0;
    if (concealed != 0 ||
        (mobile_objective(objective) != 0 &&
         g_aMissionShips[
             g_aMissionObjectives[objective].index].state != 0))
        concealed = 1;
    else
        concealed = 0;
    if (concealed == 0 && mobile_objective(objective) != 0 &&
        g_aMissionObjectives[
            g_cMissionObjectiveCount].type == 0) {
        ship = find_ship_index(
            g_aMissionObjectives[objective].index);
        if (g_aMissionShips[
                g_aMissionObjectives[objective].index].missionType ==
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
        while (g_abFlightPath[++pathIndex] != -1 &&
               pathIndex < (short)g_cMissionObjectiveCount)
            ;
        pathIndex--;
    }
    if (pathIndex >= (short)g_cMissionObjectiveCount)
        pathIndex = 0;
    if (g_abFlightPath[pathIndex] == -1)
        pathIndex = 0;
    g_cCurrentNavPointIndex = (signed char)pathIndex;
    if (hidden_objective(
            (short)g_abFlightPath[pathIndex]) != 0)
        return 0;
    g_cCurrentObjective =
        g_abFlightPath[g_cCurrentNavPointIndex];
    g_aeShipObjective[0] =
        (enum ShipObjective)g_aMissionObjectives[
            (short)g_cCurrentObjective].type;
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
               (short)((short)g_cCurrentNavPointIndex + 1)) == 0) {
        if (g_cCurrentNavPointIndex == 0)
            wraps++;
    }
    if (wraps >= 3) {
        g_cCurrentNavPointIndex = 0;
        g_cCurrentObjective = g_abFlightPath[
            g_cCurrentNavPointIndex];
    }
    return wraps < 3;
}

/* Function start: 0x43A8EA */
void set_next_destination(void)
{
    set_new_objective(0);
    while (set_new_objective(
               (short)g_cCurrentNavPointIndex) == 0 ||
           visited((short)g_abFlightPath[
               g_cCurrentNavPointIndex]) != 0) {
        g_cCurrentNavPointIndex++;
        if (g_cCurrentNavPointIndex >=
                g_cMissionObjectiveCount ||
            g_abFlightPath[
                g_cCurrentNavPointIndex] == -1)
            break;
    }
    if (g_cCurrentNavPointIndex >=
            g_cMissionObjectiveCount ||
        g_abFlightPath[
            g_cCurrentNavPointIndex] == -1) {
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
            g_aMissionObjectives[objective].index,
            &g_aMissionObjectives[objective].position);
    }
    return ship;
}

/* Function start: 0x43AA3F */
short is_any_ship_returning_to_current_nav_point(void)
{
    short ship;

    for (ship = 0; ship < 10; ship++) {
        if (g_aeObjectClass[ship] >= OBJECT_CLASS_SHIP &&
            g_asShipMissionType[ship] == MISSION_TYPE_COME_HOME &&
            g_abShipNavPointIndex[ship] <= g_cCurrentNavPointIndex)
            return 1;
    }
    return 0;
}

/* Function start: 0x43AAC3 */
short escorting_a_ship(void)
{
    return g_asShipMissionType[0] == MISSION_TYPE_ESCORT ||
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

    type = g_aMissionObjectives[objective].type;
    carrierShip = g_asShipMissionParameter[0];
    carrierObject = find_ship_index(g_asShipMissionParameter[0]);
    if (g_cCurrentObjective != objective)
        visit = 1;
    else
        visit = 0;
    moveOn = 0;
    if (g_cCurrentObjective == objective) {
        if (reached != 0 || escorting_a_ship() == 0 ||
            carrierObject == -1 ||
            g_aMissionObjectives[objective].index ==
                g_asShipMissionParameter[0]) {
            if (visited(objective) != 0) {
                CockpitMessage("Already Visited",
                               g_cYellowColour, 4);
            } else {
                CockpitMessage("Objective Reached",
                               g_cYellowColour, 4);
            }
            moveOn = visit = 1;
        } else if (type != 1 ||
                   g_aMissionShips[carrierShip].state != 1) {
            sprintf(g_pszAutopilotWaitReason,
                    "Wait for %s",
                    g_apShipMissionRecord[carrierObject]->name);
            CockpitMessage(g_pszAutopilotWaitReason,
                           g_cYellowColour, 4);
        }
    }
    if (type != 1 && visit != 0) {
        if (visited(objective) == 0 && carrierObject != -1 &&
            g_aMissionObjectives[objective].index ==
                g_asShipMissionParameter[0] &&
            g_asShipSide[carrierObject] ==
                g_asShipSide[0] &&
            g_asShipMissionIndex[carrierObject] !=
                g_6_nHomeOperationShipIndexValue &&
            g_13_abShipArrivalMessageSent[carrierObject] == 0) {
            send_message(carrierObject, 0x16);
            g_13_abShipArrivalMessageSent[carrierObject]++;
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
         g_asObjectScreenX[object] != (short)0x8001))
        flag_objective(objective, 4);
}

/* Function start: 0x43ADC4 */
void check_visit(short objective, short range)
{
    short type;
    int reachedRange;

    type = g_aMissionObjectives[objective].type;
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

    if (g_aMissionObjectives[objective].type == 6 ||
        g_aMissionObjectives[objective].type == 5)
        return;
    object = LocateMobileObjective(objective);
    savedSystem = g_1_stElapsedCampaignDate.year;
    g_1_stElapsedCampaignDate.year = g_nCurStellarSystem;
    if (IsObjectiveInDisplayedSystem(objective) == 0) {
        g_1_stElapsedCampaignDate.year = savedSystem;
        return;
    }
    if (sighted(objective) == 0 || visited(objective) == 0 ||
        g_cCurrentObjective == objective) {
        ComputeVectorDelta(
            &g_aShipPosition[0],
            &g_aMissionObjectives[objective].position, &delta);
        range = FixedToShortSaturating(Vector_magnitude(&delta));
        check_sighting(objective, range, object);
        if (mobile_objective(objective) != 0) {
            if (object != -1)
                check_visit(objective, range);
        } else if (g_aMissionNavPoints[
                       g_aMissionObjectives[objective].index]
                       .type >= 1) {
            check_visit(objective, range);
        }
    }
    g_1_stElapsedCampaignDate.year = savedSystem;
}

/* Function start: 0x43AFD3 */
short objective_lost(short objective)
{
    short state;

    state = g_aMissionShips[
        g_aMissionObjectives[objective].index].state;
    switch (g_aMissionObjectives[objective].type) {
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
    if (objective_lost((short)g_cCurrentObjective) != 0) {
        cycle_next_objective();
        InvalidateVduMode(1);
    } else {
        update_objective_location((short)g_cCurrentObjective);
    }
    if (g_nDisplayedObjectiveRange !=
        g_nCurrentObjectiveRange)
        DrawCalculatingLabel();
}

#pragma function(abs)
/* Function start: 0x43B0EB */
void rotational_pos_to_scanner_pos(signed char object,
                                   const SphericalVector *position,
                                   CockpitScannerGeometry geometry)
{
    if (abs((int)position->yaw) < 45)
        g_8_nScopePointerX =
            (short)(geometry.centerX + position->yaw / 4);
    else
        g_8_nScopePointerX =
            (short)(geometry.centerX + position->yaw / 6);

    g_9_nScannerCursorYValue =
        (short)(geometry.centerY + -(position->pitch / 3));
    g_8_nScopePointerX =
        MinShort(geometry.maximumX, g_8_nScopePointerX);
    g_8_nScopePointerX =
        MaxShort(geometry.minimumX, g_8_nScopePointerX);
    g_9_nScannerCursorYValue =
        MinShort(geometry.maximumY, g_9_nScannerCursorYValue);
    g_9_nScannerCursorYValue =
        MaxShort(geometry.minimumY, g_9_nScannerCursorYValue);
    g_15_asScannerObjectX[(int)object] =
        g_8_nScopePointerX;
    g_10_asScannerObjectYTable[(int)object] =
        g_9_nScannerCursorYValue;
}

/* Function start: 0x43B1F0 */
short mobile_objective(short objective)
{
    short type;

    type = g_aMissionObjectives[objective].type;
    return type == 1 || type == 3 || type == 4 || type == 2;
}

/* Function start: 0x43B258 */
void ResetScannerContacts(void)
{
    short i;

    for (i = 10; i > 0; i--)
        g_15_asScannerObjectX[i] = 0;
    g_nScannerTargetObject = -1;
}

/* Function start: 0x43B29D */
void clear_head_up_display(void)
{
    short object;

    if (g_nCurrentView == 4)
        RestoreTargetCameraAttitudeIndicators();
    if (g_nScannerTargetObject != -1) {
        DrawViewportPixel(&g_101_stScrViewInfo,
                          g_15_asScannerObjectX[g_nScannerTargetObject],
                          g_10_asScannerObjectYTable[g_nScannerTargetObject],
                          g_14_asScannerTargetUnderlay[0]);
        DrawViewportPixel(&g_101_stScrViewInfo,
                          (short)(g_15_asScannerObjectX[g_nScannerTargetObject] + 1),
                          g_10_asScannerObjectYTable[g_nScannerTargetObject],
                          g_14_asScannerTargetUnderlay[1]);
        DrawViewportPixel(&g_101_stScrViewInfo,
                          (short)(g_15_asScannerObjectX[g_nScannerTargetObject] - 1),
                          g_10_asScannerObjectYTable[g_nScannerTargetObject],
                          g_14_asScannerTargetUnderlay[2]);
        DrawViewportPixel(&g_101_stScrViewInfo,
                          g_15_asScannerObjectX[g_nScannerTargetObject],
                          (short)(g_10_asScannerObjectYTable[g_nScannerTargetObject] + 1),
                          g_14_asScannerTargetUnderlay[3]);
        DrawViewportPixel(&g_101_stScrViewInfo,
                          g_15_asScannerObjectX[g_nScannerTargetObject],
                          (short)(g_10_asScannerObjectYTable[g_nScannerTargetObject] - 1),
                          g_14_asScannerTargetUnderlay[4]);
        g_15_asScannerObjectX[g_nScannerTargetObject] = 0;
    }
    if (g_15_asScannerObjectX[10] != 0 &&
        g_nCurrentView != 4 &&
        IsMissionObjectiveOutOfSystem(
            (short)g_cCurrentObjective) == 0) {
        RestoreSpriteBackground(&g_101_stScrViewInfo,
                                g_4_pScannerBlipUnderlayPointer,
                                g_15_asScannerObjectX[10],
                                g_10_asScannerObjectYTable[10],
                                g_66_pCabinHudImage, 2);
        g_15_asScannerObjectX[10] = 0;
    }
    for (object = 9; object > 0; object--) {
        if (g_15_asScannerObjectX[object] != 0) {
            DrawViewportPixel(&g_101_stScrViewInfo,
                              g_15_asScannerObjectX[object],
                              g_10_asScannerObjectYTable[object],
                              g_47_asScannerBackgroundColourList[object]);
            g_15_asScannerObjectX[object] = 0;
        }
    }
    g_nScannerTargetObject = -1;
}

/* Function start: 0x43B4CF */
void set_objective_range(short showOnScanner)
{
    FixedVector delta;
    FixedVector rotated;
    SphericalVector polar;

    LocateMobileObjective((short)g_cCurrentObjective);
    ComputeVectorDelta(&g_aShipPosition[0],
                       &g_aMissionObjectives[
                           (short)g_cCurrentObjective].position,
                       &delta);
    transform_to_objects_frame(&delta, &rotated, 0);
    rectangular_to_spherical(&rotated, &polar);
    if (showOnScanner != 0)
        rotational_pos_to_scanner_pos(
            10, &polar, g_stCockpitScannerGeometry);
    g_nCurrentObjectiveRange = polar.radius >> 8;
}

/* Function start: 0x43B570 */
void AdjustScannerContactColourForRange(
    int range, short *colour)
{
    short increment;

    increment = 2;
    if (g_bRewritePacketExtensions == 1)
        return;
    if (*colour == g_bPrimaryViewBufferColour)
        *colour = 0xa0;
    if (*colour == g_cYellowColour)
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

    objectClass = g_aeObjectClass[object];
    if (objectClass != OBJECT_CLASS_NULL &&
        (g_asObjectType[object] == 0x3d ||
         g_asObjectType[object] == 0x2c)) {
        *colour = g_cYellowColour;
        return 1;
    }
    if (objectClass < OBJECT_CLASS_MISSILE)
        return 0;
    else if (objectClass == OBJECT_CLASS_SHIP) {
        if (g_asShipSide[object] == SIDE_KILRATHI)
            *colour = g_cRedColour;
        else if (g_asShipSide[object] == SIDE_IMPERIAL)
            *colour = g_cBlueColour;
        else
            *colour = g_ucPrimaryTextColour;
    } else if (objectClass >= OBJECT_CLASS_CAPITAL_SHIP) {
        if (g_asShipSide[object] == SIDE_KILRATHI)
            *colour = g_ucHostileCapitalScannerColour;
        else if (g_asShipMissionIndex[object] ==
                 g_6_nHomeOperationShipIndexValue)
            *colour = g_bPrimaryViewBufferColour;
        else
            *colour = g_ucHomeCarrierScannerColour;
    } else {
        if (g_acShipTarget[object] == 0)
            *colour = g_cYellowColour;
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

    if (g_nCurrentView == 0) {
        geometry = g_stCockpitScannerGeometry;
        gridPattern = g_aiForwardScannerGridRows;
        baseColour = 0x5b;
    } else if (g_nCurrentView == 4) {
        geometry = ((CockpitScannerGeometry *)(
            g_abGunDisplayConfiguration + 0x40))[
                g_8_nGunDisplaySlot];
        gridPattern = g_aiTargetCameraScannerGridRows;
        baseColour = 0x25;
    }

    if (g_nCockpitDisplayMode != 0) {
        lineBits = 0;
        for (gridIndex = 0; gridPattern[gridIndex] != -2; gridIndex++) {
            if (gridPattern[gridIndex] == -1) {
                lineBits++;
            } else {
                cell = gridPattern[gridIndex];
                DrawViewportPixel(
                    &g_101_stScrViewInfo,
                    (short)(geometry.centerX + lineBits),
                    (short)(geometry.centerY + cell),
                    (short)baseColour);
                if (cell != 0)
                    DrawViewportPixel(
                        &g_101_stScrViewInfo,
                        (short)(geometry.centerX + lineBits),
                        (short)(geometry.centerY - cell),
                        (short)baseColour);
                if (lineBits != 0) {
                    DrawViewportPixel(
                        &g_101_stScrViewInfo,
                        (short)(geometry.centerX - lineBits),
                        (short)(geometry.centerY + cell),
                        (short)baseColour);
                    if (cell != 0)
                        DrawViewportPixel(
                            &g_101_stScrViewInfo,
                            (short)(geometry.centerX - lineBits),
                            (short)(geometry.centerY - cell),
                            (short)baseColour);
                }
            }
        }
    }

    clear_head_up_display();
    g_nScannerTargetObject = g_acShipTarget[0];
    if ((g_nScannerTargetObject != -1 &&
         g_aeObjectClass[g_nScannerTargetObject] <
             OBJECT_CLASS_SHIP) ||
#ifdef SDL_PORT
        /* With no target the first test is false and the original reads the
         * cloak state at index -1, four bytes in front of the array.  The
         * outcome is the same either way: the target is already -1. */
        (g_nScannerTargetObject >= 0 &&
         g_anShipCloakState[
             g_nScannerTargetObject] == 1))
#else
        g_anShipCloakState[
            g_nScannerTargetObject] == 1)
#endif
        g_nScannerTargetObject = -1;

    for (obj = 1; obj <= 9; obj++) {
        if (g_anShipCloakState[obj] != 1 &&
            get_color(obj, (unsigned short *)&markerColour) != 0) {
            ComputeVectorDelta(
                &g_aShipPosition[EYE_OBJECT],
                &g_aShipPosition[obj], &difference);
            transform_to_objects_frame(
                &difference, &inShipFrame, EYE_OBJECT);
            rectangular_to_spherical(&inShipFrame, &angles);
            if (angles.radius < 0xea6000) {
                AdjustScannerContactColourForRange(
                    angles.radius, (short *)&markerColour);
                rotational_pos_to_scanner_pos(
                    (signed char)obj, &angles, geometry);
                if (g_nScannerTargetObject != obj) {
                    g_47_asScannerBackgroundColourList[obj] =
                        (short)GetViewportPixel(
                            &g_101_stScrViewInfo,
                            g_8_nScopePointerX,
                            g_9_nScannerCursorYValue);
                    DrawViewportPixel(
                        &g_101_stScrViewInfo,
                        g_8_nScopePointerX,
                        g_9_nScannerCursorYValue,
                        (short)markerColour);
                }
            }
        }
    }

    if (get_mode(1) == 5 &&
        g_nCurrentView != 4 &&
        IsMissionObjectiveOutOfSystem(
            (short)g_cCurrentObjective) == 0) {
        set_objective_range(1);
        CaptureSpriteBackground(
            &g_101_stScrViewInfo,
            g_4_pScannerBlipUnderlayPointer,
            g_8_nScopePointerX,
            g_9_nScannerCursorYValue,
            g_66_pCabinHudImage, 2);
        DrawSpriteDefault(
            &g_101_stScrViewInfo,
            g_8_nScopePointerX,
            g_9_nScannerCursorYValue,
            g_66_pCabinHudImage, 2);
    }

    if (g_nScannerTargetObject != -1 &&
        g_anShipCloakState[
            g_nScannerTargetObject] != 1 &&
        get_color(g_nScannerTargetObject,
                  (unsigned short *)&markerColour) != 0) {
        ComputeVectorDelta(
            &g_aShipPosition[EYE_OBJECT],
            &g_aShipPosition[obj], &difference);
        transform_to_objects_frame(
            &difference, &inShipFrame, EYE_OBJECT);
        rectangular_to_spherical(&inShipFrame, &angles);
        if (angles.radius < 0xea6000)
            AdjustScannerContactColourForRange(
                angles.radius, (short *)&markerColour);

        g_14_asScannerTargetUnderlay[0] =
            (short)GetViewportPixel(
                &g_101_stScrViewInfo,
                g_15_asScannerObjectX[g_nScannerTargetObject],
                g_10_asScannerObjectYTable[g_nScannerTargetObject]);
        g_14_asScannerTargetUnderlay[1] =
            (short)GetViewportPixel(
                &g_101_stScrViewInfo,
                (short)(g_15_asScannerObjectX[g_nScannerTargetObject] + 1),
                g_10_asScannerObjectYTable[g_nScannerTargetObject]);
        g_14_asScannerTargetUnderlay[2] =
            (short)GetViewportPixel(
                &g_101_stScrViewInfo,
                (short)(g_15_asScannerObjectX[g_nScannerTargetObject] - 1),
                g_10_asScannerObjectYTable[g_nScannerTargetObject]);
        g_14_asScannerTargetUnderlay[3] =
            (short)GetViewportPixel(
                &g_101_stScrViewInfo,
                g_15_asScannerObjectX[g_nScannerTargetObject],
                (short)(g_10_asScannerObjectYTable[g_nScannerTargetObject] + 1));
        g_14_asScannerTargetUnderlay[4] =
            (short)GetViewportPixel(
                &g_101_stScrViewInfo,
                g_15_asScannerObjectX[g_nScannerTargetObject],
                (short)(g_10_asScannerObjectYTable[g_nScannerTargetObject] - 1));
        DrawViewportPixel(
            &g_101_stScrViewInfo,
            g_15_asScannerObjectX[g_nScannerTargetObject],
            g_10_asScannerObjectYTable[g_nScannerTargetObject], (short)markerColour);
        DrawViewportPixel(
            &g_101_stScrViewInfo,
            (short)(g_15_asScannerObjectX[g_nScannerTargetObject] + 1),
            g_10_asScannerObjectYTable[g_nScannerTargetObject], (short)markerColour);
        DrawViewportPixel(
            &g_101_stScrViewInfo,
            (short)(g_15_asScannerObjectX[g_nScannerTargetObject] - 1),
            g_10_asScannerObjectYTable[g_nScannerTargetObject], (short)markerColour);
        DrawViewportPixel(
            &g_101_stScrViewInfo,
            g_15_asScannerObjectX[g_nScannerTargetObject],
            (short)(g_10_asScannerObjectYTable[g_nScannerTargetObject] + 1),
            (short)markerColour);
        DrawViewportPixel(
            &g_101_stScrViewInfo,
            g_15_asScannerObjectX[g_nScannerTargetObject],
            (short)(g_10_asScannerObjectYTable[g_nScannerTargetObject] - 1),
            (short)markerColour);
    }
    if (g_nCurrentView == 4)
        DrawTargetCameraAttitudeIndicators(
            g_bPrimaryViewBufferColour);
}

/* Function start: 0x43BE1B */
void start_lock(unsigned short v)
{
    g_nTargetLockMusicCooldown = -100;
    g_nTargetLockCountdown = v;
    g_bTargetLockReadoutDirty = 0;
    g_nTargetLockMarkerAngle =
        RandomBelowOrEqual(0x167);
}

/* Function start: 0x43BE55 */
short starting_lock(unsigned short v)
{
    if (g_nTargetLockCountdown == -1) {
        start_lock(v);
        return 1;
    }
    return 0;
}

/* Function start: 0x43BE8D */
void lock_off(void)
{
    if (g_nTargetLockCountdown > -1) {
        g_bTargetLockReadoutDirty = 1;
#ifdef SDL_PORT
        /* -1 is the no-weapon-selected sentinel, and the original indexes the
         * slot array with it anyway -- nine bytes before the array, into the
         * ion-drive damage table, where nothing is going to read as a
         * torpedo. */
        if (g_nSelectedReleaseWeaponIndex >= 0 &&
#else
        if (
#endif
            ((ShipWeaponSlot *)&g_aShipWeapons[0][1])[
                g_nSelectedReleaseWeaponIndex].type == 0x13) {
            StopMusic(0);
            g_bTargetLockMode = 0;
        }
    }
    remove_message(g_pszMissileLocked);
    g_nTargetLockCountdown = -1;
    g_nTargetLockMusicCooldown = -98;
}

/* Function start: 0x43BEFF */
short CheckTargetLockMalfunction(void)
{
    if (malf(5) != 0) {
        g_bTargetLockActive = 0;
        lock_off();
        g_nTargetLockCountdown =
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
    if (g_nTargetLockCountdown > 0) {
        if (malf(5) == 0) {
            g_nTargetLockCountdown--;
            g_bTargetLockAcquired =
                g_nTargetLockCountdown == 0;
            if (g_bTargetLockAcquired != 0) {
                if (CheckTargetLockMalfunction() == 0)
                    PlaySfxWaveFileByNumber(0x16, -1, 0);
                CockpitMessage(g_pszMissileLocked,
                               g_cRedColour, 2);
            } else if (((ShipWeaponSlot *)(
                           g_aShipWeapons[0] + 1))[
                           g_nSelectedReleaseWeaponIndex]
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

    if (g_nCurrentView == 4)
        return;
    if (target == -1 ||
        g_asShipSide[(short)target] == g_asShipSide[0] ||
        g_acPlayerComponentDamage[5] >= 4) {
        g_bTargetLockActive = 0;
        lock_off();
        return;
    }
    if (g_asObjectScreenX[(short)target] == -0x7fff)
        return;
    x = g_asObjectScreenX[(short)target];
    y = g_asObjectScreenY[(short)target];
    if (g_nTargetLockCountdown < -1) {
        g_nTargetLockCountdown++;
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
    if (g_nSelectedReleaseWeaponIndex < 0) {
        lock_off();
        return;
    }
#endif
    switch (((ShipWeaponSlot *)(g_aShipWeapons[0] + 1))[
                g_nSelectedReleaseWeaponIndex].type) {
    case OBJECT_DATA_JAVELIN_HEAT_SEEKING_MISSILE:
        get_facing_range_from_object(0, (short)target);
        if (g_nTargetFacing > -0x41 ||
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
        else if (g_nCurrentView != 4) {
            get_facing_range_from_object(0, (short)target);
            if (g_bTargetLockMode == 0 ||
                g_nTargetLockMusicCooldown -
                        g_nFacingToTarget >
                    4) {
                lock_off();
            } else {
                g_nTargetLockMusicCooldown =
                    g_nFacingToTarget;
                if (g_nTargetLockMusicCooldown > 100)
                    g_nTargetLockMusicCooldown = 100;
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
    menuViewport = *g_3_stDefaultStringContext.viewport;
    menuContext = g_3_stDefaultStringContext;
    menuContext.viewport = &menuViewport;
    topY = (short)(g_stHudMessageOrigin.y +
                       g_49_nViewportOffsetY);
    SetRectBounds(&menuViewport,
                  g_stHudMessageOrigin.x,
                  topY,
                  (short)(319 - g_stHudMessageOrigin.x),
                  (short)(topY + 60));
    menuContext.colour = (unsigned char)colour;
    menuContext.backgroundColour = 0xff;
    flightView = *g_168_stFlightTextContext.viewport;
    flightText = g_168_stFlightTextContext;
    flightText.viewport = &flightView;
    SetRectBounds(&flightView,
                  g_stHudMessageOrigin.x,
                  g_stHudMessageOrigin.y,
                  (short)(319 - g_stHudMessageOrigin.x),
                  (short)(g_stHudMessageOrigin.y + 60));
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
               g_stHudMessageOrigin.x,
               topY,
               wrappedText, 2);
    g_pszDisplayedHudMessage = g_pszPendingHudMessage;
}

/* Function start: 0x43C570 */
void ShowHudTextLine(void)
{
    CaptureSpriteBackground(
        &g_101_stScrViewInfo,
        g_pHudMessageBackground,
        0,
        (short)(g_stHudMessageOrigin.y +
                g_49_nViewportOffsetY),
        g_pHudMessageFrameShape,
        0);
    g_nHudMessageBackgroundDepth++;
}

/* Function start: 0x43C5B0 */
void RestoreHudMessageBackground(void)
{
    if (g_nHudMessageBackgroundDepth != 0) {
        RestoreSpriteBackground(
            &g_101_stScrViewInfo,
            g_pHudMessageBackground,
            0,
            (short)(g_stHudMessageOrigin.y +
                    g_49_nViewportOffsetY),
            g_pHudMessageFrameShape,
            0);
    }
    g_nHudMessageBackgroundDepth = 0;
}

/* Function start: 0x43C601 */
void SetHudTextColour(char *text, unsigned short colour)
{
    if (g_bCaptureHudMessageBackground != 0) {
        ShowHudTextLine();
        g_bCaptureHudMessageBackground = 0;
    }
    g_pszPendingHudMessage = text;
    g_3_cOverlayMessageView = (signed char)g_nCurrentView;
    print_message_text(g_pszPendingHudMessage, colour);
}

/* Function start: 0x43C64D */
void ClearHudMessageDisplay(short closeCommMenu)
{
    if (closeCommMenu != 0)
        EndCommMenu();
    if (g_3_cOverlayMessageView == g_nCurrentView &&
        g_pszDisplayedHudMessage != 0 &&
        g_nHudMessageBackgroundDepth != 0) {
        print_message_text(g_pszDisplayedHudMessage,
                           g_cPrimaryViewBufferColour);
        RestoreHudMessageBackground();
    }
    g_bCaptureHudMessageBackground = 1;
    if (g_bDisplayWingmanTargetData != 0)
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

    if ((int)(unsigned char)g_cPrimaryViewBufferColour ==
        (int)(short)colour) {
        visible = savedBounds->left != -0x7fff;
        bounds = *savedBounds;
    } else {
        visible = object != -1 &&
                g_asObjectScreenX[object] != -0x7fff;
        if (visible != 0) {
            midX = (short)(g_asObjectScreenX[object] +
                              g_15_nStoredScreenViewCentreX);
            midY = (short)(g_asObjectScreenY[object] +
                              g_37_nViewCenterY);
            if ((short)GetTransformedShapeBounds(
                    &g_27_stViewScratchInfo, midX, midY,
                    g_apObjectShape[object],
                    g_asObjectViewFrame[object],
                    g_asObjectScreenAngle[object],
                    g_asObjectScreenScale[object],
                    g_asObjectFlip[object],
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
        if ((int)(unsigned char)g_cRedColour ==
                (int)(short)colour &&
            g_asShipIdentified[object] == 0) {
            colour = g_cYellowColour;
        }
        if ((int)(unsigned char)g_cRedColour ==
                (int)(short)colour &&
            g_asShipSide[object] == SIDE_IMPERIAL) {
            colour = g_cBlueColour;
        }
        if (solid != 0) {
            DrawViewportBorder(&g_27_stViewScratchInfo, bounds.left, bounds.top,
                               bounds.right, bounds.bottom, colour);
        } else {
            segment =
                (short)(((int)bounds.right - bounds.left) / 6 + 1);
            DrawViewportLine(&g_27_stViewScratchInfo, bounds.left, bounds.top,
                             (short)(bounds.left + segment), bounds.top,
                             colour);
            DrawViewportLine(&g_27_stViewScratchInfo, bounds.left, bounds.bottom,
                             (short)(bounds.left + segment), bounds.bottom,
                             colour);
            DrawViewportLine(&g_27_stViewScratchInfo, bounds.right, bounds.top,
                             (short)(bounds.right - segment), bounds.top,
                             colour);
            DrawViewportLine(&g_27_stViewScratchInfo, bounds.right, bounds.bottom,
                             (short)(bounds.right - segment), bounds.bottom,
                             colour);
            segment =
                (short)(((int)bounds.bottom - bounds.top) / 6 + 1);
            DrawViewportLine(&g_27_stViewScratchInfo, bounds.left, bounds.top,
                             bounds.left, (short)(bounds.top + segment),
                             colour);
            DrawViewportLine(&g_27_stViewScratchInfo, bounds.left, bounds.bottom,
                             bounds.left,
                             (short)(bounds.bottom - segment), colour);
            DrawViewportLine(&g_27_stViewScratchInfo, bounds.right, bounds.top,
                             bounds.right, (short)(bounds.top + segment),
                             colour);
            DrawViewportLine(&g_27_stViewScratchInfo, bounds.right, bounds.bottom,
                             bounds.right,
                             (short)(bounds.bottom - segment), colour);
        }
        if (drawLockMarker != 0) {
            if ((int)(short)colour != (int)(unsigned char)g_cPrimaryViewBufferColour) {
                if (g_nTargetLockCountdown > -1) {
                    g_nTargetLockMarkerAngle = (short)(
                        g_anObjectRollRotation[0] +
                        g_anObjectPitchRotation[0] +
                        g_nTargetLockMarkerAngle);
                    size =
                        (short)(g_nTargetLockCountdown * 2);
                    if (((ShipWeaponSlot *)(g_aShipWeapons[0] +
                             1))[g_nSelectedReleaseWeaponIndex]
                            .type == 0x13)
                        size >>= 3;
                    midX = (short)(midX +
                        ((CosFixed(g_nTargetLockMarkerAngle) *
                          size) >> 8));
                    midY = (short)(midY +
                        ((SinFixed(g_nTargetLockMarkerAngle) *
                          size) >> 8));
                    DrawSpriteDefault(&g_27_stViewScratchInfo, midX, midY,
                                      g_66_pCabinHudImage, 1);
                    g_nTargetLockMarkerX = midX;
                    g_36_nTargetLockonMarkerY = midY;
                }
            } else if (g_nTargetLockMarkerX != -0x7fff) {
                DrawSolidColourSprite(&g_27_stViewScratchInfo,
                                      g_nTargetLockMarkerX,
                                      g_36_nTargetLockonMarkerY,
                                      g_66_pCabinHudImage, 1,
                                      g_cPrimaryViewBufferColour);
                g_nTargetLockMarkerX = -0x7fff;
            }
        }
        if ((int)(unsigned char)g_cPrimaryViewBufferColour == (int)(short)colour) {
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
    if (g_nNavPointerObject != -1)
        remove_object(g_nNavPointerObject);
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
        g_nTrainSimActive != 4 &&
        g_nCannedSceneMode == 0 &&
        (g_nCurrentView == 0 || g_nCurrentView == 5) &&
        IsMissionObjectiveOutOfSystem(
            (short)g_cCurrentObjective) == 0)
        onScreen = 1;
    else
        onScreen = 0;
    if (onScreen == 0) {
        remove_nav_pointer();
        return;
    }
    if (g_nNavPointerObject == -1) {
        g_nNavPointerObject = find_vacant_3d_object();
        if (g_nNavPointerObject == -1)
            return;
        g_aeObjectClass[g_nNavPointerObject] =
            OBJECT_CLASS_PLANET;
        g_asObjectViewFrame[g_nNavPointerObject] = 3;
        g_asObjectScreenAngle[g_nNavPointerObject] = 0;
        g_asObjectScreenScale[g_nNavPointerObject] = 0x100;
        g_apObjectShape[g_nNavPointerObject] =
            g_66_pCabinHudImage;
        g_acObjectOwner[g_nNavPointerObject] = -1;
    }
    g_asObjectScreenX[g_nNavPointerObject] = (short)0x8001;
    g_asObjectDistance[g_nNavPointerObject] = 0;
    objectivePos = g_aMissionObjectives[
        (signed char)g_cCurrentObjective].position;
    ComputeVectorDelta(&g_aShipPosition[EYE_OBJECT],
                       &objectivePos, &direction);
    magnitude = Vector_magnitude(&direction);
    if (g_asObjectCollisionRadius[EYE_OBJECT] * 0x100 <
        magnitude) {
        transform_to_objects_frame(&direction, &eyePos,
                                   EYE_OBJECT);
        if (g_asObjectCollisionRadius[EYE_OBJECT] * 0x100 <=
            eyePos.z) {
            if (DivideFixed(eyePos.z, magnitude) >= 0) {
                scale = ((g_nScreenWidth >> 1) << 1) << 7;
                projected = MultiplyFixed(scale, eyePos.x);
                screenCoord = DivideFixed(projected, eyePos.z);
                g_asObjectScreenX[
                    g_nNavPointerObject] =
                    (short)(screenCoord >> 8);
                projected = MultiplyFixed(scale, eyePos.y);
                screenCoord = DivideFixed(projected, eyePos.z);
                g_asObjectScreenY[
                    g_nNavPointerObject] =
                    (short)(screenCoord >> 8);
                g_asObjectDistance[
                    g_nNavPointerObject] = 0x4a38;
            }
        }
    }
}

/* Function start: 0x43CE8F */
short HasInRangeGunForTargetLead(short targetRange)
{
    short prior;
    short count;

    count = (signed char)g_aShipWeapons[0][0];
    while ((prior = count--) > 0) {
        if (((ShipWeaponSlot *)(
                g_aShipWeapons[0] + 1))[count].disabled == 0 &&
            g_aObjectTypeData[(signed char)((ShipWeaponSlot *)(
                g_aShipWeapons[0] + 1))[count].weaponType]
                    .objectClass == OBJECT_CLASS_PROJECTILE &&
            g_aObjectTypeData[(signed char)((ShipWeaponSlot *)(
                g_aShipWeapons[0] + 1))[count].weaponType]
                    .maximumVelocity *
                    g_aObjectTypeData[(signed char)(
                        (ShipWeaponSlot *)(
                            g_aShipWeapons[0] + 1))[count]
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

    if ((signed char)g_bTargetLockDisplayEnabled == 0)
        return;
    if (g_bTargetLockMode == 0) {
        g_bTargetLockActive = 0;
        return;
    }

    tracked = g_acShipTarget[0];
    rangeToTarget = distance_from_object(0, tracked);
    boltSpeed = 100;
    if (g_nCurrentView == 4) {
        boltSpeed =
            g_aObjectTypeData[
                OBJECT_TYPE_TIGERS_CLAW].maximumVelocity;
    } else {
        gun = (signed char)g_aShipWeapons[0][0];
        while (gun-- > 0) {
            if (((ShipWeaponSlot *)(
                    g_aShipWeapons[0] + 1))[gun].disabled == 0 &&
                g_aObjectTypeData[
                    ((ShipWeaponSlot *)(g_aShipWeapons[0] + 1))[
                        gun].weaponType].objectClass ==
                    OBJECT_CLASS_PROJECTILE &&
                boltSpeed <
                    g_aObjectTypeData[
                        ((ShipWeaponSlot *)(
                            g_aShipWeapons[0] + 1))[
                            gun].weaponType].maximumVelocity)
                boltSpeed =
                    g_aObjectTypeData[
                        ((ShipWeaponSlot *)(
                            g_aShipWeapons[0] + 1))[
                            gun].weaponType].maximumVelocity;
        }
    }

    vector_component_in_dir(
        &g_aShipVelocity[0],
        &g_aShipForwardVector[0],
        &combinedProjectileVelocity);
    ScaleFixedVector(
        &g_aShipForwardVector[0],
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
            &g_aShipVelocity[tracked],
            (int)frames << 8, &targetMotion);
    AddFixedVectors(
        &g_aShipPosition[tracked],
        &targetMotion, &meetPoint);
    if (HasInRangeGunForTargetLead(
            distance_from_point(0, &meetPoint)) == 0) {
        SetHudMessageText(
            "OUT OF RANGE",
            g_cRedColour, 20);
        g_bTargetLockActive = 0;
        return;
    }

    ComputeVectorDelta(
        &g_aShipPosition[0], &meetPoint, &scratch);
    targetDistance = Vector_magnitude(&scratch);
    if ((int)g_asObjectCollisionRadius[
            EYE_OBJECT] * 0x100 < targetDistance) {
        transform_to_objects_frame(
            &scratch, &eyeRelative, EYE_OBJECT);
        if ((int)g_asObjectCollisionRadius[0] * 0x100 <=
                eyeRelative.z &&
            DivideFixed(eyeRelative.z, targetDistance) >= 0) {
            g_nTargetLeadIndicatorX =
                (short)DivideFixed(
                    MultiplyFixed(g_nScreenWidth >> 1,
                                  eyeRelative.x),
                    eyeRelative.z);
            g_nTargetLeadIndicatorX =
                (short)(g_15_nStoredScreenViewCentreX +
                        g_nTargetLeadIndicatorX);
            g_1_nTargetLeadingIndicatorY =
                (short)DivideFixed(
                    MultiplyFixed(g_nScreenWidth >> 1,
                                  eyeRelative.y),
                    eyeRelative.z);
            g_1_nTargetLeadingIndicatorY =
                (short)(g_37_nViewCenterY +
                        g_1_nTargetLeadingIndicatorY);
        }
    }
    if (g_nTargetLeadIndicatorX >= 10 &&
        g_nTargetLeadIndicatorX <= 310 &&
        g_1_nTargetLeadingIndicatorY >= 5 &&
        g_1_nTargetLeadingIndicatorY <= 190) {
        CaptureSpriteBackground(
            &g_27_stViewScratchInfo,
            g_pCockpitHudBackground,
            g_nTargetLeadIndicatorX,
            g_1_nTargetLeadingIndicatorY,
            g_66_pCabinHudImage, 5);
        DrawSpriteDefault(
            &g_27_stViewScratchInfo,
            g_nTargetLeadIndicatorX,
            g_1_nTargetLeadingIndicatorY,
            g_66_pCabinHudImage, 5);
    } else {
        g_nTargetLeadIndicatorX = 0x7fff;
    }
}

/* Function start: 0x43D323 */
void RestoreTargetLeadIndicator(void)
{
    if ((signed char)g_bTargetLockDisplayEnabled == 0)
        return;
    if (g_nTargetLeadIndicatorX != 0x7fff) {
        RestoreSpriteBackground(&g_27_stViewScratchInfo,
                                g_pCockpitHudBackground,
                                g_nTargetLeadIndicatorX,
                                g_1_nTargetLeadingIndicatorY,
                                g_66_pCabinHudImage, 5);
        g_nTargetLeadIndicatorX = 0x7fff;
    }
}

/* Function start: 0x43D386 */
void overlay_head_up_display(void)
{
    ShortRect reticleBounds;

    reticleBounds.left = (short)(g_15_nStoredScreenViewCentreX - 2);
    reticleBounds.right = (short)(g_15_nStoredScreenViewCentreX + 2);
    reticleBounds.top = (short)(g_37_nViewCenterY - 2);
    reticleBounds.bottom = (short)(g_37_nViewCenterY + 2);
    target_locking(g_acShipTarget[0]);
    if (g_nTargetCameraFrame == 0 &&
        g_acShipTarget[0] != -1 &&
        g_aeObjectClass[
            g_acShipTarget[0]] <
                OBJECT_CLASS_CAPITAL_SHIP &&
        g_bTargetLockActive == 0 &&
        g_acPlayerComponentDamage[5] < 4 &&
        g_bTargetLockMode != 0 &&
        HasInRangeGunForTargetLead(
            distance_from_object(
                0, g_acShipTarget[0])) != 0 &&
        (signed char)g_bTargetLockDisplayEnabled != 0) {
        g_bTargetLockActive = 1;
        SetHudMessageText(
            "ITTS ENGAGED",
            g_cRedColour, 20);
    }
    if (g_acShipTarget[0] == -1 ||
        (g_anShipCloakState[
             g_acShipTarget[0]] == 1 &&
         g_asShipCloakElapsedFrames[
             g_acShipTarget[0]] >= 40)) {
        g_bTargetLockActive = 0;
        g_bTargetLockMode = 0;
    }
    if (g_bTargetLockActive != 0 &&
        g_nTargetCameraFrame == 0)
        UpdateTargetLeadIndicator();
    if (message_showing() && g_nCommSpeakerObject != -1) {
        g_2_cEarlierTargetObjectValue =
            (signed char)g_nCommSpeakerObject;
        draw_target_box(g_bPrimaryViewBufferColour,
                        g_2_cEarlierTargetObjectValue,
                        0, 0, 2,
                        &g_stCommTargetBracketBounds);
    }
    if (g_nCurrentView == 4 &&
        g_nAimCameraMode == 1 &&
        g_acShipTarget[0] != -1) {
        if (g_nTargetCameraFrame == 0) {
            draw_target_box(g_cYellowColour,
                            g_acShipTarget[0],
                            g_bTargetLockMode,
                            1, 1,
                            &g_stTargetBracketBounds);
            g_nTargetCameraZoom = 0x42;
        } else if (g_aObjectTypeData[
                       g_nObjectType62Index].shapeSet == 0) {
            draw_target_box(g_ucDefaultTextColour,
                            g_acShipTarget[0],
                            g_bTargetLockMode,
                            1, 1,
                            &g_stTargetBracketBounds);
        }
    }
    if (g_nCurrentView != 4 ||
        g_nAimCameraMode != 1) {
        if (g_nTargetLockCountdown == 0) {
            if (g_nRenderedSpaceFrame % 2 == 0)
                g_bTargetBracketVisible ^= 1;
            if (g_bTargetBracketVisible == 1)
                draw_target_box(
                    g_cRedColour,
                    g_acShipTarget[0],
                    g_bTargetLockMode,
                    1, 1, &g_stTargetBracketBounds);
        } else {
            draw_target_box(
                g_cRedColour,
                g_acShipTarget[0],
                g_bTargetLockMode,
                1, 1, &g_stTargetBracketBounds);
        }
    }
    if (g_nCurrentView != 4 ||
        g_nAimCameraMode != 1) {
        if (IsSpriteFrameOverlappingRect(
                &reticleBounds,
                g_nTargetLeadIndicatorX,
                g_1_nTargetLeadingIndicatorY,
                g_66_pCabinHudImage, 5) != 0) {
            g_bMissileLockAcquired++;
            DrawSpriteDefault(
                &g_27_stViewScratchInfo,
                (short)(g_15_nStoredScreenViewCentreX + 1),
                g_37_nViewCenterY,
                g_66_pCabinHudImage, 4);
        } else {
            g_bMissileLockAcquired = 0;
            DrawSpriteDefault(
                &g_27_stViewScratchInfo,
                g_15_nStoredScreenViewCentreX,
                g_37_nViewCenterY,
                g_66_pCabinHudImage, 0);
        }
    } else {
        DrawSpriteDefault(
            &g_27_stViewScratchInfo,
            g_15_nStoredScreenViewCentreX,
            g_37_nViewCenterY,
            g_66_pCabinHudImage, 0);
    }
    g_nCockpitResetState = 0;
    if (g_nCockpitControlState == 1) {
        CaptureSpriteBackground(
            &g_27_stViewScratchInfo,
            (unsigned char *)g_37_stPersonnelMousePointerState.scratchBuffer,
            g_37_stPersonnelMousePointerState.x,
            g_37_stPersonnelMousePointerState.y,
            GetInputCursorShape(), g_37_stPersonnelMousePointerState.frame);
        DrawSpriteDefault(
            &g_27_stViewScratchInfo,
            g_37_stPersonnelMousePointerState.x,
            g_37_stPersonnelMousePointerState.y,
            GetInputCursorShape(), g_37_stPersonnelMousePointerState.frame);
    }
}

/* Function start: 0x43D7F2 */
void RestoreCockpitExplosionIfVisible(void)
{
    if (IsCockpitExplosionActive() &&
        g_pCockpitExplosionBackground != 0) {
        RestoreCockpitExplosionBackground();
    }
}

/* Function start: 0x43D81F */
void RestoreTransientCockpitGraphics(void)
{
    if (g_nCockpitControlState == 1)
        RestoreSpriteBackground(
            &g_27_stViewScratchInfo,
            (unsigned char *)g_37_stPersonnelMousePointerState.scratchBuffer,
            g_37_stPersonnelMousePointerState.x,
            g_37_stPersonnelMousePointerState.y,
            GetInputCursorShape(), g_37_stPersonnelMousePointerState.frame);
    if (g_bTargetLockActive != 0)
        RestoreTargetLeadIndicator();
    if (g_2_cEarlierTargetObjectValue != -1) {
        draw_target_box(g_cPrimaryViewBufferColour,
                        g_2_cEarlierTargetObjectValue,
                        0, 0, 2,
                        &g_stCommTargetBracketBounds);
        g_2_cEarlierTargetObjectValue = -1;
    }
    draw_target_box(g_cPrimaryViewBufferColour,
                    g_acShipTarget[0],
                    g_bTargetLockMode, 1, 1,
                    &g_stTargetBracketBounds);
    if (IsCockpitExplosionActive() &&
        g_pCockpitExplosionBackground != 0) {
        CaptureSpriteBackground(
            &g_101_stScrViewInfo,
            g_pCockpitExplosionBackground,
            g_8_stCockpitExplosionPosBlock.x,
            g_8_stCockpitExplosionPosBlock.y,
            g_pCockpitExplosionShape,
            g_nCockpitExplosionFrame);
        DrawSpriteDefault(&g_101_stScrViewInfo,
                          g_8_stCockpitExplosionPosBlock.x,
                          g_8_stCockpitExplosionPosBlock.y,
                          g_pCockpitExplosionShape,
                          g_nCockpitExplosionFrame);
        g_cLastPilotHandFrame = 0xff;
    }
}

/* Function start: 0x43D956 */
void SetHudMessageText(char *text, unsigned short colour,
                       unsigned short duration)
{
    if (g_bInflightComputerActive != 0)
        return;
    if (message_showing())
        ClearHudMessageDisplay(1);
    g_pszPendingHudMessage = text;
    g_18_nHudMessageColour = colour;
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
    Viewport *viewport = vdu == 0 ? &g_2_stLeftwardVduPortBlock : &g_4_stRightVduView;

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
    g_cViableTargetCount = 0;
    if (g_nCurrentView == 4 &&
        g_nAimCameraMode == 1) {
        if (g_nTargetCameraZoom >= 33) {
            for (specialSlot = 0; specialSlot < 10;
                 specialSlot++) {
                if (((g_aeObjectClass[specialSlot] !=
                          OBJECT_CLASS_NULL &&
                      g_asObjectType[specialSlot] == 0x3d) ||
                     g_asObjectType[specialSlot] == 0x2c) &&
                    g_asObjectScreenX[specialSlot] !=
                        (short)0x8001) {
                    posX = (short)(
                        g_asObjectScreenX[specialSlot] +
                        g_15_nStoredScreenViewCentreX);
                    posY = (short)(
                        g_asObjectScreenY[specialSlot] +
                        g_37_nViewCenterY);
                    GetTransformedShapeBounds(
                        &g_27_stViewScratchInfo, posX, posY,
                        g_apObjectShape[specialSlot],
                        g_asObjectViewFrame[specialSlot],
                        g_asObjectScreenAngle[specialSlot],
                        g_asObjectScreenScale[specialSlot],
                        g_asObjectFlip[specialSlot], frameBounds);
                    if (IsSpriteFrameOverlappingRect(
                            (const ShortRect *)frameBounds,
                            g_15_nStoredScreenViewCentreX,
                            g_37_nViewCenterY,
                            g_66_pCabinHudImage,
                            0) != 0) {
                        distance = (unsigned short)
                            g_asObjectDistance[specialSlot];
                        if (distance < 4000) {
                            g_acViableTarget[
                                (int)g_cViableTargetCount] =
                                    (signed char)specialSlot;
                            g_asViableTargetDistance[
                                (int)g_cViableTargetCount] =
                                    (short)distance;
                            g_cViableTargetCount++;
                        }
                    }
                }
            }
        }
    } else {
        for (contact = 1; contact <= 9; contact++) {
            if (g_aeObjectClass[(int)contact] >=
                    OBJECT_CLASS_SHIP &&
                g_aeSpecialManeuver[(int)contact] !=
                    SPECIAL_MANEUVER_UNKNOWN_9 &&
                g_asObjectScreenX[(int)contact] !=
                    (short)0x8001) {
                distance = (unsigned short)
                    g_asObjectDistance[(int)contact];
                if (distance < 12000) {
                    g_acViableTarget[
                        (int)g_cViableTargetCount] = contact;
                    g_asViableTargetDistance[
                        (int)g_cViableTargetCount] =
                            (short)distance;
                    g_cViableTargetCount++;
                    if (g_asShipSide[(int)contact] !=
                        g_asShipSide[0])
                        *hasEnemy = 1;
                }
            }
        }
    }
    if (g_cViableTargetCount > 1)
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

    entryTarget = g_acShipTarget[0];
    build_your_target_list(&anyEnemy);
    if (g_cViableTargetCount == 0) {
        g_acShipTarget[0] = -1;
    } else {
        index = 0;
        while (index < g_cViableTargetCount &&
               g_acViableTarget[(int)index] !=
                   g_acShipTarget[0])
            index++;
        do {
            index = (signed char)((index + 1) %
                                  g_cViableTargetCount);
            g_acShipTarget[0] =
                g_acViableTarget[(int)index];
        } while (anyEnemy != 0 &&
                 g_asShipSide[g_acShipTarget[0]] ==
                     g_asShipSide[0]);
    }
    if (g_acShipTarget[0] != entryTarget)
        g_nTargetLockCountdown = -1;
}

/* Function start: 0x43DDFC */
void check_target(void)
{
    short reselect;
    short anyEnemy;
    short targetIndex;
    short previousTarget;
    short target;

    target = g_acShipTarget[0];
    reselect = 1;
    if (g_nTargetCameraFrame == 1)
        return;
    if (target != -1 &&
        g_aeSpecialManeuver[target] ==
            SPECIAL_MANEUVER_UNKNOWN_9) {
        g_acShipTarget[0] = -1;
        target = g_acShipTarget[0];
    }
    previousTarget = target;
    if (g_bTargetLockMode != 0 &&
        g_nRenderedSpaceFrame % 8 == 0 &&
        malf(5) != 0) {
        g_bTargetLockMode = 0;
        g_bTargetLockActive = 0;
        malf_sound();
    }

    /* Keep the current target while the lock holds it, or while it is on
     * screen and hostile; otherwise fall through and pick a new one. */
    if (target == -1 ||
        (g_bTargetLockMode == 0 &&
         g_asObjectScreenX[target] == (short)0x8001) ||
        (g_bTargetLockMode == 0 &&
         g_asShipSide[target] == g_asShipSide[0])) {
        if (target == -1 && g_nCurrentView != 4)
            g_bTargetLockMode = 0;

        build_your_target_list(&anyEnemy);
        if (g_cViableTargetCount == 0) {
            if (g_nCurrentView == 4 &&
                g_nAimCameraMode == 1) {
                g_acShipTarget[0] = -1;
                if (previousTarget != -1)
                    g_nTargetCameraZoom = 0x21;
            } else if (g_bTargetLockMode != 0) {
                g_acShipTarget[0] = (signed char)previousTarget;
            } else {
                g_acShipTarget[0] = -1;
            }
        } else {
            if (anyEnemy == 0 && previousTarget != -1 &&
                g_asShipSide[previousTarget] ==
                    g_asShipSide[0] &&
                (g_bTargetLockMode != 0 ||
                 g_asObjectScreenX[target] != (short)0x8001)) {
                g_acShipTarget[0] = (signed char)previousTarget;
                reselect = 0;
            }
            if (reselect != 0) {
                targetIndex = 0;
                while (g_cViableTargetCount > targetIndex &&
                       g_asShipSide[
                           (short)g_acViableTarget[
                               targetIndex]] ==
                           g_asShipSide[0])
                    targetIndex++;
                g_acShipTarget[0] =
                    g_acViableTarget[
                        targetIndex %
                        (short)g_cViableTargetCount];
            }
        }
        if (g_acShipTarget[0] != previousTarget) {
            if (g_nCurrentView != 4 && previousTarget != -1 &&
                (g_acShipTarget[0] == -1 ||
                 g_asShipSide[target] !=
                     g_asShipSide[previousTarget]))
                g_bTargetLockMode = 0;
            g_nTargetLockCountdown = -1;
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
        if (g_aeObjectClass[other] == OBJECT_CLASS_MISSILE &&
            g_acShipTarget[other] == obj) {
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
        g_12_abCockpitLightGoalTable[2] = 0;
    }
}

/* Function start: 0x43E297 */
void determine_pilot_hand(void)
{
    short yaw;
    short pitch;

    yaw = g_anFlightInput[1] / 2;
    pitch = g_anFlightInput[0] / 2;
    if (yaw > 0) {
        g_14_cPilotHandFrameCode =
            (unsigned char)MinShort((short)(yaw + 8), 12);
        return;
    }
    if (yaw < 0) {
        g_14_cPilotHandFrameCode =
            (unsigned char)MinShort((short)(4 - yaw), 8);
        return;
    }
    if (pitch > 0) {
        g_14_cPilotHandFrameCode =
            (unsigned char)MinShort((short)(pitch + 12), 16);
        return;
    }
    if (pitch < 0) {
        g_14_cPilotHandFrameCode =
            (unsigned char)MinShort((short)-pitch, 4);
        return;
    }
    g_14_cPilotHandFrameCode = 0;
}

/* Function start: 0x43E371 */
void DrawPilotHandFrame(void)
{
    short x;
    short y;

    x = (short)(g_stPilotHandOrigin.x -
                g_3_stDashViewport.left);
    y = (short)(g_stPilotHandOrigin.y -
                g_3_stDashViewport.top);
    CopyViewportContents(&g_stPilotHandBackgroundViewportBlock,
                         &g_stPilotHandViewportRecord);
    DrawSpriteDefault(&g_stPilotHandViewportRecord, x, y,
                      g_pPilotHandAnimationShapePtr,
                      (short)(signed char)g_14_cPilotHandFrameCode);
    DrawSpriteDefault(
        &g_stPilotHandViewportRecord,
        (short)(x + g_asPilotHandOffsets[
            (signed char)g_14_cPilotHandFrameCode * 2]),
        (short)(y + g_asPilotHandOffsets[
            (signed char)g_14_cPilotHandFrameCode * 2 + 1]),
        g_pPilotHandAnimationShapePtr, 0x11);
    CopyViewportContents(&g_stPilotHandViewportRecord,
                         &g_3_stDashViewport);
    g_cLastPilotHandFrame = g_14_cPilotHandFrameCode;
}

/* Function start: 0x43E43A */
void animate_pilot(void)
{
    if (g_pPilotHandAnimationShapePtr != 0) {
        determine_pilot_hand();
        if (g_cLastPilotHandFrame != g_14_cPilotHandFrameCode)
            DrawPilotHandFrame();
    }
}

/* Function start: 0x43E472 */
void ResetPilotHandAnimation(void)
{
    if (g_pPilotHandAnimationShapePtr != 0) {
        g_cLastPilotHandFrame = 0xff;
        CopyViewportContents(&g_3_stDashViewport,
                             &g_stPilotHandBackgroundViewportBlock);
        animate_pilot();
    }
}

void vid_equiv(short obj, short message);
void real_vid_transmit(short obj, short message);

/* Function start: 0x43E4A8 */
void send_message(short obj, signed char message)
{
    if (g_aeObjectClass[obj] == OBJECT_CLASS_NULL ||
        g_nTrainSimActive != 0)
        return;
    if (g_nYourWingman != -1 &&
        g_nYourWingman == obj &&
        g_bRadioSilence != 0) {
        g_acShipPendingMessage[obj] = -1;
    } else if (obj > -1 && obj < 10 &&
               g_aeObjectClass[obj] >= OBJECT_CLASS_SHIP) {
        if (g_acShipPortrait[obj] != -1) {
            g_acShipPendingMessage[obj] = message;
        } else if (g_asShipMissionIndex[obj] ==
                       g_6_nHomeOperationShipIndexValue ||
                   g_asShipMissionIndex[obj] ==
                       g_asShipMissionParameter[0]) {
            g_acShipPendingMessage[obj] = message;
        } else if (g_asShipSide[obj] == SIDE_KILRATHI) {
            g_acShipPendingMessage[obj] = message;
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

    if (g_nTrainSimActive == 0) {
        if (g_cQueuedNpcPortrait != -1) {
            keptPortrait = g_acShipPortrait[0];
            oldIndex = (signed char)g_asShipMissionIndex[0];
            savedRecord = g_apShipMissionRecord[0];
            g_acShipPortrait[0] = g_cQueuedNpcPortrait;
            for (i = 0; i < 16; i++) {
                if (g_aMissionShips[i].portrait ==
                    g_cQueuedNpcPortrait)
                    break;
            }
            if (i == 16)
                ShowOnScreenMessage(0x270f, "SHOW_MESSAGE failed");
            g_asShipMissionIndex[0] = i;
            g_apShipMissionRecord[0] = &g_aMissionShips[i];
            real_vid_transmit(0, g_cQueuedNpcMessage);
            g_apShipMissionRecord[0] =
                &g_aMissionShips[g_asShipMissionIndex[0]];
            g_acShipPortrait[0] = keptPortrait;
            g_asShipMissionIndex[0] = oldIndex;
            g_cQueuedNpcPortrait = g_cQueuedNpcMessage = -1;
        }
        for (caller = 1;
             message_showing() == 0 && g_nCommDeathSequenceFrame == 0 &&
                 caller <= 9;
             caller++) {
            if (g_aeObjectClass[caller] >= OBJECT_CLASS_SHIP &&
                g_acShipPendingMessage[caller] != -1) {
                vid_equiv(caller, g_acShipPendingMessage[caller]);
                g_acShipPendingMessage[caller] = -1;
            }
        }
        if (RandomBelowOrEqual(5000) > 4998) {
            for (caller = 1;
                 g_nCommSpeakerObject == -1 && caller <= 9;
                 caller++) {
                if (g_aeObjectClass[caller] >= OBJECT_CLASS_SHIP &&
                    g_asShipSide[caller] == SIDE_KILRATHI &&
                    (g_aeShipObjective[caller] == OBJECTIVE_ENGAGE_ENEMY ||
                     g_aeShipObjective[caller] == OBJECTIVE_DESTROY_SHIP) &&
                    (g_asPilotLevel[caller] == 5 ||
                     RandomBelowOrEqual(100) < 20)) {
                    g_acShipPendingMessage[caller] =
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

    damageState = g_69_asCockpitDmgStateList;
    for (damage = 0; damage < 4; damage++, damageState++)
        *damageState = 0;
}

/* Function start: 0x43E8B2 */
void explosion_draw(int useBackgroundViewport)
{
    short hits;
    short x;
    short y;

    if (g_pPendingCockpitDamageShape == 0)
        return;
    for (hits = 0; hits < 4; hits++) {
        if (g_69_asCockpitDmgStateList[hits] == 1) {
            switch (hits) {
            case 0:
                g_bCockpitDamageFrame0Shown = 1;
                break;
            case 2:
                g_bCockpitDamageFrame2Shown = 1;
                break;
            case 3:
                g_bFuelGaugeDamaged = 1;
                break;
            }
            x = g_aCockpitDamagePositions[hits].x;
            y = g_aCockpitDamagePositions[hits].y;
            if (useBackgroundViewport == 0) {
                DrawSpriteDefault(
                    &g_101_stScrViewInfo, x, y,
                    g_pPendingCockpitDamageShape, hits);
            } else {
                DrawSpriteDefault(
                    &g_stCockpitBackgroundViewportRecord, x, y,
                    g_pPendingCockpitDamageShape, hits);
            }
        }
    }
#ifndef SDL_PORT
    /* WCDX keeps this sheet for later hits and cockpit redraws. The SDL
     * port releases it with the other ship-specific cockpit resources. */
    FreePacketAndClear(&g_pPendingCockpitDamageShape, 0);
#endif
}

/* Function start: 0x43E9E2 */
void DrawPendingCockpitDamage(void)
{
    if (g_pPendingCockpitDamageShape == 0)
        return;
    switch (g_36_nPendingDashDamageValue) {
    case 0:
        g_bCockpitDamageFrame0Shown = 1;
        break;
    case 2:
        g_bCockpitDamageFrame2Shown = 1;
        break;
    case 3:
        g_bFuelGaugeDamaged = 1;
        break;
    }
    DrawSpriteDefault(&g_101_stScrViewInfo,
                      g_8_stCockpitExplosionPosBlock.x,
                      g_8_stCockpitExplosionPosBlock.y,
                      g_pPendingCockpitDamageShape,
                      g_36_nPendingDashDamageValue);
    if (g_pPilotHandAnimationShapePtr != 0) {
        DrawSpriteDefault(
            &g_stPilotHandBackgroundViewportBlock,
            (short)(g_8_stCockpitExplosionPosBlock.x -
                    g_3_stDashViewport.left),
            (short)(g_8_stCockpitExplosionPosBlock.y -
                    g_3_stDashViewport.top),
            g_pPendingCockpitDamageShape,
            g_36_nPendingDashDamageValue);
    }
#ifndef SDL_PORT
    FreePacketAndClear(&g_pPendingCockpitDamageShape, 0);
#endif
}

/* Function start: 0x43EAE3 */
void RestoreCockpitExplosionBackground(void)
{
    if (IsCockpitExplosionActive() &&
        g_pCockpitExplosionShape != 0 &&
        g_pCockpitExplosionBackground != 0) {
        RestoreSpriteBackground(
            &g_101_stScrViewInfo,
            g_pCockpitExplosionBackground,
            g_8_stCockpitExplosionPosBlock.x,
            g_8_stCockpitExplosionPosBlock.y,
            g_pCockpitExplosionShape,
            g_nCockpitExplosionFrame);
    }
}

/* Function start: 0x43EB46 */
void cockpit_explosion(void)
{
    if (g_nCockpitExplosionFrame == 0x7fff)
        g_nCockpitExplosionFrame = 0;
    if (IsCockpitExplosionActive()) {
        if (g_nCockpitExplosionFrame == 0)
            PlaySfxWaveFileByNumber(4, -1, 0);
        g_nCockpitExplosionFrame++;
        if (g_nCockpitExplosionFrame == 3)
            DrawPendingCockpitDamage();
        if (IsCockpitExplosionActive() &&
            g_pCockpitExplosionShape != 0 &&
            g_pCockpitExplosionBackground != 0) {
            CaptureSpriteBackground(
                &g_101_stScrViewInfo,
                g_pCockpitExplosionBackground,
                g_8_stCockpitExplosionPosBlock.x,
                g_8_stCockpitExplosionPosBlock.y,
                g_pCockpitExplosionShape,
                g_nCockpitExplosionFrame);
            DrawSpriteDefault(&g_101_stScrViewInfo,
                              g_8_stCockpitExplosionPosBlock.x,
                              g_8_stCockpitExplosionPosBlock.y,
                              g_pCockpitExplosionShape,
                              g_nCockpitExplosionFrame);
            g_cLastPilotHandFrame = 0xff;
        }
        return;
    }
#ifndef SDL_PORT
    /* The shared explosion animation must survive its first playback. */
    FreePacketAndClear(
        (unsigned char **)&g_pCockpitExplosionShape, 0);
#endif
}

/* Function start: 0x43EC50 */
void place_damage_on_cockpit(short damage)
{
    if (g_nCurrentView == 0 &&
        g_69_asCockpitDmgStateList[damage] == 0) {
        g_69_asCockpitDmgStateList[damage] = 1;
        g_36_nPendingDashDamageValue = damage;
        if (g_pCockpitExplosionShape == 0) {
            explosion_draw(0);
        } else if (IsCockpitExplosionActive() == 0) {
            g_8_stCockpitExplosionPosBlock =
                g_aCockpitDamagePositions[damage];
            g_nCockpitExplosionFrame = 0x7fff;
        }
    }
}

/* Function start: 0x43ECD9 */
void vid_transmit(void)
{
    short frame;

    if (g_nCommDeathSequenceFrame != 0) {
        if (g_nCommDeathSequenceFrame == 1) {
            DrawSpriteDefault(&g_4_stRightVduView,
                              g_4_stRightVduView.left,
                              g_4_stRightVduView.top,
                              g_pCommPortraitResource, 0);
            DrawSpriteDefault(&g_4_stRightVduView,
                              g_4_stRightVduView.left,
                              g_4_stRightVduView.top,
                              g_pCommPortraitResource, 5);
            g_nCommDeathSequenceFrame++;
        } else if (g_nCommDeathSequenceFrame == 2) {
            DrawSpriteDefault(&g_4_stRightVduView,
                              g_4_stRightVduView.left,
                              g_4_stRightVduView.top,
                              g_pCommPortraitResource, 0);
            DrawSpriteDefault(&g_4_stRightVduView,
                              g_4_stRightVduView.left,
                              g_4_stRightVduView.top,
                              g_pCommPortraitResource, 6);
            g_nCommDeathSequenceFrame++;
        } else if (g_nCommDeathSequenceFrame == 3) {
            DrawSpriteDefault(&g_4_stRightVduView,
                              g_4_stRightVduView.left,
                              g_4_stRightVduView.top,
                              g_pCommVduFrameResource, 0);
            g_nCommDeathSequenceFrame++;
        } else {
            EndCommSessionWithWingman();
            g_nCommDeathSequenceFrame = 0;
        }
    } else if (g_bCommSpeechPlaying == 0 &&
               COMM_SPEAKER_SIDE(g_nCommSpeakerObject) ==
                   SIDE_NEUTRAL) {
        EndCommMenu();
    } else if (((g_nRenderedSpaceFrame % 2 != 0 &&
                 g_nCockpitDisplayMode == 0) ||
                g_nCockpitDisplayMode != 0) &&
               g_nCommPortraitIndex != -1 &&
               g_bVideoImagesSuppressed == 0 &&
               g_szCommMessageTextText != 0) {
        if (g_aeSpecialManeuver[g_nCommSpeakerObject] ==
                SPECIAL_MANEUVER_UNKNOWN_9 &&
            g_bCommSpeechPlaying == 0 &&
            g_7_asShipBurstStageCountdown[
                g_nCommSpeakerObject] < 7) {
            if (g_7_asShipBurstStageCountdown[
                    g_nCommSpeakerObject] < 3) {
                DrawSpriteDefault(&g_4_stRightVduView,
                                  g_4_stRightVduView.left,
                                  g_4_stRightVduView.top,
                                  g_pCommVduFrameResource, 0);
            } else {
                if (g_7_asShipBurstStageCountdown[
                        g_nCommSpeakerObject] >= 5)
                    frame = 5;
                else if (g_7_asShipBurstStageCountdown[
                             g_nCommSpeakerObject] >= 3)
                    frame = 6;
                DrawSpriteDefault(&g_4_stRightVduView,
                                  g_4_stRightVduView.left,
                                  g_4_stRightVduView.top,
                                  g_pCommPortraitResource, 0);
                DrawSpriteDefault(&g_4_stRightVduView,
                                  g_4_stRightVduView.left,
                                  g_4_stRightVduView.top,
                                  g_pCommPortraitResource, frame);
            }
        } else {
            if (g_nCommPortraitFrame == -1)
                g_nCommPortraitFrame = RandomInRange(1, 4);
            g_19_nCommPortraitAnimationTickWord = RandomInRange(1, 8);
            if (g_19_nCommPortraitAnimationTickWord < 5)
                g_nCommPortraitFrame =
                    g_19_nCommPortraitAnimationTickWord;
            DrawSpriteDefault(&g_4_stRightVduView,
                              g_4_stRightVduView.left,
                              g_4_stRightVduView.top,
                              g_pCommPortraitResource, 0);
            DrawSpriteDefault(&g_4_stRightVduView,
                              g_4_stRightVduView.left,
                              g_4_stRightVduView.top,
                              g_pCommPortraitResource,
                              (short)g_nCommPortraitFrame);
        }
    }
}

/* Function start: 0x43F042 */
void vid_equiv(short obj, short message)
{
    if (get_mode(1) != 4 &&
        g_nTrainSimActive == 0 &&
        g_nCurrentView == 0 &&
        message_showing() == 0)
        real_vid_transmit(obj, message);
}

/* Function start: 0x43F09D */
void update_dead_disp(short a)
{
    malf_noise(a, 1, g_cDarkGreenColour, 0x17, 0);
}

/* Function start: 0x43F0C3 */
void check_stranded(void)
{
    if (g_aMissionShips[
            g_6_nHomeOperationShipIndexValue].state == 3 &&
        any_enemy(0, 30000) == 0)
        g_nArcadeState = 3;
}

/* Function start: 0x43F110 */
void CopyTrainSimPilotViewToVdus(void)
{
    ShortRect rightBounds;
    ShortRect cockpitBounds;
    Viewport handViewport;
    ShortRect overlap;
    ShortRect leftBounds;

    handViewport = g_stPilotHandBackgroundViewportBlock;
    rightBounds.left = g_4_stRightVduView.left;
    rightBounds.top = g_4_stRightVduView.top;
    rightBounds.right = g_4_stRightVduView.right;
    rightBounds.bottom = g_4_stRightVduView.bottom;
    leftBounds.left = g_2_stLeftwardVduPortBlock.left;
    leftBounds.top = g_2_stLeftwardVduPortBlock.top;
    leftBounds.right = g_2_stLeftwardVduPortBlock.right;
    leftBounds.bottom = g_2_stLeftwardVduPortBlock.bottom;
    cockpitBounds.left = g_3_stDashViewport.left;
    cockpitBounds.top = g_3_stDashViewport.top;
    cockpitBounds.right = g_3_stDashViewport.right;
    cockpitBounds.bottom = g_3_stDashViewport.bottom;

    if (GetViewportIntersection(&overlap, &cockpitBounds,
                                &rightBounds) != 0) {
        if (g_stTrainSimRightVduSource.left == 0) {
            g_stTrainSimRightVduSource =
                g_3_stDashViewport;
            g_stTrainSimRightVduSource.left =
                g_4_stRightVduView.left;
            g_stTrainSimRightVduSource.right =
                g_3_stDashViewport.right;
            g_stTrainSimRightVduSource.top =
                g_3_stDashViewport.top;
            g_stTrainSimRightVduSource.bottom =
                g_4_stRightVduView.bottom;
        }
        handViewport.left = (short)(g_stTrainSimRightVduSource.left -
                                    g_3_stDashViewport.left);
        handViewport.bottom =
            (short)(g_stTrainSimRightVduSource.bottom -
                    g_3_stDashViewport.top);
        CopyViewportContents(&g_stTrainSimRightVduSource,
                             &handViewport);
    }

    if (GetViewportIntersection(&overlap, &cockpitBounds,
                                &leftBounds) != 0) {
        if (g_stTrainSimLeftVduSource.left == 0) {
            g_stTrainSimLeftVduSource =
                g_3_stDashViewport;
            g_stTrainSimLeftVduSource.left =
                g_3_stDashViewport.left;
            g_stTrainSimLeftVduSource.right =
                g_2_stLeftwardVduPortBlock.right;
            g_stTrainSimLeftVduSource.top =
                g_3_stDashViewport.top;
            g_stTrainSimLeftVduSource.bottom =
                g_2_stLeftwardVduPortBlock.bottom;
        }
        handViewport.left = (short)(g_stTrainSimLeftVduSource.left -
                                    g_3_stDashViewport.left);
        handViewport.bottom =
            (short)(g_stTrainSimLeftVduSource.bottom -
                    g_3_stDashViewport.top);
        CopyViewportContents(&g_stTrainSimLeftVduSource,
                             &handViewport);
    }
}

/* Function start: 0x43F2F5 */
void update_VDUs(void)
{
    if (g_nCurrentView == 4)
        return;
    if (g_nCockpitDisplayMode != 0) {
        if (IsCockpitWeaponShapeLoaded() != 0) {
            DrawFilledViewportRect(
                &g_101_stScrViewInfo,
                g_2_stLeftwardVduPortBlock.left,
                g_2_stLeftwardVduPortBlock.top,
                g_2_stLeftwardVduPortBlock.right,
                g_2_stLeftwardVduPortBlock.bottom, 0);
        }
        DrawFilledViewportRect(
            &g_101_stScrViewInfo,
            g_4_stRightVduView.left,
            g_4_stRightVduView.top,
            g_4_stRightVduView.right,
            g_4_stRightVduView.bottom, 0);
    }
    if (IsCockpitWeaponShapeLoaded() != 0) {
        SetTextContext(&g_5_stLeftVduStringContextRecord);
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
                if (g_nCockpitDisplayMode != 0) {
                    g_bForceDamageDisplayRedraw = 1;
                    show_damage_disp();
                    g_bForceDamageDisplayRedraw = 0;
                }
                UpdateDamageDisplay();
                break;
            case 1:
                if (g_nCockpitDisplayMode != 0)
                    show_weapon_disp();
                update_status_text();
                break;
            }
        }
        if (get_mode(0) == 0)
            g_12_aHudTextSlotsArray[0].text = 0;
        else
            UpdateMessage(&g_12_aHudTextSlotsArray[0]);
    }

    SetTextContext(&g_1_stRightVduTextContextBlock);
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
            if (g_nCockpitDisplayMode != 0)
                show_communications_disp();
            RefreshCommunicationMenu();
            break;
        case 5:
            if (g_nCockpitDisplayMode != 0)
                show_navigation_disp();
            check_objectives();
            break;
        case 3:
        case 9:
            if (g_nCockpitDisplayMode != 0)
                show_target_disp();
            DrawTargetRangeReadout();
            break;
        case 6:
            vid_transmit();
            break;
        }
    }
    if (get_mode(1) == 6 || get_mode(1) == 0)
        g_12_aHudTextSlotsArray[1].text = 0;
    else
        UpdateMessage(&g_12_aHudTextSlotsArray[1]);
    if (g_nCockpitDisplayMode == 0 &&
        g_pPilotHandAnimationShapePtr != 0)
        CopyTrainSimPilotViewToVdus();
}

/* Function start: 0x43F6C9 */
void update_cockpit(void)
{
    check_target();
    repair_internal_damage();
    if (g_cMissionObjectiveCount != 0)
        update_objective_location(
            (short)(g_nSpaceFrame %
                    (short)g_cMissionObjectiveCount));
    if (g_nCurrentView == 0) {
        if (g_nCockpitDisplayMode == 0)
            RestoreCockpitExplosionBackground();
        update_lights();
        update_missile_warning();
        if (g_bCockpitDamageFrame0Shown == 0)
            draw_3d_scanner();
        else
            set_objective_range(0);
        update_digital_readouts();
        if (g_nCockpitDisplayMode == 0)
            animate_pilot();
        if (g_bCockpitDamageFrame2Shown == 0)
            update_bars();
        draw_cockpit_lights();
        if (g_nCockpitDisplayMode == 0)
            cockpit_explosion();
        npc_communication();
        update_VDUs();
    } else if (g_nCurrentView == 4) {
        if (g_nAimCameraMode == 1) {
            if (g_nTargetCameraFrame == 1)
                UpdateTargetCameraTracking();
            vdu_polygon(1, g_nTargetCameraZoom);
        } else {
            vdu_polygon(0, g_1_nGunShowEnergyPercent);
        }
        draw_3d_scanner();
        if (g_nCockpitDisplayMode == 0)
            UpdateTargetCameraCockpitHook();
    }
    fire_computer_graphic_missile();
    check_stranded();
}

/* Function start: 0x43F81B */
short IsCockpitWeaponShapeLoaded(void)
{
    if (g_1_pDashWeaponPicture != 0)
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
    if (g_nCurrentView == 0) {
        if (malf(3) != 0 ||
            (mode == 4 && malf(4) != 0)) {
            vdu_malf(vdu, 0x17);
            return;
        }
        PlayCockpitSelectionSfx(g_asVduSelectionSound[vdu]);
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
            g_68_nDamageShowTicksNumber = 0;
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
            if (g_cCurrentKey == 0x22) {
                select_new_gun();
            } else if (g_cCurrentKey == 0x11) {
                lock_off();
                select_new_release_weapon((enum ObjectType)-1);
            }
            break;
        }
    }
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
ReleaseWeaponPosition g_3_stReleaseWeaponReadoutLocationBlock; /* 0x005d1c24 */
short g_3_nReleaseWeaponShowKindWord; /* 0x005d1c28 */
short g_1_nTargetLeadingIndicatorY; /* 0x005d1c2a */
unsigned char *g_4_pScannerBlipUnderlayPointer; /* 0x005d1c2c */
signed char g_14_cPilotHandFrameCode; /* 0x005d1c30 */
short g_9_nScreenMessageTimeNumber; /* 0x005d1c32 */
short g_36_nPendingDashDamageValue; /* 0x005d1c34 */
char g_4_nSpareCockptSlot; /* 0x005d1c36 */ /* never referenced */
signed char g_3_cOverlayMessageView; /* 0x005d1c37 */
char g_16_szTextBufferBuffer[256]; /* 0x005d1c40 */
HudMessageSlot g_12_aHudTextSlotsArray[2]; /* 0x005d1d40 */
int g_28_nUnusedCockptWordWord[3]; /* 0x005d1d70 */ /* never referenced */
short g_14_asScannerTargetUnderlay[5]; /* 0x005d1d80 */
char g_12_nUnusedCockptWord1; /* 0x005d1d8a */ /* never referenced */
short g_16_nUnusedCockptWord2; /* 0x005d1d8c */ /* never referenced */
signed char g_2_cEarlierTargetObjectValue; /* 0x005d1d8e */
int g_14_nUnusedCockptWord3; /* 0x005d1d90 */ /* never referenced */
short g_19_nCommPortraitAnimationTickWord; /* 0x005d1d94 */
char g_16_szComponentHitMsg[40]; /* 0x005d1da0 */
signed char g_13_abShipArrivalMessageSent[16]; /* 0x005d1dd0 */
ShortPoint g_116_aWeaponDisplayPositions[16]; /* 0x005d1de0 */
short g_36_nTargetLockonMarkerY; /* 0x005d1e20 */
short g_6_nHomeOperationShipIndexValue; /* 0x005d1e22 */
CockpitReadout g_9_aConsoleMetersTable[6]; /* 0x005d1e30 */
short g_8_nScopePointerX; /* 0x005d1e6c */
short g_9_nScannerCursorYValue; /* 0x005d1e6e */
signed char g_4_abCockpitLightMode[7]; /* 0x005d1e70 */
short g_nTargetLockMusicCooldown; /* 0x005d1e78 */
short g_10_asScannerObjectYTable[11]; /* 0x005d1e80 */
volatile ShortPoint g_8_stCockpitExplosionPosBlock; /* 0x005d1e98 */
short g_15_asScannerObjectX[11]; /* 0x005d1ea0 */
signed char g_12_abCockpitLightGoalTable[7]; /* 0x005d1eb8 */
int g_4_nSevereDmgWarningSfxHandle; /* 0x005d1ec0 */
short g_47_asScannerBackgroundColourList[10]; /* 0x005d1ed0 */
short g_69_asCockpitDmgStateList[4]; /* 0x005d1ee8 */
unsigned short g_18_nHudMessageColour; /* 0x005d1ef0 */
char g_12_szSoundTraceScratch[0x200]; /* 0x005d1f00 */
char *g_3_apszPersonnelImportCaptionsTable[8]; /* 0x005d2100 */
short g_128_nShotHotspotLeft; /* 0x005d2120 */
short g_27_nSceneHotspotTopValue; /* 0x005d2122 */
short g_8_nSceneRegionRightWord; /* 0x005d2124 */
short g_15_nSceneAreaLowestWord; /* 0x005d2126 */
char g_1_nUnusedCockptSlotWord; /* 0x005d2128 */ /* never referenced */
void *g_13_pStartupStellarPacket; /* 0x005d212c */
char g_1_szLegacySaveFolder[0x30]; /* 0x005d2130 */
Viewport g_3_stDashViewport; /* 0x005d2160 */
Viewport g_2_stLeftwardVduPortBlock; /* 0x005d2180 */
Viewport g_101_stScrViewInfo; /* 0x005d21a0 */
TextContext g_168_stFlightTextContext; /* 0x005d21c0 */
Viewport g_9_stCockpitMeterPort; /* 0x005d21e0 */
unsigned char *g_66_pCabinHudImage; /* 0x005d21f4 */
unsigned char *g_36_apTextGlyphs[4]; /* 0x005d2200 */
FixedVector g_9_aDirectionLookRightVector[DIRECTION_VIEW_COUNT]; /* 0x005d2210 */
FixedVector g_137_aDirectionViewUpVectorArray[DIRECTION_VIEW_COUNT]; /* 0x005d2500 */
FixedVector g_2_aDirectionViewForwardAxisTable[DIRECTION_VIEW_COUNT]; /* 0x005d27f0 */
short g_78_bHighMemoryBuffersSetState; /* 0x005d2ad8 */
TextContext g_5_stLeftVduStringContextRecord; /* 0x005d2ae0 */
Viewport g_27_stViewScratchInfo; /* 0x005d2b00 */
Viewport g_4_stRightVduView; /* 0x005d2b20 */
Viewport g_stPilotHandBackgroundViewportBlock; /* 0x005d2b40 */
unsigned char *g_1_pDashWeaponPicture; /* 0x005d2b54 */
