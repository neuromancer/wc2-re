/*
 *  Navigation map.
 *
 *  Retail object 0x44FA70-0x45244F (link order; see docs/ORDER.md).
 */
/* This unit owns storage declared in globals.h as statics. */
#define NAV_C
#include "game.h"

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
    { 2, 3, &g_cRedColour_0049cb5c,
      &g_cRedColour_0049cb5c,
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
unsigned char *g_pNavLocationText_0049bc54 = 0;
char *g_apszShipMissionTypeNames_0049bc58[14] = {
    "Patrol",
    "Escort",
    "Strike",
    "Defend",
    "Wingman",
    "Flee",
    "Goto Warp",
    "Warp Arrive",
    "Rendezvous",
    "Come Home",
    "Tanker",
    "Tow",
    "Break Off",
    0
};
char g_szNoFurtherObjectives_0049bc90[32] =
    "Further objectives out-system.";
short g_bBriefingMapActive_0049bcb0 = 0;
int g_bInflightComputerActive_0049bcb4 = 0;

/* NavMapLabelFits tries a placement before TryPlaceNavMapLabel is defined. */
short TryPlaceNavMapLabel(short x, short y, short width, short force);

/* Function start: 0x44FA70 */
void DrawNavTextLine(unsigned char alignment, unsigned short colour,
                     const char *format, ...)
{
    va_list arguments;

    g_pCurrentTextContext_005c8d1c->colour = colour;
    g_pCurrentTextContext_005c8d1c->alignment = alignment;
    va_start(arguments, format);
    g_pCurrentTextContext_005c8d1c->textCursor =
        g_pCurrentTextContext_005c8d1c->text;
    /* The original pushes the list itself; the port's FormatTextTokens takes
     * its address, because va_arg has to advance the caller's copy on an ABI
     * where a vararg is not always four bytes wide. */
#ifdef SDL_PORT
    FormatTextTokens((void (*)(int))AppendTextCharacter, format,
                     &arguments);
#else
    FormatTextTokens((void (*)(int))AppendTextCharacter, format,
                     (unsigned long *)arguments);
#endif
    va_end(arguments);
    DrawTextString(g_pCurrentTextContext_005c8d1c->text);
}

/* Function start: 0x44FADA */
void SetNavMapCoordinateScaling(short enabled)
{
    g_nNavMapCoordinateScaling_0049bb80 = enabled;
}

/* Function start: 0x44FAEF */
void ScaleNavMapMarkerSize(short *size)
{
    *size = (short)(*size / (g_nNavMapScale_0049bb84 * 100));
}

/* Function start: 0x44FB19 */
void ScaleNavMapCoordinates(short *x, short *y, short mapX, short mapY)
{
    *x = (short)((mapX - g_nNavmapMapCentreX_005d16c2) /
                 g_nNavMapScale_0049bb84 + 75);
    *y = (short)(67 - (mapY - g_nNavMapCentreY_005d16ec) /
                 g_nNavMapScale_0049bb84);
}

/* Function start: 0x44FB68 */
void nav_getxy(short *x, short *y, int worldX, int worldZ)
{
    *x = (short)((worldX / 100) >> 8);
    *y = (short)((worldZ / 100) >> 8);
    if (g_nNavMapCoordinateScaling_0049bb80 != 0)
        ScaleNavMapCoordinates(x, y, *x, *y);
}

/* Function start: 0x44FBC8 */
void CheckPoint(short x, short y)
{
    g_nNavmapMapMinimumX_005d16a0 =
        MinShort(g_nNavmapMapMinimumX_005d16a0, x);
    g_nNavPlotMaxX_005d169e =
        MaxShort(g_nNavPlotMaxX_005d169e, x);
    g_nNavMapMinimumYValue_005d16c4 =
        MinShort(g_nNavMapMinimumYValue_005d16c4, y);
    g_nNavmapMapMaximumY_005d16c0 =
        MaxShort(g_nNavmapMapMaximumY_005d16c0, y);
}

/* Function start: 0x44FC37 */
void IncludeNavMapWorldPoint(int worldX, int worldZ)
{
    short x;
    short y;

    nav_getxy(&x, &y, worldX, worldZ);
    CheckPoint(x, y);
}

/* Function start: 0x44FC6D */
void SetScale(void)
{
    short objectiveIndex;
    short obj;
    short mapHeight;
    short mapWidth;

    SetNavMapCoordinateScaling(0);
    g_nNavmapMapMinimumX_005d16a0 = g_aMissionObjectives_004932a8[0].mapX;
    g_nNavPlotMaxX_005d169e = g_aMissionObjectives_004932a8[0].mapX;
    g_nNavMapMinimumYValue_005d16c4 = g_aMissionObjectives_004932a8[0].mapY;
    g_nNavmapMapMaximumY_005d16c0 = g_aMissionObjectives_004932a8[0].mapY;
    for (objectiveIndex = 0;
         objectiveIndex < (short)g_cMissionObjectiveCount_00493294;
         objectiveIndex++) {
        if (mobile_objective(objectiveIndex) != 0) {
            obj = find_ship_index((short)
                g_aMissionObjectives_004932a8[objectiveIndex].index);
            if (obj != -1)
                g_aMissionObjectives_004932a8[objectiveIndex].position =
                    g_aShipPosition_00494550[obj];
        }
        nav_getxy(
            &g_aMissionObjectives_004932a8[objectiveIndex].mapX,
            &g_aMissionObjectives_004932a8[objectiveIndex].mapY,
            g_aMissionObjectives_004932a8[objectiveIndex].position.x,
            g_aMissionObjectives_004932a8[objectiveIndex].position.z);
        CheckPoint(g_aMissionObjectives_004932a8[objectiveIndex].mapX,
                   g_aMissionObjectives_004932a8[objectiveIndex].mapY);
    }
    IncludeNavMapWorldPoint(g_aShipPosition_00494550[0].x,
                            g_aShipPosition_00494550[0].z);
    mapWidth = (short)(g_nNavPlotMaxX_005d169e -
                    g_nNavmapMapMinimumX_005d16a0);
    mapHeight = (short)(g_nNavmapMapMaximumY_005d16c0 -
                     g_nNavMapMinimumYValue_005d16c4);
    g_nNavmapMapCentreX_005d16c2 =
        (short)(mapWidth / 2 + g_nNavmapMapMinimumX_005d16a0);
    g_nNavMapCentreY_005d16ec =
        (short)(mapHeight / 2 + g_nNavMapMinimumYValue_005d16c4);
    g_nNavMapScale_0049bb84 = MaxShort(
        (short)((mapWidth / 2 + mapWidth) / 150),
        (short)((mapHeight / 2 + mapHeight) / 135));
    if (g_nNavMapScale_0049bb84 == 0)
        g_nNavMapScale_0049bb84 = 100;
    SetNavMapCoordinateScaling(1);
}

void set_sphere_point(const MissionShipRecord *record, FixedVector *position);

/* Function start: 0x44FE9F */
void Build_objective_list(void)
{
    MissionObjectiveSource *record;
    short type;
    const char *text;
    FixedVector location;
    short flightPathCount;
    short i;

    RefreshObjectiveShipNavPoints();
    SetNavMapCoordinateScaling(0);
    g_cMissionObjectiveCount_00493294 = 0;
    flightPathCount = 0;
    record = g_aMissionObjectiveSources_005d3c70;
#ifdef SDL_PORT
    /* Retail reads the next word after the eight-record table before checking
     * that all eight output slots are full.  That word is the adjacent mission
     * header and cannot affect the loop body, so bound the native read first. */
    while (g_cMissionObjectiveCount_00493294 < 8 &&
           (type = record->type) != -1) {
#else
    while ((type = record->type) != -1 &&
           g_cMissionObjectiveCount_00493294 < 8) {
#endif
        i = record->index;
        g_aMissionObjectives_004932a8[
            g_cMissionObjectiveCount_00493294].flags = 0;
        switch (type) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 6:
            text = (const char *)&g_aMissionShips_00492290[i];
            set_sphere_point(&g_aMissionShips_00492290[i], &location);
            g_abFlightPath_004932a0[flightPathCount++] =
                g_cMissionObjectiveCount_00493294;
            break;
        case 0:
        case 5:
            text =
                (const char *)&g_aMissionNavPoints_00491e98[i];
            location = g_aMissionNavPoints_00491e98[i].position;
            g_abFlightPath_004932a0[flightPathCount++] =
                g_cMissionObjectiveCount_00493294;
            break;
        }

        g_aMissionObjectives_004932a8[
            g_cMissionObjectiveCount_00493294].type = type;
        g_aMissionObjectives_004932a8[
            g_cMissionObjectiveCount_00493294].index = record->index;
        g_aMissionObjectives_004932a8[
            g_cMissionObjectiveCount_00493294].field_8 = -1;
        g_aMissionObjectives_004932a8[
            g_cMissionObjectiveCount_00493294].name = record->description;
        g_aMissionObjectives_004932a8[
            g_cMissionObjectiveCount_00493294].position = location;
        nav_getxy(
            &g_aMissionObjectives_004932a8[
                g_cMissionObjectiveCount_00493294].mapX,
            &g_aMissionObjectives_004932a8[
                g_cMissionObjectiveCount_00493294].mapY,
            location.x, location.z);
        g_aMissionObjectives_004932a8[
            g_cMissionObjectiveCount_00493294].displayName = text;
        g_cMissionObjectiveCount_00493294++;
        record++;
    }

    g_abFlightPath_004932a0[flightPathCount] = -1;
    g_cCurrentNavPointIndex_00493298 = 0;
    g_cCurrentObjective_004931cc = 0;
    if (g_cMissionObjectiveCount_00493294 != 0) {
        while (set_new_objective(
                   (short)g_cCurrentNavPointIndex_00493298) == 0)
            g_cCurrentNavPointIndex_00493298++;
    }
}

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

short TryPlaceNavMapLabel(short x, short y, short width, short force);

/* Function start: 0x450210 */
short NavMapLabelPositionAvailable(short x, short y,
                                   short width, short height)
{
    short clear;
    short rect;
    short checkX;
    short checkY;

    clear = 1;
    clear = NavMapLabelFits(x, y, width, height);
    if (clear == 1) {
        for (rect = 0;
             rect < g_nNavMapReservedAreaCount_0049bc50 && clear != 0;
             rect++) {
            for (checkX = x; checkX < x + width && clear != 0;
                 checkX++) {
                for (checkY = y; checkY < y + height && clear != 0;
                     checkY++) {
                    clear = (short)(NavMapPointInsideReservedArea(
                                        rect, checkX, checkY) == 0);
                }
            }
        }
    }
    return clear;
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
        g_aNavMapLabels_005b34e0[g_nNavMapLabelCount_0049bc4c].x = x;
        g_aNavMapLabels_005b34e0[g_nNavMapLabelCount_0049bc4c].y = y;
        placed = 1;
    }
    return placed;
}

#pragma function(strlen)
/* Function start: 0x450458 */
void PlaceNavMapLabel(short x, short y, unsigned short colour,
                      const char *text)
{
    short textWidth;
    short shift;

    textWidth = (short)(strlen(text) * 4 + 2);
    g_aNavMapLabels_005b34e0[
        g_nNavMapLabelCount_0049bc4c].colour = colour;
    g_aNavMapLabels_005b34e0[
        g_nNavMapLabelCount_0049bc4c].text = text;
    shift = -1;
    do {
        shift++;
    } while (TryPlaceNavMapLabel((short)(shift + x + 4), y,
                                 textWidth, 0) == 0 &&
             TryPlaceNavMapLabel((short)(x - textWidth / 2),
                                 (short)(shift + y + 5), textWidth,
                                 (short)(shift == 12)) == 0 &&
             TryPlaceNavMapLabel((short)(x - textWidth - 3 - shift), y,
                                 textWidth, (short)(shift == 12)) == 0 &&
             TryPlaceNavMapLabel(x, (short)(shift + y + 5), textWidth,
                                 (short)(shift == 12)) == 0 &&
             TryPlaceNavMapLabel((short)(x - textWidth / 2),
                                 (short)(y - 9 - shift), textWidth,
                                 (short)(shift == 12)) == 0 &&
             shift != 12);
    ReserveNavMapArea(
        g_aNavMapLabels_005b34e0[g_nNavMapLabelCount_0049bc4c].x,
        g_aNavMapLabels_005b34e0[g_nNavMapLabelCount_0049bc4c].y,
        textWidth, 6);
    g_nNavMapLabelCount_0049bc4c++;
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

/* Objective-state callees from the cockpit module. */
short hidden_objective(short objective);
short visited(short objective);
short achieved(short objective);

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
        if (hidden_objective(previous) != 0)
            continue;
        else if (g_aMissionObjectives_004932a8[previous].index ==
                     missionShip &&
                 ObjectivesShareNavLabel(objective, previous) != 0)
            break;
    }
    if (previous >= objective)
        PlaceNavMapLabel(x, y, colour, text);
}

/* Function start: 0x4507DA */
short IsPointInNavMapLabel(short labelIndex, short x, short y)
{
    return g_aNavMapLabels_005b34e0[labelIndex].x <= x &&
           (unsigned int)x <=
               (unsigned int)(g_aNavMapLabels_005b34e0[labelIndex].x +
                              strlen(g_aNavMapLabels_005b34e0[labelIndex].text) * 4) &&
           g_aNavMapLabels_005b34e0[labelIndex].y <= y &&
           y <= g_aNavMapLabels_005b34e0[labelIndex].y + 6;
}

#pragma intrinsic(strlen)

void DrawViewportLine(Viewport *viewport, short x1, short y1, short x2, short y2, short colour);
void DrawViewportBorder(Viewport *viewport, short left, short top, short right, short bottom, short colour);

/* Function start: 0x45087F */
void DrawNavMapLabels(void)
{
    short label;

    for (label = 0; label < (short)g_nNavMapLabelCount_0049bc4c; label++) {
        DrawFormattedText("%X%Y%F%s",
                          g_aNavMapLabels_005b34e0[label].x,
                          g_aNavMapLabels_005b34e0[label].y,
                          g_aNavMapLabels_005b34e0[label].colour,
                          g_aNavMapLabels_005b34e0[label].text);
    }
}

/* Function start: 0x450900 */
void DrawNavRectangleMarker(short x, short y, short size, short shadow,
                            unsigned short colour, short reserve)
{
    if (shadow == 0)
        DrawViewportEllipse(g_stNavLabelTextContextInfo_005d16f0.viewport,
                            x, y, size, (short)((size * 7) / 8),
                            (short)colour);
    else
        DrawViewportEllipseShadow(
            g_stNavLabelTextContextInfo_005d16f0.viewport,
            x, y, size, (short)((size * 7) / 8), (short)colour);
    if (reserve != 0 && size < 5)
        ReserveNavMapArea((short)(x - size), (short)(y - size),
                          (short)(size * 2 + 1),
                          (short)(size * 2 + 1));
}

/* Hazard and text-context callees from other modules. */
short find_ships_sphere(short missionShip);
void SetTextContext(TextContext *context);

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
        DrawViewportPixel(g_stNavLabelTextContextInfo_005d16f0.viewport,
                          x, y, colour);
        DrawViewportPixel(g_stNavLabelTextContextInfo_005d16f0.viewport,
                          (short)(x + 1), y, colour);
        DrawViewportPixel(g_stNavLabelTextContextInfo_005d16f0.viewport,
                          x, (short)(y + 1), colour);
        DrawViewportPixel(g_stNavLabelTextContextInfo_005d16f0.viewport,
                          (short)(x + 1),
                          (short)(y + 1), colour);
    } else {
        DrawNavSquareOutline(g_stNavLabelTextContextInfo_005d16f0.viewport,
                             x, y, size,
                             (signed char)colour);
    }
    if (reserve != 0 && size < 5)
        ReserveNavMapArea((short)(x - size), (short)(y - size),
                          (short)(size * 2 + 1),
                          (short)(size * 2 + 1));
    (void)shadow;
}

void AddFixedVectors(const FixedVector *left, const FixedVector *right, FixedVector *sum);

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
    DrawNavTriangleOutline(g_stNavLabelTextContextInfo_005d16f0.viewport,
                           x, y, size,
                           (signed char)colour);
    if (reserve != 0 && size < 5)
        ReserveNavMapArea((short)(x - size), (short)(y - size),
                          (short)(size * 2 + 1),
                          (short)(size * 2 + 1));
    (void)shadow;
}

/* The player's own marker is drawn by the routine further down. */
void DrawNavPlayerMarker(short colour, short reserve);

/* Function start: 0x450C2A */
void DrawNavCrossMarker(short x, short y, short size, short shadow,
                        unsigned short colour, short reserve)
{
    DrawViewportLine(g_stNavLabelTextContextInfo_005d16f0.viewport,
                     (short)(x - size), (short)(y - size),
                     (short)(x + size), (short)(y + size),
                     (short)colour);
    DrawViewportLine(g_stNavLabelTextContextInfo_005d16f0.viewport,
                     (short)(x - size), (short)(y + size),
                     (short)(x + size), (short)(y - size),
                     (short)colour);
    if (reserve != 0 && size < 5)
        ReserveNavMapArea((short)(x - size), (short)(y - size),
                          (short)(size * 2 + 1),
                          (short)(size * 2 + 1));
    (void)shadow;
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
        else
            return 0;
    default:
        navPoint = find_ships_sphere(
            g_aMissionObjectives_004932a8[objective].index);
        if (navPoint == -1)
            return 1;
        else if (g_aMissionNavPoints_00491e98[navPoint].systemIndex ==
                 g_stElapsedCampaignDate_005d170c.year)
            return 1;
        else
            return 0;
    }
}

/* Function start: 0x450DF6 */
void SetScreenClipRect(unsigned short a, unsigned short b,
                       unsigned short c, unsigned short d)
{
    SetRectBounds(&g_stSecondaryViewBufferBlock_005d2c90, a, b, c, d);
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
    x = (short)(x + g_stNavLabelTextContextInfo_005d16f0.viewport->left);
    y = (short)(y + g_stNavLabelTextContextInfo_005d16f0.viewport->top);
    DrawViewportPixel(g_stNavLabelTextContextInfo_005d16f0.viewport,
                      x, y, colour);
    DrawNavSquareMarker(x, y, 0, 0, colour, reserve);
}

/* Function start: 0x450F1E */
/* Plot the briefing beacons on the nav map: a pixel, a square marker and a
 * label for each active record. */
void DrawNavMapBriefingBeacons(void)
{
    const NavMapBeacon *beaconRecord;
    short gridX;
    short gridY;
    short navX;
    short navY;

    beaconRecord = (const NavMapBeacon *)g_abMissionBriefingData_005d3e90;
    while (beaconRecord->active != 0) {
        nav_getxy(&navX, &navY, beaconRecord->position.x,
                  beaconRecord->position.z);
        ScaleNavMapCoordinates(&gridX, &gridY, navX, navY);
        DrawViewportPixel(&g_stSecondaryViewBufferBlock_005d2c90, gridX, gridY,
                          g_ucNavObjectiveMarkerColour_0049cb78);
        DrawNavSquareMarker(gridX, gridY, 2, 0,
                            g_ucPrimaryTextColour_0049cb64, 1);
        AddUniqueObjectiveNavLabel(gridX, gridY,
                                   g_ucDefaultTextColour_0049cb7c,
                                   beaconRecord->name, 0, -1);
        beaconRecord++;
    }
}

/* Function start: 0x450FE2 */
void BuildMap(short showPlayer)
{
    unsigned short nameColour;
    MissionNavPoint *nav;
    const NavMapObjectiveStyle *mark;
    MissionShipRecord *ship;
    short x;
    short y;
    short checkpoint;
    short index;

    SetScreenClipRect(1, 1, 153, 138);
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock_005d2c90, 1, 1,
                      g_pNavMapShape_0049bc48, 0);
    SetScreenClipRect(2, 2, 152, 137);
    g_stNavLabelTextContextInfo_005d16f0.viewport =
        &g_stSecondaryViewBufferBlock_005d2c90;
    g_stNavLabelTextContextInfo_005d16f0.text = g_szDefaultTextBufferText_005d2b80;
    InitializeTextContextFromFont(&g_stNavLabelTextContextInfo_005d16f0,
                                  2, g_ucPrimaryTextColour_0049cb64, -1);
    g_stNavLabelTextContextInfo_005d16f0.alignment = 0;
    SetTextContext(&g_stNavLabelTextContextInfo_005d16f0);
    nav = g_aMissionNavPoints_00491e98;
    ResetNavMapLabels();
    ResetNavMapReservedAreas();
    SetScale();
    DrawNavMapBriefingBeacons();

    for (checkpoint = 0; checkpoint < 10; checkpoint++) {
        if (g_aMissionNavPoints_00491e98[checkpoint].type != 0 &&
            g_aMissionNavPoints_00491e98[checkpoint].systemIndex ==
                g_stElapsedCampaignDate_005d170c.year) {
            for (index = 0; index < 10; index++) {
                if (nav->missionShips[index] != -1) {
                    ship = &g_aMissionShips_00492290[
                        nav->missionShips[index]];
                    /* WC2 hazard field types; the image names neither. */
                    switch (ship->objectType) {
                    case 5:
                        DrawNavHazardMarker(
                            nav->position, ship->position,
                            ship->speed,
                            g_ucAsteroidNavMarkerColour_0049cb84,
                            g_ucAsteroidNavMarkerColour_0049cb84,
                            "Asteroids");
                        break;
                    case 6:
                        DrawNavHazardMarker(
                            nav->position, ship->position,
                            ship->speed,
                            g_cRedColour_0049cb5c,
                            g_cRedColour_0049cb5c,
                            "Mines");
                        break;
                    }
                }
            }
        }
        nav++;
    }

    for (index = 0;
         g_cMissionObjectiveCount_00493294 > index;
         index++) {
        if (mobile_objective(index) == 0 ||
            (g_aMissionShips_00492290[
                 (signed char)g_aMissionObjectives_004932a8[
                     index].index].state == 0 &&
             achieved(index) == 0)) {
        ScaleNavMapCoordinates(
            &x, &y, g_aMissionObjectives_004932a8[index].mapX,
            g_aMissionObjectives_004932a8[index].mapY);
        if (hidden_objective(index) == 0 &&
            IsObjectiveInDisplayedSystem(index) != 0) {
        mark = &g_aNavMapObjectiveStyles_0049bb88[
            g_aMissionObjectives_004932a8[index].type];
        if (visited(index) == 0)
            DrawViewportPixel(&g_stSecondaryViewBufferBlock_005d2c90, x, y,
                              *mark->unvisitedColour);
        switch (mark->markerType) {
        case 0:
            break;
        case 1:
            DrawNavSquareMarker(x, y, mark->markerSize, 0,
                                *mark->markerColour, 1);
            break;
        case 2:
            DrawNavRectangleMarker(x, y, mark->markerSize, 0,
                                   *mark->markerColour, 1);
            break;
        case 3:
            DrawNavTriangleMarker(x, y, mark->markerSize, 0,
                                  *mark->markerColour, 1);
            break;
        case 4:
            DrawNavCrossMarker(x, y, mark->markerSize, 0,
                               *mark->markerColour, 1);
            break;
        }
        if (g_cCurrentObjective_004931cc == index)
            nameColour = g_cYellowColour_0049cb58;
        else
            nameColour = *mark->labelColour;
        g_awNavObjectiveLabelIndex_005d16b0[index] =
            g_nNavMapLabelCount_0049bc4c;
        AddUniqueObjectiveNavLabel(
            x, y, nameColour, objective_name(index),
            index,
            (short)g_aMissionObjectives_004932a8[index].index);
        }
        }
    }

    if (showPlayer != 0 &&
        g_stElapsedCampaignDate_005d170c.year ==
            g_nCurStellarSystem_005d169c) {
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

void SetTextContext(TextContext *context);

/* Function start: 0x45153A */
void UpdateInflightNavText(short showColon)
{
    char time[12];
    short savedX;

    SetTextContext(&g_stNavMapTextCtx_005d16d0);
    if (g_pElapsedCampaignDatePointer_005d3e8c == 0)
        g_pElapsedCampaignDatePointer_005d3e8c =
            &g_stElapsedCampaignDate_005d170c;
    sprintf(time, "%02d",
            (int)((signed char *)g_pElapsedCampaignDatePointer_005d3e8c)[0]);
    DrawFormattedText(
        "%X%YStandard time %s",
        g_stNavMapTextCtx_005d16d0.viewport->left + 152,
        g_stNavMapTextCtx_005d16d0.viewport->top + 140,
        time);
    savedX = g_stNavMapTextCtx_005d16d0.cursorX;
    if (showColon == 0) {
        DrawFormattedText(" ");
    } else {
        g_stNavMapTextCtx_005d16d0.cursorX =
            (short)(g_stNavMapTextCtx_005d16d0.cursorX + 4);
    }
    sprintf(time, "%02d  ",
            (int)((signed char *)g_pElapsedCampaignDatePointer_005d3e8c)[1]);
    DrawFormattedText(time);
    g_stNavMapTextCtx_005d16d0.cursorX = savedX;
    if (showColon != 0)
        DrawFormattedText(":");
}

/* Function start: 0x451637 */
void FormatNavCoordinates(unsigned char *out)
{
    unsigned char tmp[4];

    if (out == 0)
        out = (unsigned char *)&g_stElapsedCampaignDate_005d170c;
    SplitGameClockTicksForNav(tmp);
    out[1] = tmp[2];
    out[0] = tmp[3];
}

/* Function start: 0x451673 */
void DrawSelectedNavLegendEntry(const char *text, short navPoint)
{
    if ((short)g_cCurrentNavPointIndex_00493298 == navPoint) {
        DrawNavTextLine(0, (unsigned short)g_cYellowColour_0049cb58, text);
        DrawNavTextLine(0, (unsigned short)g_cYellowColour_0049cb58,
                        "\n");
    }
}

/* Function start: 0x4516C0 */
void DrawNavMapLegend(void)
{
    short objective;

    objective = 0;
    while (objective < (short)g_cMissionObjectiveCount_00493294 &&
           (visited(objective) != 0 || hidden_objective(objective) != 0))
        objective++;
    SetTextCursor((unsigned short)g_stSecondaryViewBufferBlock_005d2c90.left, 120);
    DrawSelectedNavLegendEntry("MISSION FLIGHT PATH",
                               objective);
    DrawSelectedNavLegendEntry(
        "HOME BASE",
        (short)((short)g_cMissionObjectiveCount_00493294 - 1));
}

/* Function start: 0x451756 */
char *nav_note(short objective)
{
    if (IsObjectiveInDisplayedSystem(objective) == 0)
        return g_szNoFurtherObjectives_0049bc90;
    if (*g_aMissionObjectives_004932a8[objective].name == '?')
        return g_aMissionObjectives_004932a8[objective].name + 1;
    else if (*g_aMissionObjectives_004932a8[objective].name == '.')
        return strchr(g_aMissionObjectives_004932a8[objective].name, 0);
    else
        return g_aMissionObjectives_004932a8[objective].name;
}

/* Function start: 0x45180F */
void DrawNavLocationReadout(const char *title, short showFlightData)
{
    char fileName[11] = "sector.000";
    short file;
    short length;

    ClearViewport(&g_stSecondaryViewBufferBlock_005d2c90, g_cSecondaryViewBufferColour_0049cb4c);
    SetScreenClipRect(155, 2, 259, 155);
    g_stNavMapTextCtx_005d16d0.viewport = &g_stSecondaryViewBufferBlock_005d2c90;
    g_stNavMapTextCtx_005d16d0.text = g_szDefaultTextBufferText_005d2b80;
    InitializeTextContextFromFont(&g_stNavMapTextCtx_005d16d0,
                                  1, g_ucPrimaryTextColour_0049cb64, g_cSecondaryViewBufferColour_0049cb4c);
    g_stNavMapTextCtx_005d16d0.alignment = 0;
    g_stNavMapTextCtx_005d16d0.cursorX = 0;
    g_stNavMapTextCtx_005d16d0.cursorY = 0;
    SetTextContext(&g_stNavMapTextCtx_005d16d0);
    DrawNavTextLine(0, g_ucDefaultTextColour_0049cb7c,
                    "\n");
    DrawNavTextLine(2, g_ucDefaultTextColour_0049cb7c,
                    "%s\n\n", title);
    if (g_pNavLocationText_0049bc54 == 0) {
        fileName[9] =
            (char)(g_nSelectedCampaignSlot_005d3bf2 + '0');
        file = OpenDataFileOrDie(fileName);
        length = (short)filelength(file);
        g_pNavLocationText_0049bc54 =
            AllocateTaggedMemory(length + 1, 0x40);
        ReadDataFileAtOffset((unsigned short)file, 0, length,
                             g_pNavLocationText_0049bc54);
        g_pNavLocationText_0049bc54[length] = 0;
    }
    DrawNavTextLine(0, g_ucDefaultTextColour_0049cb7c,
                    "Sector: %s\n",
                    (char *)g_pNavLocationText_0049bc54);
    DrawNavTextLine(0, g_ucDefaultTextColour_0049cb7c,
                    "System: %s\n\n",
                    g_abSeriesAuxData_005d3c40);
    DrawNavTextLine(2, g_ucDefaultTextColour_0049cb7c,
                    "* %s *\n",
                    g_abMissionAuxBlockBytes_005d3c10);
    DrawNavTextLine(2, g_ucDefaultTextColour_0049cb7c,
                    "* %s *\n",
                    g_apszShipMissionTypeNames_0049bc58[
                        g_aMissionShips_00492290[
                            g_stMissionHeaderBlock_005d3e70
                                .playerMissionShip].missionType]);
    DrawNavTextLine(2, g_ucDefaultTextColour_0049cb7c,
                    "\nNotes\n");
    DrawNavTextLine(0, g_ucDefaultTextColour_0049cb7c,
                    "%s\n",
                    nav_note((short)g_cCurrentObjective_004931cc));
    if (showFlightData != 0)
        DrawNavMapLegend();
    BuildMap(showFlightData);
    if (showFlightData != 0) {
        SetScreenClipRect(0, 0, 259, 155);
        SetTextContext(&g_stNavMapTextCtx_005d16d0);
        DrawNavTextLine(0, g_ucDefaultTextColour_0049cb7c,
                        "%X%Y                         Location: %d.%d.%d",
                        8, 142,
                        g_aShipPosition_00494550[0].x >> 8,
                        g_aShipPosition_00494550[0].y >> 8,
                        g_aShipPosition_00494550[0].z >> 8);
    }
    CopyViewportContents(&g_stSecondaryViewBufferBlock_005d2c90, &g_stScrViewInfo_005d21a0);
}

/* Function start: 0x451AD3 */
void BriefingMap_LoadShapes(void)
{
    short objective;

    g_pNavMapShape_0049bc48 =
        FetchDiskPacketRetrying("cockpit.vga", 2, 0);
    SetScreenClipRect(0, 0, 259, 155);
    if (AllocateViewport(&g_stSecondaryViewBufferBlock_005d2c90, (short)g_cSecondaryViewBufferColour_0049cb4c, 0) == 0)
        ReportOutOfMemoryAndExit("VSCREEN");
    for (objective = 0; objective < (short)g_cMissionObjectiveCount_00493294;
         objective++)
        LocateMobileObjective(objective);
}

/* Function start: 0x451B70 */
void BriefingMap_DisplayMap(void)
{
    short font;
    Viewport savedViewport;

    savedViewport = g_stSecondaryViewBufferBlock_005d2c90;
    free_viewport(&g_stSecondaryViewBufferBlock_005d2c90);
    BriefingMap_LoadShapes();
    g_stScrViewInfo_005d21a0.top = 4;
    DrawNavLocationReadout("Briefing Nav Map", 0);
    free_viewport(&g_stSecondaryViewBufferBlock_005d2c90);
    ReleasePacketHandle(g_pNavMapShape_0049bc48);
    ReleasePacketSlot((void **)&g_pNavLocationText_0049bc54);
    for (font = 0; font < 4; font++) {
        if (g_apTextGlyphs_005d2200[font] != 0) {
            ReleasePacketHandle(g_apTextGlyphs_005d2200[font]);
            g_apTextGlyphs_005d2200[font] = 0;
        }
    }
    g_stSecondaryViewBufferBlock_005d2c90 = savedViewport;
    AllocateViewport(&g_stSecondaryViewBufferBlock_005d2c90, (short)g_cSecondaryViewBufferColour_0049cb4c, 0);
}

#pragma function(abs)
/* Function start: 0x451C5C */
short SelectNavObjectiveAtPoint(short mouseX, short mouseY)
{
    short mapX;
    short mapY;
    short objectiveFound;
    short step;
    short distance;
    short previousNav;

    objectiveFound = 0;
    previousNav = (short)g_cCurrentNavPointIndex_00493298;
    mouseX = (short)(mouseX - 30);
    mouseY = (short)(mouseY - 22);
    step = 0;
    while (g_abFlightPath_004932a0[step] != -1 && step < 8) {
        if (hidden_objective(g_abFlightPath_004932a0[step]) == 0 &&
            IsObjectiveInDisplayedSystem(
                g_abFlightPath_004932a0[step]) != 0) {
            ScaleNavMapCoordinates(
                &mapX, &mapY,
                g_aMissionObjectives_004932a8[
                    g_abFlightPath_004932a0[step]].mapX,
                g_aMissionObjectives_004932a8[
                    g_abFlightPath_004932a0[step]].mapY);
            distance = (short)(abs((int)mouseY - mapY) +
                               abs((int)mouseX - mapX));
            if (distance < 6 ||
                IsPointInNavMapLabel(
                    (short)g_awNavObjectiveLabelIndex_005d16b0[step],
                    mouseX, mouseY) != 0) {
                objectiveFound = 1;
                set_new_objective(step);
                if (previousNav == step)
                    return objectiveFound;
            }
        }
        step++;
    }
    return objectiveFound;
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
    SetRectBounds(&g_stScrViewInfo_005d21a0, 30, 22, 289, 177);
    if (refresh != 0)
        DisableMouseCursorDrawing();
    DrawNavLocationReadout("ConFed Nav Scan", 1);
    if (refresh != 0)
        EnableMouseCursorDrawing();
    SetRectBounds(&g_stScrViewInfo_005d21a0, 0, 0, 319, 199);
}

/* Function start: 0x451EC9 */
/* Follow the selected objective's system, unless the map is still showing a
 * different system from the one the player is in. */
void FollowSelectedObjectiveSystem(void)
{
    short navPoint;

    if (g_stElapsedCampaignDate_005d170c.year != g_nCurStellarSystem_005d169c) {
        g_stElapsedCampaignDate_005d170c.year = g_nCurStellarSystem_005d169c;
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
    Viewport navViewport;
    InputEventState inputEvent;
    unsigned char *cockpitShape;
    int inputType;
    int frame;
    int markerColor;
    short done;
    short selectedNav;
    short objectivesVisible;
    short savedNav;
    short objective;

    done = 0;
    selectedNav = (short)g_cCurrentNavPointIndex_00493298;
    objectivesVisible = 0;
    savedNav = (short)g_cCurrentNavPointIndex_00493298;
    g_bInflightComputerActive_0049bcb4 = 1;
    savedCursor = g_stPersonnelCursor_005c8470;

    if (message_showing() != 0)
        EndCommMenu();
    DisableMouseCursorDrawing();
    g_nUiInputMode_005c8d3c = 0;
    free_view_buffer();
    g_cScreenViewportMode_005c82a6 = -1;
    cockpitShape = FetchDiskPacketRetrying("cockpit.vga", 1, 0);
    ClearViewport(&g_stScrViewInfo_005d21a0,
                  g_cSecondaryViewBufferColour_0049cb4c);
    DrawSpriteDefault(&g_stScrViewInfo_005d21a0, 0, 0, cockpitShape, 0);
    ReleasePacketHandle(cockpitShape);
    SetInputViewport(&g_stScrViewInfo_005d21a0);
    g_stElapsedCampaignDate_005d170c.year = g_nCurStellarSystem_005d169c;
    BriefingMap_LoadShapes();
    ShowConfedNavScan(0);

    for (objective = 0;
         objective < (short)g_cMissionObjectiveCount_00493294;
         objective++) {
        if (hidden_objective(objective) == 0 &&
            IsObjectiveInDisplayedSystem(objective) != 0) {
            objectivesVisible = 1;
            break;
        }
    }
    MarkDibDirty();
    DIBslamReal();

    if (objectivesVisible == 0) {
        ConfigureInputPump(1, PollJoystickButtonEvents);
        WaitForAnyInputPress();
        SetFrameTimerAndWait(20);
        ConfigureInputPump(1, get_player_input);
    } else {
        navViewport = g_stScrViewInfo_005d21a0;
        SetRectBounds(&navViewport, 0x20, 0x18, 0xb6, 0x9f);
        SetInputViewport(&navViewport);
        SetMenuInputPump();
        g_nInputRepeatDelay_005c80d6 = 10;
        g_bInputCursorEnabled_005c80e6 = 6;
        CentreMouseOnCurrentNavObjective();

        do {
            if ((short)g_cCurrentNavPointIndex_00493298 !=
                selectedNav) {
                selectedNav =
                    (short)g_cCurrentNavPointIndex_00493298;
                PlaySfxWaveFileByNumber(0x19, -1, 0);
                ShowConfedNavScan(1);
            }
            SetRectBounds(&g_stScrViewInfo_005d21a0, 0x20, 0x18,
                          0x121, 0xb1);
            if (g_pElapsedCampaignDatePointer_005d3e8c == 0)
                g_pElapsedCampaignDatePointer_005d3e8c =
                    &g_stElapsedCampaignDate_005d170c;
            FormatNavCoordinates(
                (unsigned char *)g_pElapsedCampaignDatePointer_005d3e8c);
            frame = g_nInputClock_005c84a8 / 15;
            g_stNavLabelTextContextInfo_005d16f0.viewport =
                &g_stScrViewInfo_005d21a0;
            if (frame % 2 != 0)
                markerColor = g_bPrimaryViewBufferColour_0049cb50;
            else
                markerColor = g_ucHomeCarrierScannerColour_0049cb70;
            DrawNavPlayerMarker((short)markerColor, 0);
            g_stNavMapTextCtx_005d16d0.viewport =
                &g_stScrViewInfo_005d21a0;
            UpdateInflightNavText((short)((frame / 4) % 2));
            SetRectBounds(&g_stScrViewInfo_005d21a0, 0, 0, 0x13f, 0xc7);
            SetMouseCursorShape(
                g_pInputManagerState_005c8464->cursorShape, 0);
            g_nUiInputMode_005c8d3c = 1;
            EnableMouseCursorDrawing();
            /* The original passes a second argument the callee ignores. */
            inputType = PollInputEvent(&inputEvent, 0xf);
            switch (inputType) {
            case 1:
                done = 1;
                break;
            case 4:
            case 6:
                if ((short)inputEvent.value == 0x1c ||
                    (short)inputEvent.value == 0x39) {
                    done = 1;
                } else if ((short)inputEvent.value == 0x31) {
                    cycle_next_objective();
                    if (IsObjectiveInDisplayedSystem(
                            (short)g_cCurrentObjective_004931cc) == 0)
                        FollowSelectedObjectiveSystem();
                    CentreMouseOnCurrentNavObjective();
                }
                break;
            }
            SelectNavObjectiveAtPoint(
                (short)g_stHostMouseTextBlock_005d10d0.x,
                (short)g_stHostMouseTextBlock_005d10d0.y);
            MarkDibDirty();
            DIBslamReal();
        } while (done == 0 && g_bSceneEscapeRequested_0049d4b0 == 0);

        if (g_bSceneEscapeRequested_0049d4b0 != 0) {
            g_bSceneEscapeRequested_0049d4b0 = 0;
            g_cCurrentNavPointIndex_00493298 =
                (signed char)savedNav;
            set_new_objective((short)g_cCurrentNavPointIndex_00493298);
        }
        free_viewport(&g_stSecondaryViewBufferBlock_005d2c90);
        DisableMouseCursorDrawing();
        ReleasePacketHandle(g_pNavMapShape_0049bc48);
        SetTextContext(&g_stDefaultStringContext_005d2d20);
        PlaySfxWaveFileByNumber(0x19, -1, 0);
    }

    g_stPersonnelCursor_005c8470 = savedCursor;
    SetPersonnelMousePosition(g_stPersonnelCursor_005c8470.x,
                              g_stPersonnelCursor_005c8470.y);
    force_view(0, 0);
    InitializeSpaceFlightInput();
    g_bInflightComputerActive_0049bcb4 = 0;
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short g_nCurStellarSystem_005d169c;
short g_nNavPlotMaxX_005d169e;
short g_nNavmapMapMinimumX_005d16a0;
unsigned short g_awNavObjectiveLabelIndex_005d16b0[8];
short g_nNavmapMapMaximumY_005d16c0;
short g_nNavmapMapCentreX_005d16c2;
short g_nNavMapMinimumYValue_005d16c4;
TextContext g_stNavMapTextCtx_005d16d0;
short g_nNavMapCentreY_005d16ec;
TextContext g_stNavLabelTextContextInfo_005d16f0;
CampaignDate g_stElapsedCampaignDate_005d170c;
void *g_pDigiData_005d1710;
unsigned short g_wDigiLeft_005d1714;
