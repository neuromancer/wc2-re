/*
 *  Cockpit damage display.
 *
 *  Retail object 0x401120-0x40181F (link order; see docs/ORDER.md).
 */
#include "game.h"

unsigned char *g_pDamageDisplayBackground_00490060 = 0;
ShortPoint g_aDamageDisplayPositions_00490068[9] = {
    {36, 37}, {36, 28}, {36, 30}, {36, 23}, {36, 19},
    {36, 15}, {36, 24}, {36, 16}, {36, 22}
};
const char *g_apszComponentNames_00490090[9] = {
    "Ion drive",
    "Power plant",
    "Shield gen'r",
    "Computer sys",
    "InterCom unit",
    "Target track",
    "Acc. absorbers",
    "Ejector system",
    "Repair systems"
};
unsigned char g_abDamageDisplayFrames_004900b8[9] = {
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28
};
const char *g_apszDamageSeverityNames_004900c8[5] = {
    "Ok",
    "Light",
    "Moderate",
    "Heavy",
    "Destroyed"
};

/* Function start: 0x401120 */
void show_damage_disp(void)
{
    char message[40];
    signed char plural;
    signed char component;

    g_nDamageDisplayState_005d42ae = 0;
    g_cDamagedPartCount_005d42a7 = 0;
    for (component = 0; component < 9; component++) {
        if (g_acPlayerComponentDamage_00493470[component] >= 1)
            g_cDamagedPartCount_005d42a7++;
    }

    set_new_vdu(0);
    DrawTextAt(&g_stLeftVduStringContextRecord_005d2ae0,
               g_stLeftwardVduPortBlock_005d2180.left,
               g_stLeftwardVduPortBlock_005d2180.top,
               "DAMAGE REPORT", 2);
    DrawViewportLine(&g_stLeftwardVduPortBlock_005d2180,
                     g_stLeftwardVduPortBlock_005d2180.left + 2,
                     g_stLeftwardVduPortBlock_005d2180.top + 6,
                     (short)(g_stLeftwardVduPortBlock_005d2180.right - 2),
                     g_stLeftwardVduPortBlock_005d2180.top + 6,
                     g_ucPrimaryTextColour_0049cb64);
    if (g_cDamagedPartCount_005d42a7 == 0) {
        DrawTextAt(&g_stLeftVduStringContextRecord_005d2ae0,
                   g_stLeftwardVduPortBlock_005d2180.left,
                   g_stLeftwardVduPortBlock_005d2180.top + 20,
                   "NO INTERNAL\n\nDAMAGE", 2);
    } else {
        g_nWeaponDisplayOriginXValue_005d4254 =
            (short)(g_stLeftwardVduPortBlock_005d2180.left +
                    g_stWeaponDisplayOffset_0049ae8c.x);
        g_nArmsDisplayOriginY_005d4256 =
            (short)(g_stWeaponDisplayOffset_0049ae8c.y +
                    g_stLeftwardVduPortBlock_005d2180.top);
        DrawSpriteDefault(&g_stLeftwardVduPortBlock_005d2180,
                          g_nWeaponDisplayOriginXValue_005d4254,
                          g_nArmsDisplayOriginY_005d4256,
                          g_pDashWeaponPicture_005d2b54, 0);
        if (g_bForceDamageDisplayRedraw_0049b2ec == 0) {
            g_nDamageShowTicksNumber_005d4250 = 1;
            g_nHarmDisplayPhaseValue_005d42b4 = 1;
        }
        if (g_cDamagedPartCount_005d42a7 != 1)
            plural = 's';
        else
            plural = ' ';
        sprintf(message, "%d Unit%c Damaged",
                (int)g_cDamagedPartCount_005d42a7, (int)plural);
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

    componentCount = (short)g_cDamagedPartCount_005d42a7;
    g_cDamagedPartCount_005d42a7 = 0;
    for (attempts = 0; attempts < 9; attempts++) {
        if (g_acPlayerComponentDamage_00493470[attempts] >= 1)
            g_cDamagedPartCount_005d42a7++;
    }

    if ((short)g_cDamagedPartCount_005d42a7 != componentCount) {
        InvalidateVduMode(0);
        return;
    }
    if (g_cDamagedPartCount_005d42a7 == 0)
        return;

    if (g_nCockpitDisplayMode_0049d71c == 0) {
        g_nDamageShowTicksNumber_005d4250--;
        if (g_nDamageShowTicksNumber_005d4250 <= 0) {
            if (g_nHarmDisplayPhaseValue_005d42b4 == 1) {
                g_nDamageShowTicksNumber_005d4250 = 25;
                component = g_cDamageDisplayPart_005d42a6;
                for (attempts = 0; attempts < 9; attempts++) {
                    component++;
                    if (component >= 9)
                        component = 0;
                    damage = g_acPlayerComponentDamage_00493470[component];
                    if (damage >= 1) {
                        g_cDamageDisplayPart_005d42a6 = component;
                        break;
                    }
                }

                sprintf(
                    g_szDamageStatusText_005d4260,
                    "%s\nDamage: %s",
                    g_apszComponentNames_00490090[
                        g_cDamageDisplayPart_005d42a6],
                    g_apszDamageSeverityNames_004900c8[(int)damage]);
                DrawTextAt(&g_stLeftVduStringContextRecord_005d2ae0,
                           g_stLeftwardVduPortBlock_005d2180.left + 1,
                           g_stLeftwardVduPortBlock_005d2180.top + 7,
                           g_szDamageStatusText_005d4260, 2);
                g_cDamageDisplayFrame_005d42ac =
                    (signed char)g_abDamageDisplayFrames_004900b8[
                        g_cDamageDisplayPart_005d42a6];
                g_stHarmSpritePosition_005d42b0 =
                    g_aDamageDisplayPositions_00490068[
                        g_cDamageDisplayPart_005d42a6];
                g_stHarmSpritePosition_005d42b0.x =
                    (short)(g_stHarmSpritePosition_005d42b0.x +
                            g_nWeaponDisplayOriginXValue_005d4254);
                g_stHarmSpritePosition_005d42b0.y =
                    (short)(g_stHarmSpritePosition_005d42b0.y +
                            g_nArmsDisplayOriginY_005d4256);
                CaptureSpriteBackground(
                    &g_stLeftwardVduPortBlock_005d2180,
                    g_pDamageDisplayBackground_00490060,
                    g_stHarmSpritePosition_005d42b0.x,
                    g_stHarmSpritePosition_005d42b0.y,
                    g_pDashWeaponPicture_005d2b54,
                    (short)g_cDamageDisplayFrame_005d42ac);
                DrawViewportLine(
                    &g_stLeftwardVduPortBlock_005d2180,
                    g_stLeftwardVduPortBlock_005d2180.left + 36,
                    g_stLeftwardVduPortBlock_005d2180.top + 22,
                    g_stHarmSpritePosition_005d42b0.x,
                    g_stHarmSpritePosition_005d42b0.y, 0xa9);
                DrawSpriteDefault(
                    &g_stLeftwardVduPortBlock_005d2180,
                    g_stHarmSpritePosition_005d42b0.x,
                    g_stHarmSpritePosition_005d42b0.y,
                    g_pDashWeaponPicture_005d2b54,
                    (short)g_cDamageDisplayFrame_005d42ac);
            } else {
                RestoreSpriteBackground(
                    &g_stLeftwardVduPortBlock_005d2180,
                    g_pDamageDisplayBackground_00490060,
                    g_stHarmSpritePosition_005d42b0.x,
                    g_stHarmSpritePosition_005d42b0.y,
                    g_pDashWeaponPicture_005d2b54,
                    (short)g_cDamageDisplayFrame_005d42ac);
                g_stLeftVduStringContextRecord_005d2ae0.colour =
                    g_cSecondaryViewBufferColour_0049cb4c;
                DrawTextAt(&g_stLeftVduStringContextRecord_005d2ae0,
                           g_stLeftwardVduPortBlock_005d2180.left + 1,
                           g_stLeftwardVduPortBlock_005d2180.top + 7,
                           g_szDamageStatusText_005d4260, 2);
                g_stLeftVduStringContextRecord_005d2ae0.colour =
                    g_ucPrimaryTextColour_0049cb64;
                DrawViewportLine(
                    &g_stLeftwardVduPortBlock_005d2180,
                    g_stLeftwardVduPortBlock_005d2180.left + 36,
                    g_stLeftwardVduPortBlock_005d2180.top + 22,
                    g_stHarmSpritePosition_005d42b0.x,
                    g_stHarmSpritePosition_005d42b0.y,
                    g_cSecondaryViewBufferColour_0049cb4c);
                g_nDamageShowTicksNumber_005d4250 = 4;
            }
            g_nHarmDisplayPhaseValue_005d42b4 =
                g_nHarmDisplayPhaseValue_005d42b4 == 0;
        }
    } else {
        g_nDamageShowTicksNumber_005d4250--;
        if (g_nDamageShowTicksNumber_005d4250 <= 0) {
            if (g_nHarmDisplayPhaseValue_005d42b4 == 1) {
                g_nDamageShowTicksNumber_005d4250 = 25;
                component = g_cDamageDisplayPart_005d42a6;
                for (attempts = 0; attempts < 9; attempts++) {
                    component++;
                    if (component >= 9)
                        component = 0;
                    damage = g_acPlayerComponentDamage_00493470[component];
                    if (damage >= 1) {
                        g_cDamageDisplayPart_005d42a6 = component;
                        g_nDisplayedComponentDamage_005d42a8 = (int)damage;
                        break;
                    }
                }
            }
            g_nHarmDisplayPhaseValue_005d42b4 =
                g_nHarmDisplayPhaseValue_005d42b4 == 0;
        }

        sprintf(
            g_szDamageStatusText_005d4260,
            "%s\nDamage: %s",
            g_apszComponentNames_00490090[g_cDamageDisplayPart_005d42a6],
            g_apszDamageSeverityNames_004900c8[
                g_nDisplayedComponentDamage_005d42a8]);
        DrawTextAt(&g_stLeftVduStringContextRecord_005d2ae0,
                   g_stLeftwardVduPortBlock_005d2180.left + 1,
                   g_stLeftwardVduPortBlock_005d2180.top + 7,
                   g_szDamageStatusText_005d4260, 2);
        g_cDamageDisplayFrame_005d42ac =
            (signed char)g_abDamageDisplayFrames_004900b8[
                g_cDamageDisplayPart_005d42a6];
        g_stHarmSpritePosition_005d42b0 =
            g_aDamageDisplayPositions_00490068[
                g_cDamageDisplayPart_005d42a6];
        g_stHarmSpritePosition_005d42b0.x =
            (short)(g_stHarmSpritePosition_005d42b0.x +
                    g_nWeaponDisplayOriginXValue_005d4254);
        g_stHarmSpritePosition_005d42b0.y =
            (short)(g_stHarmSpritePosition_005d42b0.y +
                    g_nArmsDisplayOriginY_005d4256);
        CaptureSpriteBackground(
            &g_stLeftwardVduPortBlock_005d2180,
            g_pDamageDisplayBackground_00490060,
            g_stHarmSpritePosition_005d42b0.x,
            g_stHarmSpritePosition_005d42b0.y,
            g_pDashWeaponPicture_005d2b54,
            (short)g_cDamageDisplayFrame_005d42ac);
        DrawViewportLine(&g_stLeftwardVduPortBlock_005d2180,
                         g_stLeftwardVduPortBlock_005d2180.left + 36,
                         g_stLeftwardVduPortBlock_005d2180.top + 22,
                         g_stHarmSpritePosition_005d42b0.x,
                         g_stHarmSpritePosition_005d42b0.y, 0xa9);
        DrawSpriteDefault(&g_stLeftwardVduPortBlock_005d2180,
                          g_stHarmSpritePosition_005d42b0.x,
                          g_stHarmSpritePosition_005d42b0.y,
                          g_pDashWeaponPicture_005d2b54,
                          (short)g_cDamageDisplayFrame_005d42ac);
    }
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short g_nDamageShowTicksNumber_005d4250;
char g_nUnusedDamageWordNumber_005d4252; /* never referenced */
short g_nWeaponDisplayOriginXValue_005d4254;
short g_nArmsDisplayOriginY_005d4256;
char g_szDamageStatusText_005d4260[70];
signed char g_cDamageDisplayPart_005d42a6;
signed char g_cDamagedPartCount_005d42a7;
int g_nDisplayedComponentDamage_005d42a8;
signed char g_cDamageDisplayFrame_005d42ac;
short g_nDamageDisplayState_005d42ae;
ShortPoint g_stHarmSpritePosition_005d42b0;
short g_nHarmDisplayPhaseValue_005d42b4;
