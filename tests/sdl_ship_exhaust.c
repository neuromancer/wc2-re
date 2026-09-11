#include "game.h"

typedef struct ExhaustFrameCase {
    short objectClass;
    short objectType;
    short expandedGraphics;
    short spriteFrame;
    short loadedFrame;
    short expectedFrame;
} ExhaustFrameCase;

int main(void)
{
    const ExhaustFrameCase cases[] = {
        {OBJECT_CLASS_SHIP, 0, 0, 1, 0, 1},
        {OBJECT_CLASS_MISSILE, 0, 1, 1, 0, 1},
        {OBJECT_CLASS_SHIP, 0, 1, 0, 1, 1},
        {OBJECT_CLASS_CAPITAL_SHIP, 0, 0, 0, 1, 1},
        {OBJECT_CLASS_CAPITAL_SHIP, 0, 1, 0, 1, 1},
        {OBJECT_CLASS_BASE, 0, 0, 0, 1, 1},
        {OBJECT_CLASS_SHIP, 0x33, 0, 0, 1, 1},
        {OBJECT_CLASS_SHIP, 0x33, 1, 0, 1, 1},
        {OBJECT_CLASS_CAPITAL_SHIP, 0, 0, 0, 0, 0},
        {OBJECT_CLASS_CAPITAL_SHIP, 0, 0, 0, 2, 2},
        {OBJECT_CLASS_CAPITAL_SHIP, 0, 0, 0, 3, 3}
    };
    /* Four views: two different attachment records, an absent view, and an
     * empty view. Offsets are bytes, followed by six-word exhaust records. */
    short animation[] = {
        8, 22, -1, 36,
        0, 256, -10, 0, -24, 4, -1,
        1, 256, 10, 90, 20, -12, -1,
        -1
    };
    const short ship = 1;
    const short exhaust = 10;
    short object;
    short expectedX;
    short expectedY;
    short expectedAngle;
    short expectedDistance;
    short expectedSpriteFrame;
    float enhancedX;
    float enhancedY;
    unsigned int test;

    memset(&g_aObjectTypeData_00496d30[OBJECT_DATA_THRUSTERS], 0,
           sizeof(g_aObjectTypeData_00496d30[OBJECT_DATA_THRUSTERS]));
    g_aObjectTypeData_00496d30[OBJECT_DATA_THRUSTERS].objectClass =
        OBJECT_CLASS_FIXED_OBJECT;
    g_aObjectTypeData_00496d30[OBJECT_DATA_THRUSTERS].field_18 =
        OBJECT_DATA_THRUSTERS;
    g_apObjectExhaustShape_004953b8[ship] = (unsigned char *)animation;
    g_anShipSpeed_00494e20[ship] = 100;
    g_acShipExhaustHeat_00495660[ship] = 1;
    g_aeSpecialManeuver_00495600[ship] = SPECIAL_MANEUVER_NONE;
    g_nScreenWidth_0049d4d8 = 320;
    g_nViewCenterX_005c80d8 = 160;
    g_nViewCenterY_005c80da = 100;
    for (test = 0; test < sizeof(cases) / sizeof(cases[0]); test++) {
        for (object = 0; object < SPACE_OBJECT_COUNT; object++)
            g_aeObjectClass_00495328[object] = OBJECT_CLASS_NULL;
        g_aeObjectClass_00495328[ship] = cases[test].objectClass;
        g_asObjectType_00495298[ship] = cases[test].objectType;
        g_bExpandedShipGraphicsEnabled_004931a4 = cases[test].expandedGraphics;
        g_asObjectViewFrame_00493508[ship] = cases[test].spriteFrame;
        g_asLoadedShipViewFrame_00495d18[ship] = cases[test].loadedFrame;
        g_asObjectScreenX_00493598[ship] = 80;
        g_asObjectScreenY_00493628[ship] = -40;
        g_asObjectDistance_00493ae8[ship] = 500;
        g_asObjectScreenScale_00493a58[ship] = 512;
        g_asObjectScreenAngle_004936b8[ship] = 90;
        g_asObjectFlip_004939c8[ship] = 0x20;
        g_aObjectViewPosition_0059afa0[ship].x = 0x10000;
        g_aObjectViewPosition_0059afa0[ship].y = -0x8000;
        g_aObjectViewPosition_0059afa0[ship].z = 0x20000;

        place_exhaust_on_ships();
        if (cases[test].expectedFrame >= 2) {
            if (find_child_object(ship, OBJECT_CLASS_FIXED_OBJECT) != -1) {
                fprintf(stderr, "exhaust case %u: an empty view emitted exhaust\n",
                        test);
                return 1;
            }
            continue;
        }
        expectedX = cases[test].expectedFrame == 0 ? -24 : 20;
        expectedY = cases[test].expectedFrame == 0 ? 4 : -12;
        expectedAngle = cases[test].expectedFrame == 0 ? 90 : 0;
        expectedDistance = cases[test].expectedFrame == 0 ? 490 : 510;
        expectedSpriteFrame = cases[test].expectedFrame == 0 ? 12 : 14;
        if (find_child_object(ship, OBJECT_CLASS_FIXED_OBJECT) != exhaust ||
            g_asObjectScreenX_00493598[exhaust] != expectedX ||
            g_asObjectScreenY_00493628[exhaust] != expectedY ||
            g_asObjectScreenAngle_004936b8[exhaust] != expectedAngle ||
            g_asObjectViewFrame_00493508[exhaust] < expectedSpriteFrame ||
            g_asObjectViewFrame_00493508[exhaust] > expectedSpriteFrame + 1) {
            fprintf(stderr, "exhaust case %u: attachment uses the wrong ship view\n",
                    test);
            return 1;
        }

        /* The attachment is mirrored vertically, scaled by two, and rolled
         * 90 degrees about the ship at logical screen position (240,60).
         * Check both the indexed coordinates and the enhanced sprite anchor. */
        reposition_fixed_child_objects();
        expectedX = cases[test].expectedFrame == 0 ? 248 : 216;
        expectedY = cases[test].expectedFrame == 0 ? 12 : 100;
        SdlGetThrusterScreenPosition(exhaust, &enhancedX, &enhancedY);
        if (g_asObjectScreenX_00493598[exhaust] +
                g_nViewCenterX_005c80d8 != expectedX ||
            g_asObjectScreenY_00493628[exhaust] +
                g_nViewCenterY_005c80da != expectedY ||
            enhancedX != expectedX || enhancedY != expectedY ||
            g_asObjectDistance_00493ae8[exhaust] != expectedDistance) {
            fprintf(stderr, "exhaust case %u: attachment did not follow the ship\n",
                    test);
            return 1;
        }

        /* On the next frame the ship moves and unrolls. Reuse the exhaust
         * slot, as the flight renderer does, and retain its subpixel anchor. */
        g_aeObjectClass_00495328[exhaust] = OBJECT_CLASS_NULL;
        g_asObjectScreenX_00493598[ship] = 40;
        g_asObjectScreenY_00493628[ship] = 20;
        g_asObjectScreenAngle_004936b8[ship] = 0;
        g_asObjectFlip_004939c8[ship] = 0;
        g_aObjectViewPosition_0059afa0[ship].x = 0x8100;
        g_aObjectViewPosition_0059afa0[ship].y = 0x4100;
        place_exhaust_on_ships();
        reposition_fixed_child_objects();
        expectedX = cases[test].expectedFrame == 0 ? 152 : 240;
        expectedY = cases[test].expectedFrame == 0 ? 128 : 96;
        expectedAngle = cases[test].expectedFrame == 0 ? 0 : 90;
        SdlGetThrusterScreenPosition(exhaust, &enhancedX, &enhancedY);
        if (g_asObjectScreenX_00493598[exhaust] +
                g_nViewCenterX_005c80d8 != expectedX ||
            g_asObjectScreenY_00493628[exhaust] +
                g_nViewCenterY_005c80da != expectedY ||
            g_asObjectScreenAngle_004936b8[exhaust] != expectedAngle ||
            enhancedX != expectedX + 0.3125f ||
            enhancedY != expectedY + 0.3125f) {
            fprintf(stderr, "exhaust case %u: attachment retained its old transform\n",
                    test);
            return 1;
        }
    }
    return 0;
}
