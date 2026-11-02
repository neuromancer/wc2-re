/*
 *  Asteroid and mine hazard fields.
 *
 *  Retail object 0x417610-0x41876F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* The cockpit layout packs the hazard pitch range into the two bytes of
 * field_f7: the low byte is the minimum, the high byte the maximum. */
#define g_acHazardPitchRange \
    ((const signed char *)&g_nHazardPitchRange_0049af98)

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

    if (g_asObjectScreenX_00493598[obj] == (short)0x8001) {
        return 0;
    } else {
        x = (short)(g_asObjectScreenX_00493598[obj] + g_nViewCenterX_005c80d8);
        y = (short)(g_asObjectScreenY_00493628[obj] + g_nViewCenterY_005c80da);
        return GetTransformedShapeBounds(
            &g_stViewBuffer_005d2b00, x, y, g_apObjectShape_00493868[obj],
            g_asObjectViewFrame_00493508[obj],
            g_asObjectScreenAngle_004936b8[obj],
            g_asObjectScreenScale_00493a58[obj],
            g_asObjectFlip_004939c8[obj], bounds);
    }
}

/* Function start: 0x4176D2 */
void make_shard(short asteroid, FixedVector direction)
{
    short fragment;

    fragment = find_vacant_3d_object();
    if (fragment != -1) {
        set_objects_data(fragment, OBJECT_DATA_ROCK_CHUNK, asteroid, 0);
        g_asObjectCounter_00494be0[fragment] = 40;
        g_acObjectOwner_00495208[fragment] = (signed char)asteroid;
        SetVectorFixedPoint(
            (unsigned int *)&direction,
            (short)(g_asObjectCollisionRadius_004950e8[asteroid] >> 1));
        AddFixedVectors(&g_aShipPosition_00494550[asteroid], &direction,
                        &g_aShipPosition_00494550[fragment]);
        g_aShipForwardVector_00494208[fragment] = direction;
        fix_objects_ijk(fragment);
        alter_yaw(signed_random(20), fragment);
        alter_pitch(signed_random(20), fragment);
        g_aShipVelocity_00494898[fragment] =
            g_aShipForwardVector_00494208[fragment];
        SetVectorFixedPoint(
            (unsigned int *)&g_aShipVelocity_00494898[fragment],
            (short)(real_velocity(asteroid) + RandomInRange(0, 5)));
        RecordCannedSceneObjectEvent(fragment, 0);
    }
}

/* Function start: 0x417838 */
void remove_hazard(signed char hazard)
{
    remove_object((short)hazard);
    g_nActiveHazards_00492e5c =
        MaxShort(0, (short)(g_nActiveHazards_00492e5c - 1));
    RecordCannedSceneObjectEvent((short)hazard, 1);
}

/* Function start: 0x41787A */
void remove_all_hazards(void)
{
    short slot;

    for (slot = 0; slot < 20; slot++) {
        if (g_abHazardObjects_00493280[slot] != -1) {
            remove_hazard(g_abHazardObjects_00493280[slot]);
            g_abHazardObjects_00493280[slot] = -1;
        }
    }
    g_pActiveHazardField_00493278 = 0;
}

#pragma function(abs)
/* Function start: 0x4178E5 */
short difficulty(void)
{
    return (short)(abs(25 - (int)g_nHazardReferenceSpeed_00492e58) * 2);
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
        saved = g_aShipRightVector_00493b78[obj];
        g_aShipRightVector_00493b78[obj] =
            g_aShipForwardVector_00494208[obj];
        g_aShipForwardVector_00494208[obj] =
            g_aShipUpVector_00493ec0[obj];
        g_aShipUpVector_00493ec0[obj] = saved;
    } else {
        saved = g_aShipUpVector_00493ec0[obj];
        g_aShipUpVector_00493ec0[obj] =
            g_aShipForwardVector_00494208[obj];
        g_aShipForwardVector_00494208[obj] =
            g_aShipRightVector_00493b78[obj];
        g_aShipRightVector_00493b78[obj] = saved;
    }
    if (allowReverse != 0 && RandomBelow(100) < 50)
        negate_vector(&g_aShipForwardVector_00494208[obj]);
}

/* Function start: 0x417AD7 */
void align(int *value, short quantum)
{
    *value -= *value % quantum;
}

/* Function start: 0x417AF9 */
void init_hazard(short obj, FixedVector position, short moving)
{
    short type;
    FixedVector vector;
    short hazardMoves;
    short travelTime;
    short speed;

    hazardMoves = moving;
    if (g_pActiveHazardField_00493278->type ==
        OBJECT_DATA_ASTEROID_FIELD) {
        type = (short)(OBJECT_DATA_ASTEROID1 +
                       RandomBelowOrEqual(5));
    } else {
        type = OBJECT_DATA_SPACE_MINE;
    }
    set_objects_data(obj, type, -1, 0);
    g_aShipPosition_00494550[obj] = position;

    if (type == OBJECT_DATA_SPACE_MINE) {
        hazardMoves = 0;
        point_at(obj, g_aShipPosition_00494550[0]);
        skew_randomly(obj, 1);
        speed = 2;
        hazardMoves = (short)(speed != 0);
    } else if (hazardMoves != 0) {
        travelTime = (short)(RandomBelowOrEqual(15) +
                             g_cHazardBaseTravelTime_0049af94);
        if (g_bUseEyePositionForHazards_0049327c != 0) {
            travelTime = (short)(travelTime -
                                 RandomBelowOrEqual(difficulty()));
            travelTime = MaxShort(45, travelTime);
        } else {
            travelTime = (short)(travelTime -
                                 RandomBelowOrEqual(difficulty()));
            travelTime = MaxShort(7, travelTime);
        }
        ScaleFixedVector(&g_aShipVelocity_00494898[0],
                         (int)travelTime << 8, &vector);
        AddFixedVectors(&g_aShipPosition_00494550[0], &vector, &vector);
        point_at(obj, vector);
        speed = distance_between_points(
            &vector, &g_aShipPosition_00494550[obj]);
        travelTime = MaxShort(3,
            (short)(travelTime - RandomBelow(5)));
        speed = (short)(speed / travelTime);
    } else {
        point_at(obj, g_pActiveHazardField_00493278->center);
        skew_randomly(obj, 1);
        if (RandomBelow(100) < 20)
            speed = 0;
        else
            speed = asteroid_velocity();
    }
    if (kilrathi_near(0, 16000) != 0)
        speed = 0;
    ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                     (int)speed << 8, &g_aShipVelocity_00494898[obj]);

    if (hazardMoves == 0) {
        int separation;

        if (type == OBJECT_DATA_ASTEROID_FIELD)
            separation = 1500;
        else
            separation = RandomBelowOrEqual(1000) << 8;
        ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                         separation, &vector);
        SubtractFixedVectors(&g_aShipPosition_00494550[obj], &vector,
                             &g_aShipPosition_00494550[obj]);
    }
    if (type == OBJECT_DATA_SPACE_MINE) {
        align(&g_aShipPosition_00494550[obj].x, 200);
        align(&g_aShipPosition_00494550[obj].y, 200);
        align(&g_aShipPosition_00494550[obj].z, 200);
    }
    g_acObjectCollisionGraceTicks_00494d48[obj] = 0;
    g_asObjectCounter_00494be0[obj] = 0;
    g_nActiveHazards_00492e5c++;
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
    g_aShipPosition_00494550[SCRATCH_VIEW_OBJECT] =
        g_aShipPosition_00494550[0];
    pitchDelta = signed_random(20);
    yawDelta = signed_random(35);
    if (g_nCurrentView_00492fa8 == 0) {
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
        -15, 15, g_anObjectPitchRotation_00494f38[0], -150, 150));
    yawDelta = (short)(yawDelta + find_ratio(
        -15, 15, g_anObjectYawRotation_00494fc8[0], -150, 150));
    rotate_about_j(yawDelta,
                   &g_aShipRightVector_00493b78[SCRATCH_VIEW_OBJECT],
                   &g_aShipForwardVector_00494208[SCRATCH_VIEW_OBJECT]);
    rotate_about_i(pitchDelta,
                   &g_aShipUpVector_00493ec0[SCRATCH_VIEW_OBJECT],
                   &g_aShipForwardVector_00494208[SCRATCH_VIEW_OBJECT]);
    position_relative_ijk(spot, SCRATCH_VIEW_OBJECT, 0, 0, 3050);
    usable = !IsPointWithinRange(&g_aShipPosition_00494550[0], spot, 3000);
    usable = usable != 0 &&
             within_field(g_pActiveHazardField_00493278, spot) != 0;
    return usable;
}

/* Function start: 0x418175 */
short rear_sphere(void)
{
    return find_ratio(0, 20, (short)g_nHazardReferenceSpeed_00492e58,
                      4300, 3100);
}

/* Function start: 0x4181A2 */
short ok_hazard_spot(short obj)
{
    int range;
    short result;

    if (g_asObjectScreenX_00493598[obj] == (short)0x8001)
        range = rear_sphere();
    else
        range = 4300;
    result = IsPointWithinRange(&g_aShipPosition_00494550[0],
                                &g_aShipPosition_00494550[obj],
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
    if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_DUST)
        g_aeObjectClass_00495328[obj] = OBJECT_CLASS_NULL;
}

/* Function start: 0x4182B6 */
void approach(short obj)
{
    FixedVector vector;

    ScaleFixedVector(&g_aShipVelocity_00494898[0], 20 << 8, &vector);
    AddFixedVectors(&g_aShipPosition_00494550[0], &vector, &vector);
    point_at(obj, vector);
    ScaleFixedVector(&g_aShipForwardVector_00494208[obj], 20 << 8,
                     &vector);
    AddFixedVectors(&g_aShipVelocity_00494898[obj], &vector,
                    &g_aShipVelocity_00494898[obj]);
    if (g_nCannedSceneMode_0049021c == 0)
        RecordCannedSceneObjectEvent(obj, 3);
}

/* Function start: 0x41836E */
void manage_hazard(short obj, short slot)
{
    if (g_nRenderedSpaceFrame_00493138 % 20 == slot) {
        if (ok_hazard_spot(obj) == 0) {
            remove_hazard((signed char)obj);
            return;
        }
        if (g_asObjectType_00495298[obj] == OBJECT_DATA_PORCUPINE_MINE &&
            g_asObjectScreenX_00493598[obj] != (short)0x8001 &&
            (unsigned short)g_asObjectDistance_00493ae8[obj] > 1500 &&
            real_velocity(obj) < 20)
            approach(obj);
    }
}

/* Function start: 0x418426 */
void match_ship_to_eye(void)
{
    g_nHazardReferenceSpeed_00492e58 = 100;
    g_aShipPosition_00494550[0] =
        g_aShipPosition_00494550[EYE_OBJECT];
    g_aShipRightVector_00493b78[0] =
        g_aShipRightVector_00493b78[EYE_OBJECT];
    g_aShipUpVector_00493ec0[0] =
        g_aShipUpVector_00493ec0[EYE_OBJECT];
    g_aShipForwardVector_00494208[0] =
        g_aShipForwardVector_00494208[EYE_OBJECT];
    ScaleFixedVector(&g_aShipForwardVector_00494208[0],
                     g_nHazardReferenceSpeed_00492e58 << 8,
                     &g_aShipVelocity_00494898[0]);
    g_pActiveHazardField_00493278->center =
        g_aShipPosition_00494550[EYE_OBJECT];
}

/* Function start: 0x4184F6 */
void update_hazards(void)
{
    short freeSlot;
    short slot;

    freeSlot = -1;

    if (g_bUseEyePositionForHazards_0049327c != 0)
        match_ship_to_eye();
    else
        g_nHazardReferenceSpeed_00492e58 = real_velocity(0);
    for (slot = 0; slot < 20; slot++) {
        if (g_abHazardObjects_00493280[slot] != -1)
            manage_hazard((short)g_abHazardObjects_00493280[slot], slot);
        else
            freeSlot = slot;
    }
    if (freeSlot != -1 &&
        RandomBelowOrEqual(215) <
            (short)g_nHazardReferenceSpeed_00492e58 + 30)
        g_abHazardObjects_00493280[freeSlot] = (signed char)make_hazard();
}

/* Function start: 0x4185CC */
void start_hazard_field(short region)
{
    short slot;

    slot = 0;
    remove_all_hazards();
    g_pActiveHazardField_00493278 = &g_aHazardFields_004931d8[region];
    while (slot++ < 3)
        g_abHazardObjects_00493280[slot] = (signed char)make_hazard();
}

/* Function start: 0x418626 */
void add_hazard_field(short type, FixedVector center,
                      short radius, short density)
{
    HazardField *field;

    if (g_nHazardFieldCount_004931d0 < 7) {
        field = &g_aHazardFields_004931d8[g_nHazardFieldCount_004931d0];
        field->type = type;
        field->center = center;
        field->outerRadius = radius;
        field->innerRadius = radius;
        field->density = density;
        g_nHazardFieldCount_004931d0++;
    }
}

/* Function start: 0x4186A4 */
void check_hazards(void)
{
    HazardField *field;
    short region;

    if (g_bUseEyePositionForHazards_0049327c != 0)
        return;
    if (g_pActiveHazardField_00493278 == 0) {
        field = g_aHazardFields_004931d8;
        for (region = 0; g_nHazardFieldCount_004931d0 > region;
             region++, field++) {
            if (field != g_pActiveHazardField_00493278 &&
                near_field(field, &g_aShipPosition_00494550[0]) != 0) {
                start_hazard_field(region);
                break;
            }
        }
    } else if (near_field(g_pActiveHazardField_00493278,
                          &g_aShipPosition_00494550[0]) == 0) {
        remove_all_hazards();
    }
}
