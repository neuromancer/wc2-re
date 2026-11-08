/*
 *  Target display.
 *
 *  Retail object 0x43FAC0-0x4401BF (link order; see docs/ORDER.md).
 */
#include "game.h"

ShortRect g_aTargetArmorClipRects[4] = {
    { 12, -20, 29, 20 },
    { -11, 1, 11, 20 },
    { -11, -20, 11, 0 },
    { -29, -20, -12, 20 }
}; /* 0x0049b470 */
char g_szEmptyTargetDisplayText[4] = ""; /* 0x0049b490 */

/* WC2 reads the manoeuvre slot before it has checked the target, so with
 * nothing targeted it reads the two bytes in front of the array.  What lives
 * there is not the blowing-up marker, so the port answers "no manoeuvre" for
 * an unset target rather than reproduce the overrun. */
#ifdef SDL_PORT
#define SPECIAL_MANEUVER_OF(obj) \
    ((obj) != -1 ? g_aeSpecialManeuver[obj] \
                 : SPECIAL_MANEUVER_NONE)
#else
#define SPECIAL_MANEUVER_OF(obj) g_aeSpecialManeuver[obj]
#endif

/* Function start: 0x43FAC0 */
void show_target_disp(void)
{
    short *maximumArmor;
    short ship;
    short cel;
    short x;
    Viewport clip;
    short armor;
    short y;
    ObjectTypeData *typeData;

    DrawTextAt(&g_1_stRightVduTextContextBlock,
               g_4_stRightVduView.left,
               g_4_stRightVduView.top,
               g_szEmptyTargetDisplayText, 2);
    if (g_bTargetLockMode != 0 &&
        g_bDisplayWingmanTargetData == 0) {
        DrawFormattedText(
            "%F%s%F",
            (unsigned int)g_cRedColour,
            _("   LOCKED TARGET"),
            (unsigned int)g_ucDefaultTextColour);
    } else {
        DrawFormattedText("%F%s",
                          (unsigned int)g_ucDefaultTextColour,
                          _("  AUTO TARGETTING"));
    }
    ship = g_acShipTarget[0];
    if (ship != -1 && g_anShipCloakState[ship] == 1 &&
        (g_asShipCloakElapsedFrames[ship] >= 40 ||
         (g_asShipCloakElapsedFrames[ship] >= 20 &&
          g_anCloakVisibilityPattern[
              g_asShipCloakElapsedFrames[ship] - 20] == 1)))
        ship = -1;
    if (g_bDisplayWingmanTargetData != 0)
        ship = g_nYourWingman;
    if (ship != -1 &&
        (g_aeObjectClass[ship] < OBJECT_CLASS_SHIP ||
         g_aeSpecialManeuver[ship] ==
             SPECIAL_MANEUVER_UNKNOWN_9)) {
        g_acShipTarget[0] = -1;
        ship = g_acShipTarget[0];
        g_bDisplayWingmanTargetData = 0;
    }
    g_cTargetDisplayObject = (signed char)ship;
    DrawFormattedText(_("\nTarget:"));
    if (ship == -1) {
        DrawFormattedText(_(" None"));
        return;
    }
    typeData = &g_aObjectTypeData[g_acObjectType[ship]];
    if (g_asShipIdentified[ship] != 0) {
        DrawFormattedText(" %s",
                          g_apShipMissionRecord[ship]->name);
    } else {
        DrawFormattedText(" %s",
                          _("UNKNOWN"));
    }
    DrawFormattedText(_("\nRange : "));
    InitializeCockpitReadout(1, &g_1_stRightVduTextContextBlock);
    if (g_asObjectScreenX[ship] == -32767 &&
        g_bDisplayWingmanTargetData == 0) {
        g_cTargetDisplayObject = -1;
        return;
    }
    if (g_asShipIdentified[ship] != 0) {
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
        x = (short)(g_4_stRightVduView.left + 0x24);
#else
        x = (short)(g_4_stRightVduView.left + 0x25);
#endif
        y = (short)(g_4_stRightVduView.top + 0x26);
        cel = (short)(g_aasShipShield[ship][1] * 6 /
                      typeData->shieldAft);
        cel = (short)((3 - MinShort(cel, 3)) * 2);
        if (cel < 6) {
            DrawSpriteDefault(&g_4_stRightVduView, x, y,
                              g_18_pCockpitIndicatorShape, cel);
        }
        clip = g_4_stRightVduView;
        maximumArmor = &typeData->armorFront;
        for (armor = 0; armor < 4; armor++) {
            clip.left =
                (short)(g_aTargetArmorClipRects[armor].left + x);
            clip.top =
                (short)(g_aTargetArmorClipRects[armor].top + y);
            clip.right =
                (short)(g_aTargetArmorClipRects[armor].right + x);
            clip.bottom =
                (short)(g_aTargetArmorClipRects[armor].bottom + y);
            if (g_aasShipArmor[ship][armor] >
                maximumArmor[armor] >> 1) {
                DrawSpriteDefault(&clip, x, y, typeData->shape, 0);
            } else {
                DrawSpriteDefault(&clip, x, y, typeData->shape, 1);
            }
        }
        DrawSpriteDefault(&g_4_stRightVduView, x, y,
                          typeData->shape, 2);
        cel = (short)(g_aasShipShield[ship][0] * 6 /
                      typeData->shieldFore);
        cel = (short)((3 - MinShort(cel, 3)) * 2);
        if (cel < 6) {
            DrawSpriteDefault(&g_4_stRightVduView, x, y,
                              g_18_pCockpitIndicatorShape,
                              (short)(cel + 1));
        }
    }
}

#pragma function(strcpy, strlen)
/* Function start: 0x43FF40 */
void DrawTargetRangeReadout(void)
{
    short target;

    target = g_acShipTarget[0];
    if (g_bDisplayWingmanTargetData != 0)
        target = g_nYourWingman;
    if (SPECIAL_MANEUVER_OF(target) == SPECIAL_MANEUVER_UNKNOWN_9) {
        g_acShipTarget[0] = -1;
        g_bDisplayWingmanTargetData = 0;
        InvalidateVduMode(1);
        return;
    }
    if (target != -1 &&
        g_aeObjectClass[target] < OBJECT_CLASS_SHIP) {
        g_acShipTarget[0] = -1;
        target = g_acShipTarget[0];
    }
    if (g_cTargetDisplayObject != target ||
        g_nRenderedSpaceFrame % 8 == 0 ||
        (target != -1 && g_anShipCloakState[target] == 1 &&
         g_asShipCloakElapsedFrames[target] < 40)) {
        set_new_vdu(1);
        show_target_disp();
    }
    if (target != -1) {
        if (g_asObjectScreenX[target] != -32767) {
            if ((unsigned short)g_asObjectDistance[target] >
                    30000 ||
                (unsigned short)g_asObjectDistance[target] < 0) {
                strcpy(g_16_szTextBufferBuffer,
                       _("TOO FAR"));
            } else {
                strcpy(g_16_szTextBufferBuffer +
                           strlen(itoa((unsigned short)
                                            g_asObjectDistance[
                                                target],
                                        g_16_szTextBufferBuffer, 10)),
                       " m");
            }
        } else {
            strcpy(g_16_szTextBufferBuffer,
                   "----- m");
        }
        if (g_bDisplayWingmanTargetData != 0) {
            strcpy(g_16_szTextBufferBuffer,
                   _("DATA XMIT"));
        }
        DrawCockpitReadout(1, g_16_szTextBufferBuffer);
        if (g_nTargetLockCountdown == 0) {
            if (g_bTargetLockAcquired == 1)
                g_bTargetLockAcquired = 0;
        } else if (g_bTargetLockReadoutDirty != 0) {
            EraseCockpitReadoutRegion(
                &g_4_stRightVduView,
                g_4_stRightVduView.left,
                (short)(g_4_stRightVduView.bottom - 6),
                g_4_stRightVduView.right,
                g_4_stRightVduView.bottom,
                g_cSecondaryViewBufferColour);
            g_bTargetLockReadoutDirty = 0;
        }
    }
}
