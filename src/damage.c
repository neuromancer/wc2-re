/*
 *  Cockpit damage display.
 *
 *  Retail object 0x401120-0x40181F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x401120 */
void show_damage_disp(void)
{
    char message[40];
    signed char plural;
    signed char component;

    g_nDamageDisplayState_005d42ae = 0;
    g_cDamagedComponentCount_005d42a7 = 0;
    for (component = 0; component < 9; component++) {
        if (g_acPlayerComponentDamage_00493470[component] >= 1)
            g_cDamagedComponentCount_005d42a7++;
    }

    set_new_vdu(0);
    DrawTextAt(&g_stLeftVduTextContext_005d2ae0,
               g_stLeftVduViewport_005d2180.left,
               g_stLeftVduViewport_005d2180.top,
               g_szDamageReport_00490190, 2);
    DrawViewportLine(&g_stLeftVduViewport_005d2180,
                     g_stLeftVduViewport_005d2180.left + 2,
                     g_stLeftVduViewport_005d2180.top + 6,
                     (short)(g_stLeftVduViewport_005d2180.right - 2),
                     g_stLeftVduViewport_005d2180.top + 6,
                     g_ucPrimaryTextColour_0049cb64);
    if (g_cDamagedComponentCount_005d42a7 == 0) {
        DrawTextAt(&g_stLeftVduTextContext_005d2ae0,
                   g_stLeftVduViewport_005d2180.left,
                   g_stLeftVduViewport_005d2180.top + 20,
                   g_szNoInternalDamage_004901a0, 2);
    } else {
        g_nWeaponDisplayOriginX_005d4254 =
            (short)(g_stLeftVduViewport_005d2180.left +
                    g_nWeaponDisplayOffsetX_0049ae8c);
        g_nWeaponDisplayOriginY_005d4256 =
            (short)(g_nWeaponDisplayOffsetY_0049ae8e +
                    g_stLeftVduViewport_005d2180.top);
        DrawSpriteDefault(&g_stLeftVduViewport_005d2180,
                          g_nWeaponDisplayOriginX_005d4254,
                          g_nWeaponDisplayOriginY_005d4256,
                          g_pCockpitWeaponShape_005d2b54, 0);
        if (g_bForceDamageDisplayRedraw_0049b2ec == 0) {
            g_nDamageDisplayTicks_005d4250 = 1;
            g_nDamageDisplayPhase_005d42b4 = 1;
        }
        if (g_cDamagedComponentCount_005d42a7 != 1)
            plural = 's';
        else
            plural = ' ';
        sprintf(message, g_szDamagedUnitCountFormat_004901b4,
                (int)g_cDamagedComponentCount_005d42a7, (int)plural);
        ShowComponentHitHudMessage(message,
                                   g_ucPrimaryTextColour_0049cb64, -1);
    }
}

/* Function start: 0x4012E1 */
void UpdateDamageDisplay(void)
{
    short componentCount;
    signed char component;
    signed char attempts;
    signed char damage;

    componentCount = (short)g_cDamagedComponentCount_005d42a7;
    g_cDamagedComponentCount_005d42a7 = 0;
    for (attempts = 0; attempts < 9; attempts++) {
        if (g_acPlayerComponentDamage_00493470[attempts] >= 1)
            g_cDamagedComponentCount_005d42a7++;
    }

    if ((short)g_cDamagedComponentCount_005d42a7 != componentCount) {
        InvalidateVduMode(0);
        return;
    }
    if (g_cDamagedComponentCount_005d42a7 == 0)
        return;

    if (g_nCockpitDisplayMode_0049d71c == 0) {
        g_nDamageDisplayTicks_005d4250--;
        if (g_nDamageDisplayTicks_005d4250 <= 0) {
            if (g_nDamageDisplayPhase_005d42b4 == 1) {
                g_nDamageDisplayTicks_005d4250 = 25;
                component = g_cDamageDisplayComponent_005d42a6;
                for (attempts = 0; attempts < 9; attempts++) {
                    component++;
                    if (component >= 9)
                        component = 0;
                    damage = g_acPlayerComponentDamage_00493470[component];
                    if (damage >= 1) {
                        g_cDamageDisplayComponent_005d42a6 = component;
                        break;
                    }
                }

                sprintf(
                    g_szDamageStatusText_005d4260,
                    g_szDamageStatusFormat_004901c8,
                    g_apszComponentNames_00490090[
                        g_cDamageDisplayComponent_005d42a6],
                    g_apszDamageSeverityNames_004900c8[(int)damage]);
                DrawTextAt(&g_stLeftVduTextContext_005d2ae0,
                           g_stLeftVduViewport_005d2180.left + 1,
                           g_stLeftVduViewport_005d2180.top + 7,
                           g_szDamageStatusText_005d4260, 2);
                g_cDamageDisplayFrame_005d42ac =
                    (signed char)g_abDamageDisplayFrames_004900b8[
                        g_cDamageDisplayComponent_005d42a6];
                g_stDamageSpritePosition_005d42b0 =
                    g_aDamageDisplayPositions_00490068[
                        g_cDamageDisplayComponent_005d42a6];
                g_stDamageSpritePosition_005d42b0.x =
                    (short)(g_stDamageSpritePosition_005d42b0.x +
                            g_nWeaponDisplayOriginX_005d4254);
                g_stDamageSpritePosition_005d42b0.y =
                    (short)(g_stDamageSpritePosition_005d42b0.y +
                            g_nWeaponDisplayOriginY_005d4256);
                CaptureSpriteBackground(
                    &g_stLeftVduViewport_005d2180,
                    g_pDamageDisplayBackground_00490060,
                    g_stDamageSpritePosition_005d42b0.x,
                    g_stDamageSpritePosition_005d42b0.y,
                    g_pCockpitWeaponShape_005d2b54,
                    (short)g_cDamageDisplayFrame_005d42ac);
                DrawViewportLine(
                    &g_stLeftVduViewport_005d2180,
                    g_stLeftVduViewport_005d2180.left + 36,
                    g_stLeftVduViewport_005d2180.top + 22,
                    g_stDamageSpritePosition_005d42b0.x,
                    g_stDamageSpritePosition_005d42b0.y, 0xa9);
                DrawSpriteDefault(
                    &g_stLeftVduViewport_005d2180,
                    g_stDamageSpritePosition_005d42b0.x,
                    g_stDamageSpritePosition_005d42b0.y,
                    g_pCockpitWeaponShape_005d2b54,
                    (short)g_cDamageDisplayFrame_005d42ac);
            } else {
                RestoreSpriteBackground(
                    &g_stLeftVduViewport_005d2180,
                    g_pDamageDisplayBackground_00490060,
                    g_stDamageSpritePosition_005d42b0.x,
                    g_stDamageSpritePosition_005d42b0.y,
                    g_pCockpitWeaponShape_005d2b54,
                    (short)g_cDamageDisplayFrame_005d42ac);
                g_stLeftVduTextContext_005d2ae0.colour =
                    g_cSecondaryViewBufferColour_0049cb4c;
                DrawTextAt(&g_stLeftVduTextContext_005d2ae0,
                           g_stLeftVduViewport_005d2180.left + 1,
                           g_stLeftVduViewport_005d2180.top + 7,
                           g_szDamageStatusText_005d4260, 2);
                g_stLeftVduTextContext_005d2ae0.colour =
                    g_ucPrimaryTextColour_0049cb64;
                DrawViewportLine(
                    &g_stLeftVduViewport_005d2180,
                    g_stLeftVduViewport_005d2180.left + 36,
                    g_stLeftVduViewport_005d2180.top + 22,
                    g_stDamageSpritePosition_005d42b0.x,
                    g_stDamageSpritePosition_005d42b0.y,
                    g_cSecondaryViewBufferColour_0049cb4c);
                g_nDamageDisplayTicks_005d4250 = 4;
            }
            g_nDamageDisplayPhase_005d42b4 =
                g_nDamageDisplayPhase_005d42b4 == 0;
        }
    } else {
        g_nDamageDisplayTicks_005d4250--;
        if (g_nDamageDisplayTicks_005d4250 <= 0) {
            if (g_nDamageDisplayPhase_005d42b4 == 1) {
                g_nDamageDisplayTicks_005d4250 = 25;
                component = g_cDamageDisplayComponent_005d42a6;
                for (attempts = 0; attempts < 9; attempts++) {
                    component++;
                    if (component >= 9)
                        component = 0;
                    damage = g_acPlayerComponentDamage_00493470[component];
                    if (damage >= 1) {
                        g_cDamageDisplayComponent_005d42a6 = component;
                        g_nDisplayedComponentDamage_005d42a8 = (int)damage;
                        break;
                    }
                }
            }
            g_nDamageDisplayPhase_005d42b4 =
                g_nDamageDisplayPhase_005d42b4 == 0;
        }

        sprintf(
            g_szDamageStatusText_005d4260,
            g_szDamageStatusFormatHighRes_004901d8,
            g_apszComponentNames_00490090[g_cDamageDisplayComponent_005d42a6],
            g_apszDamageSeverityNames_004900c8[
                g_nDisplayedComponentDamage_005d42a8]);
        DrawTextAt(&g_stLeftVduTextContext_005d2ae0,
                   g_stLeftVduViewport_005d2180.left + 1,
                   g_stLeftVduViewport_005d2180.top + 7,
                   g_szDamageStatusText_005d4260, 2);
        g_cDamageDisplayFrame_005d42ac =
            (signed char)g_abDamageDisplayFrames_004900b8[
                g_cDamageDisplayComponent_005d42a6];
        g_stDamageSpritePosition_005d42b0 =
            g_aDamageDisplayPositions_00490068[
                g_cDamageDisplayComponent_005d42a6];
        g_stDamageSpritePosition_005d42b0.x =
            (short)(g_stDamageSpritePosition_005d42b0.x +
                    g_nWeaponDisplayOriginX_005d4254);
        g_stDamageSpritePosition_005d42b0.y =
            (short)(g_stDamageSpritePosition_005d42b0.y +
                    g_nWeaponDisplayOriginY_005d4256);
        CaptureSpriteBackground(
            &g_stLeftVduViewport_005d2180,
            g_pDamageDisplayBackground_00490060,
            g_stDamageSpritePosition_005d42b0.x,
            g_stDamageSpritePosition_005d42b0.y,
            g_pCockpitWeaponShape_005d2b54,
            (short)g_cDamageDisplayFrame_005d42ac);
        DrawViewportLine(&g_stLeftVduViewport_005d2180,
                         g_stLeftVduViewport_005d2180.left + 36,
                         g_stLeftVduViewport_005d2180.top + 22,
                         g_stDamageSpritePosition_005d42b0.x,
                         g_stDamageSpritePosition_005d42b0.y, 0xa9);
        DrawSpriteDefault(&g_stLeftVduViewport_005d2180,
                          g_stDamageSpritePosition_005d42b0.x,
                          g_stDamageSpritePosition_005d42b0.y,
                          g_pCockpitWeaponShape_005d2b54,
                          (short)g_cDamageDisplayFrame_005d42ac);
    }
}
