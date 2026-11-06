/*
 *  Cockpit damage display.
 *
 *  Retail object 0x401120-0x40181F (link order; see docs/ORDER.md).
 */
#include "game.h"

unsigned char *g_pDamageDisplayBackground = 0; /* 0x00490060 */
ShortPoint g_aDamageDisplayPositions[9] = {
    {36, 37}, {36, 28}, {36, 30}, {36, 23}, {36, 19},
    {36, 15}, {36, 24}, {36, 16}, {36, 22}
}; /* 0x00490068 */
const char *g_apszComponentNames[9] = {
    "Ion drive",
    "Power plant",
    "Shield gen'r",
    "Computer sys",
    "InterCom unit",
    "Target track",
    "Acc. absorbers",
    "Ejector system",
    "Repair systems"
}; /* 0x00490090 */
unsigned char g_abDamageDisplayFrames[9] = {
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28
}; /* 0x004900b8 */
const char *g_apszDamageSeverityNames[5] = {
    "Ok",
    "Light",
    "Moderate",
    "Heavy",
    "Destroyed"
}; /* 0x004900c8 */

/* Function start: 0x401120 */
void show_damage_disp(void)
{
    char message[40];
    signed char plural;
    signed char component;

    g_15_nDamageDisplayState = 0;
    g_cDamagedPartCount = 0;
    for (component = 0; component < 9; component++) {
        if (g_acPlayerComponentDamage[component] >= 1)
            g_cDamagedPartCount++;
    }

    set_new_vdu(0);
    DrawTextAt(&g_5_stLeftVduStringContextRecord,
               g_2_stLeftwardVduPortBlock.left,
               g_2_stLeftwardVduPortBlock.top,
               "DAMAGE REPORT", 2);
    DrawViewportLine(&g_2_stLeftwardVduPortBlock,
                     g_2_stLeftwardVduPortBlock.left + 2,
                     g_2_stLeftwardVduPortBlock.top + 6,
                     (short)(g_2_stLeftwardVduPortBlock.right - 2),
                     g_2_stLeftwardVduPortBlock.top + 6,
                     g_ucPrimaryTextColour);
    if (g_cDamagedPartCount == 0) {
        DrawTextAt(&g_5_stLeftVduStringContextRecord,
                   g_2_stLeftwardVduPortBlock.left,
                   g_2_stLeftwardVduPortBlock.top + 20,
                   "NO INTERNAL\n\nDAMAGE", 2);
    } else {
        g_5_nWeaponDisplayOriginXValue =
            (short)(g_2_stLeftwardVduPortBlock.left +
                    g_stWeaponDisplayOffset.x);
        g_nArmsDisplayOriginY =
            (short)(g_stWeaponDisplayOffset.y +
                    g_2_stLeftwardVduPortBlock.top);
        DrawSpriteDefault(&g_2_stLeftwardVduPortBlock,
                          g_5_nWeaponDisplayOriginXValue,
                          g_nArmsDisplayOriginY,
                          g_1_pDashWeaponPicture, 0);
        if (g_bForceDamageDisplayRedraw == 0) {
            g_68_nDamageShowTicksNumber = 1;
            g_104_nHarmDisplayPhaseValue = 1;
        }
        if (g_cDamagedPartCount != 1)
            plural = 's';
        else
            plural = ' ';
        sprintf(message, "%d Unit%c Damaged",
                (int)g_cDamagedPartCount, (int)plural);
        ShowComponentHitHudMessage(message,
                                   g_ucPrimaryTextColour, -1);
    }
}

/* Function start: 0x4012E1 */
void UpdateDamageDisplay(void)
{
    short componentCount;
    signed char component;
    signed char attempts;
    signed char damage;

    componentCount = (short)g_cDamagedPartCount;
    g_cDamagedPartCount = 0;
    for (attempts = 0; attempts < 9; attempts++) {
        if (g_acPlayerComponentDamage[attempts] >= 1)
            g_cDamagedPartCount++;
    }

    if ((short)g_cDamagedPartCount != componentCount) {
        InvalidateVduMode(0);
        return;
    }
    if (g_cDamagedPartCount == 0)
        return;

    if (g_nCockpitDisplayMode == 0) {
        g_68_nDamageShowTicksNumber--;
        if (g_68_nDamageShowTicksNumber <= 0) {
            if (g_104_nHarmDisplayPhaseValue == 1) {
                g_68_nDamageShowTicksNumber = 25;
                component = g_cDamageDisplayPart;
                for (attempts = 0; attempts < 9; attempts++) {
                    component++;
                    if (component >= 9)
                        component = 0;
                    damage = g_acPlayerComponentDamage[component];
                    if (damage >= 1) {
                        g_cDamageDisplayPart = component;
                        break;
                    }
                }

                sprintf(
                    g_12_szDamageStatusText,
                    "%s\nDamage: %s",
                    g_apszComponentNames[
                        g_cDamageDisplayPart],
                    g_apszDamageSeverityNames[(int)damage]);
                DrawTextAt(&g_5_stLeftVduStringContextRecord,
                           g_2_stLeftwardVduPortBlock.left + 1,
                           g_2_stLeftwardVduPortBlock.top + 7,
                           g_12_szDamageStatusText, 2);
                g_cDamageDisplayFrame =
                    (signed char)g_abDamageDisplayFrames[
                        g_cDamageDisplayPart];
                g_stHarmSpritePosition =
                    g_aDamageDisplayPositions[
                        g_cDamageDisplayPart];
                /* CL 4.1 folds ~~ after ordering operands, preserving the
                 * retail origin-first loads without extra instructions. */
                g_stHarmSpritePosition.x =
                    (short)(g_stHarmSpritePosition.x +
                            ~~g_5_nWeaponDisplayOriginXValue);
                g_stHarmSpritePosition.y =
                    (short)(g_stHarmSpritePosition.y +
                            ~~g_nArmsDisplayOriginY);
                CaptureSpriteBackground(
                    &g_2_stLeftwardVduPortBlock,
                    g_pDamageDisplayBackground,
                    g_stHarmSpritePosition.x,
                    g_stHarmSpritePosition.y,
                    g_1_pDashWeaponPicture,
                    (short)g_cDamageDisplayFrame);
                DrawViewportLine(
                    &g_2_stLeftwardVduPortBlock,
                    g_2_stLeftwardVduPortBlock.left + 36,
                    g_2_stLeftwardVduPortBlock.top + 22,
                    g_stHarmSpritePosition.x,
                    g_stHarmSpritePosition.y, 0xa9);
                DrawSpriteDefault(
                    &g_2_stLeftwardVduPortBlock,
                    g_stHarmSpritePosition.x,
                    g_stHarmSpritePosition.y,
                    g_1_pDashWeaponPicture,
                    (short)g_cDamageDisplayFrame);
            } else {
                RestoreSpriteBackground(
                    &g_2_stLeftwardVduPortBlock,
                    g_pDamageDisplayBackground,
                    g_stHarmSpritePosition.x,
                    g_stHarmSpritePosition.y,
                    g_1_pDashWeaponPicture,
                    (short)g_cDamageDisplayFrame);
                g_5_stLeftVduStringContextRecord.colour =
                    g_cSecondaryViewBufferColour;
                DrawTextAt(&g_5_stLeftVduStringContextRecord,
                           g_2_stLeftwardVduPortBlock.left + 1,
                           g_2_stLeftwardVduPortBlock.top + 7,
                           g_12_szDamageStatusText, 2);
                g_5_stLeftVduStringContextRecord.colour =
                    g_ucPrimaryTextColour;
                DrawViewportLine(
                    &g_2_stLeftwardVduPortBlock,
                    g_2_stLeftwardVduPortBlock.left + 36,
                    g_2_stLeftwardVduPortBlock.top + 22,
                    g_stHarmSpritePosition.x,
                    g_stHarmSpritePosition.y,
                    g_cSecondaryViewBufferColour);
                g_68_nDamageShowTicksNumber = 4;
            }
            g_104_nHarmDisplayPhaseValue =
                g_104_nHarmDisplayPhaseValue == 0;
        }
    } else {
        g_68_nDamageShowTicksNumber--;
        if (g_68_nDamageShowTicksNumber <= 0) {
            if (g_104_nHarmDisplayPhaseValue == 1) {
                g_68_nDamageShowTicksNumber = 25;
                component = g_cDamageDisplayPart;
                for (attempts = 0; attempts < 9; attempts++) {
                    component++;
                    if (component >= 9)
                        component = 0;
                    damage = g_acPlayerComponentDamage[component];
                    if (damage >= 1) {
                        g_cDamageDisplayPart = component;
                        g_nDisplayedComponentDamage = (int)damage;
                        break;
                    }
                }
            }
            g_104_nHarmDisplayPhaseValue =
                g_104_nHarmDisplayPhaseValue == 0;
        }

        sprintf(
            g_12_szDamageStatusText,
            "%s\nDamage: %s",
            g_apszComponentNames[g_cDamageDisplayPart],
            g_apszDamageSeverityNames[
                g_nDisplayedComponentDamage]);
        DrawTextAt(&g_5_stLeftVduStringContextRecord,
                   g_2_stLeftwardVduPortBlock.left + 1,
                   g_2_stLeftwardVduPortBlock.top + 7,
                   g_12_szDamageStatusText, 2);
        g_cDamageDisplayFrame =
            (signed char)g_abDamageDisplayFrames[
                g_cDamageDisplayPart];
        g_stHarmSpritePosition =
            g_aDamageDisplayPositions[
                g_cDamageDisplayPart];
        g_stHarmSpritePosition.x =
            (short)(g_stHarmSpritePosition.x +
                    ~~g_5_nWeaponDisplayOriginXValue);
        g_stHarmSpritePosition.y =
            (short)(g_stHarmSpritePosition.y +
                    ~~g_nArmsDisplayOriginY);
        CaptureSpriteBackground(
            &g_2_stLeftwardVduPortBlock,
            g_pDamageDisplayBackground,
            g_stHarmSpritePosition.x,
            g_stHarmSpritePosition.y,
            g_1_pDashWeaponPicture,
            (short)g_cDamageDisplayFrame);
        DrawViewportLine(&g_2_stLeftwardVduPortBlock,
                         g_2_stLeftwardVduPortBlock.left + 36,
                         g_2_stLeftwardVduPortBlock.top + 22,
                         g_stHarmSpritePosition.x,
                         g_stHarmSpritePosition.y, 0xa9);
        DrawSpriteDefault(&g_2_stLeftwardVduPortBlock,
                          g_stHarmSpritePosition.x,
                          g_stHarmSpritePosition.y,
                          g_1_pDashWeaponPicture,
                          (short)g_cDamageDisplayFrame);
    }
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short g_68_nDamageShowTicksNumber; /* 0x005d4250 */
char g_15_nUnusedDamageWordNumber; /* 0x005d4252 */ /* never referenced */
short g_5_nWeaponDisplayOriginXValue; /* 0x005d4254 */
short g_nArmsDisplayOriginY; /* 0x005d4256 */
char g_12_szDamageStatusText[70]; /* 0x005d4260 */
signed char g_cDamageDisplayPart; /* 0x005d42a6 */
signed char g_cDamagedPartCount; /* 0x005d42a7 */
int g_nDisplayedComponentDamage; /* 0x005d42a8 */
signed char g_cDamageDisplayFrame; /* 0x005d42ac */
short g_15_nDamageDisplayState; /* 0x005d42ae */
ShortPoint g_stHarmSpritePosition; /* 0x005d42b0 */
short g_104_nHarmDisplayPhaseValue; /* 0x005d42b4 */
