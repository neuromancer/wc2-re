/*
 *  Asteroid and mine hazard fields.
 *
 *  Retail object 0x417610-0x41876F (link order; see docs/ORDER.md).
 */
#include "game.h"

short g_nHazardReferenceSpeed = 0; /* 0x00492e58 */
short g_nActiveHazards = 0; /* 0x00492e5c */

/* The cockpit layout packs the hazard pitch range into the two bytes of
 * field_f7: the low byte is the minimum, the high byte the maximum. */
#define g_acHazardPitchRange \
    ((const signed char *)&g_nHazardPitchRange)

void make_shard(short asteroid, FixedVector direction);
void init_hazard(short obj, FixedVector position, short moving);
short try_far_spot(FixedVector *spot, short *moving);
void manage_hazard(short obj, short slot);
void extra_hazard(short obj);
void add_hazard_field(short type, FixedVector center, short radius,
                      short density);

/* Function start: 0x417610 */
short easy2see(short obj)
{
    short bounds[4];
    short x;
    short y;

    if (g_asObjectScreenX[obj] == (short)0x8001) {
        return 0;
    } else {
        x = (short)(g_asObjectScreenX[obj] + g_15_nStoredScreenViewCentreX);
        y = (short)(g_asObjectScreenY[obj] + g_37_nViewCenterY);
        return GetTransformedShapeBounds(
            &g_27_stViewScratchInfo, x, y, g_apObjectShape[obj],
            g_asObjectViewFrame[obj],
            g_asObjectScreenAngle[obj],
            g_asObjectScreenScale[obj],
            g_asObjectFlip[obj], bounds);
    }
}

/* Function start: 0x4176D2 */
void make_shard(short asteroid, FixedVector direction)
{
    short fragment;

    fragment = find_vacant_3d_object();
    if (fragment != -1) {
        set_objects_data(fragment, OBJECT_DATA_ROCK_CHUNK, asteroid, 0);
        g_asObjectCounter[fragment] = 40;
        g_acObjectOwner[fragment] = (signed char)asteroid;
        SetVectorFixedPoint(
            (unsigned int *)&direction,
            (short)(g_asObjectCollisionRadius[asteroid] >> 1));
        AddFixedVectors(&g_aShipPosition[asteroid], &direction,
                        &g_aShipPosition[fragment]);
        g_aShipForwardVector[fragment] = direction;
        fix_objects_ijk(fragment);
        alter_yaw(signed_random(20), fragment);
        alter_pitch(signed_random(20), fragment);
        g_aShipVelocity[fragment] =
            g_aShipForwardVector[fragment];
        SetVectorFixedPoint(
            (unsigned int *)&g_aShipVelocity[fragment],
            (short)(real_velocity(asteroid) + RandomInRange(0, 5)));
        RecordCannedSceneObjectEvent(fragment, 0);
    }
}

/* Function start: 0x417838 */
void remove_hazard(signed char hazard)
{
    remove_object((short)hazard);
    g_nActiveHazards =
        MaxShort(0, (short)(g_nActiveHazards - 1));
    RecordCannedSceneObjectEvent((short)hazard, 1);
}

/* Function start: 0x41787A */
void remove_all_hazards(void)
{
    short slot;

    for (slot = 0; slot < 20; slot++) {
        if (g_abHazardObjects[slot] != -1) {
            remove_hazard(g_abHazardObjects[slot]);
            g_abHazardObjects[slot] = -1;
        }
    }
    g_pActiveHazardField = 0;
}

#pragma function(abs)
/* Function start: 0x4178E5 */
short difficulty(void)
{
    return (short)(abs(25 - (int)g_nHazardReferenceSpeed) * 2);
}

/* Function start: 0x417916 */
short asteroid_velocity(void)
{
    return MinShort(20, (short)(RandomBelowOrEqual(7) + 10));
}

/* Function start: 0x417941 */
void skew_randomly(short obj, short allowReverse)
{
    FixedVector saved;

    if (RandomBelow(100) < 50) {
        saved = g_aShipRightVector[obj];
        g_aShipRightVector[obj] =
            g_aShipForwardVector[obj];
        g_aShipForwardVector[obj] =
            g_aShipUpVector[obj];
        g_aShipUpVector[obj] = saved;
    } else {
        saved = g_aShipUpVector[obj];
        g_aShipUpVector[obj] =
            g_aShipForwardVector[obj];
        g_aShipForwardVector[obj] =
            g_aShipRightVector[obj];
        g_aShipRightVector[obj] = saved;
    }
    if (allowReverse != 0 && RandomBelow(100) < 50)
        negate_vector(&g_aShipForwardVector[obj]);
}

/* Function start: 0x417AD7 */
void align(int *value, short quantum)
{
    *value -= *value % quantum;
}

/* Function start: 0x417AF9 */
void init_hazard(short obj, FixedVector position, short moving)
{
    short hazardType;
    FixedVector aimPoint;
    short moves;
    short travelTime;
    short velocity;

    moves = moving;
    if (g_pActiveHazardField->type ==
        OBJECT_DATA_ASTEROID_FIELD) {
        hazardType = (short)(OBJECT_DATA_ASTEROID1 +
                       RandomBelowOrEqual(5));
    } else {
        hazardType = OBJECT_DATA_SPACE_MINE;
    }
    set_objects_data(obj, hazardType, -1, 0);
    g_aShipPosition[obj] = position;

    if (hazardType == OBJECT_DATA_SPACE_MINE) {
        moves = 0;
        point_at(obj, g_aShipPosition[0]);
        skew_randomly(obj, 1);
        velocity = 2;
        moves = (short)(velocity == 0);
    } else if (moves != 0) {
        travelTime = (short)(RandomBelowOrEqual(15) +
                             g_cHazardBaseTravelTime);
        if (g_bUseEyePositionForHazards != 0) {
            travelTime = MaxShort(
                45, (short)(travelTime -
                            RandomBelowOrEqual(difficulty())));
        } else {
            travelTime = MaxShort(
                7, (short)(travelTime -
                           RandomBelowOrEqual(difficulty())));
        }
        ScaleFixedVector(&g_aShipVelocity[0],
                         (int)travelTime << 8, &aimPoint);
        AddFixedVectors(&g_aShipPosition[0], &aimPoint, &aimPoint);
        point_at(obj, aimPoint);
        velocity = (short)(distance_between_points(
                               &aimPoint, &g_aShipPosition[obj]) /
                           MaxShort(3, (short)(travelTime - RandomBelow(5))));
    } else {
        point_at(obj, g_pActiveHazardField->center);
        skew_randomly(obj, 1);
        if (RandomBelow(100) < 20)
            velocity = 0;
        else
            velocity = asteroid_velocity();
    }
    if (kilrathi_near(0, 16000) != 0)
        velocity = 0;
    ScaleFixedVector(&g_aShipForwardVector[obj],
                     (int)velocity << 8, &g_aShipVelocity[obj]);

    if (moves == 0) {
        int separation;

        if (hazardType == OBJECT_DATA_ASTEROID_FIELD)
            separation = 1500;
        else
            separation = RandomBelowOrEqual(1000) << 8;
        ScaleFixedVector(&g_aShipForwardVector[obj],
                         separation, &aimPoint);
        SubtractFixedVectors(&g_aShipPosition[obj], &aimPoint,
                             &g_aShipPosition[obj]);
    }
    if (hazardType == OBJECT_DATA_SPACE_MINE) {
        align(&g_aShipPosition[obj].x, 200);
        align(&g_aShipPosition[obj].y, 200);
        align(&g_aShipPosition[obj].z, 200);
    }
    g_acObjectCollisionGraceTicks[obj] = 0;
    g_asObjectCounter[obj] = 0;
    g_nActiveHazards++;
}

/* Function start: 0x417EA4 */
short near_field(const HazardField *field, const FixedVector *point)
{
    return IsPointWithinRange((FixedVector *)&field->center,
                              (FixedVector *)point,
                              (short)(field->innerRadius + 4300));
}

/* Function start: 0x417ED4 */
short within_field(const HazardField *field, const FixedVector *point)
{
    return IsPointWithinRange((FixedVector *)&field->center,
                              (FixedVector *)point, field->innerRadius);
}

/* Function start: 0x417EFF */
short try_far_spot(FixedVector *spot, short *moving)
{
    short yawDelta;
    short pitchDelta;
    short usable;

    copy_frame(0, SCRATCH_VIEW_OBJECT);
    g_aShipPosition[SCRATCH_VIEW_OBJECT] =
        g_aShipPosition[0];
    pitchDelta = signed_random(20);
    yawDelta = signed_random(35);
    if (g_nCurrentView == 0) {
        if (g_acHazardPitchRange[0] < pitchDelta &&
            g_acHazardPitchRange[1] > pitchDelta &&
            abs(yawDelta) < 19 &&
            RandomBelow(100) < 60)
            *moving = 1;
        else
            *moving = 0;
    } else {
        if (abs(pitchDelta) > 5 && abs(pitchDelta) < 20 &&
            abs(yawDelta) > 5 && abs(yawDelta) < 20 && RandomBelow(100) < 30)
            *moving = 1;
        else
            *moving = 0;
    }
    pitchDelta = (short)(pitchDelta + find_ratio(
        -15, 15, g_anObjectPitchRotation[0], -150, 150));
    yawDelta = (short)(yawDelta + find_ratio(
        -15, 15, g_anObjectYawRotation[0], -150, 150));
    rotate_about_j(yawDelta,
                   &g_aShipRightVector[SCRATCH_VIEW_OBJECT],
                   &g_aShipForwardVector[SCRATCH_VIEW_OBJECT]);
    rotate_about_i(pitchDelta,
                   &g_aShipUpVector[SCRATCH_VIEW_OBJECT],
                   &g_aShipForwardVector[SCRATCH_VIEW_OBJECT]);
    position_relative_ijk(spot, SCRATCH_VIEW_OBJECT, 0, 0, 3050);
    usable = !IsPointWithinRange(&g_aShipPosition[0], spot, 3000);
    usable = usable != 0 &&
             within_field(g_pActiveHazardField, spot) != 0;
    return usable;
}

/* Function start: 0x418175 */
short rear_sphere(void)
{
    return find_ratio(0, 20, (short)g_nHazardReferenceSpeed,
                      4300, 3100);
}

/* Function start: 0x4181A2 */
short ok_hazard_spot(short obj)
{
    int range;
    short result;

    if (g_asObjectScreenX[obj] == (short)0x8001)
        range = rear_sphere();
    else
        range = 4300;
    result = IsPointWithinRange(&g_aShipPosition[0],
                                &g_aShipPosition[obj],
                                (short)range);
    return result;
}

/* Function start: 0x41820B */
short make_hazard(void)
{
    FixedVector spot;
    short isMoving;
    short obj = find_vacant_3d_object();

    if (obj != -1 && try_far_spot(&spot, &isMoving) != 0) {
        init_hazard(obj, spot, isMoving);
        RecordCannedSceneObjectEvent(obj, 0);
    } else
        obj = -1;
    return obj;
}

/* Function start: 0x418288 */
void extra_hazard(short obj)
{
    if (g_aeObjectClass[obj] == OBJECT_CLASS_DUST)
        g_aeObjectClass[obj] = OBJECT_CLASS_NULL;
}

/* Function start: 0x4182B6 */
void approach(short obj)
{
    FixedVector vector;

    ScaleFixedVector(&g_aShipVelocity[0], 20 << 8, &vector);
    AddFixedVectors(&g_aShipPosition[0], &vector, &vector);
    point_at(obj, vector);
    ScaleFixedVector(&g_aShipForwardVector[obj], 20 << 8,
                     &vector);
    AddFixedVectors(&g_aShipVelocity[obj], &vector,
                    &g_aShipVelocity[obj]);
    if (g_nCannedSceneMode == 0)
        RecordCannedSceneObjectEvent(obj, 3);
}

/* Function start: 0x41836E */
void manage_hazard(short obj, short slot)
{
    if (g_nRenderedSpaceFrame % 20 == slot) {
        if (ok_hazard_spot(obj) == 0) {
            remove_hazard((signed char)obj);
            return;
        }
        if (g_asObjectType[obj] == OBJECT_DATA_PORCUPINE_MINE &&
            g_asObjectScreenX[obj] != (short)0x8001 &&
            (unsigned short)g_asObjectDistance[obj] > 1500 &&
            real_velocity(obj) < 20)
            approach(obj);
    }
}

/* Function start: 0x418426 */
void match_ship_to_eye(void)
{
    g_nHazardReferenceSpeed = 100;
    g_aShipPosition[0] =
        g_aShipPosition[EYE_OBJECT];
    g_aShipRightVector[0] =
        g_aShipRightVector[EYE_OBJECT];
    g_aShipUpVector[0] =
        g_aShipUpVector[EYE_OBJECT];
    g_aShipForwardVector[0] =
        g_aShipForwardVector[EYE_OBJECT];
    ScaleFixedVector(&g_aShipForwardVector[0],
                     g_nHazardReferenceSpeed << 8,
                     &g_aShipVelocity[0]);
    g_pActiveHazardField->center =
        g_aShipPosition[EYE_OBJECT];
}

/* Function start: 0x4184F6 */
void update_hazards(void)
{
    short freeSlot;
    short slot;

    freeSlot = -1;

    if (g_bUseEyePositionForHazards != 0)
        match_ship_to_eye();
    else
        g_nHazardReferenceSpeed = real_velocity(0);
    for (slot = 0; slot < 20; slot++) {
        if (g_abHazardObjects[slot] != -1)
            manage_hazard((short)g_abHazardObjects[slot], slot);
        else
            freeSlot = slot;
    }
    if (freeSlot != -1 &&
        RandomBelowOrEqual(215) <
            (short)g_nHazardReferenceSpeed + 30)
        g_abHazardObjects[freeSlot] = (signed char)make_hazard();
}

/* Function start: 0x4185CC */
void start_hazard_field(short region)
{
    short slot;

    slot = 0;
    remove_all_hazards();
    g_pActiveHazardField = &g_aHazardFields[region];
    while (slot++ < 3)
        g_abHazardObjects[slot] = (signed char)make_hazard();
}

/* Function start: 0x418626 */
void add_hazard_field(short type, FixedVector center,
                      short radius, short density)
{
    HazardField *field;

    if (g_nHazardFieldCount < 7) {
        field = &g_aHazardFields[g_nHazardFieldCount];
        field->type = type;
        field->center = center;
        field->outerRadius = radius;
        field->innerRadius = radius;
        field->density = density;
        g_nHazardFieldCount++;
    }
}

/* Function start: 0x4186A4 */
void check_hazards(void)
{
    HazardField *field;
    short region;

    if (g_bUseEyePositionForHazards != 0)
        return;
    if (g_pActiveHazardField == 0) {
        field = g_aHazardFields;
        for (region = 0; g_nHazardFieldCount > region;
             region++, field++) {
            if (field != g_pActiveHazardField &&
                near_field(field, &g_aShipPosition[0]) != 0) {
                start_hazard_field(region);
                break;
            }
        }
    } else if (near_field(g_pActiveHazardField,
                          &g_aShipPosition[0]) == 0) {
        remove_all_hazards();
    }
}
