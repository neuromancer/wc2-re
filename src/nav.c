/*
 *  Nav map, location readouts and the virtual screen.
 *
 *  Address range 0x40d000-0x40ffff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: DrawNav* family; string band 0x4687AC-0x4688F4.
 */
#include "wc1.h"

typedef struct TitleActorMotion {
    short x;
    short y;
    short velocityX;
    short velocityY;
} TitleActorMotion;

char g_szTitleConductorFrames_00491c68[] =
    "opoqopoqopoqopoqqrstrq";
char *g_apszTitleActorFrames_00491c80[10] = {
    "abcdefghijkaakkkkaaaalllllllmmll",
    "aaaaaaaabcddeeddccffgghhgghhiiih",
    "aaaaaaaaaabbccbbaaccccccccccaccc",
    "aaaabbbcccddeeddccffggaaggffcaaa",
    "aabbbaacccddddddccddeeeeeeddccca",
    "aaaaaabbbaaabbaaaabbbbaabbbbaccc",
    "aabbcdbbbeaaaaaaeeaaaaaaaaaabaaa",
    "abbbaacccdeeaaeeddaaaaaaaaaadacc",
    "aaaaaaaaabbbccbbbbccddddddccaaaa",
    "abccdefgghiijjiihhkkllmmllkkhnnn"
};
TitleActorMotion g_aTitleActorMotion_00491ca8[10] = {
    { 58, 94, -1, 1 },
    { 186, 94, 0, 1 },
    { 278, 94, 1, 1 },
    { 58, 102, -3, 2 },
    { 186, 102, 0, 2 },
    { 278, 102, 3, 2 },
    { 58, 110, -5, 3 },
    { 186, 110, 0, 3 },
    { 278, 110, 5, 3 },
    { 158, 74, 0, 4 }
};
unsigned char *g_pTitleFieldShape_00491cf8;
volatile short g_nNearHeapActive_00493044 = 0;
volatile short g_nNearHeapMaxDescriptors_00493048 = 0x80;
volatile int g_nNearHeapRelocationBytes_0049304c = 0;
int DAT_004688cc_WC1_UNMAPPED /* no-address */ = 0;
int DAT_004688d0_WC1_UNMAPPED /* no-address */ = 0;
int DAT_004688d4_WC1_UNMAPPED /* no-address */ = 0;
int DAT_004688d8_WC1_UNMAPPED /* no-address */ = 0;
short DAT_004688dc_WC1_UNMAPPED /* no-address */ = 0;




short g_nNavMapCoordinateScaling_0049bb80 = 0;
short g_nNavMapScale_0049bb84 = 1;
NavMapObjectiveStyle g_aNavMapObjectiveStyles_0049bb88[12] = {
    { 1, 2, &g_ucPrimaryTextColour_0049cb64,
      &g_ucDefaultTextColour_0049cb7c,
      &g_ucDefaultTextColour_0049cb7c },
    { 3, 2, &g_cSecondaryViewBufferColour_0049cb4c,
      &g_bPrimaryViewBufferColour_0049cb50,
      &g_ucDefaultTextColour_0049cb7c },
    { 4, 2, &g_ucNavObjectiveMarkerColour_0049cb78,
      &g_ucNavObjectiveMarkerColour_0049cb78,
      &g_ucDefaultTextColour_0049cb7c },
    { 2, 3, &g_ucNavObjectiveMarkerColour_0049cb78,
      &g_ucDefaultTextColour_0049cb7c,
      &g_ucDefaultTextColour_0049cb7c },
    { 2, 3, &g_abGamePaletteReservedColours_0049cb54[8],
      &g_abGamePaletteReservedColours_0049cb54[8],
      &g_ucDefaultTextColour_0049cb7c },
    { 1, 2, &g_ucPrimaryTextColour_0049cb64,
      &g_ucDefaultTextColour_0049cb7c,
      &g_ucDefaultTextColour_0049cb7c },
    { 4, 2, &g_ucNavObjectiveMarkerColour_0049cb78,
      &g_ucNavObjectiveMarkerColour_0049cb78,
      &g_ucDefaultTextColour_0049cb7c },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 }
};
unsigned char *g_pNavMapShape_0049bc48 = 0;
short g_nNavMapLabelCount_0049bc4c = 0;
short g_nNavMapReservedAreaCount_0049bc50 = 0;
char *g_apszShipMissionTypeNames_0049bc58[14] = {
    g_szMissionPatrol_0049bcb8,
    g_szMissionEscort_0049bcc0,
    g_szMissionStrike_0049bcc8,
    g_szMissionDefend_0049bcd0,
    g_szMissionWingman_0049bcd8,
    g_szMissionFlee_0049bce0,
    g_szMissionGotoWarp_0049bce8,
    g_szMissionWarpArrive_0049bcf4,
    g_szMissionRendezvous_0049bd00,
    g_szMissionComeHome_0049bd0c,
    g_szMissionTanker_0049bd18,
    g_szMissionTow_0049bd20,
    g_szMissionBreakOff_0049bd24,
    0
};
char g_szNoFurtherObjectives_0049bc90[32] =
    "Further objectives out-system.";
short g_bBriefingMapActive_0049bcb0 = 0;
int g_bInflightComputerActive_0049bcb4 = 0;
char g_szMissionPatrol_0049bcb8[8] = "Patrol";
char g_szMissionEscort_0049bcc0[8] = "Escort";
char g_szMissionStrike_0049bcc8[8] = "Strike";
char g_szMissionDefend_0049bcd0[8] = "Defend";
char g_szMissionWingman_0049bcd8[8] = "Wingman";
char g_szMissionFlee_0049bce0[8] = "Flee";
char g_szMissionGotoWarp_0049bce8[12] = "Goto Warp";
char g_szMissionWarpArrive_0049bcf4[12] = "Warp Arrive";
char g_szMissionRendezvous_0049bd00[12] = "Rendezvous";
char g_szMissionComeHome_0049bd0c[12] = "Come Home";
char g_szMissionTanker_0049bd18[8] = "Tanker";
char g_szMissionTow_0049bd20[4] = "Tow";
char g_szMissionBreakOff_0049bd24[12] = "Break Off";
char g_szNavLabelTextFormat_0049bd30[12] = "%X%Y%F%s";
char g_szNavAsteroids_0049bd3c[12] = "Asteroids";
char g_szNavMines_0049bd48[8] = "Mines";
char g_szInflightTimeFieldFormat_0049bd50[8] = "%02d";
char g_szStandardTimeFormat_0049bd58[24] =
    "%X%YStandard time %s";
char g_szStandardTimeBlank_0049bd70[4] = " ";
char g_szInflightTimeSuffixFormat_0049bd74[8] = "%02d  ";
char g_szStandardTimeColon_0049bd7c[4] = ":";
char g_szNavLegendNewline_0049bd80[4] = "\n";
char g_szNavMissionFlightPath_0049bd84[20] = "MISSION FLIGHT PATH";
char g_szNavHomeBase_0049bd98[12] = "HOME BASE";
char g_szNavBlankLine_0049bdb0[4] = "\n";
char g_szNavTitleFormat_0049bdb4[8] = "%s\n\n";
char g_szNavSectorFormat_0049bdbc[12] = "Sector: %s\n";
char g_szNavSystemFormat_0049bdc8[16] = "System: %s\n\n";
char g_szNavMissionFormat_0049bdd8[8] = "* %s *\n";
char g_szNavShipFormat_0049bde0[8] = "* %s *\n";
char g_szNavNotesHeading_0049bde8[8] = "\nNotes\n";
char g_szNavNoteFormat_0049bdf0[4] = "%s\n";
char g_szNavLocationFormat_0049bdf4[48] =
    "%X%Y                         Location: %d.%d.%d";
char g_szNavViewportName_0049be30[8] = "VSCREEN";
char g_szBriefingNavMapTitle_0049be38[20] = "Briefing Nav Map";
char g_szConfedNavScan_0049be4c[16] = "ConFed Nav Scan";
unsigned char *g_apTitleSections_005d3f70[13];
void *g_pTitleMusic_005d3fa4;

/* Function start: 0x450136 */
short NavMapPointInsideReservedArea(short area, short x, short y)
{
    return g_aNavMapExclusionRects_005b35a8[area].left <= x &&
           g_aNavMapExclusionRects_005b35a8[area].right >= x &&
           g_aNavMapExclusionRects_005b35a8[area].top <= y &&
           g_aNavMapExclusionRects_005b35a8[area].bottom >= y;
}

/* Function start: 0x4501B2 */
short NavMapLabelFits(short x, short y, short width, short height)
{
    return x > 0 && y > 0 && x + width < 150 && y + height < 135;
}

/* Function start: 0x450210 */
short NavMapLabelPositionAvailable(short x, short y,
                                   short width, short height)
{
    short available;
    short area;
    short checkX;
    short checkY;

    available = NavMapLabelFits(x, y, width, height);
    if (available == 1) {
        area = 0;
        while (area < g_nNavMapReservedAreaCount_0049bc50 &&
               available != 0) {
            checkX = x;
            while (checkX < x + width && available != 0) {
                checkY = y;
                while (checkY < y + height && available != 0) {
                    available = (short)((unsigned short)
                        NavMapPointInsideReservedArea(area, checkX, checkY) == 0);
                    checkY++;
                }
                checkX++;
            }
            area++;
        }
    }
    return available;
}

/* Function start: 0x450329 */
void ResetNavMapReservedAreas(void)
{
    g_nNavMapReservedAreaCount_0049bc50 = 0;
}

/* Function start: 0x45033D */
void ReserveNavMapArea(short x, short y, short width, short height)
{
    g_aNavMapExclusionRects_005b35a8[
        g_nNavMapReservedAreaCount_0049bc50].left = x;
    g_aNavMapExclusionRects_005b35a8[
        g_nNavMapReservedAreaCount_0049bc50].top = y;
    g_aNavMapExclusionRects_005b35a8[
        g_nNavMapReservedAreaCount_0049bc50].right = (short)(x + width);
    g_aNavMapExclusionRects_005b35a8[
        g_nNavMapReservedAreaCount_0049bc50].bottom = (short)(y + height);
    g_nNavMapReservedAreaCount_0049bc50++;
}

/* Function start: 0x4503A7 */
void ResetNavMapLabels(void)
{
    g_nNavMapLabelCount_0049bc4c = 0;
}

/* Function start: 0x4503BB */
short TryPlaceNavMapLabel(short x, short y, short width, short force)
{
    short placed;

    placed = 0;
    if (NavMapLabelPositionAvailable(x, y, width, 6) != 0 ||
        (force != 0 && NavMapLabelFits(x, y, width, 6) != 0)) {
        g_aNavMapLabels_00475e80_WC1_UNMAPPED[g_nNavMapLabelCount_0049bc4c].x = x;
        g_aNavMapLabels_00475e80_WC1_UNMAPPED[g_nNavMapLabelCount_0049bc4c].y = y;
        placed = 1;
    }
    return placed;
}

/* Function start: 0x450458 */
void PlaceNavMapLabel(short x, short y, unsigned short colour,
                      const char *text)
{
    short width;
    short offset;
    short force;

    width = (short)(strlen(text) * 4 + 2);
    g_aNavMapLabels_00475e80_WC1_UNMAPPED[
        g_nNavMapLabelCount_0049bc4c].colour = colour;
    g_aNavMapLabels_00475e80_WC1_UNMAPPED[
        g_nNavMapLabelCount_0049bc4c].text = text;
    offset = -1;
    do {
        offset++;
        if (TryPlaceNavMapLabel((short)(x + offset + 4), y,
                                width, 0) != 0)
            break;
        force = offset == 12;
        if (TryPlaceNavMapLabel((short)(x - width / 2),
                                (short)(y + offset + 5),
                                width, force) != 0)
            break;
        if (TryPlaceNavMapLabel((short)(x - offset - width - 3), y,
                                width, force) != 0)
            break;
        if (TryPlaceNavMapLabel(x, (short)(y + offset + 5),
                                width, force) != 0)
            break;
        if (TryPlaceNavMapLabel((short)(x - width / 2),
                                (short)(y - offset - 9),
                                width, force) != 0)
            break;
    } while (offset != 12);
    ReserveNavMapArea(
        g_aNavMapLabels_00475e80_WC1_UNMAPPED[g_nNavMapLabelCount_0049bc4c].x,
        g_aNavMapLabels_00475e80_WC1_UNMAPPED[g_nNavMapLabelCount_0049bc4c].y,
        width, 6);
    g_nNavMapLabelCount_0049bc4c++;
}

/* Function start: 0x450CF8 */
/* Whether this objective sits in the star system the nav map is showing. */
short IsObjectiveInDisplayedSystem(short objective)
{
    short navPoint;

    switch (g_aMissionObjectives_004932a8[objective].type) {
    case 0:
    case 5:
        if (g_aMissionNavPoints_00491e98[
                g_aMissionObjectives_004932a8[objective].index]
                    .systemIndex == g_stElapsedCampaignDate_005d170c.year)
            return 1;
        return 0;
    default:
        navPoint = find_ships_sphere(
            g_aMissionObjectives_004932a8[objective].index);
        if (navPoint == -1)
            return 1;
        if (g_aMissionNavPoints_00491e98[navPoint].systemIndex ==
            g_stElapsedCampaignDate_005d170c.year)
            return 1;
        return 0;
    }
}

/* Function start: 0x450623 */
/* Two objectives share a nav label when both are jump-point kinds (type 0 or
 * 5) or neither is. */
short ObjectivesShareNavLabel(short objective, short other)
{
    switch (g_aMissionObjectives_004932a8[objective].type) {
    case 0:
    case 5:
        switch (g_aMissionObjectives_004932a8[other].type) {
        case 0:
        case 5:
            return 1;
        default:
            return 0;
        }
    default:
        switch (g_aMissionObjectives_004932a8[other].type) {
        case 0:
        case 5:
            return 0;
        default:
            return 1;
        }
    }
}

/* Function start: 0x4506F8 */
void AddUniqueObjectiveNavLabel(short x, short y,
                                unsigned short colour, const char *text,
                                short objective, short missionShip)
{
    short previous;

    if (missionShip == -1) {
        PlaceNavMapLabel(x, y, colour, text);
        return;
    }
    for (previous = 0; previous < objective; previous++) {
        if (hidden_objective(previous) == 0 &&
            g_aMissionObjectives_004932a8[previous].index ==
                missionShip &&
            ObjectivesShareNavLabel(objective, previous) != 0)
            break;
    }
    if (previous < objective)
        return;
    PlaceNavMapLabel(x, y, colour, text);
}

/* Function start: 0x4507DA */
short IsPointInNavMapLabel(short labelIndex, short x, short y)
{
    if (g_aNavMapLabels_00475e80_WC1_UNMAPPED[labelIndex].x <= x &&
        (unsigned int)x <=
            (unsigned int)(g_aNavMapLabels_00475e80_WC1_UNMAPPED[labelIndex].x +
                           strlen(g_aNavMapLabels_00475e80_WC1_UNMAPPED[
                               labelIndex].text) * 4) &&
        g_aNavMapLabels_00475e80_WC1_UNMAPPED[labelIndex].y <= y &&
        y <= g_aNavMapLabels_00475e80_WC1_UNMAPPED[labelIndex].y + 6)
        return 1;
    return 0;
}

/* Function start: 0x45087F */
void DrawNavMapLabels(void)
{
    short label;

    label = 0;
    while (label < (short)g_nNavMapLabelCount_0049bc4c) {
        DrawFormattedText(g_szNavLabelTextFormat_0049bd30,
                          g_aNavMapLabels_00475e80_WC1_UNMAPPED[label].x,
                          g_aNavMapLabels_00475e80_WC1_UNMAPPED[label].y,
                          g_aNavMapLabels_00475e80_WC1_UNMAPPED[label].colour,
                          g_aNavMapLabels_00475e80_WC1_UNMAPPED[label].text);
        label++;
    }
}

/* Function start: 0x450900 */
void DrawNavRectangleMarker(short x, short y, short size, short shadow,
                            unsigned short colour, short reserve)
{
    if (shadow == 0)
        DrawViewportEllipse(g_stNavLabelTextContext_005d16f0.viewport,
                            x, y, size, (short)((size * 7) / 8),
                            (short)colour);
    else
        DrawViewportEllipseShadow(
            g_stNavLabelTextContext_005d16f0.viewport,
            x, y, size, (short)((size * 7) / 8), (short)colour);
    if (reserve != 0 && size < 5)
        ReserveNavMapArea((short)(x - size), (short)(y - size),
                          (short)(size * 2 + 1),
                          (short)(size * 2 + 1));
}

/* Function start: 0x4509C9 */
void DrawNavSquareOutline(Viewport *viewport, short x, short y,
                          short size, signed char colour)
{
    DrawViewportBorder(viewport, (short)(x - size), (short)(y - size),
                       (short)(x + size), (short)(y + size),
                       (short)colour);
}

/* Function start: 0x450A12 */
void DrawNavSquareMarker(short x, short y, short size, short shadow,
                         short colour, short reserve)
{
    if (size == 0) {
        DrawViewportPixel(g_stNavLabelTextContext_005d16f0.viewport,
                          x, y, colour);
        DrawViewportPixel(g_stNavLabelTextContext_005d16f0.viewport,
                          (short)(x + 1), y, colour);
        DrawViewportPixel(g_stNavLabelTextContext_005d16f0.viewport,
                          x, (short)(y + 1), colour);
        DrawViewportPixel(g_stNavLabelTextContext_005d16f0.viewport,
                          (short)(x + 1),
                          (short)(y + 1), colour);
    } else {
        DrawNavSquareOutline(g_stNavLabelTextContext_005d16f0.viewport,
                             x, y, size,
                             (signed char)colour);
    }
    if (reserve != 0 && size < 5)
        ReserveNavMapArea((short)(x - size), (short)(y - size),
                          (short)(size * 2 + 1),
                          (short)(size * 2 + 1));
    (void)shadow;
}

/* Function start: 0x450B03 */
void DrawNavTriangleOutline(Viewport *viewport, short x, short y,
                            short size, signed char colour)
{
    DrawViewportLine(viewport, x, (short)(y - size),
                     (short)(x + size), (short)(y + size), colour);
    DrawViewportLine(viewport, (short)(x + size), (short)(y + size),
                     (short)(x - size), (short)(y + size), colour);
    DrawViewportLine(viewport, (short)(x - size), (short)(y + size),
                     x, (short)(y - size), colour);
}

/* Function start: 0x450BBA */
void DrawNavTriangleMarker(short x, short y, short size, short shadow,
                           unsigned short colour, short reserve)
{
    DrawNavTriangleOutline(g_stNavLabelTextContext_005d16f0.viewport,
                           x, y, size,
                           (signed char)colour);
    if (reserve != 0 && size < 5)
        ReserveNavMapArea((short)(x - size), (short)(y - size),
                          (short)(size * 2 + 1),
                          (short)(size * 2 + 1));
    (void)shadow;
}

/* Function start: 0x450C2A */
void DrawNavCrossMarker(short x, short y, short size, short shadow,
                        unsigned short colour, short reserve)
{
    DrawViewportLine(g_stNavLabelTextContext_005d16f0.viewport,
                     (short)(x - size), (short)(y - size),
                     (short)(x + size), (short)(y + size),
                     (short)colour);
    DrawViewportLine(g_stNavLabelTextContext_005d16f0.viewport,
                     (short)(x - size), (short)(y + size),
                     (short)(x + size), (short)(y - size),
                     (short)colour);
    if (reserve != 0 && size < 5)
        ReserveNavMapArea((short)(x - size), (short)(y - size),
                          (short)(size * 2 + 1),
                          (short)(size * 2 + 1));
    (void)shadow;
}

/* Function start: 0x450DF6 */
void SetScreenClipRect(unsigned short a, unsigned short b,
                       unsigned short c, unsigned short d)
{
    SetRectBounds(&g_stSecondaryViewBuffer_005d2c90, a, b, c, d);
}

/* Function start: 0x450E1E */
void DrawNavHazardMarker(FixedVector navPosition, FixedVector offset,
                         short size, unsigned short markerColour,
                         unsigned short textColour, const char *text)
{
    short x;
    short y;

    AddFixedVectors(&navPosition, &offset, &navPosition);
    ScaleNavMapMarkerSize(&size);
    nav_getxy(&x, &y, navPosition.x, navPosition.z);
    DrawNavRectangleMarker(x, y, size, 0, markerColour, 1);
    PlaceNavMapLabel(x, y, textColour, text);
}

/* Function start: 0x450E98 */
void DrawNavPlayerMarker(short colour, short reserve)
{
    short x;
    short y;

    nav_getxy(&x, &y, g_aShipPosition_00494550[0].x,
              g_aShipPosition_00494550[0].z);
    x = (short)(x + g_stNavLabelTextContext_005d16f0.viewport->left);
    y = (short)(y + g_stNavLabelTextContext_005d16f0.viewport->top);
    DrawViewportPixel(g_stNavLabelTextContext_005d16f0.viewport,
                      x, y, colour);
    DrawNavSquareMarker(x, y, 0, 0, colour, reserve);
}

/* Function start: 0x450F1E */
/* Plot the briefing beacons on the nav map: a pixel, a square marker and a
 * label for each active record. */
void DrawNavMapBriefingBeacons(void)
{
    const NavMapBeacon *beacon;
    short mapX;
    short mapY;
    short worldX;
    short worldZ;

    for (beacon = (const NavMapBeacon *)g_abMissionBriefingData_005d3e90;
         beacon->active != 0; beacon++) {
        nav_getxy(&worldX, &worldZ, beacon->position.x,
                  beacon->position.z);
        ScaleNavMapCoordinates(&mapX, &mapY, worldX, worldZ);
        DrawViewportPixel(&g_stSecondaryViewBuffer_005d2c90, mapX, mapY,
                          g_ucNavObjectiveMarkerColour_0049cb78);
        DrawNavSquareMarker(mapX, mapY, 2, 0,
                            g_ucPrimaryTextColour_0049cb64, 1);
        AddUniqueObjectiveNavLabel(mapX, mapY,
                                   g_ucDefaultTextColour_0049cb7c,
                                   beacon->name, 0, -1);
    }
}

/* Function start: 0x450FE2 */
void BuildMap(short showPlayer)
{
    MissionNavPoint *navPoint;
    MissionShipRecord *missionShip;
    const NavMapObjectiveStyle *style;
    unsigned short labelColour;
    short navIndex;
    short objectiveIndex;
    short slot;
    short x;
    short y;

    SetScreenClipRect(1, 1, 153, 138);
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 1, 1,
                      g_pNavMapShape_0049bc48, 0);
    SetScreenClipRect(2, 2, 152, 137);
    g_stNavLabelTextContext_005d16f0.viewport =
        &g_stSecondaryViewBuffer_005d2c90;
    g_stNavLabelTextContext_005d16f0.text = g_szDefaultTextBuffer_005d2b80;
    InitializeTextContextFromFont(&g_stNavLabelTextContext_005d16f0,
                                  2, g_ucPrimaryTextColour_0049cb64, -1);
    g_stNavLabelTextContext_005d16f0.alignment = 0;
    SetTextContext(&g_stNavLabelTextContext_005d16f0);
    navPoint = g_aMissionNavPoints_00491e98;
    ResetNavMapLabels();
    ResetNavMapReservedAreas();
    SetScale();
    DrawNavMapBriefingBeacons();

    for (navIndex = 0; navIndex < 10; navIndex++) {
        if (g_aMissionNavPoints_00491e98[navIndex].type != 0 &&
            g_aMissionNavPoints_00491e98[navIndex].systemIndex ==
                g_stElapsedCampaignDate_005d170c.year) {
            for (slot = 0; slot < 10; slot++) {
                if (navPoint->missionShips[slot] != -1) {
                    missionShip = &g_aMissionShips_00492290[
                        navPoint->missionShips[slot]];
                    /* WC2 hazard field types; the image names neither. */
                    switch (missionShip->objectType) {
                    case 5:
                        DrawNavHazardMarker(
                            navPoint->position, missionShip->position,
                            missionShip->speed,
                            g_ucAsteroidNavMarkerColour_0049cb84,
                            g_ucAsteroidNavMarkerColour_0049cb84,
                            g_szNavAsteroids_0049bd3c);
                        break;
                    case 6:
                        DrawNavHazardMarker(
                            navPoint->position, missionShip->position,
                            missionShip->speed,
                            g_abGamePaletteReservedColours_0049cb54[8],
                            g_abGamePaletteReservedColours_0049cb54[8],
                            g_szNavMines_0049bd48);
                        break;
                    }
                }
            }
        }
        navPoint++;
    }

    for (objectiveIndex = 0;
         g_cMissionObjectiveCount_00493294 > objectiveIndex;
         objectiveIndex++) {
        if (mobile_objective(objectiveIndex) == 0 ||
            (g_aMissionShips_00492290[
                 (signed char)g_aMissionObjectives_004932a8[
                     objectiveIndex].index].state == 0 &&
             achieved(objectiveIndex) == 0)) {
        ScaleNavMapCoordinates(
            &x, &y, g_aMissionObjectives_004932a8[objectiveIndex].mapX,
            g_aMissionObjectives_004932a8[objectiveIndex].mapY);
        if (hidden_objective(objectiveIndex) == 0 &&
            IsObjectiveInDisplayedSystem(objectiveIndex) != 0) {
        style = &g_aNavMapObjectiveStyles_0049bb88[
            g_aMissionObjectives_004932a8[objectiveIndex].type];
        if (visited(objectiveIndex) == 0)
            DrawViewportPixel(&g_stSecondaryViewBuffer_005d2c90, x, y,
                              *style->unvisitedColour);
        switch (style->markerType) {
        case 0:
            break;
        case 1:
            DrawNavSquareMarker(x, y, style->markerSize, 0,
                                *style->markerColour, 1);
            break;
        case 2:
            DrawNavRectangleMarker(x, y, style->markerSize, 0,
                                   *style->markerColour, 1);
            break;
        case 3:
            DrawNavTriangleMarker(x, y, style->markerSize, 0,
                                  *style->markerColour, 1);
            break;
        case 4:
            DrawNavCrossMarker(x, y, style->markerSize, 0,
                               *style->markerColour, 1);
            break;
        }
        if (g_cCurrentObjective_004931cc == objectiveIndex)
            labelColour = g_abGamePaletteReservedColours_0049cb54[4];
        else
            labelColour = *style->labelColour;
        g_awNavObjectiveLabelIndex_005d16b0[objectiveIndex] =
            g_nNavMapLabelCount_0049bc4c;
        AddUniqueObjectiveNavLabel(
            x, y, labelColour, objective_name(objectiveIndex),
            objectiveIndex,
            (short)g_aMissionObjectives_004932a8[objectiveIndex].index);
        }
        }
    }

    if (showPlayer != 0 &&
        g_stElapsedCampaignDate_005d170c.year ==
            g_nCurrentStarSystem_005d169c) {
        DrawNavPlayerMarker(g_bPrimaryViewBufferColour_0049cb50, 1);
        nav_getxy(&x, &y, g_aShipPosition_00494550[0].x,
                  g_aShipPosition_00494550[0].z);
        PlaceNavMapLabel(
            x, y, g_ucPlayerNavLabelColour_0049cb74,
            g_stCurrentPilotProfile_00493408.callsign);
    }
    DrawNavMapLabels();
    SetScreenClipRect(0, 0, 259, 155);
}

/* Function start: 0x45153A */
void UpdateInflightNavText(short showColon)
{
    char time[12];
    short cursorX;

    SetTextContext(&g_stNavMapTextContext_005d16d0);
    if (g_pElapsedCampaignDate_005d3e8c == 0)
        g_pElapsedCampaignDate_005d3e8c =
            &g_stElapsedCampaignDate_005d170c;
    sprintf(time, g_szInflightTimeFieldFormat_0049bd50,
            (int)((signed char *)g_pElapsedCampaignDate_005d3e8c)[0]);
    DrawFormattedText(
        g_szStandardTimeFormat_0049bd58,
        (int)(short)(g_stNavMapTextContext_005d16d0.viewport->left + 152),
        (int)(short)(g_stNavMapTextContext_005d16d0.viewport->top + 140),
        time);
    cursorX = g_stNavMapTextContext_005d16d0.cursorX;
    if (showColon == 0) {
        DrawFormattedText(g_szStandardTimeBlank_0049bd70);
    } else {
        g_stNavMapTextContext_005d16d0.cursorX =
            (short)(g_stNavMapTextContext_005d16d0.cursorX + 4);
    }
    sprintf(time, g_szInflightTimeSuffixFormat_0049bd74,
            (int)((signed char *)g_pElapsedCampaignDate_005d3e8c)[1]);
    DrawFormattedText(time);
    g_stNavMapTextContext_005d16d0.cursorX = cursorX;
    if (showColon != 0)
        DrawFormattedText(g_szStandardTimeColon_0049bd7c);
}

/* Function start: 0x451637 */
void FormatNavCoordinates(unsigned char *out)
{
    unsigned char tmp[4];

    if (out == 0)
        out = (unsigned char *)&g_stElapsedCampaignDate_005d170c;
    SplitGameClockTicks(tmp);
    out[1] = tmp[2];
    out[0] = tmp[3];
}

/* Function start: 0x451673 */
void DrawSelectedNavLegendEntry(const char *text, short navPoint)
{
    if ((short)g_cCurrentNavPointIndex_00493298 == navPoint) {
        DrawNavTextLine(0, (unsigned short)g_abGamePaletteReservedColours_0049cb54[4], text);
        DrawNavTextLine(0, (unsigned short)g_abGamePaletteReservedColours_0049cb54[4],
                        g_szNavLegendNewline_0049bd80);
    }
}

/* Function start: 0x4516C0 */
void DrawNavMapLegend(void)
{
    short objective;

    objective = 0;
    while (objective < (short)g_cMissionObjectiveCount_00493294) {
        if (visited(objective) == 0 && hidden_objective(objective) == 0)
            break;
        objective++;
    }
    SetTextCursor((unsigned short)g_stSecondaryViewBuffer_005d2c90.left, 120);
    DrawSelectedNavLegendEntry(g_szNavMissionFlightPath_0049bd84,
                               objective);
    DrawSelectedNavLegendEntry(
        g_szNavHomeBase_0049bd98,
        (short)((short)g_cMissionObjectiveCount_00493294 - 1));
}

/* Function start: 0x451756 */
char *nav_note(short objective)
{
    if (IsObjectiveInDisplayedSystem(objective) == 0)
        return g_szNoFurtherObjectives_0049bc90;
    if (*g_aMissionObjectives_004932a8[objective].name == '?')
        return g_aMissionObjectives_004932a8[objective].name + 1;
    if (*g_aMissionObjectives_004932a8[objective].name == '.')
        return strchr(g_aMissionObjectives_004932a8[objective].name, 0);
    return g_aMissionObjectives_004932a8[objective].name;
}

/* Function start: 0x45180F */
void DrawNavLocationReadout(const char *title, short showFlightData)
{
    char sectorFile[11] = "sector.000";
    short file;
    short length;

    ClearViewport(&g_stSecondaryViewBuffer_005d2c90, g_cSecondaryViewBufferColour_0049cb4c);
    SetScreenClipRect(155, 2, 259, 155);
    g_stNavMapTextContext_005d16d0.viewport = &g_stSecondaryViewBuffer_005d2c90;
    g_stNavMapTextContext_005d16d0.text = g_szDefaultTextBuffer_005d2b80;
    InitializeTextContextFromFont(&g_stNavMapTextContext_005d16d0,
                                  1, g_ucPrimaryTextColour_0049cb64, g_cSecondaryViewBufferColour_0049cb4c);
    g_stNavMapTextContext_005d16d0.alignment = 0;
    g_stNavMapTextContext_005d16d0.cursorX = 0;
    g_stNavMapTextContext_005d16d0.cursorY = 0;
    SetTextContext(&g_stNavMapTextContext_005d16d0);
    DrawNavTextLine(0, g_ucDefaultTextColour_0049cb7c,
                    g_szNavBlankLine_0049bdb0);
    DrawNavTextLine(2, g_ucDefaultTextColour_0049cb7c,
                    g_szNavTitleFormat_0049bdb4, title);
    if (g_pNavLocationText_0049bc54 == 0) {
        sectorFile[9] =
            (char)(g_nSelectedCampaignSlot_005d3bf2 + '0');
        file = OpenDataFileOrDie(sectorFile);
        length = (short)_filelength(file);
        g_pNavLocationText_0049bc54 =
            AllocateTaggedMemory(length + 1, 0x40);
        ReadDataFileAtOffset((unsigned short)file, 0, length,
                             g_pNavLocationText_0049bc54);
        g_pNavLocationText_0049bc54[length] = 0;
    }
    DrawNavTextLine(0, g_ucDefaultTextColour_0049cb7c,
                    g_szNavSectorFormat_0049bdbc,
                    (char *)g_pNavLocationText_0049bc54);
    DrawNavTextLine(0, g_ucDefaultTextColour_0049cb7c,
                    g_szNavSystemFormat_0049bdc8,
                    g_abSeriesAuxData_005d3c40);
    DrawNavTextLine(2, g_ucDefaultTextColour_0049cb7c,
                    g_szNavMissionFormat_0049bdd8,
                    g_abMissionAuxData_005d3c10);
    DrawNavTextLine(2, g_ucDefaultTextColour_0049cb7c,
                    g_szNavShipFormat_0049bde0,
                    g_apszShipMissionTypeNames_0049bc58[
                        g_aMissionShips_00492290[
                            g_stMissionHeader_005d3e70
                                .playerMissionShip].missionType]);
    DrawNavTextLine(2, g_ucDefaultTextColour_0049cb7c,
                    g_szNavNotesHeading_0049bde8);
    DrawNavTextLine(0, g_ucDefaultTextColour_0049cb7c,
                    g_szNavNoteFormat_0049bdf0,
                    nav_note((short)g_cCurrentObjective_004931cc));
    if (showFlightData != 0)
        DrawNavMapLegend();
    BuildMap(showFlightData);
    if (showFlightData != 0) {
        SetScreenClipRect(0, 0, 259, 155);
        SetTextContext(&g_stNavMapTextContext_005d16d0);
        DrawNavTextLine(0, g_ucDefaultTextColour_0049cb7c,
                        g_szNavLocationFormat_0049bdf4,
                        8, 142,
                        g_aShipPosition_00494550[0].x >> 8,
                        g_aShipPosition_00494550[0].y >> 8,
                        g_aShipPosition_00494550[0].z >> 8);
    }
    CopyViewportContents(&g_stSecondaryViewBuffer_005d2c90, &g_stScreenViewport_005d21a0);
}

/* Function start: 0x451AD3 */
void BriefingMap_LoadShapes(void)
{
    short objective;

    g_pNavMapShape_0049bc48 =
        FetchDiskPacketRetrying("cockpit.vga", 2, 0);
    SetScreenClipRect(0, 0, 259, 155);
    if (AllocateViewport(&g_stSecondaryViewBuffer_005d2c90, (short)g_cSecondaryViewBufferColour_0049cb4c, 0) == 0)
        ReportOutOfMemoryAndExit(g_szNavViewportName_0049be30);
    objective = 0;
    while (objective < (short)g_cMissionObjectiveCount_00493294) {
        LocateMobileObjective(objective);
        objective++;
    }
}

/* Function start: 0x451B70 */
void BriefingMap_DisplayMap(void)
{
    Viewport savedViewport;
    short font;

    savedViewport = g_stSecondaryViewBuffer_005d2c90;
    free_viewport(&g_stSecondaryViewBuffer_005d2c90);
    BriefingMap_LoadShapes();
    g_stScreenViewport_005d21a0.top = 4;
    DrawNavLocationReadout(g_szBriefingNavMapTitle_0049be38, 0);
    free_viewport(&g_stSecondaryViewBuffer_005d2c90);
    ReleasePacketHandle(g_pNavMapShape_0049bc48);
    ReleasePacketSlot((void **)&g_pNavLocationText_0049bc54);
    for (font = 0; font < 4; font++) {
        if (g_apTextFonts_005d2200[font] != 0) {
            ReleasePacketHandle(g_apTextFonts_005d2200[font]);
            g_apTextFonts_005d2200[font] = 0;
        }
    }
    g_stSecondaryViewBuffer_005d2c90 = savedViewport;
    AllocateViewport(&g_stSecondaryViewBuffer_005d2c90, (short)g_cSecondaryViewBufferColour_0049cb4c, 0);
}

#pragma function(abs)

/* Function start: 0x451C5C */
short SelectNavObjectiveAtPoint(short mouseX, short mouseY)
{
    short mapX;
    short mapY;
    short oldNavPoint;
    short selected;
    short pathIndex;
    short distance;

    selected = 0;
    oldNavPoint = (short)g_cCurrentNavPointIndex_00493298;
    mouseX = (short)(mouseX - 30);
    mouseY = (short)(mouseY - 22);
    pathIndex = 0;
    while (g_abFlightPath_004932a0[pathIndex] != -1 && pathIndex < 8) {
        if (hidden_objective(g_abFlightPath_004932a0[pathIndex]) == 0 &&
            IsObjectiveInDisplayedSystem(
                g_abFlightPath_004932a0[pathIndex]) != 0) {
            ScaleNavMapCoordinates(
                &mapX, &mapY,
                g_aMissionObjectives_004932a8[
                    g_abFlightPath_004932a0[pathIndex]].mapX,
                g_aMissionObjectives_004932a8[
                    g_abFlightPath_004932a0[pathIndex]].mapY);
            distance = (short)(abs((int)mouseY - mapY) +
                               abs((int)mouseX - mapX));
            if (distance < 6 ||
                IsPointInNavMapLabel(
                    (short)g_awNavObjectiveLabelIndex_005d16b0[pathIndex],
                    mouseX, mouseY) != 0) {
                selected = 1;
                set_new_objective(pathIndex);
                if (oldNavPoint == pathIndex)
                    return selected;
            }
        }
        pathIndex++;
    }
    return selected;
}

/* Function start: 0x451DDB */
void CentreMouseOnCurrentNavObjective(void)
{
    short x;
    short y;

    ScaleNavMapCoordinates(
        &x, &y,
        g_aMissionObjectives_004932a8[
            g_abFlightPath_004932a0[
                g_cCurrentNavPointIndex_00493298]].mapX,
        g_aMissionObjectives_004932a8[
            g_abFlightPath_004932a0[
                g_cCurrentNavPointIndex_00493298]].mapY);
    x = (short)(x + 30);
    y = (short)(y + 22);
    SetPersonnelMousePosition(x, y);
}

/* Function start: 0x451E57 */
void ShowConfedNavScan(short refresh)
{
    SetRectBounds(&g_stScreenViewport_005d21a0, 30, 22, 289, 177);
    if (refresh != 0)
        DisableMouseCursorDrawing();
    DrawNavLocationReadout(g_szConfedNavScan_0049be4c, 1);
    if (refresh != 0)
        EnableMouseCursorDrawing();
    SetRectBounds(&g_stScreenViewport_005d21a0, 0, 0, 319, 199);
}

/* Function start: 0x451EC9 */
/* Follow the selected objective's system, unless the map is still showing a
 * different system from the one the player is in. */
void FollowSelectedObjectiveSystem(void)
{
    short navPoint;

    if (g_stElapsedCampaignDate_005d170c.year != g_nCurrentStarSystem_005d169c) {
        g_stElapsedCampaignDate_005d170c.year = g_nCurrentStarSystem_005d169c;
        return;
    }
    switch (g_aMissionObjectives_004932a8[
                g_cCurrentObjective_004931cc].type) {
    case 0:
    case 5:
        g_stElapsedCampaignDate_005d170c.year =
            g_aMissionNavPoints_00491e98[
                g_aMissionObjectives_004932a8[
                    g_cCurrentObjective_004931cc].index].systemIndex;
        return;
    default:
        navPoint = find_ships_sphere(
            g_aMissionObjectives_004932a8[
                g_cCurrentObjective_004931cc].index);
        g_stElapsedCampaignDate_005d170c.year =
            g_aMissionNavPoints_00491e98[navPoint].systemIndex;
        return;
    }
}

/* Function start: 0x451FAA */
void InflightComputer(void)
{
    MouseCursorState savedCursor;
    Viewport pointerViewport;
    InputEventState event;
    unsigned char *background;
    int eventType;
    int frame;
    int markerColour;
    short done;
    short displayedNavPoint;
    short hasObjectives;
    short savedNavPoint;
    short objective;

    done = 0;
    displayedNavPoint = (short)g_cCurrentNavPointIndex_00493298;
    hasObjectives = 0;
    savedNavPoint = (short)g_cCurrentNavPointIndex_00493298;
    g_bInflightComputerActive_0049bcb4 = 1;
    savedCursor = g_stMouseCursorState_0059ab10;

    if (message_showing() != 0)
        EndCommMenu();
    DisableMouseCursorDrawing();
    g_nUiInputMode_005c8d3c = 0;
    free_view_buffer();
    g_cScreenViewportMode_005c82a6 = -1;
    background = FetchDiskPacketRetrying("cockpit.vga", 1, 0);
    ClearViewport(&g_stScreenViewport_005d21a0,
                  g_cSecondaryViewBufferColour_0049cb4c);
    DrawSpriteDefault(&g_stScreenViewport_005d21a0, 0, 0, background, 0);
    ReleasePacketHandle(background);
    SetInputViewport(&g_stScreenViewport_005d21a0);
    g_stElapsedCampaignDate_005d170c.year = g_nCurrentStarSystem_005d169c;
    BriefingMap_LoadShapes();
    ShowConfedNavScan(0);

    for (objective = 0;
         objective < (short)g_cMissionObjectiveCount_00493294;
         objective++) {
        if (hidden_objective(objective) == 0 &&
            IsObjectiveInDisplayedSystem(objective) != 0) {
            hasObjectives = 1;
            break;
        }
    }
    MarkDibDirty();
    DIBslamReal();

    if (hasObjectives == 0) {
        ConfigureInputPump(1, PollJoystickButtonEvents);
        WaitForAnyInputPress();
        SetFrameTimerAndWait(20);
        ConfigureInputPump(1, get_player_input);
    } else {
        pointerViewport = g_stScreenViewport_005d21a0;
        SetRectBounds(&pointerViewport, 0x20, 0x18, 0xb6, 0x9f);
        SetInputViewport(&pointerViewport);
        SetMenuInputPump();
        g_nInputRepeatDelay_005c80d6 = 10;
        g_bInputCursorEnabled_005c80e6 = 6;
        CentreMouseOnCurrentNavObjective();

        do {
            if ((short)g_cCurrentNavPointIndex_00493298 !=
                displayedNavPoint) {
                displayedNavPoint =
                    (short)g_cCurrentNavPointIndex_00493298;
                PlaySfxWaveFileByNumber(0x19, -1, 0);
                ShowConfedNavScan(1);
            }
            SetRectBounds(&g_stScreenViewport_005d21a0, 0x20, 0x18,
                          0x121, 0xb1);
            if (g_pElapsedCampaignDate_005d3e8c == 0)
                g_pElapsedCampaignDate_005d3e8c =
                    &g_stElapsedCampaignDate_005d170c;
            FormatNavCoordinates(
                (unsigned char *)g_pElapsedCampaignDate_005d3e8c);
            frame = g_nInputClock_005c84a8 / 15;
            g_stNavLabelTextContext_005d16f0.viewport =
                &g_stScreenViewport_005d21a0;
            if (frame % 2 != 0)
                markerColour = g_bPrimaryViewBufferColour_0049cb50;
            else
                markerColour = g_ucHomeCarrierScannerColour_0049cb70;
            DrawNavPlayerMarker((short)markerColour, 0);
            g_stNavMapTextContext_005d16d0.viewport =
                &g_stScreenViewport_005d21a0;
            UpdateInflightNavText((short)((frame / 4) % 2));
            SetRectBounds(&g_stScreenViewport_005d21a0, 0, 0, 0x13f, 0xc7);
            SetMouseCursorShape(
                g_pInputManagerState_005c8464->cursorShape, 0);
            g_nUiInputMode_005c8d3c = 1;
            EnableMouseCursorDrawing();
            /* The original passes a second argument the callee ignores. */
            eventType = ((short (__cdecl *)(InputEventState *, int))
                             PollInputEvent)(&event, 0xf);
            switch (eventType) {
            case 1:
                done = 1;
                break;
            case 4:
            case 6:
                if ((short)event.value == 0x1c ||
                    (short)event.value == 0x39) {
                    done = 1;
                } else if ((short)event.value == 0x31) {
                    cycle_next_objective();
                    if (IsObjectiveInDisplayedSystem(
                            (short)g_cCurrentObjective_004931cc) == 0)
                        FollowSelectedObjectiveSystem();
                    CentreMouseOnCurrentNavObjective();
                }
                break;
            }
            SelectNavObjectiveAtPoint(
                (short)g_stHostMouseMessage_005d10d0.x,
                (short)g_stHostMouseMessage_005d10d0.y);
            MarkDibDirty();
            DIBslamReal();
        } while (done == 0 && g_bSceneEscapeRequested_0049d4b0 == 0);

        if (g_bSceneEscapeRequested_0049d4b0 != 0) {
            g_bSceneEscapeRequested_0049d4b0 = 0;
            g_cCurrentNavPointIndex_00493298 =
                (signed char)savedNavPoint;
            set_new_objective((short)g_cCurrentNavPointIndex_00493298);
        }
        free_viewport(&g_stSecondaryViewBuffer_005d2c90);
        DisableMouseCursorDrawing();
        ReleasePacketHandle(g_pNavMapShape_0049bc48);
        SetTextContext(&g_stDefaultTextContext_005d2d20);
        PlaySfxWaveFileByNumber(0x19, -1, 0);
    }

    g_stMouseCursorState_0059ab10 = savedCursor;
    SetPersonnelMousePosition(g_nPersonnelCursorX_005c8470,
                              g_nPersonnelCursorY_005c8472);
    force_view(0, 0);
    InitializeSpaceFlightInput();
    g_bInflightComputerActive_0049bcb4 = 0;
}

/* Function start: 0x4207B0 */
short MergeAdjacentNearHeapBlocks(int descriptorAddress)
{
    NearHeapBlock *block;

    block = DosNearPtrToFar(descriptorAddress);
    if ((block->sizeAndFlags & 0x80000000) == 0 &&
        (block[1].sizeAndFlags & 0x80000000) == 0 &&
        block->address + (block->sizeAndFlags & 0xfffff) ==
            block[1].address) {
        block[1].address = block->address;
        block[1].sizeAndFlags += block->sizeAndFlags & 0xfffff;
        while (descriptorAddress > g_nNearHeapFirstDescriptor_005d3058) {
            descriptorAddress -= 8;
            block = DosNearPtrToFar(descriptorAddress);
            block[1].address = block->address;
            block[1].sizeAndFlags = block->sizeAndFlags;
        }
        g_nNearHeapFirstDescriptor_005d3058 += 8;
        return 1;
    }
    return 0;
}

/* Function start: 0x420874 */
int ReleaseNearHeapBlock(int descriptorAddress)
{
    NearHeapBlock *block;

    block = DosNearPtrToFar(descriptorAddress);
    /* The signed read is what keeps the mask out of the descriptor
     * word: the compiler folds the unsigned form into `and [mem]`. */
    block->sizeAndFlags = (int)block->sizeAndFlags & 0x7fffffff;
    if (g_nNearHeapSize_005d3050 + g_nNearHeapBase_005d3054 >
            descriptorAddress + 8 &&
        MergeAdjacentNearHeapBlocks(descriptorAddress) != 0)
        descriptorAddress += 8;
    if (descriptorAddress > g_nNearHeapFirstDescriptor_005d3058)
        MergeAdjacentNearHeapBlocks(descriptorAddress - 8);
    return descriptorAddress;
}

/* Function start: 0x4208FC */
void PurgeNearHeapBlocks(unsigned short flags)
{
    NearHeapBlock *block;
    int descriptorAddress;
    int descriptorBytes;

    if (g_nNearHeapActive_00493044 != 0) {
        if ((flags & 0x10) != 0) {
            descriptorAddress = g_nNearHeapBase_005d3054;
            descriptorAddress += g_nNearHeapSize_005d3050;
            descriptorAddress -= 8;
            g_nNearHeapFirstDescriptor_005d3058 = descriptorAddress;
            block = DosNearPtrToFar(descriptorAddress);
            block->address = g_nNearHeapBase_005d3054;
            descriptorBytes = g_nNearHeapMaxDescriptors_00493048 * 8;
            block->sizeAndFlags =
                g_nNearHeapSize_005d3050 - descriptorBytes;
            return;
        }
        descriptorAddress = g_nNearHeapBase_005d3054 +
                            g_nNearHeapSize_005d3050 - 8;
        for (; descriptorAddress >= g_nNearHeapFirstDescriptor_005d3058;
             descriptorAddress -= 8) {
            block = DosNearPtrToFar(descriptorAddress);
            if ((block->sizeAndFlags & 0x40000000) == 0)
                descriptorAddress = ReleaseNearHeapBlock(descriptorAddress);
        }
    }
}

/* Function start: 0x4209C2 */
unsigned short InitializeNearHeap(void)
{
    short initialSize;
    int adjustedSize;

    if (g_nNearHeapActive_00493044 == 0) {
        initialSize = (short)GetNavRangeSentinel();
        g_pNearHeapAllocation_005d305c = 0;
        g_nNearHeapSize_005d3050 = initialSize;
        if (g_nNearHeapMaxDescriptors_00493048 * 8 <
            g_nNearHeapSize_005d3050) {
            g_pNearHeapAllocation_005d305c =
                AllocateTaggedMemory(g_nNearHeapSize_005d3050, 0);
            if (g_pNearHeapAllocation_005d305c != 0) {
                g_nNearHeapActive_00493044++;
                g_nNearHeapBase_005d3054 =
                    DosFarPtrToNear(g_pNearHeapAllocation_005d305c);
                if (*(unsigned short *)0x00400013 == 0x270) {
                    g_nNearHeapRelocationBytes_0049304c =
                        0x9c000 - g_nNearHeapSize_005d3050 -
                        g_nNearHeapBase_005d3054;
                    adjustedSize =
                        0x98000 - g_nNearHeapRelocationBytes_0049304c;
                    adjustedSize -= g_nNearHeapBase_005d3054;
                    g_nNearHeapSize_005d3050 = adjustedSize;
                    if (g_nNearHeapMaxDescriptors_00493048 * 8 <
                        g_nNearHeapSize_005d3050) {
                        DosMemcpy(
                            DosNearPtrToFar(
                                g_nNearHeapBase_005d3054 +
                                g_nNearHeapSize_005d3050),
                            DosNearPtrToFar(
                                0x9c000 -
                                g_nNearHeapRelocationBytes_0049304c),
                            g_nNearHeapRelocationBytes_0049304c);
                    } else {
                        FreeIfNotNull(g_pNearHeapAllocation_005d305c);
                        g_nNearHeapActive_00493044 = 0;
                    }
                }
                if (g_nNearHeapActive_00493044 != 0)
                    PurgeNearHeapBlocks(0x10);
            }
        }
    }
    return g_nNearHeapActive_00493044;
}

/* Function start: 0x420B12 */
void ShutdownNearHeap(void)
{
    if (g_nNearHeapRelocationBytes_0049304c != 0) {
        DosMemcpy(
            DosNearPtrToFar(
                0x9c000 - g_nNearHeapRelocationBytes_0049304c),
            DosNearPtrToFar(
                g_nNearHeapSize_005d3050 +
                g_nNearHeapBase_005d3054),
            g_nNearHeapRelocationBytes_0049304c);
    }
    if (g_nNearHeapActive_00493044 != 0)
        FreeIfNotNull(g_pNearHeapAllocation_005d305c);
    g_nNearHeapActive_00493044 = 0;
}

/* Function start: 0x420B88 */
void *AllocateNearHeapBlockFromEnd(int size, unsigned short flags)
{
#if 1
    NearHeapBlock *block;
    NearHeapBlock *lastBlock;
    int descriptorAddress;
    int shiftAddress;
    int allocationAddress;
    int blockSize;
    unsigned int allocationFlags;

    if (size <= 0)
        return 0;
    allocationFlags = 0x80000000;
    if ((flags & 3) == 1) {
        allocationFlags |= 0x10000000;
        size++;
    } else if ((flags & 3) == 2) {
        allocationFlags |= 0x20000000;
        size += 0xf;
    }
    if ((flags & 0x10) != 0)
        allocationFlags |= 0x40000000;

    allocationAddress = 0;
    descriptorAddress =
        g_nNearHeapBase_005d3054 + g_nNearHeapSize_005d3050 - 8;
    for (; descriptorAddress >= g_nNearHeapFirstDescriptor_005d3058;
         descriptorAddress -= 8) {
        block = DosNearPtrToFar(descriptorAddress);
        blockSize = block->sizeAndFlags & 0xfffff;
        if ((block->sizeAndFlags & 0x80000000) != 0)
            continue;
        if ((int)blockSize < size)
            continue;
        if ((int)blockSize > size) {
            if (g_nNearHeapBase_005d3054 -
                    g_nNearHeapMaxDescriptors_00493048 * 8 +
                    g_nNearHeapSize_005d3050 >=
                g_nNearHeapFirstDescriptor_005d3058) {
                lastBlock = DosNearPtrToFar(
                    g_nNearHeapBase_005d3054 +
                    g_nNearHeapSize_005d3050 - 8);
                if ((lastBlock->sizeAndFlags & 0x80000000) == 0) {
                    blockSize = lastBlock->sizeAndFlags & 0xfffff;
                    if (lastBlock->address + blockSize ==
                            g_nNearHeapFirstDescriptor_005d3058 &&
                        (int)blockSize > 8) {
                        lastBlock->sizeAndFlags -= 8;
                        g_nNearHeapMaxDescriptors_00493048++;
                    }
                }
            }

            if (g_nNearHeapBase_005d3054 -
                    g_nNearHeapMaxDescriptors_00493048 * 8 +
                    g_nNearHeapSize_005d3050 >=
                g_nNearHeapFirstDescriptor_005d3058)
                continue;

            g_nNearHeapFirstDescriptor_005d3058 -= 8;
            shiftAddress = g_nNearHeapFirstDescriptor_005d3058;
            for (; descriptorAddress > shiftAddress; shiftAddress += 8) {
                block = DosNearPtrToFar(shiftAddress);
                block->address = block[1].address;
                block->sizeAndFlags = block[1].sizeAndFlags;
            }
            descriptorAddress -= 8;
            block = DosNearPtrToFar(descriptorAddress);
            blockSize = block->sizeAndFlags - size;
            block->sizeAndFlags = blockSize;
            block[1].address += blockSize & 0xffffff;
            blockSize = allocationFlags + size;
            block[1].sizeAndFlags = blockSize;
            allocationAddress = block[1].address;
            break;
        }
        block->sizeAndFlags |= allocationFlags;
        allocationAddress = block->address;
        break;
    }

    if (allocationAddress != 0) {
        if ((flags & 3) == 1)
            allocationAddress = (allocationAddress + 1) & 0xfffffffe;
        else if ((flags & 3) == 2)
            allocationAddress = (allocationAddress + 0xf) & 0xfffffff0;
    }
    return DosNearPtrToFar(allocationAddress);
#else
    NearHeapBlock *block;
    NearHeapBlock *lastBlock;
    int descriptorAddress;
    int shiftAddress;
    unsigned int allocationAddress;
    unsigned int allocationFlags;

    if (size <= 0)
        return 0;
    allocationFlags = 0x80000000;
    if ((flags & 3) == 1) {
        allocationFlags = 0x90000000;
        size++;
    } else if ((flags & 3) == 2) {
        allocationFlags = 0xa0000000;
        size += 0xf;
    }
    if ((flags & 0x10) != 0)
        allocationFlags |= 0x40000000;

    allocationAddress = 0;
    descriptorAddress = g_nNearHeapBase_005d3054 +
                        g_nNearHeapSize_005d3050;
    do {
        descriptorAddress -= 8;
        if (descriptorAddress < g_nNearHeapFirstDescriptor_005d3058)
            goto allocation_complete;
        block = DosNearPtrToFar(descriptorAddress);
        if ((block->sizeAndFlags & 0x80000000) != 0 ||
            (int)(block->sizeAndFlags & 0xfffff) < size)
            continue;
        if ((int)(block->sizeAndFlags & 0xfffff) <= size) {
            block->sizeAndFlags |= allocationFlags;
            allocationAddress = block->address;
            goto allocation_complete;
        }
        if (g_nNearHeapFirstDescriptor_005d3058 <=
                g_nNearHeapBase_005d3054 + g_nNearHeapSize_005d3050 -
                    g_nNearHeapMaxDescriptors_00493048 * 8) {
            lastBlock = DosNearPtrToFar(
                g_nNearHeapBase_005d3054 + g_nNearHeapSize_005d3050 - 8);
            if ((lastBlock->sizeAndFlags & 0x80000000) == 0 &&
                (lastBlock->sizeAndFlags & 0xfffff) + lastBlock->address ==
                    g_nNearHeapFirstDescriptor_005d3058 &&
                (lastBlock->sizeAndFlags & 0xfffff) > 8) {
                lastBlock->sizeAndFlags -= 8;
                g_nNearHeapMaxDescriptors_00493048++;
            }
        }
    } while (g_nNearHeapFirstDescriptor_005d3058 <=
             g_nNearHeapBase_005d3054 + g_nNearHeapSize_005d3050 -
                 g_nNearHeapMaxDescriptors_00493048 * 8);

    g_nNearHeapFirstDescriptor_005d3058 -= 8;
    for (shiftAddress = g_nNearHeapFirstDescriptor_005d3058;
         shiftAddress < descriptorAddress; shiftAddress += 8) {
        block = DosNearPtrToFar(shiftAddress);
        block->address = block[1].address;
        block->sizeAndFlags = block[1].sizeAndFlags;
    }
    block = DosNearPtrToFar(descriptorAddress - 8);
    block->sizeAndFlags -= size;
    block[1].address += block->sizeAndFlags & 0xffffff;
    block[1].sizeAndFlags = allocationFlags + size;
    allocationAddress = block[1].address;

allocation_complete:
    if (allocationAddress != 0) {
        if ((flags & 3) == 1)
            allocationAddress = (allocationAddress + 1) & 0xfffffffe;
        else if ((flags & 3) == 2)
            allocationAddress = (allocationAddress + 0xf) & 0xfffffff0;
    }
    return DosNearPtrToFar(allocationAddress);
#endif
}

/* Function start: 0x420E16 */
void *AllocateNearHeapBlockByFlags(int size, unsigned short flags)
{
    NearHeapBlock *block;
    NearHeapBlock *lastBlock;
    int descriptorAddress;
    int shiftAddress;
    int allocationAddress;
    unsigned int blockSize;
    unsigned int allocationFlags;
    unsigned short alignment;

    if ((flags & 0x20) != 0)
        return AllocateNearHeapBlockFromEnd(size, flags);
    if (size <= 0)
        return 0;
    allocationFlags = 0x80000000;
    alignment = flags & 3;
    if (alignment == 1) {
        allocationFlags = 0x90000000;
        size++;
    } else if (alignment == 2) {
        allocationFlags = 0xa0000000;
        size += 0xf;
    }
    if ((flags & 0x10) != 0)
        allocationFlags |= 0x40000000;

    allocationAddress = 0;
    descriptorAddress = g_nNearHeapFirstDescriptor_005d3058;
    for (; descriptorAddress <
               g_nNearHeapBase_005d3054 + g_nNearHeapSize_005d3050;
         descriptorAddress += 8) {
        block = DosNearPtrToFar(descriptorAddress);
        blockSize = block->sizeAndFlags & 0xfffff;
        if ((block->sizeAndFlags & 0x80000000) != 0)
            continue;
        if ((int)blockSize < size)
            continue;
        if ((int)blockSize > size) {
            if (g_nNearHeapBase_005d3054 -
                    g_nNearHeapMaxDescriptors_00493048 * 8 +
                    g_nNearHeapSize_005d3050 >=
                g_nNearHeapFirstDescriptor_005d3058) {
                lastBlock = DosNearPtrToFar(
                    g_nNearHeapBase_005d3054 +
                    g_nNearHeapSize_005d3050 - 8);
                if ((lastBlock->sizeAndFlags & 0x80000000) == 0) {
                    blockSize = lastBlock->sizeAndFlags & 0xfffff;
                    if (lastBlock->address + blockSize ==
                            g_nNearHeapFirstDescriptor_005d3058 &&
                        (int)blockSize > 8) {
                        lastBlock->sizeAndFlags -= 8;
                        g_nNearHeapMaxDescriptors_00493048++;
                    }
                }
            }

            if (g_nNearHeapBase_005d3054 -
                    g_nNearHeapMaxDescriptors_00493048 * 8 +
                    g_nNearHeapSize_005d3050 >=
                g_nNearHeapFirstDescriptor_005d3058)
                continue;

            g_nNearHeapFirstDescriptor_005d3058 -= 8;
            shiftAddress = g_nNearHeapFirstDescriptor_005d3058;
            for (; descriptorAddress > shiftAddress; shiftAddress += 8) {
                block = DosNearPtrToFar(shiftAddress);
                block->address = block[1].address;
                block->sizeAndFlags = block[1].sizeAndFlags;
            }
            descriptorAddress -= 8;
            block = DosNearPtrToFar(descriptorAddress);
            block->sizeAndFlags = allocationFlags + size;
            block[1].address += size;
            block[1].sizeAndFlags -= size;
            allocationAddress = block->address;
            break;
        }
        block->sizeAndFlags |= allocationFlags;
        allocationAddress = block->address;
        break;
    }

    if (allocationAddress != 0) {
        if (alignment == 1)
            allocationAddress = (allocationAddress + 1) & 0xfffffffe;
        else if (alignment == 2)
            allocationAddress = (allocationAddress + 0xf) & 0xfffffff0;
    }
    return DosNearPtrToFar(allocationAddress);
}

/* Function start: 0x42BDFE */
void WriteMemoryStateReportHook(void)
{
}

/* Function start: 0x42BE09 */
void LogMemoryStateToFile(FILE *file)
{
    FILE *previousFile;
    unsigned char previousFileMode;

    previousFile = g_pMemoryLogFile_00499da8;
    previousFileMode = g_bMemoryLogToFile_00499bf8;
    g_bMemoryLogToFile_00499bf8 = 1;
    g_pMemoryLogFile_00499da8 = file;
    WriteMemoryStateReportHook();
    g_pMemoryLogFile_00499da8 = previousFile;
    g_bMemoryLogToFile_00499bf8 = previousFileMode;
}

/* Function start: 0x42BE4B */
void WriteDetailedMemoryStateReport(void)
{
    unsigned int availableMainMemory;
    unsigned int largestMainMemoryBlock;
    unsigned int availableFarMemory;
    unsigned int largestFarMemoryBlock;
    unsigned short originalFreeMemory;

    WriteMemoryStateReportHook();
    if (g_pMemoryLogFile_00499da8 != 0) {
        availableMainMemory = GetAvailableMainMemory();
        largestMainMemoryBlock = GetLargestMainMemoryBlock();
        availableFarMemory = GetAvailableFarMemory();
        largestFarMemoryBlock = GetLargestFreeMemoryBlock();
        originalFreeMemory = GetOriginalFreeMemory();
        fprintf(g_pMemoryLogFile_00499da8,
                "----- Mem State ------\n");
        fprintf(g_pMemoryLogFile_00499da8,
                "NEAR: %ud\n", originalFreeMemory);
        fprintf(g_pMemoryLogFile_00499da8,
                "FAR:  %6ld  Largest: %6ld\n",
                availableMainMemory, largestMainMemoryBlock);
        fprintf(g_pMemoryLogFile_00499da8,
                "EMS:  %6ld  Largest: %6ld\n",
                availableFarMemory, largestFarMemoryBlock);
        fprintf(g_pMemoryLogFile_00499da8,
                "----------------------\n");
    }
}

/* Function start: WC2_UNMAPPED */
int FullMissionScore(void)
{
    signed char *scores;
    short objective;
    short score;

    score = 0;
    scores = (signed char *)(g_pMissionCampaignData_005988bc +
        (int)g_stCampaignState_0059ca50.currentMission * 0x14 +
        (int)g_stCampaignState_0059ca50.currentSeries * 0x5a - 0x50);
    objective = 0;
    do {
        score = (short)(score + scores[objective + 4]);
        objective++;
    } while (objective < 16);
    return score;
}

/* Function start: WC2_UNMAPPED */
int PlayersMissionScore(void)
{
    signed char *scores;
    short objective;
    short score;

    scores = (signed char *)(g_pMissionCampaignData_005988bc +
        (int)g_stCampaignState_0059ca50.currentSeries * 0x5a +
        (int)g_stCampaignState_0059ca50.currentMission * 0x14 - 0x50);
    score = 0;
    for (objective = 0; objective < 16; objective++) {
        if (achieved(objective) != 0)
            score = (short)(score + scores[objective + 4]);
    }
    return score;
}

/* Function start: 0x4651B7 */
short GetViewportIntersection(ShortRect *intersection,
                              const ShortRect *first,
                              const ShortRect *second)
{
    int pointCount;
    int pointIndex;
    int xCoordinates[16];
    int yCoordinates[16];
    int inside[4];
    int x;
    int insideCount;
    int y;

    pointCount = 0;
    xCoordinates[pointCount] = first->left;
    yCoordinates[pointCount] = first->top;
    pointCount++;
    xCoordinates[pointCount] = first->left;
    yCoordinates[pointCount] = first->bottom;
    pointCount++;
    xCoordinates[pointCount] = first->right;
    yCoordinates[pointCount] = first->top;
    pointCount++;
    xCoordinates[pointCount] = first->right;
    yCoordinates[pointCount] = first->bottom;
    pointCount++;
    xCoordinates[pointCount] = second->left;
    yCoordinates[pointCount] = second->top;
    pointCount++;
    xCoordinates[pointCount] = second->left;
    yCoordinates[pointCount] = second->bottom;
    pointCount++;
    xCoordinates[pointCount] = second->right;
    yCoordinates[pointCount] = second->top;
    pointCount++;
    xCoordinates[pointCount] = second->right;
    yCoordinates[pointCount] = second->bottom;
    pointCount++;
    xCoordinates[pointCount] = first->left;
    yCoordinates[pointCount] = second->top;
    pointCount++;
    xCoordinates[pointCount] = first->left;
    yCoordinates[pointCount] = second->bottom;
    pointCount++;
    xCoordinates[pointCount] = first->right;
    yCoordinates[pointCount] = second->top;
    pointCount++;
    xCoordinates[pointCount] = first->right;
    yCoordinates[pointCount] = second->bottom;
    pointCount++;
    xCoordinates[pointCount] = second->left;
    yCoordinates[pointCount] = first->top;
    pointCount++;
    xCoordinates[pointCount] = second->left;
    yCoordinates[pointCount] = first->bottom;
    pointCount++;
    xCoordinates[pointCount] = second->right;
    yCoordinates[pointCount] = first->top;
    pointCount++;
    xCoordinates[pointCount] = second->right;
    yCoordinates[pointCount] = first->bottom;
    pointCount++;

    insideCount = 0;
    pointIndex = 0;
    do {
        if (pointIndex++ == 0)
            break;
        x = xCoordinates[pointIndex];
        y = yCoordinates[pointIndex];
        if (first->left <= x && x <= first->right &&
            second->left <= x && x <= second->right &&
            first->top <= y && y <= first->bottom &&
            second->top <= y && y <= second->bottom) {
            if (insideCount == 4)
                return 0;
            inside[insideCount] = 1;
            insideCount++;
        } else {
            if (insideCount == 4)
                return 0;
            inside[insideCount] = 0;
            insideCount++;
        }
    } while (pointIndex < pointCount);
    if (insideCount != 4)
        return 0;

    intersection->left = MinShort(
        (short)xCoordinates[inside[0]],
        MinShort((short)xCoordinates[inside[1]],
                 (short)xCoordinates[inside[2]]));
    intersection->top = MinShort(
        (short)yCoordinates[inside[0]],
        MinShort((short)yCoordinates[inside[1]],
                 (short)yCoordinates[inside[2]]));
    intersection->left = MaxShort(
        (short)xCoordinates[inside[0]],
        MaxShort((short)xCoordinates[inside[1]],
                 (short)xCoordinates[inside[2]]));
    intersection->top = MaxShort(
        (short)yCoordinates[inside[0]],
        MaxShort((short)yCoordinates[inside[1]],
                 (short)yCoordinates[inside[2]]));
    return 1;
}

/* Function start: 0x4471B0 */
void free_viewport(Viewport *viewport)
{
    unsigned char *allocation;
    int i;

    g_nFreeViewportCalls_005d1bc0++;
    allocation = viewport->allocation;
    if (allocation == 0)
        return;

    for (i = 0; i < g_nViewportAllocationCount_005d19bc; i++) {
        if (g_apViewportAllocations_005d19c0[i] == viewport->allocation) {
            g_nViewportAllocationCount_005d19bc--;
            g_apViewportAllocations_005d19c0[i] =
                g_apViewportAllocations_005d19c0[
                    g_nViewportAllocationCount_005d19bc];
            break;
        }
    }

    if (viewport->rowOffsets != 0) {
        ReleasePacketHandle(viewport->rowOffsets);
        viewport->rowOffsets = 0;
    }
    if (g_nSpacePaletteFadeMode_004901e8 != 0x13)
        printf("free_viewport not mcga\n");
    ReleasePacketHandle(allocation);
    viewport->pixels = 0;
    viewport->allocation = 0;
    if (g_stScreenViewport_005d21a0.pixels == allocation)
        g_stScreenViewport_005d21a0.pixels = 0;
    if (g_stScreenViewport_005d21a0.allocation == allocation)
        g_stScreenViewport_005d21a0.allocation = 0;
    if (g_stSecondaryViewBuffer_005d2c90.pixels == allocation)
        g_stSecondaryViewBuffer_005d2c90.pixels = 0;
    if (g_stSecondaryViewBuffer_005d2c90.allocation == allocation)
        g_stSecondaryViewBuffer_005d2c90.allocation = 0;
}

/* Function start: 0x407E40 */
void Title_Sequence(void)
{
    int waitCount;
    short section;

    waitCount = 0;
    g_nUiInputMode_005c8d3c = 0;
    g_pTitleMusic_005d3fa4 =
        FetchDiskPacketRetrying("music.r00", 19, 0);
    g_pTitleFieldShape_00491cf8 =
        FetchDiskPacketRetrying("field.v00", 1, 0);
    for (section = 0; section < 12; section++) {
        g_apTitleSections_005d3f70[section] =
            FetchDiskPacketRetrying("title.vga",
                                    (short)(section + 1), 0);
    }
    g_pTitleFireworkShape_005c8f58 =
        g_apTitleSections_005d3f70[11];
    g_apTitleSections_005d3f70[12] =
        FetchDiskPacketRetrying("title.vga", 0, 0);
    InitializeConversationViewport();
    g_bSceneEscapeRequested_0049d4b0 = 0;
    StartMusic(g_pTitleMusic_005d3fa4);
    DrawTitleOrchestra();
    if (g_bSceneEscapeRequested_0049d4b0 != 0)
        goto cleanup;
    AnimateTitleOrchestra();
    if (g_bSceneEscapeRequested_0049d4b0 != 0)
        goto cleanup;
    PlayTitleConductorCue();
    if (g_bSceneEscapeRequested_0049d4b0 != 0)
        goto cleanup;
    PushTitleOrchestraAway();
    if (g_bSceneEscapeRequested_0049d4b0 != 0)
        goto cleanup;
    section = RevealTitleLogo();
    if (g_nMusicDriverMode_0049be8c == 2 ||
        g_nMusicDriverMode_0049be8c == 1) {
        while (g_nTitleMusicSequenceStage_0049be94 < 3 &&
               waitCount < 3) {
            if (WaitForInputKey() != 0)
                g_bSceneEscapeRequested_0049d4b0 = 1;
            if (g_bSceneEscapeRequested_0049d4b0 != 0)
                break;
            waitCount++;
        }
    }
    if (g_bSceneEscapeRequested_0049d4b0 != 0)
        goto cleanup;
    ShowVictoryScreen(section);

cleanup:
    StopMusic(0);
    FadeViewportPaletteToColour(
        &g_stModalSourceViewport_005d2c50,
        g_cSecondaryViewBufferColour_0049cb4c, 1);
    ClearViewport(&g_stModalSourceViewport_005d2c50,
                  g_cSecondaryViewBufferColour_0049cb4c);
    RestoreGamePalette();
    for (section = 0; section < 13; section++)
        ReleasePacketHandle(g_apTitleSections_005d3f70[section]);
    FreePacketAndClear(&g_pTitleFieldShape_00491cf8, 0);
    ReleasePacketHandle(g_pTitleMusic_005d3fa4);
    ResetScreenClipToFullHeight();
}

/* Function start: 0x40809D */
void DrawTitleSky(short planetY)
{
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 0, 0,
                      g_pTitleFieldShape_00491cf8, 0);
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 160, planetY,
                      g_apTitleSections_005d3f70[12], 0);
}

/* Function start: 0x4080DF */
void DrawTitleOrchestra(void)
{
    short actor;

    DrawTitleSky(24);
    for (actor = 0; actor < 10; actor++) {
        DrawSpriteDefault(
            &g_stSecondaryViewBuffer_005d2c90,
            g_aTitleActorMotion_00491ca8[actor].x,
            g_aTitleActorMotion_00491ca8[actor].y,
            g_apTitleSections_005d3f70[actor + 1], 0);
    }
    PanToScreen(&g_stSecondaryViewBuffer_005d2c90,
                &g_stScreenViewport_005d21a0);
}

/* Function start: 0x40815F */
void AnimateTitleOrchestra(void)
{
    int repeat;
    short frame;
    short actor;
    short minimumFrame;
    short totalFrames;

    minimumFrame = 0;
    totalFrames = 0;
    repeat = 0;
    g_nFrameSkipCountdown_0049d760 = 1;
    do {
        for (frame = minimumFrame; frame < 32; frame++) {
            g_nFrameSkipCountdown_0049d760--;
            if (g_nFrameSkipCountdown_0049d760 < 1) {
                g_nFrameSkipCountdown_0049d760 = g_nFrameSkip_0049d764;
                DrawTitleSky(24);
                for (actor = 0; actor < 10; actor++) {
                    DrawSpriteDefault(
                        &g_stSecondaryViewBuffer_005d2c90,
                        g_aTitleActorMotion_00491ca8[actor].x,
                        g_aTitleActorMotion_00491ca8[actor].y,
                        g_apTitleSections_005d3f70[actor + 1],
                        (short)(g_apszTitleActorFrames_00491c80[actor]
                                    [frame] - 'a'));
                }
                RefreshMemoryStatusOverlay();
            }
            if (WaitForInputKey() != 0)
                g_bSceneEscapeRequested_0049d4b0 = 1;
            if (g_nTitleMusicSequenceStage_0049be94 >= 1 ||
                g_bSceneEscapeRequested_0049d4b0 != 0)
                goto done;
            totalFrames++;
        }
        minimumFrame = (short)(
            (unsigned short)RandomInRange(0, 13) + 9);
        for (frame = 31; frame >= minimumFrame; frame--) {
            g_nFrameSkipCountdown_0049d760--;
            if (g_nFrameSkipCountdown_0049d760 < 1) {
                g_nFrameSkipCountdown_0049d760 = g_nFrameSkip_0049d764;
                DrawTitleSky(24);
                for (actor = 0; actor < 10; actor++) {
                    DrawSpriteDefault(
                        &g_stSecondaryViewBuffer_005d2c90,
                        g_aTitleActorMotion_00491ca8[actor].x,
                        g_aTitleActorMotion_00491ca8[actor].y,
                        g_apTitleSections_005d3f70[actor + 1],
                        (short)(g_apszTitleActorFrames_00491c80[actor]
                                    [frame] - 'a'));
                }
                RefreshMemoryStatusOverlay();
            }
            if (WaitForInputKey() != 0)
                g_bSceneEscapeRequested_0049d4b0 = 1;
            if (g_nTitleMusicSequenceStage_0049be94 >= 1 ||
                g_bSceneEscapeRequested_0049d4b0 != 0)
                goto done;
            totalFrames++;
        }
        if ((g_nMusicDriverMode_0049be8c == 0 ||
             g_nMusicDriverMode_0049be8c == 3) &&
            totalFrames > 30)
            goto done;
        repeat++;
        if (repeat > 1)
            goto done;
    } while (1);

done:
    return;
}

/* Function start: 0x4083E6 */
void PlayTitleConductorCue(void)
{
    int waitCount;
    short cue;
    short actor;

    waitCount = 0;
    g_nFrameSkipCountdown_0049d760 = 1;
    for (cue = 0; cue < 20; cue++) {
        if (g_szTitleConductorFrames_00491c68[cue] == 'p') {
            if (g_nMusicDriverMode_0049be8c == 2)
                g_nFrameSkipCountdown_0049d760 = 1;
            else if (g_nMusicDriverMode_0049be8c == 1)
                g_nFrameSkipCountdown_0049d760 = 1;
        } else if (g_szTitleConductorFrames_00491c68[cue] == 'q') {
            if (g_nMusicDriverMode_0049be8c == 2)
                g_nFrameSkipCountdown_0049d760 = 1;
            else if (g_nMusicDriverMode_0049be8c == 1)
                g_nFrameSkipCountdown_0049d760 = 1;
        }
        g_nFrameSkipCountdown_0049d760--;
        if (g_nFrameSkipCountdown_0049d760 < 1) {
            g_nFrameSkipCountdown_0049d760 = g_nFrameSkip_0049d764;
            DrawTitleSky(24);
            for (actor = 0; actor < 9; actor++) {
                DrawSpriteDefault(
                    &g_stSecondaryViewBuffer_005d2c90,
                    g_aTitleActorMotion_00491ca8[actor].x,
                    g_aTitleActorMotion_00491ca8[actor].y,
                    g_apTitleSections_005d3f70[actor + 1],
                    (short)(g_apszTitleActorFrames_00491c80[actor][31]
                                - 'a'));
            }
            DrawSpriteDefault(
                &g_stSecondaryViewBuffer_005d2c90,
                g_aTitleActorMotion_00491ca8[9].x,
                g_aTitleActorMotion_00491ca8[9].y,
                g_apTitleSections_005d3f70[10],
                (short)(g_szTitleConductorFrames_00491c68[cue] - 'a'));
            RefreshMemoryStatusOverlay();
        }
        if (WaitForInputKey() != 0)
            g_bSceneEscapeRequested_0049d4b0 = 1;
        if (g_bSceneEscapeRequested_0049d4b0 != 0)
            goto done;
    }

    DrawTitleSky(24);
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[0].x,
        g_aTitleActorMotion_00491ca8[0].y,
        g_apTitleSections_005d3f70[1],
        (short)(g_apszTitleActorFrames_00491c80[0][31] - 'a'));
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[1].x,
        g_aTitleActorMotion_00491ca8[1].y,
        g_apTitleSections_005d3f70[2],
        (short)(g_apszTitleActorFrames_00491c80[1][31] - 'a'));
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[2].x,
        g_aTitleActorMotion_00491ca8[2].y,
        g_apTitleSections_005d3f70[3],
        (short)(g_apszTitleActorFrames_00491c80[2][31] - 'a'));
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[3].x,
        g_aTitleActorMotion_00491ca8[3].y,
        g_apTitleSections_005d3f70[4],
        (short)(g_apszTitleActorFrames_00491c80[3][31] - 'a'));
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[4].x,
        g_aTitleActorMotion_00491ca8[4].y,
        g_apTitleSections_005d3f70[5],
        (short)(g_apszTitleActorFrames_00491c80[4][31] - 'a'));
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[5].x,
        g_aTitleActorMotion_00491ca8[5].y,
        g_apTitleSections_005d3f70[6],
        (short)(g_apszTitleActorFrames_00491c80[5][31] - 'a'));
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[6].x,
        g_aTitleActorMotion_00491ca8[6].y,
        g_apTitleSections_005d3f70[7],
        (short)(g_apszTitleActorFrames_00491c80[6][31] - 'a'));
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[7].x,
        g_aTitleActorMotion_00491ca8[7].y,
        g_apTitleSections_005d3f70[8],
        (short)(g_apszTitleActorFrames_00491c80[7][31] - 'a'));
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[8].x,
        g_aTitleActorMotion_00491ca8[8].y,
        g_apTitleSections_005d3f70[9],
        (short)(g_apszTitleActorFrames_00491c80[8][31] - 'a'));
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[9].x,
        g_aTitleActorMotion_00491ca8[9].y,
        g_apTitleSections_005d3f70[10],
        (short)(g_szTitleConductorFrames_00491c68[20] - 'a'));
    if (g_nMusicDriverMode_0049be8c == 2 ||
        g_nMusicDriverMode_0049be8c == 1) {
        do {
            if (WaitForInputKey() != 0)
                g_bSceneEscapeRequested_0049d4b0 = 1;
            if (g_nTitleMusicSequenceStage_0049be94 >= 2 ||
                g_bSceneEscapeRequested_0049d4b0 != 0) {
                RefreshMemoryStatusOverlay();
                break;
            }
            waitCount++;
            if (waitCount > 1)
                break;
        } while (1);
    }

done:
    return;
}

/* Function start: 0x4087FF */
void DrawScaledTitleLogo(short y, short scale)
{
    short bounds[4];

    GetTransformedShapeBounds(
        &g_stSecondaryViewBuffer_005d2c90, 162, y,
        g_apTitleSections_005d3f70[0], 1, 0, scale, 0, bounds);
    DrawSpriteScaled(&g_stSecondaryViewBuffer_005d2c90, bounds[0], y,
                     g_apTitleSections_005d3f70[0], 0, 0, scale, 0);
    DrawSpriteScaled(&g_stSecondaryViewBuffer_005d2c90, 163, y,
                     g_apTitleSections_005d3f70[0], 1, 0, scale, 0);
    DrawSpriteScaled(&g_stSecondaryViewBuffer_005d2c90,
                     (short)(bounds[2] + 1), y,
                     g_apTitleSections_005d3f70[0], 2, 0, scale, 0);
}

/* Function start: 0x4088A9 */
void PushTitleOrchestraAway(void)
{
    short x;
    short y;
    short distance;
    short actor;

    g_nFrameSkipCountdown_0049d760 = 1;
    for (distance = 1; distance < 120;
         distance = (short)(distance + distance / 4 + 1)) {
        g_nFrameSkipCountdown_0049d760--;
        if (g_nFrameSkipCountdown_0049d760 < 1) {
            g_nFrameSkipCountdown_0049d760 = g_nFrameSkip_0049d764;
            DrawTitleSky(24);
            for (actor = 0; actor < 9; actor++) {
                x = (short)(g_aTitleActorMotion_00491ca8[actor].x +
                    g_aTitleActorMotion_00491ca8[actor].velocityX *
                        distance);
                y = (short)(g_aTitleActorMotion_00491ca8[actor].y +
                    g_aTitleActorMotion_00491ca8[actor].velocityY *
                        distance);
                DrawSpriteScaled(
                    &g_stSecondaryViewBuffer_005d2c90, x, y,
                    g_apTitleSections_005d3f70[actor + 1],
                    (short)(g_apszTitleActorFrames_00491c80[actor][31]
                                - 'a'),
                    0,
                    (short)(g_aTitleActorMotion_00491ca8[actor].velocityY *
                                distance * 4 + 0x100),
                    0);
            }
            x = (short)(g_aTitleActorMotion_00491ca8[9].x +
                g_aTitleActorMotion_00491ca8[9].velocityX * distance);
            y = (short)(g_aTitleActorMotion_00491ca8[9].y +
                g_aTitleActorMotion_00491ca8[9].velocityY * distance);
            DrawSpriteScaled(
                &g_stSecondaryViewBuffer_005d2c90, x, y,
                g_apTitleSections_005d3f70[10],
                (short)(g_szTitleConductorFrames_00491c68[21] - 'a'),
                0,
                (short)(g_aTitleActorMotion_00491ca8[9].velocityY *
                            distance * 4 + 0x100),
                0);
            RefreshMemoryStatusOverlay();
        }
        if (WaitForInputKey() != 0)
            g_bSceneEscapeRequested_0049d4b0 = 1;
        if (g_bSceneEscapeRequested_0049d4b0 != 0)
            break;
    }
}

/* Function start: 0x408A88 */
short RevealTitleLogo(void)
{
    short scale;
    short distance;
    short logoY;
    short bounds[4];
    short redraw;
    short drawPlanetFirst;
    short scaleDelay;
    short planetY;
    short positionDelay;

    drawPlanetFirst = 0;
    logoY = 59;
    g_nFrameSkipCountdown_0049d760 = 1;
    distance = 5000;
    scaleDelay = 0;
    positionDelay = 0;
    for (; distance >= 1000;
         distance--, scaleDelay--, positionDelay--) {
        redraw = 0;
        if (scaleDelay == 0) {
            redraw = 1;
            planetY = (short)(120000 / (int)distance);
            scaleDelay = 200;
        }
        if (positionDelay == 0) {
            redraw = 1;
            scale = (short)(256000 / (int)distance);
            if (distance > 3000)
                logoY = (short)(logoY - 2);
            else
                logoY = (short)(logoY + 2);
            positionDelay = 100;
        }
        if (redraw != 0) {
            g_nFrameSkipCountdown_0049d760--;
            if (g_nFrameSkipCountdown_0049d760 < 1) {
                g_nFrameSkipCountdown_0049d760 = g_nFrameSkip_0049d764;
                DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                                  0, 0,
                                  g_pTitleFieldShape_00491cf8, 0);
                GetTransformedShapeBounds(
                    &g_stSecondaryViewBuffer_005d2c90, 162, logoY,
                    g_apTitleSections_005d3f70[0], 1, 0,
                    scale, 0, bounds);
                if (bounds[3] < planetY)
                    drawPlanetFirst = 1;
                if (drawPlanetFirst == 0) {
                    DrawScaledTitleLogo(logoY, scale);
                    DrawSpriteDefault(
                        &g_stSecondaryViewBuffer_005d2c90,
                        160, planetY,
                        g_apTitleSections_005d3f70[12], 0);
                } else {
                    DrawSpriteDefault(
                        &g_stSecondaryViewBuffer_005d2c90,
                        160, planetY,
                        g_apTitleSections_005d3f70[12], 0);
                    DrawScaledTitleLogo(logoY, scale);
                }
                RefreshMemoryStatusOverlay();
            }
        }
        if (WaitForInputKey() != 0)
            g_bSceneEscapeRequested_0049d4b0 = 1;
        if (g_bSceneEscapeRequested_0049d4b0 != 0)
            return logoY;
    }
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 0, 0,
                      g_pTitleFieldShape_00491cf8, 0);
    DrawScaledTitleLogo(logoY, scale);
    RefreshMemoryStatusOverlay();
    return logoY;
}

/* Function start: 0x408CC8 */
void ShowVictoryScreen(short logoY)
{
    short finishing;
    int loopCount;
    short emptyCount;
    short finalEmptyCount;
    short index;
    short frameCount;
    short spawnFirework;

    finishing = 0;
    frameCount = 0;
    loopCount = 0;
    FlushSoundEffects();
    InitializeFireworks();
    do {
        if (finishing == 0) {
            if (g_nMusicDriverMode_0049be8c == 2 ||
                g_nMusicDriverMode_0049be8c == 1) {
                if (g_nTitleMusicSequenceStage_0049be94 >= 4) {
                    spawnFirework = (short)(
                        (unsigned short)RandomInRange(0, 0) == 0);
                } else {
                    spawnFirework = (short)(
                        (unsigned short)RandomInRange(0, 5) == 0);
                }
            } else {
                spawnFirework = (short)(
                    (unsigned short)RandomInRange(0, 0) == 0);
            }
        }
        if (spawnFirework != 0 && finishing == 0) {
            for (index = 0; index < 5; index++) {
                if (g_aFireworks_005c8df0[index].frame == -1) {
                    g_aFireworks_005c8df0[index].frame = 0;
                    g_aFireworks_005c8df0[index].x =
                        RandomInRange(0, 319);
                    g_aFireworks_005c8df0[index].y =
                        RandomInRange(0, 127);
                    g_aFireworks_005c8df0[index].variant =
                        RandomInRange(0, 2);
                    break;
                }
            }
        }
        DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 0, 0,
                          g_pTitleFieldShape_00491cf8, 0);
        emptyCount = TheEndFireWorks(
            &g_stSecondaryViewBuffer_005d2c90, 5);
        DrawScaledTitleLogo(logoY, 0x100);
        RefreshMemoryStatusOverlay();
        frameCount++;
        if (finishing == 1 && emptyCount == 5)
            break;
        if (g_nMusicDriverMode_0049be8c == 2 ||
            g_nMusicDriverMode_0049be8c == 1) {
            if (g_nTitleMusicSequenceStage_0049be94 > 4)
                finishing = 1;
        } else if (frameCount > 10) {
            finishing = 1;
        }
        if (WaitForInputKey() != 0)
            g_bSceneEscapeRequested_0049d4b0 = 1;
        if (g_bSceneEscapeRequested_0049d4b0 != 0)
            return;
        loopCount++;
    } while (loopCount <= 10);

    for (index = 0; index < 29; index++) {
        g_aFireworks_005c8df0[index].frame = 0;
        g_aFireworks_005c8df0[index].x = RandomInRange(0, 319);
        g_aFireworks_005c8df0[index].y = RandomInRange(0, 127);
        g_aFireworks_005c8df0[index].variant = RandomInRange(0, 2);
    }
    emptyCount = 0;
    finalEmptyCount = 0;
    do {
        if (emptyCount == 30)
            break;
        finalEmptyCount++;
        if (finalEmptyCount == 4) {
            g_aFireworks_005c8df0[29].frame = 0;
            g_aFireworks_005c8df0[29].x = RandomInRange(0, 319);
            g_aFireworks_005c8df0[29].y = RandomInRange(0, 127);
            g_aFireworks_005c8df0[29].variant = RandomInRange(0, 2);
        }
        DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 0, 0,
                          g_pTitleFieldShape_00491cf8, 0);
        emptyCount = TheEndFireWorks(
            &g_stSecondaryViewBuffer_005d2c90, 30);
        DrawScaledTitleLogo(logoY, 0x100);
        RefreshMemoryStatusOverlay();
        if (WaitForInputKey() != 0)
            g_bSceneEscapeRequested_0049d4b0 = 1;
    } while (g_bSceneEscapeRequested_0049d4b0 == 0);

    for (index = 29; index >= 0; index--) {
        if (g_aFireworks_005c8df0[index].frame != -1) {
            ((void (__cdecl *)(int, short))FlushSoundEffect)(
                g_aFireworks_005c8df0[index].soundHandle, index);
        }
    }
}
