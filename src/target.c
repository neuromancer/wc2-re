/*
 *  Target display.
 *
 *  Retail object 0x43FAC0-0x4401BF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* WC2 reads the manoeuvre slot before it has checked the target, so with
 * nothing targeted it reads the two bytes in front of the array.  What lives
 * there is not the blowing-up marker, so the port answers "no manoeuvre" for
 * an unset target rather than reproduce the overrun. */
#ifdef SDL_PORT
#define SPECIAL_MANEUVER_OF(obj) \
    ((obj) != -1 ? g_aeSpecialManeuver_00495600[obj] \
                 : SPECIAL_MANEUVER_NONE)
#else
#define SPECIAL_MANEUVER_OF(obj) g_aeSpecialManeuver_00495600[obj]
#endif

/* Function start: 0x43FAC0 */
void show_target_disp(void)
{
    short *maximumArmor;
    short frame;
    short target;
    short armor;
    Viewport targetViewport;
    short x;
    short y;
    ObjectTypeData *typeData;

    DrawTextAt(&g_stRightVduTextContext_005d2ce0,
               g_stRightVduViewport_005d2b20.left,
               g_stRightVduViewport_005d2b20.top,
               g_szEmptyTargetDisplayText_0049b490, 2);
    if (g_bTargetLockMode_00493500 != 0 &&
        g_bDisplayWingmanTargetData_0049347c == 0) {
        DrawFormattedText(
            g_szTextColourStringColourFormat_0049b4a8,
            (unsigned int)g_abGamePaletteReservedColours_0049cb54[8],
            g_szLockedTarget_0049b494,
            (unsigned int)g_ucDefaultTextColour_0049cb7c);
    } else {
        DrawFormattedText(g_szTextColourStringFormat_0049b4c4,
                          (unsigned int)g_ucDefaultTextColour_0049cb7c,
                          g_szAutoTargetting_0049b4b0);
    }
    target = g_acShipTarget_00495f20[0];
    if (target != -1 && g_anShipCloakState_00496020[target] == 1 &&
        (g_asShipCloakElapsedFrames_00496060[target] >= 40 ||
         (g_asShipCloakElapsedFrames_00496060[target] >= 20 &&
          g_anCloakVisibilityPattern_00492710[
              g_asShipCloakElapsedFrames_00496060[target] - 20] == 1)))
        target = -1;
    if (g_bDisplayWingmanTargetData_0049347c != 0)
        target = g_nYourWingman_0049346c;
    if (target != -1 &&
        (g_aeObjectClass_00495328[target] < OBJECT_CLASS_SHIP ||
         g_aeSpecialManeuver_00495600[target] ==
             SPECIAL_MANEUVER_UNKNOWN_9)) {
        g_acShipTarget_00495f20[0] = -1;
        target = g_acShipTarget_00495f20[0];
        g_bDisplayWingmanTargetData_0049347c = 0;
    }
    g_cTargetDisplayObject_004934f4 = (signed char)target;
    DrawFormattedText(g_szTargetLabel_0049b4cc);
    if (target == -1) {
        DrawFormattedText(g_szNoTarget_0049b4d8);
        return;
    }
    typeData = &g_aObjectTypeData_00496d30[g_acObjectType_00493980[target]];
    if (g_asShipIdentified_00496078[target] != 0) {
        DrawFormattedText(g_szIdentifiedTargetNameFormat_0049b4e0,
                          g_apShipMissionRecord_00495da8[target]->name);
    } else {
        DrawFormattedText(g_szShipTargetNameFormat_0049b4ec,
                          g_szUnknownTarget_0049b4e4);
    }
    DrawFormattedText(g_szRangeLabel_0049b4f0);
    InitializeCockpitReadout(1, &g_stRightVduTextContext_005d2ce0);
    if (g_asObjectScreenX_00493598[target] == -32767 &&
        g_bDisplayWingmanTargetData_0049347c == 0) {
        g_cTargetDisplayObject_004934f4 = -1;
        return;
    }
    if (g_asShipIdentified_00496078[target] == 0)
        return;
#ifdef SDL_PORT
    /* See issue #30: retail anchors the target silhouette (ship outline,
     * armor overlay, and shield indicator all draw from this same x) 1
     * pixel right of dead centre in the 72px-wide right VDU viewport
     * (0x25 == 37, but half of a 72px-wide viewport is 0x24 == 36).
     * That single native pixel is invisible at DOS's native resolution,
     * but the port upscales the cockpit, which turns it into a visible
     * rightward bias -- the shield bracket's right arc clips against the
     * panel edge while its left arc has a matching gap. Center it
     * properly; ship, armor, and shield all still share this one x, so
     * they stay aligned with each other. */
    x = (short)(g_stRightVduViewport_005d2b20.left + 0x24);
#else
    x = (short)(g_stRightVduViewport_005d2b20.left + 0x25);
#endif
    y = (short)(g_stRightVduViewport_005d2b20.top + 0x26);
    frame = (short)(g_aasShipShield_00495518[target][1] * 6 /
                    typeData->shieldAft);
    frame = (short)((3 - MinShort(frame, 3)) * 2);
    if (frame < 6) {
        DrawSpriteDefault(&g_stRightVduViewport_005d2b20, x, y,
                          g_pCockpitIndicatorShape_005d2c48, frame);
    }
    targetViewport = g_stRightVduViewport_005d2b20;
    maximumArmor = &typeData->armorFront;
    for (armor = 0; armor < 4; armor++) {
        targetViewport.left =
            (short)(g_aTargetArmorClipRects_0049b470[armor].left + x);
        targetViewport.top =
            (short)(g_aTargetArmorClipRects_0049b470[armor].top + y);
        targetViewport.right =
            (short)(g_aTargetArmorClipRects_0049b470[armor].right + x);
        targetViewport.bottom =
            (short)(g_aTargetArmorClipRects_0049b470[armor].bottom + y);
        if (g_aasShipArmor_00495540[target][armor] >
            (short)(maximumArmor[armor] >> 1)) {
            DrawSpriteDefault(&targetViewport, x, y, typeData->shape, 0);
        } else {
            DrawSpriteDefault(&targetViewport, x, y, typeData->shape, 1);
        }
    }
    DrawSpriteDefault(&g_stRightVduViewport_005d2b20, x, y,
                      typeData->shape, 2);
    frame = (short)(g_aasShipShield_00495518[target][0] * 6 /
                    typeData->shieldFore);
    frame = (short)((3 - MinShort(frame, 3)) * 2);
    if (frame < 6) {
        DrawSpriteDefault(&g_stRightVduViewport_005d2b20, x, y,
                          g_pCockpitIndicatorShape_005d2c48,
                          (short)(frame + 1));
    }
}

#pragma function(strcpy, strlen)
/* Function start: 0x43FF40 */
void DrawTargetRangeReadout(void)
{
    short target;

    target = g_acShipTarget_00495f20[0];
    if (g_bDisplayWingmanTargetData_0049347c != 0)
        target = g_nYourWingman_0049346c;
    if (SPECIAL_MANEUVER_OF(target) == SPECIAL_MANEUVER_UNKNOWN_9) {
        g_acShipTarget_00495f20[0] = -1;
        g_bDisplayWingmanTargetData_0049347c = 0;
        InvalidateVduMode(1);
        return;
    }
    if (target != -1 &&
        g_aeObjectClass_00495328[target] < OBJECT_CLASS_SHIP) {
        g_acShipTarget_00495f20[0] = -1;
        target = g_acShipTarget_00495f20[0];
    }
    if (g_cTargetDisplayObject_004934f4 != target ||
        g_nRenderedSpaceFrame_00493138 % 8 == 0 ||
        (target != -1 && g_anShipCloakState_00496020[target] == 1 &&
         g_asShipCloakElapsedFrames_00496060[target] < 40)) {
        set_new_vdu(1);
        show_target_disp();
    }
    if (target != -1) {
        if (g_asObjectScreenX_00493598[target] != -32767) {
            if ((unsigned short)g_asObjectDistance_00493ae8[target] >
                    30000 ||
                (unsigned short)g_asObjectDistance_00493ae8[target] < 0) {
                strcpy(g_szTextScratchBuffer_005d1c40,
                       g_szTargetTooFar_0049b4fc);
            } else {
                strcpy(g_szTextScratchBuffer_005d1c40 +
                           strlen(_itoa((unsigned short)
                                            g_asObjectDistance_00493ae8[
                                                target],
                                        g_szTextScratchBuffer_005d1c40, 10)),
                       g_szRangeMetresSuffix_0049b504);
            }
        } else {
            strcpy(g_szTextScratchBuffer_005d1c40,
                   g_szTargetOffscreenRange_0049b508);
        }
        if (g_bDisplayWingmanTargetData_0049347c != 0) {
            strcpy(g_szTextScratchBuffer_005d1c40,
                   g_szTargetDataXmit_0049b510);
        }
        DrawCockpitReadout(1, g_szTextScratchBuffer_005d1c40);
        if (g_nTargetLockCountdown_004934ec == 0) {
            if (g_bTargetLockAcquired_004934fc == 1)
                g_bTargetLockAcquired_004934fc = 0;
        } else if (g_bTargetLockReadoutDirty_004934e8 != 0) {
            EraseCockpitReadoutRegion(
                &g_stRightVduViewport_005d2b20,
                g_stRightVduViewport_005d2b20.left,
                (short)(g_stRightVduViewport_005d2b20.bottom - 6),
                g_stRightVduViewport_005d2b20.right,
                g_stRightVduViewport_005d2b20.bottom,
                g_cSecondaryViewBufferColour_0049cb4c);
            g_bTargetLockReadoutDirty_004934e8 = 0;
        }
    }
}
