/*
 *  Navigation map.
 *
 *  Retail object 0x44FA70-0x45244F (link order; see docs/ORDER.md).
 */
/* This unit owns storage declared in globals.h as statics. */
#define NAV_C
#include "game.h"

short g_nNavMapCoordinateScaling = 0; /* 0x0049bb80 */
short g_nNavMapScale = 1; /* 0x0049bb84 */
NavMapObjectiveStyle g_aNavMapObjectiveStyles[12] = {
    { 1, 2, &g_ucPrimaryTextColour,
      &g_ucDefaultTextColour,
      &g_ucDefaultTextColour },
    { 3, 2, &g_cSecondaryViewBufferColour,
      &g_bPrimaryViewBufferColour,
      &g_ucDefaultTextColour },
    { 4, 2, &g_ucNavObjectiveMarkerColour,
      &g_ucNavObjectiveMarkerColour,
      &g_ucDefaultTextColour },
    { 2, 3, &g_ucNavObjectiveMarkerColour,
      &g_ucDefaultTextColour,
      &g_ucDefaultTextColour },
    { 2, 3, &g_cRedColour,
      &g_cRedColour,
      &g_ucDefaultTextColour },
    { 1, 2, &g_ucPrimaryTextColour,
      &g_ucDefaultTextColour,
      &g_ucDefaultTextColour },
    { 4, 2, &g_ucNavObjectiveMarkerColour,
      &g_ucNavObjectiveMarkerColour,
      &g_ucDefaultTextColour },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 }
}; /* 0x0049bb88 */
unsigned char *g_pNavMapShape = 0; /* 0x0049bc48 */
short g_nNavMapLabelCount = 0; /* 0x0049bc4c */
short g_nNavMapReservedAreaCount = 0; /* 0x0049bc50 */
unsigned char *g_pNavLocationText = 0; /* 0x0049bc54 */
char *g_apszShipMissionTypeNames[14] = {
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
}; /* 0x0049bc58 */
char g_szNoFurtherObjectives[32] =
    "Further objectives out-system."; /* 0x0049bc90 */
short g_bBriefingMapActive = 0; /* 0x0049bcb0 */
int g_bInflightComputerActive = 0; /* 0x0049bcb4 */

/* NavMapLabelFits tries a placement before TryPlaceNavMapLabel is defined. */
short TryPlaceNavMapLabel(short x, short y, short width, short force);

/* Function start: 0x44FA70 */
void DrawNavTextLine(unsigned char alignment, unsigned short colour,
                     const char *format, ...)
{
    va_list arguments;

    g_29_pCurrentTextContext->colour = colour;
    g_29_pCurrentTextContext->alignment = alignment;
    va_start(arguments, format);
    g_29_pCurrentTextContext->textCursor =
        g_29_pCurrentTextContext->text;
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
    DrawTextString(g_29_pCurrentTextContext->text);
}

/* Function start: 0x44FADA */
void SetNavMapCoordinateScaling(short enabled)
{
    g_nNavMapCoordinateScaling = enabled;
}

/* Function start: 0x44FAEF */
void ScaleNavMapMarkerSize(short *size)
{
    *size = (short)(*size / (g_nNavMapScale * 100));
}

/* Function start: 0x44FB19 */
void ScaleNavMapCoordinates(short *x, short *y, short mapX, short mapY)
{
    *x = (short)((mapX - g_13_nNavmapMapCentreX) /
                 g_nNavMapScale + 75);
    *y = (short)(67 - (mapY - g_nNavMapCentreY) /
                 g_nNavMapScale);
}

/* Function start: 0x44FB68 */
void nav_getxy(short *x, short *y, int worldX, int worldZ)
{
    *x = (short)((worldX / 100) >> 8);
    *y = (short)((worldZ / 100) >> 8);
    if (g_nNavMapCoordinateScaling != 0)
        ScaleNavMapCoordinates(x, y, *x, *y);
}

/* Function start: 0x44FBC8 */
void CheckPoint(short x, short y)
{
    g_nNavmapMapMinimumX =
        MinShort(g_nNavmapMapMinimumX, x);
    g_nNavPlotMaxX =
        MaxShort(g_nNavPlotMaxX, x);
    g_2_nNavMapMinimumYValue =
        MinShort(g_2_nNavMapMinimumYValue, y);
    g_nNavmapMapMaximumY =
        MaxShort(g_nNavmapMapMaximumY, y);
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
    g_nNavmapMapMinimumX = g_aMissionObjectives[0].mapX;
    g_nNavPlotMaxX = g_aMissionObjectives[0].mapX;
    g_2_nNavMapMinimumYValue = g_aMissionObjectives[0].mapY;
    g_nNavmapMapMaximumY = g_aMissionObjectives[0].mapY;
    for (objectiveIndex = 0;
         objectiveIndex < (short)g_cMissionObjectiveCount;
         objectiveIndex++) {
        if (mobile_objective(objectiveIndex) != 0) {
            obj = find_ship_index((short)
                g_aMissionObjectives[objectiveIndex].index);
            if (obj != -1)
                g_aMissionObjectives[objectiveIndex].position =
                    g_aShipPosition[obj];
        }
        nav_getxy(
            &g_aMissionObjectives[objectiveIndex].mapX,
            &g_aMissionObjectives[objectiveIndex].mapY,
            g_aMissionObjectives[objectiveIndex].position.x,
            g_aMissionObjectives[objectiveIndex].position.z);
        CheckPoint(g_aMissionObjectives[objectiveIndex].mapX,
                   g_aMissionObjectives[objectiveIndex].mapY);
    }
    IncludeNavMapWorldPoint(g_aShipPosition[0].x,
                            g_aShipPosition[0].z);
    mapWidth = (short)(g_nNavPlotMaxX -
                    g_nNavmapMapMinimumX);
    mapHeight = (short)(g_nNavmapMapMaximumY -
                     g_2_nNavMapMinimumYValue);
    g_13_nNavmapMapCentreX =
        (short)(mapWidth / 2 + g_nNavmapMapMinimumX);
    g_nNavMapCentreY =
        (short)(mapHeight / 2 + g_2_nNavMapMinimumYValue);
    g_nNavMapScale = MaxShort(
        (short)((mapWidth / 2 + mapWidth) / 150),
        (short)((mapHeight / 2 + mapHeight) / 135));
    if (g_nNavMapScale == 0)
        g_nNavMapScale = 100;
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
    g_cMissionObjectiveCount = 0;
    flightPathCount = 0;
    record = g_69_aMissionObjectiveSources;
#ifdef SDL_PORT
    /* Retail reads the next word after the eight-record table before checking
     * that all eight output slots are full.  That word is the adjacent mission
     * header and cannot affect the loop body, so bound the native read first. */
    while (g_cMissionObjectiveCount < 8 &&
           (type = record->type) != -1) {
#else
    while ((type = record->type) != -1 &&
           g_cMissionObjectiveCount < 8) {
#endif
        i = record->index;
        g_aMissionObjectives[
            g_cMissionObjectiveCount].flags = 0;
        switch (type) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 6:
            text = (const char *)&g_aMissionShips[i];
            set_sphere_point(&g_aMissionShips[i], &location);
            g_abFlightPath[flightPathCount++] =
                g_cMissionObjectiveCount;
            break;
        case 0:
        case 5:
            text =
                (const char *)&g_aMissionNavPoints[i];
            location = g_aMissionNavPoints[i].position;
            g_abFlightPath[flightPathCount++] =
                g_cMissionObjectiveCount;
            break;
        }

        g_aMissionObjectives[
            g_cMissionObjectiveCount].type = type;
        g_aMissionObjectives[
            g_cMissionObjectiveCount].index = record->index;
        g_aMissionObjectives[
            g_cMissionObjectiveCount].field_8 = -1;
        g_aMissionObjectives[
            g_cMissionObjectiveCount].name = record->description;
        g_aMissionObjectives[
            g_cMissionObjectiveCount].position = location;
        nav_getxy(
            &g_aMissionObjectives[
                g_cMissionObjectiveCount].mapX,
            &g_aMissionObjectives[
                g_cMissionObjectiveCount].mapY,
            location.x, location.z);
        g_aMissionObjectives[
            g_cMissionObjectiveCount].displayName = text;
        g_cMissionObjectiveCount++;
        record++;
    }

    g_abFlightPath[flightPathCount] = -1;
    g_cCurrentNavPointIndex = 0;
    g_cCurrentObjective = 0;
    if (g_cMissionObjectiveCount != 0) {
        while (set_new_objective(
                   (short)g_cCurrentNavPointIndex) == 0)
            g_cCurrentNavPointIndex++;
    }
}

/* Function start: 0x450136 */
short NavMapPointInsideReservedArea(short area, short x, short y)
{
    return g_aNavMapExclusionRects[area].left <= x &&
           g_aNavMapExclusionRects[area].right >= x &&
           g_aNavMapExclusionRects[area].top <= y &&
           g_aNavMapExclusionRects[area].bottom >= y;
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
             rect < g_nNavMapReservedAreaCount && clear != 0;
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
    g_nNavMapReservedAreaCount = 0;
}

/* Function start: 0x45033D */
void ReserveNavMapArea(short x, short y, short width, short height)
{
    g_aNavMapExclusionRects[
        g_nNavMapReservedAreaCount].left = x;
    g_aNavMapExclusionRects[
        g_nNavMapReservedAreaCount].top = y;
    g_aNavMapExclusionRects[
        g_nNavMapReservedAreaCount].right = (short)(x + width);
    g_aNavMapExclusionRects[
        g_nNavMapReservedAreaCount].bottom = (short)(y + height);
    g_nNavMapReservedAreaCount++;
}

/* Function start: 0x4503A7 */
void ResetNavMapLabels(void)
{
    g_nNavMapLabelCount = 0;
}

/* Function start: 0x4503BB */
short TryPlaceNavMapLabel(short x, short y, short width, short force)
{
    short placed;

    placed = 0;
    if (NavMapLabelPositionAvailable(x, y, width, 6) != 0 ||
        (force != 0 && NavMapLabelFits(x, y, width, 6) != 0)) {
        g_1_aNavMapLabels[g_nNavMapLabelCount].x = x;
        g_1_aNavMapLabels[g_nNavMapLabelCount].y = y;
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
    g_1_aNavMapLabels[
        g_nNavMapLabelCount].colour = colour;
    g_1_aNavMapLabels[
        g_nNavMapLabelCount].text = text;
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
        g_1_aNavMapLabels[g_nNavMapLabelCount].x,
        g_1_aNavMapLabels[g_nNavMapLabelCount].y,
        textWidth, 6);
    g_nNavMapLabelCount++;
}

/* Function start: 0x450623 */
/* Two objectives share a nav label when both are jump-point kinds (type 0 or
 * 5) or neither is. */
short ObjectivesShareNavLabel(short objective, short other)
{
    switch (g_aMissionObjectives[objective].type) {
    case 0:
    case 5:
        switch (g_aMissionObjectives[other].type) {
        case 0:
        case 5:
            return 1;
        default:
            return 0;
        }
    default:
        switch (g_aMissionObjectives[other].type) {
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
        else if (g_aMissionObjectives[previous].index ==
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
    return g_1_aNavMapLabels[labelIndex].x <= x &&
           (unsigned int)x <=
               (unsigned int)(g_1_aNavMapLabels[labelIndex].x +
                              strlen(g_1_aNavMapLabels[labelIndex].text) * 4) &&
           g_1_aNavMapLabels[labelIndex].y <= y &&
           y <= g_1_aNavMapLabels[labelIndex].y + 6;
}

#pragma intrinsic(strlen)

void DrawViewportLine(Viewport *viewport, short x1, short y1, short x2, short y2, short colour);
void DrawViewportBorder(Viewport *viewport, short left, short top, short right, short bottom, short colour);

/* Function start: 0x45087F */
void DrawNavMapLabels(void)
{
    short label;

    for (label = 0; label < (short)g_nNavMapLabelCount; label++) {
        DrawFormattedText("%X%Y%F%s",
                          g_1_aNavMapLabels[label].x,
                          g_1_aNavMapLabels[label].y,
                          g_1_aNavMapLabels[label].colour,
                          g_1_aNavMapLabels[label].text);
    }
}

/* Function start: 0x450900 */
void DrawNavRectangleMarker(short x, short y, short size, short shadow,
                            unsigned short colour, short reserve)
{
    if (shadow == 0)
        DrawViewportEllipse(g_5_stNavLabelTextContextInfo.viewport,
                            x, y, size, (short)((size * 7) / 8),
                            (short)colour);
    else
        DrawViewportEllipseShadow(
            g_5_stNavLabelTextContextInfo.viewport,
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
        DrawViewportPixel(g_5_stNavLabelTextContextInfo.viewport,
                          x, y, colour);
        DrawViewportPixel(g_5_stNavLabelTextContextInfo.viewport,
                          (short)(x + 1), y, colour);
        DrawViewportPixel(g_5_stNavLabelTextContextInfo.viewport,
                          x, (short)(y + 1), colour);
        DrawViewportPixel(g_5_stNavLabelTextContextInfo.viewport,
                          (short)(x + 1),
                          (short)(y + 1), colour);
    } else {
        DrawNavSquareOutline(g_5_stNavLabelTextContextInfo.viewport,
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
    DrawNavTriangleOutline(g_5_stNavLabelTextContextInfo.viewport,
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
    DrawViewportLine(g_5_stNavLabelTextContextInfo.viewport,
                     (short)(x - size), (short)(y - size),
                     (short)(x + size), (short)(y + size),
                     (short)colour);
    DrawViewportLine(g_5_stNavLabelTextContextInfo.viewport,
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

    switch (g_aMissionObjectives[objective].type) {
    case 0:
    case 5:
        if (g_aMissionNavPoints[
                g_aMissionObjectives[objective].index]
                    .systemIndex == g_1_stElapsedCampaignDate.year)
            return 1;
        else
            return 0;
    default:
        navPoint = find_ships_sphere(
            g_aMissionObjectives[objective].index);
        if (navPoint == -1)
            return 1;
        else if (g_aMissionNavPoints[navPoint].systemIndex ==
                 g_1_stElapsedCampaignDate.year)
            return 1;
        else
            return 0;
    }
}

/* Function start: 0x450DF6 */
void SetScreenClipRect(unsigned short a, unsigned short b,
                       unsigned short c, unsigned short d)
{
    SetRectBounds(&g_stSecondaryViewBufferBlock, a, b, c, d);
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

    nav_getxy(&x, &y, g_aShipPosition[0].x,
              g_aShipPosition[0].z);
    x = (short)(x + g_5_stNavLabelTextContextInfo.viewport->left);
    y = (short)(y + g_5_stNavLabelTextContextInfo.viewport->top);
    DrawViewportPixel(g_5_stNavLabelTextContextInfo.viewport,
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

    beaconRecord = (const NavMapBeacon *)g_abMissionBriefingData;
    while (beaconRecord->active != 0) {
        nav_getxy(&navX, &navY, beaconRecord->position.x,
                  beaconRecord->position.z);
        ScaleNavMapCoordinates(&gridX, &gridY, navX, navY);
        DrawViewportPixel(&g_stSecondaryViewBufferBlock, gridX, gridY,
                          g_ucNavObjectiveMarkerColour);
        DrawNavSquareMarker(gridX, gridY, 2, 0,
                            g_ucPrimaryTextColour, 1);
        AddUniqueObjectiveNavLabel(gridX, gridY,
                                   g_ucDefaultTextColour,
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
    DrawSpriteDefault(&g_stSecondaryViewBufferBlock, 1, 1,
                      g_pNavMapShape, 0);
    SetScreenClipRect(2, 2, 152, 137);
    g_5_stNavLabelTextContextInfo.viewport =
        &g_stSecondaryViewBufferBlock;
    g_5_stNavLabelTextContextInfo.text = g_6_szDefaultTextBufferText;
    InitializeTextContextFromFont(&g_5_stNavLabelTextContextInfo,
                                  2, g_ucPrimaryTextColour, -1);
    g_5_stNavLabelTextContextInfo.alignment = 0;
    SetTextContext(&g_5_stNavLabelTextContextInfo);
    nav = g_aMissionNavPoints;
    ResetNavMapLabels();
    ResetNavMapReservedAreas();
    SetScale();
    DrawNavMapBriefingBeacons();

    for (checkpoint = 0; checkpoint < 10; checkpoint++) {
        if (g_aMissionNavPoints[checkpoint].type != 0 &&
            g_aMissionNavPoints[checkpoint].systemIndex ==
                g_1_stElapsedCampaignDate.year) {
            for (index = 0; index < 10; index++) {
                if (nav->missionShips[index] != -1) {
                    ship = &g_aMissionShips[
                        nav->missionShips[index]];
                    /* WC2 hazard field types; the image names neither. */
                    switch (ship->objectType) {
                    case 5:
                        DrawNavHazardMarker(
                            nav->position, ship->position,
                            ship->speed,
                            g_ucAsteroidNavMarkerColour,
                            g_ucAsteroidNavMarkerColour,
                            "Asteroids");
                        break;
                    case 6:
                        DrawNavHazardMarker(
                            nav->position, ship->position,
                            ship->speed,
                            g_cRedColour,
                            g_cRedColour,
                            "Mines");
                        break;
                    }
                }
            }
        }
        nav++;
    }

    for (index = 0;
         g_cMissionObjectiveCount > index;
         index++) {
        if (mobile_objective(index) == 0 ||
            (g_aMissionShips[
                 (signed char)g_aMissionObjectives[
                     index].index].state == 0 &&
             achieved(index) == 0)) {
        ScaleNavMapCoordinates(
            &x, &y, g_aMissionObjectives[index].mapX,
            g_aMissionObjectives[index].mapY);
        if (hidden_objective(index) == 0 &&
            IsObjectiveInDisplayedSystem(index) != 0) {
        mark = &g_aNavMapObjectiveStyles[
            g_aMissionObjectives[index].type];
        if (visited(index) == 0)
            DrawViewportPixel(&g_stSecondaryViewBufferBlock, x, y,
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
        if (g_cCurrentObjective == index)
            nameColour = g_cYellowColour;
        else
            nameColour = *mark->labelColour;
        g_awNavObjectiveLabelIndex[index] =
            g_nNavMapLabelCount;
        AddUniqueObjectiveNavLabel(
            x, y, nameColour, objective_name(index),
            index,
            (short)g_aMissionObjectives[index].index);
        }
        }
    }

    if (showPlayer != 0 &&
        g_1_stElapsedCampaignDate.year ==
            g_nCurStellarSystem) {
        DrawNavPlayerMarker(g_bPrimaryViewBufferColour, 1);
        nav_getxy(&x, &y, g_aShipPosition[0].x,
                  g_aShipPosition[0].z);
        PlaceNavMapLabel(
            x, y, g_ucPlayerNavLabelColour,
            g_stCurrentPilotProfile.callsign);
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

    SetTextContext(&g_stNavMapTextCtx);
    if (g_4_pElapsedCampaignDatePointer == 0)
        g_4_pElapsedCampaignDatePointer =
            &g_1_stElapsedCampaignDate;
    sprintf(time, "%02d",
            (int)((signed char *)g_4_pElapsedCampaignDatePointer)[0]);
    DrawFormattedText(
        "%X%YStandard time %s",
        g_stNavMapTextCtx.viewport->left + 152,
        g_stNavMapTextCtx.viewport->top + 140,
        time);
    savedX = g_stNavMapTextCtx.cursorX;
    if (showColon == 0) {
        DrawFormattedText(" ");
    } else {
        g_stNavMapTextCtx.cursorX =
            (short)(g_stNavMapTextCtx.cursorX + 4);
    }
    sprintf(time, "%02d  ",
            (int)((signed char *)g_4_pElapsedCampaignDatePointer)[1]);
    DrawFormattedText(time);
    g_stNavMapTextCtx.cursorX = savedX;
    if (showColon != 0)
        DrawFormattedText(":");
}

/* Function start: 0x451637 */
void FormatNavCoordinates(unsigned char *out)
{
    unsigned char tmp[4];

    if (out == 0)
        out = (unsigned char *)&g_1_stElapsedCampaignDate;
    SplitGameClockTicksForNav(tmp);
    out[1] = tmp[2];
    out[0] = tmp[3];
}

/* Function start: 0x451673 */
void DrawSelectedNavLegendEntry(const char *text, short navPoint)
{
    if ((short)g_cCurrentNavPointIndex == navPoint) {
        DrawNavTextLine(0, (unsigned short)g_cYellowColour, text);
        DrawNavTextLine(0, (unsigned short)g_cYellowColour,
                        "\n");
    }
}

/* Function start: 0x4516C0 */
void DrawNavMapLegend(void)
{
    short objective;

    objective = 0;
    while (objective < (short)g_cMissionObjectiveCount &&
           (visited(objective) != 0 || hidden_objective(objective) != 0))
        objective++;
    SetTextCursor((unsigned short)g_stSecondaryViewBufferBlock.left, 120);
    DrawSelectedNavLegendEntry("MISSION FLIGHT PATH",
                               objective);
    DrawSelectedNavLegendEntry(
        "HOME BASE",
        (short)((short)g_cMissionObjectiveCount - 1));
}

/* Function start: 0x451756 */
char *nav_note(short objective)
{
    if (IsObjectiveInDisplayedSystem(objective) == 0)
        return g_szNoFurtherObjectives;
    if (*g_aMissionObjectives[objective].name == '?')
        return g_aMissionObjectives[objective].name + 1;
    else if (*g_aMissionObjectives[objective].name == '.')
        return strchr(g_aMissionObjectives[objective].name, 0);
    else
        return g_aMissionObjectives[objective].name;
}

/* Function start: 0x45180F */
void DrawNavLocationReadout(const char *title, short showFlightData)
{
    char fileName[11] = "sector.000";
    short file;
    short length;

    ClearViewport(&g_stSecondaryViewBufferBlock, g_cSecondaryViewBufferColour);
    SetScreenClipRect(155, 2, 259, 155);
    g_stNavMapTextCtx.viewport = &g_stSecondaryViewBufferBlock;
    g_stNavMapTextCtx.text = g_6_szDefaultTextBufferText;
    InitializeTextContextFromFont(&g_stNavMapTextCtx,
                                  1, g_ucPrimaryTextColour, g_cSecondaryViewBufferColour);
    g_stNavMapTextCtx.alignment = 0;
    g_stNavMapTextCtx.cursorX = 0;
    g_stNavMapTextCtx.cursorY = 0;
    SetTextContext(&g_stNavMapTextCtx);
    DrawNavTextLine(0, g_ucDefaultTextColour,
                    "\n");
    DrawNavTextLine(2, g_ucDefaultTextColour,
                    "%s\n\n", title);
    if (g_pNavLocationText == 0) {
        fileName[9] =
            (char)(g_nSelectedCampaignSlot + '0');
        file = OpenDataFileOrDie(fileName);
        length = (short)filelength(file);
        g_pNavLocationText =
            AllocateTaggedMemory(length + 1, 0x40);
        ReadDataFileAtOffset((unsigned short)file, 0, length,
                             g_pNavLocationText);
        g_pNavLocationText[length] = 0;
    }
    DrawNavTextLine(0, g_ucDefaultTextColour,
                    "Sector: %s\n",
                    (char *)g_pNavLocationText);
    DrawNavTextLine(0, g_ucDefaultTextColour,
                    "System: %s\n\n",
                    g_abSeriesAuxData);
    DrawNavTextLine(2, g_ucDefaultTextColour,
                    "* %s *\n",
                    g_5_abMissionAuxBlockBytes);
    DrawNavTextLine(2, g_ucDefaultTextColour,
                    "* %s *\n",
                    g_apszShipMissionTypeNames[
                        g_aMissionShips[
                            g_13_stMissionHeaderBlock
                                .playerMissionShip].missionType]);
    DrawNavTextLine(2, g_ucDefaultTextColour,
                    "\nNotes\n");
    DrawNavTextLine(0, g_ucDefaultTextColour,
                    "%s\n",
                    nav_note((short)g_cCurrentObjective));
    if (showFlightData != 0)
        DrawNavMapLegend();
    BuildMap(showFlightData);
    if (showFlightData != 0) {
        SetScreenClipRect(0, 0, 259, 155);
        SetTextContext(&g_stNavMapTextCtx);
        DrawNavTextLine(0, g_ucDefaultTextColour,
                        "%X%Y                         Location: %d.%d.%d",
                        8, 142,
                        g_aShipPosition[0].x >> 8,
                        g_aShipPosition[0].y >> 8,
                        g_aShipPosition[0].z >> 8);
    }
    CopyViewportContents(&g_stSecondaryViewBufferBlock, &g_101_stScrViewInfo);
}

/* Function start: 0x451AD3 */
void BriefingMap_LoadShapes(void)
{
    short objective;

    g_pNavMapShape =
        FetchDiskPacketRetrying("cockpit.vga", 2, 0);
    SetScreenClipRect(0, 0, 259, 155);
    if (AllocateViewport(&g_stSecondaryViewBufferBlock, (short)g_cSecondaryViewBufferColour, 0) == 0)
        ReportOutOfMemoryAndExit("VSCREEN");
    for (objective = 0; objective < (short)g_cMissionObjectiveCount;
         objective++)
        LocateMobileObjective(objective);
}

/* Function start: 0x451B70 */
void BriefingMap_DisplayMap(void)
{
    short font;
    Viewport savedViewport;

    savedViewport = g_stSecondaryViewBufferBlock;
    free_viewport(&g_stSecondaryViewBufferBlock);
    BriefingMap_LoadShapes();
    g_101_stScrViewInfo.top = 4;
    DrawNavLocationReadout("Briefing Nav Map", 0);
    free_viewport(&g_stSecondaryViewBufferBlock);
    ReleasePacketHandle(g_pNavMapShape);
    ReleasePacketSlot((void **)&g_pNavLocationText);
    for (font = 0; font < 4; font++) {
        if (g_36_apTextGlyphs[font] != 0) {
            ReleasePacketHandle(g_36_apTextGlyphs[font]);
            g_36_apTextGlyphs[font] = 0;
        }
    }
    g_stSecondaryViewBufferBlock = savedViewport;
    AllocateViewport(&g_stSecondaryViewBufferBlock, (short)g_cSecondaryViewBufferColour, 0);
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
    previousNav = (short)g_cCurrentNavPointIndex;
    mouseX = (short)(mouseX - 30);
    mouseY = (short)(mouseY - 22);
    step = 0;
    while (g_abFlightPath[step] != -1 && step < 8) {
        if (hidden_objective(g_abFlightPath[step]) == 0 &&
            IsObjectiveInDisplayedSystem(
                g_abFlightPath[step]) != 0) {
            ScaleNavMapCoordinates(
                &mapX, &mapY,
                g_aMissionObjectives[
                    g_abFlightPath[step]].mapX,
                g_aMissionObjectives[
                    g_abFlightPath[step]].mapY);
            distance = (short)(abs((int)mouseY - mapY) +
                               abs((int)mouseX - mapX));
            if (distance < 6 ||
                IsPointInNavMapLabel(
                    (short)g_awNavObjectiveLabelIndex[step],
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
        g_aMissionObjectives[
            g_abFlightPath[
                g_cCurrentNavPointIndex]].mapX,
        g_aMissionObjectives[
            g_abFlightPath[
                g_cCurrentNavPointIndex]].mapY);
    x = (short)(x + 30);
    y = (short)(y + 22);
    SetPersonnelMousePosition(x, y);
}

/* Function start: 0x451E57 */
void ShowConfedNavScan(short refresh)
{
    SetRectBounds(&g_101_stScrViewInfo, 30, 22, 289, 177);
    if (refresh != 0)
        DisableMouseCursorDrawing();
    DrawNavLocationReadout("ConFed Nav Scan", 1);
    if (refresh != 0)
        EnableMouseCursorDrawing();
    SetRectBounds(&g_101_stScrViewInfo, 0, 0, 319, 199);
}

/* Function start: 0x451EC9 */
/* Follow the selected objective's system, unless the map is still showing a
 * different system from the one the player is in. */
void FollowSelectedObjectiveSystem(void)
{
    short navPoint;

    if (g_1_stElapsedCampaignDate.year != g_nCurStellarSystem) {
        g_1_stElapsedCampaignDate.year = g_nCurStellarSystem;
        return;
    }
    switch (g_aMissionObjectives[
                g_cCurrentObjective].type) {
    case 0:
    case 5:
        g_1_stElapsedCampaignDate.year =
            g_aMissionNavPoints[
                g_aMissionObjectives[
                    g_cCurrentObjective].index].systemIndex;
        return;
    default:
        navPoint = find_ships_sphere(
            g_aMissionObjectives[
                g_cCurrentObjective].index);
        g_1_stElapsedCampaignDate.year =
            g_aMissionNavPoints[navPoint].systemIndex;
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
    int frame;
    int markerColor;
    short done;
    short selectedNav;
    short objectivesVisible;
    short savedNav;
    short objective;

    done = 0;
    selectedNav = (short)g_cCurrentNavPointIndex;
    objectivesVisible = 0;
    savedNav = (short)g_cCurrentNavPointIndex;
    g_bInflightComputerActive = 1;
    savedCursor = g_37_stPersonnelMousePointerState;

    if (message_showing() != 0)
        EndCommMenu();
    DisableMouseCursorDrawing();
    g_18_nInterfaceEventMode = 0;
    free_view_buffer();
    g_1_cStoredViewDisplayModeValue = -1;
    cockpitShape = FetchDiskPacketRetrying("cockpit.vga", 1, 0);
    ClearViewport(&g_101_stScrViewInfo,
                  g_cSecondaryViewBufferColour);
    DrawSpriteDefault(&g_101_stScrViewInfo, 0, 0, cockpitShape, 0);
    ReleasePacketHandle(cockpitShape);
    SetInputViewport(&g_101_stScrViewInfo);
    g_1_stElapsedCampaignDate.year = g_nCurStellarSystem;
    BriefingMap_LoadShapes();
    ShowConfedNavScan(0);

    for (objective = 0;
         objective < (short)g_cMissionObjectiveCount;
         objective++) {
        if (hidden_objective(objective) == 0 &&
            IsObjectiveInDisplayedSystem(objective) != 0) {
            objectivesVisible = 1;
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
        navViewport = g_101_stScrViewInfo;
        SetRectBounds(&navViewport, 0x20, 0x18, 0xb6, 0x9f);
        SetInputViewport(&navViewport);
        SetMenuInputPump();
        g_88_nEventRepeatDelayState = 10;
        g_26_bMouseCursorEnabled = 6;
        CentreMouseOnCurrentNavObjective();

        do {
            if ((short)g_cCurrentNavPointIndex !=
                selectedNav) {
                selectedNav =
                    (short)g_cCurrentNavPointIndex;
                PlaySfxWaveFileByNumber(0x19, -1, 0);
                ShowConfedNavScan(1);
            }
            SetRectBounds(&g_101_stScrViewInfo, 0x20, 0x18,
                          0x121, 0xb1);
            if (g_4_pElapsedCampaignDatePointer == 0)
                g_4_pElapsedCampaignDatePointer =
                    &g_1_stElapsedCampaignDate;
            FormatNavCoordinates(
                (unsigned char *)g_4_pElapsedCampaignDatePointer);
            frame = g_196_nInputTimerTicks / 15;
            g_5_stNavLabelTextContextInfo.viewport =
                &g_101_stScrViewInfo;
            if (frame % 2 != 0)
                markerColor = g_bPrimaryViewBufferColour;
            else
                markerColor = g_ucHomeCarrierScannerColour;
            DrawNavPlayerMarker((short)markerColor, 0);
            g_stNavMapTextCtx.viewport =
                &g_101_stScrViewInfo;
            UpdateInflightNavText((short)((frame / 4) % 2));
            SetRectBounds(&g_101_stScrViewInfo, 0, 0, 0x13f, 0xc7);
            SetMouseCursorShape(
                g_18_pEventManager->cursorShape, 0);
            g_18_nInterfaceEventMode = 1;
            EnableMouseCursorDrawing();
            /* The original passes a second argument the callee ignores. */
            switch (PollInputEvent(&inputEvent, 0xf)) {
            case 4:
            case 6:
                if (inputEvent.status != 0x1c &&
                    inputEvent.status != 0x39) {
                    if (inputEvent.status == 0x31) {
                        cycle_next_objective();
                        if (IsObjectiveInDisplayedSystem(
                                (short)g_cCurrentObjective) == 0)
                            FollowSelectedObjectiveSystem();
                        CentreMouseOnCurrentNavObjective();
                    }
                    break;
                }
            case 1:
                done = 1;
                break;
            }
            SelectNavObjectiveAtPoint(
                (short)g_6_stHostMouseTextBlock.x,
                (short)g_6_stHostMouseTextBlock.y);
            MarkDibDirty();
            DIBslamReal();
        } while (done == 0 && g_bSceneEscapeRequested == 0);

        if (g_bSceneEscapeRequested != 0) {
            g_bSceneEscapeRequested = 0;
            g_cCurrentNavPointIndex =
                (signed char)savedNav;
            set_new_objective((short)g_cCurrentNavPointIndex);
        }
        free_viewport(&g_stSecondaryViewBufferBlock);
        DisableMouseCursorDrawing();
    }

    ReleasePacketHandle(g_pNavMapShape);
    SetTextContext(&g_3_stDefaultStringContext);
    PlaySfxWaveFileByNumber(0x19, -1, 0);
    g_37_stPersonnelMousePointerState = savedCursor;
    SetPersonnelMousePosition(g_37_stPersonnelMousePointerState.x,
                              g_37_stPersonnelMousePointerState.y);
    force_view(0, 0);
    InitializeSpaceFlightInput();
    g_bInflightComputerActive = 0;
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
short g_nCurStellarSystem; /* 0x005d169c */
short g_nNavPlotMaxX; /* 0x005d169e */
short g_nNavmapMapMinimumX; /* 0x005d16a0 */
unsigned short g_awNavObjectiveLabelIndex[8]; /* 0x005d16b0 */
short g_nNavmapMapMaximumY; /* 0x005d16c0 */
short g_13_nNavmapMapCentreX; /* 0x005d16c2 */
short g_2_nNavMapMinimumYValue; /* 0x005d16c4 */
TextContext g_stNavMapTextCtx; /* 0x005d16d0 */
short g_nNavMapCentreY; /* 0x005d16ec */
TextContext g_5_stNavLabelTextContextInfo; /* 0x005d16f0 */
CampaignDate g_1_stElapsedCampaignDate; /* 0x005d170c */
void *g_4_pDigiData; /* 0x005d1710 */
unsigned short g_wDigiLeft; /* 0x005d1714 */
